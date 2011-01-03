

#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/delay.h>
#include <linux/pci.h>

#include "e1000.h"

enum e1000_mng_mode {
	e1000_mng_mode_none = 0,
	e1000_mng_mode_asf,
	e1000_mng_mode_pt,
	e1000_mng_mode_ipmi,
	e1000_mng_mode_host_if_only
};

#define E1000_FACTPS_MNGCG		0x20000000


#define E1000_IAMT_SIGNATURE		0x544D4149


s32 e1000e_get_bus_info_pcie(struct e1000_hw *hw)
{
	struct e1000_bus_info *bus = &hw->bus;
	struct e1000_adapter *adapter = hw->adapter;
	u32 status;
	u16 pcie_link_status, pci_header_type, cap_offset;

	cap_offset = pci_find_capability(adapter->pdev, PCI_CAP_ID_EXP);
	if (!cap_offset) {
		bus->width = e1000_bus_width_unknown;
	} else {
		pci_read_config_word(adapter->pdev,
				     cap_offset + PCIE_LINK_STATUS,
				     &pcie_link_status);
		bus->width = (enum e1000_bus_width)((pcie_link_status &
						     PCIE_LINK_WIDTH_MASK) >>
						    PCIE_LINK_WIDTH_SHIFT);
	}

	pci_read_config_word(adapter->pdev, PCI_HEADER_TYPE_REGISTER,
			     &pci_header_type);
	if (pci_header_type & PCI_HEADER_TYPE_MULTIFUNC) {
		status = er32(STATUS);
		bus->func = (status & E1000_STATUS_FUNC_MASK)
			    >> E1000_STATUS_FUNC_SHIFT;
	} else {
		bus->func = 0;
	}

	return 0;
}


void e1000e_write_vfta(struct e1000_hw *hw, u32 offset, u32 value)
{
	E1000_WRITE_REG_ARRAY(hw, E1000_VFTA, offset, value);
	e1e_flush();
}


void e1000e_init_rx_addrs(struct e1000_hw *hw, u16 rar_count)
{
	u32 i;

	
	hw_dbg(hw, "Programming MAC Address into RAR[0]\n");

	e1000e_rar_set(hw, hw->mac.addr, 0);

	
	hw_dbg(hw, "Clearing RAR[1-%u]\n", rar_count-1);
	for (i = 1; i < rar_count; i++) {
		E1000_WRITE_REG_ARRAY(hw, E1000_RA, (i << 1), 0);
		e1e_flush();
		E1000_WRITE_REG_ARRAY(hw, E1000_RA, ((i << 1) + 1), 0);
		e1e_flush();
	}
}


void e1000e_rar_set(struct e1000_hw *hw, u8 *addr, u32 index)
{
	u32 rar_low, rar_high;

	
	rar_low = ((u32) addr[0] |
		   ((u32) addr[1] << 8) |
		    ((u32) addr[2] << 16) | ((u32) addr[3] << 24));

	rar_high = ((u32) addr[4] | ((u32) addr[5] << 8));

	rar_high |= E1000_RAH_AV;

	E1000_WRITE_REG_ARRAY(hw, E1000_RA, (index << 1), rar_low);
	E1000_WRITE_REG_ARRAY(hw, E1000_RA, ((index << 1) + 1), rar_high);
}


static u32 e1000_hash_mc_addr(struct e1000_hw *hw, u8 *mc_addr)
{
	u32 hash_value, hash_mask;
	u8 bit_shift = 0;

	
	hash_mask = (hw->mac.mta_reg_count * 32) - 1;

	
	while (hash_mask >> bit_shift != 0xFF)
		bit_shift++;

	
	switch (hw->mac.mc_filter_type) {
	default:
	case 0:
		break;
	case 1:
		bit_shift += 1;
		break;
	case 2:
		bit_shift += 2;
		break;
	case 3:
		bit_shift += 4;
		break;
	}

	hash_value = hash_mask & (((mc_addr[4] >> (8 - bit_shift)) |
				  (((u16) mc_addr[5]) << bit_shift)));

	return hash_value;
}


void e1000e_update_mc_addr_list_generic(struct e1000_hw *hw,
					u8 *mc_addr_list, u32 mc_addr_count,
					u32 rar_used_count, u32 rar_count)
{
	u32 i;
	u32 *mcarray = kzalloc(hw->mac.mta_reg_count * sizeof(u32), GFP_ATOMIC);

	if (!mcarray) {
		printk(KERN_ERR "multicast array memory allocation failed\n");
		return;
	}

	
	for (i = rar_used_count; i < rar_count; i++) {
		if (mc_addr_count) {
			e1000e_rar_set(hw, mc_addr_list, i);
			mc_addr_count--;
			mc_addr_list += ETH_ALEN;
		} else {
			E1000_WRITE_REG_ARRAY(hw, E1000_RA, i << 1, 0);
			e1e_flush();
			E1000_WRITE_REG_ARRAY(hw, E1000_RA, (i << 1) + 1, 0);
			e1e_flush();
		}
	}

	
	for (; mc_addr_count > 0; mc_addr_count--) {
		u32 hash_value, hash_reg, hash_bit, mta;
		hash_value = e1000_hash_mc_addr(hw, mc_addr_list);
		hw_dbg(hw, "Hash value = 0x%03X\n", hash_value);
		hash_reg = (hash_value >> 5) & (hw->mac.mta_reg_count - 1);
		hash_bit = hash_value & 0x1F;
		mta = (1 << hash_bit);
		mcarray[hash_reg] |= mta;
		mc_addr_list += ETH_ALEN;
	}

	
	for (i = 0; i < hw->mac.mta_reg_count; i++)
		E1000_WRITE_REG_ARRAY(hw, E1000_MTA, i, mcarray[i]);

	e1e_flush();
	kfree(mcarray);
}


void e1000e_clear_hw_cntrs_base(struct e1000_hw *hw)
{
	u32 temp;

	temp = er32(CRCERRS);
	temp = er32(SYMERRS);
	temp = er32(MPC);
	temp = er32(SCC);
	temp = er32(ECOL);
	temp = er32(MCC);
	temp = er32(LATECOL);
	temp = er32(COLC);
	temp = er32(DC);
	temp = er32(SEC);
	temp = er32(RLEC);
	temp = er32(XONRXC);
	temp = er32(XONTXC);
	temp = er32(XOFFRXC);
	temp = er32(XOFFTXC);
	temp = er32(FCRUC);
	temp = er32(GPRC);
	temp = er32(BPRC);
	temp = er32(MPRC);
	temp = er32(GPTC);
	temp = er32(GORCL);
	temp = er32(GORCH);
	temp = er32(GOTCL);
	temp = er32(GOTCH);
	temp = er32(RNBC);
	temp = er32(RUC);
	temp = er32(RFC);
	temp = er32(ROC);
	temp = er32(RJC);
	temp = er32(TORL);
	temp = er32(TORH);
	temp = er32(TOTL);
	temp = er32(TOTH);
	temp = er32(TPR);
	temp = er32(TPT);
	temp = er32(MPTC);
	temp = er32(BPTC);
}


