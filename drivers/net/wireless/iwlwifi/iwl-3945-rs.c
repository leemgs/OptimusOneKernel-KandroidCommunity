

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/wireless.h>
#include <net/mac80211.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>

#include <linux/workqueue.h>

#include "iwl-commands.h"
#include "iwl-3945.h"
#include "iwl-sta.h"

#define RS_NAME "iwl-3945-rs"

struct iwl3945_rate_scale_data {
	u64 data;
	s32 success_counter;
	s32 success_ratio;
	s32 counter;
	s32 average_tpt;
	unsigned long stamp;
};

struct iwl3945_rs_sta {
	spinlock_t lock;
	struct iwl_priv *priv;
	s32 *expected_tpt;
	unsigned long last_partial_flush;
	unsigned long last_flush;
	u32 flush_time;
	u32 last_tx_packets;
	u32 tx_packets;
	u8 tgg;
	u8 flush_pending;
	u8 start_rate;
	u8 ibss_sta_added;
	struct timer_list rate_scale_flush;
	struct iwl3945_rate_scale_data win[IWL_RATE_COUNT_3945];
#ifdef CONFIG_MAC80211_DEBUGFS
	struct dentry *rs_sta_dbgfs_stats_table_file;
#endif

	
	int last_txrate_idx;
};

static s32 iwl3945_expected_tpt_g[IWL_RATE_COUNT_3945] = {
	7, 13, 35, 58, 0, 0, 76, 104, 130, 168, 191, 202
};

static s32 iwl3945_expected_tpt_g_prot[IWL_RATE_COUNT_3945] = {
	7, 13, 35, 58, 0, 0, 0, 80, 93, 113, 123, 125
};

static s32 iwl3945_expected_tpt_a[IWL_RATE_COUNT_3945] = {
	0, 0, 0, 0, 40, 57, 72, 98, 121, 154, 177, 186
};

static s32 iwl3945_expected_tpt_b[IWL_RATE_COUNT_3945] = {
	7, 13, 35, 58, 0, 0, 0, 0, 0, 0, 0, 0
};

struct iwl3945_tpt_entry {
	s8 min_rssi;
	u8 index;
};

static struct iwl3945_tpt_entry iwl3945_tpt_table_a[] = {
	{-60, IWL_RATE_54M_INDEX},
	{-64, IWL_RATE_48M_INDEX},
	{-72, IWL_RATE_36M_INDEX},
	{-80, IWL_RATE_24M_INDEX},
	{-84, IWL_RATE_18M_INDEX},
	{-85, IWL_RATE_12M_INDEX},
	{-87, IWL_RATE_9M_INDEX},
	{-89, IWL_RATE_6M_INDEX}
};

static struct iwl3945_tpt_entry iwl3945_tpt_table_g[] = {
	{-60, IWL_RATE_54M_INDEX},
	{-64, IWL_RATE_48M_INDEX},
	{-68, IWL_RATE_36M_INDEX},
	{-80, IWL_RATE_24M_INDEX},
	{-84, IWL_RATE_18M_INDEX},
	{-85, IWL_RATE_12M_INDEX},
	{-86, IWL_RATE_11M_INDEX},
	{-88, IWL_RATE_5M_INDEX},
	{-90, IWL_RATE_2M_INDEX},
	{-92, IWL_RATE_1M_INDEX}
};

#define IWL_RATE_MAX_WINDOW          62
#define IWL_RATE_FLUSH       	 (3*HZ)
#define IWL_RATE_WIN_FLUSH       (HZ/2)
#define IWL39_RATE_HIGH_TH          11520
#define IWL_SUCCESS_UP_TH	   8960
#define IWL_SUCCESS_DOWN_TH	  10880
#define IWL_RATE_MIN_FAILURE_TH       6
#define IWL_RATE_MIN_SUCCESS_TH       8
#define IWL_RATE_DECREASE_TH       1920
#define IWL_RATE_RETRY_TH	     15

