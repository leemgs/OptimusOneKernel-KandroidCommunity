
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/delay.h>
#include "osd.h"
#include "logging.h"
#include "StorVscApi.h"
#include "VmbusPacketFormat.h"
#include "vstorage.h"


struct storvsc_request_extension {
	

	struct hv_storvsc_request *Request;
	struct hv_device *Device;

	
	struct osd_waitevent *WaitEvent;

	struct vstor_packet VStorPacket;
};


struct storvsc_device {
	struct hv_device *Device;

	
	atomic_t RefCount;

	atomic_t NumOutstandingRequests;

	
	unsigned int PortNumber;
	unsigned char PathId;
	unsigned char TargetId;

	
	

	
	struct storvsc_request_extension InitRequest;
	struct storvsc_request_extension ResetRequest;
};


static const char *gDriverName = "storvsc";


static const struct hv_guid gStorVscDeviceType = {
	.data = {
		0xd9, 0x63, 0x61, 0xba, 0xa1, 0x04, 0x29, 0x4d,
		0xb6, 0x05, 0x72, 0xe2, 0xff, 0xb1, 0xdc, 0x7f
	}
};


static inline struct storvsc_device *AllocStorDevice(struct hv_device *Device)
{
	struct storvsc_device *storDevice;

	storDevice = kzalloc(sizeof(struct storvsc_device), GFP_KERNEL);
	if (!storDevice)
		return NULL;

	
	
	atomic_cmpxchg(&storDevice->RefCount, 0, 2);

	storDevice->Device = Device;
	Device->Extension = storDevice;

	return storDevice;
}

static inline void FreeStorDevice(struct storvsc_device *Device)
{
	ASSERT(atomic_read(&Device->RefCount) == 0);
	kfree(Device);
}


static inline struct storvsc_device *GetStorDevice(struct hv_device *Device)
{
	struct storvsc_device *storDevice;

	storDevice = (struct storvsc_device *)Device->Extension;
	if (storDevice && atomic_read(&storDevice->RefCount) > 1)
		atomic_inc(&storDevice->RefCount);
	else
		storDevice = NULL;

	return storDevice;
}


static inline struct storvsc_device *MustGetStorDevice(struct hv_device *Device)
{
	struct storvsc_device *storDevice;

	storDevice = (struct storvsc_device *)Device->Extension;
	if (storDevice && atomic_read(&storDevice->RefCount))
		atomic_inc(&storDevice->RefCount);
	else
		storDevice = NULL;

	return storDevice;
}

static inline void PutStorDevice(struct hv_device *Device)
{
	struct storvsc_device *storDevice;

	storDevice = (struct storvsc_device *)Device->Extension;
	ASSERT(storDevice);

	atomic_dec(&storDevice->RefCount);
	ASSERT(atomic_read(&storDevice->RefCount));
}


static inline struct storvsc_device *ReleaseStorDevice(struct hv_device *Device)
{
	struct storvsc_device *storDevice;

	storDevice = (struct storvsc_device *)Device->Extension;
	ASSERT(storDevice);

	
	while (atomic_cmpxchg(&storDevice->RefCount, 2, 1) != 2)
		udelay(100);

	return storDevice;
}


static inline struct storvsc_device *FinalReleaseStorDevice(
			struct hv_device *Device)
{
	struct storvsc_device *storDevice;

	storDevice = (struct storvsc_device *)Device->Extension;
	ASSERT(storDevice);

	
	while (atomic_cmpxchg(&storDevice->RefCount, 1, 0) != 1)
		udelay(100);

	Device->Extension = NULL;
	return storDevice;
}

