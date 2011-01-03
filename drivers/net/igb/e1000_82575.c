



#include <linux/types.h>
#include <linux/slab.h>
#include <linux/if_ether.h>

#include "e1000_mac.h"
#include "e1000_82575.h"

static s32  igb_get_invariants_82575(struct e1000_hw *);
static s32  igb_acquire_phy_82575(struct e1000_hw *);
static void igb_release_phy_82575(struct e1000_hw *);
static s32  igb_acquire_nvm_82575(struct e1000_hw *);
static void igb_release_nvm_82575(struct e1000_hw *);
static s32  igb_check_for_link_82575(struct e1000_hw *);
static s32  igb_get_cfg_done_82575(struct e1000_hw *);
static s32  igb_init_hw_82575(struct e1000_hw *);
static s32  igb_phy_hw_reset_sgmii_82575(struct e1000_hw *);
static s32  igb_read_phy_reg_sgmii_82575(struct e1000_hw *, u32, u16 *);
static s32  igb_reset_hw_82575(struct e1000_hw *);
static s32  igb_set_d0_lplu_state_82575(struct e1000_hw *, bool);
static s32  igb_setup_copper_link_82575(struct e1000_hw *);
static s32  igb_setup_serdes_link_82575(struct e1000_hw *);
static s32  igb_write_phy_reg_sgmii_82575(struct e1000_hw *, u32, u16);
static void igb_clear_hw_cntrs_82575(struct e1000_hw *);
static s32  igb_acquire_swfw_sync_82575(struct e1000_hw *, u16);
static s32  igb_get_pcs_speed_and_duplex_82575(struct e1000_hw *, u16 *,
						 u16 *);
static s32  igb_get_phy_id_82575(struct e1000_hw *);
static void igb_release_swfw_sync_82575(struct e1000_hw *, u16);
static bool igb_sgmii_active_82575(struct e1000_hw *);
static s32  igb_reset_init_script_82575(struct e1000_hw *);
static s32  igb_read_mac_addr_82575(struct e1000_hw *);
static s32  igb_set_pcie_completion_timeout(struct e1000_hw *hw);

