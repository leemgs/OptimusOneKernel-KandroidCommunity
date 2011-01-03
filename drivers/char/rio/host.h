

#ifndef __rio_host_h__
#define __rio_host_h__



#define	MAX_EXTRA_UNITS	64


struct Host {
	struct pci_dev *pdev;
	unsigned char Type;		
	unsigned char Ivec;		
	unsigned char Mode;		
	unsigned char Slot;		
	void  __iomem *Caddr;		
	struct DpRam __iomem *CardP;	
	unsigned long PaddrP;		
	char Name[MAX_NAME_LEN];	
	unsigned int UniqueNum;		
	spinlock_t HostLock;	
	unsigned int WorkToBeDone;	
	unsigned int InIntr;		
	unsigned int IntSrvDone;	
	void (*Copy) (void *, void __iomem *, int);	
	struct timer_list timer;
	

	unsigned long Flags;			
#define RC_WAITING            0
#define RC_STARTUP            1
#define RC_RUNNING            2
#define RC_STUFFED            3
#define RC_READY              7
#define RUN_STATE             7

#define RC_BOOT_ALL           0x8		
#define RC_BOOT_OWN           0x10		
#define RC_BOOT_NONE          0x20		

	struct Top Topology[LINKS_PER_UNIT];	
	struct Map Mapping[MAX_RUP];		
	struct PHB __iomem *PhbP;		
	unsigned short __iomem *PhbNumP;	
	struct LPB __iomem *LinkStrP;		
	struct RUP __iomem *RupP;		
	struct PARM_MAP __iomem *ParmMapP;	
	unsigned int ExtraUnits[MAX_EXTRA_UNITS];	
	unsigned int NumExtraBooted;		
	
	struct UnixRup UnixRups[MAX_RUP + LINKS_PER_UNIT];
	int timeout_id;				
	int timeout_sem;			
	unsigned long locks;			
	char ____end_marker____;
};
#define Control      CardP->DpControl
#define SetInt       CardP->DpSetInt
#define ResetTpu     CardP->DpResetTpu
#define ResetInt     CardP->DpResetInt
#define Signature    CardP->DpSignature
#define Sram1        CardP->DpSram1
#define Sram2        CardP->DpSram2
#define Sram3        CardP->DpSram3
#define Scratch      CardP->DpScratch
#define __ParmMapR   CardP->DpParmMapR
#define SLX          CardP->DpSlx
#define Revision     CardP->DpRevision
#define Unique       CardP->DpUnique
#define Year         CardP->DpYear
#define Week         CardP->DpWeek

#define RIO_DUMBPARM 0x0860	

#endif
