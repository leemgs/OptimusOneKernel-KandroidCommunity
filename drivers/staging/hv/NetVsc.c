
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include <linux/io.h>
#include "osd.h"
#include "logging.h"
#include "NetVsc.h"
#include "RndisFilter.h"



static const char *gDriverName = "netvsc";


static const struct hv_guid gNetVscDeviceType = {
	.data = {
		0x63, 0x51, 0x61, 0xF8, 0x3E, 0xDF, 0xc5, 0x46,
		0x91, 0x3F, 0xF2, 0xD2, 0xF9, 0x65, 0xED, 0x0E
	}
};

static int NetVscOnDeviceAdd(struct hv_device *Device, void *AdditionalInfo);

static int NetVscOnDeviceRemove(struct hv_device *Device);

static void NetVscOnCleanup(struct hv_driver *Driver);

static void NetVscOnChannelCallback(void *context);

static int NetVscInitializeSendBufferWithNetVsp(struct hv_device *Device);

static int NetVscInitializeReceiveBufferWithNetVsp(struct hv_device *Device);

static int NetVscDestroySendBuffer(struct netvsc_device *NetDevice);

static int NetVscDestroyReceiveBuffer(struct netvsc_device *NetDevice);

static int NetVscConnectToVsp(struct hv_device *Device);

static void NetVscOnSendCompletion(struct hv_device *Device,
				   struct vmpacket_descriptor *Packet);

static int NetVscOnSend(struct hv_device *Device,
			struct hv_netvsc_packet *Packet);

static void NetVscOnReceive(struct hv_device *Device,
			    struct vmpacket_descriptor *Packet);

static void NetVscOnReceiveCompletion(void *Context);

static void NetVscSendReceiveCompletion(struct hv_device *Device,
					u64 TransactionId);


static struct netvsc_device *AllocNetDevice(struct hv_device *Device)
{
	struct netvsc_device *netDevice;

	netDevice = kzalloc(sizeof(struct netvsc_device), GFP_KERNEL);
	if (!netDevice)
		return NULL;

	
	atomic_cmpxchg(&netDevice->RefCount, 0, 2);

	netDevice->Device = Device;
	Device->Extension = netDevice;

	return netDevice;
}

static void FreeNetDevice(struct netvsc_device *Device)
{
	ASSERT(atomic_read(&Device->RefCount) == 0);
	Device->Device->Extension = NULL;
	kfree(Device);
}



static struct netvsc_device *GetOutboundNetDevice(struct hv_device *Device)
{
	struct netvsc_device *netDevice;

	netDevice = Device->Extension;
	if (netDevice && atomic_read(&netDevice->RefCount) > 1)
		atomic_inc(&netDevice->RefCount);
	else
		netDevice = NULL;

	return netDevice;
}


static struct netvsc_device *GetInboundNetDevice(struct hv_device *Device)
{
	struct netvsc_device *netDevice;

	netDevice = Device->Extension;
	if (netDevice && atomic_read(&netDevice->RefCount))
		atomic_inc(&netDevice->RefCount);
	else
		netDevice = NULL;

	return netDevice;
}

static void PutNetDevice(struct hv_device *Device)
{
	struct netvsc_device *netDevice;

	netDevice = Device->Extension;
	ASSERT(netDevice);

	atomic_dec(&netDevice->RefCount);
}

static struct netvsc_device *ReleaseOutboundNetDevice(struct hv_device *Device)
{
	struct netvsc_device *netDevice;

	netDevice = Device->Extension;
	if (netDevice == NULL)
		return NULL;

	
	while (atomic_cmpxchg(&netDevice->RefCount, 2, 1) != 2)
		udelay(100);

	return netDevice;
}

static struct netvsc_device *ReleaseInboundNetDevice(struct hv_device *Device)
{
	struct netvsc_device *netDevice;

	netDevice = Device->Extension;
	if (netDevice == NULL)
		return NULL;

	
	while (atomic_cmpxchg(&netDevice->RefCount, 1, 0) != 1)
		udelay(100);

	Device->Extension = NULL;
	return netDevice;
}


int NetVscInitialize(struct hv_driver *drv)
{
	struct netvsc_driver *driver = (struct netvsc_driver *)drv;

	DPRINT_ENTER(NETVSC);

	DPRINT_DBG(NETVSC, "sizeof(struct hv_netvsc_packet)=%zd, "
		   "sizeof(struct nvsp_message)=%zd, "
		   "sizeof(struct vmtransfer_page_packet_header)=%zd",
		   sizeof(struct hv_netvsc_packet),
		   sizeof(struct nvsp_message),
		   sizeof(struct vmtransfer_page_packet_header));

	
	ASSERT(driver->RingBufferSize >= (PAGE_SIZE << 1));

	drv->name = gDriverName;
	memcpy(&drv->deviceType, &gNetVscDeviceType, sizeof(struct hv_guid));

	
	ASSERT(driver->OnReceiveCallback);
	ASSERT(driver->OnLinkStatusChanged);

	
	driver->Base.OnDeviceAdd	= NetVscOnDeviceAdd;
	driver->Base.OnDeviceRemove	= NetVscOnDeviceRemove;
	driver->Base.OnCleanup		= NetVscOnCleanup;

	driver->OnSend			= NetVscOnSend;

	RndisFilterInit(driver);

	DPRINT_EXIT(NETVSC);

	return 0;
}

