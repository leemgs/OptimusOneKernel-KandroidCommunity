



#include <linux/wireless.h>
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>
#include <net/mac80211.h>

#include "ieee80211_i.h"
#include "driver-ops.h"
#include "mesh.h"

#define IEEE80211_PROBE_DELAY (HZ / 33)
#define IEEE80211_CHANNEL_TIME (HZ / 33)
#define IEEE80211_PASSIVE_CHANNEL_TIME (HZ / 8)

struct ieee80211_bss *
ieee80211_rx_bss_get(struct ieee80211_local *local, u8 *bssid, int freq,
		     u8 *ssid, u8 ssid_len)
{
	return (void *)cfg80211_get_bss(local->hw.wiphy,
					ieee80211_get_channel(local->hw.wiphy,
							      freq),
					bssid, ssid, ssid_len,
					0, 0);
}

static void ieee80211_rx_bss_free(struct cfg80211_bss *cbss)
{
	struct ieee80211_bss *bss = (void *)cbss;

	kfree(bss_mesh_id(bss));
	kfree(bss_mesh_cfg(bss));
}

void ieee80211_rx_bss_put(struct ieee80211_local *local,
			  struct ieee80211_bss *bss)
{
	cfg80211_put_bss((struct cfg80211_bss *)bss);
}

struct ieee80211_bss *
ieee80211_bss_info_update(struct ieee80211_local *local,
			  struct ieee80211_rx_status *rx_status,
			  struct ieee80211_mgmt *mgmt,
			  size_t len,
			  struct ieee802_11_elems *elems,
			  struct ieee80211_channel *channel,
			  bool beacon)
{
	struct ieee80211_bss *bss;
	int clen;
	s32 signal = 0;

	if (local->hw.flags & IEEE80211_HW_SIGNAL_DBM)
		signal = rx_status->signal * 100;
	else if (local->hw.flags & IEEE80211_HW_SIGNAL_UNSPEC)
		signal = (rx_status->signal * 100) / local->hw.max_signal;

	bss = (void *)cfg80211_inform_bss_frame(local->hw.wiphy, channel,
						mgmt, len, signal, GFP_ATOMIC);

	if (!bss)
		return NULL;

	bss->cbss.free_priv = ieee80211_rx_bss_free;

	
	if (elems->erp_info && elems->erp_info_len >= 1) {
		bss->erp_value = elems->erp_info[0];
		bss->has_erp_value = 1;
	}

	if (elems->tim) {
		struct ieee80211_tim_ie *tim_ie =
			(struct ieee80211_tim_ie *)elems->tim;
		bss->dtim_period = tim_ie->dtim_period;
	}

	
	if (bss->dtim_period == 0)
		bss->dtim_period = 1;

	bss->supp_rates_len = 0;
	if (elems->supp_rates) {
		clen = IEEE80211_MAX_SUPP_RATES - bss->supp_rates_len;
		if (clen > elems->supp_rates_len)
			clen = elems->supp_rates_len;
		memcpy(&bss->supp_rates[bss->supp_rates_len], elems->supp_rates,
		       clen);
		bss->supp_rates_len += clen;
	}
	if (elems->ext_supp_rates) {
		clen = IEEE80211_MAX_SUPP_RATES - bss->supp_rates_len;
		if (clen > elems->ext_supp_rates_len)
			clen = elems->ext_supp_rates_len;
		memcpy(&bss->supp_rates[bss->supp_rates_len],
		       elems->ext_supp_rates, clen);
		bss->supp_rates_len += clen;
	}

	bss->wmm_used = elems->wmm_param || elems->wmm_info;

	if (!beacon)
		bss->last_probe_resp = jiffies;

	return bss;
}

