






















#ifndef	_sxwindow_h				
#define	_sxwindow_h    1



typedef	struct	_SXCARD		*PSXCARD;	
typedef	struct	_SXMODULE	*PMOD;		
typedef	struct	_SXCHANNEL	*PCHAN;		



typedef	struct	_SXCARD
{
	BYTE	cc_init_status;			
	BYTE	cc_mem_size;			
	WORD	cc_int_count;			
	WORD	cc_revision;			
	BYTE	cc_isr_count;			
	BYTE	cc_main_count;			
	WORD	cc_int_pending;			
	WORD	cc_poll_count;			
	BYTE	cc_int_set_count;		
	BYTE	cc_rfu[0x80 - 0x0D];		

} SXCARD;


#define 	ADAPTERS_FOUND		(BYTE)0x01
#define 	NO_ADAPTERS_FOUND	(BYTE)0xFF


#define 	SX_MEMORY_SIZE		(BYTE)0x40


#define 	INT_COUNT_DEFAULT	100	



#define	TOP_POINTER(a)		((a)|0x8000)	
#define UNTOP_POINTER(a)	((a)&~0x8000)	

typedef	struct	_SXMODULE
{
	WORD	mc_next;			
	BYTE	mc_type;			
	BYTE	mc_mod_no;			
	BYTE	mc_dtr;				
	BYTE	mc_rfu1;			
	WORD	mc_uart;			
	BYTE	mc_chip;			
	BYTE	mc_current_uart;		
#ifdef	DOWNLOAD
	PCHAN	mc_chan_pointer[8];		
#else
	WORD	mc_chan_pointer[8];		
#endif
	WORD	mc_rfu2;			
	BYTE	mc_opens1;			
	BYTE	mc_opens2;			
	BYTE	mc_mods;			
	BYTE	mc_rev1;			
	BYTE	mc_rev2;			
	BYTE	mc_mtaasic_rev;			
	BYTE	mc_rfu3[0x100 - 0x22];		

} SXMODULE;


#define		FOUR_PORTS	(BYTE)4
#define 	EIGHT_PORTS	(BYTE)8


#define 	CHIP_MASK	0xF0
#define		TA		(BYTE)0
#define 	TA4		(TA | FOUR_PORTS)
#define 	TA8		(TA | EIGHT_PORTS)
#define		TA4_ASIC	(BYTE)0x0A
#define		TA8_ASIC	(BYTE)0x0B
#define 	MTA_CD1400	(BYTE)0x28
#define 	SXDC		(BYTE)0x48


#define		MOD_RS232DB25	0x00		
#define		MOD_RS232RJ45	0x01		
#define		MOD_RESERVED_2	0x02		
#define		MOD_RS422DB25	0x03		
#define		MOD_RESERVED_4	0x04		
#define		MOD_PARALLEL	0x05		
#define		MOD_RESERVED_6	0x06		
#define		MOD_RESERVED_7	0x07		
#define		MOD_2_RS232DB25	0x08		
#define		MOD_2_RS232RJ45	0x09		
#define		MOD_RESERVED_A	0x0A		
#define		MOD_2_RS422DB25	0x0B		
#define		MOD_RESERVED_C	0x0C		
#define		MOD_2_PARALLEL	0x0D		
#define		MOD_RESERVED_E	0x0E		
#define		MOD_BLANK	0x0F		



#define		TX_BUFF_OFFSET		0x60	
#define		BUFF_POINTER(a)		(((a)+TX_BUFF_OFFSET)|0x8000)
#define		UNBUFF_POINTER(a)	(jet_channel*)(((a)&~0x8000)-TX_BUFF_OFFSET) 
#define 	BUFFER_SIZE		256
#define 	HIGH_WATER		((BUFFER_SIZE / 4) * 3)
#define 	LOW_WATER		(BUFFER_SIZE / 4)

