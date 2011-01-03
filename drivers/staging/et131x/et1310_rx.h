

#ifndef __ET1310_RX_H__
#define __ET1310_RX_H__

#include "et1310_address_map.h"

#define USE_FBR0 true

#ifdef USE_FBR0

#endif



#define FBR_CHUNKS 32

#define MAX_DESC_PER_RING_RX         1024


#ifdef USE_FBR0
#define RFD_LOW_WATER_MARK	40
#define NIC_MIN_NUM_RFD		64
#define NIC_DEFAULT_NUM_RFD	1024
#else
#define RFD_LOW_WATER_MARK	20
#define NIC_MIN_NUM_RFD		64
#define NIC_DEFAULT_NUM_RFD	256
#endif

#define NUM_PACKETS_HANDLED	256

#define ALCATEL_BAD_STATUS	0xe47f0000
#define ALCATEL_MULTICAST_PKT	0x01000000
#define ALCATEL_BROADCAST_PKT	0x02000000


typedef union _FBR_WORD2_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 reserved:22;	
		u32 bi:10;		
#else
		u32 bi:10;		
		u32 reserved:22;	
#endif
	} bits;
} FBR_WORD2_t, *PFBR_WORD2_t;

typedef struct _FBR_DESC_t {
	u32 addr_lo;
	u32 addr_hi;
	FBR_WORD2_t word2;
} FBR_DESC_t, *PFBR_DESC_t;


typedef union _PKT_STAT_DESC_WORD0_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		
		
#if 0
		u32 asw_trunc:1;		
#endif
		u32 asw_long_evt:1;	
		u32 asw_VLAN_tag:1;	
		u32 asw_unsupported_op:1;	
		u32 asw_pause_frame:1;	
		u32 asw_control_frame:1;	
		u32 asw_dribble_nibble:1;	
		u32 asw_broadcast:1;	
		u32 asw_multicast:1;	
		u32 asw_OK:1;		
		u32 asw_too_long:1;	
		u32 asw_len_chk_err:1;	
		u32 asw_CRC_err:1;		
		u32 asw_code_err:1;	
		u32 asw_false_carrier_event:1;	
		u32 asw_RX_DV_event:1;	
		u32 asw_prev_pkt_dropped:1;
		u32 unused:5;		
		u32 vp:1;			
		u32 jp:1;			
		u32 ft:1;			
		u32 drop:1;		
		u32 rxmac_error:1;		
		u32 wol:1;			
		u32 tcpp:1;		
		u32 tcpa:1;		
		u32 ipp:1;			
		u32 ipa:1;			
		u32 hp:1;			
#else
		u32 hp:1;			
		u32 ipa:1;			
		u32 ipp:1;			
		u32 tcpa:1;		
		u32 tcpp:1;		
		u32 wol:1;			
		u32 rxmac_error:1;		
		u32 drop:1;		
		u32 ft:1;			
		u32 jp:1;			
		u32 vp:1;			
		u32 unused:5;		
		u32 asw_prev_pkt_dropped:1;
		u32 asw_RX_DV_event:1;	
		u32 asw_false_carrier_event:1;	
		u32 asw_code_err:1;	
		u32 asw_CRC_err:1;		
		u32 asw_len_chk_err:1;	
		u32 asw_too_long:1;	
		u32 asw_OK:1;		
		u32 asw_multicast:1;	
		u32 asw_broadcast:1;	
		u32 asw_dribble_nibble:1;	
		u32 asw_control_frame:1;	
		u32 asw_pause_frame:1;	
		u32 asw_unsupported_op:1;	
		u32 asw_VLAN_tag:1;	
		u32 asw_long_evt:1;	
#if 0
		u32 asw_trunc:1;		
#endif
#endif
	} bits;
} PKT_STAT_DESC_WORD0_t, *PPKT_STAT_WORD0_t;

typedef union _PKT_STAT_DESC_WORD1_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:4;	
		u32 ri:2;		
		u32 bi:10;		
		u32 length:16;	
#else
		u32 length:16;	
		u32 bi:10;		
		u32 ri:2;		
		u32 unused:4;	
#endif
	} bits;
} PKT_STAT_DESC_WORD1_t, *PPKT_STAT_WORD1_t;

typedef struct _PKT_STAT_DESC_t {
	PKT_STAT_DESC_WORD0_t word0;
	PKT_STAT_DESC_WORD1_t word1;
} PKT_STAT_DESC_t, *PPKT_STAT_DESC_t;




typedef union _rxstat_word0_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 FBR1unused:5;	
		u32 FBR1wrap:1;	
		u32 FBR1offset:10;	
		u32 FBR0unused:5;	
		u32 FBR0wrap:1;	
		u32 FBR0offset:10;	