ieee80211_rx_result
ieee80211_scan_rx(struct ieee80211_sub_if_data *sdata, struct sk_buff *skb)
{
	struct ieee80211_rx_status *rx_status = IEEE80211_SKB_RXCB(skb);
	struct ieee80211_mgmt *mgmt;
	struct ieee80211_bss *bss;
	u8 *elements;
	struct ieee80211_channel *channel;
	size_t baselen;
	int freq;
	__le16 fc;
	bool presp, beacon = false;
	struct ieee802_11_elems elems;

	if (skb->len < 2)
		return RX_DROP_UNUSABLE;

	mgmt = (struct ieee80211_mgmt *) skb->data;
	fc = mgmt->frame_control;

	if (ieee80211_is_ctl(fc))
		return RX_CONTINUE;

	if (skb->len < 24)
		return RX_DROP_MONITOR;

	presp = ieee80211_is_probe_resp(fc);
	if (presp) {
		
		if (memcmp(mgmt->da, sdata->dev->dev_addr, ETH_ALEN))
			return RX_DROP_MONITOR;

		presp = true;
		elements = mgmt->u.probe_resp.variable;
		baselen = offsetof(struct ieee80211_mgmt, u.probe_resp.variable);
	} else {
		beacon = ieee80211_is_beacon(fc);
		baselen = offsetof(struct ieee80211_mgmt, u.beacon.variable);
		elements = mgmt->u.beacon.variable;
	}

	if (!presp && !beacon)
		return RX_CONTINUE;

	if (baselen > skb->len)
		return RX_DROP_MONITOR;

	ieee802_11_parse_elems(elements, skb->len - baselen, &elems);

	if (elems.ds_params && elems.ds_params_len == 1)
		freq = ieee80211_channel_to_frequency(elems.ds_params[0]);
	else
		freq = rx_status->freq;

	channel = ieee80211_get_channel(sdata->local->hw.wiphy, freq);

	if (!channel || channel->flags & IEEE80211_CHAN_DISABLED)
		return RX_DROP_MONITOR;

	bss = ieee80211_bss_info_update(sdata->local, rx_status,
					mgmt, skb->len, &elems,
					channel, beacon);
	if (bss)
		ieee80211_rx_bss_put(sdata->local, bss);

	dev_kfree_skb(skb);
	return RX_QUEUED;
}


static void ieee80211_scan_ps_enable(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;

	local->scan_ps_enabled = false;

	

	del_timer_sync(&local->dynamic_ps_timer);
	cancel_work_sync(&local->dynamic_ps_enable_work);

	if (local->hw.conf.flags & IEEE80211_CONF_PS) {
		local->scan_ps_enabled = true;
		local->hw.conf.flags &= ~IEEE80211_CONF_PS;
		ieee80211_hw_config(local, IEEE80211_CONF_CHANGE_PS);
	}

	if (!(local->scan_ps_enabled) ||
	    !(local->hw.flags & IEEE80211_HW_PS_NULLFUNC_STACK))
		
		ieee80211_send_nullfunc(local, sdata, 1);
}


static void ieee80211_scan_ps_disable(struct ieee80211_sub_if_data *sdata)
{
	struct ieee80211_local *local = sdata->local;

	if (!local->ps_sdata)
		ieee80211_send_nullfunc(local, sdata, 0);
	else if (local->scan_ps_enabled) {
		
		local->hw.conf.flags |= IEEE80211_CONF_PS;
		ieee80211_hw_config(local, IEEE80211_CONF_CHANGE_PS);
	} else if (local->hw.conf.dynamic_ps_timeout > 0) {
		
		ieee80211_send_nullfunc(local, sdata, 0);
		mod_timer(&local->dynamic_ps_timer, jiffies +
			  msecs_to_jiffies(local->hw.conf.dynamic_ps_timeout));
	}
}

static void ieee80211_restore_scan_ies(struct ieee80211_local *local)
{
	kfree(local->scan_req->ie);
	local->scan_req->ie = local->orig_ies;
	local->scan_req->ie_len = local->orig_ies_len;
}

