




#define RC_TO_ISA(r)    ((((r)&0x07)<<1) | (((r)&~0x07)<<7))




#define RC_RI           0x100   
#define RC_DTR          0x100   
#define RC_BSR          0x101   
#define RC_CTOUT        0x101   




#define RC_BSR_TOUT     0x08     
#define RC_BSR_RINT     0x04     
#define RC_BSR_TINT     0x02     
#define RC_BSR_MINT     0x01     



#define RC_OSCFREQ      9830400


#define RC_ACK_MINT     0x81    
#define RC_ACK_RINT     0x82    
#define RC_ACK_TINT     0x84    


#define RC_ID           0x10


 
#define CD180_NCH       8       
#define CD180_TPC       16      
#define CD180_NFIFO	8	




#define CD180_GIVR      0x40    
#define CD180_GICR      0x41    
#define CD180_PILR1     0x61    
#define CD180_PILR2     0x62    
#define CD180_PILR3     0x63    
#define CD180_CAR       0x64    
#define CD180_GFRCR     0x6b    
#define CD180_PPRH      0x70    
#define CD180_PPRL      0x71    
#define CD180_RDR       0x78    
#define CD180_RCSR      0x7a    
#define CD180_TDR       0x7b    
#define CD180_EOIR      0x7f    




#define CD180_CCR       0x01    
#define CD180_IER       0x02    
#define CD180_COR1      0x03    
#define CD180_COR2      0x04    
#define CD180_COR3      0x05    
#define CD180_CCSR      0x06    
#define CD180_RDCR      0x07    
#define CD180_SCHR1     0x09    
#define CD180_SCHR2     0x0a    
#define CD180_SCHR3     0x0b    
#define CD180_SCHR4     0x0c    
#define CD180_MCOR1     0x10    
#define CD180_MCOR2     0x11    
#define CD180_MCR       0x12    
#define CD180_RTPR      0x18    
#define CD180_MSVR      0x28    
#define CD180_RBPRH     0x31    
#define CD180_RBPRL     0x32    
#define CD180_TBPRH     0x39    
#define CD180_TBPRL     0x3a    




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




#define CD180_C_ESC     0x00    
#define CD180_C_SBRK    0x81    
#define CD180_C_DELAY   0x82    
#define CD180_C_EBRK    0x83    