#else
		u32 FBR0offset:10;	
		u32 FBR0wrap:1;	
		u32 FBR0unused:5;	
		u32 FBR1offset:10;	
		u32 FBR1wrap:1;	
		u32 FBR1unused:5;	
#endif
	} bits;
} RXSTAT_WORD0_t, *PRXSTAT_WORD0_t;


typedef union _rxstat_word1_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 PSRunused:3;	
		u32 PSRwrap:1;	
		u32 PSRoffset:12;	
		u32 reserved:16;	
#else
		u32 reserved:16;	
		u32 PSRoffset:12;	
		u32 PSRwrap:1;	
		u32 PSRunused:3;	
#endif
	} bits;
} RXSTAT_WORD1_t, *PRXSTAT_WORD1_t;


typedef struct _rx_status_block_t {
	RXSTAT_WORD0_t Word0;
	RXSTAT_WORD1_t Word1;
} RX_STATUS_BLOCK_t, *PRX_STATUS_BLOCK_t;


typedef struct _FbrLookupTable {
	void *Va[MAX_DESC_PER_RING_RX];
	void *Buffer1[MAX_DESC_PER_RING_RX];
	void *Buffer2[MAX_DESC_PER_RING_RX];
	u32 PAHigh[MAX_DESC_PER_RING_RX];
	u32 PALow[MAX_DESC_PER_RING_RX];
} FBRLOOKUPTABLE, *PFBRLOOKUPTABLE;

typedef enum {
	ONE_PACKET_INTERRUPT,
	FOUR_PACKET_INTERRUPT
} eRX_INTERRUPT_STATE_t, *PeRX_INTERRUPT_STATE_t;


typedef struct rx_skb_list_elem {
	struct list_head skb_list_elem;
	dma_addr_t dma_addr;
	struct sk_buff *skb;
} RX_SKB_LIST_ELEM, *PRX_SKB_LIST_ELEM;


typedef struct _rx_ring_t {
#ifdef USE_FBR0
	void *pFbr0RingVa;
	dma_addr_t pFbr0RingPa;
	void *Fbr0MemVa[MAX_DESC_PER_RING_RX / FBR_CHUNKS];
	dma_addr_t Fbr0MemPa[MAX_DESC_PER_RING_RX / FBR_CHUNKS];
	uint64_t Fbr0Realpa;
	uint64_t Fbr0offset;
	u32 local_Fbr0_full;
	u32 Fbr0NumEntries;
	u32 Fbr0BufferSize;
#endif
	void *pFbr1RingVa;
	dma_addr_t pFbr1RingPa;
	void *Fbr1MemVa[MAX_DESC_PER_RING_RX / FBR_CHUNKS];
	dma_addr_t Fbr1MemPa[MAX_DESC_PER_RING_RX / FBR_CHUNKS];
	uint64_t Fbr1Realpa;
	uint64_t Fbr1offset;
	FBRLOOKUPTABLE *Fbr[2];
	u32 local_Fbr1_full;
	u32 Fbr1NumEntries;
	u32 Fbr1BufferSize;

	void *pPSRingVa;
	dma_addr_t pPSRingPa;
	uint64_t pPSRingRealPa;
	uint64_t pPSRingOffset;
	RXDMA_PSR_FULL_OFFSET_t local_psr_full;
	u32 PsrNumEntries;

	void *pRxStatusVa;
	dma_addr_t pRxStatusPa;
	uint64_t RxStatusRealPA;
	uint64_t RxStatusOffset;

	struct list_head RecvBufferPool;

	
	struct list_head RecvList;
	struct list_head RecvPendingList;
	u32 nReadyRecv;

	u32 NumRfd;

	bool UnfinishedReceives;

	struct list_head RecvPacketPool;

	
	struct kmem_cache *RecvLookaside;
} RX_RING_t, *PRX_RING_t;


struct _MP_RFD;


struct et131x_adapter;


int et131x_rx_dma_memory_alloc(struct et131x_adapter *adapter);
void et131x_rx_dma_memory_free(struct et131x_adapter *adapter);
int et131x_rfd_resources_alloc(struct et131x_adapter *adapter,
			       struct _MP_RFD *pMpRfd);
void et131x_rfd_resources_free(struct et131x_adapter *adapter,
			       struct _MP_RFD *pMpRfd);
int et131x_init_recv(struct et131x_adapter *adapter);

void ConfigRxDmaRegs(struct et131x_adapter *adapter);
void SetRxDmaTimer(struct et131x_adapter *adapter);
void et131x_rx_dma_disable(struct et131x_adapter *adapter);
void et131x_rx_dma_enable(struct et131x_adapter *adapter);

void et131x_reset_recv(struct et131x_adapter *adapter);

void et131x_handle_recv_interrupt(struct et131x_adapter *adapter);

#endif 