void ieee80211_scan_completed(struct ieee80211_hw *hw, bool aborted)
{
	struct ieee80211_local *local = hw_to_local(hw);
	struct ieee80211_sub_if_data *sdata;
	bool was_hw_scan;

	mutex_lock(&local->scan_mtx);

	
	if (WARN_ON(!local->scanning && !aborted))
		aborted = true;

	if (WARN_ON(!local->scan_req)) {
		mutex_unlock(&local->scan_mtx);
		return;
	}

	if (test_bit(SCAN_HW_SCANNING, &local->scanning))
		ieee80211_restore_scan_ies(local);

	if (local->scan_req != local->int_scan_req)
		cfg80211_scan_done(local->scan_req, aborted);
	local->scan_req = NULL;
	local->scan_sdata = NULL;

	was_hw_scan = test_bit(SCAN_HW_SCANNING, &local->scanning);
	local->scanning = 0;
	local->scan_channel = NULL;

	
	mutex_unlock(&local->scan_mtx);

	ieee80211_hw_config(local, IEEE80211_CONF_CHANGE_CHANNEL);
	if (was_hw_scan)
		goto done;

	ieee80211_configure_filter(local);

	drv_sw_scan_complete(local);

	mutex_lock(&local->iflist_mtx);
	list_for_each_entry(sdata, &local->interfaces, list) {
		if (!netif_running(sdata->dev))
			continue;

		
		if (sdata->vif.type == NL80211_IFTYPE_STATION) {
			if (sdata->u.mgd.associated) {
				ieee80211_scan_ps_disable(sdata);
				netif_tx_wake_all_queues(sdata->dev);
			}
		} else
			netif_tx_wake_all_queues(sdata->dev);

		
		if (sdata->vif.type == NL80211_IFTYPE_AP ||
		    sdata->vif.type == NL80211_IFTYPE_ADHOC ||
		    sdata->vif.type == NL80211_IFTYPE_MESH_POINT)
			ieee80211_bss_info_change_notify(
				sdata, BSS_CHANGED_BEACON_ENABLED);
	}
	mutex_unlock(&local->iflist_mtx);

 done:
	ieee80211_recalc_idle(local);
	ieee80211_mlme_notify_scan_completed(local);
	ieee80211_ibss_notify_scan_completed(local);
	ieee80211_mesh_notify_scan_completed(local);
}
EXPORT_SYMBOL(ieee80211_scan_completed);

static int ieee80211_start_sw_scan(struct ieee80211_local *local)
{
	struct ieee80211_sub_if_data *sdata;

	
	drv_sw_scan_start(local);

	mutex_lock(&local->iflist_mtx);
	list_for_each_entry(sdata, &local->interfaces, list) {
		if (!netif_running(sdata->dev))
			continue;

		
		if (sdata->vif.type == NL80211_IFTYPE_AP ||
		    sdata->vif.type == NL80211_IFTYPE_ADHOC ||
		    sdata->vif.type == NL80211_IFTYPE_MESH_POINT)
			ieee80211_bss_info_change_notify(
				sdata, BSS_CHANGED_BEACON_ENABLED);

		
		if (sdata->vif.type != NL80211_IFTYPE_STATION)
			netif_tx_stop_all_queues(sdata->dev);
	}
	mutex_unlock(&local->iflist_mtx);

	local->next_scan_state = SCAN_DECISION;
	local->scan_channel_idx = 0;

	ieee80211_configure_filter(local);

	
	ieee80211_queue_delayed_work(&local->hw,
				     &local->scan_work,
				     IEEE80211_CHANNEL_TIME);

	return 0;
}


static int __ieee80211_start_scan(struct ieee80211_sub_if_data *sdata,
				  struct cfg80211_scan_request *req)
{
	struct ieee80211_local *local = sdata->local;
	struct ieee80211_if_managed *ifmgd = &sdata->u.mgd;
	int rc;

	if (local->scan_req)
		return -EBUSY;

	if (local->ops->hw_scan) {
		u8 *ies;
		int ielen;

		ies = kmalloc(2 + IEEE80211_MAX_SSID_LEN +
			      local->scan_ies_len + req->ie_len, GFP_KERNEL);
		if (!ies)
			return -ENOMEM;

		ielen = ieee80211_build_preq_ies(local, ies,
						 req->ie, req->ie_len);
		local->orig_ies = req->ie;
		local->orig_ies_len = req->ie_len;
		req->ie = ies;
		req->ie_len = ielen;
	}

	local->scan_req = req;
	local->scan_sdata = sdata;

	if (req != local->int_scan_req &&
	    sdata->vif.type == NL80211_IFTYPE_STATION &&
	    !list_empty(&ifmgd->work_list)) {
		
		set_bit(IEEE80211_STA_REQ_SCAN, &ifmgd->request);
		return 0;
	}

	if (local->ops->hw_scan)
		__set_bit(SCAN_HW_SCANNING, &local->scanning);
	else
		__set_bit(SCAN_SW_SCANNING, &local->scanning);
	

	ieee80211_recalc_idle(local);
	mutex_unlock(&local->scan_mtx);

	if (local->ops->hw_scan)
		rc = drv_hw_scan(local, local->scan_req);
	else
		rc = ieee80211_start_sw_scan(local);

