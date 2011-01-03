

#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/timer.h>
#include <linux/rtnetlink.h>

#include <net/mac80211.h>
#include "ieee80211_i.h"
#include "driver-ops.h"
#include "rate.h"
#include "sta_info.h"
#include "debugfs_sta.h"
#include "mesh.h"




static int sta_info_hash_del(struct ieee80211_local *local,
			     struct sta_info *sta)
{
	struct sta_info *s;

	s = local->sta_hash[STA_HASH(sta->sta.addr)];
	if (!s)
		return -ENOENT;
	if (s == sta) {
		rcu_assign_pointer(local->sta_hash[STA_HASH(sta->sta.addr)],
				   s->hnext);
		return 0;
	}

	while (s->hnext && s->hnext != sta)
		s = s->hnext;
	if (s->hnext) {
		rcu_assign_pointer(s->hnext, sta->hnext);
		return 0;
	}

	return -ENOENT;
}


struct sta_info *sta_info_get(struct ieee80211_local *local, const u8 *addr)
{
	struct sta_info *sta;

	sta = rcu_dereference(local->sta_hash[STA_HASH(addr)]);
	while (sta) {
		if (memcmp(sta->sta.addr, addr, ETH_ALEN) == 0)
			break;
		sta = rcu_dereference(sta->hnext);
	}
	return sta;
}

struct sta_info *sta_info_get_by_idx(struct ieee80211_local *local, int idx,
				     struct net_device *dev)
{
	struct sta_info *sta;
	int i = 0;

	list_for_each_entry_rcu(sta, &local->sta_list, list) {
		if (dev && dev != sta->sdata->dev)
			continue;
		if (i < idx) {
			++i;
			continue;
		}
		return sta;
	}

	return NULL;
}


static void __sta_info_free(struct ieee80211_local *local,
			    struct sta_info *sta)
{
	rate_control_free_sta(sta);
	rate_control_put(sta->rate_ctrl);

#ifdef CONFIG_MAC80211_VERBOSE_DEBUG
	printk(KERN_DEBUG "%s: Destroyed STA %pM\n",
	       wiphy_name(local->hw.wiphy), sta->sta.addr);
#endif 

	kfree(sta);
}

void sta_info_destroy(struct sta_info *sta)
{
	struct ieee80211_local *local;
	struct sk_buff *skb;
	int i;

	might_sleep();

	if (!sta)
		return;

	local = sta->local;

	rate_control_remove_sta_debugfs(sta);
	ieee80211_sta_debugfs_remove(sta);

#ifdef CONFIG_MAC80211_MESH
	if (ieee80211_vif_is_mesh(&sta->sdata->vif))
		mesh_plink_deactivate(sta);
#endif

	
	ieee80211_key_todo();

#ifdef CONFIG_MAC80211_MESH
	if (ieee80211_vif_is_mesh(&sta->sdata->vif))
		del_timer_sync(&sta->plink_timer);
#endif

	while ((skb = skb_dequeue(&sta->ps_tx_buf)) != NULL) {
		local->total_ps_buffered--;
		dev_kfree_skb_any(skb);
	}

	while ((skb = skb_dequeue(&sta->tx_filtered)) != NULL)
		dev_kfree_skb_any(skb);

	for (i = 0; i <  STA_TID_NUM; i++) {
		struct tid_ampdu_rx *tid_rx;
		struct tid_ampdu_tx *tid_tx;

		spin_lock_bh(&sta->lock);
		tid_rx = sta->ampdu_mlme.tid_rx[i];
		
		if (tid_rx)
			tid_rx->shutdown = true;

		spin_unlock_bh(&sta->lock);

		
		if (tid_rx) {
			del_timer_sync(&tid_rx->session_timer);
			kfree(tid_rx);
		}

		
		tid_tx = sta->ampdu_mlme.tid_tx[i];
		if (tid_tx) {
			del_timer_sync(&tid_tx->addba_resp_timer);
			
			skb_queue_purge(&tid_tx->pending);
			kfree(tid_tx);
		}
	}

	__sta_info_free(local, sta);
}



