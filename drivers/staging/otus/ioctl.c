










#include <linux/module.h>
#include <linux/if_arp.h>
#include <linux/uaccess.h>

#include "usbdrv.h"

#define ZD_IOCTL_WPA			    (SIOCDEVPRIVATE + 1)
#define ZD_IOCTL_PARAM			    (SIOCDEVPRIVATE + 2)
#define ZD_IOCTL_GETWPAIE		    (SIOCDEVPRIVATE + 3)
#ifdef ZM_ENABLE_CENC
#define ZM_IOCTL_CENC               (SIOCDEVPRIVATE + 4)
#endif  
#define ZD_PARAM_ROAMING		    0x0001
#define ZD_PARAM_PRIVACY		    0x0002
#define ZD_PARAM_WPA			    0x0003
#define ZD_PARAM_COUNTERMEASURES	0x0004
#define ZD_PARAM_DROPUNENCRYPTED	0x0005
#define ZD_PARAM_AUTH_ALGS		    0x0006
#define ZD_PARAM_WPS_FILTER		    0x0007

#ifdef ZM_ENABLE_CENC
#define P80211_PACKET_CENCFLAG		0x0001
#endif  
#define P80211_PACKET_SETKEY     	0x0003

#define ZD_CMD_SET_ENCRYPT_KEY		0x0001
#define ZD_CMD_SET_MLME			    0x0002
#define ZD_CMD_SCAN_REQ			    0x0003
#define ZD_CMD_SET_GENERIC_ELEMENT	0x0004
#define ZD_CMD_GET_TSC			    0x0005

#define ZD_CRYPT_ALG_NAME_LEN		16
#define ZD_MAX_KEY_SIZE			    32
#define ZD_MAX_GENERIC_SIZE		    64

#include <net/iw_handler.h>

extern u16_t zfLnxGetVapId(zdev_t *dev);

static const u32_t channel_frequency_11A[] =
{
	
	36, 5180,
	40, 5200,
	44, 5220,
	48, 5240,
	52, 5260,
	56, 5280,
	60, 5300,
	64, 5320,
	100, 5500,
	104, 5520,
	108, 5540,
	112, 5560,
	116, 5580,
	120, 5600,
	124, 5620,
	128, 5640,
	132, 5660,
	136, 5680,
	140, 5700,
	
	184, 4920,
	188, 4940,
	192, 4960,
	196, 4980,
	8, 5040,
	12, 5060,
	16, 5080,
	34, 5170,
	38, 5190,
	42, 5210,
	46, 5230,
	
	149, 5745,
	153, 5765,
	157, 5785,
	161, 5805,
	165, 5825
	
};

int usbdrv_freq2chan(u32_t freq)
{
	
	if (freq > 2400 && freq < 3000) {
		return ((freq-2412)/5) + 1;
	} else {
		u16_t ii;
		u16_t num_chan = sizeof(channel_frequency_11A)/sizeof(u32_t);

		for (ii = 1; ii < num_chan; ii += 2) {
			if (channel_frequency_11A[ii] == freq)
				return channel_frequency_11A[ii-1];
		}
	}

	return 0;
}

int usbdrv_chan2freq(int chan)
{
	int freq;

	
	if (chan > 165 || chan <= 0)
		return -1;

	
	if (chan >= 1 && chan <= 13) {
		freq = (2412 + (chan - 1) * 5);
			return freq;
	} else if (chan >= 36 && chan <= 165) {
		u16_t ii;
		u16_t num_chan = sizeof(channel_frequency_11A)/sizeof(u32_t);

		for (ii = 0; ii < num_chan; ii += 2) {
			if (channel_frequency_11A[ii] == chan)
				return channel_frequency_11A[ii+1];
		}

	
	if (ii == num_chan)
		return -1;
	}

	
	return -1;
}

int usbdrv_ioctl_setessid(struct net_device *dev, struct iw_point *erq)
{
	#ifdef ZM_HOSTAPD_SUPPORT
	
	char essidbuf[IW_ESSID_MAX_SIZE+1];
	int i;

	if (!netif_running(dev))
		return -EINVAL;

	memset(essidbuf, 0, sizeof(essidbuf));

	printk(KERN_ERR "usbdrv_ioctl_setessid\n");

	
	if (erq->flags) {
		if (erq->length > (IW_ESSID_MAX_SIZE+1))
			return -E2BIG;

		if (copy_from_user(essidbuf, erq->pointer, erq->length))
			return -EFAULT;
	}

	
	

	printk(KERN_ERR "essidbuf: ");

	for (i = 0; i < erq->length; i++)
		printk(KERN_ERR "%02x ", essidbuf[i]);

	printk(KERN_ERR "\n");

	essidbuf[erq->length] = '\0';
	
	
	

	zfiWlanSetSSID(dev, essidbuf, erq->length);
	#if 0
		printk(KERN_ERR "macp->wd.ws.ssid: ");

		for (i = 0; i < macp->wd.ws.ssidLen; i++)
			printk(KERN_ERR "%02x ", macp->wd.ws.ssid[i]);

		printk(KERN_ERR "\n");
	#endif

	zfiWlanDisable(dev, 0);
	zfiWlanEnable(dev);

	#endif

	return 0;
}

int usbdrv_ioctl_getessid(struct net_device *dev, struct iw_point *erq)
{
	
	u8_t essidbuf[IW_ESSID_MAX_SIZE+1];
	u8_t len;
	u8_t i;


	
	
	zfiWlanQuerySSID(dev, essidbuf, &len);

	essidbuf[len] = 0;

	printk(KERN_ERR "ESSID: ");

	for (i = 0; i < len; i++)
		printk(KERN_ERR "%c", essidbuf[i]);

	printk(KERN_ERR "\n");

	erq->flags = 1;
	erq->length = strlen(essidbuf) + 1;

	if (erq->pointer) {
		if (copy_to_user(erq->pointer, essidbuf, erq->length))
			return -EFAULT;
	}

	return 0;
}

int usbdrv_ioctl_setrts(struct net_device *dev, struct iw_param *rrq)
{
	return 0;
}


u32 encode_ie(void *buf, u32 bufsize, const u8 *ie, u32 ielen,
		const u8 *leader, u32 leader_len)
{
	u8 *p;
	u32 i;

	if (bufsize < leader_len)
		return 0;
	p = buf;
	memcpy(p, leader, leader_len);
	bufsize -= leader_len;
	p += leader_len;
	for (i = 0; i < ielen && bufsize > 2; i++)
		p += sprintf(p, "%02x", ie[i]);
	return (i == ielen ? p - (u8 *)buf:0);
}