static int NetVscInitializeReceiveBufferWithNetVsp(struct hv_device *Device)
{
	int ret = 0;
	struct netvsc_device *netDevice;
	struct nvsp_message *initPacket;

	DPRINT_ENTER(NETVSC);

	netDevice = GetOutboundNetDevice(Device);
	if (!netDevice) {
		DPRINT_ERR(NETVSC, "unable to get net device..."
			   "device being destroyed?");
		DPRINT_EXIT(NETVSC);
		return -1;
	}
	ASSERT(netDevice->ReceiveBufferSize > 0);
	
	ASSERT((netDevice->ReceiveBufferSize & (PAGE_SIZE - 1)) == 0);

	netDevice->ReceiveBuffer =
		osd_PageAlloc(netDevice->ReceiveBufferSize >> PAGE_SHIFT);
	if (!netDevice->ReceiveBuffer) {
		DPRINT_ERR(NETVSC,
			   "unable to allocate receive buffer of size %d",
			   netDevice->ReceiveBufferSize);
		ret = -1;
		goto Cleanup;
	}
	
	ASSERT(((unsigned long)netDevice->ReceiveBuffer & (PAGE_SIZE - 1)) ==
		0);

	DPRINT_INFO(NETVSC, "Establishing receive buffer's GPADL...");

	
	ret = Device->Driver->VmbusChannelInterface.EstablishGpadl(Device,
					netDevice->ReceiveBuffer,
					netDevice->ReceiveBufferSize,
					&netDevice->ReceiveBufferGpadlHandle);
	if (ret != 0) {
		DPRINT_ERR(NETVSC,
			   "unable to establish receive buffer's gpadl");
		goto Cleanup;
	}

	

	
	DPRINT_INFO(NETVSC, "Sending NvspMessage1TypeSendReceiveBuffer...");

	initPacket = &netDevice->ChannelInitPacket;

	memset(initPacket, 0, sizeof(struct nvsp_message));

	initPacket->Header.MessageType = NvspMessage1TypeSendReceiveBuffer;
	initPacket->Messages.Version1Messages.SendReceiveBuffer.GpadlHandle = netDevice->ReceiveBufferGpadlHandle;
	initPacket->Messages.Version1Messages.SendReceiveBuffer.Id = NETVSC_RECEIVE_BUFFER_ID;

	
	ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
				initPacket,
				sizeof(struct nvsp_message),
				(unsigned long)initPacket,
				VmbusPacketTypeDataInBand,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0) {
		DPRINT_ERR(NETVSC,
			   "unable to send receive buffer's gpadl to netvsp");
		goto Cleanup;
	}

	osd_WaitEventWait(netDevice->ChannelInitEvent);

	
	if (initPacket->Messages.Version1Messages.SendReceiveBufferComplete.Status != NvspStatusSuccess) {
		DPRINT_ERR(NETVSC, "Unable to complete receive buffer "
			   "initialzation with NetVsp - status %d",
			   initPacket->Messages.Version1Messages.SendReceiveBufferComplete.Status);
		ret = -1;
		goto Cleanup;
	}

	
	ASSERT(netDevice->ReceiveSectionCount == 0);
	ASSERT(netDevice->ReceiveSections == NULL);

	netDevice->ReceiveSectionCount = initPacket->Messages.Version1Messages.SendReceiveBufferComplete.NumSections;

	netDevice->ReceiveSections = kmalloc(netDevice->ReceiveSectionCount * sizeof(struct nvsp_1_receive_buffer_section), GFP_KERNEL);
	if (netDevice->ReceiveSections == NULL) {
		ret = -1;
		goto Cleanup;
	}

	memcpy(netDevice->ReceiveSections,
		initPacket->Messages.Version1Messages.SendReceiveBufferComplete.Sections,
		netDevice->ReceiveSectionCount * sizeof(struct nvsp_1_receive_buffer_section));

	DPRINT_INFO(NETVSC, "Receive sections info (count %d, offset %d, "
		    "endoffset %d, suballoc size %d, num suballocs %d)",
		    netDevice->ReceiveSectionCount,
		    netDevice->ReceiveSections[0].Offset,
		    netDevice->ReceiveSections[0].EndOffset,
		    netDevice->ReceiveSections[0].SubAllocationSize,
		    netDevice->ReceiveSections[0].NumSubAllocations);

	
	if (netDevice->ReceiveSectionCount != 1 ||
	    netDevice->ReceiveSections->Offset != 0) {
		ret = -1;
		goto Cleanup;
	}

	goto Exit;

Cleanup:
	NetVscDestroyReceiveBuffer(netDevice);

Exit:
	PutNetDevice(Device);
	DPRINT_EXIT(NETVSC);
	return ret;
}