static int StorVscChannelInit(struct hv_device *Device)
{
	struct storvsc_device *storDevice;
	struct storvsc_request_extension *request;
	struct vstor_packet *vstorPacket;
	int ret;

	storDevice = GetStorDevice(Device);
	if (!storDevice) {
		DPRINT_ERR(STORVSC, "unable to get stor device..."
			   "device being destroyed?");
		DPRINT_EXIT(STORVSC);
		return -1;
	}

	request = &storDevice->InitRequest;
	vstorPacket = &request->VStorPacket;

	
	memset(request, 0, sizeof(struct storvsc_request_extension));
	request->WaitEvent = osd_WaitEventCreate();

	vstorPacket->Operation = VStorOperationBeginInitialization;
	vstorPacket->Flags = REQUEST_COMPLETION_FLAG;

	

	DPRINT_INFO(STORVSC, "BEGIN_INITIALIZATION_OPERATION...");

	ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
				vstorPacket,
				sizeof(struct vstor_packet),
				(unsigned long)request,
				VmbusPacketTypeDataInBand,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0) {
		DPRINT_ERR(STORVSC,
			   "unable to send BEGIN_INITIALIZATION_OPERATION");
		goto Cleanup;
	}

	osd_WaitEventWait(request->WaitEvent);

	if (vstorPacket->Operation != VStorOperationCompleteIo ||
	    vstorPacket->Status != 0) {
		DPRINT_ERR(STORVSC, "BEGIN_INITIALIZATION_OPERATION failed "
			   "(op %d status 0x%x)",
			   vstorPacket->Operation, vstorPacket->Status);
		goto Cleanup;
	}

	DPRINT_INFO(STORVSC, "QUERY_PROTOCOL_VERSION_OPERATION...");

	
	memset(vstorPacket, 0, sizeof(struct vstor_packet));
	vstorPacket->Operation = VStorOperationQueryProtocolVersion;
	vstorPacket->Flags = REQUEST_COMPLETION_FLAG;

	vstorPacket->Version.MajorMinor = VMSTOR_PROTOCOL_VERSION_CURRENT;
	FILL_VMSTOR_REVISION(vstorPacket->Version.Revision);

	ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
				vstorPacket,
				sizeof(struct vstor_packet),
				(unsigned long)request,
				VmbusPacketTypeDataInBand,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0) {
		DPRINT_ERR(STORVSC,
			   "unable to send BEGIN_INITIALIZATION_OPERATION");
		goto Cleanup;
	}

	osd_WaitEventWait(request->WaitEvent);

	
	if (vstorPacket->Operation != VStorOperationCompleteIo ||
	    vstorPacket->Status != 0) {
		DPRINT_ERR(STORVSC, "QUERY_PROTOCOL_VERSION_OPERATION failed "
			   "(op %d status 0x%x)",
			   vstorPacket->Operation, vstorPacket->Status);
		goto Cleanup;
	}

	
	DPRINT_INFO(STORVSC, "QUERY_PROPERTIES_OPERATION...");

	memset(vstorPacket, 0, sizeof(struct vstor_packet));
	vstorPacket->Operation = VStorOperationQueryProperties;
	vstorPacket->Flags = REQUEST_COMPLETION_FLAG;
	vstorPacket->StorageChannelProperties.PortNumber =
					storDevice->PortNumber;

	ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
				vstorPacket,
				sizeof(struct vstor_packet),
				(unsigned long)request,
				VmbusPacketTypeDataInBand,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	if (ret != 0) {
		DPRINT_ERR(STORVSC,
			   "unable to send QUERY_PROPERTIES_OPERATION");
		goto Cleanup;
	}

	osd_WaitEventWait(request->WaitEvent);

	
	if (vstorPacket->Operation != VStorOperationCompleteIo ||
	    vstorPacket->Status != 0) {
		DPRINT_ERR(STORVSC, "QUERY_PROPERTIES_OPERATION failed "
			   "(op %d status 0x%x)",
			   vstorPacket->Operation, vstorPacket->Status);
		goto Cleanup;
	}

	storDevice->PathId = vstorPacket->StorageChannelProperties.PathId;
	storDevice->TargetId = vstorPacket->StorageChannelProperties.TargetId;

	DPRINT_DBG(STORVSC, "channel flag 0x%x, max xfer len 0x%x",
		   vstorPacket->StorageChannelProperties.Flags,
		   vstorPacket->StorageChannelProperties.MaxTransferBytes);

	DPRINT_INFO(STORVSC, "END_INITIALIZATION_OPERATION...");

	memset(vstorPacket, 0, sizeof(struct vstor_packet));
	vstorPacket->Operation = VStorOperationEndInitialization;
	vstorPacket->Flags = REQUEST_COMPLETION_FLAG;

	ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
				vstorPacket,
				sizeof(struct vstor_packet),
				(unsigned long)request,
				VmbusPacketTypeDataInBand,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);

	if (ret != 0) {
		DPRINT_ERR(STORVSC,
			   "unable to send END_INITIALIZATION_OPERATION");
		goto Cleanup;
	}

	osd_WaitEventWait(request->WaitEvent);

	if (vstorPacket->Operation != VStorOperationCompleteIo ||
	    vstorPacket->Status != 0) {
		DPRINT_ERR(STORVSC, "END_INITIALIZATION_OPERATION failed "
			   "(op %d status 0x%x)",
			   vstorPacket->Operation, vstorPacket->Status);
		goto Cleanup;
	}

	DPRINT_INFO(STORVSC, "**** storage channel up and running!! ****");

