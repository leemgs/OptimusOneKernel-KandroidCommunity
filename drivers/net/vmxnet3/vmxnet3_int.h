

#ifndef _VMXNET3_INT_H
#define _VMXNET3_INT_H

#include <linux/types.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/netdevice.h>
#include <linux/pci.h>
#include <linux/ethtool.h>
#include <linux/compiler.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/ioport.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/skbuff.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <asm/dma.h>
#include <asm/page.h>

#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/in.h>
#include <linux/etherdevice.h>
#include <asm/checksum.h>
#include <linux/if_vlan.h>
#include <linux/if_arp.h>
#include <linux/inetdevice.h>

#include "vmxnet3_defs.h"

#ifdef DEBUG
# define VMXNET3_DRIVER_VERSION_REPORT VMXNET3_DRIVER_VERSION_STRING"-NAPI(debug)"
#else
# define VMXNET3_DRIVER_VERSION_REPORT VMXNET3_DRIVER_VERSION_STRING"-NAPI"
#endif



#define VMXNET3_DRIVER_VERSION_STRING   "1.0.5.0-k"


#define VMXNET3_DRIVER_VERSION_NUM      0x01000500




enum {
	VMNET_CAP_SG	        = 0x0001, 
	VMNET_CAP_IP4_CSUM      = 0x0002, 
	VMNET_CAP_HW_CSUM       = 0x0004, 
	VMNET_CAP_HIGH_DMA      = 0x0008, 
	VMNET_CAP_TOE	        = 0x0010, 
	VMNET_CAP_TSO	        = 0x0020, 
	VMNET_CAP_SW_TSO        = 0x0040, 
	VMNET_CAP_VMXNET_APROM  = 0x0080, 
	VMNET_CAP_HW_TX_VLAN    = 0x0100, 
	VMNET_CAP_HW_RX_VLAN    = 0x0200, 
	VMNET_CAP_SW_VLAN       = 0x0400, 
	VMNET_CAP_WAKE_PCKT_RCV = 0x0800, 
	VMNET_CAP_ENABLE_INT_INLINE = 0x1000,  
	VMNET_CAP_ENABLE_HEADER_COPY = 0x2000,  
	VMNET_CAP_TX_CHAIN      = 0x4000, 
	VMNET_CAP_RX_CHAIN      = 0x8000, 
	VMNET_CAP_LPD           = 0x10000, 
	VMNET_CAP_BPF           = 0x20000, 
	VMNET_CAP_SG_SPAN_PAGES = 0x40000, 
					   
	VMNET_CAP_IP6_CSUM      = 0x80000, 
	VMNET_CAP_TSO6         = 0x100000, 
	VMNET_CAP_TSO256k      = 0x200000, 
					   
	VMNET_CAP_UPT          = 0x400000  
};


#define PCI_VENDOR_ID_VMWARE            0x15AD
#define PCI_DEVICE_ID_VMWARE_VMXNET3    0x07B0
#define MAX_ETHERNET_CARDS		10
#define MAX_PCI_PASSTHRU_DEVICE		6

struct vmxnet3_cmd_ring {
	union Vmxnet3_GenericDesc *base;
	u32		size;
	u32		next2fill;
	u32		next2comp;
	u8		gen;
	dma_addr_t	basePA;
};

static inline void
vmxnet3_cmd_ring_adv_next2fill(struct vmxnet3_cmd_ring *ring)
{
	ring->next2fill++;
	if (unlikely(ring->next2fill == ring->size)) {
		ring->next2fill = 0;
		VMXNET3_FLIP_RING_GEN(ring->gen);
	}
}

static inline void
vmxnet3_cmd_ring_adv_next2comp(struct vmxnet3_cmd_ring *ring)
{
	VMXNET3_INC_RING_IDX_ONLY(ring->next2comp, ring->size);
}

static inline int
vmxnet3_cmd_ring_desc_avail(struct vmxnet3_cmd_ring *ring)
{
	return (ring->next2comp > ring->next2fill ? 0 : ring->size) +
		ring->next2comp - ring->next2fill - 1;
}