static void sta_info_hash_add(struct ieee80211_local *local,
			      struct sta_info *sta)
{
	sta->hnext = local->sta_hash[STA_HASH(sta->sta.addr)];
	rcu_assign_pointer(local->sta_hash[STA_HASH(sta->sta.addr)], sta);
}

struct sta_info *sta_info_alloc(struct ieee80211_sub_if_data *sdata,
				u8 *addr, gfp_t gfp)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta;
	int i;

	sta = kzalloc(sizeof(*sta) + local->hw.sta_data_size, gfp);
	if (!sta)
		return NULL;

	spin_lock_init(&sta->lock);
	spin_lock_init(&sta->flaglock);

	memcpy(sta->sta.addr, addr, ETH_ALEN);
	sta->local = local;
	sta->sdata = sdata;

	sta->rate_ctrl = rate_control_get(local->rate_ctrl);
	sta->rate_ctrl_priv = rate_control_alloc_sta(sta->rate_ctrl,
						     &sta->sta, gfp);
	if (!sta->rate_ctrl_priv) {
		rate_control_put(sta->rate_ctrl);
		kfree(sta);
		return NULL;
	}

	for (i = 0; i < STA_TID_NUM; i++) {
		
		sta->timer_to_tid[i] = i;
		
		sta->ampdu_mlme.tid_state_rx[i] = HT_AGG_STATE_IDLE;
		sta->ampdu_mlme.tid_rx[i] = NULL;
		
		sta->ampdu_mlme.tid_state_tx[i] = HT_AGG_STATE_IDLE;
		sta->ampdu_mlme.tid_tx[i] = NULL;
		sta->ampdu_mlme.addba_req_num[i] = 0;
	}
	skb_queue_head_init(&sta->ps_tx_buf);
	skb_queue_head_init(&sta->tx_filtered);

	for (i = 0; i < NUM_RX_DATA_QUEUES; i++)
		sta->last_seq_ctrl[i] = cpu_to_le16(USHORT_MAX);

#ifdef CONFIG_MAC80211_VERBOSE_DEBUG
	printk(KERN_DEBUG "%s: Allocated STA %pM\n",
	       wiphy_name(local->hw.wiphy), sta->sta.addr);
#endif 

#ifdef CONFIG_MAC80211_MESH
	sta->plink_state = PLINK_LISTEN;
	init_timer(&sta->plink_timer);
#endif

	return sta;
}

int sta_info_insert(struct sta_info *sta)
{
	struct ieee80211_local *local = sta->local;
	struct ieee80211_sub_if_data *sdata = sta->sdata;
	unsigned long flags;
	int err = 0;

	
	if (unlikely(!netif_running(sdata->dev))) {
		err = -ENETDOWN;
		goto out_free;
	}

	if (WARN_ON(compare_ether_addr(sta->sta.addr, sdata->dev->dev_addr) == 0 ||
		    is_multicast_ether_addr(sta->sta.addr))) {
		err = -EINVAL;
		goto out_free;
	}

	spin_lock_irqsave(&local->sta_lock, flags);
	
	if (sta_info_get(local, sta->sta.addr)) {
		spin_unlock_irqrestore(&local->sta_lock, flags);
		err = -EEXIST;
		goto out_free;
	}
	list_add(&sta->list, &local->sta_list);
	local->sta_generation++;
	local->num_sta++;
	sta_info_hash_add(local, sta);

	
	if (local->ops->sta_notify) {
		if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
			sdata = container_of(sdata->bss,
					     struct ieee80211_sub_if_data,
					     u.ap);

		drv_sta_notify(local, &sdata->vif, STA_NOTIFY_ADD, &sta->sta);
		sdata = sta->sdata;
	}

#ifdef CONFIG_MAC80211_VERBOSE_DEBUG
	printk(KERN_DEBUG "%s: Inserted STA %pM\n",
	       wiphy_name(local->hw.wiphy), sta->sta.addr);
#endif 

	spin_unlock_irqrestore(&local->sta_lock, flags);

#ifdef CONFIG_MAC80211_DEBUGFS
	
	schedule_work(&local->sta_debugfs_add);
#endif

	if (ieee80211_vif_is_mesh(&sdata->vif))
		mesh_accept_plinks_update(sdata);

	return 0;
 out_free:
	BUG_ON(!err);
	__sta_info_free(local, sta);
	return err;
}

