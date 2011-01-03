


#ifndef _NETVSC_API_H_
#define _NETVSC_API_H_

#include "VmbusApi.h"


#define NETVSC_DEVICE_RING_BUFFER_SIZE	(64*PAGE_SIZE)
#define HW_MACADDR_LEN			6


struct hv_netvsc_packet;


struct xferpage_packet {
	struct list_head ListEntry;

	
	u32 Count;
};


#define NETVSC_PACKET_MAXPAGE		4


struct hv_netvsc_packet {
	
	struct list_head ListEntry;

	struct hv_device *Device;
	bool IsDataPacket;

	
	struct xferpage_packet *XferPagePacket;

	union {
		struct{
			u64 ReceiveCompletionTid;
			void *ReceiveCompletionContext;
			void (*OnReceiveCompletion)(void *context);
		} Recv;
		struct{
			u64 SendCompletionTid;
			void *SendCompletionContext;
			void (*OnSendCompletion)(void *context);
		} Send;
	} Completion;

	
	void *Extension;

	u32 TotalDataBufferLength;
	
	u32 PageBufferCount;
	struct hv_page_buffer PageBuffers[NETVSC_PACKET_MAXPAGE];
};


struct netvsc_driver {
	
	
	struct hv_driver Base;

	u32 RingBufferSize;
	u32 RequestExtSize;

	
	u32 AdditionalRequestPageBufferCount;

	
	int (*OnReceiveCallback)(struct hv_device *dev,
				 struct hv_netvsc_packet *packet);
	void (*OnLinkStatusChanged)(struct hv_device *dev, u32 Status);

	
	int (*OnOpen)(struct hv_device *dev);
	int (*OnClose)(struct hv_device *dev);
	int (*OnSend)(struct hv_device *dev, struct hv_netvsc_packet *packet);

	void *Context;
};

struct netvsc_device_info {
    unsigned char MacAddr[6];
    bool LinkState;	
};


int NetVscInitialize(struct hv_driver *drv);

#endif 
