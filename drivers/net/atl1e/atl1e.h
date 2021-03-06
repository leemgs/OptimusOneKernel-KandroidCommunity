

#ifndef _ATL1E_H_
#define _ATL1E_H_

#include <linux/version.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/udp.h>
#include <linux/mii.h>
#include <linux/io.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>
#include <linux/tcp.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <linux/workqueue.h>
#include <net/checksum.h>
#include <net/ip6_checksum.h>

#include "atl1e_hw.h"

#define PCI_REG_COMMAND	 0x04    
#define CMD_IO_SPACE	 0x0001
#define CMD_MEMORY_SPACE 0x0002
#define CMD_BUS_MASTER   0x0004

#define BAR_0   0
#define BAR_1   1
#define BAR_5   5


#define AT_WUFC_LNKC 0x00000001 
#define AT_WUFC_MAG  0x00000002 
#define AT_WUFC_EX   0x00000004 
#define AT_WUFC_MC   0x00000008 
#define AT_WUFC_BC   0x00000010 

#define SPEED_0		   0xffff
#define HALF_DUPLEX        1
#define FULL_DUPLEX        2


#define AT_ERR_EEPROM      1
#define AT_ERR_PHY         2
#define AT_ERR_CONFIG      3
#define AT_ERR_PARAM       4
#define AT_ERR_MAC_TYPE    5
#define AT_ERR_PHY_TYPE    6
#define AT_ERR_PHY_SPEED   7
#define AT_ERR_PHY_RES     8
#define AT_ERR_TIMEOUT     9

#define MAX_JUMBO_FRAME_SIZE 0x2000

#define AT_VLAN_TAG_TO_TPD_TAG(_vlan, _tpd)    \
	_tpd = (((_vlan) << (4)) | (((_vlan) >> 13) & 7) |\
		 (((_vlan) >> 9) & 8))

#define AT_TPD_TAG_TO_VLAN_TAG(_tpd, _vlan)    \
	_vlan = (((_tpd) >> 8) | (((_tpd) & 0x77) << 9) |\
		   (((_tdp) & 0x88) << 5))

#define AT_MAX_RECEIVE_QUEUE    4
#define AT_PAGE_NUM_PER_QUEUE   2

#define AT_DMA_HI_ADDR_MASK     0xffffffff00000000ULL
#define AT_DMA_LO_ADDR_MASK     0x00000000ffffffffULL

#define AT_TX_WATCHDOG  (5 * HZ)
#define AT_MAX_INT_WORK		10
#define AT_TWSI_EEPROM_TIMEOUT 	100
#define AT_HW_MAX_IDLE_DELAY 	10
#define AT_SUSPEND_LINK_TIMEOUT 28

#define AT_REGS_LEN	75
#define AT_EEPROM_LEN 	512
#define AT_ADV_MASK	(ADVERTISE_10_HALF  |\
			 ADVERTISE_10_FULL  |\
			 ADVERTISE_100_HALF |\
			 ADVERTISE_100_FULL |\
			 ADVERTISE_1000_FULL)


#define TPD_BUFLEN_MASK 	0x3FFF
#define TPD_BUFLEN_SHIFT        0
#define TPD_DMAINT_MASK		0x0001
#define TPD_DMAINT_SHIFT        14
#define TPD_PKTNT_MASK          0x0001
#define TPD_PKTINT_SHIFT        15
#define TPD_VLANTAG_MASK        0xFFFF
#define TPD_VLAN_SHIFT          16


#define TPD_EOP_MASK            0x0001
#define TPD_EOP_SHIFT           0
#define TPD_IP_VERSION_MASK	0x0001
#define TPD_IP_VERSION_SHIFT	1	
#define TPD_INS_VL_TAG_MASK	0x0001
#define TPD_INS_VL_TAG_SHIFT	2
#define TPD_CC_SEGMENT_EN_MASK	0x0001
#define TPD_CC_SEGMENT_EN_SHIFT	3
#define TPD_SEGMENT_EN_MASK     0x0001
#define TPD_SEGMENT_EN_SHIFT    4


#define TPD_IP_CSUM_MASK        0x0001
#define TPD_IP_CSUM_SHIFT       5
#define TPD_TCP_CSUM_MASK       0x0001
#define TPD_TCP_CSUM_SHIFT      6
#define TPD_UDP_CSUM_MASK       0x0001
#define TPD_UDP_CSUM_SHIFT      7


