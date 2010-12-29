
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include <linux/if_arp.h>
#include <asm/unaligned.h>
#include <net/lib80211.h>

#include "host.h"
#include "decl.h"
#include "dev.h"
#include "scan.h"
#include "cmd.h"


#define MAX_SCAN_CELL_SIZE  (IW_EV_ADDR_LEN             \
                             + IW_ESSID_MAX_SIZE        \
                             + IW_EV_UINT_LEN           \
                             + IW_EV_FREQ_LEN           \
                             + IW_EV_QUAL_LEN           \
                             + IW_ESSID_MAX_SIZE        \
                             + IW_EV_PARAM_LEN          \
                             + 40)	


#define CHAN_TLV_MAX_SIZE  (sizeof(struct mrvl_ie_header)    \
                            + (MRVDRV_MAX_CHANNELS_PER_SCAN     \
                               * sizeof(struct chanscanparamset)))


#define SSID_TLV_MAX_SIZE  (1 * sizeof(struct mrvl_ie_ssid_param_set))


#define MAX_SCAN_CFG_ALLOC (sizeof(struct cmd_ds_802_11_scan)	\
                            + CHAN_TLV_MAX_SIZE + SSID_TLV_MAX_SIZE)


#define MRVDRV_MAX_CHANNELS_PER_SCAN   14


#define MRVDRV_CHANNELS_PER_SCAN_CMD   4


#define MRVDRV_PASSIVE_SCAN_CHAN_TIME  100


#define MRVDRV_ACTIVE_SCAN_CHAN_TIME   100

#define DEFAULT_MAX_SCAN_AGE (15 * HZ)

static int lbs_ret_80211_scan(struct lbs_private *priv, unsigned long dummy,
			      struct cmd_header *resp);








static void lbs_unset_basic_rate_flags(u8 *rates, size_t len)
{
	int i;

	for (i = 0; i < len; i++)
		rates[i] &= 0x7f;
}


static inline void clear_bss_descriptor(struct bss_descriptor *bss)
{
	
	memset(bss, 0, offsetof(struct bss_descriptor, list));
}


int lbs_ssid_cmp(uint8_t *ssid1, uint8_t ssid1_len, uint8_t *ssid2,
		 uint8_t ssid2_len)
{
	if (ssid1_len != ssid2_len)
		return -1;

	return memcmp(ssid1, ssid2, ssid1_len);
}

static inline int is_same_network(struct bss_descriptor *src,
				  struct bss_descriptor *dst)
{
	
	return ((src->ssid_len == dst->ssid_len) &&
		(src->channel == dst->channel) &&
		!compare_ether_addr(src->bssid, dst->bssid) &&
		!memcmp(src->ssid, dst->ssid, src->ssid_len));
}











static int lbs_scan_create_channel_list(struct lbs_private *priv,
					struct chanscanparamset *scanchanlist)
{
	struct region_channel *scanregion;
	struct chan_freq_power *cfp;
	int rgnidx;
	int chanidx;
	int nextchan;
	uint8_t scantype;

	chanidx = 0;

	
	scantype = CMD_SCAN_TYPE_ACTIVE;

	for (rgnidx = 0; rgnidx < ARRAY_SIZE(priv->region_channel); rgnidx++) {
		if (priv->enable11d && (priv->connect_status != LBS_CONNECTED)
		    && (priv->mesh_connect_status != LBS_CONNECTED)) {
			
			if (!priv->universal_channel[rgnidx].valid)
				continue;
			scanregion = &priv->universal_channel[rgnidx];

			
			memset(&priv->parsed_region_chan, 0x00,
			       sizeof(priv->parsed_region_chan));
		} else {
			if (!priv->region_channel[rgnidx].valid)
				continue;
			scanregion = &priv->region_channel[rgnidx];
		}

		for (nextchan = 0; nextchan < scanregion->nrcfp; nextchan++, chanidx++) {
			struct chanscanparamset *chan = &scanchanlist[chanidx];

			cfp = scanregion->CFP + nextchan;

			if (priv->enable11d)
				scantype = lbs_get_scan_type_11d(cfp->channel,
								 &priv->parsed_region_chan);

			if (scanregion->band == BAND_B || scanregion->band == BAND_G)
				chan->radiotype = CMD_SCAN_RADIO_TYPE_BG;

			if (scantype == CMD_SCAN_TYPE_PASSIVE) {
				chan->maxscantime = cpu_to_le16(MRVDRV_PASSIVE_SCAN_CHAN_TIME);
				chan->chanscanmode.passivescan = 1;
			} else {
				chan->maxscantime = cpu_to_le16(MRVDRV_ACTIVE_SCAN_CHAN_TIME);
				chan->chanscanmode.passivescan = 0;
			}

			chan->channumber = cfp->channel;
		}
	}
	return chanidx;
}


