



#include "ath5k.h"
#include "reg.h"
#include "debug.h"
#include "base.h"




int ath5k_hw_set_opmode(struct ath5k_hw *ah)
{
	u32 pcu_reg, beacon_reg, low_id, high_id;


	
	pcu_reg = ath5k_hw_reg_read(ah, AR5K_STA_ID1) & 0xffff0000;
	pcu_reg &= ~(AR5K_STA_ID1_ADHOC | AR5K_STA_ID1_AP
			| AR5K_STA_ID1_KEYSRCH_MODE
			| (ah->ah_version == AR5K_AR5210 ?
			(AR5K_STA_ID1_PWR_SV | AR5K_STA_ID1_NO_PSPOLL) : 0));

	beacon_reg = 0;

	ATH5K_TRACE(ah->ah_sc);

	switch (ah->ah_op_mode) {
	case NL80211_IFTYPE_ADHOC:
		pcu_reg |= AR5K_STA_ID1_ADHOC | AR5K_STA_ID1_KEYSRCH_MODE;
		beacon_reg |= AR5K_BCR_ADHOC;
		if (ah->ah_version == AR5K_AR5210)
			pcu_reg |= AR5K_STA_ID1_NO_PSPOLL;
		else
			AR5K_REG_ENABLE_BITS(ah, AR5K_CFG, AR5K_CFG_IBSS);
		break;

	case NL80211_IFTYPE_AP:
	case NL80211_IFTYPE_MESH_POINT:
		pcu_reg |= AR5K_STA_ID1_AP | AR5K_STA_ID1_KEYSRCH_MODE;
		beacon_reg |= AR5K_BCR_AP;
		if (ah->ah_version == AR5K_AR5210)
			pcu_reg |= AR5K_STA_ID1_NO_PSPOLL;
		else
			AR5K_REG_DISABLE_BITS(ah, AR5K_CFG, AR5K_CFG_IBSS);
		break;

	case NL80211_IFTYPE_STATION:
		pcu_reg |= AR5K_STA_ID1_KEYSRCH_MODE
			| (ah->ah_version == AR5K_AR5210 ?
				AR5K_STA_ID1_PWR_SV : 0);
	case NL80211_IFTYPE_MONITOR:
		pcu_reg |= AR5K_STA_ID1_KEYSRCH_MODE
			| (ah->ah_version == AR5K_AR5210 ?
				AR5K_STA_ID1_NO_PSPOLL : 0);
		break;

	default:
		return -EINVAL;
	}

	
	low_id = AR5K_LOW_ID(ah->ah_sta_id);
	high_id = AR5K_HIGH_ID(ah->ah_sta_id);
	ath5k_hw_reg_write(ah, low_id, AR5K_STA_ID0);
	ath5k_hw_reg_write(ah, pcu_reg | high_id, AR5K_STA_ID1);

	
	if (ah->ah_version == AR5K_AR5210)
		ath5k_hw_reg_write(ah, beacon_reg, AR5K_BCR);

	return 0;
}


void ath5k_hw_update_mib_counters(struct ath5k_hw *ah,
		struct ieee80211_low_level_stats  *stats)
{
	ATH5K_TRACE(ah->ah_sc);

	
	stats->dot11ACKFailureCount += ath5k_hw_reg_read(ah, AR5K_ACK_FAIL);
	stats->dot11RTSFailureCount += ath5k_hw_reg_read(ah, AR5K_RTS_FAIL);
	stats->dot11RTSSuccessCount += ath5k_hw_reg_read(ah, AR5K_RTS_OK);
	stats->dot11FCSErrorCount += ath5k_hw_reg_read(ah, AR5K_FCS_FAIL);

	
	ath5k_hw_reg_read(ah, AR5K_BEACON_CNT);

	
	if (ah->ah_version == AR5K_AR5212) {
		ath5k_hw_reg_write(ah, 0, AR5K_PROFCNT_TX);
		ath5k_hw_reg_write(ah, 0, AR5K_PROFCNT_RX);
		ath5k_hw_reg_write(ah, 0, AR5K_PROFCNT_RXCLR);
		ath5k_hw_reg_write(ah, 0, AR5K_PROFCNT_CYCLE);
	}

	
}


