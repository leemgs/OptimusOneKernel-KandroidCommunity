

#ifndef _VMXNET3_DEFS_H_
#define _VMXNET3_DEFS_H_

#include "upt1_defs.h"



enum {
	VMXNET3_REG_VRRS	= 0x0,	
	VMXNET3_REG_UVRS	= 0x8,	
	VMXNET3_REG_DSAL	= 0x10,	
	VMXNET3_REG_DSAH	= 0x18,	
	VMXNET3_REG_CMD		= 0x20,	
	VMXNET3_REG_MACL	= 0x28,	
	VMXNET3_REG_MACH	= 0x30,	
	VMXNET3_REG_ICR		= 0x38,	
	VMXNET3_REG_ECR		= 0x40	
};


enum {
	VMXNET3_REG_IMR		= 0x0,	 
	VMXNET3_REG_TXPROD	= 0x600, 
	VMXNET3_REG_RXPROD	= 0x800, 
	VMXNET3_REG_RXPROD2	= 0xA00	 
};

#define VMXNET3_PT_REG_SIZE     4096	
#define VMXNET3_VD_REG_SIZE     4096	

#define VMXNET3_REG_ALIGN       8	
#define VMXNET3_REG_ALIGN_MASK  0x7


#define VMXNET3_IO_TYPE_PT              0
#define VMXNET3_IO_TYPE_VD              1
#define VMXNET3_IO_ADDR(type, reg)      (((type) << 24) | ((reg) & 0xFFFFFF))
#define VMXNET3_IO_TYPE(addr)           ((addr) >> 24)
#define VMXNET3_IO_REG(addr)            ((addr) & 0xFFFFFF)

enum {
	VMXNET3_CMD_FIRST_SET = 0xCAFE0000,
	VMXNET3_CMD_ACTIVATE_DEV = VMXNET3_CMD_FIRST_SET,
	VMXNET3_CMD_QUIESCE_DEV,
	VMXNET3_CMD_RESET_DEV,
	VMXNET3_CMD_UPDATE_RX_MODE,
	VMXNET3_CMD_UPDATE_MAC_FILTERS,
	VMXNET3_CMD_UPDATE_VLAN_FILTERS,
	VMXNET3_CMD_UPDATE_RSSIDT,
	VMXNET3_CMD_UPDATE_IML,
	VMXNET3_CMD_UPDATE_PMCFG,
	VMXNET3_CMD_UPDATE_FEATURE,
	VMXNET3_CMD_LOAD_PLUGIN,

	VMXNET3_CMD_FIRST_GET = 0xF00D0000,
	VMXNET3_CMD_GET_QUEUE_STATUS = VMXNET3_CMD_FIRST_GET,
	VMXNET3_CMD_GET_STATS,
	VMXNET3_CMD_GET_LINK,
	VMXNET3_CMD_GET_PERM_MAC_LO,
	VMXNET3_CMD_GET_PERM_MAC_HI,
	VMXNET3_CMD_GET_DID_LO,
	VMXNET3_CMD_GET_DID_HI,
	VMXNET3_CMD_GET_DEV_EXTRA_INFO,
	VMXNET3_CMD_GET_CONF_INTR
};

struct Vmxnet3_TxDesc {
	u64		addr;

	u32		len:14;
	u32		gen:1;      
	u32		rsvd:1;
	u32		dtype:1;    
	u32		ext1:1;
	u32		msscof:14;  

	u32		hlen:10;    
	u32		om:2;       
	u32		eop:1;      
	u32		cq:1;       
	u32		ext2:1;
	u32		ti:1;       
	u32		tci:16;     
};


#define VMXNET3_OM_NONE		0
#define VMXNET3_OM_CSUM		2
#define VMXNET3_OM_TSO		3


#define VMXNET3_TXD_EOP_SHIFT	12
#define VMXNET3_TXD_CQ_SHIFT	13
#define VMXNET3_TXD_GEN_SHIFT	14

#define VMXNET3_TXD_CQ		(1 << VMXNET3_TXD_CQ_SHIFT)
#define VMXNET3_TXD_EOP		(1 << VMXNET3_TXD_EOP_SHIFT)
#define VMXNET3_TXD_GEN		(1 << VMXNET3_TXD_GEN_SHIFT)

