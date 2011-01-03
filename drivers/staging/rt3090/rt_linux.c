

#include <linux/sched.h>
#include "rt_config.h"

ULONG	RTDebugLevel = RT_DEBUG_ERROR;



char const *pWirelessSysEventText[IW_SYS_EVENT_TYPE_NUM] = {
	
    "had associated successfully",							
    "had disassociated",									
    "had deauthenticated",									
    "had been aged-out and disassociated",					
    "occurred CounterMeasures attack",						
    "occurred replay counter different in Key Handshaking",	
    "occurred RSNIE different in Key Handshaking",			
    "occurred MIC different in Key Handshaking",			
    "occurred ICV error in RX",								
    "occurred MIC error in RX",								
	"Group Key Handshaking timeout",						
	"Pairwise Key Handshaking timeout",						
	"RSN IE sanity check failure",							
	"set key done in WPA/WPAPSK",							
	"set key done in WPA2/WPA2PSK",                         
	"connects with our wireless client",                    
	"disconnects with our wireless client",                 
	"scan completed"										
	"scan terminate!! Busy!! Enqueue fail!!"				
	};


char const *pWirelessSpoofEventText[IW_SPOOF_EVENT_TYPE_NUM] = {
    "detected conflict SSID",								
    "detected spoofed association response",				
    "detected spoofed reassociation responses",				
    "detected spoofed probe response",						
    "detected spoofed beacon",								
    "detected spoofed disassociation",						
    "detected spoofed authentication",						
    "detected spoofed deauthentication",					
    "detected spoofed unknown management frame",			
	"detected replay attack"								
	};


char const *pWirelessFloodEventText[IW_FLOOD_EVENT_TYPE_NUM] = {
	"detected authentication flooding",						
    "detected association request flooding",				
    "detected reassociation request flooding",				
    "detected probe request flooding",						
    "detected disassociation flooding",						
    "detected deauthentication flooding",					
    "detected 802.1x eap-request flooding"					
	};



VOID RTMP_SetPeriodicTimer(
	IN	NDIS_MINIPORT_TIMER *pTimer,
	IN	unsigned long timeout)
{
	timeout = ((timeout*OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}


VOID RTMP_OS_Init_Timer(
	IN	PRTMP_ADAPTER pAd,
	IN	NDIS_MINIPORT_TIMER *pTimer,
	IN	TIMER_FUNCTION function,
	IN	PVOID data)
{
	init_timer(pTimer);
    pTimer->data = (unsigned long)data;
    pTimer->function = function;
}


VOID RTMP_OS_Add_Timer(
	IN	NDIS_MINIPORT_TIMER		*pTimer,
	IN	unsigned long timeout)
{
	if (timer_pending(pTimer))
		return;

	timeout = ((timeout*OS_HZ) / 1000);
	pTimer->expires = jiffies + timeout;
	add_timer(pTimer);
}

VOID RTMP_OS_Mod_Timer(
	IN	NDIS_MINIPORT_TIMER		*pTimer,
	IN	unsigned long timeout)
{
	timeout = ((timeout*OS_HZ) / 1000);
	mod_timer(pTimer, jiffies + timeout);
}

VOID RTMP_OS_Del_Timer(
	IN	NDIS_MINIPORT_TIMER		*pTimer,
	OUT	BOOLEAN					*pCancelled)
{
	if (timer_pending(pTimer))
	{
		*pCancelled = del_timer_sync(pTimer);
	}
	else
	{
		*pCancelled = TRUE;
	}

}

VOID RTMP_OS_Release_Packet(
	IN	PRTMP_ADAPTER pAd,
	IN	PQUEUE_ENTRY  pEntry)
{
	
}


VOID RTMPusecDelay(
	IN	ULONG	usec)
{
	ULONG	i;

	for (i = 0; i < (usec / 50); i++)
		udelay(50);

	if (usec % 50)
		udelay(usec % 50);
}

void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time)
{
	time->u.LowPart = jiffies;
}


NDIS_STATUS os_alloc_mem(
	IN	RTMP_ADAPTER *pAd,
	OUT	UCHAR **mem,
	IN	ULONG  size)
{
	*mem = (PUCHAR) kmalloc(size, GFP_ATOMIC);
	if (*mem)
		return (NDIS_STATUS_SUCCESS);
	else
		return (NDIS_STATUS_FAILURE);
}


NDIS_STATUS os_free_mem(
	IN	PRTMP_ADAPTER pAd,
	IN	PVOID mem)
{

	ASSERT(mem);
	kfree(mem);
	return (NDIS_STATUS_SUCCESS);
}




PNDIS_PACKET RtmpOSNetPktAlloc(
	IN RTMP_ADAPTER *pAd,
	IN int size)
{
	struct sk_buff *skb;
	
	skb = dev_alloc_skb(size+2);

	return ((PNDIS_PACKET)skb);
}


PNDIS_PACKET RTMP_AllocateFragPacketBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length)
{
	struct sk_buff *pkt;

	pkt = dev_alloc_skb(Length);

	if (pkt == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("can't allocate frag rx %ld size packet\n",Length));
	}

	if (pkt)
	{
		RTMP_SET_PACKET_SOURCE(OSPKT_TO_RTPKT(pkt), PKTSRC_NDIS);
	}

	return (PNDIS_PACKET) pkt;
}


PNDIS_PACKET RTMP_AllocateTxPacketBuffer(
	IN	PRTMP_ADAPTER pAd,
	IN	ULONG	Length,
	IN	BOOLEAN	Cached,
	OUT	PVOID	*VirtualAddress)
{
	struct sk_buff *pkt;

	pkt = dev_alloc_skb(Length);

	if (pkt == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("can't allocate tx %ld size packet\n",Length));
	}

	if (pkt)
	{
		RTMP_SET_PACKET_SOURCE(OSPKT_TO_RTPKT(pkt), PKTSRC_NDIS);
		*VirtualAddress = (PVOID) pkt->data;
	}
	else
	{
		*VirtualAddress = (PVOID) NULL;
	}

	return (PNDIS_PACKET) pkt;
}


