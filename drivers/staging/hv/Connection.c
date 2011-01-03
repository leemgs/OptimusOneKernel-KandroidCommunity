
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include "osd.h"
#include "logging.h"
#include "VmbusPrivate.h"


struct VMBUS_CONNECTION gVmbusConnection = {
	.ConnectState		= Disconnected,
	.NextGpadlHandle	= ATOMIC_INIT(0xE1E10),
};


int VmbusConnect(void)
{
	int ret = 0;
	struct vmbus_channel_msginfo *msgInfo = NULL;
	struct vmbus_channel_initiate_contact *msg;
	unsigned long flags;

	DPRINT_ENTER(VMBUS);

	
	if (gVmbusConnection.ConnectState != Disconnected)
		return -1;

	
	gVmbusConnection.ConnectState = Connecting;
	gVmbusConnection.WorkQueue = create_workqueue("hv_vmbus_con");
	if (!gVmbusConnection.WorkQueue) {
		ret = -1;
		goto Cleanup;
	}

	INIT_LIST_HEAD(&gVmbusConnection.ChannelMsgList);
	spin_lock_init(&gVmbusConnection.channelmsg_lock);

	INIT_LIST_HEAD(&gVmbusConnection.ChannelList);
	spin_lock_init(&gVmbusConnection.channel_lock);

	
	gVmbusConnection.InterruptPage = osd_PageAlloc(1);
	if (gVmbusConnection.InterruptPage == NULL) {
		ret = -1;
		goto Cleanup;
	}

	gVmbusConnection.RecvInterruptPage = gVmbusConnection.InterruptPage;
	gVmbusConnection.SendInterruptPage =
		(void *)((unsigned long)gVmbusConnection.InterruptPage +
			(PAGE_SIZE >> 1));

	
	gVmbusConnection.MonitorPages = osd_PageAlloc(2);
	if (gVmbusConnection.MonitorPages == NULL) {
		ret = -1;
		goto Cleanup;
	}

	msgInfo = kzalloc(sizeof(*msgInfo) +
			  sizeof(struct vmbus_channel_initiate_contact),
			  GFP_KERNEL);
	if (msgInfo == NULL) {
		ret = -1;
		goto Cleanup;
	}

	msgInfo->WaitEvent = osd_WaitEventCreate();
	msg = (struct vmbus_channel_initiate_contact *)msgInfo->Msg;

	msg->Header.MessageType = ChannelMessageInitiateContact;
	msg->VMBusVersionRequested = VMBUS_REVISION_NUMBER;
	msg->InterruptPage = virt_to_phys(gVmbusConnection.InterruptPage);
	msg->MonitorPage1 = virt_to_phys(gVmbusConnection.MonitorPages);
	msg->MonitorPage2 = virt_to_phys(
			(void *)((unsigned long)gVmbusConnection.MonitorPages +
				 PAGE_SIZE));

	
	spin_lock_irqsave(&gVmbusConnection.channelmsg_lock, flags);
	list_add_tail(&msgInfo->MsgListEntry,
		      &gVmbusConnection.ChannelMsgList);

	spin_unlock_irqrestore(&gVmbusConnection.channelmsg_lock, flags);

	DPRINT_DBG(VMBUS, "Vmbus connection - interrupt pfn %llx, "
		   "monitor1 pfn %llx,, monitor2 pfn %llx",
		   msg->InterruptPage, msg->MonitorPage1, msg->MonitorPage2);

	DPRINT_DBG(VMBUS, "Sending channel initiate msg...");
	ret = VmbusPostMessage(msg,
			       sizeof(struct vmbus_channel_initiate_contact));
	if (ret != 0) {
		list_del(&msgInfo->MsgListEntry);
		goto Cleanup;
	}

	
	osd_WaitEventWait(msgInfo->WaitEvent);

	list_del(&msgInfo->MsgListEntry);

	
	if (msgInfo->Response.VersionResponse.VersionSupported) {
		DPRINT_INFO(VMBUS, "Vmbus connected!!");
		gVmbusConnection.ConnectState = Connected;

	} else {
		DPRINT_ERR(VMBUS, "Vmbus connection failed!!..."
			   "current version (%d) not supported",
			   VMBUS_REVISION_NUMBER);
		ret = -1;
		goto Cleanup;
	}

	kfree(msgInfo->WaitEvent);
	kfree(msgInfo);
	DPRINT_EXIT(VMBUS);

	return 0;

Cleanup:
	gVmbusConnection.ConnectState = Disconnected;

	if (gVmbusConnection.WorkQueue)
		destroy_workqueue(gVmbusConnection.WorkQueue);

	if (gVmbusConnection.InterruptPage) {
		osd_PageFree(gVmbusConnection.InterruptPage, 1);
		gVmbusConnection.InterruptPage = NULL;
	}

	if (gVmbusConnection.MonitorPages) {
		osd_PageFree(gVmbusConnection.MonitorPages, 2);
		gVmbusConnection.MonitorPages = NULL;
	}

	if (msgInfo) {
		kfree(msgInfo->WaitEvent);
		kfree(msgInfo);
	}

	DPRINT_EXIT(VMBUS);

	return ret;
}