s32 e1000e_check_for_copper_link(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val;
	bool link;

	
	if (!mac->get_link_status)
		return 0;

	
	ret_val = e1000e_phy_has_link_generic(hw, 1, 0, &link);
	if (ret_val)
		return ret_val;

	if (!link)
		return ret_val; 

	mac->get_link_status = 0;

	
	e1000e_check_downshift(hw);

	
	if (!mac->autoneg) {
		ret_val = -E1000_ERR_CONFIG;
		return ret_val;
	}

	
	e1000e_config_collision_dist(hw);

	
	ret_val = e1000e_config_fc_after_link_up(hw);
	if (ret_val) {
		hw_dbg(hw, "Error configuring flow control\n");
	}

	return ret_val;
}


s32 e1000e_check_for_fiber_link(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	u32 rxcw;
	u32 ctrl;
	u32 status;
	s32 ret_val;

	ctrl = er32(CTRL);
	status = er32(STATUS);
	rxcw = er32(RXCW);

	
	
	if ((ctrl & E1000_CTRL_SWDPIN1) && (!(status & E1000_STATUS_LU)) &&
	    (!(rxcw & E1000_RXCW_C))) {
		if (mac->autoneg_failed == 0) {
			mac->autoneg_failed = 1;
			return 0;
		}
		hw_dbg(hw, "NOT RXing /C/, disable AutoNeg and force link.\n");

		
		ew32(TXCW, (mac->txcw & ~E1000_TXCW_ANE));

		
		ctrl = er32(CTRL);
		ctrl |= (E1000_CTRL_SLU | E1000_CTRL_FD);
		ew32(CTRL, ctrl);

		
		ret_val = e1000e_config_fc_after_link_up(hw);
		if (ret_val) {
			hw_dbg(hw, "Error configuring flow control\n");
			return ret_val;
		}
	} else if ((ctrl & E1000_CTRL_SLU) && (rxcw & E1000_RXCW_C)) {
		
		hw_dbg(hw, "RXing /C/, enable AutoNeg and stop forcing link.\n");
		ew32(TXCW, mac->txcw);
		ew32(CTRL, (ctrl & ~E1000_CTRL_SLU));

		mac->serdes_has_link = true;
	}

	return 0;
}


s32 e1000e_check_for_serdes_link(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	u32 rxcw;
	u32 ctrl;
	u32 status;
	s32 ret_val;

	ctrl = er32(CTRL);
	status = er32(STATUS);
	rxcw = er32(RXCW);

	
	
	if ((!(status & E1000_STATUS_LU)) && (!(rxcw & E1000_RXCW_C))) {
		if (mac->autoneg_failed == 0) {
			mac->autoneg_failed = 1;
			return 0;
		}
		hw_dbg(hw, "NOT RXing /C/, disable AutoNeg and force link.\n");

		
		ew32(TXCW, (mac->txcw & ~E1000_TXCW_ANE));

		
		ctrl = er32(CTRL);
		ctrl |= (E1000_CTRL_SLU | E1000_CTRL_FD);
		ew32(CTRL, ctrl);

		
		ret_val = e1000e_config_fc_after_link_up(hw);
		if (ret_val) {
			hw_dbg(hw, "Error configuring flow control\n");
			return ret_val;
		}
	} else if ((ctrl & E1000_CTRL_SLU) && (rxcw & E1000_RXCW_C)) {
		
		hw_dbg(hw, "RXing /C/, enable AutoNeg and stop forcing link.\n");
		ew32(TXCW, mac->txcw);
		ew32(CTRL, (ctrl & ~E1000_CTRL_SLU));

		mac->serdes_has_link = true;
	} else if (!(E1000_TXCW_ANE & er32(TXCW))) {
		
		
		udelay(10);
		rxcw = er32(RXCW);
		if (rxcw & E1000_RXCW_SYNCH) {
			if (!(rxcw & E1000_RXCW_IV)) {
				mac->serdes_has_link = true;
				hw_dbg(hw, "SERDES: Link up - forced.\n");
			}
		} else {
			mac->serdes_has_link = false;
			hw_dbg(hw, "SERDES: Link down - force failed.\n");
		}
	}

	if (E1000_TXCW_ANE & er32(TXCW)) {
		status = er32(STATUS);
		if (status & E1000_STATUS_LU) {
			
			udelay(10);
			rxcw = er32(RXCW);
			if (rxcw & E1000_RXCW_SYNCH) {
				if (!(rxcw & E1000_RXCW_IV)) {
					mac->serdes_has_link = true;
					hw_dbg(hw, "SERDES: Link up - autoneg "
					   "completed sucessfully.\n");
				} else {
					mac->serdes_has_link = false;
					hw_dbg(hw, "SERDES: Link down - invalid"
					   "codewords detected in autoneg.\n");
				}
			} else {
				mac->serdes_has_link = false;
				hw_dbg(hw, "SERDES: Link down - no sync.\n");
			}
		} else {
			mac->serdes_has_link = false;
			hw_dbg(hw, "SERDES: Link down - autoneg failed\n");
		}
	}

	return 0;
}


static s32 e1000_set_default_fc_generic(struct e1000_hw *hw)
{
	s32 ret_val;
	u16 nvm_data;

	
	ret_val = e1000_read_nvm(hw, NVM_INIT_CONTROL2_REG, 1, &nvm_data);

	if (ret_val) {
		hw_dbg(hw, "NVM Read Error\n");
		return ret_val;
	}

	if ((nvm_data & NVM_WORD0F_PAUSE_MASK) == 0)
		hw->fc.requested_mode = e1000_fc_none;
	else if ((nvm_data & NVM_WORD0F_PAUSE_MASK) ==
		 NVM_WORD0F_ASM_DIR)
		hw->fc.requested_mode = e1000_fc_tx_pause;
	else
		hw->fc.requested_mode = e1000_fc_full;

	return 0;
}


