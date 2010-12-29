

#include "ath9k.h"

#define FUDGE 2


static int ath_beaconq_config(struct ath_softc *sc)
{
	struct ath_hw *ah = sc->sc_ah;
	struct ath9k_tx_queue_info qi;

	ath9k_hw_get_txq_props(ah, sc->beacon.beaconq, &qi);
	if (sc->sc_ah->opmode == NL80211_IFTYPE_AP) {
		
		qi.tqi_aifs = 1;
		qi.tqi_cwmin = 0;
		qi.tqi_cwmax = 0;
	} else {
		
		qi.tqi_aifs = sc->beacon.beacon_qi.tqi_aifs;
		qi.tqi_cwmin = 2*sc->beacon.beacon_qi.tqi_cwmin;
		qi.tqi_cwmax = sc->beacon.beacon_qi.tqi_cwmax;
	}

	if (!ath9k_hw_set_txq_props(ah, sc->beacon.beaconq, &qi)) {
		DPRINTF(sc, ATH_DBG_FATAL,
			"Unable to update h/w beacon queue parameters\n");
		return 0;
	} else {
		ath9k_hw_resettxqueue(ah, sc->beacon.beaconq);
		return 1;
	}
}


static void ath_beacon_setup(struct ath_softc *sc, struct ath_vif *avp,
			     struct ath_buf *bf)
{
	struct sk_buff *skb = bf->bf_mpdu;
	struct ath_hw *ah = sc->sc_ah;
	struct ath_desc *ds;
	struct ath9k_11n_rate_series series[4];
	const struct ath_rate_table *rt;
	int flags, antenna, ctsrate = 0, ctsduration = 0;
	u8 rate;

	ds = bf->bf_desc;
	flags = ATH9K_TXDESC_NOACK;

	if (((sc->sc_ah->opmode == NL80211_IFTYPE_ADHOC) ||
	     (sc->sc_ah->opmode == NL80211_IFTYPE_MESH_POINT)) &&
	    (ah->caps.hw_caps & ATH9K_HW_CAP_VEOL)) {
		ds->ds_link = bf->bf_daddr; 
		flags |= ATH9K_TXDESC_VEOL;
		
		antenna = 0;
	} else {
		ds->ds_link = 0;
		
		antenna = ((sc->beacon.ast_be_xmit / sc->nbcnvifs) & 1 ? 2 : 1);
	}

	ds->ds_data = bf->bf_buf_addr;

	rt = sc->cur_rate_table;
	rate = rt->info[0].ratecode;
	if (sc->sc_flags & SC_OP_PREAMBLE_SHORT)
		rate |= rt->info[0].short_preamble;

	ath9k_hw_set11n_txdesc(ah, ds, skb->len + FCS_LEN,
			       ATH9K_PKT_TYPE_BEACON,
			       MAX_RATE_POWER,
			       ATH9K_TXKEYIX_INVALID,
			       ATH9K_KEY_TYPE_CLEAR,
			       flags);

	
	ath9k_hw_filltxdesc(ah, ds, roundup(skb->len, 4),
			    true, true, ds);

	memset(series, 0, sizeof(struct ath9k_11n_rate_series) * 4);
	series[0].Tries = 1;
	series[0].Rate = rate;
	series[0].ChSel = sc->tx_chainmask;
	series[0].RateFlags = (ctsrate) ? ATH9K_RATESERIES_RTS_CTS : 0;
	ath9k_hw_set11n_ratescenario(ah, ds, ds, 0, ctsrate, ctsduration,
				     series, 4, 0);
}

static struct ath_buf *ath_beacon_generate(struct ieee80211_hw *hw,
					   struct ieee80211_vif *vif)
{
	struct ath_wiphy *aphy = hw->priv;
	struct ath_softc *sc = aphy->sc;
	struct ath_buf *bf;
	struct ath_vif *avp;
	struct sk_buff *skb;
	struct ath_txq *cabq;
	struct ieee80211_tx_info *info;
	int cabq_depth;