#define TPD_V6_IPHLLO_MASK	0x0007
#define TPD_V6_IPHLLO_SHIFT	7


#define TPD_VL_TAGGED_MASK      0x0001
#define TPD_VL_TAGGED_SHIFT     8
#define TPD_ETHTYPE_MASK        0x0001
#define TPD_ETHTYPE_SHIFT       9


#define TDP_V4_IPHL_MASK	0x000F
#define TPD_V4_IPHL_SHIFT	10


#define TPD_V6_IPHLHI_MASK	0x000F
#define TPD_V6_IPHLHI_SHIFT	10


#define TPD_TCPHDRLEN_MASK      0x000F
#define TPD_TCPHDRLEN_SHIFT     14
#define TPD_HDRFLAG_MASK        0x0001
#define TPD_HDRFLAG_SHIFT       18
#define TPD_MSS_MASK            0x1FFF
#define TPD_MSS_SHIFT           19


#define TPD_PLOADOFFSET_MASK    0x00FF
#define TPD_PLOADOFFSET_SHIFT   16
#define TPD_CCSUMOFFSET_MASK    0x00FF
#define TPD_CCSUMOFFSET_SHIFT   24

struct atl1e_tpd_desc {
	__le64 buffer_addr;
	__le32 word2;
	__le32 word3;
};


#define MAX_TX_BUF_LEN      0x2000
#define MAX_TX_BUF_SHIFT    13



#define RRS_RX_CSUM_MASK	0xFFFF
#define RRS_RX_CSUM_SHIFT	0
#define RRS_PKT_SIZE_MASK	0x3FFF
#define RRS_PKT_SIZE_SHIFT	16
#define RRS_CPU_NUM_MASK	0x0003
#define	RRS_CPU_NUM_SHIFT	30

#define	RRS_IS_RSS_IPV4		0x0001
#define RRS_IS_RSS_IPV4_TCP	0x0002
#define RRS_IS_RSS_IPV6		0x0004
#define RRS_IS_RSS_IPV6_TCP	0x0008
#define RRS_IS_IPV6		0x0010
#define RRS_IS_IP_FRAG		0x0020
#define RRS_IS_IP_DF		0x0040
#define RRS_IS_802_3		0x0080
#define RRS_IS_VLAN_TAG		0x0100
#define RRS_IS_ERR_FRAME	0x0200
#define RRS_IS_IPV4		0x0400
#define RRS_IS_UDP		0x0800
#define RRS_IS_TCP		0x1000
#define RRS_IS_BCAST		0x2000
#define RRS_IS_MCAST		0x4000
#define RRS_IS_PAUSE		0x8000

#define RRS_ERR_BAD_CRC		0x0001
#define RRS_ERR_CODE		0x0002
#define RRS_ERR_DRIBBLE		0x0004
#define RRS_ERR_RUNT		0x0008
#define RRS_ERR_RX_OVERFLOW	0x0010
#define RRS_ERR_TRUNC		0x0020
#define RRS_ERR_IP_CSUM		0x0040
#define RRS_ERR_L4_CSUM		0x0080
#define RRS_ERR_LENGTH		0x0100
#define RRS_ERR_DES_ADDR	0x0200

struct atl1e_recv_ret_status {
	u16 seq_num;
	u16 hash_lo;
	__le32	word1;
	u16 pkt_flag;
	u16 err_flag;
	u16 hash_hi;
	u16 vtag;
};

enum atl1e_dma_req_block {
	atl1e_dma_req_128 = 0,
	atl1e_dma_req_256 = 1,
	atl1e_dma_req_512 = 2,
	atl1e_dma_req_1024 = 3,
	atl1e_dma_req_2048 = 4,
	atl1e_dma_req_4096 = 5
};

enum atl1e_rrs_type {
	atl1e_rrs_disable = 0,
	atl1e_rrs_ipv4 = 1,
	atl1e_rrs_ipv4_tcp = 2,
	atl1e_rrs_ipv6 = 4,
	atl1e_rrs_ipv6_tcp = 8
};

enum atl1e_nic_type {
	athr_l1e = 0,
	athr_l2e_revA = 1,
	athr_l2e_revB = 2
};

