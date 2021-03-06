

#include <linux/if.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/nl80211.h>
#include <linux/debugfs.h>
#include <linux/notifier.h>
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/sched.h>
#include <net/genetlink.h>
#include <net/cfg80211.h>
#include "nl80211.h"
#include "core.h"
#include "sysfs.h"
#include "debugfs.h"
#include "wext-compat.h"


#define PHY_NAME "phy"

MODULE_AUTHOR("Johannes Berg");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wireless configuration support");


LIST_HEAD(cfg80211_rdev_list);
int cfg80211_rdev_list_generation;


DEFINE_MUTEX(cfg80211_mutex);


static struct dentry *ieee80211_debugfs_dir;


struct cfg80211_registered_device *cfg80211_rdev_by_wiphy_idx(int wiphy_idx)
{
	struct cfg80211_registered_device *result = NULL, *rdev;

	if (!wiphy_idx_valid(wiphy_idx))
		return NULL;

	assert_cfg80211_lock();

	list_for_each_entry(rdev, &cfg80211_rdev_list, list) {
		if (rdev->wiphy_idx == wiphy_idx) {
			result = rdev;
			break;
		}
	}

	return result;
}

int get_wiphy_idx(struct wiphy *wiphy)
{
	struct cfg80211_registered_device *rdev;
	if (!wiphy)
		return WIPHY_IDX_STALE;
	rdev = wiphy_to_dev(wiphy);
	return rdev->wiphy_idx;
}


struct wiphy *wiphy_idx_to_wiphy(int wiphy_idx)
{
	struct cfg80211_registered_device *rdev;

	if (!wiphy_idx_valid(wiphy_idx))
		return NULL;

	assert_cfg80211_lock();

	rdev = cfg80211_rdev_by_wiphy_idx(wiphy_idx);
	if (!rdev)
		return NULL;
	return &rdev->wiphy;
}


struct cfg80211_registered_device *
__cfg80211_rdev_from_info(struct genl_info *info)
{
	int ifindex;
	struct cfg80211_registered_device *bywiphyidx = NULL, *byifidx = NULL;
	struct net_device *dev;
	int err = -EINVAL;

	assert_cfg80211_lock();

	if (info->attrs[NL80211_ATTR_WIPHY]) {
		bywiphyidx = cfg80211_rdev_by_wiphy_idx(
				nla_get_u32(info->attrs[NL80211_ATTR_WIPHY]));
		err = -ENODEV;
	}

	if (info->attrs[NL80211_ATTR_IFINDEX]) {
		ifindex = nla_get_u32(info->attrs[NL80211_ATTR_IFINDEX]);
		dev = dev_get_by_index(genl_info_net(info), ifindex);
		if (dev) {
			if (dev->ieee80211_ptr)
				byifidx =
					wiphy_to_dev(dev->ieee80211_ptr->wiphy);
			dev_put(dev);
		}
		err = -ENODEV;
	}

	if (bywiphyidx && byifidx) {
		if (bywiphyidx != byifidx)
			return ERR_PTR(-EINVAL);
		else
			return bywiphyidx; 
	}
	if (bywiphyidx)
		return bywiphyidx;

	if (byifidx)
		return byifidx;

	return ERR_PTR(err);
}

struct cfg80211_registered_device *
cfg80211_get_dev_from_info(struct genl_info *info)
{
	struct cfg80211_registered_device *rdev;

	mutex_lock(&cfg80211_mutex);
	rdev = __cfg80211_rdev_from_info(info);

	
	if (!IS_ERR(rdev))
		mutex_lock(&rdev->mtx);

	mutex_unlock(&cfg80211_mutex);

	return rdev;
}

struct cfg80211_registered_device *
cfg80211_get_dev_from_ifindex(struct net *net, int ifindex)
{
	struct cfg80211_registered_device *rdev = ERR_PTR(-ENODEV);
	struct net_device *dev;

	mutex_lock(&cfg80211_mutex);
	dev = dev_get_by_index(net, ifindex);
	if (!dev)
		goto out;
	if (dev->ieee80211_ptr) {
		rdev = wiphy_to_dev(dev->ieee80211_ptr->wiphy);
		mutex_lock(&rdev->mtx);
	} else
		rdev = ERR_PTR(-ENODEV);
	dev_put(dev);
 out:
	mutex_unlock(&cfg80211_mutex);
	return rdev;
}