static u8 iwl3945_get_rate_index_by_rssi(s32 rssi, enum ieee80211_band band)
{
	u32 index = 0;
	u32 table_size = 0;
	struct iwl3945_tpt_entry *tpt_table = NULL;

	if ((rssi < IWL_MIN_RSSI_VAL) || (rssi > IWL_MAX_RSSI_VAL))
		rssi = IWL_MIN_RSSI_VAL;

	switch (band) {
	case IEEE80211_BAND_2GHZ:
		tpt_table = iwl3945_tpt_table_g;
		table_size = ARRAY_SIZE(iwl3945_tpt_table_g);
		break;

	case IEEE80211_BAND_5GHZ:
		tpt_table = iwl3945_tpt_table_a;
		table_size = ARRAY_SIZE(iwl3945_tpt_table_a);
		break;

	default:
		BUG();
		break;
	}

	while ((index < table_size) && (rssi < tpt_table[index].min_rssi))
		index++;

	index = min(index, (table_size - 1));

	return tpt_table[index].index;
}

static void iwl3945_clear_window(struct iwl3945_rate_scale_data *window)
{
	window->data = 0;
	window->success_counter = 0;
	window->success_ratio = -1;
	window->counter = 0;
	window->average_tpt = IWL_INVALID_VALUE;
	window->stamp = 0;
}


static int iwl3945_rate_scale_flush_windows(struct iwl3945_rs_sta *rs_sta)
{
	int unflushed = 0;
	int i;
	unsigned long flags;
	struct iwl_priv *priv __maybe_unused = rs_sta->priv;

	
	for (i = 0; i < IWL_RATE_COUNT_3945; i++) {
		if (!rs_sta->win[i].counter)
			continue;

		spin_lock_irqsave(&rs_sta->lock, flags);
		if (time_after(jiffies, rs_sta->win[i].stamp +
			       IWL_RATE_WIN_FLUSH)) {
			IWL_DEBUG_RATE(priv, "flushing %d samples of rate "
				       "index %d\n",
				       rs_sta->win[i].counter, i);
			iwl3945_clear_window(&rs_sta->win[i]);
		} else
			unflushed++;
		spin_unlock_irqrestore(&rs_sta->lock, flags);
	}

	return unflushed;
}

#define IWL_RATE_FLUSH_MAX              5000	
#define IWL_RATE_FLUSH_MIN              50	
#define IWL_AVERAGE_PACKETS             1500

static void iwl3945_bg_rate_scale_flush(unsigned long data)
{
	struct iwl3945_rs_sta *rs_sta = (void *)data;
	struct iwl_priv *priv __maybe_unused = rs_sta->priv;
	int unflushed = 0;
	unsigned long flags;
	u32 packet_count, duration, pps;

	IWL_DEBUG_RATE(priv, "enter\n");

	unflushed = iwl3945_rate_scale_flush_windows(rs_sta);

	spin_lock_irqsave(&rs_sta->lock, flags);

	
	packet_count = (rs_sta->tx_packets - rs_sta->last_tx_packets) + 1;

	rs_sta->last_tx_packets = rs_sta->tx_packets + 1;

	if (unflushed) {
		duration =
		    jiffies_to_msecs(jiffies - rs_sta->last_partial_flush);

		IWL_DEBUG_RATE(priv, "Tx'd %d packets in %dms\n",
			       packet_count, duration);

		
		if (duration)
			pps = (packet_count * 1000) / duration;
		else
			pps = 0;

		if (pps) {
			duration = (IWL_AVERAGE_PACKETS * 1000) / pps;
			if (duration < IWL_RATE_FLUSH_MIN)
				duration = IWL_RATE_FLUSH_MIN;
			else if (duration > IWL_RATE_FLUSH_MAX)
				duration = IWL_RATE_FLUSH_MAX;
		} else
			duration = IWL_RATE_FLUSH_MAX;

		rs_sta->flush_time = msecs_to_jiffies(duration);

		IWL_DEBUG_RATE(priv, "new flush period: %d msec ave %d\n",
			       duration, packet_count);

		mod_timer(&rs_sta->rate_scale_flush, jiffies +
			  rs_sta->flush_time);

		rs_sta->last_partial_flush = jiffies;
	} else {
		rs_sta->flush_time = IWL_RATE_FLUSH;
		rs_sta->flush_pending = 0;
	}
	

	rs_sta->last_flush = jiffies;

	spin_unlock_irqrestore(&rs_sta->lock, flags);

	IWL_DEBUG_RATE(priv, "leave\n");
}


