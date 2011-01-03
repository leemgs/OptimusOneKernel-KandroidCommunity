

#include "rt_config.h"


#ifdef CONFIG_APSTA_MIXED_SUPPORT
UINT32 CW_MAX_IN_BITS;
#endif 





PSTRING mac = "";		   
PSTRING hostname = "";		   
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (mac, "s");
#else
module_param (mac, charp, 0);
#endif
MODULE_PARM_DESC (mac, "rt28xx: wireless mac addr");







int rt28xx_close(IN struct net_device *net_dev);
int rt28xx_open(struct net_device *net_dev);


static INT rt28xx_send_packets(IN struct sk_buff *skb_p, IN struct net_device *net_dev);


static struct net_device_stats *RT28xx_get_ether_stats(
    IN  struct net_device *net_dev);


int MainVirtualIF_close(IN struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);

	
	if (pAd == NULL)
		return 0; 

	netif_carrier_off(pAd->net_dev);
	netif_stop_queue(pAd->net_dev);




#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		BOOLEAN			Cancelled;
#ifdef QOS_DLS_SUPPORT
		
		if (pAd->CommonCfg.bDLSCapable)
		{
			UCHAR i;

			
			for (i=0; i<MAX_NUM_OF_INIT_DLS_ENTRY; i++)
			{
				if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
				{
					RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
					pAd->StaCfg.DLSEntry[i].Status	= DLS_NONE;
					pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
				}
			}

			
			for (i=MAX_NUM_OF_INIT_DLS_ENTRY; i<MAX_NUM_OF_DLS_ENTRY; i++)
			{
				if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
				{
					RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
					pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
					pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
				}
			}
			RTMP_MLME_HANDLER(pAd);
		}
#endif 

		if (INFRA_ON(pAd) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			MLME_DISASSOC_REQ_STRUCT	DisReq;
			MLME_QUEUE_ELEM *MsgElem = (MLME_QUEUE_ELEM *) kmalloc(sizeof(MLME_QUEUE_ELEM), MEM_ALLOC_FLAG);

			if (MsgElem)
			{
			COPY_MAC_ADDR(DisReq.Addr, pAd->CommonCfg.Bssid);
			DisReq.Reason =  REASON_DEAUTH_STA_LEAVING;

			MsgElem->Machine = ASSOC_STATE_MACHINE;
			MsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
			MsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
			NdisMoveMemory(MsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

			
			pAd->MlmeAux.AutoReconnectSsidLen= 32;
			NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
			MlmeDisassocReqAction(pAd, MsgElem);
			kfree(MsgElem);
			}

			RTMPusecDelay(1000);
		}

		RTMPCancelTimer(&pAd->StaCfg.StaQuickResponeForRateUpTimer, &Cancelled);
		RTMPCancelTimer(&pAd->StaCfg.WpaDisassocAndBlockAssocTimer, &Cancelled);

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
		
		RtmpOSWrielessEventSend(pAd, IWEVCUSTOM, RT_INTERFACE_DOWN, NULL, NULL, 0);
#endif 
#endif 


	}
#endif 

	VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();

	return 0; 
}


int MainVirtualIF_open(IN struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);

	
	if (pAd == NULL)
		return 0; 

	if (VIRTUAL_IF_UP(pAd) != 0)
		return -1;

	
	RT_MOD_INC_USE_COUNT();

	netif_start_queue(net_dev);
	netif_carrier_on(net_dev);
	netif_wake_queue(net_dev);

	return 0;
}


int rt28xx_close(IN PNET_DEV dev)
{
	struct net_device * net_dev = (struct net_device *)dev;
    RTMP_ADAPTER	*pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);
	BOOLEAN			Cancelled;
	UINT32			i = 0;


	DBGPRINT(RT_DEBUG_TRACE, ("===> rt28xx_close\n"));

	Cancelled = FALSE;
	
	if (pAd == NULL)
		return 0; 



#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif 

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef RTMP_MAC_PCI
		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_CLOSE);
