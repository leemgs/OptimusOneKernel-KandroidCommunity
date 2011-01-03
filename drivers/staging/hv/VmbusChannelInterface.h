

#ifndef __VMBUSCHANNELINTERFACE_H
#define __VMBUSCHANNELINTERFACE_H


#define VMBUS_REVISION_NUMBER		13


#define MAX_PIPE_DATA_PAYLOAD		(sizeof(u8) * 16384)


#define VMBUS_PIPE_TYPE_BYTE		0x00000000
#define VMBUS_PIPE_TYPE_MESSAGE		0x00000004


#define MAX_USER_DEFINED_BYTES		120


#define MAX_PIPE_USER_DEFINED_BYTES	116


struct vmbus_channel_offer {
	struct hv_guid InterfaceType;
	struct hv_guid InterfaceInstance;
	u64 InterruptLatencyIn100nsUnits;
	u32 InterfaceRevision;
	u32 ServerContextAreaSize;	
	u16 ChannelFlags;
	u16 MmioMegabytes;		

	union {
		
		struct {
			unsigned char UserDefined[MAX_USER_DEFINED_BYTES];
		} Standard;

		
		struct {
			u32  PipeMode;
			unsigned char UserDefined[MAX_PIPE_USER_DEFINED_BYTES];
		} Pipe;
	} u;
	u32 Padding;
} __attribute__((packed));


#define VMBUS_CHANNEL_ENUMERATE_DEVICE_INTERFACE	1
#define VMBUS_CHANNEL_SERVER_SUPPORTS_TRANSFER_PAGES	2
#define VMBUS_CHANNEL_SERVER_SUPPORTS_GPADLS		4
#define VMBUS_CHANNEL_NAMED_PIPE_MODE			0x10
#define VMBUS_CHANNEL_LOOPBACK_OFFER			0x100
#define VMBUS_CHANNEL_PARENT_OFFER			0x200
#define VMBUS_CHANNEL_REQUEST_MONITORED_NOTIFICATION	0x400

#endif