	if (aphy->state != ATH_WIPHY_ACTIVE)
		return NULL;

	avp = (void *)vif->drv_priv;
	cabq = sc->beacon.cabq;

	if (avp->av_bcbuf == NULL)
		return NULL;

	

	bf = avp->av_bcbuf;
	skb = bf->bf_mpdu;
	if (skb) {
		dma_unmap_single(sc->dev, bf->bf_dmacontext,
				 skb->len, DMA_TO_DEVICE);
		dev_kfree_skb_any(skb);
	}

	

	skb = ieee80211_beacon_get(hw, vif);
	bf->bf_mpdu = skb;
	if (skb == NULL)
		return NULL;
	((struct ieee80211_mgmt *)skb->data)->u.beacon.timestamp =
		avp->tsf_adjust;

	info = IEEE80211_SKB_CB(skb);
	if (info->flags & IEEE80211_TX_CTL_ASSIGN_SEQ) {
		
		struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
		sc->tx.seq_no += 0x10;
		hdr->seq_ctrl &= cpu_to_le16(IEEE80211_SCTL_FRAG);
		hdr->seq_ctrl |= cpu_to_le16(sc->tx.seq_no);
	}

	bf->bf_buf_addr = bf->bf_dmacontext =
		dma_map_single(sc->dev, skb->data,
			       skb->len, DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(sc->dev, bf->bf_buf_addr))) {
		dev_kfree_skb_any(skb);
		bf->bf_mpdu = NULL;
		DPRINTF(sc, ATH_DBG_FATAL, "dma_mapping_error on beaconing\n");
		return NULL;
	}

	skb = ieee80211_get_buffered_bc(hw, vif);

	
	spin_lock_bh(&cabq->axq_lock);
	cabq_depth = cabq->axq_depth;
	spin_unlock_bh(&cabq->axq_lock);

	if (skb && cabq_depth) {
		if (sc->nvifs > 1) {
			DPRINTF(sc, ATH_DBG_BEACON,
				"Flushing previous cabq traffic\n");
			ath_draintxq(sc, cabq, false);
		}
	}

	ath_beacon_setup(sc, avp, bf);

	while (skb) {
		ath_tx_cabq(hw, skb);
		skb = ieee80211_get_buffered_bc(hw, vif);
	}

	return bf;
}


static void ath_beacon_start_adhoc(struct ath_softc *sc,
				   struct ieee80211_vif *vif)
{
	struct ath_hw *ah = sc->sc_ah;
	struct ath_buf *bf;
	struct ath_vif *avp;
	struct sk_buff *skb;

	avp = (void *)vif->drv_priv;

	if (avp->av_bcbuf == NULL)
		return;

	bf = avp->av_bcbuf;
	skb = bf->bf_mpdu;

	ath_beacon_setup(sc, avp, bf);

	
	ath9k_hw_puttxbuf(ah, sc->beacon.beaconq, bf->bf_daddr);
	ath9k_hw_txstart(ah, sc->beacon.beaconq);
	DPRINTF(sc, ATH_DBG_BEACON, "TXDP%u = %llx (%p)\n",
		sc->beacon.beaconq, ito64(bf->bf_daddr), bf->bf_desc);
}

int ath_beaconq_setup(struct ath_hw *ah)
{
	struct ath9k_tx_queue_info qi;

	memset(&qi, 0, sizeof(qi));
	qi.tqi_aifs = 1;
	qi.tqi_cwmin = 0;
	qi.tqi_cwmax = 0;
	
	return ath9k_hw_setuptxqueue(ah, ATH9K_TX_QUEUE_BEACON, &qi);
}

