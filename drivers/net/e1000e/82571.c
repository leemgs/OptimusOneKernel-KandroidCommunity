



#include <linux/netdevice.h>
#include <linux/delay.h>
#include <linux/pci.h>

#include "e1000.h"

#define ID_LED_RESERVED_F746 0xF746
#define ID_LED_DEFAULT_82573 ((ID_LED_DEF1_DEF2 << 12) | \
			      (ID_LED_OFF1_ON2  <<  8) | \
			      (ID_LED_DEF1_DEF2 <<  4) | \
			      (ID_LED_DEF1_DEF2))

#define E1000_GCR_L1_ACT_WITHOUT_L0S_RX 0x08000000

#define E1000_NVM_INIT_CTRL2_MNGM 0x6000 

static s32 e1000_get_phy_id_82571(struct e1000_hw *hw);
static s32 e1000_setup_copper_link_82571(struct e1000_hw *hw);
static s32 e1000_setup_fiber_serdes_link_82571(struct e1000_hw *hw);
static s32 e1000_check_for_serdes_link_82571(struct e1000_hw *hw);
static s32 e1000_write_nvm_eewr_82571(struct e1000_hw *hw, u16 offset,
				      u16 words, u16 *data);
static s32 e1000_fix_nvm_checksum_82571(struct e1000_hw *hw);
static void e1000_initialize_hw_bits_82571(struct e1000_hw *hw);
static s32 e1000_setup_link_82571(struct e1000_hw *hw);
static void e1000_clear_hw_cntrs_82571(struct e1000_hw *hw);
static bool e1000_check_mng_mode_82574(struct e1000_hw *hw);
static s32 e1000_led_on_82574(struct e1000_hw *hw);
static void e1000_put_hw_semaphore_82571(struct e1000_hw *hw);


static s32 e1000_init_phy_params_82571(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;

	if (hw->phy.media_type != e1000_media_type_copper) {
		phy->type = e1000_phy_none;
		return 0;
	}

	phy->addr			 = 1;
	phy->autoneg_mask		 = AUTONEG_ADVERTISE_SPEED_DEFAULT;
	phy->reset_delay_us		 = 100;

	switch (hw->mac.type) {
	case e1000_82571:
	case e1000_82572:
		phy->type		 = e1000_phy_igp_2;
		break;
	case e1000_82573:
		phy->type		 = e1000_phy_m88;
		break;
	case e1000_82574:
	case e1000_82583:
		phy->type		 = e1000_phy_bm;
		break;
	default:
		return -E1000_ERR_PHY;
		break;
	}

	
	ret_val = e1000_get_phy_id_82571(hw);

	
	switch (hw->mac.type) {
	case e1000_82571:
	case e1000_82572:
		if (phy->id != IGP01E1000_I_PHY_ID)
			return -E1000_ERR_PHY;
		break;
	case e1000_82573:
		if (phy->id != M88E1111_I_PHY_ID)
			return -E1000_ERR_PHY;
		break;
	case e1000_82574:
	case e1000_82583:
		if (phy->id != BME1000_E_PHY_ID_R2)
			return -E1000_ERR_PHY;
		break;
	default:
		return -E1000_ERR_PHY;
		break;
	}

	return 0;
}


static s32 e1000_init_nvm_params_82571(struct e1000_hw *hw)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	u32 eecd = er32(EECD);
	u16 size;

	nvm->opcode_bits = 8;
	nvm->delay_usec = 1;
	switch (nvm->override) {
	case e1000_nvm_override_spi_large:
		nvm->page_size = 32;
		nvm->address_bits = 16;
		break;
	case e1000_nvm_override_spi_small:
		nvm->page_size = 8;
		nvm->address_bits = 8;
		break;
	default:
		nvm->page_size = eecd & E1000_EECD_ADDR_BITS ? 32 : 8;
		nvm->address_bits = eecd & E1000_EECD_ADDR_BITS ? 16 : 8;
		break;
	}

	switch (hw->mac.type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		if (((eecd >> 15) & 0x3) == 0x3) {
			nvm->type = e1000_nvm_flash_hw;
			nvm->word_size = 2048;
			
			eecd &= ~E1000_EECD_AUPDEN;
			ew32(EECD, eecd);
			break;
		}
		
	default:
		nvm->type = e1000_nvm_eeprom_spi;
		size = (u16)((eecd & E1000_EECD_SIZE_EX_MASK) >>
				  E1000_EECD_SIZE_EX_SHIFT);
		
		size += NVM_WORD_SIZE_BASE_SHIFT;

		
		if (size > 14)
			size = 14;
		nvm->word_size	= 1 << size;
		break;
	}

	return 0;
}


