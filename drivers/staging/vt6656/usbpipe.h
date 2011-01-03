

#ifndef __USBPIPE_H__
#define __USBPIPE_H__

#include "ttype.h"
#include "device.h"









NTSTATUS
PIPEnsControlOut(
    IN PSDevice     pDevice,
    IN BYTE         byRequest,
    IN WORD         wValue,
    IN WORD         wIndex,
    IN WORD         wLength,
    IN PBYTE        pbyBuffer
    );



NTSTATUS
PIPEnsControlOutAsyn(
    IN PSDevice     pDevice,
    IN BYTE         byRequest,
    IN WORD         wValue,
    IN WORD         wIndex,
    IN WORD         wLength,
    IN PBYTE        pbyBuffer
    );

NTSTATUS
PIPEnsControlIn(
    IN PSDevice     pDevice,
    IN BYTE         byRequest,
    IN WORD         wValue,
    IN WORD         wIndex,
    IN WORD         wLength,
    IN OUT  PBYTE   pbyBuffer
    );




NTSTATUS
PIPEnsInterruptRead(
    IN PSDevice pDevice
    );

NTSTATUS
PIPEnsBulkInUsbRead(
    IN PSDevice pDevice,
    IN PRCB     pRCB
    );

NTSTATUS
PIPEnsSendBulkOut(
    IN  PSDevice pDevice,
    IN  PUSB_SEND_CONTEXT pContext
    );

#endif 



