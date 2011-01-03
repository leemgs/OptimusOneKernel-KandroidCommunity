



#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/pci.h>

#include "e1000.h"

#define ICH_FLASH_GFPREG		0x0000
#define ICH_FLASH_HSFSTS		0x0004
#define ICH_FLASH_HSFCTL		0x0006
#define ICH_FLASH_FADDR			0x0008
#define ICH_FLASH_FDATA0		0x0010
#define ICH_FLASH_PR0			0x0074

#define ICH_FLASH_READ_COMMAND_TIMEOUT	500
#define ICH_FLASH_WRITE_COMMAND_TIMEOUT	500
#define ICH_FLASH_ERASE_COMMAND_TIMEOUT	3000000
#define ICH_FLASH_LINEAR_ADDR_MASK	0x00FFFFFF
#define ICH_FLASH_CYCLE_REPEAT_COUNT	10

#define ICH_CYCLE_READ			0
#define ICH_CYCLE_WRITE			2
#define ICH_CYCLE_ERASE			3

#define FLASH_GFPREG_BASE_MASK		0x1FFF
#define FLASH_SECTOR_ADDR_SHIFT		12

#define ICH_FLASH_SEG_SIZE_256		256
#define ICH_FLASH_SEG_SIZE_4K		4096
#define ICH_FLASH_SEG_SIZE_8K		8192
#define ICH_FLASH_SEG_SIZE_64K		65536


#define E1000_ICH_FWSM_RSPCIPHY	0x00000040 

#define E1000_ICH_MNG_IAMT_MODE		0x2

#define ID_LED_DEFAULT_ICH8LAN  ((ID_LED_DEF1_DEF2 << 12) | \
				 (ID_LED_DEF1_OFF2 <<  8) | \
				 (ID_LED_DEF1_ON2  <<  4) | \
				 (ID_LED_DEF1_DEF2))

#define E1000_ICH_NVM_SIG_WORD		0x13
#define E1000_ICH_NVM_SIG_MASK		0xC000
#define E1000_ICH_NVM_VALID_SIG_MASK    0xC0
#define E1000_ICH_NVM_SIG_VALUE         0x80

#define E1000_ICH8_LAN_INIT_TIMEOUT	1500

#define E1000_FEXTNVM_SW_CONFIG		1
#define E1000_FEXTNVM_SW_CONFIG_ICH8M (1 << 27) 

#define PCIE_ICH8_SNOOP_ALL		PCIE_NO_SNOOP_ALL

#define E1000_ICH_RAR_ENTRIES		7

#define PHY_PAGE_SHIFT 5
#define PHY_REG(page, reg) (((page) << PHY_PAGE_SHIFT) | \
			   ((reg) & MAX_PHY_REG_ADDRESS))
#define IGP3_KMRN_DIAG  PHY_REG(770, 19) 
#define IGP3_VR_CTRL    PHY_REG(776, 18) 

#define IGP3_KMRN_DIAG_PCS_LOCK_LOSS	0x0002
#define IGP3_VR_CTRL_DEV_POWERDOWN_MODE_MASK 0x0300
#define IGP3_VR_CTRL_MODE_SHUTDOWN	0x0200

#define HV_LED_CONFIG		PHY_REG(768, 30) 

#define SW_FLAG_TIMEOUT    1000 


#define HV_SMB_ADDR            PHY_REG(768, 26)
#define HV_SMB_ADDR_PEC_EN     0x0200
#define HV_SMB_ADDR_VALID      0x0080


#define E1000_STRAP                     0x0000C
#define E1000_STRAP_SMBUS_ADDRESS_MASK  0x00FE0000
#define E1000_STRAP_SMBUS_ADDRESS_SHIFT 17


#define HV_OEM_BITS            PHY_REG(768, 25)
#define HV_OEM_BITS_LPLU       0x0004 
#define HV_OEM_BITS_GBE_DIS    0x0040 
#define HV_OEM_BITS_RESTART_AN 0x0400 

#define E1000_NVM_K1_CONFIG 0x1B 
#define E1000_NVM_K1_ENABLE 0x1  



union ich8_hws_flash_status {
	struct ich8_hsfsts {
		u16 flcdone    :1; 
		u16 flcerr     :1; 
		u16 dael       :1; 
		u16 berasesz   :2; 
		u16 flcinprog  :1; 
		u16 reserved1  :2; 
		u16 reserved2  :6; 
		u16 fldesvalid :1; 
		u16 flockdn    :1; 
	} hsf_status;
	u16 regval;
};



union ich8_hws_flash_ctrl {
	struct ich8_hsflctl {
		u16 flcgo      :1;   
		u16 flcycle    :2;   
		u16 reserved   :5;   
		u16 fldbcount  :2;   
		u16 flockdn    :6;   
	} hsf_ctrl;
	u16 regval;
};


union ich8_hws_flash_regacc {
	struct ich8_flracc {
		u32 grra      :8; 
		u32 grwa      :8; 
		u32 gmrag     :8; 
		u32 gmwag     :8; 
	} hsf_flregacc;
	u16 regval;
};


union ich8_flash_protected_range {
	struct ich8_pr {
		u32 base:13;     
		u32 reserved1:2; 
		u32 rpe:1;       
		u32 limit:13;    
		u32 reserved2:2; 
		u32 wpe:1;       
	} range;
	u32 regval;
};

static s32 e1000_setup_link_ich8lan(struct e1000_hw *hw);
static void e1000_clear_hw_cntrs_ich8lan(struct e1000_hw *hw);
static void e1000_initialize_hw_bits_ich8lan(struct e1000_hw *hw);
static s32 e1000_check_polarity_ife_ich8lan(struct e1000_hw *hw);
static s32 e1000_erase_flash_bank_ich8lan(struct e1000_hw *hw, u32 bank);
static s32 e1000_retry_write_flash_byte_ich8lan(struct e1000_hw *hw,
						u32 offset, u8 byte);
static s32 e1000_read_flash_byte_ich8lan(struct e1000_hw *hw, u32 offset,
					 u8 *data);
static s32 e1000_read_flash_word_ich8lan(struct e1000_hw *hw, u32 offset,
					 u16 *data);
static s32 e1000_read_flash_data_ich8lan(struct e1000_hw *hw, u32 offset,
					 u8 size, u16 *data);
static s32 e1000_setup_copper_link_ich8lan(struct e1000_hw *hw);
static s32 e1000_kmrn_lock_loss_workaround_ich8lan(struct e1000_hw *hw);
static s32 e1000_get_cfg_done_ich8lan(struct e1000_hw *hw);
static s32 e1000_cleanup_led_ich8lan(struct e1000_hw *hw);
static s32 e1000_led_on_ich8lan(struct e1000_hw *hw);
static s32 e1000_led_off_ich8lan(struct e1000_hw *hw);
static s32 e1000_id_led_init_pchlan(struct e1000_hw *hw);
static s32 e1000_setup_led_pchlan(struct e1000_hw *hw);
static s32 e1000_cleanup_led_pchlan(struct e1000_hw *hw);
static s32 e1000_led_on_pchlan(struct e1000_hw *hw);
static s32 e1000_led_off_pchlan(struct e1000_hw *hw);
static s32 e1000_set_lplu_state_pchlan(struct e1000_hw *hw, bool active);
static void e1000_lan_init_done_ich8lan(struct e1000_hw *hw);
static s32  e1000_k1_gig_workaround_hv(struct e1000_hw *hw, bool link);
static s32 e1000_configure_k1_ich8lan(struct e1000_hw *hw, bool k1_enable);

static inline u16 __er16flash(struct e1000_hw *hw, unsigned long reg)
{
	return readw(hw->flash_address + reg);
}

static inline u32 __er32flash(struct e1000_hw *hw, unsigned long reg)
{
	return readl(hw->flash_address + reg);
}

static inline void __ew16flash(struct e1000_hw *hw, unsigned long reg, u16 val)
{
	writew(val, hw->flash_address + reg);
}

static inline void __ew32flash(struct e1000_hw *hw, unsigned long reg, u32 val)
{
	writel(val, hw->flash_address + reg);
}

#define er16flash(reg)		__er16flash(hw, (reg))
#define er32flash(reg)		__er32flash(hw, (reg))
#define ew16flash(reg,val)	__ew16flash(hw, (reg), (val))
#define ew32flash(reg,val)	__ew32flash(hw, (reg), (val))


static s32 e1000_init_phy_params_pchlan(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val = 0;

	phy->addr                     = 1;
	phy->reset_delay_us           = 100;

	phy->ops.check_polarity       = e1000_check_polarity_ife_ich8lan;
	phy->ops.read_phy_reg         = e1000_read_phy_reg_hv;
	phy->ops.read_phy_reg_locked  = e1000_read_phy_reg_hv_locked;
	phy->ops.set_d0_lplu_state    = e1000_set_lplu_state_pchlan;
	phy->ops.set_d3_lplu_state    = e1000_set_lplu_state_pchlan;
	phy->ops.write_phy_reg        = e1000_write_phy_reg_hv;
	phy->ops.write_phy_reg_locked = e1000_write_phy_reg_hv_locked;
	phy->autoneg_mask             = AUTONEG_ADVERTISE_SPEED_DEFAULT;

	phy->id = e1000_phy_unknown;
	e1000e_get_phy_id(hw);
	phy->type = e1000e_get_phy_type_from_id(phy->id);

	if (phy->type == e1000_phy_82577) {
		phy->ops.check_polarity = e1000_check_polarity_82577;
		phy->ops.force_speed_duplex =
			e1000_phy_force_speed_duplex_82577;
		phy->ops.get_cable_length   = e1000_get_cable_length_82577;
		phy->ops.get_phy_info = e1000_get_phy_info_82577;
		phy->ops.commit_phy = e1000e_phy_sw_reset;
	}

	return ret_val;
}


static s32 e1000_init_phy_params_ich8lan(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 i = 0;

	phy->addr			= 1;
	phy->reset_delay_us		= 100;

	
	ret_val = e1000e_determine_phy_address(hw);
	if (ret_val) {
		hw->phy.ops.write_phy_reg = e1000e_write_phy_reg_bm;
		hw->phy.ops.read_phy_reg  = e1000e_read_phy_reg_bm;
		ret_val = e1000e_determine_phy_address(hw);
		if (ret_val)
			return ret_val;
	}

	phy->id = 0;
	while ((e1000_phy_unknown == e1000e_get_phy_type_from_id(phy->id)) &&
	       (i++ < 100)) {
		msleep(1);
		ret_val = e1000e_get_phy_id(hw);
		if (ret_val)
			return ret_val;
	}

	
	switch (phy->id) {
	case IGP03E1000_E_PHY_ID:
		phy->type = e1000_phy_igp_3;
		phy->autoneg_mask = AUTONEG_ADVERTISE_SPEED_DEFAULT;
		phy->ops.read_phy_reg_locked = e1000e_read_phy_reg_igp_locked;
		phy->ops.write_phy_reg_locked = e1000e_write_phy_reg_igp_locked;
		break;
	case IFE_E_PHY_ID:
	case IFE_PLUS_E_PHY_ID:
	case IFE_C_E_PHY_ID:
		phy->type = e1000_phy_ife;
		phy->autoneg_mask = E1000_ALL_NOT_GIG;
		break;
	case BME1000_E_PHY_ID:
		phy->type = e1000_phy_bm;
		phy->autoneg_mask = AUTONEG_ADVERTISE_SPEED_DEFAULT;
		hw->phy.ops.read_phy_reg = e1000e_read_phy_reg_bm;
		hw->phy.ops.write_phy_reg = e1000e_write_phy_reg_bm;
		hw->phy.ops.commit_phy = e1000e_phy_sw_reset;
		break;
	default:
		return -E1000_ERR_PHY;
		break;
	}

	phy->ops.check_polarity = e1000_check_polarity_ife_ich8lan;

	return 0;
}


