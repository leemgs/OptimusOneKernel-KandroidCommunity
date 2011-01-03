


#ifndef _CHANNEL_MGMT_H_
#define _CHANNEL_MGMT_H_

#include <linux/list.h>
#include <linux/timer.h>
#include "RingBuffer.h"
#include "VmbusChannelInterface.h"
#include "VmbusPacketFormat.h"


enum vmbus_channel_message_type {
	ChannelMessageInvalid			=  0,
	ChannelMessageOfferChannel		=  1,
	ChannelMessageRescindChannelOffer	=  2,
	ChannelMessageRequestOffers		=  3,
	ChannelMessageAllOffersDelivered	=  4,
	ChannelMessageOpenChannel		=  5,
	ChannelMessageOpenChannelResult		=  6,
	ChannelMessageCloseChannel		=  7,
	ChannelMessageGpadlHeader		=  8,
	ChannelMessageGpadlBody			=  9,
	ChannelMessageGpadlCreated		= 10,
	ChannelMessageGpadlTeardown		= 11,
	ChannelMessageGpadlTorndown		= 12,
	ChannelMessageRelIdReleased		= 13,
	ChannelMessageInitiateContact		= 14,
	ChannelMessageVersionResponse		= 15,
	ChannelMessageUnload			= 16,
#ifdef VMBUS_FEATURE_PARENT_OR_PEER_MEMORY_MAPPED_INTO_A_CHILD
	ChannelMessageViewRangeAdd		= 17,
	ChannelMessageViewRangeRemove		= 18,
#endif
	ChannelMessageCount
};

struct vmbus_channel_message_header {
	enum vmbus_channel_message_type MessageType;
	u32 Padding;
} __attribute__((packed));


struct vmbus_channel_query_vmbus_version {
	struct vmbus_channel_message_header Header;
	u32 Version;
} __attribute__((packed));


struct vmbus_channel_version_supported {
	struct vmbus_channel_message_header Header;
	bool VersionSupported;
} __attribute__((packed));


struct vmbus_channel_offer_channel {
	struct vmbus_channel_message_header Header;
	struct vmbus_channel_offer Offer;
	u32 ChildRelId;
	u8 MonitorId;
	bool MonitorAllocated;
} __attribute__((packed));


struct vmbus_channel_rescind_offer {
	struct vmbus_channel_message_header Header;
	u32 ChildRelId;
} __attribute__((packed));




struct vmbus_channel_open_channel {
	struct vmbus_channel_message_header Header;

	
	u32 ChildRelId;

	
	u32 OpenId;

	
	u32 RingBufferGpadlHandle;

	
	u32 ServerContextAreaGpadlHandle;

	
	u32 DownstreamRingBufferPageOffset;

	
	unsigned char UserData[MAX_USER_DEFINED_BYTES];
} __attribute__((packed));


struct vmbus_channel_open_result {
	struct vmbus_channel_message_header Header;
	u32 ChildRelId;
	u32 OpenId;
	u32 Status;
} __attribute__((packed));


struct vmbus_channel_close_channel {
	struct vmbus_channel_message_header Header;
	u32 ChildRelId;
} __attribute__((packed));


#define GPADL_TYPE_RING_BUFFER		1
#define GPADL_TYPE_SERVER_SAVE_AREA	2
#define GPADL_TYPE_TRANSACTION		8


struct vmbus_channel_gpadl_header {
	struct vmbus_channel_message_header Header;
	u32 ChildRelId;
	u32 Gpadl;
	u16 RangeBufLen;
	u16 RangeCount;
	struct gpa_range Range[0];
} __attribute__((packed));


struct vmbus_channel_gpadl_body {
	struct vmbus_channel_message_header Header;
	u32 MessageNumber;
	u32 Gpadl;
	u64 Pfn[0];
} __attribute__((packed));

struct vmbus_channel_gpadl_created {
	struct vmbus_channel_message_header Header;
	u32 ChildRelId;
	u32 Gpadl;
	u32 CreationStatus;
} __attribute__((packed));