static int lbs_scan_add_ssid_tlv(struct lbs_private *priv, u8 *tlv)
{
	struct mrvl_ie_ssid_param_set *ssid_tlv = (void *)tlv;

	ssid_tlv->header.type = cpu_to_le16(TLV_TYPE_SSID);
	ssid_tlv->header.len = cpu_to_le16(priv->scan_ssid_len);
	memcpy(ssid_tlv->ssid, priv->scan_ssid, priv->scan_ssid_len);
	return sizeof(ssid_tlv->header) + priv->scan_ssid_len;
}


static int lbs_scan_add_chanlist_tlv(uint8_t *tlv,
				     struct chanscanparamset *chan_list,
				     int chan_count)
{
	size_t size = sizeof(struct chanscanparamset) *chan_count;
	struct mrvl_ie_chanlist_param_set *chan_tlv = (void *)tlv;

	chan_tlv->header.type = cpu_to_le16(TLV_TYPE_CHANLIST);
	memcpy(chan_tlv->chanscanparam, chan_list, size);
	chan_tlv->header.len = cpu_to_le16(size);
	return sizeof(chan_tlv->header) + size;
}


static int lbs_scan_add_rates_tlv(uint8_t *tlv)
{
	int i;
	struct mrvl_ie_rates_param_set *rate_tlv = (void *)tlv;

	rate_tlv->header.type = cpu_to_le16(TLV_TYPE_RATES);
	tlv += sizeof(rate_tlv->header);
	for (i = 0; i < MAX_RATES; i++) {
		*tlv = lbs_bg_rates[i];
		if (*tlv == 0)
			break;
		
		if (*tlv == 0x02 || *tlv == 0x04 ||
		    *tlv == 0x0b || *tlv == 0x16)
			*tlv |= 0x80;
		tlv++;
	}
	rate_tlv->header.len = cpu_to_le16(i);
	return sizeof(rate_tlv->header) + i;
}


static int lbs_do_scan(struct lbs_private *priv, uint8_t bsstype,
		       struct chanscanparamset *chan_list, int chan_count)
{
	int ret = -ENOMEM;
	struct cmd_ds_802_11_scan *scan_cmd;
	uint8_t *tlv;	

	lbs_deb_enter_args(LBS_DEB_SCAN, "bsstype %d, chanlist[].chan %d, chan_count %d",
		bsstype, chan_list ? chan_list[0].channumber : -1,
		chan_count);

	
	scan_cmd = kzalloc(MAX_SCAN_CFG_ALLOC, GFP_KERNEL);
	if (scan_cmd == NULL)
		goto out;

	tlv = scan_cmd->tlvbuffer;
	
	scan_cmd->bsstype = bsstype;

	
	if (priv->scan_ssid_len)
		tlv += lbs_scan_add_ssid_tlv(priv, tlv);
	if (chan_list && chan_count)
		tlv += lbs_scan_add_chanlist_tlv(tlv, chan_list, chan_count);
	tlv += lbs_scan_add_rates_tlv(tlv);

	
	scan_cmd->hdr.size = cpu_to_le16(tlv - (uint8_t *)scan_cmd);
	lbs_deb_hex(LBS_DEB_SCAN, "SCAN_CMD", (void *)scan_cmd,
		    sizeof(*scan_cmd));
	lbs_deb_hex(LBS_DEB_SCAN, "SCAN_TLV", scan_cmd->tlvbuffer,
		    tlv - scan_cmd->tlvbuffer);

	ret = __lbs_cmd(priv, CMD_802_11_SCAN, &scan_cmd->hdr,
			le16_to_cpu(scan_cmd->hdr.size),
			lbs_ret_80211_scan, 0);

out:
	kfree(scan_cmd);
	lbs_deb_leave_args(LBS_DEB_SCAN, "ret %d", ret);
	return ret;
}