static s32 igb_get_invariants_82575(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	struct e1000_nvm_info *nvm = &hw->nvm;
	struct e1000_mac_info *mac = &hw->mac;
	struct e1000_dev_spec_82575 * dev_spec = &hw->dev_spec._82575;
	u32 eecd;
	s32 ret_val;
	u16 size;
	u32 ctrl_ext = 0;

	switch (hw->device_id) {
	case E1000_DEV_ID_82575EB_COPPER:
	case E1000_DEV_ID_82575EB_FIBER_SERDES:
	case E1000_DEV_ID_82575GB_QUAD_COPPER:
		mac->type = e1000_82575;
		break;
	case E1000_DEV_ID_82576:
	case E1000_DEV_ID_82576_NS:
	case E1000_DEV_ID_82576_FIBER:
	case E1000_DEV_ID_82576_SERDES:
	case E1000_DEV_ID_82576_QUAD_COPPER:
	case E1000_DEV_ID_82576_SERDES_QUAD:
		mac->type = e1000_82576;
		break;
	default:
		return -E1000_ERR_MAC_INIT;
		break;
	}

	
	
	phy->media_type = e1000_media_type_copper;
	dev_spec->sgmii_active = false;

	ctrl_ext = rd32(E1000_CTRL_EXT);
	switch (ctrl_ext & E1000_CTRL_EXT_LINK_MODE_MASK) {
	case E1000_CTRL_EXT_LINK_MODE_SGMII:
		dev_spec->sgmii_active = true;
		ctrl_ext |= E1000_CTRL_I2C_ENA;
		break;
	case E1000_CTRL_EXT_LINK_MODE_PCIE_SERDES:
		hw->phy.media_type = e1000_media_type_internal_serdes;
		ctrl_ext |= E1000_CTRL_I2C_ENA;
		break;
	default:
		ctrl_ext &= ~E1000_CTRL_I2C_ENA;
		break;
	}

	wr32(E1000_CTRL_EXT, ctrl_ext);

	
	mac->mta_reg_count = 128;
	
	mac->rar_entry_count = E1000_RAR_ENTRIES_82575;
	if (mac->type == e1000_82576)
		mac->rar_entry_count = E1000_RAR_ENTRIES_82576;
	
	mac->asf_firmware_present = true;
	
	mac->arc_subsystem_valid =
		(rd32(E1000_FWSM) & E1000_FWSM_MODE_MASK)
			? true : false;

	
	mac->ops.setup_physical_interface =
		(hw->phy.media_type == e1000_media_type_copper)
			? igb_setup_copper_link_82575
			: igb_setup_serdes_link_82575;

	
	eecd = rd32(E1000_EECD);

	nvm->opcode_bits        = 8;
	nvm->delay_usec         = 1;
	switch (nvm->override) {
	case e1000_nvm_override_spi_large:
		nvm->page_size    = 32;
		nvm->address_bits = 16;
		break;
	case e1000_nvm_override_spi_small:
		nvm->page_size    = 8;
		nvm->address_bits = 8;
		break;
	default:
		nvm->page_size    = eecd & E1000_EECD_ADDR_BITS ? 32 : 8;
		nvm->address_bits = eecd & E1000_EECD_ADDR_BITS ? 16 : 8;
		break;
	}

	nvm->type = e1000_nvm_eeprom_spi;

	size = (u16)((eecd & E1000_EECD_SIZE_EX_MASK) >>
		     E1000_EECD_SIZE_EX_SHIFT);

	
	size += NVM_WORD_SIZE_BASE_SHIFT;

	
	if (size > 14)
		size = 14;
	nvm->word_size = 1 << size;

	
	if (mac->type == e1000_82576)
		igb_init_mbx_params_pf(hw);

	
	if (phy->media_type != e1000_media_type_copper) {
		phy->type = e1000_phy_none;
		return 0;
	}

	phy->autoneg_mask        = AUTONEG_ADVERTISE_SPEED_DEFAULT;
	phy->reset_delay_us      = 100;

	
	if (igb_sgmii_active_82575(hw)) {
		phy->ops.reset              = igb_phy_hw_reset_sgmii_82575;
		phy->ops.read_reg           = igb_read_phy_reg_sgmii_82575;
		phy->ops.write_reg          = igb_write_phy_reg_sgmii_82575;
	} else {
		phy->ops.reset              = igb_phy_hw_reset;
		phy->ops.read_reg           = igb_read_phy_reg_igp;
		phy->ops.write_reg          = igb_write_phy_reg_igp;
	}

	
	hw->bus.func = (rd32(E1000_STATUS) & E1000_STATUS_FUNC_MASK) >>
	               E1000_STATUS_FUNC_SHIFT;

	
	ret_val = igb_get_phy_id_82575(hw);
	if (ret_val)
		return ret_val;

	
	switch (phy->id) {
	case M88E1111_I_PHY_ID:
		phy->type                   = e1000_phy_m88;
		phy->ops.get_phy_info       = igb_get_phy_info_m88;
		phy->ops.get_cable_length   = igb_get_cable_length_m88;
		phy->ops.force_speed_duplex = igb_phy_force_speed_duplex_m88;
		break;
	case IGP03E1000_E_PHY_ID:
		phy->type                   = e1000_phy_igp_3;
		phy->ops.get_phy_info       = igb_get_phy_info_igp;
		phy->ops.get_cable_length   = igb_get_cable_length_igp_2;
		phy->ops.force_speed_duplex = igb_phy_force_speed_duplex_igp;
		phy->ops.set_d0_lplu_state  = igb_set_d0_lplu_state_82575;
		phy->ops.set_d3_lplu_state  = igb_set_d3_lplu_state;
		break;
	default:
		return -E1000_ERR_PHY;
	}

	return 0;
}


static s32 igb_acquire_phy_82575(struct e1000_hw *hw)
{
	u16 mask;

	mask = hw->bus.func ? E1000_SWFW_PHY1_SM : E1000_SWFW_PHY0_SM;

	return igb_acquire_swfw_sync_82575(hw, mask);
}


static void igb_release_phy_82575(struct e1000_hw *hw)
{
	u16 mask;

	mask = hw->bus.func ? E1000_SWFW_PHY1_SM : E1000_SWFW_PHY0_SM;
	igb_release_swfw_sync_82575(hw, mask);
}


static s32 igb_read_phy_reg_sgmii_82575(struct e1000_hw *hw, u32 offset,
					  u16 *data)
{
	struct e1000_phy_info *phy = &hw->phy;
	u32 i, i2ccmd = 0;

	if (offset > E1000_MAX_SGMII_PHY_REG_ADDR) {
		hw_dbg("PHY Address %u is out of range\n", offset);
		return -E1000_ERR_PARAM;
	}

	
	i2ccmd = ((offset << E1000_I2CCMD_REG_ADDR_SHIFT) |
		  (phy->addr << E1000_I2CCMD_PHY_ADDR_SHIFT) |
		  (E1000_I2CCMD_OPCODE_READ));

	wr32(E1000_I2CCMD, i2ccmd);

	
	for (i = 0; i < E1000_I2CCMD_PHY_TIMEOUT; i++) {
		udelay(50);
		i2ccmd = rd32(E1000_I2CCMD);
		if (i2ccmd & E1000_I2CCMD_READY)
			break;
	}
	if (!(i2ccmd & E1000_I2CCMD_READY)) {
		hw_dbg("I2CCMD Read did not complete\n");
		return -E1000_ERR_PHY;
	}
	if (i2ccmd & E1000_I2CCMD_ERROR) {
		hw_dbg("I2CCMD Error bit set\n");
		return -E1000_ERR_PHY;
	}

	
	*data = ((i2ccmd >> 8) & 0x00FF) | ((i2ccmd << 8) & 0xFF00);

	return 0;
}