void ath5k_hw_set_ack_bitrate_high(struct ath5k_hw *ah, bool high)
{
	if (ah->ah_version != AR5K_AR5212)
		return;
	else {
		u32 val = AR5K_STA_ID1_BASE_RATE_11B | AR5K_STA_ID1_ACKCTS_6MB;
		if (high)
			AR5K_REG_ENABLE_BITS(ah, AR5K_STA_ID1, val);
		else
			AR5K_REG_DISABLE_BITS(ah, AR5K_STA_ID1, val);
	}
}





unsigned int ath5k_hw_get_ack_timeout(struct ath5k_hw *ah)
{
	ATH5K_TRACE(ah->ah_sc);

	return ath5k_hw_clocktoh(AR5K_REG_MS(ath5k_hw_reg_read(ah,
			AR5K_TIME_OUT), AR5K_TIME_OUT_ACK), ah->ah_turbo);
}


int ath5k_hw_set_ack_timeout(struct ath5k_hw *ah, unsigned int timeout)
{
	ATH5K_TRACE(ah->ah_sc);
	if (ath5k_hw_clocktoh(AR5K_REG_MS(0xffffffff, AR5K_TIME_OUT_ACK),
			ah->ah_turbo) <= timeout)
		return -EINVAL;

	AR5K_REG_WRITE_BITS(ah, AR5K_TIME_OUT, AR5K_TIME_OUT_ACK,
		ath5k_hw_htoclock(timeout, ah->ah_turbo));

	return 0;
}


unsigned int ath5k_hw_get_cts_timeout(struct ath5k_hw *ah)
{
	ATH5K_TRACE(ah->ah_sc);
	return ath5k_hw_clocktoh(AR5K_REG_MS(ath5k_hw_reg_read(ah,
			AR5K_TIME_OUT), AR5K_TIME_OUT_CTS), ah->ah_turbo);
}


int ath5k_hw_set_cts_timeout(struct ath5k_hw *ah, unsigned int timeout)
{
	ATH5K_TRACE(ah->ah_sc);
	if (ath5k_hw_clocktoh(AR5K_REG_MS(0xffffffff, AR5K_TIME_OUT_CTS),
			ah->ah_turbo) <= timeout)
		return -EINVAL;

	AR5K_REG_WRITE_BITS(ah, AR5K_TIME_OUT, AR5K_TIME_OUT_CTS,
			ath5k_hw_htoclock(timeout, ah->ah_turbo));

	return 0;
}





void ath5k_hw_get_lladdr(struct ath5k_hw *ah, u8 *mac)
{
	ATH5K_TRACE(ah->ah_sc);
	memcpy(mac, ah->ah_sta_id, ETH_ALEN);
}


int ath5k_hw_set_lladdr(struct ath5k_hw *ah, const u8 *mac)
{
	u32 low_id, high_id;
	u32 pcu_reg;

	ATH5K_TRACE(ah->ah_sc);
	
	memcpy(ah->ah_sta_id, mac, ETH_ALEN);

	pcu_reg = ath5k_hw_reg_read(ah, AR5K_STA_ID1) & 0xffff0000;

	low_id = AR5K_LOW_ID(mac);
	high_id = AR5K_HIGH_ID(mac);

	ath5k_hw_reg_write(ah, low_id, AR5K_STA_ID0);
	ath5k_hw_reg_write(ah, pcu_reg | high_id, AR5K_STA_ID1);

	return 0;
}


void ath5k_hw_set_associd(struct ath5k_hw *ah, const u8 *bssid, u16 assoc_id)
{
	u32 low_id, high_id;
	u16 tim_offset = 0;

	
	if (ah->ah_version == AR5K_AR5212) {
		ath5k_hw_reg_write(ah, AR5K_LOW_ID(ah->ah_bssid_mask),
							AR5K_BSS_IDM0);
		ath5k_hw_reg_write(ah, AR5K_HIGH_ID(ah->ah_bssid_mask),
							AR5K_BSS_IDM1);
	}

	
	low_id = AR5K_LOW_ID(bssid);
	high_id = AR5K_HIGH_ID(bssid);
	ath5k_hw_reg_write(ah, low_id, AR5K_BSS_ID0);
	ath5k_hw_reg_write(ah, high_id | ((assoc_id & 0x3fff) <<
				AR5K_BSS_ID1_AID_S), AR5K_BSS_ID1);

	if (assoc_id == 0) {
		ath5k_hw_disable_pspoll(ah);
		return;
	}

	AR5K_REG_WRITE_BITS(ah, AR5K_BEACON, AR5K_BEACON_TIM,
			tim_offset ? tim_offset + 4 : 0);

	ath5k_hw_enable_pspoll(ah, NULL, 0);
}



