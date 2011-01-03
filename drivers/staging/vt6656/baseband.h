

#ifndef __BASEBAND_H__
#define __BASEBAND_H__

#include "ttype.h"
#include "tether.h"
#include "device.h"



#define PREAMBLE_LONG   0
#define PREAMBLE_SHORT  1




#define BB_MAX_CONTEXT_SIZE 256

#define C_SIFS_A      16      
#define C_SIFS_BG     10

#define C_EIFS      80      


#define C_SLOT_SHORT   9      
#define C_SLOT_LONG   20

#define C_CWMIN_A     15       
#define C_CWMIN_B     31

#define C_CWMAX      1023     


#define BB_TYPE_11A    0
#define BB_TYPE_11B    1
#define BB_TYPE_11G    2


#define PK_TYPE_11A     0
#define PK_TYPE_11B     1
#define PK_TYPE_11GB    2
#define PK_TYPE_11GA    3

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












UINT
BBuGetFrameTime(
    IN BYTE byPreambleType,
    IN BYTE byFreqType,
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



VOID
TimerSQ3CallBack (
    IN  HANDLE      hDeviceContext
    );

VOID
TimerSQ3Tmax3CallBack (
    IN  HANDLE      hDeviceContext
    );

VOID BBvAntennaDiversity (PSDevice pDevice, BYTE byRxRate, BYTE bySQ3);
void BBvLoopbackOn (PSDevice pDevice);
void BBvLoopbackOff (PSDevice pDevice);
void BBvSoftwareReset (PSDevice pDevice);

void BBvSetShortSlotTime(PSDevice pDevice);
VOID BBvSetVGAGainOffset(PSDevice pDevice, BYTE byData);
void BBvSetAntennaMode(PSDevice pDevice, BYTE byAntennaMode);
BOOL BBbVT3184Init (PSDevice pDevice);
VOID BBvSetDeepSleep (PSDevice pDevice);
VOID BBvExitDeepSleep (PSDevice pDevice);
VOID BBvUpdatePreEDThreshold(
     IN  PSDevice    pDevice,
     IN  BOOL        bScanning
     );

#endif 
