
#include <linux/kernel.h>
#include <linux/mm.h>
#include "osd.h"
#include "logging.h"
#include "VmbusPrivate.h"


static int VmbusChannelCreateGpadlHeader(
	void *Kbuffer,	
	u32 Size,	
	struct vmbus_channel_msginfo **msgInfo,
	u32 *MessageCount);
static void DumpVmbusChannel(struct vmbus_channel *channel);
static void VmbusChannelSetEvent(struct vmbus_channel *channel);


#if 0
static void DumpMonitorPage(struct hv_monitor_page *MonitorPage)
{
	int i = 0;
	int j = 0;

	DPRINT_DBG(VMBUS, "monitorPage - %p, trigger state - %d",
		   MonitorPage, MonitorPage->TriggerState);

	for (i = 0; i < 4; i++)
		DPRINT_DBG(VMBUS, "trigger group (%d) - %llx", i,
			   MonitorPage->TriggerGroup[i].AsUINT64);

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 32; j++) {
			DPRINT_DBG(VMBUS, "latency (%d)(%d) - %llx", i, j,
				   MonitorPage->Latency[i][j]);
		}
	}
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 32; j++) {
			DPRINT_DBG(VMBUS, "param-conn id (%d)(%d) - %d", i, j,
			       MonitorPage->Parameter[i][j].ConnectionId.Asu32);
			DPRINT_DBG(VMBUS, "param-flag (%d)(%d) - %d", i, j,
				MonitorPage->Parameter[i][j].FlagNumber);
		}
	}
}
#endif


static void VmbusChannelSetEvent(struct vmbus_channel *Channel)
{
	struct hv_monitor_page *monitorPage;

	DPRINT_ENTER(VMBUS);

	if (Channel->OfferMsg.MonitorAllocated) {
		
		set_bit(Channel->OfferMsg.ChildRelId & 31,
			(unsigned long *) gVmbusConnection.SendInterruptPage +
			(Channel->OfferMsg.ChildRelId >> 5));

		monitorPage = gVmbusConnection.MonitorPages;
		monitorPage++; 

		set_bit(Channel->MonitorBit,
			(unsigned long *)&monitorPage->TriggerGroup
					[Channel->MonitorGroup].Pending);

	} else {
		VmbusSetEvent(Channel->OfferMsg.ChildRelId);
	}

	DPRINT_EXIT(VMBUS);
}

#if 0
static void VmbusChannelClearEvent(struct vmbus_channel *channel)
{
	struct hv_monitor_page *monitorPage;

	DPRINT_ENTER(VMBUS);

	if (Channel->OfferMsg.MonitorAllocated) {
		
		clear_bit(Channel->OfferMsg.ChildRelId & 31,
			  (unsigned long *)gVmbusConnection.SendInterruptPage +
			  (Channel->OfferMsg.ChildRelId >> 5));

		monitorPage =
			(struct hv_monitor_page *)gVmbusConnection.MonitorPages;
		monitorPage++; 

		clear_bit(Channel->MonitorBit,
			  (unsigned long *)&monitorPage->TriggerGroup
					[Channel->MonitorGroup].Pending);
	}

	DPRINT_EXIT(VMBUS);
}

#endif

void VmbusChannelGetDebugInfo(struct vmbus_channel *Channel,
			      struct vmbus_channel_debug_info *DebugInfo)
{
	struct hv_monitor_page *monitorPage;
	u8 monitorGroup = (u8)Channel->OfferMsg.MonitorId / 32;
	u8 monitorOffset = (u8)Channel->OfferMsg.MonitorId % 32;
	

	DebugInfo->RelId = Channel->OfferMsg.ChildRelId;
	DebugInfo->State = Channel->State;
	memcpy(&DebugInfo->InterfaceType,
	       &Channel->OfferMsg.Offer.InterfaceType, sizeof(struct hv_guid));
	memcpy(&DebugInfo->InterfaceInstance,
	       &Channel->OfferMsg.Offer.InterfaceInstance,
	       sizeof(struct hv_guid));

	monitorPage = (struct hv_monitor_page *)gVmbusConnection.MonitorPages;

	DebugInfo->MonitorId = Channel->OfferMsg.MonitorId;

