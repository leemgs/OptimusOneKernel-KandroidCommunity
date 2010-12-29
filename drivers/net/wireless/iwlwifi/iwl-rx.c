

#include <linux/etherdevice.h>
#include <net/mac80211.h>
#include <asm/unaligned.h>
#include "iwl-eeprom.h"
#include "iwl-dev.h"
#include "iwl-core.h"
#include "iwl-sta.h"
#include "iwl-io.h"
#include "iwl-calib.h"
#include "iwl-helpers.h"




int iwl_rx_queue_space(const struct iwl_rx_queue *q)
{
	int s = q->read - q->write;
	if (s <= 0)
		s += RX_QUEUE_SIZE;
	
	s -= 2;
	if (s < 0)
		s = 0;
	return s;
}
EXPORT_SYMBOL(iwl_rx_queue_space);


int iwl_rx_queue_update_write_ptr(struct iwl_priv *priv, struct iwl_rx_queue *q)
{
	unsigned long flags;
	u32 rx_wrt_ptr_reg = priv->hw_params.rx_wrt_ptr_reg;
	u32 reg;
	int ret = 0;

	spin_lock_irqsave(&q->lock, flags);

	if (q->need_update == 0)
		goto exit_unlock;

	
	if (test_bit(STATUS_POWER_PMI, &priv->status)) {
		reg = iwl_read32(priv, CSR_UCODE_DRV_GP1);

		if (reg & CSR_UCODE_DRV_GP1_BIT_MAC_SLEEP) {
			iwl_set_bit(priv, CSR_GP_CNTRL,
				    CSR_GP_CNTRL_REG_FLAG_MAC_ACCESS_REQ);
			goto exit_unlock;
		}

		q->write_actual = (q->write & ~0x7);
		iwl_write_direct32(priv, rx_wrt_ptr_reg, q->write_actual);

	
	} else {
		
		q->write_actual = (q->write & ~0x7);
		iwl_write_direct32(priv, rx_wrt_ptr_reg, q->write_actual);
	}

	q->need_update = 0;

 exit_unlock:
	spin_unlock_irqrestore(&q->lock, flags);
	return ret;
}
EXPORT_SYMBOL(iwl_rx_queue_update_write_ptr);

static inline __le32 iwl_dma_addr2rbd_ptr(struct iwl_priv *priv,
					  dma_addr_t dma_addr)
{
	return cpu_to_le32((u32)(dma_addr >> 8));
}


int iwl_rx_queue_restock(struct iwl_priv *priv)
{
	struct iwl_rx_queue *rxq = &priv->rxq;
	struct list_head *element;
	struct iwl_rx_mem_buffer *rxb;
	unsigned long flags;
	int write;
	int ret = 0;

	spin_lock_irqsave(&rxq->lock, flags);
	write = rxq->write & ~0x7;
	while ((iwl_rx_queue_space(rxq) > 0) && (rxq->free_count)) {
		
		element = rxq->rx_free.next;
		rxb = list_entry(element, struct iwl_rx_mem_buffer, list);
		list_del(element);

		
		rxq->bd[rxq->write] = iwl_dma_addr2rbd_ptr(priv, rxb->aligned_dma_addr);
		rxq->queue[rxq->write] = rxb;
		rxq->write = (rxq->write + 1) & RX_QUEUE_MASK;
		rxq->free_count--;
	}
	spin_unlock_irqrestore(&rxq->lock, flags);
	
	if (rxq->free_count <= RX_LOW_WATERMARK)
		queue_work(priv->workqueue, &priv->rx_replenish);


	
	if (rxq->write_actual != (rxq->write & ~0x7)) {
		spin_lock_irqsave(&rxq->lock, flags);
		rxq->need_update = 1;
		spin_unlock_irqrestore(&rxq->lock, flags);
		ret = iwl_rx_queue_update_write_ptr(priv, rxq);
	}

	return ret;
}
EXPORT_SYMBOL(iwl_rx_queue_restock);



