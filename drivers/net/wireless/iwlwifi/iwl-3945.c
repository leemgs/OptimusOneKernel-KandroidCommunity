

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/wireless.h>
#include <linux/firmware.h>
#include <linux/etherdevice.h>
#include <asm/unaligned.h>
#include <net/mac80211.h>

#include "iwl-fh.h"
#include "iwl-3945-fh.h"
#include "iwl-commands.h"
#include "iwl-sta.h"
#include "iwl-3945.h"
#include "iwl-eeprom.h"
#include "iwl-helpers.h"
#include "iwl-core.h"
#include "iwl-agn-rs.h"

#define IWL_DECLARE_RATE_INFO(r, ip, in, rp, rn, pp, np)    \
	[IWL_RATE_##r##M_INDEX] = { IWL_RATE_##r##M_PLCP,   \
				    IWL_RATE_##r##M_IEEE,   \
				    IWL_RATE_##ip##M_INDEX, \
				    IWL_RATE_##in##M_INDEX, \
				    IWL_RATE_##rp##M_INDEX, \
				    IWL_RATE_##rn##M_INDEX, \
				    IWL_RATE_##pp##M_INDEX, \
				    IWL_RATE_##np##M_INDEX, \
				    IWL_RATE_##r##M_INDEX_TABLE, \
				    IWL_RATE_##ip##M_INDEX_TABLE }


const struct iwl3945_rate_info iwl3945_rates[IWL_RATE_COUNT_3945] = {
	IWL_DECLARE_RATE_INFO(1, INV, 2, INV, 2, INV, 2),    
	IWL_DECLARE_RATE_INFO(2, 1, 5, 1, 5, 1, 5),          
	IWL_DECLARE_RATE_INFO(5, 2, 6, 2, 11, 2, 11),        
	IWL_DECLARE_RATE_INFO(11, 9, 12, 5, 12, 5, 18),      
	IWL_DECLARE_RATE_INFO(6, 5, 9, 5, 11, 5, 11),        
	IWL_DECLARE_RATE_INFO(9, 6, 11, 5, 11, 5, 11),       
	IWL_DECLARE_RATE_INFO(12, 11, 18, 11, 18, 11, 18),   
	IWL_DECLARE_RATE_INFO(18, 12, 24, 12, 24, 11, 24),   
	IWL_DECLARE_RATE_INFO(24, 18, 36, 18, 36, 18, 36),   
	IWL_DECLARE_RATE_INFO(36, 24, 48, 24, 48, 24, 48),   
	IWL_DECLARE_RATE_INFO(48, 36, 54, 36, 54, 36, 54),   
	IWL_DECLARE_RATE_INFO(54, 48, INV, 48, INV, 48, INV),
};


#define IWL_EVT_DISABLE (0)
#define IWL_EVT_DISABLE_SIZE (1532/32)


void iwl3945_disable_events(struct iwl_priv *priv)
{
	int i;
	u32 base;		
	u32 disable_ptr;	
	u32 array_size;		
	u32 evt_disable[IWL_EVT_DISABLE_SIZE] = {
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
		0x00000000,	
	};

	base = le32_to_cpu(priv->card_alive.log_event_table_ptr);
	if (!iwl3945_hw_valid_rtc_data_addr(base)) {
		IWL_ERR(priv, "Invalid event log pointer 0x%08X\n", base);
		return;
	}

	disable_ptr = iwl_read_targ_mem(priv, base + (4 * sizeof(u32)));
	array_size = iwl_read_targ_mem(priv, base + (5 * sizeof(u32)));

	if (IWL_EVT_DISABLE && (array_size == IWL_EVT_DISABLE_SIZE)) {
		IWL_DEBUG_INFO(priv, "Disabling selected uCode log events at 0x%x\n",
			       disable_ptr);
		for (i = 0; i < IWL_EVT_DISABLE_SIZE; i++)
			iwl_write_targ_mem(priv,
					   disable_ptr + (i * sizeof(u32)),
					   evt_disable[i]);

	} else {
		IWL_DEBUG_INFO(priv, "Selected uCode log events may be disabled\n");
		IWL_DEBUG_INFO(priv, "  by writing \"1\"s into disable bitmap\n");
		IWL_DEBUG_INFO(priv, "  in SRAM at 0x%x, size %d u32s\n",
			       disable_ptr, array_size);
	}

}

static int iwl3945_hwrate_to_plcp_idx(u8 plcp)
{
	int idx;

	for (idx = 0; idx < IWL_RATE_COUNT; idx++)
		if (iwl3945_rates[idx].plcp == plcp)
			return idx;
	return -1;
}

#ifdef CONFIG_IWLWIFI_DEBUG
#define TX_STATUS_ENTRY(x) case TX_STATUS_FAIL_ ## x: return #x

static const char *iwl3945_get_tx_fail_reason(u32 status)
{
	switch (status & TX_STATUS_MSK) {
	case TX_STATUS_SUCCESS:
		return "SUCCESS";
		TX_STATUS_ENTRY(SHORT_LIMIT);
		TX_STATUS_ENTRY(LONG_LIMIT);
		TX_STATUS_ENTRY(FIFO_UNDERRUN);
		TX_STATUS_ENTRY(MGMNT_ABORT);
		TX_STATUS_ENTRY(NEXT_FRAG);
		TX_STATUS_ENTRY(LIFE_EXPIRE);
		TX_STATUS_ENTRY(DEST_PS);
		TX_STATUS_ENTRY(ABORTED);
		TX_STATUS_ENTRY(BT_RETRY);
		TX_STATUS_ENTRY(STA_INVALID);
		TX_STATUS_ENTRY(FRAG_DROPPED);
		TX_STATUS_ENTRY(TID_DISABLE);
		TX_STATUS_ENTRY(FRAME_FLUSHED);
		TX_STATUS_ENTRY(INSUFFICIENT_CF_POLL);
		TX_STATUS_ENTRY(TX_LOCKED);
		TX_STATUS_ENTRY(NO_BEACON_ON_RADAR);
	}

	return "UNKNOWN";
}
#else
static inline const char *iwl3945_get_tx_fail_reason(u32 status)
{
	return "";
}
#endif


int iwl3945_rs_next_rate(struct iwl_priv *priv, int rate)
{
	int next_rate = iwl3945_get_prev_ieee_rate(rate);

	switch (priv->band) {
	case IEEE80211_BAND_5GHZ:
		if (rate == IWL_RATE_12M_INDEX)
			next_rate = IWL_RATE_9M_INDEX;
		else if (rate == IWL_RATE_6M_INDEX)
			next_rate = IWL_RATE_6M_INDEX;
		break;
	case IEEE80211_BAND_2GHZ:
		if (!(priv->sta_supp_rates & IWL_OFDM_RATES_MASK) &&
		    iwl_is_associated(priv)) {
			if (rate == IWL_RATE_11M_INDEX)
				next_rate = IWL_RATE_5M_INDEX;
		}
		break;

	default:
		break;
	}

	return next_rate;
}



static void iwl3945_tx_queue_reclaim(struct iwl_priv *priv,
				     int txq_id, int index)
{
	struct iwl_tx_queue *txq = &priv->txq[txq_id];
	struct iwl_queue *q = &txq->q;
	struct iwl_tx_info *tx_info;

	BUG_ON(txq_id == IWL_CMD_QUEUE_NUM);

	for (index = iwl_queue_inc_wrap(index, q->n_bd); q->read_ptr != index;
		q->read_ptr = iwl_queue_inc_wrap(q->read_ptr, q->n_bd)) {

		tx_info = &txq->txb[txq->q.read_ptr];
		ieee80211_tx_status_irqsafe(priv->hw, tx_info->skb[0]);
		tx_info->skb[0] = NULL;
		priv->cfg->ops->lib->txq_free_tfd(priv, txq);
	}

	if (iwl_queue_space(q) > q->low_mark && (txq_id >= 0) &&
			(txq_id != IWL_CMD_QUEUE_NUM) &&
			priv->mac80211_registered)
		iwl_wake_queue(priv, txq_id);
}


static void iwl3945_rx_reply_tx(struct iwl_priv *priv,
			    struct iwl_rx_mem_buffer *rxb)
{
	struct iwl_rx_packet *pkt = (void *)rxb->skb->data;
	u16 sequence = le16_to_cpu(pkt->hdr.sequence);
	int txq_id = SEQ_TO_QUEUE(sequence);
	int index = SEQ_TO_INDEX(sequence);
	struct iwl_tx_queue *txq = &priv->txq[txq_id];
	struct ieee80211_tx_info *info;
	struct iwl3945_tx_resp *tx_resp = (void *)&pkt->u.raw[0];
	u32  status = le32_to_cpu(tx_resp->status);
	int rate_idx;
	int fail;

	if ((index >= txq->q.n_bd) || (iwl_queue_used(&txq->q, index) == 0)) {
		IWL_ERR(priv, "Read index for DMA queue txq_id (%d) index %d "
			  "is out of range [0-%d] %d %d\n", txq_id,
			  index, txq->q.n_bd, txq->q.write_ptr,
			  txq->q.read_ptr);
		return;
	}

	info = IEEE80211_SKB_CB(txq->txb[txq->q.read_ptr].skb[0]);
	ieee80211_tx_info_clear_status(info);

	
	rate_idx = iwl3945_hwrate_to_plcp_idx(tx_resp->rate);
	if (info->band == IEEE80211_BAND_5GHZ)
		rate_idx -= IWL_FIRST_OFDM_RATE;

	fail = tx_resp->failure_frame;

	info->status.rates[0].idx = rate_idx;
	info->status.rates[0].count = fail + 1; 

	
	info->flags |= ((status & TX_STATUS_MSK) == TX_STATUS_SUCCESS) ?
				IEEE80211_TX_STAT_ACK : 0;

	IWL_DEBUG_TX(priv, "Tx queue %d Status %s (0x%08x) plcp rate %d retries %d\n",
			txq_id, iwl3945_get_tx_fail_reason(status), status,
			tx_resp->rate, tx_resp->failure_frame);

	IWL_DEBUG_TX_REPLY(priv, "Tx queue reclaim %d\n", index);
	iwl3945_tx_queue_reclaim(priv, txq_id, index);

	if (iwl_check_bits(status, TX_ABORT_REQUIRED_MSK))
		IWL_ERR(priv, "TODO:  Implement Tx ABORT REQUIRED!!!\n");
}





void iwl3945_hw_rx_statistics(struct iwl_priv *priv,
		struct iwl_rx_mem_buffer *rxb)
{
	struct iwl_rx_packet *pkt = (void *)rxb->skb->data;
	IWL_DEBUG_RX(priv, "Statistics notification received (%d vs %d).\n",
		     (int)sizeof(struct iwl3945_notif_statistics),
		     le32_to_cpu(pkt->len_n_flags) & FH_RSCSR_FRAME_SIZE_MSK);

	memcpy(&priv->statistics_39, pkt->u.raw, sizeof(priv->statistics_39));

	iwl3945_led_background(priv);

	priv->last_statistics_time = jiffies;
}


#ifdef CONFIG_IWLWIFI_DEBUG


static void _iwl3945_dbg_report_frame(struct iwl_priv *priv,
		      struct iwl_rx_packet *pkt,
		      struct ieee80211_hdr *header, int group100)
{
	u32 to_us;
	u32 print_summary = 0;
	u32 print_dump = 0;	
	u32 hundred = 0;
	u32 dataframe = 0;
	__le16 fc;
	u16 seq_ctl;
	u16 channel;
	u16 phy_flags;
	u16 length;
	u16 status;
	u16 bcn_tmr;
	u32 tsf_low;
	u64 tsf;
	u8 rssi;
	u8 agc;
	u16 sig_avg;
	u16 noise_diff;
	struct iwl3945_rx_frame_stats *rx_stats = IWL_RX_STATS(pkt);
	struct iwl3945_rx_frame_hdr *rx_hdr = IWL_RX_HDR(pkt);
	struct iwl3945_rx_frame_end *rx_end = IWL_RX_END(pkt);
	u8 *data = IWL_RX_DATA(pkt);

	
	fc = header->frame_control;
	seq_ctl = le16_to_cpu(header->seq_ctrl);

	
	channel = le16_to_cpu(rx_hdr->channel);
	phy_flags = le16_to_cpu(rx_hdr->phy_flags);
	length = le16_to_cpu(rx_hdr->len);

	
	status = le32_to_cpu(rx_end->status);
	bcn_tmr = le32_to_cpu(rx_end->beacon_timestamp);
	tsf_low = le64_to_cpu(rx_end->timestamp) & 0x0ffffffff;
	tsf = le64_to_cpu(rx_end->timestamp);

	
	rssi = rx_stats->rssi;
	agc = rx_stats->agc;
	sig_avg = le16_to_cpu(rx_stats->sig_avg);
	noise_diff = le16_to_cpu(rx_stats->noise_diff);

	to_us = !compare_ether_addr(header->addr1, priv->mac_addr);

	
	if (to_us && (fc & ~cpu_to_le16(IEEE80211_FCTL_PROTECTED)) ==
	    cpu_to_le16(IEEE80211_FCTL_FROMDS | IEEE80211_FTYPE_DATA)) {
		dataframe = 1;
		if (!group100)
			print_summary = 1;	
		else if (priv->framecnt_to_us < 100) {
			priv->framecnt_to_us++;
			print_summary = 0;
		} else {
			priv->framecnt_to_us = 0;
			print_summary = 1;
			hundred = 1;
		}
	} else {
		
		print_summary = 1;
	}

	if (print_summary) {
		char *title;
		int rate;

		if (hundred)
			title = "100Frames";
		else if (ieee80211_has_retry(fc))
			title = "Retry";
		else if (ieee80211_is_assoc_resp(fc))
			title = "AscRsp";
		else if (ieee80211_is_reassoc_resp(fc))
			title = "RasRsp";
		else if (ieee80211_is_probe_resp(fc)) {
			title = "PrbRsp";
			print_dump = 1;	
		} else if (ieee80211_is_beacon(fc)) {
			title = "Beacon";
			print_dump = 1;	
		} else if (ieee80211_is_atim(fc))
			title = "ATIM";
		else if (ieee80211_is_auth(fc))
			title = "Auth";
		else if (ieee80211_is_deauth(fc))
			title = "DeAuth";
		else if (ieee80211_is_disassoc(fc))
			title = "DisAssoc";
		else
			title = "Frame";

		rate = iwl3945_hwrate_to_plcp_idx(rx_hdr->rate);
		if (rate == -1)
			rate = 0;
		else
			rate = iwl3945_rates[rate].ieee / 2;

		
		if (dataframe)
			IWL_DEBUG_RX(priv, "%s: mhd=0x%04x, dst=0x%02x, "
				     "len=%u, rssi=%d, chnl=%d, rate=%d, \n",
				     title, le16_to_cpu(fc), header->addr1[5],
				     length, rssi, channel, rate);
		else {
			
			IWL_DEBUG_RX(priv, "%s: 0x%04x, dst=0x%02x, "
				     "src=0x%02x, rssi=%u, tim=%lu usec, "
				     "phy=0x%02x, chnl=%d\n",
				     title, le16_to_cpu(fc), header->addr1[5],
				     header->addr3[5], rssi,
				     tsf_low - priv->scan_start_tsf,
				     phy_flags, channel);
		}
	}
	if (print_dump)
		iwl_print_hex_dump(priv, IWL_DL_RX, data, length);
}

static void iwl3945_dbg_report_frame(struct iwl_priv *priv,
		      struct iwl_rx_packet *pkt,
		      struct ieee80211_hdr *header, int group100)
{
	if (iwl_get_debug_level(priv) & IWL_DL_RX)
		_iwl3945_dbg_report_frame(priv, pkt, header, group100);
}

#else
static inline void iwl3945_dbg_report_frame(struct iwl_priv *priv,
		      struct iwl_rx_packet *pkt,
		      struct ieee80211_hdr *header, int group100)
{
}
#endif


static int iwl3945_is_network_packet(struct iwl_priv *priv,
		struct ieee80211_hdr *header)
{
	
	switch (priv->iw_mode) {
	case NL80211_IFTYPE_ADHOC: 
		
		return !compare_ether_addr(header->addr3, priv->bssid);
	case NL80211_IFTYPE_STATION: 
		
		return !compare_ether_addr(header->addr2, priv->bssid);
	default:
		return 1;
	}
}

static void iwl3945_pass_packet_to_mac80211(struct iwl_priv *priv,
				   struct iwl_rx_mem_buffer *rxb,
				   struct ieee80211_rx_status *stats)
{
	struct iwl_rx_packet *pkt = (struct iwl_rx_packet *)rxb->skb->data;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)IWL_RX_DATA(pkt);
	struct iwl3945_rx_frame_hdr *rx_hdr = IWL_RX_HDR(pkt);
	struct iwl3945_rx_frame_end *rx_end = IWL_RX_END(pkt);
	short len = le16_to_cpu(rx_hdr->len);

	
	if (unlikely((len + IWL39_RX_FRAME_SIZE) > skb_tailroom(rxb->skb))) {
		IWL_DEBUG_DROP(priv, "Corruption detected!\n");
		return;
	}

	
	if (unlikely(!priv->is_open)) {
		IWL_DEBUG_DROP_LIMIT(priv,
			"Dropping packet while interface is not open.\n");
		return;
	}

	skb_reserve(rxb->skb, (void *)rx_hdr->payload - (void *)pkt);
	
	skb_put(rxb->skb, le16_to_cpu(rx_hdr->len));

	if (!iwl3945_mod_params.sw_crypto)
		iwl_set_decrypted_flag(priv,
				       (struct ieee80211_hdr *)rxb->skb->data,
				       le32_to_cpu(rx_end->status), stats);

#ifdef CONFIG_IWLWIFI_LEDS
	if (ieee80211_is_data(hdr->frame_control))
		priv->rxtxpackets += len;
#endif
	iwl_update_stats(priv, false, hdr->frame_control, len);

	memcpy(IEEE80211_SKB_RXCB(rxb->skb), stats, sizeof(*stats));
	ieee80211_rx_irqsafe(priv->hw, rxb->skb);
	rxb->skb = NULL;
}