s32 e1000e_setup_link(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val;

	
	if (e1000_check_reset_block(hw))
		return 0;

	
	if (hw->fc.requested_mode == e1000_fc_default) {
		ret_val = e1000_set_default_fc_generic(hw);
		if (ret_val)
			return ret_val;
	}

	
	hw->fc.current_mode = hw->fc.requested_mode;

	hw_dbg(hw, "After fix-ups FlowControl is now = %x\n",
		hw->fc.current_mode);

	
	ret_val = mac->ops.setup_physical_interface(hw);
	if (ret_val)
		return ret_val;

	
	hw_dbg(hw, "Initializing the Flow Control address, type and timer regs\n");
	ew32(FCT, FLOW_CONTROL_TYPE);
	ew32(FCAH, FLOW_CONTROL_ADDRESS_HIGH);
	ew32(FCAL, FLOW_CONTROL_ADDRESS_LOW);

	ew32(FCTTV, hw->fc.pause_time);

	return e1000e_set_fc_watermarks(hw);
}


static s32 e1000_commit_fc_settings_generic(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	u32 txcw;

	
	switch (hw->fc.current_mode) {
	case e1000_fc_none:
		
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD);
		break;
	case e1000_fc_rx_pause:
		
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD | E1000_TXCW_PAUSE_MASK);
		break;
	case e1000_fc_tx_pause:
		
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD | E1000_TXCW_ASM_DIR);
		break;
	case e1000_fc_full:
		
		txcw = (E1000_TXCW_ANE | E1000_TXCW_FD | E1000_TXCW_PAUSE_MASK);
		break;
	default:
		hw_dbg(hw, "Flow control param set incorrectly\n");
		return -E1000_ERR_CONFIG;
		break;
	}

	ew32(TXCW, txcw);
	mac->txcw = txcw;

	return 0;
}


static s32 e1000_poll_fiber_serdes_link_generic(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	u32 i, status;
	s32 ret_val;

	
	for (i = 0; i < FIBER_LINK_UP_LIMIT; i++) {
		msleep(10);
		status = er32(STATUS);
		if (status & E1000_STATUS_LU)
			break;
	}
	if (i == FIBER_LINK_UP_LIMIT) {
		hw_dbg(hw, "Never got a valid link from auto-neg!!!\n");
		mac->autoneg_failed = 1;
		
		ret_val = mac->ops.check_for_link(hw);
		if (ret_val) {
			hw_dbg(hw, "Error while checking for link\n");
			return ret_val;
		}
		mac->autoneg_failed = 0;
	} else {
		mac->autoneg_failed = 0;
		hw_dbg(hw, "Valid Link Found\n");
	}

	return 0;
}


s32 e1000e_setup_fiber_serdes_link(struct e1000_hw *hw)
{
	u32 ctrl;
	s32 ret_val;

	ctrl = er32(CTRL);

	
	ctrl &= ~E1000_CTRL_LRST;

	e1000e_config_collision_dist(hw);

	ret_val = e1000_commit_fc_settings_generic(hw);
	if (ret_val)
		return ret_val;

	
	hw_dbg(hw, "Auto-negotiation enabled\n");

	ew32(CTRL, ctrl);
	e1e_flush();
	msleep(1);

	
	if (hw->phy.media_type == e1000_media_type_internal_serdes ||
	    (er32(CTRL) & E1000_CTRL_SWDPIN1)) {
		ret_val = e1000_poll_fiber_serdes_link_generic(hw);
	} else {
		hw_dbg(hw, "No signal detected\n");
	}

	return 0;
}


void e1000e_config_collision_dist(struct e1000_hw *hw)
{
	u32 tctl;

	tctl = er32(TCTL);

	tctl &= ~E1000_TCTL_COLD;
	tctl |= E1000_COLLISION_DISTANCE << E1000_COLD_SHIFT;

	ew32(TCTL, tctl);
	e1e_flush();
}


s32 e1000e_set_fc_watermarks(struct e1000_hw *hw)
{
	u32 fcrtl = 0, fcrth = 0;

	
	if (hw->fc.current_mode & e1000_fc_tx_pause) {
		
		fcrtl = hw->fc.low_water;
		fcrtl |= E1000_FCRTL_XONE;
		fcrth = hw->fc.high_water;
	}
	ew32(FCRTL, fcrtl);
	ew32(FCRTH, fcrth);

	return 0;
}


s32 e1000e_force_mac_fc(struct e1000_hw *hw)
{
	u32 ctrl;

	ctrl = er32(CTRL);

	
	hw_dbg(hw, "hw->fc.current_mode = %u\n", hw->fc.current_mode);

	switch (hw->fc.current_mode) {
	case e1000_fc_none:
		ctrl &= (~(E1000_CTRL_TFCE | E1000_CTRL_RFCE));
		break;
	case e1000_fc_rx_pause:
		ctrl &= (~E1000_CTRL_TFCE);
		ctrl |= E1000_CTRL_RFCE;
		break;
	case e1000_fc_tx_pause:
		ctrl &= (~E1000_CTRL_RFCE);
		ctrl |= E1000_CTRL_TFCE;
		break;
	case e1000_fc_full:
		ctrl |= (E1000_CTRL_TFCE | E1000_CTRL_RFCE);
		break;
	default:
		hw_dbg(hw, "Flow control param set incorrectly\n");
		return -E1000_ERR_CONFIG;
	}

	ew32(CTRL, ctrl);

	return 0;
}