int ath5k_hw_set_bssid_mask(struct ath5k_hw *ah, const u8 *mask)
{
	u32 low_id, high_id;
	ATH5K_TRACE(ah->ah_sc);

	
	memcpy(ah->ah_bssid_mask, mask, ETH_ALEN);
	if (ah->ah_version == AR5K_AR5212) {
		low_id = AR5K_LOW_ID(mask);
		high_id = AR5K_HIGH_ID(mask);

		ath5k_hw_reg_write(ah, low_id, AR5K_BSS_IDM0);
		ath5k_hw_reg_write(ah, high_id, AR5K_BSS_IDM1);

		return 0;
	}

	return -EIO;
}





void ath5k_hw_start_rx_pcu(struct ath5k_hw *ah)
{
	ATH5K_TRACE(ah->ah_sc);
	AR5K_REG_DISABLE_BITS(ah, AR5K_DIAG_SW, AR5K_DIAG_SW_DIS_RX);
}


void ath5k_hw_stop_rx_pcu(struct ath5k_hw *ah)
{
	ATH5K_TRACE(ah->ah_sc);
	AR5K_REG_ENABLE_BITS(ah, AR5K_DIAG_SW, AR5K_DIAG_SW_DIS_RX);
}


void ath5k_hw_set_mcast_filter(struct ath5k_hw *ah, u32 filter0, u32 filter1)
{
	ATH5K_TRACE(ah->ah_sc);
	
	ath5k_hw_reg_write(ah, filter0, AR5K_MCAST_FILTER0);
	ath5k_hw_reg_write(ah, filter1, AR5K_MCAST_FILTER1);
}


int ath5k_hw_set_mcast_filter_idx(struct ath5k_hw *ah, u32 index)
{

	ATH5K_TRACE(ah->ah_sc);
	if (index >= 64)
		return -EINVAL;
	else if (index >= 32)
		AR5K_REG_ENABLE_BITS(ah, AR5K_MCAST_FILTER1,
				(1 << (index - 32)));
	else
		AR5K_REG_ENABLE_BITS(ah, AR5K_MCAST_FILTER0, (1 << index));

	return 0;
}


int ath5k_hw_clear_mcast_filter_idx(struct ath5k_hw *ah, u32 index)
{

	ATH5K_TRACE(ah->ah_sc);
	if (index >= 64)
		return -EINVAL;
	else if (index >= 32)
		AR5K_REG_DISABLE_BITS(ah, AR5K_MCAST_FILTER1,
				(1 << (index - 32)));
	else
		AR5K_REG_DISABLE_BITS(ah, AR5K_MCAST_FILTER0, (1 << index));

	return 0;
}


u32 ath5k_hw_get_rx_filter(struct ath5k_hw *ah)
{
	u32 data, filter = 0;

	ATH5K_TRACE(ah->ah_sc);
	filter = ath5k_hw_reg_read(ah, AR5K_RX_FILTER);

	
	if (ah->ah_version == AR5K_AR5212) {
		data = ath5k_hw_reg_read(ah, AR5K_PHY_ERR_FIL);

		if (data & AR5K_PHY_ERR_FIL_RADAR)
			filter |= AR5K_RX_FILTER_RADARERR;
		if (data & (AR5K_PHY_ERR_FIL_OFDM | AR5K_PHY_ERR_FIL_CCK))
			filter |= AR5K_RX_FILTER_PHYERR;
	}

	return filter;
}


void ath5k_hw_set_rx_filter(struct ath5k_hw *ah, u32 filter)
{
	u32 data = 0;

	ATH5K_TRACE(ah->ah_sc);

	
	if (ah->ah_version == AR5K_AR5212) {
		if (filter & AR5K_RX_FILTER_RADARERR)
			data |= AR5K_PHY_ERR_FIL_RADAR;
		if (filter & AR5K_RX_FILTER_PHYERR)
			data |= AR5K_PHY_ERR_FIL_OFDM | AR5K_PHY_ERR_FIL_CCK;
	}

	
	if (ah->ah_version == AR5K_AR5210 &&
			(filter & AR5K_RX_FILTER_RADARERR)) {
		filter &= ~AR5K_RX_FILTER_RADARERR;
		filter |= AR5K_RX_FILTER_PROM;
	}

	
	if (data)
		AR5K_REG_ENABLE_BITS(ah, AR5K_RXCFG, AR5K_RXCFG_ZLFDMA);
	else
		AR5K_REG_DISABLE_BITS(ah, AR5K_RXCFG, AR5K_RXCFG_ZLFDMA);

	
	ath5k_hw_reg_write(ah, filter & 0xff, AR5K_RX_FILTER);

	
	if (ah->ah_version == AR5K_AR5212)
		ath5k_hw_reg_write(ah, data, AR5K_PHY_ERR_FIL);

}