#define IWL_DELAY_NEXT_SCAN_AFTER_ASSOC (HZ*6)

static void iwl3945_rx_reply_rx(struct iwl_priv *priv,
				struct iwl_rx_mem_buffer *rxb)
{
	struct ieee80211_hdr *header;
	struct ieee80211_rx_status rx_status;
	struct iwl_rx_packet *pkt = (void *)rxb->skb->data;
	struct iwl3945_rx_frame_stats *rx_stats = IWL_RX_STATS(pkt);
	struct iwl3945_rx_frame_hdr *rx_hdr = IWL_RX_HDR(pkt);
	struct iwl3945_rx_frame_end *rx_end = IWL_RX_END(pkt);
	int snr;
	u16 rx_stats_sig_avg = le16_to_cpu(rx_stats->sig_avg);
	u16 rx_stats_noise_diff = le16_to_cpu(rx_stats->noise_diff);
	u8 network_packet;

	rx_status.flag = 0;
	rx_status.mactime = le64_to_cpu(rx_end->timestamp);
	rx_status.freq =
		ieee80211_channel_to_frequency(le16_to_cpu(rx_hdr->channel));
	rx_status.band = (rx_hdr->phy_flags & RX_RES_PHY_FLAGS_BAND_24_MSK) ?
				IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ;

	rx_status.rate_idx = iwl3945_hwrate_to_plcp_idx(rx_hdr->rate);
	if (rx_status.band == IEEE80211_BAND_5GHZ)
		rx_status.rate_idx -= IWL_FIRST_OFDM_RATE;

	rx_status.antenna = (le16_to_cpu(rx_hdr->phy_flags) &
					RX_RES_PHY_FLAGS_ANTENNA_MSK) >> 4;

	
	if (rx_hdr->phy_flags & RX_RES_PHY_FLAGS_SHORT_PREAMBLE_MSK)
		rx_status.flag |= RX_FLAG_SHORTPRE;

	if ((unlikely(rx_stats->phy_count > 20))) {
		IWL_DEBUG_DROP(priv, "dsp size out of range [0,20]: %d/n",
				rx_stats->phy_count);
		return;
	}

	if (!(rx_end->status & RX_RES_STATUS_NO_CRC32_ERROR)
	    || !(rx_end->status & RX_RES_STATUS_NO_RXE_OVERFLOW)) {
		IWL_DEBUG_RX(priv, "Bad CRC or FIFO: 0x%08X.\n", rx_end->status);
		return;
	}



	
	rx_status.signal = rx_stats->rssi - IWL39_RSSI_OFFSET;

	
	if (priv->last_rx_noise == 0)
		priv->last_rx_noise = IWL_NOISE_MEAS_NOT_AVAILABLE;

	
	if (rx_stats_noise_diff) {
		snr = rx_stats_sig_avg / rx_stats_noise_diff;
		rx_status.noise = rx_status.signal -
					iwl3945_calc_db_from_ratio(snr);
		rx_status.qual = iwl3945_calc_sig_qual(rx_status.signal,
							 rx_status.noise);

	
	} else {
		rx_status.noise = priv->last_rx_noise;
		rx_status.qual = iwl3945_calc_sig_qual(rx_status.signal, 0);
	}


	IWL_DEBUG_STATS(priv, "Rssi %d noise %d qual %d sig_avg %d noise_diff %d\n",
			rx_status.signal, rx_status.noise, rx_status.qual,
			rx_stats_sig_avg, rx_stats_noise_diff);

	header = (struct ieee80211_hdr *)IWL_RX_DATA(pkt);

	network_packet = iwl3945_is_network_packet(priv, header);

	IWL_DEBUG_STATS_LIMIT(priv, "[%c] %d RSSI:%d Signal:%u, Noise:%u, Rate:%u\n",
			      network_packet ? '*' : ' ',
			      le16_to_cpu(rx_hdr->channel),
			      rx_status.signal, rx_status.signal,
			      rx_status.noise, rx_status.rate_idx);

	
	iwl3945_dbg_report_frame(priv, pkt, header, 1);
	iwl_dbg_log_rx_data_frame(priv, le16_to_cpu(rx_hdr->len), header);

	if (network_packet) {
		priv->last_beacon_time = le32_to_cpu(rx_end->beacon_timestamp);
		priv->last_tsf = le64_to_cpu(rx_end->timestamp);
		priv->last_rx_rssi = rx_status.signal;
		priv->last_rx_noise = rx_status.noise;
	}

	iwl3945_pass_packet_to_mac80211(priv, rxb, &rx_status);
}

int iwl3945_hw_txq_attach_buf_to_tfd(struct iwl_priv *priv,
				     struct iwl_tx_queue *txq,
				     dma_addr_t addr, u16 len, u8 reset, u8 pad)
{
	int count;
	struct iwl_queue *q;
	struct iwl3945_tfd *tfd, *tfd_tmp;

	q = &txq->q;
	tfd_tmp = (struct iwl3945_tfd *)txq->tfds;
	tfd = &tfd_tmp[q->write_ptr];

	if (reset)
		memset(tfd, 0, sizeof(*tfd));

	count = TFD_CTL_COUNT_GET(le32_to_cpu(tfd->control_flags));

	if ((count >= NUM_TFD_CHUNKS) || (count < 0)) {
		IWL_ERR(priv, "Error can not send more than %d chunks\n",
			  NUM_TFD_CHUNKS);
		return -EINVAL;
	}

	tfd->tbs[count].addr = cpu_to_le32(addr);
	tfd->tbs[count].len = cpu_to_le32(len);

	count++;

	tfd->control_flags = cpu_to_le32(TFD_CTL_COUNT_SET(count) |
					 TFD_CTL_PAD_SET(pad));

	return 0;
}


