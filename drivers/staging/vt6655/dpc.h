

#ifndef __DPC_H__
#define __DPC_H__

#include "ttype.h"
#include "device.h"
#include "wcmd.h"









BOOL
device_receive_frame (
    IN  PSDevice pDevice,
    IN  PSRxDesc pCurrRD
    );

VOID	MngWorkItem(PVOID Context);

#endif 