char *usbdrv_translate_scan(struct net_device *dev,
	struct iw_request_info *info, char *current_ev,
	char *end_buf, struct zsBssInfo *list)
{
	struct iw_event iwe;   
	u16_t capabilities;
	char *current_val;     
	char *last_ev;
	int i;
	char    buf[64*2 + 30];

	last_ev = current_ev;

	
	iwe.cmd = SIOCGIWAP;
	iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
	memcpy(iwe.u.ap_addr.sa_data, list->bssid, ETH_ALEN);
	current_ev = iwe_stream_add_event(info,	current_ev,
					end_buf, &iwe, IW_EV_ADDR_LEN);

	
	if (last_ev == current_ev)
		return end_buf;

	last_ev = current_ev;

	

	
	iwe.u.data.length = list->ssid[1];
	if (iwe.u.data.length > 32)
		iwe.u.data.length = 32;
	iwe.cmd = SIOCGIWESSID;
	iwe.u.data.flags = 1;
	current_ev = iwe_stream_add_point(info,	current_ev,
					end_buf, &iwe, &list->ssid[2]);

	
	if (last_ev == current_ev)
		return end_buf;

	last_ev = current_ev;

	
	iwe.cmd = SIOCGIWMODE;
	capabilities = (list->capability[1] << 8) + list->capability[0];
	if (capabilities & (0x01 | 0x02)) {
		if (capabilities & 0x01)
			iwe.u.mode = IW_MODE_MASTER;
		else
			iwe.u.mode = IW_MODE_ADHOC;
			current_ev = iwe_stream_add_event(info,	current_ev,
						end_buf, &iwe, IW_EV_UINT_LEN);
	}

	
	if (last_ev == current_ev)
		return end_buf;

	last_ev = current_ev;

	
	iwe.cmd = SIOCGIWFREQ;
	iwe.u.freq.m = list->channel;
	
	if (iwe.u.freq.m > 14) {
		if ((184 <= iwe.u.freq.m) && (iwe.u.freq.m <= 196))
			iwe.u.freq.m = 4000 + iwe.u.freq.m * 5;
		else
			iwe.u.freq.m = 5000 + iwe.u.freq.m * 5;
	} else {
		if (iwe.u.freq.m == 14)
			iwe.u.freq.m = 2484;
		else
			iwe.u.freq.m = 2412 + (iwe.u.freq.m - 1) * 5;
	}
	iwe.u.freq.e = 6;
	current_ev = iwe_stream_add_event(info, current_ev,
					end_buf, &iwe, IW_EV_FREQ_LEN);

	
	if (last_ev == current_ev)
		return end_buf;

	last_ev = current_ev;

	
	iwe.cmd = IWEVQUAL;
	iwe.u.qual.updated = IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_UPDATED
				| IW_QUAL_NOISE_UPDATED;
	iwe.u.qual.level = list->signalStrength;
	iwe.u.qual.noise = 0;
	iwe.u.qual.qual = list->signalQuality;
	current_ev = iwe_stream_add_event(info,	current_ev,
					end_buf, &iwe, IW_EV_QUAL_LEN);

	
	if (last_ev == current_ev)
		return end_buf;

	last_ev = current_ev;

	

	iwe.cmd = SIOCGIWENCODE;
	if (capabilities & 0x10)
		iwe.u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
	else
		iwe.u.data.flags = IW_ENCODE_DISABLED;

	iwe.u.data.length = 0;
	current_ev = iwe_stream_add_point(info,	current_ev,
					end_buf, &iwe, list->ssid);

	
	if (last_ev == current_ev)
		return end_buf;

	last_ev = current_ev;

	
	current_val = current_ev + IW_EV_LCP_LEN;

	iwe.cmd = SIOCGIWRATE;
	
	iwe.u.bitrate.fixed = iwe.u.bitrate.disabled = 0;

	for (i = 0 ; i < list->supportedRates[1] ; i++) {
		
		iwe.u.bitrate.value = ((list->supportedRates[i+2] & 0x7f)
					* 500000);
		
		current_val = iwe_stream_add_value(info, current_ev,
				current_val, end_buf, &iwe, IW_EV_PARAM_LEN);

		
		if (last_ev == current_val)
			return end_buf;

		last_ev = current_val;
	}

	for (i = 0 ; i < list->extSupportedRates[1] ; i++) {
		
		iwe.u.bitrate.value = ((list->extSupportedRates[i+2] & 0x7f)
					* 500000);
		
		current_val = iwe_stream_add_value(info, current_ev,
				current_val, end_buf, &iwe, IW_EV_PARAM_LEN);

		
		if (last_ev == current_val)
			return end_buf;

		last_ev = current_ev;
	}

	
	if ((current_val - current_ev) > IW_EV_LCP_LEN)
		current_ev = current_val;
		#define IEEE80211_ELEMID_RSN 0x30
	memset(&iwe, 0, sizeof(iwe));
	iwe.cmd = IWEVCUSTOM;
	snprintf(buf, sizeof(buf), "bcn_int=%d", (list->beaconInterval[1] << 8)
						+ list->beaconInterval[0]);
	iwe.u.data.length = strlen(buf);
	current_ev = iwe_stream_add_point(info, current_ev,
						end_buf, &iwe, buf);

	
	if (last_ev == current_ev)
		return end_buf;

	last_ev = current_ev;

	if (list->wpaIe[1] != 0) {
		static const char rsn_leader[] = "rsn_ie=";
		static const char wpa_leader[] = "wpa_ie=";

		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = IWEVCUSTOM;
		if (list->wpaIe[0] == IEEE80211_ELEMID_RSN)
			iwe.u.data.length = encode_ie(buf, sizeof(buf),
					list->wpaIe, list->wpaIe[1]+2,
					rsn_leader, sizeof(rsn_leader)-1);
		else
			iwe.u.data.length = encode_ie(buf, sizeof(buf),
					list->wpaIe, list->wpaIe[1]+2,
					wpa_leader, sizeof(wpa_leader)-1);

		if (iwe.u.data.length != 0)
			current_ev = iwe_stream_add_point(info, current_ev,
							end_buf, &iwe, buf);

		
		if (last_ev == current_ev)
			return end_buf;

		last_ev = current_ev;
	}

	if (list->rsnIe[1] != 0) {
		static const char rsn_leader[] = "rsn_ie=";
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = IWEVCUSTOM;

		if (list->rsnIe[0] == IEEE80211_ELEMID_RSN) {
			iwe.u.data.length = encode_ie(buf, sizeof(buf),
			list->rsnIe, list->rsnIe[1]+2,
			rsn_leader, sizeof(rsn_leader)-1);
			if (iwe.u.data.length != 0)
				current_ev = iwe_stream_add_point(info,
					current_ev, end_buf,  &iwe, buf);

			
			if (last_ev == current_ev)
				return end_buf;

			last_ev = current_ev;
		}
	}
	
	return current_ev;
}

int usbdrvwext_giwname(struct net_device *dev,
		struct iw_request_info *info,
		union iwreq_data *wrq, char *extra)
{
	

	strcpy(wrq->name, "IEEE 802.11-MIMO");

	return 0;
}

int usbdrvwext_siwfreq(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_freq *freq, char *extra)
{
	u32_t FreqKHz;
	struct usbdrv_private *macp = dev->ml_priv;

	if (!netif_running(dev))
		return -EINVAL;

	if (freq->e > 1)
		return -EINVAL;

	if (freq->e == 1) {
		FreqKHz = (freq->m / 100000);

		if (FreqKHz > 4000000) {
			if (FreqKHz > 5825000)
				FreqKHz = 5825000;
			else if (FreqKHz < 4920000)
				FreqKHz = 4920000;
			else if (FreqKHz < 5000000)
				FreqKHz = (((FreqKHz - 4000000) / 5000) * 5000)
						+ 4000000;
			else
				FreqKHz = (((FreqKHz - 5000000) / 5000) * 5000)
						+ 5000000;
		} else {
			if (FreqKHz > 2484000)
				FreqKHz = 2484000;
			else if (FreqKHz < 2412000)
				FreqKHz = 2412000;
			else
				FreqKHz = (((FreqKHz - 2412000) / 5000) * 5000)
						+ 2412000;
		}
	} else {
		FreqKHz = usbdrv_chan2freq(freq->m);

		if (FreqKHz != -1)
			FreqKHz *= 1000;
		else
			FreqKHz = 2412000;
	}

	
	

	if (macp->DeviceOpened == 1) {
		zfiWlanSetFrequency(dev, FreqKHz, 0); 
		
		
		zfiWlanDisable(dev, 0);
		zfiWlanEnable(dev);
		
		
	}

	return 0;
}