static s32 igb_write_phy_reg_sgmii_82575(struct e1000_hw *hw, u32 offset,
					   u16 data)
{
	struct e1000_phy_info *phy = &hw->phy;
	u32 i, i2ccmd = 0;
	u16 phy_data_swapped;

	if (offset > E1000_MAX_SGMII_PHY_REG_ADDR) {
		hw_dbg("PHY Address %d is out of range\n", offset);
		return -E1000_ERR_PARAM;
	}

	
	phy_data_swapped = ((data >> 8) & 0x00FF) | ((data << 8) & 0xFF00);

	
	i2ccmd = ((offset << E1000_I2CCMD_REG_ADDR_SHIFT) |
		  (phy->addr << E1000_I2CCMD_PHY_ADDR_SHIFT) |
		  E1000_I2CCMD_OPCODE_WRITE |
		  phy_data_swapped);

	wr32(E1000_I2CCMD, i2ccmd);

	
	for (i = 0; i < E1000_I2CCMD_PHY_TIMEOUT; i++) {
		udelay(50);
		i2ccmd = rd32(E1000_I2CCMD);
		if (i2ccmd & E1000_I2CCMD_READY)
			break;
	}
	if (!(i2ccmd & E1000_I2CCMD_READY)) {
		hw_dbg("I2CCMD Write did not complete\n");
		return -E1000_ERR_PHY;
	}
	if (i2ccmd & E1000_I2CCMD_ERROR) {
		hw_dbg("I2CCMD Error bit set\n");
		return -E1000_ERR_PHY;
	}

	return 0;
}


static s32 igb_get_phy_id_82575(struct e1000_hw *hw)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32  ret_val = 0;
	u16 phy_id;
	u32 ctrl_ext;

	
	if (!(igb_sgmii_active_82575(hw))) {
		phy->addr = 1;
		ret_val = igb_get_phy_id(hw);
		goto out;
	}

	
	ctrl_ext = rd32(E1000_CTRL_EXT);
	wr32(E1000_CTRL_EXT, ctrl_ext & ~E1000_CTRL_EXT_SDP3_DATA);
	wrfl();
	msleep(300);

	
	for (phy->addr = 1; phy->addr < 8; phy->addr++) {
		ret_val = igb_read_phy_reg_sgmii_82575(hw, PHY_ID1, &phy_id);
		if (ret_val == 0) {
			hw_dbg("Vendor ID 0x%08X read at address %u\n",
			       phy_id, phy->addr);
			
			if (phy_id == M88_VENDOR)
				break;
		} else {
			hw_dbg("PHY address %u was unreadable\n", phy->addr);
		}
	}

	
	if (phy->addr == 8) {
		phy->addr = 0;
		ret_val = -E1000_ERR_PHY;
		goto out;
	} else {
		ret_val = igb_get_phy_id(hw);
	}

	
	wr32(E1000_CTRL_EXT, ctrl_ext);

out:
	return ret_val;
}


static s32 igb_phy_hw_reset_sgmii_82575(struct e1000_hw *hw)
{
	s32 ret_val;

	

	hw_dbg("Soft resetting SGMII attached PHY...\n");

	
	ret_val = hw->phy.ops.write_reg(hw, 0x1B, 0x8084);
	if (ret_val)
		goto out;

	ret_val = igb_phy_sw_reset(hw);

out:
	return ret_val;
}


