



#include <linux/kernel.h>
#include <linux/module.h>

#include "rt2x00.h"
#include "rt2x00lib.h"


int rt2x00lib_enable_radio(struct rt2x00_dev *rt2x00dev)
{
	int status;

	
	if (test_bit(DEVICE_STATE_ENABLED_RADIO, &rt2x00dev->flags))
		return 0;

	
	rt2x00queue_init_queues(rt2x00dev);

	
	status =
	    rt2x00dev->ops->lib->set_device_state(rt2x00dev, STATE_RADIO_ON);
	if (status)
		return status;

	rt2x00dev->ops->lib->set_device_state(rt2x00dev, STATE_RADIO_IRQ_ON);

	rt2x00leds_led_radio(rt2x00dev, true);
	rt2x00led_led_activity(rt2x00dev, true);

	set_bit(DEVICE_STATE_ENABLED_RADIO, &rt2x00dev->flags);

	
	rt2x00lib_toggle_rx(rt2x00dev, STATE_RADIO_RX_ON);

	
	ieee80211_wake_queues(rt2x00dev->hw);

	return 0;
}

void rt2x00lib_disable_radio(struct rt2x00_dev *rt2x00dev)
{
	if (!test_and_clear_bit(DEVICE_STATE_ENABLED_RADIO, &rt2x00dev->flags))
		return;

	
	ieee80211_stop_queues(rt2x00dev->hw);
	rt2x00queue_stop_queues(rt2x00dev);

	
	rt2x00lib_toggle_rx(rt2x00dev, STATE_RADIO_RX_OFF);

	
	rt2x00dev->ops->lib->set_device_state(rt2x00dev, STATE_RADIO_OFF);
	rt2x00dev->ops->lib->set_device_state(rt2x00dev, STATE_RADIO_IRQ_OFF);
	rt2x00led_led_activity(rt2x00dev, false);
	rt2x00leds_led_radio(rt2x00dev, false);
}

void rt2x00lib_toggle_rx(struct rt2x00_dev *rt2x00dev, enum dev_state state)
{
	
	if (state == STATE_RADIO_RX_OFF)
		rt2x00link_stop_tuner(rt2x00dev);

	rt2x00dev->ops->lib->set_device_state(rt2x00dev, state);

	
	if (state == STATE_RADIO_RX_ON)
		rt2x00link_start_tuner(rt2x00dev);
}

static void rt2x00lib_intf_scheduled_iter(void *data, u8 *mac,
					  struct ieee80211_vif *vif)
{
	struct rt2x00_dev *rt2x00dev = data;
	struct rt2x00_intf *intf = vif_to_intf(vif);
	int delayed_flags;

	
	spin_lock(&intf->lock);

	delayed_flags = intf->delayed_flags;
	intf->delayed_flags = 0;

	spin_unlock(&intf->lock);

	
	if (!test_bit(DEVICE_STATE_ENABLED_RADIO, &rt2x00dev->flags))
		return;

	if (delayed_flags & DELAYED_UPDATE_BEACON)
		rt2x00queue_update_beacon(rt2x00dev, vif, true);
}

static void rt2x00lib_intf_scheduled(struct work_struct *work)
{
	struct rt2x00_dev *rt2x00dev =
	    container_of(work, struct rt2x00_dev, intf_work);

	
	ieee80211_iterate_active_interfaces(rt2x00dev->hw,
					    rt2x00lib_intf_scheduled_iter,
					    rt2x00dev);
}


static void rt2x00lib_beacondone_iter(void *data, u8 *mac,
				      struct ieee80211_vif *vif)
{
	struct rt2x00_intf *intf = vif_to_intf(vif);

	if (vif->type != NL80211_IFTYPE_AP &&
	    vif->type != NL80211_IFTYPE_ADHOC &&
	    vif->type != NL80211_IFTYPE_MESH_POINT &&
	    vif->type != NL80211_IFTYPE_WDS)
		return;