u32 ath5k_hw_get_tsf32(struct ath5k_hw *ah)
{
	ATH5K_TRACE(ah->ah_sc);
	return ath5k_hw_reg_read(ah, AR5K_TSF_L32);
}


u64 ath5k_hw_get_tsf64(struct ath5k_hw *ah)
{
	u64 tsf = ath5k_hw_reg_read(ah, AR5K_TSF_U32);
	ATH5K_TRACE(ah->ah_sc);

	return ath5k_hw_reg_read(ah, AR5K_TSF_L32) | (tsf << 32);
}


void ath5k_hw_set_tsf64(struct ath5k_hw *ah, u64 tsf64)
{
	ATH5K_TRACE(ah->ah_sc);

	ath5k_hw_reg_write(ah, tsf64 & 0xffffffff, AR5K_TSF_L32);
	ath5k_hw_reg_write(ah, (tsf64 >> 32) & 0xffffffff, AR5K_TSF_U32);
}


void ath5k_hw_reset_tsf(struct ath5k_hw *ah)
{
	u32 val;

	ATH5K_TRACE(ah->ah_sc);

	val = ath5k_hw_reg_read(ah, AR5K_BEACON) | AR5K_BEACON_RESET_TSF;

	
	ath5k_hw_reg_write(ah, val, AR5K_BEACON);
	ath5k_hw_reg_write(ah, val, AR5K_BEACON);
}


void ath5k_hw_init_beacon(struct ath5k_hw *ah, u32 next_beacon, u32 interval)
{
	u32 timer1, timer2, timer3;

	ATH5K_TRACE(ah->ah_sc);
	
	switch (ah->ah_op_mode) {
	case NL80211_IFTYPE_MONITOR:
	case NL80211_IFTYPE_STATION:
		
		
		if (ah->ah_version == AR5K_AR5210) {
			timer1 = 0xffffffff;
			timer2 = 0xffffffff;
		} else {
			timer1 = 0x0000ffff;
			timer2 = 0x0007ffff;
		}
		
		AR5K_REG_DISABLE_BITS(ah, AR5K_STA_ID1, AR5K_STA_ID1_PCF);
		break;
	case NL80211_IFTYPE_ADHOC:
		AR5K_REG_ENABLE_BITS(ah, AR5K_TXCFG, AR5K_TXCFG_ADHOC_BCN_ATIM);
	default:
		
		timer1 = (next_beacon - AR5K_TUNE_DMA_BEACON_RESP) << 3;
		timer2 = (next_beacon - AR5K_TUNE_SW_BEACON_RESP) << 3;
		break;
	}

	
	timer3 = next_beacon + (ah->ah_atim_window ? ah->ah_atim_window : 1);

	
	
	if (ah->ah_op_mode == NL80211_IFTYPE_AP ||
	    ah->ah_op_mode == NL80211_IFTYPE_MESH_POINT)
		ath5k_hw_reg_write(ah, 0, AR5K_TIMER0);

	ath5k_hw_reg_write(ah, next_beacon, AR5K_TIMER0);
	ath5k_hw_reg_write(ah, timer1, AR5K_TIMER1);
	ath5k_hw_reg_write(ah, timer2, AR5K_TIMER2);
	ath5k_hw_reg_write(ah, timer3, AR5K_TIMER3);

	
	if (interval & AR5K_BEACON_RESET_TSF)
		ath5k_hw_reset_tsf(ah);

	ath5k_hw_reg_write(ah, interval & (AR5K_BEACON_PERIOD |
					AR5K_BEACON_ENABLE),
						AR5K_BEACON);

	
	if (ah->ah_version == AR5K_AR5210)
		ath5k_hw_reg_write(ah, AR5K_ISR_BMISS, AR5K_ISR);
	else
		ath5k_hw_reg_write(ah, AR5K_ISR_BMISS, AR5K_PISR);

	
	AR5K_REG_DISABLE_BITS(ah, AR5K_STA_ID1, AR5K_STA_ID1_PWR_SV);

}

