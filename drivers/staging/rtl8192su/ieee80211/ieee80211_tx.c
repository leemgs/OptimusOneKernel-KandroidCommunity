

#include <linux/compiler.h>

#include <linux/errno.h>
#include <linux/if_arp.h>
#include <linux/in6.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/tcp.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/wireless.h>
#include <linux/etherdevice.h>
#include <asm/uaccess.h>
#include <linux/if_vlan.h>

#include "ieee80211.h"




static u8 P802_1H_OUI[P80211_OUI_LEN] = { 0x00, 0x00, 0xf8 };
static u8 RFC1042_OUI[P80211_OUI_LEN] = { 0x00, 0x00, 0x00 };

static inline int ieee80211_put_snap(u8 *data, u16 h_proto)
{
	struct ieee80211_snap_hdr *snap;
	u8 *oui;

	snap = (struct ieee80211_snap_hdr *)data;
	snap->dsap = 0xaa;
	snap->ssap = 0xaa;
	snap->ctrl = 0x03;

	if (h_proto == 0x8137 || h_proto == 0x80f3)
		oui = P802_1H_OUI;
	else
		oui = RFC1042_OUI;
	snap->oui[0] = oui[0];
	snap->oui[1] = oui[1];
	snap->oui[2] = oui[2];

	*(u16 *)(data + SNAP_SIZE) = htons(h_proto);

	return SNAP_SIZE + sizeof(u16);
}

int ieee80211_encrypt_fragment(
	struct ieee80211_device *ieee,
	struct sk_buff *frag,
	int hdr_len)
{
	struct ieee80211_crypt_data* crypt = ieee->crypt[ieee->tx_keyidx];
	int res;

	if (!(crypt && crypt->ops))
	{
		printk("=========>%s(), crypt is null\n", __FUNCTION__);
		return -1;
	}
#ifdef CONFIG_IEEE80211_CRYPT_TKIP
	struct rtl_ieee80211_hdr *header;

	if (ieee->tkip_countermeasures &&
	    crypt && crypt->ops && strcmp(crypt->ops->name, "TKIP") == 0) {
		header = (struct rtl_ieee80211_hdr *)frag->data;
		if (net_ratelimit()) {
			printk(KERN_DEBUG "%s: TKIP countermeasures: dropped "
			       "TX packet to " MAC_FMT "\n",
			       ieee->dev->name, MAC_ARG(header->addr1));
		}
		return -1;
	}
#endif
	

	
	
	atomic_inc(&crypt->refcnt);
	res = 0;
	if (crypt->ops->encrypt_msdu)
		res = crypt->ops->encrypt_msdu(frag, hdr_len, crypt->priv);
	if (res == 0 && crypt->ops->encrypt_mpdu)
		res = crypt->ops->encrypt_mpdu(frag, hdr_len, crypt->priv);

	atomic_dec(&crypt->refcnt);
	if (res < 0) {
		printk(KERN_INFO "%s: Encryption failed: len=%d.\n",
		       ieee->dev->name, frag->len);
		ieee->ieee_stats.tx_discards++;
		return -1;
	}

	return 0;
}


void ieee80211_txb_free(struct ieee80211_txb *txb) {
	
	if (unlikely(!txb))
		return;
	kfree(txb);
}

struct ieee80211_txb *ieee80211_alloc_txb(int nr_frags, int txb_size,
					  int gfp_mask)
{
	struct ieee80211_txb *txb;
	int i;
	txb = kmalloc(
		sizeof(struct ieee80211_txb) + (sizeof(u8*) * nr_frags),
		gfp_mask);
	if (!txb)
		return NULL;

	memset(txb, 0, sizeof(struct ieee80211_txb));
	txb->nr_frags = nr_frags;
	txb->frag_size = txb_size;

	for (i = 0; i < nr_frags; i++) {
		txb->fragments[i] = dev_alloc_skb(txb_size);
		if (unlikely(!txb->fragments[i])) {
			i--;
			break;
		}
		memset(txb->fragments[i]->cb, 0, sizeof(txb->fragments[i]->cb));
	}
	if (unlikely(i != nr_frags)) {
		while (i >= 0)
			dev_kfree_skb_any(txb->fragments[i--]);
		kfree(txb);
		return NULL;
	}
	return txb;
}



