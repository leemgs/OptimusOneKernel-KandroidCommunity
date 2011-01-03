
#include "r8192E.h"
#include "r8192E_hw.h"
#include "r819xE_cmdpkt.h"


#define		CMPK_DEBOUNCE_CNT			1

#define		CMPK_PRINT(Address)\
{\
	unsigned char	i;\
	u32	temp[10];\
	\
	memcpy(temp, Address, 40);\
	for (i = 0; i <40; i+=4)\
		printk("\r\n %08x", temp[i]);\
}\



RT_STATUS cmpk_message_handle_tx(
	struct net_device *dev,
	u8*	code_virtual_address,
	u32	packettype,
	u32	buffer_len)
{

	RT_STATUS 	    rt_status = RT_STATUS_SUCCESS;
#ifdef RTL8192U
	return rt_status;
#else
	struct r8192_priv   *priv = ieee80211_priv(dev);
	u16		    frag_threshold;
	u16		    frag_length = 0, frag_offset = 0;
	rt_firmware	    *pfirmware = priv->pFirmware;
	struct sk_buff	    *skb;
	unsigned char	    *seg_ptr;
	cb_desc		    *tcb_desc;
	u8                  bLastIniPkt;

	PTX_FWINFO_8190PCI      pTxFwInfo = NULL;
        int i;

	
	RT_TRACE(COMP_CMDPKT,"%s(),buffer_len is %d\n",__FUNCTION__,buffer_len);
	firmware_init_param(dev);
	
	frag_threshold = pfirmware->cmdpacket_frag_thresold;
        do {
            if((buffer_len - frag_offset) > frag_threshold) {
                frag_length = frag_threshold ;
                bLastIniPkt = 0;

            } else {
                frag_length =(u16)(buffer_len - frag_offset);
                bLastIniPkt = 1;

            }

            
#ifdef RTL8192U
            skb  = dev_alloc_skb(USB_HWDESC_HEADER_LEN + frag_length + 4);
#else
            skb  = dev_alloc_skb(frag_length + priv->ieee80211->tx_headroom + 4);
#endif
            if(skb == NULL) {
                rt_status = RT_STATUS_FAILURE;
                goto Failed;
            }

            memcpy((unsigned char *)(skb->cb),&dev,sizeof(dev));
            tcb_desc = (cb_desc*)(skb->cb + MAX_DEV_ADDR_SIZE);
            tcb_desc->queue_index = TXCMD_QUEUE;
            tcb_desc->bCmdOrInit = packettype;
            tcb_desc->bLastIniPkt = bLastIniPkt;
            tcb_desc->pkt_size = frag_length;

#ifdef RTL8192U
            skb_reserve(skb, USB_HWDESC_HEADER_LEN);
#endif

            
            seg_ptr = skb_put(skb, priv->ieee80211->tx_headroom);

            pTxFwInfo = (PTX_FWINFO_8190PCI)seg_ptr;
            memset(pTxFwInfo,0,sizeof(TX_FWINFO_8190PCI));
            memset(pTxFwInfo,0x12,8);

            seg_ptr +=sizeof(TX_FWINFO_8190PCI);

            
            seg_ptr = skb->tail;
            for(i=0 ; i < frag_length; i+=4) {
                *seg_ptr++ = ((i+0)<frag_length)?code_virtual_address[i+3]:0;
                *seg_ptr++ = ((i+1)<frag_length)?code_virtual_address[i+2]:0;
                *seg_ptr++ = ((i+2)<frag_length)?code_virtual_address[i+1]:0;
                *seg_ptr++ = ((i+3)<frag_length)?code_virtual_address[i+0]:0;
            }
            skb_put(skb, i);
            priv->ieee80211->softmac_hard_start_xmit(skb,dev);

            code_virtual_address += frag_length;
            frag_offset += frag_length;

        }while(frag_offset < buffer_len);

Failed:
	
	return rt_status;


#endif
}	


static	void
cmpk_count_txstatistic(
	struct net_device *dev,
	cmpk_txfb_t	*pstx_fb)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
#ifdef ENABLE_PS
	RT_RF_POWER_STATE	rtState;

	pAdapter->HalFunc.GetHwRegHandler(pAdapter, HW_VAR_RF_STATE, (pu1Byte)(&rtState));

	
	
	
	if (rtState == eRfOff)
	{
		return;
	}
#endif

#ifdef TODO
	if(pAdapter->bInHctTest)
		return;