static s32 e1000_init_mac_params_82571(struct e1000_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	struct e1000_mac_info *mac = &hw->mac;
	struct e1000_mac_operations *func = &mac->ops;
	u32 swsm = 0;
	u32 swsm2 = 0;
	bool force_clear_smbi = false;

	
	switch (adapter->pdev->device) {
	case E1000_DEV_ID_82571EB_FIBER:
	case E1000_DEV_ID_82572EI_FIBER:
	case E1000_DEV_ID_82571EB_QUAD_FIBER:
		hw->phy.media_type = e1000_media_type_fiber;
		break;
	case E1000_DEV_ID_82571EB_SERDES:
	case E1000_DEV_ID_82572EI_SERDES:
	case E1000_DEV_ID_82571EB_SERDES_DUAL:
	case E1000_DEV_ID_82571EB_SERDES_QUAD:
		hw->phy.media_type = e1000_media_type_internal_serdes;
		break;
	default:
		hw->phy.media_type = e1000_media_type_copper;
		break;
	}

	
	mac->mta_reg_count = 128;
	
	mac->rar_entry_count = E1000_RAR_ENTRIES;
	
	mac->arc_subsystem_valid = (er32(FWSM) & E1000_FWSM_MODE_MASK) ? 1 : 0;

	
	switch (hw->phy.media_type) {
	case e1000_media_type_copper:
		func->setup_physical_interface = e1000_setup_copper_link_82571;
		func->check_for_link = e1000e_check_for_copper_link;
		func->get_link_up_info = e1000e_get_speed_and_duplex_copper;
		break;
	case e1000_media_type_fiber:
		func->setup_physical_interface =
			e1000_setup_fiber_serdes_link_82571;
		func->check_for_link = e1000e_check_for_fiber_link;
		func->get_link_up_info =
			e1000e_get_speed_and_duplex_fiber_serdes;
		break;
	case e1000_media_type_internal_serdes:
		func->setup_physical_interface =
			e1000_setup_fiber_serdes_link_82571;
		func->check_for_link = e1000_check_for_serdes_link_82571;
		func->get_link_up_info =
			e1000e_get_speed_and_duplex_fiber_serdes;
		break;
	default:
		return -E1000_ERR_CONFIG;
		break;
	}

	switch (hw->mac.type) {
	case e1000_82574:
	case e1000_82583:
		func->check_mng_mode = e1000_check_mng_mode_82574;
		func->led_on = e1000_led_on_82574;
		break;
	default:
		func->check_mng_mode = e1000e_check_mng_mode_generic;
		func->led_on = e1000e_led_on_generic;
		break;
	}

	
	switch (hw->mac.type) {
	case e1000_82571:
	case e1000_82572:
		swsm2 = er32(SWSM2);

		if (!(swsm2 & E1000_SWSM2_LOCK)) {
			
			ew32(SWSM2,
			    swsm2 | E1000_SWSM2_LOCK);
			force_clear_smbi = true;
		} else
			force_clear_smbi = false;
		break;
	default:
		force_clear_smbi = true;
		break;
	}

	if (force_clear_smbi) {
		
		swsm = er32(SWSM);
		if (swsm & E1000_SWSM_SMBI) {
			
			hw_dbg(hw, "Please update your 82571 Bootagent\n");
		}
		ew32(SWSM, swsm & ~E1000_SWSM_SMBI);
	}

	
	 hw->dev_spec.e82571.smb_counter = 0;

	return 0;
}

static s32 e1000_get_variants_82571(struct e1000_adapter *adapter)
{
	struct e1000_hw *hw = &adapter->hw;
	static int global_quad_port_a; 
	struct pci_dev *pdev = adapter->pdev;
	u16 eeprom_data = 0;
	int is_port_b = er32(STATUS) & E1000_STATUS_FUNC_1;
	s32 rc;

	rc = e1000_init_mac_params_82571(adapter);
	if (rc)
		return rc;

	rc = e1000_init_nvm_params_82571(hw);
	if (rc)
		return rc;

	rc = e1000_init_phy_params_82571(hw);
	if (rc)
		return rc;

	
	switch (pdev->device) {
	case E1000_DEV_ID_82571EB_QUAD_COPPER:
	case E1000_DEV_ID_82571EB_QUAD_FIBER:
	case E1000_DEV_ID_82571EB_QUAD_COPPER_LP:
	case E1000_DEV_ID_82571PT_QUAD_COPPER:
		adapter->flags |= FLAG_IS_QUAD_PORT;
		
		if (global_quad_port_a == 0)
			adapter->flags |= FLAG_IS_QUAD_PORT_A;
		
		global_quad_port_a++;
		if (global_quad_port_a == 4)
			global_quad_port_a = 0;
		break;
	default:
		break;
	}

	switch (adapter->hw.mac.type) {
	case e1000_82571:
		
		if (((pdev->device == E1000_DEV_ID_82571EB_FIBER) ||
		     (pdev->device == E1000_DEV_ID_82571EB_SERDES) ||
		     (pdev->device == E1000_DEV_ID_82571EB_COPPER)) &&
		    (is_port_b))
			adapter->flags &= ~FLAG_HAS_WOL;
		
		if (adapter->flags & FLAG_IS_QUAD_PORT &&
		    (!(adapter->flags & FLAG_IS_QUAD_PORT_A)))
			adapter->flags &= ~FLAG_HAS_WOL;
		
		if (pdev->device == E1000_DEV_ID_82571EB_SERDES_QUAD)
			adapter->flags &= ~FLAG_HAS_WOL;
		break;

	case e1000_82573:
		if (pdev->device == E1000_DEV_ID_82573L) {
			if (e1000_read_nvm(&adapter->hw, NVM_INIT_3GIO_3, 1,
				       &eeprom_data) < 0)
				break;
			if (!(eeprom_data & NVM_WORD1A_ASPM_MASK)) {
				adapter->flags |= FLAG_HAS_JUMBO_FRAMES;
				adapter->max_hw_frame_size = DEFAULT_JUMBO;
			}
		}
		break;
	default:
		break;
	}

	return 0;
}