#define VMXNET3_HDR_COPY_SIZE   128


struct Vmxnet3_TxDataDesc {
	u8		data[VMXNET3_HDR_COPY_SIZE];
};


struct Vmxnet3_TxCompDesc {
	u32		txdIdx:12;    
	u32		ext1:20;

	u32		ext2;
	u32		ext3;

	u32		rsvd:24;
	u32		type:7;       
	u32		gen:1;        
};


struct Vmxnet3_RxDesc {
	u64		addr;

	u32		len:14;
	u32		btype:1;      
	u32		dtype:1;      
	u32		rsvd:15;
	u32		gen:1;        

	u32		ext1;
};


#define VMXNET3_RXD_BTYPE_HEAD   0    
#define VMXNET3_RXD_BTYPE_BODY   1    


#define VMXNET3_RXD_BTYPE_SHIFT  14
#define VMXNET3_RXD_GEN_SHIFT    31


struct Vmxnet3_RxCompDesc {
	u32		rxdIdx:12;    
	u32		ext1:2;
	u32		eop:1;        
	u32		sop:1;        
	u32		rqID:10;      
	u32		rssType:4;    
	u32		cnc:1;        
	u32		ext2:1;

	u32		rssHash;      

	u32		len:14;       
	u32		err:1;        
	u32		ts:1;         
	u32		tci:16;       

	u32		csum:16;
	u32		tuc:1;        
	u32		udp:1;        
	u32		tcp:1;        
	u32		ipc:1;        
	u32		v6:1;         
	u32		v4:1;         
	u32		frg:1;        
	u32		fcs:1;        
	u32		type:7;       
	u32		gen:1;        
};


#define VMXNET3_RCD_TUC_SHIFT	16
#define VMXNET3_RCD_IPC_SHIFT	19


#define VMXNET3_RCD_TYPE_SHIFT	56
#define VMXNET3_RCD_GEN_SHIFT	63


#define VMXNET3_RCD_CSUM_OK (1 << VMXNET3_RCD_TUC_SHIFT | \
			     1 << VMXNET3_RCD_IPC_SHIFT)


enum {
	VMXNET3_RCD_RSS_TYPE_NONE     = 0,
	VMXNET3_RCD_RSS_TYPE_IPV4     = 1,
	VMXNET3_RCD_RSS_TYPE_TCPIPV4  = 2,
	VMXNET3_RCD_RSS_TYPE_IPV6     = 3,
	VMXNET3_RCD_RSS_TYPE_TCPIPV6  = 4,
};



union Vmxnet3_GenericDesc {
	u64				qword[2];
	u32				dword[4];
	u16				word[8];
	struct Vmxnet3_TxDesc		txd;
	struct Vmxnet3_RxDesc		rxd;
	struct Vmxnet3_TxCompDesc	tcd;
	struct Vmxnet3_RxCompDesc	rcd;
};

#define VMXNET3_INIT_GEN       1


#define VMXNET3_MAX_TX_BUF_SIZE  (1 << 14)


#define VMXNET3_TXD_NEEDED(size) (((size) + VMXNET3_MAX_TX_BUF_SIZE - 1) / \
				  VMXNET3_MAX_TX_BUF_SIZE)


#define VMXNET3_MAX_TXD_PER_PKT 16


#define VMXNET3_MAX_RX_BUF_SIZE  ((1 << 14) - 1)

#define VMXNET3_MIN_T0_BUF_SIZE  128
#define VMXNET3_MAX_CSUM_OFFSET  1024


#define VMXNET3_RING_BA_ALIGN   512
#define VMXNET3_RING_BA_MASK    (VMXNET3_RING_BA_ALIGN - 1)


#define VMXNET3_RING_SIZE_ALIGN 32
#define VMXNET3_RING_SIZE_MASK  (VMXNET3_RING_SIZE_ALIGN - 1)


#define VMXNET3_TX_RING_MAX_SIZE   4096
#define VMXNET3_TC_RING_MAX_SIZE   4096
#define VMXNET3_RX_RING_MAX_SIZE   4096
#define VMXNET3_RC_RING_MAX_SIZE   8192