	DebugInfo->ServerMonitorPending =
			monitorPage->TriggerGroup[monitorGroup].Pending;
	DebugInfo->ServerMonitorLatency =
			monitorPage->Latency[monitorGroup][monitorOffset];
	DebugInfo->ServerMonitorConnectionId =
			monitorPage->Parameter[monitorGroup]
					      [monitorOffset].ConnectionId.u.Id;

	monitorPage++;

	DebugInfo->ClientMonitorPending =
			monitorPage->TriggerGroup[monitorGroup].Pending;
	DebugInfo->ClientMonitorLatency =
			monitorPage->Latency[monitorGroup][monitorOffset];
	DebugInfo->ClientMonitorConnectionId =
			monitorPage->Parameter[monitorGroup]
					      [monitorOffset].ConnectionId.u.Id;

	RingBufferGetDebugInfo(&Channel->Inbound, &DebugInfo->Inbound);
	RingBufferGetDebugInfo(&Channel->Outbound, &DebugInfo->Outbound);
}


int VmbusChannelOpen(struct vmbus_channel *NewChannel, u32 SendRingBufferSize,
		     u32 RecvRingBufferSize, void *UserData, u32 UserDataLen,
		     void (*OnChannelCallback)(void *context), void *Context)
{
	struct vmbus_channel_open_channel *openMsg;
	struct vmbus_channel_msginfo *openInfo;
	void *in, *out;
	unsigned long flags;
	int ret;

	DPRINT_ENTER(VMBUS);

	
	ASSERT(!(SendRingBufferSize & (PAGE_SIZE - 1)));
	ASSERT(!(RecvRingBufferSize & (PAGE_SIZE - 1)));

	NewChannel->OnChannelCallback = OnChannelCallback;
	NewChannel->ChannelCallbackContext = Context;

	
	out = osd_PageAlloc((SendRingBufferSize + RecvRingBufferSize)
			     >> PAGE_SHIFT);
	ASSERT(out);
	ASSERT(((unsigned long)out & (PAGE_SIZE-1)) == 0);

	in = (void *)((unsigned long)out + SendRingBufferSize);

	NewChannel->RingBufferPages = out;
	NewChannel->RingBufferPageCount = (SendRingBufferSize +
					   RecvRingBufferSize) >> PAGE_SHIFT;

	RingBufferInit(&NewChannel->Outbound, out, SendRingBufferSize);

	RingBufferInit(&NewChannel->Inbound, in, RecvRingBufferSize);

	
	DPRINT_DBG(VMBUS, "Establishing ring buffer's gpadl for channel %p...",
		   NewChannel);

	NewChannel->RingBufferGpadlHandle = 0;

	ret = VmbusChannelEstablishGpadl(NewChannel,
					 NewChannel->Outbound.RingBuffer,
					 SendRingBufferSize +
					 RecvRingBufferSize,
					 &NewChannel->RingBufferGpadlHandle);

	DPRINT_DBG(VMBUS, "channel %p <relid %d gpadl 0x%x send ring %p "
		   "size %d recv ring %p size %d, downstreamoffset %d>",
		   NewChannel, NewChannel->OfferMsg.ChildRelId,
		   NewChannel->RingBufferGpadlHandle,
		   NewChannel->Outbound.RingBuffer,
		   NewChannel->Outbound.RingSize,
		   NewChannel->Inbound.RingBuffer,
		   NewChannel->Inbound.RingSize,
		   SendRingBufferSize);

	
	openInfo = kmalloc(sizeof(*openInfo) +
			   sizeof(struct vmbus_channel_open_channel),
			   GFP_KERNEL);
	ASSERT(openInfo != NULL);

	openInfo->WaitEvent = osd_WaitEventCreate();

	openMsg = (struct vmbus_channel_open_channel *)openInfo->Msg;
	openMsg->Header.MessageType = ChannelMessageOpenChannel;
	openMsg->OpenId = NewChannel->OfferMsg.ChildRelId; 
	openMsg->ChildRelId = NewChannel->OfferMsg.ChildRelId;
	openMsg->RingBufferGpadlHandle = NewChannel->RingBufferGpadlHandle;
	ASSERT(openMsg->RingBufferGpadlHandle);
	openMsg->DownstreamRingBufferPageOffset = SendRingBufferSize >>
						  PAGE_SHIFT;
	openMsg->ServerContextAreaGpadlHandle = 0; 

	ASSERT(UserDataLen <= MAX_USER_DEFINED_BYTES);
	if (UserDataLen)
		memcpy(openMsg->UserData, UserData, UserDataLen);

	spin_lock_irqsave(&gVmbusConnection.channelmsg_lock, flags);
	list_add_tail(&openInfo->MsgListEntry,
		      &gVmbusConnection.ChannelMsgList);
	spin_unlock_irqrestore(&gVmbusConnection.channelmsg_lock, flags);

	DPRINT_DBG(VMBUS, "Sending channel open msg...");

	ret = VmbusPostMessage(openMsg,
			       sizeof(struct vmbus_channel_open_channel));
	if (ret != 0) {
		DPRINT_ERR(VMBUS, "unable to open channel - %d", ret);
		goto Cleanup;
	}

	
	osd_WaitEventWait(openInfo->WaitEvent);

	if (openInfo->Response.OpenResult.Status == 0)
		DPRINT_INFO(VMBUS, "channel <%p> open success!!", NewChannel);
	else
		DPRINT_INFO(VMBUS, "channel <%p> open failed - %d!!",
			    NewChannel, openInfo->Response.OpenResult.Status);

Cleanup:
	spin_lock_irqsave(&gVmbusConnection.channelmsg_lock, flags);
	list_del(&openInfo->MsgListEntry);
	spin_unlock_irqrestore(&gVmbusConnection.channelmsg_lock, flags);

	kfree(openInfo->WaitEvent);
	kfree(openInfo);

	DPRINT_EXIT(VMBUS);

	return 0;
}