void iwl_rx_allocate(struct iwl_priv *priv, gfp_t priority)
{
	struct iwl_rx_queue *rxq = &priv->rxq;
	struct list_head *element;
	struct iwl_rx_mem_buffer *rxb;
	struct sk_buff *skb;
	unsigned long flags;

	while (1) {
		spin_lock_irqsave(&rxq->lock, flags);
		if (list_empty(&rxq->rx_used)) {
			spin_unlock_irqrestore(&rxq->lock, flags);
			return;
		}
		spin_unlock_irqrestore(&rxq->lock, flags);

		if (rxq->free_count > RX_LOW_WATERMARK)
			priority |= __GFP_NOWARN;
		
		skb = alloc_skb(priv->hw_params.rx_buf_size + 256,
						priority);

		if (!skb) {
			if (net_ratelimit())
				IWL_DEBUG_INFO(priv, "Failed to allocate SKB buffer.\n");
			if ((rxq->free_count <= RX_LOW_WATERMARK) &&
			    net_ratelimit())
				IWL_CRIT(priv, "Failed to allocate SKB buffer with %s. Only %u free buffers remaining.\n",
					 priority == GFP_ATOMIC ?  "GFP_ATOMIC" : "GFP_KERNEL",
					 rxq->free_count);
			
			break;
		}

		spin_lock_irqsave(&rxq->lock, flags);

		if (list_empty(&rxq->rx_used)) {
			spin_unlock_irqrestore(&rxq->lock, flags);
			dev_kfree_skb_any(skb);
			return;
		}
		element = rxq->rx_used.next;
		rxb = list_entry(element, struct iwl_rx_mem_buffer, list);
		list_del(element);

		spin_unlock_irqrestore(&rxq->lock, flags);

		rxb->skb = skb;
		
		rxb->real_dma_addr = pci_map_single(
					priv->pci_dev,
					rxb->skb->data,
					priv->hw_params.rx_buf_size + 256,
					PCI_DMA_FROMDEVICE);
		
		BUG_ON(rxb->real_dma_addr & ~DMA_BIT_MASK(36));
		
		rxb->aligned_dma_addr = ALIGN(rxb->real_dma_addr, 256);
		skb_reserve(rxb->skb, rxb->aligned_dma_addr - rxb->real_dma_addr);

		spin_lock_irqsave(&rxq->lock, flags);

		list_add_tail(&rxb->list, &rxq->rx_free);
		rxq->free_count++;
		priv->alloc_rxb_skb++;

		spin_unlock_irqrestore(&rxq->lock, flags);
	}
}

void iwl_rx_replenish(struct iwl_priv *priv)
{
	unsigned long flags;

	iwl_rx_allocate(priv, GFP_KERNEL);

	spin_lock_irqsave(&priv->lock, flags);
	iwl_rx_queue_restock(priv);
	spin_unlock_irqrestore(&priv->lock, flags);
}
EXPORT_SYMBOL(iwl_rx_replenish);

void iwl_rx_replenish_now(struct iwl_priv *priv)
{
	iwl_rx_allocate(priv, GFP_ATOMIC);

	iwl_rx_queue_restock(priv);
}
EXPORT_SYMBOL(iwl_rx_replenish_now);



void iwl_rx_queue_free(struct iwl_priv *priv, struct iwl_rx_queue *rxq)
{
	int i;
	for (i = 0; i < RX_QUEUE_SIZE + RX_FREE_BUFFERS; i++) {
		if (rxq->pool[i].skb != NULL) {
			pci_unmap_single(priv->pci_dev,
					 rxq->pool[i].real_dma_addr,
					 priv->hw_params.rx_buf_size + 256,
					 PCI_DMA_FROMDEVICE);
			dev_kfree_skb(rxq->pool[i].skb);
		}
	}

	pci_free_consistent(priv->pci_dev, 4 * RX_QUEUE_SIZE, rxq->bd,
			    rxq->dma_addr);
	pci_free_consistent(priv->pci_dev, sizeof(struct iwl_rb_status),
			    rxq->rb_stts, rxq->rb_stts_dma);
	rxq->bd = NULL;
	rxq->rb_stts  = NULL;
}
EXPORT_SYMBOL(iwl_rx_queue_free);

