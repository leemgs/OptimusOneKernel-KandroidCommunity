

#include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/debugfs.h>
#include <net/mac80211.h>
#include "rate.h"
#include "mesh.h"
#include "rc80211_pid.h"






static void rate_control_pid_adjust_rate(struct ieee80211_supported_band *sband,
					 struct ieee80211_sta *sta,
					 struct rc_pid_sta_info *spinfo, int adj,
					 struct rc_pid_rateinfo *rinfo)
{
	int cur_sorted, new_sorted, probe, tmp, n_bitrates, band;
	int cur = spinfo->txrate_idx;

	band = sband->band;
	n_bitrates = sband->n_bitrates;

	
	cur_sorted = rinfo[cur].rev_index;
	new_sorted = cur_sorted + adj;

	
	if (new_sorted < 0)
		new_sorted = rinfo[0].rev_index;
	else if (new_sorted >= n_bitrates)
		new_sorted = rinfo[n_bitrates - 1].rev_index;

	tmp = new_sorted;

	if (adj < 0) {
		
		for (probe = cur_sorted; probe >= new_sorted; probe--)
			if (rinfo[probe].diff <= rinfo[cur_sorted].diff &&
			    rate_supported(sta, band, rinfo[probe].index))
				tmp = probe;
	} else {
		
		for (probe = new_sorted + 1; probe < n_bitrates; probe++)
			if (rinfo[probe].diff <= rinfo[new_sorted].diff &&
			    rate_supported(sta, band, rinfo[probe].index))
				tmp = probe;
	}

	
	do {
		if (rate_supported(sta, band, rinfo[tmp].index)) {
			spinfo->txrate_idx = rinfo[tmp].index;
			break;
		}
		if (adj < 0)
			tmp--;
		else
			tmp++;
	} while (tmp < n_bitrates && tmp >= 0);

#ifdef CONFIG_MAC80211_DEBUGFS
	rate_control_pid_event_rate_change(&spinfo->events,
		spinfo->txrate_idx,
		sband->bitrates[spinfo->txrate_idx].bitrate);
#endif
}


static void rate_control_pid_normalize(struct rc_pid_info *pinfo, int l)
{
	int i, norm_offset = pinfo->norm_offset;
	struct rc_pid_rateinfo *r = pinfo->rinfo;

	if (r[0].diff > norm_offset)
		r[0].diff -= norm_offset;
	else if (r[0].diff < -norm_offset)
		r[0].diff += norm_offset;
	for (i = 0; i < l - 1; i++)
		if (r[i + 1].diff > r[i].diff + norm_offset)
			r[i + 1].diff -= norm_offset;
		else if (r[i + 1].diff <= r[i].diff)
			r[i + 1].diff += norm_offset;
}

static void rate_control_pid_sample(struct rc_pid_info *pinfo,
				    struct ieee80211_supported_band *sband,
				    struct ieee80211_sta *sta,
				    struct rc_pid_sta_info *spinfo)
{
	struct rc_pid_rateinfo *rinfo = pinfo->rinfo;
	u32 pf;
	s32 err_avg;
	u32 err_prop;
	u32 err_int;
	u32 err_der;
	int adj, i, j, tmp;
	unsigned long period;

	
	period = (HZ * pinfo->sampling_period + 500) / 1000;
	if (!period)
		period = 1;
	if (jiffies - spinfo->last_sample > 2 * period)
		spinfo->sharp_cnt = pinfo->sharpen_duration;

	spinfo->last_sample = jiffies;

	
	if (unlikely(spinfo->tx_num_xmit == 0))
		pf = spinfo->last_pf;
	else
		pf = spinfo->tx_num_failed * 100 / spinfo->tx_num_xmit;

	spinfo->tx_num_xmit = 0;
	spinfo->tx_num_failed = 0;

	
	if (pinfo->oldrate != spinfo->txrate_idx) {

		i = rinfo[pinfo->oldrate].rev_index;
		j = rinfo[spinfo->txrate_idx].rev_index;

		tmp = (pf - spinfo->last_pf);
		tmp = RC_PID_DO_ARITH_RIGHT_SHIFT(tmp, RC_PID_ARITH_SHIFT);

		rinfo[j].diff = rinfo[i].diff + tmp;
		pinfo->oldrate = spinfo->txrate_idx;
	}
	rate_control_pid_normalize(pinfo, sband->n_bitrates);

	
	err_prop = (pinfo->target << RC_PID_ARITH_SHIFT) - pf;

	err_avg = spinfo->err_avg_sc >> pinfo->smoothing_shift;
	spinfo->err_avg_sc = spinfo->err_avg_sc - err_avg + err_prop;
	err_int = spinfo->err_avg_sc >> pinfo->smoothing_shift;