static void DumpGpadlBody(struct vmbus_channel_gpadl_body *Gpadl, u32 Len)
{
	int i;
	int pfnCount;

	pfnCount = (Len - sizeof(struct vmbus_channel_gpadl_body)) /
		   sizeof(u64);
	DPRINT_DBG(VMBUS, "gpadl body - len %d pfn count %d", Len, pfnCount);

	for (i = 0; i < pfnCount; i++)
		DPRINT_DBG(VMBUS, "gpadl body  - %d) pfn %llu",
			   i, Gpadl->Pfn[i]);
}


static void DumpGpadlHeader(struct vmbus_channel_gpadl_header *Gpadl)
{
	int i, j;
	int pageCount;

	DPRINT_DBG(VMBUS,
		   "gpadl header - relid %d, range count %d, range buflen %d",
		   Gpadl->ChildRelId, Gpadl->RangeCount, Gpadl->RangeBufLen);
	for (i = 0; i < Gpadl->RangeCount; i++) {
		pageCount = Gpadl->Range[i].ByteCount >> PAGE_SHIFT;
		pageCount = (pageCount > 26) ? 26 : pageCount;

		DPRINT_DBG(VMBUS, "gpadl range %d - len %d offset %d "
			   "page count %d", i, Gpadl->Range[i].ByteCount,
			   Gpadl->Range[i].ByteOffset, pageCount);

		for (j = 0; j < pageCount; j++)
			DPRINT_DBG(VMBUS, "%d) pfn %llu", j,
				   Gpadl->Range[i].PfnArray[j]);
	}
}


