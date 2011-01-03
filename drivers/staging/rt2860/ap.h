
#ifndef __AP_H__
#define __AP_H__



#ifdef RT2870
VOID BeaconUpdateExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3);
#endif 

VOID RTMPSetPiggyBack(
	IN PRTMP_ADAPTER	pAd,
	IN BOOLEAN			bPiggyBack);



VOID MacTableReset(
    IN  PRTMP_ADAPTER   pAd);

MAC_TABLE_ENTRY *MacTableInsertEntry(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr,
	IN	UCHAR			apidx,
	IN BOOLEAN	CleanAll);

BOOLEAN MacTableDeleteEntry(
    IN  PRTMP_ADAPTER   pAd,
	IN USHORT wcid,
    IN  PUCHAR          pAddr);

MAC_TABLE_ENTRY *MacTableLookup(
    IN  PRTMP_ADAPTER   pAd,
    IN  PUCHAR          pAddr);

#endif  