int iwl_rx_queue_alloc(struct iwl_priv *priv)
{
	struct iwl_rx_queue *rxq = &priv->rxq;
	struct pci_dev *dev = priv->pci_dev;
	int i;

	spin_lock_init(&rxq->lock);
	INIT_LIST_HEAD(&rxq->rx_free);
	INIT_LIST_HEAD(&rxq->rx_used);

	
	rxq->bd = pci_alloc_consistent(dev, 4 * RX_QUEUE_SIZE, &rxq->dma_addr);
	if (!rxq->bd)
		goto err_bd;

	rxq->rb_stts = pci_alloc_consistent(dev, sizeof(struct iwl_rb_status),
					&rxq->rb_stts_dma);
	if (!rxq->rb_stts)
		goto err_rb;

	
	for (i = 0; i < RX_FREE_BUFFERS + RX_QUEUE_SIZE; i++)
		list_add_tail(&rxq->pool[i].list, &rxq->rx_used);

	
	rxq->read = rxq->write = 0;
	rxq->write_actual = 0;
	rxq->free_count = 0;
	rxq->need_update = 0;
	return 0;

err_rb:
	pci_free_consistent(priv->pci_dev, 4 * RX_QUEUE_SIZE, rxq->bd,
			    rxq->dma_addr);
err_bd:
	return -ENOMEM;
}
EXPORT_SYMBOL(iwl_rx_queue_alloc);

void iwl_rx_queue_reset(struct iwl_priv *priv, struct iwl_rx_queue *rxq)
{
	unsigned long flags;
	int i;
	spin_lock_irqsave(&rxq->lock, flags);
	INIT_LIST_HEAD(&rxq->rx_free);
	INIT_LIST_HEAD(&rxq->rx_used);
	
	for (i = 0; i < RX_FREE_BUFFERS + RX_QUEUE_SIZE; i++) {
		
		if (rxq->pool[i].skb != NULL) {
			pci_unmap_single(priv->pci_dev,
					 rxq->pool[i].real_dma_addr,
					 priv->hw_params.rx_buf_size + 256,
					 PCI_DMA_FROMDEVICE);
			priv->alloc_rxb_skb--;
			dev_kfree_skb(rxq->pool[i].skb);
			rxq->pool[i].skb = NULL;
		}
		list_add_tail(&rxq->pool[i].list, &rxq->rx_used);
	}

	
	rxq->read = rxq->write = 0;
	rxq->write_actual = 0;
	rxq->free_count = 0;
	spin_unlock_irqrestore(&rxq->lock, flags);
}

int iwl_rx_init(struct iwl_priv *priv, struct iwl_rx_queue *rxq)
{
	u32 rb_size;
	const u32 rfdnlog = RX_QUEUE_SIZE_LOG; 
	u32 rb_timeout = 0; 

	if (!priv->cfg->use_isr_legacy)
		rb_timeout = RX_RB_TIMEOUT;

	if (priv->cfg->mod_params->amsdu_size_8K)
		rb_size = FH_RCSR_RX_CONFIG_REG_VAL_RB_SIZE_8K;
	else
		rb_size = FH_RCSR_RX_CONFIG_REG_VAL_RB_SIZE_4K;

	
	iwl_write_direct32(priv, FH_MEM_RCSR_CHNL0_CONFIG_REG, 0);

	
	iwl_write_direct32(priv, FH_RSCSR_CHNL0_RBDCB_WPTR_REG, 0);

	
	iwl_write_direct32(priv, FH_RSCSR_CHNL0_RBDCB_BASE_REG,
			   (u32)(rxq->dma_addr >> 8));

	
	iwl_write_direct32(priv, FH_RSCSR_CHNL0_STTS_WPTR_REG,
			   rxq->rb_stts_dma >> 4);

	
	iwl_write_direct32(priv, FH_MEM_RCSR_CHNL0_CONFIG_REG,
			   FH_RCSR_RX_CONFIG_CHNL_EN_ENABLE_VAL |
			   FH_RCSR_CHNL0_RX_IGNORE_RXF_EMPTY |
			   FH_RCSR_CHNL0_RX_CONFIG_IRQ_DEST_INT_HOST_VAL |
			   FH_RCSR_CHNL0_RX_CONFIG_SINGLE_FRAME_MSK |
			   rb_size|
			   (rb_timeout << FH_RCSR_RX_CONFIG_REG_IRQ_RBTH_POS)|
			   (rfdnlog << FH_RCSR_RX_CONFIG_RBDCB_SIZE_POS));

	iwl_write32(priv, CSR_INT_COALESCING, 0x40);

	return 0;
}

