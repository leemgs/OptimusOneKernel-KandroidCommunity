


#ifndef _SCC_H
#define _SCC_H

#include <linux/delay.h>





#define TIOCGATSCC	0x54c0	
#define TIOCSATSCC	0x54c1	
#define TIOCDATSCC	0x54c2	



#define CLK_RTxC	0
#define CLK_TRxC	1
#define CLK_PCLK	2


   
#define SCC_BAUD_BASE_TIMC	19200	
#define SCC_BAUD_BASE_BCLK	153600	
#define SCC_BAUD_BASE_PCLK4	229500	
#define SCC_BAUD_BASE_PCLK	503374	
#define SCC_BAUD_BASE_NONE	0	



struct scc_clock_config {
	unsigned	RTxC_base;	
	unsigned	TRxC_base;	
	unsigned	PCLK_base;	
	struct {
		unsigned clksrc;	
		unsigned divisor;	
	} baud_table[17];		
};



struct scc_port {
	struct gs_port		gs;
	volatile unsigned char	*ctrlp;
	volatile unsigned char	*datap;
	int			x_char;		
	int			c_dcd;
	int			channel;
	struct scc_port		*port_a;	
	struct scc_port		*port_b;	
};

#define SCC_MAGIC	0x52696368









#define	COMMAND_REG		0	
#define	INT_AND_DMA_REG		1	
#define	INT_VECTOR_REG		2	
#define	RX_CTRL_REG		3	
#define	AUX1_CTRL_REG		4	
#define	TX_CTRL_REG		5	
#define	SYNC_ADR_REG		6	
#define	SYNC_CHAR_REG		7	
#define	SDLC_OPTION_REG		33	
#define	TX_DATA_REG		8	
#define	MASTER_INT_CTRL		9	
#define	AUX2_CTRL_REG		10	
#define	CLK_CTRL_REG		11	
#define	TIMER_LOW_REG		12	
#define	TIMER_HIGH_REG		13	
#define	DPLL_CTRL_REG		14	
#define	INT_CTRL_REG		15	

#define	STATUS_REG		16	
#define	SPCOND_STATUS_REG	17	

#define	CURR_VECTOR_REG		18	
#define	INT_PENDING_REG		19	


#define	FS_FIFO_LOW_REG		22	
#define	FS_FIFO_HIGH_REG	23	
#define	RX_DATA_REG		24	

#define	DPLL_STATUS_REG		26	
















#define	CR_RX_CRC_RESET		0x40
#define	CR_TX_CRC_RESET		0x80
#define	CR_TX_UNDERRUN_RESET	0xc0

#define	CR_EXTSTAT_RESET	0x10
#define	CR_SEND_ABORT		0x18
#define	CR_ENAB_INT_NEXT_RX	0x20
#define	CR_TX_PENDING_RESET	0x28
#define	CR_ERROR_RESET		0x30
#define	CR_HIGHEST_IUS_RESET	0x38




#define	IDR_EXTSTAT_INT_ENAB	0x01
#define	IDR_TX_INT_ENAB		0x02
#define	IDR_PARERR_AS_SPCOND	0x04

#define	IDR_RX_INT_DISAB	0x00
#define	IDR_RX_INT_FIRST	0x08
#define	IDR_RX_INT_ALL		0x10
#define	IDR_RX_INT_SPCOND	0x18
#define	IDR_RX_INT_MASK		0x18

#define	IDR_WAITREQ_RX		0x20
#define	IDR_WAITREQ_IS_REQ	0x40
#define	IDR_WAITREQ_ENAB	0x80




#define	RCR_RX_ENAB		0x01
#define	RCR_DISCARD_SYNC_CHARS	0x02
#define	RCR_ADDR_SEARCH		0x04
#define	RCR_CRC_ENAB		0x08
#define	RCR_SEARCH_MODE		0x10
#define	RCR_AUTO_ENAB_MODE	0x20

#define	RCR_CHSIZE_MASK		0xc0
#define	RCR_CHSIZE_5		0x00
#define	RCR_CHSIZE_6		0x40
#define	RCR_CHSIZE_7		0x80
#define	RCR_CHSIZE_8		0xc0




