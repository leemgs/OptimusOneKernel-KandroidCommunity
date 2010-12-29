

#define _ATH5K_RESET



#include <linux/pci.h> 		
#include <linux/log2.h>
#include "ath5k.h"
#include "reg.h"
#include "base.h"
#include "debug.h"


static inline int ath5k_hw_write_ofdm_timings(struct ath5k_hw *ah,
	struct ieee80211_channel *channel)
{
	
	u32 coef_scaled, coef_exp, coef_man,
		ds_coef_exp, ds_coef_man, clock;

	BUG_ON(!(ah->ah_version == AR5K_AR5212) ||
		!(channel->hw_value & CHANNEL_OFDM));

	
	
	clock =  ath5k_hw_htoclock(1, channel->hw_value & CHANNEL_TURBO);

	coef_scaled = ((5 * (clock << 24)) / 2) / channel->center_freq;

	
	coef_exp = ilog2(coef_scaled);

	
	if (!coef_scaled || !coef_exp)
		return -EINVAL;

	
	coef_exp = 14 - (coef_exp - 24);


	
	coef_man = coef_scaled +
		(1 << (24 - coef_exp - 1));

	
	ds_coef_man = coef_man >> (24 - coef_exp);
	ds_coef_exp = coef_exp - 16;

	AR5K_REG_WRITE_BITS(ah, AR5K_PHY_TIMING_3,
		AR5K_PHY_TIMING_3_DSC_MAN, ds_coef_man);
	AR5K_REG_WRITE_BITS(ah, AR5K_PHY_TIMING_3,
		AR5K_PHY_TIMING_3_DSC_EXP, ds_coef_exp);

	return 0;
}



static const unsigned int control_rates[] =
	{ 0, 1, 1, 1, 4, 4, 6, 6, 8, 8, 8, 8 };


static inline void ath5k_hw_write_rate_duration(struct ath5k_hw *ah,
       unsigned int mode)
{
	struct ath5k_softc *sc = ah->ah_sc;
	struct ieee80211_rate *rate;
	unsigned int i;

	
	for (i = 0; i < sc->sbands[IEEE80211_BAND_2GHZ].n_bitrates; i++) {
		u32 reg;
		u16 tx_time;

		rate = &sc->sbands[IEEE80211_BAND_2GHZ].bitrates[control_rates[i]];

		
		reg = AR5K_RATE_DUR(rate->hw_value);

		
		tx_time = le16_to_cpu(ieee80211_generic_frame_duration(sc->hw,
							sc->vif, 10, rate));

		ath5k_hw_reg_write(ah, tx_time, reg);

		if (!(rate->flags & IEEE80211_RATE_SHORT_PREAMBLE))
			continue;

		
		ath5k_hw_reg_write(ah, tx_time,
			reg + (AR5K_SET_SHORT_PREAMBLE << 2));
	}
}


static int ath5k_hw_nic_reset(struct ath5k_hw *ah, u32 val)
{
	int ret;
	u32 mask = val ? val : ~0U;

	ATH5K_TRACE(ah->ah_sc);

	
	ath5k_hw_reg_read(ah, AR5K_RXDP);

	
	ath5k_hw_reg_write(ah, val, AR5K_RESET_CTL);

	
	udelay(15);

	if (ah->ah_version == AR5K_AR5210) {
		val &= AR5K_RESET_CTL_PCU | AR5K_RESET_CTL_DMA
			| AR5K_RESET_CTL_MAC | AR5K_RESET_CTL_PHY;
		mask &= AR5K_RESET_CTL_PCU | AR5K_RESET_CTL_DMA
			| AR5K_RESET_CTL_MAC | AR5K_RESET_CTL_PHY;
	} else {
		val &= AR5K_RESET_CTL_PCU | AR5K_RESET_CTL_BASEBAND;
		mask &= AR5K_RESET_CTL_PCU | AR5K_RESET_CTL_BASEBAND;
	}

	ret = ath5k_hw_register_timeout(ah, AR5K_RESET_CTL, mask, val, false);

	
	if ((val & AR5K_RESET_CTL_PCU) == 0)
		ath5k_hw_reg_write(ah, AR5K_INIT_CFG, AR5K_CFG);

	return ret;
}