static void iwl3945_collect_tx_data(struct iwl3945_rs_sta *rs_sta,
				struct iwl3945_rate_scale_data *window,
				int success, int retries, int index)
{
	unsigned long flags;
	s32 fail_count;
	struct iwl_priv *priv __maybe_unused = rs_sta->priv;

	if (!retries) {
		IWL_DEBUG_RATE(priv, "leave: retries == 0 -- should be at least 1\n");
		return;
	}

	spin_lock_irqsave(&rs_sta->lock, flags);

	
	while (retries > 0) {
		if (window->counter >= IWL_RATE_MAX_WINDOW) {

			
			window->counter = IWL_RATE_MAX_WINDOW - 1;

			if (window->data & (1ULL << (IWL_RATE_MAX_WINDOW - 1))) {
				window->data &= ~(1ULL << (IWL_RATE_MAX_WINDOW - 1));
				window->success_counter--;
			}
		}

		
		window->counter++;

		
		window->data <<= 1;
		if (success > 0) {
			window->success_counter++;
			window->data |= 0x1;
			success--;
		}

		retries--;
	}

	
	if (window->counter > 0)
		window->success_ratio = 128 * (100 * window->success_counter)
					/ window->counter;
	else
		window->success_ratio = IWL_INVALID_VALUE;

	fail_count = window->counter - window->success_counter;

	
	if ((fail_count >= IWL_RATE_MIN_FAILURE_TH) ||
	    (window->success_counter >= IWL_RATE_MIN_SUCCESS_TH))
		window->average_tpt = ((window->success_ratio *
				rs_sta->expected_tpt[index] + 64) / 128);
	else
		window->average_tpt = IWL_INVALID_VALUE;

	
	window->stamp = jiffies;

	spin_unlock_irqrestore(&rs_sta->lock, flags);

}

static void rs_rate_init(void *priv_r, struct ieee80211_supported_band *sband,
			 struct ieee80211_sta *sta, void *priv_sta)
{
	struct iwl3945_rs_sta *rs_sta = priv_sta;
	struct iwl_priv *priv = (struct iwl_priv *)priv_r;
	int i;

	IWL_DEBUG_RATE(priv, "enter\n");

	

	for (i = sband->n_bitrates - 1; i >= 0; i--) {
		if (sta->supp_rates[sband->band] & (1 << i)) {
			rs_sta->last_txrate_idx = i;
			break;
		}
	}

	priv->sta_supp_rates = sta->supp_rates[sband->band];
	
	if (sband->band == IEEE80211_BAND_5GHZ) {
		rs_sta->last_txrate_idx += IWL_FIRST_OFDM_RATE;
		priv->sta_supp_rates = priv->sta_supp_rates <<
						IWL_FIRST_OFDM_RATE;
	}


	IWL_DEBUG_RATE(priv, "leave\n");
}

static void *rs_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
{
	return hw->priv;
}


static void rs_free(void *priv)
{
	return;
}

static void *rs_alloc_sta(void *iwl_priv, struct ieee80211_sta *sta, gfp_t gfp)
{
	struct iwl3945_rs_sta *rs_sta;
	struct iwl3945_sta_priv *psta = (void *) sta->drv_priv;
	struct iwl_priv *priv = iwl_priv;
	int i;

	

	IWL_DEBUG_RATE(priv, "enter\n");

	rs_sta = kzalloc(sizeof(struct iwl3945_rs_sta), gfp);
	if (!rs_sta) {
		IWL_DEBUG_RATE(priv, "leave: ENOMEM\n");
		return NULL;
	}

	psta->rs_sta = rs_sta;

	spin_lock_init(&rs_sta->lock);

	rs_sta->priv = priv;

	rs_sta->start_rate = IWL_RATE_INVALID;

	
	rs_sta->expected_tpt = iwl3945_expected_tpt_b;

	rs_sta->last_partial_flush = jiffies;
	rs_sta->last_flush = jiffies;
	rs_sta->flush_time = IWL_RATE_FLUSH;
	rs_sta->last_tx_packets = 0;
	rs_sta->ibss_sta_added = 0;

	init_timer(&rs_sta->rate_scale_flush);
	rs_sta->rate_scale_flush.data = (unsigned long)rs_sta;
	rs_sta->rate_scale_flush.function = &iwl3945_bg_rate_scale_flush;

	for (i = 0; i < IWL_RATE_COUNT_3945; i++)
		iwl3945_clear_window(&rs_sta->win[i]);

	IWL_DEBUG_RATE(priv, "leave\n");

	return rs_sta;
}