int usbdrvwext_giwfreq(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_freq *freq, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;

	if (macp->DeviceOpened != 1)
		return 0;

	freq->m = zfiWlanQueryFrequency(dev);
	freq->e = 3;

	return 0;
}

int usbdrvwext_siwmode(struct net_device *dev,
		struct iw_request_info *info,
		union iwreq_data *wrq, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;
	u8_t WlanMode;

	if (!netif_running(dev))
		return -EINVAL;

	if (macp->DeviceOpened != 1)
		return 0;

	switch (wrq->mode) {
	case IW_MODE_MASTER:
		WlanMode = ZM_MODE_AP;
		break;
	case IW_MODE_INFRA:
		WlanMode = ZM_MODE_INFRASTRUCTURE;
		break;
	case IW_MODE_ADHOC:
		WlanMode = ZM_MODE_IBSS;
		break;
	default:
		WlanMode = ZM_MODE_IBSS;
		break;
	}

	zfiWlanSetWlanMode(dev, WlanMode);
	zfiWlanDisable(dev, 1);
	zfiWlanEnable(dev);

	return 0;
}

int usbdrvwext_giwmode(struct net_device *dev,
	struct iw_request_info *info,
	__u32 *mode, char *extra)
{
	unsigned long irqFlag;
	struct usbdrv_private *macp = dev->ml_priv;

	if (!netif_running(dev))
		return -EINVAL;

	if (macp->DeviceOpened != 1)
		return 0;

	spin_lock_irqsave(&macp->cs_lock, irqFlag);

	switch (zfiWlanQueryWlanMode(dev)) {
	case ZM_MODE_AP:
		*mode = IW_MODE_MASTER;
		break;
	case ZM_MODE_INFRASTRUCTURE:
		*mode = IW_MODE_INFRA;
		break;
	case ZM_MODE_IBSS:
		*mode = IW_MODE_ADHOC;
		break;
	default:
		*mode = IW_MODE_ADHOC;
		break;
	}

	spin_unlock_irqrestore(&macp->cs_lock, irqFlag);

	return 0;
}

int usbdrvwext_siwsens(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *sens, char *extra)
{
	return 0;
}

int usbdrvwext_giwsens(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *sens, char *extra)
{
	sens->value = 0;
	sens->fixed = 1;

	return 0;
}

int usbdrvwext_giwrange(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_point *data, char *extra)
{
	struct iw_range *range = (struct iw_range *) extra;
	int i, val;
	
	u16_t channels[60];
	u16_t channel_num;

	if (!netif_running(dev))
		return -EINVAL;

	range->txpower_capa = IW_TXPOW_DBM;
	

	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 13;

	range->retry_capa = IW_RETRY_LIMIT;
	range->retry_flags = IW_RETRY_LIMIT;
	range->min_retry = 0;
	range->max_retry = 255;

	channel_num = zfiWlanQueryAllowChannels(dev, channels);

	
	if (channel_num > IW_MAX_FREQUENCIES)
		channel_num = IW_MAX_FREQUENCIES;

	val = 0;

	for (i = 0; i < channel_num; i++) {
		range->freq[val].i = usbdrv_freq2chan(channels[i]);
		range->freq[val].m = channels[i];
		range->freq[val].e = 6;
		val++;
	}

	range->num_channels = channel_num;
	range->num_frequency = channel_num;

	#if 0
	range->num_channels = 14; 

	
	val = 0;
	
	for (i = 1; i <= 14; i++) {
		range->freq[val].i = i;
		if (i == 14)
			range->freq[val].m = 2484000;
		else
			range->freq[val].m = (2412+(i-1)*5)*1000;
		range->freq[val].e = 3;
		val++;
	}

	num_band_a = (IW_MAX_FREQUENCIES - val);
	
	for (i = 0; i < num_band_a; i++) {
		range->freq[val].i = channel_frequency_11A[2 * i];
		range->freq[val].m = channel_frequency_11A[2 * i + 1] * 1000;
		range->freq[val].e = 3;
		val++;
	}
	
	range->num_frequency = val;
	#endif

	
	range->max_qual.qual = 100;  
	range->max_qual.level = 154; 
	range->max_qual.noise = 154; 
	range->sensitivity = 3;      

	
	range->min_rts = 0;
	range->max_rts = 2347;
	range->min_frag = 256;
	range->max_frag = 2346;
	range->max_encoding_tokens = 4 ;
	range->num_encoding_sizes = 2; 

	range->encoding_size[0] = 5; 
	range->encoding_size[1] = 13; 

	
	range->num_bitrates = 0; 

	

	range->throughput = 300000000;

	return 0;
}

int usbdrvwext_siwap(struct net_device *dev, struct iw_request_info *info,
		struct sockaddr *MacAddr, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;

	if (!netif_running(dev))
		return -EINVAL;

	if (zfiWlanQueryWlanMode(dev) == ZM_MODE_AP) {
		
		zfiWlanSetMacAddress(dev, (u16_t *)&MacAddr->sa_data[0]);
	} else {
		
		zfiWlanSetBssid(dev, &MacAddr->sa_data[0]);
	}

	if (macp->DeviceOpened == 1) {
		
		
		zfiWlanDisable(dev, 0);
		zfiWlanEnable(dev);
		
		
	}

	return 0;
}

int usbdrvwext_giwap(struct net_device *dev,
		struct iw_request_info *info,
		struct sockaddr *MacAddr, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;

	if (macp->DeviceOpened != 1)
		return 0;

	if (zfiWlanQueryWlanMode(dev) == ZM_MODE_AP) {
		
		zfiWlanQueryMacAddress(dev, &MacAddr->sa_data[0]);
	} else {
		
		if (macp->adapterState == ZM_STATUS_MEDIA_CONNECT) {
			zfiWlanQueryBssid(dev, &MacAddr->sa_data[0]);
		} else {
			u8_t zero_addr[6] = { 0x00, 0x00, 0x00, 0x00,
								0x00, 0x00 };
			memcpy(&MacAddr->sa_data[0], zero_addr,
							sizeof(zero_addr));
		}
	}

	return 0;
}

int usbdrvwext_iwaplist(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *data, char *extra)
{
	
	return 0;

}

int usbdrvwext_siwscan(struct net_device *dev, struct iw_request_info *info,
	struct iw_point *data, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;

	if (macp->DeviceOpened != 1)
		return 0;

	printk(KERN_WARNING "CWY - usbdrvwext_siwscan\n");

	zfiWlanScan(dev);

	return 0;
}

