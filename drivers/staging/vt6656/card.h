

#ifndef __CARD_H__
#define __CARD_H__

#include "ttype.h"








typedef enum _CARD_PHY_TYPE {

    PHY_TYPE_AUTO=0,
    PHY_TYPE_11B,
    PHY_TYPE_11G,
    PHY_TYPE_11A
} CARD_PHY_TYPE, *PCARD_PHY_TYPE;

typedef enum _CARD_OP_MODE {

    OP_MODE_INFRASTRUCTURE=0,
    OP_MODE_ADHOC,
    OP_MODE_AP,
    OP_MODE_UNKNOWN
} CARD_OP_MODE, *PCARD_OP_MODE;

#define CB_MAX_CHANNEL_24G  14

#define CB_MAX_CHANNEL_5G       42 
#define CB_MAX_CHANNEL      (CB_MAX_CHANNEL_24G+CB_MAX_CHANNEL_5G)





BOOL CARDbSetMediaChannel(PVOID pDeviceHandler, UINT uConnectionChannel);
void CARDvSetRSPINF(PVOID pDeviceHandler, BYTE byBBType);
void vUpdateIFS(PVOID pDeviceHandler);
void CARDvUpdateBasicTopRate(PVOID pDeviceHandler);
BOOL CARDbAddBasicRate(PVOID pDeviceHandler, WORD wRateIdx);
BOOL CARDbIsOFDMinBasicRate(PVOID pDeviceHandler);
void CARDvAdjustTSF(PVOID pDeviceHandler, BYTE byRxRate, QWORD qwBSSTimestamp, QWORD qwLocalTSF);
BOOL CARDbGetCurrentTSF (PVOID pDeviceHandler, PQWORD pqwCurrTSF);
BOOL CARDbClearCurrentTSF(PVOID pDeviceHandler);
void CARDvSetFirstNextTBTT(PVOID pDeviceHandler, WORD wBeaconInterval);
void CARDvUpdateNextTBTT(PVOID pDeviceHandler, QWORD qwTSF, WORD wBeaconInterval);
QWORD CARDqGetNextTBTT(QWORD qwTSF, WORD wBeaconInterval);
QWORD CARDqGetTSFOffset(BYTE byRxRate, QWORD qwTSF1, QWORD qwTSF2);
BOOL CARDbRadioPowerOff(PVOID pDeviceHandler);
BOOL CARDbRadioPowerOn(PVOID pDeviceHandler);
BYTE CARDbyGetPktType(PVOID pDeviceHandler);
void CARDvSetBSSMode(PVOID pDeviceHandler);

BOOL
CARDbChannelSwitch (
    IN PVOID            pDeviceHandler,
    IN BYTE             byMode,
    IN BYTE             byNewChannel,
    IN BYTE             byCount
    );

#endif 