static s32 igb_set_d0_lplu_state_82575(struct e1000_hw *hw, bool active)
{
	struct e1000_phy_info *phy = &hw->phy;
	s32 ret_val;
	u16 data;

	ret_val = phy->ops.read_reg(hw, IGP02E1000_PHY_POWER_MGMT, &data);
	if (ret_val)
		goto out;

	if (active) {
		data |= IGP02E1000_PM_D0_LPLU;
		ret_val = phy->ops.write_reg(hw, IGP02E1000_PHY_POWER_MGMT,
						 data);
		if (ret_val)
			goto out;

		
		ret_val = phy->ops.read_reg(hw, IGP01E1000_PHY_PORT_CONFIG,
						&data);
		data &= ~IGP01E1000_PSCFR_SMART_SPEED;
		ret_val = phy->ops.write_reg(hw, IGP01E1000_PHY_PORT_CONFIG,
						 data);
		if (ret_val)
			goto out;
	} else {
		data &= ~IGP02E1000_PM_D0_LPLU;
		ret_val = phy->ops.write_reg(hw, IGP02E1000_PHY_POWER_MGMT,
						 data);
		
		if (phy->smart_speed == e1000_smart_speed_on) {
			ret_val = phy->ops.read_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, &data);
			if (ret_val)
				goto out;

			data |= IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = phy->ops.write_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, data);
			if (ret_val)
				goto out;
		} else if (phy->smart_speed == e1000_smart_speed_off) {
			ret_val = phy->ops.read_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, &data);
			if (ret_val)
				goto out;

			data &= ~IGP01E1000_PSCFR_SMART_SPEED;
			ret_val = phy->ops.write_reg(hw,
					IGP01E1000_PHY_PORT_CONFIG, data);
			if (ret_val)
				goto out;
		}
	}

out:
	return ret_val;
}


static s32 igb_acquire_nvm_82575(struct e1000_hw *hw)
{
	s32 ret_val;

	ret_val = igb_acquire_swfw_sync_82575(hw, E1000_SWFW_EEP_SM);
	if (ret_val)
		goto out;

	ret_val = igb_acquire_nvm(hw);

	if (ret_val)
		igb_release_swfw_sync_82575(hw, E1000_SWFW_EEP_SM);

out:
	return ret_val;
}


static void igb_release_nvm_82575(struct e1000_hw *hw)
{
	igb_release_nvm(hw);
	igb_release_swfw_sync_82575(hw, E1000_SWFW_EEP_SM);
}


static s32 igb_acquire_swfw_sync_82575(struct e1000_hw *hw, u16 mask)
{
	u32 swfw_sync;
	u32 swmask = mask;
	u32 fwmask = mask << 16;
	s32 ret_val = 0;
	s32 i = 0, timeout = 200; 

	while (i < timeout) {
		if (igb_get_hw_semaphore(hw)) {
			ret_val = -E1000_ERR_SWFW_SYNC;
			goto out;
		}

		swfw_sync = rd32(E1000_SW_FW_SYNC);
		if (!(swfw_sync & (fwmask | swmask)))
			break;

		
		igb_put_hw_semaphore(hw);
		mdelay(5);
		i++;
	}

	if (i == timeout) {
		hw_dbg("Driver can't access resource, SW_FW_SYNC timeout.\n");
		ret_val = -E1000_ERR_SWFW_SYNC;
		goto out;
	}

	swfw_sync |= swmask;
	wr32(E1000_SW_FW_SYNC, swfw_sync);

	igb_put_hw_semaphore(hw);

out:
	return ret_val;
}


static void igb_release_swfw_sync_82575(struct e1000_hw *hw, u16 mask)
{
	u32 swfw_sync;

	while (igb_get_hw_semaphore(hw) != 0);
	

	swfw_sync = rd32(E1000_SW_FW_SYNC);
	swfw_sync &= ~mask;
	wr32(E1000_SW_FW_SYNC, swfw_sync);

	igb_put_hw_semaphore(hw);
}


static s32 igb_get_cfg_done_82575(struct e1000_hw *hw)
{
	s32 timeout = PHY_CFG_TIMEOUT;
	s32 ret_val = 0;
	u32 mask = E1000_NVM_CFG_DONE_PORT_0;

	if (hw->bus.func == 1)
		mask = E1000_NVM_CFG_DONE_PORT_1;

	while (timeout) {
		if (rd32(E1000_EEMNGCTL) & mask)
			break;
		msleep(1);
		timeout--;
	}
	if (!timeout)
		hw_dbg("MNG configuration cycle has not completed.\n");

	
	if (((rd32(E1000_EECD) & E1000_EECD_PRES) == 0) &&
	    (hw->phy.type == e1000_phy_igp_3))
		igb_phy_init_script_igp3(hw);

	return ret_val;
}


static s32 igb_check_for_link_82575(struct e1000_hw *hw)
{
	s32 ret_val;
	u16 speed, duplex;

	
	if ((hw->phy.media_type != e1000_media_type_copper) ||
	    (igb_sgmii_active_82575(hw))) {
		ret_val = igb_get_pcs_speed_and_duplex_82575(hw, &speed,
		                                             &duplex);
		
		hw->mac.get_link_status = !hw->mac.serdes_has_link;
	} else {
		ret_val = igb_check_for_copper_link(hw);
	}

	return ret_val;
}