	spin_lock(&intf->lock);
	intf->delayed_flags |= DELAYED_UPDATE_BEACON;
	spin_unlock(&intf->lock);
}

void rt2x00lib_beacondone(struct rt2x00_dev *rt2x00dev)
{
	if (!test_bit(DEVICE_STATE_ENABLED_RADIO, &rt2x00dev->flags))
		return;

	ieee80211_iterate_active_interfaces_atomic(rt2x00dev->hw,
						   rt2x00lib_beacondone_iter,
						   rt2x00dev);

	ieee80211_queue_work(rt2x00dev->hw, &rt2x00dev->intf_work);
}
EXPORT_SYMBOL_GPL(rt2x00lib_beacondone);

void rt2x00lib_txdone(struct queue_entry *entry,
		      struct txdone_entry_desc *txdesc)
{
	struct rt2x00_dev *rt2x00dev = entry->queue->rt2x00dev;
	struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(entry->skb);
	struct skb_frame_desc *skbdesc = get_skb_frame_desc(entry->skb);
	enum data_queue_qid qid = skb_get_queue_mapping(entry->skb);
	unsigned int header_length = ieee80211_get_hdrlen_from_skb(entry->skb);
	u8 rate_idx, rate_flags, retry_rates;
	unsigned int i;
	bool success;

	
	rt2x00queue_unmap_skb(rt2x00dev, entry->skb);

	
	if (test_bit(DRIVER_REQUIRE_L2PAD, &rt2x00dev->flags))
		rt2x00queue_remove_l2pad(entry->skb, header_length);

	
	if (test_bit(CONFIG_SUPPORT_HW_CRYPTO, &rt2x00dev->flags))
		rt2x00crypto_tx_insert_iv(entry->skb, header_length);

	
	rt2x00debug_dump_frame(rt2x00dev, DUMP_FRAME_TXDONE, entry->skb);

	
	success =
	    test_bit(TXDONE_SUCCESS, &txdesc->flags) ||
	    test_bit(TXDONE_UNKNOWN, &txdesc->flags) ||
	    test_bit(TXDONE_FALLBACK, &txdesc->flags);

	
	rt2x00dev->link.qual.tx_success += success;
	rt2x00dev->link.qual.tx_failed += !success;

	rate_idx = skbdesc->tx_rate_idx;
	rate_flags = skbdesc->tx_rate_flags;
	retry_rates = test_bit(TXDONE_FALLBACK, &txdesc->flags) ?
	    (txdesc->retry + 1) : 1;

	
	memset(&tx_info->status, 0, sizeof(tx_info->status));
	tx_info->status.ack_signal = 0;

	
	for (i = 0; i < retry_rates && i < IEEE80211_TX_MAX_RATES; i++) {
		tx_info->status.rates[i].idx = rate_idx - i;
		tx_info->status.rates[i].flags = rate_flags;
		tx_info->status.rates[i].count = 1;
	}
	if (i < (IEEE80211_TX_MAX_RATES - 1))
		tx_info->status.rates[i].idx = -1; 

	if (!(tx_info->flags & IEEE80211_TX_CTL_NO_ACK)) {
		if (success)
			tx_info->flags |= IEEE80211_TX_STAT_ACK;
		else
			rt2x00dev->low_level_stats.dot11ACKFailureCount++;
	}

	if (rate_flags & IEEE80211_TX_RC_USE_RTS_CTS) {
		if (success)
			rt2x00dev->low_level_stats.dot11RTSSuccessCount++;
		else
			rt2x00dev->low_level_stats.dot11RTSFailureCount++;
	}

	
	if (tx_info->flags & IEEE80211_TX_CTL_REQ_TX_STATUS)
		ieee80211_tx_status_irqsafe(rt2x00dev->hw, entry->skb);
	else
		dev_kfree_skb_irq(entry->skb);

	
	entry->skb = NULL;
	entry->flags = 0;