static s32 e1000_init_nvm_params_ich8lan(struct e1000_hw *hw)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	struct e1000_dev_spec_ich8lan *dev_spec = &hw->dev_spec.ich8lan;
	u32 gfpreg, sector_base_addr, sector_end_addr;
	u16 i;

	
	if (!hw->flash_address) {
		hw_dbg(hw, "ERROR: Flash registers not mapped\n");
		return -E1000_ERR_CONFIG;
	}

	nvm->type = e1000_nvm_flash_sw;

	gfpreg = er32flash(ICH_FLASH_GFPREG);

	
	sector_base_addr = gfpreg & FLASH_GFPREG_BASE_MASK;
	sector_end_addr = ((gfpreg >> 16) & FLASH_GFPREG_BASE_MASK) + 1;

	
	nvm->flash_base_addr = sector_base_addr << FLASH_SECTOR_ADDR_SHIFT;

	
	nvm->flash_bank_size = (sector_end_addr - sector_base_addr)
				<< FLASH_SECTOR_ADDR_SHIFT;
	nvm->flash_bank_size /= 2;
	
	nvm->flash_bank_size /= sizeof(u16);

	nvm->word_size = E1000_ICH8_SHADOW_RAM_WORDS;

	
	for (i = 0; i < nvm->word_size; i++) {
		dev_spec->shadow_ram[i].modified = 0;
		dev_spec->shadow_ram[i].value    = 0xFFFF;
	}

	return 0;
}


static s32 e1000_init_mac_params_ich8lan(struct e1000_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	struct e1000_mac_info *mac = &hw->mac;

	
	hw->phy.media_type = e1000_media_type_copper;

	
	mac->mta_reg_count = 32;
	
	mac->rar_entry_count = E1000_ICH_RAR_ENTRIES;
	if (mac->type == e1000_ich8lan)
		mac->rar_entry_count--;
	
	mac->arc_subsystem_valid = 1;

	
	switch (mac->type) {
	case e1000_ich8lan:
	case e1000_ich9lan:
	case e1000_ich10lan:
		
		mac->ops.id_led_init = e1000e_id_led_init;
		
		mac->ops.setup_led = e1000e_setup_led_generic;
		
		mac->ops.cleanup_led = e1000_cleanup_led_ich8lan;
		
		mac->ops.led_on = e1000_led_on_ich8lan;
		mac->ops.led_off = e1000_led_off_ich8lan;
		break;
	case e1000_pchlan:
		
		mac->ops.id_led_init = e1000_id_led_init_pchlan;
		
		mac->ops.setup_led = e1000_setup_led_pchlan;
		
		mac->ops.cleanup_led = e1000_cleanup_led_pchlan;
		
		mac->ops.led_on = e1000_led_on_pchlan;
		mac->ops.led_off = e1000_led_off_pchlan;
		break;
	default:
		break;
	}

	
	if (mac->type == e1000_ich8lan)
		e1000e_set_kmrn_lock_loss_workaround_ich8lan(hw, 1);

	return 0;
}


static s32 e1000_check_for_copper_link_ich8lan(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val;
	bool link;

	
	if (!mac->get_link_status) {
		ret_val = 0;
		goto out;
	}

	
	ret_val = e1000e_phy_has_link_generic(hw, 1, 0, &link);
	if (ret_val)
		goto out;

	if (hw->mac.type == e1000_pchlan) {
		ret_val = e1000_k1_gig_workaround_hv(hw, link);
		if (ret_val)
			goto out;
	}

	if (!link)
		goto out; 

	mac->get_link_status = false;

	if (hw->phy.type == e1000_phy_82578) {
		ret_val = e1000_link_stall_workaround_hv(hw);
		if (ret_val)
			goto out;
	}

	
	e1000e_check_downshift(hw);

	
	if (!mac->autoneg) {
		ret_val = -E1000_ERR_CONFIG;
		goto out;
	}

	
	e1000e_config_collision_dist(hw);

	
	ret_val = e1000e_config_fc_after_link_up(hw);
	if (ret_val)
		hw_dbg(hw, "Error configuring flow control\n");

out:
	return ret_val;
}

static s32 e1000_get_variants_ich8lan(struct e1000_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	s32 rc;

	rc = e1000_init_mac_params_ich8lan(adapter);
	if (rc)
		return rc;

	rc = e1000_init_nvm_params_ich8lan(hw);
	if (rc)
		return rc;

	if (hw->mac.type == e1000_pchlan)
		rc = e1000_init_phy_params_pchlan(hw);
	else
		rc = e1000_init_phy_params_ich8lan(hw);
	if (rc)
		return rc;

	if (adapter->hw.phy.type == e1000_phy_ife) {
		adapter->flags &= ~FLAG_HAS_JUMBO_FRAMES;
		adapter->max_hw_frame_size = ETH_FRAME_LEN + ETH_FCS_LEN;
	}

	if ((adapter->hw.mac.type == e1000_ich8lan) &&
	    (adapter->hw.phy.type == e1000_phy_igp_3))
		adapter->flags |= FLAG_LSC_GIG_SPEED_DROP;

	return 0;
}

static DEFINE_MUTEX(nvm_mutex);


static s32 e1000_acquire_nvm_ich8lan(struct e1000_hw *hw)
{
	mutex_lock(&nvm_mutex);

	return 0;
}


static void e1000_release_nvm_ich8lan(struct e1000_hw *hw)
{
	mutex_unlock(&nvm_mutex);

	return;
}

static DEFINE_MUTEX(swflag_mutex);


static s32 e1000_acquire_swflag_ich8lan(struct e1000_hw *hw)
{
	u32 extcnf_ctrl, timeout = PHY_CFG_TIMEOUT;
	s32 ret_val = 0;

	might_sleep();

	mutex_lock(&swflag_mutex);

	while (timeout) {
		extcnf_ctrl = er32(EXTCNF_CTRL);
		if (!(extcnf_ctrl & E1000_EXTCNF_CTRL_SWFLAG))
			break;

		mdelay(1);
		timeout--;
	}

	if (!timeout) {
		hw_dbg(hw, "SW/FW/HW has locked the resource for too long.\n");
		ret_val = -E1000_ERR_CONFIG;
		goto out;
	}

	timeout = SW_FLAG_TIMEOUT;

	extcnf_ctrl |= E1000_EXTCNF_CTRL_SWFLAG;
	ew32(EXTCNF_CTRL, extcnf_ctrl);

	while (timeout) {
		extcnf_ctrl = er32(EXTCNF_CTRL);
		if (extcnf_ctrl & E1000_EXTCNF_CTRL_SWFLAG)
			break;

		mdelay(1);
		timeout--;
	}

	if (!timeout) {
		hw_dbg(hw, "Failed to acquire the semaphore.\n");
		extcnf_ctrl &= ~E1000_EXTCNF_CTRL_SWFLAG;
		ew32(EXTCNF_CTRL, extcnf_ctrl);
		ret_val = -E1000_ERR_CONFIG;
		goto out;
	}

out:
	if (ret_val)
		mutex_unlock(&swflag_mutex);

	return ret_val;
}


static void e1000_release_swflag_ich8lan(struct e1000_hw *hw)
{
	u32 extcnf_ctrl;

	extcnf_ctrl = er32(EXTCNF_CTRL);
	extcnf_ctrl &= ~E1000_EXTCNF_CTRL_SWFLAG;
	ew32(EXTCNF_CTRL, extcnf_ctrl);

	mutex_unlock(&swflag_mutex);

	return;
}


static bool e1000_check_mng_mode_ich8lan(struct e1000_hw *hw)
{
	u32 fwsm = er32(FWSM);

	return (fwsm & E1000_FWSM_MODE_MASK) ==
		(E1000_ICH_MNG_IAMT_MODE << E1000_FWSM_MODE_SHIFT);
}


static s32 e1000_check_reset_block_ich8lan(struct e1000_hw *hw)
{
	u32 fwsm;

	fwsm = er32(FWSM);

	return (fwsm & E1000_ICH_FWSM_RSPCIPHY) ? 0 : E1000_BLK_PHY_RESET;
}


static s32 e1000_phy_force_speed_duplex_ich8lan(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 data;
	bool link;

	if (phy->type != e1000_phy_ife) {
		ret_val = e1000e_phy_force_speed_duplex_igp(hw);
		return ret_val;
	}

	ret_val = e1e_rphy(hw, PHY_CONTROL, &data);
	if (ret_val)
		return ret_val;

	e1000e_phy_force_speed_duplex_setup(hw, &data);

	ret_val = e1e_wphy(hw, PHY_CONTROL, data);
	if (ret_val)
		return ret_val;

	
	ret_val = e1e_rphy(hw, IFE_PHY_MDIX_CONTROL, &data);
	if (ret_val)
		return ret_val;

	data &= ~IFE_PMC_AUTO_MDIX;
	data &= ~IFE_PMC_FORCE_MDIX;

	ret_val = e1e_wphy(hw, IFE_PHY_MDIX_CONTROL, data);
	if (ret_val)
		return ret_val;

	hw_dbg(hw, "IFE PMC: %X\n", data);

	udelay(1);

	if (phy->autoneg_wait_to_complete) {
		hw_dbg(hw, "Waiting for forced speed/duplex link on IFE phy.\n");

		ret_val = e1000e_phy_has_link_generic(hw,
						     PHY_FORCE_LIMIT,
						     100000,
						     &link);
		if (ret_val)
			return ret_val;

		if (!link)
			hw_dbg(hw, "Link taking longer than expected.\n");

		
		ret_val = e1000e_phy_has_link_generic(hw,
						     PHY_FORCE_LIMIT,
						     100000,
						     &link);
		if (ret_val)
			return ret_val;
	}

	return 0;
}


