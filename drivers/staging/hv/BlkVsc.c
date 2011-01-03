
#include <linux/kernel.h>
#include <linux/mm.h>
#include "osd.h"
#include "StorVsc.c"

static const char *gBlkDriverName = "blkvsc";


static const struct hv_guid gBlkVscDeviceType = {
	.data = {
		0x32, 0x26, 0x41, 0x32, 0xcb, 0x86, 0xa2, 0x44,
		0x9b, 0x5c, 0x50, 0xd1, 0x41, 0x73, 0x54, 0xf5
	}
};

static int BlkVscOnDeviceAdd(struct hv_device *Device, void *AdditionalInfo)
{
	struct storvsc_device_info *deviceInfo;
	int ret = 0;

	DPRINT_ENTER(BLKVSC);

	deviceInfo = (struct storvsc_device_info *)AdditionalInfo;

	ret = StorVscOnDeviceAdd(Device, AdditionalInfo);
	if (ret != 0) {
		DPRINT_EXIT(BLKVSC);
		return ret;
	}

	
	deviceInfo->PathId = Device->deviceInstance.data[3] << 24 |
			     Device->deviceInstance.data[2] << 16 |
			     Device->deviceInstance.data[1] << 8  |
			     Device->deviceInstance.data[0];

	deviceInfo->TargetId = Device->deviceInstance.data[5] << 8 |
			       Device->deviceInstance.data[4];

	DPRINT_EXIT(BLKVSC);

	return ret;
}

int BlkVscInitialize(struct hv_driver *Driver)
{
	struct storvsc_driver_object *storDriver;
	int ret = 0;

	DPRINT_ENTER(BLKVSC);

	storDriver = (struct storvsc_driver_object *)Driver;

	
	ASSERT(storDriver->RingBufferSize >= (PAGE_SIZE << 1));

	Driver->name = gBlkDriverName;
	memcpy(&Driver->deviceType, &gBlkVscDeviceType, sizeof(struct hv_guid));

	storDriver->RequestExtSize = sizeof(struct storvsc_request_extension);

	
	storDriver->MaxOutstandingRequestsPerChannel =
		((storDriver->RingBufferSize - PAGE_SIZE) /
		  ALIGN_UP(MAX_MULTIPAGE_BUFFER_PACKET +
			   sizeof(struct vstor_packet) + sizeof(u64),
			   sizeof(u64)));

	DPRINT_INFO(BLKVSC, "max io outstd %u",
		    storDriver->MaxOutstandingRequestsPerChannel);

	
	storDriver->Base.OnDeviceAdd = BlkVscOnDeviceAdd;
	storDriver->Base.OnDeviceRemove = StorVscOnDeviceRemove;
	storDriver->Base.OnCleanup = StorVscOnCleanup;
	storDriver->OnIORequest	= StorVscOnIORequest;

	DPRINT_EXIT(BLKVSC);

	return ret;
}
