

#ifndef LGS8913_PRIV_H
#define LGS8913_PRIV_H

struct lgs8gxx_state {
	struct i2c_adapter *i2c;
	
	const struct lgs8gxx_config *config;
	struct dvb_frontend frontend;
	u16 curr_gi; 
};

#define SC_MASK		0x1C	
#define SC_QAM64	0x10	
#define SC_QAM32	0x0C	
#define SC_QAM16	0x08	
#define SC_QAM4NR	0x04	
#define SC_QAM4		0x00	

#define LGS_FEC_MASK	0x03	
#define LGS_FEC_0_4	0x00	
#define LGS_FEC_0_6	0x01	
#define LGS_FEC_0_8	0x02	

#define TIM_MASK	  0x20	
#define TIM_LONG	  0x20	
#define TIM_MIDDLE     0x00   

#define CF_MASK	0x80	
#define CF_EN	0x80	

#define GI_MASK	0x03	
#define GI_420	0x00	
#define GI_595	0x01	
#define GI_945	0x02	


#define TS_PARALLEL	0x00	
#define TS_SERIAL	0x01	
#define TS_CLK_NORMAL		0x00	
#define TS_CLK_INVERTED		0x02	
#define TS_CLK_GATED		0x00	
#define TS_CLK_FREERUN		0x04	


#endif