static s32 e1000_get_phy_id_82571(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 phy_id = 0;

	switch (hw->mac.type) {
	case e1000_82571:
	case e1000_82572:
		
		phy->id = IGP01E1000_I_PHY_ID;
		break;
	case e1000_82573:
		return e1000e_get_phy_id(hw);
		break;
	case e1000_82574:
	case e1000_82583:
		ret_val = e1e_rphy(hw, PHY_ID1, &phy_id);
		if (ret_val)
			return ret_val;

		phy->id = (u32)(phy_id << 16);
		udelay(20);
		ret_val = e1e_rphy(hw, PHY_ID2, &phy_id);
		if (ret_val)
			return ret_val;

		phy->id |= (u32)(phy_id);
		phy->revision = (u32)(phy_id & ~PHY_REVISION_MASK);
		break;
	default:
		return -E1000_ERR_PHY;
		break;
	}

	return 0;
}


static s32 e1000_get_hw_semaphore_82571(struct e1000_hw *hw)
{
	u32 swsm;
	s32 sw_timeout = hw->nvm.word_size + 1;
	s32 fw_timeout = hw->nvm.word_size + 1;
	s32 i = 0;

	
	if (hw->dev_spec.e82571.smb_counter > 2)
		sw_timeout = 1;

	
	while (i < sw_timeout) {
		swsm = er32(SWSM);
		if (!(swsm & E1000_SWSM_SMBI))
			break;

		udelay(50);
		i++;
	}

	if (i == sw_timeout) {
		hw_dbg(hw, "Driver can't access device - SMBI bit is set.\n");
		hw->dev_spec.e82571.smb_counter++;
	}
	
	for (i = 0; i < fw_timeout; i++) {
		swsm = er32(SWSM);
		ew32(SWSM, swsm | E1000_SWSM_SWESMBI);

		
		if (er32(SWSM) & E1000_SWSM_SWESMBI)
			break;

		udelay(50);
	}

	if (i == fw_timeout) {
		
		e1000_put_hw_semaphore_82571(hw);
		hw_dbg(hw, "Driver can't access the NVM\n");
		return -E1000_ERR_NVM;
	}

	return 0;
}


static void e1000_put_hw_semaphore_82571(struct e1000_hw *hw)
{
	u32 swsm;

	swsm = er32(SWSM);
	swsm &= ~(E1000_SWSM_SMBI | E1000_SWSM_SWESMBI);
	ew32(SWSM, swsm);
}


static s32 e1000_acquire_nvm_82571(struct e1000_hw *hw)
{
	s32 ret_val;

	ret_val = e1000_get_hw_semaphore_82571(hw);
	if (ret_val)
		return ret_val;

	switch (hw->mac.type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		break;
	default:
		ret_val = e1000e_acquire_nvm(hw);
		break;
	}

	if (ret_val)
		e1000_put_hw_semaphore_82571(hw);

	return ret_val;
}


static void e1000_release_nvm_82571(struct e1000_hw *hw)
{
	e1000e_release_nvm(hw);
	e1000_put_hw_semaphore_82571(hw);
}


static s32 e1000_write_nvm_82571(struct e1000_hw *hw, u16 offset, u16 words,
				 u16 *data)
{
	s32 ret_val;

	switch (hw->mac.type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		ret_val = e1000_write_nvm_eewr_82571(hw, offset, words, data);
		break;
	case e1000_82571:
	case e1000_82572:
		ret_val = e1000e_write_nvm_spi(hw, offset, words, data);
		break;
	default:
		ret_val = -E1000_ERR_NVM;
		break;
	}

	return ret_val;
}


static s32 e1000_update_nvm_checksum_82571(struct e1000_hw *hw)
{
	u32 eecd;
	s32 ret_val;
	u16 i;

	ret_val = e1000e_update_nvm_checksum_generic(hw);
	if (ret_val)
		return ret_val;

	
	if (hw->nvm.type != e1000_nvm_flash_hw)
		return ret_val;

	
	for (i = 0; i < E1000_FLASH_UPDATES; i++) {
		msleep(1);
		if ((er32(EECD) & E1000_EECD_FLUPD) == 0)
			break;
	}

	if (i == E1000_FLASH_UPDATES)
		return -E1000_ERR_NVM;

	
	if ((er32(FLOP) & 0xFF00) == E1000_STM_OPCODE) {
		
		ew32(HICR, E1000_HICR_FW_RESET_ENABLE);
		e1e_flush();
		ew32(HICR, E1000_HICR_FW_RESET);
	}

	
	eecd = er32(EECD) | E1000_EECD_FLUPD;
	ew32(EECD, eecd);

	for (i = 0; i < E1000_FLASH_UPDATES; i++) {
		msleep(1);
		if ((er32(EECD) & E1000_EECD_FLUPD) == 0)
			break;
	}

	if (i == E1000_FLASH_UPDATES)
		return -E1000_ERR_NVM;

	return 0;
}


static s32 e1000_validate_nvm_checksum_82571(struct e1000_hw *hw)
{
	if (hw->nvm.type == e1000_nvm_flash_hw)
		e1000_fix_nvm_checksum_82571(hw);

	return e1000e_validate_nvm_checksum_generic(hw);
}


static s32 e1000_write_nvm_eewr_82571(struct e1000_hw *hw, u16 offset,
				      u16 words, u16 *data)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	u32 i;
	u32 eewr = 0;
	s32 ret_val = 0;

	
	if ((offset >= nvm->word_size) || (words > (nvm->word_size - offset)) ||
	    (words == 0)) {
		hw_dbg(hw, "nvm parameter(s) out of bounds\n");
		return -E1000_ERR_NVM;
	}

	for (i = 0; i < words; i++) {
		eewr = (data[i] << E1000_NVM_RW_REG_DATA) |
		       ((offset+i) << E1000_NVM_RW_ADDR_SHIFT) |
		       E1000_NVM_RW_REG_START;

		ret_val = e1000e_poll_eerd_eewr_done(hw, E1000_NVM_POLL_WRITE);
		if (ret_val)
			break;

		ew32(EEWR, eewr);

		ret_val = e1000e_poll_eerd_eewr_done(hw, E1000_NVM_POLL_WRITE);
		if (ret_val)
			break;
	}

	return ret_val;
}