typedef	struct	_SXCHANNEL
{
	WORD	next_item;			
	WORD 	addr_uart;			
	WORD	module;				
	BYTE 	type;				
	BYTE	chan_number;			
	WORD	xc_status;			
	BYTE	hi_rxipos;			
	BYTE	hi_rxopos;			
	BYTE	hi_txopos;			
	BYTE	hi_txipos;			
	BYTE	hi_hstat;			
	BYTE	dtr_bit;			
	BYTE	txon;				
	BYTE	txoff;				
	BYTE	rxon;				
	BYTE	rxoff;				
	BYTE	hi_mr1;				
	BYTE	hi_mr2;				
	BYTE	hi_csr;				
	BYTE	hi_op;				
	BYTE	hi_ip;				
	BYTE	hi_state;			
	BYTE	hi_prtcl;			
	BYTE	hi_txon;			
	BYTE	hi_txoff;			
	BYTE	hi_rxon;			
	BYTE	hi_rxoff;			
	BYTE	close_prev;			
	BYTE	hi_break;			
	BYTE	break_state;			
	BYTE	hi_mask;			
	BYTE	mask;				
	BYTE	mod_type;			
	BYTE	ccr_state;			
	BYTE	ip_mask;			
	BYTE	hi_parallel;			
	BYTE	par_error;			
	BYTE	any_sent;			
	BYTE	asic_txfifo_size;		
	BYTE	rfu1[2];			
	BYTE	csr;				
#ifdef	DOWNLOAD
	PCHAN	nextp;				
#else
	WORD	nextp;				
#endif
	BYTE	prtcl;				
	BYTE	mr1;				
	BYTE	mr2;				
	BYTE	hi_txbaud;			
	BYTE	hi_rxbaud;			
	BYTE	txbreak_state;			
	BYTE	txbaud;				
	BYTE	rxbaud;				
	WORD	err_framing;			
	WORD	err_parity;			
	WORD	err_overrun;			
	WORD	err_overflow;			
	BYTE	rfu2[TX_BUFF_OFFSET - 0x40];	
	BYTE	hi_txbuf[BUFFER_SIZE];		
	BYTE	hi_rxbuf[BUFFER_SIZE];		
	BYTE	rfu3[0x300 - 0x260];		

} SXCHANNEL;


#define		FASTPATH	0x1000		


#define		X_TANY		0x0001		
#define		X_TION		0x0001		
#define		X_TXEN		0x0002		
#define		X_RTSEN		0x0002		
#define		X_TXRC		0x0004		
#define		X_RTSLOW	0x0004		
#define		X_RXEN		0x0008		
#define		X_ANYXO		0x0010		
#define		X_RXSE		0x0020		
#define		X_NPEND		0x0040		
#define		X_FPEND		0x0080		
#define		C_CRSE		0x0100		
#define		C_TEMR		0x0100		
#define		C_TEMA		0x0200		
#define		C_ANYP		0x0200		
#define		C_EN		0x0400		
#define		C_HIGH		0x0800		
#define		C_CTSEN		0x1000		
#define		C_DCDEN		0x2000		
#define		C_BREAK		0x4000		
#define		C_RTSEN		0x8000		
#define		C_PARITY	0x8000		


#define		HS_IDLE_OPEN	0x00		
#define		HS_LOPEN	0x02		
#define		HS_MOPEN	0x04		
#define		HS_IDLE_MPEND	0x06		
#define		HS_CONFIG	0x08		
#define		HS_CLOSE	0x0A		
#define		HS_START	0x0C		
#define		HS_STOP		0x0E		
#define		HS_IDLE_CLOSED	0x10		
#define		HS_IDLE_BREAK	0x12		
#define		HS_FORCE_CLOSED	0x14		
#define		HS_RESUME	0x16		
#define		HS_WFLUSH	0x18		
#define		HS_RFLUSH	0x1A		
#define		HS_SUSPEND	0x1C		
#define		PARALLEL	0x1E		
#define		ENABLE_RX_INTS	0x20		
#define		ENABLE_TX_INTS	0x22		
#define		ENABLE_MDM_INTS	0x24		
#define		DISABLE_INTS	0x26		