static void rs_free_sta(void *iwl_priv, struct ieee80211_sta *sta,
			void *priv_sta)
{
	struct iwl3945_sta_priv *psta = (void *) sta->drv_priv;
	struct iwl3945_rs_sta *rs_sta = priv_sta;
	struct iwl_priv *priv __maybe_unused = rs_sta->priv;

	psta->rs_sta = NULL;

	IWL_DEBUG_RATE(priv, "enter\n");
	del_timer_sync(&rs_sta->rate_scale_flush);
	kfree(rs_sta);
	IWL_DEBUG_RATE(priv, "leave\n");
}



static void rs_tx_status(void *priv_rate, struct ieee80211_supported_band *sband,
			 struct ieee80211_sta *sta, void *priv_sta,
			 struct sk_buff *skb)
{
	s8 retries = 0, current_count;
	int scale_rate_index, first_index, last_index;
	unsigned long flags;
	struct iwl_priv *priv = (struct iwl_priv *)priv_rate;
	struct iwl3945_rs_sta *rs_sta = priv_sta;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

	IWL_DEBUG_RATE(priv, "enter\n");

	retries = info->status.rates[0].count;
	
	if (retries > IWL_RATE_RETRY_TH)
		retries = IWL_RATE_RETRY_TH;

	first_index = sband->bitrates[info->status.rates[0].idx].hw_value;
	if ((first_index < 0) || (first_index >= IWL_RATE_COUNT_3945)) {
		IWL_DEBUG_RATE(priv, "leave: Rate out of bounds: %d\n", first_index);
		return;
	}

	if (!priv_sta) {
		IWL_DEBUG_RATE(priv, "leave: No STA priv data to update!\n");
		return;
	}

	rs_sta->tx_packets++;

	scale_rate_index = first_index;
	last_index = first_index;

	
	while (retries > 1) {
		if ((retries - 1) < priv->retry_rate) {
			current_count = (retries - 1);
			last_index = scale_rate_index;
		} else {
			current_count = priv->retry_rate;
			last_index = iwl3945_rs_next_rate(priv,
							 scale_rate_index);
		}

		
		iwl3945_collect_tx_data(rs_sta,
				    &rs_sta->win[scale_rate_index],
				    0, current_count, scale_rate_index);
		IWL_DEBUG_RATE(priv, "Update rate %d for %d retries.\n",
			       scale_rate_index, current_count);

		retries -= current_count;

		scale_rate_index = last_index;
	}


	
	IWL_DEBUG_RATE(priv, "Update rate %d with %s.\n",
		       last_index,
		       (info->flags & IEEE80211_TX_STAT_ACK) ?
		       "success" : "failure");
	iwl3945_collect_tx_data(rs_sta,
			    &rs_sta->win[last_index],
			    info->flags & IEEE80211_TX_STAT_ACK, 1, last_index);

	
	spin_lock_irqsave(&rs_sta->lock, flags);

	if (!rs_sta->flush_pending &&
	    time_after(jiffies, rs_sta->last_flush +
		       rs_sta->flush_time)) {

		rs_sta->last_partial_flush = jiffies;
		rs_sta->flush_pending = 1;
		mod_timer(&rs_sta->rate_scale_flush,
			  jiffies + rs_sta->flush_time);
	}

	spin_unlock_irqrestore(&rs_sta->lock, flags);

	IWL_DEBUG_RATE(priv, "leave\n");

	return;
}

static u16 iwl3945_get_adjacent_rate(struct iwl3945_rs_sta *rs_sta,
				 u8 index, u16 rate_mask, enum ieee80211_band band)
{
	u8 high = IWL_RATE_INVALID;
	u8 low = IWL_RATE_INVALID;
	struct iwl_priv *priv __maybe_unused = rs_sta->priv;

	
	if (unlikely(band == IEEE80211_BAND_5GHZ)) {
		int i;
		u32 mask;

		
		i = index - 1;
		for (mask = (1 << i); i >= 0; i--, mask >>= 1) {
			if (rate_mask & mask) {
				low = i;
				break;
			}
		}

		
		i = index + 1;
		for (mask = (1 << i); i < IWL_RATE_COUNT_3945;
		     i++, mask <<= 1) {
			if (rate_mask & mask) {
				high = i;
				break;
			}
		}

		return (high << 8) | low;
	}

	low = index;
	while (low != IWL_RATE_INVALID) {
		if (rs_sta->tgg)
			low = iwl3945_rates[low].prev_rs_tgg;
		else
			low = iwl3945_rates[low].prev_rs;
		if (low == IWL_RATE_INVALID)
			break;
		if (rate_mask & (1 << low))
			break;
		IWL_DEBUG_RATE(priv, "Skipping masked lower rate: %d\n", low);
	}

	high = index;
	while (high != IWL_RATE_INVALID) {
		if (rs_sta->tgg)
			high = iwl3945_rates[high].next_rs_tgg;
		else
			high = iwl3945_rates[high].next_rs;
		if (high == IWL_RATE_INVALID)
			break;
		if (rate_mask & (1 << high))
			break;
		IWL_DEBUG_RATE(priv, "Skipping masked higher rate: %d\n", high);
	}

	return (high << 8) | low;
}