	mutex_lock(&local->scan_mtx);

	if (rc) {
		if (local->ops->hw_scan)
			ieee80211_restore_scan_ies(local);
		local->scanning = 0;

		ieee80211_recalc_idle(local);

		local->scan_req = NULL;
		local->scan_sdata = NULL;
	}

	return rc;
}

static int ieee80211_scan_state_decision(struct ieee80211_local *local,
					 unsigned long *next_delay)
{
	bool associated = false;
	struct ieee80211_sub_if_data *sdata;

	
	if (local->scan_channel_idx >= local->scan_req->n_channels) {
		ieee80211_scan_completed(&local->hw, false);
		return 1;
	}

	
	mutex_lock(&local->iflist_mtx);
	list_for_each_entry(sdata, &local->interfaces, list) {
		if (!netif_running(sdata->dev))
			continue;

		if (sdata->vif.type == NL80211_IFTYPE_STATION) {
			if (sdata->u.mgd.associated) {
				associated = true;
				break;
			}
		}
	}
	mutex_unlock(&local->iflist_mtx);

	if (local->scan_channel) {
		
		if (associated)
			local->next_scan_state = SCAN_ENTER_OPER_CHANNEL;
		else
			local->next_scan_state = SCAN_SET_CHANNEL;
	} else {
		
		local->next_scan_state = SCAN_LEAVE_OPER_CHANNEL;
	}

	*next_delay = 0;
	return 0;
}

static void ieee80211_scan_state_leave_oper_channel(struct ieee80211_local *local,
						    unsigned long *next_delay)
{
	struct ieee80211_sub_if_data *sdata;

	
	mutex_lock(&local->iflist_mtx);
	list_for_each_entry(sdata, &local->interfaces, list) {
		if (!netif_running(sdata->dev))
			continue;

		if (sdata->vif.type == NL80211_IFTYPE_STATION) {
			netif_tx_stop_all_queues(sdata->dev);
			if (sdata->u.mgd.associated)
				ieee80211_scan_ps_enable(sdata);
		}
	}
	mutex_unlock(&local->iflist_mtx);

	__set_bit(SCAN_OFF_CHANNEL, &local->scanning);

	
	*next_delay = HZ / 10;
	local->next_scan_state = SCAN_SET_CHANNEL;
}

static void ieee80211_scan_state_enter_oper_channel(struct ieee80211_local *local,
						    unsigned long *next_delay)
{
	struct ieee80211_sub_if_data *sdata = local->scan_sdata;

	
	local->scan_channel = NULL;
	ieee80211_hw_config(local, IEEE80211_CONF_CHANGE_CHANNEL);

	
	mutex_lock(&local->iflist_mtx);
	list_for_each_entry(sdata, &local->interfaces, list) {
		if (!netif_running(sdata->dev))
			continue;

		
		if (sdata->vif.type == NL80211_IFTYPE_STATION) {
			if (sdata->u.mgd.associated)
				ieee80211_scan_ps_disable(sdata);
			netif_tx_wake_all_queues(sdata->dev);
		}
	}
	mutex_unlock(&local->iflist_mtx);

	__clear_bit(SCAN_OFF_CHANNEL, &local->scanning);

	*next_delay = HZ / 5;
	local->next_scan_state = SCAN_DECISION;
}

static void ieee80211_scan_state_set_channel(struct ieee80211_local *local,
					     unsigned long *next_delay)
{
	int skip;
	struct ieee80211_channel *chan;
	struct ieee80211_sub_if_data *sdata = local->scan_sdata;

	skip = 0;
	chan = local->scan_req->channels[local->scan_channel_idx];

	if (chan->flags & IEEE80211_CHAN_DISABLED ||
	    (sdata->vif.type == NL80211_IFTYPE_ADHOC &&
	     chan->flags & IEEE80211_CHAN_NO_IBSS))
		skip = 1;

	if (!skip) {
		local->scan_channel = chan;
		if (ieee80211_hw_config(local,
					IEEE80211_CONF_CHANGE_CHANNEL))
			skip = 1;
	}

	
	local->scan_channel_idx++;

	if (skip) {
		
		local->next_scan_state = SCAN_DECISION;
		return;
	}

	
	if (chan->flags & IEEE80211_CHAN_PASSIVE_SCAN ||
	    !local->scan_req->n_ssids) {
		*next_delay = IEEE80211_PASSIVE_CHANNEL_TIME;
		local->next_scan_state = SCAN_DECISION;
		return;
	}

	
	*next_delay = IEEE80211_PROBE_DELAY;
	local->next_scan_state = SCAN_SEND_PROBE;
}