static s32 igb_get_pcs_speed_and_duplex_82575(struct e1000_hw *hw, u16 *speed,
						u16 *duplex)
{
	struct e1000_mac_info *mac = &hw->mac;
	u32 pcs;

	
	mac->serdes_has_link = false;
	*speed = 0;
	*duplex = 0;

	
	pcs = rd32(E1000_PCS_LSTAT);

	
	if ((pcs & E1000_PCS_LSTS_LINK_OK) && (pcs & E1000_PCS_LSTS_SYNK_OK)) {
		mac->serdes_has_link = true;

		
		if (pcs & E1000_PCS_LSTS_SPEED_1000) {
			*speed = SPEED_1000;
		} else if (pcs & E1000_PCS_LSTS_SPEED_100) {
			*speed = SPEED_100;
		} else {
			*speed = SPEED_10;
		}

		
		if (pcs & E1000_PCS_LSTS_DUPLEX_FULL) {
			*duplex = FULL_DUPLEX;
		} else {
			*duplex = HALF_DUPLEX;
		}
	}

	return 0;
}


void igb_shutdown_serdes_link_82575(struct e1000_hw *hw)
{
	u32 reg;

	if (hw->phy.media_type != e1000_media_type_internal_serdes ||
	    igb_sgmii_active_82575(hw))
		return;

	
	if (!igb_enable_mng_pass_thru(hw)) {
		
		reg = rd32(E1000_PCS_CFG0);
		reg &= ~E1000_PCS_CFG_PCS_EN;
		wr32(E1000_PCS_CFG0, reg);

		
		reg = rd32(E1000_CTRL_EXT);
		reg |= E1000_CTRL_EXT_SDP3_DATA;
		wr32(E1000_CTRL_EXT, reg);

		
		wrfl();
		msleep(1);
	}

	return;
}


static s32 igb_reset_hw_82575(struct e1000_hw *hw)
{
	u32 ctrl, icr;
	s32 ret_val;

	
	ret_val = igb_disable_pcie_master(hw);
	if (ret_val)
		hw_dbg("PCI-E Master disable polling has failed.\n");

	
	ret_val = igb_set_pcie_completion_timeout(hw);
	if (ret_val) {
		hw_dbg("PCI-E Set completion timeout has failed.\n");
	}

	hw_dbg("Masking off all interrupts\n");
	wr32(E1000_IMC, 0xffffffff);

	wr32(E1000_RCTL, 0);
	wr32(E1000_TCTL, E1000_TCTL_PSP);
	wrfl();

	msleep(10);

	ctrl = rd32(E1000_CTRL);

	hw_dbg("Issuing a global reset to MAC\n");
	wr32(E1000_CTRL, ctrl | E1000_CTRL_RST);

	ret_val = igb_get_auto_rd_done(hw);
	if (ret_val) {
		
		hw_dbg("Auto Read Done did not complete\n");
	}

	
	if ((rd32(E1000_EECD) & E1000_EECD_PRES) == 0)
		igb_reset_init_script_82575(hw);

	
	wr32(E1000_IMC, 0xffffffff);
	icr = rd32(E1000_ICR);

	
	ret_val = igb_check_alt_mac_addr(hw);

	return ret_val;
}


static s32 igb_init_hw_82575(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val;
	u16 i, rar_count = mac->rar_entry_count;

	
	ret_val = igb_id_led_init(hw);
	if (ret_val) {
		hw_dbg("Error initializing identification LED\n");
		
	}

	
	hw_dbg("Initializing the IEEE VLAN\n");
	igb_clear_vfta(hw);

	
	igb_init_rx_addrs(hw, rar_count);

	
	hw_dbg("Zeroing the MTA\n");
	for (i = 0; i < mac->mta_reg_count; i++)
		array_wr32(E1000_MTA, i, 0);

	
	ret_val = igb_setup_link(hw);

	
	igb_clear_hw_cntrs_82575(hw);

	return ret_val;
}


