

#ifndef BNX2X_LINK_H
#define BNX2X_LINK_H






#define DEFAULT_PHY_DEV_ADDR 3



#define BNX2X_FLOW_CTRL_AUTO		PORT_FEATURE_FLOW_CONTROL_AUTO
#define BNX2X_FLOW_CTRL_TX		PORT_FEATURE_FLOW_CONTROL_TX
#define BNX2X_FLOW_CTRL_RX		PORT_FEATURE_FLOW_CONTROL_RX
#define BNX2X_FLOW_CTRL_BOTH		PORT_FEATURE_FLOW_CONTROL_BOTH
#define BNX2X_FLOW_CTRL_NONE		PORT_FEATURE_FLOW_CONTROL_NONE

#define SPEED_AUTO_NEG	    0
#define SPEED_12000		12000
#define SPEED_12500		12500
#define SPEED_13000		13000
#define SPEED_15000		15000
#define SPEED_16000		16000

#define SFP_EEPROM_VENDOR_NAME_ADDR		0x14
#define SFP_EEPROM_VENDOR_NAME_SIZE		16
#define SFP_EEPROM_VENDOR_OUI_ADDR		0x25
#define SFP_EEPROM_VENDOR_OUI_SIZE		3
#define SFP_EEPROM_PART_NO_ADDR 		0x28
#define SFP_EEPROM_PART_NO_SIZE		16
#define PWR_FLT_ERR_MSG_LEN			250




struct link_params {

	u8 port;

	
	u8 loopback_mode;
#define LOOPBACK_NONE	0
#define LOOPBACK_EMAC	1
#define LOOPBACK_BMAC	2
#define LOOPBACK_XGXS_10	3
#define LOOPBACK_EXT_PHY	4
#define LOOPBACK_EXT 	5

	u16 req_duplex;
	u16 req_flow_ctrl;
	u16 req_fc_auto_adv; 
	u16 req_line_speed; 

	
	u8 mac_addr[6];

	
	u32 shmem_base;
	u32 speed_cap_mask;
	u32 switch_cfg;
#define SWITCH_CFG_1G		PORT_FEATURE_CON_SWITCH_1G_SWITCH
#define SWITCH_CFG_10G		PORT_FEATURE_CON_SWITCH_10G_SWITCH
#define SWITCH_CFG_AUTO_DETECT	PORT_FEATURE_CON_SWITCH_AUTO_DETECT

	u16 hw_led_mode; 

	
	u8 phy_addr;
	

	u32 lane_config;
	u32 ext_phy_config;
#define XGXS_EXT_PHY_TYPE(ext_phy_config) \
		((ext_phy_config) & PORT_HW_CFG_XGXS_EXT_PHY_TYPE_MASK)
#define XGXS_EXT_PHY_ADDR(ext_phy_config) \
		(((ext_phy_config) & PORT_HW_CFG_XGXS_EXT_PHY_ADDR_MASK) >> \
		 PORT_HW_CFG_XGXS_EXT_PHY_ADDR_SHIFT)
#define SERDES_EXT_PHY_TYPE(ext_phy_config) \
		((ext_phy_config) & PORT_HW_CFG_SERDES_EXT_PHY_TYPE_MASK)

	
	u32 chip_id;

	u16 xgxs_config_rx[4]; 
	u16 xgxs_config_tx[4]; 

	u32 feature_config_flags;
#define FEATURE_CONFIG_OVERRIDE_PREEMPHASIS_ENABLED (1<<0)
#define FEATURE_CONFIG_BC_SUPPORTS_OPT_MDL_VRFY	(1<<2)
#define FEATURE_CONFIG_BCM8727_NOC			(1<<3)

	
	struct bnx2x *bp;
};


struct link_vars {
	u8 phy_flags;

	u8 mac_type;
#define MAC_TYPE_NONE		0
#define MAC_TYPE_EMAC		1
#define MAC_TYPE_BMAC		2

	u8 phy_link_up; 
	u8 link_up;

	u16 line_speed;
	u16 duplex;

	u16 flow_ctrl;
	u16 ieee_fc;

	u32 autoneg;
#define AUTO_NEG_DISABLED			0x0
#define AUTO_NEG_ENABLED			0x1
#define AUTO_NEG_COMPLETE			0x2
#define AUTO_NEG_PARALLEL_DETECTION_USED	0x3

	
	u32 link_status;
};






u8 bnx2x_phy_init(struct link_params *input, struct link_vars *output);


u8 bnx2x_link_reset(struct link_params *params, struct link_vars *vars,
		  u8 reset_ext_phy);


u8 bnx2x_link_update(struct link_params *input, struct link_vars *output);


u8 bnx2x_cl45_read(struct bnx2x *bp, u8 port, u32 ext_phy_type,
		 u8 phy_addr, u8 devad, u16 reg, u16 *ret_val);

u8 bnx2x_cl45_write(struct bnx2x *bp, u8 port, u32 ext_phy_type,
		  u8 phy_addr, u8 devad, u16 reg, u16 val);


void bnx2x_link_status_update(struct link_params *input,
			    struct link_vars *output);

u8 bnx2x_get_ext_phy_fw_version(struct link_params *params, u8 driver_loaded,
			      u8 *version, u16 len);


u8 bnx2x_set_led(struct bnx2x *bp, u8 port, u8 mode, u32 speed,
	       u16 hw_led_mode, u32 chip_id);
#define LED_MODE_OFF	0
#define LED_MODE_OPER 	2

u8 bnx2x_override_led_value(struct bnx2x *bp, u8 port, u32 led_idx, u32 value);


void bnx2x_handle_module_detect_int(struct link_params *params);


u8 bnx2x_test_link(struct link_params *input, struct link_vars *vars);


u8 bnx2x_common_init_phy(struct bnx2x *bp, u32 shmem_base);


void bnx2x_ext_phy_hw_reset(struct bnx2x *bp, u8 port);

void bnx2x_sfx7101_sp_sw_reset(struct bnx2x *bp, u8 port, u8 phy_addr);

u8 bnx2x_read_sfp_module_eeprom(struct link_params *params, u16 addr,
			      u8 byte_cnt, u8 *o_buf);

#endif 