int iwl_rxq_stop(struct iwl_priv *priv)
{

	
	iwl_write_direct32(priv, FH_MEM_RCSR_CHNL0_CONFIG_REG, 0);
	iwl_poll_direct_bit(priv, FH_MEM_RSSR_RX_STATUS_REG,
			    FH_RSSR_CHNL0_RX_STATUS_CHNL_IDLE, 1000);

	return 0;
}
EXPORT_SYMBOL(iwl_rxq_stop);

void iwl_rx_missed_beacon_notif(struct iwl_priv *priv,
				struct iwl_rx_mem_buffer *rxb)

{
	struct iwl_rx_packet *pkt = (struct iwl_rx_packet *)rxb->skb->data;
	struct iwl_missed_beacon_notif *missed_beacon;

	missed_beacon = &pkt->u.missed_beacon;
	if (le32_to_cpu(missed_beacon->consequtive_missed_beacons) > 5) {
		IWL_DEBUG_CALIB(priv, "missed bcn cnsq %d totl %d rcd %d expctd %d\n",
		    le32_to_cpu(missed_beacon->consequtive_missed_beacons),
		    le32_to_cpu(missed_beacon->total_missed_becons),
		    le32_to_cpu(missed_beacon->num_recvd_beacons),
		    le32_to_cpu(missed_beacon->num_expected_beacons));
		if (!test_bit(STATUS_SCANNING, &priv->status))
			iwl_init_sensitivity(priv);
	}
}
EXPORT_SYMBOL(iwl_rx_missed_beacon_notif);



static void iwl_rx_calc_noise(struct iwl_priv *priv)
{
	struct statistics_rx_non_phy *rx_info
				= &(priv->statistics.rx.general);
	int num_active_rx = 0;
	int total_silence = 0;
	int bcn_silence_a =
		le32_to_cpu(rx_info->beacon_silence_rssi_a) & IN_BAND_FILTER;
	int bcn_silence_b =
		le32_to_cpu(rx_info->beacon_silence_rssi_b) & IN_BAND_FILTER;
	int bcn_silence_c =
		le32_to_cpu(rx_info->beacon_silence_rssi_c) & IN_BAND_FILTER;

	if (bcn_silence_a) {
		total_silence += bcn_silence_a;
		num_active_rx++;
	}
	if (bcn_silence_b) {
		total_silence += bcn_silence_b;
		num_active_rx++;
	}
	if (bcn_silence_c) {
		total_silence += bcn_silence_c;
		num_active_rx++;
	}

	
	if (num_active_rx)
		priv->last_rx_noise = (total_silence / num_active_rx) - 107;
	else
		priv->last_rx_noise = IWL_NOISE_MEAS_NOT_AVAILABLE;

	IWL_DEBUG_CALIB(priv, "inband silence a %u, b %u, c %u, dBm %d\n",
			bcn_silence_a, bcn_silence_b, bcn_silence_c,
			priv->last_rx_noise);
}

#define REG_RECALIB_PERIOD (60)

void iwl_rx_statistics(struct iwl_priv *priv,
			      struct iwl_rx_mem_buffer *rxb)
{
	int change;
	struct iwl_rx_packet *pkt = (struct iwl_rx_packet *)rxb->skb->data;

	IWL_DEBUG_RX(priv, "Statistics notification received (%d vs %d).\n",
		     (int)sizeof(priv->statistics),
		     le32_to_cpu(pkt->len_n_flags) & FH_RSCSR_FRAME_SIZE_MSK);

