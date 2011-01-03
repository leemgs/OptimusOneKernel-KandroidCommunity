

#include "rt_config.h"

#define FORTY_MHZ_INTOLERANT_INTERVAL	(60*1000) 






char *mac = "";		   
char *hostname = "";		   
module_param (mac, charp, 0);
MODULE_PARM_DESC (mac, "rt28xx: wireless mac addr");





extern BOOLEAN ba_reordering_resource_init(PRTMP_ADAPTER pAd, int num);
extern void ba_reordering_resource_release(PRTMP_ADAPTER pAd);
extern NDIS_STATUS NICLoadRateSwitchingParams(IN PRTMP_ADAPTER pAd);

#ifdef RT2860
extern void init_thread_task(PRTMP_ADAPTER pAd);
#endif


INT __devinit rt28xx_probe(IN void *_dev_p, IN void *_dev_id_p,
							IN UINT argc, OUT PRTMP_ADAPTER *ppAd);


static int rt28xx_init(IN struct net_device *net_dev);
INT rt28xx_send_packets(IN struct sk_buff *skb_p, IN struct net_device *net_dev);

static void CfgInitHook(PRTMP_ADAPTER pAd);

extern	const struct iw_handler_def rt28xx_iw_handler_def;


struct iw_statistics *rt28xx_get_wireless_stats(
    IN struct net_device *net_dev);

struct net_device_stats *RT28xx_get_ether_stats(
    IN  struct net_device *net_dev);


int MainVirtualIF_close(IN struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = net_dev->ml_priv;

	
	if (pAd == NULL)
		return 0; 

	netif_carrier_off(pAd->net_dev);
	netif_stop_queue(pAd->net_dev);


	VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();

	return 0; 
}


int MainVirtualIF_open(IN struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = net_dev->ml_priv;

	
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
    RTMP_ADAPTER	*pAd = net_dev->ml_priv;
	BOOLEAN 		Cancelled = FALSE;
	UINT32			i = 0;
#ifdef RT2870
	DECLARE_WAIT_QUEUE_HEAD_ONSTACK(unlink_wakeup);
	DECLARE_WAITQUEUE(wait, current);

	
#endif 


    DBGPRINT(RT_DEBUG_TRACE, ("===> rt28xx_close\n"));

	
	if (pAd == NULL)
		return 0; 

	{
		
		
#ifdef RT2860
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) ||
			RTMP_SET_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND) ||
			RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
#endif
#ifdef RT2870
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
#endif
        {
#ifdef RT2860
		    AsicForceWakeup(pAd, RTMP_HALT);
#endif
#ifdef RT2870
		    AsicForceWakeup(pAd, TRUE);
#endif
        }

		if (INFRA_ON(pAd) &&
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			MLME_DISASSOC_REQ_STRUCT	DisReq;
			MLME_QUEUE_ELEM *MsgElem = (MLME_QUEUE_ELEM *) kmalloc(sizeof(MLME_QUEUE_ELEM), MEM_ALLOC_FLAG);

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

			RTMPusecDelay(1000);
		}

#ifdef RT2870
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);
#endif 

#ifdef CCX_SUPPORT
		RTMPCancelTimer(&pAd->StaCfg.LeapAuthTimer, &Cancelled);
#endif

		RTMPCancelTimer(&pAd->StaCfg.StaQuickResponeForRateUpTimer, &Cancelled);
		RTMPCancelTimer(&pAd->StaCfg.WpaDisassocAndBlockAssocTimer, &Cancelled);

		MlmeRadioOff(pAd);
#ifdef RT2860
		pAd->bPCIclkOff = FALSE;
#endif
	}

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	for (i = 0 ; i < NUM_OF_TX_RING; i++)
	{
		while (pAd->DeQueueRunning[i] == TRUE)
		{
			printk("Waiting for TxQueue[%d] done..........\n", i);
			RTMPusecDelay(1000);
		}
	}

#ifdef RT2870
	
	add_wait_queue (&unlink_wakeup, &wait);
	pAd->wait = &unlink_wakeup;

	
	i = 0;
	
	while(i < 25)
	{
		unsigned long IrqFlags;

		RTMP_IRQ_LOCK(&pAd->BulkInLock, IrqFlags);
		if (pAd->PendingRx == 0)
		{
			RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);
			break;
		}
		RTMP_IRQ_UNLOCK(&pAd->BulkInLock, IrqFlags);

		msleep(UNLINK_TIMEOUT_MS);	
		i++;
	}
	pAd->wait = NULL;
	remove_wait_queue (&unlink_wakeup, &wait);
#endif 

#ifdef RT2870
	
	RT2870_TimerQ_Exit(pAd);
	
	RT28xxThreadTerminate(pAd);