static inline void __bss_tim_set(struct ieee80211_if_ap *bss, u16 aid)
{
	
	bss->tim[aid / 8] |= (1 << (aid % 8));
}

static inline void __bss_tim_clear(struct ieee80211_if_ap *bss, u16 aid)
{
	
	bss->tim[aid / 8] &= ~(1 << (aid % 8));
}

static void __sta_info_set_tim_bit(struct ieee80211_if_ap *bss,
				   struct sta_info *sta)
{
	BUG_ON(!bss);

	__bss_tim_set(bss, sta->sta.aid);

	if (sta->local->ops->set_tim) {
		sta->local->tim_in_locked_section = true;
		drv_set_tim(sta->local, &sta->sta, true);
		sta->local->tim_in_locked_section = false;
	}
}

void sta_info_set_tim_bit(struct sta_info *sta)
{
	unsigned long flags;

	BUG_ON(!sta->sdata->bss);

	spin_lock_irqsave(&sta->local->sta_lock, flags);
	__sta_info_set_tim_bit(sta->sdata->bss, sta);
	spin_unlock_irqrestore(&sta->local->sta_lock, flags);
}

static void __sta_info_clear_tim_bit(struct ieee80211_if_ap *bss,
				     struct sta_info *sta)
{
	BUG_ON(!bss);

	__bss_tim_clear(bss, sta->sta.aid);

	if (sta->local->ops->set_tim) {
		sta->local->tim_in_locked_section = true;
		drv_set_tim(sta->local, &sta->sta, false);
		sta->local->tim_in_locked_section = false;
	}
}

void sta_info_clear_tim_bit(struct sta_info *sta)
{
	unsigned long flags;

	BUG_ON(!sta->sdata->bss);

	spin_lock_irqsave(&sta->local->sta_lock, flags);
	__sta_info_clear_tim_bit(sta->sdata->bss, sta);
	spin_unlock_irqrestore(&sta->local->sta_lock, flags);
}

static void __sta_info_unlink(struct sta_info **sta)
{
	struct ieee80211_local *local = (*sta)->local;
	struct ieee80211_sub_if_data *sdata = (*sta)->sdata;
	
	if (sta_info_hash_del(local, *sta)) {
		*sta = NULL;
		return;
	}

	if ((*sta)->key) {
		ieee80211_key_free((*sta)->key);
		WARN_ON((*sta)->key);
	}

	list_del(&(*sta)->list);

	if (test_and_clear_sta_flags(*sta, WLAN_STA_PS)) {
		BUG_ON(!sdata->bss);

		atomic_dec(&sdata->bss->num_sta_ps);
		__sta_info_clear_tim_bit(sdata->bss, *sta);
	}

	local->num_sta--;
	local->sta_generation++;

	if (local->ops->sta_notify) {
		if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
			sdata = container_of(sdata->bss,
					     struct ieee80211_sub_if_data,
					     u.ap);

		drv_sta_notify(local, &sdata->vif, STA_NOTIFY_REMOVE,
			       &(*sta)->sta);
		sdata = (*sta)->sdata;
	}

	if (ieee80211_vif_is_mesh(&sdata->vif)) {
		mesh_accept_plinks_update(sdata);
#ifdef CONFIG_MAC80211_MESH
		del_timer(&(*sta)->plink_timer);
#endif
	}

#ifdef CONFIG_MAC80211_VERBOSE_DEBUG
	printk(KERN_DEBUG "%s: Removed STA %pM\n",
	       wiphy_name(local->hw.wiphy), (*sta)->sta.addr);
#endif 

	
	if ((*sta)->pin_status == STA_INFO_PIN_STAT_PINNED) {
		(*sta)->pin_status = STA_INFO_PIN_STAT_DESTROY;
		*sta = NULL;
		return;
	}
}