	change = ((priv->statistics.general.temperature !=
		   pkt->u.stats.general.temperature) ||
		  ((priv->statistics.flag &
		    STATISTICS_REPLY_FLG_HT40_MODE_MSK) !=
		   (pkt->u.stats.flag & STATISTICS_REPLY_FLG_HT40_MODE_MSK)));

	memcpy(&priv->statistics, &pkt->u.stats, sizeof(priv->statistics));

	set_bit(STATUS_STATISTICS, &priv->status);

	
	mod_timer(&priv->statistics_periodic, jiffies +
		  msecs_to_jiffies(REG_RECALIB_PERIOD * 1000));

	if (unlikely(!test_bit(STATUS_SCANNING, &priv->status)) &&
	    (pkt->hdr.cmd == STATISTICS_NOTIFICATION)) {
		iwl_rx_calc_noise(priv);
		queue_work(priv->workqueue, &priv->run_time_calib_work);
	}

	iwl_leds_background(priv);

	if (priv->cfg->ops->lib->temp_ops.temperature && change)
		priv->cfg->ops->lib->temp_ops.temperature(priv);
}
EXPORT_SYMBOL(iwl_rx_statistics);

#define PERFECT_RSSI (-20) 
#define WORST_RSSI (-95)   
#define RSSI_RANGE (PERFECT_RSSI - WORST_RSSI)


static int iwl_calc_sig_qual(int rssi_dbm, int noise_dbm)
{
	int sig_qual;
	int degradation = PERFECT_RSSI - rssi_dbm;

	
	if (noise_dbm) {
		if (rssi_dbm - noise_dbm >= 40)
			return 100;
		else if (rssi_dbm < noise_dbm)
			return 0;
		sig_qual = ((rssi_dbm - noise_dbm) * 5) / 2;

	
	} else
		sig_qual = (100 * (RSSI_RANGE * RSSI_RANGE) - degradation *
			    (15 * RSSI_RANGE + 62 * degradation)) /
			   (RSSI_RANGE * RSSI_RANGE);

	if (sig_qual > 100)
		sig_qual = 100;
	else if (sig_qual < 1)
		sig_qual = 0;

	return sig_qual;
}


static inline int iwl_calc_rssi(struct iwl_priv *priv,
				struct iwl_rx_phy_res *rx_resp)
{
	return priv->cfg->ops->utils->calc_rssi(priv, rx_resp);
}

#ifdef CONFIG_IWLWIFI_DEBUG

static void iwl_dbg_report_frame(struct iwl_priv *priv,
		      struct iwl_rx_phy_res *phy_res, u16 length,
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
	u32 rate_n_flags;
	u32 tsf_low;
	int rssi;

	if (likely(!(iwl_get_debug_level(priv) & IWL_DL_RX)))
		return;

	
	fc = header->frame_control;
	seq_ctl = le16_to_cpu(header->seq_ctrl);

	
	channel = le16_to_cpu(phy_res->channel);
	phy_flags = le16_to_cpu(phy_res->phy_flags);
	rate_n_flags = le32_to_cpu(phy_res->rate_n_flags);

	
	rssi = iwl_calc_rssi(priv, phy_res);
	tsf_low = le64_to_cpu(phy_res->timestamp) & 0x0ffffffff;

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
		int rate_idx;
		u32 bitrate;

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

		rate_idx = iwl_hwrate_to_plcp_idx(rate_n_flags);
		if (unlikely((rate_idx < 0) || (rate_idx >= IWL_RATE_COUNT))) {
			bitrate = 0;
			WARN_ON_ONCE(1);
		} else {
			bitrate = iwl_rates[rate_idx].ieee / 2;
		}

		
		if (dataframe)
			IWL_DEBUG_RX(priv, "%s: mhd=0x%04x, dst=0x%02x, "
				     "len=%u, rssi=%d, chnl=%d, rate=%u, \n",
				     title, le16_to_cpu(fc), header->addr1[5],
				     length, rssi, channel, bitrate);
		else {
			
			IWL_DEBUG_RX(priv, "%s: 0x%04x, dst=0x%02x, src=0x%02x, "
				     "len=%u, rssi=%d, tim=%lu usec, "
				     "phy=0x%02x, chnl=%d\n",
				     title, le16_to_cpu(fc), header->addr1[5],
				     header->addr3[5], length, rssi,
				     tsf_low - priv->scan_start_tsf,
				     phy_flags, channel);
		}
	}
	if (print_dump)
		iwl_print_hex_dump(priv, IWL_DL_RX, header, length);
}
#endif


