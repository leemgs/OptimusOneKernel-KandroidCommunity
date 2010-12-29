



#include "ath5k.h"
#include "reg.h"
#include "debug.h"
#include "base.h"


int ath5k_hw_get_tx_queueprops(struct ath5k_hw *ah, int queue,
		struct ath5k_txq_info *queue_info)
{
	ATH5K_TRACE(ah->ah_sc);
	memcpy(queue_info, &ah->ah_txq[queue], sizeof(struct ath5k_txq_info));
	return 0;
}


int ath5k_hw_set_tx_queueprops(struct ath5k_hw *ah, int queue,
				const struct ath5k_txq_info *queue_info)
{
	ATH5K_TRACE(ah->ah_sc);
	AR5K_ASSERT_ENTRY(queue, ah->ah_capabilities.cap_queues.q_tx_num);

	if (ah->ah_txq[queue].tqi_type == AR5K_TX_QUEUE_INACTIVE)
		return -EIO;

	memcpy(&ah->ah_txq[queue], queue_info, sizeof(struct ath5k_txq_info));

	
	if ((queue_info->tqi_type == AR5K_TX_QUEUE_DATA &&
			((queue_info->tqi_subtype == AR5K_WME_AC_VI) ||
			(queue_info->tqi_subtype == AR5K_WME_AC_VO))) ||
			queue_info->tqi_type == AR5K_TX_QUEUE_UAPSD)
		ah->ah_txq[queue].tqi_flags |= AR5K_TXQ_FLAG_POST_FR_BKOFF_DIS;

	return 0;
}


int ath5k_hw_setup_tx_queue(struct ath5k_hw *ah, enum ath5k_tx_queue queue_type,
		struct ath5k_txq_info *queue_info)
{
	unsigned int queue;
	int ret;

	ATH5K_TRACE(ah->ah_sc);

	
	
	if (ah->ah_version == AR5K_AR5210) {
		switch (queue_type) {
		case AR5K_TX_QUEUE_DATA:
			queue = AR5K_TX_QUEUE_ID_NOQCU_DATA;
			break;
		case AR5K_TX_QUEUE_BEACON:
		case AR5K_TX_QUEUE_CAB:
			queue = AR5K_TX_QUEUE_ID_NOQCU_BEACON;
			break;
		default:
			return -EINVAL;
		}
	} else {
		switch (queue_type) {
		case AR5K_TX_QUEUE_DATA:
			for (queue = AR5K_TX_QUEUE_ID_DATA_MIN;
				ah->ah_txq[queue].tqi_type !=
				AR5K_TX_QUEUE_INACTIVE; queue++) {

				if (queue > AR5K_TX_QUEUE_ID_DATA_MAX)
					return -EINVAL;
			}
			break;
		case AR5K_TX_QUEUE_UAPSD:
			queue = AR5K_TX_QUEUE_ID_UAPSD;
			break;
		case AR5K_TX_QUEUE_BEACON:
			queue = AR5K_TX_QUEUE_ID_BEACON;
			break;
		case AR5K_TX_QUEUE_CAB:
			queue = AR5K_TX_QUEUE_ID_CAB;
			break;
		case AR5K_TX_QUEUE_XR_DATA:
			if (ah->ah_version != AR5K_AR5212)
				ATH5K_ERR(ah->ah_sc,
					"XR data queues only supported in"
					" 5212!\n");
			queue = AR5K_TX_QUEUE_ID_XR_DATA;
			break;
		default:
			return -EINVAL;
		}
	}

	
	memset(&ah->ah_txq[queue], 0, sizeof(struct ath5k_txq_info));
	ah->ah_txq[queue].tqi_type = queue_type;

	if (queue_info != NULL) {
		queue_info->tqi_type = queue_type;
		ret = ath5k_hw_set_tx_queueprops(ah, queue, queue_info);
		if (ret)
			return ret;
	}

	
	AR5K_Q_ENABLE_BITS(ah->ah_txq_status, queue);