s32 e1000e_config_fc_after_link_up(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val = 0;
	u16 mii_status_reg, mii_nway_adv_reg, mii_nway_lp_ability_reg;
	u16 speed, duplex;

	
	if (mac->autoneg_failed) {
		if (hw->phy.media_type == e1000_media_type_fiber ||
		    hw->phy.media_type == e1000_media_type_internal_serdes)
			ret_val = e1000e_force_mac_fc(hw);
	} else {
		if (hw->phy.media_type == e1000_media_type_copper)
			ret_val = e1000e_force_mac_fc(hw);
	}

	if (ret_val) {
		hw_dbg(hw, "Error forcing flow control settings\n");
		return ret_val;
	}

	
	if ((hw->phy.media_type == e1000_media_type_copper) && mac->autoneg) {
		
		ret_val = e1e_rphy(hw, PHY_STATUS, &mii_status_reg);
		if (ret_val)
			return ret_val;
		ret_val = e1e_rphy(hw, PHY_STATUS, &mii_status_reg);
		if (ret_val)
			return ret_val;

		if (!(mii_status_reg & MII_SR_AUTONEG_COMPLETE)) {
			hw_dbg(hw, "Copper PHY and Auto Neg "
				 "has not completed.\n");
			return ret_val;
		}

		
		ret_val = e1e_rphy(hw, PHY_AUTONEG_ADV, &mii_nway_adv_reg);
		if (ret_val)
			return ret_val;
		ret_val = e1e_rphy(hw, PHY_LP_ABILITY, &mii_nway_lp_ability_reg);
		if (ret_val)
			return ret_val;

		
		if ((mii_nway_adv_reg & NWAY_AR_PAUSE) &&
		    (mii_nway_lp_ability_reg & NWAY_LPAR_PAUSE)) {
			
			if (hw->fc.requested_mode == e1000_fc_full) {
				hw->fc.current_mode = e1000_fc_full;
				hw_dbg(hw, "Flow Control = FULL.\r\n");
			} else {
				hw->fc.current_mode = e1000_fc_rx_pause;
				hw_dbg(hw, "Flow Control = "
					 "RX PAUSE frames only.\r\n");
			}
		}
		
		else if (!(mii_nway_adv_reg & NWAY_AR_PAUSE) &&
			  (mii_nway_adv_reg & NWAY_AR_ASM_DIR) &&
			  (mii_nway_lp_ability_reg & NWAY_LPAR_PAUSE) &&
			  (mii_nway_lp_ability_reg & NWAY_LPAR_ASM_DIR)) {
			hw->fc.current_mode = e1000_fc_tx_pause;
			hw_dbg(hw, "Flow Control = Tx PAUSE frames only.\r\n");
		}
		
		else if ((mii_nway_adv_reg & NWAY_AR_PAUSE) &&
			 (mii_nway_adv_reg & NWAY_AR_ASM_DIR) &&
			 !(mii_nway_lp_ability_reg & NWAY_LPAR_PAUSE) &&
			 (mii_nway_lp_ability_reg & NWAY_LPAR_ASM_DIR)) {
			hw->fc.current_mode = e1000_fc_rx_pause;
			hw_dbg(hw, "Flow Control = Rx PAUSE frames only.\r\n");
		} else {
			
			hw->fc.current_mode = e1000_fc_none;
			hw_dbg(hw, "Flow Control = NONE.\r\n");
		}

		
		ret_val = mac->ops.get_link_up_info(hw, &speed, &duplex);
		if (ret_val) {
			hw_dbg(hw, "Error getting link speed and duplex\n");
			return ret_val;
		}

		if (duplex == HALF_DUPLEX)
			hw->fc.current_mode = e1000_fc_none;

		
		ret_val = e1000e_force_mac_fc(hw);
		if (ret_val) {
			hw_dbg(hw, "Error forcing flow control settings\n");
			return ret_val;
		}
	}

	return 0;
}


s32 e1000e_get_speed_and_duplex_copper(struct e1000_hw *hw, u16 *speed, u16 *duplex)
{
	u32 status;

	status = er32(STATUS);
	if (status & E1000_STATUS_SPEED_1000) {
		*speed = SPEED_1000;
		hw_dbg(hw, "1000 Mbs, ");
	} else if (status & E1000_STATUS_SPEED_100) {
		*speed = SPEED_100;
		hw_dbg(hw, "100 Mbs, ");
	} else {
		*speed = SPEED_10;
		hw_dbg(hw, "10 Mbs, ");
	}

	if (status & E1000_STATUS_FD) {
		*duplex = FULL_DUPLEX;
		hw_dbg(hw, "Full Duplex\n");
	} else {
		*duplex = HALF_DUPLEX;
		hw_dbg(hw, "Half Duplex\n");
	}

	return 0;
}


s32 e1000e_get_speed_and_duplex_fiber_serdes(struct e1000_hw *hw, u16 *speed, u16 *duplex)
{
	*speed = SPEED_1000;
	*duplex = FULL_DUPLEX;

	return 0;
}


s32 e1000e_get_hw_semaphore(struct e1000_hw *hw)
{
	u32 swsm;
	s32 timeout = hw->nvm.word_size + 1;
	s32 i = 0;

	
	while (i < timeout) {
		swsm = er32(SWSM);
		if (!(swsm & E1000_SWSM_SMBI))
			break;

		udelay(50);
		i++;
	}

	if (i == timeout) {
		hw_dbg(hw, "Driver can't access device - SMBI bit is set.\n");
		return -E1000_ERR_NVM;
	}

	
	for (i = 0; i < timeout; i++) {
		swsm = er32(SWSM);
		ew32(SWSM, swsm | E1000_SWSM_SWESMBI);

		
		if (er32(SWSM) & E1000_SWSM_SWESMBI)
			break;

		udelay(50);
	}

	if (i == timeout) {
		
		e1000e_put_hw_semaphore(hw);
		hw_dbg(hw, "Driver can't access the NVM\n");
		return -E1000_ERR_NVM;
	}

	return 0;
}


void e1000e_put_hw_semaphore(struct e1000_hw *hw)
{
	u32 swsm;

	swsm = er32(SWSM);
	swsm &= ~(E1000_SWSM_SMBI | E1000_SWSM_SWESMBI);
	ew32(SWSM, swsm);
}


s32 e1000e_get_auto_rd_done(struct e1000_hw *hw)
{
	s32 i = 0;

	while (i < AUTO_READ_DONE_TIMEOUT) {
		if (er32(EECD) & E1000_EECD_AUTO_RD)
			break;
		msleep(1);
		i++;
	}

	if (i == AUTO_READ_DONE_TIMEOUT) {
		hw_dbg(hw, "Auto read by HW from NVM has not completed.\n");
		return -E1000_ERR_RESET;
	}

	return 0;
}


s32 e1000e_valid_led_default(struct e1000_hw *hw, u16 *data)
{
	s32 ret_val;

	ret_val = e1000_read_nvm(hw, NVM_ID_LED_SETTINGS, 1, data);
	if (ret_val) {
		hw_dbg(hw, "NVM Read Error\n");
		return ret_val;
	}

	if (*data == ID_LED_RESERVED_0000 || *data == ID_LED_RESERVED_FFFF)
		*data = ID_LED_DEFAULT;

	return 0;
}


