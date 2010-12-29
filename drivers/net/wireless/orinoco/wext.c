
#include <linux/kernel.h>
#include <linux/if_arp.h>
#include <linux/wireless.h>
#include <linux/ieee80211.h>
#include <net/iw_handler.h>
#include <net/cfg80211.h>

#include "hermes.h"
#include "hermes_rid.h"
#include "orinoco.h"

#include "hw.h"
#include "mic.h"
#include "scan.h"
#include "main.h"

#include "wext.h"

#define MAX_RID_LEN 1024


static int orinoco_set_key(struct orinoco_private *priv, int index,
			   enum orinoco_alg alg, const u8 *key, int key_len,
			   const u8 *seq, int seq_len)
{
	kzfree(priv->keys[index].key);
	kzfree(priv->keys[index].seq);

	if (key_len) {
		priv->keys[index].key = kzalloc(key_len, GFP_ATOMIC);
		if (!priv->keys[index].key)
			goto nomem;
	} else
		priv->keys[index].key = NULL;

	if (seq_len) {
		priv->keys[index].seq = kzalloc(seq_len, GFP_ATOMIC);
		if (!priv->keys[index].seq)
			goto free_key;
	} else
		priv->keys[index].seq = NULL;

	priv->keys[index].key_len = key_len;
	priv->keys[index].seq_len = seq_len;

	if (key_len)
		memcpy(priv->keys[index].key, key, key_len);
	if (seq_len)
		memcpy(priv->keys[index].seq, seq, seq_len);

	switch (alg) {
	case ORINOCO_ALG_TKIP:
		priv->keys[index].cipher = WLAN_CIPHER_SUITE_TKIP;
		break;

	case ORINOCO_ALG_WEP:
		priv->keys[index].cipher = (key_len > SMALL_KEY_SIZE) ?
			WLAN_CIPHER_SUITE_WEP104 : WLAN_CIPHER_SUITE_WEP40;
		break;

	case ORINOCO_ALG_NONE:
	default:
		priv->keys[index].cipher = 0;
		break;
	}

	return 0;

free_key:
	kfree(priv->keys[index].key);
	priv->keys[index].key = NULL;

nomem:
	priv->keys[index].key_len = 0;
	priv->keys[index].seq_len = 0;
	priv->keys[index].cipher = 0;

	return -ENOMEM;
}

static struct iw_statistics *orinoco_get_wireless_stats(struct net_device *dev)
{
	struct orinoco_private *priv = ndev_priv(dev);
	hermes_t *hw = &priv->hw;
	struct iw_statistics *wstats = &priv->wstats;
	int err;
	unsigned long flags;

	if (!netif_device_present(dev)) {
		printk(KERN_WARNING "%s: get_wireless_stats() called while device not present\n",
		       dev->name);
		return NULL; 
	}

	
	if (orinoco_lock(priv, &flags) != 0)
		return wstats;

	
	
	hermes_inquire(hw, HERMES_INQ_TALLIES);

	if (priv->iw_mode == NL80211_IFTYPE_ADHOC) {
		memset(&wstats->qual, 0, sizeof(wstats->qual));
		
		if (SPY_NUMBER(priv)) {
			wstats->qual.qual = priv->spy_data.spy_stat[0].qual;
			wstats->qual.level = priv->spy_data.spy_stat[0].level;
			wstats->qual.noise = priv->spy_data.spy_stat[0].noise;
			wstats->qual.updated =
				priv->spy_data.spy_stat[0].updated;
		}
	} else {
		struct {
			__le16 qual, signal, noise, unused;
		} __attribute__ ((packed)) cq;

		err = HERMES_READ_RECORD(hw, USER_BAP,
					 HERMES_RID_COMMSQUALITY, &cq);

		if (!err) {
			wstats->qual.qual = (int)le16_to_cpu(cq.qual);
			wstats->qual.level = (int)le16_to_cpu(cq.signal) - 0x95;
			wstats->qual.noise = (int)le16_to_cpu(cq.noise) - 0x95;
			wstats->qual.updated =
				IW_QUAL_ALL_UPDATED | IW_QUAL_DBM;
		}
	}

	orinoco_unlock(priv, &flags);
	return wstats;
}