#define	A1CR_PARITY_MASK	0x03
#define	A1CR_PARITY_NONE	0x00
#define	A1CR_PARITY_ODD		0x01
#define	A1CR_PARITY_EVEN	0x03

#define	A1CR_MODE_MASK		0x0c
#define	A1CR_MODE_SYNCR		0x00
#define	A1CR_MODE_ASYNC_1	0x04
#define	A1CR_MODE_ASYNC_15	0x08
#define	A1CR_MODE_ASYNC_2	0x0c

#define	A1CR_SYNCR_MODE_MASK	0x30
#define	A1CR_SYNCR_MONOSYNC	0x00
#define	A1CR_SYNCR_BISYNC	0x10
#define	A1CR_SYNCR_SDLC		0x20
#define	A1CR_SYNCR_EXTCSYNC	0x30

#define	A1CR_CLKMODE_MASK	0xc0
#define	A1CR_CLKMODE_x1		0x00
#define	A1CR_CLKMODE_x16	0x40
#define	A1CR_CLKMODE_x32	0x80
#define	A1CR_CLKMODE_x64	0xc0




#define	TCR_TX_CRC_ENAB		0x01
#define	TCR_RTS			0x02
#define	TCR_USE_CRC_CCITT	0x00
#define	TCR_USE_CRC_16		0x04
#define	TCR_TX_ENAB		0x08
#define	TCR_SEND_BREAK		0x10

#define	TCR_CHSIZE_MASK		0x60
#define	TCR_CHSIZE_5		0x00
#define	TCR_CHSIZE_6		0x20
#define	TCR_CHSIZE_7		0x40
#define	TCR_CHSIZE_8		0x60

#define	TCR_DTR			0x80




#define	SOR_AUTO_TX_ENAB	0x01
#define	SOR_AUTO_EOM_RESET	0x02
#define	SOR_AUTO_RTS_MODE	0x04
#define	SOR_NRZI_DISAB_HIGH	0x08
#define	SOR_ALT_DTRREQ_TIMING	0x10
#define	SOR_READ_CRC_CHARS	0x20
#define	SOR_EXTENDED_REG_ACCESS	0x40




#define	MIC_VEC_INCL_STAT	0x01
#define	MIC_NO_VECTOR		0x02
#define	MIC_DISAB_LOWER_CHAIN	0x04
#define	MIC_MASTER_INT_ENAB	0x08
#define	MIC_STATUS_HIGH		0x10
#define	MIC_IGN_INTACK		0x20

#define	MIC_NO_RESET		0x00
#define	MIC_CH_A_RESET		0x40
#define	MIC_CH_B_RESET		0x80
#define	MIC_HARD_RESET		0xc0




#define	A2CR_SYNC_6		0x01
#define	A2CR_LOOP_MODE		0x02
#define	A2CR_ABORT_ON_UNDERRUN	0x04
#define	A2CR_MARK_IDLE		0x08
#define	A2CR_GO_ACTIVE_ON_POLL	0x10

#define	A2CR_CODING_MASK	0x60
#define	A2CR_CODING_NRZ		0x00
#define	A2CR_CODING_NRZI	0x20
#define	A2CR_CODING_FM1		0x40
#define	A2CR_CODING_FM0		0x60

#define	A2CR_PRESET_CRC_1	0x80




#define	CCR_TRxCOUT_MASK	0x03
#define	CCR_TRxCOUT_XTAL	0x00
#define	CCR_TRxCOUT_TXCLK	0x01
#define	CCR_TRxCOUT_BRG		0x02
#define	CCR_TRxCOUT_DPLL	0x03

#define	CCR_TRxC_OUTPUT		0x04

#define	CCR_TXCLK_MASK		0x18
#define	CCR_TXCLK_RTxC		0x00
#define	CCR_TXCLK_TRxC		0x08
#define	CCR_TXCLK_BRG		0x10
#define	CCR_TXCLK_DPLL		0x18

