

#ifndef STA_INFO_H
#define STA_INFO_H

#include <linux/list.h>
#include <linux/types.h>
#include <linux/if_ether.h>
#include "key.h"


enum ieee80211_sta_info_flags {
	WLAN_STA_AUTH		= 1<<0,
	WLAN_STA_ASSOC		= 1<<1,
	WLAN_STA_PS		= 1<<2,
	WLAN_STA_AUTHORIZED	= 1<<3,
	WLAN_STA_SHORT_PREAMBLE	= 1<<4,
	WLAN_STA_ASSOC_AP	= 1<<5,
	WLAN_STA_WME		= 1<<6,
	WLAN_STA_WDS		= 1<<7,
	WLAN_STA_CLEAR_PS_FILT	= 1<<9,
	WLAN_STA_MFP		= 1<<10,
	WLAN_STA_SUSPEND	= 1<<11
};

#define STA_TID_NUM 16
#define ADDBA_RESP_INTERVAL HZ
#define HT_AGG_MAX_RETRIES		(0x3)

#define HT_AGG_STATE_INITIATOR_SHIFT	(4)

#define HT_ADDBA_REQUESTED_MSK		BIT(0)
#define HT_ADDBA_DRV_READY_MSK		BIT(1)
#define HT_ADDBA_RECEIVED_MSK		BIT(2)
#define HT_AGG_STATE_REQ_STOP_BA_MSK	BIT(3)
#define HT_AGG_STATE_INITIATOR_MSK      BIT(HT_AGG_STATE_INITIATOR_SHIFT)
#define HT_AGG_STATE_IDLE		(0x0)
#define HT_AGG_STATE_OPERATIONAL	(HT_ADDBA_REQUESTED_MSK |	\
					 HT_ADDBA_DRV_READY_MSK |	\
					 HT_ADDBA_RECEIVED_MSK)


struct tid_ampdu_tx {
	struct timer_list addba_resp_timer;
	struct sk_buff_head pending;
	u16 ssn;
	u8 dialog_token;
};


struct tid_ampdu_rx {
	struct sk_buff **reorder_buf;
	unsigned long *reorder_time;
	struct timer_list session_timer;
	u16 head_seq_num;
	u16 stored_mpdu_num;
	u16 ssn;
	u16 buf_size;
	u16 timeout;
	u8 dialog_token;
	bool shutdown;
};


enum plink_state {
	PLINK_LISTEN,
	PLINK_OPN_SNT,
	PLINK_OPN_RCVD,
	PLINK_CNF_RCVD,
	PLINK_ESTAB,
	PLINK_HOLDING,
	PLINK_BLOCKED
};


struct sta_ampdu_mlme {
	
	u8 tid_state_rx[STA_TID_NUM];
	struct tid_ampdu_rx *tid_rx[STA_TID_NUM];
	
	u8 tid_state_tx[STA_TID_NUM];
	struct tid_ampdu_tx *tid_tx[STA_TID_NUM];
	u8 addba_req_num[STA_TID_NUM];
	u8 dialog_token_allocator;
};



#define STA_INFO_PIN_STAT_NORMAL	0
#define STA_INFO_PIN_STAT_PINNED	1
#define STA_INFO_PIN_STAT_DESTROY	2


struct sta_info {
	
	struct list_head list;
	struct sta_info *hnext;
	struct ieee80211_local *local;
	struct ieee80211_sub_if_data *sdata;
	struct ieee80211_key *key;
	struct rate_control_ref *rate_ctrl;
	void *rate_ctrl_priv;
	spinlock_t lock;
	spinlock_t flaglock;

	u16 listen_interval;

	
	u8 pin_status;

	
	u32 flags;

	
	struct sk_buff_head ps_tx_buf;
	struct sk_buff_head tx_filtered;

	
	unsigned long rx_packets, rx_bytes;
	unsigned long wep_weak_iv_count;
	unsigned long last_rx;
	unsigned long num_duplicates;
	unsigned long rx_fragments;
	unsigned long rx_dropped;
	int last_signal;
	int last_qual;
	int last_noise;
	__le16 last_seq_ctrl[NUM_RX_DATA_QUEUES];

	
	unsigned long tx_filtered_count;
	unsigned long tx_retry_failed, tx_retry_count;
	
	unsigned int fail_avg;

	
	unsigned long tx_packets;
	unsigned long tx_bytes;
	unsigned long tx_fragments;
	struct ieee80211_tx_rate last_tx_rate;
	u16 tid_seq[IEEE80211_QOS_CTL_TID_MASK + 1];

	
	struct sta_ampdu_mlme ampdu_mlme;
	u8 timer_to_tid[STA_TID_NUM];

#ifdef CONFIG_MAC80211_MESH
	