#endif
	
	if (pstx_fb->tok)
	{
		priv->stats.txfeedbackok++;
		priv->stats.txoktotal++;
		priv->stats.txokbytestotal += pstx_fb->pkt_length;
		priv->stats.txokinperiod++;

		
		if (pstx_fb->pkt_type == PACKET_MULTICAST)
		{
			priv->stats.txmulticast++;
			priv->stats.txbytesmulticast += pstx_fb->pkt_length;
		}
		else if (pstx_fb->pkt_type == PACKET_BROADCAST)
		{
			priv->stats.txbroadcast++;
			priv->stats.txbytesbroadcast += pstx_fb->pkt_length;
		}
		else
		{
			priv->stats.txunicast++;
			priv->stats.txbytesunicast += pstx_fb->pkt_length;
		}
	}
	else
	{
		priv->stats.txfeedbackfail++;
		priv->stats.txerrtotal++;
		priv->stats.txerrbytestotal += pstx_fb->pkt_length;

		
		if (pstx_fb->pkt_type == PACKET_MULTICAST)
		{
			priv->stats.txerrmulticast++;
		}
		else if (pstx_fb->pkt_type == PACKET_BROADCAST)
		{
			priv->stats.txerrbroadcast++;
		}
		else
		{
			priv->stats.txerrunicast++;
		}
	}

	priv->stats.txretrycount += pstx_fb->retry_cnt;
	priv->stats.txfeedbackretry += pstx_fb->retry_cnt;

}	




static	void
cmpk_handle_tx_feedback(
	struct net_device *dev,
	u8	*	pmsg)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	cmpk_txfb_t		rx_tx_fb;	

	priv->stats.txfeedback++;

	
	

	
	
#if 0		
	
	
	rx_tx_fb.TOK 			= pMsg[2]>>7;
	rx_tx_fb.Fail_Reason 	= (pMsg[2] & 0x70) >> 4;
	rx_tx_fb.TID 			= (pMsg[2] & 0x0F);
	rx_tx_fb.Qos_Pkt 		= pMsg[3] >> 7;
	rx_tx_fb.Bandwidth 		= (pMsg[3] & 0x40) >> 6;
	rx_tx_fb.Retry_Cnt 		= pMsg[5];
	rx_tx_fb.Pkt_ID 		= (pMsg[6] << 8) | pMsg[7];
	rx_tx_fb.Seq_Num 		= (pMsg[8] << 8) | pMsg[9];
	rx_tx_fb.S_Rate 		= pMsg[10];
	rx_tx_fb.F_Rate 		= pMsg[11];
	rx_tx_fb.S_RTS_Rate 	= pMsg[12];
	rx_tx_fb.F_RTS_Rate 	= pMsg[13];
	rx_tx_fb.pkt_length	= (pMsg[14] << 8) | pMsg[15];
#endif
	
	
	memcpy((u8*)&rx_tx_fb, pmsg, sizeof(cmpk_txfb_t));
	
	cmpk_count_txstatistic(dev, &rx_tx_fb);
#if 0
	
	if (pAdapter->RegWirelessMode == WIRELESS_MODE_A ||
		pAdapter->RegWirelessMode == WIRELESS_MODE_B ||
		pAdapter->RegWirelessMode == WIRELESS_MODE_G)
	{
		pMgntInfo->CurrentOperaRate = (rx_tx_fb.F_Rate & 0x7F);
	}
	else if (pAdapter->RegWirelessMode == WIRELESS_MODE_N_24G ||
		 	 pAdapter->RegWirelessMode == WIRELESS_MODE_N_5G)
	{
		pMgntInfo->HTCurrentOperaRate = (rx_tx_fb.F_Rate & 0x8F);
	}
#endif
	
	
	
	

}	

static void cmdpkt_beacontimerinterrupt_819xusb(struct net_device *dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	u16 tx_rate;
	{
		
		
		
		if((priv->ieee80211->current_network.mode == IEEE_A)  ||
			(priv->ieee80211->current_network.mode == IEEE_N_5G) ||
			((priv->ieee80211->current_network.mode == IEEE_N_24G)  && (!priv->ieee80211->pHTInfo->bCurSuppCCK)))
		{
			tx_rate = 60;
			DMESG("send beacon frame  tx rate is 6Mbpm\n");
		}
		else
		{
			tx_rate =10;
			DMESG("send beacon frame  tx rate is 1Mbpm\n");
		}

		

	}

}