static int
ieee80211_classify(struct sk_buff *skb, struct ieee80211_network *network)
{
	struct ethhdr *eth;
	struct iphdr *ip;
	eth = (struct ethhdr *)skb->data;
	if (eth->h_proto != htons(ETH_P_IP))
		return 0;


	ip = ip_hdr(skb);

	switch (ip->tos & 0xfc) {
		case 0x20:
			return 2;
		case 0x40:
			return 1;
		case 0x60:
			return 3;
		case 0x80:
			return 4;
		case 0xa0:
			return 5;
		case 0xc0:
			return 6;
		case 0xe0:
			return 7;
		default:
			return 0;
	}
}

void ieee80211_tx_query_agg_cap(struct ieee80211_device* ieee, struct sk_buff* skb, cb_desc* tcb_desc)
{
	PRT_HIGH_THROUGHPUT	pHTInfo = ieee->pHTInfo;
	PTX_TS_RECORD			pTxTs = NULL;
	struct ieee80211_hdr_1addr* hdr = (struct ieee80211_hdr_1addr*)skb->data;

	if (!pHTInfo->bCurrentHTSupport||!pHTInfo->bEnableHT)
		return;
	if (!IsQoSDataFrame(skb->data))
		return;

	if (is_multicast_ether_addr(hdr->addr1) || is_broadcast_ether_addr(hdr->addr1))
		return;
	
#ifdef TO_DO_LIST
	if(pTcb->PacketLength >= 4096)
		return;
	
	if(!Adapter->HalFunc.GetNmodeSupportBySecCfgHandler(Adapter))
		return;
#endif

	if(pHTInfo->IOTAction & HT_IOT_ACT_TX_NO_AGGREGATION)
		return;

#if 1
	if(!ieee->GetNmodeSupportBySecCfg(ieee->dev))
	{
		return;
	}
#endif
	if(pHTInfo->bCurrentAMPDUEnable)
	{
		if (!GetTs(ieee, (PTS_COMMON_INFO*)(&pTxTs), hdr->addr1, skb->priority, TX_DIR, true))
		{
			printk("===>can't get TS\n");
			return;
		}
		if (pTxTs->TxAdmittedBARecord.bValid == false)
		{
			
			if (ieee->wpa_ie_len && (ieee->pairwise_key_type == KEY_TYPE_NA))
			;
			else
			TsStartAddBaProcess(ieee, pTxTs);
			goto FORCED_AGG_SETTING;
		}
		else if (pTxTs->bUsingBa == false)
		{
			if (SN_LESS(pTxTs->TxAdmittedBARecord.BaStartSeqCtrl.field.SeqNum, (pTxTs->TxCurSeq+1)%4096))
				pTxTs->bUsingBa = true;
			else
				goto FORCED_AGG_SETTING;
		}

		if (ieee->iw_mode == IW_MODE_INFRA)
		{
			tcb_desc->bAMPDUEnable = true;
			tcb_desc->ampdu_factor = pHTInfo->CurrentAMPDUFactor;
			tcb_desc->ampdu_density = pHTInfo->CurrentMPDUDensity;
		}
	}
FORCED_AGG_SETTING:
	switch(pHTInfo->ForcedAMPDUMode )
	{
		case HT_AGG_AUTO:
			break;

		case HT_AGG_FORCE_ENABLE:
			tcb_desc->bAMPDUEnable = true;
			tcb_desc->ampdu_density = pHTInfo->ForcedMPDUDensity;
			tcb_desc->ampdu_factor = pHTInfo->ForcedAMPDUFactor;
			break;

		case HT_AGG_FORCE_DISABLE:
			tcb_desc->bAMPDUEnable = false;
			tcb_desc->ampdu_density = 0;
			tcb_desc->ampdu_factor = 0;
			break;

	}
		return;
}