	return queue;
}


u32 ath5k_hw_num_tx_pending(struct ath5k_hw *ah, unsigned int queue)
{
	u32 pending;
	ATH5K_TRACE(ah->ah_sc);
	AR5K_ASSERT_ENTRY(queue, ah->ah_capabilities.cap_queues.q_tx_num);

	
	if (ah->ah_txq[queue].tqi_type == AR5K_TX_QUEUE_INACTIVE)
		return false;

	
	if (ah->ah_version == AR5K_AR5210)
		return false;

	pending = ath5k_hw_reg_read(ah, AR5K_QUEUE_STATUS(queue));
	pending &= AR5K_QCU_STS_FRMPENDCNT;

	
	if (!pending && AR5K_REG_READ_Q(ah, AR5K_QCU_TXE, queue))
		return true;

	return pending;
}


void ath5k_hw_release_tx_queue(struct ath5k_hw *ah, unsigned int queue)
{
	ATH5K_TRACE(ah->ah_sc);
	if (WARN_ON(queue >= ah->ah_capabilities.cap_queues.q_tx_num))
		return;

	
	ah->ah_txq[queue].tqi_type = AR5K_TX_QUEUE_INACTIVE;
	
	AR5K_Q_DISABLE_BITS(ah->ah_txq_status, queue);
}


