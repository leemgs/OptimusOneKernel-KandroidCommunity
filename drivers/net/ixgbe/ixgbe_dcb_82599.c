

#include "ixgbe.h"
#include "ixgbe_type.h"
#include "ixgbe_dcb.h"
#include "ixgbe_dcb_82599.h"


s32 ixgbe_dcb_get_tc_stats_82599(struct ixgbe_hw *hw,
                                 struct ixgbe_hw_stats *stats,
                                 u8 tc_count)
{
	int tc;

	if (tc_count > MAX_TRAFFIC_CLASS)
		return DCB_ERR_PARAM;
	
	for (tc = 0; tc < tc_count; tc++) {
		
		stats->qptc[tc] += IXGBE_READ_REG(hw, IXGBE_QPTC(tc));
		
		stats->qbtc[tc] += IXGBE_READ_REG(hw, IXGBE_QBTC(tc));
		
		stats->qprc[tc] += IXGBE_READ_REG(hw, IXGBE_QPRC(tc));
		
		stats->qbrc[tc] += IXGBE_READ_REG(hw, IXGBE_QBRC(tc));
	}

	return 0;
}


s32 ixgbe_dcb_get_pfc_stats_82599(struct ixgbe_hw *hw,
                                  struct ixgbe_hw_stats *stats,
                                  u8 tc_count)
{
	int tc;

	if (tc_count > MAX_TRAFFIC_CLASS)
		return DCB_ERR_PARAM;
	for (tc = 0; tc < tc_count; tc++) {
		
		stats->pxofftxc[tc] += IXGBE_READ_REG(hw, IXGBE_PXOFFTXC(tc));
		
		stats->pxoffrxc[tc] += IXGBE_READ_REG(hw, IXGBE_PXOFFRXCNT(tc));
	}

	return 0;
}


s32 ixgbe_dcb_config_packet_buffers_82599(struct ixgbe_hw *hw,
                                          struct ixgbe_dcb_config *dcb_config)
{
	s32 ret_val = 0;
	u32 value = IXGBE_RXPBSIZE_64KB;
	u8  i = 0;

	
	switch (dcb_config->rx_pba_cfg) {
	case pba_80_48:
		
		value = IXGBE_RXPBSIZE_80KB;
		for (; i < 4; i++)
			IXGBE_WRITE_REG(hw, IXGBE_RXPBSIZE(i), value);
		
		value = IXGBE_RXPBSIZE_48KB;
		
	case pba_equal:
	default:
		for (; i < IXGBE_MAX_PACKET_BUFFERS; i++)
			IXGBE_WRITE_REG(hw, IXGBE_RXPBSIZE(i), value);

		
		for (i = 0; i < IXGBE_MAX_PACKET_BUFFERS; i++) {
			IXGBE_WRITE_REG(hw, IXGBE_TXPBSIZE(i),
			                IXGBE_TXPBSIZE_20KB);
			IXGBE_WRITE_REG(hw, IXGBE_TXPBTHRESH(i),
			                IXGBE_TXPBTHRESH_DCB);
		}
		break;
	}

	return ret_val;
}


s32 ixgbe_dcb_config_rx_arbiter_82599(struct ixgbe_hw *hw,
                                      struct ixgbe_dcb_config *dcb_config)
{
	struct tc_bw_alloc    *p;
	u32    reg           = 0;
	u32    credit_refill = 0;
	u32    credit_max    = 0;
	u8     i             = 0;

	
	reg = IXGBE_RTRPCS_RRM | IXGBE_RTRPCS_RAC | IXGBE_RTRPCS_ARBDIS;
	IXGBE_WRITE_REG(hw, IXGBE_RTRPCS, reg);

	
	reg = 0;
	for (i = 0; i < MAX_TRAFFIC_CLASS; i++)
		reg |= (i << (i * IXGBE_RTRUP2TC_UP_SHIFT));
	IXGBE_WRITE_REG(hw, IXGBE_RTRUP2TC, reg);

	
	for (i = 0; i < MAX_TRAFFIC_CLASS; i++) {
		p = &dcb_config->tc_config[i].path[DCB_RX_CONFIG];

		credit_refill = p->data_credits_refill;
		credit_max    = p->data_credits_max;
		reg = credit_refill | (credit_max << IXGBE_RTRPT4C_MCL_SHIFT);

		reg |= (u32)(p->bwg_id) << IXGBE_RTRPT4C_BWG_SHIFT;

		if (p->prio_type == prio_link)
			reg |= IXGBE_RTRPT4C_LSP;

		IXGBE_WRITE_REG(hw, IXGBE_RTRPT4C(i), reg);
	}

	
	reg = IXGBE_RTRPCS_RRM | IXGBE_RTRPCS_RAC;
	IXGBE_WRITE_REG(hw, IXGBE_RTRPCS, reg);