static s32 e1000_get_cfg_done_82571(struct e1000_hw *hw)
{
	s32 timeout = PHY_CFG_TIMEOUT;

	while (timeout) {
		if (er32(EEMNGCTL) &
		    E1000_NVM_CFG_DONE_PORT_0)
			break;
		msleep(1);
		timeout--;
	}
	if (!timeout) {
		hw_dbg(hw, "MNG configuration cycle has not completed.\n");
		return -E1000_ERR_RESET;
	}

	return 0;
}


static s32 e1000_set_d0_lplu_state_82571(struct e1000_hw *hw, bool active)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 data;

	ret_val = e1e_rphy(hw, IGP02E1000_PHY_POWER_MGMT, &data);
	if (ret_val)
		return ret_val;

	if (active) {
		data |= IGP02E1000_PM_D0_LPLU;
		ret_val = e1e_wphy(hw, IGP02E1000_PHY_POWER_MGMT, data);
		if (ret_val)
			return ret_val;

		
		ret_val = e1e_rphy(hw, IGP01E1000_PHY_PORT_CONFIG, &data);
		data &= ~IGP01E1000_PSCFR_SMART_SPEED;
		ret_val = e1e_wphy(hw, IGP01E1000_PHY_PORT_CONFIG, data);
		if (ret_val)
			return ret_val;
	} else {
		data &= ~IGP02E1000_PM_D0_LPLU;
		ret_val = e1e_wphy(hw, IGP02E1000_PHY_POWER_MGMT, data);
		
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


static s32 e1000_reset_hw_82571(struct e1000_hw *hw)
{
	u32 ctrl;
	u32 extcnf_ctrl;
	u32 ctrl_ext;
	u32 icr;
	s32 ret_val;
	u16 i = 0;

	
	ret_val = e1000e_disable_pcie_master(hw);
	if (ret_val)
		hw_dbg(hw, "PCI-E Master disable polling has failed.\n");

	hw_dbg(hw, "Masking off all interrupts\n");
	ew32(IMC, 0xffffffff);

	ew32(RCTL, 0);
	ew32(TCTL, E1000_TCTL_PSP);
	e1e_flush();

	msleep(10);

	
	switch (hw->mac.type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		extcnf_ctrl = er32(EXTCNF_CTRL);
		extcnf_ctrl |= E1000_EXTCNF_CTRL_MDIO_SW_OWNERSHIP;

		do {
			ew32(EXTCNF_CTRL, extcnf_ctrl);
			extcnf_ctrl = er32(EXTCNF_CTRL);

			if (extcnf_ctrl & E1000_EXTCNF_CTRL_MDIO_SW_OWNERSHIP)
				break;

			extcnf_ctrl |= E1000_EXTCNF_CTRL_MDIO_SW_OWNERSHIP;

			msleep(2);
			i++;
		} while (i < MDIO_OWNERSHIP_TIMEOUT);
		break;
	default:
		break;
	}

	ctrl = er32(CTRL);

	hw_dbg(hw, "Issuing a global reset to MAC\n");
	ew32(CTRL, ctrl | E1000_CTRL_RST);

	if (hw->nvm.type == e1000_nvm_flash_hw) {
		udelay(10);
		ctrl_ext = er32(CTRL_EXT);
		ctrl_ext |= E1000_CTRL_EXT_EE_RST;
		ew32(CTRL_EXT, ctrl_ext);
		e1e_flush();
	}

	ret_val = e1000e_get_auto_rd_done(hw);
	if (ret_val)
		
		return ret_val;

	

	switch (hw->mac.type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		msleep(25);
		break;
	default:
		break;
	}

	
	ew32(IMC, 0xffffffff);
	icr = er32(ICR);

	if (hw->mac.type == e1000_82571 &&
		hw->dev_spec.e82571.alt_mac_addr_is_present)
			e1000e_set_laa_state_82571(hw, true);

	
	if (hw->phy.media_type == e1000_media_type_internal_serdes)
		hw->mac.serdes_link_state = e1000_serdes_link_down;

	return 0;
}


static s32 e1000_init_hw_82571(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	u32 reg_data;
	s32 ret_val;
	u16 i;
	u16 rar_count = mac->rar_entry_count;

	e1000_initialize_hw_bits_82571(hw);

	
	ret_val = e1000e_id_led_init(hw);
	if (ret_val) {
		hw_dbg(hw, "Error initializing identification LED\n");
		return ret_val;
	}

	
	hw_dbg(hw, "Initializing the IEEE VLAN\n");
	e1000e_clear_vfta(hw);

	
	
	if (e1000e_get_laa_state_82571(hw))
		rar_count--;
	e1000e_init_rx_addrs(hw, rar_count);

	
	hw_dbg(hw, "Zeroing the MTA\n");
	for (i = 0; i < mac->mta_reg_count; i++)
		E1000_WRITE_REG_ARRAY(hw, E1000_MTA, i, 0);

	
	ret_val = e1000_setup_link_82571(hw);

	
	reg_data = er32(TXDCTL(0));
	reg_data = (reg_data & ~E1000_TXDCTL_WTHRESH) |
		   E1000_TXDCTL_FULL_TX_DESC_WB |
		   E1000_TXDCTL_COUNT_DESC;
	ew32(TXDCTL(0), reg_data);

	
	switch (mac->type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		e1000e_enable_tx_pkt_filtering(hw);
		reg_data = er32(GCR);
		reg_data |= E1000_GCR_L1_ACT_WITHOUT_L0S_RX;
		ew32(GCR, reg_data);
		break;
	default:
		reg_data = er32(TXDCTL(1));
		reg_data = (reg_data & ~E1000_TXDCTL_WTHRESH) |
			   E1000_TXDCTL_FULL_TX_DESC_WB |
			   E1000_TXDCTL_COUNT_DESC;
		ew32(TXDCTL(1), reg_data);
		break;
	}

	
	e1000_clear_hw_cntrs_82571(hw);

	return ret_val;
}


static void e1000_initialize_hw_bits_82571(struct e1000_hw *hw)
{
	u32 reg;

	
	reg = er32(TXDCTL(0));
	reg |= (1 << 22);
	ew32(TXDCTL(0), reg);

	
	reg = er32(TXDCTL(1));
	reg |= (1 << 22);
	ew32(TXDCTL(1), reg);

	
	reg = er32(TARC(0));
	reg &= ~(0xF << 27); 
	switch (hw->mac.type) {
	case e1000_82571:
	case e1000_82572:
		reg |= (1 << 23) | (1 << 24) | (1 << 25) | (1 << 26);
		break;
	default:
		break;
	}
	ew32(TARC(0), reg);

	
	reg = er32(TARC(1));
	switch (hw->mac.type) {
	case e1000_82571:
	case e1000_82572:
		reg &= ~((1 << 29) | (1 << 30));
		reg |= (1 << 22) | (1 << 24) | (1 << 25) | (1 << 26);
		if (er32(TCTL) & E1000_TCTL_MULR)
			reg &= ~(1 << 28);
		else
			reg |= (1 << 28);
		ew32(TARC(1), reg);
		break;
	default:
		break;
	}

	
	switch (hw->mac.type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		reg = er32(CTRL);
		reg &= ~(1 << 29);
		ew32(CTRL, reg);
		break;
	default:
		break;
	}

	
	switch (hw->mac.type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		reg = er32(CTRL_EXT);
		reg &= ~(1 << 23);
		reg |= (1 << 22);
		ew32(CTRL_EXT, reg);
		break;
	default:
		break;
	}

	if (hw->mac.type == e1000_82571) {
		reg = er32(PBA_ECC);
		reg |= E1000_PBA_ECC_CORR_EN;
		ew32(PBA_ECC, reg);
	}
	

        if ((hw->mac.type == e1000_82571) ||
           (hw->mac.type == e1000_82572)) {
                reg = er32(CTRL_EXT);
                reg &= ~E1000_CTRL_EXT_DMA_DYN_CLK_EN;
                ew32(CTRL_EXT, reg);
        }


	
	switch (hw->mac.type) {
	case e1000_82574:
	case e1000_82583:
		reg = er32(GCR);
		reg |= (1 << 22);
		ew32(GCR, reg);

		reg = er32(GCR2);
		reg |= 1;
		ew32(GCR2, reg);
		break;
	default:
		break;
	}

	return;
}


void e1000e_clear_vfta(struct e1000_hw *hw)
{
	u32 offset;
	u32 vfta_value = 0;
	u32 vfta_offset = 0;
	u32 vfta_bit_in_reg = 0;

	switch (hw->mac.type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		if (hw->mng_cookie.vlan_id != 0) {
			
			vfta_offset = (hw->mng_cookie.vlan_id >>
				       E1000_VFTA_ENTRY_SHIFT) &
				      E1000_VFTA_ENTRY_MASK;
			vfta_bit_in_reg = 1 << (hw->mng_cookie.vlan_id &
					       E1000_VFTA_ENTRY_BIT_SHIFT_MASK);
		}
		break;
	default:
		break;
	}
	for (offset = 0; offset < E1000_VLAN_FILTER_TBL_SIZE; offset++) {
		
		vfta_value = (offset == vfta_offset) ? vfta_bit_in_reg : 0;
		E1000_WRITE_REG_ARRAY(hw, E1000_VFTA, offset, vfta_value);
		e1e_flush();
	}
}


static bool e1000_check_mng_mode_82574(struct e1000_hw *hw)
{
	u16 data;

	e1000_read_nvm(hw, NVM_INIT_CONTROL2_REG, 1, &data);
	return (data & E1000_NVM_INIT_CTRL2_MNGM) != 0;
}


static s32 e1000_led_on_82574(struct e1000_hw *hw)
{
	u32 ctrl;
	u32 i;

	ctrl = hw->mac.ledctl_mode2;
	if (!(E1000_STATUS_LU & er32(STATUS))) {
		
		for (i = 0; i < 4; i++)
			if (((hw->mac.ledctl_mode2 >> (i * 8)) & 0xFF) ==
			    E1000_LEDCTL_MODE_LED_ON)
				ctrl |= (E1000_LEDCTL_LED0_IVRT << (i * 8));
	}
	ew32(LEDCTL, ctrl);

	return 0;
}


static void e1000_update_mc_addr_list_82571(struct e1000_hw *hw,
					    u8 *mc_addr_list,
					    u32 mc_addr_count,
					    u32 rar_used_count,
					    u32 rar_count)
{
	if (e1000e_get_laa_state_82571(hw))
		rar_count--;

	e1000e_update_mc_addr_list_generic(hw, mc_addr_list, mc_addr_count,
					   rar_used_count, rar_count);
}


static s32 e1000_setup_link_82571(struct e1000_hw *hw)
{
	
	switch (hw->mac.type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		if (hw->fc.requested_mode == e1000_fc_default)
			hw->fc.requested_mode = e1000_fc_full;
		break;
	default:
		break;
	}

	return e1000e_setup_link(hw);
}


static s32 e1000_setup_copper_link_82571(struct e1000_hw *hw)
{
	u32 ctrl;
	u32 led_ctrl;
	s32 ret_val;

	ctrl = er32(CTRL);
	ctrl |= E1000_CTRL_SLU;
	ctrl &= ~(E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);
	ew32(CTRL, ctrl);

	switch (hw->phy.type) {
	case e1000_phy_m88:
	case e1000_phy_bm:
		ret_val = e1000e_copper_link_setup_m88(hw);
		break;
	case e1000_phy_igp_2:
		ret_val = e1000e_copper_link_setup_igp(hw);
		
		led_ctrl = er32(LEDCTL);
		led_ctrl &= IGP_ACTIVITY_LED_MASK;
		led_ctrl |= (IGP_ACTIVITY_LED_ENABLE | IGP_LED3_MODE);
		ew32(LEDCTL, led_ctrl);
		break;
	default:
		return -E1000_ERR_PHY;
		break;
	}

	if (ret_val)
		return ret_val;

	ret_val = e1000e_setup_copper_link(hw);

	return ret_val;
}


static s32 e1000_setup_fiber_serdes_link_82571(struct e1000_hw *hw)
{
	switch (hw->mac.type) {
	case e1000_82571:
	case e1000_82572:
		
		ew32(SCTL, E1000_SCTL_DISABLE_SERDES_LOOPBACK);
		break;
	default:
		break;
	}

	return e1000e_setup_fiber_serdes_link(hw);
}


static s32 e1000_check_for_serdes_link_82571(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	u32 rxcw;
	u32 ctrl;
	u32 status;
	s32 ret_val = 0;

	ctrl = er32(CTRL);
	status = er32(STATUS);
	rxcw = er32(RXCW);

	if ((rxcw & E1000_RXCW_SYNCH) && !(rxcw & E1000_RXCW_IV)) {

		
		switch (mac->serdes_link_state) {
		case e1000_serdes_link_autoneg_complete:
			if (!(status & E1000_STATUS_LU)) {
				
				mac->serdes_link_state =
				    e1000_serdes_link_autoneg_progress;
				hw_dbg(hw, "AN_UP     -> AN_PROG\n");
			}
		break;

		case e1000_serdes_link_forced_up:
			
			if (rxcw & E1000_RXCW_C) {
				
				ew32(TXCW, mac->txcw);
				ew32(CTRL,
				    (ctrl & ~E1000_CTRL_SLU));
				mac->serdes_link_state =
				    e1000_serdes_link_autoneg_progress;
				hw_dbg(hw, "FORCED_UP -> AN_PROG\n");
			}
			break;

		case e1000_serdes_link_autoneg_progress:
			
			if (status & E1000_STATUS_LU)  {
				mac->serdes_link_state =
				    e1000_serdes_link_autoneg_complete;
				hw_dbg(hw, "AN_PROG   -> AN_UP\n");
			} else {
				
				ew32(TXCW,
				    (mac->txcw & ~E1000_TXCW_ANE));
				ctrl |= (E1000_CTRL_SLU | E1000_CTRL_FD);
				ew32(CTRL, ctrl);

				
				ret_val =
				    e1000e_config_fc_after_link_up(hw);
				if (ret_val) {
					hw_dbg(hw, "Error config flow control\n");
					break;
				}
				mac->serdes_link_state =
				    e1000_serdes_link_forced_up;
				hw_dbg(hw, "AN_PROG   -> FORCED_UP\n");
			}
			mac->serdes_has_link = true;
			break;

		case e1000_serdes_link_down:
		default:
			
			ew32(TXCW, mac->txcw);
			ew32(CTRL,
			    (ctrl & ~E1000_CTRL_SLU));
			mac->serdes_link_state =
			    e1000_serdes_link_autoneg_progress;
			hw_dbg(hw, "DOWN      -> AN_PROG\n");
			break;
		}
	} else {
		if (!(rxcw & E1000_RXCW_SYNCH)) {
			mac->serdes_has_link = false;
			mac->serdes_link_state = e1000_serdes_link_down;
			hw_dbg(hw, "ANYSTATE  -> DOWN\n");
		} else {
			
			udelay(10);
			rxcw = er32(RXCW);
			if (rxcw & E1000_RXCW_IV) {
				mac->serdes_link_state = e1000_serdes_link_down;
				mac->serdes_has_link = false;
				hw_dbg(hw, "ANYSTATE  -> DOWN\n");
			}
		}
	}

	return ret_val;
}


static s32 e1000_valid_led_default_82571(struct e1000_hw *hw, u16 *data)
{
	s32 ret_val;

	ret_val = e1000_read_nvm(hw, NVM_ID_LED_SETTINGS, 1, data);
	if (ret_val) {
		hw_dbg(hw, "NVM Read Error\n");
		return ret_val;
	}

	switch (hw->mac.type) {
	case e1000_82573:
	case e1000_82574:
	case e1000_82583:
		if (*data == ID_LED_RESERVED_F746)
			*data = ID_LED_DEFAULT_82573;
		break;
	default:
		if (*data == ID_LED_RESERVED_0000 ||
		    *data == ID_LED_RESERVED_FFFF)
			*data = ID_LED_DEFAULT;
		break;
	}

	return 0;
}


bool e1000e_get_laa_state_82571(struct e1000_hw *hw)
{
	if (hw->mac.type != e1000_82571)
		return 0;

	return hw->dev_spec.e82571.laa_is_present;
}


void e1000e_set_laa_state_82571(struct e1000_hw *hw, bool state)
{
	if (hw->mac.type != e1000_82571)
		return;

	hw->dev_spec.e82571.laa_is_present = state;

	
	if (state)
		
		e1000e_rar_set(hw, hw->mac.addr, hw->mac.rar_entry_count - 1);
}


static s32 e1000_fix_nvm_checksum_82571(struct e1000_hw *hw)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	s32 ret_val;
	u16 data;

	if (nvm->type != e1000_nvm_flash_hw)
		return 0;

	
	ret_val = e1000_read_nvm(hw, 0x10, 1, &data);
	if (ret_val)
		return ret_val;

	if (!(data & 0x10)) {
		
		ret_val = e1000_read_nvm(hw, 0x23, 1, &data);
		if (ret_val)
			return ret_val;

		if (!(data & 0x8000)) {
			data |= 0x8000;
			ret_val = e1000_write_nvm(hw, 0x23, 1, &data);
			if (ret_val)
				return ret_val;
			ret_val = e1000e_update_nvm_checksum(hw);
		}
	}

	return 0;
}