static int VmbusChannelCreateGpadlHeader(void *Kbuffer, u32 Size,
					 struct vmbus_channel_msginfo **MsgInfo,
					 u32 *MessageCount)
{
	int i;
	int pageCount;
	unsigned long long pfn;
	struct vmbus_channel_gpadl_header *gpaHeader;
	struct vmbus_channel_gpadl_body *gpadlBody;
	struct vmbus_channel_msginfo *msgHeader;
	struct vmbus_channel_msginfo *msgBody;
	u32 msgSize;

	int pfnSum, pfnCount, pfnLeft, pfnCurr, pfnSize;

	
	ASSERT((Size & (PAGE_SIZE-1)) == 0);

	pageCount = Size >> PAGE_SHIFT;
	pfn = virt_to_phys(Kbuffer) >> PAGE_SHIFT;

	
	pfnSize = MAX_SIZE_CHANNEL_MESSAGE -
		  sizeof(struct vmbus_channel_gpadl_header) -
		  sizeof(struct gpa_range);
	pfnCount = pfnSize / sizeof(u64);

	if (pageCount > pfnCount) {
		
		
		msgSize = sizeof(struct vmbus_channel_msginfo) +
			  sizeof(struct vmbus_channel_gpadl_header) +
			  sizeof(struct gpa_range) + pfnCount * sizeof(u64);
		msgHeader =  kzalloc(msgSize, GFP_KERNEL);

		INIT_LIST_HEAD(&msgHeader->SubMsgList);
		msgHeader->MessageSize = msgSize;

		gpaHeader = (struct vmbus_channel_gpadl_header *)msgHeader->Msg;
		gpaHeader->RangeCount = 1;
		gpaHeader->RangeBufLen = sizeof(struct gpa_range) +
					 pageCount * sizeof(u64);
		gpaHeader->Range[0].ByteOffset = 0;
		gpaHeader->Range[0].ByteCount = Size;
		for (i = 0; i < pfnCount; i++)
			gpaHeader->Range[0].PfnArray[i] = pfn+i;
		*MsgInfo = msgHeader;
		*MessageCount = 1;

		pfnSum = pfnCount;
		pfnLeft = pageCount - pfnCount;

		
		pfnSize = MAX_SIZE_CHANNEL_MESSAGE -
			  sizeof(struct vmbus_channel_gpadl_body);
		pfnCount = pfnSize / sizeof(u64);

		
		while (pfnLeft) {
			if (pfnLeft > pfnCount)
				pfnCurr = pfnCount;
			else
				pfnCurr = pfnLeft;

			msgSize = sizeof(struct vmbus_channel_msginfo) +
				  sizeof(struct vmbus_channel_gpadl_body) +
				  pfnCurr * sizeof(u64);
			msgBody = kzalloc(msgSize, GFP_KERNEL);
			ASSERT(msgBody);
			msgBody->MessageSize = msgSize;
			(*MessageCount)++;
			gpadlBody =
				(struct vmbus_channel_gpadl_body *)msgBody->Msg;

			
			
			for (i = 0; i < pfnCurr; i++)
				gpadlBody->Pfn[i] = pfn + pfnSum + i;

			
			list_add_tail(&msgBody->MsgListEntry,
				      &msgHeader->SubMsgList);
			pfnSum += pfnCurr;
			pfnLeft -= pfnCurr;
		}
	} else {
		
		msgSize = sizeof(struct vmbus_channel_msginfo) +
			  sizeof(struct vmbus_channel_gpadl_header) +
			  sizeof(struct gpa_range) + pageCount * sizeof(u64);
		msgHeader = kzalloc(msgSize, GFP_KERNEL);
		msgHeader->MessageSize = msgSize;

		gpaHeader = (struct vmbus_channel_gpadl_header *)msgHeader->Msg;
		gpaHeader->RangeCount = 1;
		gpaHeader->RangeBufLen = sizeof(struct gpa_range) +
					 pageCount * sizeof(u64);
		gpaHeader->Range[0].ByteOffset = 0;
		gpaHeader->Range[0].ByteCount = Size;
		for (i = 0; i < pageCount; i++)
			gpaHeader->Range[0].PfnArray[i] = pfn+i;

		*MsgInfo = msgHeader;
		*MessageCount = 1;
	}

	return 0;
}