int VmbusDisconnect(void)
{
	int ret = 0;
	struct vmbus_channel_message_header *msg;

	DPRINT_ENTER(VMBUS);

	
	if (gVmbusConnection.ConnectState != Connected)
		return -1;

	msg = kzalloc(sizeof(struct vmbus_channel_message_header), GFP_KERNEL);

	msg->MessageType = ChannelMessageUnload;

	ret = VmbusPostMessage(msg,
			       sizeof(struct vmbus_channel_message_header));
	if (ret != 0)
		goto Cleanup;

	osd_PageFree(gVmbusConnection.InterruptPage, 1);

	
	destroy_workqueue(gVmbusConnection.WorkQueue);

	gVmbusConnection.ConnectState = Disconnected;

	DPRINT_INFO(VMBUS, "Vmbus disconnected!!");

Cleanup:
	kfree(msg);
	DPRINT_EXIT(VMBUS);
	return ret;
}


struct vmbus_channel *GetChannelFromRelId(u32 relId)
{
	struct vmbus_channel *channel;
	struct vmbus_channel *foundChannel  = NULL;
	unsigned long flags;

	spin_lock_irqsave(&gVmbusConnection.channel_lock, flags);
	list_for_each_entry(channel, &gVmbusConnection.ChannelList, ListEntry) {
		if (channel->OfferMsg.ChildRelId == relId) {
			foundChannel = channel;
			break;
		}
	}
	spin_unlock_irqrestore(&gVmbusConnection.channel_lock, flags);

	return foundChannel;
}


static void VmbusProcessChannelEvent(void *context)
{
	struct vmbus_channel *channel;
	u32 relId = (u32)(unsigned long)context;

	ASSERT(relId > 0);

	
	channel = GetChannelFromRelId(relId);

	if (channel) {
		VmbusChannelOnChannelEvent(channel);
		
	} else {
		DPRINT_ERR(VMBUS, "channel not found for relid - %d.", relId);
	}
}


void VmbusOnEvents(void)
{
	int dword;
	int maxdword = MAX_NUM_CHANNELS_SUPPORTED >> 5;
	int bit;
	int relid;
	u32 *recvInterruptPage = gVmbusConnection.RecvInterruptPage;

	DPRINT_ENTER(VMBUS);

	
	if (recvInterruptPage) {
		for (dword = 0; dword < maxdword; dword++) {
			if (recvInterruptPage[dword]) {
				for (bit = 0; bit < 32; bit++) {
					if (test_and_clear_bit(bit, (unsigned long *)&recvInterruptPage[dword])) {
						relid = (dword << 5) + bit;
						DPRINT_DBG(VMBUS, "event detected for relid - %d", relid);

						if (relid == 0) {
							
							DPRINT_DBG(VMBUS, "invalid relid - %d", relid);
							continue;
						} else {
							
							
							VmbusProcessChannelEvent((void *)(unsigned long)relid);
						}
					}
				}
			}
		 }
	}
	DPRINT_EXIT(VMBUS);

	return;
}


int VmbusPostMessage(void *buffer, size_t bufferLen)
{
	union hv_connection_id connId;

	connId.Asu32 = 0;
	connId.u.Id = VMBUS_MESSAGE_CONNECTION_ID;
	return HvPostMessage(connId, 1, buffer, bufferLen);
}


int VmbusSetEvent(u32 childRelId)
{
	int ret = 0;

	DPRINT_ENTER(VMBUS);

	
	set_bit(childRelId & 31,
		(unsigned long *)gVmbusConnection.SendInterruptPage +
		(childRelId >> 5));

	ret = HvSignalEvent();

	DPRINT_EXIT(VMBUS);

	return ret;
}