#define		MR1_BITS	0x03		
#define		MR1_5_BITS	0x00		
#define		MR1_6_BITS	0x01		
#define		MR1_7_BITS	0x02		
#define		MR1_8_BITS	0x03		
#define		MR1_PARITY	0x1C		
#define		MR1_ODD		0x04		
#define		MR1_EVEN	0x00		
#define		MR1_WITH	0x00		
#define		MR1_FORCE	0x08		
#define		MR1_NONE	0x10		
#define		MR1_NOPARITY	MR1_NONE		
#define		MR1_ODDPARITY	(MR1_WITH|MR1_ODD)	
#define		MR1_EVENPARITY	(MR1_WITH|MR1_EVEN)	
#define		MR1_MARKPARITY	(MR1_FORCE|MR1_ODD)	
#define		MR1_SPACEPARITY	(MR1_FORCE|MR1_EVEN)	
#define		MR1_RTS_RXFLOW	0x80		


#define		MR2_STOP	0x0F		
#define		MR2_1_STOP	0x07		
#define		MR2_2_STOP	0x0F		
#define		MR2_CTS_TXFLOW	0x10		
#define		MR2_RTS_TOGGLE	0x20		
#define		MR2_NORMAL	0x00		
#define		MR2_AUTO	0x40		
#define		MR2_LOCAL	0x80		
#define		MR2_REMOTE	0xC0		


#define		CSR_75		0x0		
#define		CSR_110		0x1		
#define		CSR_38400	0x2		
#define		CSR_150		0x3		
#define		CSR_300		0x4		
#define		CSR_600		0x5		
#define		CSR_1200	0x6		
#define		CSR_2000	0x7		
#define		CSR_2400	0x8		
#define		CSR_4800	0x9		
#define		CSR_1800	0xA		
#define		CSR_9600	0xB		
#define		CSR_19200	0xC		
#define		CSR_57600	0xD		
#define		CSR_EXTBAUD	0xF		


#define		OP_RTS		0x01		
#define		OP_DTR		0x02		


#define		IP_CTS		0x02		
#define		IP_DCD		0x04		
#define		IP_DSR		0x20		
#define		IP_RI		0x40		


#define		ST_BREAK	0x01		
#define		ST_DCD		0x02		


#define		SP_TANY		0x01		
#define		SP_TXEN		0x02		
#define		SP_CEN		0x04		
#define		SP_RXEN		0x08		
#define		SP_DCEN		0x20		
#define		SP_DTR_RXFLOW	0x40		
#define		SP_PAEN		0x80		


#define		BR_IGN		0x01		
#define		BR_INT		0x02		
#define		BR_PARMRK	0x04		
#define		BR_PARIGN	0x08		
#define 	BR_ERRINT	0x80		


#define		DIAG_IRQ_RX	0x01		
#define		DIAG_IRQ_TX	0x02		
#define		DIAG_IRQ_MD	0x04		


#define		BAUD_75		0x00		
#define		BAUD_115200	0x01		
#define		BAUD_38400	0x02		
#define		BAUD_150	0x03		
#define		BAUD_300	0x04		
#define		BAUD_600	0x05		
#define		BAUD_1200	0x06		
#define		BAUD_2000	0x07		
#define		BAUD_2400	0x08		
#define		BAUD_4800	0x09		
#define		BAUD_1800	0x0A		
#define		BAUD_9600	0x0B		
#define		BAUD_19200	0x0C		
#define		BAUD_57600	0x0D		
#define		BAUD_230400	0x0E		
#define		BAUD_460800	0x0F		
#define		BAUD_921600	0x10		
#define		BAUD_50		0x11    	
#define		BAUD_110	0x12		
#define		BAUD_134_5	0x13		
#define		BAUD_200	0x14		
#define		BAUD_7200	0x15		
#define		BAUD_56000	0x16		
#define		BAUD_64000	0x17		
#define		BAUD_76800	0x18		
#define		BAUD_128000	0x19		
#define		BAUD_150000	0x1A		
#define		BAUD_14400	0x1B		
#define		BAUD_256000	0x1C		
#define		BAUD_28800	0x1D		


#define		TXBREAK_OFF	0		
#define		TXBREAK_START	1		
#define		TXBREAK_START1	2		
#define		TXBREAK_ON	3		
#define		TXBREAK_STOP	4		
#define		TXBREAK_STOP1	5		

#endif						