int lbs_scan_networks(struct lbs_private *priv, int full_scan)
{
	int ret = -ENOMEM;
	struct chanscanparamset *chan_list;
	struct chanscanparamset *curr_chans;
	int chan_count;
	uint8_t bsstype = CMD_BSS_TYPE_ANY;
	int numchannels = MRVDRV_CHANNELS_PER_SCAN_CMD;
	union iwreq_data wrqu;
#ifdef CONFIG_LIBERTAS_DEBUG
	struct bss_descriptor *iter;
	int i = 0;
	DECLARE_SSID_BUF(ssid);
#endif

	lbs_deb_enter_args(LBS_DEB_SCAN, "full_scan %d", full_scan);

	
	if (full_scan && delayed_work_pending(&priv->scan_work))
		cancel_delayed_work(&priv->scan_work);

	

	lbs_deb_scan("numchannels %d, bsstype %d\n", numchannels, bsstype);

	
	chan_list = kzalloc(sizeof(struct chanscanparamset) *
			    LBS_IOCTL_USER_SCAN_CHAN_MAX, GFP_KERNEL);
	if (!chan_list) {
		lbs_pr_alert("SCAN: chan_list empty\n");
		goto out;
	}

	
	chan_count = lbs_scan_create_channel_list(priv, chan_list);

	netif_stop_queue(priv->dev);
	if (priv->mesh_dev)
		netif_stop_queue(priv->mesh_dev);

	
	lbs_deb_scan("chan_count %d, scan_channel %d\n",
		     chan_count, priv->scan_channel);
	curr_chans = chan_list;
	
	if (priv->scan_channel > 0) {
		curr_chans += priv->scan_channel;
		chan_count -= priv->scan_channel;
	}

	

	while (chan_count) {
		int to_scan = min(numchannels, chan_count);
		lbs_deb_scan("scanning %d of %d channels\n",
			     to_scan, chan_count);
		ret = lbs_do_scan(priv, bsstype, curr_chans,
				  to_scan);
		if (ret) {
			lbs_pr_err("SCAN_CMD failed\n");
			goto out2;
		}
		curr_chans += to_scan;
		chan_count -= to_scan;

		
		if (chan_count && !full_scan &&
		    !priv->surpriseremoved) {
			
			if (priv->scan_channel < 0)
				priv->scan_channel = to_scan;
			else
				priv->scan_channel += to_scan;
			cancel_delayed_work(&priv->scan_work);
			queue_delayed_work(priv->work_thread, &priv->scan_work,
					   msecs_to_jiffies(300));
			
			goto out;
		}

	}
	memset(&wrqu, 0, sizeof(union iwreq_data));
	wireless_send_event(priv->dev, SIOCGIWSCAN, &wrqu, NULL);

#ifdef CONFIG_LIBERTAS_DEBUG
	
	mutex_lock(&priv->lock);
	lbs_deb_scan("scan table:\n");
	list_for_each_entry(iter, &priv->network_list, list)
		lbs_deb_scan("%02d: BSSID %pM, RSSI %d, SSID '%s'\n",
			     i++, iter->bssid, iter->rssi,
			     print_ssid(ssid, iter->ssid, iter->ssid_len));
	mutex_unlock(&priv->lock);
#endif

out2:
	priv->scan_channel = 0;

out:
	if (priv->connect_status == LBS_CONNECTED && !priv->tx_pending_len)
		netif_wake_queue(priv->dev);

	if (priv->mesh_dev && (priv->mesh_connect_status == LBS_CONNECTED) &&
	    !priv->tx_pending_len)
		netif_wake_queue(priv->mesh_dev);

	kfree(chan_list);

	lbs_deb_leave_args(LBS_DEB_SCAN, "ret %d", ret);
	return ret;
}

void lbs_scan_worker(struct work_struct *work)
{
	struct lbs_private *priv =
		container_of(work, struct lbs_private, scan_work.work);

	lbs_deb_enter(LBS_DEB_SCAN);
	lbs_scan_networks(priv, 0);
	lbs_deb_leave(LBS_DEB_SCAN);
}