int ath_beacon_alloc(struct ath_wiphy *aphy, struct ieee80211_vif *vif)
{
	struct ath_softc *sc = aphy->sc;
	struct ath_vif *avp;
	struct ath_buf *bf;
	struct sk_buff *skb;
	__le64 tstamp;

	avp = (void *)vif->drv_priv;

	
	if (!avp->av_bcbuf) {
		
		avp->av_bcbuf = list_first_entry(&sc->beacon.bbuf,
						 struct ath_buf, list);
		list_del(&avp->av_bcbuf->list);

		if (sc->sc_ah->opmode == NL80211_IFTYPE_AP ||
		    !(sc->sc_ah->caps.hw_caps & ATH9K_HW_CAP_VEOL)) {
			int slot;
			
			avp->av_bslot = 0;
			for (slot = 0; slot < ATH_BCBUF; slot++)
				if (sc->beacon.bslot[slot] == NULL) {
					
					if (slot+1 < ATH_BCBUF &&
					    sc->beacon.bslot[slot+1] == NULL) {
						avp->av_bslot = slot+1;
						break;
					}
					avp->av_bslot = slot;
					
				}
			BUG_ON(sc->beacon.bslot[avp->av_bslot] != NULL);
			sc->beacon.bslot[avp->av_bslot] = vif;
			sc->beacon.bslot_aphy[avp->av_bslot] = aphy;
			sc->nbcnvifs++;
		}
	}

	
	bf = avp->av_bcbuf;
	if (bf->bf_mpdu != NULL) {
		skb = bf->bf_mpdu;
		dma_unmap_single(sc->dev, bf->bf_dmacontext,
				 skb->len, DMA_TO_DEVICE);
		dev_kfree_skb_any(skb);
		bf->bf_mpdu = NULL;
	}

	
	skb = ieee80211_beacon_get(sc->hw, vif);
	if (skb == NULL) {
		DPRINTF(sc, ATH_DBG_BEACON, "cannot get skb\n");
		return -ENOMEM;
	}

	tstamp = ((struct ieee80211_mgmt *)skb->data)->u.beacon.timestamp;
	sc->beacon.bc_tstamp = le64_to_cpu(tstamp);
	
	if (avp->av_bslot > 0) {
		u64 tsfadjust;
		int intval;

		intval = sc->beacon_interval ? : ATH_DEFAULT_BINTVAL;

		
		tsfadjust = intval * avp->av_bslot / ATH_BCBUF;
		avp->tsf_adjust = cpu_to_le64(TU_TO_USEC(tsfadjust));

		DPRINTF(sc, ATH_DBG_BEACON,
			"stagger beacons, bslot %d intval %u tsfadjust %llu\n",
			avp->av_bslot, intval, (unsigned long long)tsfadjust);

		((struct ieee80211_mgmt *)skb->data)->u.beacon.timestamp =
			avp->tsf_adjust;
	} else
		avp->tsf_adjust = cpu_to_le64(0);

	bf->bf_mpdu = skb;
	bf->bf_buf_addr = bf->bf_dmacontext =
		dma_map_single(sc->dev, skb->data,
			       skb->len, DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(sc->dev, bf->bf_buf_addr))) {
		dev_kfree_skb_any(skb);
		bf->bf_mpdu = NULL;
		DPRINTF(sc, ATH_DBG_FATAL,
			"dma_mapping_error on beacon alloc\n");
		return -ENOMEM;
	}

	return 0;
}

void ath_beacon_return(struct ath_softc *sc, struct ath_vif *avp)
{
	if (avp->av_bcbuf != NULL) {
		struct ath_buf *bf;

		if (avp->av_bslot != -1) {
			sc->beacon.bslot[avp->av_bslot] = NULL;
			sc->beacon.bslot_aphy[avp->av_bslot] = NULL;
			sc->nbcnvifs--;
		}

		bf = avp->av_bcbuf;
		if (bf->bf_mpdu != NULL) {
			struct sk_buff *skb = bf->bf_mpdu;
			dma_unmap_single(sc->dev, bf->bf_dmacontext,
					 skb->len, DMA_TO_DEVICE);
			dev_kfree_skb_any(skb);
			bf->bf_mpdu = NULL;
		}
		list_add_tail(&bf->list, &sc->beacon.bbuf);

		avp->av_bcbuf = NULL;
	}
}