void sta_info_unlink(struct sta_info **sta)
{
	struct ieee80211_local *local = (*sta)->local;
	unsigned long flags;

	spin_lock_irqsave(&local->sta_lock, flags);
	__sta_info_unlink(sta);
	spin_unlock_irqrestore(&local->sta_lock, flags);
}

static int sta_info_buffer_expired(struct sta_info *sta,
				   struct sk_buff *skb)
{
	struct ieee80211_tx_info *info;
	int timeout;

	if (!skb)
		return 0;

	info = IEEE80211_SKB_CB(skb);

	
	timeout = (sta->listen_interval *
		   sta->sdata->vif.bss_conf.beacon_int *
		   32 / 15625) * HZ;
	if (timeout < STA_TX_BUFFER_EXPIRE)
		timeout = STA_TX_BUFFER_EXPIRE;
	return time_after(jiffies, info->control.jiffies + timeout);
}


static void sta_info_cleanup_expire_buffered(struct ieee80211_local *local,
					     struct sta_info *sta)
{
	unsigned long flags;
	struct sk_buff *skb;
	struct ieee80211_sub_if_data *sdata;

	if (skb_queue_empty(&sta->ps_tx_buf))
		return;

	for (;;) {
		spin_lock_irqsave(&sta->ps_tx_buf.lock, flags);
		skb = skb_peek(&sta->ps_tx_buf);
		if (sta_info_buffer_expired(sta, skb))
			skb = __skb_dequeue(&sta->ps_tx_buf);
		else
			skb = NULL;
		spin_unlock_irqrestore(&sta->ps_tx_buf.lock, flags);

		if (!skb)
			break;

		sdata = sta->sdata;
		local->total_ps_buffered--;
#ifdef CONFIG_MAC80211_VERBOSE_PS_DEBUG
		printk(KERN_DEBUG "Buffered frame expired (STA %pM)\n",
		       sta->sta.addr);
#endif
		dev_kfree_skb(skb);

		if (skb_queue_empty(&sta->ps_tx_buf))
			sta_info_clear_tim_bit(sta);
	}
}


static void sta_info_cleanup(unsigned long data)
{
	struct ieee80211_local *local = (struct ieee80211_local *) data;
	struct sta_info *sta;

	rcu_read_lock();
	list_for_each_entry_rcu(sta, &local->sta_list, list)
		sta_info_cleanup_expire_buffered(local, sta);
	rcu_read_unlock();

	if (local->quiescing)
		return;

	local->sta_cleanup.expires =
		round_jiffies(jiffies + STA_INFO_CLEANUP_INTERVAL);
	add_timer(&local->sta_cleanup);
}

#ifdef CONFIG_MAC80211_DEBUGFS

static void __sta_info_pin(struct sta_info *sta)
{
	WARN_ON(sta->pin_status != STA_INFO_PIN_STAT_NORMAL);
	sta->pin_status = STA_INFO_PIN_STAT_PINNED;
}


static struct sta_info *__sta_info_unpin(struct sta_info *sta)
{
	struct sta_info *ret = NULL;
	unsigned long flags;

	spin_lock_irqsave(&sta->local->sta_lock, flags);
	WARN_ON(sta->pin_status != STA_INFO_PIN_STAT_DESTROY &&
		sta->pin_status != STA_INFO_PIN_STAT_PINNED);
	if (sta->pin_status == STA_INFO_PIN_STAT_DESTROY)
		ret = sta;
	sta->pin_status = STA_INFO_PIN_STAT_NORMAL;
	spin_unlock_irqrestore(&sta->local->sta_lock, flags);

	return ret;
}