void iwl3945_hw_txq_free_tfd(struct iwl_priv *priv, struct iwl_tx_queue *txq)
{
	struct iwl3945_tfd *tfd_tmp = (struct iwl3945_tfd *)txq->tfds;
	int index = txq->q.read_ptr;
	struct iwl3945_tfd *tfd = &tfd_tmp[index];
	struct pci_dev *dev = priv->pci_dev;
	int i;
	int counter;

	
	counter = TFD_CTL_COUNT_GET(le32_to_cpu(tfd->control_flags));
	if (counter > NUM_TFD_CHUNKS) {
		IWL_ERR(priv, "Too many chunks: %i\n", counter);
		
		return;
	}

	
	if (counter)
		pci_unmap_single(dev,
				pci_unmap_addr(&txq->meta[index], mapping),
				pci_unmap_len(&txq->meta[index], len),
				PCI_DMA_TODEVICE);

	

	for (i = 1; i < counter; i++) {
		pci_unmap_single(dev, le32_to_cpu(tfd->tbs[i].addr),
			 le32_to_cpu(tfd->tbs[i].len), PCI_DMA_TODEVICE);
		if (txq->txb[txq->q.read_ptr].skb[0]) {
			struct sk_buff *skb = txq->txb[txq->q.read_ptr].skb[0];
			if (txq->txb[txq->q.read_ptr].skb[0]) {
				
				dev_kfree_skb_any(skb);
				txq->txb[txq->q.read_ptr].skb[0] = NULL;
			}
		}
	}
	return ;
}


void iwl3945_hw_build_tx_cmd_rate(struct iwl_priv *priv,
				  struct iwl_device_cmd *cmd,
				  struct ieee80211_tx_info *info,
				  struct ieee80211_hdr *hdr,
				  int sta_id, int tx_id)
{
	u16 hw_value = ieee80211_get_tx_rate(priv->hw, info)->hw_value;
	u16 rate_index = min(hw_value & 0xffff, IWL_RATE_COUNT - 1);
	u16 rate_mask;
	int rate;
	u8 rts_retry_limit;
	u8 data_retry_limit;
	__le32 tx_flags;
	__le16 fc = hdr->frame_control;
	struct iwl3945_tx_cmd *tx = (struct iwl3945_tx_cmd *)cmd->cmd.payload;

	rate = iwl3945_rates[rate_index].plcp;
	tx_flags = tx->tx_flags;

	
	rate_mask = IWL_RATES_MASK;

	if (tx_id >= IWL_CMD_QUEUE_NUM)
		rts_retry_limit = 3;
	else
		rts_retry_limit = 7;

	if (ieee80211_is_probe_resp(fc)) {
		data_retry_limit = 3;
		if (data_retry_limit < rts_retry_limit)
			rts_retry_limit = data_retry_limit;
	} else
		data_retry_limit = IWL_DEFAULT_TX_RETRY;

	if (priv->data_retry_limit != -1)
		data_retry_limit = priv->data_retry_limit;

	if (ieee80211_is_mgmt(fc)) {
		switch (fc & cpu_to_le16(IEEE80211_FCTL_STYPE)) {
		case cpu_to_le16(IEEE80211_STYPE_AUTH):
		case cpu_to_le16(IEEE80211_STYPE_DEAUTH):
		case cpu_to_le16(IEEE80211_STYPE_ASSOC_REQ):
		case cpu_to_le16(IEEE80211_STYPE_REASSOC_REQ):
			if (tx_flags & TX_CMD_FLG_RTS_MSK) {
				tx_flags &= ~TX_CMD_FLG_RTS_MSK;
				tx_flags |= TX_CMD_FLG_CTS_MSK;
			}
			break;
		default:
			break;
		}
	}

	tx->rts_retry_limit = rts_retry_limit;
	tx->data_retry_limit = data_retry_limit;
	tx->rate = rate;
	tx->tx_flags = tx_flags;

	
	tx->supp_rates[0] =
	   ((rate_mask & IWL_OFDM_RATES_MASK) >> IWL_FIRST_OFDM_RATE) & 0xFF;

	
	tx->supp_rates[1] = (rate_mask & 0xF);

	IWL_DEBUG_RATE(priv, "Tx sta id: %d, rate: %d (plcp), flags: 0x%4X "
		       "cck/ofdm mask: 0x%x/0x%x\n", sta_id,
		       tx->rate, le32_to_cpu(tx->tx_flags),
		       tx->supp_rates[1], tx->supp_rates[0]);
}

u8 iwl3945_sync_sta(struct iwl_priv *priv, int sta_id, u16 tx_rate, u8 flags)
{
	unsigned long flags_spin;
	struct iwl_station_entry *station;

	if (sta_id == IWL_INVALID_STATION)
		return IWL_INVALID_STATION;

	spin_lock_irqsave(&priv->sta_lock, flags_spin);
	station = &priv->stations[sta_id];

	station->sta.sta.modify_mask = STA_MODIFY_TX_RATE_MSK;
	station->sta.rate_n_flags = cpu_to_le16(tx_rate);
	station->sta.mode = STA_CONTROL_MODIFY_MSK;

	spin_unlock_irqrestore(&priv->sta_lock, flags_spin);

	iwl_send_add_sta(priv, &station->sta, flags);
	IWL_DEBUG_RATE(priv, "SCALE sync station %d to rate %d\n",
			sta_id, tx_rate);
	return sta_id;
}

static int iwl3945_set_pwr_src(struct iwl_priv *priv, enum iwl_pwr_src src)
{
	if (src == IWL_PWR_SRC_VAUX) {
		if (pci_pme_capable(priv->pci_dev, PCI_D3cold)) {
			iwl_set_bits_mask_prph(priv, APMG_PS_CTRL_REG,
					APMG_PS_CTRL_VAL_PWR_SRC_VAUX,
					~APMG_PS_CTRL_MSK_PWR_SRC);

			iwl_poll_bit(priv, CSR_GPIO_IN,
				     CSR_GPIO_IN_VAL_VAUX_PWR_SRC,
				     CSR_GPIO_IN_BIT_AUX_POWER, 5000);
		}
	} else {
		iwl_set_bits_mask_prph(priv, APMG_PS_CTRL_REG,
				APMG_PS_CTRL_VAL_PWR_SRC_VMAIN,
				~APMG_PS_CTRL_MSK_PWR_SRC);

		iwl_poll_bit(priv, CSR_GPIO_IN, CSR_GPIO_IN_VAL_VMAIN_PWR_SRC,
			     CSR_GPIO_IN_BIT_AUX_POWER, 5000);	
	}

	return 0;
}

static int iwl3945_rx_init(struct iwl_priv *priv, struct iwl_rx_queue *rxq)
{
	iwl_write_direct32(priv, FH39_RCSR_RBD_BASE(0), rxq->dma_addr);
	iwl_write_direct32(priv, FH39_RCSR_RPTR_ADDR(0), rxq->rb_stts_dma);
	iwl_write_direct32(priv, FH39_RCSR_WPTR(0), 0);
	iwl_write_direct32(priv, FH39_RCSR_CONFIG(0),
		FH39_RCSR_RX_CONFIG_REG_VAL_DMA_CHNL_EN_ENABLE |
		FH39_RCSR_RX_CONFIG_REG_VAL_RDRBD_EN_ENABLE |
		FH39_RCSR_RX_CONFIG_REG_BIT_WR_STTS_EN |
		FH39_RCSR_RX_CONFIG_REG_VAL_MAX_FRAG_SIZE_128 |
		(RX_QUEUE_SIZE_LOG << FH39_RCSR_RX_CONFIG_REG_POS_RBDC_SIZE) |
		FH39_RCSR_RX_CONFIG_REG_VAL_IRQ_DEST_INT_HOST |
		(1 << FH39_RCSR_RX_CONFIG_REG_POS_IRQ_RBTH) |
		FH39_RCSR_RX_CONFIG_REG_VAL_MSG_MODE_FH);

	
	iwl_read_direct32(priv, FH39_RSSR_CTRL);

	return 0;
}

static int iwl3945_tx_reset(struct iwl_priv *priv)
{

	
	iwl_write_prph(priv, ALM_SCD_MODE_REG, 0x2);

	
	iwl_write_prph(priv, ALM_SCD_ARASTAT_REG, 0x01);

	
	iwl_write_prph(priv, ALM_SCD_TXFACT_REG, 0x3f);

	iwl_write_prph(priv, ALM_SCD_SBYP_MODE_1_REG, 0x010000);
	iwl_write_prph(priv, ALM_SCD_SBYP_MODE_2_REG, 0x030002);
	iwl_write_prph(priv, ALM_SCD_TXF4MF_REG, 0x000004);
	iwl_write_prph(priv, ALM_SCD_TXF5MF_REG, 0x000005);

	iwl_write_direct32(priv, FH39_TSSR_CBB_BASE,
			     priv->shared_phys);

	iwl_write_direct32(priv, FH39_TSSR_MSG_CONFIG,
		FH39_TSSR_TX_MSG_CONFIG_REG_VAL_SNOOP_RD_TXPD_ON |
		FH39_TSSR_TX_MSG_CONFIG_REG_VAL_ORDER_RD_TXPD_ON |
		FH39_TSSR_TX_MSG_CONFIG_REG_VAL_MAX_FRAG_SIZE_128B |
		FH39_TSSR_TX_MSG_CONFIG_REG_VAL_SNOOP_RD_TFD_ON |
		FH39_TSSR_TX_MSG_CONFIG_REG_VAL_ORDER_RD_CBB_ON |
		FH39_TSSR_TX_MSG_CONFIG_REG_VAL_ORDER_RSP_WAIT_TH |
		FH39_TSSR_TX_MSG_CONFIG_REG_VAL_RSP_WAIT_TH);


	return 0;
}


static int iwl3945_txq_ctx_reset(struct iwl_priv *priv)
{
	int rc;
	int txq_id, slots_num;

	iwl3945_hw_txq_ctx_free(priv);

	
	rc = iwl3945_tx_reset(priv);
	if (rc)
		goto error;

	
	for (txq_id = 0; txq_id < priv->hw_params.max_txq_num; txq_id++) {
		slots_num = (txq_id == IWL_CMD_QUEUE_NUM) ?
				TFD_CMD_SLOTS : TFD_TX_CMD_SLOTS;
		rc = iwl_tx_queue_init(priv, &priv->txq[txq_id], slots_num,
				       txq_id);
		if (rc) {
			IWL_ERR(priv, "Tx %d queue init failed\n", txq_id);
			goto error;
		}
	}

	return rc;

 error:
	iwl3945_hw_txq_ctx_free(priv);
	return rc;
}

static int iwl3945_apm_init(struct iwl_priv *priv)
{
	int ret;

	iwl_power_initialize(priv);

	iwl_set_bit(priv, CSR_GIO_CHICKEN_BITS,
			  CSR_GIO_CHICKEN_BITS_REG_BIT_DIS_L0S_EXIT_TIMER);

	
	iwl_set_bit(priv, CSR_GIO_CHICKEN_BITS,
			  CSR_GIO_CHICKEN_BITS_REG_BIT_L1A_NO_L0S_RX);

	
	iwl_set_bit(priv, CSR_GP_CNTRL, CSR_GP_CNTRL_REG_FLAG_INIT_DONE);

	ret = iwl_poll_direct_bit(priv, CSR_GP_CNTRL,
			    CSR_GP_CNTRL_REG_FLAG_MAC_CLOCK_READY, 25000);
	if (ret < 0) {
		IWL_DEBUG_INFO(priv, "Failed to init the card\n");
		goto out;
	}

	
	iwl_write_prph(priv, APMG_CLK_CTRL_REG, APMG_CLK_VAL_DMA_CLK_RQT |
						APMG_CLK_VAL_BSM_CLK_RQT);

	udelay(20);

	
	iwl_set_bits_prph(priv, APMG_PCIDEV_STT_REG,
			  APMG_PCIDEV_STT_VAL_L1_ACT_DIS);

out:
	return ret;
}

