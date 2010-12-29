



#include <linux/kernel.h>
#include <linux/module.h>

#include "rt2x00.h"
#include "rt2x00lib.h"

static int rt2x00mac_tx_rts_cts(struct rt2x00_dev *rt2x00dev,
				struct data_queue *queue,
				struct sk_buff *frag_skb)
{
	struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(frag_skb);
	struct ieee80211_tx_info *rts_info;
	struct sk_buff *skb;
	unsigned int data_length;
	int retval = 0;

	if (tx_info->control.rates[0].flags & IEEE80211_TX_RC_USE_CTS_PROTECT)
		data_length = sizeof(struct ieee80211_cts);
	else
		data_length = sizeof(struct ieee80211_rts);

	skb = dev_alloc_skb(data_length + rt2x00dev->hw->extra_tx_headroom);
	if (unlikely(!skb)) {
		WARNING(rt2x00dev, "Failed to create RTS/CTS frame.\n");
		return -ENOMEM;
	}

	skb_reserve(skb, rt2x00dev->hw->extra_tx_headroom);
	skb_put(skb, data_length);

	
	memcpy(skb->cb, frag_skb->cb, sizeof(skb->cb));
	rts_info = IEEE80211_SKB_CB(skb);
	rts_info->control.rates[0].flags &= ~IEEE80211_TX_RC_USE_RTS_CTS;
	rts_info->control.rates[0].flags &= ~IEEE80211_TX_RC_USE_CTS_PROTECT;
	rts_info->flags &= ~IEEE80211_TX_CTL_REQ_TX_STATUS;

	if (tx_info->control.rates[0].flags & IEEE80211_TX_RC_USE_CTS_PROTECT)
		rts_info->flags |= IEEE80211_TX_CTL_NO_ACK;
	else
		rts_info->flags &= ~IEEE80211_TX_CTL_NO_ACK;

	
	rts_info->control.hw_key = NULL;

	
	data_length += rt2x00crypto_tx_overhead(rt2x00dev, skb);

	if (tx_info->control.rates[0].flags & IEEE80211_TX_RC_USE_CTS_PROTECT)
		ieee80211_ctstoself_get(rt2x00dev->hw, tx_info->control.vif,
					frag_skb->data, data_length, tx_info,
					(struct ieee80211_cts *)(skb->data));
	else
		ieee80211_rts_get(rt2x00dev->hw, tx_info->control.vif,
				  frag_skb->data, data_length, tx_info,
				  (struct ieee80211_rts *)(skb->data));

	retval = rt2x00queue_write_tx_frame(queue, skb);
	if (retval) {
		dev_kfree_skb_any(skb);
		WARNING(rt2x00dev, "Failed to send RTS/CTS frame.\n");
	}

	return retval;
}

int rt2x00mac_tx(struct ieee80211_hw *hw, struct sk_buff *skb)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
	struct ieee80211_hdr *ieee80211hdr = (struct ieee80211_hdr *)skb->data;
	enum data_queue_qid qid = skb_get_queue_mapping(skb);
	struct data_queue *queue;
	u16 frame_control;

	
	if (!test_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags))
		goto exit_fail;

	
	if (tx_info->flags & IEEE80211_TX_CTL_SEND_AFTER_DTIM &&
	    test_bit(DRIVER_REQUIRE_ATIM_QUEUE, &rt2x00dev->flags))
		queue = rt2x00queue_get_queue(rt2x00dev, QID_ATIM);
	else
		queue = rt2x00queue_get_queue(rt2x00dev, qid);
	if (unlikely(!queue)) {
		ERROR(rt2x00dev,
		      "Attempt to send packet over invalid queue %d.\n"
		      "Please file bug report to %s.\n", qid, DRV_PROJECT);
		goto exit_fail;
	}

	
	frame_control = le16_to_cpu(ieee80211hdr->frame_control);
	if ((tx_info->control.rates[0].flags & (IEEE80211_TX_RC_USE_RTS_CTS |
						IEEE80211_TX_RC_USE_CTS_PROTECT)) &&
	    !rt2x00dev->ops->hw->set_rts_threshold) {
		if (rt2x00queue_available(queue) <= 1)
			goto exit_fail;

		if (rt2x00mac_tx_rts_cts(rt2x00dev, queue, skb))
			goto exit_fail;
	}

	if (rt2x00queue_write_tx_frame(queue, skb))
		goto exit_fail;

	if (rt2x00queue_threshold(queue))
		ieee80211_stop_queue(rt2x00dev->hw, qid);

	return NETDEV_TX_OK;

 exit_fail:
	ieee80211_stop_queue(rt2x00dev->hw, qid);
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}
EXPORT_SYMBOL_GPL(rt2x00mac_tx);