static int lbs_process_bss(struct bss_descriptor *bss,
			   uint8_t **pbeaconinfo, int *bytesleft)
{
	struct ieee_ie_fh_param_set *fh;
	struct ieee_ie_ds_param_set *ds;
	struct ieee_ie_cf_param_set *cf;
	struct ieee_ie_ibss_param_set *ibss;
	DECLARE_SSID_BUF(ssid);
	struct ieee_ie_country_info_set *pcountryinfo;
	uint8_t *pos, *end, *p;
	uint8_t n_ex_rates = 0, got_basic_rates = 0, n_basic_rates = 0;
	uint16_t beaconsize = 0;
	int ret;

	lbs_deb_enter(LBS_DEB_SCAN);

	if (*bytesleft >= sizeof(beaconsize)) {
		
		beaconsize = get_unaligned_le16(*pbeaconinfo);
		*bytesleft -= sizeof(beaconsize);
		*pbeaconinfo += sizeof(beaconsize);
	}

	if (beaconsize == 0 || beaconsize > *bytesleft) {
		*pbeaconinfo += *bytesleft;
		*bytesleft = 0;
		ret = -1;
		goto done;
	}

	
	pos = *pbeaconinfo;
	end = pos + beaconsize;

	
	*pbeaconinfo += beaconsize;
	*bytesleft -= beaconsize;

	memcpy(bss->bssid, pos, ETH_ALEN);
	lbs_deb_scan("process_bss: BSSID %pM\n", bss->bssid);
	pos += ETH_ALEN;

	if ((end - pos) < 12) {
		lbs_deb_scan("process_bss: Not enough bytes left\n");
		ret = -1;
		goto done;
	}

	

	
	bss->rssi = *pos;
	lbs_deb_scan("process_bss: RSSI %d\n", *pos);
	pos++;

	
	pos += 8;

	
	bss->beaconperiod = get_unaligned_le16(pos);
	pos += 2;

	
	bss->capability = get_unaligned_le16(pos);
	lbs_deb_scan("process_bss: capabilities 0x%04x\n", bss->capability);
	pos += 2;

	if (bss->capability & WLAN_CAPABILITY_PRIVACY)
		lbs_deb_scan("process_bss: WEP enabled\n");
	if (bss->capability & WLAN_CAPABILITY_IBSS)
		bss->mode = IW_MODE_ADHOC;
	else
		bss->mode = IW_MODE_INFRA;

	
	lbs_deb_scan("process_bss: IE len %zd\n", end - pos);
	lbs_deb_hex(LBS_DEB_SCAN, "process_bss: IE info", pos, end - pos);

	
	while (pos <= end - 2) {
		if (pos + pos[1] > end) {
			lbs_deb_scan("process_bss: error in processing IE, "
				     "bytes left < IE length\n");
			break;
		}

		switch (pos[0]) {
		case WLAN_EID_SSID:
			bss->ssid_len = min_t(int, IEEE80211_MAX_SSID_LEN, pos[1]);
			memcpy(bss->ssid, pos + 2, bss->ssid_len);
			lbs_deb_scan("got SSID IE: '%s', len %u\n",
			             print_ssid(ssid, bss->ssid, bss->ssid_len),
			             bss->ssid_len);
			break;

		case WLAN_EID_SUPP_RATES:
			n_basic_rates = min_t(uint8_t, MAX_RATES, pos[1]);
			memcpy(bss->rates, pos + 2, n_basic_rates);
			got_basic_rates = 1;
			lbs_deb_scan("got RATES IE\n");
			break;

		case WLAN_EID_FH_PARAMS:
			fh = (struct ieee_ie_fh_param_set *) pos;
			memcpy(&bss->phy.fh, fh, sizeof(*fh));
			lbs_deb_scan("got FH IE\n");
			break;

		case WLAN_EID_DS_PARAMS:
			ds = (struct ieee_ie_ds_param_set *) pos;
			bss->channel = ds->channel;
			memcpy(&bss->phy.ds, ds, sizeof(*ds));
			lbs_deb_scan("got DS IE, channel %d\n", bss->channel);
			break;

		case WLAN_EID_CF_PARAMS:
			cf = (struct ieee_ie_cf_param_set *) pos;
			memcpy(&bss->ss.cf, cf, sizeof(*cf));
			lbs_deb_scan("got CF IE\n");
			break;

		case WLAN_EID_IBSS_PARAMS:
			ibss = (struct ieee_ie_ibss_param_set *) pos;
			bss->atimwindow = ibss->atimwindow;
			memcpy(&bss->ss.ibss, ibss, sizeof(*ibss));
			lbs_deb_scan("got IBSS IE\n");
			break;

		case WLAN_EID_COUNTRY:
			pcountryinfo = (struct ieee_ie_country_info_set *) pos;
			lbs_deb_scan("got COUNTRY IE\n");
			if (pcountryinfo->header.len < sizeof(pcountryinfo->countrycode)
			    || pcountryinfo->header.len > 254) {
				lbs_deb_scan("%s: 11D- Err CountryInfo len %d, min %zd, max 254\n",
					     __func__,
					     pcountryinfo->header.len,
					     sizeof(pcountryinfo->countrycode));
				ret = -1;
				goto done;
			}

			memcpy(&bss->countryinfo, pcountryinfo,
				pcountryinfo->header.len + 2);
			lbs_deb_hex(LBS_DEB_SCAN, "process_bss: 11d countryinfo",
				    (uint8_t *) pcountryinfo,
				    (int) (pcountryinfo->header.len + 2));
			break;

		case WLAN_EID_EXT_SUPP_RATES:
			
			lbs_deb_scan("got RATESEX IE\n");
			if (!got_basic_rates) {
				lbs_deb_scan("... but ignoring it\n");
				break;
			}

			n_ex_rates = pos[1];
			if (n_basic_rates + n_ex_rates > MAX_RATES)
				n_ex_rates = MAX_RATES - n_basic_rates;

			p = bss->rates + n_basic_rates;
			memcpy(p, pos + 2, n_ex_rates);
			break;

		case WLAN_EID_GENERIC:
			if (pos[1] >= 4 &&
			    pos[2] == 0x00 && pos[3] == 0x50 &&
			    pos[4] == 0xf2 && pos[5] == 0x01) {
				bss->wpa_ie_len = min(pos[1] + 2, MAX_WPA_IE_LEN);
				memcpy(bss->wpa_ie, pos, bss->wpa_ie_len);
				lbs_deb_scan("got WPA IE\n");
				lbs_deb_hex(LBS_DEB_SCAN, "WPA IE", bss->wpa_ie,
					    bss->wpa_ie_len);
			} else if (pos[1] >= MARVELL_MESH_IE_LENGTH &&
				   pos[2] == 0x00 && pos[3] == 0x50 &&
				   pos[4] == 0x43 && pos[5] == 0x04) {
				lbs_deb_scan("got mesh IE\n");
				bss->mesh = 1;
			} else {
				lbs_deb_scan("got generic IE: %02x:%02x:%02x:%02x, len %d\n",
					pos[2], pos[3],
					pos[4], pos[5],
					pos[1]);
			}
			break;

		case WLAN_EID_RSN:
			lbs_deb_scan("got RSN IE\n");
			bss->rsn_ie_len = min(pos[1] + 2, MAX_WPA_IE_LEN);
			memcpy(bss->rsn_ie, pos, bss->rsn_ie_len);
			lbs_deb_hex(LBS_DEB_SCAN, "process_bss: RSN_IE",
				    bss->rsn_ie, bss->rsn_ie_len);
			break;

		default:
			lbs_deb_scan("got IE 0x%04x, len %d\n",
				     pos[0], pos[1]);
			break;
		}

		pos += pos[1] + 2;
	}

	
	bss->last_scanned = jiffies;
	lbs_unset_basic_rate_flags(bss->rates, sizeof(bss->rates));

	ret = 0;

done:
	lbs_deb_leave_args(LBS_DEB_SCAN, "ret %d", ret);
	return ret;
}