static int orinoco_ioctl_setwap(struct net_device *dev,
				struct iw_request_info *info,
				struct sockaddr *ap_addr,
				char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int err = -EINPROGRESS;		
	unsigned long flags;
	static const u8 off_addr[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const u8 any_addr[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	
	if (memcmp(&ap_addr->sa_data, off_addr, ETH_ALEN) == 0 ||
	    memcmp(&ap_addr->sa_data, any_addr, ETH_ALEN) == 0) {
		priv->bssid_fixed = 0;
		memset(priv->desired_bssid, 0, ETH_ALEN);

		
		if (ap_addr->sa_data[0] == 0) {
			__orinoco_hw_set_wap(priv);
			err = 0;
		}
		goto out;
	}

	if (priv->firmware_type == FIRMWARE_TYPE_AGERE) {
		printk(KERN_WARNING "%s: Lucent/Agere firmware doesn't "
		       "support manual roaming\n",
		       dev->name);
		err = -EOPNOTSUPP;
		goto out;
	}

	if (priv->iw_mode != NL80211_IFTYPE_STATION) {
		printk(KERN_WARNING "%s: Manual roaming supported only in "
		       "managed mode\n", dev->name);
		err = -EOPNOTSUPP;
		goto out;
	}

	
	if (priv->firmware_type == FIRMWARE_TYPE_INTERSIL &&
	    strlen(priv->desired_essid) == 0) {
		printk(KERN_WARNING "%s: Desired ESSID must be set for "
		       "manual roaming\n", dev->name);
		err = -EOPNOTSUPP;
		goto out;
	}

	
	priv->bssid_fixed = 1;
	memcpy(priv->desired_bssid, &ap_addr->sa_data, ETH_ALEN);

 out:
	orinoco_unlock(priv, &flags);
	return err;
}

static int orinoco_ioctl_getwap(struct net_device *dev,
				struct iw_request_info *info,
				struct sockaddr *ap_addr,
				char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);

	int err = 0;
	unsigned long flags;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	ap_addr->sa_family = ARPHRD_ETHER;
	err = orinoco_hw_get_current_bssid(priv, ap_addr->sa_data);

	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_setiwencode(struct net_device *dev,
				     struct iw_request_info *info,
				     struct iw_point *erq,
				     char *keybuf)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int index = (erq->flags & IW_ENCODE_INDEX) - 1;
	int setindex = priv->tx_key;
	enum orinoco_alg encode_alg = priv->encode_alg;
	int restricted = priv->wep_restrict;
	int err = -EINPROGRESS;		
	unsigned long flags;

	if (!priv->has_wep)
		return -EOPNOTSUPP;

	if (erq->pointer) {
		
		if (erq->length > LARGE_KEY_SIZE)
			return -E2BIG;

		if ((erq->length > SMALL_KEY_SIZE) && !priv->has_big_wep)
			return -E2BIG;
	}

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	
	if ((priv->has_wpa) && (priv->encode_alg == ORINOCO_ALG_TKIP))
		(void) orinoco_clear_tkip_key(priv, setindex);

	if (erq->length > 0) {
		if ((index < 0) || (index >= ORINOCO_MAX_KEYS))
			index = priv->tx_key;

		
		if (encode_alg != ORINOCO_ALG_WEP) {
			setindex = index;
			encode_alg = ORINOCO_ALG_WEP;
		}
	} else {
		
		if ((index < 0) || (index >= ORINOCO_MAX_KEYS)) {
			if ((index != -1) || (erq->flags == 0)) {
				err = -EINVAL;
				goto out;
			}
		} else {
			
			if (priv->keys[index].key_len == 0) {
				err = -EINVAL;
				goto out;
			}
			setindex = index;
		}
	}

	if (erq->flags & IW_ENCODE_DISABLED)
		encode_alg = ORINOCO_ALG_NONE;
	if (erq->flags & IW_ENCODE_OPEN)
		restricted = 0;
	if (erq->flags & IW_ENCODE_RESTRICTED)
		restricted = 1;

	if (erq->pointer && erq->length > 0) {
		err = orinoco_set_key(priv, index, ORINOCO_ALG_WEP, keybuf,
				      erq->length, NULL, 0);
	}
	priv->tx_key = setindex;

	
	if ((priv->encode_alg == encode_alg) &&
	    (priv->wep_restrict == restricted) &&
	    netif_carrier_ok(dev)) {
		err = __orinoco_hw_setup_wepkeys(priv);
		
		goto out;
	}

	priv->encode_alg = encode_alg;
	priv->wep_restrict = restricted;

 out:
	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_getiwencode(struct net_device *dev,
				     struct iw_request_info *info,
				     struct iw_point *erq,
				     char *keybuf)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int index = (erq->flags & IW_ENCODE_INDEX) - 1;
	unsigned long flags;

	if (!priv->has_wep)
		return -EOPNOTSUPP;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	if ((index < 0) || (index >= ORINOCO_MAX_KEYS))
		index = priv->tx_key;

	erq->flags = 0;
	if (!priv->encode_alg)
		erq->flags |= IW_ENCODE_DISABLED;
	erq->flags |= index + 1;

	if (priv->wep_restrict)
		erq->flags |= IW_ENCODE_RESTRICTED;
	else
		erq->flags |= IW_ENCODE_OPEN;

	erq->length = priv->keys[index].key_len;

	memcpy(keybuf, priv->keys[index].key, erq->length);

	orinoco_unlock(priv, &flags);
	return 0;
}

static int orinoco_ioctl_setessid(struct net_device *dev,
				  struct iw_request_info *info,
				  struct iw_point *erq,
				  char *essidbuf)
{
	struct orinoco_private *priv = ndev_priv(dev);
	unsigned long flags;

	

	
	if (erq->length > IW_ESSID_MAX_SIZE)
		return -E2BIG;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	
	memset(priv->desired_essid, 0, sizeof(priv->desired_essid));

	
	if (erq->flags)
		memcpy(priv->desired_essid, essidbuf, erq->length);

	orinoco_unlock(priv, &flags);

	return -EINPROGRESS;		
}

static int orinoco_ioctl_getessid(struct net_device *dev,
				  struct iw_request_info *info,
				  struct iw_point *erq,
				  char *essidbuf)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int active;
	int err = 0;
	unsigned long flags;

	if (netif_running(dev)) {
		err = orinoco_hw_get_essid(priv, &active, essidbuf);
		if (err < 0)
			return err;
		erq->length = err;
	} else {
		if (orinoco_lock(priv, &flags) != 0)
			return -EBUSY;
		memcpy(essidbuf, priv->desired_essid, IW_ESSID_MAX_SIZE);
		erq->length = strlen(priv->desired_essid);
		orinoco_unlock(priv, &flags);
	}

	erq->flags = 1;

	return 0;
}

