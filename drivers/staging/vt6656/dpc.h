

#ifndef __DPC_H__
#define __DPC_H__

#include "ttype.h"
#include "device.h"
#include "wcmd.h"









VOID
RXvWorkItem(
    PVOID Context
    );

VOID
RXvMngWorkItem(
    PVOID Context
    );

VOID
RXvFreeRCB(
    IN PRCB pRCB,
    IN BOOL bReAllocSkb
    );

BOOL
RXbBulkInProcessData(
    IN PSDevice         pDevice,
    IN PRCB             pRCB,
    IN ULONG            BytesToIndicate
    );

#endif 