	err_der = (pf - spinfo->last_pf) *
		  (1 + pinfo->sharpen_factor * spinfo->sharp_cnt);
	spinfo->last_pf = pf;
	if (spinfo->sharp_cnt)
			spinfo->sharp_cnt--;

#ifdef CONFIG_MAC80211_DEBUGFS
	rate_control_pid_event_pf_sample(&spinfo->events, pf, err_prop, err_int,
					 err_der);
#endif

	
	adj = (err_prop * pinfo->coeff_p + err_int * pinfo->coeff_i
	      + err_der * pinfo->coeff_d);
	adj = RC_PID_DO_ARITH_RIGHT_SHIFT(adj, 2 * RC_PID_ARITH_SHIFT);

	
	if (adj)
		rate_control_pid_adjust_rate(sband, sta, spinfo, adj, rinfo);
}

static void rate_control_pid_tx_status(void *priv, struct ieee80211_supported_band *sband,
				       struct ieee80211_sta *sta, void *priv_sta,
				       struct sk_buff *skb)
{
	struct rc_pid_info *pinfo = priv;
	struct rc_pid_sta_info *spinfo = priv_sta;
	unsigned long period;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

	if (!spinfo)
		return;

	
	if (info->status.rates[0].idx != spinfo->txrate_idx)
		return;

	spinfo->tx_num_xmit++;

#ifdef CONFIG_MAC80211_DEBUGFS
	rate_control_pid_event_tx_status(&spinfo->events, info);
#endif

	
	if (!(info->flags & IEEE80211_TX_STAT_ACK)) {
		spinfo->tx_num_failed += 2;
		spinfo->tx_num_xmit++;
	} else if (info->status.rates[0].count > 1) {
		spinfo->tx_num_failed++;
		spinfo->tx_num_xmit++;
	}

	
	period = (HZ * pinfo->sampling_period + 500) / 1000;
	if (!period)
		period = 1;
	if (time_after(jiffies, spinfo->last_sample + period))
		rate_control_pid_sample(pinfo, sband, sta, spinfo);
}

static void
rate_control_pid_get_rate(void *priv, struct ieee80211_sta *sta,
			  void *priv_sta,
			  struct ieee80211_tx_rate_control *txrc)
{
	struct sk_buff *skb = txrc->skb;
	struct ieee80211_supported_band *sband = txrc->sband;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct rc_pid_sta_info *spinfo = priv_sta;
	int rateidx;

	if (txrc->rts)
		info->control.rates[0].count =
			txrc->hw->conf.long_frame_max_tx_count;
	else
		info->control.rates[0].count =
			txrc->hw->conf.short_frame_max_tx_count;

	
	if (rate_control_send_low(sta, priv_sta, txrc))
		return;

	rateidx = spinfo->txrate_idx;

	if (rateidx >= sband->n_bitrates)
		rateidx = sband->n_bitrates - 1;

	info->control.rates[0].idx = rateidx;

#ifdef CONFIG_MAC80211_DEBUGFS
	rate_control_pid_event_tx_rate(&spinfo->events,
		rateidx, sband->bitrates[rateidx].bitrate);
#endif
}

static void
rate_control_pid_rate_init(void *priv, struct ieee80211_supported_band *sband,
			   struct ieee80211_sta *sta, void *priv_sta)
{
	struct rc_pid_sta_info *spinfo = priv_sta;
	struct rc_pid_info *pinfo = priv;
	struct rc_pid_rateinfo *rinfo = pinfo->rinfo;
	int i, j, tmp;
	bool s;

	

	
	for (i = 0; i < sband->n_bitrates; i++) {
		rinfo[i].index = i;
		rinfo[i].rev_index = i;
		if (RC_PID_FAST_START)
			rinfo[i].diff = 0;
		else
			rinfo[i].diff = i * pinfo->norm_offset;
	}
	for (i = 1; i < sband->n_bitrates; i++) {
		s = 0;
		for (j = 0; j < sband->n_bitrates - i; j++)
			if (unlikely(sband->bitrates[rinfo[j].index].bitrate >
				     sband->bitrates[rinfo[j + 1].index].bitrate)) {
				tmp = rinfo[j].index;
				rinfo[j].index = rinfo[j + 1].index;
				rinfo[j + 1].index = tmp;
				rinfo[rinfo[j].index].rev_index = j;
				rinfo[rinfo[j + 1].index].rev_index = j + 1;
				s = 1;
			}
		if (!s)
			break;
	}

	spinfo->txrate_idx = rate_lowest_index(sband, sta);
}

static void *rate_control_pid_alloc(struct ieee80211_hw *hw,
				    struct dentry *debugfsdir)
{
	struct rc_pid_info *pinfo;
	struct rc_pid_rateinfo *rinfo;
	struct ieee80211_supported_band *sband;
	int i, max_rates = 0;
#ifdef CONFIG_MAC80211_DEBUGFS
	struct rc_pid_debugfs_entries *de;
#endif

	pinfo = kmalloc(sizeof(*pinfo), GFP_ATOMIC);
	if (!pinfo)
		return NULL;

	for (i = 0; i < IEEE80211_NUM_BANDS; i++) {
		sband = hw->wiphy->bands[i];
		if (sband && sband->n_bitrates > max_rates)
			max_rates = sband->n_bitrates;
	}

