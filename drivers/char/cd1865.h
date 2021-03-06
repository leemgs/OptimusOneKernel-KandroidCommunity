






#define SX_ACK_MINT     0x75    
#define SX_ACK_TINT     0x76    
#define SX_ACK_RINT     0x77    


#define SX_ID           0x10


 
#define CD186x_NCH       8       
#define CD186x_TPC       16      
#define CD186x_NFIFO	 8	 




#define CD186x_GIVR      0x40    
#define CD186x_GICR      0x41    
#define CD186x_PILR1     0x61    
#define CD186x_PILR2     0x62    
#define CD186x_PILR3     0x63    
#define CD186x_CAR       0x64    
#define CD186x_SRSR      0x65    
#define CD186x_GFRCR     0x6b    
#define CD186x_PPRH      0x70    
#define CD186x_PPRL      0x71    
#define CD186x_RDR       0x78    
#define CD186x_RCSR      0x7a    
#define CD186x_TDR       0x7b    
#define CD186x_EOIR      0x7f    
#define CD186x_MRAR      0x75    
#define CD186x_TRAR      0x76    
#define CD186x_RRAR      0x77    
#define CD186x_SRCR      0x66    



#define CD186x_CCR       0x01    
#define CD186x_IER       0x02    
#define CD186x_COR1      0x03    
#define CD186x_COR2      0x04    
#define CD186x_COR3      0x05    
#define CD186x_CCSR      0x06    
#define CD186x_RDCR      0x07    
#define CD186x_SCHR1     0x09    
#define CD186x_SCHR2     0x0a    
#define CD186x_SCHR3     0x0b    
#define CD186x_SCHR4     0x0c    
#define CD186x_MCOR1     0x10    
#define CD186x_MCOR2     0x11    
#define CD186x_MCR       0x12    
#define CD186x_RTPR      0x18    
#define CD186x_MSVR      0x28    
#define CD186x_MSVRTS    0x29    
#define CD186x_MSVDTR    0x2a    
#define CD186x_RBPRH     0x31    
#define CD186x_RBPRL     0x32    
#define CD186x_TBPRH     0x39    
#define CD186x_TBPRL     0x3a    




#define GIVR_ITMASK     0x07     
#define  GIVR_IT_MODEM   0x01    
#define  GIVR_IT_TX      0x02    
#define  GIVR_IT_RCV     0x03    
#define  GIVR_IT_REXC    0x07    



 
#define GICR_CHAN       0x1c    
#define GICR_CHAN_OFF   2       




#define CAR_CHAN        0x07    
#define CAR_A7          0x08    




#define RCSR_TOUT       0x80    
#define RCSR_SCDET      0x70    
#define  RCSR_NO_SC      0x00   
#define  RCSR_SC_1       0x10   
#define  RCSR_SC_2       0x20   
#define  RCSR_SC_3       0x30   
#define  RCSR_SC_4       0x40   
#define RCSR_BREAK      0x08    
#define RCSR_PE         0x04    
#define RCSR_FE         0x02    
#define RCSR_OE         0x01    




#define CCR_HARDRESET   0x81    

#define CCR_SOFTRESET   0x80    

#define CCR_CORCHG1     0x42    
#define CCR_CORCHG2     0x44    
#define CCR_CORCHG3     0x48    

#define CCR_SSCH1       0x21    

#define CCR_SSCH2       0x22    

#define CCR_SSCH3       0x23    

#define CCR_SSCH4       0x24    

#define CCR_TXEN        0x18    
#define CCR_RXEN        0x12    

#define CCR_TXDIS       0x14    
#define CCR_RXDIS       0x11    




#define IER_DSR         0x80    
#define IER_CD          0x40    
#define IER_CTS         0x20    
#define IER_RXD         0x10    
#define IER_RXSC        0x08    
#define IER_TXRDY       0x04    
#define IER_TXEMPTY     0x02    
#define IER_RET         0x01    




#define COR1_ODDP       0x80    
#define COR1_PARMODE    0x60    
#define  COR1_NOPAR      0x00   
#define  COR1_FORCEPAR   0x20   
#define  COR1_NORMPAR    0x40   
#define COR1_IGNORE     0x10    
#define COR1_STOPBITS   0x0c    
#define  COR1_1SB        0x00   
#define  COR1_15SB       0x04   
#define  COR1_2SB        0x08   
#define COR1_CHARLEN    0x03    
#define  COR1_5BITS      0x00   
#define  COR1_6BITS      0x01   
#define  COR1_7BITS      0x02   
#define  COR1_8BITS      0x03   




#define COR2_IXM        0x80    
#define COR2_TXIBE      0x40    
#define COR2_ETC        0x20    
#define COR2_LLM        0x10    
#define COR2_RLM        0x08    
#define COR2_RTSAO      0x04    
#define COR2_CTSAE      0x02    
#define COR2_DSRAE      0x01    




#define COR3_XONCH      0x80    
#define COR3_XOFFCH     0x40    
#define COR3_FCT        0x20    
#define COR3_SCDE       0x10    
#define COR3_RXTH       0x0f    




#define CCSR_RXEN       0x80    
#define CCSR_RXFLOFF    0x40    
#define CCSR_RXFLON     0x20    
#define CCSR_TXEN       0x08    
#define CCSR_TXFLOFF    0x04    
#define CCSR_TXFLON     0x02    




#define MCOR1_DSRZD     0x80    
#define MCOR1_CDZD      0x40    
#define MCOR1_CTSZD     0x20    
#define MCOR1_DTRTH     0x0f    
#define  MCOR1_NODTRFC   0x0     




#define MCOR2_DSROD     0x80    
#define MCOR2_CDOD      0x40    
#define MCOR2_CTSOD     0x20    



#define MCR_DSRCHG      0x80    
#define MCR_CDCHG       0x40    
#define MCR_CTSCHG      0x20    




#define MSVR_DSR        0x80    
#define MSVR_CD         0x40    
#define MSVR_CTS        0x20    
#define MSVR_DTR        0x02    
#define MSVR_RTS        0x01    




#define CD186x_C_ESC     0x00    
#define CD186x_C_SBRK    0x81    
#define CD186x_C_DELAY   0x82    
#define CD186x_C_EBRK    0x83    

#define SRSR_RREQint     0x10    
#define SRSR_TREQint     0x04    
#define SRSR_MREQint     0x01    



#define SRCR_PKGTYPE    0x80
#define SRCR_REGACKEN   0x40
#define SRCR_DAISYEN    0x20
#define SRCR_GLOBPRI    0x10
#define SRCR_UNFAIR     0x08
#define SRCR_AUTOPRI    0x02
#define SRCR_PRISEL     0x01