int VmbusChannelEstablishGpadl(struct vmbus_channel *Channel, void *Kbuffer,
			       u32 Size, u32 *GpadlHandle)
{
	struct vmbus_channel_gpadl_header *gpadlMsg;
	struct vmbus_channel_gpadl_body *gpadlBody;
	
	struct vmbus_channel_msginfo *msgInfo;
	struct vmbus_channel_msginfo *subMsgInfo;
	u32 msgCount;
	struct list_head *curr;
	u32 nextGpadlHandle;
	unsigned long flags;
	int ret;

	DPRINT_ENTER(VMBUS);

	nextGpadlHandle = atomic_read(&gVmbusConnection.NextGpadlHandle);
	atomic_inc(&gVmbusConnection.NextGpadlHandle);

	VmbusChannelCreateGpadlHeader(Kbuffer, Size, &msgInfo, &msgCount);
	ASSERT(msgInfo != NULL);
	ASSERT(msgCount > 0);

	msgInfo->WaitEvent = osd_WaitEventCreate();
	gpadlMsg = (struct vmbus_channel_gpadl_header *)msgInfo->Msg;
	gpadlMsg->Header.MessageType = ChannelMessageGpadlHeader;
	gpadlMsg->ChildRelId = Channel->OfferMsg.ChildRelId;
	gpadlMsg->Gpadl = nextGpadlHandle;

	DumpGpadlHeader(gpadlMsg);

	spin_lock_irqsave(&gVmbusConnection.channelmsg_lock, flags);
	list_add_tail(&msgInfo->MsgListEntry,
		      &gVmbusConnection.ChannelMsgList);

	spin_unlock_irqrestore(&gVmbusConnection.channelmsg_lock, flags);
	DPRINT_DBG(VMBUS, "buffer %p, size %d msg cnt %d",
		   Kbuffer, Size, msgCount);

	DPRINT_DBG(VMBUS, "Sending GPADL Header - len %zd",
		   msgInfo->MessageSize - sizeof(*msgInfo));

	ret = VmbusPostMessage(gpadlMsg, msgInfo->MessageSize -
			       sizeof(*msgInfo));
	if (ret != 0) {
		DPRINT_ERR(VMBUS, "Unable to open channel - %d", ret);
		goto Cleanup;
	}

	if (msgCount > 1) {
		list_for_each(curr, &msgInfo->SubMsgList) {

			
			subMsgInfo = (struct vmbus_channel_msginfo *)curr;
			gpadlBody =
			     (struct vmbus_channel_gpadl_body *)subMsgInfo->Msg;

			gpadlBody->Header.MessageType = ChannelMessageGpadlBody;
			gpadlBody->Gpadl = nextGpadlHandle;

			DPRINT_DBG(VMBUS, "Sending GPADL Body - len %zd",
				   subMsgInfo->MessageSize -
				   sizeof(*subMsgInfo));

			DumpGpadlBody(gpadlBody, subMsgInfo->MessageSize -
				      sizeof(*subMsgInfo));
			ret = VmbusPostMessage(gpadlBody,
					       subMsgInfo->MessageSize -
					       sizeof(*subMsgInfo));
			ASSERT(ret == 0);
		}
	}
	osd_WaitEventWait(msgInfo->WaitEvent);

	
	DPRINT_DBG(VMBUS, "Received GPADL created "
		   "(relid %d, status %d handle %x)",
		   Channel->OfferMsg.ChildRelId,
		   msgInfo->Response.GpadlCreated.CreationStatus,
		   gpadlMsg->Gpadl);

	*GpadlHandle = gpadlMsg->Gpadl;

Cleanup:
	spin_lock_irqsave(&gVmbusConnection.channelmsg_lock, flags);
	list_del(&msgInfo->MsgListEntry);
	spin_unlock_irqrestore(&gVmbusConnection.channelmsg_lock, flags);

	kfree(msgInfo->WaitEvent);
	kfree(msgInfo);

	DPRINT_EXIT(VMBUS);

	return ret;
}


int VmbusChannelTeardownGpadl(struct vmbus_channel *Channel, u32 GpadlHandle)
{
	struct vmbus_channel_gpadl_teardown *msg;
	struct vmbus_channel_msginfo *info;
	unsigned long flags;
	int ret;

	DPRINT_ENTER(VMBUS);

	ASSERT(GpadlHandle != 0);

	info = kmalloc(sizeof(*info) +
		       sizeof(struct vmbus_channel_gpadl_teardown), GFP_KERNEL);
	ASSERT(info != NULL);

	info->WaitEvent = osd_WaitEventCreate();

	msg = (struct vmbus_channel_gpadl_teardown *)info->Msg;

	msg->Header.MessageType = ChannelMessageGpadlTeardown;
	msg->ChildRelId = Channel->OfferMsg.ChildRelId;
	msg->Gpadl = GpadlHandle;

	spin_lock_irqsave(&gVmbusConnection.channelmsg_lock, flags);
	list_add_tail(&info->MsgListEntry,
		      &gVmbusConnection.ChannelMsgList);
	spin_unlock_irqrestore(&gVmbusConnection.channelmsg_lock, flags);

	ret = VmbusPostMessage(msg,
			       sizeof(struct vmbus_channel_gpadl_teardown));
	if (ret != 0) {
		
		
	}

	osd_WaitEventWait(info->WaitEvent);

	
	spin_lock_irqsave(&gVmbusConnection.channelmsg_lock, flags);
	list_del(&info->MsgListEntry);
	spin_unlock_irqrestore(&gVmbusConnection.channelmsg_lock, flags);

	kfree(info->WaitEvent);
	kfree(info);

	DPRINT_EXIT(VMBUS);

	return ret;
}