	rinfo = kmalloc(sizeof(*rinfo) * max_rates, GFP_ATOMIC);
	if (!rinfo) {
		kfree(pinfo);
		return NULL;
	}

	pinfo->target = RC_PID_TARGET_PF;
	pinfo->sampling_period = RC_PID_INTERVAL;
	pinfo->coeff_p = RC_PID_COEFF_P;
	pinfo->coeff_i = RC_PID_COEFF_I;
	pinfo->coeff_d = RC_PID_COEFF_D;
	pinfo->smoothing_shift = RC_PID_SMOOTHING_SHIFT;
	pinfo->sharpen_factor = RC_PID_SHARPENING_FACTOR;
	pinfo->sharpen_duration = RC_PID_SHARPENING_DURATION;
	pinfo->norm_offset = RC_PID_NORM_OFFSET;
	pinfo->rinfo = rinfo;
	pinfo->oldrate = 0;

#ifdef CONFIG_MAC80211_DEBUGFS
	de = &pinfo->dentries;
	de->target = debugfs_create_u32("target_pf", S_IRUSR | S_IWUSR,
					debugfsdir, &pinfo->target);
	de->sampling_period = debugfs_create_u32("sampling_period",
						 S_IRUSR | S_IWUSR, debugfsdir,
						 &pinfo->sampling_period);
	de->coeff_p = debugfs_create_u32("coeff_p", S_IRUSR | S_IWUSR,
					 debugfsdir, (u32 *)&pinfo->coeff_p);
	de->coeff_i = debugfs_create_u32("coeff_i", S_IRUSR | S_IWUSR,
					 debugfsdir, (u32 *)&pinfo->coeff_i);
	de->coeff_d = debugfs_create_u32("coeff_d", S_IRUSR | S_IWUSR,
					 debugfsdir, (u32 *)&pinfo->coeff_d);
	de->smoothing_shift = debugfs_create_u32("smoothing_shift",
						 S_IRUSR | S_IWUSR, debugfsdir,
						 &pinfo->smoothing_shift);
	de->sharpen_factor = debugfs_create_u32("sharpen_factor",
					       S_IRUSR | S_IWUSR, debugfsdir,
					       &pinfo->sharpen_factor);
	de->sharpen_duration = debugfs_create_u32("sharpen_duration",
						  S_IRUSR | S_IWUSR, debugfsdir,
						  &pinfo->sharpen_duration);
	de->norm_offset = debugfs_create_u32("norm_offset",
					     S_IRUSR | S_IWUSR, debugfsdir,
					     &pinfo->norm_offset);
#endif

	return pinfo;
}

static void rate_control_pid_free(void *priv)
{
	struct rc_pid_info *pinfo = priv;
#ifdef CONFIG_MAC80211_DEBUGFS
	struct rc_pid_debugfs_entries *de = &pinfo->dentries;

	debugfs_remove(de->norm_offset);
	debugfs_remove(de->sharpen_duration);
	debugfs_remove(de->sharpen_factor);
	debugfs_remove(de->smoothing_shift);
	debugfs_remove(de->coeff_d);
	debugfs_remove(de->coeff_i);
	debugfs_remove(de->coeff_p);
	debugfs_remove(de->sampling_period);
	debugfs_remove(de->target);
#endif

	kfree(pinfo->rinfo);
	kfree(pinfo);
}

static void *rate_control_pid_alloc_sta(void *priv, struct ieee80211_sta *sta,
					gfp_t gfp)
{
	struct rc_pid_sta_info *spinfo;

	spinfo = kzalloc(sizeof(*spinfo), gfp);
	if (spinfo == NULL)
		return NULL;

	spinfo->last_sample = jiffies;

#ifdef CONFIG_MAC80211_DEBUGFS
	spin_lock_init(&spinfo->events.lock);
	init_waitqueue_head(&spinfo->events.waitqueue);
#endif

	return spinfo;
}

static void rate_control_pid_free_sta(void *priv, struct ieee80211_sta *sta,
				      void *priv_sta)
{
	kfree(priv_sta);
}

static struct rate_control_ops mac80211_rcpid = {
	.name = "pid",
	.tx_status = rate_control_pid_tx_status,
	.get_rate = rate_control_pid_get_rate,
	.rate_init = rate_control_pid_rate_init,
	.alloc = rate_control_pid_alloc,
	.free = rate_control_pid_free,
	.alloc_sta = rate_control_pid_alloc_sta,
	.free_sta = rate_control_pid_free_sta,
#ifdef CONFIG_MAC80211_DEBUGFS
	.add_sta_debugfs = rate_control_pid_add_sta_debugfs,
	.remove_sta_debugfs = rate_control_pid_remove_sta_debugfs,
#endif
};

int __init rc80211_pid_init(void)
{
	return ieee80211_rate_control_register(&mac80211_rcpid);
}

void rc80211_pid_exit(void)
{
	ieee80211_rate_control_unregister(&mac80211_rcpid);
}