static int NetVscInitializeSendBufferWithNetVsp(struct hv_device *Device)
{
	int ret = 0;
	struct netvsc_device *netDevice;
	struct nvsp_message *initPacket;

	DPRINT_ENTER(NETVSC);

	netDevice = GetOutboundNetDevice(Device);
	if (!netDevice) {
		DPRINT_ERR(NETVSC, "unable to get net device..."
			   "device being destroyed?");
		DPRINT_EXIT(NETVSC);
		return -1;
	}
	ASSERT(netDevice->SendBufferSize > 0);
	
	ASSERT((netDevice->SendBufferSize & (PAGE_SIZE - 1)) == 0);

	netDevice->SendBuffer =
		osd_PageAlloc(netDevice->SendBufferSize >> PAGE_SHIFT);
	if (!netDevice->SendBuffer) {
		DPRINT_ERR(NETVSC, "unable to allocate send buffer of size %d",
			   netDevice->SendBufferSize);
		ret = -1;
		goto Cleanup;
	}
	
	ASSERT(((unsigned long)netDevice->SendBuffer & (PAGE_SIZE - 1)) == 0);

	DPRINT_INFO(NETVSC, "Establishing send buffer's GPADL...");

	
	ret = Device->Driver->VmbusChannelInterface.EstablishGpadl(Device,
					netDevice->SendBuffer,
					netDevice->SendBufferSize,
					&netDevice->SendBufferGpadlHandle);
	if (ret != 0) {
		DPRINT_ERR(NETVSC, "unable to establish send buffer's gpadl");
		goto Cleanup;
	}

	

	
	DPRINT_INFO(NETVSC, "Sending NvspMessage1TypeSendSendBuffer...");

	initPacket = &netDevice->ChannelInitPacket;

	memset(initPacket, 0, sizeof(struct nvsp_message));

	initPacket->Header.MessageType = NvspMessage1TypeSendSendBuffer;
	initPacket->Messages.Version1Messages.SendReceiveBuffer.GpadlHandle = netDevice->SendBufferGpadlHandle;
	initPacket->Messages.Version1Messages.SendReceiveBuffer.Id = NETVSC_SEND_BUFFER_ID;

	
	ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
				initPacket, sizeof(struct nvsp_message),
				(unsigned long)initPacket,
				VmbusPacketTypeDataInBand,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0) {
		DPRINT_ERR(NETVSC,
			   "unable to send receive buffer's gpadl to netvsp");
		goto Cleanup;
	}

	osd_WaitEventWait(netDevice->ChannelInitEvent);

	
	if (initPacket->Messages.Version1Messages.SendSendBufferComplete.Status != NvspStatusSuccess) {
		DPRINT_ERR(NETVSC, "Unable to complete send buffer "
			   "initialzation with NetVsp - status %d",
			   initPacket->Messages.Version1Messages.SendSendBufferComplete.Status);
		ret = -1;
		goto Cleanup;
	}

	netDevice->SendSectionSize = initPacket->Messages.Version1Messages.SendSendBufferComplete.SectionSize;

	goto Exit;

Cleanup:
	NetVscDestroySendBuffer(netDevice);

Exit:
	PutNetDevice(Device);
	DPRINT_EXIT(NETVSC);
	return ret;
}

static int NetVscDestroyReceiveBuffer(struct netvsc_device *NetDevice)
{
	struct nvsp_message *revokePacket;
	int ret = 0;

	DPRINT_ENTER(NETVSC);

	
	if (NetDevice->ReceiveSectionCount) {
		DPRINT_INFO(NETVSC,
			    "Sending NvspMessage1TypeRevokeReceiveBuffer...");

		
		revokePacket = &NetDevice->RevokePacket;
		memset(revokePacket, 0, sizeof(struct nvsp_message));

		revokePacket->Header.MessageType = NvspMessage1TypeRevokeReceiveBuffer;
		revokePacket->Messages.Version1Messages.RevokeReceiveBuffer.Id = NETVSC_RECEIVE_BUFFER_ID;

		ret = NetDevice->Device->Driver->VmbusChannelInterface.SendPacket(
						NetDevice->Device,
						revokePacket,
						sizeof(struct nvsp_message),
						(unsigned long)revokePacket,
						VmbusPacketTypeDataInBand, 0);
		
		if (ret != 0) {
			DPRINT_ERR(NETVSC, "unable to send revoke receive "
				   "buffer to netvsp");
			DPRINT_EXIT(NETVSC);
			return -1;
		}
	}

	
	if (NetDevice->ReceiveBufferGpadlHandle) {
		DPRINT_INFO(NETVSC, "Tearing down receive buffer's GPADL...");

		ret = NetDevice->Device->Driver->VmbusChannelInterface.TeardownGpadl(
					NetDevice->Device,
					NetDevice->ReceiveBufferGpadlHandle);

		
		if (ret != 0) {
			DPRINT_ERR(NETVSC,
				   "unable to teardown receive buffer's gpadl");
			DPRINT_EXIT(NETVSC);
			return -1;
		}
		NetDevice->ReceiveBufferGpadlHandle = 0;
	}

	if (NetDevice->ReceiveBuffer) {
		DPRINT_INFO(NETVSC, "Freeing up receive buffer...");

		
		osd_PageFree(NetDevice->ReceiveBuffer,
			     NetDevice->ReceiveBufferSize >> PAGE_SHIFT);
		NetDevice->ReceiveBuffer = NULL;
	}

	if (NetDevice->ReceiveSections) {
		NetDevice->ReceiveSectionCount = 0;
		kfree(NetDevice->ReceiveSections);
		NetDevice->ReceiveSections = NULL;
	}

	DPRINT_EXIT(NETVSC);

	return ret;
}