int cfg80211_dev_rename(struct cfg80211_registered_device *rdev,
			char *newname)
{
	struct cfg80211_registered_device *rdev2;
	int wiphy_idx, taken = -1, result, digits;

	assert_cfg80211_lock();

	
	sscanf(newname, PHY_NAME "%d%n", &wiphy_idx, &taken);
	if (taken == strlen(newname) && wiphy_idx != rdev->wiphy_idx) {
		
		digits = 1;
		while (wiphy_idx /= 10)
			digits++;
		
		if (taken == strlen(PHY_NAME) + digits)
			return -EINVAL;
	}


	
	if (strcmp(newname, dev_name(&rdev->wiphy.dev)) == 0)
		return 0;

	
	list_for_each_entry(rdev2, &cfg80211_rdev_list, list)
		if (strcmp(newname, dev_name(&rdev2->wiphy.dev)) == 0)
			return -EINVAL;

	result = device_rename(&rdev->wiphy.dev, newname);
	if (result)
		return result;

	if (rdev->wiphy.debugfsdir &&
	    !debugfs_rename(rdev->wiphy.debugfsdir->d_parent,
			    rdev->wiphy.debugfsdir,
			    rdev->wiphy.debugfsdir->d_parent,
			    newname))
		printk(KERN_ERR "cfg80211: failed to rename debugfs dir to %s!\n",
		       newname);

	nl80211_notify_dev_rename(rdev);

	return 0;
}

int cfg80211_switch_netns(struct cfg80211_registered_device *rdev,
			  struct net *net)
{
	struct wireless_dev *wdev;
	int err = 0;

	if (!rdev->wiphy.netnsok)
		return -EOPNOTSUPP;

	list_for_each_entry(wdev, &rdev->netdev_list, list) {
		wdev->netdev->features &= ~NETIF_F_NETNS_LOCAL;
		err = dev_change_net_namespace(wdev->netdev, net, "wlan%d");
		if (err)
			break;
		wdev->netdev->features |= NETIF_F_NETNS_LOCAL;
	}

	if (err) {
		
		net = wiphy_net(&rdev->wiphy);

		list_for_each_entry_continue_reverse(wdev, &rdev->netdev_list,
						     list) {
			wdev->netdev->features &= ~NETIF_F_NETNS_LOCAL;
			err = dev_change_net_namespace(wdev->netdev, net,
							"wlan%d");
			WARN_ON(err);
			wdev->netdev->features |= NETIF_F_NETNS_LOCAL;
		}
	}

	wiphy_net_set(&rdev->wiphy, net);

	return err;
}

static void cfg80211_rfkill_poll(struct rfkill *rfkill, void *data)
{
	struct cfg80211_registered_device *rdev = data;

	rdev->ops->rfkill_poll(&rdev->wiphy);
}

static int cfg80211_rfkill_set_block(void *data, bool blocked)
{
	struct cfg80211_registered_device *rdev = data;
	struct wireless_dev *wdev;

	if (!blocked)
		return 0;

	rtnl_lock();
	mutex_lock(&rdev->devlist_mtx);

	list_for_each_entry(wdev, &rdev->netdev_list, list)
		dev_close(wdev->netdev);

	mutex_unlock(&rdev->devlist_mtx);
	rtnl_unlock();

	return 0;
}

static void cfg80211_rfkill_sync_work(struct work_struct *work)
{
	struct cfg80211_registered_device *rdev;

	rdev = container_of(work, struct cfg80211_registered_device, rfkill_sync);
	cfg80211_rfkill_set_block(rdev, rfkill_blocked(rdev->rfkill));
}

static void cfg80211_event_work(struct work_struct *work)
{
	struct cfg80211_registered_device *rdev;

	rdev = container_of(work, struct cfg80211_registered_device,
			    event_work);

	rtnl_lock();
	cfg80211_lock_rdev(rdev);

	cfg80211_process_rdev_events(rdev);
	cfg80211_unlock_rdev(rdev);
	rtnl_unlock();
}



struct wiphy *wiphy_new(const struct cfg80211_ops *ops, int sizeof_priv)
{
	static int wiphy_counter;