s32 e1000e_id_led_init(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;
	s32 ret_val;
	const u32 ledctl_mask = 0x000000FF;
	const u32 ledctl_on = E1000_LEDCTL_MODE_LED_ON;
	const u32 ledctl_off = E1000_LEDCTL_MODE_LED_OFF;
	u16 data, i, temp;
	const u16 led_mask = 0x0F;

	ret_val = hw->nvm.ops.valid_led_default(hw, &data);
	if (ret_val)
		return ret_val;

	mac->ledctl_default = er32(LEDCTL);
	mac->ledctl_mode1 = mac->ledctl_default;
	mac->ledctl_mode2 = mac->ledctl_default;

	for (i = 0; i < 4; i++) {
		temp = (data >> (i << 2)) & led_mask;
		switch (temp) {
		case ID_LED_ON1_DEF2:
		case ID_LED_ON1_ON2:
		case ID_LED_ON1_OFF2:
			mac->ledctl_mode1 &= ~(ledctl_mask << (i << 3));
			mac->ledctl_mode1 |= ledctl_on << (i << 3);
			break;
		case ID_LED_OFF1_DEF2:
		case ID_LED_OFF1_ON2:
		case ID_LED_OFF1_OFF2:
			mac->ledctl_mode1 &= ~(ledctl_mask << (i << 3));
			mac->ledctl_mode1 |= ledctl_off << (i << 3);
			break;
		default:
			
			break;
		}
		switch (temp) {
		case ID_LED_DEF1_ON2:
		case ID_LED_ON1_ON2:
		case ID_LED_OFF1_ON2:
			mac->ledctl_mode2 &= ~(ledctl_mask << (i << 3));
			mac->ledctl_mode2 |= ledctl_on << (i << 3);
			break;
		case ID_LED_DEF1_OFF2:
		case ID_LED_ON1_OFF2:
		case ID_LED_OFF1_OFF2:
			mac->ledctl_mode2 &= ~(ledctl_mask << (i << 3));
			mac->ledctl_mode2 |= ledctl_off << (i << 3);
			break;
		default:
			
			break;
		}
	}

	return 0;
}


s32 e1000e_setup_led_generic(struct e1000_hw *hw)
{
	u32 ledctl;

	if (hw->mac.ops.setup_led != e1000e_setup_led_generic) {
		return -E1000_ERR_CONFIG;
	}

	if (hw->phy.media_type == e1000_media_type_fiber) {
		ledctl = er32(LEDCTL);
		hw->mac.ledctl_default = ledctl;
		
		ledctl &= ~(E1000_LEDCTL_LED0_IVRT |
		            E1000_LEDCTL_LED0_BLINK |
		            E1000_LEDCTL_LED0_MODE_MASK);
		ledctl |= (E1000_LEDCTL_MODE_LED_OFF <<
		           E1000_LEDCTL_LED0_MODE_SHIFT);
		ew32(LEDCTL, ledctl);
	} else if (hw->phy.media_type == e1000_media_type_copper) {
		ew32(LEDCTL, hw->mac.ledctl_mode1);
	}

	return 0;
}


s32 e1000e_cleanup_led_generic(struct e1000_hw *hw)
{
	ew32(LEDCTL, hw->mac.ledctl_default);
	return 0;
}


s32 e1000e_blink_led(struct e1000_hw *hw)
{
	u32 ledctl_blink = 0;
	u32 i;

	if (hw->phy.media_type == e1000_media_type_fiber) {
		
		ledctl_blink = E1000_LEDCTL_LED0_BLINK |
		     (E1000_LEDCTL_MODE_LED_ON << E1000_LEDCTL_LED0_MODE_SHIFT);
	} else {
		
		ledctl_blink = hw->mac.ledctl_mode2;
		for (i = 0; i < 4; i++)
			if (((hw->mac.ledctl_mode2 >> (i * 8)) & 0xFF) ==
			    E1000_LEDCTL_MODE_LED_ON)
				ledctl_blink |= (E1000_LEDCTL_LED0_BLINK <<
						 (i * 8));
	}

	ew32(LEDCTL, ledctl_blink);

	return 0;
}


s32 e1000e_led_on_generic(struct e1000_hw *hw)
{
	u32 ctrl;

	switch (hw->phy.media_type) {
	case e1000_media_type_fiber:
		ctrl = er32(CTRL);
		ctrl &= ~E1000_CTRL_SWDPIN0;
		ctrl |= E1000_CTRL_SWDPIO0;
		ew32(CTRL, ctrl);
		break;
	case e1000_media_type_copper:
		ew32(LEDCTL, hw->mac.ledctl_mode2);
		break;
	default:
		break;
	}

	return 0;
}


s32 e1000e_led_off_generic(struct e1000_hw *hw)
{
	u32 ctrl;

	switch (hw->phy.media_type) {
	case e1000_media_type_fiber:
		ctrl = er32(CTRL);
		ctrl |= E1000_CTRL_SWDPIN0;
		ctrl |= E1000_CTRL_SWDPIO0;
		ew32(CTRL, ctrl);
		break;
	case e1000_media_type_copper:
		ew32(LEDCTL, hw->mac.ledctl_mode1);
		break;
	default:
		break;
	}

	return 0;
}


void e1000e_set_pcie_no_snoop(struct e1000_hw *hw, u32 no_snoop)
{
	u32 gcr;

	if (no_snoop) {
		gcr = er32(GCR);
		gcr &= ~(PCIE_NO_SNOOP_ALL);
		gcr |= no_snoop;
		ew32(GCR, gcr);
	}
}


s32 e1000e_disable_pcie_master(struct e1000_hw *hw)
{
	u32 ctrl;
	s32 timeout = MASTER_DISABLE_TIMEOUT;

	ctrl = er32(CTRL);
	ctrl |= E1000_CTRL_GIO_MASTER_DISABLE;
	ew32(CTRL, ctrl);

	while (timeout) {
		if (!(er32(STATUS) &
		      E1000_STATUS_GIO_MASTER_ENABLE))
			break;
		udelay(100);
		timeout--;
	}

	if (!timeout) {
		hw_dbg(hw, "Master requests are pending.\n");
		return -E1000_ERR_MASTER_REQUESTS_PENDING;
	}

	return 0;
}


void e1000e_reset_adaptive(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;

	mac->current_ifs_val = 0;
	mac->ifs_min_val = IFS_MIN;
	mac->ifs_max_val = IFS_MAX;
	mac->ifs_step_size = IFS_STEP;
	mac->ifs_ratio = IFS_RATIO;

	mac->in_ifs_mode = 0;
	ew32(AIT, 0);
}


void e1000e_update_adaptive(struct e1000_hw *hw)
{
	struct e1000_mac_info *mac = &hw->mac;

	if ((mac->collision_delta * mac->ifs_ratio) > mac->tx_packet_delta) {
		if (mac->tx_packet_delta > MIN_NUM_XMITS) {
			mac->in_ifs_mode = 1;
			if (mac->current_ifs_val < mac->ifs_max_val) {
				if (!mac->current_ifs_val)
					mac->current_ifs_val = mac->ifs_min_val;
				else
					mac->current_ifs_val +=
						mac->ifs_step_size;
				ew32(AIT, mac->current_ifs_val);
			}
		}
	} else {
		if (mac->in_ifs_mode &&
		    (mac->tx_packet_delta <= MIN_NUM_XMITS)) {
			mac->current_ifs_val = 0;
			mac->in_ifs_mode = 0;
			ew32(AIT, 0);
		}
	}
}


