




#define SIB_OFF   0x00040000  



#define SIB_START (IO_START + SIB_OFF) 
#define SIB_BASE  (IO_BASE  + SIB_OFF) 





typedef struct
{
     unsigned int MCCR;    
     unsigned int RES1;    
     unsigned int MCDR0;   
     unsigned int MCDR1;   
     unsigned int MCDR2;   
     unsigned int RES2;    
     unsigned int MCSR;    
} SIB_Interface;

#define SIB ((volatile SIB_Interface *) (SIB_BASE))



#define INTERNAL_FREQ   9216000  
#define AUDIO_FREQ         5000  
#define TELECOM_FREQ       5000  

#define AUDIO_DIVIDE    (INTERNAL_FREQ / (32 * AUDIO_FREQ))
#define TELECOM_DIVIDE  (INTERNAL_FREQ / (32 * TELECOM_FREQ))

#define MCCR_ASD57      AUDIO_DIVIDE
#define MCCR_TSD57      (TELECOM_DIVIDE << 8)
#define MCCR_MCE        (1 << 16)             
#define MCCR_ECS        (1 << 17)             
#define MCCR_ADM        (1 << 18)             
#define MCCR_PMC        (1 << 26)             


#define GET_ASD ((SIB->MCCR >>  0) & 0x3f) 
#define GET_TSD ((SIB->MCCR >>  8) & 0x3f) 
#define GET_MCE ((SIB->MCCR >> 16) & 0x01) 
#define GET_ECS ((SIB->MCCR >> 17) & 0x01) 
#define GET_ADM ((SIB->MCCR >> 18) & 0x01) 
#define GET_TTM ((SIB->MCCR >> 19) & 0x01)  
#define GET_TRM ((SIB->MCCR >> 20) & 0x01) 
#define GET_ATM ((SIB->MCCR >> 21) & 0x01)  
#define GET_ARM ((SIB->MCCR >> 22) & 0x01) 
#define GET_LBM ((SIB->MCCR >> 23) & 0x01) 
#define GET_ECP ((SIB->MCCR >> 24) & 0x03) 
#define GET_PMC ((SIB->MCCR >> 26) & 0x01) 
#define GET_ERI ((SIB->MCCR >> 27) & 0x01) 
#define GET_EWI ((SIB->MCCR >> 28) & 0x01) 



#define AUDIO_RECV     ((SIB->MCDR0 >> 4) & 0xfff)
#define AUDIO_WRITE(v) ((SIB->MCDR0 = (v & 0xfff) << 4))



#define TELECOM_RECV     ((SIB->MCDR1 >> 2) & 032fff)
#define TELECOM_WRITE(v) ((SIB->MCDR1 = (v & 0x3fff) << 2))




#define MCSR_ATU (1 << 4)  
#define MCSR_ARO (1 << 5)  
#define MCSR_TTU (1 << 6)  
#define MCSR_TRO (1 << 7)  

#define MCSR_CLEAR_UNDERUN_BITS (MCSR_ATU | MCSR_ARO | MCSR_TTU | MCSR_TRO)


#define GET_ATS ((SIB->MCSR >>  0) & 0x01) 
#define GET_ARS ((SIB->MCSR >>  1) & 0x01) 
#define GET_TTS ((SIB->MCSR >>  2) & 0x01) 
#define GET_TRS ((SIB->MCSR >>  3) & 0x01) 
#define GET_ATU ((SIB->MCSR >>  4) & 0x01) 
#define GET_ARO ((SIB->MCSR >>  5) & 0x01) 
#define GET_TTU ((SIB->MCSR >>  6) & 0x01) 
#define GET_TRO ((SIB->MCSR >>  7) & 0x01) 
#define GET_ANF ((SIB->MCSR >>  8) & 0x01) 
#define GET_ANE ((SIB->MCSR >>  9) & 0x01) 
#define GET_TNF ((SIB->MCSR >> 10) & 0x01) 
#define GET_TNE ((SIB->MCSR >> 11) & 0x01) 
#define GET_CWC ((SIB->MCSR >> 12) & 0x01) 
#define GET_CRC ((SIB->MCSR >> 13) & 0x01) 
#define GET_ACE ((SIB->MCSR >> 14) & 0x01) 
#define GET_TCE ((SIB->MCSR >> 15) & 0x01) 



#define MCDR2_rW               (1 << 16)

#define WRITE_MCDR2(reg, data) (SIB->MCDR2 =((reg<<17)|MCDR2_rW|(data&0xffff)))
#define MCDR2_WRITE_COMPLETE   GET_CWC

#define INITIATE_MCDR2_READ(reg) (SIB->MCDR2 = (reg << 17))
#define MCDR2_READ_COMPLETE      GET_CRC
#define MCDR2_READ               (SIB->MCDR2 & 0xffff)
