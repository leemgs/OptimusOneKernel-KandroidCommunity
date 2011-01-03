






   


#ifndef _SER_A2232_H_
#define _SER_A2232_H_


#define MAX_A2232_BOARDS 5

#ifndef A2232_NORMAL_MAJOR

#define A2232_NORMAL_MAJOR  224	
#define A2232_CALLOUT_MAJOR 225	
#endif


#define A2232_MAGIC 0x000a2232


struct a2232_port{
	struct gs_port gs;
	unsigned int which_a2232;
	unsigned int which_port_on_a2232;
	short disable_rx;
	short throttle_input;
	short cd_status;
};

#define	NUMLINES		7	
#define	A2232_IOBUFLEN		256	
#define	A2232_IOBUFLENMASK	0xff	


#define	A2232_UNKNOWN	0	
#define	A2232_NORMAL	1	
#define	A2232_TURBO	2	


struct a2232common {
	char   Crystal;	
	u_char Pad_a;
	u_char TimerH;	
	u_char TimerL;
	u_char CDHead;	
	u_char CDTail;	
	u_char CDStatus;
	u_char Pad_b;
};

struct a2232status {
	u_char InHead;		
	u_char InTail;		
	u_char OutDisable;	
	u_char OutHead;		
	u_char OutTail;		
	u_char OutCtrl;		
	u_char OutFlush;	
	u_char Setup;		
	u_char Param;		
	u_char Command;		
	u_char SoftFlow;	
	
	u_char XonOff;		
};

#define	A2232_MEMPAD1	\
	(0x0200 - NUMLINES * sizeof(struct a2232status)	-	\
	sizeof(struct a2232common))
#define	A2232_MEMPAD2	(0x2000 - NUMLINES * A2232_IOBUFLEN - A2232_IOBUFLEN)

struct a2232memory {
	struct a2232status Status[NUMLINES];	
	struct a2232common Common;		
	u_char Dummy1[A2232_MEMPAD1];		
	u_char OutBuf[NUMLINES][A2232_IOBUFLEN];
	u_char InBuf[NUMLINES][A2232_IOBUFLEN];	
	u_char InCtl[NUMLINES][A2232_IOBUFLEN];	
	u_char CDBuf[A2232_IOBUFLEN];		
	u_char Dummy2[A2232_MEMPAD2];		
	u_char Code[0x1000];			
	u_short InterruptAck;			
	u_char Dummy3[0x3ffe];			
	u_short Enable6502Reset;		
						
	u_char Dummy4[0x3ffe];			
	u_short ResetBoard;			
						
};

#undef A2232_MEMPAD1
#undef A2232_MEMPAD2

#define	A2232INCTL_CHAR		0	
#define	A2232INCTL_EVENT	1	

#define	A2232EVENT_Break	1	
#define	A2232EVENT_CarrierOn	2	
#define	A2232EVENT_CarrierOff	3	
#define A2232EVENT_Sync		4	

#define	A2232CMD_Enable		0x1	
#define	A2232CMD_Close		0x2	
#define	A2232CMD_Open		0xb	
#define	A2232CMD_CMask		0xf	
#define	A2232CMD_RTSOff		0x0  	
#define	A2232CMD_RTSOn		0x8	
#define	A2232CMD_Break		0xd	
#define	A2232CMD_RTSMask	0xc	
#define	A2232CMD_NoParity	0x00	
#define	A2232CMD_OddParity	0x20	
#define	A2232CMD_EvenParity	0x60	
#define	A2232CMD_ParityMask	0xe0	

#define	A2232PARAM_B115200	0x0	
#define	A2232PARAM_B50		0x1
#define	A2232PARAM_B75		0x2
#define	A2232PARAM_B110		0x3
#define	A2232PARAM_B134		0x4
#define	A2232PARAM_B150		0x5
#define	A2232PARAM_B300		0x6
#define	A2232PARAM_B600		0x7
#define	A2232PARAM_B1200	0x8
#define	A2232PARAM_B1800	0x9
#define	A2232PARAM_B2400	0xa
#define	A2232PARAM_B3600	0xb
#define	A2232PARAM_B4800	0xc
#define	A2232PARAM_B7200	0xd
#define	A2232PARAM_B9600	0xe
#define	A2232PARAM_B19200	0xf
#define	A2232PARAM_BaudMask	0xf	
#define	A2232PARAM_RcvBaud	0x10	
#define	A2232PARAM_8Bit		0x00	
#define	A2232PARAM_7Bit		0x20
#define	A2232PARAM_6Bit		0x40
#define	A2232PARAM_5Bit		0x60
#define	A2232PARAM_BitMask	0x60	



#define A2232_BAUD_TABLE_NOAVAIL -1
#define A2232_BAUD_TABLE_NUM_RATES (18)
static int a2232_baud_table[A2232_BAUD_TABLE_NUM_RATES*3] = {
	
	50,	A2232PARAM_B50,			A2232_BAUD_TABLE_NOAVAIL,
	75,	A2232PARAM_B75,			A2232_BAUD_TABLE_NOAVAIL,
	110,	A2232PARAM_B110,		A2232_BAUD_TABLE_NOAVAIL,
	134,	A2232PARAM_B134,		A2232_BAUD_TABLE_NOAVAIL,
	150,	A2232PARAM_B150,		A2232PARAM_B75,
	200,	A2232_BAUD_TABLE_NOAVAIL,	A2232_BAUD_TABLE_NOAVAIL,
	300,	A2232PARAM_B300,		A2232PARAM_B150,
	600,	A2232PARAM_B600,		A2232PARAM_B300,
	1200,	A2232PARAM_B1200,		A2232PARAM_B600,
	1800,	A2232PARAM_B1800,		A2232_BAUD_TABLE_NOAVAIL,
	2400,	A2232PARAM_B2400,		A2232PARAM_B1200,
	4800,	A2232PARAM_B4800,		A2232PARAM_B2400,
	9600,	A2232PARAM_B9600,		A2232PARAM_B4800,
	19200,	A2232PARAM_B19200,		A2232PARAM_B9600,
	38400,	A2232_BAUD_TABLE_NOAVAIL,	A2232PARAM_B19200,
	57600,	A2232_BAUD_TABLE_NOAVAIL,	A2232_BAUD_TABLE_NOAVAIL,
#ifdef A2232_SPEEDHACK
	115200,	A2232PARAM_B115200,		A2232_BAUD_TABLE_NOAVAIL,
	230400,	A2232_BAUD_TABLE_NOAVAIL,	A2232PARAM_B115200
#else
	115200,	A2232_BAUD_TABLE_NOAVAIL,	A2232_BAUD_TABLE_NOAVAIL,
	230400,	A2232_BAUD_TABLE_NOAVAIL,	A2232_BAUD_TABLE_NOAVAIL
#endif
};
#endif