int iwl_set_decrypted_flag(struct iwl_priv *priv,
			   struct ieee80211_hdr *hdr,
			   u32 decrypt_res,
			   struct ieee80211_rx_status *stats)
{
	u16 fc = le16_to_cpu(hdr->frame_control);

	if (priv->active_rxon.filter_flags & RXON_FILTER_DIS_DECRYPT_MSK)
		return 0;

	if (!(fc & IEEE80211_FCTL_PROTECTED))
		return 0;

	IWL_DEBUG_RX(priv, "decrypt_res:0x%x\n", decrypt_res);
	switch (decrypt_res & RX_RES_STATUS_SEC_TYPE_MSK) {
	case RX_RES_STATUS_SEC_TYPE_TKIP:
		
		if ((decrypt_res & RX_RES_STATUS_DECRYPT_TYPE_MSK) ==
		    RX_RES_STATUS_BAD_KEY_TTAK)
			break;

	case RX_RES_STATUS_SEC_TYPE_WEP:
		if ((decrypt_res & RX_RES_STATUS_DECRYPT_TYPE_MSK) ==
		    RX_RES_STATUS_BAD_ICV_MIC) {
			
			IWL_DEBUG_RX(priv, "Packet destroyed\n");
			return -1;
		}
	case RX_RES_STATUS_SEC_TYPE_CCMP:
		if ((decrypt_res & RX_RES_STATUS_DECRYPT_TYPE_MSK) ==
		    RX_RES_STATUS_DECRYPT_OK) {
			IWL_DEBUG_RX(priv, "hw decrypt successfully!!!\n");
			stats->flag |= RX_FLAG_DECRYPTED;
		}
		break;

	default:
		break;
	}
	return 0;
}
EXPORT_SYMBOL(iwl_set_decrypted_flag);

static u32 iwl_translate_rx_status(struct iwl_priv *priv, u32 decrypt_in)
{
	u32 decrypt_out = 0;

	if ((decrypt_in & RX_RES_STATUS_STATION_FOUND) ==
					RX_RES_STATUS_STATION_FOUND)
		decrypt_out |= (RX_RES_STATUS_STATION_FOUND |
				RX_RES_STATUS_NO_STATION_INFO_MISMATCH);

	decrypt_out |= (decrypt_in & RX_RES_STATUS_SEC_TYPE_MSK);

	
	if ((decrypt_in & RX_RES_STATUS_SEC_TYPE_MSK) ==
					RX_RES_STATUS_SEC_TYPE_NONE)
		return decrypt_out;

	
	if ((decrypt_in & RX_RES_STATUS_SEC_TYPE_MSK) ==
					RX_RES_STATUS_SEC_TYPE_ERR)
		return decrypt_out;

	
	if ((decrypt_in & RX_MPDU_RES_STATUS_DEC_DONE_MSK) !=
					RX_MPDU_RES_STATUS_DEC_DONE_MSK)
		return decrypt_out;

	switch (decrypt_in & RX_RES_STATUS_SEC_TYPE_MSK) {

	case RX_RES_STATUS_SEC_TYPE_CCMP:
		
		if (!(decrypt_in & RX_MPDU_RES_STATUS_MIC_OK))
			
			decrypt_out |= RX_RES_STATUS_BAD_ICV_MIC;
		else
			decrypt_out |= RX_RES_STATUS_DECRYPT_OK;

		break;

	case RX_RES_STATUS_SEC_TYPE_TKIP:
		if (!(decrypt_in & RX_MPDU_RES_STATUS_TTAK_OK)) {
			
			decrypt_out |= RX_RES_STATUS_BAD_KEY_TTAK;
			break;
		}
		
	default:
		if (!(decrypt_in & RX_MPDU_RES_STATUS_ICV_OK))
			decrypt_out |= RX_RES_STATUS_BAD_ICV_MIC;
		else
			decrypt_out |= RX_RES_STATUS_DECRYPT_OK;
		break;
	};

	IWL_DEBUG_RX(priv, "decrypt_in:0x%x  decrypt_out = 0x%x\n",
					decrypt_in, decrypt_out);

	return decrypt_out;
}