int usbdrvwext_giwscan(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_point *data, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;
	
	char *current_ev = extra;
	char *end_buf;
	int i;
	
	struct zsBssListV1 *pBssList = kmalloc(sizeof(struct zsBssListV1),
								GFP_KERNEL);
	
	

	if (macp->DeviceOpened != 1)
		return 0;

	if (data->length == 0)
		end_buf = extra + IW_SCAN_MAX_DATA;
	else
		end_buf = extra + data->length;

	printk(KERN_WARNING "giwscan - Report Scan Results\n");
	
	zfiWlanQueryBssListV1(dev, pBssList);
	

	
	printk(KERN_WARNING "giwscan - pBssList->bssCount : %d\n",
						pBssList->bssCount);
	

	for (i = 0; i < pBssList->bssCount; i++) {
		
		current_ev = usbdrv_translate_scan(dev, info, current_ev,
					end_buf, &pBssList->bssInfo[i]);

		if (current_ev == end_buf) {
			kfree(pBssList);
			data->length = current_ev - extra;
			return -E2BIG;
		}
	}

	
	data->length = (current_ev - extra);
	data->flags = 0;   

	kfree(pBssList);

	return 0;
}

int usbdrvwext_siwessid(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_point *essid, char *extra)
{
	char EssidBuf[IW_ESSID_MAX_SIZE + 1];
	struct usbdrv_private *macp = dev->ml_priv;

	if (!netif_running(dev))
		return -EINVAL;

	if (essid->flags == 1) {
		if (essid->length > (IW_ESSID_MAX_SIZE + 1))
			return -E2BIG;

		if (copy_from_user(&EssidBuf, essid->pointer, essid->length))
			return -EFAULT;

		EssidBuf[essid->length] = '\0';
		
		
		
		if (macp->DeviceOpened == 1) {
			zfiWlanSetSSID(dev, EssidBuf, strlen(EssidBuf));
			zfiWlanSetFrequency(dev, zfiWlanQueryFrequency(dev),
						FALSE);
			zfiWlanSetEncryMode(dev, zfiWlanQueryEncryMode(dev));
			
			
			zfiWlanDisable(dev, 0);
			zfiWlanEnable(dev);
			
			
		}
	}

	return 0;
}

int usbdrvwext_giwessid(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_point *essid, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;
	u8_t EssidLen;
	char EssidBuf[IW_ESSID_MAX_SIZE + 1];
	int ssid_len;

	if (!netif_running(dev))
		return -EINVAL;

	if (macp->DeviceOpened != 1)
		return 0;

	zfiWlanQuerySSID(dev, &EssidBuf[0], &EssidLen);

	
	ssid_len = (int)EssidLen;

	
	if (ssid_len > IW_ESSID_MAX_SIZE)
		ssid_len = IW_ESSID_MAX_SIZE;

	EssidBuf[ssid_len] = '\0';

	essid->flags = 1;
	essid->length = strlen(EssidBuf);

	memcpy(extra, EssidBuf, essid->length);
	
	

	return 0;
}

int usbdrvwext_siwnickn(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *data, char *nickname)
{
	
	return 0;
}

int usbdrvwext_giwnickn(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *data, char *nickname)
{
	struct usbdrv_private *macp = dev->ml_priv;
	u8_t EssidLen;
	char EssidBuf[IW_ESSID_MAX_SIZE + 1];

	if (macp->DeviceOpened != 1)
		return 0;

	zfiWlanQuerySSID(dev, &EssidBuf[0], &EssidLen);
	EssidBuf[EssidLen] = 0;

	data->flags = 1;
	data->length = strlen(EssidBuf);

	memcpy(nickname, EssidBuf, data->length);

	return 0;
}

int usbdrvwext_siwrate(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_param *frq, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;
	
	u16_t zcIndextoRateBG[16] = {1000, 2000, 5500, 11000, 0, 0, 0, 0,
			48000, 24000, 12000, 6000, 54000, 36000, 18000, 9000};
	u16_t zcRateToMCS[] = {0xff, 0, 1, 2, 3, 0xb, 0xf, 0xa, 0xe, 0x9, 0xd,
				0x8, 0xc};
	u8_t i, RateIndex = 4;
	u16_t RateKbps;

	
	

	RateKbps = frq->value / 1000;
	
	for (i = 0; i < 16; i++) {
		if (RateKbps == zcIndextoRateBG[i])
			RateIndex = i;
	}

	if (zcIndextoRateBG[RateIndex] == 0)
		RateIndex = 0xff;
	
	for (i = 0; i < 13; i++)
		if (RateIndex == zcRateToMCS[i])
			break;
	
	if (RateKbps == 65000) {
		RateIndex = 20;
		printk(KERN_WARNING "RateIndex : %d\n", RateIndex);
	}

	if (macp->DeviceOpened == 1) {
		zfiWlanSetTxRate(dev, i);
		
		
	}

	return 0;
}

int usbdrvwext_giwrate(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_param *frq, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;

	if (!netif_running(dev))
		return -EINVAL;

	if (macp->DeviceOpened != 1)
		return 0;

	frq->fixed = 0;
	frq->disabled = 0;
	frq->value = zfiWlanQueryRxRate(dev) * 1000;

	return 0;
}

int usbdrvwext_siwrts(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_param *rts, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;
	int val = rts->value;

	if (macp->DeviceOpened != 1)
		return 0;

	if (rts->disabled)
		val = 2347;

	if ((val < 0) || (val > 2347))
		return -EINVAL;

	zfiWlanSetRtsThreshold(dev, val);

	return 0;
}

int usbdrvwext_giwrts(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_param *rts, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;

	if (!netif_running(dev))
		return -EINVAL;

	if (macp->DeviceOpened != 1)
		return 0;

	rts->value = zfiWlanQueryRtsThreshold(dev);
	rts->disabled = (rts->value >= 2347);
	rts->fixed = 1;

	return 0;
}

int usbdrvwext_siwfrag(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_param *frag, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;
	u16_t fragThreshold;

	if (macp->DeviceOpened != 1)
		return 0;

	if (frag->disabled)
		fragThreshold = 0;
	else
		fragThreshold = frag->value;

	zfiWlanSetFragThreshold(dev, fragThreshold);

	return 0;
}

int usbdrvwext_giwfrag(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_param *frag, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;
	u16 val;
	unsigned long irqFlag;

	if (!netif_running(dev))
		return -EINVAL;

	if (macp->DeviceOpened != 1)
		return 0;

	spin_lock_irqsave(&macp->cs_lock, irqFlag);

	val = zfiWlanQueryFragThreshold(dev);

	frag->value = val;

	frag->disabled = (val >= 2346);
	frag->fixed = 1;

	spin_unlock_irqrestore(&macp->cs_lock, irqFlag);

	return 0;
}

int usbdrvwext_siwtxpow(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *rrq, char *extra)
{
	
	return 0;
}

int usbdrvwext_giwtxpow(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *rrq, char *extra)
{
	
	return 0;
}

int usbdrvwext_siwretry(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *rrq, char *extra)
{
	
	return 0;
}

int usbdrvwext_giwretry(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_param *rrq, char *extra)
{
	
	return 0;
}