int ath5k_hw_set_power(struct ath5k_hw *ah, enum ath5k_power_mode mode,
		bool set_chip, u16 sleep_duration)
{
	unsigned int i;
	u32 staid, data;

	ATH5K_TRACE(ah->ah_sc);
	staid = ath5k_hw_reg_read(ah, AR5K_STA_ID1);

	switch (mode) {
	case AR5K_PM_AUTO:
		staid &= ~AR5K_STA_ID1_DEFAULT_ANTENNA;
		
	case AR5K_PM_NETWORK_SLEEP:
		if (set_chip)
			ath5k_hw_reg_write(ah,
				AR5K_SLEEP_CTL_SLE_ALLOW |
				sleep_duration,
				AR5K_SLEEP_CTL);

		staid |= AR5K_STA_ID1_PWR_SV;
		break;

	case AR5K_PM_FULL_SLEEP:
		if (set_chip)
			ath5k_hw_reg_write(ah, AR5K_SLEEP_CTL_SLE_SLP,
				AR5K_SLEEP_CTL);

		staid |= AR5K_STA_ID1_PWR_SV;
		break;

	case AR5K_PM_AWAKE:

		staid &= ~AR5K_STA_ID1_PWR_SV;

		if (!set_chip)
			goto commit;

		data = ath5k_hw_reg_read(ah, AR5K_SLEEP_CTL);

		
		if (data & 0xffc00000)
			data = 0;
		else
			
			data = data & ~AR5K_SLEEP_CTL_SLE;

		ath5k_hw_reg_write(ah, data | AR5K_SLEEP_CTL_SLE_WAKE,
							AR5K_SLEEP_CTL);
		udelay(15);

		for (i = 200; i > 0; i--) {
			
			if ((ath5k_hw_reg_read(ah, AR5K_PCICFG) &
					AR5K_PCICFG_SPWR_DN) == 0)
				break;

			
			udelay(50);
			ath5k_hw_reg_write(ah, data | AR5K_SLEEP_CTL_SLE_WAKE,
							AR5K_SLEEP_CTL);
		}

		
		if (i == 0)
			return -EIO;

		break;

	default:
		return -EINVAL;
	}

commit:
	ath5k_hw_reg_write(ah, staid, AR5K_STA_ID1);

	return 0;
}


int ath5k_hw_on_hold(struct ath5k_hw *ah)
{
	struct pci_dev *pdev = ah->ah_sc->pdev;
	u32 bus_flags;
	int ret;

	
	ret = ath5k_hw_set_power(ah, AR5K_PM_AWAKE, true, 0);
	if (ret) {
		ATH5K_ERR(ah->ah_sc, "failed to wakeup the MAC Chip\n");
		return ret;
	}

	
	bus_flags = (pdev->is_pcie) ? 0 : AR5K_RESET_CTL_PCI;

	if (ah->ah_version == AR5K_AR5210) {
		ret = ath5k_hw_nic_reset(ah, AR5K_RESET_CTL_PCU |
			AR5K_RESET_CTL_MAC | AR5K_RESET_CTL_DMA |
			AR5K_RESET_CTL_PHY | AR5K_RESET_CTL_PCI);
			mdelay(2);
	} else {
		ret = ath5k_hw_nic_reset(ah, AR5K_RESET_CTL_PCU |
			AR5K_RESET_CTL_BASEBAND | bus_flags);
	}

	if (ret) {
		ATH5K_ERR(ah->ah_sc, "failed to put device on warm reset\n");
		return -EIO;
	}

	
	ret = ath5k_hw_set_power(ah, AR5K_PM_AWAKE, true, 0);
	if (ret) {
		ATH5K_ERR(ah->ah_sc, "failed to put device on hold\n");
		return ret;
	}

	return ret;
}


int ath5k_hw_nic_wakeup(struct ath5k_hw *ah, int flags, bool initial)
{
	struct pci_dev *pdev = ah->ah_sc->pdev;
	u32 turbo, mode, clock, bus_flags;
	int ret;

	turbo = 0;
	mode = 0;
	clock = 0;

	ATH5K_TRACE(ah->ah_sc);

	
	ret = ath5k_hw_set_power(ah, AR5K_PM_AWAKE, true, 0);
	if (ret) {
		ATH5K_ERR(ah->ah_sc, "failed to wakeup the MAC Chip\n");
		return ret;
	}

	
	bus_flags = (pdev->is_pcie) ? 0 : AR5K_RESET_CTL_PCI;

	if (ah->ah_version == AR5K_AR5210) {
		ret = ath5k_hw_nic_reset(ah, AR5K_RESET_CTL_PCU |
			AR5K_RESET_CTL_MAC | AR5K_RESET_CTL_DMA |
			AR5K_RESET_CTL_PHY | AR5K_RESET_CTL_PCI);
			mdelay(2);
	} else {
		ret = ath5k_hw_nic_reset(ah, AR5K_RESET_CTL_PCU |
			AR5K_RESET_CTL_BASEBAND | bus_flags);
	}

	if (ret) {
		ATH5K_ERR(ah->ah_sc, "failed to reset the MAC Chip\n");
		return -EIO;
	}

	
	ret = ath5k_hw_set_power(ah, AR5K_PM_AWAKE, true, 0);
	if (ret) {
		ATH5K_ERR(ah->ah_sc, "failed to resume the MAC Chip\n");
		return ret;
	}

	
	if (ath5k_hw_nic_reset(ah, 0)) {
		ATH5K_ERR(ah->ah_sc, "failed to warm reset the MAC Chip\n");
		return -EIO;
	}

	
	if (initial)
		return 0;

	if (ah->ah_version != AR5K_AR5210) {
		

		if (ah->ah_radio >= AR5K_RF5112) {
			mode = AR5K_PHY_MODE_RAD_RF5112;
			clock = AR5K_PHY_PLL_RF5112;
		} else {
			mode = AR5K_PHY_MODE_RAD_RF5111;	
			clock = AR5K_PHY_PLL_RF5111;		
		}

		if (flags & CHANNEL_2GHZ) {
			mode |= AR5K_PHY_MODE_FREQ_2GHZ;
			clock |= AR5K_PHY_PLL_44MHZ;

			if (flags & CHANNEL_CCK) {
				mode |= AR5K_PHY_MODE_MOD_CCK;
			} else if (flags & CHANNEL_OFDM) {
				
				if (ah->ah_version == AR5K_AR5211)
					mode |= AR5K_PHY_MODE_MOD_OFDM;
				else
					mode |= AR5K_PHY_MODE_MOD_DYN;
			} else {
				ATH5K_ERR(ah->ah_sc,
					"invalid radio modulation mode\n");
				return -EINVAL;
			}
		} else if (flags & CHANNEL_5GHZ) {
			mode |= AR5K_PHY_MODE_FREQ_5GHZ;

			if (ah->ah_radio == AR5K_RF5413)
				clock = AR5K_PHY_PLL_40MHZ_5413;
			else
				clock |= AR5K_PHY_PLL_40MHZ;

			if (flags & CHANNEL_OFDM)
				mode |= AR5K_PHY_MODE_MOD_OFDM;
			else {
				ATH5K_ERR(ah->ah_sc,
					"invalid radio modulation mode\n");
				return -EINVAL;
			}
		} else {
			ATH5K_ERR(ah->ah_sc, "invalid radio frequency mode\n");
			return -EINVAL;
		}

		if (flags & CHANNEL_TURBO)
			turbo = AR5K_PHY_TURBO_MODE | AR5K_PHY_TURBO_SHORT;
	} else { 

		
		if (flags & CHANNEL_TURBO)
			ath5k_hw_reg_write(ah, AR5K_PHY_TURBO_MODE,
					AR5K_PHY_TURBO);
	}

	if (ah->ah_version != AR5K_AR5210) {

		
		if (ath5k_hw_reg_read(ah, AR5K_PHY_PLL) != clock) {
			ath5k_hw_reg_write(ah, clock, AR5K_PHY_PLL);
			udelay(300);
		}

		
		ath5k_hw_reg_write(ah, mode, AR5K_PHY_MODE);
		ath5k_hw_reg_write(ah, turbo, AR5K_PHY_TURBO);
	}

	return 0;
}