	rt2x00dev->ops->lib->clear_entry(entry);

	clear_bit(ENTRY_OWNER_DEVICE_DATA, &entry->flags);
	rt2x00queue_index_inc(entry->queue, Q_INDEX_DONE);

	
	if (!rt2x00queue_threshold(entry->queue))
		ieee80211_wake_queue(rt2x00dev->hw, qid);
}
EXPORT_SYMBOL_GPL(rt2x00lib_txdone);

static int rt2x00lib_rxdone_read_signal(struct rt2x00_dev *rt2x00dev,
					struct rxdone_entry_desc *rxdesc)
{
	struct ieee80211_supported_band *sband;
	const struct rt2x00_rate *rate;
	unsigned int i;
	int signal;
	int type;

	
	if (rxdesc->dev_flags & RXDONE_SIGNAL_MCS)
		signal = RATE_MCS(rxdesc->rate_mode, rxdesc->signal);
	else
		signal = rxdesc->signal;

	type = (rxdesc->dev_flags & RXDONE_SIGNAL_MASK);

	sband = &rt2x00dev->bands[rt2x00dev->curr_band];
	for (i = 0; i < sband->n_bitrates; i++) {
		rate = rt2x00_get_rate(sband->bitrates[i].hw_value);

		if (((type == RXDONE_SIGNAL_PLCP) &&
		     (rate->plcp == signal)) ||
		    ((type == RXDONE_SIGNAL_BITRATE) &&
		      (rate->bitrate == signal)) ||
		    ((type == RXDONE_SIGNAL_MCS) &&
		      (rate->mcs == signal))) {
			return i;
		}
	}

	WARNING(rt2x00dev, "Frame received with unrecognized signal, "
		"signal=0x%.4x, type=%d.\n", signal, type);
	return 0;
}

void rt2x00lib_rxdone(struct rt2x00_dev *rt2x00dev,
		      struct queue_entry *entry)
{
	struct rxdone_entry_desc rxdesc;
	struct sk_buff *skb;
	struct ieee80211_rx_status *rx_status = &rt2x00dev->rx_status;
	unsigned int header_length;
	int rate_idx;
	
	skb = rt2x00queue_alloc_rxskb(rt2x00dev, entry);
	if (!skb)
		return;

	
	rt2x00queue_unmap_skb(rt2x00dev, entry->skb);

	
	memset(&rxdesc, 0, sizeof(rxdesc));
	rt2x00dev->ops->lib->fill_rxdone(entry, &rxdesc);

	
	skb_trim(entry->skb, rxdesc.size);

	
	header_length = ieee80211_get_hdrlen_from_skb(entry->skb);

	
	if ((rxdesc.dev_flags & RXDONE_CRYPTO_IV) &&
	    (rxdesc.flags & RX_FLAG_IV_STRIPPED))
		rt2x00crypto_rx_insert_iv(entry->skb, header_length,
					  &rxdesc);
	else if (rxdesc.dev_flags & RXDONE_L2PAD)
		rt2x00queue_remove_l2pad(entry->skb, header_length);
	else
		rt2x00queue_align_payload(entry->skb, header_length);

	
	if (rxdesc.rate_mode == RATE_MODE_CCK ||
	    rxdesc.rate_mode == RATE_MODE_OFDM) {
		rate_idx = rt2x00lib_rxdone_read_signal(rt2x00dev, &rxdesc);
	} else {
		rxdesc.flags |= RX_FLAG_HT;
		rate_idx = rxdesc.signal;
	}

	
	rt2x00link_update_stats(rt2x00dev, entry->skb, &rxdesc);
	rt2x00debug_update_crypto(rt2x00dev, &rxdesc);