int rt2x00mac_start(struct ieee80211_hw *hw)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;

	if (!test_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags))
		return 0;

	return rt2x00lib_start(rt2x00dev);
}
EXPORT_SYMBOL_GPL(rt2x00mac_start);

void rt2x00mac_stop(struct ieee80211_hw *hw)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;

	if (!test_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags))
		return;

	rt2x00lib_stop(rt2x00dev);
}
EXPORT_SYMBOL_GPL(rt2x00mac_stop);

int rt2x00mac_add_interface(struct ieee80211_hw *hw,
			    struct ieee80211_if_init_conf *conf)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	struct rt2x00_intf *intf = vif_to_intf(conf->vif);
	struct data_queue *queue = rt2x00queue_get_queue(rt2x00dev, QID_BEACON);
	struct queue_entry *entry = NULL;
	unsigned int i;

	
	if (!test_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags) ||
	    !test_bit(DEVICE_STATE_STARTED, &rt2x00dev->flags))
		return -ENODEV;

	switch (conf->type) {
	case NL80211_IFTYPE_AP:
		
		if (rt2x00dev->intf_sta_count)
			return -ENOBUFS;

		
		if (rt2x00dev->intf_ap_count >= rt2x00dev->ops->max_ap_intf)
			return -ENOBUFS;

		break;
	case NL80211_IFTYPE_STATION:
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_MESH_POINT:
	case NL80211_IFTYPE_WDS:
		
		if (rt2x00dev->intf_ap_count)
			return -ENOBUFS;

		
		if (rt2x00dev->intf_sta_count >= rt2x00dev->ops->max_sta_intf)
			return -ENOBUFS;

		break;
	default:
		return -EINVAL;
	}

	
	for (i = 0; i < queue->limit; i++) {
		entry = &queue->entries[i];
		if (!test_and_set_bit(ENTRY_BCN_ASSIGNED, &entry->flags))
			break;
	}

	if (unlikely(i == queue->limit))
		return -ENOBUFS;

	

	if (conf->type == NL80211_IFTYPE_AP)
		rt2x00dev->intf_ap_count++;
	else
		rt2x00dev->intf_sta_count++;

	spin_lock_init(&intf->lock);
	spin_lock_init(&intf->seqlock);
	mutex_init(&intf->beacon_skb_mutex);
	intf->beacon = entry;

	if (conf->type == NL80211_IFTYPE_AP)
		memcpy(&intf->bssid, conf->mac_addr, ETH_ALEN);
	memcpy(&intf->mac, conf->mac_addr, ETH_ALEN);

	
	rt2x00lib_config_intf(rt2x00dev, intf, conf->type, intf->mac, NULL);

	
	rt2x00dev->packet_filter = 0;

	return 0;
}
EXPORT_SYMBOL_GPL(rt2x00mac_add_interface);

void rt2x00mac_remove_interface(struct ieee80211_hw *hw,
				struct ieee80211_if_init_conf *conf)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	struct rt2x00_intf *intf = vif_to_intf(conf->vif);

	
	if (!test_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags) ||
	    (conf->type == NL80211_IFTYPE_AP && !rt2x00dev->intf_ap_count) ||
	    (conf->type != NL80211_IFTYPE_AP && !rt2x00dev->intf_sta_count))
		return;

	if (conf->type == NL80211_IFTYPE_AP)
		rt2x00dev->intf_ap_count--;
	else
		rt2x00dev->intf_sta_count--;

	
	clear_bit(ENTRY_BCN_ASSIGNED, &intf->beacon->flags);

	
	rt2x00lib_config_intf(rt2x00dev, intf,
			      NL80211_IFTYPE_UNSPECIFIED, NULL, NULL);
}
EXPORT_SYMBOL_GPL(rt2x00mac_remove_interface);

int rt2x00mac_config(struct ieee80211_hw *hw, u32 changed)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	struct ieee80211_conf *conf = &hw->conf;

	
	if (!test_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags))
		return 0;

	
	rt2x00lib_toggle_rx(rt2x00dev, STATE_RADIO_RX_OFF);

	
	rt2x00lib_config(rt2x00dev, conf, changed);

	
	rt2x00lib_config_antenna(rt2x00dev, rt2x00dev->default_ant);

	
	rt2x00lib_toggle_rx(rt2x00dev, STATE_RADIO_RX_ON);

	return 0;
}
EXPORT_SYMBOL_GPL(rt2x00mac_config);