static void ath5k_hw_set_sleep_clock(struct ath5k_hw *ah, bool enable)
{
	struct ath5k_eeprom_info *ee = &ah->ah_capabilities.cap_eeprom;
	u32 scal, spending, usec32;

	
	if ((AR5K_EEPROM_HAS32KHZCRYSTAL(ee->ee_misc1) ||
	AR5K_EEPROM_HAS32KHZCRYSTAL_OLD(ee->ee_misc1)) &&
	enable) {

		
		AR5K_REG_WRITE_BITS(ah, AR5K_USEC_5211, AR5K_USEC_32, 1);
		
		AR5K_REG_WRITE_BITS(ah, AR5K_TSF_PARM, AR5K_TSF_PARM_INC, 61);

		
		ath5k_hw_reg_write(ah, 0x1f, AR5K_PHY_SCR);

		if ((ah->ah_radio == AR5K_RF5112) ||
		(ah->ah_radio == AR5K_RF5413) ||
		(ah->ah_mac_version == (AR5K_SREV_AR2417 >> 4)))
			spending = 0x14;
		else
			spending = 0x18;
		ath5k_hw_reg_write(ah, spending, AR5K_PHY_SPENDING);

		if ((ah->ah_radio == AR5K_RF5112) ||
		(ah->ah_radio == AR5K_RF5413) ||
		(ah->ah_mac_version == (AR5K_SREV_AR2417 >> 4))) {
			ath5k_hw_reg_write(ah, 0x26, AR5K_PHY_SLMT);
			ath5k_hw_reg_write(ah, 0x0d, AR5K_PHY_SCAL);
			ath5k_hw_reg_write(ah, 0x07, AR5K_PHY_SCLOCK);
			ath5k_hw_reg_write(ah, 0x3f, AR5K_PHY_SDELAY);
			AR5K_REG_WRITE_BITS(ah, AR5K_PCICFG,
				AR5K_PCICFG_SLEEP_CLOCK_RATE, 0x02);
		} else {
			ath5k_hw_reg_write(ah, 0x0a, AR5K_PHY_SLMT);
			ath5k_hw_reg_write(ah, 0x0c, AR5K_PHY_SCAL);
			ath5k_hw_reg_write(ah, 0x03, AR5K_PHY_SCLOCK);
			ath5k_hw_reg_write(ah, 0x20, AR5K_PHY_SDELAY);
			AR5K_REG_WRITE_BITS(ah, AR5K_PCICFG,
				AR5K_PCICFG_SLEEP_CLOCK_RATE, 0x03);
		}

		
		AR5K_REG_ENABLE_BITS(ah, AR5K_PCICFG,
				AR5K_PCICFG_SLEEP_CLOCK_EN);

	} else {

		
		AR5K_REG_DISABLE_BITS(ah, AR5K_PCICFG,
				AR5K_PCICFG_SLEEP_CLOCK_EN);

		AR5K_REG_WRITE_BITS(ah, AR5K_PCICFG,
				AR5K_PCICFG_SLEEP_CLOCK_RATE, 0);

		ath5k_hw_reg_write(ah, 0x1f, AR5K_PHY_SCR);
		ath5k_hw_reg_write(ah, AR5K_PHY_SLMT_32MHZ, AR5K_PHY_SLMT);

		if (ah->ah_mac_version == (AR5K_SREV_AR2417 >> 4))
			scal = AR5K_PHY_SCAL_32MHZ_2417;
		else if (ee->ee_is_hb63)
			scal = AR5K_PHY_SCAL_32MHZ_HB63;
		else
			scal = AR5K_PHY_SCAL_32MHZ;
		ath5k_hw_reg_write(ah, scal, AR5K_PHY_SCAL);

		ath5k_hw_reg_write(ah, AR5K_PHY_SCLOCK_32MHZ, AR5K_PHY_SCLOCK);
		ath5k_hw_reg_write(ah, AR5K_PHY_SDELAY_32MHZ, AR5K_PHY_SDELAY);

		if ((ah->ah_radio == AR5K_RF5112) ||
		(ah->ah_radio == AR5K_RF5413) ||
		(ah->ah_mac_version == (AR5K_SREV_AR2417 >> 4)))
			spending = 0x14;
		else
			spending = 0x18;
		ath5k_hw_reg_write(ah, spending, AR5K_PHY_SPENDING);

		if ((ah->ah_radio == AR5K_RF5112) ||
		(ah->ah_radio == AR5K_RF5413))
			usec32 = 39;
		else
			usec32 = 31;
		AR5K_REG_WRITE_BITS(ah, AR5K_USEC_5211, AR5K_USEC_32, usec32);

		AR5K_REG_WRITE_BITS(ah, AR5K_TSF_PARM, AR5K_TSF_PARM_INC, 1);
	}
	return;
}