enum {
 VMXNET3_ERR_NOEOP        = 0x80000000,  
 VMXNET3_ERR_TXD_REUSE    = 0x80000001,  
 VMXNET3_ERR_BIG_PKT      = 0x80000002,  
 VMXNET3_ERR_DESC_NOT_SPT = 0x80000003,  
 VMXNET3_ERR_SMALL_BUF    = 0x80000004,  
 VMXNET3_ERR_STRESS       = 0x80000005,  
 VMXNET3_ERR_SWITCH       = 0x80000006,  
 VMXNET3_ERR_TXD_INVALID  = 0x80000007,  
};


#define VMXNET3_CDTYPE_TXCOMP      0    
#define VMXNET3_CDTYPE_RXCOMP      3    

enum {
	VMXNET3_GOS_BITS_UNK    = 0,   
	VMXNET3_GOS_BITS_32     = 1,
	VMXNET3_GOS_BITS_64     = 2,
};

#define VMXNET3_GOS_TYPE_LINUX	1


struct Vmxnet3_GOSInfo {
	u32				gosBits:2;	
	u32				gosType:4;   
	u32				gosVer:16;   
	u32				gosMisc:10;  
};


struct Vmxnet3_DriverInfo {
	u32				version;
	struct Vmxnet3_GOSInfo		gos;
	u32				vmxnet3RevSpt;
	u32				uptVerSpt;
};


#define VMXNET3_REV1_MAGIC  0xbabefee1


#define VMXNET3_QUEUE_DESC_ALIGN  128


struct Vmxnet3_MiscConf {
	struct Vmxnet3_DriverInfo driverInfo;
	u64		uptFeatures;
	u64		ddPA;         
	u64		queueDescPA;  
	u32		ddLen;        
	u32		queueDescLen; 
	u32		mtu;
	u16		maxNumRxSG;
	u8		numTxQueues;
	u8		numRxQueues;
	u32		reserved[4];
};


struct Vmxnet3_TxQueueConf {
	u64		txRingBasePA;
	u64		dataRingBasePA;
	u64		compRingBasePA;
	u64		ddPA;         
	u64		reserved;
	u32		txRingSize;   
	u32		dataRingSize; 
	u32		compRingSize; 
	u32		ddLen;        
	u8		intrIdx;
	u8		_pad[7];
};


struct Vmxnet3_RxQueueConf {
	u64		rxRingBasePA[2];
	u64		compRingBasePA;
	u64		ddPA;            
	u64		reserved;
	u32		rxRingSize[2];   
	u32		compRingSize;    
	u32		ddLen;           
	u8		intrIdx;
	u8		_pad[7];
};


enum vmxnet3_intr_mask_mode {
	VMXNET3_IMM_AUTO   = 0,
	VMXNET3_IMM_ACTIVE = 1,
	VMXNET3_IMM_LAZY   = 2
};

enum vmxnet3_intr_type {
	VMXNET3_IT_AUTO = 0,
	VMXNET3_IT_INTX = 1,
	VMXNET3_IT_MSI  = 2,
	VMXNET3_IT_MSIX = 3
};

#define VMXNET3_MAX_TX_QUEUES  8
#define VMXNET3_MAX_RX_QUEUES  16

#define VMXNET3_MAX_INTRS      25


struct Vmxnet3_IntrConf {
	bool		autoMask;
	u8		numIntrs;      
	u8		eventIntrIdx;
	u8		modLevels[VMXNET3_MAX_INTRS];	
	u32		reserved[3];
};


#define VMXNET3_VFT_SIZE  (4096 / (sizeof(u32) * 8))


struct Vmxnet3_QueueStatus {
	bool		stopped;
	u8		_pad[3];
	u32		error;
};


struct Vmxnet3_TxQueueCtrl {
	u32		txNumDeferred;
	u32		txThreshold;
	u64		reserved;
};


struct Vmxnet3_RxQueueCtrl {
	bool		updateRxProd;
	u8		_pad[7];
	u64		reserved;
};

enum {
	VMXNET3_RXM_UCAST     = 0x01,  
	VMXNET3_RXM_MCAST     = 0x02,  
	VMXNET3_RXM_BCAST     = 0x04,  
	VMXNET3_RXM_ALL_MULTI = 0x08,  
	VMXNET3_RXM_PROMISC   = 0x10  
};