	rx_status->mactime = rxdesc.timestamp;
	rx_status->rate_idx = rate_idx;
	rx_status->qual = rt2x00link_calculate_signal(rt2x00dev, rxdesc.rssi);
	rx_status->signal = rxdesc.rssi;
	rx_status->noise = rxdesc.noise;
	rx_status->flag = rxdesc.flags;
	rx_status->antenna = rt2x00dev->link.ant.active.rx;

	
	rt2x00debug_dump_frame(rt2x00dev, DUMP_FRAME_RXDONE, entry->skb);
	memcpy(IEEE80211_SKB_RXCB(entry->skb), rx_status, sizeof(*rx_status));
	ieee80211_rx_irqsafe(rt2x00dev->hw, entry->skb);

	
	entry->skb = skb;
	entry->flags = 0;

	rt2x00dev->ops->lib->clear_entry(entry);

	rt2x00queue_index_inc(entry->queue, Q_INDEX);
}
EXPORT_SYMBOL_GPL(rt2x00lib_rxdone);


const struct rt2x00_rate rt2x00_supported_rates[12] = {
	{
		.flags = DEV_RATE_CCK,
		.bitrate = 10,
		.ratemask = BIT(0),
		.plcp = 0x00,
		.mcs = RATE_MCS(RATE_MODE_CCK, 0),
	},
	{
		.flags = DEV_RATE_CCK | DEV_RATE_SHORT_PREAMBLE,
		.bitrate = 20,
		.ratemask = BIT(1),
		.plcp = 0x01,
		.mcs = RATE_MCS(RATE_MODE_CCK, 1),
	},
	{
		.flags = DEV_RATE_CCK | DEV_RATE_SHORT_PREAMBLE,
		.bitrate = 55,
		.ratemask = BIT(2),
		.plcp = 0x02,
		.mcs = RATE_MCS(RATE_MODE_CCK, 2),
	},
	{
		.flags = DEV_RATE_CCK | DEV_RATE_SHORT_PREAMBLE,
		.bitrate = 110,
		.ratemask = BIT(3),
		.plcp = 0x03,
		.mcs = RATE_MCS(RATE_MODE_CCK, 3),
	},
	{
		.flags = DEV_RATE_OFDM,
		.bitrate = 60,
		.ratemask = BIT(4),
		.plcp = 0x0b,
		.mcs = RATE_MCS(RATE_MODE_OFDM, 0),
	},
	{
		.flags = DEV_RATE_OFDM,
		.bitrate = 90,
		.ratemask = BIT(5),
		.plcp = 0x0f,
		.mcs = RATE_MCS(RATE_MODE_OFDM, 1),
	},
	{
		.flags = DEV_RATE_OFDM,
		.bitrate = 120,
		.ratemask = BIT(6),
		.plcp = 0x0a,
		.mcs = RATE_MCS(RATE_MODE_OFDM, 2),
	},
	{
		.flags = DEV_RATE_OFDM,
		.bitrate = 180,
		.ratemask = BIT(7),
		.plcp = 0x0e,
		.mcs = RATE_MCS(RATE_MODE_OFDM, 3),
	},
	{
		.flags = DEV_RATE_OFDM,
		.bitrate = 240,
		.ratemask = BIT(8),
		.plcp = 0x09,
		.mcs = RATE_MCS(RATE_MODE_OFDM, 4),
	},
	{
		.flags = DEV_RATE_OFDM,
		.bitrate = 360,
		.ratemask = BIT(9),
		.plcp = 0x0d,
		.mcs = RATE_MCS(RATE_MODE_OFDM, 5),
	},
	{
		.flags = DEV_RATE_OFDM,
		.bitrate = 480,
		.ratemask = BIT(10),
		.plcp = 0x08,
		.mcs = RATE_MCS(RATE_MODE_OFDM, 6),
	},
	{
		.flags = DEV_RATE_OFDM,
		.bitrate = 540,
		.ratemask = BIT(11),
		.plcp = 0x0c,
		.mcs = RATE_MCS(RATE_MODE_OFDM, 7),
	},
};