	struct cfg80211_registered_device *rdev;
	int alloc_size;

	WARN_ON(ops->add_key && (!ops->del_key || !ops->set_default_key));
	WARN_ON(ops->auth && (!ops->assoc || !ops->deauth || !ops->disassoc));
	WARN_ON(ops->connect && !ops->disconnect);
	WARN_ON(ops->join_ibss && !ops->leave_ibss);
	WARN_ON(ops->add_virtual_intf && !ops->del_virtual_intf);
	WARN_ON(ops->add_station && !ops->del_station);
	WARN_ON(ops->add_mpath && !ops->del_mpath);

	alloc_size = sizeof(*rdev) + sizeof_priv;

	rdev = kzalloc(alloc_size, GFP_KERNEL);
	if (!rdev)
		return NULL;

	rdev->ops = ops;

	mutex_lock(&cfg80211_mutex);

	rdev->wiphy_idx = wiphy_counter++;

	if (unlikely(!wiphy_idx_valid(rdev->wiphy_idx))) {
		wiphy_counter--;
		mutex_unlock(&cfg80211_mutex);
		
		kfree(rdev);
		return NULL;
	}

	mutex_unlock(&cfg80211_mutex);

	
	dev_set_name(&rdev->wiphy.dev, PHY_NAME "%d", rdev->wiphy_idx);

	mutex_init(&rdev->mtx);
	mutex_init(&rdev->devlist_mtx);
	INIT_LIST_HEAD(&rdev->netdev_list);
	spin_lock_init(&rdev->bss_lock);
	INIT_LIST_HEAD(&rdev->bss_list);
	INIT_WORK(&rdev->scan_done_wk, __cfg80211_scan_done);

	device_initialize(&rdev->wiphy.dev);
	rdev->wiphy.dev.class = &ieee80211_class;
	rdev->wiphy.dev.platform_data = rdev;

	rdev->wiphy.ps_default = CONFIG_CFG80211_DEFAULT_PS_VALUE;

	wiphy_net_set(&rdev->wiphy, &init_net);

	rdev->rfkill_ops.set_block = cfg80211_rfkill_set_block;
	rdev->rfkill = rfkill_alloc(dev_name(&rdev->wiphy.dev),
				   &rdev->wiphy.dev, RFKILL_TYPE_WLAN,
				   &rdev->rfkill_ops, rdev);

	if (!rdev->rfkill) {
		kfree(rdev);
		return NULL;
	}

	INIT_WORK(&rdev->rfkill_sync, cfg80211_rfkill_sync_work);
	INIT_WORK(&rdev->conn_work, cfg80211_conn_work);
	INIT_WORK(&rdev->event_work, cfg80211_event_work);

	init_waitqueue_head(&rdev->dev_wait);

	
	rdev->wiphy.retry_short = 7;
	rdev->wiphy.retry_long = 4;
	rdev->wiphy.frag_threshold = (u32) -1;
	rdev->wiphy.rts_threshold = (u32) -1;

	return &rdev->wiphy;
}
EXPORT_SYMBOL(wiphy_new);