static s32 e1000_sw_lcd_config_ich8lan(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	u32 i, data, cnf_size, cnf_base_addr, sw_cfg_mask;
	s32 ret_val;
	u16 word_addr, reg_data, reg_addr, phy_page = 0;

	ret_val = hw->phy.ops.acquire_phy(hw);
	if (ret_val)
		return ret_val;

	
	if ((hw->mac.type == e1000_ich8lan && phy->type == e1000_phy_igp_3) ||
		(hw->mac.type == e1000_pchlan)) {
		struct e1000_adapter *adapter = hw->adapter;

		
		if ((adapter->pdev->device == E1000_DEV_ID_ICH8_IGP_M_AMT) ||
		    (adapter->pdev->device == E1000_DEV_ID_ICH8_IGP_M) ||
		    (hw->mac.type == e1000_pchlan))
			sw_cfg_mask = E1000_FEXTNVM_SW_CONFIG_ICH8M;
		else
			sw_cfg_mask = E1000_FEXTNVM_SW_CONFIG;

		data = er32(FEXTNVM);
		if (!(data & sw_cfg_mask))
			goto out;

		
		e1000_lan_init_done_ich8lan(hw);

		
		data = er32(EXTCNF_CTRL);
		if (data & E1000_EXTCNF_CTRL_LCD_WRITE_ENABLE)
			goto out;

		cnf_size = er32(EXTCNF_SIZE);
		cnf_size &= E1000_EXTCNF_SIZE_EXT_PCIE_LENGTH_MASK;
		cnf_size >>= E1000_EXTCNF_SIZE_EXT_PCIE_LENGTH_SHIFT;
		if (!cnf_size)
			goto out;

		cnf_base_addr = data & E1000_EXTCNF_CTRL_EXT_CNF_POINTER_MASK;
		cnf_base_addr >>= E1000_EXTCNF_CTRL_EXT_CNF_POINTER_SHIFT;

		if (!(data & E1000_EXTCNF_CTRL_OEM_WRITE_ENABLE) &&
		    (hw->mac.type == e1000_pchlan)) {
			
			data = er32(STRAP);
			data &= E1000_STRAP_SMBUS_ADDRESS_MASK;
			reg_data = data >> E1000_STRAP_SMBUS_ADDRESS_SHIFT;
			reg_data |= HV_SMB_ADDR_PEC_EN | HV_SMB_ADDR_VALID;
			ret_val = e1000_write_phy_reg_hv_locked(hw, HV_SMB_ADDR,
			                                        reg_data);
			if (ret_val)
				goto out;

			data = er32(LEDCTL);
			ret_val = e1000_write_phy_reg_hv_locked(hw,
			                                        HV_LED_CONFIG,
			                                        (u16)data);
			if (ret_val)
				goto out;
		}
		

		
		word_addr = (u16)(cnf_base_addr << 1);

		for (i = 0; i < cnf_size; i++) {
			ret_val = e1000_read_nvm(hw, (word_addr + i * 2), 1,
			                           &reg_data);
			if (ret_val)
				goto out;

			ret_val = e1000_read_nvm(hw, (word_addr + i * 2 + 1),
			                           1, &reg_addr);
			if (ret_val)
				goto out;

			
			if (reg_addr == IGP01E1000_PHY_PAGE_SELECT) {
				phy_page = reg_data;
				continue;
			}

			reg_addr &= PHY_REG_MASK;
			reg_addr |= phy_page;

			ret_val = phy->ops.write_phy_reg_locked(hw,
			                                    (u32)reg_addr,
			                                    reg_data);
			if (ret_val)
				goto out;
		}
	}

out:
	hw->phy.ops.release_phy(hw);
	return ret_val;
}


static s32 e1000_k1_gig_workaround_hv(struct e1000_hw *hw, bool link)
{
	s32 ret_val = 0;
	u16 status_reg = 0;
	bool k1_enable = hw->dev_spec.ich8lan.nvm_k1_enabled;

	if (hw->mac.type != e1000_pchlan)
		goto out;

	
	ret_val = hw->phy.ops.acquire_phy(hw);
	if (ret_val)
		goto out;

	
	if (link) {
		if (hw->phy.type == e1000_phy_82578) {
			ret_val = hw->phy.ops.read_phy_reg_locked(hw,
			                                          BM_CS_STATUS,
			                                          &status_reg);
			if (ret_val)
				goto release;

			status_reg &= BM_CS_STATUS_LINK_UP |
			              BM_CS_STATUS_RESOLVED |
			              BM_CS_STATUS_SPEED_MASK;

			if (status_reg == (BM_CS_STATUS_LINK_UP |
			                   BM_CS_STATUS_RESOLVED |
			                   BM_CS_STATUS_SPEED_1000))
				k1_enable = false;
		}

		if (hw->phy.type == e1000_phy_82577) {
			ret_val = hw->phy.ops.read_phy_reg_locked(hw,
			                                          HV_M_STATUS,
			                                          &status_reg);
			if (ret_val)
				goto release;

			status_reg &= HV_M_STATUS_LINK_UP |
			              HV_M_STATUS_AUTONEG_COMPLETE |
			              HV_M_STATUS_SPEED_MASK;

			if (status_reg == (HV_M_STATUS_LINK_UP |
			                   HV_M_STATUS_AUTONEG_COMPLETE |
			                   HV_M_STATUS_SPEED_1000))
				k1_enable = false;
		}

		
		ret_val = hw->phy.ops.write_phy_reg_locked(hw, PHY_REG(770, 19),
		                                           0x0100);
		if (ret_val)
			goto release;

	} else {
		
		ret_val = hw->phy.ops.write_phy_reg_locked(hw, PHY_REG(770, 19),
		                                           0x4100);
		if (ret_val)
			goto release;
	}

	ret_val = e1000_configure_k1_ich8lan(hw, k1_enable);

release:
	hw->phy.ops.release_phy(hw);
out:
	return ret_val;
}


static s32 e1000_configure_k1_ich8lan(struct e1000_hw *hw, bool k1_enable)
{
	s32 ret_val = 0;
	u32 ctrl_reg = 0;
	u32 ctrl_ext = 0;
	u32 reg = 0;
	u16 kmrn_reg = 0;

	ret_val = e1000e_read_kmrn_reg_locked(hw,
	                                     E1000_KMRNCTRLSTA_K1_CONFIG,
	                                     &kmrn_reg);
	if (ret_val)
		goto out;

	if (k1_enable)
		kmrn_reg |= E1000_KMRNCTRLSTA_K1_ENABLE;
	else
		kmrn_reg &= ~E1000_KMRNCTRLSTA_K1_ENABLE;

	ret_val = e1000e_write_kmrn_reg_locked(hw,
	                                      E1000_KMRNCTRLSTA_K1_CONFIG,
	                                      kmrn_reg);
	if (ret_val)
		goto out;

	udelay(20);
	ctrl_ext = er32(CTRL_EXT);
	ctrl_reg = er32(CTRL);

	reg = ctrl_reg & ~(E1000_CTRL_SPD_1000 | E1000_CTRL_SPD_100);
	reg |= E1000_CTRL_FRCSPD;
	ew32(CTRL, reg);

	ew32(CTRL_EXT, ctrl_ext | E1000_CTRL_EXT_SPD_BYPS);
	udelay(20);
	ew32(CTRL, ctrl_reg);
	ew32(CTRL_EXT, ctrl_ext);
	udelay(20);

out:
	return ret_val;
}


static s32 e1000_oem_bits_config_ich8lan(struct e1000_hw *hw, bool d0_state)
{
	s32 ret_val = 0;
	u32 mac_reg;
	u16 oem_reg;

	if (hw->mac.type != e1000_pchlan)
		return ret_val;

	ret_val = hw->phy.ops.acquire_phy(hw);
	if (ret_val)
		return ret_val;

	mac_reg = er32(EXTCNF_CTRL);
	if (mac_reg & E1000_EXTCNF_CTRL_OEM_WRITE_ENABLE)
		goto out;

	mac_reg = er32(FEXTNVM);
	if (!(mac_reg & E1000_FEXTNVM_SW_CONFIG_ICH8M))
		goto out;

	mac_reg = er32(PHY_CTRL);

	ret_val = hw->phy.ops.read_phy_reg_locked(hw, HV_OEM_BITS, &oem_reg);
	if (ret_val)
		goto out;

	oem_reg &= ~(HV_OEM_BITS_GBE_DIS | HV_OEM_BITS_LPLU);

	if (d0_state) {
		if (mac_reg & E1000_PHY_CTRL_GBE_DISABLE)
			oem_reg |= HV_OEM_BITS_GBE_DIS;

		if (mac_reg & E1000_PHY_CTRL_D0A_LPLU)
			oem_reg |= HV_OEM_BITS_LPLU;
	} else {
		if (mac_reg & E1000_PHY_CTRL_NOND0A_GBE_DISABLE)
			oem_reg |= HV_OEM_BITS_GBE_DIS;

		if (mac_reg & E1000_PHY_CTRL_NOND0A_LPLU)
			oem_reg |= HV_OEM_BITS_LPLU;
	}
	
	if (!e1000_check_reset_block(hw))
		oem_reg |= HV_OEM_BITS_RESTART_AN;
	ret_val = hw->phy.ops.write_phy_reg_locked(hw, HV_OEM_BITS, oem_reg);

out:
	hw->phy.ops.release_phy(hw);

	return ret_val;
}



static s32 e1000_hv_phy_workarounds_ich8lan(struct e1000_hw *hw)
{
	s32 ret_val = 0;

	if (hw->mac.type != e1000_pchlan)
		return ret_val;

	if (((hw->phy.type == e1000_phy_82577) &&
	     ((hw->phy.revision == 1) || (hw->phy.revision == 2))) ||
	    ((hw->phy.type == e1000_phy_82578) && (hw->phy.revision == 1))) {
		
		ret_val = e1e_wphy(hw, PHY_REG(769, 25), 0x4431);
		if (ret_val)
			return ret_val;

		
		ret_val = e1e_wphy(hw, PHY_REG(770, 16), 0xA204);
		if (ret_val)
			return ret_val;
	}

	if (hw->phy.type == e1000_phy_82578) {
		
		if (hw->phy.revision < 2) {
			e1000e_phy_sw_reset(hw);
			ret_val = e1e_wphy(hw, PHY_CONTROL, 0x3140);
		}
	}

	
	ret_val = hw->phy.ops.acquire_phy(hw);
	if (ret_val)
		return ret_val;

	hw->phy.addr = 1;
	ret_val = e1000e_write_phy_reg_mdic(hw, IGP01E1000_PHY_PAGE_SELECT, 0);
	if (ret_val)
		goto out;
	hw->phy.ops.release_phy(hw);

	
	ret_val = e1000_k1_gig_workaround_hv(hw, true);

out:
	return ret_val;
}


static void e1000_lan_init_done_ich8lan(struct e1000_hw *hw)
{
	u32 data, loop = E1000_ICH8_LAN_INIT_TIMEOUT;

	
	do {
		data = er32(STATUS);
		data &= E1000_STATUS_LAN_INIT_DONE;
		udelay(100);
	} while ((!data) && --loop);

	
	if (loop == 0)
		hw_dbg(hw, "LAN_INIT_DONE not set, increase timeout\n");

	
	data = er32(STATUS);
	data &= ~E1000_STATUS_LAN_INIT_DONE;
	ew32(STATUS, data);
}


static s32 e1000_phy_hw_reset_ich8lan(struct e1000_hw *hw)
{
	s32 ret_val = 0;
	u16 reg;

	ret_val = e1000e_phy_hw_reset_generic(hw);
	if (ret_val)
		return ret_val;

	
	mdelay(10);

	if (hw->mac.type == e1000_pchlan) {
		ret_val = e1000_hv_phy_workarounds_ich8lan(hw);
		if (ret_val)
			return ret_val;
	}

	
	if (hw->mac.type == e1000_pchlan)
		e1e_rphy(hw, BM_WUC, &reg);

	
	ret_val = e1000_sw_lcd_config_ich8lan(hw);
	if (ret_val)
		goto out;

	
	if (hw->mac.type == e1000_pchlan)
		ret_val = e1000_oem_bits_config_ich8lan(hw, true);

out:
	return 0;
}