static void iwl3945_nic_config(struct iwl_priv *priv)
{
	struct iwl3945_eeprom *eeprom = (struct iwl3945_eeprom *)priv->eeprom;
	unsigned long flags;
	u8 rev_id = 0;

	spin_lock_irqsave(&priv->lock, flags);

	
	pci_read_config_byte(priv->pci_dev, PCI_REVISION_ID, &rev_id);

	IWL_DEBUG_INFO(priv, "HW Revision ID = 0x%X\n", rev_id);

	if (rev_id & PCI_CFG_REV_ID_BIT_RTP)
		IWL_DEBUG_INFO(priv, "RTP type \n");
	else if (rev_id & PCI_CFG_REV_ID_BIT_BASIC_SKU) {
		IWL_DEBUG_INFO(priv, "3945 RADIO-MB type\n");
		iwl_set_bit(priv, CSR_HW_IF_CONFIG_REG,
			    CSR39_HW_IF_CONFIG_REG_BIT_3945_MB);
	} else {
		IWL_DEBUG_INFO(priv, "3945 RADIO-MM type\n");
		iwl_set_bit(priv, CSR_HW_IF_CONFIG_REG,
			    CSR39_HW_IF_CONFIG_REG_BIT_3945_MM);
	}

	if (EEPROM_SKU_CAP_OP_MODE_MRC == eeprom->sku_cap) {
		IWL_DEBUG_INFO(priv, "SKU OP mode is mrc\n");
		iwl_set_bit(priv, CSR_HW_IF_CONFIG_REG,
			    CSR39_HW_IF_CONFIG_REG_BIT_SKU_MRC);
	} else
		IWL_DEBUG_INFO(priv, "SKU OP mode is basic\n");

	if ((eeprom->board_revision & 0xF0) == 0xD0) {
		IWL_DEBUG_INFO(priv, "3945ABG revision is 0x%X\n",
			       eeprom->board_revision);
		iwl_set_bit(priv, CSR_HW_IF_CONFIG_REG,
			    CSR39_HW_IF_CONFIG_REG_BIT_BOARD_TYPE);
	} else {
		IWL_DEBUG_INFO(priv, "3945ABG revision is 0x%X\n",
			       eeprom->board_revision);
		iwl_clear_bit(priv, CSR_HW_IF_CONFIG_REG,
			      CSR39_HW_IF_CONFIG_REG_BIT_BOARD_TYPE);
	}

	if (eeprom->almgor_m_version <= 1) {
		iwl_set_bit(priv, CSR_HW_IF_CONFIG_REG,
			    CSR39_HW_IF_CONFIG_REG_BITS_SILICON_TYPE_A);
		IWL_DEBUG_INFO(priv, "Card M type A version is 0x%X\n",
			       eeprom->almgor_m_version);
	} else {
		IWL_DEBUG_INFO(priv, "Card M type B version is 0x%X\n",
			       eeprom->almgor_m_version);
		iwl_set_bit(priv, CSR_HW_IF_CONFIG_REG,
			    CSR39_HW_IF_CONFIG_REG_BITS_SILICON_TYPE_B);
	}
	spin_unlock_irqrestore(&priv->lock, flags);

	if (eeprom->sku_cap & EEPROM_SKU_CAP_SW_RF_KILL_ENABLE)
		IWL_DEBUG_RF_KILL(priv, "SW RF KILL supported in EEPROM.\n");

	if (eeprom->sku_cap & EEPROM_SKU_CAP_HW_RF_KILL_ENABLE)
		IWL_DEBUG_RF_KILL(priv, "HW RF KILL supported in EEPROM.\n");
}

int iwl3945_hw_nic_init(struct iwl_priv *priv)
{
	int rc;
	unsigned long flags;
	struct iwl_rx_queue *rxq = &priv->rxq;

	spin_lock_irqsave(&priv->lock, flags);
	priv->cfg->ops->lib->apm_ops.init(priv);
	spin_unlock_irqrestore(&priv->lock, flags);

	rc = priv->cfg->ops->lib->apm_ops.set_pwr_src(priv, IWL_PWR_SRC_VMAIN);
	if (rc)
		return rc;

	priv->cfg->ops->lib->apm_ops.config(priv);

	
	if (!rxq->bd) {
		rc = iwl_rx_queue_alloc(priv);
		if (rc) {
			IWL_ERR(priv, "Unable to initialize Rx queue\n");
			return -ENOMEM;
		}
	} else
		iwl3945_rx_queue_reset(priv, rxq);

	iwl3945_rx_replenish(priv);

	iwl3945_rx_init(priv, rxq);


	

	iwl_write_direct32(priv, FH39_RCSR_WPTR(0), rxq->write & ~7);

	rc = iwl3945_txq_ctx_reset(priv);
	if (rc)
		return rc;

	set_bit(STATUS_INIT, &priv->status);

	return 0;
}


void iwl3945_hw_txq_ctx_free(struct iwl_priv *priv)
{
	int txq_id;

	
	for (txq_id = 0; txq_id < priv->hw_params.max_txq_num; txq_id++)
		if (txq_id == IWL_CMD_QUEUE_NUM)
			iwl_cmd_queue_free(priv);
		else
			iwl_tx_queue_free(priv, txq_id);

}

void iwl3945_hw_txq_ctx_stop(struct iwl_priv *priv)
{
	int txq_id;

	
	iwl_write_prph(priv, ALM_SCD_MODE_REG, 0);

	
	for (txq_id = 0; txq_id < priv->hw_params.max_txq_num; txq_id++) {
		iwl_write_direct32(priv, FH39_TCSR_CONFIG(txq_id), 0x0);
		iwl_poll_direct_bit(priv, FH39_TSSR_TX_STATUS,
				FH39_TSSR_TX_STATUS_REG_MSK_CHNL_IDLE(txq_id),
				1000);
	}

	iwl3945_hw_txq_ctx_free(priv);
}

static int iwl3945_apm_stop_master(struct iwl_priv *priv)
{
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&priv->lock, flags);

	
	iwl_set_bit(priv, CSR_RESET, CSR_RESET_REG_FLAG_STOP_MASTER);

	iwl_poll_direct_bit(priv, CSR_RESET,
			    CSR_RESET_REG_FLAG_MASTER_DISABLED, 100);

	if (ret < 0)
		goto out;

out:
	spin_unlock_irqrestore(&priv->lock, flags);
	IWL_DEBUG_INFO(priv, "stop master\n");

	return ret;
}

static void iwl3945_apm_stop(struct iwl_priv *priv)
{
	unsigned long flags;

	iwl3945_apm_stop_master(priv);

	spin_lock_irqsave(&priv->lock, flags);

	iwl_set_bit(priv, CSR_RESET, CSR_RESET_REG_FLAG_SW_RESET);

	udelay(10);
	
	iwl_clear_bit(priv, CSR_GP_CNTRL, CSR_GP_CNTRL_REG_FLAG_INIT_DONE);
	spin_unlock_irqrestore(&priv->lock, flags);
}

static int iwl3945_apm_reset(struct iwl_priv *priv)
{
	iwl3945_apm_stop_master(priv);


	iwl_set_bit(priv, CSR_RESET, CSR_RESET_REG_FLAG_SW_RESET);
	udelay(10);

	iwl_set_bit(priv, CSR_GP_CNTRL, CSR_GP_CNTRL_REG_FLAG_INIT_DONE);

	iwl_poll_direct_bit(priv, CSR_GP_CNTRL,
			 CSR_GP_CNTRL_REG_FLAG_MAC_CLOCK_READY, 25000);

	iwl_write_prph(priv, APMG_CLK_CTRL_REG,
				APMG_CLK_VAL_BSM_CLK_RQT);

	iwl_write_prph(priv, APMG_RTC_INT_MSK_REG, 0x0);
	iwl_write_prph(priv, APMG_RTC_INT_STT_REG,
					0xFFFFFFFF);

	
	iwl_write_prph(priv, APMG_CLK_EN_REG,
				APMG_CLK_VAL_DMA_CLK_RQT |
				APMG_CLK_VAL_BSM_CLK_RQT);
	udelay(10);

	iwl_set_bits_prph(priv, APMG_PS_CTRL_REG,
				APMG_PS_CTRL_VAL_RESET_REQ);
	udelay(5);
	iwl_clear_bits_prph(priv, APMG_PS_CTRL_REG,
				APMG_PS_CTRL_VAL_RESET_REQ);

	
	clear_bit(STATUS_HCMD_ACTIVE, &priv->status);

	wake_up_interruptible(&priv->wait_command_queue);

	return 0;
}


static int iwl3945_hw_reg_adjust_power_by_temp(int new_reading, int old_reading)
{
	return (new_reading - old_reading) * (-11) / 100;
}


static inline int iwl3945_hw_reg_temp_out_of_range(int temperature)
{
	return ((temperature < -260) || (temperature > 25)) ? 1 : 0;
}

int iwl3945_hw_get_temperature(struct iwl_priv *priv)
{
	return iwl_read32(priv, CSR_UCODE_DRV_GP2);
}


static int iwl3945_hw_reg_txpower_get_temperature(struct iwl_priv *priv)
{
	struct iwl3945_eeprom *eeprom = (struct iwl3945_eeprom *)priv->eeprom;
	int temperature;

	temperature = iwl3945_hw_get_temperature(priv);

	
	IWL_DEBUG_INFO(priv, "Temperature: %d\n", temperature + IWL_TEMP_CONVERT);

	
	if (iwl3945_hw_reg_temp_out_of_range(temperature)) {
		IWL_ERR(priv, "Error bad temperature value  %d\n", temperature);

		
		if (priv->last_temperature > 100)
			temperature = eeprom->groups[2].temperature;
		else 
			temperature = priv->last_temperature;
	}

	return temperature;	
}


#define IWL_TEMPERATURE_LIMIT_TIMER   6


static int is_temp_calib_needed(struct iwl_priv *priv)
{
	int temp_diff;

	priv->temperature = iwl3945_hw_reg_txpower_get_temperature(priv);
	temp_diff = priv->temperature - priv->last_temperature;

	
	if (temp_diff < 0) {
		IWL_DEBUG_POWER(priv, "Getting cooler, delta %d,\n", temp_diff);
		temp_diff = -temp_diff;
	} else if (temp_diff == 0)
		IWL_DEBUG_POWER(priv, "Same temp,\n");
	else
		IWL_DEBUG_POWER(priv, "Getting warmer, delta %d,\n", temp_diff);

	
	if (temp_diff < IWL_TEMPERATURE_LIMIT_TIMER) {
		IWL_DEBUG_POWER(priv, "Timed thermal calib not needed\n");
		return 0;
	}

	IWL_DEBUG_POWER(priv, "Timed thermal calib needed\n");

	
	priv->last_temperature = priv->temperature;
	return 1;
}

#define IWL_MAX_GAIN_ENTRIES 78
#define IWL_CCK_FROM_OFDM_POWER_DIFF  -5
#define IWL_CCK_FROM_OFDM_INDEX_DIFF (10)