static void iwl_pass_packet_to_mac80211(struct iwl_priv *priv,
					struct ieee80211_hdr *hdr,
					u16 len,
					u32 ampdu_status,
					struct iwl_rx_mem_buffer *rxb,
					struct ieee80211_rx_status *stats)
{
	
	if (unlikely(!priv->is_open)) {
		IWL_DEBUG_DROP_LIMIT(priv,
		    "Dropping packet while interface is not open.\n");
		return;
	}

	
	if (!priv->cfg->mod_params->sw_crypto &&
	    iwl_set_decrypted_flag(priv, hdr, ampdu_status, stats))
		return;

	
	skb_reserve(rxb->skb, (void *)hdr - (void *)rxb->skb->data);
	skb_put(rxb->skb, len);

	iwl_update_stats(priv, false, hdr->frame_control, len);
	memcpy(IEEE80211_SKB_RXCB(rxb->skb), stats, sizeof(*stats));
	ieee80211_rx_irqsafe(priv->hw, rxb->skb);
	priv->alloc_rxb_skb--;
	rxb->skb = NULL;
}


static int iwl_is_network_packet(struct iwl_priv *priv,
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


void iwl_rx_reply_rx(struct iwl_priv *priv,
				struct iwl_rx_mem_buffer *rxb)
{
	struct ieee80211_hdr *header;
	struct ieee80211_rx_status rx_status;
	struct iwl_rx_packet *pkt = (struct iwl_rx_packet *)rxb->skb->data;
	struct iwl_rx_phy_res *phy_res;
	__le32 rx_pkt_status;
	struct iwl4965_rx_mpdu_res_start *amsdu;
	u32 len;
	u32 ampdu_status;
	u16 fc;
	u32 rate_n_flags;

	
	if (pkt->hdr.cmd == REPLY_RX) {
		phy_res = (struct iwl_rx_phy_res *)pkt->u.raw;
		header = (struct ieee80211_hdr *)(pkt->u.raw + sizeof(*phy_res)
				+ phy_res->cfg_phy_cnt);

		len = le16_to_cpu(phy_res->byte_count);
		rx_pkt_status = *(__le32 *)(pkt->u.raw + sizeof(*phy_res) +
				phy_res->cfg_phy_cnt + len);
		ampdu_status = le32_to_cpu(rx_pkt_status);
	} else {
		if (!priv->last_phy_res[0]) {
			IWL_ERR(priv, "MPDU frame without cached PHY data\n");
			return;
		}
		phy_res = (struct iwl_rx_phy_res *)&priv->last_phy_res[1];
		amsdu = (struct iwl4965_rx_mpdu_res_start *)pkt->u.raw;
		header = (struct ieee80211_hdr *)(pkt->u.raw + sizeof(*amsdu));
		len = le16_to_cpu(amsdu->byte_count);
		rx_pkt_status = *(__le32 *)(pkt->u.raw + sizeof(*amsdu) + len);
		ampdu_status = iwl_translate_rx_status(priv,
				le32_to_cpu(rx_pkt_status));
	}

	if ((unlikely(phy_res->cfg_phy_cnt > 20))) {
		IWL_DEBUG_DROP(priv, "dsp size out of range [0,20]: %d/n",
				phy_res->cfg_phy_cnt);
		return;
	}

	if (!(rx_pkt_status & RX_RES_STATUS_NO_CRC32_ERROR) ||
	    !(rx_pkt_status & RX_RES_STATUS_NO_RXE_OVERFLOW)) {
		IWL_DEBUG_RX(priv, "Bad CRC or FIFO: 0x%08X.\n",
				le32_to_cpu(rx_pkt_status));
		return;
	}

	
	rate_n_flags = le32_to_cpu(phy_res->rate_n_flags);

	
	rx_status.mactime = le64_to_cpu(phy_res->timestamp);
	rx_status.freq =
		ieee80211_channel_to_frequency(le16_to_cpu(phy_res->channel));
	rx_status.band = (phy_res->phy_flags & RX_RES_PHY_FLAGS_BAND_24_MSK) ?
				IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ;
	rx_status.rate_idx =
		iwl_hwrate_to_mac80211_idx(rate_n_flags, rx_status.band);
	rx_status.flag = 0;

	
	

	priv->ucode_beacon_time = le32_to_cpu(phy_res->beacon_time_stamp);

	
	rx_status.signal = iwl_calc_rssi(priv, phy_res);

	
	if (iwl_is_associated(priv) &&
	    !test_bit(STATUS_SCANNING, &priv->status)) {
		rx_status.noise = priv->last_rx_noise;
		rx_status.qual = iwl_calc_sig_qual(rx_status.signal,
							 rx_status.noise);
	} else {
		rx_status.noise = IWL_NOISE_MEAS_NOT_AVAILABLE;
		rx_status.qual = iwl_calc_sig_qual(rx_status.signal, 0);
	}

	
	if (!iwl_is_associated(priv))
		priv->last_rx_noise = IWL_NOISE_MEAS_NOT_AVAILABLE;

#ifdef CONFIG_IWLWIFI_DEBUG
	
	if (unlikely(iwl_get_debug_level(priv) & IWL_DL_RX))
		iwl_dbg_report_frame(priv, phy_res, len, header, 1);
#endif
	iwl_dbg_log_rx_data_frame(priv, len, header);
	IWL_DEBUG_STATS_LIMIT(priv, "Rssi %d, noise %d, qual %d, TSF %llu\n",
		rx_status.signal, rx_status.noise, rx_status.qual,
		(unsigned long long)rx_status.mactime);

	
	rx_status.antenna =
		(le16_to_cpu(phy_res->phy_flags) & RX_RES_PHY_FLAGS_ANTENNA_MSK)
		>> RX_RES_PHY_FLAGS_ANTENNA_POS;

	
	if (phy_res->phy_flags & RX_RES_PHY_FLAGS_SHORT_PREAMBLE_MSK)
		rx_status.flag |= RX_FLAG_SHORTPRE;

	
	if (rate_n_flags & RATE_MCS_HT_MSK)
		rx_status.flag |= RX_FLAG_HT;
	if (rate_n_flags & RATE_MCS_HT40_MSK)
		rx_status.flag |= RX_FLAG_40MHZ;
	if (rate_n_flags & RATE_MCS_SGI_MSK)
		rx_status.flag |= RX_FLAG_SHORT_GI;

	if (iwl_is_network_packet(priv, header)) {
		priv->last_rx_rssi = rx_status.signal;
		priv->last_beacon_time =  priv->ucode_beacon_time;
		priv->last_tsf = le64_to_cpu(phy_res->timestamp);
	}

	fc = le16_to_cpu(header->frame_control);
	switch (fc & IEEE80211_FCTL_FTYPE) {
	case IEEE80211_FTYPE_MGMT:
	case IEEE80211_FTYPE_DATA:
		if (priv->iw_mode == NL80211_IFTYPE_AP)
			iwl_update_ps_mode(priv, fc  & IEEE80211_FCTL_PM,
						header->addr2);
		
	default:
		iwl_pass_packet_to_mac80211(priv, header, len, ampdu_status,
				rxb, &rx_status);
		break;

	}
}
EXPORT_SYMBOL(iwl_rx_reply_rx);


void iwl_rx_reply_rx_phy(struct iwl_priv *priv,
				    struct iwl_rx_mem_buffer *rxb)
{
	struct iwl_rx_packet *pkt = (struct iwl_rx_packet *)rxb->skb->data;
	priv->last_phy_res[0] = 1;
	memcpy(&priv->last_phy_res[1], &(pkt->u.raw[0]),
	       sizeof(struct iwl_rx_phy_res));
}
EXPORT_SYMBOL(iwl_rx_reply_rx_phy);
