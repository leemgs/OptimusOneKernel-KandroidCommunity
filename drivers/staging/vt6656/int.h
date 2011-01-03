

#ifndef __INT_H__
#define __INT_H__

#include "ttype.h"
#include "device.h"


#pragma pack(1)
typedef struct tagSINTData {
    BYTE    byTSR0;
    BYTE    byPkt0;
    WORD    wTime0;
    BYTE    byTSR1;
    BYTE    byPkt1;
    WORD    wTime1;
    BYTE    byTSR2;
    BYTE    byPkt2;
    WORD    wTime2;
    BYTE    byTSR3;
    BYTE    byPkt3;
    WORD    wTime3;
    DWORD   dwLoTSF;
    DWORD   dwHiTSF;
    BYTE    byISR0;
    BYTE    byISR1;
    BYTE    byRTSSuccess;
    BYTE    byRTSFail;
    BYTE    byACKFail;
    BYTE    byFCSErr;
    BYTE    abySW[2];
}__attribute__ ((__packed__))
SINTData, *PSINTData;








VOID
INTvWorkItem(
    PVOID Context
    );

NTSTATUS
INTnsProcessData(
    IN  PSDevice pDevice
    );

#endif 



