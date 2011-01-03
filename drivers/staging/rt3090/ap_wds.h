

#ifndef _AP_WDS_H_
#define _AP_WDS_H_

#define WDS_ENTRY_RETRY_INTERVAL	(100 * OS_HZ / 1000)


static inline BOOLEAN WDS_IF_UP_CHECK(
	IN  PRTMP_ADAPTER   pAd,
	IN  ULONG ifidx)
{
	if ((pAd->flg_wds_init != TRUE) ||
		(ifidx >= MAX_WDS_ENTRY))
		return FALSE;



if ((pAd->WdsTab.WdsEntry[ifidx].dev != NULL) && (pAd->WdsTab.WdsEntry[ifidx].dev->flags & IFF_UP))
		return TRUE;

	return FALSE;
}

LONG WdsEntryAlloc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr);

VOID WdsEntryDel(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr);

MAC_TABLE_ENTRY *MacTableInsertWDSEntry(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR pAddr,
	UINT WdsTabIdx);

BOOLEAN MacTableDeleteWDSEntry(
	IN PRTMP_ADAPTER pAd,
	IN USHORT wcid,
	IN PUCHAR pAddr);


BOOLEAN ApWdsAllowToSendPacket(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	OUT	UCHAR		*pWcid);

MAC_TABLE_ENTRY *WdsTableLookupByWcid(
    IN  PRTMP_ADAPTER   pAd,
	IN UCHAR wcid,
	IN PUCHAR pAddr,
	IN BOOLEAN bResetIdelCount);

MAC_TABLE_ENTRY *WdsTableLookup(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr,
	IN BOOLEAN bResetIdelCount);

MAC_TABLE_ENTRY *FindWdsEntry(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Wcid,
	IN PUCHAR			pAddr,
	IN UINT32			PhyMode);

VOID WdsTableMaintenance(
    IN PRTMP_ADAPTER    pAd);

VOID RT28xx_WDS_Init(
	IN PRTMP_ADAPTER pAd,
	IN PNET_DEV net_dev);

VOID RT28xx_WDS_Close(
	IN PRTMP_ADAPTER pAd);

VOID RT28xx_WDS_Remove(
	IN PRTMP_ADAPTER pAd);

VOID WdsDown(
	IN PRTMP_ADAPTER pAd);

VOID AsicUpdateWdsRxWCIDTable(
	IN PRTMP_ADAPTER pAd);

VOID AsicUpdateWdsEncryption(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid);

VOID WdsPeerBeaconProc(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN USHORT CapabilityInfo,
	IN UCHAR MaxSupportedRateIn500Kbps,
	IN UCHAR MaxSupportedRateLen,
	IN BOOLEAN bWmmCapable,
	IN ULONG ClientRalinkIe,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen);

VOID APWdsInitialize(
	IN PRTMP_ADAPTER pAd);

INT	Show_WdsTable_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

VOID rtmp_read_wds_from_file(
			IN  PRTMP_ADAPTER pAd,
			PSTRING tmpbuf,
			PSTRING buffer);

VOID WdsPrepareWepKeyFromMainBss(
	IN  PRTMP_ADAPTER pAd);

INT WdsVirtualIFSendPackets(
	IN PNDIS_PACKET pSkb,
	IN PNET_DEV dev);

INT WdsVirtualIF_open(
	IN PNET_DEV dev);

INT WdsVirtualIF_close(
	IN PNET_DEV dev);

INT WdsVirtualIF_ioctl(
	IN PNET_DEV net_dev,
	IN OUT struct ifreq *rq,
	IN INT cmd);


static inline BOOLEAN ValidWdsEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR WdsIndex)
{
	BOOLEAN result;
	PMAC_TABLE_ENTRY pMacEntry;

	do
	{
		if (WdsIndex >= MAX_WDS_ENTRY)
		{
			result = FALSE;
			break;
		}

		if (pAd->WdsTab.WdsEntry[WdsIndex].Valid != TRUE)
		{
			result = FALSE;
			break;
		}

		if ((pAd->WdsTab.WdsEntry[WdsIndex].MacTabMatchWCID==0)
			|| (pAd->WdsTab.WdsEntry[WdsIndex].MacTabMatchWCID >= MAX_LEN_OF_MAC_TABLE))
		{
			result = FALSE;
			break;
		}

		pMacEntry = &pAd->MacTab.Content[pAd->WdsTab.WdsEntry[WdsIndex].MacTabMatchWCID];
		if (pMacEntry->ValidAsWDS != TRUE)
		{
			result = FALSE;
			break;
		}

		result = TRUE;
	} while(FALSE);

	return result;
}
#endif 