static	void
cmpk_handle_interrupt_status(
	struct net_device *dev,
	u8*	pmsg)
{
	cmpk_intr_sta_t		rx_intr_status;	
	struct r8192_priv *priv = ieee80211_priv(dev);

	DMESG("---> cmpk_Handle_Interrupt_Status()\n");

	
	

	
	
	
	
	rx_intr_status.length = pmsg[1];
	if (rx_intr_status.length != (sizeof(cmpk_intr_sta_t) - 2))
	{
		DMESG("cmpk_Handle_Interrupt_Status: wrong length!\n");
		return;
	}


	
	if(	priv->ieee80211->iw_mode == IW_MODE_ADHOC)
	{
		
		rx_intr_status.interrupt_status = *((u32 *)(pmsg + 4));
		

		DMESG("interrupt status = 0x%x\n", rx_intr_status.interrupt_status);

		if (rx_intr_status.interrupt_status & ISR_TxBcnOk)
		{
			priv->ieee80211->bibsscoordinator = true;
			priv->stats.txbeaconokint++;
		}
		else if (rx_intr_status.interrupt_status & ISR_TxBcnErr)
		{
			priv->ieee80211->bibsscoordinator = false;
			priv->stats.txbeaconerr++;
		}

		if (rx_intr_status.interrupt_status & ISR_BcnTimerIntr)
		{
			cmdpkt_beacontimerinterrupt_819xusb(dev);
		}

	}

	 


	DMESG("<---- cmpk_handle_interrupt_status()\n");

}	



static	void
cmpk_handle_query_config_rx(
	struct net_device *dev,
	u8*	   pmsg)
{
	cmpk_query_cfg_t	rx_query_cfg;	

	
	

	
	
	
	
	rx_query_cfg.cfg_action 	= (pmsg[4] & 0x80000000)>>31;
	rx_query_cfg.cfg_type 		= (pmsg[4] & 0x60) >> 5;
	rx_query_cfg.cfg_size 		= (pmsg[4] & 0x18) >> 3;
	rx_query_cfg.cfg_page 		= (pmsg[6] & 0x0F) >> 0;
	rx_query_cfg.cfg_offset 		= pmsg[7];
	rx_query_cfg.value 			= (pmsg[8] << 24) | (pmsg[9] << 16) |
								  (pmsg[10] << 8) | (pmsg[11] << 0);
	rx_query_cfg.mask 			= (pmsg[12] << 24) | (pmsg[13] << 16) |
								  (pmsg[14] << 8) | (pmsg[15] << 0);

}	



static	void	cmpk_count_tx_status(	struct net_device *dev,
									cmpk_tx_status_t 	*pstx_status)
{
	struct r8192_priv *priv = ieee80211_priv(dev);

#ifdef ENABLE_PS

	RT_RF_POWER_STATE	rtstate;

	pAdapter->HalFunc.GetHwRegHandler(pAdapter, HW_VAR_RF_STATE, (pu1Byte)(&rtState));

	
	
	
	if (rtState == eRfOff)
	{
		return;
	}
#endif

	priv->stats.txfeedbackok	+= pstx_status->txok;
	priv->stats.txoktotal		+= pstx_status->txok;

	priv->stats.txfeedbackfail	+= pstx_status->txfail;
	priv->stats.txerrtotal		+= pstx_status->txfail;

	priv->stats.txretrycount		+= pstx_status->txretry;
	priv->stats.txfeedbackretry	+= pstx_status->txretry;

	
	
	

	priv->stats.txmulticast	+= pstx_status->txmcok;
	priv->stats.txbroadcast	+= pstx_status->txbcok;
	priv->stats.txunicast		+= pstx_status->txucok;

	priv->stats.txerrmulticast	+= pstx_status->txmcfail;
	priv->stats.txerrbroadcast	+= pstx_status->txbcfail;
	priv->stats.txerrunicast	+= pstx_status->txucfail;

	priv->stats.txbytesmulticast	+= pstx_status->txmclength;
	priv->stats.txbytesbroadcast	+= pstx_status->txbclength;
	priv->stats.txbytesunicast		+= pstx_status->txuclength;

	priv->stats.last_packet_rate		= pstx_status->rate;
}	




static	void
cmpk_handle_tx_status(
	struct net_device *dev,
	u8*	   pmsg)
{
	cmpk_tx_status_t	rx_tx_sts;	

	memcpy((void*)&rx_tx_sts, (void*)pmsg, sizeof(cmpk_tx_status_t));
	
	cmpk_count_tx_status(dev, &rx_tx_sts);

}	