#endif 

	
	MlmeHalt(pAd);

	
	kill_thread_task(pAd);

	MacTableReset(pAd);

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);

#ifdef RT2860
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		NICDisableInterrupt(pAd);
	}

	
	NICIssueReset(pAd);

	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		
		RT28XX_IRQ_RELEASE(net_dev)
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
	}
#endif

	
	RTMPFreeTxRxRingMemory(pAd);

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	
	ba_reordering_resource_release(pAd);

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);

	return 0; 
} 

static int rt28xx_init(IN struct net_device *net_dev)
{
#ifdef RT2860
	PRTMP_ADAPTER 			pAd = (PRTMP_ADAPTER)net_dev->ml_priv;
#endif
#ifdef RT2870
	PRTMP_ADAPTER 			pAd = net_dev->ml_priv;
#endif
	UINT					index;
	UCHAR					TmpPhy;
	NDIS_STATUS				Status;
	UINT32 		MacCsr0 = 0;

	
	ba_reordering_resource_init(pAd, MAX_REORDERING_MPDU_NUM);

	
	index = 0;
	do
	{
		RTMP_IO_READ32(pAd, MAC_CSR0, &MacCsr0);
		pAd->MACVersion = MacCsr0;

		if ((pAd->MACVersion != 0x00) && (pAd->MACVersion != 0xFFFFFFFF))
			break;

		RTMPusecDelay(10);
	} while (index++ < 100);

	DBGPRINT(RT_DEBUG_TRACE, ("MAC_CSR0  [ Ver:Rev=0x%08x]\n", pAd->MACVersion));


	
	RT28XXDMADisable(pAd);

	
	Status = NICLoadFirmware(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICLoadFirmware failed, Status[=0x%08x]\n", Status));
		goto err1;
	}

	NICLoadRateSwitchingParams(pAd);

	
	
#ifdef RT2860
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		NICDisableInterrupt(pAd);
	}
#endif

	Status = RTMPAllocTxRxRingMemory(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("RTMPAllocDMAMemory failed, Status[=0x%08x]\n", Status));
		goto err1;
	}

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);

	
	

	Status = MlmeInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("MlmeInit failed, Status[=0x%08x]\n", Status));
		goto err2;
	}

	
	
	UserCfgInit(pAd);

#ifdef RT2870
	
	RT2870_TimerQ_Init(pAd);
#endif 

	RT28XX_TASK_THREAD_INIT(pAd, Status);
	if (Status != NDIS_STATUS_SUCCESS)
		goto err1;

	CfgInitHook(pAd);

	NdisAllocateSpinLock(&pAd->MacTabLock);

	MeasureReqTabInit(pAd);
	TpcReqTabInit(pAd);

	
	
	
	Status = NICInitializeAdapter(pAd, TRUE);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICInitializeAdapter failed, Status[=0x%08x]\n", Status));
		if (Status != NDIS_STATUS_SUCCESS)
		goto err3;
	}

	
	Status = RTMPReadParametersHook(pAd);

	printk("1. Phy Mode = %d\n", pAd->CommonCfg.PhyMode);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICReadRegParameters failed, Status[=0x%08x]\n",Status));
		goto err4;
	}

#ifdef RT2870
	pAd->CommonCfg.bMultipleIRP = FALSE;

	if (pAd->CommonCfg.bMultipleIRP)
		pAd->CommonCfg.NumOfBulkInIRP = RX_RING_SIZE;
	else
		pAd->CommonCfg.NumOfBulkInIRP = 1;
#endif 


   	
	pAd->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	pAd->CommonCfg.DesiredHtPhy.AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	pAd->CommonCfg.DesiredHtPhy.AmsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.DesiredHtPhy.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	
	pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;

	printk("2. Phy Mode = %d\n", pAd->CommonCfg.PhyMode);

	
	NICReadEEPROMParameters(pAd, mac);

	printk("3. Phy Mode = %d\n", pAd->CommonCfg.PhyMode);

	NICInitAsicFromEEPROM(pAd); 

	
	TmpPhy = pAd->CommonCfg.PhyMode;
	pAd->CommonCfg.PhyMode = 0xff;
	RTMPSetPhyMode(pAd, TmpPhy);
	SetCommonHT(pAd);

	
	if (pAd->ChannelListNum == 0)
	{
		printk("Wrong configuration. No valid channel found. Check \"ContryCode\" and \"ChannelGeography\" setting.\n");
		goto err4;
	}

	printk("MCS Set = %02x %02x %02x %02x %02x\n", pAd->CommonCfg.HtCapability.MCSSet[0],
           pAd->CommonCfg.HtCapability.MCSSet[1], pAd->CommonCfg.HtCapability.MCSSet[2],
           pAd->CommonCfg.HtCapability.MCSSet[3], pAd->CommonCfg.HtCapability.MCSSet[4]);