int usbdrvwext_siwencode(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_point *erq, char *key)
{
	struct zsKeyInfo keyInfo;
	int i;
	int WepState = ZM_ENCRYPTION_WEP_DISABLED;
	struct usbdrv_private *macp = dev->ml_priv;

	if (!netif_running(dev))
		return -EINVAL;

	if ((erq->flags & IW_ENCODE_DISABLED) == 0) {
		keyInfo.key = key;
		keyInfo.keyLength = erq->length;
		keyInfo.keyIndex = (erq->flags & IW_ENCODE_INDEX) - 1;
		if (keyInfo.keyIndex >= 4)
			keyInfo.keyIndex = 0;
		keyInfo.flag = ZM_KEY_FLAG_DEFAULT_KEY;

		zfiWlanSetKey(dev, keyInfo);
		WepState = ZM_ENCRYPTION_WEP_ENABLED;
	} else {
		for (i = 1; i < 4; i++)
			zfiWlanRemoveKey(dev, 0, i);
		WepState = ZM_ENCRYPTION_WEP_DISABLED;
		
	}

	if (macp->DeviceOpened == 1) {
		zfiWlanSetWepStatus(dev, WepState);
		zfiWlanSetFrequency(dev, zfiWlanQueryFrequency(dev), FALSE);
		
		
		
		zfiWlanDisable(dev, 0);
		zfiWlanEnable(dev);
		
		
	}

	return 0;
}

int usbdrvwext_giwencode(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_point *erq, char *key)
{
	struct usbdrv_private *macp = dev->ml_priv;
	u8_t EncryptionMode;
	u8_t keyLen = 0;

	if (macp->DeviceOpened != 1)
		return 0;

	EncryptionMode = zfiWlanQueryEncryMode(dev);

	if (EncryptionMode)
		erq->flags = IW_ENCODE_ENABLED;
	else
		erq->flags = IW_ENCODE_DISABLED;

	
	erq->flags |= IW_ENCODE_NOKEY;
	memset(key, 0, 16);

	
	switch (EncryptionMode) {
	case ZM_WEP64:
		keyLen = 5;
		break;
	case ZM_WEP128:
		keyLen = 13;
		break;
	case ZM_WEP256:
		keyLen = 29;
		break;
	case ZM_AES:
		keyLen = 16;
		break;
	case ZM_TKIP:
		keyLen = 32;
		break;
	#ifdef ZM_ENABLE_CENC
	case ZM_CENC:
		
		keyLen = 32;
		break;
	#endif
	case ZM_NO_WEP:
		keyLen = 0;
		break;
	default:
		keyLen = 0;
		printk(KERN_ERR "Unknown EncryMode\n");
		break;
	}
	erq->length = keyLen;

	return 0;
}

int usbdrvwext_siwpower(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_param *frq, char *extra)
{
	struct usbdrv_private *macp = dev->ml_priv;
	u8_t PSMode;

	if (macp->DeviceOpened != 1)
		return 0;

	if (frq->disabled)
		PSMode = ZM_STA_PS_NONE;
	else
		PSMode = ZM_STA_PS_MAX;

	zfiWlanSetPowerSaveMode(dev, PSMode);

	return 0;
}

int usbdrvwext_giwpower(struct net_device *dev,
		struct iw_request_info *info,
		struct iw_param *frq, char *extra)
{
	unsigned long irqFlag;
	struct usbdrv_private *macp = dev->ml_priv;

	if (macp->DeviceOpened != 1)
		return 0;

	spin_lock_irqsave(&macp->cs_lock, irqFlag);

	if (zfiWlanQueryPowerSaveMode(dev) == ZM_STA_PS_NONE)
		frq->disabled = 1;
	else
		frq->disabled = 0;

	spin_unlock_irqrestore(&macp->cs_lock, irqFlag);

	return 0;
}



int usbdrvwext_setmode(struct net_device *dev, struct iw_request_info *info,
			void *w, char *extra)
{
	return 0;
}

int usbdrvwext_getmode(struct net_device *dev, struct iw_request_info *info,
			void *w, char *extra)
{
	
	struct iw_point *wri = (struct iw_point *)extra;
	char mode[8];

	strcpy(mode, "11g");
	return copy_to_user(wri->pointer, mode, 6) ? -EFAULT : 0;
}

int zfLnxPrivateIoctl(struct net_device *dev, struct zdap_ioctl* zdreq)
{
	
	u16_t cmd;
	
	u32_t *p;
	u32_t i;

	cmd = zdreq->cmd;
	switch (cmd) {
	case ZM_IOCTL_REG_READ:
		zfiDbgReadReg(dev, zdreq->addr);
		break;
	case ZM_IOCTL_REG_WRITE:
		zfiDbgWriteReg(dev, zdreq->addr, zdreq->value);
		break;
	case ZM_IOCTL_MEM_READ:
		p = (u32_t *) bus_to_virt(zdreq->addr);
		printk(KERN_WARNING
				"usbdrv: read memory addr: 0x%08x value:"
				" 0x%08x\n", zdreq->addr, *p);
		break;
	case ZM_IOCTL_MEM_WRITE:
		p = (u32_t *) bus_to_virt(zdreq->addr);
		*p = zdreq->value;
		printk(KERN_WARNING
			"usbdrv : write value : 0x%08x to memory addr :"
			" 0x%08x\n", zdreq->value, zdreq->addr);
		break;
	case ZM_IOCTL_TALLY:
		zfiWlanShowTally(dev);
		if (zdreq->addr)
			zfiWlanResetTally(dev);
		break;
	case ZM_IOCTL_TEST:
		printk(KERN_WARNING
				"ZM_IOCTL_TEST:len=%d\n", zdreq->addr);
		
		
		printk(KERN_WARNING "IOCTL TEST\n");
		#if 1
		
		for (i = 0; i < zdreq->addr; i++) {
			if ((i&0x7) == 0)
				printk(KERN_WARNING "\n");
			printk(KERN_WARNING "%02X ",
					(unsigned char)zdreq->data[i]);
		}
		printk(KERN_WARNING "\n");
		#endif

		
		#if 0
			struct sk_buff *s;

			
			s = alloc_skb(2000, GFP_ATOMIC);

			
			for (i = 0; i < zdreq->addr; i++)
				s->data[i] = zdreq->data[i];
			s->len = zdreq->addr;

			
			zfiRecv80211(dev, s, NULL);
		#endif
		break;
	
	case ZM_IOCTL_FRAG:
		zfiWlanSetFragThreshold(dev, zdreq->addr);
		break;
	case ZM_IOCTL_RTS:
		zfiWlanSetRtsThreshold(dev, zdreq->addr);
		break;
	case ZM_IOCTL_SCAN:
		zfiWlanScan(dev);
		break;
	case ZM_IOCTL_KEY: {
		u8_t key[29];
		struct zsKeyInfo keyInfo;
		u32_t i;

		for (i = 0; i < 29; i++)
			key[i] = 0;

		for (i = 0; i < zdreq->addr; i++)
			key[i] = zdreq->data[i];

		printk(KERN_WARNING
			"key len=%d, key=%02x%02x%02x%02x%02x...\n",
			zdreq->addr, key[0], key[1], key[2], key[3], key[4]);

		keyInfo.keyLength = zdreq->addr;
		keyInfo.keyIndex = 0;
		keyInfo.flag = 0;
		keyInfo.key = key;
		zfiWlanSetKey(dev, keyInfo);
	}
		break;
	case ZM_IOCTL_RATE:
		zfiWlanSetTxRate(dev, zdreq->addr);
		break;
	case ZM_IOCTL_ENCRYPTION_MODE:
		zfiWlanSetEncryMode(dev, zdreq->addr);

		zfiWlanDisable(dev, 0);
		zfiWlanEnable(dev);
		break;
		
	case ZM_IOCTL_SIGNAL_STRENGTH: {
		u8_t buffer[2];
		zfiWlanQuerySignalInfo(dev, &buffer[0]);
		printk(KERN_WARNING
			"Current Signal Strength : %02d\n", buffer[0]);
	}
		break;
		
	case ZM_IOCTL_SIGNAL_QUALITY: {
		u8_t buffer[2];
		zfiWlanQuerySignalInfo(dev, &buffer[0]);
		printk(KERN_WARNING
			"Current Signal Quality : %02d\n", buffer[1]);
	}
		break;
	case ZM_IOCTL_SET_PIBSS_MODE:
		if (zdreq->addr == 1)
			zfiWlanSetWlanMode(dev, ZM_MODE_PSEUDO);
		else
			zfiWlanSetWlanMode(dev, ZM_MODE_INFRASTRUCTURE);

		zfiWlanDisable(dev, 0);
		zfiWlanEnable(dev);
		break;
	
	default:
		printk(KERN_ERR "usbdrv: error command = %x\n", cmd);
		break;
	}

	return 0;
}