static void ath5k_hw_tweak_initval_settings(struct ath5k_hw *ah,
				struct ieee80211_channel *channel)
{
	if (ah->ah_version == AR5K_AR5212 &&
	    ah->ah_phy_revision >= AR5K_SREV_PHY_5212A) {

		
		ath5k_hw_reg_write(ah,
				(AR5K_REG_SM(2,
				AR5K_PHY_ADC_CTL_INBUFGAIN_OFF) |
				AR5K_REG_SM(2,
				AR5K_PHY_ADC_CTL_INBUFGAIN_ON) |
				AR5K_PHY_ADC_CTL_PWD_DAC_OFF |
				AR5K_PHY_ADC_CTL_PWD_ADC_OFF),
				AR5K_PHY_ADC_CTL);



		
		AR5K_REG_DISABLE_BITS(ah, AR5K_PHY_DAG_CCK_CTL,
				AR5K_PHY_DAG_CCK_CTL_EN_RSSI_THR);

		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_DAG_CCK_CTL,
			AR5K_PHY_DAG_CCK_CTL_RSSI_THR, 2);

		
		ath5k_hw_reg_write(ah, 0x0000000f, AR5K_SEQ_MASK);
	}

	
	if (ah->ah_phy_revision >= AR5K_SREV_PHY_5212B)
		ath5k_hw_reg_write(ah, 0, AR5K_PHY_BLUETOOTH);

	
	if (ah->ah_phy_revision > AR5K_SREV_PHY_5212B)
		AR5K_REG_DISABLE_BITS(ah, AR5K_TXCFG,
				AR5K_TXCFG_DCU_DBL_BUF_DIS);

	
	if (ah->ah_version == AR5K_AR5212) {
		u32 scal;
		struct ath5k_eeprom_info *ee = &ah->ah_capabilities.cap_eeprom;
		if (ah->ah_mac_version == (AR5K_SREV_AR2417 >> 4))
			scal = AR5K_PHY_SCAL_32MHZ_2417;
		else if (ee->ee_is_hb63)
			scal = AR5K_PHY_SCAL_32MHZ_HB63;
		else
			scal = AR5K_PHY_SCAL_32MHZ;
		ath5k_hw_reg_write(ah, scal, AR5K_PHY_SCAL);
	}

	
	if ((ah->ah_radio == AR5K_RF5413) ||
	(ah->ah_mac_version == (AR5K_SREV_AR2417 >> 4))) {
		u32 fast_adc = true;

		if (channel->center_freq == 2462 ||
		channel->center_freq == 2467)
			fast_adc = 0;

		
		if (ath5k_hw_reg_read(ah, AR5K_PHY_FAST_ADC) != fast_adc)
				ath5k_hw_reg_write(ah, fast_adc,
						AR5K_PHY_FAST_ADC);
	}

	
	if (ah->ah_radio == AR5K_RF5112 &&
			ah->ah_radio_5ghz_revision <
			AR5K_SREV_RAD_5112A) {
		u32 data;
		ath5k_hw_reg_write(ah, AR5K_PHY_CCKTXCTL_WORLD,
				AR5K_PHY_CCKTXCTL);
		if (channel->hw_value & CHANNEL_5GHZ)
			data = 0xffb81020;
		else
			data = 0xffb80d20;
		ath5k_hw_reg_write(ah, data, AR5K_PHY_FRAME_CTL);
	}

	if (ah->ah_mac_srev < AR5K_SREV_AR5211) {
		u32 usec_reg;
		
		usec_reg = ath5k_hw_reg_read(ah, AR5K_USEC_5211);
		ath5k_hw_reg_write(ah, usec_reg & (AR5K_USEC_1 |
				AR5K_USEC_32 |
				AR5K_USEC_TX_LATENCY_5211 |
				AR5K_REG_SM(29,
				AR5K_USEC_RX_LATENCY_5210)),
				AR5K_USEC_5211);
		
		ath5k_hw_reg_write(ah, 0, AR5K_QCUDCU_CLKGT);
		
		ath5k_hw_reg_write(ah, 0x08, AR5K_PHY_SCAL);
		
		AR5K_REG_ENABLE_BITS(ah, AR5K_DIAG_SW_5211,
					AR5K_DIAG_SW_ECO_ENABLE);
	}
}