VOID build_tx_packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	PUCHAR	pFrame,
	IN	ULONG	FrameLen)
{

	struct sk_buff	*pTxPkt;

	ASSERT(pPacket);
	pTxPkt = RTPKT_TO_OSPKT(pPacket);

	NdisMoveMemory(skb_put(pTxPkt, FrameLen), pFrame, FrameLen);
}

VOID	RTMPFreeAdapter(
	IN	PRTMP_ADAPTER	pAd)
{
	POS_COOKIE os_cookie;
	int index;

	os_cookie=(POS_COOKIE)pAd->OS_Cookie;

	if (pAd->BeaconBuf)
		kfree(pAd->BeaconBuf);


	NdisFreeSpinLock(&pAd->MgmtRingLock);

#ifdef RTMP_MAC_PCI
	NdisFreeSpinLock(&pAd->RxRingLock);
#ifdef RT3090
NdisFreeSpinLock(&pAd->McuCmdLock);
#endif 
#endif 

	for (index =0 ; index < NUM_OF_TX_RING; index++)
	{
		NdisFreeSpinLock(&pAd->TxSwQueueLock[index]);
		NdisFreeSpinLock(&pAd->DeQueueLock[index]);
		pAd->DeQueueRunning[index] = FALSE;
	}

	NdisFreeSpinLock(&pAd->irq_lock);


	vfree(pAd); 
	if (os_cookie)
		kfree(os_cookie);
}

BOOLEAN OS_Need_Clone_Packet(void)
{
	return (FALSE);
}




NDIS_STATUS RTMPCloneNdisPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	BOOLEAN			pInsAMSDUHdr,
	IN	PNDIS_PACKET	pInPacket,
	OUT PNDIS_PACKET   *ppOutPacket)
{

	struct sk_buff *pkt;

	ASSERT(pInPacket);
	ASSERT(ppOutPacket);

	
	pkt = dev_alloc_skb(2048);

	if (pkt == NULL)
	{
		return NDIS_STATUS_FAILURE;
	}

	skb_put(pkt, GET_OS_PKT_LEN(pInPacket));
	NdisMoveMemory(pkt->data, GET_OS_PKT_DATAPTR(pInPacket), GET_OS_PKT_LEN(pInPacket));
	*ppOutPacket = OSPKT_TO_RTPKT(pkt);


	RTMP_SET_PACKET_SOURCE(OSPKT_TO_RTPKT(pkt), PKTSRC_NDIS);

	printk("###Clone###\n");

	return NDIS_STATUS_SUCCESS;
}



NDIS_STATUS RTMPAllocateNdisPacket(
	IN	PRTMP_ADAPTER	pAd,
	OUT PNDIS_PACKET   *ppPacket,
	IN	PUCHAR			pHeader,
	IN	UINT			HeaderLen,
	IN	PUCHAR			pData,
	IN	UINT			DataLen)
{
	PNDIS_PACKET	pPacket;
	ASSERT(pData);
	ASSERT(DataLen);

	
	pPacket = (PNDIS_PACKET *) dev_alloc_skb(HeaderLen + DataLen + RTMP_PKT_TAIL_PADDING);
	if (pPacket == NULL)
	{
		*ppPacket = NULL;
#ifdef DEBUG
		printk("RTMPAllocateNdisPacket Fail\n\n");
#endif
		return NDIS_STATUS_FAILURE;
	}

	
	if (HeaderLen > 0)
		NdisMoveMemory(GET_OS_PKT_DATAPTR(pPacket), pHeader, HeaderLen);
	if (DataLen > 0)
		NdisMoveMemory(GET_OS_PKT_DATAPTR(pPacket) + HeaderLen, pData, DataLen);

	
	skb_put(GET_OS_PKT_TYPE(pPacket), HeaderLen+DataLen);

	RTMP_SET_PACKET_SOURCE(pPacket, PKTSRC_NDIS);

	*ppPacket = pPacket;
	return NDIS_STATUS_SUCCESS;
}


VOID RTMPFreeNdisPacket(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET  pPacket)
{
	dev_kfree_skb_any(RTPKT_TO_OSPKT(pPacket));
}





NDIS_STATUS Sniff2BytesFromNdisBuffer(
	IN	PNDIS_BUFFER	pFirstBuffer,
	IN	UCHAR			DesiredOffset,
	OUT PUCHAR			pByte0,
	OUT PUCHAR			pByte1)
{
    *pByte0 = *(PUCHAR)(pFirstBuffer + DesiredOffset);
    *pByte1 = *(PUCHAR)(pFirstBuffer + DesiredOffset + 1);

	return NDIS_STATUS_SUCCESS;
}


void RTMP_QueryPacketInfo(
	IN  PNDIS_PACKET pPacket,
	OUT PACKET_INFO  *pPacketInfo,
	OUT PUCHAR		 *pSrcBufVA,
	OUT	UINT		 *pSrcBufLen)
{
	pPacketInfo->BufferCount = 1;
	pPacketInfo->pFirstBuffer = (PNDIS_BUFFER)GET_OS_PKT_DATAPTR(pPacket);
	pPacketInfo->PhysicalBufferCount = 1;
	pPacketInfo->TotalPacketLength = GET_OS_PKT_LEN(pPacket);

	*pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
	*pSrcBufLen = GET_OS_PKT_LEN(pPacket);
}

