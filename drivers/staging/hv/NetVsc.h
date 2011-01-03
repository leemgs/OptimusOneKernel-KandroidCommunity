


#ifndef _NETVSC_H_
#define _NETVSC_H_

#include <linux/list.h>
#include "VmbusPacketFormat.h"
#include "VmbusChannelInterface.h"
#include "NetVscApi.h"


#define NVSP_INVALID_PROTOCOL_VERSION	((u32)0xFFFFFFFF)

#define NVSP_PROTOCOL_VERSION_1		2
#define NVSP_MIN_PROTOCOL_VERSION	NVSP_PROTOCOL_VERSION_1
#define NVSP_MAX_PROTOCOL_VERSION	NVSP_PROTOCOL_VERSION_1

enum {
	NvspMessageTypeNone = 0,

	
	NvspMessageTypeInit			= 1,
	NvspMessageTypeInitComplete		= 2,

	NvspVersionMessageStart			= 100,

	
	NvspMessage1TypeSendNdisVersion		= NvspVersionMessageStart,

	NvspMessage1TypeSendReceiveBuffer,
	NvspMessage1TypeSendReceiveBufferComplete,
	NvspMessage1TypeRevokeReceiveBuffer,

	NvspMessage1TypeSendSendBuffer,
	NvspMessage1TypeSendSendBufferComplete,
	NvspMessage1TypeRevokeSendBuffer,

	NvspMessage1TypeSendRNDISPacket,
	NvspMessage1TypeSendRNDISPacketComplete,

	
	NvspNumMessagePerVersion		= 9,
};

enum {
	NvspStatusNone = 0,
	NvspStatusSuccess,
	NvspStatusFailure,
	NvspStatusProtocolVersionRangeTooNew,
	NvspStatusProtocolVersionRangeTooOld,
	NvspStatusInvalidRndisPacket,
	NvspStatusBusy,
	NvspStatusMax,
};

struct nvsp_message_header {
	u32 MessageType;
};




struct nvsp_message_init {
	u32 MinProtocolVersion;
	u32 MaxProtocolVersion;
} __attribute__((packed));


struct nvsp_message_init_complete {
	u32 NegotiatedProtocolVersion;
	u32 MaximumMdlChainLength;
	u32 Status;
} __attribute__((packed));

union nvsp_message_init_uber {
	struct nvsp_message_init Init;
	struct nvsp_message_init_complete InitComplete;
} __attribute__((packed));




struct nvsp_1_message_send_ndis_version {
	u32 NdisMajorVersion;
	u32 NdisMinorVersion;
} __attribute__((packed));


struct nvsp_1_message_send_receive_buffer {
	u32 GpadlHandle;
	u16 Id;
} __attribute__((packed));

struct nvsp_1_receive_buffer_section {
	u32 Offset;
	u32 SubAllocationSize;
	u32 NumSubAllocations;
	u32 EndOffset;
} __attribute__((packed));


struct nvsp_1_message_send_receive_buffer_complete {
	u32 Status;
	u32 NumSections;

	

	

	

	struct nvsp_1_receive_buffer_section Sections[1];
} __attribute__((packed));


struct nvsp_1_message_revoke_receive_buffer {
	u16 Id;
};


struct nvsp_1_message_send_send_buffer {
	u32 GpadlHandle;
	u16 Id;
} __attribute__((packed));


struct nvsp_1_message_send_send_buffer_complete {
	u32 Status;

	
	u32 SectionSize;
} __attribute__((packed));


struct nvsp_1_message_revoke_send_buffer {
	u16 Id;
};


struct nvsp_1_message_send_rndis_packet {
	
	u32 ChannelType;

	
	u32 SendBufferSectionIndex;
	u32 SendBufferSectionSize;
} __attribute__((packed));


struct nvsp_1_message_send_rndis_packet_complete {
	u32 Status;
};

union nvsp_1_message_uber {
	struct nvsp_1_message_send_ndis_version SendNdisVersion;

	struct nvsp_1_message_send_receive_buffer SendReceiveBuffer;
	struct nvsp_1_message_send_receive_buffer_complete
						SendReceiveBufferComplete;
	struct nvsp_1_message_revoke_receive_buffer RevokeReceiveBuffer;

	struct nvsp_1_message_send_send_buffer SendSendBuffer;
	struct nvsp_1_message_send_send_buffer_complete SendSendBufferComplete;
	struct nvsp_1_message_revoke_send_buffer RevokeSendBuffer;

	struct nvsp_1_message_send_rndis_packet SendRNDISPacket;
	struct nvsp_1_message_send_rndis_packet_complete
						SendRNDISPacketComplete;
} __attribute__((packed));

union nvsp_all_messages {
	union nvsp_message_init_uber InitMessages;
	union nvsp_1_message_uber Version1Messages;
} __attribute__((packed));


struct nvsp_message {
	struct nvsp_message_header Header;
	union nvsp_all_messages Messages;
} __attribute__((packed));







#define NETVSC_SEND_BUFFER_SIZE			(64*1024)	
#define NETVSC_SEND_BUFFER_ID			0xface


#define NETVSC_RECEIVE_BUFFER_SIZE		(1024*1024)	

#define NETVSC_RECEIVE_BUFFER_ID		0xcafe

#define NETVSC_RECEIVE_SG_COUNT			1


#define NETVSC_RECEIVE_PACKETLIST_COUNT		256



struct netvsc_device {
	struct hv_device *Device;

	atomic_t RefCount;
	atomic_t NumOutstandingSends;
	
	struct list_head ReceivePacketList;
	spinlock_t receive_packet_list_lock;

	
	void *SendBuffer;
	u32 SendBufferSize;
	u32 SendBufferGpadlHandle;
	u32 SendSectionSize;

	
	void *ReceiveBuffer;
	u32 ReceiveBufferSize;
	u32 ReceiveBufferGpadlHandle;
	u32 ReceiveSectionCount;
	struct nvsp_1_receive_buffer_section *ReceiveSections;

	
	struct osd_waitevent *ChannelInitEvent;
	struct nvsp_message ChannelInitPacket;

	struct nvsp_message RevokePacket;
	

	
	void *Extension;
};

#endif 