int lbs_send_specific_ssid_scan(struct lbs_private *priv, uint8_t *ssid,
				uint8_t ssid_len)
{
	DECLARE_SSID_BUF(ssid_buf);
	int ret = 0;

	lbs_deb_enter_args(LBS_DEB_SCAN, "SSID '%s'\n",
			   print_ssid(ssid_buf, ssid, ssid_len));

	if (!ssid_len)
		goto out;

	memcpy(priv->scan_ssid, ssid, ssid_len);
	priv->scan_ssid_len = ssid_len;

	lbs_scan_networks(priv, 1);
	if (priv->surpriseremoved) {
		ret = -1;
		goto out;
	}

out:
	lbs_deb_leave_args(LBS_DEB_SCAN, "ret %d", ret);
	return ret;
}











#define MAX_CUSTOM_LEN 64

static inline char *lbs_translate_scan(struct lbs_private *priv,
					    struct iw_request_info *info,
					    char *start, char *stop,
					    struct bss_descriptor *bss)
{
	struct chan_freq_power *cfp;
	char *current_val;	
	struct iw_event iwe;	
	int j;
#define PERFECT_RSSI ((uint8_t)50)
#define WORST_RSSI   ((uint8_t)0)
#define RSSI_DIFF    ((uint8_t)(PERFECT_RSSI - WORST_RSSI))
	uint8_t rssi;