int wiphy_register(struct wiphy *wiphy)
{
	struct cfg80211_registered_device *rdev = wiphy_to_dev(wiphy);
	int res;
	enum ieee80211_band band;
	struct ieee80211_supported_band *sband;
	bool have_band = false;
	int i;
	u16 ifmodes = wiphy->interface_modes;

	
	WARN_ON(!ifmodes);
	ifmodes &= ((1 << __NL80211_IFTYPE_AFTER_LAST) - 1) & ~1;
	if (WARN_ON(ifmodes != wiphy->interface_modes))
		wiphy->interface_modes = ifmodes;

	
	for (band = 0; band < IEEE80211_NUM_BANDS; band++) {
		sband = wiphy->bands[band];
		if (!sband)
			continue;

		sband->band = band;

		if (WARN_ON(!sband->n_channels || !sband->n_bitrates))
			return -EINVAL;

		
		if (WARN_ON(sband->n_bitrates > 32))
			return -EINVAL;

		for (i = 0; i < sband->n_channels; i++) {
			sband->channels[i].orig_flags =
				sband->channels[i].flags;
			sband->channels[i].orig_mag =
				sband->channels[i].max_antenna_gain;
			sband->channels[i].orig_mpwr =
				sband->channels[i].max_power;
			sband->channels[i].band = band;
		}

		have_band = true;
	}

	if (!have_band) {
		WARN_ON(1);
		return -EINVAL;
	}

	
	ieee80211_set_bitrate_flags(wiphy);

	res = device_add(&rdev->wiphy.dev);
	if (res)
		return res;

	res = rfkill_register(rdev->rfkill);
	if (res)
		goto out_rm_dev;

	mutex_lock(&cfg80211_mutex);

	
	wiphy_update_regulatory(wiphy, NL80211_REGDOM_SET_BY_CORE);

	list_add(&rdev->list, &cfg80211_rdev_list);
	cfg80211_rdev_list_generation++;

	mutex_unlock(&cfg80211_mutex);

	
	rdev->wiphy.debugfsdir =
		debugfs_create_dir(wiphy_name(&rdev->wiphy),
				   ieee80211_debugfs_dir);
	if (IS_ERR(rdev->wiphy.debugfsdir))
		rdev->wiphy.debugfsdir = NULL;

	if (wiphy->custom_regulatory) {
		struct regulatory_request request;

		request.wiphy_idx = get_wiphy_idx(wiphy);
		request.initiator = NL80211_REGDOM_SET_BY_DRIVER;
		request.alpha2[0] = '9';
		request.alpha2[1] = '9';

		nl80211_send_reg_change_event(&request);
	}

	cfg80211_debugfs_rdev_add(rdev);

	return 0;

 out_rm_dev:
	device_del(&rdev->wiphy.dev);
	return res;
}
EXPORT_SYMBOL(wiphy_register);

void wiphy_rfkill_start_polling(struct wiphy *wiphy)
{
	struct cfg80211_registered_device *rdev = wiphy_to_dev(wiphy);

	if (!rdev->ops->rfkill_poll)
		return;
	rdev->rfkill_ops.poll = cfg80211_rfkill_poll;
	rfkill_resume_polling(rdev->rfkill);
}
EXPORT_SYMBOL(wiphy_rfkill_start_polling);

void wiphy_rfkill_stop_polling(struct wiphy *wiphy)
{
	struct cfg80211_registered_device *rdev = wiphy_to_dev(wiphy);

	rfkill_pause_polling(rdev->rfkill);
}
EXPORT_SYMBOL(wiphy_rfkill_stop_polling);

void wiphy_unregister(struct wiphy *wiphy)
{
	struct cfg80211_registered_device *rdev = wiphy_to_dev(wiphy);

	rfkill_unregister(rdev->rfkill);

	
	mutex_lock(&cfg80211_mutex);

	wait_event(rdev->dev_wait, ({
		int __count;
		mutex_lock(&rdev->devlist_mtx);
		__count = rdev->opencount;
		mutex_unlock(&rdev->devlist_mtx);
		__count == 0;}));

	mutex_lock(&rdev->devlist_mtx);
	BUG_ON(!list_empty(&rdev->netdev_list));
	mutex_unlock(&rdev->devlist_mtx);

	
	cfg80211_debugfs_rdev_del(rdev);
	list_del(&rdev->list);

	
	cfg80211_lock_rdev(rdev);
	
	cfg80211_unlock_rdev(rdev);

	
	reg_device_remove(wiphy);

	cfg80211_rdev_list_generation++;
	device_del(&rdev->wiphy.dev);
	debugfs_remove(rdev->wiphy.debugfsdir);

	mutex_unlock(&cfg80211_mutex);

	flush_work(&rdev->scan_done_wk);
	cancel_work_sync(&rdev->conn_work);
	flush_work(&rdev->event_work);
}
EXPORT_SYMBOL(wiphy_unregister);

void cfg80211_dev_free(struct cfg80211_registered_device *rdev)
{
	struct cfg80211_internal_bss *scan, *tmp;
	rfkill_destroy(rdev->rfkill);
	mutex_destroy(&rdev->mtx);
	mutex_destroy(&rdev->devlist_mtx);
	list_for_each_entry_safe(scan, tmp, &rdev->bss_list, list)
		cfg80211_put_bss(&scan->pub);
	kfree(rdev);
}