void VmbusChannelClose(struct vmbus_channel *Channel)
{
	struct vmbus_channel_close_channel *msg;
	struct vmbus_channel_msginfo *info;
	unsigned long flags;
	int ret;

	DPRINT_ENTER(VMBUS);

	
	Channel->OnChannelCallback = NULL;
	del_timer_sync(&Channel->poll_timer);

	
	info = kmalloc(sizeof(*info) +
		       sizeof(struct vmbus_channel_close_channel), GFP_KERNEL);
	ASSERT(info != NULL);

	

	msg = (struct vmbus_channel_close_channel *)info->Msg;
	msg->Header.MessageType = ChannelMessageCloseChannel;
	msg->ChildRelId = Channel->OfferMsg.ChildRelId;

	ret = VmbusPostMessage(msg, sizeof(struct vmbus_channel_close_channel));
	if (ret != 0) {
		
		
	}

	
	if (Channel->RingBufferGpadlHandle)
		VmbusChannelTeardownGpadl(Channel,
					  Channel->RingBufferGpadlHandle);

	

	
	RingBufferCleanup(&Channel->Outbound);
	RingBufferCleanup(&Channel->Inbound);

	osd_PageFree(Channel->RingBufferPages, Channel->RingBufferPageCount);

	kfree(info);

	

	if (Channel->State == CHANNEL_OPEN_STATE) {
		spin_lock_irqsave(&gVmbusConnection.channel_lock, flags);
		list_del(&Channel->ListEntry);
		spin_unlock_irqrestore(&gVmbusConnection.channel_lock, flags);

		FreeVmbusChannel(Channel);
	}

	DPRINT_EXIT(VMBUS);
}


int VmbusChannelSendPacket(struct vmbus_channel *Channel, const void *Buffer,
			   u32 BufferLen, u64 RequestId,
			   enum vmbus_packet_type Type, u32 Flags)
{
	struct vmpacket_descriptor desc;
	u32 packetLen = sizeof(struct vmpacket_descriptor) + BufferLen;
	u32 packetLenAligned = ALIGN_UP(packetLen, sizeof(u64));
	struct scatterlist bufferList[3];
	u64 alignedData = 0;
	int ret;

	DPRINT_ENTER(VMBUS);
	DPRINT_DBG(VMBUS, "channel %p buffer %p len %d",
		   Channel, Buffer, BufferLen);

	DumpVmbusChannel(Channel);

	ASSERT((packetLenAligned - packetLen) < sizeof(u64));

	
	desc.Type = Type; 
	desc.Flags = Flags; 
	
	desc.DataOffset8 = sizeof(struct vmpacket_descriptor) >> 3;
	desc.Length8 = (u16)(packetLenAligned >> 3);
	desc.TransactionId = RequestId;

	sg_init_table(bufferList, 3);
	sg_set_buf(&bufferList[0], &desc, sizeof(struct vmpacket_descriptor));
	sg_set_buf(&bufferList[1], Buffer, BufferLen);
	sg_set_buf(&bufferList[2], &alignedData, packetLenAligned - packetLen);

	ret = RingBufferWrite(&Channel->Outbound, bufferList, 3);

	
	if (ret == 0 && !GetRingBufferInterruptMask(&Channel->Outbound))
		VmbusChannelSetEvent(Channel);

	DPRINT_EXIT(VMBUS);

	return ret;
}


