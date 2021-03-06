
#include <linux/kernel.h>
#include <linux/mm.h>
#include "osd.h"
#include "logging.h"
#include "VersionInfo.h"
#include "VmbusPrivate.h"

static const char *gDriverName = "vmbus";



static const struct hv_guid gVmbusDeviceType = {
	.data = {
		0x16, 0x58, 0x29, 0xc5, 0x3a, 0xf6, 0x5f, 0x4d,
		0x8d, 0x1a, 0x4d, 0xaf, 0x99, 0x9c, 0xa1, 0x85
	}
};


static const struct hv_guid gVmbusDeviceId = {
	.data = {
		0xfc, 0x60, 0x37, 0xac, 0xdf, 0x9a, 0xaa, 0x40,
		0x94, 0x27, 0xa7, 0x0e, 0xd6, 0xde, 0x95, 0xc5
	}
};

static struct hv_driver *gDriver; 
static struct hv_device *gDevice; 


static void VmbusGetChannelOffers(void)
{
	DPRINT_ENTER(VMBUS);
	VmbusChannelRequestOffers();
	DPRINT_EXIT(VMBUS);
}


static void VmbusGetChannelInterface(struct vmbus_channel_interface *Interface)
{
	GetChannelInterface(Interface);
}


static void VmbusGetChannelInfo(struct hv_device *DeviceObject,
				struct hv_device_info *DeviceInfo)
{
	GetChannelInfo(DeviceObject, DeviceInfo);
}


struct hv_device *VmbusChildDeviceCreate(struct hv_guid *DeviceType,
					 struct hv_guid *DeviceInstance,
					 void *Context)
{
	struct vmbus_driver *vmbusDriver = (struct vmbus_driver *)gDriver;

	return vmbusDriver->OnChildDeviceCreate(DeviceType, DeviceInstance,
						Context);
}


int VmbusChildDeviceAdd(struct hv_device *ChildDevice)
{
	struct vmbus_driver *vmbusDriver = (struct vmbus_driver *)gDriver;

	return vmbusDriver->OnChildDeviceAdd(gDevice, ChildDevice);
}


void VmbusChildDeviceRemove(struct hv_device *ChildDevice)
{
	struct vmbus_driver *vmbusDriver = (struct vmbus_driver *)gDriver;

	vmbusDriver->OnChildDeviceRemove(ChildDevice);
}


static int VmbusOnDeviceAdd(struct hv_device *dev, void *AdditionalInfo)
{
	u32 *irqvector = AdditionalInfo;
	int ret;

	DPRINT_ENTER(VMBUS);

	gDevice = dev;

	memcpy(&gDevice->deviceType, &gVmbusDeviceType, sizeof(struct hv_guid));
	memcpy(&gDevice->deviceInstance, &gVmbusDeviceId,
	       sizeof(struct hv_guid));

	
	
	on_each_cpu(HvSynicInit, (void *)irqvector, 1);

	
	ret = VmbusConnect();

	
	DPRINT_EXIT(VMBUS);

	return ret;
}


static int VmbusOnDeviceRemove(struct hv_device *dev)
{
	int ret = 0;

	DPRINT_ENTER(VMBUS);
	VmbusChannelReleaseUnattachedChannels();
	VmbusDisconnect();
	on_each_cpu(HvSynicCleanup, NULL, 1);
	DPRINT_EXIT(VMBUS);

	return ret;
}


static void VmbusOnCleanup(struct hv_driver *drv)
{
	

	DPRINT_ENTER(VMBUS);
	HvCleanup();
	DPRINT_EXIT(VMBUS);
}