static s32 igb_setup_copper_link_82575(struct e1000_hw *hw)
{
	u32 ctrl;
	s32  ret_val;
	bool link;

	ctrl = rd32(E1000_CTRL);
	ctrl |= E1000_CTRL_SLU;
	ctrl &= ~(E1000_CTRL_FRCSPD | E1000_CTRL_FRCDPX);
	wr32(E1000_CTRL, ctrl);

	ret_val = igb_setup_serdes_link_82575(hw);
	if (ret_val)
		goto out;

	if (igb_sgmii_active_82575(hw) && !hw->phy.reset_disable) {
		ret_val = hw->phy.ops.reset(hw);
		if (ret_val) {
			hw_dbg("Error resetting the PHY.\n");
			goto out;
		}
	}
	switch (hw->phy.type) {
	case e1000_phy_m88:
		ret_val = igb_copper_link_setup_m88(hw);
		break;
	case e1000_phy_igp_3:
		ret_val = igb_copper_link_setup_igp(hw);
		break;
	default:
		ret_val = -E1000_ERR_PHY;
		break;
	}

	if (ret_val)
		goto out;

	if (hw->mac.autoneg) {
		
		ret_val = igb_copper_link_autoneg(hw);
		if (ret_val)
			goto out;
	} else {
		
		hw_dbg("Forcing Speed and Duplex\n");
		ret_val = hw->phy.ops.force_speed_duplex(hw);
		if (ret_val) {
			hw_dbg("Error Forcing Speed and Duplex\n");
			goto out;
		}
	}

	
	ret_val = igb_phy_has_link(hw, COPPER_LINK_UP_LIMIT, 10, &link);
	if (ret_val)
		goto out;

	if (link) {
		hw_dbg("Valid link established!!!\n");
		
		igb_config_collision_dist(hw);
		ret_val = igb_config_fc_after_link_up(hw);
	} else {
		hw_dbg("Unable to establish link!!!\n");
	}

out:
	return ret_val;
}


static s32 igb_setup_serdes_link_82575(struct e1000_hw *hw)
{
	u32 ctrl_reg, reg;

	if ((hw->phy.media_type != e1000_media_type_internal_serdes) &&
	    !igb_sgmii_active_82575(hw))
		return 0;

	
	wr32(E1000_SCTL, E1000_SCTL_DISABLE_SERDES_LOOPBACK);

	
	reg = rd32(E1000_CTRL_EXT);
	reg &= ~E1000_CTRL_EXT_SDP3_DATA;
	wr32(E1000_CTRL_EXT, reg);

	ctrl_reg = rd32(E1000_CTRL);
	ctrl_reg |= E1000_CTRL_SLU;

	if (hw->mac.type == e1000_82575 || hw->mac.type == e1000_82576) {
		
		ctrl_reg |= E1000_CTRL_SWDPIN0 | E1000_CTRL_SWDPIN1;

		
		reg = rd32(E1000_CONNSW);
		reg |= E1000_CONNSW_ENRGSRC;
		wr32(E1000_CONNSW, reg);
	}

	reg = rd32(E1000_PCS_LCTL);

	if (igb_sgmii_active_82575(hw)) {
		
		msleep(300);

		
		reg &= ~(E1000_PCS_LCTL_AN_TIMEOUT);
	} else {
		ctrl_reg |= E1000_CTRL_SPD_1000 | E1000_CTRL_FRCSPD |
		            E1000_CTRL_FD | E1000_CTRL_FRCDPX;
	}

	wr32(E1000_CTRL, ctrl_reg);

	

	reg &= ~(E1000_PCS_LCTL_AN_ENABLE | E1000_PCS_LCTL_FLV_LINK_UP |
		E1000_PCS_LCTL_FSD | E1000_PCS_LCTL_FORCE_LINK);

	
	reg |= E1000_PCS_LCTL_FORCE_FCTRL;

	
	if (hw->mac.autoneg || igb_sgmii_active_82575(hw)) {
		
		reg |= E1000_PCS_LCTL_FSV_1000 |      
		       E1000_PCS_LCTL_FDV_FULL |      
		       E1000_PCS_LCTL_AN_ENABLE |     
		       E1000_PCS_LCTL_AN_RESTART;     
		hw_dbg("Configuring Autoneg; PCS_LCTL = 0x%08X\n", reg);
	} else {
		
		reg |= E1000_PCS_LCTL_FLV_LINK_UP |   
		       E1000_PCS_LCTL_FSV_1000 |      
		       E1000_PCS_LCTL_FDV_FULL |      
		       E1000_PCS_LCTL_FSD |           
		       E1000_PCS_LCTL_FORCE_LINK;     
		hw_dbg("Configuring Forced Link; PCS_LCTL = 0x%08X\n", reg);
	}

	wr32(E1000_PCS_LCTL, reg);

	if (!igb_sgmii_active_82575(hw))
		igb_force_mac_fc(hw);

	return 0;
}


static bool igb_sgmii_active_82575(struct e1000_hw *hw)
{
	struct e1000_dev_spec_82575 *dev_spec = &hw->dev_spec._82575;
	return dev_spec->sgmii_active;
}


