

#ifndef	__rio_port_h__
#define	__rio_port_h__


struct Port {
	struct gs_port gs;
	int PortNum;			
	struct Host *HostP;
	void __iomem *Caddr;
	unsigned short HostPort;	
	unsigned char RupNum;		
	unsigned char ID2;		
	unsigned long State;		
#define	RIO_LOPEN	0x00001		
#define	RIO_MOPEN	0x00002		
#define	RIO_WOPEN	0x00004		
#define	RIO_CLOSING	0x00008		
#define	RIO_XPBUSY	0x00010		
#define	RIO_BREAKING	0x00020		
#define	RIO_DIRECT	0x00040		
#define	RIO_EXCLUSIVE	0x00080		
#define	RIO_NDELAY	0x00100		
#define	RIO_CARR_ON	0x00200		
#define	RIO_XPWANTR	0x00400		
#define	RIO_RBLK	0x00800		
#define	RIO_BUSY	0x01000		
#define	RIO_TIMEOUT	0x02000		
#define	RIO_TXSTOP	0x04000		
#define	RIO_WAITFLUSH	0x08000		
#define	RIO_DYNOROD	0x10000		
#define	RIO_DELETED	0x20000		
#define RIO_ISSCANCODE	0x40000		
#define	RIO_USING_EUC	0x100000	
#define	RIO_CAN_COOK	0x200000	
#define RIO_TRIAD_MODE  0x400000	
#define RIO_TRIAD_BLOCK 0x800000	
#define RIO_TRIAD_FUNC  0x1000000	
#define RIO_THROTTLE_RX 0x2000000	

	unsigned long Config;		
#define	RIO_NOREAD	0x0001		
#define	RIO_NOWRITE	0x0002		
#define	RIO_NOXPRINT	0x0004		
#define	RIO_NOMASK	0x0007		
#define RIO_IXANY	0x0008		
#define	RIO_MODEM	0x0010		
#define	RIO_IXON	0x0020		
#define RIO_WAITDRAIN	0x0040		
#define RIO_MAP_50_TO_50	0x0080	
#define RIO_MAP_110_TO_110	0x0100	


#define RIO_CTSFLOW	0x0200		
#define RIO_RTSFLOW	0x0400		


	struct PHB __iomem *PhbP;	
	u16 __iomem *TxAdd;		
	u16 __iomem *TxStart;		
	u16 __iomem *TxEnd;		
	u16 __iomem *RxRemove;		
	u16 __iomem *RxStart;		
	u16 __iomem *RxEnd;		
	unsigned int RtaUniqueNum;	
	unsigned short PortState;	
	unsigned short ModemState;	
	unsigned long ModemLines;	
	unsigned char CookMode;		
	unsigned char ParamSem;		
	unsigned char Mapped;		
	unsigned char SecondBlock;	
	unsigned char InUse;		
	unsigned char Lock;		
	unsigned char Store;		
	unsigned char FirstOpen;	
	unsigned char FlushCmdBodge;	
	unsigned char MagicFlags;	
#define	MAGIC_FLUSH	0x01		
#define	MAGIC_REBOOT	0x02		
#define	MORE_OUTPUT_EYGOR 0x04		
	unsigned char WflushFlag;	

	struct Xprint {
#ifndef MAX_XP_CTRL_LEN
#define MAX_XP_CTRL_LEN		16	
#endif
		unsigned int XpCps;
		char XpOn[MAX_XP_CTRL_LEN];
		char XpOff[MAX_XP_CTRL_LEN];
		unsigned short XpLen;	
		unsigned char XpActive;
		unsigned char XpLastTickOk;	
#define	XP_OPEN		00001
#define	XP_RUNABLE	00002
		struct ttystatics *XttyP;
	} Xprint;
	unsigned char RxDataStart;
	unsigned char Cor2Copy;		
	char *Name;			
	char *TxRingBuffer;
	unsigned short TxBufferIn;	
	unsigned short TxBufferOut;	
	unsigned short OldTxBufferOut;	
	int TimeoutId;			
	unsigned int Debug;
	unsigned char WaitUntilBooted;	
	unsigned int statsGather;	
	unsigned long txchars;		
	unsigned long rxchars;		
	unsigned long opens;		
	unsigned long closes;		
	unsigned long ioctls;		
	unsigned char LastRxTgl;	
	spinlock_t portSem;		
	int MonitorTstate;		
	int timeout_id;			
	int timeout_sem;		
	int firstOpen;			
	char *p;			
};

struct ModuleInfo {
	char *Name;
	unsigned int Flags[4];		
};


struct PortParams {
	unsigned int Port;
	unsigned long Config;
	unsigned long State;
	struct ttystatics *TtyP;
};

#endif
