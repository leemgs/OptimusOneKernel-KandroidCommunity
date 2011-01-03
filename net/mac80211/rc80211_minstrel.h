

#ifndef __RC_MINSTREL_H
#define __RC_MINSTREL_H

struct minstrel_rate {
	int bitrate;
	int rix;

	unsigned int perfect_tx_time;
	unsigned int ack_time;

	int sample_limit;
	unsigned int retry_count;
	unsigned int retry_count_cts;
	unsigned int retry_count_rtscts;
	unsigned int adjusted_retry_count;

	u32 success;
	u32 attempts;
	u32 last_attempts;
	u32 last_success;

	
	u32 cur_prob;
	u32 probability;

	
	u32 cur_tp;

	u64 succ_hist;
	u64 att_hist;
};

struct minstrel_sta_info {
	unsigned long stats_update;
	unsigned int sp_ack_dur;
	unsigned int rate_avg;

	unsigned int lowest_rix;

	unsigned int max_tp_rate;
	unsigned int max_tp_rate2;
	unsigned int max_prob_rate;
	unsigned int packet_count;
	unsigned int sample_count;
	int sample_deferred;

	unsigned int sample_idx;
	unsigned int sample_column;

	int n_rates;
	struct minstrel_rate *r;
	bool prev_sample;

	
	u8 *sample_table;

#ifdef CONFIG_MAC80211_DEBUGFS
	struct dentry *dbg_stats;
#endif
};

struct minstrel_priv {
	struct ieee80211_hw *hw;
	bool has_mrr;
	unsigned int cw_min;
	unsigned int cw_max;
	unsigned int max_retry;
	unsigned int ewma_level;
	unsigned int segment_size;
	unsigned int update_interval;
	unsigned int lookaround_rate;
	unsigned int lookaround_rate_mrr;
};

void minstrel_add_sta_debugfs(void *priv, void *priv_sta, struct dentry *dir);
void minstrel_remove_sta_debugfs(void *priv, void *priv_sta);

#endif