static void rs_get_rate(void *priv_r, struct ieee80211_sta *sta,
			void *priv_sta,	struct ieee80211_tx_rate_control *txrc)
{
	struct ieee80211_supported_band *sband = txrc->sband;
	struct sk_buff *skb = txrc->skb;
	u8 low = IWL_RATE_INVALID;
	u8 high = IWL_RATE_INVALID;
	u16 high_low;
	int index;
	struct iwl3945_rs_sta *rs_sta = priv_sta;
	struct iwl3945_rate_scale_data *window = NULL;
	int current_tpt = IWL_INVALID_VALUE;
	int low_tpt = IWL_INVALID_VALUE;
	int high_tpt = IWL_INVALID_VALUE;
	u32 fail_count;
	s8 scale_action = 0;
	unsigned long flags;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	u16 rate_mask = sta ? sta->supp_rates[sband->band] : 0;
	s8 max_rate_idx = -1;
	struct iwl_priv *priv = (struct iwl_priv *)priv_r;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

	IWL_DEBUG_RATE(priv, "enter\n");

	if (rate_control_send_low(sta, priv_sta, txrc))
		return;

	rate_mask = sta->supp_rates[sband->band];

	
	max_rate_idx = txrc->max_rate_idx;
	if ((sband->band == IEEE80211_BAND_5GHZ) && (max_rate_idx != -1))
		max_rate_idx += IWL_FIRST_OFDM_RATE;
	if ((max_rate_idx < 0) || (max_rate_idx >= IWL_RATE_COUNT))
		max_rate_idx = -1;

	index = min(rs_sta->last_txrate_idx & 0xffff, IWL_RATE_COUNT_3945 - 1);

	if (sband->band == IEEE80211_BAND_5GHZ)
		rate_mask = rate_mask << IWL_FIRST_OFDM_RATE;

	if ((priv->iw_mode == NL80211_IFTYPE_ADHOC) &&
	    !rs_sta->ibss_sta_added) {
		u8 sta_id = iwl_find_station(priv, hdr->addr1);

		if (sta_id == IWL_INVALID_STATION) {
			IWL_DEBUG_RATE(priv, "LQ: ADD station %pM\n",
				       hdr->addr1);
			sta_id = iwl_add_station(priv, hdr->addr1, false,
				CMD_ASYNC, NULL);
		}
		if (sta_id != IWL_INVALID_STATION)
			rs_sta->ibss_sta_added = 1;
	}

	spin_lock_irqsave(&rs_sta->lock, flags);

	
	if (rs_sta->start_rate != IWL_RATE_INVALID) {
		if (rs_sta->start_rate < index &&
		   (rate_mask & (1 << rs_sta->start_rate)))
			index = rs_sta->start_rate;
		rs_sta->start_rate = IWL_RATE_INVALID;
	}

	
	if ((max_rate_idx != -1) && (max_rate_idx < index)) {
		if (rate_mask & (1 << max_rate_idx))
			index = max_rate_idx;
	}

	window = &(rs_sta->win[index]);

	fail_count = window->counter - window->success_counter;

	if (((fail_count < IWL_RATE_MIN_FAILURE_TH) &&
	     (window->success_counter < IWL_RATE_MIN_SUCCESS_TH))) {
		spin_unlock_irqrestore(&rs_sta->lock, flags);

		IWL_DEBUG_RATE(priv, "Invalid average_tpt on rate %d: "
			       "counter: %d, success_counter: %d, "
			       "expected_tpt is %sNULL\n",
			       index,
			       window->counter,
			       window->success_counter,
			       rs_sta->expected_tpt ? "not " : "");

	   
		window->average_tpt = IWL_INVALID_VALUE;
		goto out;

	}

	current_tpt = window->average_tpt;

	high_low = iwl3945_get_adjacent_rate(rs_sta, index, rate_mask,
					     sband->band);
	low = high_low & 0xff;
	high = (high_low >> 8) & 0xff;

	
	if ((max_rate_idx != -1) && (max_rate_idx < high))
		high = IWL_RATE_INVALID;

	
	if (low != IWL_RATE_INVALID)
		low_tpt = rs_sta->win[low].average_tpt;

	if (high != IWL_RATE_INVALID)
		high_tpt = rs_sta->win[high].average_tpt;

	spin_unlock_irqrestore(&rs_sta->lock, flags);

	scale_action = 0;

	
	if ((window->success_ratio < IWL_RATE_DECREASE_TH) || !current_tpt) {
		IWL_DEBUG_RATE(priv, "decrease rate because of low success_ratio\n");
		scale_action = -1;
	
	} else if ((low_tpt == IWL_INVALID_VALUE) &&
		   (high_tpt == IWL_INVALID_VALUE)) {

		if (high != IWL_RATE_INVALID && window->success_ratio >= IWL_RATE_INCREASE_TH)
			scale_action = 1;
		else if (low != IWL_RATE_INVALID)
			scale_action = 0;

	
	} else if ((low_tpt != IWL_INVALID_VALUE) &&
		 (high_tpt != IWL_INVALID_VALUE) &&
		 (low_tpt < current_tpt) && (high_tpt < current_tpt)) {

		IWL_DEBUG_RATE(priv, "No action -- low [%d] & high [%d] < "
			       "current_tpt [%d]\n",
			       low_tpt, high_tpt, current_tpt);
		scale_action = 0;

	
	} else {
		if (high_tpt != IWL_INVALID_VALUE) {

			
			if (high_tpt > current_tpt &&
				window->success_ratio >= IWL_RATE_INCREASE_TH)
				scale_action = 1;
			else {
				IWL_DEBUG_RATE(priv,
				    "decrease rate because of high tpt\n");
				scale_action = 0;
			}
		} else if (low_tpt != IWL_INVALID_VALUE) {
			if (low_tpt > current_tpt) {
				IWL_DEBUG_RATE(priv,
				    "decrease rate because of low tpt\n");
				scale_action = -1;
			} else if (window->success_ratio >= IWL_RATE_INCREASE_TH) {
				
				scale_action = 1;
			}
		}
	}

	
	if ((scale_action == -1) && (low != IWL_RATE_INVALID) &&
		    ((window->success_ratio > IWL_RATE_HIGH_TH) ||
		     (current_tpt > (100 * rs_sta->expected_tpt[low]))))
		scale_action = 0;

	switch (scale_action) {
	case -1:

		
		if (low != IWL_RATE_INVALID)
			index = low;
		break;

	case 1:
		
		if (high != IWL_RATE_INVALID)
			index = high;

		break;

	case 0:
	default:
		
		break;
	}

	IWL_DEBUG_RATE(priv, "Selected %d (action %d) - low %d high %d\n",
		       index, scale_action, low, high);

 out:

	rs_sta->last_txrate_idx = index;
	if (sband->band == IEEE80211_BAND_5GHZ)
		info->control.rates[0].idx = rs_sta->last_txrate_idx -
				IWL_FIRST_OFDM_RATE;
	else
		info->control.rates[0].idx = rs_sta->last_txrate_idx;

	IWL_DEBUG_RATE(priv, "leave: %d\n", index);
}