#endif 

		
		
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        {
		    AsicForceWakeup(pAd, TRUE);
        }


		MlmeRadioOff(pAd);
#ifdef RTMP_MAC_PCI
		pAd->bPCIclkOff = FALSE;
#endif 
	}
#endif 

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	for (i = 0 ; i < NUM_OF_TX_RING; i++)
	{
		while (pAd->DeQueueRunning[i] == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Waiting for TxQueue[%d] done..........\n", i));
			RTMPusecDelay(1000);
		}
	}



	
	MlmeHalt(pAd);

	
	RtmpNetTaskExit(pAd);


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		MacTableReset(pAd);
	}
#endif 


	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);


	
	RtmpMgmtTaskExit(pAd);

#ifdef RTMP_MAC_PCI
	{
			BOOLEAN brc;
			

			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
			{
				RTMP_ASIC_INTERRUPT_DISABLE(pAd);
			}

			
			
			
			


			brc=RT28xxPciAsicRadioOff(pAd, RTMP_HALT, 0);


			pAd->bPCIclkOff = FALSE;

			if (brc==FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s call RT28xxPciAsicRadioOff fail !!\n", __FUNCTION__));
			}
	}



#endif 

	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
#ifdef RTMP_MAC_PCI
		
		RTMP_IRQ_RELEASE(net_dev)
#endif 
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
	}

	
	RTMPFreeTxRxRingMemory(pAd);

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef DOT11_N_SUPPORT
	
	ba_reordering_resource_release(pAd);
#endif 

#ifdef CONFIG_STA_SUPPORT
#endif 

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
	}
#endif 

	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt28xx_close\n"));
	return 0; 
} 



int rt28xx_open(IN PNET_DEV dev)
{
	struct net_device * net_dev = (struct net_device *)dev;
	PRTMP_ADAPTER pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);
	int retval = 0;
	


	
	if (pAd == NULL)
	{
		
		return -1;
	}

#ifdef CONFIG_APSTA_MIXED_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		CW_MAX_IN_BITS = 6;
	}
	else if (pAd->OpMode == OPMODE_STA)
	{
		CW_MAX_IN_BITS = 10;
	}
#endif 

#if WIRELESS_EXT >= 12
	if (net_dev->priv_flags == INT_MAIN)
	{
#ifdef CONFIG_APSTA_MIXED_SUPPORT
		if (pAd->OpMode == OPMODE_AP)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_ap_iw_handler_def;
#endif 
#ifdef CONFIG_STA_SUPPORT
		if (pAd->OpMode == OPMODE_STA)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_iw_handler_def;
#endif 
	}
#endif 

	
	
	RTMP_IRQ_REQUEST(net_dev);

	
	RTMP_IRQ_INIT(pAd);

	
	if (rt28xx_init(pAd, mac, hostname) == FALSE)
		goto err;

#ifdef CONFIG_STA_SUPPORT
#endif 

	
	RTMP_IRQ_ENABLE(pAd);

	
	RTMPEnableRxTx(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);

	{
	UINT32 reg = 0;
	RTMP_IO_READ32(pAd, 0x1300, &reg);  
	printk("0x1300 = %08x\n", reg);
	}

	{










	}


#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_PCI
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
        RTMPInitPCIeLinkCtrlValue(pAd);
#endif 
#endif 

	return (retval);

err:

	RTMP_IRQ_RELEASE(net_dev);

	return (-1);
} 

static const struct net_device_ops rt3090_netdev_ops = {
	.ndo_open		= MainVirtualIF_open,
	.ndo_stop		= MainVirtualIF_close,
	.ndo_do_ioctl		= rt28xx_ioctl,
	.ndo_get_stats		= RT28xx_get_ether_stats,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_change_mtu		= eth_change_mtu,
#ifdef IKANOS_VX_1X0
	.ndo_start_xmit		= IKANOS_DataFramesTx,
#else
	.ndo_start_xmit		= rt28xx_send_packets,
#endif
};