#if 0

int ath5k_hw_set_beacon_timers(struct ath5k_hw *ah,
		const struct ath5k_beacon_state *state)
{
	u32 cfp_period, next_cfp, dtim, interval, next_beacon;

	
	u32 dtim_count = 0; 
	u32 cfp_count = 0; 
	u32 tsf = 0; 

	ATH5K_TRACE(ah->ah_sc);
	
	if (state->bs_interval < 1)
		return -EINVAL;

	interval = state->bs_interval;
	dtim = state->bs_dtim_period;

	
	if (state->bs_cfp_period > 0) {
		
		cfp_period = state->bs_cfp_period * state->bs_dtim_period *
			state->bs_interval;
		next_cfp = (cfp_count * state->bs_dtim_period + dtim_count) *
			state->bs_interval;

		AR5K_REG_ENABLE_BITS(ah, AR5K_STA_ID1,
				AR5K_STA_ID1_DEFAULT_ANTENNA |
				AR5K_STA_ID1_PCF);
		ath5k_hw_reg_write(ah, cfp_period, AR5K_CFP_PERIOD);
		ath5k_hw_reg_write(ah, state->bs_cfp_max_duration,
				AR5K_CFP_DUR);
		ath5k_hw_reg_write(ah, (tsf + (next_cfp == 0 ? cfp_period :
						next_cfp)) << 3, AR5K_TIMER2);
	} else {
		
		AR5K_REG_DISABLE_BITS(ah, AR5K_STA_ID1,
				AR5K_STA_ID1_DEFAULT_ANTENNA |
				AR5K_STA_ID1_PCF);
	}

	
	ath5k_hw_reg_write(ah, state->bs_next_beacon, AR5K_TIMER0);

	
	ath5k_hw_reg_write(ah, (ath5k_hw_reg_read(ah, AR5K_BEACON) &
		~(AR5K_BEACON_PERIOD | AR5K_BEACON_TIM)) |
		AR5K_REG_SM(state->bs_tim_offset ? state->bs_tim_offset + 4 : 0,
		AR5K_BEACON_TIM) | AR5K_REG_SM(state->bs_interval,
		AR5K_BEACON_PERIOD), AR5K_BEACON);

	

	AR5K_REG_WRITE_BITS(ah, AR5K_RSSI_THR, AR5K_RSSI_THR_BMISS,
			state->bs_bmiss_threshold);

	
	AR5K_REG_WRITE_BITS(ah, AR5K_SLEEP_CTL, AR5K_SLEEP_CTL_SLDUR,
			(state->bs_sleep_duration - 3) << 3);

	
	if (ah->ah_version == AR5K_AR5212) {
		if (state->bs_sleep_duration > state->bs_interval &&
				roundup(state->bs_sleep_duration, interval) ==
				state->bs_sleep_duration)
			interval = state->bs_sleep_duration;

		if (state->bs_sleep_duration > dtim && (dtim == 0 ||
				roundup(state->bs_sleep_duration, dtim) ==
				state->bs_sleep_duration))
			dtim = state->bs_sleep_duration;

		if (interval > dtim)
			return -EINVAL;

		next_beacon = interval == dtim ? state->bs_next_dtim :
			state->bs_next_beacon;

		ath5k_hw_reg_write(ah,
			AR5K_REG_SM((state->bs_next_dtim - 3) << 3,
			AR5K_SLEEP0_NEXT_DTIM) |
			AR5K_REG_SM(10, AR5K_SLEEP0_CABTO) |
			AR5K_SLEEP0_ENH_SLEEP_EN |
			AR5K_SLEEP0_ASSUME_DTIM, AR5K_SLEEP0);

		ath5k_hw_reg_write(ah, AR5K_REG_SM((next_beacon - 3) << 3,
			AR5K_SLEEP1_NEXT_TIM) |
			AR5K_REG_SM(10, AR5K_SLEEP1_BEACON_TO), AR5K_SLEEP1);

		ath5k_hw_reg_write(ah,
			AR5K_REG_SM(interval, AR5K_SLEEP2_TIM_PER) |
			AR5K_REG_SM(dtim, AR5K_SLEEP2_DTIM_PER), AR5K_SLEEP2);
	}

	return 0;
}