int usbdrv_wpa_ioctl(struct net_device *dev, struct athr_wlan_param *zdparm)
{
	int ret = 0;
	u8_t bc_addr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	u8_t mac_addr[80];
	struct zsKeyInfo keyInfo;
	struct usbdrv_private *macp = dev->ml_priv;
	u16_t vapId = 0;
	int ii;

	

	switch (zdparm->cmd) {
	case ZD_CMD_SET_ENCRYPT_KEY:
		
		keyInfo.keyLength = zdparm->u.crypt.key_len;
		keyInfo.keyIndex = zdparm->u.crypt.idx;
		if (zfiWlanQueryWlanMode(dev) == ZM_MODE_AP) {
			
			keyInfo.flag = ZM_KEY_FLAG_AUTHENTICATOR;
		} else
			keyInfo.flag = 0;
		keyInfo.key = zdparm->u.crypt.key;
		keyInfo.initIv = zdparm->u.crypt.seq;
		keyInfo.macAddr = (u16_t *)zdparm->sta_addr;

		
		if (memcmp(zdparm->sta_addr, bc_addr, sizeof(bc_addr)) == 0)
			keyInfo.flag |= ZM_KEY_FLAG_GK;
		else
			keyInfo.flag |= ZM_KEY_FLAG_PK;

		if (!strcmp(zdparm->u.crypt.alg, "NONE")) {
			

			
			keyInfo.keyLength = 0;

			
			if (zdparm->sta_addr[0] & 1) {
				
			} else {
				
			}

			printk(KERN_ERR "Set Encryption Type NONE\n");
			return ret;
		} else if (!strcmp(zdparm->u.crypt.alg, "TKIP")) {
			zfiWlanSetEncryMode(dev, ZM_TKIP);
			
		} else if (!strcmp(zdparm->u.crypt.alg, "CCMP")) {
			zfiWlanSetEncryMode(dev, ZM_AES);
			
		} else if (!strcmp(zdparm->u.crypt.alg, "WEP")) {
			if (keyInfo.keyLength == 5) {
				
				zfiWlanSetEncryMode(dev, ZM_WEP64);
				
				
			} else if (keyInfo.keyLength == 13) {
				
				zfiWlanSetEncryMode(dev, ZM_WEP128);
				
				
			} else {
				zfiWlanSetEncryMode(dev, ZM_WEP256);
			}

	
		}

		
		
		if (keyInfo.keyLength > 0) {
			printk(KERN_WARNING
						"Otus: Key Context:\n");
			for (ii = 0; ii < keyInfo.keyLength; ) {
				printk(KERN_WARNING
						"0x%02x ", keyInfo.key[ii]);
				if ((++ii % 16) == 0)
					printk(KERN_WARNING "\n");
			}
			printk(KERN_WARNING "\n");
		}
		

		
		
		vapId = zfLnxGetVapId(dev);
		if (vapId == 0xffff)
			keyInfo.vapId = 0;
		else
			keyInfo.vapId = vapId + 1;
		keyInfo.vapAddr[0] = keyInfo.macAddr[0];
		keyInfo.vapAddr[1] = keyInfo.macAddr[1];
		keyInfo.vapAddr[2] = keyInfo.macAddr[2];

		zfiWlanSetKey(dev, keyInfo);

		
		
		break;
	case ZD_CMD_SET_MLME:
		printk(KERN_ERR "usbdrv_wpa_ioctl: ZD_CMD_SET_MLME\n");

		
		sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x",
			zdparm->sta_addr[0], zdparm->sta_addr[1],
			zdparm->sta_addr[2], zdparm->sta_addr[3],
			zdparm->sta_addr[4], zdparm->sta_addr[5]);

		switch (zdparm->u.mlme.cmd) {
		case MLME_STA_DEAUTH:
			printk(KERN_WARNING
				" -------Call zfiWlanDeauth, reason:%d\n",
				zdparm->u.mlme.reason_code);
			if (zfiWlanDeauth(dev, (u16_t *) zdparm->sta_addr,
				zdparm->u.mlme.reason_code) != 0)
				printk(KERN_ERR "Can't deauthencate STA: %s\n",
					mac_addr);
			else
				printk(KERN_ERR "Deauthenticate STA: %s"
					"with reason code: %d\n",
					mac_addr, zdparm->u.mlme.reason_code);
			break;
		case MLME_STA_DISASSOC:
			printk(KERN_WARNING
				" -------Call zfiWlanDeauth, reason:%d\n",
				zdparm->u.mlme.reason_code);
			if (zfiWlanDeauth(dev, (u16_t *) zdparm->sta_addr,
				zdparm->u.mlme.reason_code) != 0)
				printk(KERN_ERR "Can't disassociate STA: %s\n",
					mac_addr);
			else
				printk(KERN_ERR "Disassociate STA: %s"
					"with reason code: %d\n",
					mac_addr, zdparm->u.mlme.reason_code);
			break;
		default:
			printk(KERN_ERR "MLME command: 0x%04x not support\n",
				zdparm->u.mlme.cmd);
			break;
		}

		break;
	case ZD_CMD_SCAN_REQ:
		printk(KERN_ERR "usbdrv_wpa_ioctl: ZD_CMD_SCAN_REQ\n");
		break;
	case ZD_CMD_SET_GENERIC_ELEMENT:
		printk(KERN_ERR "usbdrv_wpa_ioctl:"
					" ZD_CMD_SET_GENERIC_ELEMENT\n");

		
		printk(KERN_ERR "wpaie Length : % d\n",
						zdparm->u.generic_elem.len);
		if (zfiWlanQueryWlanMode(dev) == ZM_MODE_AP) {
			
			zfiWlanSetWpaIe(dev, zdparm->u.generic_elem.data,
					zdparm->u.generic_elem.len);
		} else {
			macp->supLen = zdparm->u.generic_elem.len;
			memcpy(macp->supIe, zdparm->u.generic_elem.data,
				zdparm->u.generic_elem.len);
		}
		zfiWlanSetWpaSupport(dev, 1);
		
		u8_t len = zdparm->u.generic_elem.len;
		u8_t *wpaie = (u8_t *)zdparm->u.generic_elem.data;

		printk(KERN_ERR "wd->ap.wpaLen : % d\n", len);

		
		for(ii = 0; ii < len;) {
			printk(KERN_ERR "0x%02x ", wpaie[ii]);

			if((++ii % 16) == 0)
				printk(KERN_ERR "\n");
		}
		printk(KERN_ERR "\n");

		
		break;

	
	case ZD_CMD_GET_TSC:
		printk(KERN_ERR "usbdrv_wpa_ioctl : ZD_CMD_GET_TSC\n");
		break;
	

	default:
		printk(KERN_ERR "usbdrv_wpa_ioctl default : 0x%04x\n",
			zdparm->cmd);
		ret = -EINVAL;
		break;
	}

	return ret;
}