	return 0;
}


s32 ixgbe_dcb_config_tx_desc_arbiter_82599(struct ixgbe_hw *hw,
                                           struct ixgbe_dcb_config *dcb_config)
{
	struct tc_bw_alloc *p;
	u32    reg, max_credits;
	u8     i;

	
	for (i = 0; i < 128; i++) {
		IXGBE_WRITE_REG(hw, IXGBE_RTTDQSEL, i);
		IXGBE_WRITE_REG(hw, IXGBE_RTTDT1C, 0);
	}

	
	for (i = 0; i < MAX_TRAFFIC_CLASS; i++) {
		p = &dcb_config->tc_config[i].path[DCB_TX_CONFIG];
		max_credits = dcb_config->tc_config[i].desc_credits_max;
		reg = max_credits << IXGBE_RTTDT2C_MCL_SHIFT;
		reg |= p->data_credits_refill;
		reg |= (u32)(p->bwg_id) << IXGBE_RTTDT2C_BWG_SHIFT;

		if (p->prio_type == prio_group)
			reg |= IXGBE_RTTDT2C_GSP;

		if (p->prio_type == prio_link)
			reg |= IXGBE_RTTDT2C_LSP;

		IXGBE_WRITE_REG(hw, IXGBE_RTTDT2C(i), reg);
	}

	
	reg = IXGBE_RTTDCS_TDPAC | IXGBE_RTTDCS_TDRM;
	IXGBE_WRITE_REG(hw, IXGBE_RTTDCS, reg);

	return 0;
}


s32 ixgbe_dcb_config_tx_data_arbiter_82599(struct ixgbe_hw *hw,
                                           struct ixgbe_dcb_config *dcb_config)
{
	struct tc_bw_alloc *p;
	u32 reg;
	u8 i;

	
	reg = IXGBE_RTTPCS_TPPAC | IXGBE_RTTPCS_TPRM |
	      (IXGBE_RTTPCS_ARBD_DCB << IXGBE_RTTPCS_ARBD_SHIFT) |
	      IXGBE_RTTPCS_ARBDIS;
	IXGBE_WRITE_REG(hw, IXGBE_RTTPCS, reg);

	
	reg = 0;
	for (i = 0; i < MAX_TRAFFIC_CLASS; i++)
		reg |= (i << (i * IXGBE_RTTUP2TC_UP_SHIFT));
	IXGBE_WRITE_REG(hw, IXGBE_RTTUP2TC, reg);

	
	for (i = 0; i < MAX_TRAFFIC_CLASS; i++) {
		p = &dcb_config->tc_config[i].path[DCB_TX_CONFIG];
		reg = p->data_credits_refill;
		reg |= (u32)(p->data_credits_max) << IXGBE_RTTPT2C_MCL_SHIFT;
		reg |= (u32)(p->bwg_id) << IXGBE_RTTPT2C_BWG_SHIFT;

		if (p->prio_type == prio_group)
			reg |= IXGBE_RTTPT2C_GSP;

		if (p->prio_type == prio_link)
			reg |= IXGBE_RTTPT2C_LSP;

		IXGBE_WRITE_REG(hw, IXGBE_RTTPT2C(i), reg);
	}

	
	reg = IXGBE_RTTPCS_TPPAC | IXGBE_RTTPCS_TPRM |
	      (IXGBE_RTTPCS_ARBD_DCB << IXGBE_RTTPCS_ARBD_SHIFT);
	IXGBE_WRITE_REG(hw, IXGBE_RTTPCS, reg);

	return 0;
}