static void rt2x00lib_channel(struct ieee80211_channel *entry,
			      const int channel, const int tx_power,
			      const int value)
{
	entry->center_freq = ieee80211_channel_to_frequency(channel);
	entry->hw_value = value;
	entry->max_power = tx_power;
	entry->max_antenna_gain = 0xff;
}

static void rt2x00lib_rate(struct ieee80211_rate *entry,
			   const u16 index, const struct rt2x00_rate *rate)
{
	entry->flags = 0;
	entry->bitrate = rate->bitrate;
	entry->hw_value =index;
	entry->hw_value_short = index;

	if (rate->flags & DEV_RATE_SHORT_PREAMBLE)
		entry->flags |= IEEE80211_RATE_SHORT_PREAMBLE;
}

static int rt2x00lib_probe_hw_modes(struct rt2x00_dev *rt2x00dev,
				    struct hw_mode_spec *spec)
{
	struct ieee80211_hw *hw = rt2x00dev->hw;
	struct ieee80211_channel *channels;
	struct ieee80211_rate *rates;
	unsigned int num_rates;
	unsigned int i;

	num_rates = 0;
	if (spec->supported_rates & SUPPORT_RATE_CCK)
		num_rates += 4;
	if (spec->supported_rates & SUPPORT_RATE_OFDM)
		num_rates += 8;

	channels = kzalloc(sizeof(*channels) * spec->num_channels, GFP_KERNEL);
	if (!channels)
		return -ENOMEM;

	rates = kzalloc(sizeof(*rates) * num_rates, GFP_KERNEL);
	if (!rates)
		goto exit_free_channels;

	
	for (i = 0; i < num_rates; i++)
		rt2x00lib_rate(&rates[i], i, rt2x00_get_rate(i));

	
	for (i = 0; i < spec->num_channels; i++) {
		rt2x00lib_channel(&channels[i],
				  spec->channels[i].channel,
				  spec->channels_info[i].tx_power1, i);
	}

	
	if (spec->supported_bands & SUPPORT_BAND_2GHZ) {
		rt2x00dev->bands[IEEE80211_BAND_2GHZ].n_channels = 14;
		rt2x00dev->bands[IEEE80211_BAND_2GHZ].n_bitrates = num_rates;
		rt2x00dev->bands[IEEE80211_BAND_2GHZ].channels = channels;
		rt2x00dev->bands[IEEE80211_BAND_2GHZ].bitrates = rates;
		hw->wiphy->bands[IEEE80211_BAND_2GHZ] =
		    &rt2x00dev->bands[IEEE80211_BAND_2GHZ];
		memcpy(&rt2x00dev->bands[IEEE80211_BAND_2GHZ].ht_cap,
		       &spec->ht, sizeof(spec->ht));
	}

	
	if (spec->supported_bands & SUPPORT_BAND_5GHZ) {
		rt2x00dev->bands[IEEE80211_BAND_5GHZ].n_channels =
		    spec->num_channels - 14;
		rt2x00dev->bands[IEEE80211_BAND_5GHZ].n_bitrates =
		    num_rates - 4;
		rt2x00dev->bands[IEEE80211_BAND_5GHZ].channels = &channels[14];
		rt2x00dev->bands[IEEE80211_BAND_5GHZ].bitrates = &rates[4];
		hw->wiphy->bands[IEEE80211_BAND_5GHZ] =
		    &rt2x00dev->bands[IEEE80211_BAND_5GHZ];
		memcpy(&rt2x00dev->bands[IEEE80211_BAND_5GHZ].ht_cap,
		       &spec->ht, sizeof(spec->ht));
	}

	return 0;

 exit_free_channels:
	kfree(channels);
	ERROR(rt2x00dev, "Allocation ieee80211 modes failed.\n");
	return -ENOMEM;
}