struct Vmxnet3_RxFilterConf {
	u32		rxMode;       
	u16		mfTableLen;   
	u16		_pad1;
	u64		mfTablePA;    
	u32		vfTable[VMXNET3_VFT_SIZE]; 
};


#define VMXNET3_PM_MAX_FILTERS        6
#define VMXNET3_PM_MAX_PATTERN_SIZE   128
#define VMXNET3_PM_MAX_MASK_SIZE      (VMXNET3_PM_MAX_PATTERN_SIZE / 8)

#define VMXNET3_PM_WAKEUP_MAGIC       0x01  
#define VMXNET3_PM_WAKEUP_FILTER      0x02  


struct Vmxnet3_PM_PktFilter {
	u8		maskSize;
	u8		patternSize;
	u8		mask[VMXNET3_PM_MAX_MASK_SIZE];
	u8		pattern[VMXNET3_PM_MAX_PATTERN_SIZE];
	u8		pad[6];
};


struct Vmxnet3_PMConf {
	u16		wakeUpEvents;  
	u8		numFilters;
	u8		pad[5];
	struct Vmxnet3_PM_PktFilter filters[VMXNET3_PM_MAX_FILTERS];
};


struct Vmxnet3_VariableLenConfDesc {
	u32		confVer;
	u32		confLen;
	u64		confPA;
};


struct Vmxnet3_TxQueueDesc {
	struct Vmxnet3_TxQueueCtrl		ctrl;
	struct Vmxnet3_TxQueueConf		conf;

	
	struct Vmxnet3_QueueStatus		status;
	struct UPT1_TxStats			stats;
	u8					_pad[88]; 
};


struct Vmxnet3_RxQueueDesc {
	struct Vmxnet3_RxQueueCtrl		ctrl;
	struct Vmxnet3_RxQueueConf		conf;
	
	struct Vmxnet3_QueueStatus		status;
	struct UPT1_RxStats			stats;
	u8				      __pad[88]; 
};


struct Vmxnet3_DSDevRead {
	
	struct Vmxnet3_MiscConf			misc;
	struct Vmxnet3_IntrConf			intrConf;
	struct Vmxnet3_RxFilterConf		rxFilterConf;
	struct Vmxnet3_VariableLenConfDesc	rssConfDesc;
	struct Vmxnet3_VariableLenConfDesc	pmConfDesc;
	struct Vmxnet3_VariableLenConfDesc	pluginConfDesc;
};


struct Vmxnet3_DriverShared {
	u32				magic;
	
	u32					pad;
	struct Vmxnet3_DSDevRead		devRead;
	u32					ecr;
	u32					reserved[5];
};


#define VMXNET3_ECR_RQERR       (1 << 0)
#define VMXNET3_ECR_TQERR       (1 << 1)
#define VMXNET3_ECR_LINK        (1 << 2)
#define VMXNET3_ECR_DIC         (1 << 3)
#define VMXNET3_ECR_DEBUG       (1 << 4)


#define VMXNET3_FLIP_RING_GEN(gen) ((gen) = (gen) ^ 0x1)


#define VMXNET3_INC_RING_IDX_ONLY(idx, ring_size) \
	do {\
		(idx)++;\
		if (unlikely((idx) == (ring_size))) {\
			(idx) = 0;\
		} \
	} while (0)

#define VMXNET3_SET_VFTABLE_ENTRY(vfTable, vid) \
	(vfTable[vid >> 5] |= (1 << (vid & 31)))
#define VMXNET3_CLEAR_VFTABLE_ENTRY(vfTable, vid) \
	(vfTable[vid >> 5] &= ~(1 << (vid & 31)))

#define VMXNET3_VFTABLE_ENTRY_IS_SET(vfTable, vid) \
	((vfTable[vid >> 5] & (1 << (vid & 31))) != 0)

#define VMXNET3_MAX_MTU     9000
#define VMXNET3_MIN_MTU     60

#define VMXNET3_LINK_UP         (10000 << 16 | 1)    
#define VMXNET3_LINK_DOWN       0

#endif 
