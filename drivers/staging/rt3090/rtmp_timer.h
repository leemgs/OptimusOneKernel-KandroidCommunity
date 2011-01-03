

#ifndef __RTMP_TIMER_H__
#define  __RTMP_TIMER_H__

#include "rtmp_os.h"


#define DECLARE_TIMER_FUNCTION(_func)			\
	void rtmp_timer_##_func(unsigned long data)

#define GET_TIMER_FUNCTION(_func)				\
	rtmp_timer_##_func







#ifdef RTMP_TIMER_TASK_SUPPORT
typedef VOID (*RTMP_TIMER_TASK_HANDLE)(
	IN  PVOID   SystemSpecific1,
	IN  PVOID   FunctionContext,
	IN  PVOID   SystemSpecific2,
	IN  PVOID   SystemSpecific3);
#endif 

typedef struct  _RALINK_TIMER_STRUCT    {
	RTMP_OS_TIMER		TimerObj;       
	BOOLEAN				Valid;			
	BOOLEAN				State;          
	BOOLEAN				PeriodicType;	
	BOOLEAN				Repeat;         
	ULONG				TimerValue;     
	ULONG				cookie;			
#ifdef RTMP_TIMER_TASK_SUPPORT
	RTMP_TIMER_TASK_HANDLE	handle;
	void					*pAd;
#endif 
}RALINK_TIMER_STRUCT, *PRALINK_TIMER_STRUCT;


#ifdef RTMP_TIMER_TASK_SUPPORT
typedef struct _RTMP_TIMER_TASK_ENTRY_
{
	RALINK_TIMER_STRUCT			*pRaTimer;
	struct _RTMP_TIMER_TASK_ENTRY_	*pNext;
}RTMP_TIMER_TASK_ENTRY;


#define TIMER_QUEUE_SIZE_MAX	128
typedef struct _RTMP_TIMER_TASK_QUEUE_
{
	unsigned int				status;
	unsigned char				*pTimerQPoll;
	RTMP_TIMER_TASK_ENTRY	*pQPollFreeList;
	RTMP_TIMER_TASK_ENTRY	*pQHead;
	RTMP_TIMER_TASK_ENTRY	*pQTail;
}RTMP_TIMER_TASK_QUEUE;

#define BUILD_TIMER_FUNCTION(_func)										\
void rtmp_timer_##_func(unsigned long data)										\
{																			\
	PRALINK_TIMER_STRUCT	_pTimer = (PRALINK_TIMER_STRUCT)data;				\
	RTMP_TIMER_TASK_ENTRY	*_pQNode;										\
	RTMP_ADAPTER			*_pAd;											\
																			\
	_pTimer->handle = _func;													\
	_pAd = (RTMP_ADAPTER *)_pTimer->pAd;										\
	_pQNode = RtmpTimerQInsert(_pAd, _pTimer);								\
	if ((_pQNode == NULL) && (_pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT))	\
		RTMP_OS_Add_Timer(&_pTimer->TimerObj, OS_HZ);							\
}
#else
#define BUILD_TIMER_FUNCTION(_func)										\
void rtmp_timer_##_func(unsigned long data)										\
{																			\
	PRALINK_TIMER_STRUCT	pTimer = (PRALINK_TIMER_STRUCT) data;				\
																			\
	_func(NULL, (PVOID) pTimer->cookie, NULL, pTimer);							\
	if (pTimer->Repeat)														\
		RTMP_OS_Add_Timer(&pTimer->TimerObj, pTimer->TimerValue);			\
}
#endif 


DECLARE_TIMER_FUNCTION(MlmePeriodicExec);
DECLARE_TIMER_FUNCTION(MlmeRssiReportExec);
DECLARE_TIMER_FUNCTION(AsicRxAntEvalTimeout);
DECLARE_TIMER_FUNCTION(APSDPeriodicExec);
DECLARE_TIMER_FUNCTION(AsicRfTuningExec);


#ifdef CONFIG_STA_SUPPORT
DECLARE_TIMER_FUNCTION(BeaconTimeout);
DECLARE_TIMER_FUNCTION(ScanTimeout);
DECLARE_TIMER_FUNCTION(AuthTimeout);
DECLARE_TIMER_FUNCTION(AssocTimeout);
DECLARE_TIMER_FUNCTION(ReassocTimeout);
DECLARE_TIMER_FUNCTION(DisassocTimeout);
DECLARE_TIMER_FUNCTION(LinkDownExec);
DECLARE_TIMER_FUNCTION(StaQuickResponeForRateUpExec);
DECLARE_TIMER_FUNCTION(WpaDisassocApAndBlockAssoc);
DECLARE_TIMER_FUNCTION(PsPollWakeExec);
DECLARE_TIMER_FUNCTION(RadioOnExec);

#ifdef QOS_DLS_SUPPORT
DECLARE_TIMER_FUNCTION(DlsTimeoutAction);
#endif 


#endif 




#if defined(AP_LED) || defined(STA_LED)
DECLARE_TIMER_FUNCTION(LedCtrlMain);
#endif



#endif 
