

#ifndef __ET131X_ADAPTER_H__
#define __ET131X_ADAPTER_H__

#include "et1310_address_map.h"
#include "et1310_tx.h"
#include "et1310_rx.h"


#define NUM_DESC_PER_RING_TX         512	
#define NUM_TCB                      64


#define NUM_TRAFFIC_CLASSES          1


#define TX_PACKETS_IN_SAMPLE        10000
#define TX_MAX_ERRORS_IN_SAMPLE     50

#define TX_ERROR_PERIOD             1000
#define TX_MAX_ERRORS_IN_PERIOD     10

#define LINK_DETECTION_TIMER        5000

#define TX_CONSEC_RANGE             5
#define TX_CONSEC_ERRORED_THRESH    10

#define LO_MARK_PERCENT_FOR_PSR     15
#define LO_MARK_PERCENT_FOR_RX      15


#define MP_TCB_RESOURCES_AVAILABLE(_M) ((_M)->TxRing.nBusySend < NUM_TCB)
#define MP_TCB_RESOURCES_NOT_AVAILABLE(_M) ((_M)->TxRing.nBusySend >= NUM_TCB)

#define MP_SHOULD_FAIL_SEND(_M)   ((_M)->Flags & fMP_ADAPTER_FAIL_SEND_MASK)


typedef struct _MP_ERR_COUNTERS {
	u32 PktCountTxPackets;
	u32 PktCountTxErrors;
	u32 TimerBasedTxErrors;
	u32 PktCountLastError;
	u32 ErredConsecPackets;
} MP_ERR_COUNTERS, *PMP_ERR_COUNTERS;


typedef struct _MP_RFD {
	struct list_head list_node;
	struct sk_buff *Packet;
	u32 PacketSize;	
	u16 bufferindex;
	u8 ringindex;
} MP_RFD, *PMP_RFD;


typedef enum _eflow_control_t {
	Both = 0,
	TxOnly = 1,
	RxOnly = 2,
	None = 3
} eFLOW_CONTROL_t, *PeFLOW_CONTROL_t;


typedef struct _ce_stats_t {
	
	uint64_t ipackets;	
	uint64_t opackets;	

	
	u32 unircv;	
	atomic_t unixmt;	
	u32 multircv;	
	atomic_t multixmt;	
	u32 brdcstrcv;	
	atomic_t brdcstxmt;	
	u32 norcvbuf;	
	u32 noxmtbuf;	

	
	u8 xcvr_addr;
	u32 xcvr_id;

	
	u32 tx_uflo;		

	u32 collisions;
	u32 excessive_collisions;
	u32 first_collision;
	u32 late_collisions;
	u32 max_pkt_error;
	u32 tx_deferred;

	
	u32 rx_ov_flow;	

	u32 length_err;
	u32 alignment_err;
	u32 crc_err;
	u32 code_violations;
	u32 other_errors;

	u32 SynchrounousIterations;
	u32 InterruptStatus;
} CE_STATS_t, *PCE_STATS_t;


struct et131x_adapter {
	struct net_device *netdev;
	struct pci_dev *pdev;

	struct work_struct task;

	
	u32 Flags;
	u32 HwErrCount;

	
	u8 PermanentAddress[ETH_ALEN];
	u8 CurrentAddress[ETH_ALEN];
	bool has_eeprom;
	u8 eepromData[2];

	
	spinlock_t Lock;

	spinlock_t TCBSendQLock;
	spinlock_t TCBReadyQLock;
	spinlock_t SendHWLock;
	spinlock_t SendWaitLock;

	spinlock_t RcvLock;
	spinlock_t RcvPendLock;
	spinlock_t FbrLock;

	spinlock_t PHYLock;

	
	u32 PacketFilter;
	u32 linkspeed;
	u32 duplex_mode;

	
	u32 MCAddressCount;
	u8 MCList[NIC_MAX_MCAST_LIST][ETH_ALEN];

	
	TXMAC_TXTEST_t TxMacTest;

	
	ADDRESS_MAP_t __iomem *regs;

	
	u8 SpeedDuplex;		
	eFLOW_CONTROL_t RegistryFlowControl;	
	u8 RegistryPhyComa;	

	u32 RegistryRxMemEnd;	
	u32 RegistryJumboPacket;	

	
	u8 RegistryNMIDisable;
	u8 RegistryPhyLoopbk;	

	
	u8 AiForceDpx;		
	u16 AiForceSpeed;		
	eFLOW_CONTROL_t FlowControl;	
	enum {
		NETIF_STATUS_INVALID = 0,
		NETIF_STATUS_MEDIA_CONNECT,
		NETIF_STATUS_MEDIA_DISCONNECT,
		NETIF_STATUS_MAX
	} MediaState;
	u8 DriverNoPhyAccess;

	
	struct timer_list ErrorTimer;
	MP_POWER_MGMT PoMgmt;
	u32 CachedMaskValue;

	
	MI_BMSR_t Bmsr;

	
	TX_RING_t TxRing;

	
	RX_RING_t RxRing;

	
	u8 ReplicaPhyLoopbk;	
	u8 ReplicaPhyLoopbkPF;	

	
	CE_STATS_t Stats;

	struct net_device_stats net_stats;
	struct net_device_stats net_stats_prev;
};

#endif 