static void e1000_raise_eec_clk(struct e1000_hw *hw, u32 *eecd)
{
	*eecd = *eecd | E1000_EECD_SK;
	ew32(EECD, *eecd);
	e1e_flush();
	udelay(hw->nvm.delay_usec);
}


static void e1000_lower_eec_clk(struct e1000_hw *hw, u32 *eecd)
{
	*eecd = *eecd & ~E1000_EECD_SK;
	ew32(EECD, *eecd);
	e1e_flush();
	udelay(hw->nvm.delay_usec);
}


static void e1000_shift_out_eec_bits(struct e1000_hw *hw, u16 data, u16 count)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	u32 eecd = er32(EECD);
	u32 mask;

	mask = 0x01 << (count - 1);
	if (nvm->type == e1000_nvm_eeprom_spi)
		eecd |= E1000_EECD_DO;

	do {
		eecd &= ~E1000_EECD_DI;

		if (data & mask)
			eecd |= E1000_EECD_DI;

		ew32(EECD, eecd);
		e1e_flush();

		udelay(nvm->delay_usec);

		e1000_raise_eec_clk(hw, &eecd);
		e1000_lower_eec_clk(hw, &eecd);

		mask >>= 1;
	} while (mask);

	eecd &= ~E1000_EECD_DI;
	ew32(EECD, eecd);
}


static u16 e1000_shift_in_eec_bits(struct e1000_hw *hw, u16 count)
{
	u32 eecd;
	u32 i;
	u16 data;

	eecd = er32(EECD);

	eecd &= ~(E1000_EECD_DO | E1000_EECD_DI);
	data = 0;

	for (i = 0; i < count; i++) {
		data <<= 1;
		e1000_raise_eec_clk(hw, &eecd);

		eecd = er32(EECD);

		eecd &= ~E1000_EECD_DI;
		if (eecd & E1000_EECD_DO)
			data |= 1;

		e1000_lower_eec_clk(hw, &eecd);
	}

	return data;
}


s32 e1000e_poll_eerd_eewr_done(struct e1000_hw *hw, int ee_reg)
{
	u32 attempts = 100000;
	u32 i, reg = 0;

	for (i = 0; i < attempts; i++) {
		if (ee_reg == E1000_NVM_POLL_READ)
			reg = er32(EERD);
		else
			reg = er32(EEWR);

		if (reg & E1000_NVM_RW_REG_DONE)
			return 0;

		udelay(5);
	}

	return -E1000_ERR_NVM;
}


s32 e1000e_acquire_nvm(struct e1000_hw *hw)
{
	u32 eecd = er32(EECD);
	s32 timeout = E1000_NVM_GRANT_ATTEMPTS;

	ew32(EECD, eecd | E1000_EECD_REQ);
	eecd = er32(EECD);

	while (timeout) {
		if (eecd & E1000_EECD_GNT)
			break;
		udelay(5);
		eecd = er32(EECD);
		timeout--;
	}

	if (!timeout) {
		eecd &= ~E1000_EECD_REQ;
		ew32(EECD, eecd);
		hw_dbg(hw, "Could not acquire NVM grant\n");
		return -E1000_ERR_NVM;
	}

	return 0;
}


static void e1000_standby_nvm(struct e1000_hw *hw)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	u32 eecd = er32(EECD);

	if (nvm->type == e1000_nvm_eeprom_spi) {
		
		eecd |= E1000_EECD_CS;
		ew32(EECD, eecd);
		e1e_flush();
		udelay(nvm->delay_usec);
		eecd &= ~E1000_EECD_CS;
		ew32(EECD, eecd);
		e1e_flush();
		udelay(nvm->delay_usec);
	}
}


static void e1000_stop_nvm(struct e1000_hw *hw)
{
	u32 eecd;

	eecd = er32(EECD);
	if (hw->nvm.type == e1000_nvm_eeprom_spi) {
		
		eecd |= E1000_EECD_CS;
		e1000_lower_eec_clk(hw, &eecd);
	}
}


void e1000e_release_nvm(struct e1000_hw *hw)
{
	u32 eecd;

	e1000_stop_nvm(hw);

	eecd = er32(EECD);
	eecd &= ~E1000_EECD_REQ;
	ew32(EECD, eecd);
}


static s32 e1000_ready_nvm_eeprom(struct e1000_hw *hw)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	u32 eecd = er32(EECD);
	u16 timeout = 0;
	u8 spi_stat_reg;

	if (nvm->type == e1000_nvm_eeprom_spi) {
		
		eecd &= ~(E1000_EECD_CS | E1000_EECD_SK);
		ew32(EECD, eecd);
		udelay(1);
		timeout = NVM_MAX_RETRY_SPI;

		
		while (timeout) {
			e1000_shift_out_eec_bits(hw, NVM_RDSR_OPCODE_SPI,
						 hw->nvm.opcode_bits);
			spi_stat_reg = (u8)e1000_shift_in_eec_bits(hw, 8);
			if (!(spi_stat_reg & NVM_STATUS_RDY_SPI))
				break;

			udelay(5);
			e1000_standby_nvm(hw);
			timeout--;
		}

		if (!timeout) {
			hw_dbg(hw, "SPI NVM Status error\n");
			return -E1000_ERR_NVM;
		}
	}

	return 0;
}


s32 e1000e_read_nvm_eerd(struct e1000_hw *hw, u16 offset, u16 words, u16 *data)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	u32 i, eerd = 0;
	s32 ret_val = 0;

	
	if ((offset >= nvm->word_size) || (words > (nvm->word_size - offset)) ||
	    (words == 0)) {
		hw_dbg(hw, "nvm parameter(s) out of bounds\n");
		return -E1000_ERR_NVM;
	}

	for (i = 0; i < words; i++) {
		eerd = ((offset+i) << E1000_NVM_RW_ADDR_SHIFT) +
		       E1000_NVM_RW_REG_START;

		ew32(EERD, eerd);
		ret_val = e1000e_poll_eerd_eewr_done(hw, E1000_NVM_POLL_READ);
		if (ret_val)
			break;

		data[i] = (er32(EERD) >> E1000_NVM_RW_REG_DATA);
	}

	return ret_val;
}


