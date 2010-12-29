



#include "ath5k.h"
#include "reg.h"
#include "debug.h"
#include "base.h"




static int
ath5k_hw_setup_2word_tx_desc(struct ath5k_hw *ah, struct ath5k_desc *desc,
	unsigned int pkt_len, unsigned int hdr_len, enum ath5k_pkt_type type,
	unsigned int tx_power, unsigned int tx_rate0, unsigned int tx_tries0,
	unsigned int key_index, unsigned int antenna_mode, unsigned int flags,
	unsigned int rtscts_rate, unsigned int rtscts_duration)
{
	u32 frame_type;
	struct ath5k_hw_2w_tx_ctl *tx_ctl;
	unsigned int frame_len;

	tx_ctl = &desc->ud.ds_tx5210.tx_ctl;

	
	if (unlikely(tx_tries0 == 0)) {
		ATH5K_ERR(ah->ah_sc, "zero retries\n");
		WARN_ON(1);
		return -EINVAL;
	}
	if (unlikely(tx_rate0 == 0)) {
		ATH5K_ERR(ah->ah_sc, "zero rate\n");
		WARN_ON(1);
		return -EINVAL;
	}

	
	memset(&desc->ud.ds_tx5210, 0, sizeof(struct ath5k_hw_5210_tx_desc));

	

	

	
	frame_len = pkt_len - ath5k_pad_size(hdr_len) + FCS_LEN;

	if (frame_len & ~AR5K_2W_TX_DESC_CTL0_FRAME_LEN)
		return -EINVAL;

	tx_ctl->tx_control_0 = frame_len & AR5K_2W_TX_DESC_CTL0_FRAME_LEN;

	

	
	if (type == AR5K_PKT_TYPE_BEACON)
		pkt_len = roundup(pkt_len, 4);

	if (pkt_len & ~AR5K_2W_TX_DESC_CTL1_BUF_LEN)
		return -EINVAL;

	tx_ctl->tx_control_1 = pkt_len & AR5K_2W_TX_DESC_CTL1_BUF_LEN;

	
	if (ah->ah_version == AR5K_AR5210) {
		if (hdr_len & ~AR5K_2W_TX_DESC_CTL0_HEADER_LEN)
			return -EINVAL;
		tx_ctl->tx_control_0 |=
			AR5K_REG_SM(hdr_len, AR5K_2W_TX_DESC_CTL0_HEADER_LEN);
	}

	
	if (ah->ah_version == AR5K_AR5210) {
		switch (type) {
		case AR5K_PKT_TYPE_BEACON:
		case AR5K_PKT_TYPE_PROBE_RESP:
			frame_type = AR5K_AR5210_TX_DESC_FRAME_TYPE_NO_DELAY;
		case AR5K_PKT_TYPE_PIFS:
			frame_type = AR5K_AR5210_TX_DESC_FRAME_TYPE_PIFS;
		default:
			frame_type = type ;
		}

		tx_ctl->tx_control_0 |=
		AR5K_REG_SM(frame_type, AR5K_2W_TX_DESC_CTL0_FRAME_TYPE) |
		AR5K_REG_SM(tx_rate0, AR5K_2W_TX_DESC_CTL0_XMIT_RATE);

	} else {
		tx_ctl->tx_control_0 |=
			AR5K_REG_SM(tx_rate0, AR5K_2W_TX_DESC_CTL0_XMIT_RATE) |
			AR5K_REG_SM(antenna_mode,
				AR5K_2W_TX_DESC_CTL0_ANT_MODE_XMIT);
		tx_ctl->tx_control_1 |=
			AR5K_REG_SM(type, AR5K_2W_TX_DESC_CTL1_FRAME_TYPE);
	}
#define _TX_FLAGS(_c, _flag)					\
	if (flags & AR5K_TXDESC_##_flag) {			\
		tx_ctl->tx_control_##_c |=			\
			AR5K_2W_TX_DESC_CTL##_c##_##_flag;	\
	}

	_TX_FLAGS(0, CLRDMASK);
	_TX_FLAGS(0, VEOL);
	_TX_FLAGS(0, INTREQ);
	_TX_FLAGS(0, RTSENA);
	_TX_FLAGS(1, NOACK);

#undef _TX_FLAGS

	
	if (key_index != AR5K_TXKEYIX_INVALID) {
		tx_ctl->tx_control_0 |=
			AR5K_2W_TX_DESC_CTL0_ENCRYPT_KEY_VALID;
		tx_ctl->tx_control_1 |=
			AR5K_REG_SM(key_index,
			AR5K_2W_TX_DESC_CTL1_ENCRYPT_KEY_INDEX);
	}

	
	if ((ah->ah_version == AR5K_AR5210) &&
			(flags & (AR5K_TXDESC_RTSENA | AR5K_TXDESC_CTSENA)))
		tx_ctl->tx_control_1 |= rtscts_duration &
				AR5K_2W_TX_DESC_CTL1_RTS_DURATION;

	return 0;
}


