

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/if_arp.h>
#include <linux/types.h>
#include <net/ip.h>
#include <net/pkt_sched.h>

#include <net/mac80211.h>
#include "ieee80211_i.h"
#include "wme.h"


const int ieee802_1d_to_ac[8] = { 2, 3, 3, 2, 1, 1, 0, 0 };

static int wme_downgrade_ac(struct sk_buff *skb)
{
	switch (skb->priority) {
	case 6:
	case 7:
		skb->priority = 5; 
		return 0;
	case 4:
	case 5:
		skb->priority = 3; 
		return 0;
	case 0:
	case 3:
		skb->priority = 2; 
		return 0;
	default:
		return -1;
	}
}



u16 ieee80211_select_queue(struct ieee80211_sub_if_data *sdata,
			   struct sk_buff *skb)
{
	struct ieee80211_local *local = sdata->local;
	struct sta_info *sta = NULL;
	u32 sta_flags = 0;
	const u8 *ra = NULL;
	bool qos = false;

	if (local->hw.queues < 4 || skb->len < 6) {
		skb->priority = 0; 
		return min_t(u16, local->hw.queues - 1,
			     ieee802_1d_to_ac[skb->priority]);
	}

	rcu_read_lock();
	switch (sdata->vif.type) {
	case NL80211_IFTYPE_AP_VLAN:
	case NL80211_IFTYPE_AP:
		ra = skb->data;
		break;
	case NL80211_IFTYPE_WDS:
		ra = sdata->u.wds.remote_addr;
		break;
#ifdef CONFIG_MAC80211_MESH
	case NL80211_IFTYPE_MESH_POINT:
		
		break;
#endif
	case NL80211_IFTYPE_STATION:
		ra = sdata->u.mgd.bssid;
		break;
	case NL80211_IFTYPE_ADHOC:
		ra = skb->data;
		break;
	default:
		break;
	}

	if (!sta && ra && !is_multicast_ether_addr(ra)) {
		sta = sta_info_get(local, ra);
		if (sta)
			sta_flags = get_sta_flags(sta);
	}

	if (sta_flags & WLAN_STA_WME)
		qos = true;

	rcu_read_unlock();

	if (!qos) {
		skb->priority = 0; 
		return ieee802_1d_to_ac[skb->priority];
	}

	
	skb->priority = cfg80211_classify8021d(skb);

	return ieee80211_downgrade_queue(local, skb);
}

u16 ieee80211_downgrade_queue(struct ieee80211_local *local,
			      struct sk_buff *skb)
{
	
	while (unlikely(local->wmm_acm & BIT(skb->priority))) {
		if (wme_downgrade_ac(skb)) {
			
			break;
		}
	}

	
	return ieee802_1d_to_ac[skb->priority];
}

void ieee80211_set_qos_hdr(struct ieee80211_local *local, struct sk_buff *skb)
{
	struct ieee80211_hdr *hdr = (void *)skb->data;

	
	if (ieee80211_is_data_qos(hdr->frame_control)) {
		u8 *p = ieee80211_get_qos_ctl(hdr);
		u8 ack_policy = 0, tid;

		tid = skb->priority & IEEE80211_QOS_CTL_TAG1D_MASK;

		if (unlikely(local->wifi_wme_noack_test))
			ack_policy |= QOS_CONTROL_ACK_POLICY_NOACK <<
					QOS_CONTROL_ACK_POLICY_SHIFT;
		
		*p++ = ack_policy | tid;
		*p = 0;
	}
}