Cleanup:
	kfree(request->WaitEvent);
	request->WaitEvent = NULL;

	PutStorDevice(Device);

	DPRINT_EXIT(STORVSC);
	return ret;
}

static void StorVscOnIOCompletion(struct hv_device *Device,
				  struct vstor_packet *VStorPacket,
				  struct storvsc_request_extension *RequestExt)
{
	struct hv_storvsc_request *request;
	struct storvsc_device *storDevice;

	DPRINT_ENTER(STORVSC);

	storDevice = MustGetStorDevice(Device);
	if (!storDevice) {
		DPRINT_ERR(STORVSC, "unable to get stor device..."
			   "device being destroyed?");
		DPRINT_EXIT(STORVSC);
		return;
	}

	DPRINT_DBG(STORVSC, "IO_COMPLETE_OPERATION - request extension %p "
		   "completed bytes xfer %u", RequestExt,
		   VStorPacket->VmSrb.DataTransferLength);

	ASSERT(RequestExt != NULL);
	ASSERT(RequestExt->Request != NULL);

	request = RequestExt->Request;

	ASSERT(request->OnIOCompletion != NULL);

	
	request->Status = VStorPacket->VmSrb.ScsiStatus;

	if (request->Status != 0 || VStorPacket->VmSrb.SrbStatus != 1) {
		DPRINT_WARN(STORVSC,
			    "cmd 0x%x scsi status 0x%x srb status 0x%x\n",
			    request->Cdb[0], VStorPacket->VmSrb.ScsiStatus,
			    VStorPacket->VmSrb.SrbStatus);
	}

	if ((request->Status & 0xFF) == 0x02) {
		
		if (VStorPacket->VmSrb.SrbStatus & 0x80) {
			
			DPRINT_WARN(STORVSC, "storvsc pkt %p autosense data "
				    "valid - len %d\n", RequestExt,
				    VStorPacket->VmSrb.SenseInfoLength);

			ASSERT(VStorPacket->VmSrb.SenseInfoLength <=
				request->SenseBufferSize);
			memcpy(request->SenseBuffer,
			       VStorPacket->VmSrb.SenseData,
			       VStorPacket->VmSrb.SenseInfoLength);

			request->SenseBufferSize =
					VStorPacket->VmSrb.SenseInfoLength;
		}
	}

	
	request->BytesXfer = VStorPacket->VmSrb.DataTransferLength;

	request->OnIOCompletion(request);

	atomic_dec(&storDevice->NumOutstandingRequests);

	PutStorDevice(Device);

	DPRINT_EXIT(STORVSC);
}

static void StorVscOnReceive(struct hv_device *Device,
			     struct vstor_packet *VStorPacket,
			     struct storvsc_request_extension *RequestExt)
{
	switch (VStorPacket->Operation) {
	case VStorOperationCompleteIo:
		DPRINT_DBG(STORVSC, "IO_COMPLETE_OPERATION");
		StorVscOnIOCompletion(Device, VStorPacket, RequestExt);
		break;
	case VStorOperationRemoveDevice:
		DPRINT_INFO(STORVSC, "REMOVE_DEVICE_OPERATION");
		
		break;

	default:
		DPRINT_INFO(STORVSC, "Unknown operation received - %d",
			    VStorPacket->Operation);
		break;
	}
}