static struct iwl3945_tx_power power_gain_table[2][IWL_MAX_GAIN_ENTRIES] = {
	{
	 {251, 127},		
	 {251, 127},
	 {251, 127},
	 {251, 127},
	 {251, 125},
	 {251, 110},
	 {251, 105},
	 {251, 98},
	 {187, 125},
	 {187, 115},
	 {187, 108},
	 {187, 99},
	 {243, 119},
	 {243, 111},
	 {243, 105},
	 {243, 97},
	 {243, 92},
	 {211, 106},
	 {211, 100},
	 {179, 120},
	 {179, 113},
	 {179, 107},
	 {147, 125},
	 {147, 119},
	 {147, 112},
	 {147, 106},
	 {147, 101},
	 {147, 97},
	 {147, 91},
	 {115, 107},
	 {235, 121},
	 {235, 115},
	 {235, 109},
	 {203, 127},
	 {203, 121},
	 {203, 115},
	 {203, 108},
	 {203, 102},
	 {203, 96},
	 {203, 92},
	 {171, 110},
	 {171, 104},
	 {171, 98},
	 {139, 116},
	 {227, 125},
	 {227, 119},
	 {227, 113},
	 {227, 107},
	 {227, 101},
	 {227, 96},
	 {195, 113},
	 {195, 106},
	 {195, 102},
	 {195, 95},
	 {163, 113},
	 {163, 106},
	 {163, 102},
	 {163, 95},
	 {131, 113},
	 {131, 106},
	 {131, 102},
	 {131, 95},
	 {99, 113},
	 {99, 106},
	 {99, 102},
	 {99, 95},
	 {67, 113},
	 {67, 106},
	 {67, 102},
	 {67, 95},
	 {35, 113},
	 {35, 106},
	 {35, 102},
	 {35, 95},
	 {3, 113},
	 {3, 106},
	 {3, 102},
	 {3, 95} },		
	{
	 {251, 127},		
	 {251, 120},
	 {251, 114},
	 {219, 119},
	 {219, 101},
	 {187, 113},
	 {187, 102},
	 {155, 114},
	 {155, 103},
	 {123, 117},
	 {123, 107},
	 {123, 99},
	 {123, 92},
	 {91, 108},
	 {59, 125},
	 {59, 118},
	 {59, 109},
	 {59, 102},
	 {59, 96},
	 {59, 90},
	 {27, 104},
	 {27, 98},
	 {27, 92},
	 {115, 118},
	 {115, 111},
	 {115, 104},
	 {83, 126},
	 {83, 121},
	 {83, 113},
	 {83, 105},
	 {83, 99},
	 {51, 118},
	 {51, 111},
	 {51, 104},
	 {51, 98},
	 {19, 116},
	 {19, 109},
	 {19, 102},
	 {19, 98},
	 {19, 93},
	 {171, 113},
	 {171, 107},
	 {171, 99},
	 {139, 120},
	 {139, 113},
	 {139, 107},
	 {139, 99},
	 {107, 120},
	 {107, 113},
	 {107, 107},
	 {107, 99},
	 {75, 120},
	 {75, 113},
	 {75, 107},
	 {75, 99},
	 {43, 120},
	 {43, 113},
	 {43, 107},
	 {43, 99},
	 {11, 120},
	 {11, 113},
	 {11, 107},
	 {11, 99},
	 {131, 107},
	 {131, 99},
	 {99, 120},
	 {99, 113},
	 {99, 107},
	 {99, 99},
	 {67, 120},
	 {67, 113},
	 {67, 107},
	 {67, 99},
	 {35, 120},
	 {35, 113},
	 {35, 107},
	 {35, 99},
	 {3, 120} }		
};

static inline u8 iwl3945_hw_reg_fix_power_index(int index)
{
	if (index < 0)
		return 0;
	if (index >= IWL_MAX_GAIN_ENTRIES)
		return IWL_MAX_GAIN_ENTRIES - 1;
	return (u8) index;
}


#define REG_RECALIB_PERIOD (60)


static void iwl3945_hw_reg_set_scan_power(struct iwl_priv *priv, u32 scan_tbl_index,
			       s32 rate_index, const s8 *clip_pwrs,
			       struct iwl_channel_info *ch_info,
			       int band_index)
{
	struct iwl3945_scan_power_info *scan_power_info;
	s8 power;
	u8 power_index;

	scan_power_info = &ch_info->scan_pwr_info[scan_tbl_index];

	
	power = min(ch_info->scan_power, clip_pwrs[IWL_RATE_6M_INDEX_TABLE]);

	
	power = min(power, priv->tx_power_user_lmt);
	scan_power_info->requested_power = power;

	
	power_index = ch_info->power_info[rate_index].power_table_index
	    - (power - ch_info->power_info
	       [IWL_RATE_6M_INDEX_TABLE].requested_power) * 2;

	

	
	power_index = iwl3945_hw_reg_fix_power_index(power_index);

	scan_power_info->power_table_index = power_index;
	scan_power_info->tpc.tx_gain =
	    power_gain_table[band_index][power_index].tx_gain;
	scan_power_info->tpc.dsp_atten =
	    power_gain_table[band_index][power_index].dsp_atten;
}


static int iwl3945_send_tx_power(struct iwl_priv *priv)
{
	int rate_idx, i;
	const struct iwl_channel_info *ch_info = NULL;
	struct iwl3945_txpowertable_cmd txpower = {
		.channel = priv->active_rxon.channel,
	};

	txpower.band = (priv->band == IEEE80211_BAND_5GHZ) ? 0 : 1;
	ch_info = iwl_get_channel_info(priv,
				       priv->band,
				       le16_to_cpu(priv->active_rxon.channel));
	if (!ch_info) {
		IWL_ERR(priv,
			"Failed to get channel info for channel %d [%d]\n",
			le16_to_cpu(priv->active_rxon.channel), priv->band);
		return -EINVAL;
	}

	if (!is_channel_valid(ch_info)) {
		IWL_DEBUG_POWER(priv, "Not calling TX_PWR_TABLE_CMD on "
				"non-Tx channel.\n");
		return 0;
	}

	
	
	for (rate_idx = IWL_FIRST_OFDM_RATE, i = 0;
	     rate_idx <= IWL39_LAST_OFDM_RATE; rate_idx++, i++) {

		txpower.power[i].tpc = ch_info->power_info[i].tpc;
		txpower.power[i].rate = iwl3945_rates[rate_idx].plcp;

		IWL_DEBUG_POWER(priv, "ch %d:%d rf %d dsp %3d rate code 0x%02x\n",
				le16_to_cpu(txpower.channel),
				txpower.band,
				txpower.power[i].tpc.tx_gain,
				txpower.power[i].tpc.dsp_atten,
				txpower.power[i].rate);
	}
	
	for (rate_idx = IWL_FIRST_CCK_RATE;
	     rate_idx <= IWL_LAST_CCK_RATE; rate_idx++, i++) {
		txpower.power[i].tpc = ch_info->power_info[i].tpc;
		txpower.power[i].rate = iwl3945_rates[rate_idx].plcp;

		IWL_DEBUG_POWER(priv, "ch %d:%d rf %d dsp %3d rate code 0x%02x\n",
				le16_to_cpu(txpower.channel),
				txpower.band,
				txpower.power[i].tpc.tx_gain,
				txpower.power[i].tpc.dsp_atten,
				txpower.power[i].rate);
	}

	return iwl_send_cmd_pdu(priv, REPLY_TX_PWR_TABLE_CMD,
				sizeof(struct iwl3945_txpowertable_cmd),
				&txpower);

}


static int iwl3945_hw_reg_set_new_power(struct iwl_priv *priv,
			     struct iwl_channel_info *ch_info)
{
	struct iwl3945_channel_power_info *power_info;
	int power_changed = 0;
	int i;
	const s8 *clip_pwrs;
	int power;

	
	clip_pwrs = priv->clip39_groups[ch_info->group_index].clip_powers;

	
	power_info = ch_info->power_info;

	
	for (i = IWL_RATE_6M_INDEX_TABLE; i <= IWL_RATE_54M_INDEX_TABLE;
	     i++, ++power_info) {
		int delta_idx;

		
		power = min(ch_info->curr_txpow, clip_pwrs[i]);
		if (power == power_info->requested_power)
			continue;

		
		delta_idx = (power - power_info->requested_power) * 2;
		power_info->base_power_index -= delta_idx;

		
		power_info->requested_power = power;

		power_changed = 1;
	}

	
	if (power_changed) {
		power =
		    ch_info->power_info[IWL_RATE_12M_INDEX_TABLE].
		    requested_power + IWL_CCK_FROM_OFDM_POWER_DIFF;

		
		for (i = IWL_RATE_1M_INDEX_TABLE; i <= IWL_RATE_11M_INDEX_TABLE; i++) {
			power_info->requested_power = power;
			power_info->base_power_index =
			    ch_info->power_info[IWL_RATE_12M_INDEX_TABLE].
			    base_power_index + IWL_CCK_FROM_OFDM_INDEX_DIFF;
			++power_info;
		}
	}

	return 0;
}


static int iwl3945_hw_reg_get_ch_txpower_limit(struct iwl_channel_info *ch_info)
{
	s8 max_power;

#if 0
	
	if (ch_info->tgd_data.max_power != 0)
		max_power = min(ch_info->tgd_data.max_power,
				ch_info->eeprom.max_power_avg);

	
	else
#endif
		max_power = ch_info->eeprom.max_power_avg;

	return min(max_power, ch_info->max_power_avg);
}


static int iwl3945_hw_reg_comp_txpower_temp(struct iwl_priv *priv)
{
	struct iwl_channel_info *ch_info = NULL;
	struct iwl3945_eeprom *eeprom = (struct iwl3945_eeprom *)priv->eeprom;
	int delta_index;
	const s8 *clip_pwrs; 
	u8 a_band;
	u8 rate_index;
	u8 scan_tbl_index;
	u8 i;
	int ref_temp;
	int temperature = priv->temperature;

	
	for (i = 0; i < priv->channel_count; i++) {
		ch_info = &priv->channel_info[i];
		a_band = is_channel_a_band(ch_info);

		
		ref_temp = (s16)eeprom->groups[ch_info->group_index].
		    temperature;

		
		delta_index = iwl3945_hw_reg_adjust_power_by_temp(temperature,
							      ref_temp);

		
		for (rate_index = 0; rate_index < IWL_RATE_COUNT;
		     rate_index++) {
			int power_idx =
			    ch_info->power_info[rate_index].base_power_index;

			
			power_idx += delta_index;

			
			power_idx = iwl3945_hw_reg_fix_power_index(power_idx);
			ch_info->power_info[rate_index].
			    power_table_index = (u8) power_idx;
			ch_info->power_info[rate_index].tpc =
			    power_gain_table[a_band][power_idx];
		}

		
		clip_pwrs = priv->clip39_groups[ch_info->group_index].clip_powers;

		
		for (scan_tbl_index = 0;
		     scan_tbl_index < IWL_NUM_SCAN_RATES; scan_tbl_index++) {
			s32 actual_index = (scan_tbl_index == 0) ?
			    IWL_RATE_1M_INDEX_TABLE : IWL_RATE_6M_INDEX_TABLE;
			iwl3945_hw_reg_set_scan_power(priv, scan_tbl_index,
					   actual_index, clip_pwrs,
					   ch_info, a_band);
		}
	}

	
	return priv->cfg->ops->lib->send_tx_power(priv);
}

int iwl3945_hw_reg_set_txpower(struct iwl_priv *priv, s8 power)
{
	struct iwl_channel_info *ch_info;
	s8 max_power;
	u8 a_band;
	u8 i;

	if (priv->tx_power_user_lmt == power) {
		IWL_DEBUG_POWER(priv, "Requested Tx power same as current "
				"limit: %ddBm.\n", power);
		return 0;
	}

	IWL_DEBUG_POWER(priv, "Setting upper limit clamp to %ddBm.\n", power);
	priv->tx_power_user_lmt = power;

	

	for (i = 0; i < priv->channel_count; i++) {
		ch_info = &priv->channel_info[i];
		a_band = is_channel_a_band(ch_info);

		
		max_power = iwl3945_hw_reg_get_ch_txpower_limit(ch_info);
		max_power = min(power, max_power);
		if (max_power != ch_info->curr_txpow) {
			ch_info->curr_txpow = max_power;

			
			iwl3945_hw_reg_set_new_power(priv, ch_info);
		}
	}

	
	is_temp_calib_needed(priv);
	iwl3945_hw_reg_comp_txpower_temp(priv);

	return 0;
}

