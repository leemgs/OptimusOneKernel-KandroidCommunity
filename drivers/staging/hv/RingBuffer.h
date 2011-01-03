


#ifndef _RING_BUFFER_H_
#define _RING_BUFFER_H_

#include <linux/scatterlist.h>

typedef struct _RING_BUFFER {
	
	volatile u32 WriteIndex;

	
	volatile u32 ReadIndex;

	volatile u32 InterruptMask;

	
	u8	Reserved[4084];

	
	

	
	u8 Buffer[0];
} __attribute__((packed)) RING_BUFFER;

typedef struct _RING_BUFFER_INFO {
	RING_BUFFER *RingBuffer;
	u32 RingSize;			
	spinlock_t ring_lock;

	u32 RingDataSize;		
	u32 RingDataStartOffset;

} RING_BUFFER_INFO;

typedef struct _RING_BUFFER_DEBUG_INFO {
	u32 CurrentInterruptMask;
	u32 CurrentReadIndex;
	u32 CurrentWriteIndex;
	u32 BytesAvailToRead;
	u32 BytesAvailToWrite;
} RING_BUFFER_DEBUG_INFO;






int RingBufferInit(RING_BUFFER_INFO *RingInfo, void *Buffer, u32 BufferLen);

void RingBufferCleanup(RING_BUFFER_INFO *RingInfo);

int RingBufferWrite(RING_BUFFER_INFO *RingInfo,
		    struct scatterlist *sglist,
		    u32 sgcount);

int RingBufferPeek(RING_BUFFER_INFO *RingInfo, void *Buffer, u32 BufferLen);

int RingBufferRead(RING_BUFFER_INFO *RingInfo,
		   void *Buffer,
		   u32 BufferLen,
		   u32 Offset);

u32 GetRingBufferInterruptMask(RING_BUFFER_INFO *RingInfo);

void DumpRingInfo(RING_BUFFER_INFO *RingInfo, char *Prefix);

void RingBufferGetDebugInfo(RING_BUFFER_INFO *RingInfo,
			    RING_BUFFER_DEBUG_INFO *DebugInfo);

#endif 