static s32 e1000_get_phy_info_ife_ich8lan(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 data;
	bool link;

	ret_val = e1000e_phy_has_link_generic(hw, 1, 0, &link);
	if (ret_val)
		return ret_val;

	if (!link) {
		hw_dbg(hw, "Phy info is only valid if link is up\n");
		return -E1000_ERR_CONFIG;
	}

	ret_val = e1e_rphy(hw, IFE_PHY_SPECIAL_CONTROL, &data);
	if (ret_val)
		return ret_val;
	phy->polarity_correction = (!(data & IFE_PSC_AUTO_POLARITY_DISABLE));

	if (phy->polarity_correction) {
		ret_val = phy->ops.check_polarity(hw);
		if (ret_val)
			return ret_val;
	} else {
		
		phy->cable_polarity = (data & IFE_PSC_FORCE_POLARITY)
				      ? e1000_rev_polarity_reversed
				      : e1000_rev_polarity_normal;
	}

	ret_val = e1e_rphy(hw, IFE_PHY_MDIX_CONTROL, &data);
	if (ret_val)
		return ret_val;

	phy->is_mdix = (data & IFE_PMC_MDIX_STATUS);

	
	phy->cable_length = E1000_CABLE_LENGTH_UNDEFINED;
	phy->local_rx = e1000_1000t_rx_status_undefined;
	phy->remote_rx = e1000_1000t_rx_status_undefined;

	return 0;
}


static s32 e1000_get_phy_info_ich8lan(struct e1000_hw *hw)
{
	switch (hw->phy.type) {
	case e1000_phy_ife:
		return e1000_get_phy_info_ife_ich8lan(hw);
		break;
	case e1000_phy_igp_3:
	case e1000_phy_bm:
	case e1000_phy_82578:
	case e1000_phy_82577:
		return e1000e_get_phy_info_igp(hw);
		break;
	default:
		break;
	}

	return -E1000_ERR_PHY_TYPE;
}


static s32 e1000_check_polarity_ife_ich8lan(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 phy_data, offset, mask;

	
	if (phy->polarity_correction) {
		offset	= IFE_PHY_EXTENDED_STATUS_CONTROL;
		mask	= IFE_PESC_POLARITY_REVERSED;
	} else {
		offset	= IFE_PHY_SPECIAL_CONTROL;
		mask	= IFE_PSC_FORCE_POLARITY;
	}

	ret_val = e1e_rphy(hw, offset, &phy_data);

	if (!ret_val)
		phy->cable_polarity = (phy_data & mask)
				      ? e1000_rev_polarity_reversed
				      : e1000_rev_polarity_normal;

	return ret_val;
}


static s32 e1000_set_lplu_state_pchlan(struct e1000_hw *hw, bool active)
{
	s32 ret_val = 0;
	u16 oem_reg;

	ret_val = e1e_rphy(hw, HV_OEM_BITS, &oem_reg);
	if (ret_val)
		goto out;

	if (active)
		oem_reg |= HV_OEM_BITS_LPLU;
	else
		oem_reg &= ~HV_OEM_BITS_LPLU;

	oem_reg |= HV_OEM_BITS_RESTART_AN;
	ret_val = e1e_wphy(hw, HV_OEM_BITS, oem_reg);

out:
	return ret_val;
}


static s32 e1000_set_d0_lplu_state_ich8lan(struct e1000_hw *hw, bool active)
{
	struct e1000_phy_info *phy = &hw->phy;
	u32 phy_ctrl;
	s32 ret_val = 0;
	u16 data;

	if (phy->type == e1000_phy_ife)
		return ret_val;

	phy_ctrl = er32(PHY_CTRL);

	if (active) {
		phy_ctrl |= E1000_PHY_CTRL_D0A_LPLU;
		ew32(PHY_CTRL, phy_ctrl);

		if (phy->type != e1000_phy_igp_3)
			return 0;

		
		if (hw->mac.type == e1000_ich8lan)
			e1000e_gig_downshift_workaround_ich8lan(hw);

		
		ret_val = e1e_rphy(hw, IGP01E1000_PHY_PORT_CONFIG, &data);
		data &= ~IGP01E1000_PSCFR_SMART_SPEED;
		ret_val = e1e_wphy(hw, IGP01E1000_PHY_PORT_CONFIG, data);
		if (ret_val)
			return ret_val;
	} else {
		phy_ctrl &= ~E1000_PHY_CTRL_D0A_LPLU;
		ew32(PHY_CTRL, phy_ctrl);

		if (phy->type != e1000_phy_igp_3)
			return 0;

		
		if (phy->smart_speed == e1000_smart_speed_on) {
			ret_val = e1e_rphy(hw, IGP01E1000_PHY_PORT_CONFIG,
					   &data);
			if (ret_val)
				return ret_val;

			data |= IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = e1e_wphy(hw, IGP01E1000_PHY_PORT_CONFIG,
					   data);
			if (ret_val)
				return ret_val;
		} else if (phy->smart_speed == e1000_smart_speed_off) {
			ret_val = e1e_rphy(hw, IGP01E1000_PHY_PORT_CONFIG,
					   &data);
			if (ret_val)
				return ret_val;

			data &= ~IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = e1e_wphy(hw, IGP01E1000_PHY_PORT_CONFIG,
					   data);
			if (ret_val)
				return ret_val;
		}
	}

	return 0;
}


static s32 e1000_set_d3_lplu_state_ich8lan(struct e1000_hw *hw, bool active)
{
	struct e1000_phy_info *phy = &hw->phy;
	u32 phy_ctrl;
	s32 ret_val;
	u16 data;

	phy_ctrl = er32(PHY_CTRL);

	if (!active) {
		phy_ctrl &= ~E1000_PHY_CTRL_NOND0A_LPLU;
		ew32(PHY_CTRL, phy_ctrl);

		if (phy->type != e1000_phy_igp_3)
			return 0;

		
		if (phy->smart_speed == e1000_smart_speed_on) {
			ret_val = e1e_rphy(hw, IGP01E1000_PHY_PORT_CONFIG,
					   &data);
			if (ret_val)
				return ret_val;

			data |= IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = e1e_wphy(hw, IGP01E1000_PHY_PORT_CONFIG,
					   data);
			if (ret_val)
				return ret_val;
		} else if (phy->smart_speed == e1000_smart_speed_off) {
			ret_val = e1e_rphy(hw, IGP01E1000_PHY_PORT_CONFIG,
					   &data);
			if (ret_val)
				return ret_val;

			data &= ~IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = e1e_wphy(hw, IGP01E1000_PHY_PORT_CONFIG,
					   data);
			if (ret_val)
				return ret_val;
		}
	} else if ((phy->autoneg_advertised == E1000_ALL_SPEED_DUPLEX) ||
		   (phy->autoneg_advertised == E1000_ALL_NOT_GIG) ||
		   (phy->autoneg_advertised == E1000_ALL_10_SPEED)) {
		phy_ctrl |= E1000_PHY_CTRL_NOND0A_LPLU;
		ew32(PHY_CTRL, phy_ctrl);

		if (phy->type != e1000_phy_igp_3)
			return 0;

		
		if (hw->mac.type == e1000_ich8lan)
			e1000e_gig_downshift_workaround_ich8lan(hw);

		
		ret_val = e1e_rphy(hw, IGP01E1000_PHY_PORT_CONFIG, &data);
		if (ret_val)
			return ret_val;

		data &= ~IGP01E1000_PSCFR_SMART_SPEED;
		ret_val = e1e_wphy(hw, IGP01E1000_PHY_PORT_CONFIG, data);
	}

	return 0;
}


static s32 e1000_valid_nvm_bank_detect_ich8lan(struct e1000_hw *hw, u32 *bank)
{
	u32 eecd;
	struct e1000_nvm_info *nvm = &hw->nvm;
	u32 bank1_offset = nvm->flash_bank_size * sizeof(u16);
	u32 act_offset = E1000_ICH_NVM_SIG_WORD * 2 + 1;
	u8 sig_byte = 0;
	s32 ret_val = 0;

	switch (hw->mac.type) {
	case e1000_ich8lan:
	case e1000_ich9lan:
		eecd = er32(EECD);
		if ((eecd & E1000_EECD_SEC1VAL_VALID_MASK) ==
		    E1000_EECD_SEC1VAL_VALID_MASK) {
			if (eecd & E1000_EECD_SEC1VAL)
				*bank = 1;
			else
				*bank = 0;

			return 0;
		}
		hw_dbg(hw, "Unable to determine valid NVM bank via EEC - "
		       "reading flash signature\n");
		
	default:
		
		*bank = 0;

		
		ret_val = e1000_read_flash_byte_ich8lan(hw, act_offset,
		                                        &sig_byte);
		if (ret_val)
			return ret_val;
		if ((sig_byte & E1000_ICH_NVM_VALID_SIG_MASK) ==
		    E1000_ICH_NVM_SIG_VALUE) {
			*bank = 0;
			return 0;
		}

		
		ret_val = e1000_read_flash_byte_ich8lan(hw, act_offset +
		                                        bank1_offset,
		                                        &sig_byte);
		if (ret_val)
			return ret_val;
		if ((sig_byte & E1000_ICH_NVM_VALID_SIG_MASK) ==
		    E1000_ICH_NVM_SIG_VALUE) {
			*bank = 1;
			return 0;
		}

		hw_dbg(hw, "ERROR: No valid NVM bank present\n");
		return -E1000_ERR_NVM;
	}

	return 0;
}


static s32 e1000_read_nvm_ich8lan(struct e1000_hw *hw, u16 offset, u16 words,
				  u16 *data)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	struct e1000_dev_spec_ich8lan *dev_spec = &hw->dev_spec.ich8lan;
	u32 act_offset;
	s32 ret_val = 0;
	u32 bank = 0;
	u16 i, word;

	if ((offset >= nvm->word_size) || (words > nvm->word_size - offset) ||
	    (words == 0)) {
		hw_dbg(hw, "nvm parameter(s) out of bounds\n");
		ret_val = -E1000_ERR_NVM;
		goto out;
	}

	nvm->ops.acquire_nvm(hw);

	ret_val = e1000_valid_nvm_bank_detect_ich8lan(hw, &bank);
	if (ret_val) {
		hw_dbg(hw, "Could not detect valid bank, assuming bank 0\n");
		bank = 0;
	}

	act_offset = (bank) ? nvm->flash_bank_size : 0;
	act_offset += offset;

	ret_val = 0;
	for (i = 0; i < words; i++) {
		if ((dev_spec->shadow_ram) &&
		    (dev_spec->shadow_ram[offset+i].modified)) {
			data[i] = dev_spec->shadow_ram[offset+i].value;
		} else {
			ret_val = e1000_read_flash_word_ich8lan(hw,
								act_offset + i,
								&word);
			if (ret_val)
				break;
			data[i] = word;
		}
	}

	nvm->ops.release_nvm(hw);

out:
	if (ret_val)
		hw_dbg(hw, "NVM read error: %d\n", ret_val);

	return ret_val;
}


static s32 e1000_flash_cycle_init_ich8lan(struct e1000_hw *hw)
{
	union ich8_hws_flash_status hsfsts;
	s32 ret_val = -E1000_ERR_NVM;
	s32 i = 0;

	hsfsts.regval = er16flash(ICH_FLASH_HSFSTS);

	
	if (hsfsts.hsf_status.fldesvalid == 0) {
		hw_dbg(hw, "Flash descriptor invalid.  "
			 "SW Sequencing must be used.");
		return -E1000_ERR_NVM;
	}

	
	hsfsts.hsf_status.flcerr = 1;
	hsfsts.hsf_status.dael = 1;

	ew16flash(ICH_FLASH_HSFSTS, hsfsts.regval);

	

	if (hsfsts.hsf_status.flcinprog == 0) {
		
		hsfsts.hsf_status.flcdone = 1;
		ew16flash(ICH_FLASH_HSFSTS, hsfsts.regval);
		ret_val = 0;
	} else {
		
		for (i = 0; i < ICH_FLASH_READ_COMMAND_TIMEOUT; i++) {
			hsfsts.regval = __er16flash(hw, ICH_FLASH_HSFSTS);
			if (hsfsts.hsf_status.flcinprog == 0) {
				ret_val = 0;
				break;
			}
			udelay(1);
		}
		if (ret_val == 0) {
			
			hsfsts.hsf_status.flcdone = 1;
			ew16flash(ICH_FLASH_HSFSTS, hsfsts.regval);
		} else {
			hw_dbg(hw, "Flash controller busy, cannot get access");
		}
	}

	return ret_val;
}