void ath5k_hw_reset_beacon(struct ath5k_hw *ah)
{
	ATH5K_TRACE(ah->ah_sc);
	
	ath5k_hw_reg_write(ah, 0, AR5K_TIMER0);

	
	AR5K_REG_DISABLE_BITS(ah, AR5K_STA_ID1,
			AR5K_STA_ID1_DEFAULT_ANTENNA | AR5K_STA_ID1_PCF);
	ath5k_hw_reg_write(ah, AR5K_BEACON_PERIOD, AR5K_BEACON);
}


int ath5k_hw_beaconq_finish(struct ath5k_hw *ah, unsigned long phys_addr)
{
	unsigned int i;
	int ret;

	ATH5K_TRACE(ah->ah_sc);

	
	if (ah->ah_version == AR5K_AR5210) {
		
		for (i = AR5K_TUNE_BEACON_INTERVAL / 2; i > 0; i--) {
			if (!(ath5k_hw_reg_read(ah, AR5K_BSR) & AR5K_BSR_TXQ1F)
					||
			    !(ath5k_hw_reg_read(ah, AR5K_CR) & AR5K_BSR_TXQ1F))
				break;
			udelay(10);
		}

		
		if (i <= 0) {
			
			ath5k_hw_reg_write(ah, phys_addr, AR5K_NOQCU_TXDP1);
			ath5k_hw_reg_write(ah, AR5K_BCR_TQ1V | AR5K_BCR_BDMAE,
					AR5K_BCR);

			return -EIO;
		}
		ret = 0;
	} else {
	
		ret = ath5k_hw_register_timeout(ah,
			AR5K_QUEUE_STATUS(AR5K_TX_QUEUE_ID_BEACON),
			AR5K_QCU_STS_FRMPENDCNT, 0, false);

		if (AR5K_REG_READ_Q(ah, AR5K_QCU_TXE, AR5K_TX_QUEUE_ID_BEACON))
			return -EIO;
	}

	return ret;
}
#endif





int ath5k_hw_reset_key(struct ath5k_hw *ah, u16 entry)
{
	unsigned int i, type;
	u16 micentry = entry + AR5K_KEYTABLE_MIC_OFFSET;

	ATH5K_TRACE(ah->ah_sc);
	AR5K_ASSERT_ENTRY(entry, AR5K_KEYTABLE_SIZE);

	type = ath5k_hw_reg_read(ah, AR5K_KEYTABLE_TYPE(entry));

	for (i = 0; i < AR5K_KEYCACHE_SIZE; i++)
		ath5k_hw_reg_write(ah, 0, AR5K_KEYTABLE_OFF(entry, i));

	
	if (type == AR5K_KEYTABLE_TYPE_TKIP) {
		AR5K_ASSERT_ENTRY(micentry, AR5K_KEYTABLE_SIZE);
		for (i = 0; i < AR5K_KEYCACHE_SIZE / 2 ; i++)
			ath5k_hw_reg_write(ah, 0,
				AR5K_KEYTABLE_OFF(micentry, i));
	}

	
	if (ah->ah_version >= AR5K_AR5211) {
		ath5k_hw_reg_write(ah, AR5K_KEYTABLE_TYPE_NULL,
				AR5K_KEYTABLE_TYPE(entry));

		if (type == AR5K_KEYTABLE_TYPE_TKIP) {
			ath5k_hw_reg_write(ah, AR5K_KEYTABLE_TYPE_NULL,
				AR5K_KEYTABLE_TYPE(micentry));
		}
	}

	return 0;
}


int ath5k_hw_is_key_valid(struct ath5k_hw *ah, u16 entry)
{
	ATH5K_TRACE(ah->ah_sc);
	AR5K_ASSERT_ENTRY(entry, AR5K_KEYTABLE_SIZE);

	
	return ath5k_hw_reg_read(ah, AR5K_KEYTABLE_MAC1(entry)) &
		AR5K_KEYTABLE_VALID;
}

static
int ath5k_keycache_type(const struct ieee80211_key_conf *key)
{
	switch (key->alg) {
	case ALG_TKIP:
		return AR5K_KEYTABLE_TYPE_TKIP;
	case ALG_CCMP:
		return AR5K_KEYTABLE_TYPE_CCM;
	case ALG_WEP:
		if (key->keylen == WLAN_KEY_LEN_WEP40)
			return AR5K_KEYTABLE_TYPE_40;
		else if (key->keylen == WLAN_KEY_LEN_WEP104)
			return AR5K_KEYTABLE_TYPE_104;
		return -EINVAL;
	default:
		return -EINVAL;
	}
	return -EINVAL;
}