struct atl1e_hw_stats {
	
	unsigned long rx_ok;	      
	unsigned long rx_bcast;       
	unsigned long rx_mcast;       
	unsigned long rx_pause;       
	unsigned long rx_ctrl;        
	unsigned long rx_fcs_err;     
	unsigned long rx_len_err;     
	unsigned long rx_byte_cnt;    
	unsigned long rx_runt;        
	unsigned long rx_frag;        
	unsigned long rx_sz_64;       
	unsigned long rx_sz_65_127;   
	unsigned long rx_sz_128_255;  
	unsigned long rx_sz_256_511;  
	unsigned long rx_sz_512_1023; 
	unsigned long rx_sz_1024_1518;    
	unsigned long rx_sz_1519_max; 
	unsigned long rx_sz_ov;       
	unsigned long rx_rxf_ov;      
	unsigned long rx_rrd_ov;      
	unsigned long rx_align_err;   
	unsigned long rx_bcast_byte_cnt;  
	unsigned long rx_mcast_byte_cnt;  
	unsigned long rx_err_addr;    

	
	unsigned long tx_ok;      
	unsigned long tx_bcast;       
	unsigned long tx_mcast;       
	unsigned long tx_pause;       
	unsigned long tx_exc_defer;   
	unsigned long tx_ctrl;        
	unsigned long tx_defer;       
	unsigned long tx_byte_cnt;    
	unsigned long tx_sz_64;       
	unsigned long tx_sz_65_127;   
	unsigned long tx_sz_128_255;  
	unsigned long tx_sz_256_511;  
	unsigned long tx_sz_512_1023; 
	unsigned long tx_sz_1024_1518;    
	unsigned long tx_sz_1519_max; 
	unsigned long tx_1_col;       
	unsigned long tx_2_col;       
	unsigned long tx_late_col;    
	unsigned long tx_abort_col;   
	unsigned long tx_underrun;    
	unsigned long tx_rd_eop;      
	unsigned long tx_len_err;     
	unsigned long tx_trunc;       
	unsigned long tx_bcast_byte;  
	unsigned long tx_mcast_byte;  
};

struct atl1e_hw {
	u8 __iomem      *hw_addr;            
	resource_size_t mem_rang;
	struct atl1e_adapter *adapter;
	enum atl1e_nic_type  nic_type;
	u16 device_id;
	u16 vendor_id;
	u16 subsystem_id;
	u16 subsystem_vendor_id;
	u8  revision_id;
	u16 pci_cmd_word;
	u8 mac_addr[ETH_ALEN];
	u8 perm_mac_addr[ETH_ALEN];
	u8 preamble_len;
	u16 max_frame_size;
	u16 rx_jumbo_th;
	u16 tx_jumbo_th;

	u16 media_type;
#define MEDIA_TYPE_AUTO_SENSOR  0
#define MEDIA_TYPE_100M_FULL    1
#define MEDIA_TYPE_100M_HALF    2
#define MEDIA_TYPE_10M_FULL     3
#define MEDIA_TYPE_10M_HALF     4

	u16 autoneg_advertised;
#define ADVERTISE_10_HALF               0x0001
#define ADVERTISE_10_FULL               0x0002
#define ADVERTISE_100_HALF              0x0004
#define ADVERTISE_100_FULL              0x0008
#define ADVERTISE_1000_HALF             0x0010 
#define ADVERTISE_1000_FULL             0x0020
	u16 mii_autoneg_adv_reg;
	u16 mii_1000t_ctrl_reg;

	u16 imt;        
	u16 ict;        
	u32 smb_timer;
	u16 rrd_thresh; 
	u16 tpd_thresh;
	u16 rx_count_down; 
	u16 tx_count_down;

	u8 tpd_burst;   
	enum atl1e_rrs_type rrs_type;
	u32 base_cpu;
	u32 indirect_tab;

	enum atl1e_dma_req_block dmar_block;
	enum atl1e_dma_req_block dmaw_block;
	u8 dmaw_dly_cnt;
	u8 dmar_dly_cnt;

	bool phy_configured;
	bool re_autoneg;
	bool emi_ca;
};