void ath_beacon_tasklet(unsigned long data)
{
	struct ath_softc *sc = (struct ath_softc *)data;
	struct ath_hw *ah = sc->sc_ah;
	struct ath_buf *bf = NULL;
	struct ieee80211_vif *vif;
	struct ath_wiphy *aphy;
	int slot;
	u32 bfaddr, bc = 0, tsftu;
	u64 tsf;
	u16 intval;

	
	if (ath9k_hw_numtxpending(ah, sc->beacon.beaconq) != 0) {
		sc->beacon.bmisscnt++;

		if (sc->beacon.bmisscnt < BSTUCK_THRESH) {
			DPRINTF(sc, ATH_DBG_BEACON,
				"missed %u consecutive beacons\n",
				sc->beacon.bmisscnt);
		} else if (sc->beacon.bmisscnt >= BSTUCK_THRESH) {
			DPRINTF(sc, ATH_DBG_BEACON,
				"beacon is officially stuck\n");
			sc->sc_flags |= SC_OP_TSF_RESET;
			ath_reset(sc, false);
		}

		return;
	}

	if (sc->beacon.bmisscnt != 0) {
		DPRINTF(sc, ATH_DBG_BEACON,
			"resume beacon xmit after %u misses\n",
			sc->beacon.bmisscnt);
		sc->beacon.bmisscnt = 0;
	}

	

	intval = sc->beacon_interval ? : ATH_DEFAULT_BINTVAL;

	tsf = ath9k_hw_gettsf64(ah);
	tsftu = TSF_TO_TU(tsf>>32, tsf);
	slot = ((tsftu % intval) * ATH_BCBUF) / intval;
	
	slot = ATH_BCBUF - slot - 1;
	vif = sc->beacon.bslot[slot];
	aphy = sc->beacon.bslot_aphy[slot];

	DPRINTF(sc, ATH_DBG_BEACON,
		"slot %d [tsf %llu tsftu %u intval %u] vif %p\n",
		slot, tsf, tsftu, intval, vif);

	bfaddr = 0;
	if (vif) {
		bf = ath_beacon_generate(aphy->hw, vif);
		if (bf != NULL) {
			bfaddr = bf->bf_daddr;
			bc = 1;
		}
	}

	
	if (sc->beacon.updateslot == UPDATE) {
		sc->beacon.updateslot = COMMIT; 
		sc->beacon.slotupdate = slot;
	} else if (sc->beacon.updateslot == COMMIT && sc->beacon.slotupdate == slot) {
		ath9k_hw_setslottime(sc->sc_ah, sc->beacon.slottime);
		sc->beacon.updateslot = OK;
	}
	if (bfaddr != 0) {
		
		if (!ath9k_hw_stoptxdma(ah, sc->beacon.beaconq)) {
			DPRINTF(sc, ATH_DBG_FATAL,
				"beacon queue %u did not stop?\n", sc->beacon.beaconq);
		}

		
		ath9k_hw_puttxbuf(ah, sc->beacon.beaconq, bfaddr);
		ath9k_hw_txstart(ah, sc->beacon.beaconq);

		sc->beacon.ast_be_xmit += bc;     
	}
}


static void ath_beacon_config_ap(struct ath_softc *sc,
				 struct ath_beacon_config *conf)
{
	u32 nexttbtt, intval;

	

	if (!(sc->sc_flags & SC_OP_TSF_RESET))
		return;

	
	intval = conf->beacon_interval & ATH9K_BEACON_PERIOD;
	intval /= ATH_BCBUF;    
	nexttbtt = intval;
	intval |= ATH9K_BEACON_RESET_TSF;

	
	intval |= ATH9K_BEACON_ENA;
	sc->imask |= ATH9K_INT_SWBA;
	ath_beaconq_config(sc);

	