static void ath5k_hw_commit_eeprom_settings(struct ath5k_hw *ah,
		struct ieee80211_channel *channel, u8 *ant, u8 ee_mode)
{
	struct ath5k_eeprom_info *ee = &ah->ah_capabilities.cap_eeprom;
	s16 cck_ofdm_pwr_delta;

	
	if (channel->center_freq == 2484)
		cck_ofdm_pwr_delta =
			((ee->ee_cck_ofdm_power_delta -
			ee->ee_scaled_cck_delta) * 2) / 10;
	else
		cck_ofdm_pwr_delta =
			(ee->ee_cck_ofdm_power_delta * 2) / 10;

	
	if (ah->ah_phy_revision >= AR5K_SREV_PHY_5212A) {
		if (channel->hw_value == CHANNEL_G)
			ath5k_hw_reg_write(ah,
			AR5K_REG_SM((ee->ee_cck_ofdm_gain_delta * -1),
				AR5K_PHY_TX_PWR_ADJ_CCK_GAIN_DELTA) |
			AR5K_REG_SM((cck_ofdm_pwr_delta * -1),
				AR5K_PHY_TX_PWR_ADJ_CCK_PCDAC_INDEX),
				AR5K_PHY_TX_PWR_ADJ);
		else
			ath5k_hw_reg_write(ah, 0, AR5K_PHY_TX_PWR_ADJ);
	} else {
		
		ah->ah_txpower.txp_cck_ofdm_pwr_delta = cck_ofdm_pwr_delta;
		ah->ah_txpower.txp_cck_ofdm_gainf_delta =
						ee->ee_cck_ofdm_gain_delta;
	}

	
	AR5K_REG_WRITE_BITS(ah, AR5K_PHY_ANT_CTL,
			AR5K_PHY_ANT_CTL_SWTABLE_IDLE,
			(ah->ah_ant_ctl[ee_mode][0] |
			AR5K_PHY_ANT_CTL_TXRX_EN));

	
	ath5k_hw_reg_write(ah, ah->ah_ant_ctl[ee_mode][ant[0]],
		AR5K_PHY_ANT_SWITCH_TABLE_0);
	ath5k_hw_reg_write(ah, ah->ah_ant_ctl[ee_mode][ant[1]],
		AR5K_PHY_ANT_SWITCH_TABLE_1);

	
	ath5k_hw_reg_write(ah,
		AR5K_PHY_NF_SVAL(ee->ee_noise_floor_thr[ee_mode]),
		AR5K_PHY_NFTHRES);

	if ((channel->hw_value & CHANNEL_TURBO) &&
	(ah->ah_ee_version >= AR5K_EEPROM_VERSION_5_0)) {
		
		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_SETTLING,
				AR5K_PHY_SETTLING_SWITCH,
				ee->ee_switch_settling_turbo[ee_mode]);

		
		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_GAIN,
				AR5K_PHY_GAIN_TXRX_ATTEN,
				ee->ee_atn_tx_rx_turbo[ee_mode]);

		
		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_DESIRED_SIZE,
				AR5K_PHY_DESIRED_SIZE_ADC,
				ee->ee_adc_desired_size_turbo[ee_mode]);

		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_DESIRED_SIZE,
				AR5K_PHY_DESIRED_SIZE_PGA,
				ee->ee_pga_desired_size_turbo[ee_mode]);

		
		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_GAIN_2GHZ,
				AR5K_PHY_GAIN_2GHZ_MARGIN_TXRX,
				ee->ee_margin_tx_rx_turbo[ee_mode]);

	} else {
		
		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_SETTLING,
				AR5K_PHY_SETTLING_SWITCH,
				ee->ee_switch_settling[ee_mode]);

		
		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_GAIN,
				AR5K_PHY_GAIN_TXRX_ATTEN,
				ee->ee_atn_tx_rx[ee_mode]);

		
		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_DESIRED_SIZE,
				AR5K_PHY_DESIRED_SIZE_ADC,
				ee->ee_adc_desired_size[ee_mode]);

		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_DESIRED_SIZE,
				AR5K_PHY_DESIRED_SIZE_PGA,
				ee->ee_pga_desired_size[ee_mode]);

		
		if (ah->ah_ee_version >= AR5K_EEPROM_VERSION_4_1)
			AR5K_REG_WRITE_BITS(ah, AR5K_PHY_GAIN_2GHZ,
				AR5K_PHY_GAIN_2GHZ_MARGIN_TXRX,
				ee->ee_margin_tx_rx[ee_mode]);
	}

	
	ath5k_hw_reg_write(ah,
		(ee->ee_tx_end2xpa_disable[ee_mode] << 24) |
		(ee->ee_tx_end2xpa_disable[ee_mode] << 16) |
		(ee->ee_tx_frm2xpa_enable[ee_mode] << 8) |
		(ee->ee_tx_frm2xpa_enable[ee_mode]), AR5K_PHY_RF_CTL4);

	
	AR5K_REG_WRITE_BITS(ah, AR5K_PHY_RF_CTL3,
			AR5K_PHY_RF_CTL3_TXE2XLNA_ON,
			ee->ee_tx_end2xlna_enable[ee_mode]);

	
	AR5K_REG_WRITE_BITS(ah, AR5K_PHY_NF,
			AR5K_PHY_NF_THRESH62,
			ee->ee_thr_62[ee_mode]);


	
	if (ath5k_hw_chan_has_spur_noise(ah, channel))
		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_OFDM_SELFCORR,
				AR5K_PHY_OFDM_SELFCORR_CYPWR_THR1,
				AR5K_INIT_CYCRSSI_THR1 +
				ee->ee_false_detect[ee_mode]);
	else
		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_OFDM_SELFCORR,
				AR5K_PHY_OFDM_SELFCORR_CYPWR_THR1,
				AR5K_INIT_CYCRSSI_THR1);

	
	AR5K_REG_ENABLE_BITS(ah, AR5K_PHY_IQ,
		AR5K_PHY_IQ_CORR_ENABLE |
		(ee->ee_i_cal[ee_mode] << AR5K_PHY_IQ_CORR_Q_I_COFF_S) |
		ee->ee_q_cal[ee_mode]);

	
	if (ah->ah_ee_version >= AR5K_EEPROM_VERSION_5_1)
		ath5k_hw_reg_write(ah, 0, AR5K_PHY_HEAVY_CLIP_ENABLE);

	return;
}


