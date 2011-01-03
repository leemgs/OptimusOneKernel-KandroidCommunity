

#ifndef __BASEBAND_H__
#define __BASEBAND_H__

#include "ttype.h"
#include "tether.h"
#include "device.h"






#define BB_MAX_CONTEXT_SIZE 256









#define PREAMBLE_LONG   0
#define PREAMBLE_SHORT  1


#define F5G             0
#define F2_4G           1

#define TOP_RATE_54M        0x80000000
#define TOP_RATE_48M        0x40000000
#define TOP_RATE_36M        0x20000000
#define TOP_RATE_24M        0x10000000
#define TOP_RATE_18M        0x08000000
#define TOP_RATE_12M        0x04000000
#define TOP_RATE_11M        0x02000000
#define TOP_RATE_9M         0x01000000
#define TOP_RATE_6M         0x00800000
#define TOP_RATE_55M        0x00400000
#define TOP_RATE_2M         0x00200000
#define TOP_RATE_1M         0x00100000






#define BBvClearFOE(dwIoBase)                               \
{                                                           \
    BBbWriteEmbeded(dwIoBase, 0xB1, 0);                     \
}

#define BBvSetFOE(dwIoBase)                                 \
{                                                           \
    BBbWriteEmbeded(dwIoBase, 0xB1, 0x0C);                  \
}








UINT
BBuGetFrameTime(
    IN BYTE byPreambleType,
    IN BYTE byPktType,
    IN UINT cbFrameLength,
    IN WORD wRate
    );

VOID
BBvCaculateParameter (
    IN  PSDevice pDevice,
    IN  UINT cbFrameLength,
    IN  WORD wRate,
    IN  BYTE byPacketType,
    OUT PWORD pwPhyLen,
    OUT PBYTE pbyPhySrv,
    OUT PBYTE pbyPhySgn
    );

BOOL BBbReadEmbeded(DWORD_PTR dwIoBase, BYTE byBBAddr, PBYTE pbyData);
BOOL BBbWriteEmbeded(DWORD_PTR dwIoBase, BYTE byBBAddr, BYTE byData);

VOID BBvReadAllRegs(DWORD_PTR dwIoBase, PBYTE pbyBBRegs);
void BBvLoopbackOn(PSDevice pDevice);
void BBvLoopbackOff(PSDevice pDevice);
void BBvSetShortSlotTime(PSDevice pDevice);
BOOL BBbIsRegBitsOn(DWORD_PTR dwIoBase, BYTE byBBAddr, BYTE byTestBits);
BOOL BBbIsRegBitsOff(DWORD_PTR dwIoBase, BYTE byBBAddr, BYTE byTestBits);
VOID BBvSetVGAGainOffset(PSDevice pDevice, BYTE byData);


BOOL BBbVT3253Init(PSDevice pDevice);
VOID BBvSoftwareReset(DWORD_PTR dwIoBase);
VOID BBvPowerSaveModeON(DWORD_PTR dwIoBase);
VOID BBvPowerSaveModeOFF(DWORD_PTR dwIoBase);
VOID BBvSetTxAntennaMode(DWORD_PTR dwIoBase, BYTE byAntennaMode);
VOID BBvSetRxAntennaMode(DWORD_PTR dwIoBase, BYTE byAntennaMode);
VOID BBvSetDeepSleep(DWORD_PTR dwIoBase, BYTE byLocalID);
VOID BBvExitDeepSleep(DWORD_PTR dwIoBase, BYTE byLocalID);



VOID
TimerSQ3CallBack (
    IN  HANDLE      hDeviceContext
    );

VOID
TimerState1CallBack(
    IN  HANDLE      hDeviceContext
    );

void BBvAntennaDiversity(PSDevice pDevice, BYTE byRxRate, BYTE bySQ3);
VOID
BBvClearAntDivSQ3Value (PSDevice pDevice);

#endif 