int ath5k_hw_set_key(struct ath5k_hw *ah, u16 entry,
		const struct ieee80211_key_conf *key, const u8 *mac)
{
	unsigned int i;
	int keylen;
	__le32 key_v[5] = {};
	__le32 key0 = 0, key1 = 0;
	__le32 *rxmic, *txmic;
	int keytype;
	u16 micentry = entry + AR5K_KEYTABLE_MIC_OFFSET;
	bool is_tkip;
	const u8 *key_ptr;

	ATH5K_TRACE(ah->ah_sc);

	is_tkip = (key->alg == ALG_TKIP);

	
	keylen = (is_tkip) ? (128 / 8) : key->keylen;

	if (entry > AR5K_KEYTABLE_SIZE ||
		(is_tkip && micentry > AR5K_KEYTABLE_SIZE))
		return -EOPNOTSUPP;

	if (unlikely(keylen > 16))
		return -EOPNOTSUPP;

	keytype = ath5k_keycache_type(key);
	if (keytype < 0)
		return keytype;

	
	key_ptr = key->key;
	for (i = 0; keylen >= 6; keylen -= 6) {
		memcpy(&key_v[i], key_ptr, 6);
		i += 2;
		key_ptr += 6;
	}
	if (keylen)
		memcpy(&key_v[i], key_ptr, keylen);

	
	if (is_tkip) {
		key0 = key_v[0] = ~key_v[0];
		key1 = key_v[1] = ~key_v[1];
	}

	for (i = 0; i < ARRAY_SIZE(key_v); i++)
		ath5k_hw_reg_write(ah, le32_to_cpu(key_v[i]),
				AR5K_KEYTABLE_OFF(entry, i));

	ath5k_hw_reg_write(ah, keytype, AR5K_KEYTABLE_TYPE(entry));

	if (is_tkip) {
		
		rxmic = (__le32 *) &key->key[16];
		txmic = (__le32 *) &key->key[24];

		if (ah->ah_combined_mic) {
			key_v[0] = rxmic[0];
			key_v[1] = cpu_to_le32(le32_to_cpu(txmic[0]) >> 16);
			key_v[2] = rxmic[1];
			key_v[3] = cpu_to_le32(le32_to_cpu(txmic[0]) & 0xffff);
			key_v[4] = txmic[1];
		} else {
			key_v[0] = rxmic[0];
			key_v[1] = 0;
			key_v[2] = rxmic[1];
			key_v[3] = 0;
			key_v[4] = 0;
		}
		for (i = 0; i < ARRAY_SIZE(key_v); i++)
			ath5k_hw_reg_write(ah, le32_to_cpu(key_v[i]),
				AR5K_KEYTABLE_OFF(micentry, i));

		ath5k_hw_reg_write(ah, AR5K_KEYTABLE_TYPE_NULL,
			AR5K_KEYTABLE_TYPE(micentry));
		ath5k_hw_reg_write(ah, 0, AR5K_KEYTABLE_MAC0(micentry));
		ath5k_hw_reg_write(ah, 0, AR5K_KEYTABLE_MAC1(micentry));

		
		ath5k_hw_reg_write(ah, le32_to_cpu(~key0),
			AR5K_KEYTABLE_OFF(entry, 0));
		ath5k_hw_reg_write(ah, le32_to_cpu(~key1),
			AR5K_KEYTABLE_OFF(entry, 1));
	}

	return ath5k_hw_set_key_lladdr(ah, entry, mac);
}

int ath5k_hw_set_key_lladdr(struct ath5k_hw *ah, u16 entry, const u8 *mac)
{
	u32 low_id, high_id;

	ATH5K_TRACE(ah->ah_sc);
	 
	AR5K_ASSERT_ENTRY(entry, AR5K_KEYTABLE_SIZE);

	
	if (!mac) {
		low_id = 0xffffffff;
		high_id = 0xffff | AR5K_KEYTABLE_VALID;
	} else {
		low_id = AR5K_LOW_ID(mac);
		high_id = AR5K_HIGH_ID(mac) | AR5K_KEYTABLE_VALID;
	}

	ath5k_hw_reg_write(ah, low_id, AR5K_KEYTABLE_MAC0(entry));
	ath5k_hw_reg_write(ah, high_id, AR5K_KEYTABLE_MAC1(entry));

	return 0;
}