int ath5k_hw_reset(struct ath5k_hw *ah, enum nl80211_iftype op_mode,
	struct ieee80211_channel *channel, bool change_channel)
{
	u32 s_seq[10], s_ant, s_led[3], staid1_flags, tsf_up, tsf_lo;
	u32 phy_tst1;
	u8 mode, freq, ee_mode, ant[2];
	int i, ret;

	ATH5K_TRACE(ah->ah_sc);

	s_ant = 0;
	ee_mode = 0;
	staid1_flags = 0;
	tsf_up = 0;
	tsf_lo = 0;
	freq = 0;
	mode = 0;

	
	
	if (ah->ah_version != AR5K_AR5210) {

		switch (channel->hw_value & CHANNEL_MODES) {
		case CHANNEL_A:
			mode = AR5K_MODE_11A;
			freq = AR5K_INI_RFGAIN_5GHZ;
			ee_mode = AR5K_EEPROM_MODE_11A;
			break;
		case CHANNEL_G:
			mode = AR5K_MODE_11G;
			freq = AR5K_INI_RFGAIN_2GHZ;
			ee_mode = AR5K_EEPROM_MODE_11G;
			break;
		case CHANNEL_B:
			mode = AR5K_MODE_11B;
			freq = AR5K_INI_RFGAIN_2GHZ;
			ee_mode = AR5K_EEPROM_MODE_11B;
			break;
		case CHANNEL_T:
			mode = AR5K_MODE_11A_TURBO;
			freq = AR5K_INI_RFGAIN_5GHZ;
			ee_mode = AR5K_EEPROM_MODE_11A;
			break;
		case CHANNEL_TG:
			if (ah->ah_version == AR5K_AR5211) {
				ATH5K_ERR(ah->ah_sc,
					"TurboG mode not available on 5211");
				return -EINVAL;
			}
			mode = AR5K_MODE_11G_TURBO;
			freq = AR5K_INI_RFGAIN_2GHZ;
			ee_mode = AR5K_EEPROM_MODE_11G;
			break;
		case CHANNEL_XR:
			if (ah->ah_version == AR5K_AR5211) {
				ATH5K_ERR(ah->ah_sc,
					"XR mode not available on 5211");
				return -EINVAL;
			}
			mode = AR5K_MODE_XR;
			freq = AR5K_INI_RFGAIN_5GHZ;
			ee_mode = AR5K_EEPROM_MODE_11A;
			break;
		default:
			ATH5K_ERR(ah->ah_sc,
				"invalid channel: %d\n", channel->center_freq);
			return -EINVAL;
		}

		if (change_channel) {
			
			if (ah->ah_mac_srev < AR5K_SREV_AR5211) {

				for (i = 0; i < 10; i++)
					s_seq[i] = ath5k_hw_reg_read(ah,
						AR5K_QUEUE_DCU_SEQNUM(i));

			} else {
				s_seq[0] = ath5k_hw_reg_read(ah,
						AR5K_QUEUE_DCU_SEQNUM(0));
			}

			
			if (ah->ah_version == AR5K_AR5211) {
				tsf_up = ath5k_hw_reg_read(ah, AR5K_TSF_U32);
				tsf_lo = ath5k_hw_reg_read(ah, AR5K_TSF_L32);
			}
		}

		
		s_ant = ath5k_hw_reg_read(ah, AR5K_DEFAULT_ANTENNA);

		if (ah->ah_version == AR5K_AR5212) {
			
			ath5k_hw_set_sleep_clock(ah, false);

			
			if (change_channel && ah->ah_rf_banks != NULL)
				ath5k_hw_gainf_calibrate(ah);
		}
	}

	
	s_led[0] = ath5k_hw_reg_read(ah, AR5K_PCICFG) &
					AR5K_PCICFG_LEDSTATE;
	s_led[1] = ath5k_hw_reg_read(ah, AR5K_GPIOCR);
	s_led[2] = ath5k_hw_reg_read(ah, AR5K_GPIODO);

	
	staid1_flags = ath5k_hw_reg_read(ah, AR5K_STA_ID1) &
			(AR5K_STA_ID1_DEFAULT_ANTENNA |
			AR5K_STA_ID1_DESC_ANTENNA |
			AR5K_STA_ID1_RTS_DEF_ANTENNA |
			AR5K_STA_ID1_ACKCTS_6MB |
			AR5K_STA_ID1_BASE_RATE_11B |
			AR5K_STA_ID1_SELFGEN_DEF_ANT);

	
	ret = ath5k_hw_nic_wakeup(ah, channel->hw_value, false);
	if (ret)
		return ret;

	
	ah->ah_op_mode = op_mode;

	
	if (ah->ah_mac_srev >= AR5K_SREV_AR5211)
		ath5k_hw_reg_write(ah, AR5K_PHY_SHIFT_5GHZ, AR5K_PHY(0));
	else
		ath5k_hw_reg_write(ah, AR5K_PHY_SHIFT_5GHZ | 0x40,
							AR5K_PHY(0));

	
	ret = ath5k_hw_write_initvals(ah, mode, change_channel);
	if (ret)
		return ret;

	
	if (ah->ah_version != AR5K_AR5210) {

		
		ret = ath5k_hw_rfgain_init(ah, freq);
		if (ret)
			return ret;

		mdelay(1);

		
		ath5k_hw_tweak_initval_settings(ah, channel);

		
		ret = ath5k_hw_txpower(ah, channel, ee_mode,
					ah->ah_txpower.txp_max_pwr / 2);
		if (ret)
			return ret;

		
		if (ah->ah_version == AR5K_AR5212 &&
			ah->ah_sc->vif != NULL)
			ath5k_hw_write_rate_duration(ah, mode);

		
		ret = ath5k_hw_rfregs_init(ah, channel, mode);
		if (ret)
			return ret;


		
		if (ah->ah_version == AR5K_AR5212 &&
			channel->hw_value & CHANNEL_OFDM) {
			struct ath5k_eeprom_info *ee =
					&ah->ah_capabilities.cap_eeprom;

			ret = ath5k_hw_write_ofdm_timings(ah, channel);
			if (ret)
				return ret;

			
			if (ah->ah_mac_srev >= AR5K_SREV_AR5424 ||
			(ah->ah_mac_srev >= AR5K_SREV_AR5424 &&
			ee->ee_version >= AR5K_EEPROM_VERSION_5_3))
				ath5k_hw_set_spur_mitigation_filter(ah,
								channel);
		}

		
		if (ah->ah_radio == AR5K_RF5111) {
			if (mode == AR5K_MODE_11B)
				AR5K_REG_ENABLE_BITS(ah, AR5K_TXCFG,
				    AR5K_TXCFG_B_MODE);
			else
				AR5K_REG_DISABLE_BITS(ah, AR5K_TXCFG,
				    AR5K_TXCFG_B_MODE);
		}

		
		if (ah->ah_ant_mode == AR5K_ANTMODE_FIXED_A)
				ant[0] = ant[1] = AR5K_ANT_SWTABLE_A;
		else if (ah->ah_ant_mode == AR5K_ANTMODE_FIXED_B)
				ant[0] = ant[1] = AR5K_ANT_SWTABLE_B;
		else {
			ant[0] = AR5K_ANT_SWTABLE_A;
			ant[1] = AR5K_ANT_SWTABLE_B;
		}

		
		ath5k_hw_commit_eeprom_settings(ah, channel, ant, ee_mode);

	} else {
		
		mdelay(1);
		
		ath5k_hw_reg_write(ah, AR5K_PHY_ACT_DISABLE, AR5K_PHY_ACT);
		mdelay(1);
	}

	

	
	if (ah->ah_version != AR5K_AR5210) {

		if (change_channel) {
			if (ah->ah_mac_srev < AR5K_SREV_AR5211) {
				for (i = 0; i < 10; i++)
					ath5k_hw_reg_write(ah, s_seq[i],
						AR5K_QUEUE_DCU_SEQNUM(i));
			} else {
				ath5k_hw_reg_write(ah, s_seq[0],
					AR5K_QUEUE_DCU_SEQNUM(0));
			}


			if (ah->ah_version == AR5K_AR5211) {
				ath5k_hw_reg_write(ah, tsf_up, AR5K_TSF_U32);
				ath5k_hw_reg_write(ah, tsf_lo, AR5K_TSF_L32);
			}
		}

		ath5k_hw_reg_write(ah, s_ant, AR5K_DEFAULT_ANTENNA);
	}

	
	AR5K_REG_ENABLE_BITS(ah, AR5K_PCICFG, s_led[0]);

	
	ath5k_hw_reg_write(ah, s_led[1], AR5K_GPIOCR);
	ath5k_hw_reg_write(ah, s_led[2], AR5K_GPIODO);

	
	ath5k_hw_reg_write(ah, AR5K_LOW_ID(ah->ah_sta_id),
						AR5K_STA_ID0);
	ath5k_hw_reg_write(ah, staid1_flags | AR5K_HIGH_ID(ah->ah_sta_id),
						AR5K_STA_ID1);


	

	
	
