#ifndef __ASM_ARCH_REGS_SSP_H
#define __ASM_ARCH_REGS_SSP_H



#define SSCR0		(0x00)  
#define SSCR1		(0x04)  
#define SSSR		(0x08)  
#define SSITR		(0x0C)  
#define SSDR		(0x10)  

#define SSTO		(0x28)  
#define SSPSP		(0x2C)  
#define SSTSA		(0x30)  
#define SSRSA		(0x34)  
#define SSTSS		(0x38)  
#define SSACD		(0x3C)  

#if defined(CONFIG_PXA3xx)
#define SSACDD		(0x40)	
#endif


#define SSCR0_DSS	(0x0000000f)	
#define SSCR0_DataSize(x)  ((x) - 1)	
#define SSCR0_FRF	(0x00000030)	
#define SSCR0_Motorola	(0x0 << 4)	
#define SSCR0_TI	(0x1 << 4)	
#define SSCR0_National	(0x2 << 4)	
#define SSCR0_ECS	(1 << 6)	
#define SSCR0_SSE	(1 << 7)	

#if defined(CONFIG_PXA25x)
#define SSCR0_SCR	(0x0000ff00)	
#define SSCR0_SerClkDiv(x) ((((x) - 2)/2) << 8) 
#elif defined(CONFIG_PXA27x) || defined(CONFIG_PXA3xx)
#define SSCR0_SCR	(0x000fff00)	
#define SSCR0_SerClkDiv(x) (((x) - 1) << 8) 
#endif

#if defined(CONFIG_PXA27x) || defined(CONFIG_PXA3xx)
#define SSCR0_EDSS	(1 << 20)	
#define SSCR0_NCS	(1 << 21)	
#define SSCR0_RIM	(1 << 22)	
#define SSCR0_TUM	(1 << 23)	
#define SSCR0_FRDC	(0x07000000)	
#define SSCR0_SlotsPerFrm(x) (((x) - 1) << 24)	
#define SSCR0_ACS	(1 << 30)	
#define SSCR0_MOD	(1 << 31)	
#endif

#if defined(CONFIG_PXA3xx)
#define SSCR0_FPCKE	(1 << 29)	
#endif

#define SSCR1_RIE	(1 << 0)	
#define SSCR1_TIE	(1 << 1)	
#define SSCR1_LBM	(1 << 2)	
#define SSCR1_SPO	(1 << 3)	
#define SSCR1_SPH	(1 << 4)	
#define SSCR1_MWDS	(1 << 5)	
#define SSCR1_TFT	(0x000003c0)	
#define SSCR1_TxTresh(x) (((x) - 1) << 6) 
#define SSCR1_RFT	(0x00003c00)	
#define SSCR1_RxTresh(x) (((x) - 1) << 10) 

#define SSSR_TNF	(1 << 2)	
#define SSSR_RNE	(1 << 3)	
#define SSSR_BSY	(1 << 4)	
#define SSSR_TFS	(1 << 5)	
#define SSSR_RFS	(1 << 6)	
#define SSSR_ROR	(1 << 7)	

#define SSCR0_TIM		(1 << 23)	
#define SSCR0_RIM		(1 << 22)	
#define SSCR0_NCS		(1 << 21)	
#define SSCR0_EDSS		(1 << 20)	


#define SSCR0_TISSP		(1 << 4)	
#define SSCR0_PSP		(3 << 4)	
#define SSCR1_TTELP		(1 << 31)	
#define SSCR1_TTE		(1 << 30)	
#define SSCR1_EBCEI		(1 << 29)	
#define SSCR1_SCFR		(1 << 28)	
#define SSCR1_ECRA		(1 << 27)	
#define SSCR1_ECRB		(1 << 26)	
#define SSCR1_SCLKDIR		(1 << 25)	
#define SSCR1_SFRMDIR		(1 << 24)	
#define SSCR1_RWOT		(1 << 23)	
#define SSCR1_TRAIL		(1 << 22)	
#define SSCR1_TSRE		(1 << 21)	
#define SSCR1_RSRE		(1 << 20)	
#define SSCR1_TINTE		(1 << 19)	
#define SSCR1_PINTE		(1 << 18)	
#define SSCR1_IFS		(1 << 16)	
#define SSCR1_STRF		(1 << 15)	
#define SSCR1_EFWR		(1 << 14)	

#define SSSR_BCE		(1 << 23)	
#define SSSR_CSS		(1 << 22)	
#define SSSR_TUR		(1 << 21)	
#define SSSR_EOC		(1 << 20)	
#define SSSR_TINT		(1 << 19)	
#define SSSR_PINT		(1 << 18)	

#if defined(CONFIG_PXA3xx)
#define SSPSP_EDMYSTOP(x)	((x) << 28)     
#define SSPSP_EDMYSTRT(x)	((x) << 26)     
#endif

#define SSPSP_FSRT		(1 << 25)	
#define SSPSP_DMYSTOP(x)	((x) << 23)	
#define SSPSP_SFRMWDTH(x)	((x) << 16)	
#define SSPSP_SFRMDLY(x)	((x) << 9)	
#define SSPSP_DMYSTRT(x)	((x) << 7)	
#define SSPSP_STRTDLY(x)	((x) << 4)	
#define SSPSP_ETDS		(1 << 3)	
#define SSPSP_SFRMP		(1 << 2)	
#define SSPSP_SCMODE(x)		((x) << 0)	

#define SSACD_SCDB		(1 << 3)	
#define SSACD_ACPS(x)		((x) << 4)	
#define SSACD_ACDS(x)		((x) << 0)	
#if defined(CONFIG_PXA3xx)
#define SSACD_SCDX8		(1 << 7)	
#endif


#endif 