void rt2x00mac_configure_filter(struct ieee80211_hw *hw,
				unsigned int changed_flags,
				unsigned int *total_flags,
				u64 multicast)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;

	
	*total_flags &=
	    FIF_ALLMULTI |
	    FIF_FCSFAIL |
	    FIF_PLCPFAIL |
	    FIF_CONTROL |
	    FIF_PSPOLL |
	    FIF_OTHER_BSS |
	    FIF_PROMISC_IN_BSS;

	
	*total_flags |= FIF_ALLMULTI;
	if (*total_flags & FIF_OTHER_BSS ||
	    *total_flags & FIF_PROMISC_IN_BSS)
		*total_flags |= FIF_PROMISC_IN_BSS | FIF_OTHER_BSS;

	
	if (!test_bit(DRIVER_SUPPORT_CONTROL_FILTERS, &rt2x00dev->flags)) {
		if (*total_flags & FIF_CONTROL || *total_flags & FIF_PSPOLL)
			*total_flags |= FIF_CONTROL | FIF_PSPOLL;
	}
	if (!test_bit(DRIVER_SUPPORT_CONTROL_FILTER_PSPOLL, &rt2x00dev->flags)) {
		if (*total_flags & FIF_CONTROL)
			*total_flags |= FIF_PSPOLL;
	}

	
	if (rt2x00dev->packet_filter == *total_flags)
		return;
	rt2x00dev->packet_filter = *total_flags;

	rt2x00dev->ops->lib->config_filter(rt2x00dev, *total_flags);
}
EXPORT_SYMBOL_GPL(rt2x00mac_configure_filter);

int rt2x00mac_set_tim(struct ieee80211_hw *hw, struct ieee80211_sta *sta,
		      bool set)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;

	rt2x00lib_beacondone(rt2x00dev);
	return 0;
}
EXPORT_SYMBOL_GPL(rt2x00mac_set_tim);

#ifdef CONFIG_RT2X00_LIB_CRYPTO
static void memcpy_tkip(struct rt2x00lib_crypto *crypto, u8 *key, u8 key_len)
{
	if (key_len > NL80211_TKIP_DATA_OFFSET_ENCR_KEY)
		memcpy(&crypto->key,
		       &key[NL80211_TKIP_DATA_OFFSET_ENCR_KEY],
		       sizeof(crypto->key));

	if (key_len > NL80211_TKIP_DATA_OFFSET_TX_MIC_KEY)
		memcpy(&crypto->tx_mic,
		       &key[NL80211_TKIP_DATA_OFFSET_TX_MIC_KEY],
		       sizeof(crypto->tx_mic));

	if (key_len > NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY)
		memcpy(&crypto->rx_mic,
		       &key[NL80211_TKIP_DATA_OFFSET_RX_MIC_KEY],
		       sizeof(crypto->rx_mic));
}

int rt2x00mac_set_key(struct ieee80211_hw *hw, enum set_key_cmd cmd,
		      struct ieee80211_vif *vif, struct ieee80211_sta *sta,
		      struct ieee80211_key_conf *key)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	struct rt2x00_intf *intf = vif_to_intf(vif);
	int (*set_key) (struct rt2x00_dev *rt2x00dev,
			struct rt2x00lib_crypto *crypto,
			struct ieee80211_key_conf *key);
	struct rt2x00lib_crypto crypto;
	static const u8 bcast_addr[ETH_ALEN] =
		{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, };

	if (!test_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags))
		return 0;
	else if (!test_bit(CONFIG_SUPPORT_HW_CRYPTO, &rt2x00dev->flags))
		return -EOPNOTSUPP;
	else if (key->keylen > 32)
		return -ENOSPC;

	memset(&crypto, 0, sizeof(crypto));

	
	if (rt2x00dev->intf_sta_count)
		crypto.bssidx = 0;
	else
		crypto.bssidx = intf->mac[5] & (rt2x00dev->ops->max_ap_intf - 1);

	crypto.cipher = rt2x00crypto_key_to_cipher(key);
	if (crypto.cipher == CIPHER_NONE)
		return -EOPNOTSUPP;

	crypto.cmd = cmd;

	if (sta) {
		
		crypto.aid = sta->aid;
		crypto.address = sta->addr;
	} else
		crypto.address = bcast_addr;

	if (crypto.cipher == CIPHER_TKIP)
		memcpy_tkip(&crypto, &key->key[0], key->keylen);
	else
		memcpy(&crypto.key, &key->key[0], key->keylen);
	
	if (cmd == SET_KEY)
		key->hw_key_idx = 0;

	if (key->flags & IEEE80211_KEY_FLAG_PAIRWISE)
		set_key = rt2x00dev->ops->lib->config_pairwise_key;
	else
		set_key = rt2x00dev->ops->lib->config_shared_key;

	if (!set_key)
		return -EOPNOTSUPP;

	return set_key(rt2x00dev, &crypto, key);
}
EXPORT_SYMBOL_GPL(rt2x00mac_set_key);
#endif 