#define	CCR_RXCLK_MASK		0x60
#define	CCR_RXCLK_RTxC		0x00
#define	CCR_RXCLK_TRxC		0x20
#define	CCR_RXCLK_BRG		0x40
#define	CCR_RXCLK_DPLL		0x60

#define	CCR_RTxC_XTAL		0x80




#define	DCR_BRG_ENAB		0x01
#define	DCR_BRG_USE_PCLK	0x02
#define	DCR_DTRREQ_IS_REQ	0x04
#define	DCR_AUTO_ECHO		0x08
#define	DCR_LOCAL_LOOPBACK	0x10

#define	DCR_DPLL_EDGE_SEARCH	0x20
#define	DCR_DPLL_ERR_RESET	0x40
#define	DCR_DPLL_DISAB		0x60
#define	DCR_DPLL_CLK_BRG	0x80
#define	DCR_DPLL_CLK_RTxC	0xa0
#define	DCR_DPLL_FM		0xc0
#define	DCR_DPLL_NRZI		0xe0




#define	ICR_OPTIONREG_SELECT	0x01
#define	ICR_ENAB_BRG_ZERO_INT	0x02
#define	ICR_USE_FS_FIFO		0x04
#define	ICR_ENAB_DCD_INT	0x08
#define	ICR_ENAB_SYNC_INT	0x10
#define	ICR_ENAB_CTS_INT	0x20
#define	ICR_ENAB_UNDERRUN_INT	0x40
#define	ICR_ENAB_BREAK_INT	0x80




#define	SR_CHAR_AVAIL		0x01
#define	SR_BRG_ZERO		0x02
#define	SR_TX_BUF_EMPTY		0x04
#define	SR_DCD			0x08
#define	SR_SYNC_ABORT		0x10
#define	SR_CTS			0x20
#define	SR_TX_UNDERRUN		0x40
#define	SR_BREAK		0x80




#define	SCSR_ALL_SENT		0x01
#define	SCSR_RESIDUAL_MASK	0x0e
#define	SCSR_PARITY_ERR		0x10
#define	SCSR_RX_OVERRUN		0x20
#define	SCSR_CRC_FRAME_ERR	0x40
#define	SCSR_END_OF_FRAME	0x80




#define	IPR_B_EXTSTAT		0x01
#define	IPR_B_TX		0x02
#define	IPR_B_RX		0x04
#define	IPR_A_EXTSTAT		0x08
#define	IPR_A_TX		0x10
#define	IPR_A_RX		0x20




#define	FFHR_CNT_MASK		0x3f
#define	FFHR_IS_FROM_FIFO	0x40
#define	FFHR_FIFO_OVERRUN	0x80




#define	DSR_ON_LOOP		0x02
#define	DSR_ON_LOOP_SENDING	0x10
#define	DSR_TWO_CLK_MISSING	0x40
#define	DSR_ONE_CLK_MISSING	0x80










#define scc_reg_delay() \
    do {			\
	if (MACH_IS_MVME16x || MACH_IS_BVME6000 || MACH_IS_MVME147)	\
		__asm__ __volatile__ ( " nop; nop");			\
	else if (MACH_IS_ATARI)						\
		__asm__ __volatile__ ( "tstb %0" : : "g" (*_scc_del) : "cc" );\
    } while (0)

static unsigned char scc_shadow[2][16];