struct vmxnet3_comp_ring {
	union Vmxnet3_GenericDesc *base;
	u32               size;
	u32               next2proc;
	u8                gen;
	u8                intr_idx;
	dma_addr_t           basePA;
};

static inline void
vmxnet3_comp_ring_adv_next2proc(struct vmxnet3_comp_ring *ring)
{
	ring->next2proc++;
	if (unlikely(ring->next2proc == ring->size)) {
		ring->next2proc = 0;
		VMXNET3_FLIP_RING_GEN(ring->gen);
	}
}

struct vmxnet3_tx_data_ring {
	struct Vmxnet3_TxDataDesc *base;
	u32              size;
	dma_addr_t          basePA;
};

enum vmxnet3_buf_map_type {
	VMXNET3_MAP_INVALID = 0,
	VMXNET3_MAP_NONE,
	VMXNET3_MAP_SINGLE,
	VMXNET3_MAP_PAGE,
};

struct vmxnet3_tx_buf_info {
	u32      map_type;
	u16      len;
	u16      sop_idx;
	dma_addr_t  dma_addr;
	struct sk_buff *skb;
};

struct vmxnet3_tq_driver_stats {
	u64 drop_total;     
	u64 drop_too_many_frags;
	u64 drop_oversized_hdr;
	u64 drop_hdr_inspect_err;
	u64 drop_tso;

	u64 tx_ring_full;
	u64 linearized;         
	u64 copy_skb_header;    
	u64 oversized_hdr;
};

struct vmxnet3_tx_ctx {
	bool   ipv4;
	u16 mss;
	u32 eth_ip_hdr_size; 
	u32 l4_hdr_size;     
	u32 copy_size;       
	union Vmxnet3_GenericDesc *sop_txd;
	union Vmxnet3_GenericDesc *eop_txd;
};

struct vmxnet3_tx_queue {
	spinlock_t                      tx_lock;
	struct vmxnet3_cmd_ring         tx_ring;
	struct vmxnet3_tx_buf_info     *buf_info;
	struct vmxnet3_tx_data_ring     data_ring;
	struct vmxnet3_comp_ring        comp_ring;
	struct Vmxnet3_TxQueueCtrl            *shared;
	struct vmxnet3_tq_driver_stats  stats;
	bool                            stopped;
	int                             num_stop;  
} __attribute__((__aligned__(SMP_CACHE_BYTES)));

enum vmxnet3_rx_buf_type {
	VMXNET3_RX_BUF_NONE = 0,
	VMXNET3_RX_BUF_SKB = 1,
	VMXNET3_RX_BUF_PAGE = 2
};

struct vmxnet3_rx_buf_info {
	enum vmxnet3_rx_buf_type buf_type;
	u16     len;
	union {
		struct sk_buff *skb;
		struct page    *page;
	};
	dma_addr_t dma_addr;
};

struct vmxnet3_rx_ctx {
	struct sk_buff *skb;
	u32 sop_idx;
};

struct vmxnet3_rq_driver_stats {
	u64 drop_total;
	u64 drop_err;
	u64 drop_fcs;
	u64 rx_buf_alloc_failure;
};

struct vmxnet3_rx_queue {
	struct vmxnet3_cmd_ring   rx_ring[2];
	struct vmxnet3_comp_ring  comp_ring;
	struct vmxnet3_rx_ctx     rx_ctx;
	u32 qid;            
	u32 qid2;           
	u32 uncommitted[2]; 
	struct vmxnet3_rx_buf_info     *buf_info[2];
	struct Vmxnet3_RxQueueCtrl            *shared;
	struct vmxnet3_rq_driver_stats  stats;
} __attribute__((__aligned__(SMP_CACHE_BYTES)));

#define VMXNET3_LINUX_MAX_MSIX_VECT     1