int VmbusChannelSendPacketPageBuffer(struct vmbus_channel *Channel,
				     struct hv_page_buffer PageBuffers[],
				     u32 PageCount, void *Buffer, u32 BufferLen,
				     u64 RequestId)
{
	int ret;
	int i;
	struct VMBUS_CHANNEL_PACKET_PAGE_BUFFER desc;
	u32 descSize;
	u32 packetLen;
	u32 packetLenAligned;
	struct scatterlist bufferList[3];
	u64 alignedData = 0;

	DPRINT_ENTER(VMBUS);

	ASSERT(PageCount <= MAX_PAGE_BUFFER_COUNT);

	DumpVmbusChannel(Channel);

	
	descSize = sizeof(struct VMBUS_CHANNEL_PACKET_PAGE_BUFFER) -
			  ((MAX_PAGE_BUFFER_COUNT - PageCount) *
			  sizeof(struct hv_page_buffer));
	packetLen = descSize + BufferLen;
	packetLenAligned = ALIGN_UP(packetLen, sizeof(u64));

	ASSERT((packetLenAligned - packetLen) < sizeof(u64));

	
	desc.Type = VmbusPacketTypeDataUsingGpaDirect;
	desc.Flags = VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED;
	desc.DataOffset8 = descSize >> 3; 
	desc.Length8 = (u16)(packetLenAligned >> 3);
	desc.TransactionId = RequestId;
	desc.RangeCount = PageCount;

	for (i = 0; i < PageCount; i++) {
		desc.Range[i].Length = PageBuffers[i].Length;
		desc.Range[i].Offset = PageBuffers[i].Offset;
		desc.Range[i].Pfn	 = PageBuffers[i].Pfn;
	}

	sg_init_table(bufferList, 3);
	sg_set_buf(&bufferList[0], &desc, descSize);
	sg_set_buf(&bufferList[1], Buffer, BufferLen);
	sg_set_buf(&bufferList[2], &alignedData, packetLenAligned - packetLen);

	ret = RingBufferWrite(&Channel->Outbound, bufferList, 3);

	
	if (ret == 0 && !GetRingBufferInterruptMask(&Channel->Outbound))
		VmbusChannelSetEvent(Channel);

	DPRINT_EXIT(VMBUS);

	return ret;
}


int VmbusChannelSendPacketMultiPageBuffer(struct vmbus_channel *Channel,
				struct hv_multipage_buffer *MultiPageBuffer,
				void *Buffer, u32 BufferLen, u64 RequestId)
{
	int ret;
	struct VMBUS_CHANNEL_PACKET_MULITPAGE_BUFFER desc;
	u32 descSize;
	u32 packetLen;
	u32 packetLenAligned;
	struct scatterlist bufferList[3];
	u64 alignedData = 0;
	u32 PfnCount = NUM_PAGES_SPANNED(MultiPageBuffer->Offset,
					 MultiPageBuffer->Length);

	DPRINT_ENTER(VMBUS);

	DumpVmbusChannel(Channel);

	DPRINT_DBG(VMBUS, "data buffer - offset %u len %u pfn count %u",
		   MultiPageBuffer->Offset, MultiPageBuffer->Length, PfnCount);

	ASSERT(PfnCount > 0);
	ASSERT(PfnCount <= MAX_MULTIPAGE_BUFFER_COUNT);

	
	descSize = sizeof(struct VMBUS_CHANNEL_PACKET_MULITPAGE_BUFFER) -
			  ((MAX_MULTIPAGE_BUFFER_COUNT - PfnCount) *
			  sizeof(u64));
	packetLen = descSize + BufferLen;
	packetLenAligned = ALIGN_UP(packetLen, sizeof(u64));

	ASSERT((packetLenAligned - packetLen) < sizeof(u64));

	
	desc.Type = VmbusPacketTypeDataUsingGpaDirect;
	desc.Flags = VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED;
	desc.DataOffset8 = descSize >> 3; 
	desc.Length8 = (u16)(packetLenAligned >> 3);
	desc.TransactionId = RequestId;
	desc.RangeCount = 1;

	desc.Range.Length = MultiPageBuffer->Length;
	desc.Range.Offset = MultiPageBuffer->Offset;

	memcpy(desc.Range.PfnArray, MultiPageBuffer->PfnArray,
	       PfnCount * sizeof(u64));

	sg_init_table(bufferList, 3);
	sg_set_buf(&bufferList[0], &desc, descSize);
	sg_set_buf(&bufferList[1], Buffer, BufferLen);
	sg_set_buf(&bufferList[2], &alignedData, packetLenAligned - packetLen);

	ret = RingBufferWrite(&Channel->Outbound, bufferList, 3);

	
	if (ret == 0 && !GetRingBufferInterruptMask(&Channel->Outbound))
		VmbusChannelSetEvent(Channel);

	DPRINT_EXIT(VMBUS);

	return ret;
}