void RTMP_QueryNextPacketInfo(
	IN  PNDIS_PACKET *ppPacket,
	OUT PACKET_INFO  *pPacketInfo,
	OUT PUCHAR		 *pSrcBufVA,
	OUT	UINT		 *pSrcBufLen)
{
	PNDIS_PACKET pPacket = NULL;

	if (*ppPacket)
		pPacket = GET_OS_PKT_NEXT(*ppPacket);

	if (pPacket)
	{
		pPacketInfo->BufferCount = 1;
		pPacketInfo->pFirstBuffer = (PNDIS_BUFFER)GET_OS_PKT_DATAPTR(pPacket);
		pPacketInfo->PhysicalBufferCount = 1;
		pPacketInfo->TotalPacketLength = GET_OS_PKT_LEN(pPacket);

		*pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
		*pSrcBufLen = GET_OS_PKT_LEN(pPacket);
		*ppPacket = GET_OS_PKT_NEXT(pPacket);
	}
	else
	{
		pPacketInfo->BufferCount = 0;
		pPacketInfo->pFirstBuffer = NULL;
		pPacketInfo->PhysicalBufferCount = 0;
		pPacketInfo->TotalPacketLength = 0;

		*pSrcBufVA = NULL;
		*pSrcBufLen = 0;
		*ppPacket = NULL;
	}
}


PNDIS_PACKET DuplicatePacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			FromWhichBSSID)
{
	struct sk_buff	*skb;
	PNDIS_PACKET	pRetPacket = NULL;
	USHORT			DataSize;
	UCHAR			*pData;

	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);
	pData = (PUCHAR) GET_OS_PKT_DATAPTR(pPacket);


	skb = skb_clone(RTPKT_TO_OSPKT(pPacket), MEM_ALLOC_FLAG);
	if (skb)
	{
		skb->dev = get_netdev_from_bssid(pAd, FromWhichBSSID);
		pRetPacket = OSPKT_TO_RTPKT(skb);
	}


	return pRetPacket;

}

PNDIS_PACKET duplicate_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pHeader802_3,
    IN  UINT            HdrLen,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			FromWhichBSSID)
{
	struct sk_buff	*skb;
	PNDIS_PACKET	pPacket = NULL;


	if ((skb = __dev_alloc_skb(HdrLen + DataSize + 2, MEM_ALLOC_FLAG)) != NULL)
	{
		skb_reserve(skb, 2);
		NdisMoveMemory(skb_tail_pointer(skb), pHeader802_3, HdrLen);
		skb_put(skb, HdrLen);
		NdisMoveMemory(skb_tail_pointer(skb), pData, DataSize);
		skb_put(skb, DataSize);
		skb->dev = get_netdev_from_bssid(pAd, FromWhichBSSID);
		pPacket = OSPKT_TO_RTPKT(skb);
	}

	return pPacket;
}


#define TKIP_TX_MIC_SIZE		8
PNDIS_PACKET duplicate_pkt_with_TKIP_MIC(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket)
{
	struct sk_buff	*skb, *newskb;


	skb = RTPKT_TO_OSPKT(pPacket);
	if (skb_tailroom(skb) < TKIP_TX_MIC_SIZE)
	{
		
		newskb = skb_copy_expand(skb, skb_headroom(skb), TKIP_TX_MIC_SIZE, GFP_ATOMIC);
		dev_kfree_skb_any(skb);
		if (newskb == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Extend Tx.MIC for packet failed!, dropping packet!\n"));
			return NULL;
		}
		skb = newskb;
	}

	return OSPKT_TO_RTPKT(skb);


}




PNDIS_PACKET ClonePacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize)
{
	struct sk_buff	*pRxPkt;
	struct sk_buff	*pClonedPkt;

	ASSERT(pPacket);
	pRxPkt = RTPKT_TO_OSPKT(pPacket);

	
	pClonedPkt = skb_clone(pRxPkt, MEM_ALLOC_FLAG);

	if (pClonedPkt)
	{
	
	pClonedPkt->dev = pRxPkt->dev;
	pClonedPkt->data = pData;
	pClonedPkt->len = DataSize;
	pClonedPkt->tail = pClonedPkt->data + pClonedPkt->len;
		ASSERT(DataSize < 1530);
	}
	return pClonedPkt;
}




void  update_os_packet_info(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN  UCHAR			FromWhichBSSID)
{
	struct sk_buff	*pOSPkt;

	ASSERT(pRxBlk->pRxPacket);
	pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);

	pOSPkt->dev = get_netdev_from_bssid(pAd, FromWhichBSSID);
	pOSPkt->data = pRxBlk->pData;
	pOSPkt->len = pRxBlk->DataSize;
	pOSPkt->tail = pOSPkt->data + pOSPkt->len;
}


void wlan_802_11_to_802_3_packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	PUCHAR			pHeader802_3,
	IN  UCHAR			FromWhichBSSID)
{
	struct sk_buff	*pOSPkt;

	ASSERT(pRxBlk->pRxPacket);
	ASSERT(pHeader802_3);

	pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);

	pOSPkt->dev = get_netdev_from_bssid(pAd, FromWhichBSSID);
	pOSPkt->data = pRxBlk->pData;
	pOSPkt->len = pRxBlk->DataSize;
	pOSPkt->tail = pOSPkt->data + pOSPkt->len;

	
	
	
	

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		NdisMoveMemory(skb_push(pOSPkt, LENGTH_802_3), pHeader802_3, LENGTH_802_3);
#endif 
	}



void announce_802_3_packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket)
{

	struct sk_buff	*pRxPkt;
#ifdef INF_AMAZON_PPA
        int             ret = 0;
        unsigned int ppa_flags = 0; 
#endif 

	ASSERT(pPacket);

	pRxPkt = RTPKT_TO_OSPKT(pPacket);

#ifdef CONFIG_STA_SUPPORT
#endif 

    
#ifdef IKANOS_VX_1X0
	IKANOS_DataFrameRx(pAd, pRxPkt->dev, pRxPkt, pRxPkt->len);
#else
#ifdef INF_AMAZON_SE
#ifdef BG_FT_SUPPORT
            BG_FTPH_PacketFromApHandle(pRxPkt);
            return;
#endif 
#endif 
	pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);