#ifdef RT2870
    
	NICInitRT30xxRFRegisters(pAd);
#endif 


		
	
	
	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);

#ifndef RT2870
	
	AsicSendCommandToMcu(pAd, 0x72, 0xFF, 0x00, 0x00);
#endif

	if (pAd && (Status != NDIS_STATUS_SUCCESS))
	{
		
		
		
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		{
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
		}
	}
	else if (pAd)
	{
		
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MEDIA_STATE_CHANGE);

		DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event B!\n"));


#ifdef RT2870
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS);
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS);

		
		
		
		
		for(index=0; index<pAd->CommonCfg.NumOfBulkInIRP; index++)
		{
			RTUSBBulkReceive(pAd);
			DBGPRINT(RT_DEBUG_TRACE, ("RTUSBBulkReceive!\n" ));
		}
#endif 
	}


	DBGPRINT_S(Status, ("<==== RTMPInitialize, Status=%x\n", Status));

	return TRUE;


err4:
err3:
	MlmeHalt(pAd);
err2:
	RTMPFreeTxRxRingMemory(pAd);
err1:
	os_free_mem(pAd, pAd->mpdu_blk_pool.mem); 
	RT28XX_IRQ_RELEASE(net_dev);

	
	

	printk("!!! %s Initialized fail !!!\n", RT28xx_CHIP_NAME);
	return FALSE;
} 



int rt28xx_open(IN PNET_DEV dev)
{
	struct net_device * net_dev = (struct net_device *)dev;
	PRTMP_ADAPTER pAd = net_dev->ml_priv;
	int retval = 0;
 	POS_COOKIE pObj;


	
	if (pAd == NULL)
	{
		
		return -1;
	}

	
 	pObj = (POS_COOKIE)pAd->OS_Cookie;

	
	RTMP_CLEAR_FLAGS(pAd);

	
	
	RT28XX_IRQ_REQUEST(net_dev);


	


	
	if (rt28xx_init(net_dev) == FALSE)
		goto err;

	NdisZeroMemory(pAd->StaCfg.dev_name, 16);
	NdisMoveMemory(pAd->StaCfg.dev_name, net_dev->name, strlen(net_dev->name));

	
	NdisMoveMemory(net_dev->dev_addr, (void *) pAd->CurrentAddress, 6);

	
	RT28XX_IRQ_INIT(pAd);

	

	
	RT28XX_IRQ_ENABLE(pAd);

	
	RTMPEnableRxTx(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);

	{
	UINT32 reg = 0;
	RTMP_IO_READ32(pAd, 0x1300, &reg);  
	printk("0x1300 = %08x\n", reg);
	}

#ifdef RT2860
        RTMPInitPCIeLinkCtrlValue(pAd);
#endif
	return (retval);

err:
	return (-1);
} 

static const struct net_device_ops rt2860_netdev_ops = {
	.ndo_open		= MainVirtualIF_open,
	.ndo_stop		= MainVirtualIF_close,
	.ndo_do_ioctl		= rt28xx_sta_ioctl,
	.ndo_get_stats		= RT28xx_get_ether_stats,
	.ndo_validate_addr	= NULL,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_start_xmit		= rt28xx_send_packets,
};


static NDIS_STATUS rt_ieee80211_if_setup(struct net_device *dev, PRTMP_ADAPTER pAd)
{
	NDIS_STATUS Status;
	INT     i=0;
	CHAR    slot_name[IFNAMSIZ];
	struct net_device   *device;

	if (pAd->OpMode == OPMODE_STA)
	{
		dev->wireless_handlers = &rt28xx_iw_handler_def;
	}

	dev->priv_flags = INT_MAIN;
	dev->netdev_ops = &rt2860_netdev_ops;
	
	for (i = 0; i < 8; i++)
	{
		sprintf(slot_name, "wlan%d", i);

		device = dev_get_by_name(dev_net(dev), slot_name);
		if (device != NULL)
			dev_put(device);

		if (device == NULL)
			break;
	}

	if(i == 8)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("No available slot name\n"));
		Status = NDIS_STATUS_FAILURE;
	}
	else
	{
		sprintf(dev->name, "wlan%d", i);
		Status = NDIS_STATUS_SUCCESS;
	}

	return Status;

}


