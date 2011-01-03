


#ifndef _CHANNEL_INTERFACE_H_
#define _CHANNEL_INTERFACE_H_

#include "VmbusApi.h"

void GetChannelInterface(struct vmbus_channel_interface *ChannelInterface);

void GetChannelInfo(struct hv_device *Device,
		    struct hv_device_info *DeviceInfo);

#endif 