struct vmbus_channel_gpadl_teardown {
	struct vmbus_channel_message_header Header;
	u32 ChildRelId;
	u32 Gpadl;
} __attribute__((packed));

struct vmbus_channel_gpadl_torndown {
	struct vmbus_channel_message_header Header;
	u32 Gpadl;
} __attribute__((packed));

#ifdef VMBUS_FEATURE_PARENT_OR_PEER_MEMORY_MAPPED_INTO_A_CHILD
struct vmbus_channel_view_range_add {
	struct vmbus_channel_message_header Header;
	PHYSICAL_ADDRESS ViewRangeBase;
	u64 ViewRangeLength;
	u32 ChildRelId;
} __attribute__((packed));

struct vmbus_channel_view_range_remove {
	struct vmbus_channel_message_header Header;
	PHYSICAL_ADDRESS ViewRangeBase;
	u32 ChildRelId;
} __attribute__((packed));
#endif

struct vmbus_channel_relid_released {
	struct vmbus_channel_message_header Header;
	u32 ChildRelId;
} __attribute__((packed));

struct vmbus_channel_initiate_contact {
	struct vmbus_channel_message_header Header;
	u32 VMBusVersionRequested;
	u32 Padding2;
	u64 InterruptPage;
	u64 MonitorPage1;
	u64 MonitorPage2;
} __attribute__((packed));

struct vmbus_channel_version_response {
	struct vmbus_channel_message_header Header;
	bool VersionSupported;
} __attribute__((packed));

enum vmbus_channel_state {
	CHANNEL_OFFER_STATE,
	CHANNEL_OPENING_STATE,
	CHANNEL_OPEN_STATE,
};

struct vmbus_channel {
	struct list_head ListEntry;

	struct hv_device *DeviceObject;

	struct timer_list poll_timer; 

	enum vmbus_channel_state State;

	struct vmbus_channel_offer_channel OfferMsg;
	
	u8 MonitorGroup;
	u8 MonitorBit;

	u32 RingBufferGpadlHandle;

	
	void *RingBufferPages;
	u32 RingBufferPageCount;
	RING_BUFFER_INFO Outbound;	
	RING_BUFFER_INFO Inbound;	
	spinlock_t inbound_lock;
	struct workqueue_struct *ControlWQ;

	
	

	void (*OnChannelCallback)(void *context);
	void *ChannelCallbackContext;
};

struct vmbus_channel_debug_info {
	u32 RelId;
	enum vmbus_channel_state State;
	struct hv_guid InterfaceType;
	struct hv_guid InterfaceInstance;
	u32 MonitorId;
	u32 ServerMonitorPending;
	u32 ServerMonitorLatency;
	u32 ServerMonitorConnectionId;
	u32 ClientMonitorPending;
	u32 ClientMonitorLatency;
	u32 ClientMonitorConnectionId;

	RING_BUFFER_DEBUG_INFO Inbound;
	RING_BUFFER_DEBUG_INFO Outbound;
};


struct vmbus_channel_msginfo {
	
	struct list_head MsgListEntry;

	
	struct list_head SubMsgList;

	
	struct osd_waitevent *WaitEvent;

	union {
		struct vmbus_channel_version_supported VersionSupported;
		struct vmbus_channel_open_result OpenResult;
		struct vmbus_channel_gpadl_torndown GpadlTorndown;
		struct vmbus_channel_gpadl_created GpadlCreated;
		struct vmbus_channel_version_response VersionResponse;
	} Response;

	u32 MessageSize;
	
	unsigned char Msg[0];
};


struct vmbus_channel *AllocVmbusChannel(void);

void FreeVmbusChannel(struct vmbus_channel *Channel);

void VmbusOnChannelMessage(void *Context);

int VmbusChannelRequestOffers(void);

void VmbusChannelReleaseUnattachedChannels(void);

#endif 