int ath5k_hw_reset_tx_queue(struct ath5k_hw *ah, unsigned int queue)
{
	u32 cw_min, cw_max, retry_lg, retry_sh;
	struct ath5k_txq_info *tq = &ah->ah_txq[queue];

	ATH5K_TRACE(ah->ah_sc);
	AR5K_ASSERT_ENTRY(queue, ah->ah_capabilities.cap_queues.q_tx_num);

	tq = &ah->ah_txq[queue];

	if (tq->tqi_type == AR5K_TX_QUEUE_INACTIVE)
		return 0;

	if (ah->ah_version == AR5K_AR5210) {
		
		if (tq->tqi_type != AR5K_TX_QUEUE_DATA)
			return 0;

		
		ath5k_hw_reg_write(ah, ah->ah_turbo ?
			AR5K_INIT_SLOT_TIME_TURBO : AR5K_INIT_SLOT_TIME,
			AR5K_SLOT_TIME);
		
		ath5k_hw_reg_write(ah, ah->ah_turbo ?
			AR5K_INIT_ACK_CTS_TIMEOUT_TURBO :
			AR5K_INIT_ACK_CTS_TIMEOUT, AR5K_SLOT_TIME);
		
		ath5k_hw_reg_write(ah, ah->ah_turbo ?
			AR5K_INIT_TRANSMIT_LATENCY_TURBO :
			AR5K_INIT_TRANSMIT_LATENCY, AR5K_USEC_5210);

		
		if (ah->ah_turbo) {
			 ath5k_hw_reg_write(ah, ((AR5K_INIT_SIFS_TURBO +
				(ah->ah_aifs + tq->tqi_aifs) *
				AR5K_INIT_SLOT_TIME_TURBO) <<
				AR5K_IFS0_DIFS_S) | AR5K_INIT_SIFS_TURBO,
				AR5K_IFS0);
		} else {
			ath5k_hw_reg_write(ah, ((AR5K_INIT_SIFS +
				(ah->ah_aifs + tq->tqi_aifs) *
				AR5K_INIT_SLOT_TIME) << AR5K_IFS0_DIFS_S) |
				AR5K_INIT_SIFS, AR5K_IFS0);
		}

		
		ath5k_hw_reg_write(ah, ah->ah_turbo ?
			AR5K_INIT_PROTO_TIME_CNTRL_TURBO :
			AR5K_INIT_PROTO_TIME_CNTRL, AR5K_IFS1);
		
		ath5k_hw_reg_write(ah, ah->ah_turbo ?
			(ath5k_hw_reg_read(ah, AR5K_PHY_SETTLING) & ~0x7F)
			| 0x38 :
			(ath5k_hw_reg_read(ah, AR5K_PHY_SETTLING) & ~0x7F)
			| 0x1C,
			AR5K_PHY_SETTLING);
		
		ath5k_hw_reg_write(ah, ah->ah_turbo ?
			(AR5K_PHY_FRAME_CTL_INI | AR5K_PHY_TURBO_MODE |
			AR5K_PHY_TURBO_SHORT | 0x2020) :
			(AR5K_PHY_FRAME_CTL_INI | 0x1020),
			AR5K_PHY_FRAME_CTL_5210);
	}

	
	cw_min = ah->ah_cw_min = AR5K_TUNE_CWMIN;
	cw_max = ah->ah_cw_max = AR5K_TUNE_CWMAX;
	ah->ah_aifs = AR5K_TUNE_AIFS;
	
	if (IS_CHAN_XR(ah->ah_current_channel) &&
			ah->ah_version == AR5K_AR5212) {
		cw_min = ah->ah_cw_min = AR5K_TUNE_CWMIN_XR;
		cw_max = ah->ah_cw_max = AR5K_TUNE_CWMAX_XR;
		ah->ah_aifs = AR5K_TUNE_AIFS_XR;
	
	} else if (IS_CHAN_B(ah->ah_current_channel) &&
			ah->ah_version != AR5K_AR5210) {
		cw_min = ah->ah_cw_min = AR5K_TUNE_CWMIN_11B;
		cw_max = ah->ah_cw_max = AR5K_TUNE_CWMAX_11B;
		ah->ah_aifs = AR5K_TUNE_AIFS_11B;
	}

	cw_min = 1;
	while (cw_min < ah->ah_cw_min)
		cw_min = (cw_min << 1) | 1;

	cw_min = tq->tqi_cw_min < 0 ? (cw_min >> (-tq->tqi_cw_min)) :
		((cw_min << tq->tqi_cw_min) + (1 << tq->tqi_cw_min) - 1);
	cw_max = tq->tqi_cw_max < 0 ? (cw_max >> (-tq->tqi_cw_max)) :
		((cw_max << tq->tqi_cw_max) + (1 << tq->tqi_cw_max) - 1);

	
	if (ah->ah_software_retry) {
		
		retry_lg = ah->ah_limit_tx_retries;
		retry_sh = retry_lg = retry_lg > AR5K_DCU_RETRY_LMT_SH_RETRY ?
			AR5K_DCU_RETRY_LMT_SH_RETRY : retry_lg;
	} else {
		retry_lg = AR5K_INIT_LG_RETRY;
		retry_sh = AR5K_INIT_SH_RETRY;
	}

	
	if (ah->ah_version == AR5K_AR5210) {
		ath5k_hw_reg_write(ah,
			(cw_min << AR5K_NODCU_RETRY_LMT_CW_MIN_S)
			| AR5K_REG_SM(AR5K_INIT_SLG_RETRY,
				AR5K_NODCU_RETRY_LMT_SLG_RETRY)
			| AR5K_REG_SM(AR5K_INIT_SSH_RETRY,
				AR5K_NODCU_RETRY_LMT_SSH_RETRY)
			| AR5K_REG_SM(retry_lg, AR5K_NODCU_RETRY_LMT_LG_RETRY)
			| AR5K_REG_SM(retry_sh, AR5K_NODCU_RETRY_LMT_SH_RETRY),
			AR5K_NODCU_RETRY_LMT);
	} else {
		
		ath5k_hw_reg_write(ah,
			AR5K_REG_SM(AR5K_INIT_SLG_RETRY,
				AR5K_DCU_RETRY_LMT_SLG_RETRY) |
			AR5K_REG_SM(AR5K_INIT_SSH_RETRY,
				AR5K_DCU_RETRY_LMT_SSH_RETRY) |
			AR5K_REG_SM(retry_lg, AR5K_DCU_RETRY_LMT_LG_RETRY) |
			AR5K_REG_SM(retry_sh, AR5K_DCU_RETRY_LMT_SH_RETRY),
			AR5K_QUEUE_DFS_RETRY_LIMIT(queue));

	

		
		ath5k_hw_reg_write(ah,
			AR5K_REG_SM(cw_min, AR5K_DCU_LCL_IFS_CW_MIN) |
			AR5K_REG_SM(cw_max, AR5K_DCU_LCL_IFS_CW_MAX) |
			AR5K_REG_SM(ah->ah_aifs + tq->tqi_aifs,
				AR5K_DCU_LCL_IFS_AIFS),
			AR5K_QUEUE_DFS_LOCAL_IFS(queue));

		
		
		AR5K_REG_ENABLE_BITS(ah, AR5K_QUEUE_MISC(queue),
					AR5K_QCU_MISC_DCU_EARLY);

		
		AR5K_REG_ENABLE_BITS(ah, AR5K_QUEUE_DFS_MISC(queue),
					AR5K_DCU_MISC_FRAG_WAIT);

		
		if (ah->ah_mac_version < AR5K_SREV_AR5211)
			AR5K_REG_ENABLE_BITS(ah, AR5K_QUEUE_DFS_MISC(queue),
						AR5K_DCU_MISC_SEQNUM_CTL);

		if (tq->tqi_cbr_period) {
			ath5k_hw_reg_write(ah, AR5K_REG_SM(tq->tqi_cbr_period,
				AR5K_QCU_CBRCFG_INTVAL) |
				AR5K_REG_SM(tq->tqi_cbr_overflow_limit,
				AR5K_QCU_CBRCFG_ORN_THRES),
				AR5K_QUEUE_CBRCFG(queue));
			AR5K_REG_ENABLE_BITS(ah, AR5K_QUEUE_MISC(queue),
				AR5K_QCU_MISC_FRSHED_CBR);
			if (tq->tqi_cbr_overflow_limit)
				AR5K_REG_ENABLE_BITS(ah,
					AR5K_QUEUE_MISC(queue),
					AR5K_QCU_MISC_CBR_THRES_ENABLE);
		}

		if (tq->tqi_ready_time &&
		(tq->tqi_type != AR5K_TX_QUEUE_CAB))
			ath5k_hw_reg_write(ah, AR5K_REG_SM(tq->tqi_ready_time,
				AR5K_QCU_RDYTIMECFG_INTVAL) |
				AR5K_QCU_RDYTIMECFG_ENABLE,
				AR5K_QUEUE_RDYTIMECFG(queue));

		if (tq->tqi_burst_time) {
			ath5k_hw_reg_write(ah, AR5K_REG_SM(tq->tqi_burst_time,
				AR5K_DCU_CHAN_TIME_DUR) |
				AR5K_DCU_CHAN_TIME_ENABLE,
				AR5K_QUEUE_DFS_CHANNEL_TIME(queue));

			if (tq->tqi_flags
			& AR5K_TXQ_FLAG_RDYTIME_EXP_POLICY_ENABLE)
				AR5K_REG_ENABLE_BITS(ah,
					AR5K_QUEUE_MISC(queue),
					AR5K_QCU_MISC_RDY_VEOL_POLICY);
		}

		if (tq->tqi_flags & AR5K_TXQ_FLAG_BACKOFF_DISABLE)
			ath5k_hw_reg_write(ah, AR5K_DCU_MISC_POST_FR_BKOFF_DIS,
				AR5K_QUEUE_DFS_MISC(queue));

		if (tq->tqi_flags & AR5K_TXQ_FLAG_FRAG_BURST_BACKOFF_ENABLE)
			ath5k_hw_reg_write(ah, AR5K_DCU_MISC_BACKOFF_FRAG,
				AR5K_QUEUE_DFS_MISC(queue));

		
		switch (tq->tqi_type) {
		case AR5K_TX_QUEUE_BEACON:
			AR5K_REG_ENABLE_BITS(ah, AR5K_QUEUE_MISC(queue),
				AR5K_QCU_MISC_FRSHED_DBA_GT |
				AR5K_QCU_MISC_CBREXP_BCN_DIS |
				AR5K_QCU_MISC_BCN_ENABLE);

			AR5K_REG_ENABLE_BITS(ah, AR5K_QUEUE_DFS_MISC(queue),
				(AR5K_DCU_MISC_ARBLOCK_CTL_GLOBAL <<
				AR5K_DCU_MISC_ARBLOCK_CTL_S) |
				AR5K_DCU_MISC_ARBLOCK_IGNORE |
				AR5K_DCU_MISC_POST_FR_BKOFF_DIS |
				AR5K_DCU_MISC_BCN_ENABLE);
			break;

		case AR5K_TX_QUEUE_CAB:
			AR5K_REG_ENABLE_BITS(ah, AR5K_QUEUE_MISC(queue),
				AR5K_QCU_MISC_FRSHED_BCN_SENT_GT |
				AR5K_QCU_MISC_CBREXP_DIS |
				AR5K_QCU_MISC_CBREXP_BCN_DIS);

			ath5k_hw_reg_write(ah, ((AR5K_TUNE_BEACON_INTERVAL -
				(AR5K_TUNE_SW_BEACON_RESP -
				AR5K_TUNE_DMA_BEACON_RESP) -
				AR5K_TUNE_ADDITIONAL_SWBA_BACKOFF) * 1024) |
				AR5K_QCU_RDYTIMECFG_ENABLE,
				AR5K_QUEUE_RDYTIMECFG(queue));

			AR5K_REG_ENABLE_BITS(ah, AR5K_QUEUE_DFS_MISC(queue),
				(AR5K_DCU_MISC_ARBLOCK_CTL_GLOBAL <<
				AR5K_DCU_MISC_ARBLOCK_CTL_S));
			break;

		case AR5K_TX_QUEUE_UAPSD:
			AR5K_REG_ENABLE_BITS(ah, AR5K_QUEUE_MISC(queue),
				AR5K_QCU_MISC_CBREXP_DIS);
			break;

		case AR5K_TX_QUEUE_DATA:
		default:
			break;
		}

		

		
		if (tq->tqi_flags & AR5K_TXQ_FLAG_TXOKINT_ENABLE)
			AR5K_Q_ENABLE_BITS(ah->ah_txq_imr_txok, queue);

		if (tq->tqi_flags & AR5K_TXQ_FLAG_TXERRINT_ENABLE)
			AR5K_Q_ENABLE_BITS(ah->ah_txq_imr_txerr, queue);

		if (tq->tqi_flags & AR5K_TXQ_FLAG_TXURNINT_ENABLE)
			AR5K_Q_ENABLE_BITS(ah->ah_txq_imr_txurn, queue);

		if (tq->tqi_flags & AR5K_TXQ_FLAG_TXDESCINT_ENABLE)
			AR5K_Q_ENABLE_BITS(ah->ah_txq_imr_txdesc, queue);

		if (tq->tqi_flags & AR5K_TXQ_FLAG_TXEOLINT_ENABLE)
			AR5K_Q_ENABLE_BITS(ah->ah_txq_imr_txeol, queue);

		if (tq->tqi_flags & AR5K_TXQ_FLAG_CBRORNINT_ENABLE)
			AR5K_Q_ENABLE_BITS(ah->ah_txq_imr_cbrorn, queue);

		if (tq->tqi_flags & AR5K_TXQ_FLAG_CBRURNINT_ENABLE)
			AR5K_Q_ENABLE_BITS(ah->ah_txq_imr_cbrurn, queue);

		if (tq->tqi_flags & AR5K_TXQ_FLAG_QTRIGINT_ENABLE)
			AR5K_Q_ENABLE_BITS(ah->ah_txq_imr_qtrig, queue);

		if (tq->tqi_flags & AR5K_TXQ_FLAG_TXNOFRMINT_ENABLE)
			AR5K_Q_ENABLE_BITS(ah->ah_txq_imr_nofrm, queue);

		

		
		ah->ah_txq_imr_txok &= ah->ah_txq_status;
		ah->ah_txq_imr_txerr &= ah->ah_txq_status;
		ah->ah_txq_imr_txurn &= ah->ah_txq_status;
		ah->ah_txq_imr_txdesc &= ah->ah_txq_status;
		ah->ah_txq_imr_txeol &= ah->ah_txq_status;
		ah->ah_txq_imr_cbrorn &= ah->ah_txq_status;
		ah->ah_txq_imr_cbrurn &= ah->ah_txq_status;
		ah->ah_txq_imr_qtrig &= ah->ah_txq_status;
		ah->ah_txq_imr_nofrm &= ah->ah_txq_status;

		ath5k_hw_reg_write(ah, AR5K_REG_SM(ah->ah_txq_imr_txok,
			AR5K_SIMR0_QCU_TXOK) |
			AR5K_REG_SM(ah->ah_txq_imr_txdesc,
			AR5K_SIMR0_QCU_TXDESC), AR5K_SIMR0);
		ath5k_hw_reg_write(ah, AR5K_REG_SM(ah->ah_txq_imr_txerr,
			AR5K_SIMR1_QCU_TXERR) |
			AR5K_REG_SM(ah->ah_txq_imr_txeol,
			AR5K_SIMR1_QCU_TXEOL), AR5K_SIMR1);
		
		AR5K_REG_DISABLE_BITS(ah, AR5K_SIMR2, AR5K_SIMR2_QCU_TXURN);
		AR5K_REG_ENABLE_BITS(ah, AR5K_SIMR2,
			AR5K_REG_SM(ah->ah_txq_imr_txurn,
			AR5K_SIMR2_QCU_TXURN));
		ath5k_hw_reg_write(ah, AR5K_REG_SM(ah->ah_txq_imr_cbrorn,
			AR5K_SIMR3_QCBRORN) |
			AR5K_REG_SM(ah->ah_txq_imr_cbrurn,
			AR5K_SIMR3_QCBRURN), AR5K_SIMR3);
		ath5k_hw_reg_write(ah, AR5K_REG_SM(ah->ah_txq_imr_qtrig,
			AR5K_SIMR4_QTRIG), AR5K_SIMR4);
		
		ath5k_hw_reg_write(ah, AR5K_REG_SM(ah->ah_txq_imr_nofrm,
			AR5K_TXNOFRM_QCU), AR5K_TXNOFRM);
		
		if (ah->ah_txq_imr_nofrm == 0)
			ath5k_hw_reg_write(ah, 0, AR5K_TXNOFRM);

		
		AR5K_REG_WRITE_Q(ah, AR5K_QUEUE_QCUMASK(queue), queue);
	}

	return 0;
}


unsigned int ath5k_hw_get_slot_time(struct ath5k_hw *ah)
{
	ATH5K_TRACE(ah->ah_sc);
	if (ah->ah_version == AR5K_AR5210)
		return ath5k_hw_clocktoh(ath5k_hw_reg_read(ah,
				AR5K_SLOT_TIME) & 0xffff, ah->ah_turbo);
	else
		return ath5k_hw_reg_read(ah, AR5K_DCU_GBL_IFS_SLOT) & 0xffff;
}


int ath5k_hw_set_slot_time(struct ath5k_hw *ah, unsigned int slot_time)
{
	ATH5K_TRACE(ah->ah_sc);
	if (slot_time < AR5K_SLOT_TIME_9 || slot_time > AR5K_SLOT_TIME_MAX)
		return -EINVAL;

	if (ah->ah_version == AR5K_AR5210)
		ath5k_hw_reg_write(ah, ath5k_hw_htoclock(slot_time,
				ah->ah_turbo), AR5K_SLOT_TIME);
	else
		ath5k_hw_reg_write(ah, slot_time, AR5K_DCU_GBL_IFS_SLOT);

	return 0;
}