INT __devinit   rt28xx_probe(
    IN  void *_dev_p,
    IN  void *_dev_id_p,
	IN  UINT argc,
	OUT PRTMP_ADAPTER *ppAd)
{
    struct  net_device	*net_dev;
    PRTMP_ADAPTER       pAd = (PRTMP_ADAPTER) NULL;
    INT                 status;
	PVOID				handle;
#ifdef RT2860
	struct pci_dev *dev_p = (struct pci_dev *)_dev_p;
#endif
#ifdef RT2870
	struct usb_interface *intf = (struct usb_interface *)_dev_p;
	struct usb_device *dev_p = interface_to_usbdev(intf);

	dev_p = usb_get_dev(dev_p);
#endif 

    DBGPRINT(RT_DEBUG_TRACE, ("STA Driver version-%s\n", STA_DRIVER_VERSION));

    net_dev = alloc_etherdev(sizeof(PRTMP_ADAPTER));
    if (net_dev == NULL)
    {
        printk("alloc_netdev failed\n");

        goto err_out;
    }

	netif_stop_queue(net_dev);



    SET_NETDEV_DEV(net_dev, &(dev_p->dev));

	
	handle = kmalloc(sizeof(struct os_cookie), GFP_KERNEL);
	if (handle == NULL)
		goto err_out_free_netdev;;
	RT28XX_HANDLE_DEV_ASSIGN(handle, dev_p);

	status = RTMPAllocAdapterBlock(handle, &pAd);
	if (status != NDIS_STATUS_SUCCESS)
		goto err_out_free_netdev;

	net_dev->ml_priv = (PVOID)pAd;
    pAd->net_dev = net_dev; 

	RT28XXNetDevInit(_dev_p, net_dev, pAd);

    pAd->StaCfg.OriDevType = net_dev->type;

	
	if (RT28XXProbePostConfig(_dev_p, pAd, 0) == FALSE)
		goto err_out_unmap;

	pAd->OpMode = OPMODE_STA;

	
	if (rt_ieee80211_if_setup(net_dev, pAd) != NDIS_STATUS_SUCCESS)
		goto err_out_unmap;

    
    status = register_netdev(net_dev);
    if (status)
        goto err_out_unmap;

    
	RT28XX_DRVDATA_SET(_dev_p);

	*ppAd = pAd;
    return 0; 


	
err_out_unmap:
	RTMPFreeAdapter(pAd);
	RT28XX_UNMAP();

err_out_free_netdev:
	free_netdev(net_dev);

err_out:
	RT28XX_PUT_DEVICE(dev_p);

	return -ENODEV; 
} 



int rt28xx_packet_xmit(struct sk_buff *skb)
{
	struct net_device *net_dev = skb->dev;
	PRTMP_ADAPTER pAd = net_dev->ml_priv;
	int status = NETDEV_TX_OK;
	PNDIS_PACKET pPacket = (PNDIS_PACKET) skb;

	{
		
		if (MONITOR_ON(pAd))
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			goto done;
		}
	}

        
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

	STASendPackets((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1);

	status = NETDEV_TX_OK;
done:

	return status;
}



INT rt28xx_send_packets(
	IN struct sk_buff 		*skb_p,
	IN struct net_device 	*net_dev)
{
    RTMP_ADAPTER *pAd = net_dev->ml_priv;
	if (!(net_dev->flags & IFF_UP))
	{
		RELEASE_NDIS_PACKET(pAd, (PNDIS_PACKET)skb_p, NDIS_STATUS_FAILURE);
		return NETDEV_TX_OK;
	}

	NdisZeroMemory((PUCHAR)&skb_p->cb[CB_OFF], 15);
	RTMP_SET_PACKET_NET_DEVICE_MBSSID(skb_p, MAIN_MBSSID);

	return rt28xx_packet_xmit(skb_p);

} 




void CfgInitHook(PRTMP_ADAPTER pAd)
{
	pAd->bBroadComHT = TRUE;
} 



struct iw_statistics *rt28xx_get_wireless_stats(
    IN struct net_device *net_dev)
{
	PRTMP_ADAPTER pAd = net_dev->ml_priv;


	DBGPRINT(RT_DEBUG_TRACE, ("rt28xx_get_wireless_stats --->\n"));

	pAd->iw_stats.status = 0; 

	
	pAd->iw_stats.qual.qual = ((pAd->Mlme.ChannelQuality * 12)/10 + 10);
	if(pAd->iw_stats.qual.qual > 100)
		pAd->iw_stats.qual.qual = 100;

	if (pAd->OpMode == OPMODE_STA)
		pAd->iw_stats.qual.level = RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.LastRssi0, pAd->StaCfg.RssiSample.LastRssi1, pAd->StaCfg.RssiSample.LastRssi2);

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



void tbtt_tasklet(unsigned long data)
{
#define MAX_TX_IN_TBTT		(16)

}


struct net_device_stats *RT28xx_get_ether_stats(
    IN  struct net_device *net_dev)
{
    RTMP_ADAPTER *pAd = NULL;

	if (net_dev)
		pAd = net_dev->ml_priv;

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