	lbs_deb_enter(LBS_DEB_SCAN);

	cfp = lbs_find_cfp_by_band_and_channel(priv, 0, bss->channel);
	if (!cfp) {
		lbs_deb_scan("Invalid channel number %d\n", bss->channel);
		start = NULL;
		goto out;
	}

	
	iwe.cmd = SIOCGIWAP;
	iwe.u.ap_addr.sa_family = ARPHRD_ETHER;
	memcpy(iwe.u.ap_addr.sa_data, &bss->bssid, ETH_ALEN);
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_ADDR_LEN);

	
	iwe.cmd = SIOCGIWESSID;
	iwe.u.data.flags = 1;
	iwe.u.data.length = min((uint32_t) bss->ssid_len, (uint32_t) IW_ESSID_MAX_SIZE);
	start = iwe_stream_add_point(info, start, stop, &iwe, bss->ssid);

	
	iwe.cmd = SIOCGIWMODE;
	iwe.u.mode = bss->mode;
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_UINT_LEN);

	
	iwe.cmd = SIOCGIWFREQ;
	iwe.u.freq.m = (long)cfp->freq * 100000;
	iwe.u.freq.e = 1;
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_FREQ_LEN);

	
	iwe.cmd = IWEVQUAL;
	iwe.u.qual.updated = IW_QUAL_ALL_UPDATED;
	iwe.u.qual.level = SCAN_RSSI(bss->rssi);

	rssi = iwe.u.qual.level - MRVDRV_NF_DEFAULT_SCAN_VALUE;
	iwe.u.qual.qual =
		(100 * RSSI_DIFF * RSSI_DIFF - (PERFECT_RSSI - rssi) *
		 (15 * (RSSI_DIFF) + 62 * (PERFECT_RSSI - rssi))) /
		(RSSI_DIFF * RSSI_DIFF);
	if (iwe.u.qual.qual > 100)
		iwe.u.qual.qual = 100;

	if (priv->NF[TYPE_BEACON][TYPE_NOAVG] == 0) {
		iwe.u.qual.noise = MRVDRV_NF_DEFAULT_SCAN_VALUE;
	} else {
		iwe.u.qual.noise = CAL_NF(priv->NF[TYPE_BEACON][TYPE_NOAVG]);
	}

	
	if ((priv->mode == IW_MODE_ADHOC) && priv->adhoccreate
	    && !lbs_ssid_cmp(priv->curbssparams.ssid,
			     priv->curbssparams.ssid_len,
			     bss->ssid, bss->ssid_len)) {
		int snr, nf;
		snr = priv->SNR[TYPE_RXPD][TYPE_AVG] / AVG_SCALE;
		nf = priv->NF[TYPE_RXPD][TYPE_AVG] / AVG_SCALE;
		iwe.u.qual.level = CAL_RSSI(snr, nf);
	}
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_QUAL_LEN);

	
	iwe.cmd = SIOCGIWENCODE;
	if (bss->capability & WLAN_CAPABILITY_PRIVACY) {
		iwe.u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
	} else {
		iwe.u.data.flags = IW_ENCODE_DISABLED;
	}
	iwe.u.data.length = 0;
	start = iwe_stream_add_point(info, start, stop, &iwe, bss->ssid);

	current_val = start + iwe_stream_lcp_len(info);

	iwe.cmd = SIOCGIWRATE;
	iwe.u.bitrate.fixed = 0;
	iwe.u.bitrate.disabled = 0;
	iwe.u.bitrate.value = 0;

	for (j = 0; j < ARRAY_SIZE(bss->rates) && bss->rates[j]; j++) {
		
		iwe.u.bitrate.value = bss->rates[j] * 500000;
		current_val = iwe_stream_add_value(info, start, current_val,
						   stop, &iwe, IW_EV_PARAM_LEN);
	}
	if ((bss->mode == IW_MODE_ADHOC) && priv->adhoccreate
	    && !lbs_ssid_cmp(priv->curbssparams.ssid,
			     priv->curbssparams.ssid_len,
			     bss->ssid, bss->ssid_len)) {
		iwe.u.bitrate.value = 22 * 500000;
		current_val = iwe_stream_add_value(info, start, current_val,
						   stop, &iwe, IW_EV_PARAM_LEN);
	}
	
	if ((current_val - start) > iwe_stream_lcp_len(info))
		start = current_val;

	memset(&iwe, 0, sizeof(iwe));
	if (bss->wpa_ie_len) {
		char buf[MAX_WPA_IE_LEN];
		memcpy(buf, bss->wpa_ie, bss->wpa_ie_len);
		iwe.cmd = IWEVGENIE;
		iwe.u.data.length = bss->wpa_ie_len;
		start = iwe_stream_add_point(info, start, stop, &iwe, buf);
	}

	memset(&iwe, 0, sizeof(iwe));
	if (bss->rsn_ie_len) {
		char buf[MAX_WPA_IE_LEN];
		memcpy(buf, bss->rsn_ie, bss->rsn_ie_len);
		iwe.cmd = IWEVGENIE;
		iwe.u.data.length = bss->rsn_ie_len;
		start = iwe_stream_add_point(info, start, stop, &iwe, buf);
	}

	if (bss->mesh) {
		char custom[MAX_CUSTOM_LEN];
		char *p = custom;

		iwe.cmd = IWEVCUSTOM;
		p += snprintf(p, MAX_CUSTOM_LEN, "mesh-type: olpc");
		iwe.u.data.length = p - custom;
		if (iwe.u.data.length)
			start = iwe_stream_add_point(info, start, stop,
						     &iwe, custom);
	}