extern void ieee80211_qurey_ShortPreambleMode(struct ieee80211_device* ieee, cb_desc* tcb_desc)
{
	tcb_desc->bUseShortPreamble = false;
	if (tcb_desc->data_rate == 2)
	{
		return;
	}
	else if (ieee->current_network.capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
	{
		tcb_desc->bUseShortPreamble = true;
	}
	return;
}
extern	void
ieee80211_query_HTCapShortGI(struct ieee80211_device *ieee, cb_desc *tcb_desc)
{
	PRT_HIGH_THROUGHPUT		pHTInfo = ieee->pHTInfo;

	tcb_desc->bUseShortGI 		= false;

	if(!pHTInfo->bCurrentHTSupport||!pHTInfo->bEnableHT)
		return;

	if(pHTInfo->bForcedShortGI)
	{
		tcb_desc->bUseShortGI = true;
		return;
	}

	if((pHTInfo->bCurBW40MHz==true) && pHTInfo->bCurShortGI40MHz)
		tcb_desc->bUseShortGI = true;
	else if((pHTInfo->bCurBW40MHz==false) && pHTInfo->bCurShortGI20MHz)
		tcb_desc->bUseShortGI = true;
}

void ieee80211_query_BandwidthMode(struct ieee80211_device* ieee, cb_desc *tcb_desc)
{
	PRT_HIGH_THROUGHPUT	pHTInfo = ieee->pHTInfo;

	tcb_desc->bPacketBW = false;

	if(!pHTInfo->bCurrentHTSupport||!pHTInfo->bEnableHT)
		return;

	if(tcb_desc->bMulticast || tcb_desc->bBroadcast)
		return;

	if((tcb_desc->data_rate & 0x80)==0) 
		return;
	
	if(pHTInfo->bCurBW40MHz && pHTInfo->bCurTxBW40MHz && !ieee->bandwidth_auto_switch.bforced_tx20Mhz)
		tcb_desc->bPacketBW = true;
	return;
}

void ieee80211_query_protectionmode(struct ieee80211_device* ieee, cb_desc* tcb_desc, struct sk_buff* skb)
{
	
	tcb_desc->bRTSSTBC			= false;
	tcb_desc->bRTSUseShortGI		= false; 
	tcb_desc->bCTSEnable			= false; 
	tcb_desc->RTSSC				= 0;		
	tcb_desc->bRTSBW			= false; 

	if(tcb_desc->bBroadcast || tcb_desc->bMulticast)
		return;

	if (is_broadcast_ether_addr(skb->data+16))  
		return;

	if (ieee->mode < IEEE_N_24G) 
	{
			
			
			
			
		if (skb->len > ieee->rts)
		{
			tcb_desc->bRTSEnable = true;
			tcb_desc->rts_rate = MGN_24M;
		}
		else if (ieee->current_network.buseprotection)
		{
			
			tcb_desc->bRTSEnable = true;
			tcb_desc->bCTSEnable = true;
			tcb_desc->rts_rate = MGN_24M;
		}
		
		return;
	}
	else
	{
		PRT_HIGH_THROUGHPUT pHTInfo = ieee->pHTInfo;
		while (true)
		{
			
			if(pHTInfo->IOTAction & HT_IOT_ACT_FORCED_CTS2SELF)
			{
				tcb_desc->bCTSEnable	= true;
				tcb_desc->rts_rate  = 	MGN_24M;
				tcb_desc->bRTSEnable = false;
				break;
			}
			else if(pHTInfo->IOTAction & (HT_IOT_ACT_FORCED_RTS|HT_IOT_ACT_PURE_N_MODE))
			{
				tcb_desc->bRTSEnable = true;
				tcb_desc->rts_rate  = 	MGN_24M;
				break;
			}
			
			if (ieee->current_network.buseprotection)
			{
				tcb_desc->bRTSEnable = true;
				tcb_desc->bCTSEnable = true;
				tcb_desc->rts_rate = MGN_24M;
				break;
			}
			
			if(pHTInfo->bCurrentHTSupport  && pHTInfo->bEnableHT)
			{
				u8 HTOpMode = pHTInfo->CurrentOpMode;
				if((pHTInfo->bCurBW40MHz && (HTOpMode == 2 || HTOpMode == 3)) ||
							(!pHTInfo->bCurBW40MHz && HTOpMode == 3) )
				{
					tcb_desc->rts_rate = MGN_24M; 
					tcb_desc->bRTSEnable = true;
					break;
				}
			}
			
			if (skb->len > ieee->rts)
			{
				tcb_desc->rts_rate = MGN_24M; 
				tcb_desc->bRTSEnable = true;
				break;
			}
			
			
			if(tcb_desc->bAMPDUEnable)
			{
				tcb_desc->rts_rate = MGN_24M; 
				
				
				tcb_desc->bRTSEnable = false;
				break;
			}
			
			goto NO_PROTECTION;
		}
		}
	
	if( 0 )
	{
		tcb_desc->bCTSEnable	= true;
		tcb_desc->rts_rate = MGN_24M;
		tcb_desc->bRTSEnable 	= true;
	}
	if (ieee->current_network.capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
		tcb_desc->bUseShortPreamble = true;
	if (ieee->mode == IW_MODE_MASTER)
			goto NO_PROTECTION;
	return;
NO_PROTECTION:
	tcb_desc->bRTSEnable	= false;
	tcb_desc->bCTSEnable	= false;
	tcb_desc->rts_rate		= 0;
	tcb_desc->RTSSC		= 0;
	tcb_desc->bRTSBW		= false;
}


void ieee80211_txrate_selectmode(struct ieee80211_device* ieee, cb_desc* tcb_desc)
{
#ifdef TO_DO_LIST
	if(!IsDataFrame(pFrame))
	{
		pTcb->bTxDisableRateFallBack = TRUE;
		pTcb->bTxUseDriverAssingedRate = TRUE;
		pTcb->RATRIndex = 7;
		return;
	}

	if(pMgntInfo->ForcedDataRate!= 0)
	{
		pTcb->bTxDisableRateFallBack = TRUE;
		pTcb->bTxUseDriverAssingedRate = TRUE;
		return;
	}
#endif
	if(ieee->bTxDisableRateFallBack)
		tcb_desc->bTxDisableRateFallBack = true;

	if(ieee->bTxUseDriverAssingedRate)
		tcb_desc->bTxUseDriverAssingedRate = true;
	if(!tcb_desc->bTxDisableRateFallBack || !tcb_desc->bTxUseDriverAssingedRate)
	{
		if (ieee->iw_mode == IW_MODE_INFRA || ieee->iw_mode == IW_MODE_ADHOC)
			tcb_desc->RATRIndex = 0;
	}
}

void ieee80211_query_seqnum(struct ieee80211_device*ieee, struct sk_buff* skb, u8* dst)
{
	if (is_multicast_ether_addr(dst) || is_broadcast_ether_addr(dst))
		return;
	if (IsQoSDataFrame(skb->data)) 
	{
		PTX_TS_RECORD pTS = NULL;
		if (!GetTs(ieee, (PTS_COMMON_INFO*)(&pTS), dst, skb->priority, TX_DIR, true))
		{
			return;
		}
		pTS->TxCurSeq = (pTS->TxCurSeq+1)%4096;
	}
}

int rtl8192_ieee80211_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ieee80211_device *ieee = netdev_priv(dev);
	struct ieee80211_txb *txb = NULL;
	struct ieee80211_hdr_3addrqos *frag_hdr;
	int i, bytes_per_frag, nr_frags, bytes_last_frag, frag_size;
	unsigned long flags;
	struct net_device_stats *stats = &ieee->stats;
	int ether_type = 0, encrypt;
	int bytes, fc, qos_ctl = 0, hdr_len;
	struct sk_buff *skb_frag;
	struct ieee80211_hdr_3addrqos header = { 
		.duration_id = 0,
		.seq_ctl = 0,
		.qos_ctl = 0
	};
	u8 dest[ETH_ALEN], src[ETH_ALEN];
	int qos_actived = ieee->current_network.qos_data.active;

	struct ieee80211_crypt_data* crypt;

	cb_desc *tcb_desc;

	spin_lock_irqsave(&ieee->lock, flags);

	
	if ((!ieee->hard_start_xmit && !(ieee->softmac_features & IEEE_SOFTMAC_TX_QUEUE))||
	   ((!ieee->softmac_data_hard_start_xmit && (ieee->softmac_features & IEEE_SOFTMAC_TX_QUEUE)))) {
		printk(KERN_WARNING "%s: No xmit handler.\n",
		       ieee->dev->name);
		goto success;
	}


	if(likely(ieee->raw_tx == 0)){
		if (unlikely(skb->len < SNAP_SIZE + sizeof(u16))) {
			printk(KERN_WARNING "%s: skb too small (%d).\n",
			ieee->dev->name, skb->len);
			goto success;
		}

		memset(skb->cb, 0, sizeof(skb->cb));
		ether_type = ntohs(((struct ethhdr *)skb->data)->h_proto);

		crypt = ieee->crypt[ieee->tx_keyidx];

		encrypt = !(ether_type == ETH_P_PAE && ieee->ieee802_1x) &&
			ieee->host_encrypt && crypt && crypt->ops;

		if (!encrypt && ieee->ieee802_1x &&
		ieee->drop_unencrypted && ether_type != ETH_P_PAE) {
			stats->tx_dropped++;
			goto success;
		}
	#ifdef CONFIG_IEEE80211_DEBUG
		if (crypt && !encrypt && ether_type == ETH_P_PAE) {
			struct eapol *eap = (struct eapol *)(skb->data +
				sizeof(struct ethhdr) - SNAP_SIZE - sizeof(u16));
			IEEE80211_DEBUG_EAP("TX: IEEE 802.11 EAPOL frame: %s\n",
				eap_get_type(eap->type));
		}
	#endif

		
		memcpy(&dest, skb->data, ETH_ALEN);
		memcpy(&src, skb->data+ETH_ALEN, ETH_ALEN);

                
                skb_pull(skb, sizeof(struct ethhdr));

                
                bytes = skb->len + SNAP_SIZE + sizeof(u16);

		if (encrypt)
			fc = IEEE80211_FTYPE_DATA | IEEE80211_FCTL_WEP;
		else

                        fc = IEEE80211_FTYPE_DATA;

		
		if(qos_actived)
			fc |= IEEE80211_STYPE_QOS_DATA;
		else
			fc |= IEEE80211_STYPE_DATA;

		if (ieee->iw_mode == IW_MODE_INFRA) {
			fc |= IEEE80211_FCTL_TODS;
			
			memcpy(&header.addr1, ieee->current_network.bssid, ETH_ALEN);
			memcpy(&header.addr2, &src, ETH_ALEN);
			memcpy(&header.addr3, &dest, ETH_ALEN);
		} else if (ieee->iw_mode == IW_MODE_ADHOC) {
			
			memcpy(&header.addr1, dest, ETH_ALEN);
			memcpy(&header.addr2, src, ETH_ALEN);
			memcpy(&header.addr3, ieee->current_network.bssid, ETH_ALEN);
		}

                header.frame_ctl = cpu_to_le16(fc);

		
		if (is_multicast_ether_addr(header.addr1) ||
		is_broadcast_ether_addr(header.addr1)) {
			frag_size = MAX_FRAG_THRESHOLD;
			qos_ctl |= QOS_CTL_NOTCONTAIN_ACK;
		}
		else {
			frag_size = ieee->fts;
			qos_ctl = 0;
		}

		
		if(qos_actived)
		{
			hdr_len = IEEE80211_3ADDR_LEN + 2;

			skb->priority = ieee80211_classify(skb, &ieee->current_network);
			qos_ctl |= skb->priority; 
			header.qos_ctl = cpu_to_le16(qos_ctl & IEEE80211_QOS_TID);
		} else {
			hdr_len = IEEE80211_3ADDR_LEN;
		}
		
		bytes_per_frag = frag_size - hdr_len;
		if (ieee->config &
		(CFG_IEEE80211_COMPUTE_FCS | CFG_IEEE80211_RESERVE_FCS))
			bytes_per_frag -= IEEE80211_FCS_LEN;

		
		if (encrypt)
			bytes_per_frag -= crypt->ops->extra_prefix_len +
				crypt->ops->extra_postfix_len;

		
		nr_frags = bytes / bytes_per_frag;
		bytes_last_frag = bytes % bytes_per_frag;
		if (bytes_last_frag)
			nr_frags++;
		else
			bytes_last_frag = bytes_per_frag;

		
		txb = ieee80211_alloc_txb(nr_frags, frag_size + ieee->tx_headroom, GFP_ATOMIC);
		if (unlikely(!txb)) {
			printk(KERN_WARNING "%s: Could not allocate TXB\n",
			ieee->dev->name);
			goto failed;
		}
		txb->encrypted = encrypt;
		txb->payload_size = bytes;

		
		if(qos_actived)
		{
			txb->queue_index = UP2AC(skb->priority);
		} else {
			txb->queue_index = WME_AC_BK;;
		}



		for (i = 0; i < nr_frags; i++) {
			skb_frag = txb->fragments[i];
			tcb_desc = (cb_desc *)(skb_frag->cb + MAX_DEV_ADDR_SIZE);
			if(qos_actived){
				skb_frag->priority = skb->priority;
				tcb_desc->queue_index =  UP2AC(skb->priority);
			} else {
				skb_frag->priority = WME_AC_BK;
				tcb_desc->queue_index = WME_AC_BK;
			}
			skb_reserve(skb_frag, ieee->tx_headroom);

			if (encrypt){
				if (ieee->hwsec_active)
					tcb_desc->bHwSec = 1;
				else
					tcb_desc->bHwSec = 0;
				skb_reserve(skb_frag, crypt->ops->extra_prefix_len);
			}
			else
			{
				tcb_desc->bHwSec = 0;
			}
			frag_hdr = (struct ieee80211_hdr_3addrqos *)skb_put(skb_frag, hdr_len);
			memcpy(frag_hdr, &header, hdr_len);

			
			if (i != nr_frags - 1) {
				frag_hdr->frame_ctl = cpu_to_le16(
					fc | IEEE80211_FCTL_MOREFRAGS);
				bytes = bytes_per_frag;

			} else {
				
				bytes = bytes_last_frag;
			}
			
			if(qos_actived)
			{
				
				frag_hdr->seq_ctl = cpu_to_le16(ieee->seq_ctrl[UP2AC(skb->priority)+1]<<4 | i);
			} else {
				frag_hdr->seq_ctl = cpu_to_le16(ieee->seq_ctrl[0]<<4 | i);
			}

			
			if (i == 0) {
				ieee80211_put_snap(
					skb_put(skb_frag, SNAP_SIZE + sizeof(u16)),
					ether_type);
				bytes -= SNAP_SIZE + sizeof(u16);
			}

			memcpy(skb_put(skb_frag, bytes), skb->data, bytes);

			
			skb_pull(skb, bytes);

			
			if (encrypt)
				ieee80211_encrypt_fragment(ieee, skb_frag, hdr_len);
			if (ieee->config &
			(CFG_IEEE80211_COMPUTE_FCS | CFG_IEEE80211_RESERVE_FCS))
				skb_put(skb_frag, 4);
		}

		if(qos_actived)
		{
		  if (ieee->seq_ctrl[UP2AC(skb->priority) + 1] == 0xFFF)
			ieee->seq_ctrl[UP2AC(skb->priority) + 1] = 0;
		  else
			ieee->seq_ctrl[UP2AC(skb->priority) + 1]++;
		} else {
  		  if (ieee->seq_ctrl[0] == 0xFFF)
			ieee->seq_ctrl[0] = 0;
		  else
			ieee->seq_ctrl[0]++;
		}
	}else{
		if (unlikely(skb->len < sizeof(struct ieee80211_hdr_3addr))) {
			printk(KERN_WARNING "%s: skb too small (%d).\n",
			ieee->dev->name, skb->len);
			goto success;
		}

		txb = ieee80211_alloc_txb(1, skb->len, GFP_ATOMIC);
		if(!txb){
			printk(KERN_WARNING "%s: Could not allocate TXB\n",
			ieee->dev->name);
			goto failed;
		}

		txb->encrypted = 0;
		txb->payload_size = skb->len;
		memcpy(skb_put(txb->fragments[0],skb->len), skb->data, skb->len);
	}

 success:

	if (txb)
	{
#if 1
		cb_desc *tcb_desc = (cb_desc *)(txb->fragments[0]->cb + MAX_DEV_ADDR_SIZE);
		tcb_desc->bTxEnableFwCalcDur = 1;
		if (is_multicast_ether_addr(header.addr1))
			tcb_desc->bMulticast = 1;
		if (is_broadcast_ether_addr(header.addr1))
			tcb_desc->bBroadcast = 1;
		ieee80211_txrate_selectmode(ieee, tcb_desc);
		if ( tcb_desc->bMulticast ||  tcb_desc->bBroadcast)
			tcb_desc->data_rate = ieee->basic_rate;
		else
			
			tcb_desc->data_rate = CURRENT_RATE(ieee->mode, ieee->rate, ieee->HTCurrentOperaRate);
		ieee80211_qurey_ShortPreambleMode(ieee, tcb_desc);
		ieee80211_tx_query_agg_cap(ieee, txb->fragments[0], tcb_desc);
		ieee80211_query_HTCapShortGI(ieee, tcb_desc);
		ieee80211_query_BandwidthMode(ieee, tcb_desc);
		ieee80211_query_protectionmode(ieee, tcb_desc, txb->fragments[0]);
		ieee80211_query_seqnum(ieee, txb->fragments[0], header.addr1);

		
#endif
	}
	spin_unlock_irqrestore(&ieee->lock, flags);
	dev_kfree_skb_any(skb);
	if (txb) {
		if (ieee->softmac_features & IEEE_SOFTMAC_TX_QUEUE){
			ieee80211_softmac_xmit(txb, ieee);
		}else{
			if ((*ieee->hard_start_xmit)(txb, dev) == 0) {
				stats->tx_packets++;
				stats->tx_bytes += txb->payload_size;
				return 0;
			}
			ieee80211_txb_free(txb);
		}
	}

	return 0;

 failed:
	spin_unlock_irqrestore(&ieee->lock, flags);
	netif_stop_queue(dev);
	stats->tx_errors++;
	return 1;

}