#ifdef INF_AMAZON_PPA
	if (ppa_hook_directpath_send_fn && pAd->PPAEnable==TRUE )
	{
		memset(pRxPkt->head,0,pRxPkt->data-pRxPkt->head-14);
		DBGPRINT(RT_DEBUG_TRACE, ("ppa_hook_directpath_send_fn rx :ret:%d headroom:%d dev:%s pktlen:%d<===\n",ret,skb_headroom(pRxPkt)
			,pRxPkt->dev->name,pRxPkt->len));
		hex_dump("rx packet", pRxPkt->data, 32);
		ret = ppa_hook_directpath_send_fn(pAd->g_if_id, pRxPkt, pRxPkt->len, ppa_flags);
		pRxPkt=NULL;
		return;

	}
#endif 





	{
		netif_rx(pRxPkt);
	}

#endif 
}


PRTMP_SCATTER_GATHER_LIST
rt_get_sg_list_from_packet(PNDIS_PACKET pPacket, RTMP_SCATTER_GATHER_LIST *sg)
{
	sg->NumberOfElements = 1;
	sg->Elements[0].Address =  GET_OS_PKT_DATAPTR(pPacket);
	sg->Elements[0].Length = GET_OS_PKT_LEN(pPacket);
	return (sg);
}

void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
	unsigned char *pt;
	int x;

	if (RTDebugLevel < RT_DEBUG_TRACE)
		return;

	pt = pSrcBufVA;
	printk("%s: %p, len = %d\n",str,  pSrcBufVA, SrcBufLen);
	for (x=0; x<SrcBufLen; x++)
	{
		if (x % 16 == 0)
			printk("0x%04x : ", x);
		printk("%02x ", ((unsigned char)pt[x]));
		if (x%16 == 15) printk("\n");
	}
	printk("\n");
}


VOID RTMPSendWirelessEvent(
	IN	PRTMP_ADAPTER	pAd,
	IN	USHORT			Event_flag,
	IN	PUCHAR			pAddr,
	IN	UCHAR			BssIdx,
	IN	CHAR			Rssi)
{
#if WIRELESS_EXT >= 15

	
	PSTRING	pBuf = NULL, pBufPtr = NULL;
	USHORT	event, type, BufLen;
	UCHAR	event_table_len = 0;

	type = Event_flag & 0xFF00;
	event = Event_flag & 0x00FF;

	switch (type)
	{
		case IW_SYS_EVENT_FLAG_START:
			event_table_len = IW_SYS_EVENT_TYPE_NUM;
			break;

		case IW_SPOOF_EVENT_FLAG_START:
			event_table_len = IW_SPOOF_EVENT_TYPE_NUM;
			break;

		case IW_FLOOD_EVENT_FLAG_START:
			event_table_len = IW_FLOOD_EVENT_TYPE_NUM;
			break;
	}

	if (event_table_len == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : The type(%0x02x) is not valid.\n", __FUNCTION__, type));
		return;
	}

	if (event >= event_table_len)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : The event(%0x02x) is not valid.\n", __FUNCTION__, event));
		return;
	}

	
	if((pBuf = kmalloc(IW_CUSTOM_MAX_LEN, GFP_ATOMIC)) != NULL)
	{
		
		memset(pBuf, 0, IW_CUSTOM_MAX_LEN);

		pBufPtr = pBuf;

		if (pAddr)
			pBufPtr += sprintf(pBufPtr, "(RT2860) STA(%02x:%02x:%02x:%02x:%02x:%02x) ", PRINT_MAC(pAddr));
		else if (BssIdx < MAX_MBSSID_NUM)
			pBufPtr += sprintf(pBufPtr, "(RT2860) BSS(ra%d) ", BssIdx);
		else
			pBufPtr += sprintf(pBufPtr, "(RT2860) ");

		if (type == IW_SYS_EVENT_FLAG_START)
			pBufPtr += sprintf(pBufPtr, "%s", pWirelessSysEventText[event]);
		else if (type == IW_SPOOF_EVENT_FLAG_START)
			pBufPtr += sprintf(pBufPtr, "%s (RSSI=%d)", pWirelessSpoofEventText[event], Rssi);
		else if (type == IW_FLOOD_EVENT_FLAG_START)
			pBufPtr += sprintf(pBufPtr, "%s", pWirelessFloodEventText[event]);
		else
			pBufPtr += sprintf(pBufPtr, "%s", "unknown event");

		pBufPtr[pBufPtr - pBuf] = '\0';
		BufLen = pBufPtr - pBuf;

		RtmpOSWrielessEventSend(pAd, IWEVCUSTOM, Event_flag, NULL, (PUCHAR)pBuf, BufLen);
		

		kfree(pBuf);
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s : Can't allocate memory for wireless event.\n", __FUNCTION__));
#else
	DBGPRINT(RT_DEBUG_ERROR, ("%s : The Wireless Extension MUST be v15 or newer.\n", __FUNCTION__));
#endif  
}




