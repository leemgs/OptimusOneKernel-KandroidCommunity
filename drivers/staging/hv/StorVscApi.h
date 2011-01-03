


#ifndef _STORVSC_API_H_
#define _STORVSC_API_H_

#include "VmbusApi.h"


#define STORVSC_RING_BUFFER_SIZE			(10*PAGE_SIZE)
#define BLKVSC_RING_BUFFER_SIZE				(20*PAGE_SIZE)

#define STORVSC_MAX_IO_REQUESTS				64


#define STORVSC_MAX_LUNS_PER_TARGET			64
#define STORVSC_MAX_TARGETS				1
#define STORVSC_MAX_CHANNELS				1

struct hv_storvsc_request;


enum storvsc_request_type{
	WRITE_TYPE,
	READ_TYPE,
	UNKNOWN_TYPE,
};

struct hv_storvsc_request {
	enum storvsc_request_type Type;
	u32 Host;
	u32 Bus;
	u32 TargetId;
	u32 LunId;
	u8 *Cdb;
	u32 CdbLen;
	u32 Status;
	u32 BytesXfer;

	unsigned char *SenseBuffer;
	u32 SenseBufferSize;

	void *Context;

	void (*OnIOCompletion)(struct hv_storvsc_request *Request);

	
	void *Extension;

	struct hv_multipage_buffer DataBuffer;
};


struct storvsc_driver_object {
	
	
	struct hv_driver Base;

	
	u32 RingBufferSize;

	
	u32 RequestExtSize;

	
	u32 MaxOutstandingRequestsPerChannel;

	
	void (*OnHostRescan)(struct hv_device *Device);

	
	int (*OnIORequest)(struct hv_device *Device,
			   struct hv_storvsc_request *Request);
	int (*OnHostReset)(struct hv_device *Device);
};

struct storvsc_device_info {
	unsigned int PortNumber;
	unsigned char PathId;
	unsigned char TargetId;
};


int StorVscInitialize(struct hv_driver *driver);
int BlkVscInitialize(struct hv_driver *driver);

#endif 