PNET_DEV RtmpPhyNetDevInit(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_OS_NETDEV_OP_HOOK *pNetDevHook)
{
	struct net_device	*net_dev = NULL;


	net_dev = RtmpOSNetDevCreate(pAd, INT_MAIN, 0, sizeof(PRTMP_ADAPTER), INF_MAIN_DEV_NAME);
	if (net_dev == NULL)
	{
		printk("RtmpPhyNetDevInit(): creation failed for main physical net device!\n");
		return NULL;
	}

	NdisZeroMemory((unsigned char *)pNetDevHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	pNetDevHook->netdev_ops = &rt3090_netdev_ops;
	pNetDevHook->priv_flags = INT_MAIN;
	pNetDevHook->needProtcted = FALSE;

	RTMP_OS_NETDEV_SET_PRIV(net_dev, pAd);
	
	pAd->net_dev = net_dev;



#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
	SET_MODULE_OWNER(net_dev);
#endif

	netif_stop_queue(net_dev);

	return net_dev;

}



int rt28xx_packet_xmit(struct sk_buff *skb)
{
	struct net_device *net_dev = skb->dev;
	PRTMP_ADAPTER pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);
	int status = 0;
	PNDIS_PACKET pPacket = (PNDIS_PACKET) skb;

	
#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_RESOURCES);
		return 0;
	}
#endif 

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		
		if (MONITOR_ON(pAd))
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			goto done;
		}
	}
#endif 

        
	if (skb->len < 14)
	{
		
		hex_dump("bad packet", skb->data, skb->len);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		goto done;
	}



	RTMP_SET_PACKET_5VT(pPacket, 0);

#ifdef CONFIG_5VT_ENHANCE
    if (*(int*)(skb->cb) == BRIDGE_TAG) {
		RTMP_SET_PACKET_5VT(pPacket, 1);
    }
#endif



#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{

		STASendPackets((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1);
	}

#endif 

	status = 0;
done:

	return status;
}



static int rt28xx_send_packets(
	IN struct sk_buff		*skb_p,
	IN struct net_device	*net_dev)
{
	RTMP_ADAPTER *pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);

	if (!(net_dev->flags & IFF_UP))
	{
		RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)skb_p, NDIS_STATUS_FAILURE);
		return 0;
	}

	NdisZeroMemory((PUCHAR)&skb_p->cb[CB_OFF], 15);
	RTMP_SET_PACKET_NET_DEVICE_MBSSID(skb_p, MAIN_MBSSID);

	return rt28xx_packet_xmit(skb_p);
}


#if WIRELESS_EXT >= 12

struct iw_statistics *rt28xx_get_wireless_stats(
    IN struct net_device *net_dev)
{
	PRTMP_ADAPTER pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);



	DBGPRINT(RT_DEBUG_TRACE, ("rt28xx_get_wireless_stats --->\n"));

	pAd->iw_stats.status = 0; 

	
#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
	pAd->iw_stats.qual.qual = ((pAd->Mlme.ChannelQuality * 12)/10 + 10);
#endif 

	if(pAd->iw_stats.qual.qual > 100)
		pAd->iw_stats.qual.qual = 100;

#ifdef CONFIG_STA_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
	{
		pAd->iw_stats.qual.level =
			RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi0,
							pAd->StaCfg.RssiSample.LastRssi1,
							pAd->StaCfg.RssiSample.LastRssi2);
	}
#endif 

	pAd->iw_stats.qual.noise = pAd->BbpWriteLatch[66]; 

	pAd->iw_stats.qual.noise += 256 - 143;
	pAd->iw_stats.qual.updated = 1;     
#ifdef IW_QUAL_DBM
	pAd->iw_stats.qual.updated |= IW_QUAL_DBM;	
#endif 

	pAd->iw_stats.discard.nwid = 0;     
	pAd->iw_stats.miss.beacon = 0;      

	DBGPRINT(RT_DEBUG_TRACE, ("<--- rt28xx_get_wireless_stats\n"));
	return &pAd->iw_stats;
}
#endif 