	ath9k_hw_set_interrupts(sc->sc_ah, 0);
	ath9k_hw_beaconinit(sc->sc_ah, nexttbtt, intval);
	sc->beacon.bmisscnt = 0;
	ath9k_hw_set_interrupts(sc->sc_ah, sc->imask);

	

	sc->sc_flags &= ~SC_OP_TSF_RESET;
}


static void ath_beacon_config_sta(struct ath_softc *sc,
				  struct ath_beacon_config *conf)
{
	struct ath9k_beacon_state bs;
	int dtimperiod, dtimcount, sleepduration;
	int cfpperiod, cfpcount;
	u32 nexttbtt = 0, intval, tsftu;
	u64 tsf;
	int num_beacons, offset, dtim_dec_count, cfp_dec_count;

	memset(&bs, 0, sizeof(bs));
	intval = conf->beacon_interval & ATH9K_BEACON_PERIOD;

	
	dtimperiod = conf->dtim_period;
	if (dtimperiod <= 0)		
		dtimperiod = 1;
	dtimcount = conf->dtim_count;
	if (dtimcount >= dtimperiod)	
		dtimcount = 0;
	cfpperiod = 1;			
	cfpcount = 0;

	sleepduration = conf->listen_interval * intval;
	if (sleepduration <= 0)
		sleepduration = intval;

	
	tsf = ath9k_hw_gettsf64(sc->sc_ah);
	tsftu = TSF_TO_TU(tsf>>32, tsf) + FUDGE;

	num_beacons = tsftu / intval + 1;
	offset = tsftu % intval;
	nexttbtt = tsftu - offset;
	if (offset)
		nexttbtt += intval;

	
	dtim_dec_count = num_beacons % dtimperiod;
	
	cfp_dec_count = (num_beacons / dtimperiod) % cfpperiod;
	if (dtim_dec_count)
		cfp_dec_count++;

	dtimcount -= dtim_dec_count;
	if (dtimcount < 0)
		dtimcount += dtimperiod;

	cfpcount -= cfp_dec_count;
	if (cfpcount < 0)
		cfpcount += cfpperiod;

	bs.bs_intval = intval;
	bs.bs_nexttbtt = nexttbtt;
	bs.bs_dtimperiod = dtimperiod*intval;
	bs.bs_nextdtim = bs.bs_nexttbtt + dtimcount*intval;
	bs.bs_cfpperiod = cfpperiod*bs.bs_dtimperiod;
	bs.bs_cfpnext = bs.bs_nextdtim + cfpcount*bs.bs_dtimperiod;
	bs.bs_cfpmaxduration = 0;

	
	if (sleepduration > intval) {
		bs.bs_bmissthreshold = conf->listen_interval *
			ATH_DEFAULT_BMISS_LIMIT / 2;
	} else {
		bs.bs_bmissthreshold = DIV_ROUND_UP(conf->bmiss_timeout, intval);
		if (bs.bs_bmissthreshold > 15)
			bs.bs_bmissthreshold = 15;
		else if (bs.bs_bmissthreshold <= 0)
			bs.bs_bmissthreshold = 1;
	}

	

	bs.bs_sleepduration = roundup(IEEE80211_MS_TO_TU(100), sleepduration);
	if (bs.bs_sleepduration > bs.bs_dtimperiod)
		bs.bs_sleepduration = bs.bs_dtimperiod;

	
	bs.bs_tsfoor_threshold = ATH9K_TSFOOR_THRESHOLD;