static void ieee80211_scan_state_send_probe(struct ieee80211_local *local,
					    unsigned long *next_delay)
{
	int i;
	struct ieee80211_sub_if_data *sdata = local->scan_sdata;

	for (i = 0; i < local->scan_req->n_ssids; i++)
		ieee80211_send_probe_req(
			sdata, NULL,
			local->scan_req->ssids[i].ssid,
			local->scan_req->ssids[i].ssid_len,
			local->scan_req->ie, local->scan_req->ie_len);

	
	*next_delay = IEEE80211_CHANNEL_TIME;
	local->next_scan_state = SCAN_DECISION;
}

void ieee80211_scan_work(struct work_struct *work)
{
	struct ieee80211_local *local =
		container_of(work, struct ieee80211_local, scan_work.work);
	struct ieee80211_sub_if_data *sdata = local->scan_sdata;
	unsigned long next_delay = 0;

	mutex_lock(&local->scan_mtx);
	if (!sdata || !local->scan_req) {
		mutex_unlock(&local->scan_mtx);
		return;
	}

	if (local->scan_req && !local->scanning) {
		struct cfg80211_scan_request *req = local->scan_req;
		int rc;

		local->scan_req = NULL;
		local->scan_sdata = NULL;

		rc = __ieee80211_start_scan(sdata, req);
		mutex_unlock(&local->scan_mtx);

		if (rc)
			ieee80211_scan_completed(&local->hw, true);
		return;
	}

	mutex_unlock(&local->scan_mtx);

	
	if (!netif_running(sdata->dev)) {
		ieee80211_scan_completed(&local->hw, true);
		return;
	}

	
	do {
		switch (local->next_scan_state) {
		case SCAN_DECISION:
			if (ieee80211_scan_state_decision(local, &next_delay))
				return;
			break;
		case SCAN_SET_CHANNEL:
			ieee80211_scan_state_set_channel(local, &next_delay);
			break;
		case SCAN_SEND_PROBE:
			ieee80211_scan_state_send_probe(local, &next_delay);
			break;
		case SCAN_LEAVE_OPER_CHANNEL:
			ieee80211_scan_state_leave_oper_channel(local, &next_delay);
			break;
		case SCAN_ENTER_OPER_CHANNEL:
			ieee80211_scan_state_enter_oper_channel(local, &next_delay);
			break;
		}
	} while (next_delay == 0);

	ieee80211_queue_delayed_work(&local->hw, &local->scan_work, next_delay);
}

int ieee80211_request_scan(struct ieee80211_sub_if_data *sdata,
			   struct cfg80211_scan_request *req)
{
	int res;

	mutex_lock(&sdata->local->scan_mtx);
	res = __ieee80211_start_scan(sdata, req);
	mutex_unlock(&sdata->local->scan_mtx);

	return res;
}

int ieee80211_request_internal_scan(struct ieee80211_sub_if_data *sdata,
				    const u8 *ssid, u8 ssid_len)
{
	struct ieee80211_local *local = sdata->local;
	int ret = -EBUSY;

	mutex_lock(&local->scan_mtx);

	
	if (local->scan_req)
		goto unlock;

	memcpy(local->int_scan_req->ssids[0].ssid, ssid, IEEE80211_MAX_SSID_LEN);
	local->int_scan_req->ssids[0].ssid_len = ssid_len;

	ret = __ieee80211_start_scan(sdata, sdata->local->int_scan_req);
 unlock:
	mutex_unlock(&local->scan_mtx);
	return ret;
}

void ieee80211_scan_cancel(struct ieee80211_local *local)
{
	bool abortscan;

	cancel_delayed_work_sync(&local->scan_work);

	
	mutex_lock(&local->scan_mtx);
	abortscan = test_bit(SCAN_SW_SCANNING, &local->scanning) ||
		    (!local->scanning && local->scan_req);
	mutex_unlock(&local->scan_mtx);

	if (abortscan)
		ieee80211_scan_completed(&local->hw, true);
}