#ifdef ZM_ENABLE_CENC
int usbdrv_cenc_ioctl(struct net_device *dev, struct zydas_cenc_param *zdparm)
{
	
	struct zsKeyInfo keyInfo;
	u16_t apId;
	u8_t bc_addr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	int ret = 0;
	int ii;

	
	apId = zfLnxGetVapId(dev);

	if (apId == 0xffff) {
		apId = 0;
	} else {
		apId = apId + 1;
	}

	switch (zdparm->cmd) {
	case ZM_CMD_CENC_SETCENC:
		printk(KERN_ERR "ZM_CMD_CENC_SETCENC\n");
		printk(KERN_ERR "length : % d\n", zdparm->len);
		printk(KERN_ERR "policy : % d\n", zdparm->u.info.cenc_policy);
		break;
	case ZM_CMD_CENC_SETKEY:
		
		printk(KERN_ERR "ZM_CMD_CENC_SETKEY\n");

		printk(KERN_ERR "MAC address = ");
		for (ii = 0; ii < 6; ii++) {
			printk(KERN_ERR "0x%02x ",
				zdparm->u.crypt.sta_addr[ii]);
		}
		printk(KERN_ERR "\n");

		printk(KERN_ERR "Key Index : % d\n", zdparm->u.crypt.keyid);
		printk(KERN_ERR "Encryption key = ");
		for (ii = 0; ii < 16; ii++) {
			printk(KERN_ERR "0x%02x ", zdparm->u.crypt.key[ii]);
		}
		printk(KERN_ERR "\n");

		printk(KERN_ERR "MIC key = ");
		for(ii = 16; ii < ZM_CENC_KEY_SIZE; ii++) {
			printk(KERN_ERR "0x%02x ", zdparm->u.crypt.key[ii]);
		}
		printk(KERN_ERR "\n");

		
		keyInfo.keyLength = ZM_CENC_KEY_SIZE;
		keyInfo.keyIndex = zdparm->u.crypt.keyid;
		keyInfo.flag = ZM_KEY_FLAG_AUTHENTICATOR | ZM_KEY_FLAG_CENC;
		keyInfo.key = zdparm->u.crypt.key;
		keyInfo.macAddr = (u16_t *)zdparm->u.crypt.sta_addr;

		
		if (memcmp(zdparm->u.crypt.sta_addr, bc_addr,
				sizeof(bc_addr)) == 0) {
			keyInfo.flag |= ZM_KEY_FLAG_GK;
			keyInfo.vapId = apId;
			memcpy(keyInfo.vapAddr, dev->dev_addr, ETH_ALEN);
		} else {
			keyInfo.flag |= ZM_KEY_FLAG_PK;
		}

		zfiWlanSetKey(dev, keyInfo);

		break;
	case ZM_CMD_CENC_REKEY:
		
		printk(KERN_ERR "ZM_CMD_CENC_REKEY\n");
		break;
	default:
		ret = -EOPNOTSUPP;
		break;
	}

	
	

	return ret;
}
#endif 