static int NetVscDestroySendBuffer(struct netvsc_device *NetDevice)
{
	struct nvsp_message *revokePacket;
	int ret = 0;

	DPRINT_ENTER(NETVSC);

	
	if (NetDevice->SendSectionSize) {
		DPRINT_INFO(NETVSC,
			    "Sending NvspMessage1TypeRevokeSendBuffer...");

		
		revokePacket = &NetDevice->RevokePacket;
		memset(revokePacket, 0, sizeof(struct nvsp_message));

		revokePacket->Header.MessageType = NvspMessage1TypeRevokeSendBuffer;
		revokePacket->Messages.Version1Messages.RevokeSendBuffer.Id = NETVSC_SEND_BUFFER_ID;

		ret = NetDevice->Device->Driver->VmbusChannelInterface.SendPacket(NetDevice->Device,
					revokePacket,
					sizeof(struct nvsp_message),
					(unsigned long)revokePacket,
					VmbusPacketTypeDataInBand, 0);
		
		if (ret != 0) {
			DPRINT_ERR(NETVSC, "unable to send revoke send buffer "
				   "to netvsp");
			DPRINT_EXIT(NETVSC);
			return -1;
		}
	}

	
	if (NetDevice->SendBufferGpadlHandle) {
		DPRINT_INFO(NETVSC, "Tearing down send buffer's GPADL...");

		ret = NetDevice->Device->Driver->VmbusChannelInterface.TeardownGpadl(NetDevice->Device, NetDevice->SendBufferGpadlHandle);

		
		if (ret != 0) {
			DPRINT_ERR(NETVSC, "unable to teardown send buffer's "
				   "gpadl");
			DPRINT_EXIT(NETVSC);
			return -1;
		}
		NetDevice->SendBufferGpadlHandle = 0;
	}

	if (NetDevice->SendBuffer) {
		DPRINT_INFO(NETVSC, "Freeing up send buffer...");

		
		osd_PageFree(NetDevice->SendBuffer,
			     NetDevice->SendBufferSize >> PAGE_SHIFT);
		NetDevice->SendBuffer = NULL;
	}

	DPRINT_EXIT(NETVSC);

	return ret;
}


static int NetVscConnectToVsp(struct hv_device *Device)
{
	int ret;
	struct netvsc_device *netDevice;
	struct nvsp_message *initPacket;
	int ndisVersion;

	DPRINT_ENTER(NETVSC);

	netDevice = GetOutboundNetDevice(Device);
	if (!netDevice) {
		DPRINT_ERR(NETVSC, "unable to get net device..."
			   "device being destroyed?");
		DPRINT_EXIT(NETVSC);
		return -1;
	}

	initPacket = &netDevice->ChannelInitPacket;

	memset(initPacket, 0, sizeof(struct nvsp_message));
	initPacket->Header.MessageType = NvspMessageTypeInit;
	initPacket->Messages.InitMessages.Init.MinProtocolVersion = NVSP_MIN_PROTOCOL_VERSION;
	initPacket->Messages.InitMessages.Init.MaxProtocolVersion = NVSP_MAX_PROTOCOL_VERSION;

	DPRINT_INFO(NETVSC, "Sending NvspMessageTypeInit...");

	
	ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
				initPacket,
				sizeof(struct nvsp_message),
				(unsigned long)initPacket,
				VmbusPacketTypeDataInBand,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	if (ret != 0) {
		DPRINT_ERR(NETVSC, "unable to send NvspMessageTypeInit");
		goto Cleanup;
	}

	osd_WaitEventWait(netDevice->ChannelInitEvent);

	
	
	DPRINT_INFO(NETVSC, "NvspMessageTypeInit status(%d) max mdl chain (%d)",
		initPacket->Messages.InitMessages.InitComplete.Status,
		initPacket->Messages.InitMessages.InitComplete.MaximumMdlChainLength);

	if (initPacket->Messages.InitMessages.InitComplete.Status !=
	    NvspStatusSuccess) {
		DPRINT_ERR(NETVSC,
			"unable to initialize with netvsp (status 0x%x)",
			initPacket->Messages.InitMessages.InitComplete.Status);
		ret = -1;
		goto Cleanup;
	}

	if (initPacket->Messages.InitMessages.InitComplete.NegotiatedProtocolVersion != NVSP_PROTOCOL_VERSION_1) {
		DPRINT_ERR(NETVSC, "unable to initialize with netvsp "
			   "(version expected 1 got %d)",
			   initPacket->Messages.InitMessages.InitComplete.NegotiatedProtocolVersion);
		ret = -1;
		goto Cleanup;
	}
	DPRINT_INFO(NETVSC, "Sending NvspMessage1TypeSendNdisVersion...");

	
	memset(initPacket, 0, sizeof(struct nvsp_message));

	ndisVersion = 0x00050000;

	initPacket->Header.MessageType = NvspMessage1TypeSendNdisVersion;
	initPacket->Messages.Version1Messages.SendNdisVersion.NdisMajorVersion =
				(ndisVersion & 0xFFFF0000) >> 16;
	initPacket->Messages.Version1Messages.SendNdisVersion.NdisMinorVersion =
				ndisVersion & 0xFFFF;

	
	ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
					initPacket,
					sizeof(struct nvsp_message),
					(unsigned long)initPacket,
					VmbusPacketTypeDataInBand, 0);
	if (ret != 0) {
		DPRINT_ERR(NETVSC,
			   "unable to send NvspMessage1TypeSendNdisVersion");
		ret = -1;
		goto Cleanup;
	}
	
	 

	
	ret = NetVscInitializeReceiveBufferWithNetVsp(Device);
	if (ret == 0)
		ret = NetVscInitializeSendBufferWithNetVsp(Device);

Cleanup:
	PutNetDevice(Device);
	DPRINT_EXIT(NETVSC);
	return ret;
}

static void NetVscDisconnectFromVsp(struct netvsc_device *NetDevice)
{
	DPRINT_ENTER(NETVSC);

	NetVscDestroyReceiveBuffer(NetDevice);
	NetVscDestroySendBuffer(NetDevice);

	DPRINT_EXIT(NETVSC);
}


