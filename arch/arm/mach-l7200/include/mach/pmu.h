




#define PMU_OFF   0x00050000  



#define PMU_START (IO_START + PMU_OFF)  
#define PMU_BASE  (IO_BASE  + PMU_OFF)  




typedef struct {
     unsigned int CURRENT;  
     unsigned int NEXT;     
     unsigned int reserved;
     unsigned int RUN;      
     unsigned int COMM;     
     unsigned int SDRAM;    
} pmu_interface;

#define PMU ((volatile pmu_interface *)(PMU_BASE))




#define GET_TRANSOP(reg)  ((reg >> 25) & 0x03) 
#define GET_OSCEN(reg)    ((reg >> 16) & 0x01)
#define GET_OSCMUX(reg)   ((reg >> 15) & 0x01)
#define GET_PLLMUL(reg)   ((reg >>  9) & 0x3f) 
#define GET_PLLEN(reg)    ((reg >>  8) & 0x01)
#define GET_PLLMUX(reg)   ((reg >>  7) & 0x01)
#define GET_BCLK_DIV(reg) ((reg >>  3) & 0x03) 
#define GET_SDRB_SEL(reg) ((reg >>  2) & 0x01)
#define GET_SDRF_SEL(reg) ((reg >>  1) & 0x01)
#define GET_FASTBUS(reg)  (reg & 0x1)



#define CFG_NEXT_CLOCKRECOVERY ((PMU->NEXT >> 18) & 0x7f)   
#define CFG_NEXT_INTRET        ((PMU->NEXT >> 17) & 0x01)
#define CFG_NEXT_SDR_STOP      ((PMU->NEXT >>  6) & 0x01)
#define CFG_NEXT_SYSCLKEN      ((PMU->NEXT >>  5) & 0x01)



#define TRANSOP_NOP      0<<25  
#define NOCHANGE_STALL   1<<25
#define CHANGE_NOSTALL   2<<25
#define CHANGE_STALL     3<<25

#define INTRET           1<<17
#define OSCEN            1<<16
#define OSCMUX           1<<15



#define PLLMUL_0         0<<9         
#define PLLMUL_1         1<<9         
#define PLLMUL_5         5<<9         
#define PLLMUL_10       10<<9         
#define PLLMUL_18       18<<9         
#define PLLMUL_20       20<<9         
#define PLLMUL_32       32<<9         
#define PLLMUL_35       35<<9         
#define PLLMUL_36       36<<9         
#define PLLMUL_39       39<<9         
#define PLLMUL_40       40<<9         



#define CRCLOCK_1        1<<18
#define CRCLOCK_2        2<<18
#define CRCLOCK_4        4<<18
#define CRCLOCK_8        8<<18
#define CRCLOCK_16      16<<18
#define CRCLOCK_32      32<<18
#define CRCLOCK_63      63<<18
#define CRCLOCK_127    127<<18

#define PLLEN            1<<8
#define PLLMUX           1<<7
#define SDR_STOP         1<<6
#define SYSCLKEN         1<<5

#define BCLK_DIV_4       2<<3
#define BCLK_DIV_2       1<<3
#define BCLK_DIV_1       0<<3

#define SDRB_SEL         1<<2
#define SDRF_SEL         1<<1
#define FASTBUS          1<<0




#define SDRREFFQ         1<<0  
#define SDRREFACK        1<<1  
#define SDRSTOPRQ        1<<2  
#define SDRSTOPACK       1<<3  
#define PICEN            1<<4  
#define PICTEST          1<<5

#define GET_SDRREFFQ    ((PMU->SDRAM >> 0) & 0x01)
#define GET_SDRREFACK   ((PMU->SDRAM >> 1) & 0x01) 
#define GET_SDRSTOPRQ   ((PMU->SDRAM >> 2) & 0x01)
#define GET_SDRSTOPACK  ((PMU->SDRAM >> 3) & 0x01) 
#define GET_PICEN       ((PMU->SDRAM >> 4) & 0x01)
#define GET_PICTEST     ((PMU->SDRAM >> 5) & 0x01)