static void e1000_clear_hw_cntrs_82571(struct e1000_hw *hw)
{
	u32 temp;

	e1000e_clear_hw_cntrs_base(hw);

	temp = er32(PRC64);
	temp = er32(PRC127);
	temp = er32(PRC255);
	temp = er32(PRC511);
	temp = er32(PRC1023);
	temp = er32(PRC1522);
	temp = er32(PTC64);
	temp = er32(PTC127);
	temp = er32(PTC255);
	temp = er32(PTC511);
	temp = er32(PTC1023);
	temp = er32(PTC1522);

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

	temp = er32(ICRXPTC);
	temp = er32(ICRXATC);
	temp = er32(ICTXPTC);
	temp = er32(ICTXATC);
	temp = er32(ICTXQEC);
	temp = er32(ICTXQMTC);
	temp = er32(ICRXDMTC);
}

static struct e1000_mac_operations e82571_mac_ops = {
	
	
	.id_led_init		= e1000e_id_led_init,
	.cleanup_led		= e1000e_cleanup_led_generic,
	.clear_hw_cntrs		= e1000_clear_hw_cntrs_82571,
	.get_bus_info		= e1000e_get_bus_info_pcie,
	
	
	.led_off		= e1000e_led_off_generic,
	.update_mc_addr_list	= e1000_update_mc_addr_list_82571,
	.reset_hw		= e1000_reset_hw_82571,
	.init_hw		= e1000_init_hw_82571,
	.setup_link		= e1000_setup_link_82571,
	