	__le16 llid;
	__le16 plid;
	__le16 reason;
	u8 plink_retries;
	bool ignore_plink_timer;
	bool plink_timer_was_running;
	enum plink_state plink_state;
	u32 plink_timeout;
	struct timer_list plink_timer;
#endif

#ifdef CONFIG_MAC80211_DEBUGFS
	struct sta_info_debugfsdentries {
		struct dentry *dir;
		struct dentry *flags;
		struct dentry *num_ps_buf_frames;
		struct dentry *inactive_ms;
		struct dentry *last_seq_ctrl;
		struct dentry *agg_status;
		struct dentry *aid;
		struct dentry *dev;
		struct dentry *rx_packets;
		struct dentry *tx_packets;
		struct dentry *rx_bytes;
		struct dentry *tx_bytes;
		struct dentry *rx_duplicates;
		struct dentry *rx_fragments;
		struct dentry *rx_dropped;
		struct dentry *tx_fragments;
		struct dentry *tx_filtered;
		struct dentry *tx_retry_failed;
		struct dentry *tx_retry_count;
		struct dentry *last_signal;
		struct dentry *last_qual;
		struct dentry *last_noise;
		struct dentry *wep_weak_iv_count;
		bool add_has_run;
	} debugfs;
#endif

	
	struct ieee80211_sta sta;
};

static inline enum plink_state sta_plink_state(struct sta_info *sta)
{
#ifdef CONFIG_MAC80211_MESH
	return sta->plink_state;
#endif
	return PLINK_LISTEN;
}

static inline void set_sta_flags(struct sta_info *sta, const u32 flags)
{
	unsigned long irqfl;

	spin_lock_irqsave(&sta->flaglock, irqfl);
	sta->flags |= flags;
	spin_unlock_irqrestore(&sta->flaglock, irqfl);
}

static inline void clear_sta_flags(struct sta_info *sta, const u32 flags)
{
	unsigned long irqfl;

	spin_lock_irqsave(&sta->flaglock, irqfl);
	sta->flags &= ~flags;
	spin_unlock_irqrestore(&sta->flaglock, irqfl);
}

static inline u32 test_sta_flags(struct sta_info *sta, const u32 flags)
{
	u32 ret;
	unsigned long irqfl;

	spin_lock_irqsave(&sta->flaglock, irqfl);
	ret = sta->flags & flags;
	spin_unlock_irqrestore(&sta->flaglock, irqfl);

	return ret;
}

static inline u32 test_and_clear_sta_flags(struct sta_info *sta,
					   const u32 flags)
{
	u32 ret;
	unsigned long irqfl;

	spin_lock_irqsave(&sta->flaglock, irqfl);
	ret = sta->flags & flags;
	sta->flags &= ~flags;
	spin_unlock_irqrestore(&sta->flaglock, irqfl);

	return ret;
}

static inline u32 get_sta_flags(struct sta_info *sta)
{
	u32 ret;
	unsigned long irqfl;

	spin_lock_irqsave(&sta->flaglock, irqfl);
	ret = sta->flags;
	spin_unlock_irqrestore(&sta->flaglock, irqfl);

	return ret;
}



#define STA_HASH_SIZE 256
#define STA_HASH(sta) (sta[5])



#define STA_MAX_TX_BUFFER 128


#define STA_TX_BUFFER_EXPIRE (10 * HZ)


#define STA_INFO_CLEANUP_INTERVAL (10 * HZ)


struct sta_info *sta_info_get(struct ieee80211_local *local, const u8 *addr);

struct sta_info *sta_info_get_by_idx(struct ieee80211_local *local, int idx,
				      struct net_device *dev);

struct sta_info *sta_info_alloc(struct ieee80211_sub_if_data *sdata,
				u8 *addr, gfp_t gfp);

int sta_info_insert(struct sta_info *sta);

void sta_info_unlink(struct sta_info **sta);

void sta_info_destroy(struct sta_info *sta);
void sta_info_set_tim_bit(struct sta_info *sta);
void sta_info_clear_tim_bit(struct sta_info *sta);

void sta_info_init(struct ieee80211_local *local);
int sta_info_start(struct ieee80211_local *local);
void sta_info_stop(struct ieee80211_local *local);
int sta_info_flush(struct ieee80211_local *local,
		   struct ieee80211_sub_if_data *sdata);
void ieee80211_sta_expire(struct ieee80211_sub_if_data *sdata,
			  unsigned long exp_time);

#endif 