static s32 e1000_flash_cycle_ich8lan(struct e1000_hw *hw, u32 timeout)
{
	union ich8_hws_flash_ctrl hsflctl;
	union ich8_hws_flash_status hsfsts;
	s32 ret_val = -E1000_ERR_NVM;
	u32 i = 0;

	
	hsflctl.regval = er16flash(ICH_FLASH_HSFCTL);
	hsflctl.hsf_ctrl.flcgo = 1;
	ew16flash(ICH_FLASH_HSFCTL, hsflctl.regval);

	
	do {
		hsfsts.regval = er16flash(ICH_FLASH_HSFSTS);
		if (hsfsts.hsf_status.flcdone == 1)
			break;
		udelay(1);
	} while (i++ < timeout);

	if (hsfsts.hsf_status.flcdone == 1 && hsfsts.hsf_status.flcerr == 0)
		return 0;

	return ret_val;
}


static s32 e1000_read_flash_word_ich8lan(struct e1000_hw *hw, u32 offset,
					 u16 *data)
{
	
	offset <<= 1;

	return e1000_read_flash_data_ich8lan(hw, offset, 2, data);
}


static s32 e1000_read_flash_byte_ich8lan(struct e1000_hw *hw, u32 offset,
					 u8 *data)
{
	s32 ret_val;
	u16 word = 0;

	ret_val = e1000_read_flash_data_ich8lan(hw, offset, 1, &word);
	if (ret_val)
		return ret_val;

	*data = (u8)word;

	return 0;
}


static s32 e1000_read_flash_data_ich8lan(struct e1000_hw *hw, u32 offset,
					 u8 size, u16 *data)
{
	union ich8_hws_flash_status hsfsts;
	union ich8_hws_flash_ctrl hsflctl;
	u32 flash_linear_addr;
	u32 flash_data = 0;
	s32 ret_val = -E1000_ERR_NVM;
	u8 count = 0;

	if (size < 1  || size > 2 || offset > ICH_FLASH_LINEAR_ADDR_MASK)
		return -E1000_ERR_NVM;

	flash_linear_addr = (ICH_FLASH_LINEAR_ADDR_MASK & offset) +
			    hw->nvm.flash_base_addr;

	do {
		udelay(1);
		
		ret_val = e1000_flash_cycle_init_ich8lan(hw);
		if (ret_val != 0)
			break;

		hsflctl.regval = er16flash(ICH_FLASH_HSFCTL);
		
		hsflctl.hsf_ctrl.fldbcount = size - 1;
		hsflctl.hsf_ctrl.flcycle = ICH_CYCLE_READ;
		ew16flash(ICH_FLASH_HSFCTL, hsflctl.regval);

		ew32flash(ICH_FLASH_FADDR, flash_linear_addr);

		ret_val = e1000_flash_cycle_ich8lan(hw,
						ICH_FLASH_READ_COMMAND_TIMEOUT);

		
		if (ret_val == 0) {
			flash_data = er32flash(ICH_FLASH_FDATA0);
			if (size == 1) {
				*data = (u8)(flash_data & 0x000000FF);
			} else if (size == 2) {
				*data = (u16)(flash_data & 0x0000FFFF);
			}
			break;
		} else {
			
			hsfsts.regval = er16flash(ICH_FLASH_HSFSTS);
			if (hsfsts.hsf_status.flcerr == 1) {
				
				continue;
			} else if (hsfsts.hsf_status.flcdone == 0) {
				hw_dbg(hw, "Timeout error - flash cycle "
					 "did not complete.");
				break;
			}
		}
	} while (count++ < ICH_FLASH_CYCLE_REPEAT_COUNT);

	return ret_val;
}


static s32 e1000_write_nvm_ich8lan(struct e1000_hw *hw, u16 offset, u16 words,
				   u16 *data)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	struct e1000_dev_spec_ich8lan *dev_spec = &hw->dev_spec.ich8lan;
	u16 i;

	if ((offset >= nvm->word_size) || (words > nvm->word_size - offset) ||
	    (words == 0)) {
		hw_dbg(hw, "nvm parameter(s) out of bounds\n");
		return -E1000_ERR_NVM;
	}

	nvm->ops.acquire_nvm(hw);

	for (i = 0; i < words; i++) {
		dev_spec->shadow_ram[offset+i].modified = 1;
		dev_spec->shadow_ram[offset+i].value = data[i];
	}

	nvm->ops.release_nvm(hw);

	return 0;
}


static s32 e1000_update_nvm_checksum_ich8lan(struct e1000_hw *hw)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	struct e1000_dev_spec_ich8lan *dev_spec = &hw->dev_spec.ich8lan;
	u32 i, act_offset, new_bank_offset, old_bank_offset, bank;
	s32 ret_val;
	u16 data;

	ret_val = e1000e_update_nvm_checksum_generic(hw);
	if (ret_val)
		goto out;

	if (nvm->type != e1000_nvm_flash_sw)
		goto out;

	nvm->ops.acquire_nvm(hw);

	
	ret_val =  e1000_valid_nvm_bank_detect_ich8lan(hw, &bank);
	if (ret_val) {
		hw_dbg(hw, "Could not detect valid bank, assuming bank 0\n");
		bank = 0;
	}

	if (bank == 0) {
		new_bank_offset = nvm->flash_bank_size;
		old_bank_offset = 0;
		ret_val = e1000_erase_flash_bank_ich8lan(hw, 1);
		if (ret_val) {
			nvm->ops.release_nvm(hw);
			goto out;
		}
	} else {
		old_bank_offset = nvm->flash_bank_size;
		new_bank_offset = 0;
		ret_val = e1000_erase_flash_bank_ich8lan(hw, 0);
		if (ret_val) {
			nvm->ops.release_nvm(hw);
			goto out;
		}
	}

	for (i = 0; i < E1000_ICH8_SHADOW_RAM_WORDS; i++) {
		
		if (dev_spec->shadow_ram[i].modified) {
			data = dev_spec->shadow_ram[i].value;
		} else {
			ret_val = e1000_read_flash_word_ich8lan(hw, i +
			                                        old_bank_offset,
			                                        &data);
			if (ret_val)
				break;
		}

		
		if (i == E1000_ICH_NVM_SIG_WORD)
			data |= E1000_ICH_NVM_SIG_MASK;

		
		act_offset = (i + new_bank_offset) << 1;

		udelay(100);
		
		ret_val = e1000_retry_write_flash_byte_ich8lan(hw,
							       act_offset,
							       (u8)data);
		if (ret_val)
			break;

		udelay(100);
		ret_val = e1000_retry_write_flash_byte_ich8lan(hw,
							  act_offset + 1,
							  (u8)(data >> 8));
		if (ret_val)
			break;
	}

	
	if (ret_val) {
		
		hw_dbg(hw, "Flash commit failed.\n");
		nvm->ops.release_nvm(hw);
		goto out;
	}

	
	act_offset = new_bank_offset + E1000_ICH_NVM_SIG_WORD;
	ret_val = e1000_read_flash_word_ich8lan(hw, act_offset, &data);
	if (ret_val) {
		nvm->ops.release_nvm(hw);
		goto out;
	}
	data &= 0xBFFF;
	ret_val = e1000_retry_write_flash_byte_ich8lan(hw,
						       act_offset * 2 + 1,
						       (u8)(data >> 8));
	if (ret_val) {
		nvm->ops.release_nvm(hw);
		goto out;
	}

	
	act_offset = (old_bank_offset + E1000_ICH_NVM_SIG_WORD) * 2 + 1;
	ret_val = e1000_retry_write_flash_byte_ich8lan(hw, act_offset, 0);
	if (ret_val) {
		nvm->ops.release_nvm(hw);
		goto out;
	}

	
	for (i = 0; i < E1000_ICH8_SHADOW_RAM_WORDS; i++) {
		dev_spec->shadow_ram[i].modified = 0;
		dev_spec->shadow_ram[i].value = 0xFFFF;
	}

	nvm->ops.release_nvm(hw);

	
	e1000e_reload_nvm(hw);
	msleep(10);

out:
	if (ret_val)
		hw_dbg(hw, "NVM update error: %d\n", ret_val);

	return ret_val;
}


static s32 e1000_validate_nvm_checksum_ich8lan(struct e1000_hw *hw)
{
	s32 ret_val;
	u16 data;

	
	ret_val = e1000_read_nvm(hw, 0x19, 1, &data);
	if (ret_val)
		return ret_val;

	if ((data & 0x40) == 0) {
		data |= 0x40;
		ret_val = e1000_write_nvm(hw, 0x19, 1, &data);
		if (ret_val)
			return ret_val;
		ret_val = e1000e_update_nvm_checksum(hw);
		if (ret_val)
			return ret_val;
	}

	return e1000e_validate_nvm_checksum_generic(hw);
}


void e1000e_write_protect_nvm_ich8lan(struct e1000_hw *hw)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	union ich8_flash_protected_range pr0;
	union ich8_hws_flash_status hsfsts;
	u32 gfpreg;

	nvm->ops.acquire_nvm(hw);

	gfpreg = er32flash(ICH_FLASH_GFPREG);

	
	pr0.regval = er32flash(ICH_FLASH_PR0);
	pr0.range.base = gfpreg & FLASH_GFPREG_BASE_MASK;
	pr0.range.limit = ((gfpreg >> 16) & FLASH_GFPREG_BASE_MASK);
	pr0.range.wpe = true;
	ew32flash(ICH_FLASH_PR0, pr0.regval);

	
	hsfsts.regval = er16flash(ICH_FLASH_HSFSTS);
	hsfsts.hsf_status.flockdn = true;
	ew32flash(ICH_FLASH_HSFSTS, hsfsts.regval);

	nvm->ops.release_nvm(hw);
}


static s32 e1000_write_flash_data_ich8lan(struct e1000_hw *hw, u32 offset,
					  u8 size, u16 data)
{
	union ich8_hws_flash_status hsfsts;
	union ich8_hws_flash_ctrl hsflctl;
	u32 flash_linear_addr;
	u32 flash_data = 0;
	s32 ret_val;
	u8 count = 0;

	if (size < 1 || size > 2 || data > size * 0xff ||
	    offset > ICH_FLASH_LINEAR_ADDR_MASK)
		return -E1000_ERR_NVM;

	flash_linear_addr = (ICH_FLASH_LINEAR_ADDR_MASK & offset) +
			    hw->nvm.flash_base_addr;

	do {
		udelay(1);
		
		ret_val = e1000_flash_cycle_init_ich8lan(hw);
		if (ret_val)
			break;

		hsflctl.regval = er16flash(ICH_FLASH_HSFCTL);
		
		hsflctl.hsf_ctrl.fldbcount = size -1;
		hsflctl.hsf_ctrl.flcycle = ICH_CYCLE_WRITE;
		ew16flash(ICH_FLASH_HSFCTL, hsflctl.regval);

		ew32flash(ICH_FLASH_FADDR, flash_linear_addr);

		if (size == 1)
			flash_data = (u32)data & 0x00FF;
		else
			flash_data = (u32)data;

		ew32flash(ICH_FLASH_FDATA0, flash_data);

		
		ret_val = e1000_flash_cycle_ich8lan(hw,
					       ICH_FLASH_WRITE_COMMAND_TIMEOUT);
		if (!ret_val)
			break;

		
		hsfsts.regval = er16flash(ICH_FLASH_HSFSTS);
		if (hsfsts.hsf_status.flcerr == 1)
			
			continue;
		if (hsfsts.hsf_status.flcdone == 0) {
			hw_dbg(hw, "Timeout error - flash cycle "
				 "did not complete.");
			break;
		}
	} while (count++ < ICH_FLASH_CYCLE_REPEAT_COUNT);

	return ret_val;
}