static void StorVscOnChannelCallback(void *context)
{
	struct hv_device *device = (struct hv_device *)context;
	struct storvsc_device *storDevice;
	u32 bytesRecvd;
	u64 requestId;
	unsigned char packet[ALIGN_UP(sizeof(struct vstor_packet), 8)];
	struct storvsc_request_extension *request;
	int ret;

	DPRINT_ENTER(STORVSC);

	ASSERT(device);

	storDevice = MustGetStorDevice(device);
	if (!storDevice) {
		DPRINT_ERR(STORVSC, "unable to get stor device..."
			   "device being destroyed?");
		DPRINT_EXIT(STORVSC);
		return;
	}

	do {
		ret = device->Driver->VmbusChannelInterface.RecvPacket(device,
				packet,
				ALIGN_UP(sizeof(struct vstor_packet), 8),
				&bytesRecvd, &requestId);
		if (ret == 0 && bytesRecvd > 0) {
			DPRINT_DBG(STORVSC, "receive %d bytes - tid %llx",
				   bytesRecvd, requestId);

			

			request = (struct storvsc_request_extension *)
					(unsigned long)requestId;
			ASSERT(request);

			
			if ((request == &storDevice->InitRequest) ||
			    (request == &storDevice->ResetRequest)) {
				

				memcpy(&request->VStorPacket, packet,
				       sizeof(struct vstor_packet));

				osd_WaitEventSet(request->WaitEvent);
			} else {
				StorVscOnReceive(device,
						(struct vstor_packet *)packet,
						request);
			}
		} else {
			
			break;
		}
	} while (1);

	PutStorDevice(device);

	DPRINT_EXIT(STORVSC);
	return;
}

static int StorVscConnectToVsp(struct hv_device *Device)
{
	struct vmstorage_channel_properties props;
	struct storvsc_driver_object *storDriver;
	int ret;

	storDriver = (struct storvsc_driver_object *)Device->Driver;
	memset(&props, 0, sizeof(struct vmstorage_channel_properties));

	
	ret = Device->Driver->VmbusChannelInterface.Open(Device,
			storDriver->RingBufferSize,
			storDriver->RingBufferSize,
			(void *)&props,
			sizeof(struct vmstorage_channel_properties),
			StorVscOnChannelCallback,
			Device);

	DPRINT_DBG(STORVSC, "storage props: path id %d, tgt id %d, max xfer %d",
		   props.PathId, props.TargetId, props.MaxTransferBytes);

	if (ret != 0) {
		DPRINT_ERR(STORVSC, "unable to open channel: %d", ret);
		return -1;
	}

	ret = StorVscChannelInit(Device);

	return ret;
}


static int StorVscOnDeviceAdd(struct hv_device *Device, void *AdditionalInfo)
{
	struct storvsc_device *storDevice;
	
	struct storvsc_device_info *deviceInfo;
	int ret = 0;

	DPRINT_ENTER(STORVSC);

	deviceInfo = (struct storvsc_device_info *)AdditionalInfo;
	storDevice = AllocStorDevice(Device);
	if (!storDevice) {
		ret = -1;
		goto Cleanup;
	}

	
	

	
	

	

	storDevice->PortNumber = deviceInfo->PortNumber;
	
	ret = StorVscConnectToVsp(Device);

	
	deviceInfo->PathId = storDevice->PathId;
	deviceInfo->TargetId = storDevice->TargetId;

	DPRINT_DBG(STORVSC, "assigned port %u, path %u target %u\n",
		   storDevice->PortNumber, storDevice->PathId,
		   storDevice->TargetId);

Cleanup:
	DPRINT_EXIT(STORVSC);

	return ret;
}