s32 ixgbe_dcb_config_pfc_82599(struct ixgbe_hw *hw,
                               struct ixgbe_dcb_config *dcb_config)
{
	u32 i, reg, rx_pba_size;

	
	if (!dcb_config->pfc_mode_enable) {
		for (i = 0; i < MAX_TRAFFIC_CLASS; i++)
			hw->mac.ops.fc_enable(hw, i);
		goto out;
	}

	
	for (i = 0; i < MAX_TRAFFIC_CLASS; i++) {
		if (dcb_config->rx_pba_cfg == pba_equal)
			rx_pba_size = IXGBE_RXPBSIZE_64KB;
		else
			rx_pba_size = (i < 4) ? IXGBE_RXPBSIZE_80KB
			                      : IXGBE_RXPBSIZE_48KB;

		reg = ((rx_pba_size >> 5) & 0xFFE0);
		if (dcb_config->tc_config[i].dcb_pfc == pfc_enabled_full ||
		    dcb_config->tc_config[i].dcb_pfc == pfc_enabled_tx)
			reg |= IXGBE_FCRTL_XONE;
		IXGBE_WRITE_REG(hw, IXGBE_FCRTL_82599(i), reg);

		reg = ((rx_pba_size >> 2) & 0xFFE0);
		if (dcb_config->tc_config[i].dcb_pfc == pfc_enabled_full ||
		    dcb_config->tc_config[i].dcb_pfc == pfc_enabled_tx)
			reg |= IXGBE_FCRTH_FCEN;
		IXGBE_WRITE_REG(hw, IXGBE_FCRTH_82599(i), reg);
	}

	
	reg = hw->fc.pause_time | (hw->fc.pause_time << 16);
	for (i = 0; i < (MAX_TRAFFIC_CLASS / 2); i++)
		IXGBE_WRITE_REG(hw, IXGBE_FCTTV(i), reg);

	
	IXGBE_WRITE_REG(hw, IXGBE_FCRTV, hw->fc.pause_time / 2);

	
	reg = IXGBE_FCCFG_TFCE_PRIORITY;
	IXGBE_WRITE_REG(hw, IXGBE_FCCFG, reg);

	
	reg = IXGBE_READ_REG(hw, IXGBE_MFLCN);
	reg &= ~IXGBE_MFLCN_RFCE;
	reg |= IXGBE_MFLCN_RPFCE;
	IXGBE_WRITE_REG(hw, IXGBE_MFLCN, reg);
out:
	return 0;
}


s32 ixgbe_dcb_config_tc_stats_82599(struct ixgbe_hw *hw)
{
	u32 reg = 0;
	u8  i   = 0;

	
	for (i = 0; i < 32; i++) {
		reg = 0x01010101 * (i / 4);
		IXGBE_WRITE_REG(hw, IXGBE_RQSMR(i), reg);
	}
	
	for (i = 0; i < 32; i++) {
		if (i < 8)
			reg = 0x00000000;
		else if (i < 16)
			reg = 0x01010101;
		else if (i < 20)
			reg = 0x02020202;
		else if (i < 24)
			reg = 0x03030303;
		else if (i < 26)
			reg = 0x04040404;
		else if (i < 28)
			reg = 0x05050505;
		else if (i < 30)
			reg = 0x06060606;
		else
			reg = 0x07070707;
		IXGBE_WRITE_REG(hw, IXGBE_TQSM(i), reg);
	}

	return 0;
}


s32 ixgbe_dcb_config_82599(struct ixgbe_hw *hw)
{
	u32 reg;
	u32 q;

	
	reg = IXGBE_READ_REG(hw, IXGBE_RTTDCS);
	reg |= IXGBE_RTTDCS_ARBDIS;
	IXGBE_WRITE_REG(hw, IXGBE_RTTDCS, reg);

	
	reg = IXGBE_READ_REG(hw, IXGBE_MRQC);
	switch (reg & IXGBE_MRQC_MRQE_MASK) {
	case 0:
	case IXGBE_MRQC_RT4TCEN:
		
		reg = (reg & ~IXGBE_MRQC_MRQE_MASK) | IXGBE_MRQC_RT8TCEN;
		break;
	case IXGBE_MRQC_RSSEN:
	case IXGBE_MRQC_RTRSS4TCEN:
		
		reg = (reg & ~IXGBE_MRQC_MRQE_MASK) | IXGBE_MRQC_RTRSS8TCEN;
		break;
	default:
		
		reg = (reg & ~IXGBE_MRQC_MRQE_MASK) | IXGBE_MRQC_RT8TCEN;
	}
	IXGBE_WRITE_REG(hw, IXGBE_MRQC, reg);

	
	reg = IXGBE_MTQC_RT_ENA | IXGBE_MTQC_8TC_8TQ;
	IXGBE_WRITE_REG(hw, IXGBE_MTQC, reg);

	
	for (q = 0; q < 128; q++)
		IXGBE_WRITE_REG(hw, IXGBE_QDE, q << IXGBE_QDE_IDX_SHIFT);

	
	reg = IXGBE_READ_REG(hw, IXGBE_RTTDCS);
	reg &= ~IXGBE_RTTDCS_ARBDIS;
	IXGBE_WRITE_REG(hw, IXGBE_RTTDCS, reg);

	return 0;
}


s32 ixgbe_dcb_hw_config_82599(struct ixgbe_hw *hw,
                              struct ixgbe_dcb_config *dcb_config)
{
	ixgbe_dcb_config_packet_buffers_82599(hw, dcb_config);
	ixgbe_dcb_config_82599(hw);
	ixgbe_dcb_config_rx_arbiter_82599(hw, dcb_config);
	ixgbe_dcb_config_tx_desc_arbiter_82599(hw, dcb_config);
	ixgbe_dcb_config_tx_data_arbiter_82599(hw, dcb_config);
	ixgbe_dcb_config_pfc_82599(hw, dcb_config);
	ixgbe_dcb_config_tc_stats_82599(hw);

	return 0;
}