static int NetVscOnDeviceAdd(struct hv_device *Device, void *AdditionalInfo)
{
	int ret = 0;
	int i;
	struct netvsc_device *netDevice;
	struct hv_netvsc_packet *packet, *pos;
	struct netvsc_driver *netDriver =
				(struct netvsc_driver *)Device->Driver;

	DPRINT_ENTER(NETVSC);

	netDevice = AllocNetDevice(Device);
	if (!netDevice) {
		ret = -1;
		goto Cleanup;
	}

	DPRINT_DBG(NETVSC, "netvsc channel object allocated - %p", netDevice);

	
	netDevice->ReceiveBufferSize = NETVSC_RECEIVE_BUFFER_SIZE;
	spin_lock_init(&netDevice->receive_packet_list_lock);

	netDevice->SendBufferSize = NETVSC_SEND_BUFFER_SIZE;

	INIT_LIST_HEAD(&netDevice->ReceivePacketList);

	for (i = 0; i < NETVSC_RECEIVE_PACKETLIST_COUNT; i++) {
		packet = kzalloc(sizeof(struct hv_netvsc_packet) +
				 (NETVSC_RECEIVE_SG_COUNT *
				  sizeof(struct hv_page_buffer)), GFP_KERNEL);
		if (!packet) {
			DPRINT_DBG(NETVSC, "unable to allocate netvsc pkts "
				   "for receive pool (wanted %d got %d)",
				   NETVSC_RECEIVE_PACKETLIST_COUNT, i);
			break;
		}
		list_add_tail(&packet->ListEntry,
			      &netDevice->ReceivePacketList);
	}
	netDevice->ChannelInitEvent = osd_WaitEventCreate();

	
	ret = Device->Driver->VmbusChannelInterface.Open(Device,
						netDriver->RingBufferSize,
						netDriver->RingBufferSize,
						NULL, 0,
						NetVscOnChannelCallback,
						Device);

	if (ret != 0) {
		DPRINT_ERR(NETVSC, "unable to open channel: %d", ret);
		ret = -1;
		goto Cleanup;
	}

	
	DPRINT_INFO(NETVSC, "*** NetVSC channel opened successfully! ***");

	
	ret = NetVscConnectToVsp(Device);
	if (ret != 0) {
		DPRINT_ERR(NETVSC, "unable to connect to NetVSP - %d", ret);
		ret = -1;
		goto Close;
	}

	DPRINT_INFO(NETVSC, "*** NetVSC channel handshake result - %d ***",
		    ret);

	DPRINT_EXIT(NETVSC);
	return ret;

Close:
	
	Device->Driver->VmbusChannelInterface.Close(Device);

Cleanup:

	if (netDevice) {
		kfree(netDevice->ChannelInitEvent);

		list_for_each_entry_safe(packet, pos,
					 &netDevice->ReceivePacketList,
					 ListEntry) {
			list_del(&packet->ListEntry);
			kfree(packet);
		}

		ReleaseOutboundNetDevice(Device);
		ReleaseInboundNetDevice(Device);

		FreeNetDevice(netDevice);
	}

	DPRINT_EXIT(NETVSC);
	return ret;
}


static int NetVscOnDeviceRemove(struct hv_device *Device)
{
	struct netvsc_device *netDevice;
	struct hv_netvsc_packet *netvscPacket, *pos;

	DPRINT_ENTER(NETVSC);

	DPRINT_INFO(NETVSC, "Disabling outbound traffic on net device (%p)...",
		    Device->Extension);

	
	netDevice = ReleaseOutboundNetDevice(Device);
	if (!netDevice) {
		DPRINT_ERR(NETVSC, "No net device present!!");
		return -1;
	}

	
	while (atomic_read(&netDevice->NumOutstandingSends)) {
		DPRINT_INFO(NETVSC, "waiting for %d requests to complete...",
			    atomic_read(&netDevice->NumOutstandingSends));
		udelay(100);
	}

	DPRINT_INFO(NETVSC, "Disconnecting from netvsp...");

	NetVscDisconnectFromVsp(netDevice);

	DPRINT_INFO(NETVSC, "Disabling inbound traffic on net device (%p)...",
		    Device->Extension);

	
	netDevice = ReleaseInboundNetDevice(Device);

	
	DPRINT_INFO(NETVSC, "net device (%p) safe to remove", netDevice);

	
	Device->Driver->VmbusChannelInterface.Close(Device);

	
	list_for_each_entry_safe(netvscPacket, pos,
				 &netDevice->ReceivePacketList, ListEntry) {
		list_del(&netvscPacket->ListEntry);
		kfree(netvscPacket);
	}

	kfree(netDevice->ChannelInitEvent);
	FreeNetDevice(netDevice);

	DPRINT_EXIT(NETVSC);
	return 0;
}


static void NetVscOnCleanup(struct hv_driver *drv)
{
	DPRINT_ENTER(NETVSC);
	DPRINT_EXIT(NETVSC);
}

static void NetVscOnSendCompletion(struct hv_device *Device,
				   struct vmpacket_descriptor *Packet)
{
	struct netvsc_device *netDevice;
	struct nvsp_message *nvspPacket;
	struct hv_netvsc_packet *nvscPacket;

	DPRINT_ENTER(NETVSC);

	netDevice = GetInboundNetDevice(Device);
	if (!netDevice) {
		DPRINT_ERR(NETVSC, "unable to get net device..."
			   "device being destroyed?");
		DPRINT_EXIT(NETVSC);
		return;
	}