struct atl1e_tx_buffer {
	struct sk_buff *skb;
	u16 flags;
#define ATL1E_TX_PCIMAP_SINGLE		0x0001
#define ATL1E_TX_PCIMAP_PAGE		0x0002
#define ATL1E_TX_PCIMAP_TYPE_MASK	0x0003
	u16 length;
	dma_addr_t dma;
};

#define ATL1E_SET_PCIMAP_TYPE(tx_buff, type) do {		\
	((tx_buff)->flags) &= ~ATL1E_TX_PCIMAP_TYPE_MASK;	\
	((tx_buff)->flags) |= (type);				\
	} while (0)

struct atl1e_rx_page {
	dma_addr_t	dma;    
	u8		*addr;   
	dma_addr_t	write_offset_dma;  
	u32		*write_offset_addr; 
	u32		read_offset;       
};

struct atl1e_rx_page_desc {
	struct atl1e_rx_page   rx_page[AT_PAGE_NUM_PER_QUEUE];
	u8  rx_using;
	u16 rx_nxseq;
};


struct atl1e_tx_ring {
	struct atl1e_tpd_desc *desc;  
	dma_addr_t	   dma;    
	u16       	   count;  
	rwlock_t	   tx_lock;
	u16		   next_to_use;
	atomic_t	   next_to_clean;
	struct atl1e_tx_buffer *tx_buffer;
	dma_addr_t	   cmb_dma;
	u32		   *cmb;
};


struct atl1e_rx_ring {
	void        	*desc;
	dma_addr_t  	dma;
	int         	size;
	u32	    	page_size; 
	u32		real_page_size; 
	struct atl1e_rx_page_desc	rx_page_desc[AT_MAX_RECEIVE_QUEUE];
};


struct atl1e_adapter {
	struct net_device   *netdev;
	struct pci_dev      *pdev;
	struct vlan_group   *vlgrp;
	struct napi_struct  napi;
	struct mii_if_info  mii;    
	struct atl1e_hw        hw;
	struct atl1e_hw_stats  hw_stats;

	bool have_msi;
	u32 wol;
	u16 link_speed;
	u16 link_duplex;

	spinlock_t mdio_lock;
	spinlock_t tx_lock;
	atomic_t irq_sem;

	struct work_struct reset_task;
	struct work_struct link_chg_task;
	struct timer_list watchdog_timer;
	struct timer_list phy_config_timer;

	
	dma_addr_t  	ring_dma;
	void     	*ring_vir_addr;
	u32             ring_size;

	struct atl1e_tx_ring tx_ring;
	struct atl1e_rx_ring rx_ring;
	int num_rx_queues;
	unsigned long flags;
#define __AT_TESTING        0x0001
#define __AT_RESETTING      0x0002
#define __AT_DOWN           0x0003

	u32 bd_number;     
	u32 pci_state[16];
	u32 *config_space;
};

#define AT_WRITE_REG(a, reg, value) ( \
		writel((value), ((a)->hw_addr + reg)))

#define AT_WRITE_FLUSH(a) (\
		readl((a)->hw_addr))

#define AT_READ_REG(a, reg) ( \
		readl((a)->hw_addr + reg))

#define AT_WRITE_REGB(a, reg, value) (\
		writeb((value), ((a)->hw_addr + reg)))

#define AT_READ_REGB(a, reg) (\
		readb((a)->hw_addr + reg))

#define AT_WRITE_REGW(a, reg, value) (\
		writew((value), ((a)->hw_addr + reg)))

#define AT_READ_REGW(a, reg) (\
		readw((a)->hw_addr + reg))

#define AT_WRITE_REG_ARRAY(a, reg, offset, value) ( \
		writel((value), (((a)->hw_addr + reg) + ((offset) << 2))))

#define AT_READ_REG_ARRAY(a, reg, offset) ( \
		readl(((a)->hw_addr + reg) + ((offset) << 2)))

extern char atl1e_driver_name[];
extern char atl1e_driver_version[];

extern void atl1e_check_options(struct atl1e_adapter *adapter);
extern int atl1e_up(struct atl1e_adapter *adapter);
extern void atl1e_down(struct atl1e_adapter *adapter);
extern void atl1e_reinit_locked(struct atl1e_adapter *adapter);
extern s32 atl1e_reset_hw(struct atl1e_hw *hw);
extern void atl1e_set_ethtool_ops(struct net_device *netdev);
#endif 