static s32 e1000_write_flash_byte_ich8lan(struct e1000_hw *hw, u32 offset,
					  u8 data)
{
	u16 word = (u16)data;

	return e1000_write_flash_data_ich8lan(hw, offset, 1, word);
}


static s32 e1000_retry_write_flash_byte_ich8lan(struct e1000_hw *hw,
						u32 offset, u8 byte)
{
	s32 ret_val;
	u16 program_retries;

	ret_val = e1000_write_flash_byte_ich8lan(hw, offset, byte);
	if (!ret_val)
		return ret_val;

	for (program_retries = 0; program_retries < 100; program_retries++) {
		hw_dbg(hw, "Retrying Byte %2.2X at offset %u\n", byte, offset);
		udelay(100);
		ret_val = e1000_write_flash_byte_ich8lan(hw, offset, byte);
		if (!ret_val)
			break;
	}
	if (program_retries == 100)
		return -E1000_ERR_NVM;

	return 0;
}


static s32 e1000_erase_flash_bank_ich8lan(struct e1000_hw *hw, u32 bank)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	union ich8_hws_flash_status hsfsts;
	union ich8_hws_flash_ctrl hsflctl;
	u32 flash_linear_addr;
	
	u32 flash_bank_size = nvm->flash_bank_size * 2;
	s32 ret_val;
	s32 count = 0;
	s32 iteration;
	s32 sector_size;
	s32 j;

	hsfsts.regval = er16flash(ICH_FLASH_HSFSTS);

	
	switch (hsfsts.hsf_status.berasesz) {
	case 0:
		
		sector_size = ICH_FLASH_SEG_SIZE_256;
		iteration = flash_bank_size / ICH_FLASH_SEG_SIZE_256;
		break;
	case 1:
		sector_size = ICH_FLASH_SEG_SIZE_4K;
		iteration = 1;
		break;
	case 2:
		sector_size = ICH_FLASH_SEG_SIZE_8K;
		iteration = 1;
		break;
	case 3:
		sector_size = ICH_FLASH_SEG_SIZE_64K;
		iteration = 1;
		break;
	default:
		return -E1000_ERR_NVM;
	}

	
	flash_linear_addr = hw->nvm.flash_base_addr;
	flash_linear_addr += (bank) ? flash_bank_size : 0;

	for (j = 0; j < iteration ; j++) {
		do {
			
			ret_val = e1000_flash_cycle_init_ich8lan(hw);
			if (ret_val)
				return ret_val;

			
			hsflctl.regval = er16flash(ICH_FLASH_HSFCTL);
			hsflctl.hsf_ctrl.flcycle = ICH_CYCLE_ERASE;
			ew16flash(ICH_FLASH_HSFCTL, hsflctl.regval);

			
			flash_linear_addr += (j * sector_size);
			ew32flash(ICH_FLASH_FADDR, flash_linear_addr);

			ret_val = e1000_flash_cycle_ich8lan(hw,
					       ICH_FLASH_ERASE_COMMAND_TIMEOUT);
			if (ret_val == 0)
				break;

			
			hsfsts.regval = er16flash(ICH_FLASH_HSFSTS);
			if (hsfsts.hsf_status.flcerr == 1)
				
				continue;
			else if (hsfsts.hsf_status.flcdone == 0)
				return ret_val;
		} while (++count < ICH_FLASH_CYCLE_REPEAT_COUNT);
	}

	return 0;
}


static s32 e1000_valid_led_default_ich8lan(struct e1000_hw *hw, u16 *data)
{
	s32 ret_val;

	ret_val = e1000_read_nvm(hw, NVM_ID_LED_SETTINGS, 1, data);
	if (ret_val) {
		hw_dbg(hw, "NVM Read Error\n");
		return ret_val;
	}

	if (*data == ID_LED_RESERVED_0000 ||
	    *data == ID_LED_RESERVED_FFFF)
		*data = ID_LED_DEFAULT_ICH8LAN;

	return 0;
}


static s32 e1000_id_led_init_pchlan(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val;
	const u32 ledctl_on = E1000_LEDCTL_MODE_LINK_UP;
	const u32 ledctl_off = E1000_LEDCTL_MODE_LINK_UP | E1000_PHY_LED0_IVRT;
	u16 data, i, temp, shift;

	
	ret_val = hw->nvm.ops.valid_led_default(hw, &data);
	if (ret_val)
		goto out;

	mac->ledctl_default = er32(LEDCTL);
	mac->ledctl_mode1 = mac->ledctl_default;
	mac->ledctl_mode2 = mac->ledctl_default;

	for (i = 0; i < 4; i++) {
		temp = (data >> (i << 2)) & E1000_LEDCTL_LED0_MODE_MASK;
		shift = (i * 5);
		switch (temp) {
		case ID_LED_ON1_DEF2:
		case ID_LED_ON1_ON2:
		case ID_LED_ON1_OFF2:
			mac->ledctl_mode1 &= ~(E1000_PHY_LED0_MASK << shift);
			mac->ledctl_mode1 |= (ledctl_on << shift);
			break;
		case ID_LED_OFF1_DEF2:
		case ID_LED_OFF1_ON2:
		case ID_LED_OFF1_OFF2:
			mac->ledctl_mode1 &= ~(E1000_PHY_LED0_MASK << shift);
			mac->ledctl_mode1 |= (ledctl_off << shift);
			break;
		default:
			
			break;
		}
		switch (temp) {
		case ID_LED_DEF1_ON2:
		case ID_LED_ON1_ON2:
		case ID_LED_OFF1_ON2:
			mac->ledctl_mode2 &= ~(E1000_PHY_LED0_MASK << shift);
			mac->ledctl_mode2 |= (ledctl_on << shift);
			break;
		case ID_LED_DEF1_OFF2:
		case ID_LED_ON1_OFF2:
		case ID_LED_OFF1_OFF2:
			mac->ledctl_mode2 &= ~(E1000_PHY_LED0_MASK << shift);
			mac->ledctl_mode2 |= (ledctl_off << shift);
			break;
		default:
			
			break;
		}
	}

out:
	return ret_val;
}


static s32 e1000_get_bus_info_ich8lan(struct e1000_hw *hw)
{
	struct e1000_bus_info *bus = &hw->bus;
	s32 ret_val;

	ret_val = e1000e_get_bus_info_pcie(hw);

	
	if (bus->width == e1000_bus_width_unknown)
		bus->width = e1000_bus_width_pcie_x1;

	return ret_val;
}


static s32 e1000_reset_hw_ich8lan(struct e1000_hw *hw)
{
	struct e1000_dev_spec_ich8lan *dev_spec = &hw->dev_spec.ich8lan;
	u16 reg;
	u32 ctrl, icr, kab;
	s32 ret_val;

	
	ret_val = e1000e_disable_pcie_master(hw);
	if (ret_val) {
		hw_dbg(hw, "PCI-E Master disable polling has failed.\n");
	}

	hw_dbg(hw, "Masking off all interrupts\n");
	ew32(IMC, 0xffffffff);

	
	ew32(RCTL, 0);
	ew32(TCTL, E1000_TCTL_PSP);
	e1e_flush();

	msleep(10);

	
	if (hw->mac.type == e1000_ich8lan) {
		
		ew32(PBA, E1000_PBA_8K);
		
		ew32(PBS, E1000_PBS_16K);
	}

	if (hw->mac.type == e1000_pchlan) {
		
		ret_val = e1000_read_nvm(hw, E1000_NVM_K1_CONFIG, 1, &reg);
		if (ret_val)
			return ret_val;

		if (reg & E1000_NVM_K1_ENABLE)
			dev_spec->nvm_k1_enabled = true;
		else
			dev_spec->nvm_k1_enabled = false;
	}

	ctrl = er32(CTRL);

	if (!e1000_check_reset_block(hw)) {
		
		if (hw->mac.type >= e1000_pchlan) {
			u32 status = er32(STATUS);
			ew32(STATUS, status & ~E1000_STATUS_PHYRA);
		}

		
		ctrl |= E1000_CTRL_PHY_RST;
	}
	ret_val = e1000_acquire_swflag_ich8lan(hw);
	
	hw_dbg(hw, "Issuing a global reset to ich8lan\n");
	ew32(CTRL, (ctrl | E1000_CTRL_RST));
	msleep(20);

	if (!ret_val)
		e1000_release_swflag_ich8lan(hw);

	if (ctrl & E1000_CTRL_PHY_RST)
		ret_val = hw->phy.ops.get_cfg_done(hw);

	if (hw->mac.type >= e1000_ich10lan) {
		e1000_lan_init_done_ich8lan(hw);
	} else {
		ret_val = e1000e_get_auto_rd_done(hw);
		if (ret_val) {
			
			hw_dbg(hw, "Auto Read Done did not complete\n");
		}
	}
	
	if (hw->mac.type == e1000_pchlan)
		e1e_rphy(hw, BM_WUC, &reg);

	ret_val = e1000_sw_lcd_config_ich8lan(hw);
	if (ret_val)
		goto out;

	if (hw->mac.type == e1000_pchlan) {
		ret_val = e1000_oem_bits_config_ich8lan(hw, true);
		if (ret_val)
			goto out;
	}
	
	if (hw->mac.type == e1000_pchlan)
		ew32(CRC_OFFSET, 0x65656565);

	ew32(IMC, 0xffffffff);
	icr = er32(ICR);

	kab = er32(KABGTXD);
	kab |= E1000_KABGTXD_BGSQLBIAS;
	ew32(KABGTXD, kab);

	if (hw->mac.type == e1000_pchlan)
		ret_val = e1000_hv_phy_workarounds_ich8lan(hw);

out:
	return ret_val;
}