static void sta_info_debugfs_add_work(struct work_struct *work)
{
	struct ieee80211_local *local =
		container_of(work, struct ieee80211_local, sta_debugfs_add);
	struct sta_info *sta, *tmp;
	unsigned long flags;

	
	rtnl_lock();
	while (1) {
		sta = NULL;

		spin_lock_irqsave(&local->sta_lock, flags);
		list_for_each_entry(tmp, &local->sta_list, list) {
			
			if (!tmp->debugfs.add_has_run) {
				sta = tmp;
				__sta_info_pin(sta);
				break;
			}
		}
		spin_unlock_irqrestore(&local->sta_lock, flags);

		if (!sta)
			break;

		ieee80211_sta_debugfs_add(sta);
		rate_control_add_sta_debugfs(sta);

		sta = __sta_info_unpin(sta);
		sta_info_destroy(sta);
	}
	rtnl_unlock();
}
#endif

void sta_info_init(struct ieee80211_local *local)
{
	spin_lock_init(&local->sta_lock);
	INIT_LIST_HEAD(&local->sta_list);

	setup_timer(&local->sta_cleanup, sta_info_cleanup,
		    (unsigned long)local);
	local->sta_cleanup.expires =
		round_jiffies(jiffies + STA_INFO_CLEANUP_INTERVAL);

#ifdef CONFIG_MAC80211_DEBUGFS
	INIT_WORK(&local->sta_debugfs_add, sta_info_debugfs_add_work);
#endif
}

int sta_info_start(struct ieee80211_local *local)
{
	add_timer(&local->sta_cleanup);
	return 0;
}

void sta_info_stop(struct ieee80211_local *local)
{
	del_timer(&local->sta_cleanup);
#ifdef CONFIG_MAC80211_DEBUGFS
	
	cancel_work_sync(&local->sta_debugfs_add);
#endif

	sta_info_flush(local, NULL);
}


int sta_info_flush(struct ieee80211_local *local,
		   struct ieee80211_sub_if_data *sdata)
{
	struct sta_info *sta, *tmp;
	LIST_HEAD(tmp_list);
	int ret = 0;
	unsigned long flags;

	might_sleep();

	spin_lock_irqsave(&local->sta_lock, flags);
	list_for_each_entry_safe(sta, tmp, &local->sta_list, list) {
		if (!sdata || sdata == sta->sdata) {
			__sta_info_unlink(&sta);
			if (sta) {
				list_add_tail(&sta->list, &tmp_list);
				ret++;
			}
		}
	}
	spin_unlock_irqrestore(&local->sta_lock, flags);

	list_for_each_entry_safe(sta, tmp, &tmp_list, list)
		sta_info_destroy(sta);

	return ret;
}

void ieee80211_sta_expire(struct ieee80211_sub_if_data *sdata,
			  unsigned long exp_time)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta, *tmp;
	LIST_HEAD(tmp_list);
	unsigned long flags;

	spin_lock_irqsave(&local->sta_lock, flags);
	list_for_each_entry_safe(sta, tmp, &local->sta_list, list)
		if (time_after(jiffies, sta->last_rx + exp_time)) {
#ifdef CONFIG_MAC80211_IBSS_DEBUG
			printk(KERN_DEBUG "%s: expiring inactive STA %pM\n",
			       sdata->dev->name, sta->sta.addr);
#endif
			__sta_info_unlink(&sta);
			if (sta)
				list_add(&sta->list, &tmp_list);
		}
	spin_unlock_irqrestore(&local->sta_lock, flags);

	list_for_each_entry_safe(sta, tmp, &tmp_list, list)
		sta_info_destroy(sta);
}

struct ieee80211_sta *ieee80211_find_sta(struct ieee80211_hw *hw,
					 const u8 *addr)
{
	struct sta_info *sta = sta_info_get(hw_to_local(hw), addr);

	if (!sta)
		return NULL;
	return &sta->sta;
}
EXPORT_SYMBOL(ieee80211_find_sta);