struct vmxnet3_intr {
	enum vmxnet3_intr_mask_mode  mask_mode;
	enum vmxnet3_intr_type       type;	
	u8  num_intrs;			
	u8  event_intr_idx;		
	u8  mod_levels[VMXNET3_LINUX_MAX_MSIX_VECT]; 
#ifdef CONFIG_PCI_MSI
	struct msix_entry msix_entries[VMXNET3_LINUX_MAX_MSIX_VECT];
#endif
};

#define VMXNET3_STATE_BIT_RESETTING   0
#define VMXNET3_STATE_BIT_QUIESCED    1
struct vmxnet3_adapter {
	struct vmxnet3_tx_queue         tx_queue;
	struct vmxnet3_rx_queue         rx_queue;
	struct napi_struct              napi;
	struct vlan_group              *vlan_grp;

	struct vmxnet3_intr             intr;

	struct Vmxnet3_DriverShared    *shared;
	struct Vmxnet3_PMConf          *pm_conf;
	struct Vmxnet3_TxQueueDesc     *tqd_start;     
	struct Vmxnet3_RxQueueDesc     *rqd_start;     
	struct net_device              *netdev;
	struct pci_dev                 *pdev;

	u8				*hw_addr0; 
	u8				*hw_addr1; 

	
	bool				rxcsum;
	bool				lro;
	bool				jumbo_frame;

	
	unsigned			skb_buf_size;
	int		rx_buf_per_pkt;  
	dma_addr_t			shared_pa;
	dma_addr_t queue_desc_pa;

	
	u32     wol;

	
	u32     link_speed; 

	u64     tx_timeout_count;
	struct work_struct work;

	unsigned long  state;    

	int dev_number;
};

#define VMXNET3_WRITE_BAR0_REG(adapter, reg, val)  \
	writel((val), (adapter)->hw_addr0 + (reg))
#define VMXNET3_READ_BAR0_REG(adapter, reg)        \
	readl((adapter)->hw_addr0 + (reg))

#define VMXNET3_WRITE_BAR1_REG(adapter, reg, val)  \
	writel((val), (adapter)->hw_addr1 + (reg))
#define VMXNET3_READ_BAR1_REG(adapter, reg)        \
	readl((adapter)->hw_addr1 + (reg))

#define VMXNET3_WAKE_QUEUE_THRESHOLD(tq)  (5)
#define VMXNET3_RX_ALLOC_THRESHOLD(rq, ring_idx, adapter) \
	((rq)->rx_ring[ring_idx].size >> 3)

#define VMXNET3_GET_ADDR_LO(dma)   ((u32)(dma))
#define VMXNET3_GET_ADDR_HI(dma)   ((u32)(((u64)(dma)) >> 32))


#define VMXNET3_DEF_TX_RING_SIZE    512
#define VMXNET3_DEF_RX_RING_SIZE    256

#define VMXNET3_MAX_ETH_HDR_SIZE    22
#define VMXNET3_MAX_SKB_BUF_SIZE    (3*1024)

int
vmxnet3_quiesce_dev(struct vmxnet3_adapter *adapter);

int
vmxnet3_activate_dev(struct vmxnet3_adapter *adapter);

void
vmxnet3_force_close(struct vmxnet3_adapter *adapter);

void
vmxnet3_reset_dev(struct vmxnet3_adapter *adapter);

void
vmxnet3_tq_destroy(struct vmxnet3_tx_queue *tq,
		   struct vmxnet3_adapter *adapter);

void
vmxnet3_rq_destroy(struct vmxnet3_rx_queue *rq,
		   struct vmxnet3_adapter *adapter);

int
vmxnet3_create_queues(struct vmxnet3_adapter *adapter,
		      u32 tx_ring_size, u32 rx_ring_size, u32 rx_ring2_size);

extern void vmxnet3_set_ethtool_ops(struct net_device *netdev);
extern struct net_device_stats *vmxnet3_get_stats(struct net_device *netdev);

extern char vmxnet3_driver_name[];
#endif