static s32 e1000_init_hw_ich8lan(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	u32 ctrl_ext, txdctl, snoop;
	s32 ret_val;
	u16 i;

	e1000_initialize_hw_bits_ich8lan(hw);

	
	ret_val = mac->ops.id_led_init(hw);
	if (ret_val) {
		hw_dbg(hw, "Error initializing identification LED\n");
		return ret_val;
	}

	
	e1000e_init_rx_addrs(hw, mac->rar_entry_count);

	
	hw_dbg(hw, "Zeroing the MTA\n");
	for (i = 0; i < mac->mta_reg_count; i++)
		E1000_WRITE_REG_ARRAY(hw, E1000_MTA, i, 0);

	
	if (hw->phy.type == e1000_phy_82578) {
		hw->phy.ops.read_phy_reg(hw, BM_WUC, &i);
		ret_val = e1000_phy_hw_reset_ich8lan(hw);
		if (ret_val)
			return ret_val;
	}

	
	ret_val = e1000_setup_link_ich8lan(hw);

	
	txdctl = er32(TXDCTL(0));
	txdctl = (txdctl & ~E1000_TXDCTL_WTHRESH) |
		 E1000_TXDCTL_FULL_TX_DESC_WB;
	txdctl = (txdctl & ~E1000_TXDCTL_PTHRESH) |
		 E1000_TXDCTL_MAX_TX_DESC_PREFETCH;
	ew32(TXDCTL(0), txdctl);
	txdctl = er32(TXDCTL(1));
	txdctl = (txdctl & ~E1000_TXDCTL_WTHRESH) |
		 E1000_TXDCTL_FULL_TX_DESC_WB;
	txdctl = (txdctl & ~E1000_TXDCTL_PTHRESH) |
		 E1000_TXDCTL_MAX_TX_DESC_PREFETCH;
	ew32(TXDCTL(1), txdctl);

	
	if (mac->type == e1000_ich8lan)
		snoop = PCIE_ICH8_SNOOP_ALL;
	else
		snoop = (u32) ~(PCIE_NO_SNOOP_ALL);
	e1000e_set_pcie_no_snoop(hw, snoop);

	ctrl_ext = er32(CTRL_EXT);
	ctrl_ext |= E1000_CTRL_EXT_RO_DIS;
	ew32(CTRL_EXT, ctrl_ext);

	
	e1000_clear_hw_cntrs_ich8lan(hw);

	return 0;
}

static void e1000_initialize_hw_bits_ich8lan(struct e1000_hw *hw)
{
	u32 reg;

	
	reg = er32(CTRL_EXT);
	reg |= (1 << 22);
	
	if (hw->mac.type >= e1000_pchlan)
		reg |= E1000_CTRL_EXT_PHYPDEN;
	ew32(CTRL_EXT, reg);

	
	reg = er32(TXDCTL(0));
	reg |= (1 << 22);
	ew32(TXDCTL(0), reg);

	
	reg = er32(TXDCTL(1));
	reg |= (1 << 22);
	ew32(TXDCTL(1), reg);

	
	reg = er32(TARC(0));
	if (hw->mac.type == e1000_ich8lan)
		reg |= (1 << 28) | (1 << 29);
	reg |= (1 << 23) | (1 << 24) | (1 << 26) | (1 << 27);
	ew32(TARC(0), reg);

	
	reg = er32(TARC(1));
	if (er32(TCTL) & E1000_TCTL_MULR)
		reg &= ~(1 << 28);
	else
		reg |= (1 << 28);
	reg |= (1 << 24) | (1 << 26) | (1 << 30);
	ew32(TARC(1), reg);

	
	if (hw->mac.type == e1000_ich8lan) {
		reg = er32(STATUS);
		reg &= ~(1 << 31);
		ew32(STATUS, reg);
	}
}


static s32 e1000_setup_link_ich8lan(struct e1000_hw *hw)
{
	s32 ret_val;

	if (e1000_check_reset_block(hw))
		return 0;

	
	if (hw->fc.requested_mode == e1000_fc_default) {
		
		if (hw->mac.type == e1000_pchlan)
			hw->fc.requested_mode = e1000_fc_rx_pause;
		else
			hw->fc.requested_mode = e1000_fc_full;
	}

	
	hw->fc.current_mode = hw->fc.requested_mode;

	hw_dbg(hw, "After fix-ups FlowControl is now = %x\n",
		hw->fc.current_mode);

	
	ret_val = e1000_setup_copper_link_ich8lan(hw);
	if (ret_val)
		return ret_val;

	ew32(FCTTV, hw->fc.pause_time);
	if ((hw->phy.type == e1000_phy_82578) ||
	    (hw->phy.type == e1000_phy_82577)) {
		ret_val = hw->phy.ops.write_phy_reg(hw,
		                             PHY_REG(BM_PORT_CTRL_PAGE, 27),
		                             hw->fc.pause_time);
		if (ret_val)
			return ret_val;
	}

	return e1000e_set_fc_watermarks(hw);
}


static s32 e1000_setup_copper_link_ich8lan(struct e1000_hw *hw)
{
	u32 ctrl;
	s32 ret_val;
	u16 reg_data;

	ctrl = er32(CTRL);
	ctrl |= E1000_CTRL_SLU;
	ctrl &= ~(E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);
	ew32(CTRL, ctrl);

	
	ret_val = e1000e_write_kmrn_reg(hw, GG82563_REG(0x34, 4), 0xFFFF);
	if (ret_val)
		return ret_val;
	ret_val = e1000e_read_kmrn_reg(hw, GG82563_REG(0x34, 9), &reg_data);
	if (ret_val)
		return ret_val;
	reg_data |= 0x3F;
	ret_val = e1000e_write_kmrn_reg(hw, GG82563_REG(0x34, 9), reg_data);
	if (ret_val)
		return ret_val;

	switch (hw->phy.type) {
	case e1000_phy_igp_3:
		ret_val = e1000e_copper_link_setup_igp(hw);
		if (ret_val)
			return ret_val;
		break;
	case e1000_phy_bm:
	case e1000_phy_82578:
		ret_val = e1000e_copper_link_setup_m88(hw);
		if (ret_val)
			return ret_val;
		break;
	case e1000_phy_82577:
		ret_val = e1000_copper_link_setup_82577(hw);
		if (ret_val)
			return ret_val;
		break;
	case e1000_phy_ife:
		ret_val = hw->phy.ops.read_phy_reg(hw, IFE_PHY_MDIX_CONTROL,
		                               &reg_data);
		if (ret_val)
			return ret_val;

		reg_data &= ~IFE_PMC_AUTO_MDIX;

		switch (hw->phy.mdix) {
		case 1:
			reg_data &= ~IFE_PMC_FORCE_MDIX;
			break;
		case 2:
			reg_data |= IFE_PMC_FORCE_MDIX;
			break;
		case 0:
		default:
			reg_data |= IFE_PMC_AUTO_MDIX;
			break;
		}
		ret_val = hw->phy.ops.write_phy_reg(hw, IFE_PHY_MDIX_CONTROL,
		                                reg_data);
		if (ret_val)
			return ret_val;
		break;
	default:
		break;
	}
	return e1000e_setup_copper_link(hw);
}


static s32 e1000_get_link_up_info_ich8lan(struct e1000_hw *hw, u16 *speed,
					  u16 *duplex)
{
	s32 ret_val;

	ret_val = e1000e_get_speed_and_duplex_copper(hw, speed, duplex);
	if (ret_val)
		return ret_val;

	if ((hw->mac.type == e1000_ich8lan) &&
	    (hw->phy.type == e1000_phy_igp_3) &&
	    (*speed == SPEED_1000)) {
		ret_val = e1000_kmrn_lock_loss_workaround_ich8lan(hw);
	}

	return ret_val;
}


static s32 e1000_kmrn_lock_loss_workaround_ich8lan(struct e1000_hw *hw)
{
	struct e1000_dev_spec_ich8lan *dev_spec = &hw->dev_spec.ich8lan;
	u32 phy_ctrl;
	s32 ret_val;
	u16 i, data;
	bool link;

	if (!dev_spec->kmrn_lock_loss_workaround_enabled)
		return 0;

	
	ret_val = e1000e_phy_has_link_generic(hw, 1, 0, &link);
	if (!link)
		return 0;

	for (i = 0; i < 10; i++) {
		
		ret_val = e1e_rphy(hw, IGP3_KMRN_DIAG, &data);
		if (ret_val)
			return ret_val;
		
		ret_val = e1e_rphy(hw, IGP3_KMRN_DIAG, &data);
		if (ret_val)
			return ret_val;

		
		if (!(data & IGP3_KMRN_DIAG_PCS_LOCK_LOSS))
			return 0;

		
		e1000_phy_hw_reset(hw);
		mdelay(5);
	}
	
	phy_ctrl = er32(PHY_CTRL);
	phy_ctrl |= (E1000_PHY_CTRL_GBE_DISABLE |
		     E1000_PHY_CTRL_NOND0A_GBE_DISABLE);
	ew32(PHY_CTRL, phy_ctrl);

	
	e1000e_gig_downshift_workaround_ich8lan(hw);

	
	return -E1000_ERR_PHY;
}


void e1000e_set_kmrn_lock_loss_workaround_ich8lan(struct e1000_hw *hw,
						 bool state)
{
	struct e1000_dev_spec_ich8lan *dev_spec = &hw->dev_spec.ich8lan;

	if (hw->mac.type != e1000_ich8lan) {
		hw_dbg(hw, "Workaround applies to ICH8 only.\n");
		return;
	}

	dev_spec->kmrn_lock_loss_workaround_enabled = state;
}


void e1000e_igp3_phy_powerdown_workaround_ich8lan(struct e1000_hw *hw)
{
	u32 reg;
	u16 data;
	u8  retry = 0;

	if (hw->phy.type != e1000_phy_igp_3)
		return;

	
	do {
		
		reg = er32(PHY_CTRL);
		reg |= (E1000_PHY_CTRL_GBE_DISABLE |
			E1000_PHY_CTRL_NOND0A_GBE_DISABLE);
		ew32(PHY_CTRL, reg);

		
		if (hw->mac.type == e1000_ich8lan)
			e1000e_gig_downshift_workaround_ich8lan(hw);

		
		e1e_rphy(hw, IGP3_VR_CTRL, &data);
		data &= ~IGP3_VR_CTRL_DEV_POWERDOWN_MODE_MASK;
		e1e_wphy(hw, IGP3_VR_CTRL, data | IGP3_VR_CTRL_MODE_SHUTDOWN);

		
		e1e_rphy(hw, IGP3_VR_CTRL, &data);
		data &= IGP3_VR_CTRL_DEV_POWERDOWN_MODE_MASK;
		if ((data == IGP3_VR_CTRL_MODE_SHUTDOWN) || retry)
			break;

		
		reg = er32(CTRL);
		ew32(CTRL, reg | E1000_CTRL_PHY_RST);
		retry++;
	} while (retry);
}


void e1000e_gig_downshift_workaround_ich8lan(struct e1000_hw *hw)
{
	s32 ret_val;
	u16 reg_data;

	if ((hw->mac.type != e1000_ich8lan) ||
	    (hw->phy.type != e1000_phy_igp_3))
		return;

	ret_val = e1000e_read_kmrn_reg(hw, E1000_KMRNCTRLSTA_DIAG_OFFSET,
				      &reg_data);
	if (ret_val)
		return;
	reg_data |= E1000_KMRNCTRLSTA_DIAG_NELPBK;
	ret_val = e1000e_write_kmrn_reg(hw, E1000_KMRNCTRLSTA_DIAG_OFFSET,
				       reg_data);
	if (ret_val)
		return;
	reg_data &= ~E1000_KMRNCTRLSTA_DIAG_NELPBK;
	ret_val = e1000e_write_kmrn_reg(hw, E1000_KMRNCTRLSTA_DIAG_OFFSET,
				       reg_data);
}


void e1000e_disable_gig_wol_ich8lan(struct e1000_hw *hw)
{
	u32 phy_ctrl;

	switch (hw->mac.type) {
	case e1000_ich9lan:
	case e1000_ich10lan:
	case e1000_pchlan:
		phy_ctrl = er32(PHY_CTRL);
		phy_ctrl |= E1000_PHY_CTRL_D0A_LPLU |
		            E1000_PHY_CTRL_GBE_DISABLE;
		ew32(PHY_CTRL, phy_ctrl);

		if (hw->mac.type == e1000_pchlan)
			e1000_phy_hw_reset_ich8lan(hw);
	default:
		break;
	}

	return;
}


