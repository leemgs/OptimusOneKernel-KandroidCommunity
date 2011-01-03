

#ifndef __ET1310_TX_H__
#define __ET1310_TX_H__





typedef union _txdesc_word2_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 vlan_prio:3;		
		u32 vlan_cfi:1;		
		u32 vlan_tag:12;		
		u32 length_in_bytes:16;	
#else
		u32 length_in_bytes:16;	
		u32 vlan_tag:12;		
		u32 vlan_cfi:1;		
		u32 vlan_prio:3;		
#endif	
	} bits;
} TXDESC_WORD2_t, *PTXDESC_WORD2_t;


typedef union _txdesc_word3_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:17;	
		u32 udpa:1;	
		u32 tcpa:1;	
		u32 ipa:1;		
		u32 vlan:1;	
		u32 hp:1;		
		u32 pp:1;		
		u32 mac:1;		
		u32 crc:1;		
		u32 e:1;		
		u32 pf:1;		
		u32 bp:1;		
		u32 cw:1;		
		u32 ir:1;		
		u32 f:1;		
		u32 l:1;		
#else
		u32 l:1;		
		u32 f:1;		
		u32 ir:1;		
		u32 cw:1;		
		u32 bp:1;		
		u32 pf:1;		
		u32 e:1;		
		u32 crc:1;		
		u32 mac:1;		
		u32 pp:1;		
		u32 hp:1;		
		u32 vlan:1;	
		u32 ipa:1;		
		u32 tcpa:1;	
		u32 udpa:1;	
		u32 unused:17;	
#endif	
	} bits;
} TXDESC_WORD3_t, *PTXDESC_WORD3_t;


typedef struct _tx_desc_entry_t {
	u32 DataBufferPtrHigh;
	u32 DataBufferPtrLow;
	TXDESC_WORD2_t word2;	
	TXDESC_WORD3_t word3;	
} TX_DESC_ENTRY_t, *PTX_DESC_ENTRY_t;





typedef union _tx_status_block_t {
	u32 value;
	struct {
#ifdef _BIT_FIELDS_HTOL
		u32 unused:21;		
		u32 serv_cpl_wrap:1;	
		u32 serv_cpl:10;		
#else
		u32 serv_cpl:10;		
		u32 serv_cpl_wrap:1;	
		u32 unused:21;		
#endif
	} bits;
} TX_STATUS_BLOCK_t, *PTX_STATUS_BLOCK_t;


typedef struct _MP_TCB {
	struct _MP_TCB *Next;
	u32 Flags;
	u32 Count;
	u32 PacketStaleCount;
	struct sk_buff *Packet;
	u32 PacketLength;
	u32 WrIndex;
	u32 WrIndexStart;
} MP_TCB, *PMP_TCB;


typedef struct tx_skb_list_elem {
	struct list_head skb_list_elem;
	struct sk_buff *skb;
} TX_SKB_LIST_ELEM, *PTX_SKB_LIST_ELEM;


typedef struct _tx_ring_t {
	
	PMP_TCB MpTcbMem;

	
	PMP_TCB TCBReadyQueueHead;
	PMP_TCB TCBReadyQueueTail;

	
	PMP_TCB CurrSendHead;
	PMP_TCB CurrSendTail;
	int32_t nBusySend;

	
	struct list_head SendWaitQueue;
	int32_t nWaitSend;

	
	PTX_DESC_ENTRY_t pTxDescRingVa;
	dma_addr_t pTxDescRingPa;
	uint64_t pTxDescRingAdjustedPa;
	uint64_t TxDescOffset;

	
	u32 txDmaReadyToSend;

	
	PTX_STATUS_BLOCK_t pTxStatusVa;
	dma_addr_t pTxStatusPa;

	
	void *pTxDummyBlkVa;
	dma_addr_t pTxDummyBlkPa;

	TXMAC_ERR_t TxMacErr;

	
	int32_t TxPacketsSinceLastinterrupt;
} TX_RING_t, *PTX_RING_t;


typedef struct _MP_FRAG_LIST MP_FRAG_LIST, *PMP_FRAG_LIST;


struct et131x_adapter;


int et131x_tx_dma_memory_alloc(struct et131x_adapter *adapter);
void et131x_tx_dma_memory_free(struct et131x_adapter *adapter);
void ConfigTxDmaRegs(struct et131x_adapter *pAdapter);
void et131x_init_send(struct et131x_adapter *adapter);
void et131x_tx_dma_disable(struct et131x_adapter *pAdapter);
void et131x_tx_dma_enable(struct et131x_adapter *pAdapter);
void et131x_handle_send_interrupt(struct et131x_adapter *pAdapter);
void et131x_free_busy_send_packets(struct et131x_adapter *pAdapter);
int et131x_send_packets(struct sk_buff *skb, struct net_device *netdev);

#endif 