int VmbusChannelRecvPacket(struct vmbus_channel *Channel, void *Buffer,
			   u32 BufferLen, u32 *BufferActualLen, u64 *RequestId)
{
	struct vmpacket_descriptor desc;
	u32 packetLen;
	u32 userLen;
	int ret;
	unsigned long flags;

	DPRINT_ENTER(VMBUS);

	*BufferActualLen = 0;
	*RequestId = 0;

	spin_lock_irqsave(&Channel->inbound_lock, flags);

	ret = RingBufferPeek(&Channel->Inbound, &desc,
			     sizeof(struct vmpacket_descriptor));
	if (ret != 0) {
		spin_unlock_irqrestore(&Channel->inbound_lock, flags);

		
		DPRINT_EXIT(VMBUS);
		return 0;
	}

	

	packetLen = desc.Length8 << 3;
	userLen = packetLen - (desc.DataOffset8 << 3);
	

	DPRINT_DBG(VMBUS, "packet received on channel %p relid %d <type %d "
		   "flag %d tid %llx pktlen %d datalen %d> ",
		   Channel, Channel->OfferMsg.ChildRelId, desc.Type,
		   desc.Flags, desc.TransactionId, packetLen, userLen);

	*BufferActualLen = userLen;

	if (userLen > BufferLen) {
		spin_unlock_irqrestore(&Channel->inbound_lock, flags);

		DPRINT_ERR(VMBUS, "buffer too small - got %d needs %d",
			   BufferLen, userLen);
		DPRINT_EXIT(VMBUS);

		return -1;
	}

	*RequestId = desc.TransactionId;

	
	ret = RingBufferRead(&Channel->Inbound, Buffer, userLen,
			     (desc.DataOffset8 << 3));

	spin_unlock_irqrestore(&Channel->inbound_lock, flags);

	DPRINT_EXIT(VMBUS);

	return 0;
}


int VmbusChannelRecvPacketRaw(struct vmbus_channel *Channel, void *Buffer,
			      u32 BufferLen, u32 *BufferActualLen,
			      u64 *RequestId)
{
	struct vmpacket_descriptor desc;
	u32 packetLen;
	u32 userLen;
	int ret;
	unsigned long flags;

	DPRINT_ENTER(VMBUS);

	*BufferActualLen = 0;
	*RequestId = 0;

	spin_lock_irqsave(&Channel->inbound_lock, flags);

	ret = RingBufferPeek(&Channel->Inbound, &desc,
			     sizeof(struct vmpacket_descriptor));
	if (ret != 0) {
		spin_unlock_irqrestore(&Channel->inbound_lock, flags);

		
		DPRINT_EXIT(VMBUS);
		return 0;
	}

	

	packetLen = desc.Length8 << 3;
	userLen = packetLen - (desc.DataOffset8 << 3);

	DPRINT_DBG(VMBUS, "packet received on channel %p relid %d <type %d "
		   "flag %d tid %llx pktlen %d datalen %d> ",
		   Channel, Channel->OfferMsg.ChildRelId, desc.Type,
		   desc.Flags, desc.TransactionId, packetLen, userLen);

	*BufferActualLen = packetLen;

	if (packetLen > BufferLen) {
		spin_unlock_irqrestore(&Channel->inbound_lock, flags);

		DPRINT_ERR(VMBUS, "buffer too small - needed %d bytes but "
			   "got space for only %d bytes", packetLen, BufferLen);
		DPRINT_EXIT(VMBUS);
		return -2;
	}

	*RequestId = desc.TransactionId;

	
	ret = RingBufferRead(&Channel->Inbound, Buffer, packetLen, 0);

	spin_unlock_irqrestore(&Channel->inbound_lock, flags);

	DPRINT_EXIT(VMBUS);

	return 0;
}


void VmbusChannelOnChannelEvent(struct vmbus_channel *Channel)
{
	DumpVmbusChannel(Channel);
	ASSERT(Channel->OnChannelCallback);

	Channel->OnChannelCallback(Channel->ChannelCallbackContext);

	mod_timer(&Channel->poll_timer, jiffies + usecs_to_jiffies(100));
}


void VmbusChannelOnTimer(unsigned long data)
{
	struct vmbus_channel *channel = (struct vmbus_channel *)data;

	if (channel->OnChannelCallback) {
		channel->OnChannelCallback(channel->ChannelCallbackContext);
	}
}


static void DumpVmbusChannel(struct vmbus_channel *Channel)
{
	DPRINT_DBG(VMBUS, "Channel (%d)", Channel->OfferMsg.ChildRelId);
	DumpRingInfo(&Channel->Outbound, "Outbound ");
	DumpRingInfo(&Channel->Inbound, "Inbound ");
}
