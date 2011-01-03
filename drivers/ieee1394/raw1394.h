#ifndef IEEE1394_RAW1394_H
#define IEEE1394_RAW1394_H



#define RAW1394_KERNELAPI_VERSION 4


#define RAW1394_REQ_INITIALIZE    1


#define RAW1394_REQ_LIST_CARDS    2
#define RAW1394_REQ_SET_CARD      3


#define RAW1394_REQ_ASYNC_READ      100
#define RAW1394_REQ_ASYNC_WRITE     101
#define RAW1394_REQ_LOCK            102
#define RAW1394_REQ_LOCK64          103
#define RAW1394_REQ_ISO_SEND        104 
#define RAW1394_REQ_ASYNC_SEND      105
#define RAW1394_REQ_ASYNC_STREAM    106

#define RAW1394_REQ_ISO_LISTEN      200 
#define RAW1394_REQ_FCP_LISTEN      201
#define RAW1394_REQ_RESET_BUS       202
#define RAW1394_REQ_GET_ROM         203
#define RAW1394_REQ_UPDATE_ROM      204
#define RAW1394_REQ_ECHO            205
#define RAW1394_REQ_MODIFY_ROM      206

#define RAW1394_REQ_ARM_REGISTER    300
#define RAW1394_REQ_ARM_UNREGISTER  301
#define RAW1394_REQ_ARM_SET_BUF     302
#define RAW1394_REQ_ARM_GET_BUF     303

#define RAW1394_REQ_RESET_NOTIFY    400

#define RAW1394_REQ_PHYPACKET       500


#define RAW1394_REQ_BUS_RESET        10000
#define RAW1394_REQ_ISO_RECEIVE      10001
#define RAW1394_REQ_FCP_REQUEST      10002
#define RAW1394_REQ_ARM              10003
#define RAW1394_REQ_RAWISO_ACTIVITY  10004


#define RAW1394_ERROR_NONE        0
#define RAW1394_ERROR_COMPAT      (-1001)
#define RAW1394_ERROR_STATE_ORDER (-1002)
#define RAW1394_ERROR_GENERATION  (-1003)
#define RAW1394_ERROR_INVALID_ARG (-1004)
#define RAW1394_ERROR_MEMFAULT    (-1005)
#define RAW1394_ERROR_ALREADY     (-1006)

#define RAW1394_ERROR_EXCESSIVE   (-1020)
#define RAW1394_ERROR_UNTIDY_LEN  (-1021)

#define RAW1394_ERROR_SEND_ERROR  (-1100)
#define RAW1394_ERROR_ABORTED     (-1101)
#define RAW1394_ERROR_TIMEOUT     (-1102)


#define ARM_READ   1
#define ARM_WRITE  2
#define ARM_LOCK   4

#define RAW1394_LONG_RESET  0
#define RAW1394_SHORT_RESET 1


#define RAW1394_NOTIFY_OFF 0
#define RAW1394_NOTIFY_ON  1

#include <asm/types.h>

struct raw1394_request {
        __u32 type;
        __s32 error;
        __u32 misc;

        __u32 generation;
        __u32 length;

        __u64 address;

        __u64 tag;

        __u64 sendb;
        __u64 recvb;
};

struct raw1394_khost_list {
        __u32 nodes;
        __u8 name[32];
};

typedef struct arm_request {
        __u16           destination_nodeid;
        __u16           source_nodeid;
        __u64           destination_offset;
        __u8            tlabel;
        __u8            tcode;
        __u8            extended_transaction_code;
        __u32           generation;
        __u16           buffer_length;
        __u8            __user *buffer;
} *arm_request_t;

typedef struct arm_response {
        __s32           response_code;
        __u16           buffer_length;
        __u8            __user *buffer;
} *arm_response_t;

typedef struct arm_request_response {
        struct arm_request  __user *request;
        struct arm_response __user *response;
} *arm_request_response_t;


#include "ieee1394-ioctl.h"



struct raw1394_iso_packet_info {
	__u32 offset;
	__u16 len;
	__u16 cycle;   
	__u8  channel; 
	__u8  tag;
	__u8  sy;
};


struct raw1394_iso_packets {
	__u32 n_packets;
	struct raw1394_iso_packet_info __user *infos;
};

struct raw1394_iso_config {
	
	__u32 data_buf_size;

	
	__u32 buf_packets;

	
	__s32 channel;

	
	__u8 speed;

	
	__u8 dma_mode;

	
	__s32 irq_interval;
};


struct raw1394_iso_status {
	
	struct raw1394_iso_config config;

	
	__u32 n_packets;

	
	__u32 overflows;

	
	__s16 xmit_cycle;
};


struct raw1394_cycle_timer {
	
	__u32 cycle_timer;

	
	__u64 local_time;
};
#endif 