static int iwl3945_send_rxon_assoc(struct iwl_priv *priv)
{
	int rc = 0;
	struct iwl_rx_packet *res = NULL;
	struct iwl3945_rxon_assoc_cmd rxon_assoc;
	struct iwl_host_cmd cmd = {
		.id = REPLY_RXON_ASSOC,
		.len = sizeof(rxon_assoc),
		.flags = CMD_WANT_SKB,
		.data = &rxon_assoc,
	};
	const struct iwl_rxon_cmd *rxon1 = &priv->staging_rxon;
	const struct iwl_rxon_cmd *rxon2 = &priv->active_rxon;

	if ((rxon1->flags == rxon2->flags) &&
	    (rxon1->filter_flags == rxon2->filter_flags) &&
	    (rxon1->cck_basic_rates == rxon2->cck_basic_rates) &&
	    (rxon1->ofdm_basic_rates == rxon2->ofdm_basic_rates)) {
		IWL_DEBUG_INFO(priv, "Using current RXON_ASSOC.  Not resending.\n");
		return 0;
	}

	rxon_assoc.flags = priv->staging_rxon.flags;
	rxon_assoc.filter_flags = priv->staging_rxon.filter_flags;
	rxon_assoc.ofdm_basic_rates = priv->staging_rxon.ofdm_basic_rates;
	rxon_assoc.cck_basic_rates = priv->staging_rxon.cck_basic_rates;
	rxon_assoc.reserved = 0;

	rc = iwl_send_cmd_sync(priv, &cmd);
	if (rc)
		return rc;

	res = (struct iwl_rx_packet *)cmd.reply_skb->data;
	if (res->hdr.flags & IWL_CMD_FAILED_MSK) {
		IWL_ERR(priv, "Bad return from REPLY_RXON_ASSOC command\n");
		rc = -EIO;
	}

	priv->alloc_rxb_skb--;
	dev_kfree_skb_any(cmd.reply_skb);

	return rc;
}


static int iwl3945_commit_rxon(struct iwl_priv *priv)
{
	
	struct iwl3945_rxon_cmd *active_rxon = (void *)&priv->active_rxon;
	struct iwl3945_rxon_cmd *staging_rxon = (void *)&priv->staging_rxon;
	int rc = 0;
	bool new_assoc =
		!!(priv->staging_rxon.filter_flags & RXON_FILTER_ASSOC_MSK);

	if (!iwl_is_alive(priv))
		return -1;

	
	staging_rxon->flags |= RXON_FLG_TSF2HOST_MSK;

	
	staging_rxon->flags &=
	    ~(RXON_FLG_DIS_DIV_MSK | RXON_FLG_ANT_SEL_MSK);
	staging_rxon->flags |= iwl3945_get_antenna_flags(priv);

	rc = iwl_check_rxon_cmd(priv);
	if (rc) {
		IWL_ERR(priv, "Invalid RXON configuration.  Not committing.\n");
		return -EINVAL;
	}

	
	if (!iwl_full_rxon_required(priv)) {
		rc = iwl_send_rxon_assoc(priv);
		if (rc) {
			IWL_ERR(priv, "Error setting RXON_ASSOC "
				  "configuration (%d).\n", rc);
			return rc;
		}

		memcpy(active_rxon, staging_rxon, sizeof(*active_rxon));

		return 0;
	}

	
	if (iwl_is_associated(priv) && new_assoc) {
		IWL_DEBUG_INFO(priv, "Toggling associated bit on current RXON\n");
		active_rxon->filter_flags &= ~RXON_FILTER_ASSOC_MSK;

		
		active_rxon->reserved4 = 0;
		active_rxon->reserved5 = 0;
		rc = iwl_send_cmd_pdu(priv, REPLY_RXON,
				      sizeof(struct iwl3945_rxon_cmd),
				      &priv->active_rxon);

		
		if (rc) {
			active_rxon->filter_flags |= RXON_FILTER_ASSOC_MSK;
			IWL_ERR(priv, "Error clearing ASSOC_MSK on current "
				  "configuration (%d).\n", rc);
			return rc;
		}
	}

	IWL_DEBUG_INFO(priv, "Sending RXON\n"
		       "* with%s RXON_FILTER_ASSOC_MSK\n"
		       "* channel = %d\n"
		       "* bssid = %pM\n",
		       (new_assoc ? "" : "out"),
		       le16_to_cpu(staging_rxon->channel),
		       staging_rxon->bssid_addr);

	
	staging_rxon->reserved4 = 0;
	staging_rxon->reserved5 = 0;

	iwl_set_rxon_hwcrypto(priv, !iwl3945_mod_params.sw_crypto);

	
	rc = iwl_send_cmd_pdu(priv, REPLY_RXON,
			      sizeof(struct iwl3945_rxon_cmd),
			      staging_rxon);
	if (rc) {
		IWL_ERR(priv, "Error setting new configuration (%d).\n", rc);
		return rc;
	}

	memcpy(active_rxon, staging_rxon, sizeof(*active_rxon));

	iwl_clear_stations_table(priv);

	
	rc = priv->cfg->ops->lib->send_tx_power(priv);
	if (rc) {
		IWL_ERR(priv, "Error setting Tx power (%d).\n", rc);
		return rc;
	}

	
	if (iwl_add_station(priv, iwl_bcast_addr, false, CMD_SYNC, NULL) ==
	    IWL_INVALID_STATION) {
		IWL_ERR(priv, "Error adding BROADCAST address for transmit.\n");
		return -EIO;
	}

	
	if (iwl_is_associated(priv) &&
	    (priv->iw_mode == NL80211_IFTYPE_STATION))
		if (iwl_add_station(priv, priv->active_rxon.bssid_addr,
				true, CMD_SYNC, NULL) == IWL_INVALID_STATION) {
			IWL_ERR(priv, "Error adding AP address for transmit\n");
			return -EIO;
		}

	
	rc = iwl3945_init_hw_rate_table(priv);
	if (rc) {
		IWL_ERR(priv, "Error setting HW rate table: %02X\n", rc);
		return -EIO;
	}

	return 0;
}


int iwl3945_hw_channel_switch(struct iwl_priv *priv, u16 channel)
{
	return 0;
}


void iwl3945_reg_txpower_periodic(struct iwl_priv *priv)
{
	
	if (!is_temp_calib_needed(priv))
		goto reschedule;

	
	iwl3945_hw_reg_comp_txpower_temp(priv);

 reschedule:
	queue_delayed_work(priv->workqueue,
			   &priv->thermal_periodic, REG_RECALIB_PERIOD * HZ);
}

static void iwl3945_bg_reg_txpower_periodic(struct work_struct *work)
{
	struct iwl_priv *priv = container_of(work, struct iwl_priv,
					     thermal_periodic.work);

	if (test_bit(STATUS_EXIT_PENDING, &priv->status))
		return;

	mutex_lock(&priv->mutex);
	iwl3945_reg_txpower_periodic(priv);
	mutex_unlock(&priv->mutex);
}


static u16 iwl3945_hw_reg_get_ch_grp_index(struct iwl_priv *priv,
				       const struct iwl_channel_info *ch_info)
{
	struct iwl3945_eeprom *eeprom = (struct iwl3945_eeprom *)priv->eeprom;
	struct iwl3945_eeprom_txpower_group *ch_grp = &eeprom->groups[0];
	u8 group;
	u16 group_index = 0;	
	u8 grp_channel;

	
	if (is_channel_a_band(ch_info)) {
		for (group = 1; group < 5; group++) {
			grp_channel = ch_grp[group].group_channel;
			if (ch_info->channel <= grp_channel) {
				group_index = group;
				break;
			}
		}
		
		if (group == 5)
			group_index = 4;
	} else
		group_index = 0;	

	IWL_DEBUG_POWER(priv, "Chnl %d mapped to grp %d\n", ch_info->channel,
			group_index);
	return group_index;
}


static int iwl3945_hw_reg_get_matched_power_index(struct iwl_priv *priv,
				       s8 requested_power,
				       s32 setting_index, s32 *new_index)
{
	const struct iwl3945_eeprom_txpower_group *chnl_grp = NULL;
	struct iwl3945_eeprom *eeprom = (struct iwl3945_eeprom *)priv->eeprom;
	s32 index0, index1;
	s32 power = 2 * requested_power;
	s32 i;
	const struct iwl3945_eeprom_txpower_sample *samples;
	s32 gains0, gains1;
	s32 res;
	s32 denominator;

	chnl_grp = &eeprom->groups[setting_index];
	samples = chnl_grp->samples;
	for (i = 0; i < 5; i++) {
		if (power == samples[i].power) {
			*new_index = samples[i].gain_index;
			return 0;
		}
	}

	if (power > samples[1].power) {
		index0 = 0;
		index1 = 1;
	} else if (power > samples[2].power) {
		index0 = 1;
		index1 = 2;
	} else if (power > samples[3].power) {
		index0 = 2;
		index1 = 3;
	} else {
		index0 = 3;
		index1 = 4;
	}

	denominator = (s32) samples[index1].power - (s32) samples[index0].power;
	if (denominator == 0)
		return -EINVAL;
	gains0 = (s32) samples[index0].gain_index * (1 << 19);
	gains1 = (s32) samples[index1].gain_index * (1 << 19);
	res = gains0 + (gains1 - gains0) *
	    ((s32) power - (s32) samples[index0].power) / denominator +
	    (1 << 18);
	*new_index = res >> 19;
	return 0;
}

static void iwl3945_hw_reg_init_channel_groups(struct iwl_priv *priv)
{
	u32 i;
	s32 rate_index;
	struct iwl3945_eeprom *eeprom = (struct iwl3945_eeprom *)priv->eeprom;
	const struct iwl3945_eeprom_txpower_group *group;

	IWL_DEBUG_POWER(priv, "Initializing factory calib info from EEPROM\n");

	for (i = 0; i < IWL_NUM_TX_CALIB_GROUPS; i++) {
		s8 *clip_pwrs;	
		s8 satur_pwr;	
		group = &eeprom->groups[i];

		
		if (group->saturation_power < 40) {
			IWL_WARN(priv, "Error: saturation power is %d, "
				    "less than minimum expected 40\n",
				    group->saturation_power);
			return;
		}

		
		
		clip_pwrs = (s8 *) priv->clip39_groups[i].clip_powers;

		
		satur_pwr = (s8) (group->saturation_power >> 1);

		
		for (rate_index = 0;
		     rate_index < IWL_RATE_COUNT; rate_index++, clip_pwrs++) {
			switch (rate_index) {
			case IWL_RATE_36M_INDEX_TABLE:
				if (i == 0)	
					*clip_pwrs = satur_pwr;
				else	
					*clip_pwrs = satur_pwr - 5;
				break;
			case IWL_RATE_48M_INDEX_TABLE:
				if (i == 0)
					*clip_pwrs = satur_pwr - 7;
				else
					*clip_pwrs = satur_pwr - 10;
				break;
			case IWL_RATE_54M_INDEX_TABLE:
				if (i == 0)
					*clip_pwrs = satur_pwr - 9;
				else
					*clip_pwrs = satur_pwr - 12;
				break;
			default:
				*clip_pwrs = satur_pwr;
				break;
			}
		}
	}
}


