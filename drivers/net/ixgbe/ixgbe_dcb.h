

#ifndef _DCB_CONFIG_H_
#define _DCB_CONFIG_H_

#include "ixgbe_type.h"



#define IXGBE_MAX_PACKET_BUFFERS 8
#define MAX_USER_PRIORITY        8
#define MAX_TRAFFIC_CLASS        8
#define MAX_BW_GROUP             8
#define BW_PERCENT               100

#define DCB_TX_CONFIG            0
#define DCB_RX_CONFIG            1


#define DCB_SUCCESS              0
#define DCB_ERR_CONFIG           -1
#define DCB_ERR_PARAM            -2



#define DCB_ERR_BW_GROUP        -3

#define DCB_ERR_TC_BW           -4

#define DCB_ERR_LS_GS           -5

#define DCB_ERR_LS_BW_NONZERO   -6

#define DCB_ERR_LS_BWG_NONZERO  -7

#define DCB_ERR_TC_BW_ZERO      -8

#define DCB_NOT_IMPLEMENTED      0x7FFFFFFF

struct dcb_pfc_tc_debug {
	u8  tc;
	u8  pause_status;
	u64 pause_quanta;
};

enum strict_prio_type {
	prio_none = 0,
	prio_group,
	prio_link
};


#define IXGBE_DCB_PG_SUPPORT        0x00000001
#define IXGBE_DCB_PFC_SUPPORT       0x00000002
#define IXGBE_DCB_BCN_SUPPORT       0x00000004
#define IXGBE_DCB_UP2TC_SUPPORT     0x00000008
#define IXGBE_DCB_GSP_SUPPORT       0x00000010

#define IXGBE_DCB_8_TC_SUPPORT      0x80

struct dcb_support {
	
	u32 capabilities;

	
	u8  traffic_classes;
	u8  pfc_traffic_classes;
};


struct tc_bw_alloc {
	u8 bwg_id;		  
	u8 bwg_percent;		  
	u8 link_percent;	  
	u8 up_to_tc_bitmap;	  
	u16 data_credits_refill;  
	u16 data_credits_max;	  
	enum strict_prio_type prio_type; 
};

enum dcb_pfc_type {
	pfc_disabled = 0,
	pfc_enabled_full,
	pfc_enabled_tx,
	pfc_enabled_rx
};


struct tc_configuration {
	struct tc_bw_alloc path[2]; 
	enum dcb_pfc_type  dcb_pfc; 

	u16 desc_credits_max; 
	u8 tc; 
};

enum dcb_rx_pba_cfg {
	pba_equal,     
	pba_80_48      
};

struct dcb_num_tcs {
	u8 pg_tcs;
	u8 pfc_tcs;
};

struct ixgbe_dcb_config {
	struct dcb_support support;
	struct dcb_num_tcs num_tcs;
	struct tc_configuration tc_config[MAX_TRAFFIC_CLASS];
	u8     bw_percentage[2][MAX_BW_GROUP]; 
	bool   pfc_mode_enable;
	bool   round_robin_enable;

	enum dcb_rx_pba_cfg rx_pba_cfg;

	u32  dcb_cfg_version; 
	u32  link_speed; 
};




s32 ixgbe_dcb_check_config(struct ixgbe_dcb_config *config);


s32 ixgbe_dcb_calculate_tc_credits(struct ixgbe_dcb_config *, u8);


s32 ixgbe_dcb_config_pfc(struct ixgbe_hw *, struct ixgbe_dcb_config *g);
s32 ixgbe_dcb_get_pfc_stats(struct ixgbe_hw *, struct ixgbe_hw_stats *, u8);


s32 ixgbe_dcb_config_tc_stats(struct ixgbe_hw *);
s32 ixgbe_dcb_get_tc_stats(struct ixgbe_hw *, struct ixgbe_hw_stats *, u8);


s32 ixgbe_dcb_config_tx_desc_arbiter(struct ixgbe_hw *,
                                     struct ixgbe_dcb_config *);
s32 ixgbe_dcb_config_tx_data_arbiter(struct ixgbe_hw *,
                                     struct ixgbe_dcb_config *);
s32 ixgbe_dcb_config_rx_arbiter(struct ixgbe_hw *, struct ixgbe_dcb_config *);


s32 ixgbe_dcb_hw_config(struct ixgbe_hw *, struct ixgbe_dcb_config *);


#define MAX_CREDIT_REFILL       511  
#define MINIMUM_CREDIT_REFILL   5    
#define MINIMUM_CREDIT_FOR_JUMBO 145  
#define DCB_MAX_TSO_SIZE        (32*1024) 
#define MINIMUM_CREDIT_FOR_TSO  (DCB_MAX_TSO_SIZE/64 + 1) 
#define MAX_CREDIT              4095 

#endif 
