




#ifndef _IGB_H_
#define _IGB_H_

#include "e1000_mac.h"
#include "e1000_82575.h"

#include <linux/clocksource.h>
#include <linux/timecompare.h>
#include <linux/net_tstamp.h>

struct igb_adapter;


#define IGB_START_ITR 648


#define IGB_DEFAULT_TXD                  256
#define IGB_MIN_TXD                       80
#define IGB_MAX_TXD                     4096

#define IGB_DEFAULT_RXD                  256
#define IGB_MIN_RXD                       80
#define IGB_MAX_RXD                     4096

#define IGB_DEFAULT_ITR                    3 
#define IGB_MAX_ITR_USECS              10000
#define IGB_MIN_ITR_USECS                 10


#define IGB_MAX_RX_QUEUES     (adapter->vfs_allocated_count ? \
                               (adapter->vfs_allocated_count > 6 ? 1 : 2) : 4)
#define IGB_MAX_TX_QUEUES     IGB_MAX_RX_QUEUES
#define IGB_ABS_MAX_TX_QUEUES     4

#define IGB_MAX_VF_MC_ENTRIES              30
#define IGB_MAX_VF_FUNCTIONS               8
#define IGB_MAX_VFTA_ENTRIES               128

struct vf_data_storage {
	unsigned char vf_mac_addresses[ETH_ALEN];
	u16 vf_mc_hashes[IGB_MAX_VF_MC_ENTRIES];
	u16 num_vf_mc_hashes;
	u16 vlans_enabled;
	bool clear_to_send;
};


#define IGB_RX_PTHRESH                    16
#define IGB_RX_HTHRESH                     8
#define IGB_RX_WTHRESH                     1


#define MAXIMUM_ETHERNET_VLAN_SIZE 1522


#define IGB_RXBUFFER_128   128    
#define IGB_RXBUFFER_256   256    
#define IGB_RXBUFFER_512   512
#define IGB_RXBUFFER_1024  1024
#define IGB_RXBUFFER_2048  2048
#define IGB_RXBUFFER_16384 16384

#define MAX_STD_JUMBO_FRAME_SIZE 9234


#define IGB_TX_QUEUE_WAKE	16

#define IGB_RX_BUFFER_WRITE	16	

#define AUTO_ALL_MODES            0
#define IGB_EEPROM_APME         0x0400

#ifndef IGB_MASTER_SLAVE

#define IGB_MASTER_SLAVE	e1000_ms_hw_default
#endif

#define IGB_MNG_VLAN_NONE -1


struct igb_buffer {
	struct sk_buff *skb;
	dma_addr_t dma;
	union {
		
		struct {
			unsigned long time_stamp;
			u16 length;
			u16 next_to_watch;
		};
		
		struct {
			struct page *page;
			u64 page_dma;
			unsigned int page_offset;
		};
	};
};

struct igb_tx_queue_stats {
	u64 packets;
	u64 bytes;
};

struct igb_rx_queue_stats {
	u64 packets;
	u64 bytes;
	u64 drops;
};

struct igb_ring {
	struct igb_adapter *adapter; 
	void *desc;                  
	dma_addr_t dma;              
	unsigned int size;           
	unsigned int count;          
	u16 next_to_use;
	u16 next_to_clean;
	u16 head;
	u16 tail;
	struct igb_buffer *buffer_info; 

	u32 eims_value;
	u32 itr_val;
	u16 itr_register;
	u16 cpu;

	u16 queue_index;
	u16 reg_idx;
	unsigned int total_bytes;
	unsigned int total_packets;

	union {
		
		struct {
			struct igb_tx_queue_stats tx_stats;
			bool detect_tx_hung;
		};
		
		struct {
			struct igb_rx_queue_stats rx_stats;
			u64 rx_queue_drops;
			struct napi_struct napi;
			int set_itr;
			struct igb_ring *buddy;
		};
	};

	char name[IFNAMSIZ + 5];
};

#define E1000_RX_DESC_ADV(R, i)	    \
	(&(((union e1000_adv_rx_desc *)((R).desc))[i]))
#define E1000_TX_DESC_ADV(R, i)	    \
	(&(((union e1000_adv_tx_desc *)((R).desc))[i]))
#define E1000_TX_CTXTDESC_ADV(R, i)	    \
	(&(((struct e1000_adv_tx_context_desc *)((R).desc))[i]))