#ifdef CONFIG_STA_SUPPORT
void send_monitor_packets(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk)
{
    struct sk_buff	*pOSPkt;
    wlan_ng_prism2_header *ph;
    int rate_index = 0;
    USHORT header_len = 0;
    UCHAR temp_header[40] = {0};

    u_int32_t ralinkrate[256] = {2,4,11,22, 12,18,24,36,48,72,96,  108,   109, 110, 111, 112, 13, 26, 39, 52,78,104, 117, 130, 26, 52, 78,104, 156, 208, 234, 260, 27, 54,81,108,162, 216, 243, 270, 
	54, 108, 162, 216, 324, 432, 486, 540,  14, 29, 43, 57, 87, 115, 130, 144, 29, 59,87,115, 173, 230,260, 288, 30, 60,90,120,180,240,270,300,60,120,180,240,360,480,540,600, 0,1,2,3,4,5,6,7,8,9,10,
	11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80};


    ASSERT(pRxBlk->pRxPacket);
    if (pRxBlk->DataSize < 10)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too small! (%d)\n", __FUNCTION__, pRxBlk->DataSize));
		goto err_free_sk_buff;
    }

    if (pRxBlk->DataSize + sizeof(wlan_ng_prism2_header) > RX_BUFFER_AGGRESIZE)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too large! (%d)\n", __FUNCTION__, pRxBlk->DataSize + sizeof(wlan_ng_prism2_header)));
		goto err_free_sk_buff;
    }

    pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);
	pOSPkt->dev = get_netdev_from_bssid(pAd, BSS0);
    if (pRxBlk->pHeader->FC.Type == BTYPE_DATA)
    {
        pRxBlk->DataSize -= LENGTH_802_11;
        if ((pRxBlk->pHeader->FC.ToDs == 1) &&
            (pRxBlk->pHeader->FC.FrDs == 1))
            header_len = LENGTH_802_11_WITH_ADDR4;
        else
            header_len = LENGTH_802_11;

        
	if (pRxBlk->pHeader->FC.SubType & 0x08)
	{
	    header_len += 2;
		
		pRxBlk->DataSize -=2;
	}

	
	if (pRxBlk->pHeader->FC.Order)
	{
	    header_len += 4;
			
			pRxBlk->DataSize -= 4;
	}

        
        if (header_len <= 40)
            NdisMoveMemory(temp_header, pRxBlk->pData, header_len);

        
	if (pRxBlk->RxD.L2PAD)
	    pRxBlk->pData += (header_len + 2);
        else
            pRxBlk->pData += header_len;
    } 


	if (pRxBlk->DataSize < pOSPkt->len) {
        skb_trim(pOSPkt,pRxBlk->DataSize);
    } else {
        skb_put(pOSPkt,(pRxBlk->DataSize - pOSPkt->len));
    } 

    if ((pRxBlk->pData - pOSPkt->data) > 0) {
	    skb_put(pOSPkt,(pRxBlk->pData - pOSPkt->data));
	    skb_pull(pOSPkt,(pRxBlk->pData - pOSPkt->data));
    } 

    if (skb_headroom(pOSPkt) < (sizeof(wlan_ng_prism2_header)+ header_len)) {
        if (pskb_expand_head(pOSPkt, (sizeof(wlan_ng_prism2_header) + header_len), 0, GFP_ATOMIC)) {
	        DBGPRINT(RT_DEBUG_ERROR, ("%s : Reallocate header size of sk_buff fail!\n", __FUNCTION__));
			goto err_free_sk_buff;
	    } 
    } 

    if (header_len > 0)
        NdisMoveMemory(skb_push(pOSPkt, header_len), temp_header, header_len);

    ph = (wlan_ng_prism2_header *) skb_push(pOSPkt, sizeof(wlan_ng_prism2_header));
	NdisZeroMemory(ph, sizeof(wlan_ng_prism2_header));

    ph->msgcode		    = DIDmsg_lnxind_wlansniffrm;
	ph->msglen		    = sizeof(wlan_ng_prism2_header);
	strcpy((PSTRING) ph->devname, (PSTRING) pAd->net_dev->name);

    ph->hosttime.did = DIDmsg_lnxind_wlansniffrm_hosttime;
	ph->hosttime.status = 0;
	ph->hosttime.len = 4;
	ph->hosttime.data = jiffies;

	ph->mactime.did = DIDmsg_lnxind_wlansniffrm_mactime;
	ph->mactime.status = 0;
	ph->mactime.len = 0;
	ph->mactime.data = 0;

    ph->istx.did = DIDmsg_lnxind_wlansniffrm_istx;
	ph->istx.status = 0;
	ph->istx.len = 0;
	ph->istx.data = 0;

    ph->channel.did = DIDmsg_lnxind_wlansniffrm_channel;
	ph->channel.status = 0;
	ph->channel.len = 4;

    ph->channel.data = (u_int32_t)pAd->CommonCfg.Channel;

    ph->rssi.did = DIDmsg_lnxind_wlansniffrm_rssi;
	ph->rssi.status = 0;
	ph->rssi.len = 4;
    ph->rssi.data = (u_int32_t)RTMPMaxRssi(pAd, ConvertToRssi(pAd, pRxBlk->pRxWI->RSSI0, RSSI_0), ConvertToRssi(pAd, pRxBlk->pRxWI->RSSI1, RSSI_1), ConvertToRssi(pAd, pRxBlk->pRxWI->RSSI2, RSSI_2));;

	ph->signal.did = DIDmsg_lnxind_wlansniffrm_signal;
	ph->signal.status = 0;
	ph->signal.len = 4;
	ph->signal.data = 0; 

	ph->noise.did = DIDmsg_lnxind_wlansniffrm_noise;
	ph->noise.status = 0;
	ph->noise.len = 4;
	ph->noise.data = 0;

#ifdef DOT11_N_SUPPORT
    if (pRxBlk->pRxWI->PHYMODE >= MODE_HTMIX)
    {
	rate_index = 16 + ((UCHAR)pRxBlk->pRxWI->BW *16) + ((UCHAR)pRxBlk->pRxWI->ShortGI *32) + ((UCHAR)pRxBlk->pRxWI->MCS);
    }
    else