out:
	lbs_deb_leave_args(LBS_DEB_SCAN, "start %p", start);
	return start;
}



int lbs_set_scan(struct net_device *dev, struct iw_request_info *info,
		 union iwreq_data *wrqu, char *extra)
{
	DECLARE_SSID_BUF(ssid);
	struct lbs_private *priv = dev->ml_priv;
	int ret = 0;

	lbs_deb_enter(LBS_DEB_WEXT);

	if (!priv->radio_on) {
		ret = -EINVAL;
		goto out;
	}

	if (!netif_running(dev)) {
		ret = -ENETDOWN;
		goto out;
	}

	

	if (wrqu->data.length == sizeof(struct iw_scan_req) &&
	    wrqu->data.flags & IW_SCAN_THIS_ESSID) {
		struct iw_scan_req *req = (struct iw_scan_req *)extra;
		priv->scan_ssid_len = req->essid_len;
		memcpy(priv->scan_ssid, req->essid, priv->scan_ssid_len);
		lbs_deb_wext("set_scan, essid '%s'\n",
			print_ssid(ssid, priv->scan_ssid, priv->scan_ssid_len));
	} else {
		priv->scan_ssid_len = 0;
	}

	if (!delayed_work_pending(&priv->scan_work))
		queue_delayed_work(priv->work_thread, &priv->scan_work,
				   msecs_to_jiffies(50));
	
	priv->scan_channel = -1;

	if (priv->surpriseremoved)
		ret = -EIO;

out:
	lbs_deb_leave_args(LBS_DEB_WEXT, "ret %d", ret);
	return ret;
}



int lbs_get_scan(struct net_device *dev, struct iw_request_info *info,
		 struct iw_point *dwrq, char *extra)
{
#define SCAN_ITEM_SIZE 128
	struct lbs_private *priv = dev->ml_priv;
	int err = 0;
	char *ev = extra;
	char *stop = ev + dwrq->length;
	struct bss_descriptor *iter_bss;
	struct bss_descriptor *safe;

	lbs_deb_enter(LBS_DEB_WEXT);

	
	if (priv->scan_channel)
		return -EAGAIN;

	
	if ((priv->mode == IW_MODE_ADHOC) && priv->adhoccreate)
		lbs_prepare_and_send_command(priv, CMD_802_11_RSSI, 0,
					     CMD_OPTION_WAITFORRSP, 0, NULL);