int rt2x00mac_get_stats(struct ieee80211_hw *hw,
			struct ieee80211_low_level_stats *stats)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;

	
	memcpy(stats, &rt2x00dev->low_level_stats, sizeof(*stats));

	return 0;
}
EXPORT_SYMBOL_GPL(rt2x00mac_get_stats);

int rt2x00mac_get_tx_stats(struct ieee80211_hw *hw,
			   struct ieee80211_tx_queue_stats *stats)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	unsigned int i;

	for (i = 0; i < rt2x00dev->ops->tx_queues; i++) {
		stats[i].len = rt2x00dev->tx[i].length;
		stats[i].limit = rt2x00dev->tx[i].limit;
		stats[i].count = rt2x00dev->tx[i].count;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(rt2x00mac_get_tx_stats);

void rt2x00mac_bss_info_changed(struct ieee80211_hw *hw,
				struct ieee80211_vif *vif,
				struct ieee80211_bss_conf *bss_conf,
				u32 changes)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	struct rt2x00_intf *intf = vif_to_intf(vif);
	int update_bssid = 0;

	
	if (!test_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags))
		return;

	spin_lock(&intf->lock);

	
	if (changes & BSS_CHANGED_BSSID) {
		update_bssid = 1;
		memcpy(&intf->bssid, bss_conf->bssid, ETH_ALEN);
	}

	spin_unlock(&intf->lock);

	
	if (changes & BSS_CHANGED_BSSID)
		rt2x00lib_config_intf(rt2x00dev, intf, vif->type, NULL,
				      update_bssid ? bss_conf->bssid : NULL);

	
	if (changes & (BSS_CHANGED_BEACON | BSS_CHANGED_BEACON_ENABLED))
		rt2x00queue_update_beacon(rt2x00dev, vif,
					  bss_conf->enable_beacon);

	
	if (changes & BSS_CHANGED_ASSOC) {
		rt2x00dev->link.count = 0;

		if (bss_conf->assoc)
			rt2x00dev->intf_associated++;
		else
			rt2x00dev->intf_associated--;

		rt2x00leds_led_assoc(rt2x00dev, !!rt2x00dev->intf_associated);
	}

	
	if (changes & ~(BSS_CHANGED_ASSOC | BSS_CHANGED_HT))
		rt2x00lib_config_erp(rt2x00dev, intf, bss_conf);
}
EXPORT_SYMBOL_GPL(rt2x00mac_bss_info_changed);

int rt2x00mac_conf_tx(struct ieee80211_hw *hw, u16 queue_idx,
		      const struct ieee80211_tx_queue_params *params)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	struct data_queue *queue;

	queue = rt2x00queue_get_queue(rt2x00dev, queue_idx);
	if (unlikely(!queue))
		return -EINVAL;

	
	if (params->cw_min > 0)
		queue->cw_min = fls(params->cw_min);
	else
		queue->cw_min = 5; 

	if (params->cw_max > 0)
		queue->cw_max = fls(params->cw_max);
	else
		queue->cw_max = 10; 

	queue->aifs = params->aifs;
	queue->txop = params->txop;

	INFO(rt2x00dev,
	     "Configured TX queue %d - CWmin: %d, CWmax: %d, Aifs: %d, TXop: %d.\n",
	     queue_idx, queue->cw_min, queue->cw_max, queue->aifs, queue->txop);

	return 0;
}
EXPORT_SYMBOL_GPL(rt2x00mac_conf_tx);

void rt2x00mac_rfkill_poll(struct ieee80211_hw *hw)
{
	struct rt2x00_dev *rt2x00dev = hw->priv;
	bool active = !!rt2x00dev->ops->lib->rfkill_poll(rt2x00dev);

	wiphy_rfkill_set_hw_state(hw->wiphy, !active);
}
EXPORT_SYMBOL_GPL(rt2x00mac_rfkill_poll);
