


#ifndef _VMBUS_PRIVATE_H_
#define _VMBUS_PRIVATE_H_

#include "Hv.h"
#include "VmbusApi.h"
#include "Channel.h"
#include "ChannelMgmt.h"
#include "ChannelInterface.h"
#include "RingBuffer.h"
#include <linux/list.h>



#define MAX_NUM_CHANNELS	((PAGE_SIZE >> 1) << 3)	



#define MAX_NUM_CHANNELS_SUPPORTED	256


enum VMBUS_CONNECT_STATE {
	Disconnected,
	Connecting,
	Connected,
	Disconnecting
};

#define MAX_SIZE_CHANNEL_MESSAGE	HV_MESSAGE_PAYLOAD_BYTE_COUNT

struct VMBUS_CONNECTION {
	enum VMBUS_CONNECT_STATE ConnectState;

	atomic_t NextGpadlHandle;

	
	void *InterruptPage;
	void *SendInterruptPage;
	void *RecvInterruptPage;

	
	void *MonitorPages;
	struct list_head ChannelMsgList;
	spinlock_t channelmsg_lock;

	
	struct list_head ChannelList;
	spinlock_t channel_lock;

	struct workqueue_struct *WorkQueue;
};


struct VMBUS_MSGINFO {
	
	struct list_head MsgListEntry;

	
	struct osd_waitevent *WaitEvent;

	
	unsigned char Msg[0];
};


extern struct VMBUS_CONNECTION gVmbusConnection;



struct hv_device *VmbusChildDeviceCreate(struct hv_guid *deviceType,
					 struct hv_guid *deviceInstance,
					 void *context);

int VmbusChildDeviceAdd(struct hv_device *Device);

void VmbusChildDeviceRemove(struct hv_device *Device);





struct vmbus_channel *GetChannelFromRelId(u32 relId);




int VmbusConnect(void);

int VmbusDisconnect(void);

int VmbusPostMessage(void *buffer, size_t bufSize);

int VmbusSetEvent(u32 childRelId);

void VmbusOnEvents(void);


#endif 