static int StorVscOnDeviceRemove(struct hv_device *Device)
{
	struct storvsc_device *storDevice;

	DPRINT_ENTER(STORVSC);

	DPRINT_INFO(STORVSC, "disabling storage device (%p)...",
		    Device->Extension);

	storDevice = ReleaseStorDevice(Device);

	
	while (atomic_read(&storDevice->NumOutstandingRequests)) {
		DPRINT_INFO(STORVSC, "waiting for %d requests to complete...",
			    atomic_read(&storDevice->NumOutstandingRequests));
		udelay(100);
	}

	DPRINT_INFO(STORVSC, "removing storage device (%p)...",
		    Device->Extension);

	storDevice = FinalReleaseStorDevice(Device);

	DPRINT_INFO(STORVSC, "storage device (%p) safe to remove", storDevice);

	
	Device->Driver->VmbusChannelInterface.Close(Device);

	FreeStorDevice(storDevice);

	DPRINT_EXIT(STORVSC);
	return 0;
}

static int StorVscOnHostReset(struct hv_device *Device)
{
	struct storvsc_device *storDevice;
	struct storvsc_request_extension *request;
	struct vstor_packet *vstorPacket;
	int ret;

	DPRINT_ENTER(STORVSC);

	DPRINT_INFO(STORVSC, "resetting host adapter...");

	storDevice = GetStorDevice(Device);
	if (!storDevice) {
		DPRINT_ERR(STORVSC, "unable to get stor device..."
			   "device being destroyed?");
		DPRINT_EXIT(STORVSC);
		return -1;
	}

	request = &storDevice->ResetRequest;
	vstorPacket = &request->VStorPacket;

	request->WaitEvent = osd_WaitEventCreate();

	vstorPacket->Operation = VStorOperationResetBus;
	vstorPacket->Flags = REQUEST_COMPLETION_FLAG;
	vstorPacket->VmSrb.PathId = storDevice->PathId;

	ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
				vstorPacket,
				sizeof(struct vstor_packet),
				(unsigned long)&storDevice->ResetRequest,
				VmbusPacketTypeDataInBand,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	if (ret != 0) {
		DPRINT_ERR(STORVSC, "Unable to send reset packet %p ret %d",
			   vstorPacket, ret);
		goto Cleanup;
	}

	
	osd_WaitEventWait(request->WaitEvent);

	kfree(request->WaitEvent);
	DPRINT_INFO(STORVSC, "host adapter reset completed");

	

Cleanup:
	PutStorDevice(Device);
	DPRINT_EXIT(STORVSC);
	return ret;
}


static int StorVscOnIORequest(struct hv_device *Device,
			      struct hv_storvsc_request *Request)
{
	struct storvsc_device *storDevice;
	struct storvsc_request_extension *requestExtension;
	struct vstor_packet *vstorPacket;
	int ret = 0;

	DPRINT_ENTER(STORVSC);

	requestExtension =
		(struct storvsc_request_extension *)Request->Extension;
	vstorPacket = &requestExtension->VStorPacket;
	storDevice = GetStorDevice(Device);

	DPRINT_DBG(STORVSC, "enter - Device %p, DeviceExt %p, Request %p, "
		   "Extension %p", Device, storDevice, Request,
		   requestExtension);

	DPRINT_DBG(STORVSC, "req %p len %d bus %d, target %d, lun %d cdblen %d",
		   Request, Request->DataBuffer.Length, Request->Bus,
		   Request->TargetId, Request->LunId, Request->CdbLen);

	if (!storDevice) {
		DPRINT_ERR(STORVSC, "unable to get stor device..."
			   "device being destroyed?");
		DPRINT_EXIT(STORVSC);
		return -2;
	}

	

	requestExtension->Request = Request;
	requestExtension->Device  = Device;

	memset(vstorPacket, 0 , sizeof(struct vstor_packet));

	vstorPacket->Flags |= REQUEST_COMPLETION_FLAG;

	vstorPacket->VmSrb.Length = sizeof(struct vmscsi_request);

	vstorPacket->VmSrb.PortNumber = Request->Host;
	vstorPacket->VmSrb.PathId = Request->Bus;
	vstorPacket->VmSrb.TargetId = Request->TargetId;
	vstorPacket->VmSrb.Lun = Request->LunId;