s32 e1000e_write_nvm_spi(struct e1000_hw *hw, u16 offset, u16 words, u16 *data)
{
	struct e1000_nvm_info *nvm = &hw->nvm;
	s32 ret_val;
	u16 widx = 0;

	
	if ((offset >= nvm->word_size) || (words > (nvm->word_size - offset)) ||
	    (words == 0)) {
		hw_dbg(hw, "nvm parameter(s) out of bounds\n");
		return -E1000_ERR_NVM;
	}

	ret_val = nvm->ops.acquire_nvm(hw);
	if (ret_val)
		return ret_val;

	msleep(10);

	while (widx < words) {
		u8 write_opcode = NVM_WRITE_OPCODE_SPI;

		ret_val = e1000_ready_nvm_eeprom(hw);
		if (ret_val) {
			nvm->ops.release_nvm(hw);
			return ret_val;
		}

		e1000_standby_nvm(hw);

		
		e1000_shift_out_eec_bits(hw, NVM_WREN_OPCODE_SPI,
					 nvm->opcode_bits);

		e1000_standby_nvm(hw);

		
		if ((nvm->address_bits == 8) && (offset >= 128))
			write_opcode |= NVM_A8_OPCODE_SPI;

		
		e1000_shift_out_eec_bits(hw, write_opcode, nvm->opcode_bits);
		e1000_shift_out_eec_bits(hw, (u16)((offset + widx) * 2),
					 nvm->address_bits);

		
		while (widx < words) {
			u16 word_out = data[widx];
			word_out = (word_out >> 8) | (word_out << 8);
			e1000_shift_out_eec_bits(hw, word_out, 16);
			widx++;

			if ((((offset + widx) * 2) % nvm->page_size) == 0) {
				e1000_standby_nvm(hw);
				break;
			}
		}
	}

	msleep(10);
	nvm->ops.release_nvm(hw);
	return 0;
}


s32 e1000e_read_mac_addr(struct e1000_hw *hw)
{
	s32 ret_val;
	u16 offset, nvm_data, i;
	u16 mac_addr_offset = 0;

	if (hw->mac.type == e1000_82571) {
		
		ret_val = e1000_read_nvm(hw, NVM_ALT_MAC_ADDR_PTR, 1,
					 &mac_addr_offset);
		if (ret_val) {
			hw_dbg(hw, "NVM Read Error\n");
			return ret_val;
		}
		if (mac_addr_offset == 0xFFFF)
			mac_addr_offset = 0;

		if (mac_addr_offset) {
			if (hw->bus.func == E1000_FUNC_1)
				mac_addr_offset += ETH_ALEN/sizeof(u16);

			
			ret_val = e1000_read_nvm(hw, mac_addr_offset, 1,
						 &nvm_data);
			if (ret_val) {
				hw_dbg(hw, "NVM Read Error\n");
				return ret_val;
			}
			if (nvm_data & 0x0001)
				mac_addr_offset = 0;
		}

		if (mac_addr_offset)
		hw->dev_spec.e82571.alt_mac_addr_is_present = 1;
	}

	for (i = 0; i < ETH_ALEN; i += 2) {
		offset = mac_addr_offset + (i >> 1);
		ret_val = e1000_read_nvm(hw, offset, 1, &nvm_data);
		if (ret_val) {
			hw_dbg(hw, "NVM Read Error\n");
			return ret_val;
		}
		hw->mac.perm_addr[i] = (u8)(nvm_data & 0xFF);
		hw->mac.perm_addr[i+1] = (u8)(nvm_data >> 8);
	}

	
	if (!mac_addr_offset && hw->bus.func == E1000_FUNC_1)
		hw->mac.perm_addr[5] ^= 1;

	for (i = 0; i < ETH_ALEN; i++)
		hw->mac.addr[i] = hw->mac.perm_addr[i];

	return 0;
}


s32 e1000e_validate_nvm_checksum_generic(struct e1000_hw *hw)
{
	s32 ret_val;
	u16 checksum = 0;
	u16 i, nvm_data;

	for (i = 0; i < (NVM_CHECKSUM_REG + 1); i++) {
		ret_val = e1000_read_nvm(hw, i, 1, &nvm_data);
		if (ret_val) {
			hw_dbg(hw, "NVM Read Error\n");
			return ret_val;
		}
		checksum += nvm_data;
	}

	if (checksum != (u16) NVM_SUM) {
		hw_dbg(hw, "NVM Checksum Invalid\n");
		return -E1000_ERR_NVM;
	}

	return 0;
}


s32 e1000e_update_nvm_checksum_generic(struct e1000_hw *hw)
{
	s32 ret_val;
	u16 checksum = 0;
	u16 i, nvm_data;

	for (i = 0; i < NVM_CHECKSUM_REG; i++) {
		ret_val = e1000_read_nvm(hw, i, 1, &nvm_data);
		if (ret_val) {
			hw_dbg(hw, "NVM Read Error while updating checksum.\n");
			return ret_val;
		}
		checksum += nvm_data;
	}
	checksum = (u16) NVM_SUM - checksum;
	ret_val = e1000_write_nvm(hw, NVM_CHECKSUM_REG, 1, &checksum);
	if (ret_val)
		hw_dbg(hw, "NVM Write Error while updating checksum.\n");

	return ret_val;
}


void e1000e_reload_nvm(struct e1000_hw *hw)
{
	u32 ctrl_ext;

	udelay(10);
	ctrl_ext = er32(CTRL_EXT);
	ctrl_ext |= E1000_CTRL_EXT_EE_RST;
	ew32(CTRL_EXT, ctrl_ext);
	e1e_flush();
}


static u8 e1000_calculate_checksum(u8 *buffer, u32 length)
{
	u32 i;
	u8  sum = 0;

	if (!buffer)
		return 0;

	for (i = 0; i < length; i++)
		sum += buffer[i];

	return (u8) (0 - sum);
}


static s32 e1000_mng_enable_host_if(struct e1000_hw *hw)
{
	u32 hicr;
	u8 i;

	
	hicr = er32(HICR);
	if ((hicr & E1000_HICR_EN) == 0) {
		hw_dbg(hw, "E1000_HOST_EN bit disabled.\n");
		return -E1000_ERR_HOST_INTERFACE_COMMAND;
	}
	
	for (i = 0; i < E1000_MNG_DHCP_COMMAND_TIMEOUT; i++) {
		hicr = er32(HICR);
		if (!(hicr & E1000_HICR_C))
			break;
		mdelay(1);
	}

	if (i == E1000_MNG_DHCP_COMMAND_TIMEOUT) {
		hw_dbg(hw, "Previous command timeout failed .\n");
		return -E1000_ERR_HOST_INTERFACE_COMMAND;
	}

	return 0;
}


bool e1000e_check_mng_mode_generic(struct e1000_hw *hw)
{
	u32 fwsm = er32(FWSM);

	return (fwsm & E1000_FWSM_MODE_MASK) ==
		(E1000_MNG_IAMT_MODE << E1000_FWSM_MODE_SHIFT);
}