static int ath5k_hw_setup_4word_tx_desc(struct ath5k_hw *ah,
	struct ath5k_desc *desc, unsigned int pkt_len, unsigned int hdr_len,
	enum ath5k_pkt_type type, unsigned int tx_power, unsigned int tx_rate0,
	unsigned int tx_tries0, unsigned int key_index,
	unsigned int antenna_mode, unsigned int flags,
	unsigned int rtscts_rate,
	unsigned int rtscts_duration)
{
	struct ath5k_hw_4w_tx_ctl *tx_ctl;
	unsigned int frame_len;

	ATH5K_TRACE(ah->ah_sc);
	tx_ctl = &desc->ud.ds_tx5212.tx_ctl;

	
	if (unlikely(tx_tries0 == 0)) {
		ATH5K_ERR(ah->ah_sc, "zero retries\n");
		WARN_ON(1);
		return -EINVAL;
	}
	if (unlikely(tx_rate0 == 0)) {
		ATH5K_ERR(ah->ah_sc, "zero rate\n");
		WARN_ON(1);
		return -EINVAL;
	}

	tx_power += ah->ah_txpower.txp_offset;
	if (tx_power > AR5K_TUNE_MAX_TXPOWER)
		tx_power = AR5K_TUNE_MAX_TXPOWER;

	
	memset(&desc->ud.ds_tx5212, 0, sizeof(struct ath5k_hw_5212_tx_desc));

	

	

	
	frame_len = pkt_len - ath5k_pad_size(hdr_len) + FCS_LEN;

	if (frame_len & ~AR5K_4W_TX_DESC_CTL0_FRAME_LEN)
		return -EINVAL;

	tx_ctl->tx_control_0 = frame_len & AR5K_4W_TX_DESC_CTL0_FRAME_LEN;

	

	
	if (type == AR5K_PKT_TYPE_BEACON)
		pkt_len = roundup(pkt_len, 4);

	if (pkt_len & ~AR5K_4W_TX_DESC_CTL1_BUF_LEN)
		return -EINVAL;

	tx_ctl->tx_control_1 = pkt_len & AR5K_4W_TX_DESC_CTL1_BUF_LEN;

	tx_ctl->tx_control_0 |=
		AR5K_REG_SM(tx_power, AR5K_4W_TX_DESC_CTL0_XMIT_POWER) |
		AR5K_REG_SM(antenna_mode, AR5K_4W_TX_DESC_CTL0_ANT_MODE_XMIT);
	tx_ctl->tx_control_1 |= AR5K_REG_SM(type,
					AR5K_4W_TX_DESC_CTL1_FRAME_TYPE);
	tx_ctl->tx_control_2 = AR5K_REG_SM(tx_tries0 + AR5K_TUNE_HWTXTRIES,
					AR5K_4W_TX_DESC_CTL2_XMIT_TRIES0);
	tx_ctl->tx_control_3 = tx_rate0 & AR5K_4W_TX_DESC_CTL3_XMIT_RATE0;

#define _TX_FLAGS(_c, _flag)					\
	if (flags & AR5K_TXDESC_##_flag) {			\
		tx_ctl->tx_control_##_c |=			\
			AR5K_4W_TX_DESC_CTL##_c##_##_flag;	\
	}

	_TX_FLAGS(0, CLRDMASK);
	_TX_FLAGS(0, VEOL);
	_TX_FLAGS(0, INTREQ);
	_TX_FLAGS(0, RTSENA);
	_TX_FLAGS(0, CTSENA);
	_TX_FLAGS(1, NOACK);

#undef _TX_FLAGS

	
	if (key_index != AR5K_TXKEYIX_INVALID) {
		tx_ctl->tx_control_0 |= AR5K_4W_TX_DESC_CTL0_ENCRYPT_KEY_VALID;
		tx_ctl->tx_control_1 |= AR5K_REG_SM(key_index,
				AR5K_4W_TX_DESC_CTL1_ENCRYPT_KEY_INDEX);
	}

	
	if (flags & (AR5K_TXDESC_RTSENA | AR5K_TXDESC_CTSENA)) {
		if ((flags & AR5K_TXDESC_RTSENA) &&
				(flags & AR5K_TXDESC_CTSENA))
			return -EINVAL;
		tx_ctl->tx_control_2 |= rtscts_duration &
				AR5K_4W_TX_DESC_CTL2_RTS_DURATION;
		tx_ctl->tx_control_3 |= AR5K_REG_SM(rtscts_rate,
				AR5K_4W_TX_DESC_CTL3_RTS_CTS_RATE);
	}

	return 0;
}