static int orinoco_ioctl_setfreq(struct net_device *dev,
				 struct iw_request_info *info,
				 struct iw_freq *frq,
				 char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int chan = -1;
	unsigned long flags;
	int err = -EINPROGRESS;		

	
	if (priv->iw_mode == NL80211_IFTYPE_STATION)
		return -EBUSY;

	if ((frq->e == 0) && (frq->m <= 1000)) {
		
		chan = frq->m;
	} else {
		
		int denom = 1;
		int i;

		
		for (i = 0; i < (6 - frq->e); i++)
			denom *= 10;

		chan = ieee80211_freq_to_dsss_chan(frq->m / denom);
	}

	if ((chan < 1) || (chan > NUM_CHANNELS) ||
	     !(priv->channel_mask & (1 << (chan-1))))
		return -EINVAL;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	priv->channel = chan;
	if (priv->iw_mode == NL80211_IFTYPE_MONITOR) {
		
		hermes_t *hw = &priv->hw;
		err = hermes_docmd_wait(hw, HERMES_CMD_TEST |
					    HERMES_TEST_SET_CHANNEL,
					chan, NULL);
	}
	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_getfreq(struct net_device *dev,
				 struct iw_request_info *info,
				 struct iw_freq *frq,
				 char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int tmp;

	
	tmp = orinoco_hw_get_freq(priv);
	if (tmp < 0)
		return tmp;

	frq->m = tmp * 100000;
	frq->e = 1;

	return 0;
}

static int orinoco_ioctl_getsens(struct net_device *dev,
				 struct iw_request_info *info,
				 struct iw_param *srq,
				 char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	hermes_t *hw = &priv->hw;
	u16 val;
	int err;
	unsigned long flags;

	if (!priv->has_sensitivity)
		return -EOPNOTSUPP;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;
	err = hermes_read_wordrec(hw, USER_BAP,
				  HERMES_RID_CNFSYSTEMSCALE, &val);
	orinoco_unlock(priv, &flags);

	if (err)
		return err;

	srq->value = val;
	srq->fixed = 0; 

	return 0;
}

static int orinoco_ioctl_setsens(struct net_device *dev,
				 struct iw_request_info *info,
				 struct iw_param *srq,
				 char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int val = srq->value;
	unsigned long flags;

	if (!priv->has_sensitivity)
		return -EOPNOTSUPP;

	if ((val < 1) || (val > 3))
		return -EINVAL;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;
	priv->ap_density = val;
	orinoco_unlock(priv, &flags);

	return -EINPROGRESS;		
}

static int orinoco_ioctl_setrts(struct net_device *dev,
				struct iw_request_info *info,
				struct iw_param *rrq,
				char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int val = rrq->value;
	unsigned long flags;

	if (rrq->disabled)
		val = 2347;

	if ((val < 0) || (val > 2347))
		return -EINVAL;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	priv->rts_thresh = val;
	orinoco_unlock(priv, &flags);

	return -EINPROGRESS;		
}

static int orinoco_ioctl_getrts(struct net_device *dev,
				struct iw_request_info *info,
				struct iw_param *rrq,
				char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);

	rrq->value = priv->rts_thresh;
	rrq->disabled = (rrq->value == 2347);
	rrq->fixed = 1;

	return 0;
}