#ifdef CONFIG_MAC80211_DEBUGFS
static int iwl3945_open_file_generic(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static ssize_t iwl3945_sta_dbgfs_stats_table_read(struct file *file,
						  char __user *user_buf,
						  size_t count, loff_t *ppos)
{
	char *buff;
	int desc = 0;
	int j;
	ssize_t ret;
	struct iwl3945_rs_sta *lq_sta = file->private_data;

	buff = kmalloc(1024, GFP_KERNEL);
	if (!buff)
		return -ENOMEM;

	desc += sprintf(buff + desc, "tx packets=%d last rate index=%d\n"
			"rate=0x%X flush time %d\n",
			lq_sta->tx_packets,
			lq_sta->last_txrate_idx,
			lq_sta->start_rate, jiffies_to_msecs(lq_sta->flush_time));
	for (j = 0; j < IWL_RATE_COUNT_3945; j++) {
		desc += sprintf(buff+desc,
				"counter=%d success=%d %%=%d\n",
				lq_sta->win[j].counter,
				lq_sta->win[j].success_counter,
				lq_sta->win[j].success_ratio);
	}
	ret = simple_read_from_buffer(user_buf, count, ppos, buff, desc);
	kfree(buff);
	return ret;
}

static const struct file_operations rs_sta_dbgfs_stats_table_ops = {
	.read = iwl3945_sta_dbgfs_stats_table_read,
	.open = iwl3945_open_file_generic,
};

static void iwl3945_add_debugfs(void *priv, void *priv_sta,
				struct dentry *dir)
{
	struct iwl3945_rs_sta *lq_sta = priv_sta;

	lq_sta->rs_sta_dbgfs_stats_table_file =
		debugfs_create_file("rate_stats_table", 0600, dir,
		lq_sta, &rs_sta_dbgfs_stats_table_ops);

}

static void iwl3945_remove_debugfs(void *priv, void *priv_sta)
{
	struct iwl3945_rs_sta *lq_sta = priv_sta;
	debugfs_remove(lq_sta->rs_sta_dbgfs_stats_table_file);
}
#endif

static struct rate_control_ops rs_ops = {
	.module = NULL,
	.name = RS_NAME,
	.tx_status = rs_tx_status,
	.get_rate = rs_get_rate,
	.rate_init = rs_rate_init,
	.alloc = rs_alloc,
	.free = rs_free,
	.alloc_sta = rs_alloc_sta,
	.free_sta = rs_free_sta,
#ifdef CONFIG_MAC80211_DEBUGFS
	.add_sta_debugfs = iwl3945_add_debugfs,
	.remove_sta_debugfs = iwl3945_remove_debugfs,
#endif

};

void iwl3945_rate_scale_init(struct ieee80211_hw *hw, s32 sta_id)
{
	struct iwl_priv *priv = hw->priv;
	s32 rssi = 0;
	unsigned long flags;
	struct iwl3945_rs_sta *rs_sta;
	struct ieee80211_sta *sta;
	struct iwl3945_sta_priv *psta;

	IWL_DEBUG_RATE(priv, "enter\n");

	rcu_read_lock();

	sta = ieee80211_find_sta(hw, priv->stations[sta_id].sta.sta.addr);
	if (!sta) {
		rcu_read_unlock();
		return;
	}

	psta = (void *) sta->drv_priv;
	rs_sta = psta->rs_sta;

	spin_lock_irqsave(&rs_sta->lock, flags);

	rs_sta->tgg = 0;
	switch (priv->band) {
	case IEEE80211_BAND_2GHZ:
		
		if (priv->active_rxon.flags & RXON_FLG_TGG_PROTECT_MSK) {
			rs_sta->tgg = 1;
			rs_sta->expected_tpt = iwl3945_expected_tpt_g_prot;
		} else
			rs_sta->expected_tpt = iwl3945_expected_tpt_g;
		break;

	case IEEE80211_BAND_5GHZ:
		rs_sta->expected_tpt = iwl3945_expected_tpt_a;
		break;
	case IEEE80211_NUM_BANDS:
		BUG();
		break;
	}

	spin_unlock_irqrestore(&rs_sta->lock, flags);

	rssi = priv->last_rx_rssi;
	if (rssi == 0)
		rssi = IWL_MIN_RSSI_VAL;

	IWL_DEBUG_RATE(priv, "Network RSSI: %d\n", rssi);

	rs_sta->start_rate = iwl3945_get_rate_index_by_rssi(rssi, priv->band);

	IWL_DEBUG_RATE(priv, "leave: rssi %d assign rate index: "
		       "%d (plcp 0x%x)\n", rssi, rs_sta->start_rate,
		       iwl3945_rates[rs_sta->start_rate].plcp);
	rcu_read_unlock();
}

int iwl3945_rate_control_register(void)
{
	return ieee80211_rate_control_register(&rs_ops);
}

void iwl3945_rate_control_unregister(void)
{
	ieee80211_rate_control_unregister(&rs_ops);
}