void wiphy_free(struct wiphy *wiphy)
{
	put_device(&wiphy->dev);
}
EXPORT_SYMBOL(wiphy_free);

void wiphy_rfkill_set_hw_state(struct wiphy *wiphy, bool blocked)
{
	struct cfg80211_registered_device *rdev = wiphy_to_dev(wiphy);

	if (rfkill_set_hw_state(rdev->rfkill, blocked))
		schedule_work(&rdev->rfkill_sync);
}
EXPORT_SYMBOL(wiphy_rfkill_set_hw_state);

static void wdev_cleanup_work(struct work_struct *work)
{
	struct wireless_dev *wdev;
	struct cfg80211_registered_device *rdev;

	wdev = container_of(work, struct wireless_dev, cleanup_work);
	rdev = wiphy_to_dev(wdev->wiphy);

	cfg80211_lock_rdev(rdev);

	if (WARN_ON(rdev->scan_req && rdev->scan_req->dev == wdev->netdev)) {
		rdev->scan_req->aborted = true;
		___cfg80211_scan_done(rdev, true);
	}

	cfg80211_unlock_rdev(rdev);

	mutex_lock(&rdev->devlist_mtx);
	rdev->opencount--;
	mutex_unlock(&rdev->devlist_mtx);
	wake_up(&rdev->dev_wait);

	dev_put(wdev->netdev);
}

