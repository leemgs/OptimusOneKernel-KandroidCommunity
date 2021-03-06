

#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/rtnetlink.h>
#include <net/mac80211.h>
#include "ieee80211_i.h"
#include "driver-ops.h"
#include "debugfs_key.h"
#include "aes_ccm.h"
#include "aes_cmac.h"




static const u8 bcast_addr[ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


static DEFINE_MUTEX(key_mutex);
static DEFINE_SPINLOCK(todo_lock);
static LIST_HEAD(todo_list);

static void key_todo(struct work_struct *work)
{
	ieee80211_key_todo();
}

static DECLARE_WORK(todo_work, key_todo);


static void add_todo(struct ieee80211_key *key, u32 flag)
{
	if (!key)
		return;

	spin_lock(&todo_lock);
	key->flags |= flag;
	
	if (!list_empty(&key->todo))
		list_del(&key->todo);
	list_add_tail(&key->todo, &todo_list);
	schedule_work(&todo_work);
	spin_unlock(&todo_lock);
}


static void ieee80211_key_lock(void)
{
	mutex_lock(&key_mutex);
}


static void ieee80211_key_unlock(void)
{
	mutex_unlock(&key_mutex);
}

static void assert_key_lock(void)
{
	WARN_ON(!mutex_is_locked(&key_mutex));
}

static struct ieee80211_sta *get_sta_for_key(struct ieee80211_key *key)
{
	if (key->sta)
		return &key->sta->sta;

	return NULL;
}

static void ieee80211_key_enable_hw_accel(struct ieee80211_key *key)
{
	struct ieee80211_sub_if_data *sdata;
	struct ieee80211_sta *sta;
	int ret;

	assert_key_lock();
	might_sleep();

	if (!key->local->ops->set_key)
		return;

	sta = get_sta_for_key(key);

	sdata = key->sdata;
	if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
		sdata = container_of(sdata->bss,
				     struct ieee80211_sub_if_data,
				     u.ap);

	ret = drv_set_key(key->local, SET_KEY, &sdata->vif, sta, &key->conf);

	if (!ret) {
		spin_lock_bh(&todo_lock);
		key->flags |= KEY_FLAG_UPLOADED_TO_HARDWARE;
		spin_unlock_bh(&todo_lock);
	}

	if (ret && ret != -ENOSPC && ret != -EOPNOTSUPP)
		printk(KERN_ERR "mac80211-%s: failed to set key "
		       "(%d, %pM) to hardware (%d)\n",
		       wiphy_name(key->local->hw.wiphy),
		       key->conf.keyidx, sta ? sta->addr : bcast_addr, ret);
}

static void ieee80211_key_disable_hw_accel(struct ieee80211_key *key)
{
	struct ieee80211_sub_if_data *sdata;
	struct ieee80211_sta *sta;
	int ret;

	assert_key_lock();
	might_sleep();

	if (!key || !key->local->ops->set_key)
		return;

	spin_lock_bh(&todo_lock);
	if (!(key->flags & KEY_FLAG_UPLOADED_TO_HARDWARE)) {
		spin_unlock_bh(&todo_lock);
		return;
	}
	spin_unlock_bh(&todo_lock);

	sta = get_sta_for_key(key);
	sdata = key->sdata;

	if (sdata->vif.type == NL80211_IFTYPE_AP_VLAN)
		sdata = container_of(sdata->bss,
				     struct ieee80211_sub_if_data,
				     u.ap);

	ret = drv_set_key(key->local, DISABLE_KEY, &sdata->vif,
			  sta, &key->conf);

	if (ret)
		printk(KERN_ERR "mac80211-%s: failed to remove key "
		       "(%d, %pM) from hardware (%d)\n",
		       wiphy_name(key->local->hw.wiphy),
		       key->conf.keyidx, sta ? sta->addr : bcast_addr, ret);

	spin_lock_bh(&todo_lock);
	key->flags &= ~KEY_FLAG_UPLOADED_TO_HARDWARE;
	spin_unlock_bh(&todo_lock);
}

static void __ieee80211_set_default_key(struct ieee80211_sub_if_data *sdata,
					int idx)
{
	struct ieee80211_key *key = NULL;

	if (idx >= 0 && idx < NUM_DEFAULT_KEYS)
		key = sdata->keys[idx];

	rcu_assign_pointer(sdata->default_key, key);

	if (key)
		add_todo(key, KEY_FLAG_TODO_DEFKEY);
}

void ieee80211_set_default_key(struct ieee80211_sub_if_data *sdata, int idx)
{
	unsigned long flags;

	spin_lock_irqsave(&sdata->local->key_lock, flags);
	__ieee80211_set_default_key(sdata, idx);
	spin_unlock_irqrestore(&sdata->local->key_lock, flags);
}

static void
__ieee80211_set_default_mgmt_key(struct ieee80211_sub_if_data *sdata, int idx)
{
	struct ieee80211_key *key = NULL;

	if (idx >= NUM_DEFAULT_KEYS &&
	    idx < NUM_DEFAULT_KEYS + NUM_DEFAULT_MGMT_KEYS)
		key = sdata->keys[idx];

	rcu_assign_pointer(sdata->default_mgmt_key, key);

	if (key)
		add_todo(key, KEY_FLAG_TODO_DEFMGMTKEY);
}

void ieee80211_set_default_mgmt_key(struct ieee80211_sub_if_data *sdata,
				    int idx)
{
	unsigned long flags;

	spin_lock_irqsave(&sdata->local->key_lock, flags);
	__ieee80211_set_default_mgmt_key(sdata, idx);
	spin_unlock_irqrestore(&sdata->local->key_lock, flags);
}


static void __ieee80211_key_replace(struct ieee80211_sub_if_data *sdata,
				    struct sta_info *sta,
				    struct ieee80211_key *old,
				    struct ieee80211_key *new)
{
	int idx, defkey, defmgmtkey;

	if (new)
		list_add(&new->list, &sdata->key_list);

	if (sta) {
		rcu_assign_pointer(sta->key, new);
	} else {
		WARN_ON(new && old && new->conf.keyidx != old->conf.keyidx);

		if (old)
			idx = old->conf.keyidx;
		else
			idx = new->conf.keyidx;

		defkey = old && sdata->default_key == old;
		defmgmtkey = old && sdata->default_mgmt_key == old;

		if (defkey && !new)
			__ieee80211_set_default_key(sdata, -1);
		if (defmgmtkey && !new)
			__ieee80211_set_default_mgmt_key(sdata, -1);

		rcu_assign_pointer(sdata->keys[idx], new);
		if (defkey && new)
			__ieee80211_set_default_key(sdata, new->conf.keyidx);
		if (defmgmtkey && new)
			__ieee80211_set_default_mgmt_key(sdata,
							 new->conf.keyidx);
	}

	if (old) {
		
		list_del_init(&old->list);
	}
}

struct ieee80211_key *ieee80211_key_alloc(enum ieee80211_key_alg alg,
					  int idx,
					  size_t key_len,
					  const u8 *key_data,
					  size_t seq_len, const u8 *seq)
{
	struct ieee80211_key *key;
	int i, j;

	BUG_ON(idx < 0 || idx >= NUM_DEFAULT_KEYS + NUM_DEFAULT_MGMT_KEYS);

	key = kzalloc(sizeof(struct ieee80211_key) + key_len, GFP_KERNEL);
	if (!key)
		return NULL;

	
	key->conf.flags = 0;
	key->flags = 0;

	key->conf.alg = alg;
	key->conf.keyidx = idx;
	key->conf.keylen = key_len;
	switch (alg) {
	case ALG_WEP:
		key->conf.iv_len = WEP_IV_LEN;
		key->conf.icv_len = WEP_ICV_LEN;
		break;
	case ALG_TKIP:
		key->conf.iv_len = TKIP_IV_LEN;
		key->conf.icv_len = TKIP_ICV_LEN;
		if (seq) {
			for (i = 0; i < NUM_RX_DATA_QUEUES; i++) {
				key->u.tkip.rx[i].iv32 =
					get_unaligned_le32(&seq[2]);
				key->u.tkip.rx[i].iv16 =
					get_unaligned_le16(seq);
			}
		}
		break;
	case ALG_CCMP:
		key->conf.iv_len = CCMP_HDR_LEN;
		key->conf.icv_len = CCMP_MIC_LEN;
		if (seq) {
			for (i = 0; i < NUM_RX_DATA_QUEUES; i++)
				for (j = 0; j < CCMP_PN_LEN; j++)
					key->u.ccmp.rx_pn[i][j] =
						seq[CCMP_PN_LEN - j - 1];
		}
		break;
	case ALG_AES_CMAC:
		key->conf.iv_len = 0;
		key->conf.icv_len = sizeof(struct ieee80211_mmie);
		if (seq)
			for (j = 0; j < 6; j++)
				key->u.aes_cmac.rx_pn[j] = seq[6 - j - 1];
		break;
	}
	memcpy(key->conf.key, key_data, key_len);
	INIT_LIST_HEAD(&key->list);
	INIT_LIST_HEAD(&key->todo);

	if (alg == ALG_CCMP) {
		
		key->u.ccmp.tfm = ieee80211_aes_key_setup_encrypt(key_data);
		if (!key->u.ccmp.tfm) {
			kfree(key);
			return NULL;
		}
	}

	if (alg == ALG_AES_CMAC) {
		
		key->u.aes_cmac.tfm =
			ieee80211_aes_cmac_key_setup(key_data);
		if (!key->u.aes_cmac.tfm) {
			kfree(key);
			return NULL;
		}
	}

	return key;
}

void ieee80211_key_link(struct ieee80211_key *key,
			struct ieee80211_sub_if_data *sdata,
			struct sta_info *sta)
{
	struct ieee80211_key *old_key;
	unsigned long flags;
	int idx;

	BUG_ON(!sdata);
	BUG_ON(!key);

	idx = key->conf.keyidx;
	key->local = sdata->local;
	key->sdata = sdata;
	key->sta = sta;

	if (sta) {
		
		if (test_sta_flags(sta, WLAN_STA_WME))
			key->conf.flags |= IEEE80211_KEY_FLAG_WMM_STA;

		
		key->conf.flags |= IEEE80211_KEY_FLAG_PAIRWISE;
	} else {
		if (sdata->vif.type == NL80211_IFTYPE_STATION) {
			struct sta_info *ap;

			

			
			ap = sta_info_get(key->local, key->sdata->u.mgd.bssid);
			if (ap) {
				if (test_sta_flags(ap, WLAN_STA_WME))
					key->conf.flags |=
						IEEE80211_KEY_FLAG_WMM_STA;
			}
		}
	}

	spin_lock_irqsave(&sdata->local->key_lock, flags);

	if (sta)
		old_key = sta->key;
	else
		old_key = sdata->keys[idx];

	__ieee80211_key_replace(sdata, sta, old_key, key);

	
	add_todo(old_key, KEY_FLAG_TODO_DELETE);

	add_todo(key, KEY_FLAG_TODO_ADD_DEBUGFS);
	if (netif_running(sdata->dev))
		add_todo(key, KEY_FLAG_TODO_HWACCEL_ADD);

	spin_unlock_irqrestore(&sdata->local->key_lock, flags);
}

static void __ieee80211_key_free(struct ieee80211_key *key)
{
	
	if (key->sdata)
		__ieee80211_key_replace(key->sdata, key->sta,
					key, NULL);

	add_todo(key, KEY_FLAG_TODO_DELETE);
}

void ieee80211_key_free(struct ieee80211_key *key)
{
	unsigned long flags;

	if (!key)
		return;

	if (!key->sdata) {
		
		if (key->conf.alg == ALG_CCMP)
			ieee80211_aes_key_free(key->u.ccmp.tfm);
		kfree(key);
		return;
	}

	spin_lock_irqsave(&key->sdata->local->key_lock, flags);
	__ieee80211_key_free(key);
	spin_unlock_irqrestore(&key->sdata->local->key_lock, flags);
}


static void ieee80211_todo_for_each_key(struct ieee80211_sub_if_data *sdata,
					u32 todo_flags)
{
	struct ieee80211_key *key;
	unsigned long flags;

	might_sleep();

	spin_lock_irqsave(&sdata->local->key_lock, flags);
	list_for_each_entry(key, &sdata->key_list, list)
		add_todo(key, todo_flags);
	spin_unlock_irqrestore(&sdata->local->key_lock, flags);

	ieee80211_key_todo();
}

void ieee80211_enable_keys(struct ieee80211_sub_if_data *sdata)
{
	ASSERT_RTNL();

	if (WARN_ON(!netif_running(sdata->dev)))
		return;

	ieee80211_todo_for_each_key(sdata, KEY_FLAG_TODO_HWACCEL_ADD);
}

void ieee80211_disable_keys(struct ieee80211_sub_if_data *sdata)
{
	ASSERT_RTNL();

	ieee80211_todo_for_each_key(sdata, KEY_FLAG_TODO_HWACCEL_REMOVE);
}

static void __ieee80211_key_destroy(struct ieee80211_key *key)
{
	if (!key)
		return;

	ieee80211_key_disable_hw_accel(key);

	if (key->conf.alg == ALG_CCMP)
		ieee80211_aes_key_free(key->u.ccmp.tfm);
	if (key->conf.alg == ALG_AES_CMAC)
		ieee80211_aes_cmac_key_free(key->u.aes_cmac.tfm);
	ieee80211_debugfs_key_remove(key);

	kfree(key);
}

static void __ieee80211_key_todo(void)
{
	struct ieee80211_key *key;
	bool work_done;
	u32 todoflags;

	
	synchronize_rcu();

	spin_lock_bh(&todo_lock);
	while (!list_empty(&todo_list)) {
		key = list_first_entry(&todo_list, struct ieee80211_key, todo);
		list_del_init(&key->todo);
		todoflags = key->flags & (KEY_FLAG_TODO_ADD_DEBUGFS |
					  KEY_FLAG_TODO_DEFKEY |
					  KEY_FLAG_TODO_DEFMGMTKEY |
					  KEY_FLAG_TODO_HWACCEL_ADD |
					  KEY_FLAG_TODO_HWACCEL_REMOVE |
					  KEY_FLAG_TODO_DELETE);
		key->flags &= ~todoflags;
		spin_unlock_bh(&todo_lock);

		work_done = false;

		if (todoflags & KEY_FLAG_TODO_ADD_DEBUGFS) {
			ieee80211_debugfs_key_add(key);
			work_done = true;
		}
		if (todoflags & KEY_FLAG_TODO_DEFKEY) {
			ieee80211_debugfs_key_remove_default(key->sdata);
			ieee80211_debugfs_key_add_default(key->sdata);
			work_done = true;
		}
		if (todoflags & KEY_FLAG_TODO_DEFMGMTKEY) {
			ieee80211_debugfs_key_remove_mgmt_default(key->sdata);
			ieee80211_debugfs_key_add_mgmt_default(key->sdata);
			work_done = true;
		}
		if (todoflags & KEY_FLAG_TODO_HWACCEL_ADD) {
			ieee80211_key_enable_hw_accel(key);
			work_done = true;
		}
		if (todoflags & KEY_FLAG_TODO_HWACCEL_REMOVE) {
			ieee80211_key_disable_hw_accel(key);
			work_done = true;
		}
		if (todoflags & KEY_FLAG_TODO_DELETE) {
			__ieee80211_key_destroy(key);
			work_done = true;
		}

		WARN_ON(!work_done);

		spin_lock_bh(&todo_lock);
	}
	spin_unlock_bh(&todo_lock);
}

void ieee80211_key_todo(void)
{
	ieee80211_key_lock();
	__ieee80211_key_todo();
	ieee80211_key_unlock();
}

void ieee80211_free_keys(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_key *key, *tmp;
	unsigned long flags;

	ieee80211_key_lock();

	ieee80211_debugfs_key_remove_default(sdata);
	ieee80211_debugfs_key_remove_mgmt_default(sdata);

	spin_lock_irqsave(&sdata->local->key_lock, flags);
	list_for_each_entry_safe(key, tmp, &sdata->key_list, list)
		__ieee80211_key_free(key);
	spin_unlock_irqrestore(&sdata->local->key_lock, flags);

	__ieee80211_key_todo();

	ieee80211_key_unlock();
}