#endif 
	if (pRxBlk->pRxWI->PHYMODE == MODE_OFDM)
	rate_index = (UCHAR)(pRxBlk->pRxWI->MCS) + 4;
    else
	rate_index = (UCHAR)(pRxBlk->pRxWI->MCS);
    if (rate_index < 0)
        rate_index = 0;
    if (rate_index > 255)
        rate_index = 255;

	ph->rate.did = DIDmsg_lnxind_wlansniffrm_rate;
	ph->rate.status = 0;
	ph->rate.len = 4;
    ph->rate.data = ralinkrate[rate_index];

	ph->frmlen.did = DIDmsg_lnxind_wlansniffrm_frmlen;
    ph->frmlen.status = 0;
	ph->frmlen.len = 4;
	ph->frmlen.data	= (u_int32_t)pRxBlk->DataSize;


    pOSPkt->pkt_type = PACKET_OTHERHOST;
    pOSPkt->protocol = eth_type_trans(pOSPkt, pOSPkt->dev);
    pOSPkt->ip_summed = CHECKSUM_NONE;
    netif_rx(pOSPkt);

    return;

err_free_sk_buff:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;

}
#endif 




RTMP_OS_FD RtmpOSFileOpen(char *pPath,  int flag, int mode)
{
	struct file	*filePtr;

	filePtr = filp_open(pPath, flag, 0);
	if (IS_ERR(filePtr))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Error %ld opening %s\n", __FUNCTION__, -PTR_ERR(filePtr), pPath));
	}

	return (RTMP_OS_FD)filePtr;
}

int RtmpOSFileClose(RTMP_OS_FD osfd)
{
	filp_close(osfd, NULL);
	return 0;
}


void RtmpOSFileSeek(RTMP_OS_FD osfd, int offset)
{
	osfd->f_pos = offset;
}


int RtmpOSFileRead(RTMP_OS_FD osfd, char *pDataPtr, int readLen)
{
	
	if (osfd->f_op && osfd->f_op->read)
	{
		return osfd->f_op->read(osfd,  pDataPtr, readLen, &osfd->f_pos);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("no file read method\n"));
		return -1;
	}
}


int RtmpOSFileWrite(RTMP_OS_FD osfd, char *pDataPtr, int writeLen)
{
	return osfd->f_op->write(osfd, pDataPtr, (size_t)writeLen, &osfd->f_pos);
}


void RtmpOSFSInfoChange(RTMP_OS_FS_INFO *pOSFSInfo, BOOLEAN bSet)
{
	if (bSet)
	{
		
		
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
		pOSFSInfo->fsuid= current->fsuid;
		pOSFSInfo->fsgid = current->fsgid;
		current->fsuid = current->fsgid = 0;
#else
		pOSFSInfo->fsuid = current_fsuid();
		pOSFSInfo->fsgid = current_fsgid();
#endif
		pOSFSInfo->fs = get_fs();
		set_fs(KERNEL_DS);
	}
	else
	{
		set_fs(pOSFSInfo->fs);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,29)
		current->fsuid = pOSFSInfo->fsuid;
		current->fsgid = pOSFSInfo->fsgid;
#endif
	}
}




NDIS_STATUS RtmpOSTaskKill(
	IN RTMP_OS_TASK *pTask)
{
	RTMP_ADAPTER *pAd;
	int ret = NDIS_STATUS_FAILURE;

	pAd = (RTMP_ADAPTER *)pTask->priv;

#ifdef KTHREAD_SUPPORT
	if (pTask->kthread_task)
	{
		kthread_stop(pTask->kthread_task);
		ret = NDIS_STATUS_SUCCESS;
	}
#else
	CHECK_PID_LEGALITY(pTask->taskPID)
	{
		printk("Terminate the task(%s) with pid(%d)!\n", pTask->taskName, GET_PID_NUMBER(pTask->taskPID));
		mb();
		pTask->task_killed = 1;
		mb();
		ret = KILL_THREAD_PID(pTask->taskPID, SIGTERM, 1);
		if (ret)
		{
			printk(KERN_WARNING "kill task(%s) with pid(%d) failed(retVal=%d)!\n",
				pTask->taskName, GET_PID_NUMBER(pTask->taskPID), ret);
		}
		else
		{
			wait_for_completion(&pTask->taskComplete);
			pTask->taskPID = THREAD_PID_INIT_VALUE;
			pTask->task_killed = 0;
			ret = NDIS_STATUS_SUCCESS;
		}
	}
#endif

	return ret;

}


INT RtmpOSTaskNotifyToExit(
	IN RTMP_OS_TASK *pTask)
{

#ifndef KTHREAD_SUPPORT
	complete_and_exit(&pTask->taskComplete, 0);
#endif

	return 0;
}


void RtmpOSTaskCustomize(
	IN RTMP_OS_TASK *pTask)
{

#ifndef KTHREAD_SUPPORT

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	daemonize((PSTRING)&pTask->taskName[0]);

	allow_signal(SIGTERM);
	allow_signal(SIGKILL);
	current->flags |= PF_NOFREEZE;
#else
	unsigned long flags;

	daemonize();
	reparent_to_init();
	strcpy(current->comm, &pTask->taskName[0]);

	siginitsetinv(&current->blocked, sigmask(SIGTERM) | sigmask(SIGKILL));

	
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,4,22)
	spin_lock_irqsave(&current->sigmask_lock, flags);
	flush_signals(current);
	recalc_sigpending(current);
	spin_unlock_irqrestore(&current->sigmask_lock, flags);
#endif
#endif

    
	complete(&pTask->taskComplete);

#endif
}