static s32 e1000_cleanup_led_ich8lan(struct e1000_hw *hw)
{
	if (hw->phy.type == e1000_phy_ife)
		return e1e_wphy(hw, IFE_PHY_SPECIAL_CONTROL_LED, 0);

	ew32(LEDCTL, hw->mac.ledctl_default);
	return 0;
}


static s32 e1000_led_on_ich8lan(struct e1000_hw *hw)
{
	if (hw->phy.type == e1000_phy_ife)
		return e1e_wphy(hw, IFE_PHY_SPECIAL_CONTROL_LED,
				(IFE_PSCL_PROBE_MODE | IFE_PSCL_PROBE_LEDS_ON));

	ew32(LEDCTL, hw->mac.ledctl_mode2);
	return 0;
}


static s32 e1000_led_off_ich8lan(struct e1000_hw *hw)
{
	if (hw->phy.type == e1000_phy_ife)
		return e1e_wphy(hw, IFE_PHY_SPECIAL_CONTROL_LED,
			       (IFE_PSCL_PROBE_MODE | IFE_PSCL_PROBE_LEDS_OFF));

	ew32(LEDCTL, hw->mac.ledctl_mode1);
	return 0;
}


static s32 e1000_setup_led_pchlan(struct e1000_hw *hw)
{
	return hw->phy.ops.write_phy_reg(hw, HV_LED_CONFIG,
					(u16)hw->mac.ledctl_mode1);
}


static s32 e1000_cleanup_led_pchlan(struct e1000_hw *hw)
{
	return hw->phy.ops.write_phy_reg(hw, HV_LED_CONFIG,
					(u16)hw->mac.ledctl_default);
}


static s32 e1000_led_on_pchlan(struct e1000_hw *hw)
{
	u16 data = (u16)hw->mac.ledctl_mode2;
	u32 i, led;

	
	if (!(er32(STATUS) & E1000_STATUS_LU)) {
		for (i = 0; i < 3; i++) {
			led = (data >> (i * 5)) & E1000_PHY_LED0_MASK;
			if ((led & E1000_PHY_LED0_MODE_MASK) !=
			    E1000_LEDCTL_MODE_LINK_UP)
				continue;
			if (led & E1000_PHY_LED0_IVRT)
				data &= ~(E1000_PHY_LED0_IVRT << (i * 5));
			else
				data |= (E1000_PHY_LED0_IVRT << (i * 5));
		}
	}

	return hw->phy.ops.write_phy_reg(hw, HV_LED_CONFIG, data);
}


static s32 e1000_led_off_pchlan(struct e1000_hw *hw)
{
	u16 data = (u16)hw->mac.ledctl_mode1;
	u32 i, led;

	
	if (!(er32(STATUS) & E1000_STATUS_LU)) {
		for (i = 0; i < 3; i++) {
			led = (data >> (i * 5)) & E1000_PHY_LED0_MASK;
			if ((led & E1000_PHY_LED0_MODE_MASK) !=
			    E1000_LEDCTL_MODE_LINK_UP)
				continue;
			if (led & E1000_PHY_LED0_IVRT)
				data &= ~(E1000_PHY_LED0_IVRT << (i * 5));
			else
				data |= (E1000_PHY_LED0_IVRT << (i * 5));
		}
	}

	return hw->phy.ops.write_phy_reg(hw, HV_LED_CONFIG, data);
}


static s32 e1000_get_cfg_done_ich8lan(struct e1000_hw *hw)
{
	u32 bank = 0;

	if (hw->mac.type >= e1000_pchlan) {
		u32 status = er32(STATUS);

		if (status & E1000_STATUS_PHYRA)
			ew32(STATUS, status & ~E1000_STATUS_PHYRA);
		else
			hw_dbg(hw,
			       "PHY Reset Asserted not set - needs delay\n");
	}

	e1000e_get_cfg_done(hw);

	
	if ((hw->mac.type != e1000_ich10lan) &&
	    (hw->mac.type != e1000_pchlan)) {
		if (((er32(EECD) & E1000_EECD_PRES) == 0) &&
		    (hw->phy.type == e1000_phy_igp_3)) {
			e1000e_phy_init_script_igp3(hw);
		}
	} else {
		if (e1000_valid_nvm_bank_detect_ich8lan(hw, &bank)) {
			
			hw_dbg(hw, "EEPROM not present\n");
			return -E1000_ERR_CONFIG;
		}
	}

	return 0;
}


static void e1000_clear_hw_cntrs_ich8lan(struct e1000_hw *hw)
{
	u32 temp;
	u16 phy_data;

	e1000e_clear_hw_cntrs_base(hw);

	temp = er32(ALGNERRC);
	temp = er32(RXERRC);
	temp = er32(TNCRS);
	temp = er32(CEXTERR);
	temp = er32(TSCTC);
	temp = er32(TSCTFC);

	temp = er32(MGTPRC);
	temp = er32(MGTPDC);
	temp = er32(MGTPTC);

	temp = er32(IAC);
	temp = er32(ICRXOC);

	
	if ((hw->phy.type == e1000_phy_82578) ||
	    (hw->phy.type == e1000_phy_82577)) {
		hw->phy.ops.read_phy_reg(hw, HV_SCC_UPPER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_SCC_LOWER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_ECOL_UPPER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_ECOL_LOWER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_MCC_UPPER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_MCC_LOWER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_LATECOL_UPPER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_LATECOL_LOWER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_COLC_UPPER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_COLC_LOWER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_DC_UPPER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_DC_LOWER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_TNCRS_UPPER, &phy_data);
		hw->phy.ops.read_phy_reg(hw, HV_TNCRS_LOWER, &phy_data);
	}
}

static struct e1000_mac_operations ich8_mac_ops = {
	.id_led_init		= e1000e_id_led_init,
	.check_mng_mode		= e1000_check_mng_mode_ich8lan,
	.check_for_link		= e1000_check_for_copper_link_ich8lan,
	
	.clear_hw_cntrs		= e1000_clear_hw_cntrs_ich8lan,
	.get_bus_info		= e1000_get_bus_info_ich8lan,
	.get_link_up_info	= e1000_get_link_up_info_ich8lan,
	
	
	.update_mc_addr_list	= e1000e_update_mc_addr_list_generic,
	.reset_hw		= e1000_reset_hw_ich8lan,
	.init_hw		= e1000_init_hw_ich8lan,
	.setup_link		= e1000_setup_link_ich8lan,
	.setup_physical_interface= e1000_setup_copper_link_ich8lan,
	
};

static struct e1000_phy_operations ich8_phy_ops = {
	.acquire_phy		= e1000_acquire_swflag_ich8lan,
	.check_reset_block	= e1000_check_reset_block_ich8lan,
	.commit_phy		= NULL,
	.force_speed_duplex	= e1000_phy_force_speed_duplex_ich8lan,
	.get_cfg_done		= e1000_get_cfg_done_ich8lan,
	.get_cable_length	= e1000e_get_cable_length_igp_2,
	.get_phy_info		= e1000_get_phy_info_ich8lan,
	.read_phy_reg		= e1000e_read_phy_reg_igp,
	.release_phy		= e1000_release_swflag_ich8lan,
	.reset_phy		= e1000_phy_hw_reset_ich8lan,
	.set_d0_lplu_state	= e1000_set_d0_lplu_state_ich8lan,
	.set_d3_lplu_state	= e1000_set_d3_lplu_state_ich8lan,
	.write_phy_reg		= e1000e_write_phy_reg_igp,
};

static struct e1000_nvm_operations ich8_nvm_ops = {
	.acquire_nvm		= e1000_acquire_nvm_ich8lan,
	.read_nvm	 	= e1000_read_nvm_ich8lan,
	.release_nvm		= e1000_release_nvm_ich8lan,
	.update_nvm		= e1000_update_nvm_checksum_ich8lan,
	.valid_led_default	= e1000_valid_led_default_ich8lan,
	.validate_nvm		= e1000_validate_nvm_checksum_ich8lan,
	.write_nvm		= e1000_write_nvm_ich8lan,
};

struct e1000_info e1000_ich8_info = {
	.mac			= e1000_ich8lan,
	.flags			= FLAG_HAS_WOL
				  | FLAG_IS_ICH
				  | FLAG_RX_CSUM_ENABLED
				  | FLAG_HAS_CTRLEXT_ON_LOAD
				  | FLAG_HAS_AMT
				  | FLAG_HAS_FLASH
				  | FLAG_APME_IN_WUC,
	.pba			= 8,
	.max_hw_frame_size	= ETH_FRAME_LEN + ETH_FCS_LEN,
	.get_variants		= e1000_get_variants_ich8lan,
	.mac_ops		= &ich8_mac_ops,
	.phy_ops		= &ich8_phy_ops,
	.nvm_ops		= &ich8_nvm_ops,
};

struct e1000_info e1000_ich9_info = {
	.mac			= e1000_ich9lan,
	.flags			= FLAG_HAS_JUMBO_FRAMES
				  | FLAG_IS_ICH
				  | FLAG_HAS_WOL
				  | FLAG_RX_CSUM_ENABLED
				  | FLAG_HAS_CTRLEXT_ON_LOAD
				  | FLAG_HAS_AMT
				  | FLAG_HAS_ERT
				  | FLAG_HAS_FLASH
				  | FLAG_APME_IN_WUC,
	.pba			= 10,
	.max_hw_frame_size	= DEFAULT_JUMBO,
	.get_variants		= e1000_get_variants_ich8lan,
	.mac_ops		= &ich8_mac_ops,
	.phy_ops		= &ich8_phy_ops,
	.nvm_ops		= &ich8_nvm_ops,
};

struct e1000_info e1000_ich10_info = {
	.mac			= e1000_ich10lan,
	.flags			= FLAG_HAS_JUMBO_FRAMES
				  | FLAG_IS_ICH
				  | FLAG_HAS_WOL
				  | FLAG_RX_CSUM_ENABLED
				  | FLAG_HAS_CTRLEXT_ON_LOAD
				  | FLAG_HAS_AMT
				  | FLAG_HAS_ERT
				  | FLAG_HAS_FLASH
				  | FLAG_APME_IN_WUC,
	.pba			= 10,
	.max_hw_frame_size	= DEFAULT_JUMBO,
	.get_variants		= e1000_get_variants_ich8lan,
	.mac_ops		= &ich8_mac_ops,
	.phy_ops		= &ich8_phy_ops,
	.nvm_ops		= &ich8_nvm_ops,
};

struct e1000_info e1000_pch_info = {
	.mac			= e1000_pchlan,
	.flags			= FLAG_IS_ICH
				  | FLAG_HAS_WOL
				  | FLAG_RX_CSUM_ENABLED
				  | FLAG_HAS_CTRLEXT_ON_LOAD
				  | FLAG_HAS_AMT
				  | FLAG_HAS_FLASH
				  | FLAG_HAS_JUMBO_FRAMES
				  | FLAG_DISABLE_FC_PAUSE_TIME 
				  | FLAG_APME_IN_WUC,
	.pba			= 26,
	.max_hw_frame_size	= 4096,
	.get_variants		= e1000_get_variants_ich8lan,
	.mac_ops		= &ich8_mac_ops,
	.phy_ops		= &ich8_phy_ops,
	.nvm_ops		= &ich8_nvm_ops,
};
