


#include <linux/netdevice.h>

#include "iwm.h"
#include "commands.h"
#include "cfg80211.h"
#include "debug.h"

static int iwm_open(struct net_device *ndev)
{
	struct iwm_priv *iwm = ndev_to_iwm(ndev);

	return iwm_up(iwm);
}

static int iwm_stop(struct net_device *ndev)
{
	struct iwm_priv *iwm = ndev_to_iwm(ndev);

	return iwm_down(iwm);
}


static const u16 iwm_1d_to_queue[8] = { 1, 0, 0, 1, 2, 2, 3, 3 };

static u16 iwm_select_queue(struct net_device *dev, struct sk_buff *skb)
{
	skb->priority = cfg80211_classify8021d(skb);

	return iwm_1d_to_queue[skb->priority];
}

static const struct net_device_ops iwm_netdev_ops = {
	.ndo_open		= iwm_open,
	.ndo_stop		= iwm_stop,
	.ndo_start_xmit		= iwm_xmit_frame,
	.ndo_select_queue	= iwm_select_queue,
};

void *iwm_if_alloc(int sizeof_bus, struct device *dev,
		   struct iwm_if_ops *if_ops)
{
	struct net_device *ndev;
	struct wireless_dev *wdev;
	struct iwm_priv *iwm;
	int ret = 0;

	wdev = iwm_wdev_alloc(sizeof_bus, dev);
	if (IS_ERR(wdev))
		return wdev;

	iwm = wdev_to_iwm(wdev);
	iwm->bus_ops = if_ops;
	iwm->wdev = wdev;

	ret = iwm_priv_init(iwm);
	if (ret) {
		dev_err(dev, "failed to init iwm_priv\n");
		goto out_wdev;
	}

	wdev->iftype = iwm_mode_to_nl80211_iftype(iwm->conf.mode);

	ndev = alloc_netdev_mq(0, "wlan%d", ether_setup, IWM_TX_QUEUES);
	if (!ndev) {
		dev_err(dev, "no memory for network device instance\n");
		goto out_priv;
	}

	ndev->netdev_ops = &iwm_netdev_ops;
	ndev->ieee80211_ptr = wdev;
	SET_NETDEV_DEV(ndev, wiphy_dev(wdev->wiphy));
	wdev->netdev = ndev;

	iwm->umac_profile = kmalloc(sizeof(struct iwm_umac_profile),
				    GFP_KERNEL);
	if (!iwm->umac_profile) {
		dev_err(dev, "Couldn't alloc memory for profile\n");
		goto out_profile;
	}

	iwm_init_default_profile(iwm, iwm->umac_profile);

	return iwm;

 out_profile:
	free_netdev(ndev);

 out_priv:
	iwm_priv_deinit(iwm);

 out_wdev:
	iwm_wdev_free(iwm);
	return ERR_PTR(ret);
}

void iwm_if_free(struct iwm_priv *iwm)
{
	if (!iwm_to_ndev(iwm))
		return;

	free_netdev(iwm_to_ndev(iwm));
	iwm_priv_deinit(iwm);
	kfree(iwm->umac_profile);
	iwm->umac_profile = NULL;
	iwm_wdev_free(iwm);
}

int iwm_if_add(struct iwm_priv *iwm)
{
	struct net_device *ndev = iwm_to_ndev(iwm);
	int ret;

	ret = register_netdev(ndev);
	if (ret < 0) {
		dev_err(&ndev->dev, "Failed to register netdev: %d\n", ret);
		return ret;
	}

	return 0;
}

void iwm_if_remove(struct iwm_priv *iwm)
{
	unregister_netdev(iwm_to_ndev(iwm));
}