static s32 igb_reset_init_script_82575(struct e1000_hw *hw)
{
	if (hw->mac.type == e1000_82575) {
		hw_dbg("Running reset init script for 82575\n");
		
		igb_write_8bit_ctrl_reg(hw, E1000_SCTL, 0x00, 0x0C);
		igb_write_8bit_ctrl_reg(hw, E1000_SCTL, 0x01, 0x78);
		igb_write_8bit_ctrl_reg(hw, E1000_SCTL, 0x1B, 0x23);
		igb_write_8bit_ctrl_reg(hw, E1000_SCTL, 0x23, 0x15);

		
		igb_write_8bit_ctrl_reg(hw, E1000_CCMCTL, 0x14, 0x00);
		igb_write_8bit_ctrl_reg(hw, E1000_CCMCTL, 0x10, 0x00);

		
		igb_write_8bit_ctrl_reg(hw, E1000_GIOCTL, 0x00, 0xEC);
		igb_write_8bit_ctrl_reg(hw, E1000_GIOCTL, 0x61, 0xDF);
		igb_write_8bit_ctrl_reg(hw, E1000_GIOCTL, 0x34, 0x05);
		igb_write_8bit_ctrl_reg(hw, E1000_GIOCTL, 0x2F, 0x81);

		
		igb_write_8bit_ctrl_reg(hw, E1000_SCCTL, 0x02, 0x47);
		igb_write_8bit_ctrl_reg(hw, E1000_SCCTL, 0x14, 0x00);
		igb_write_8bit_ctrl_reg(hw, E1000_SCCTL, 0x10, 0x00);
	}

	return 0;
}


static s32 igb_read_mac_addr_82575(struct e1000_hw *hw)
{
	s32 ret_val = 0;

	if (igb_check_alt_mac_addr(hw))
		ret_val = igb_read_mac_addr(hw);

	return ret_val;
}


static void igb_clear_hw_cntrs_82575(struct e1000_hw *hw)
{
	u32 temp;

	igb_clear_hw_cntrs_base(hw);

	temp = rd32(E1000_PRC64);
	temp = rd32(E1000_PRC127);
	temp = rd32(E1000_PRC255);
	temp = rd32(E1000_PRC511);
	temp = rd32(E1000_PRC1023);
	temp = rd32(E1000_PRC1522);
	temp = rd32(E1000_PTC64);
	temp = rd32(E1000_PTC127);
	temp = rd32(E1000_PTC255);
	temp = rd32(E1000_PTC511);
	temp = rd32(E1000_PTC1023);
	temp = rd32(E1000_PTC1522);

	temp = rd32(E1000_ALGNERRC);
	temp = rd32(E1000_RXERRC);
	temp = rd32(E1000_TNCRS);
	temp = rd32(E1000_CEXTERR);
	temp = rd32(E1000_TSCTC);
	temp = rd32(E1000_TSCTFC);

	temp = rd32(E1000_MGTPRC);
	temp = rd32(E1000_MGTPDC);
	temp = rd32(E1000_MGTPTC);

	temp = rd32(E1000_IAC);
	temp = rd32(E1000_ICRXOC);

	temp = rd32(E1000_ICRXPTC);
	temp = rd32(E1000_ICRXATC);
	temp = rd32(E1000_ICTXPTC);
	temp = rd32(E1000_ICTXATC);
	temp = rd32(E1000_ICTXQEC);
	temp = rd32(E1000_ICTXQMTC);
	temp = rd32(E1000_ICRXDMTC);

	temp = rd32(E1000_CBTMPC);
	temp = rd32(E1000_HTDPMC);
	temp = rd32(E1000_CBRMPC);
	temp = rd32(E1000_RPTHC);
	temp = rd32(E1000_HGPTC);
	temp = rd32(E1000_HTCBDPC);
	temp = rd32(E1000_HGORCL);
	temp = rd32(E1000_HGORCH);
	temp = rd32(E1000_HGOTCL);
	temp = rd32(E1000_HGOTCH);
	temp = rd32(E1000_LENERRS);

	
	if (hw->phy.media_type == e1000_media_type_internal_serdes ||
	    igb_sgmii_active_82575(hw))
		temp = rd32(E1000_SCVPC);
}