	nvspPacket = (struct nvsp_message *)((unsigned long)Packet + (Packet->DataOffset8 << 3));

	DPRINT_DBG(NETVSC, "send completion packet - type %d",
		   nvspPacket->Header.MessageType);

	if ((nvspPacket->Header.MessageType == NvspMessageTypeInitComplete) ||
	    (nvspPacket->Header.MessageType ==
	     NvspMessage1TypeSendReceiveBufferComplete) ||
	    (nvspPacket->Header.MessageType ==
	     NvspMessage1TypeSendSendBufferComplete)) {
		
		memcpy(&netDevice->ChannelInitPacket, nvspPacket,
		       sizeof(struct nvsp_message));
		osd_WaitEventSet(netDevice->ChannelInitEvent);
	} else if (nvspPacket->Header.MessageType ==
		   NvspMessage1TypeSendRNDISPacketComplete) {
		
		nvscPacket = (struct hv_netvsc_packet *)(unsigned long)Packet->TransactionId;
		ASSERT(nvscPacket);

		
		nvscPacket->Completion.Send.OnSendCompletion(nvscPacket->Completion.Send.SendCompletionContext);

		atomic_dec(&netDevice->NumOutstandingSends);
	} else {
		DPRINT_ERR(NETVSC, "Unknown send completion packet type - "
			   "%d received!!", nvspPacket->Header.MessageType);
	}

	PutNetDevice(Device);
	DPRINT_EXIT(NETVSC);
}

static int NetVscOnSend(struct hv_device *Device,
			struct hv_netvsc_packet *Packet)
{
	struct netvsc_device *netDevice;
	int ret = 0;

	struct nvsp_message sendMessage;

	DPRINT_ENTER(NETVSC);

	netDevice = GetOutboundNetDevice(Device);
	if (!netDevice) {
		DPRINT_ERR(NETVSC, "net device (%p) shutting down..."
			   "ignoring outbound packets", netDevice);
		DPRINT_EXIT(NETVSC);
		return -2;
	}

	sendMessage.Header.MessageType = NvspMessage1TypeSendRNDISPacket;
	if (Packet->IsDataPacket) {
		
		sendMessage.Messages.Version1Messages.SendRNDISPacket.ChannelType = 0;
	} else {
		
		sendMessage.Messages.Version1Messages.SendRNDISPacket.ChannelType = 1;
	}

	
	sendMessage.Messages.Version1Messages.SendRNDISPacket.SendBufferSectionIndex = 0xFFFFFFFF;
	sendMessage.Messages.Version1Messages.SendRNDISPacket.SendBufferSectionSize = 0;

	if (Packet->PageBufferCount) {
		ret = Device->Driver->VmbusChannelInterface.SendPacketPageBuffer(
					Device, Packet->PageBuffers,
					Packet->PageBufferCount,
					&sendMessage,
					sizeof(struct nvsp_message),
					(unsigned long)Packet);
	} else {
		ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
				&sendMessage,
				sizeof(struct nvsp_message),
				(unsigned long)Packet,
				VmbusPacketTypeDataInBand,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	}

	if (ret != 0)
		DPRINT_ERR(NETVSC, "Unable to send packet %p ret %d",
			   Packet, ret);

	atomic_inc(&netDevice->NumOutstandingSends);
	PutNetDevice(Device);

	DPRINT_EXIT(NETVSC);
	return ret;
}

static void NetVscOnReceive(struct hv_device *Device,
			    struct vmpacket_descriptor *Packet)
{
	struct netvsc_device *netDevice;
	struct vmtransfer_page_packet_header *vmxferpagePacket;
	struct nvsp_message *nvspPacket;
	struct hv_netvsc_packet *netvscPacket = NULL;
	unsigned long start;
	unsigned long end, endVirtual;
	
	struct xferpage_packet *xferpagePacket = NULL;
	int i, j;
	int count = 0, bytesRemain = 0;
	unsigned long flags;
	LIST_HEAD(listHead);

	DPRINT_ENTER(NETVSC);

	netDevice = GetInboundNetDevice(Device);
	if (!netDevice) {
		DPRINT_ERR(NETVSC, "unable to get net device..."
			   "device being destroyed?");
		DPRINT_EXIT(NETVSC);
		return;
	}

	
	if (Packet->Type != VmbusPacketTypeDataUsingTransferPages) {
		DPRINT_ERR(NETVSC, "Unknown packet type received - %d",
			   Packet->Type);
		PutNetDevice(Device);
		return;
	}

	nvspPacket = (struct nvsp_message *)((unsigned long)Packet +
			(Packet->DataOffset8 << 3));

	
	if (nvspPacket->Header.MessageType != NvspMessage1TypeSendRNDISPacket) {
		DPRINT_ERR(NETVSC, "Unknown nvsp packet type received - %d",
			   nvspPacket->Header.MessageType);
		PutNetDevice(Device);
		return;
	}

	DPRINT_DBG(NETVSC, "NVSP packet received - type %d",
		   nvspPacket->Header.MessageType);

	vmxferpagePacket = (struct vmtransfer_page_packet_header *)Packet;

	if (vmxferpagePacket->TransferPageSetId != NETVSC_RECEIVE_BUFFER_ID) {
		DPRINT_ERR(NETVSC, "Invalid xfer page set id - "
			   "expecting %x got %x", NETVSC_RECEIVE_BUFFER_ID,
			   vmxferpagePacket->TransferPageSetId);
		PutNetDevice(Device);
		return;
	}