static int orinoco_ioctl_setfrag(struct net_device *dev,
				 struct iw_request_info *info,
				 struct iw_param *frq,
				 char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int err = -EINPROGRESS;		
	unsigned long flags;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	if (priv->has_mwo) {
		if (frq->disabled)
			priv->mwo_robust = 0;
		else {
			if (frq->fixed)
				printk(KERN_WARNING "%s: Fixed fragmentation "
				       "is not supported on this firmware. "
				       "Using MWO robust instead.\n",
				       dev->name);
			priv->mwo_robust = 1;
		}
	} else {
		if (frq->disabled)
			priv->frag_thresh = 2346;
		else {
			if ((frq->value < 256) || (frq->value > 2346))
				err = -EINVAL;
			else
				
				priv->frag_thresh = frq->value & ~0x1;
		}
	}

	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_getfrag(struct net_device *dev,
				 struct iw_request_info *info,
				 struct iw_param *frq,
				 char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	hermes_t *hw = &priv->hw;
	int err;
	u16 val;
	unsigned long flags;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	if (priv->has_mwo) {
		err = hermes_read_wordrec(hw, USER_BAP,
					  HERMES_RID_CNFMWOROBUST_AGERE,
					  &val);
		if (err)
			val = 0;

		frq->value = val ? 2347 : 0;
		frq->disabled = !val;
		frq->fixed = 0;
	} else {
		err = hermes_read_wordrec(hw, USER_BAP,
					  HERMES_RID_CNFFRAGMENTATIONTHRESHOLD,
					  &val);
		if (err)
			val = 0;

		frq->value = val;
		frq->disabled = (val >= 2346);
		frq->fixed = 1;
	}

	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_setrate(struct net_device *dev,
				 struct iw_request_info *info,
				 struct iw_param *rrq,
				 char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int ratemode;
	int bitrate; 
	unsigned long flags;

	
	if (rrq->value == -1)
		bitrate = 110;
	else {
		if (rrq->value % 100000)
			return -EINVAL;
		bitrate = rrq->value / 100000;
	}

	ratemode = orinoco_get_bitratemode(bitrate, !rrq->fixed);

	if (ratemode == -1)
		return -EINVAL;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;
	priv->bitratemode = ratemode;
	orinoco_unlock(priv, &flags);

	return -EINPROGRESS;
}

static int orinoco_ioctl_getrate(struct net_device *dev,
				 struct iw_request_info *info,
				 struct iw_param *rrq,
				 char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int err = 0;
	int bitrate, automatic;
	unsigned long flags;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	orinoco_get_ratemode_cfg(priv->bitratemode, &bitrate, &automatic);

	
	if (netif_running(dev))
		err = orinoco_hw_get_act_bitrate(priv, &bitrate);

	orinoco_unlock(priv, &flags);

	rrq->value = bitrate;
	rrq->fixed = !automatic;
	rrq->disabled = 0;

	return err;
}

static int orinoco_ioctl_setpower(struct net_device *dev,
				  struct iw_request_info *info,
				  struct iw_param *prq,
				  char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int err = -EINPROGRESS;		
	unsigned long flags;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	if (prq->disabled) {
		priv->pm_on = 0;
	} else {
		switch (prq->flags & IW_POWER_MODE) {
		case IW_POWER_UNICAST_R:
			priv->pm_mcast = 0;
			priv->pm_on = 1;
			break;
		case IW_POWER_ALL_R:
			priv->pm_mcast = 1;
			priv->pm_on = 1;
			break;
		case IW_POWER_ON:
			
			break;
		default:
			err = -EINVAL;
			goto out;
		}

		if (prq->flags & IW_POWER_TIMEOUT) {
			priv->pm_on = 1;
			priv->pm_timeout = prq->value / 1000;
		}
		if (prq->flags & IW_POWER_PERIOD) {
			priv->pm_on = 1;
			priv->pm_period = prq->value / 1000;
		}
		
		if (!priv->pm_on) {
			err = -EINVAL;
			goto out;
		}
	}

 out:
	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_getpower(struct net_device *dev,
				  struct iw_request_info *info,
				  struct iw_param *prq,
				  char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	hermes_t *hw = &priv->hw;
	int err = 0;
	u16 enable, period, timeout, mcast;
	unsigned long flags;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	err = hermes_read_wordrec(hw, USER_BAP,
				  HERMES_RID_CNFPMENABLED, &enable);
	if (err)
		goto out;

	err = hermes_read_wordrec(hw, USER_BAP,
				  HERMES_RID_CNFMAXSLEEPDURATION, &period);
	if (err)
		goto out;

	err = hermes_read_wordrec(hw, USER_BAP,
				  HERMES_RID_CNFPMHOLDOVERDURATION, &timeout);
	if (err)
		goto out;

	err = hermes_read_wordrec(hw, USER_BAP,
				  HERMES_RID_CNFMULTICASTRECEIVE, &mcast);
	if (err)
		goto out;

	prq->disabled = !enable;
	
	if ((prq->flags & IW_POWER_TYPE) == IW_POWER_TIMEOUT) {
		prq->flags = IW_POWER_TIMEOUT;
		prq->value = timeout * 1000;
	} else {
		prq->flags = IW_POWER_PERIOD;
		prq->value = period * 1000;
	}
	if (mcast)
		prq->flags |= IW_POWER_ALL_R;
	else
		prq->flags |= IW_POWER_UNICAST_R;

 out:
	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_set_encodeext(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu,
				       char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	struct iw_point *encoding = &wrqu->encoding;
	struct iw_encode_ext *ext = (struct iw_encode_ext *)extra;
	int idx, alg = ext->alg, set_key = 1;
	unsigned long flags;
	int err = -EINVAL;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	
	idx = encoding->flags & IW_ENCODE_INDEX;
	if (idx) {
		if ((idx < 1) || (idx > 4))
			goto out;
		idx--;
	} else
		idx = priv->tx_key;

	if (encoding->flags & IW_ENCODE_DISABLED)
		alg = IW_ENCODE_ALG_NONE;

	if (priv->has_wpa && (alg != IW_ENCODE_ALG_TKIP)) {
		
		(void) orinoco_clear_tkip_key(priv, priv->tx_key);
	}

	if (ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY) {
		priv->tx_key = idx;
		set_key = ((alg == IW_ENCODE_ALG_TKIP) ||
			   (ext->key_len > 0)) ? 1 : 0;
	}

	if (set_key) {
		
		switch (alg) {
		case IW_ENCODE_ALG_NONE:
			priv->encode_alg = ORINOCO_ALG_NONE;
			err = orinoco_set_key(priv, idx, ORINOCO_ALG_NONE,
					      NULL, 0, NULL, 0);
			break;

		case IW_ENCODE_ALG_WEP:
			if (ext->key_len <= 0)
				goto out;

			priv->encode_alg = ORINOCO_ALG_WEP;
			err = orinoco_set_key(priv, idx, ORINOCO_ALG_WEP,
					      ext->key, ext->key_len, NULL, 0);
			break;

		case IW_ENCODE_ALG_TKIP:
		{
			u8 *tkip_iv = NULL;

			if (!priv->has_wpa ||
			    (ext->key_len > sizeof(struct orinoco_tkip_key)))
				goto out;

			priv->encode_alg = ORINOCO_ALG_TKIP;

			if (ext->ext_flags & IW_ENCODE_EXT_RX_SEQ_VALID)
				tkip_iv = &ext->rx_seq[0];

			err = orinoco_set_key(priv, idx, ORINOCO_ALG_TKIP,
					      ext->key, ext->key_len, tkip_iv,
					      ORINOCO_SEQ_LEN);

			err = __orinoco_hw_set_tkip_key(priv, idx,
				 ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY,
				 priv->keys[idx].key,
				 tkip_iv, ORINOCO_SEQ_LEN, NULL, 0);
			if (err)
				printk(KERN_ERR "%s: Error %d setting TKIP key"
				       "\n", dev->name, err);

			goto out;
		}
		default:
			goto out;
		}
	}
	err = -EINPROGRESS;
 out:
	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_get_encodeext(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu,
				       char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	struct iw_point *encoding = &wrqu->encoding;
	struct iw_encode_ext *ext = (struct iw_encode_ext *)extra;
	int idx, max_key_len;
	unsigned long flags;
	int err;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	err = -EINVAL;
	max_key_len = encoding->length - sizeof(*ext);
	if (max_key_len < 0)
		goto out;

	idx = encoding->flags & IW_ENCODE_INDEX;
	if (idx) {
		if ((idx < 1) || (idx > 4))
			goto out;
		idx--;
	} else
		idx = priv->tx_key;

	encoding->flags = idx + 1;
	memset(ext, 0, sizeof(*ext));

	switch (priv->encode_alg) {
	case ORINOCO_ALG_NONE:
		ext->alg = IW_ENCODE_ALG_NONE;
		ext->key_len = 0;
		encoding->flags |= IW_ENCODE_DISABLED;
		break;
	case ORINOCO_ALG_WEP:
		ext->alg = IW_ENCODE_ALG_WEP;
		ext->key_len = min(priv->keys[idx].key_len, max_key_len);
		memcpy(ext->key, priv->keys[idx].key, ext->key_len);
		encoding->flags |= IW_ENCODE_ENABLED;
		break;
	case ORINOCO_ALG_TKIP:
		ext->alg = IW_ENCODE_ALG_TKIP;
		ext->key_len = min(priv->keys[idx].key_len, max_key_len);
		memcpy(ext->key, priv->keys[idx].key, ext->key_len);
		encoding->flags |= IW_ENCODE_ENABLED;
		break;
	}

	err = 0;
 out:
	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_set_auth(struct net_device *dev,
				  struct iw_request_info *info,
				  union iwreq_data *wrqu, char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	hermes_t *hw = &priv->hw;
	struct iw_param *param = &wrqu->param;
	unsigned long flags;
	int ret = -EINPROGRESS;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	switch (param->flags & IW_AUTH_INDEX) {
	case IW_AUTH_WPA_VERSION:
	case IW_AUTH_CIPHER_PAIRWISE:
	case IW_AUTH_CIPHER_GROUP:
	case IW_AUTH_RX_UNENCRYPTED_EAPOL:
	case IW_AUTH_PRIVACY_INVOKED:
	case IW_AUTH_DROP_UNENCRYPTED:
		
		break;

	case IW_AUTH_KEY_MGMT:
		
		priv->key_mgmt = param->value;
		break;

	case IW_AUTH_TKIP_COUNTERMEASURES:
		
		if (param->value) {
			priv->tkip_cm_active = 1;
			ret = hermes_enable_port(hw, 0);
		} else {
			priv->tkip_cm_active = 0;
			ret = hermes_disable_port(hw, 0);
		}
		break;

	case IW_AUTH_80211_AUTH_ALG:
		if (param->value & IW_AUTH_ALG_SHARED_KEY)
			priv->wep_restrict = 1;
		else if (param->value & IW_AUTH_ALG_OPEN_SYSTEM)
			priv->wep_restrict = 0;
		else
			ret = -EINVAL;
		break;

	case IW_AUTH_WPA_ENABLED:
		if (priv->has_wpa) {
			priv->wpa_enabled = param->value ? 1 : 0;
		} else {
			if (param->value)
				ret = -EOPNOTSUPP;
			
			priv->wpa_enabled = 0;
		}
		break;

	default:
		ret = -EOPNOTSUPP;
	}

	orinoco_unlock(priv, &flags);
	return ret;
}

static int orinoco_ioctl_get_auth(struct net_device *dev,
				  struct iw_request_info *info,
				  union iwreq_data *wrqu, char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	struct iw_param *param = &wrqu->param;
	unsigned long flags;
	int ret = 0;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	switch (param->flags & IW_AUTH_INDEX) {
	case IW_AUTH_KEY_MGMT:
		param->value = priv->key_mgmt;
		break;

	case IW_AUTH_TKIP_COUNTERMEASURES:
		param->value = priv->tkip_cm_active;
		break;

	case IW_AUTH_80211_AUTH_ALG:
		if (priv->wep_restrict)
			param->value = IW_AUTH_ALG_SHARED_KEY;
		else
			param->value = IW_AUTH_ALG_OPEN_SYSTEM;
		break;

	case IW_AUTH_WPA_ENABLED:
		param->value = priv->wpa_enabled;
		break;

	default:
		ret = -EOPNOTSUPP;
	}

	orinoco_unlock(priv, &flags);
	return ret;
}

static int orinoco_ioctl_set_genie(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	u8 *buf;
	unsigned long flags;

	
	if ((wrqu->data.length > IEEE80211_MAX_DATA_LEN) ||
	    (wrqu->data.length && (extra == NULL)))
		return -EINVAL;

	if (wrqu->data.length) {
		buf = kmalloc(wrqu->data.length, GFP_KERNEL);
		if (buf == NULL)
			return -ENOMEM;

		memcpy(buf, extra, wrqu->data.length);
	} else
		buf = NULL;

	if (orinoco_lock(priv, &flags) != 0) {
		kfree(buf);
		return -EBUSY;
	}

	kfree(priv->wpa_ie);
	priv->wpa_ie = buf;
	priv->wpa_ie_len = wrqu->data.length;

	if (priv->wpa_ie) {
		
	}

	orinoco_unlock(priv, &flags);
	return 0;
}

static int orinoco_ioctl_get_genie(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	unsigned long flags;
	int err = 0;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	if ((priv->wpa_ie_len == 0) || (priv->wpa_ie == NULL)) {
		wrqu->data.length = 0;
		goto out;
	}

	if (wrqu->data.length < priv->wpa_ie_len) {
		err = -E2BIG;
		goto out;
	}

	wrqu->data.length = priv->wpa_ie_len;
	memcpy(extra, priv->wpa_ie, priv->wpa_ie_len);

out:
	orinoco_unlock(priv, &flags);
	return err;
}

static int orinoco_ioctl_set_mlme(struct net_device *dev,
				  struct iw_request_info *info,
				  union iwreq_data *wrqu, char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	struct iw_mlme *mlme = (struct iw_mlme *)extra;
	unsigned long flags;
	int ret = 0;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	switch (mlme->cmd) {
	case IW_MLME_DEAUTH:
		
		break;

	case IW_MLME_DISASSOC:

		ret = orinoco_hw_disassociate(priv, mlme->addr.sa_data,
					      mlme->reason_code);
		break;

	default:
		ret = -EOPNOTSUPP;
	}

	orinoco_unlock(priv, &flags);
	return ret;
}

static int orinoco_ioctl_getretry(struct net_device *dev,
				  struct iw_request_info *info,
				  struct iw_param *rrq,
				  char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	hermes_t *hw = &priv->hw;
	int err = 0;
	u16 short_limit, long_limit, lifetime;
	unsigned long flags;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	err = hermes_read_wordrec(hw, USER_BAP, HERMES_RID_SHORTRETRYLIMIT,
				  &short_limit);
	if (err)
		goto out;

	err = hermes_read_wordrec(hw, USER_BAP, HERMES_RID_LONGRETRYLIMIT,
				  &long_limit);
	if (err)
		goto out;

	err = hermes_read_wordrec(hw, USER_BAP, HERMES_RID_MAXTRANSMITLIFETIME,
				  &lifetime);
	if (err)
		goto out;

	rrq->disabled = 0;		

	
	if ((rrq->flags & IW_RETRY_TYPE) == IW_RETRY_LIFETIME) {
		rrq->flags = IW_RETRY_LIFETIME;
		rrq->value = lifetime * 1000;	
	} else {
		
		if ((rrq->flags & IW_RETRY_LONG)) {
			rrq->flags = IW_RETRY_LIMIT | IW_RETRY_LONG;
			rrq->value = long_limit;
		} else {
			rrq->flags = IW_RETRY_LIMIT;
			rrq->value = short_limit;
			if (short_limit != long_limit)
				rrq->flags |= IW_RETRY_SHORT;
		}
	}

 out:
	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_reset(struct net_device *dev,
			       struct iw_request_info *info,
			       void *wrqu,
			       char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (info->cmd == (SIOCIWFIRSTPRIV + 0x1)) {
		printk(KERN_DEBUG "%s: Forcing reset!\n", dev->name);

		
		orinoco_reset(&priv->reset_work);
	} else {
		printk(KERN_DEBUG "%s: Force scheduling reset!\n", dev->name);

		schedule_work(&priv->reset_work);
	}

	return 0;
}

static int orinoco_ioctl_setibssport(struct net_device *dev,
				     struct iw_request_info *info,
				     void *wrqu,
				     char *extra)

{
	struct orinoco_private *priv = ndev_priv(dev);
	int val = *((int *) extra);
	unsigned long flags;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	priv->ibss_port = val;

	
	set_port_type(priv);

	orinoco_unlock(priv, &flags);
	return -EINPROGRESS;		
}

static int orinoco_ioctl_getibssport(struct net_device *dev,
				     struct iw_request_info *info,
				     void *wrqu,
				     char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int *val = (int *) extra;

	*val = priv->ibss_port;
	return 0;
}

static int orinoco_ioctl_setport3(struct net_device *dev,
				  struct iw_request_info *info,
				  void *wrqu,
				  char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int val = *((int *) extra);
	int err = 0;
	unsigned long flags;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	switch (val) {
	case 0: 
		if (!priv->has_ibss) {
			err = -EINVAL;
			break;
		}
		priv->prefer_port3 = 0;

		break;

	case 1: 
		if (!priv->has_port3) {
			err = -EINVAL;
			break;
		}
		priv->prefer_port3 = 1;
		break;

	default:
		err = -EINVAL;
	}

	if (!err) {
		
		set_port_type(priv);
		err = -EINPROGRESS;
	}

	orinoco_unlock(priv, &flags);

	return err;
}

static int orinoco_ioctl_getport3(struct net_device *dev,
				  struct iw_request_info *info,
				  void *wrqu,
				  char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int *val = (int *) extra;

	*val = priv->prefer_port3;
	return 0;
}

static int orinoco_ioctl_setpreamble(struct net_device *dev,
				     struct iw_request_info *info,
				     void *wrqu,
				     char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	unsigned long flags;
	int val;

	if (!priv->has_preamble)
		return -EOPNOTSUPP;

	
	val = *((int *) extra);

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	if (val)
		priv->preamble = 1;
	else
		priv->preamble = 0;

	orinoco_unlock(priv, &flags);

	return -EINPROGRESS;		
}

static int orinoco_ioctl_getpreamble(struct net_device *dev,
				     struct iw_request_info *info,
				     void *wrqu,
				     char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	int *val = (int *) extra;

	if (!priv->has_preamble)
		return -EOPNOTSUPP;

	*val = priv->preamble;
	return 0;
}


static int orinoco_ioctl_getrid(struct net_device *dev,
				struct iw_request_info *info,
				struct iw_point *data,
				char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	hermes_t *hw = &priv->hw;
	int rid = data->flags;
	u16 length;
	int err;
	unsigned long flags;

	
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (rid < 0xfc00 || rid > 0xffff)
		return -EINVAL;

	if (orinoco_lock(priv, &flags) != 0)
		return -EBUSY;

	err = hermes_read_ltv(hw, USER_BAP, rid, MAX_RID_LEN, &length,
			      extra);
	if (err)
		goto out;

	data->length = min_t(u16, HERMES_RECLEN_TO_BYTES(length),
			     MAX_RID_LEN);

 out:
	orinoco_unlock(priv, &flags);
	return err;
}



static int orinoco_ioctl_commit(struct net_device *dev,
				struct iw_request_info *info,
				void *wrqu,
				char *extra)
{
	struct orinoco_private *priv = ndev_priv(dev);
	unsigned long flags;
	int err = 0;

	if (!priv->open)
		return 0;

	if (orinoco_lock(priv, &flags) != 0)
		return err;

	err = orinoco_commit(priv);

	orinoco_unlock(priv, &flags);
	return err;
}

static const struct iw_priv_args orinoco_privtab[] = {
	{ SIOCIWFIRSTPRIV + 0x0, 0, 0, "force_reset" },
	{ SIOCIWFIRSTPRIV + 0x1, 0, 0, "card_reset" },
	{ SIOCIWFIRSTPRIV + 0x2, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	  0, "set_port3" },
	{ SIOCIWFIRSTPRIV + 0x3, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	  "get_port3" },
	{ SIOCIWFIRSTPRIV + 0x4, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	  0, "set_preamble" },
	{ SIOCIWFIRSTPRIV + 0x5, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	  "get_preamble" },
	{ SIOCIWFIRSTPRIV + 0x6, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	  0, "set_ibssport" },
	{ SIOCIWFIRSTPRIV + 0x7, 0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	  "get_ibssport" },
	{ SIOCIWFIRSTPRIV + 0x9, 0, IW_PRIV_TYPE_BYTE | MAX_RID_LEN,
	  "get_rid" },
};




#define STD_IW_HANDLER(id, func) \
	[IW_IOCTL_IDX(id)] = (iw_handler) func
static const iw_handler	orinoco_handler[] = {
	STD_IW_HANDLER(SIOCSIWCOMMIT,	orinoco_ioctl_commit),
	STD_IW_HANDLER(SIOCGIWNAME,	cfg80211_wext_giwname),
	STD_IW_HANDLER(SIOCSIWFREQ,	orinoco_ioctl_setfreq),
	STD_IW_HANDLER(SIOCGIWFREQ,	orinoco_ioctl_getfreq),
	STD_IW_HANDLER(SIOCSIWMODE,	cfg80211_wext_siwmode),
	STD_IW_HANDLER(SIOCGIWMODE,	cfg80211_wext_giwmode),
	STD_IW_HANDLER(SIOCSIWSENS,	orinoco_ioctl_setsens),
	STD_IW_HANDLER(SIOCGIWSENS,	orinoco_ioctl_getsens),
	STD_IW_HANDLER(SIOCGIWRANGE,	cfg80211_wext_giwrange),
	STD_IW_HANDLER(SIOCSIWSPY,	iw_handler_set_spy),
	STD_IW_HANDLER(SIOCGIWSPY,	iw_handler_get_spy),
	STD_IW_HANDLER(SIOCSIWTHRSPY,	iw_handler_set_thrspy),
	STD_IW_HANDLER(SIOCGIWTHRSPY,	iw_handler_get_thrspy),
	STD_IW_HANDLER(SIOCSIWAP,	orinoco_ioctl_setwap),
	STD_IW_HANDLER(SIOCGIWAP,	orinoco_ioctl_getwap),
	STD_IW_HANDLER(SIOCSIWSCAN,	cfg80211_wext_siwscan),
	STD_IW_HANDLER(SIOCGIWSCAN,	cfg80211_wext_giwscan),
	STD_IW_HANDLER(SIOCSIWESSID,	orinoco_ioctl_setessid),
	STD_IW_HANDLER(SIOCGIWESSID,	orinoco_ioctl_getessid),
	STD_IW_HANDLER(SIOCSIWRATE,	orinoco_ioctl_setrate),
	STD_IW_HANDLER(SIOCGIWRATE,	orinoco_ioctl_getrate),
	STD_IW_HANDLER(SIOCSIWRTS,	orinoco_ioctl_setrts),
	STD_IW_HANDLER(SIOCGIWRTS,	orinoco_ioctl_getrts),
	STD_IW_HANDLER(SIOCSIWFRAG,	orinoco_ioctl_setfrag),
	STD_IW_HANDLER(SIOCGIWFRAG,	orinoco_ioctl_getfrag),
	STD_IW_HANDLER(SIOCGIWRETRY,	orinoco_ioctl_getretry),
	STD_IW_HANDLER(SIOCSIWENCODE,	orinoco_ioctl_setiwencode),
	STD_IW_HANDLER(SIOCGIWENCODE,	orinoco_ioctl_getiwencode),
	STD_IW_HANDLER(SIOCSIWPOWER,	orinoco_ioctl_setpower),
	STD_IW_HANDLER(SIOCGIWPOWER,	orinoco_ioctl_getpower),
	STD_IW_HANDLER(SIOCSIWGENIE,	orinoco_ioctl_set_genie),
	STD_IW_HANDLER(SIOCGIWGENIE,	orinoco_ioctl_get_genie),
	STD_IW_HANDLER(SIOCSIWMLME,	orinoco_ioctl_set_mlme),
	STD_IW_HANDLER(SIOCSIWAUTH,	orinoco_ioctl_set_auth),
	STD_IW_HANDLER(SIOCGIWAUTH,	orinoco_ioctl_get_auth),
	STD_IW_HANDLER(SIOCSIWENCODEEXT, orinoco_ioctl_set_encodeext),
	STD_IW_HANDLER(SIOCGIWENCODEEXT, orinoco_ioctl_get_encodeext),
};



static const iw_handler	orinoco_private_handler[] = {
	[0] = (iw_handler) orinoco_ioctl_reset,
	[1] = (iw_handler) orinoco_ioctl_reset,
	[2] = (iw_handler) orinoco_ioctl_setport3,
	[3] = (iw_handler) orinoco_ioctl_getport3,
	[4] = (iw_handler) orinoco_ioctl_setpreamble,
	[5] = (iw_handler) orinoco_ioctl_getpreamble,
	[6] = (iw_handler) orinoco_ioctl_setibssport,
	[7] = (iw_handler) orinoco_ioctl_getibssport,
	[9] = (iw_handler) orinoco_ioctl_getrid,
};

const struct iw_handler_def orinoco_handler_def = {
	.num_standard = ARRAY_SIZE(orinoco_handler),
	.num_private = ARRAY_SIZE(orinoco_private_handler),
	.num_private_args = ARRAY_SIZE(orinoco_privtab),
	.standard = orinoco_handler,
	.private = orinoco_private_handler,
	.private_args = orinoco_privtab,
	.get_wireless_stats = orinoco_get_wireless_stats,
};