void tbtt_tasklet(unsigned long data)
{


}

INT rt28xx_ioctl(
	IN	PNET_DEV	net_dev,
	IN	OUT	struct ifreq	*rq,
	IN	INT					cmd)
{
	RTMP_ADAPTER	*pAd = NULL;
	INT				ret = 0;

	pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);
	if (pAd == NULL)
	{
		
		return -ENETDOWN;
	}


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		ret = rt28xx_sta_ioctl(net_dev, rq, cmd);
	}
#endif 

	return ret;
}



static struct net_device_stats *RT28xx_get_ether_stats(
    IN  struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = NULL;

	if (net_dev)
		pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);

	if (pAd)
	{

		pAd->stats.rx_packets = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
		pAd->stats.tx_packets = pAd->WlanCounters.TransmittedFragmentCount.QuadPart;

		pAd->stats.rx_bytes = pAd->RalinkCounters.ReceivedByteCount;
		pAd->stats.tx_bytes = pAd->RalinkCounters.TransmittedByteCount;

		pAd->stats.rx_errors = pAd->Counters8023.RxErrors;
		pAd->stats.tx_errors = pAd->Counters8023.TxErrors;

		pAd->stats.rx_dropped = 0;
		pAd->stats.tx_dropped = 0;

	    pAd->stats.multicast = pAd->WlanCounters.MulticastReceivedFrameCount.QuadPart;   
	    pAd->stats.collisions = pAd->Counters8023.OneCollision + pAd->Counters8023.MoreCollisions;  

	    pAd->stats.rx_length_errors = 0;
	    pAd->stats.rx_over_errors = pAd->Counters8023.RxNoBuffer;                   
	    pAd->stats.rx_crc_errors = 0;
	    pAd->stats.rx_frame_errors = pAd->Counters8023.RcvAlignmentErrors;          
	    pAd->stats.rx_fifo_errors = pAd->Counters8023.RxNoBuffer;                   
	    pAd->stats.rx_missed_errors = 0;                                            

	    
	    pAd->stats.tx_aborted_errors = 0;
	    pAd->stats.tx_carrier_errors = 0;
	    pAd->stats.tx_fifo_errors = 0;
	    pAd->stats.tx_heartbeat_errors = 0;
	    pAd->stats.tx_window_errors = 0;

	    
	    pAd->stats.rx_compressed = 0;
	    pAd->stats.tx_compressed = 0;

		return &pAd->stats;
	}
	else
	return NULL;
}


BOOLEAN RtmpPhyNetDevExit(
	IN RTMP_ADAPTER *pAd,
	IN PNET_DEV net_dev)
{



#ifdef INF_AMAZON_PPA
	if (ppa_hook_directpath_register_dev_fn && pAd->PPAEnable==TRUE)
	{
		UINT status;
		status=ppa_hook_directpath_register_dev_fn(&pAd->g_if_id, pAd->net_dev, NULL, PPA_F_DIRECTPATH_DEREGISTER);
		printk("unregister PPA:g_if_id=%d status=%d\n",pAd->g_if_id,status);
	}
	kfree(pAd->pDirectpathCb);
#endif 

	
	if (net_dev != NULL)
	{
		printk("RtmpOSNetDevDetach(): RtmpOSNetDeviceDetach(), dev->name=%s!\n", net_dev->name);
		RtmpOSNetDevDetach(net_dev);
	}

	return TRUE;

}



NDIS_STATUS AdapterBlockAllocateMemory(
	IN PVOID	handle,
	OUT	PVOID	*ppAd)
{

	*ppAd = (PVOID)vmalloc(sizeof(RTMP_ADAPTER)); 

	if (*ppAd)
	{
		NdisZeroMemory(*ppAd, sizeof(RTMP_ADAPTER));
		((PRTMP_ADAPTER)*ppAd)->OS_Cookie = handle;
		return (NDIS_STATUS_SUCCESS);
	} else {
		return (NDIS_STATUS_FAILURE);
	}
}
