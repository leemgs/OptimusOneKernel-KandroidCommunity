

#ifndef __riodrvr_h
#define __riodrvr_h

#include <asm/param.h>		

#define MEMDUMP_SIZE	32
#define	MOD_DISABLE	(RIO_NOREAD|RIO_NOWRITE|RIO_NOXPRINT)


struct rio_info {
	int mode;		
	spinlock_t RIOIntrSem;	
	int current_chan;	
	int RIOFailed;		
	int RIOInstallAttempts;	
	int RIOLastPCISearch;	
	int RIONumHosts;	
	struct Host *RIOHosts;	
	struct Port **RIOPortp;	

	int RIOPrintDisabled;	
	int RIOPrintLogState;	
	int RIOPolling;		

	int RIOHalted;		
	int RIORtaDisCons;	
	unsigned int RIOReadCheck;	
	unsigned int RIONoMessage;	
	unsigned int RIONumBootPkts;	
	unsigned int RIOBootCount;	
	unsigned int RIOBooting;	
	unsigned int RIOSystemUp;	
	unsigned int RIOCounting;	
	unsigned int RIOIntCount;	
	unsigned int RIOTxCount;	
	unsigned int RIORxCount;	
	unsigned int RIORupCount;	
	int RIXTimer;
	int RIOBufferSize;	
	int RIOBufferMask;	

	int RIOFirstMajor;	

	unsigned int RIOLastPortsMapped;	
	unsigned int RIOFirstPortsMapped;	

	unsigned int RIOLastPortsBooted;	
	unsigned int RIOFirstPortsBooted;	

	unsigned int RIOLastPortsOpened;	
	unsigned int RIOFirstPortsOpened;	

	
	unsigned int RIOQuickCheck;
	unsigned int CdRegister;	
	int RIOSignalProcess;	
	int rio_debug;		
	int RIODebugWait;	
	int tpri;		
	int tid;		
	unsigned int _RIO_Polled;	
	unsigned int _RIO_Interrupted;	
	int intr_tid;		
	int TxEnSem;		


	struct Error RIOError;	
	struct Conf RIOConf;	
	struct ttystatics channel[RIO_PORTS];	
	char RIOBootPackets[1 + (SIXTY_FOUR_K / RTA_BOOT_DATA_SIZE)]
	    [RTA_BOOT_DATA_SIZE];
	struct Map RIOConnectTable[TOTAL_MAP_ENTRIES];
	struct Map RIOSavedTable[TOTAL_MAP_ENTRIES];

	
	unsigned long RIOBindTab[MAX_RTA_BINDINGS];
	
	unsigned char RIOMemDump[MEMDUMP_SIZE];
	struct ModuleInfo RIOModuleTypes[MAX_MODULE_TYPES];

};


#ifdef linux
#define debug(x)        printk x
#else
#define debug(x)	kkprintf x
#endif



#define RIO_RESET_INT	0x7d80

#endif				