	vstorPacket->VmSrb.SenseInfoLength = SENSE_BUFFER_SIZE;

	
	vstorPacket->VmSrb.CdbLength = Request->CdbLen;
	memcpy(&vstorPacket->VmSrb.Cdb, Request->Cdb, Request->CdbLen);

	vstorPacket->VmSrb.DataIn = Request->Type;
	vstorPacket->VmSrb.DataTransferLength = Request->DataBuffer.Length;

	vstorPacket->Operation = VStorOperationExecuteSRB;

	DPRINT_DBG(STORVSC, "srb - len %d port %d, path %d, target %d, "
		   "lun %d senselen %d cdblen %d",
		   vstorPacket->VmSrb.Length,
		   vstorPacket->VmSrb.PortNumber,
		   vstorPacket->VmSrb.PathId,
		   vstorPacket->VmSrb.TargetId,
		   vstorPacket->VmSrb.Lun,
		   vstorPacket->VmSrb.SenseInfoLength,
		   vstorPacket->VmSrb.CdbLength);

	if (requestExtension->Request->DataBuffer.Length) {
		ret = Device->Driver->VmbusChannelInterface.
			SendPacketMultiPageBuffer(Device,
				&requestExtension->Request->DataBuffer,
				vstorPacket,
				sizeof(struct vstor_packet),
				(unsigned long)requestExtension);
	} else {
		ret = Device->Driver->VmbusChannelInterface.SendPacket(Device,
				vstorPacket,
				sizeof(struct vstor_packet),
				(unsigned long)requestExtension,
				VmbusPacketTypeDataInBand,
				VMBUS_DATA_PACKET_FLAG_COMPLETION_REQUESTED);
	}

	if (ret != 0) {
		DPRINT_DBG(STORVSC, "Unable to send packet %p ret %d",
			   vstorPacket, ret);
	}

	atomic_inc(&storDevice->NumOutstandingRequests);

	PutStorDevice(Device);

	DPRINT_EXIT(STORVSC);
	return ret;
}


static void StorVscOnCleanup(struct hv_driver *Driver)
{
	DPRINT_ENTER(STORVSC);
	DPRINT_EXIT(STORVSC);
}


int StorVscInitialize(struct hv_driver *Driver)
{
	struct storvsc_driver_object *storDriver;

	DPRINT_ENTER(STORVSC);

	storDriver = (struct storvsc_driver_object *)Driver;

	DPRINT_DBG(STORVSC, "sizeof(STORVSC_REQUEST)=%zd "
		   "sizeof(struct storvsc_request_extension)=%zd "
		   "sizeof(struct vstor_packet)=%zd, "
		   "sizeof(struct vmscsi_request)=%zd",
		   sizeof(struct hv_storvsc_request),
		   sizeof(struct storvsc_request_extension),
		   sizeof(struct vstor_packet),
		   sizeof(struct vmscsi_request));

	
	ASSERT(storDriver->RingBufferSize >= (PAGE_SIZE << 1));

	Driver->name = gDriverName;
	memcpy(&Driver->deviceType, &gStorVscDeviceType,
	       sizeof(struct hv_guid));

	storDriver->RequestExtSize = sizeof(struct storvsc_request_extension);

	
	storDriver->MaxOutstandingRequestsPerChannel =
		((storDriver->RingBufferSize - PAGE_SIZE) /
		  ALIGN_UP(MAX_MULTIPAGE_BUFFER_PACKET +
			   sizeof(struct vstor_packet) + sizeof(u64),
			   sizeof(u64)));

	DPRINT_INFO(STORVSC, "max io %u, currently %u\n",
		    storDriver->MaxOutstandingRequestsPerChannel,
		    STORVSC_MAX_IO_REQUESTS);

	
	storDriver->Base.OnDeviceAdd	= StorVscOnDeviceAdd;
	storDriver->Base.OnDeviceRemove	= StorVscOnDeviceRemove;
	storDriver->Base.OnCleanup	= StorVscOnCleanup;

	storDriver->OnIORequest		= StorVscOnIORequest;
	storDriver->OnHostReset		= StorVscOnHostReset;

	DPRINT_EXIT(STORVSC);

	return 0;
}