int usbdrv_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	
	
	struct zdap_ioctl zdreq;
	struct iwreq *wrq = (struct iwreq *)ifr;
	struct athr_wlan_param zdparm;
	struct usbdrv_private *macp = dev->ml_priv;

	int err = 0, val = 0;
	int changed = 0;

	

	if (!netif_running(dev))
		return -EINVAL;

	switch (cmd) {
	case SIOCGIWNAME:
		strcpy(wrq->u.name, "IEEE 802.11-DS");
		break;
	case SIOCGIWAP:
		err = usbdrvwext_giwap(dev, NULL, &wrq->u.ap_addr, NULL);
		break;
	case SIOCSIWAP:
		err = usbdrvwext_siwap(dev, NULL, &wrq->u.ap_addr, NULL);
		break;
	case SIOCGIWMODE:
		err = usbdrvwext_giwmode(dev, NULL, &wrq->u.mode, NULL);
		break;
	case SIOCSIWESSID:
		printk(KERN_ERR "CWY - usbdrvwext_siwessid\n");
		
		err = usbdrvwext_siwessid(dev, NULL, &wrq->u.essid, NULL);

		if (!err)
			changed = 1;
		break;
	case SIOCGIWESSID:
		err = usbdrvwext_giwessid(dev, NULL, &wrq->u.essid, NULL);
		break;
	case SIOCSIWRTS:
		err = usbdrv_ioctl_setrts(dev, &wrq->u.rts);
		if (! err)
			changed = 1;
		break;
	
	case SIOCIWFIRSTPRIV + 0x2: {
		
		if (!capable(CAP_NET_ADMIN)) {
			err = -EPERM;
			break;
		}
		val = *((int *) wrq->u.name);
		if ((val < 0) || (val > 2)) {
			err = -EINVAL;
			break;
		} else {
			zfiWlanSetAuthenticationMode(dev, val);

			if (macp->DeviceOpened == 1) {
				zfiWlanDisable(dev, 0);
				zfiWlanEnable(dev);
			}

			err = 0;
			changed = 1;
		}
	}
		break;
	
	case SIOCIWFIRSTPRIV + 0x3: {
		int AuthMode = ZM_AUTH_MODE_OPEN;

		

		if (wrq->u.data.pointer) {
			wrq->u.data.flags = 1;

			AuthMode = zfiWlanQueryAuthenticationMode(dev, 0);
			if (AuthMode == ZM_AUTH_MODE_OPEN) {
				wrq->u.data.length = 12;

				if (copy_to_user(wrq->u.data.pointer,
					"open system", 12)) {
						return -EFAULT;
				}
			} else if (AuthMode == ZM_AUTH_MODE_SHARED_KEY)	{
				wrq->u.data.length = 11;

				if (copy_to_user(wrq->u.data.pointer,
					"shared key", 11)) {
							return -EFAULT;
				}
			} else if (AuthMode == ZM_AUTH_MODE_AUTO) {
				wrq->u.data.length = 10;

				if (copy_to_user(wrq->u.data.pointer,
					"auto mode", 10)) {
							return -EFAULT;
				}
			} else {
				return -EFAULT;
			}
		}
	}
		break;
	
	case ZDAPIOCTL:
		if (copy_from_user(&zdreq, ifr->ifr_data, sizeof(zdreq))) {
			printk(KERN_ERR "usbdrv : copy_from_user error\n");
			return -EFAULT;
		}

		
		zfLnxPrivateIoctl(dev, &zdreq);

		err = 0;
		break;
	case ZD_IOCTL_WPA:
		if (copy_from_user(&zdparm, ifr->ifr_data,
			sizeof(struct athr_wlan_param))) {
			printk(KERN_ERR "usbdrv : copy_from_user error\n");
			return -EFAULT;
		}

		usbdrv_wpa_ioctl(dev, &zdparm);
		err = 0;
		break;
	case ZD_IOCTL_PARAM: {
		int *p;
		int op;
		int arg;

		
		p = (int *)wrq->u.name;
		op = *p++;
		arg = *p;

		if (op == ZD_PARAM_ROAMING) {
			printk(KERN_ERR
			"*************ZD_PARAM_ROAMING : % d\n", arg);
			
		}
		if (op == ZD_PARAM_PRIVACY) {
			printk(KERN_ERR "ZD_IOCTL_PRIVACY : ");

			
			if (arg) {
				
				
				printk(KERN_ERR "enable\n");

			} else {
				
				
				printk(KERN_ERR "disable\n");
			}
			
		}
		if (op == ZD_PARAM_WPA) {

		printk(KERN_ERR "ZD_PARAM_WPA : ");

		if (arg) {
			printk(KERN_ERR "enable\n");

			if (zfiWlanQueryWlanMode(dev) != ZM_MODE_AP) {
				printk(KERN_ERR "Station Mode\n");
				
				
				
				if ((macp->supIe[21] == 0x50) &&
					(macp->supIe[22] == 0xf2) &&
					(macp->supIe[23] == 0x2)) {
					printk(KERN_ERR
				"wd->sta.authMode = ZM_AUTH_MODE_WPAPSK\n");
				
				
				zfiWlanSetAuthenticationMode(dev,
							ZM_AUTH_MODE_WPAPSK);
				} else if ((macp->supIe[21] == 0x50) &&
					(macp->supIe[22] == 0xf2) &&
					(macp->supIe[23] == 0x1)) {
					printk(KERN_ERR
				"wd->sta.authMode = ZM_AUTH_MODE_WPA\n");
				
				
				zfiWlanSetAuthenticationMode(dev,
							ZM_AUTH_MODE_WPA);
					} else if ((macp->supIe[17] == 0xf) &&
						(macp->supIe[18] == 0xac) &&
						(macp->supIe[19] == 0x2))
					{
						printk(KERN_ERR
				"wd->sta.authMode = ZM_AUTH_MODE_WPA2PSK\n");
				
				
				zfiWlanSetAuthenticationMode(dev,
				ZM_AUTH_MODE_WPA2PSK);
			} else if ((macp->supIe[17] == 0xf) &&
				(macp->supIe[18] == 0xac) &&
				(macp->supIe[19] == 0x1))
				{
					printk(KERN_ERR
				"wd->sta.authMode = ZM_AUTH_MODE_WPA2\n");
				
				
				zfiWlanSetAuthenticationMode(dev,
							ZM_AUTH_MODE_WPA2);
			}
			
			if ((macp->supIe[21] == 0x50) ||
				(macp->supIe[22] == 0xf2)) {
				if (macp->supIe[11] == 0x2) {
					printk(KERN_ERR
				"wd->sta.wepStatus = ZM_ENCRYPTION_TKIP\n");
				
				
				zfiWlanSetWepStatus(dev, ZM_ENCRYPTION_TKIP);
			} else {
				printk(KERN_ERR
				"wd->sta.wepStatus = ZM_ENCRYPTION_AES\n");
				
				
				zfiWlanSetWepStatus(dev, ZM_ENCRYPTION_AES);
				}
			}
			
			if ((macp->supIe[17] == 0xf) ||
				(macp->supIe[18] == 0xac)) {
				if (macp->supIe[13] == 0x2) {
					printk(KERN_ERR
				"wd->sta.wepStatus = ZM_ENCRYPTION_TKIP\n");
				
				
				zfiWlanSetWepStatus(dev, ZM_ENCRYPTION_TKIP);
				} else {
					printk(KERN_ERR
				"wd->sta.wepStatus = ZM_ENCRYPTION_AES\n");
				
				
				zfiWlanSetWepStatus(dev, ZM_ENCRYPTION_AES);
					}
				}
			}
			zfiWlanSetWpaSupport(dev, 1);
		} else {
			
			printk(KERN_ERR "disable\n");

			zfiWlanSetWpaSupport(dev, 0);
			zfiWlanSetAuthenticationMode(dev, ZM_AUTH_MODE_OPEN);
			zfiWlanSetWepStatus(dev, ZM_ENCRYPTION_WEP_DISABLED);

			
			}
		}

		if (op == ZD_PARAM_COUNTERMEASURES) {
			printk(KERN_ERR
				"****************ZD_PARAM_COUNTERMEASURES : ");

			if(arg) {
				
				printk(KERN_ERR "enable\n");
			} else {
				
				printk(KERN_ERR "disable\n");
			}
		}
		if (op == ZD_PARAM_DROPUNENCRYPTED) {
			printk(KERN_ERR "ZD_PARAM_DROPUNENCRYPTED : ");

			if(arg) {
				printk(KERN_ERR "enable\n");
			} else {
				printk(KERN_ERR "disable\n");
			}
		}
		if (op == ZD_PARAM_AUTH_ALGS) {
			printk(KERN_ERR "ZD_PARAM_AUTH_ALGS : ");

			if (arg == 0) {
				printk(KERN_ERR "OPEN_SYSTEM\n");
			} else {
				printk(KERN_ERR "SHARED_KEY\n");
			}
		}
		if (op == ZD_PARAM_WPS_FILTER) {
			printk(KERN_ERR "ZD_PARAM_WPS_FILTER : ");

			if (arg) {
				
				macp->forwardMgmt = 1;
				printk(KERN_ERR "enable\n");
			} else {
				
				macp->forwardMgmt = 0;
				printk(KERN_ERR "disable\n");
			}
		}
	}
		err = 0;
		break;
	case ZD_IOCTL_GETWPAIE: {
		struct ieee80211req_wpaie req_wpaie;
		u16_t apId, i, j;

		
		apId = zfLnxGetVapId(dev);

		if (apId == 0xffff) {
			apId = 0;
		} else {
			apId = apId + 1;
		}

		if (copy_from_user(&req_wpaie, ifr->ifr_data,
					sizeof(struct ieee80211req_wpaie))) {
			printk(KERN_ERR "usbdrv : copy_from_user error\n");
			return -EFAULT;
		}

		for (i = 0; i < ZM_OAL_MAX_STA_SUPPORT; i++) {
			for (j = 0; j < IEEE80211_ADDR_LEN; j++) {
				if (macp->stawpaie[i].wpa_macaddr[j] !=
						req_wpaie.wpa_macaddr[j])
				break;
			}
			if (j == 6)
			break;
		}

		if (i < ZM_OAL_MAX_STA_SUPPORT) {
		
		memcpy(req_wpaie.wpa_ie, macp->stawpaie[i].wpa_ie,
							IEEE80211_MAX_IE_SIZE);
		}

		if (copy_to_user(wrq->u.data.pointer, &req_wpaie,
				sizeof(struct ieee80211req_wpaie))) {
			return -EFAULT;
		}
	}

		err = 0;
		break;
	#ifdef ZM_ENABLE_CENC
	case ZM_IOCTL_CENC:
		if (copy_from_user(&macp->zd_wpa_req, ifr->ifr_data,
			sizeof(struct athr_wlan_param))) {
			printk(KERN_ERR "usbdrv : copy_from_user error\n");
			return -EFAULT;
		}

		usbdrv_cenc_ioctl(dev,
				(struct zydas_cenc_param *)&macp->zd_wpa_req);
		err = 0;
		break;
	#endif 
	default:
		err = -EOPNOTSUPP;
		break;
	}

	return err;
}