	ath5k_hw_set_associd(ah, ah->ah_bssid, 0);

	
	ath5k_hw_set_opmode(ah);

	
	if (ah->ah_version != AR5K_AR5210)
		ath5k_hw_reg_write(ah, 0xffffffff, AR5K_PISR);

	
	ath5k_hw_reg_write(ah, (AR5K_TUNE_RSSI_THRES |
				AR5K_TUNE_BMISS_THRES <<
				AR5K_RSSI_THR_BMISS_S),
				AR5K_RSSI_THR);

	
	if (ah->ah_mac_srev >= AR5K_SREV_AR2413) {
		ath5k_hw_reg_write(ah, 0x000100aa, AR5K_MIC_QOS_CTL);
		ath5k_hw_reg_write(ah, 0x00003210, AR5K_MIC_QOS_SEL);
	}

	
	if (ah->ah_version == AR5K_AR5212) {
		ath5k_hw_reg_write(ah,
			AR5K_REG_SM(2, AR5K_QOS_NOACK_2BIT_VALUES) |
			AR5K_REG_SM(5, AR5K_QOS_NOACK_BIT_OFFSET)  |
			AR5K_REG_SM(0, AR5K_QOS_NOACK_BYTE_OFFSET),
			AR5K_QOS_NOACK);
	}


	

	
	ret = ath5k_hw_channel(ah, channel);
	if (ret)
		return ret;

	
	ath5k_hw_reg_write(ah, AR5K_PHY_ACT_ENABLE, AR5K_PHY_ACT);

	
	if (ah->ah_version != AR5K_AR5210) {
		u32 delay;
		delay = ath5k_hw_reg_read(ah, AR5K_PHY_RX_DELAY) &
			AR5K_PHY_RX_DELAY_M;
		delay = (channel->hw_value & CHANNEL_CCK) ?
			((delay << 2) / 22) : (delay / 10);

		udelay(100 + (2 * delay));
	} else {
		mdelay(1);
	}

	
	phy_tst1 = ath5k_hw_reg_read(ah, AR5K_PHY_TST1);
	ath5k_hw_reg_write(ah, AR5K_PHY_TST1_TXHOLD, AR5K_PHY_TST1);
	for (i = 0; i <= 20; i++) {
		if (!(ath5k_hw_reg_read(ah, AR5K_PHY_ADC_TEST) & 0x10))
			break;
		udelay(200);
	}
	ath5k_hw_reg_write(ah, phy_tst1, AR5K_PHY_TST1);

	
	AR5K_REG_ENABLE_BITS(ah, AR5K_PHY_AGCCTL,
				AR5K_PHY_AGCCTL_CAL);

	
	ah->ah_calibration = false;
	if (!(mode == AR5K_MODE_11B)) {
		ah->ah_calibration = true;
		AR5K_REG_WRITE_BITS(ah, AR5K_PHY_IQ,
				AR5K_PHY_IQ_CAL_NUM_LOG_MAX, 15);
		AR5K_REG_ENABLE_BITS(ah, AR5K_PHY_IQ,
				AR5K_PHY_IQ_RUN);
	}

	
	if (ath5k_hw_register_timeout(ah, AR5K_PHY_AGCCTL,
			AR5K_PHY_AGCCTL_CAL, 0, false)) {
		ATH5K_ERR(ah->ah_sc, "gain calibration timeout (%uMHz)\n",
			channel->center_freq);
	}

	
	ath5k_hw_noise_floor_calibration(ah, channel->center_freq);

	
	ath5k_hw_set_antenna_mode(ah, ah->ah_ant_mode);

	

	
	

	
	for (i = 0; i < ah->ah_capabilities.cap_queues.q_tx_num; i++) {
		ret = ath5k_hw_reset_tx_queue(ah, i);
		if (ret) {
			ATH5K_ERR(ah->ah_sc,
				"failed to reset TX queue #%d\n", i);
			return ret;
		}
	}


	

	
	if (ah->ah_version != AR5K_AR5210) {
		AR5K_REG_WRITE_BITS(ah, AR5K_TXCFG,
			AR5K_TXCFG_SDMAMR, AR5K_DMASIZE_128B);
		AR5K_REG_WRITE_BITS(ah, AR5K_RXCFG,
			AR5K_RXCFG_SDMAMW, AR5K_DMASIZE_128B);
	}

	
	if (ah->ah_version != AR5K_AR5210)
		ath5k_hw_set_imr(ah, ah->ah_imr);

	
	if (ah->ah_version == AR5K_AR5212)
			ath5k_hw_set_sleep_clock(ah, true);

	
	AR5K_REG_DISABLE_BITS(ah, AR5K_BEACON, AR5K_BEACON_ENABLE |
			AR5K_BEACON_RESET_TSF);

	return 0;
}

#undef _ATH5K_RESET