static void VmbusOnMsgDPC(struct hv_driver *drv)
{
	int cpu = smp_processor_id();
	void *page_addr = gHvContext.synICMessagePage[cpu];
	struct hv_message *msg = (struct hv_message *)page_addr +
				  VMBUS_MESSAGE_SINT;
	struct hv_message *copied;

	while (1) {
		if (msg->Header.MessageType == HvMessageTypeNone) {
			
			break;
		} else {
			copied = kmalloc(sizeof(*copied), GFP_ATOMIC);
			if (copied == NULL)
				continue;

			memcpy(copied, msg, sizeof(*copied));
			osd_schedule_callback(gVmbusConnection.WorkQueue,
					      VmbusOnChannelMessage,
					      (void *)copied);
		}

		msg->Header.MessageType = HvMessageTypeNone;

		
		mb();

		if (msg->Header.MessageFlags.MessagePending) {
			
			wrmsrl(HV_X64_MSR_EOM, 0);
		}
	}
}


static void VmbusOnEventDPC(struct hv_driver *drv)
{
	
	VmbusOnEvents();
}


static int VmbusOnISR(struct hv_driver *drv)
{
	int ret = 0;
	int cpu = smp_processor_id();
	void *page_addr;
	struct hv_message *msg;
	union hv_synic_event_flags *event;

	page_addr = gHvContext.synICMessagePage[cpu];
	msg = (struct hv_message *)page_addr + VMBUS_MESSAGE_SINT;

	DPRINT_ENTER(VMBUS);

	
	if (msg->Header.MessageType != HvMessageTypeNone) {
		DPRINT_DBG(VMBUS, "received msg type %d size %d",
				msg->Header.MessageType,
				msg->Header.PayloadSize);
		ret |= 0x1;
	}

	
	page_addr = gHvContext.synICEventPage[cpu];
	event = (union hv_synic_event_flags *)page_addr + VMBUS_MESSAGE_SINT;

	
	if (test_and_clear_bit(0, (unsigned long *) &event->Flags32[0])) {
		DPRINT_DBG(VMBUS, "received event %d", event->Flags32[0]);
		ret |= 0x2;
	}

	DPRINT_EXIT(VMBUS);
	return ret;
}


int VmbusInitialize(struct hv_driver *drv)
{
	struct vmbus_driver *driver = (struct vmbus_driver *)drv;
	int ret;

	DPRINT_ENTER(VMBUS);

	DPRINT_INFO(VMBUS, "+++++++ Build Date=%s %s +++++++",
			VersionDate, VersionTime);
	DPRINT_INFO(VMBUS, "+++++++ Build Description=%s +++++++",
			VersionDesc);
	DPRINT_INFO(VMBUS, "+++++++ Vmbus supported version = %d +++++++",
			VMBUS_REVISION_NUMBER);
	DPRINT_INFO(VMBUS, "+++++++ Vmbus using SINT %d +++++++",
			VMBUS_MESSAGE_SINT);
	DPRINT_DBG(VMBUS, "sizeof(VMBUS_CHANNEL_PACKET_PAGE_BUFFER)=%zd, "
			"sizeof(VMBUS_CHANNEL_PACKET_MULITPAGE_BUFFER)=%zd",
			sizeof(struct VMBUS_CHANNEL_PACKET_PAGE_BUFFER),
			sizeof(struct VMBUS_CHANNEL_PACKET_MULITPAGE_BUFFER));

	drv->name = gDriverName;
	memcpy(&drv->deviceType, &gVmbusDeviceType, sizeof(struct hv_guid));

	
	driver->Base.OnDeviceAdd	= VmbusOnDeviceAdd;
	driver->Base.OnDeviceRemove	= VmbusOnDeviceRemove;
	driver->Base.OnCleanup		= VmbusOnCleanup;
	driver->OnIsr			= VmbusOnISR;
	driver->OnMsgDpc		= VmbusOnMsgDPC;
	driver->OnEventDpc		= VmbusOnEventDPC;
	driver->GetChannelOffers	= VmbusGetChannelOffers;
	driver->GetChannelInterface	= VmbusGetChannelInterface;
	driver->GetChannelInfo		= VmbusGetChannelInfo;

	
	ret = HvInit();
	if (ret != 0)
		DPRINT_ERR(VMBUS, "Unable to initialize the hypervisor - 0x%x",
				ret);
	gDriver = drv;

	DPRINT_EXIT(VMBUS);

	return ret;
}