static void rt2x00lib_remove_hw(struct rt2x00_dev *rt2x00dev)
{
	if (test_bit(DEVICE_STATE_REGISTERED_HW, &rt2x00dev->flags))
		ieee80211_unregister_hw(rt2x00dev->hw);

	if (likely(rt2x00dev->hw->wiphy->bands[IEEE80211_BAND_2GHZ])) {
		kfree(rt2x00dev->hw->wiphy->bands[IEEE80211_BAND_2GHZ]->channels);
		kfree(rt2x00dev->hw->wiphy->bands[IEEE80211_BAND_2GHZ]->bitrates);
		rt2x00dev->hw->wiphy->bands[IEEE80211_BAND_2GHZ] = NULL;
		rt2x00dev->hw->wiphy->bands[IEEE80211_BAND_5GHZ] = NULL;
	}

	kfree(rt2x00dev->spec.channels_info);
}

static int rt2x00lib_probe_hw(struct rt2x00_dev *rt2x00dev)
{
	struct hw_mode_spec *spec = &rt2x00dev->spec;
	int status;

	if (test_bit(DEVICE_STATE_REGISTERED_HW, &rt2x00dev->flags))
		return 0;

	
	status = rt2x00lib_probe_hw_modes(rt2x00dev, spec);
	if (status)
		return status;

	
	rt2x00dev->hw->queues = rt2x00dev->ops->tx_queues;

	
	status = ieee80211_register_hw(rt2x00dev->hw);
	if (status)
		return status;

	set_bit(DEVICE_STATE_REGISTERED_HW, &rt2x00dev->flags);

	return 0;
}


static void rt2x00lib_uninitialize(struct rt2x00_dev *rt2x00dev)
{
	if (!test_and_clear_bit(DEVICE_STATE_INITIALIZED, &rt2x00dev->flags))
		return;

	
	rt2x00rfkill_unregister(rt2x00dev);

	
	rt2x00dev->ops->lib->uninitialize(rt2x00dev);

	
	rt2x00queue_uninitialize(rt2x00dev);
}

static int rt2x00lib_initialize(struct rt2x00_dev *rt2x00dev)
{
	int status;

	if (test_bit(DEVICE_STATE_INITIALIZED, &rt2x00dev->flags))
		return 0;

	
	status = rt2x00queue_initialize(rt2x00dev);
	if (status)
		return status;

	
	status = rt2x00dev->ops->lib->initialize(rt2x00dev);
	if (status) {
		rt2x00queue_uninitialize(rt2x00dev);
		return status;
	}

	set_bit(DEVICE_STATE_INITIALIZED, &rt2x00dev->flags);

	
	rt2x00rfkill_register(rt2x00dev);

	return 0;
}

int rt2x00lib_start(struct rt2x00_dev *rt2x00dev)
{
	int retval;

	if (test_bit(DEVICE_STATE_STARTED, &rt2x00dev->flags))
		return 0;

	
	retval = rt2x00lib_load_firmware(rt2x00dev);
	if (retval)
		return retval;

	
	retval = rt2x00lib_initialize(rt2x00dev);
	if (retval)
		return retval;

	rt2x00dev->intf_ap_count = 0;
	rt2x00dev->intf_sta_count = 0;
	rt2x00dev->intf_associated = 0;

	
	retval = rt2x00lib_enable_radio(rt2x00dev);
	if (retval) {
		rt2x00queue_uninitialize(rt2x00dev);
		return retval;
	}

	set_bit(DEVICE_STATE_STARTED, &rt2x00dev->flags);

	return 0;
}

void rt2x00lib_stop(struct rt2x00_dev *rt2x00dev)
{
	if (!test_and_clear_bit(DEVICE_STATE_STARTED, &rt2x00dev->flags))
		return;

	
	rt2x00lib_disable_radio(rt2x00dev);

	rt2x00dev->intf_ap_count = 0;
	rt2x00dev->intf_sta_count = 0;
	rt2x00dev->intf_associated = 0;
}