static __inline__ void _SCCwrite(
	struct scc_port *port,
	unsigned char *shadow,
	volatile unsigned char *_scc_del,
	int regno,
	unsigned char val, int final_delay )
{
	switch( regno ) {

	  case COMMAND_REG:
		
		*port->ctrlp = val;
		break;

	  case SYNC_CHAR_REG:
		
		if (shadow[INT_CTRL_REG] & ICR_OPTIONREG_SELECT) {
			*port->ctrlp = 15;
			shadow[INT_CTRL_REG] &= ~ICR_OPTIONREG_SELECT;
			scc_reg_delay();
			*port->ctrlp = shadow[INT_CTRL_REG];
			scc_reg_delay();
		}
		goto normal_case;
		
	  case SDLC_OPTION_REG:
		
		if (!(shadow[INT_CTRL_REG] & ICR_OPTIONREG_SELECT)) {
			*port->ctrlp = 15;
			shadow[INT_CTRL_REG] |= ICR_OPTIONREG_SELECT;
			scc_reg_delay();
			*port->ctrlp = shadow[INT_CTRL_REG];
			scc_reg_delay();
		}
		*port->ctrlp = 7;
		shadow[8] = val;	
		scc_reg_delay();
		*port->ctrlp = val;
		break;

	  case TX_DATA_REG:		
		
		if (MACH_IS_MVME16x || MACH_IS_BVME6000 || MACH_IS_MVME147)
		{
			*port->ctrlp = regno;
			scc_reg_delay();
			*port->ctrlp = val;
		}
		else
			*port->datap = val;
		break;

	  case MASTER_INT_CTRL:
		*port->ctrlp = regno;
		val &= 0x3f;	
		scc_shadow[0][regno] = val;
		scc_reg_delay();
		*port->ctrlp = val;
		break;

	  case DPLL_CTRL_REG:
		*port->ctrlp = regno;
		val &= 0x1f;			
		shadow[regno] = val;
		scc_reg_delay();
		*port->ctrlp = val;
		break;

	  case 1 ... 6:	
	  case 10 ... 13:
	  case 15:
	  normal_case:
		*port->ctrlp = regno;
		shadow[regno] = val;
		scc_reg_delay();
		*port->ctrlp = val;
		break;
		
	  default:
		printk( "Bad SCC write access to WR%d\n", regno );
		break;
		
	}

	if (final_delay)
		scc_reg_delay();
}


static __inline__ unsigned char _SCCread(
	struct scc_port *port,
	unsigned char *shadow,
	volatile unsigned char *_scc_del,
	int regno, int final_delay )
{
	unsigned char rv;

	switch( regno ) {

		
	  case STATUS_REG:
		rv = *port->ctrlp;
		break;

	  case INT_PENDING_REG:
		
		port = port->port_a;
		goto normal_case;

	  case RX_DATA_REG:
		
		if (MACH_IS_MVME16x || MACH_IS_BVME6000 || MACH_IS_MVME147)
		{
			*port->ctrlp = 8;
			scc_reg_delay();
			rv = *port->ctrlp;
		}
		else
			rv = *port->datap;
		break;

	  case CURR_VECTOR_REG:
		
		port = port->port_b;
		goto normal_case;
		
		
	  case 1 ... 7:
	  case 10 ... 15:
		return shadow[regno]; 

		
	  case SDLC_OPTION_REG:
		return shadow[8]; 

		
	  case MASTER_INT_CTRL:
		return scc_shadow[0][9]; 

	  default:
		printk( "Bad SCC read access to %cR%d\n", (regno & 16) ? 'R' : 'W',
				regno & ~16 );
		break;
		
	  case SPCOND_STATUS_REG:
	  case FS_FIFO_LOW_REG:
	  case FS_FIFO_HIGH_REG:
	  case DPLL_STATUS_REG:
	  normal_case:
		*port->ctrlp = regno & 0x0f;
		scc_reg_delay();
		rv = *port->ctrlp;
		break;
		
	}

	if (final_delay)
		scc_reg_delay();
	return rv;
}

#define SCC_ACCESS_INIT(port)						\
	unsigned char *_scc_shadow = &scc_shadow[port->channel][0]

#define	SCCwrite(reg,val)	_SCCwrite(port,_scc_shadow,scc_del,(reg),(val),1)
#define	SCCwrite_NB(reg,val)	_SCCwrite(port,_scc_shadow,scc_del,(reg),(val),0)
#define	SCCread(reg)		_SCCread(port,_scc_shadow,scc_del,(reg),1)
#define	SCCread_NB(reg)		_SCCread(port,_scc_shadow,scc_del,(reg),0)

#define SCCmod(reg,and,or)	SCCwrite((reg),(SCCread(reg)&(and))|(or))

#endif 