	.setup_led		= e1000e_setup_led_generic,
};

static struct e1000_phy_operations e82_phy_ops_igp = {
	.acquire_phy		= e1000_get_hw_semaphore_82571,
	.check_reset_block	= e1000e_check_reset_block_generic,
	.commit_phy		= NULL,
	.force_speed_duplex	= e1000e_phy_force_speed_duplex_igp,
	.get_cfg_done		= e1000_get_cfg_done_82571,
	.get_cable_length	= e1000e_get_cable_length_igp_2,
	.get_phy_info		= e1000e_get_phy_info_igp,
	.read_phy_reg		= e1000e_read_phy_reg_igp,
	.release_phy		= e1000_put_hw_semaphore_82571,
	.reset_phy		= e1000e_phy_hw_reset_generic,
	.set_d0_lplu_state	= e1000_set_d0_lplu_state_82571,
	.set_d3_lplu_state	= e1000e_set_d3_lplu_state,
	.write_phy_reg		= e1000e_write_phy_reg_igp,
	.cfg_on_link_up      	= NULL,
};

static struct e1000_phy_operations e82_phy_ops_m88 = {
	.acquire_phy		= e1000_get_hw_semaphore_82571,
	.check_reset_block	= e1000e_check_reset_block_generic,
	.commit_phy		= e1000e_phy_sw_reset,
	.force_speed_duplex	= e1000e_phy_force_speed_duplex_m88,
	.get_cfg_done		= e1000e_get_cfg_done,
	.get_cable_length	= e1000e_get_cable_length_m88,
	.get_phy_info		= e1000e_get_phy_info_m88,
	.read_phy_reg		= e1000e_read_phy_reg_m88,
	.release_phy		= e1000_put_hw_semaphore_82571,
	.reset_phy		= e1000e_phy_hw_reset_generic,
	.set_d0_lplu_state	= e1000_set_d0_lplu_state_82571,
	.set_d3_lplu_state	= e1000e_set_d3_lplu_state,
	.write_phy_reg		= e1000e_write_phy_reg_m88,
	.cfg_on_link_up      	= NULL,
};

