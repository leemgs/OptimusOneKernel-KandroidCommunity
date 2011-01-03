

#ifndef __POWER_H__
#define __POWER_H__



#define     C_PWBT                   1000      
#define     PS_FAST_INTERVAL         1         
#define     PS_MAX_INTERVAL          4         














BOOL
PSbConsiderPowerDown(
    IN HANDLE hDeviceContext,
    IN BOOL bCheckRxDMA,
    IN BOOL bCheckCountToWakeUp
    );

VOID
PSvDisablePowerSaving(
    IN HANDLE hDeviceContext
    );

VOID
PSvEnablePowerSaving(
    IN HANDLE hDeviceContext,
    IN WORD wListenInterval
    );

VOID
PSvSendPSPOLL(
    IN HANDLE hDeviceContext
    );

BOOL
PSbSendNullPacket(
    IN HANDLE hDeviceContext
    );

BOOL
PSbIsNextTBTTWakeUp(
    IN HANDLE hDeviceContext
    );

#endif 