	mutex_lock(&priv->lock);
	list_for_each_entry_safe (iter_bss, safe, &priv->network_list, list) {
		char *next_ev;
		unsigned long stale_time;

		if (stop - ev < SCAN_ITEM_SIZE) {
			err = -E2BIG;
			break;
		}

		
		if (dev == priv->mesh_dev && !iter_bss->mesh)
			continue;

		
		stale_time = iter_bss->last_scanned + DEFAULT_MAX_SCAN_AGE;
		if (time_after(jiffies, stale_time)) {
			list_move_tail(&iter_bss->list, &priv->network_free_list);
			clear_bss_descriptor(iter_bss);
			continue;
		}

		
		next_ev = lbs_translate_scan(priv, info, ev, stop, iter_bss);
		if (next_ev == NULL)
			continue;
		ev = next_ev;
	}
	mutex_unlock(&priv->lock);

	dwrq->length = (ev - extra);
	dwrq->flags = 0;

	lbs_deb_leave_args(LBS_DEB_WEXT, "ret %d", err);
	return err;
}












static int lbs_ret_80211_scan(struct lbs_private *priv, unsigned long dummy,
			      struct cmd_header *resp)
{
	struct cmd_ds_802_11_scan_rsp *scanresp = (void *)resp;
	struct bss_descriptor *iter_bss;
	struct bss_descriptor *safe;
	uint8_t *bssinfo;
	uint16_t scanrespsize;
	int bytesleft;
	int idx;
	int tlvbufsize;
	int ret;

	lbs_deb_enter(LBS_DEB_SCAN);

	
	list_for_each_entry_safe (iter_bss, safe, &priv->network_list, list) {
		unsigned long stale_time = iter_bss->last_scanned + DEFAULT_MAX_SCAN_AGE;
		if (time_before(jiffies, stale_time))
			continue;
		list_move_tail (&iter_bss->list, &priv->network_free_list);
		clear_bss_descriptor(iter_bss);
	}

	if (scanresp->nr_sets > MAX_NETWORK_COUNT) {
		lbs_deb_scan("SCAN_RESP: too many scan results (%d, max %d)\n",
			     scanresp->nr_sets, MAX_NETWORK_COUNT);
		ret = -1;
		goto done;
	}

	bytesleft = get_unaligned_le16(&scanresp->bssdescriptsize);
	lbs_deb_scan("SCAN_RESP: bssdescriptsize %d\n", bytesleft);

	scanrespsize = le16_to_cpu(resp->size);
	lbs_deb_scan("SCAN_RESP: scan results %d\n", scanresp->nr_sets);

	bssinfo = scanresp->bssdesc_and_tlvbuffer;

	
	tlvbufsize = scanrespsize - (bytesleft + sizeof(scanresp->bssdescriptsize)
				     + sizeof(scanresp->nr_sets)
				     + S_DS_GEN);

	
	for (idx = 0; idx < scanresp->nr_sets && bytesleft; idx++) {
		struct bss_descriptor new;
		struct bss_descriptor *found = NULL;
		struct bss_descriptor *oldest = NULL;

		
		memset(&new, 0, sizeof (struct bss_descriptor));
		if (lbs_process_bss(&new, &bssinfo, &bytesleft) != 0) {
			
			lbs_deb_scan("SCAN_RESP: process_bss returned ERROR\n");
			continue;
		}

		
		list_for_each_entry (iter_bss, &priv->network_list, list) {
			if (is_same_network(iter_bss, &new)) {
				found = iter_bss;
				break;
			}

			if ((oldest == NULL) ||
			    (iter_bss->last_scanned < oldest->last_scanned))
				oldest = iter_bss;
		}

		if (found) {
			
			clear_bss_descriptor(found);
		} else if (!list_empty(&priv->network_free_list)) {
			
			found = list_entry(priv->network_free_list.next,
					   struct bss_descriptor, list);
			list_move_tail(&found->list, &priv->network_list);
		} else if (oldest) {
			
			found = oldest;
			clear_bss_descriptor(found);
			list_move_tail(&found->list, &priv->network_list);
		} else {
			continue;
		}

		lbs_deb_scan("SCAN_RESP: BSSID %pM\n", new.bssid);

		
		memcpy(found, &new, offsetof(struct bss_descriptor, list));
	}

	ret = 0;

done:
	lbs_deb_leave_args(LBS_DEB_SCAN, "ret %d", ret);
	return ret;
}