static struct e1000_phy_operations e82_phy_ops_bm = {
	.acquire_phy		= e1000_get_hw_semaphore_82571,
	.check_reset_block	= e1000e_check_reset_block_generic,
	.commit_phy		= e1000e_phy_sw_reset,
	.force_speed_duplex	= e1000e_phy_force_speed_duplex_m88,
	.get_cfg_done		= e1000e_get_cfg_done,
	.get_cable_length	= e1000e_get_cable_length_m88,
	.get_phy_info		= e1000e_get_phy_info_m88,
	.read_phy_reg		= e1000e_read_phy_reg_bm2,
	.release_phy		= e1000_put_hw_semaphore_82571,
	.reset_phy		= e1000e_phy_hw_reset_generic,
	.set_d0_lplu_state	= e1000_set_d0_lplu_state_82571,
	.set_d3_lplu_state	= e1000e_set_d3_lplu_state,
	.write_phy_reg		= e1000e_write_phy_reg_bm2,
	.cfg_on_link_up      	= NULL,
};

static struct e1000_nvm_operations e82571_nvm_ops = {
	.acquire_nvm		= e1000_acquire_nvm_82571,
	.read_nvm		= e1000e_read_nvm_eerd,
	.release_nvm		= e1000_release_nvm_82571,
	.update_nvm		= e1000_update_nvm_checksum_82571,
	.valid_led_default	= e1000_valid_led_default_82571,
	.validate_nvm		= e1000_validate_nvm_checksum_82571,
	.write_nvm		= e1000_write_nvm_82571,
};