int rt2x00lib_probe_dev(struct rt2x00_dev *rt2x00dev)
{
	int retval = -ENOMEM;

	mutex_init(&rt2x00dev->csr_mutex);

	set_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags);

	
	rt2x00dev->hw->vif_data_size = sizeof(struct rt2x00_intf);

	
	rt2x00dev->hw->wiphy->interface_modes = BIT(NL80211_IFTYPE_STATION);
	if (rt2x00dev->ops->bcn->entry_num > 0)
		rt2x00dev->hw->wiphy->interface_modes |=
		    BIT(NL80211_IFTYPE_ADHOC) |
		    BIT(NL80211_IFTYPE_AP) |
		    BIT(NL80211_IFTYPE_MESH_POINT) |
		    BIT(NL80211_IFTYPE_WDS);

	
	retval = rt2x00dev->ops->lib->probe_hw(rt2x00dev);
	if (retval) {
		ERROR(rt2x00dev, "Failed to allocate device.\n");
		goto exit;
	}

	
	INIT_WORK(&rt2x00dev->intf_work, rt2x00lib_intf_scheduled);

	
	retval = rt2x00queue_allocate(rt2x00dev);
	if (retval)
		goto exit;

	
	retval = rt2x00lib_probe_hw(rt2x00dev);
	if (retval) {
		ERROR(rt2x00dev, "Failed to initialize hw.\n");
		goto exit;
	}

	
	rt2x00link_register(rt2x00dev);
	rt2x00leds_register(rt2x00dev);
	rt2x00debug_register(rt2x00dev);

	return 0;

exit:
	rt2x00lib_remove_dev(rt2x00dev);

	return retval;
}
EXPORT_SYMBOL_GPL(rt2x00lib_probe_dev);

void rt2x00lib_remove_dev(struct rt2x00_dev *rt2x00dev)
{
	clear_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags);

	
	rt2x00lib_disable_radio(rt2x00dev);

	
	cancel_work_sync(&rt2x00dev->intf_work);

	
	rt2x00lib_uninitialize(rt2x00dev);

	
	rt2x00debug_deregister(rt2x00dev);
	rt2x00leds_unregister(rt2x00dev);

	
	rt2x00lib_remove_hw(rt2x00dev);

	
	rt2x00lib_free_firmware(rt2x00dev);

	
	rt2x00queue_free(rt2x00dev);
}
EXPORT_SYMBOL_GPL(rt2x00lib_remove_dev);


#ifdef CONFIG_PM
int rt2x00lib_suspend(struct rt2x00_dev *rt2x00dev, pm_message_t state)
{
	NOTICE(rt2x00dev, "Going to sleep.\n");

	
	if (!test_and_clear_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags))
		return 0;

	
	rt2x00lib_uninitialize(rt2x00dev);

	
	rt2x00leds_suspend(rt2x00dev);
	rt2x00debug_deregister(rt2x00dev);

	
	if (rt2x00dev->ops->lib->set_device_state(rt2x00dev, STATE_SLEEP))
		WARNING(rt2x00dev, "Device failed to enter sleep state, "
			"continue suspending.\n");

	return 0;
}
EXPORT_SYMBOL_GPL(rt2x00lib_suspend);

int rt2x00lib_resume(struct rt2x00_dev *rt2x00dev)
{
	NOTICE(rt2x00dev, "Waking up.\n");

	
	rt2x00debug_register(rt2x00dev);
	rt2x00leds_resume(rt2x00dev);

	
	set_bit(DEVICE_STATE_PRESENT, &rt2x00dev->flags);

	return 0;
}
EXPORT_SYMBOL_GPL(rt2x00lib_resume);
#endif 


MODULE_AUTHOR(DRV_PROJECT);
MODULE_VERSION(DRV_VERSION);
MODULE_DESCRIPTION("rt2x00 library");
MODULE_LICENSE("GPL");