	DPRINT_DBG(NETVSC, "xfer page - range count %d",
		   vmxferpagePacket->RangeCount);

	
	spin_lock_irqsave(&netDevice->receive_packet_list_lock, flags);
	while (!list_empty(&netDevice->ReceivePacketList)) {
		list_move_tail(netDevice->ReceivePacketList.next, &listHead);
		if (++count == vmxferpagePacket->RangeCount + 1)
			break;
	}
	spin_unlock_irqrestore(&netDevice->receive_packet_list_lock, flags);

	
	if (count < 2) {
		DPRINT_ERR(NETVSC, "Got only %d netvsc pkt...needed %d pkts. "
			   "Dropping this xfer page packet completely!",
			   count, vmxferpagePacket->RangeCount + 1);

		
		spin_lock_irqsave(&netDevice->receive_packet_list_lock, flags);
		for (i = count; i != 0; i--) {
			list_move_tail(listHead.next,
				       &netDevice->ReceivePacketList);
		}
		spin_unlock_irqrestore(&netDevice->receive_packet_list_lock,
				       flags);

		NetVscSendReceiveCompletion(Device,
					    vmxferpagePacket->d.TransactionId);

		PutNetDevice(Device);
		return;
	}

	
	xferpagePacket = (struct xferpage_packet*)listHead.next;
	list_del(&xferpagePacket->ListEntry);

	
	xferpagePacket->Count = count - 1;
	ASSERT(xferpagePacket->Count > 0 && xferpagePacket->Count <=
		vmxferpagePacket->RangeCount);

	if (xferpagePacket->Count != vmxferpagePacket->RangeCount) {
		DPRINT_INFO(NETVSC, "Needed %d netvsc pkts to satisy this xfer "
			    "page...got %d", vmxferpagePacket->RangeCount,
			    xferpagePacket->Count);
	}

	
	for (i = 0; i < (count - 1); i++) {
		netvscPacket = (struct hv_netvsc_packet*)listHead.next;
		list_del(&netvscPacket->ListEntry);

		
		netvscPacket->XferPagePacket = xferpagePacket;
		netvscPacket->Completion.Recv.OnReceiveCompletion =
					NetVscOnReceiveCompletion;
		netvscPacket->Completion.Recv.ReceiveCompletionContext =
					netvscPacket;
		netvscPacket->Device = Device;
		
		netvscPacket->Completion.Recv.ReceiveCompletionTid =
					vmxferpagePacket->d.TransactionId;

		netvscPacket->TotalDataBufferLength =
					vmxferpagePacket->Ranges[i].ByteCount;
		netvscPacket->PageBufferCount = 1;

		ASSERT(vmxferpagePacket->Ranges[i].ByteOffset +
			vmxferpagePacket->Ranges[i].ByteCount <
			netDevice->ReceiveBufferSize);

		netvscPacket->PageBuffers[0].Length =
					vmxferpagePacket->Ranges[i].ByteCount;

		start = virt_to_phys((void *)((unsigned long)netDevice->ReceiveBuffer + vmxferpagePacket->Ranges[i].ByteOffset));

		netvscPacket->PageBuffers[0].Pfn = start >> PAGE_SHIFT;
		endVirtual = (unsigned long)netDevice->ReceiveBuffer
		    + vmxferpagePacket->Ranges[i].ByteOffset
		    + vmxferpagePacket->Ranges[i].ByteCount - 1;
		end = virt_to_phys((void *)endVirtual);

		
		netvscPacket->PageBuffers[0].Offset =
			vmxferpagePacket->Ranges[i].ByteOffset & (PAGE_SIZE - 1);
		if ((end >> PAGE_SHIFT) != (start >> PAGE_SHIFT)) {
			
			netvscPacket->PageBuffers[0].Length =
				(netvscPacket->PageBuffers[0].Pfn << PAGE_SHIFT)
				+ PAGE_SIZE - start;
			bytesRemain = netvscPacket->TotalDataBufferLength -
					netvscPacket->PageBuffers[0].Length;
			for (j = 1; j < NETVSC_PACKET_MAXPAGE; j++) {
				netvscPacket->PageBuffers[j].Offset = 0;
				if (bytesRemain <= PAGE_SIZE) {
					netvscPacket->PageBuffers[j].Length = bytesRemain;
					bytesRemain = 0;
				} else {
					netvscPacket->PageBuffers[j].Length = PAGE_SIZE;
					bytesRemain -= PAGE_SIZE;
				}
				netvscPacket->PageBuffers[j].Pfn =
				    virt_to_phys((void *)(endVirtual - bytesRemain)) >> PAGE_SHIFT;
				netvscPacket->PageBufferCount++;
				if (bytesRemain == 0)
					break;
			}
			ASSERT(bytesRemain == 0);
		}
		DPRINT_DBG(NETVSC, "[%d] - (abs offset %u len %u) => "
			   "(pfn %llx, offset %u, len %u)", i,
			   vmxferpagePacket->Ranges[i].ByteOffset,
			   vmxferpagePacket->Ranges[i].ByteCount,
			   netvscPacket->PageBuffers[0].Pfn,
			   netvscPacket->PageBuffers[0].Offset,
			   netvscPacket->PageBuffers[0].Length);

		
		((struct netvsc_driver *)Device->Driver)->OnReceiveCallback(Device, netvscPacket);

		NetVscOnReceiveCompletion(netvscPacket->Completion.Recv.ReceiveCompletionContext);
	}