NDIS_STATUS RtmpOSTaskAttach(
	IN RTMP_OS_TASK *pTask,
	IN int (*fn)(void *),
	IN void *arg)
{
	NDIS_STATUS status = NDIS_STATUS_SUCCESS;
	pid_t pid_number = -1;

#ifdef KTHREAD_SUPPORT
	pTask->task_killed = 0;
	pTask->kthread_task = NULL;
	pTask->kthread_task = kthread_run(fn, arg, pTask->taskName);
	if (IS_ERR(pTask->kthread_task))
		status = NDIS_STATUS_FAILURE;
#else
	pid_number = kernel_thread(fn, arg, RTMP_OS_MGMT_TASK_FLAGS);
	if (pid_number < 0)
	{
		DBGPRINT (RT_DEBUG_ERROR, ("Attach task(%s) failed!\n", pTask->taskName));
		status = NDIS_STATUS_FAILURE;
	}
	else
	{
		pTask->taskPID = GET_PID(pid_number);

		
		wait_for_completion(&pTask->taskComplete);
		status = NDIS_STATUS_SUCCESS;
	}
#endif
	return status;
}


NDIS_STATUS RtmpOSTaskInit(
	IN RTMP_OS_TASK *pTask,
	IN PSTRING		pTaskName,
	IN VOID			*pPriv)
{
	int len;

	ASSERT(pTask);

#ifndef KTHREAD_SUPPORT
	NdisZeroMemory((PUCHAR)(pTask), sizeof(RTMP_OS_TASK));
#endif

	len = strlen(pTaskName);
	len = len > (RTMP_OS_TASK_NAME_LEN -1) ? (RTMP_OS_TASK_NAME_LEN-1) : len;
	NdisMoveMemory(&pTask->taskName[0], pTaskName, len);
	pTask->priv = pPriv;

#ifndef KTHREAD_SUPPORT
	RTMP_SEM_EVENT_INIT_LOCKED(&(pTask->taskSema));
	pTask->taskPID = THREAD_PID_INIT_VALUE;

	init_completion (&pTask->taskComplete);
#endif

	return NDIS_STATUS_SUCCESS;
}


void RTMP_IndicateMediaState(
	IN	PRTMP_ADAPTER	pAd)
{
	if (pAd->CommonCfg.bWirelessEvent)
	{
		if (pAd->IndicateMediaState == NdisMediaStateConnected)
		{
			RTMPSendWirelessEvent(pAd, IW_STA_LINKUP_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
		}
		else
		{
			RTMPSendWirelessEvent(pAd, IW_STA_LINKDOWN_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
		}
	}
}


#if LINUX_VERSION_CODE <= 0x20402	

struct net_device *alloc_netdev(
	int sizeof_priv,
	const char *mask,
	void (*setup)(struct net_device *))
{
    struct net_device	*dev;
    INT					alloc_size;


    
    alloc_size = sizeof (*dev) + sizeof_priv + 31;

    dev = (struct net_device *) kmalloc(alloc_size, GFP_KERNEL);
    if (dev == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR,
				("alloc_netdev: Unable to allocate device memory.\n"));
        return NULL;
    }

    memset(dev, 0, alloc_size);

    if (sizeof_priv)
        dev->priv = (void *) (((long)(dev + 1) + 31) & ~31);

    setup(dev);
    strcpy(dev->name, mask);

    return dev;
}
#endif 


int RtmpOSWrielessEventSend(
	IN RTMP_ADAPTER *pAd,
	IN UINT32		eventType,
	IN INT			flags,
	IN PUCHAR		pSrcMac,
	IN PUCHAR		pData,
	IN UINT32		dataLen)
{
	union iwreq_data    wrqu;

       memset(&wrqu, 0, sizeof(wrqu));

	if (flags>-1)
	       wrqu.data.flags = flags;

	if (pSrcMac)
		memcpy(wrqu.ap_addr.sa_data, pSrcMac, MAC_ADDR_LEN);

	if ((pData!= NULL) && (dataLen > 0))
		wrqu.data.length = dataLen;

       wireless_send_event(pAd->net_dev, eventType, &wrqu, (char *)pData);
	return 0;
}


int RtmpOSNetDevAddrSet(
	IN PNET_DEV pNetDev,
	IN PUCHAR	pMacAddr)
{
	struct net_device *net_dev;
	RTMP_ADAPTER *pAd;

	net_dev = pNetDev;
	
	pAd=RTMP_OS_NETDEV_GET_PRIV(pNetDev);

#ifdef CONFIG_STA_SUPPORT
	
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		NdisZeroMemory(pAd->StaCfg.dev_name, 16);
		NdisMoveMemory(pAd->StaCfg.dev_name, net_dev->name, strlen(net_dev->name));
	}
#endif 

	NdisMoveMemory(net_dev->dev_addr, pMacAddr, 6);

	return 0;
}