struct igb_adapter {
	struct timer_list watchdog_timer;
	struct timer_list phy_info_timer;
	struct vlan_group *vlgrp;
	u16 mng_vlan_id;
	u32 bd_number;
	u32 rx_buffer_len;
	u32 wol;
	u32 en_mng_pt;
	u16 link_speed;
	u16 link_duplex;
	unsigned int total_tx_bytes;
	unsigned int total_tx_packets;
	unsigned int total_rx_bytes;
	unsigned int total_rx_packets;
	
	u32 itr;
	u32 itr_setting;
	u16 tx_itr;
	u16 rx_itr;

	struct work_struct reset_task;
	struct work_struct watchdog_task;
	bool fc_autoneg;
	u8  tx_timeout_factor;
	struct timer_list blink_timer;
	unsigned long led_status;

	
	struct igb_ring *tx_ring;      
	unsigned int restart_queue;
	unsigned long tx_queue_len;
	u32 txd_cmd;
	u32 gotc;
	u64 gotc_old;
	u64 tpt_old;
	u64 colc_old;
	u32 tx_timeout_count;

	
	struct igb_ring *rx_ring;      
	int num_tx_queues;
	int num_rx_queues;

	u64 hw_csum_err;
	u64 hw_csum_good;
	u32 alloc_rx_buff_failed;
	u32 gorc;
	u64 gorc_old;
	u16 rx_ps_hdr_size;
	u32 max_frame_size;
	u32 min_frame_size;

	
	struct net_device *netdev;
	struct napi_struct napi;
	struct pci_dev *pdev;
	struct net_device_stats net_stats;
	struct cyclecounter cycles;
	struct timecounter clock;
	struct timecompare compare;
	struct hwtstamp_config hwtstamp_config;

	
	struct e1000_hw hw;
	struct e1000_hw_stats stats;
	struct e1000_phy_info phy_info;
	struct e1000_phy_stats phy_stats;

	u32 test_icr;
	struct igb_ring test_tx_ring;
	struct igb_ring test_rx_ring;

	int msg_enable;
	struct msix_entry *msix_entries;
	u32 eims_enable_mask;
	u32 eims_other;

	
	unsigned long state;
	unsigned int flags;
	u32 eeprom_wol;

	struct igb_ring *multi_tx_table[IGB_ABS_MAX_TX_QUEUES];
	unsigned int tx_ring_count;
	unsigned int rx_ring_count;
	unsigned int vfs_allocated_count;
	struct vf_data_storage *vf_data;
};

#define IGB_FLAG_HAS_MSI           (1 << 0)
#define IGB_FLAG_DCA_ENABLED       (1 << 1)
#define IGB_FLAG_QUAD_PORT_A       (1 << 2)
#define IGB_FLAG_NEED_CTX_IDX      (1 << 3)
#define IGB_FLAG_RX_CSUM_DISABLED  (1 << 4)

enum e1000_state_t {
	__IGB_TESTING,
	__IGB_RESETTING,
	__IGB_DOWN
};

enum igb_boards {
	board_82575,
};

extern char igb_driver_name[];
extern char igb_driver_version[];

extern char *igb_get_hw_dev_name(struct e1000_hw *hw);
extern int igb_up(struct igb_adapter *);
extern void igb_down(struct igb_adapter *);
extern void igb_reinit_locked(struct igb_adapter *);
extern void igb_reset(struct igb_adapter *);
extern int igb_set_spd_dplx(struct igb_adapter *, u16);
extern int igb_setup_tx_resources(struct igb_adapter *, struct igb_ring *);
extern int igb_setup_rx_resources(struct igb_adapter *, struct igb_ring *);
extern void igb_free_tx_resources(struct igb_ring *);
extern void igb_free_rx_resources(struct igb_ring *);
extern void igb_update_stats(struct igb_adapter *);
extern void igb_set_ethtool_ops(struct net_device *);

static inline s32 igb_reset_phy(struct e1000_hw *hw)
{
	if (hw->phy.ops.reset)
		return hw->phy.ops.reset(hw);

	return 0;
}

static inline s32 igb_read_phy_reg(struct e1000_hw *hw, u32 offset, u16 *data)
{
	if (hw->phy.ops.read_reg)
		return hw->phy.ops.read_reg(hw, offset, data);

	return 0;
}

static inline s32 igb_write_phy_reg(struct e1000_hw *hw, u32 offset, u16 data)
{
	if (hw->phy.ops.write_reg)
		return hw->phy.ops.write_reg(hw, offset, data);

	return 0;
}

static inline s32 igb_get_phy_info(struct e1000_hw *hw)
{
	if (hw->phy.ops.get_phy_info)
		return hw->phy.ops.get_phy_info(hw);

	return 0;
}

#endif 