bool e1000e_enable_tx_pkt_filtering(struct e1000_hw *hw)
{
	struct e1000_host_mng_dhcp_cookie *hdr = &hw->mng_cookie;
	u32 *buffer = (u32 *)&hw->mng_cookie;
	u32 offset;
	s32 ret_val, hdr_csum, csum;
	u8 i, len;

	
	if (!e1000e_check_mng_mode(hw)) {
		hw->mac.tx_pkt_filtering = 0;
		return 0;
	}

	
	ret_val = e1000_mng_enable_host_if(hw);
	if (ret_val != 0) {
		hw->mac.tx_pkt_filtering = 0;
		return ret_val;
	}

	
	len    = E1000_MNG_DHCP_COOKIE_LENGTH >> 2;
	offset = E1000_MNG_DHCP_COOKIE_OFFSET >> 2;
	for (i = 0; i < len; i++)
		*(buffer + i) = E1000_READ_REG_ARRAY(hw, E1000_HOST_IF, offset + i);
	hdr_csum = hdr->checksum;
	hdr->checksum = 0;
	csum = e1000_calculate_checksum((u8 *)hdr,
					E1000_MNG_DHCP_COOKIE_LENGTH);
	
	if ((hdr_csum != csum) || (hdr->signature != E1000_IAMT_SIGNATURE)) {
		hw->mac.tx_pkt_filtering = 1;
		return 1;
	}

	
	if (!(hdr->status & E1000_MNG_DHCP_COOKIE_STATUS_PARSING)) {
		hw->mac.tx_pkt_filtering = 0;
		return 0;
	}

	hw->mac.tx_pkt_filtering = 1;
	return 1;
}


static s32 e1000_mng_write_cmd_header(struct e1000_hw *hw,
				  struct e1000_host_mng_command_header *hdr)
{
	u16 i, length = sizeof(struct e1000_host_mng_command_header);

	

	hdr->checksum = e1000_calculate_checksum((u8 *)hdr, length);

	length >>= 2;
	
	for (i = 0; i < length; i++) {
		E1000_WRITE_REG_ARRAY(hw, E1000_HOST_IF, i,
					    *((u32 *) hdr + i));
		e1e_flush();
	}

	return 0;
}


static s32 e1000_mng_host_if_write(struct e1000_hw *hw, u8 *buffer,
				   u16 length, u16 offset, u8 *sum)
{
	u8 *tmp;
	u8 *bufptr = buffer;
	u32 data = 0;
	u16 remaining, i, j, prev_bytes;

	

	if (length == 0 || offset + length > E1000_HI_MAX_MNG_DATA_LENGTH)
		return -E1000_ERR_PARAM;

	tmp = (u8 *)&data;
	prev_bytes = offset & 0x3;
	offset >>= 2;

	if (prev_bytes) {
		data = E1000_READ_REG_ARRAY(hw, E1000_HOST_IF, offset);
		for (j = prev_bytes; j < sizeof(u32); j++) {
			*(tmp + j) = *bufptr++;
			*sum += *(tmp + j);
		}
		E1000_WRITE_REG_ARRAY(hw, E1000_HOST_IF, offset, data);
		length -= j - prev_bytes;
		offset++;
	}

	remaining = length & 0x3;
	length -= remaining;

	
	length >>= 2;

	
	for (i = 0; i < length; i++) {
		for (j = 0; j < sizeof(u32); j++) {
			*(tmp + j) = *bufptr++;
			*sum += *(tmp + j);
		}

		E1000_WRITE_REG_ARRAY(hw, E1000_HOST_IF, offset + i, data);
	}
	if (remaining) {
		for (j = 0; j < sizeof(u32); j++) {
			if (j < remaining)
				*(tmp + j) = *bufptr++;
			else
				*(tmp + j) = 0;

			*sum += *(tmp + j);
		}
		E1000_WRITE_REG_ARRAY(hw, E1000_HOST_IF, offset + i, data);
	}

	return 0;
}


s32 e1000e_mng_write_dhcp_info(struct e1000_hw *hw, u8 *buffer, u16 length)
{
	struct e1000_host_mng_command_header hdr;
	s32 ret_val;
	u32 hicr;

	hdr.command_id = E1000_MNG_DHCP_TX_PAYLOAD_CMD;
	hdr.command_length = length;
	hdr.reserved1 = 0;
	hdr.reserved2 = 0;
	hdr.checksum = 0;

	
	ret_val = e1000_mng_enable_host_if(hw);
	if (ret_val)
		return ret_val;

	
	ret_val = e1000_mng_host_if_write(hw, buffer, length,
					  sizeof(hdr), &(hdr.checksum));
	if (ret_val)
		return ret_val;

	
	ret_val = e1000_mng_write_cmd_header(hw, &hdr);
	if (ret_val)
		return ret_val;

	
	hicr = er32(HICR);
	ew32(HICR, hicr | E1000_HICR_C);

	return 0;
}


bool e1000e_enable_mng_pass_thru(struct e1000_hw *hw)
{
	u32 manc;
	u32 fwsm, factps;
	bool ret_val = 0;

	manc = er32(MANC);

	if (!(manc & E1000_MANC_RCV_TCO_EN) ||
	    !(manc & E1000_MANC_EN_MAC_ADDR_FILTER))
		return ret_val;

	if (hw->mac.arc_subsystem_valid) {
		fwsm = er32(FWSM);
		factps = er32(FACTPS);

		if (!(factps & E1000_FACTPS_MNGCG) &&
		    ((fwsm & E1000_FWSM_MODE_MASK) ==
		     (e1000_mng_mode_pt << E1000_FWSM_MODE_SHIFT))) {
			ret_val = 1;
			return ret_val;
		}
	} else {
		if ((manc & E1000_MANC_SMBUS_EN) &&
		    !(manc & E1000_MANC_ASF_EN)) {
			ret_val = 1;
			return ret_val;
		}
	}

	return ret_val;
}

s32 e1000e_read_pba_num(struct e1000_hw *hw, u32 *pba_num)
{
	s32 ret_val;
	u16 nvm_data;

	ret_val = e1000_read_nvm(hw, NVM_PBA_OFFSET_0, 1, &nvm_data);
	if (ret_val) {
		hw_dbg(hw, "NVM Read Error\n");
		return ret_val;
	}
	*pba_num = (u32)(nvm_data << 16);

	ret_val = e1000_read_nvm(hw, NVM_PBA_OFFSET_1, 1, &nvm_data);
	if (ret_val) {
		hw_dbg(hw, "NVM Read Error\n");
		return ret_val;
	}
	*pba_num |= nvm_data;

	return 0;
}