int iwl3945_txpower_set_from_eeprom(struct iwl_priv *priv)
{
	struct iwl_channel_info *ch_info = NULL;
	struct iwl3945_channel_power_info *pwr_info;
	struct iwl3945_eeprom *eeprom = (struct iwl3945_eeprom *)priv->eeprom;
	int delta_index;
	u8 rate_index;
	u8 scan_tbl_index;
	const s8 *clip_pwrs;	
	u8 gain, dsp_atten;
	s8 power;
	u8 pwr_index, base_pwr_index, a_band;
	u8 i;
	int temperature;

	
	temperature = iwl3945_hw_reg_txpower_get_temperature(priv);
	priv->last_temperature = temperature;

	iwl3945_hw_reg_init_channel_groups(priv);

	
	for (i = 0, ch_info = priv->channel_info; i < priv->channel_count;
	     i++, ch_info++) {
		a_band = is_channel_a_band(ch_info);
		if (!is_channel_valid(ch_info))
			continue;

		
		ch_info->group_index =
			iwl3945_hw_reg_get_ch_grp_index(priv, ch_info);

		
		clip_pwrs = priv->clip39_groups[ch_info->group_index].clip_powers;

		
		delta_index = iwl3945_hw_reg_adjust_power_by_temp(temperature,
				eeprom->groups[ch_info->group_index].
				temperature);

		IWL_DEBUG_POWER(priv, "Delta index for channel %d: %d [%d]\n",
				ch_info->channel, delta_index, temperature +
				IWL_TEMP_CONVERT);

		
		for (rate_index = 0; rate_index < IWL_OFDM_RATES;
		     rate_index++) {
			s32 uninitialized_var(power_idx);
			int rc;

			
			s8 pwr = min(ch_info->max_power_avg,
				     clip_pwrs[rate_index]);

			pwr_info = &ch_info->power_info[rate_index];

			
			rc = iwl3945_hw_reg_get_matched_power_index(priv, pwr,
							 ch_info->group_index,
							 &power_idx);
			if (rc) {
				IWL_ERR(priv, "Invalid power index\n");
				return rc;
			}
			pwr_info->base_power_index = (u8) power_idx;

			
			power_idx += delta_index;

			
			power_idx = iwl3945_hw_reg_fix_power_index(power_idx);

			
			pwr_info->requested_power = pwr;
			pwr_info->power_table_index = (u8) power_idx;
			pwr_info->tpc.tx_gain =
			    power_gain_table[a_band][power_idx].tx_gain;
			pwr_info->tpc.dsp_atten =
			    power_gain_table[a_band][power_idx].dsp_atten;
		}

		
		pwr_info = &ch_info->power_info[IWL_RATE_12M_INDEX_TABLE];
		power = pwr_info->requested_power +
			IWL_CCK_FROM_OFDM_POWER_DIFF;
		pwr_index = pwr_info->power_table_index +
			IWL_CCK_FROM_OFDM_INDEX_DIFF;
		base_pwr_index = pwr_info->base_power_index +
			IWL_CCK_FROM_OFDM_INDEX_DIFF;

		
		pwr_index = iwl3945_hw_reg_fix_power_index(pwr_index);
		gain = power_gain_table[a_band][pwr_index].tx_gain;
		dsp_atten = power_gain_table[a_band][pwr_index].dsp_atten;

		
		for (rate_index = 0;
		     rate_index < IWL_CCK_RATES; rate_index++) {
			pwr_info = &ch_info->power_info[rate_index+IWL_OFDM_RATES];
			pwr_info->requested_power = power;
			pwr_info->power_table_index = pwr_index;
			pwr_info->base_power_index = base_pwr_index;
			pwr_info->tpc.tx_gain = gain;
			pwr_info->tpc.dsp_atten = dsp_atten;
		}

		
		for (scan_tbl_index = 0;
		     scan_tbl_index < IWL_NUM_SCAN_RATES; scan_tbl_index++) {
			s32 actual_index = (scan_tbl_index == 0) ?
				IWL_RATE_1M_INDEX_TABLE : IWL_RATE_6M_INDEX_TABLE;
			iwl3945_hw_reg_set_scan_power(priv, scan_tbl_index,
				actual_index, clip_pwrs, ch_info, a_band);
		}
	}

	return 0;
}

int iwl3945_hw_rxq_stop(struct iwl_priv *priv)
{
	int rc;

	iwl_write_direct32(priv, FH39_RCSR_CONFIG(0), 0);
	rc = iwl_poll_direct_bit(priv, FH39_RSSR_STATUS,
			FH39_RSSR_CHNL0_RX_STATUS_CHNL_IDLE, 1000);
	if (rc < 0)
		IWL_ERR(priv, "Can't stop Rx DMA.\n");

	return 0;
}

int iwl3945_hw_tx_queue_init(struct iwl_priv *priv, struct iwl_tx_queue *txq)
{
	int txq_id = txq->q.id;

	struct iwl3945_shared *shared_data = priv->shared_virt;

	shared_data->tx_base_ptr[txq_id] = cpu_to_le32((u32)txq->q.dma_addr);

	iwl_write_direct32(priv, FH39_CBCC_CTRL(txq_id), 0);
	iwl_write_direct32(priv, FH39_CBCC_BASE(txq_id), 0);

	iwl_write_direct32(priv, FH39_TCSR_CONFIG(txq_id),
		FH39_TCSR_TX_CONFIG_REG_VAL_CIRQ_RTC_NOINT |
		FH39_TCSR_TX_CONFIG_REG_VAL_MSG_MODE_TXF |
		FH39_TCSR_TX_CONFIG_REG_VAL_CIRQ_HOST_IFTFD |
		FH39_TCSR_TX_CONFIG_REG_VAL_DMA_CREDIT_ENABLE_VAL |
		FH39_TCSR_TX_CONFIG_REG_VAL_DMA_CHNL_ENABLE);

	
	iwl_read32(priv, FH39_TSSR_CBB_BASE);

	return 0;
}


static u16 iwl3945_get_hcmd_size(u8 cmd_id, u16 len)
{
	switch (cmd_id) {
	case REPLY_RXON:
		return sizeof(struct iwl3945_rxon_cmd);
	case POWER_TABLE_CMD:
		return sizeof(struct iwl3945_powertable_cmd);
	default:
		return len;
	}
}


static u16 iwl3945_build_addsta_hcmd(const struct iwl_addsta_cmd *cmd, u8 *data)
{
	struct iwl3945_addsta_cmd *addsta = (struct iwl3945_addsta_cmd *)data;
	addsta->mode = cmd->mode;
	memcpy(&addsta->sta, &cmd->sta, sizeof(struct sta_id_modify));
	memcpy(&addsta->key, &cmd->key, sizeof(struct iwl4965_keyinfo));
	addsta->station_flags = cmd->station_flags;
	addsta->station_flags_msk = cmd->station_flags_msk;
	addsta->tid_disable_tx = cpu_to_le16(0);
	addsta->rate_n_flags = cmd->rate_n_flags;
	addsta->add_immediate_ba_tid = cmd->add_immediate_ba_tid;
	addsta->remove_immediate_ba_tid = cmd->remove_immediate_ba_tid;
	addsta->add_immediate_ba_ssn = cmd->add_immediate_ba_ssn;

	return (u16)sizeof(struct iwl3945_addsta_cmd);
}



int iwl3945_init_hw_rate_table(struct iwl_priv *priv)
{
	int rc, i, index, prev_index;
	struct iwl3945_rate_scaling_cmd rate_cmd = {
		.reserved = {0, 0, 0},
	};
	struct iwl3945_rate_scaling_info *table = rate_cmd.table;

	for (i = 0; i < ARRAY_SIZE(iwl3945_rates); i++) {
		index = iwl3945_rates[i].table_rs_index;

		table[index].rate_n_flags =
			iwl3945_hw_set_rate_n_flags(iwl3945_rates[i].plcp, 0);
		table[index].try_cnt = priv->retry_rate;
		prev_index = iwl3945_get_prev_ieee_rate(i);
		table[index].next_rate_index =
				iwl3945_rates[prev_index].table_rs_index;
	}

	switch (priv->band) {
	case IEEE80211_BAND_5GHZ:
		IWL_DEBUG_RATE(priv, "Select A mode rate scale\n");
		
		for (i = IWL_RATE_1M_INDEX_TABLE;
			i <= IWL_RATE_11M_INDEX_TABLE; i++)
			table[i].next_rate_index =
			  iwl3945_rates[IWL_FIRST_OFDM_RATE].table_rs_index;

		
		table[IWL_RATE_12M_INDEX_TABLE].next_rate_index =
						IWL_RATE_9M_INDEX_TABLE;

		
		table[IWL_RATE_6M_INDEX_TABLE].next_rate_index =
		    iwl3945_rates[IWL_FIRST_OFDM_RATE].table_rs_index;
		break;

	case IEEE80211_BAND_2GHZ:
		IWL_DEBUG_RATE(priv, "Select B/G mode rate scale\n");
		

		if (!(priv->sta_supp_rates & IWL_OFDM_RATES_MASK) &&
		    iwl_is_associated(priv)) {

			index = IWL_FIRST_CCK_RATE;
			for (i = IWL_RATE_6M_INDEX_TABLE;
			     i <= IWL_RATE_54M_INDEX_TABLE; i++)
				table[i].next_rate_index =
					iwl3945_rates[index].table_rs_index;

			index = IWL_RATE_11M_INDEX_TABLE;
			
			table[index].next_rate_index = IWL_RATE_5M_INDEX_TABLE;
		}
		break;

	default:
		WARN_ON(1);
		break;
	}

	
	rate_cmd.table_id = 0;
	rc = iwl_send_cmd_pdu(priv, REPLY_RATE_SCALE, sizeof(rate_cmd),
			      &rate_cmd);
	if (rc)
		return rc;

	
	rate_cmd.table_id = 1;
	return iwl_send_cmd_pdu(priv, REPLY_RATE_SCALE, sizeof(rate_cmd),
				&rate_cmd);
}


int iwl3945_hw_set_hw_params(struct iwl_priv *priv)
{
	memset((void *)&priv->hw_params, 0,
	       sizeof(struct iwl_hw_params));

	priv->shared_virt =
	    pci_alloc_consistent(priv->pci_dev,
				 sizeof(struct iwl3945_shared),
				 &priv->shared_phys);

	if (!priv->shared_virt) {
		IWL_ERR(priv, "failed to allocate pci memory\n");
		mutex_unlock(&priv->mutex);
		return -ENOMEM;
	}

	
	priv->hw_params.max_txq_num = IWL39_NUM_QUEUES;

	priv->hw_params.tfd_size = sizeof(struct iwl3945_tfd);
	priv->hw_params.rx_buf_size = IWL_RX_BUF_SIZE_3K;
	priv->hw_params.max_pkt_size = 2342;
	priv->hw_params.max_rxq_size = RX_QUEUE_SIZE;
	priv->hw_params.max_rxq_log = RX_QUEUE_SIZE_LOG;
	priv->hw_params.max_stations = IWL3945_STATION_COUNT;
	priv->hw_params.bcast_sta_id = IWL3945_BROADCAST_ID;

	priv->hw_params.rx_wrt_ptr_reg = FH39_RSCSR_CHNL0_WPTR;
	priv->hw_params.max_beacon_itrvl = IWL39_MAX_UCODE_BEACON_INTERVAL;

	return 0;
}

unsigned int iwl3945_hw_get_beacon_cmd(struct iwl_priv *priv,
			  struct iwl3945_frame *frame, u8 rate)
{
	struct iwl3945_tx_beacon_cmd *tx_beacon_cmd;
	unsigned int frame_size;

	tx_beacon_cmd = (struct iwl3945_tx_beacon_cmd *)&frame->u;
	memset(tx_beacon_cmd, 0, sizeof(*tx_beacon_cmd));

	tx_beacon_cmd->tx.sta_id = priv->hw_params.bcast_sta_id;
	tx_beacon_cmd->tx.stop_time.life_time = TX_CMD_LIFE_TIME_INFINITE;