	ASSERT(list_empty(&listHead));

	PutNetDevice(Device);
	DPRINT_EXIT(NETVSC);
}

static void NetVscSendReceiveCompletion(struct hv_device *Device,
					u64 TransactionId)
{
	struct nvsp_message recvcompMessage;
	int retries = 0;
	int ret;

	DPRINT_DBG(NETVSC, "Sending receive completion pkt - %llx",
		   TransactionId);

	recvcompMessage.Header.MessageType =
				NvspMessage1TypeSendRNDISPacketComplete;

	
	recvcompMessage.Messages.Version1Messages.SendRNDISPacketComplete.Status = NvspStatusSuccess;

retry_send_cmplt:
	
	ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
					&recvcompMessage,
					sizeof(struct nvsp_message),
					TransactionId,
					VmbusPacketTypeCompletion, 0);
	if (ret == 0) {
		
		
	} else if (ret == -1) {
		
		retries++;
		DPRINT_ERR(NETVSC, "unable to send receive completion pkt "
			   "(tid %llx)...retrying %d", TransactionId, retries);

		if (retries < 4) {
			udelay(100);
			goto retry_send_cmplt;
		} else {
			DPRINT_ERR(NETVSC, "unable to send receive completion "
				  "pkt (tid %llx)...give up retrying",
				  TransactionId);
		}
	} else {
		DPRINT_ERR(NETVSC, "unable to send receive completion pkt - "
			   "%llx", TransactionId);
	}
}


static void NetVscOnReceiveCompletion(void *Context)
{
	struct hv_netvsc_packet *packet = Context;
	struct hv_device *device = (struct hv_device *)packet->Device;
	struct netvsc_device *netDevice;
	u64 transactionId = 0;
	bool fSendReceiveComp = false;
	unsigned long flags;

	DPRINT_ENTER(NETVSC);

	ASSERT(packet->XferPagePacket);

	
	netDevice = GetInboundNetDevice(device);
	if (!netDevice) {
		DPRINT_ERR(NETVSC, "unable to get net device..."
			   "device being destroyed?");
		DPRINT_EXIT(NETVSC);
		return;
	}

	
	spin_lock_irqsave(&netDevice->receive_packet_list_lock, flags);

	ASSERT(packet->XferPagePacket->Count > 0);
	packet->XferPagePacket->Count--;

	
	if (packet->XferPagePacket->Count == 0) {
		fSendReceiveComp = true;
		transactionId = packet->Completion.Recv.ReceiveCompletionTid;
		list_add_tail(&packet->XferPagePacket->ListEntry,
			      &netDevice->ReceivePacketList);

	}

	
	list_add_tail(&packet->ListEntry, &netDevice->ReceivePacketList);
	spin_unlock_irqrestore(&netDevice->receive_packet_list_lock, flags);

	
	if (fSendReceiveComp)
		NetVscSendReceiveCompletion(device, transactionId);

	PutNetDevice(device);
	DPRINT_EXIT(NETVSC);
}

void NetVscOnChannelCallback(void *Context)
{
	const int netPacketSize = 2048;
	int ret;
	struct hv_device *device = Context;
	struct netvsc_device *netDevice;
	u32 bytesRecvd;
	u64 requestId;
	unsigned char packet[netPacketSize];
	struct vmpacket_descriptor *desc;
	unsigned char *buffer = packet;
	int bufferlen = netPacketSize;


	DPRINT_ENTER(NETVSC);

	ASSERT(device);

	netDevice = GetInboundNetDevice(device);
	if (!netDevice) {
		DPRINT_ERR(NETVSC, "net device (%p) shutting down..."
			   "ignoring inbound packets", netDevice);
		DPRINT_EXIT(NETVSC);
		return;
	}

	do {
		ret = device->Driver->VmbusChannelInterface.RecvPacketRaw(
						device, buffer, bufferlen,
						&bytesRecvd, &requestId);
		if (ret == 0) {
			if (bytesRecvd > 0) {
				DPRINT_DBG(NETVSC, "receive %d bytes, tid %llx",
					   bytesRecvd, requestId);

				desc = (struct vmpacket_descriptor *)buffer;
				switch (desc->Type) {
				case VmbusPacketTypeCompletion:
					NetVscOnSendCompletion(device, desc);
					break;

				case VmbusPacketTypeDataUsingTransferPages:
					NetVscOnReceive(device, desc);
					break;

				default:
					DPRINT_ERR(NETVSC,
						   "unhandled packet type %d, "
						   "tid %llx len %d\n",
						   desc->Type, requestId,
						   bytesRecvd);
					break;
				}

				
				if (bufferlen > netPacketSize) {
					kfree(buffer);
					buffer = packet;
					bufferlen = netPacketSize;
				}
			} else {
				
				if (bufferlen > netPacketSize) {
					kfree(buffer);
					buffer = packet;
					bufferlen = netPacketSize;
				}

				break;
			}
		} else if (ret == -2) {
			
			buffer = kmalloc(bytesRecvd, GFP_ATOMIC);
			if (buffer == NULL) {
				
				DPRINT_ERR(NETVSC,
					   "unable to allocate buffer of size "
					   "(%d)!!", bytesRecvd);
				break;
			}

			bufferlen = bytesRecvd;
		} else {
			ASSERT(0);
		}
	} while (1);

	PutNetDevice(device);
	DPRINT_EXIT(NETVSC);
	return;
}