	DPRINTF(sc, ATH_DBG_BEACON, "tsf: %llu tsftu: %u\n", tsf, tsftu);
	DPRINTF(sc, ATH_DBG_BEACON,
		"bmiss: %u sleep: %u cfp-period: %u maxdur: %u next: %u\n",
		bs.bs_bmissthreshold, bs.bs_sleepduration,
		bs.bs_cfpperiod, bs.bs_cfpmaxduration, bs.bs_cfpnext);

	

	ath9k_hw_set_interrupts(sc->sc_ah, 0);
	ath9k_hw_set_sta_beacon_timers(sc->sc_ah, &bs);
	sc->imask |= ATH9K_INT_BMISS;
	ath9k_hw_set_interrupts(sc->sc_ah, sc->imask);
}

static void ath_beacon_config_adhoc(struct ath_softc *sc,
				    struct ath_beacon_config *conf,
				    struct ieee80211_vif *vif)
{
	u64 tsf;
	u32 tsftu, intval, nexttbtt;

	intval = conf->beacon_interval & ATH9K_BEACON_PERIOD;


	

	nexttbtt = TSF_TO_TU(sc->beacon.bc_tstamp >> 32, sc->beacon.bc_tstamp);
	if (nexttbtt == 0)
                nexttbtt = intval;
        else if (intval)
                nexttbtt = roundup(nexttbtt, intval);

	tsf = ath9k_hw_gettsf64(sc->sc_ah);
	tsftu = TSF_TO_TU((u32)(tsf>>32), (u32)tsf) + FUDGE;
	do {
		nexttbtt += intval;
	} while (nexttbtt < tsftu);

	DPRINTF(sc, ATH_DBG_BEACON,
		"IBSS nexttbtt %u intval %u (%u)\n",
		nexttbtt, intval, conf->beacon_interval);

	
	intval |= ATH9K_BEACON_ENA;
	if (!(sc->sc_ah->caps.hw_caps & ATH9K_HW_CAP_VEOL))
		sc->imask |= ATH9K_INT_SWBA;

	ath_beaconq_config(sc);

	

	ath9k_hw_set_interrupts(sc->sc_ah, 0);
	ath9k_hw_beaconinit(sc->sc_ah, nexttbtt, intval);
	sc->beacon.bmisscnt = 0;
	ath9k_hw_set_interrupts(sc->sc_ah, sc->imask);

	
	if (vif && sc->sc_ah->caps.hw_caps & ATH9K_HW_CAP_VEOL)
		ath_beacon_start_adhoc(sc, vif);
}

void ath_beacon_config(struct ath_softc *sc, struct ieee80211_vif *vif)
{
	struct ath_beacon_config *cur_conf = &sc->cur_beacon_conf;
	enum nl80211_iftype iftype;

	

	if (vif) {
		struct ieee80211_bss_conf *bss_conf = &vif->bss_conf;

		iftype = vif->type;

		cur_conf->beacon_interval = bss_conf->beacon_int;
		cur_conf->dtim_period = bss_conf->dtim_period;
		cur_conf->listen_interval = 1;
		cur_conf->dtim_count = 1;
		cur_conf->bmiss_timeout =
			ATH_DEFAULT_BMISS_LIMIT * cur_conf->beacon_interval;
	} else {
		iftype = sc->sc_ah->opmode;
	}

	
	if (cur_conf->beacon_interval == 0)
		cur_conf->beacon_interval = 100;

	switch (iftype) {
	case NL80211_IFTYPE_AP:
		ath_beacon_config_ap(sc, cur_conf);
		break;
	case NL80211_IFTYPE_ADHOC:
	case NL80211_IFTYPE_MESH_POINT:
		ath_beacon_config_adhoc(sc, cur_conf, vif);
		break;
	case NL80211_IFTYPE_STATION:
		ath_beacon_config_sta(sc, cur_conf);
		break;
	default:
		DPRINTF(sc, ATH_DBG_CONFIG,
			"Unsupported beaconing mode\n");
		return;
	}

	sc->sc_flags |= SC_OP_BEACONS;
}