static int RtmpOSNetDevRequestName(
	IN RTMP_ADAPTER *pAd,
	IN PNET_DEV dev,
	IN PSTRING pPrefixStr,
	IN INT	devIdx)
{
	PNET_DEV		existNetDev;
	STRING		suffixName[IFNAMSIZ];
	STRING		desiredName[IFNAMSIZ];
	int	ifNameIdx, prefixLen, slotNameLen;
	int Status;


	prefixLen = strlen(pPrefixStr);
	ASSERT((prefixLen < IFNAMSIZ));

	for (ifNameIdx = devIdx; ifNameIdx < 32; ifNameIdx++)
	{
		memset(suffixName, 0, IFNAMSIZ);
		memset(desiredName, 0, IFNAMSIZ);
		strncpy(&desiredName[0], pPrefixStr, prefixLen);

#ifdef MULTIPLE_CARD_SUPPORT
		if (pAd->MC_RowID >= 0)
			sprintf(suffixName, "%02d_%d", pAd->MC_RowID, ifNameIdx);
		else
#endif 
		sprintf(suffixName, "%d", ifNameIdx);

		slotNameLen = strlen(suffixName);
		ASSERT(((slotNameLen + prefixLen) < IFNAMSIZ));
		strcat(desiredName, suffixName);

		existNetDev = RtmpOSNetDevGetByName(dev, &desiredName[0]);
		if (existNetDev == NULL)
			break;
		else
			RtmpOSNetDeviceRefPut(existNetDev);
	}

	if(ifNameIdx < 32)
	{
		strcpy(&dev->name[0], &desiredName[0]);
		Status = NDIS_STATUS_SUCCESS;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,
					("Cannot request DevName with preifx(%s) and in range(0~32) as suffix from OS!\n", pPrefixStr));
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


void RtmpOSNetDevClose(
	IN PNET_DEV pNetDev)
{
	dev_close(pNetDev);
}


void RtmpOSNetDevFree(PNET_DEV pNetDev)
{
	ASSERT(pNetDev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	free_netdev(pNetDev);
#else
	kfree(pNetDev);
#endif
}


INT RtmpOSNetDevAlloc(
	IN PNET_DEV *new_dev_p,
	IN UINT32	privDataSize)
{
	
	*new_dev_p = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("Allocate a net device with private data size=%d!\n", privDataSize));
#if LINUX_VERSION_CODE <= 0x20402 
	*new_dev_p = alloc_netdev(privDataSize, "eth%d", ether_setup);
#else
	*new_dev_p = alloc_etherdev(privDataSize);
#endif 

	if (*new_dev_p)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


PNET_DEV RtmpOSNetDevGetByName(PNET_DEV pNetDev, PSTRING pDevName)
{
	PNET_DEV	pTargetNetDev = NULL;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	pTargetNetDev = dev_get_by_name(dev_net(pNetDev), pDevName);
#else
	ASSERT(pNetDev);
	pTargetNetDev = dev_get_by_name(pNetDev->nd_net, pDevName);
#endif
#else
	pTargetNetDev = dev_get_by_name(pDevName);
#endif 

#else
	int	devNameLen;

	devNameLen = strlen(pDevName);
	ASSERT((devNameLen <= IFNAMSIZ));

	for(pTargetNetDev=dev_base; pTargetNetDev!=NULL; pTargetNetDev=pTargetNetDev->next)
	{
		if (strncmp(pTargetNetDev->name, pDevName, devNameLen) == 0)
			break;
	}
#endif 

	return pTargetNetDev;
}


void RtmpOSNetDeviceRefPut(PNET_DEV pNetDev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	
	if(pNetDev)
		dev_put(pNetDev);
#endif 
}


INT RtmpOSNetDevDestory(
	IN RTMP_ADAPTER *pAd,
	IN PNET_DEV		pNetDev)
{

	
	printk("WARNING: This function(%s) not implement yet!!!\n", __FUNCTION__);
	return 0;
}


void RtmpOSNetDevDetach(PNET_DEV pNetDev)
{
	unregister_netdev(pNetDev);
}


int RtmpOSNetDevAttach(
	IN PNET_DEV pNetDev,
	IN RTMP_OS_NETDEV_OP_HOOK *pDevOpHook)
{
	int ret, rtnl_locked = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("RtmpOSNetDevAttach()--->\n"));
	
	if (pDevOpHook)
	{
		PRTMP_ADAPTER pAd = RTMP_OS_NETDEV_GET_PRIV(pNetDev);

		pNetDev->netdev_ops = pDevOpHook->netdev_ops;

		
		pNetDev->priv_flags = pDevOpHook->priv_flags;

#if (WIRELESS_EXT < 21) && (WIRELESS_EXT >= 12)
		pNetDev->get_wireless_stats = rt28xx_get_wireless_stats;
#endif

#ifdef CONFIG_STA_SUPPORT
#if WIRELESS_EXT >= 12
		if (pAd->OpMode == OPMODE_STA)
		{
			pNetDev->wireless_handlers = &rt28xx_iw_handler_def;
		}
#endif 
#endif 

#ifdef CONFIG_APSTA_MIXED_SUPPORT
#if WIRELESS_EXT >= 12
		if (pAd->OpMode == OPMODE_AP)
		{
			pNetDev->wireless_handlers = &rt28xx_ap_iw_handler_def;
		}
#endif 
#endif 

		
		NdisMoveMemory(pNetDev->dev_addr, &pDevOpHook->devAddr[0], MAC_ADDR_LEN);

		rtnl_locked = pDevOpHook->needProtcted;
	}

	if (rtnl_locked)
		ret = register_netdevice(pNetDev);
	else
		ret = register_netdev(pNetDev);

	DBGPRINT(RT_DEBUG_TRACE, ("<---RtmpOSNetDevAttach(), ret=%d\n", ret));
	if (ret == 0)
		return NDIS_STATUS_SUCCESS;
	else
		return NDIS_STATUS_FAILURE;
}


PNET_DEV RtmpOSNetDevCreate(
	IN RTMP_ADAPTER *pAd,
	IN INT			devType,
	IN INT			devNum,
	IN INT			privMemSize,
	IN PSTRING		pNamePrefix)
{
	struct net_device *pNetDev = NULL;
	int status;


	
	status = RtmpOSNetDevAlloc(&pNetDev, 0 );
	if (status != NDIS_STATUS_SUCCESS)
	{
		
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate network device fail (%s)...\n", pNamePrefix));
		return NULL;
	}


	
	status = RtmpOSNetDevRequestName(pAd, pNetDev, pNamePrefix, devNum);
	if (status != NDIS_STATUS_SUCCESS)
	{
		
		DBGPRINT(RT_DEBUG_ERROR, ("Assign interface name (%s with suffix 0~32) failed...\n", pNamePrefix));
		RtmpOSNetDevFree(pNetDev);

		return NULL;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("The name of the new %s interface is %s...\n", pNamePrefix, pNetDev->name));
	}

	return pNetDev;
}