struct e1000_info e1000_82571_info = {
	.mac			= e1000_82571,
	.flags			= FLAG_HAS_HW_VLAN_FILTER
				  | FLAG_HAS_JUMBO_FRAMES
				  | FLAG_HAS_WOL
				  | FLAG_APME_IN_CTRL3
				  | FLAG_RX_CSUM_ENABLED
				  | FLAG_HAS_CTRLEXT_ON_LOAD
				  | FLAG_HAS_SMART_POWER_DOWN
				  | FLAG_RESET_OVERWRITES_LAA 
				  | FLAG_TARC_SPEED_MODE_BIT 
				  | FLAG_APME_CHECK_PORT_B,
	.pba			= 38,
	.max_hw_frame_size	= DEFAULT_JUMBO,
	.get_variants		= e1000_get_variants_82571,
	.mac_ops		= &e82571_mac_ops,
	.phy_ops		= &e82_phy_ops_igp,
	.nvm_ops		= &e82571_nvm_ops,
};

struct e1000_info e1000_82572_info = {
	.mac			= e1000_82572,
	.flags			= FLAG_HAS_HW_VLAN_FILTER
				  | FLAG_HAS_JUMBO_FRAMES
				  | FLAG_HAS_WOL
				  | FLAG_APME_IN_CTRL3
				  | FLAG_RX_CSUM_ENABLED
				  | FLAG_HAS_CTRLEXT_ON_LOAD
				  | FLAG_TARC_SPEED_MODE_BIT, 
	.pba			= 38,
	.max_hw_frame_size	= DEFAULT_JUMBO,
	.get_variants		= e1000_get_variants_82571,
	.mac_ops		= &e82571_mac_ops,
	.phy_ops		= &e82_phy_ops_igp,
	.nvm_ops		= &e82571_nvm_ops,
};

struct e1000_info e1000_82573_info = {
	.mac			= e1000_82573,
	.flags			= FLAG_HAS_HW_VLAN_FILTER
				  | FLAG_HAS_JUMBO_FRAMES
				  | FLAG_HAS_WOL
				  | FLAG_APME_IN_CTRL3
				  | FLAG_RX_CSUM_ENABLED
				  | FLAG_HAS_SMART_POWER_DOWN
				  | FLAG_HAS_AMT
				  | FLAG_HAS_ERT
				  | FLAG_HAS_SWSM_ON_LOAD,
	.pba			= 20,
	.max_hw_frame_size	= ETH_FRAME_LEN + ETH_FCS_LEN,
	.get_variants		= e1000_get_variants_82571,
	.mac_ops		= &e82571_mac_ops,
	.phy_ops		= &e82_phy_ops_m88,
	.nvm_ops		= &e82571_nvm_ops,
};

struct e1000_info e1000_82574_info = {
	.mac			= e1000_82574,
	.flags			= FLAG_HAS_HW_VLAN_FILTER
				  | FLAG_HAS_MSIX
				  | FLAG_HAS_JUMBO_FRAMES
				  | FLAG_HAS_WOL
				  | FLAG_APME_IN_CTRL3
				  | FLAG_RX_CSUM_ENABLED
				  | FLAG_HAS_SMART_POWER_DOWN
				  | FLAG_HAS_AMT
				  | FLAG_HAS_CTRLEXT_ON_LOAD,
	.pba			= 20,
	.max_hw_frame_size	= DEFAULT_JUMBO,
	.get_variants		= e1000_get_variants_82571,
	.mac_ops		= &e82571_mac_ops,
	.phy_ops		= &e82_phy_ops_bm,
	.nvm_ops		= &e82571_nvm_ops,
};

struct e1000_info e1000_82583_info = {
	.mac			= e1000_82583,
	.flags			= FLAG_HAS_HW_VLAN_FILTER
				  | FLAG_HAS_WOL
				  | FLAG_APME_IN_CTRL3
				  | FLAG_RX_CSUM_ENABLED
				  | FLAG_HAS_SMART_POWER_DOWN
				  | FLAG_HAS_AMT
				  | FLAG_HAS_CTRLEXT_ON_LOAD,
	.pba			= 20,
	.max_hw_frame_size	= ETH_FRAME_LEN + ETH_FCS_LEN,
	.get_variants		= e1000_get_variants_82571,
	.mac_ops		= &e82571_mac_ops,
	.phy_ops		= &e82_phy_ops_bm,
	.nvm_ops		= &e82571_nvm_ops,
};