	frame_size = iwl3945_fill_beacon_frame(priv,
				tx_beacon_cmd->frame,
				sizeof(frame->u) - sizeof(*tx_beacon_cmd));

	BUG_ON(frame_size > MAX_MPDU_SIZE);
	tx_beacon_cmd->tx.len = cpu_to_le16((u16)frame_size);

	tx_beacon_cmd->tx.rate = rate;
	tx_beacon_cmd->tx.tx_flags = (TX_CMD_FLG_SEQ_CTL_MSK |
				      TX_CMD_FLG_TSF_MSK);

	
	tx_beacon_cmd->tx.supp_rates[0] =
		(IWL_OFDM_BASIC_RATES_MASK >> IWL_FIRST_OFDM_RATE) & 0xFF;

	tx_beacon_cmd->tx.supp_rates[1] =
		(IWL_CCK_BASIC_RATES_MASK & 0xF);

	return sizeof(struct iwl3945_tx_beacon_cmd) + frame_size;
}

void iwl3945_hw_rx_handler_setup(struct iwl_priv *priv)
{
	priv->rx_handlers[REPLY_TX] = iwl3945_rx_reply_tx;
	priv->rx_handlers[REPLY_3945_RX] = iwl3945_rx_reply_rx;
}

void iwl3945_hw_setup_deferred_work(struct iwl_priv *priv)
{
	INIT_DELAYED_WORK(&priv->thermal_periodic,
			  iwl3945_bg_reg_txpower_periodic);
}

void iwl3945_hw_cancel_deferred_work(struct iwl_priv *priv)
{
	cancel_delayed_work(&priv->thermal_periodic);
}


static int iwl3945_verify_bsm(struct iwl_priv *priv)
 {
	__le32 *image = priv->ucode_boot.v_addr;
	u32 len = priv->ucode_boot.len;
	u32 reg;
	u32 val;

	IWL_DEBUG_INFO(priv, "Begin verify bsm\n");

	
	val = iwl_read_prph(priv, BSM_WR_DWCOUNT_REG);
	for (reg = BSM_SRAM_LOWER_BOUND;
	     reg < BSM_SRAM_LOWER_BOUND + len;
	     reg += sizeof(u32), image++) {
		val = iwl_read_prph(priv, reg);
		if (val != le32_to_cpu(*image)) {
			IWL_ERR(priv, "BSM uCode verification failed at "
				  "addr 0x%08X+%u (of %u), is 0x%x, s/b 0x%x\n",
				  BSM_SRAM_LOWER_BOUND,
				  reg - BSM_SRAM_LOWER_BOUND, len,
				  val, le32_to_cpu(*image));
			return -EIO;
		}
	}

	IWL_DEBUG_INFO(priv, "BSM bootstrap uCode image OK\n");

	return 0;
}





static int iwl3945_eeprom_acquire_semaphore(struct iwl_priv *priv)
{
	_iwl_clear_bit(priv, CSR_EEPROM_GP, CSR_EEPROM_GP_IF_OWNER_MSK);
	return 0;
}


static void iwl3945_eeprom_release_semaphore(struct iwl_priv *priv)
{
	return;
}

 
static int iwl3945_load_bsm(struct iwl_priv *priv)
{
	__le32 *image = priv->ucode_boot.v_addr;
	u32 len = priv->ucode_boot.len;
	dma_addr_t pinst;
	dma_addr_t pdata;
	u32 inst_len;
	u32 data_len;
	int rc;
	int i;
	u32 done;
	u32 reg_offset;

	IWL_DEBUG_INFO(priv, "Begin load bsm\n");

	
	if (len > IWL39_MAX_BSM_SIZE)
		return -EINVAL;

	
	pinst = priv->ucode_init.p_addr;
	pdata = priv->ucode_init_data.p_addr;
	inst_len = priv->ucode_init.len;
	data_len = priv->ucode_init_data.len;

	iwl_write_prph(priv, BSM_DRAM_INST_PTR_REG, pinst);
	iwl_write_prph(priv, BSM_DRAM_DATA_PTR_REG, pdata);
	iwl_write_prph(priv, BSM_DRAM_INST_BYTECOUNT_REG, inst_len);
	iwl_write_prph(priv, BSM_DRAM_DATA_BYTECOUNT_REG, data_len);

	
	for (reg_offset = BSM_SRAM_LOWER_BOUND;
	     reg_offset < BSM_SRAM_LOWER_BOUND + len;
	     reg_offset += sizeof(u32), image++)
		_iwl_write_prph(priv, reg_offset,
					  le32_to_cpu(*image));

	rc = iwl3945_verify_bsm(priv);
	if (rc)
		return rc;

	
	iwl_write_prph(priv, BSM_WR_MEM_SRC_REG, 0x0);
	iwl_write_prph(priv, BSM_WR_MEM_DST_REG,
				 IWL39_RTC_INST_LOWER_BOUND);
	iwl_write_prph(priv, BSM_WR_DWCOUNT_REG, len / sizeof(u32));

	
	iwl_write_prph(priv, BSM_WR_CTRL_REG,
		BSM_WR_CTRL_REG_BIT_START);

	
	for (i = 0; i < 100; i++) {
		done = iwl_read_prph(priv, BSM_WR_CTRL_REG);
		if (!(done & BSM_WR_CTRL_REG_BIT_START))
			break;
		udelay(10);
	}
	if (i < 100)
		IWL_DEBUG_INFO(priv, "BSM write complete, poll %d iterations\n", i);
	else {
		IWL_ERR(priv, "BSM write did not complete!\n");
		return -EIO;
	}

	
	iwl_write_prph(priv, BSM_WR_CTRL_REG,
		BSM_WR_CTRL_REG_BIT_START_EN);

	return 0;
}

#define IWL3945_UCODE_GET(item)						\
static u32 iwl3945_ucode_get_##item(const struct iwl_ucode_header *ucode,\
				    u32 api_ver)			\
{									\
	return le32_to_cpu(ucode->u.v1.item);				\
}

static u32 iwl3945_ucode_get_header_size(u32 api_ver)
{
	return UCODE_HEADER_SIZE(1);
}
static u32 iwl3945_ucode_get_build(const struct iwl_ucode_header *ucode,
				   u32 api_ver)
{
	return 0;
}
static u8 *iwl3945_ucode_get_data(const struct iwl_ucode_header *ucode,
				  u32 api_ver)
{
	return (u8 *) ucode->u.v1.data;
}

IWL3945_UCODE_GET(inst_size);
IWL3945_UCODE_GET(data_size);
IWL3945_UCODE_GET(init_size);
IWL3945_UCODE_GET(init_data_size);
IWL3945_UCODE_GET(boot_size);

static struct iwl_hcmd_ops iwl3945_hcmd = {
	.rxon_assoc = iwl3945_send_rxon_assoc,
	.commit_rxon = iwl3945_commit_rxon,
};

static struct iwl_ucode_ops iwl3945_ucode = {
	.get_header_size = iwl3945_ucode_get_header_size,
	.get_build = iwl3945_ucode_get_build,
	.get_inst_size = iwl3945_ucode_get_inst_size,
	.get_data_size = iwl3945_ucode_get_data_size,
	.get_init_size = iwl3945_ucode_get_init_size,
	.get_init_data_size = iwl3945_ucode_get_init_data_size,
	.get_boot_size = iwl3945_ucode_get_boot_size,
	.get_data = iwl3945_ucode_get_data,
};

static struct iwl_lib_ops iwl3945_lib = {
	.txq_attach_buf_to_tfd = iwl3945_hw_txq_attach_buf_to_tfd,
	.txq_free_tfd = iwl3945_hw_txq_free_tfd,
	.txq_init = iwl3945_hw_tx_queue_init,
	.load_ucode = iwl3945_load_bsm,
	.dump_nic_event_log = iwl3945_dump_nic_event_log,
	.dump_nic_error_log = iwl3945_dump_nic_error_log,
	.apm_ops = {
		.init = iwl3945_apm_init,
		.reset = iwl3945_apm_reset,
		.stop = iwl3945_apm_stop,
		.config = iwl3945_nic_config,
		.set_pwr_src = iwl3945_set_pwr_src,
	},
	.eeprom_ops = {
		.regulatory_bands = {
			EEPROM_REGULATORY_BAND_1_CHANNELS,
			EEPROM_REGULATORY_BAND_2_CHANNELS,
			EEPROM_REGULATORY_BAND_3_CHANNELS,
			EEPROM_REGULATORY_BAND_4_CHANNELS,
			EEPROM_REGULATORY_BAND_5_CHANNELS,
			EEPROM_REGULATORY_BAND_NO_HT40,
			EEPROM_REGULATORY_BAND_NO_HT40,
		},
		.verify_signature  = iwlcore_eeprom_verify_signature,
		.acquire_semaphore = iwl3945_eeprom_acquire_semaphore,
		.release_semaphore = iwl3945_eeprom_release_semaphore,
		.query_addr = iwlcore_eeprom_query_addr,
	},
	.send_tx_power	= iwl3945_send_tx_power,
	.is_valid_rtc_data_addr = iwl3945_hw_valid_rtc_data_addr,
	.post_associate = iwl3945_post_associate,
	.isr = iwl_isr_legacy,
	.config_ap = iwl3945_config_ap,
};

static struct iwl_hcmd_utils_ops iwl3945_hcmd_utils = {
	.get_hcmd_size = iwl3945_get_hcmd_size,
	.build_addsta_hcmd = iwl3945_build_addsta_hcmd,
};

static struct iwl_ops iwl3945_ops = {
	.ucode = &iwl3945_ucode,
	.lib = &iwl3945_lib,
	.hcmd = &iwl3945_hcmd,
	.utils = &iwl3945_hcmd_utils,
};

static struct iwl_cfg iwl3945_bg_cfg = {
	.name = "3945BG",
	.fw_name_pre = IWL3945_FW_PRE,
	.ucode_api_max = IWL3945_UCODE_API_MAX,
	.ucode_api_min = IWL3945_UCODE_API_MIN,
	.sku = IWL_SKU_G,
	.eeprom_size = IWL3945_EEPROM_IMG_SIZE,
	.eeprom_ver = EEPROM_3945_EEPROM_VERSION,
	.ops = &iwl3945_ops,
	.mod_params = &iwl3945_mod_params,
	.use_isr_legacy = true,
	.ht_greenfield_support = false,
	.broken_powersave = true,
};

static struct iwl_cfg iwl3945_abg_cfg = {
	.name = "3945ABG",
	.fw_name_pre = IWL3945_FW_PRE,
	.ucode_api_max = IWL3945_UCODE_API_MAX,
	.ucode_api_min = IWL3945_UCODE_API_MIN,
	.sku = IWL_SKU_A|IWL_SKU_G,
	.eeprom_size = IWL3945_EEPROM_IMG_SIZE,
	.eeprom_ver = EEPROM_3945_EEPROM_VERSION,
	.ops = &iwl3945_ops,
	.mod_params = &iwl3945_mod_params,
	.use_isr_legacy = true,
	.ht_greenfield_support = false,
	.broken_powersave = true,
};

struct pci_device_id iwl3945_hw_card_ids[] = {
	{IWL_PCI_DEVICE(0x4222, 0x1005, iwl3945_bg_cfg)},
	{IWL_PCI_DEVICE(0x4222, 0x1034, iwl3945_bg_cfg)},
	{IWL_PCI_DEVICE(0x4222, 0x1044, iwl3945_bg_cfg)},
	{IWL_PCI_DEVICE(0x4227, 0x1014, iwl3945_bg_cfg)},
	{IWL_PCI_DEVICE(0x4222, PCI_ANY_ID, iwl3945_abg_cfg)},
	{IWL_PCI_DEVICE(0x4227, PCI_ANY_ID, iwl3945_abg_cfg)},
	{0}
};

MODULE_DEVICE_TABLE(pci, iwl3945_hw_card_ids);
