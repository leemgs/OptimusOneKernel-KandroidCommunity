

#ifndef _AP_APCLI_H_
#define _AP_APCLI_H_

#ifdef APCLI_SUPPORT

#include "rtmp.h"

#define AUTH_TIMEOUT	300         
#define ASSOC_TIMEOUT	300         

#define PROBE_TIMEOUT	1000        

#define APCLI_ROOT_BSSID_GET(pAd, wcid) ((pAd)->MacTab.Content[(wcid)].Addr)
#define APCLI_IF_UP_CHECK(pAd, ifidx) ((pAd)->ApCfg.ApCliTab[(ifidx)].dev->flags & IFF_UP)


#define APCLI_MR_APIDX_SANITY_CHECK(idx) \
{ \
	if ((idx) >= MAX_APCLI_NUM) \
	{ \
		(idx) = 0; \
		DBGPRINT(RT_DEBUG_ERROR, ("%s> Error! apcli-idx > MAX_APCLI_NUM!\n", __FUNCTION__)); \
	} \
}

typedef struct _APCLI_MLME_JOIN_REQ_STRUCT {
	UCHAR	Bssid[MAC_ADDR_LEN];
	UCHAR	SsidLen;
	UCHAR	Ssid[MAX_LEN_OF_SSID];
} APCLI_MLME_JOIN_REQ_STRUCT;

typedef struct _STA_CTRL_JOIN_REQ_STRUCT {
	USHORT	Status;
} APCLI_CTRL_MSG_STRUCT, *PSTA_CTRL_MSG_STRUCT;

BOOLEAN isValidApCliIf(
	SHORT ifIndex);




VOID ApCliCtrlStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE_EX *Sm,
	OUT STATE_MACHINE_FUNC_EX Trans[]);




VOID ApCliSyncStateMachineInit(
    IN PRTMP_ADAPTER pAd,
    IN STATE_MACHINE_EX *Sm,
    OUT STATE_MACHINE_FUNC_EX Trans[]);




VOID ApCliAuthStateMachineInit(
    IN PRTMP_ADAPTER pAd,
    IN STATE_MACHINE_EX *Sm,
    OUT STATE_MACHINE_FUNC_EX Trans[]);




VOID ApCliAssocStateMachineInit(
    IN PRTMP_ADAPTER pAd,
    IN STATE_MACHINE_EX *Sm,
    OUT STATE_MACHINE_FUNC_EX Trans[]);

MAC_TABLE_ENTRY *ApCliTableLookUpByWcid(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid,
	IN PUCHAR pAddrs);


BOOLEAN ApCliAllowToSendPacket(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	OUT UCHAR		*pWcid);

BOOLEAN		ApCliValidateRSNIE(
	IN		PRTMP_ADAPTER	pAd,
	IN		PEID_STRUCT	pEid_ptr,
	IN		USHORT			eid_len,
	IN		USHORT			idx);

VOID RT28xx_ApCli_Init(
	IN PRTMP_ADAPTER	pAd,
	IN PNET_DEV			pPhyNetDev);

VOID RT28xx_ApCli_Close(
	IN PRTMP_ADAPTER	pAd);

VOID RT28xx_ApCli_Remove(
	IN PRTMP_ADAPTER	pAd);


VOID RT28xx_ApCli_Remove(
	IN PRTMP_ADAPTER ad_p);

INT ApCliIfLookUp(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr);

INT ApCli_VirtualIF_Open(
	IN	PNET_DEV	dev_p);

INT ApCli_VirtualIF_Close(
	IN	PNET_DEV	dev_p);

INT ApCli_VirtualIF_PacketSend(
	IN PNDIS_PACKET		skb_p,
	IN PNET_DEV			dev_p);

INT ApCli_VirtualIF_Ioctl(
	IN PNET_DEV				dev_p,
	IN OUT struct ifreq	*rq_p,
	IN INT cmd);


VOID ApCliMgtMacHeaderInit(
    IN	PRTMP_ADAPTER	pAd,
    IN OUT PHEADER_802_11 pHdr80211,
    IN UCHAR SubType,
    IN UCHAR ToDs,
    IN PUCHAR pDA,
    IN PUCHAR pBssid,
    IN USHORT ifIndex);

#ifdef DOT11_N_SUPPORT
BOOLEAN ApCliCheckHt(
	IN		PRTMP_ADAPTER		pAd,
	IN		USHORT				IfIndex,
	IN OUT	HT_CAPABILITY_IE	*pHtCapability,
	IN OUT	ADD_HT_INFO_IE		*pAddHtInfo);
#endif 

BOOLEAN ApCliLinkUp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

VOID ApCliLinkDown(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

VOID ApCliIfUp(
	IN PRTMP_ADAPTER pAd);

VOID ApCliIfDown(
	IN PRTMP_ADAPTER pAd);

VOID ApCliIfMonitor(
	IN PRTMP_ADAPTER pAd);

BOOLEAN ApCliMsgTypeSubst(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

BOOLEAN preCheckMsgTypeSubset(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

BOOLEAN ApCliPeerAssocRspSanity(
    IN PRTMP_ADAPTER pAd,
    IN VOID *pMsg,
    IN ULONG MsgLen,
    OUT PUCHAR pAddr2,
    OUT USHORT *pCapabilityInfo,
    OUT USHORT *pStatus,
    OUT USHORT *pAid,
    OUT UCHAR SupRate[],
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[],
    OUT UCHAR *pExtRateLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT ADD_HT_INFO_IE *pAddHtInfo,	
    OUT UCHAR *pHtCapabilityLen,
    OUT UCHAR *pAddHtInfoLen,
    OUT UCHAR *pNewExtChannelOffset,
    OUT PEDCA_PARM pEdcaParm,
    OUT UCHAR *pCkipFlag);

VOID	ApCliPeerPairMsg1Action(
	IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

VOID	ApCliPeerPairMsg3Action(
	IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

VOID	ApCliPeerGroupMsg1Action(
	IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

BOOLEAN ApCliCheckRSNIE(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pData,
	IN  UCHAR           DataLen,
	IN  MAC_TABLE_ENTRY *pEntry,
	OUT	UCHAR			*Offset);

BOOLEAN ApCliParseKeyData(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKeyData,
	IN  UCHAR           KeyDataLen,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	UCHAR			IfIdx,
	IN	UCHAR			bPairewise);

BOOLEAN  ApCliHandleRxBroadcastFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN	RX_BLK			*pRxBlk,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	UCHAR			FromWhichBSSID);

VOID APCliUpdatePairwiseKeyTable(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			*KeyRsc,
	IN  MAC_TABLE_ENTRY *pEntry);

BOOLEAN APCliUpdateSharedKeyTable(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKey,
	IN  UCHAR           KeyLen,
	IN	UCHAR			DefaultKeyIdx,
	IN  MAC_TABLE_ENTRY *pEntry);

#endif 

#endif 