static int
ath5k_hw_setup_mrr_tx_desc(struct ath5k_hw *ah, struct ath5k_desc *desc,
	unsigned int tx_rate1, u_int tx_tries1, u_int tx_rate2,
	u_int tx_tries2, unsigned int tx_rate3, u_int tx_tries3)
{
	struct ath5k_hw_4w_tx_ctl *tx_ctl;

	
	if (unlikely((tx_rate1 == 0 && tx_tries1 != 0) ||
		     (tx_rate2 == 0 && tx_tries2 != 0) ||
		     (tx_rate3 == 0 && tx_tries3 != 0))) {
		ATH5K_ERR(ah->ah_sc, "zero rate\n");
		WARN_ON(1);
		return -EINVAL;
	}

	if (ah->ah_version == AR5K_AR5212) {
		tx_ctl = &desc->ud.ds_tx5212.tx_ctl;

#define _XTX_TRIES(_n)							\
	if (tx_tries##_n) {						\
		tx_ctl->tx_control_2 |=					\
		    AR5K_REG_SM(tx_tries##_n,				\
		    AR5K_4W_TX_DESC_CTL2_XMIT_TRIES##_n);		\
		tx_ctl->tx_control_3 |=					\
		    AR5K_REG_SM(tx_rate##_n,				\
		    AR5K_4W_TX_DESC_CTL3_XMIT_RATE##_n);		\
	}

		_XTX_TRIES(1);
		_XTX_TRIES(2);
		_XTX_TRIES(3);

#undef _XTX_TRIES

		return 1;
	}

	return 0;
}


static int
ath5k_hw_setup_no_mrr(struct ath5k_hw *ah, struct ath5k_desc *desc,
	unsigned int tx_rate1, u_int tx_tries1, u_int tx_rate2,
	u_int tx_tries2, unsigned int tx_rate3, u_int tx_tries3)
{
	return 0;
}


static int ath5k_hw_proc_2word_tx_status(struct ath5k_hw *ah,
		struct ath5k_desc *desc, struct ath5k_tx_status *ts)
{
	struct ath5k_hw_2w_tx_ctl *tx_ctl;
	struct ath5k_hw_tx_status *tx_status;

	ATH5K_TRACE(ah->ah_sc);

	tx_ctl = &desc->ud.ds_tx5210.tx_ctl;
	tx_status = &desc->ud.ds_tx5210.tx_stat;

	
	if (unlikely((tx_status->tx_status_1 & AR5K_DESC_TX_STATUS1_DONE) == 0))
		return -EINPROGRESS;

	
	ts->ts_tstamp = AR5K_REG_MS(tx_status->tx_status_0,
		AR5K_DESC_TX_STATUS0_SEND_TIMESTAMP);
	ts->ts_shortretry = AR5K_REG_MS(tx_status->tx_status_0,
		AR5K_DESC_TX_STATUS0_SHORT_RETRY_COUNT);
	ts->ts_longretry = AR5K_REG_MS(tx_status->tx_status_0,
		AR5K_DESC_TX_STATUS0_LONG_RETRY_COUNT);
	
	ts->ts_seqnum = AR5K_REG_MS(tx_status->tx_status_1,
		AR5K_DESC_TX_STATUS1_SEQ_NUM);
	ts->ts_rssi = AR5K_REG_MS(tx_status->tx_status_1,
		AR5K_DESC_TX_STATUS1_ACK_SIG_STRENGTH);
	ts->ts_antenna = 1;
	ts->ts_status = 0;
	ts->ts_rate[0] = AR5K_REG_MS(tx_ctl->tx_control_0,
		AR5K_2W_TX_DESC_CTL0_XMIT_RATE);
	ts->ts_retry[0] = ts->ts_longretry;
	ts->ts_final_idx = 0;

	if (!(tx_status->tx_status_0 & AR5K_DESC_TX_STATUS0_FRAME_XMIT_OK)) {
		if (tx_status->tx_status_0 &
				AR5K_DESC_TX_STATUS0_EXCESSIVE_RETRIES)
			ts->ts_status |= AR5K_TXERR_XRETRY;

		if (tx_status->tx_status_0 & AR5K_DESC_TX_STATUS0_FIFO_UNDERRUN)
			ts->ts_status |= AR5K_TXERR_FIFO;

		if (tx_status->tx_status_0 & AR5K_DESC_TX_STATUS0_FILTERED)
			ts->ts_status |= AR5K_TXERR_FILT;
	}

	return 0;
}


static int ath5k_hw_proc_4word_tx_status(struct ath5k_hw *ah,
		struct ath5k_desc *desc, struct ath5k_tx_status *ts)
{
	struct ath5k_hw_4w_tx_ctl *tx_ctl;
	struct ath5k_hw_tx_status *tx_status;

	ATH5K_TRACE(ah->ah_sc);

	tx_ctl = &desc->ud.ds_tx5212.tx_ctl;
	tx_status = &desc->ud.ds_tx5212.tx_stat;

	
	if (unlikely(!(tx_status->tx_status_1 & AR5K_DESC_TX_STATUS1_DONE)))
		return -EINPROGRESS;

	
	ts->ts_tstamp = AR5K_REG_MS(tx_status->tx_status_0,
		AR5K_DESC_TX_STATUS0_SEND_TIMESTAMP);
	ts->ts_shortretry = AR5K_REG_MS(tx_status->tx_status_0,
		AR5K_DESC_TX_STATUS0_SHORT_RETRY_COUNT);
	ts->ts_longretry = AR5K_REG_MS(tx_status->tx_status_0,
		AR5K_DESC_TX_STATUS0_LONG_RETRY_COUNT);
	ts->ts_seqnum = AR5K_REG_MS(tx_status->tx_status_1,
		AR5K_DESC_TX_STATUS1_SEQ_NUM);
	ts->ts_rssi = AR5K_REG_MS(tx_status->tx_status_1,
		AR5K_DESC_TX_STATUS1_ACK_SIG_STRENGTH);
	ts->ts_antenna = (tx_status->tx_status_1 &
		AR5K_DESC_TX_STATUS1_XMIT_ANTENNA) ? 2 : 1;
	ts->ts_status = 0;

	ts->ts_final_idx = AR5K_REG_MS(tx_status->tx_status_1,
			AR5K_DESC_TX_STATUS1_FINAL_TS_INDEX);

	
	ts->ts_retry[ts->ts_final_idx] = ts->ts_longretry;
	switch (ts->ts_final_idx) {
	case 3:
		ts->ts_rate[3] = AR5K_REG_MS(tx_ctl->tx_control_3,
			AR5K_4W_TX_DESC_CTL3_XMIT_RATE3);

		ts->ts_retry[2] = AR5K_REG_MS(tx_ctl->tx_control_2,
			AR5K_4W_TX_DESC_CTL2_XMIT_TRIES2);
		ts->ts_longretry += ts->ts_retry[2];
		
	case 2:
		ts->ts_rate[2] = AR5K_REG_MS(tx_ctl->tx_control_3,
			AR5K_4W_TX_DESC_CTL3_XMIT_RATE2);

		ts->ts_retry[1] = AR5K_REG_MS(tx_ctl->tx_control_2,
			AR5K_4W_TX_DESC_CTL2_XMIT_TRIES1);
		ts->ts_longretry += ts->ts_retry[1];
		
	case 1:
		ts->ts_rate[1] = AR5K_REG_MS(tx_ctl->tx_control_3,
			AR5K_4W_TX_DESC_CTL3_XMIT_RATE1);

		ts->ts_retry[0] = AR5K_REG_MS(tx_ctl->tx_control_2,
			AR5K_4W_TX_DESC_CTL2_XMIT_TRIES1);
		ts->ts_longretry += ts->ts_retry[0];
		
	case 0:
		ts->ts_rate[0] = tx_ctl->tx_control_3 &
			AR5K_4W_TX_DESC_CTL3_XMIT_RATE0;
		break;
	}

	
	if (!(tx_status->tx_status_0 & AR5K_DESC_TX_STATUS0_FRAME_XMIT_OK)) {
		if (tx_status->tx_status_0 &
				AR5K_DESC_TX_STATUS0_EXCESSIVE_RETRIES)
			ts->ts_status |= AR5K_TXERR_XRETRY;

		if (tx_status->tx_status_0 & AR5K_DESC_TX_STATUS0_FIFO_UNDERRUN)
			ts->ts_status |= AR5K_TXERR_FIFO;

		if (tx_status->tx_status_0 & AR5K_DESC_TX_STATUS0_FILTERED)
			ts->ts_status |= AR5K_TXERR_FILT;
	}

	return 0;
}




static int ath5k_hw_setup_rx_desc(struct ath5k_hw *ah, struct ath5k_desc *desc,
			u32 size, unsigned int flags)
{
	struct ath5k_hw_rx_ctl *rx_ctl;

	ATH5K_TRACE(ah->ah_sc);
	rx_ctl = &desc->ud.ds_rx.rx_ctl;

	
	memset(&desc->ud.ds_rx, 0, sizeof(struct ath5k_hw_all_rx_desc));

	
	rx_ctl->rx_control_1 = size & AR5K_DESC_RX_CTL1_BUF_LEN;
	if (unlikely(rx_ctl->rx_control_1 != size))
		return -EINVAL;

	if (flags & AR5K_RXDESC_INTREQ)
		rx_ctl->rx_control_1 |= AR5K_DESC_RX_CTL1_INTREQ;

	return 0;
}


static int ath5k_hw_proc_5210_rx_status(struct ath5k_hw *ah,
		struct ath5k_desc *desc, struct ath5k_rx_status *rs)
{
	struct ath5k_hw_rx_status *rx_status;

	rx_status = &desc->ud.ds_rx.u.rx_stat;

	
	if (unlikely(!(rx_status->rx_status_1 &
	AR5K_5210_RX_DESC_STATUS1_DONE)))
		return -EINPROGRESS;

	
	rs->rs_datalen = rx_status->rx_status_0 &
		AR5K_5210_RX_DESC_STATUS0_DATA_LEN;
	rs->rs_rssi = AR5K_REG_MS(rx_status->rx_status_0,
		AR5K_5210_RX_DESC_STATUS0_RECEIVE_SIGNAL);
	rs->rs_rate = AR5K_REG_MS(rx_status->rx_status_0,
		AR5K_5210_RX_DESC_STATUS0_RECEIVE_RATE);
	rs->rs_antenna = AR5K_REG_MS(rx_status->rx_status_0,
		AR5K_5210_RX_DESC_STATUS0_RECEIVE_ANTENNA);
	rs->rs_more = !!(rx_status->rx_status_0 &
		AR5K_5210_RX_DESC_STATUS0_MORE);
	
	rs->rs_tstamp = AR5K_REG_MS(rx_status->rx_status_1,
		AR5K_5210_RX_DESC_STATUS1_RECEIVE_TIMESTAMP);
	rs->rs_status = 0;
	rs->rs_phyerr = 0;

	
	if (rx_status->rx_status_1 & AR5K_5210_RX_DESC_STATUS1_KEY_INDEX_VALID)
		rs->rs_keyix = AR5K_REG_MS(rx_status->rx_status_1,
			AR5K_5210_RX_DESC_STATUS1_KEY_INDEX);
	else
		rs->rs_keyix = AR5K_RXKEYIX_INVALID;

	
	if (!(rx_status->rx_status_1 &
	AR5K_5210_RX_DESC_STATUS1_FRAME_RECEIVE_OK)) {
		if (rx_status->rx_status_1 &
				AR5K_5210_RX_DESC_STATUS1_CRC_ERROR)
			rs->rs_status |= AR5K_RXERR_CRC;

		if (rx_status->rx_status_1 &
				AR5K_5210_RX_DESC_STATUS1_FIFO_OVERRUN)
			rs->rs_status |= AR5K_RXERR_FIFO;

		if (rx_status->rx_status_1 &
				AR5K_5210_RX_DESC_STATUS1_PHY_ERROR) {
			rs->rs_status |= AR5K_RXERR_PHY;
			rs->rs_phyerr |= AR5K_REG_MS(rx_status->rx_status_1,
				AR5K_5210_RX_DESC_STATUS1_PHY_ERROR);
		}

		if (rx_status->rx_status_1 &
				AR5K_5210_RX_DESC_STATUS1_DECRYPT_CRC_ERROR)
			rs->rs_status |= AR5K_RXERR_DECRYPT;
	}

	return 0;
}


static int ath5k_hw_proc_5212_rx_status(struct ath5k_hw *ah,
		struct ath5k_desc *desc, struct ath5k_rx_status *rs)
{
	struct ath5k_hw_rx_status *rx_status;
	struct ath5k_hw_rx_error *rx_err;

	ATH5K_TRACE(ah->ah_sc);
	rx_status = &desc->ud.ds_rx.u.rx_stat;

	
	rx_err = &desc->ud.ds_rx.u.rx_err;

	
	if (unlikely(!(rx_status->rx_status_1 &
	AR5K_5212_RX_DESC_STATUS1_DONE)))
		return -EINPROGRESS;

	
	rs->rs_datalen = rx_status->rx_status_0 &
		AR5K_5212_RX_DESC_STATUS0_DATA_LEN;
	rs->rs_rssi = AR5K_REG_MS(rx_status->rx_status_0,
		AR5K_5212_RX_DESC_STATUS0_RECEIVE_SIGNAL);
	rs->rs_rate = AR5K_REG_MS(rx_status->rx_status_0,
		AR5K_5212_RX_DESC_STATUS0_RECEIVE_RATE);
	rs->rs_antenna = AR5K_REG_MS(rx_status->rx_status_0,
		AR5K_5212_RX_DESC_STATUS0_RECEIVE_ANTENNA);
	rs->rs_more = !!(rx_status->rx_status_0 &
		AR5K_5212_RX_DESC_STATUS0_MORE);
	rs->rs_tstamp = AR5K_REG_MS(rx_status->rx_status_1,
		AR5K_5212_RX_DESC_STATUS1_RECEIVE_TIMESTAMP);
	rs->rs_status = 0;
	rs->rs_phyerr = 0;

	
	if (rx_status->rx_status_1 & AR5K_5212_RX_DESC_STATUS1_KEY_INDEX_VALID)
		rs->rs_keyix = AR5K_REG_MS(rx_status->rx_status_1,
				AR5K_5212_RX_DESC_STATUS1_KEY_INDEX);
	else
		rs->rs_keyix = AR5K_RXKEYIX_INVALID;

	
	if (!(rx_status->rx_status_1 &
	AR5K_5212_RX_DESC_STATUS1_FRAME_RECEIVE_OK)) {
		if (rx_status->rx_status_1 &
				AR5K_5212_RX_DESC_STATUS1_CRC_ERROR)
			rs->rs_status |= AR5K_RXERR_CRC;

		if (rx_status->rx_status_1 &
				AR5K_5212_RX_DESC_STATUS1_PHY_ERROR) {
			rs->rs_status |= AR5K_RXERR_PHY;
			rs->rs_phyerr |= AR5K_REG_MS(rx_err->rx_error_1,
					   AR5K_RX_DESC_ERROR1_PHY_ERROR_CODE);
		}

		if (rx_status->rx_status_1 &
				AR5K_5212_RX_DESC_STATUS1_DECRYPT_CRC_ERROR)
			rs->rs_status |= AR5K_RXERR_DECRYPT;

		if (rx_status->rx_status_1 &
				AR5K_5212_RX_DESC_STATUS1_MIC_ERROR)
			rs->rs_status |= AR5K_RXERR_MIC;
	}

	return 0;
}


int ath5k_hw_init_desc_functions(struct ath5k_hw *ah)
{

	if (ah->ah_version != AR5K_AR5210 &&
		ah->ah_version != AR5K_AR5211 &&
		ah->ah_version != AR5K_AR5212)
			return -ENOTSUPP;

	
	if (ah->ah_version == AR5K_AR5212)
		ah->ah_magic = AR5K_EEPROM_MAGIC_5212;
	else if (ah->ah_version == AR5K_AR5211)
		ah->ah_magic = AR5K_EEPROM_MAGIC_5211;

	if (ah->ah_version == AR5K_AR5212) {
		ah->ah_setup_rx_desc = ath5k_hw_setup_rx_desc;
		ah->ah_setup_tx_desc = ath5k_hw_setup_4word_tx_desc;
		ah->ah_setup_mrr_tx_desc = ath5k_hw_setup_mrr_tx_desc;
		ah->ah_proc_tx_desc = ath5k_hw_proc_4word_tx_status;
	} else {
		ah->ah_setup_rx_desc = ath5k_hw_setup_rx_desc;
		ah->ah_setup_tx_desc = ath5k_hw_setup_2word_tx_desc;
		ah->ah_setup_mrr_tx_desc = ath5k_hw_setup_no_mrr;
		ah->ah_proc_tx_desc = ath5k_hw_proc_2word_tx_status;
	}

	if (ah->ah_version == AR5K_AR5212)
		ah->ah_proc_rx_desc = ath5k_hw_proc_5212_rx_status;
	else if (ah->ah_version <= AR5K_AR5211)
		ah->ah_proc_rx_desc = ath5k_hw_proc_5210_rx_status;

	return 0;
}