void igb_rx_fifo_flush_82575(struct e1000_hw *hw)
{
	u32 rctl, rlpml, rxdctl[4], rfctl, temp_rctl, rx_enabled;
	int i, ms_wait;

	if (hw->mac.type != e1000_82575 ||
	    !(rd32(E1000_MANC) & E1000_MANC_RCV_TCO_EN))
		return;

	
	for (i = 0; i < 4; i++) {
		rxdctl[i] = rd32(E1000_RXDCTL(i));
		wr32(E1000_RXDCTL(i),
		     rxdctl[i] & ~E1000_RXDCTL_QUEUE_ENABLE);
	}
	
	for (ms_wait = 0; ms_wait < 10; ms_wait++) {
		msleep(1);
		rx_enabled = 0;
		for (i = 0; i < 4; i++)
			rx_enabled |= rd32(E1000_RXDCTL(i));
		if (!(rx_enabled & E1000_RXDCTL_QUEUE_ENABLE))
			break;
	}

	if (ms_wait == 10)
		hw_dbg("Queue disable timed out after 10ms\n");

	
	rfctl = rd32(E1000_RFCTL);
	wr32(E1000_RFCTL, rfctl & ~E1000_RFCTL_LEF);

	rlpml = rd32(E1000_RLPML);
	wr32(E1000_RLPML, 0);

	rctl = rd32(E1000_RCTL);
	temp_rctl = rctl & ~(E1000_RCTL_EN | E1000_RCTL_SBP);
	temp_rctl |= E1000_RCTL_LPE;

	wr32(E1000_RCTL, temp_rctl);
	wr32(E1000_RCTL, temp_rctl | E1000_RCTL_EN);
	wrfl();
	msleep(2);

	
	for (i = 0; i < 4; i++)
		wr32(E1000_RXDCTL(i), rxdctl[i]);
	wr32(E1000_RCTL, rctl);
	wrfl();

	wr32(E1000_RLPML, rlpml);
	wr32(E1000_RFCTL, rfctl);

	
	rd32(E1000_ROC);
	rd32(E1000_RNBC);
	rd32(E1000_MPC);
}


static s32 igb_set_pcie_completion_timeout(struct e1000_hw *hw)
{
	u32 gcr = rd32(E1000_GCR);
	s32 ret_val = 0;
	u16 pcie_devctl2;

	
	if (gcr & E1000_GCR_CMPL_TMOUT_MASK)
		goto out;

	
	if (!(gcr & E1000_GCR_CAP_VER2)) {
		gcr |= E1000_GCR_CMPL_TMOUT_10ms;
		goto out;
	}

	
	ret_val = igb_read_pcie_cap_reg(hw, PCIE_DEVICE_CONTROL2,
	                                &pcie_devctl2);
	if (ret_val)
		goto out;

	pcie_devctl2 |= PCIE_DEVICE_CONTROL2_16ms;

	ret_val = igb_write_pcie_cap_reg(hw, PCIE_DEVICE_CONTROL2,
	                                 &pcie_devctl2);
out:
	
	gcr &= ~E1000_GCR_CMPL_TMOUT_RESEND;

	wr32(E1000_GCR, gcr);
	return ret_val;
}


void igb_vmdq_set_loopback_pf(struct e1000_hw *hw, bool enable)
{
	u32 dtxswc = rd32(E1000_DTXSWC);

	if (enable)
		dtxswc |= E1000_DTXSWC_VMDQ_LOOPBACK_EN;
	else
		dtxswc &= ~E1000_DTXSWC_VMDQ_LOOPBACK_EN;

	wr32(E1000_DTXSWC, dtxswc);
}


void igb_vmdq_set_replication_pf(struct e1000_hw *hw, bool enable)
{
	u32 vt_ctl = rd32(E1000_VT_CTL);

	if (enable)
		vt_ctl |= E1000_VT_CTL_VM_REPL_EN;
	else
		vt_ctl &= ~E1000_VT_CTL_VM_REPL_EN;

	wr32(E1000_VT_CTL, vt_ctl);
}

static struct e1000_mac_operations e1000_mac_ops_82575 = {
	.reset_hw             = igb_reset_hw_82575,
	.init_hw              = igb_init_hw_82575,
	.check_for_link       = igb_check_for_link_82575,
	.rar_set              = igb_rar_set,
	.read_mac_addr        = igb_read_mac_addr_82575,
	.get_speed_and_duplex = igb_get_speed_and_duplex_copper,
};

static struct e1000_phy_operations e1000_phy_ops_82575 = {
	.acquire              = igb_acquire_phy_82575,
	.get_cfg_done         = igb_get_cfg_done_82575,
	.release              = igb_release_phy_82575,
};

static struct e1000_nvm_operations e1000_nvm_ops_82575 = {
	.acquire              = igb_acquire_nvm_82575,
	.read                 = igb_read_nvm_eerd,
	.release              = igb_release_nvm_82575,
	.write                = igb_write_nvm_spi,
};

const struct e1000_info e1000_82575_info = {
	.get_invariants = igb_get_invariants_82575,
	.mac_ops = &e1000_mac_ops_82575,
	.phy_ops = &e1000_phy_ops_82575,
	.nvm_ops = &e1000_nvm_ops_82575,
};