static	void
cmpk_handle_tx_rate_history(
	struct net_device *dev,
	u8*	   pmsg)
{
	cmpk_tx_rahis_t	*ptxrate;

	u8				i, j;
	u16				length = sizeof(cmpk_tx_rahis_t);
	u32				*ptemp;
	struct r8192_priv *priv = ieee80211_priv(dev);


#ifdef ENABLE_PS
	pAdapter->HalFunc.GetHwRegHandler(pAdapter, HW_VAR_RF_STATE, (pu1Byte)(&rtState));

	
	
	
	if (rtState == eRfOff)
	{
		return;
	}
#endif

	ptemp = (u32 *)pmsg;

	
	
	
	
	for (i = 0; i < (length/4); i++)
	{
		u16	 temp1, temp2;

		temp1 = ptemp[i]&0x0000FFFF;
		temp2 = ptemp[i]>>16;
		ptemp[i] = (temp1<<16)|temp2;
	}

	ptxrate = (cmpk_tx_rahis_t *)pmsg;

	if (ptxrate == NULL )
	{
		return;
	}

	for (i = 0; i < 16; i++)
	{
		
		if (i < 4)
			priv->stats.txrate.cck[i] += ptxrate->cck[i];

		
		if (i< 8)
			priv->stats.txrate.ofdm[i] += ptxrate->ofdm[i];

		for (j = 0; j < 4; j++)
			priv->stats.txrate.ht_mcs[j][i] += ptxrate->ht_mcs[j][i];
	}

}	



u32 cmpk_message_handle_rx(struct net_device *dev, struct ieee80211_rx_stats *pstats)
{

	struct r8192_priv *priv = ieee80211_priv(dev);
	int			total_length;
	u8			cmd_length, exe_cnt = 0;
	u8			element_id;
	u8			*pcmd_buff;

        RT_TRACE(COMP_EVENTS, "---->cmpk_message_handle_rx()\n");

	
	if ((pstats== NULL))
	{
		
		
		return 0;	
	}

	
	total_length = pstats->Length;

	
	pcmd_buff = pstats->virtual_address;

	
	element_id = pcmd_buff[0];
	

	
	
	while (total_length > 0 || exe_cnt++ >100)
	{
		
		element_id = pcmd_buff[0];

		switch(element_id)
		{
			case RX_TX_FEEDBACK:

        			RT_TRACE(COMP_EVENTS, "---->cmpk_message_handle_rx():RX_TX_FEEDBACK\n");
				cmpk_handle_tx_feedback (dev, pcmd_buff);
				cmd_length = CMPK_RX_TX_FB_SIZE;
				break;

			case RX_INTERRUPT_STATUS:

			        RT_TRACE(COMP_EVENTS, "---->cmpk_message_handle_rx():RX_INTERRUPT_STATUS\n");
				cmpk_handle_interrupt_status(dev, pcmd_buff);
				cmd_length = sizeof(cmpk_intr_sta_t);
				break;

			case BOTH_QUERY_CONFIG:

			        RT_TRACE(COMP_EVENTS, "---->cmpk_message_handle_rx():BOTH_QUERY_CONFIG\n");
				cmpk_handle_query_config_rx(dev, pcmd_buff);
				cmd_length = CMPK_BOTH_QUERY_CONFIG_SIZE;
				break;

			case RX_TX_STATUS:

			        RT_TRACE(COMP_EVENTS, "---->cmpk_message_handle_rx():RX_TX_STATUS\n");
				cmpk_handle_tx_status(dev, pcmd_buff);
				cmd_length = CMPK_RX_TX_STS_SIZE;
				break;

			case RX_TX_PER_PKT_FEEDBACK:
				
				
				
			        RT_TRACE(COMP_EVENTS, "---->cmpk_message_handle_rx():RX_TX_PER_PKT_FEEDBACK\n");
				cmd_length = CMPK_RX_TX_FB_SIZE;
				break;

			case RX_TX_RATE_HISTORY:
				

			        RT_TRACE(COMP_EVENTS, "---->cmpk_message_handle_rx():RX_TX_HISTORY\n");
				cmpk_handle_tx_rate_history(dev, pcmd_buff);
				cmd_length = CMPK_TX_RAHIS_SIZE;
				break;

			default:

			        RT_TRACE(COMP_EVENTS, "---->cmpk_message_handle_rx():unknow CMD Element\n");
				return 1;	
		}
		
		

		
		

		
		priv->stats.rxcmdpkt[element_id]++;

		total_length -= cmd_length;
		pcmd_buff    += cmd_length;
	}	
	return	1;	

	RT_TRACE(COMP_EVENTS, "<----cmpk_message_handle_rx()\n");
}	