static int cfg80211_netdev_notifier_call(struct notifier_block * nb,
					 unsigned long state,
					 void *ndev)
{
	struct net_device *dev = ndev;
	struct wireless_dev *wdev = dev->ieee80211_ptr;
	struct cfg80211_registered_device *rdev;

	if (!wdev)
		return NOTIFY_DONE;

	rdev = wiphy_to_dev(wdev->wiphy);

	WARN_ON(wdev->iftype == NL80211_IFTYPE_UNSPECIFIED);

	switch (state) {
	case NETDEV_REGISTER:
		
		mutex_init(&wdev->mtx);
		INIT_WORK(&wdev->cleanup_work, wdev_cleanup_work);
		INIT_LIST_HEAD(&wdev->event_list);
		spin_lock_init(&wdev->event_lock);
		mutex_lock(&rdev->devlist_mtx);
		list_add(&wdev->list, &rdev->netdev_list);
		rdev->devlist_generation++;
		
		dev->features |= NETIF_F_NETNS_LOCAL;

		if (sysfs_create_link(&dev->dev.kobj, &rdev->wiphy.dev.kobj,
				      "phy80211")) {
			printk(KERN_ERR "wireless: failed to add phy80211 "
				"symlink to netdev!\n");
		}
		wdev->netdev = dev;
		wdev->sme_state = CFG80211_SME_IDLE;
		mutex_unlock(&rdev->devlist_mtx);
#ifdef CONFIG_WIRELESS_EXT
		if (!dev->wireless_handlers)
			dev->wireless_handlers = &cfg80211_wext_handler;
		wdev->wext.default_key = -1;
		wdev->wext.default_mgmt_key = -1;
		wdev->wext.connect.auth_type = NL80211_AUTHTYPE_AUTOMATIC;
		wdev->wext.ps = wdev->wiphy->ps_default;
		wdev->wext.ps_timeout = 100;
		if (rdev->ops->set_power_mgmt)
			if (rdev->ops->set_power_mgmt(wdev->wiphy, dev,
						      wdev->wext.ps,
						      wdev->wext.ps_timeout)) {
				
				wdev->wext.ps = false;
			}
#endif
		break;
	case NETDEV_GOING_DOWN:
		switch (wdev->iftype) {
		case NL80211_IFTYPE_ADHOC:
			cfg80211_leave_ibss(rdev, dev, true);
			break;
		case NL80211_IFTYPE_STATION:
			wdev_lock(wdev);
#ifdef CONFIG_WIRELESS_EXT
			kfree(wdev->wext.ie);
			wdev->wext.ie = NULL;
			wdev->wext.ie_len = 0;
			wdev->wext.connect.auth_type = NL80211_AUTHTYPE_AUTOMATIC;
#endif
			__cfg80211_disconnect(rdev, dev,
					      WLAN_REASON_DEAUTH_LEAVING, true);
			cfg80211_mlme_down(rdev, dev);
			wdev_unlock(wdev);
			break;
		default:
			break;
		}
		break;
	case NETDEV_DOWN:
		dev_hold(dev);
		schedule_work(&wdev->cleanup_work);
		break;
	case NETDEV_UP:
		
		if (cancel_work_sync(&wdev->cleanup_work)) {
			mutex_lock(&rdev->devlist_mtx);
			rdev->opencount--;
			mutex_unlock(&rdev->devlist_mtx);
			dev_put(dev);
		}
#ifdef CONFIG_WIRELESS_EXT
		cfg80211_lock_rdev(rdev);
		mutex_lock(&rdev->devlist_mtx);
		wdev_lock(wdev);
		switch (wdev->iftype) {
		case NL80211_IFTYPE_ADHOC:
			cfg80211_ibss_wext_join(rdev, wdev);
			break;
		case NL80211_IFTYPE_STATION:
			cfg80211_mgd_wext_connect(rdev, wdev);
			break;
		default:
			break;
		}
		wdev_unlock(wdev);
		rdev->opencount++;
		mutex_unlock(&rdev->devlist_mtx);
		cfg80211_unlock_rdev(rdev);
#endif
		break;
	case NETDEV_UNREGISTER:
		
		mutex_lock(&rdev->devlist_mtx);
		
		if (!list_empty(&wdev->list)) {
			sysfs_remove_link(&dev->dev.kobj, "phy80211");
			list_del_init(&wdev->list);
			rdev->devlist_generation++;
#ifdef CONFIG_WIRELESS_EXT
			kfree(wdev->wext.keys);
#endif
		}
		mutex_unlock(&rdev->devlist_mtx);
		break;
	case NETDEV_PRE_UP:
		if (!(wdev->wiphy->interface_modes & BIT(wdev->iftype)))
			return notifier_from_errno(-EOPNOTSUPP);
		if (rfkill_blocked(rdev->rfkill))
			return notifier_from_errno(-ERFKILL);
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block cfg80211_netdev_notifier = {
	.notifier_call = cfg80211_netdev_notifier_call,
};

static void __net_exit cfg80211_pernet_exit(struct net *net)
{
	struct cfg80211_registered_device *rdev;

	rtnl_lock();
	mutex_lock(&cfg80211_mutex);
	list_for_each_entry(rdev, &cfg80211_rdev_list, list) {
		if (net_eq(wiphy_net(&rdev->wiphy), net))
			WARN_ON(cfg80211_switch_netns(rdev, &init_net));
	}
	mutex_unlock(&cfg80211_mutex);
	rtnl_unlock();
}

static struct pernet_operations cfg80211_pernet_ops = {
	.exit = cfg80211_pernet_exit,
};

static int __init cfg80211_init(void)
{
	int err;

	err = register_pernet_device(&cfg80211_pernet_ops);
	if (err)
		goto out_fail_pernet;

	err = wiphy_sysfs_init();
	if (err)
		goto out_fail_sysfs;

	err = register_netdevice_notifier(&cfg80211_netdev_notifier);
	if (err)
		goto out_fail_notifier;

	err = nl80211_init();
	if (err)
		goto out_fail_nl80211;

	ieee80211_debugfs_dir = debugfs_create_dir("ieee80211", NULL);

	err = regulatory_init();
	if (err)
		goto out_fail_reg;

	return 0;

out_fail_reg:
	debugfs_remove(ieee80211_debugfs_dir);
out_fail_nl80211:
	unregister_netdevice_notifier(&cfg80211_netdev_notifier);
out_fail_notifier:
	wiphy_sysfs_exit();
out_fail_sysfs:
	unregister_pernet_device(&cfg80211_pernet_ops);
out_fail_pernet:
	return err;
}
subsys_initcall(cfg80211_init);

static void cfg80211_exit(void)
{
	debugfs_remove(ieee80211_debugfs_dir);
	nl80211_exit();
	unregister_netdevice_notifier(&cfg80211_netdev_notifier);
	wiphy_sysfs_exit();
	regulatory_exit();
	unregister_pernet_device(&cfg80211_pernet_ops);
}
module_exit(cfg80211_exit);
