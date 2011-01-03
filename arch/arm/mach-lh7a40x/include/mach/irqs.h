




#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H


#define FIQ_START	80

#if defined (CONFIG_ARCH_LH7A400)

  

# define IRQ_GPIO0FIQ	0	
# define IRQ_BLINT	1	
# define IRQ_WEINT	2	
# define IRQ_MCINT	3	

  

# define IRQ_CSINT	4	
# define IRQ_GPIO1INTR	5	
# define IRQ_GPIO2INTR	6	
# define IRQ_GPIO3INTR	7	
# define IRQ_T1UI	8	
# define IRQ_T2UI	9	
# define IRQ_RTCMI	10
# define IRQ_TINTR	11	
# define IRQ_UART1INTR	12
# define IRQ_UART2INTR	13
# define IRQ_LCDINTR	14
# define IRQ_SSIEOT	15	
# define IRQ_UART3INTR	16
# define IRQ_SCIINTR	17	
# define IRQ_AACINTR	18	
# define IRQ_MMCINTR	19	
# define IRQ_USBINTR	20
# define IRQ_DMAINTR	21
# define IRQ_T3UI	22	
# define IRQ_GPIO4INTR	23	
# define IRQ_GPIO5INTR	24	
# define IRQ_GPIO6INTR	25	
# define IRQ_GPIO7INTR	26	
# define IRQ_BMIINTR	27	

# define NR_IRQ_CPU	28	

	
# define IRQ_TO_GPIO(i)  ((i) \
	- (((i) > IRQ_GPIO3INTR) ? IRQ_GPIO4INTR - IRQ_GPIO3INTR - 1 : 0)\
	- (((i) > IRQ_GPIO0INTR) ? IRQ_GPIO1INTR - IRQ_GPIO0INTR - 1 : 0))

#endif

#if defined (CONFIG_ARCH_LH7A404)

# define IRQ_BROWN	0	
# define IRQ_WDTINTR	1	
# define IRQ_COMMRX	2	
# define IRQ_COMMTX	3	
# define IRQ_T1UI	4	
# define IRQ_T2UI	5	
# define IRQ_CSINT	6	
# define IRQ_DMAM2P0	7	
# define IRQ_DMAM2P1	8
# define IRQ_DMAM2P2	9
# define IRQ_DMAM2P3	10
# define IRQ_DMAM2P4	11
# define IRQ_DMAM2P5	12
# define IRQ_DMAM2P6	13
# define IRQ_DMAM2P7	14
# define IRQ_DMAM2P8	15
# define IRQ_DMAM2P9	16
# define IRQ_DMAM2M0	17	
# define IRQ_DMAM2M1	18
# define IRQ_GPIO0INTR	19	
# define IRQ_GPIO1INTR	20
# define IRQ_GPIO2INTR	21
# define IRQ_GPIO3INTR	22
# define IRQ_SOFT_V1_23	23	
# define IRQ_SOFT_V1_24	24
# define IRQ_SOFT_V1_25	25
# define IRQ_SOFT_V1_26	26
# define IRQ_SOFT_V1_27	27
# define IRQ_SOFT_V1_28	28
# define IRQ_SOFT_V1_29	29
# define IRQ_SOFT_V1_30	30
# define IRQ_SOFT_V1_31	31

# define IRQ_BLINT	32	
# define IRQ_BMIINTR	33	
# define IRQ_MCINTR	34	
# define IRQ_TINTR	35	
# define IRQ_WEINT	36	
# define IRQ_RTCMI	37	
# define IRQ_UART1INTR	38	
# define IRQ_UART1ERR	39	
# define IRQ_UART2INTR	40	
# define IRQ_UART2ERR	41	
# define IRQ_UART3INTR	42	
# define IRQ_UART3ERR	43	
# define IRQ_SCIINTR	44	
# define IRQ_TSCINTR	45	
# define IRQ_KMIINTR	46	
# define IRQ_GPIO4INTR	47	
# define IRQ_GPIO5INTR	48
# define IRQ_GPIO6INTR	49
# define IRQ_GPIO7INTR	50
# define IRQ_T3UI	51	
# define IRQ_LCDINTR	52	
# define IRQ_SSPINTR	53	
# define IRQ_SDINTR	54	
# define IRQ_USBINTR	55	
# define IRQ_USHINTR	56	
# define IRQ_SOFT_V2_25	57	
# define IRQ_SOFT_V2_26	58
# define IRQ_SOFT_V2_27	59
# define IRQ_SOFT_V2_28	60
# define IRQ_SOFT_V2_29	61
# define IRQ_SOFT_V2_30	62
# define IRQ_SOFT_V2_31	63

# define NR_IRQ_CPU	64	

	
# define IRQ_TO_GPIO(i)  ((i) \
	- (((i) > IRQ_GPIO3INTR) ? IRQ_GPIO4INTR - IRQ_GPIO3INTR - 1 : 0)\
	- IRQ_GPIO0INTR)

			
# define VA_VECTORED	0x100	
# define VA_VIC1DEFAULT	0x200	
# define VA_VIC2DEFAULT	0x400	

#endif

  

#if !defined (IRQ_GPIO0INTR)
# define IRQ_GPIO0INTR	IRQ_GPIO0FIQ
#endif
#define IRQ_TICK	IRQ_TINTR
#define IRQ_PCC1_RDY	IRQ_GPIO6INTR	
#define IRQ_PCC2_RDY	IRQ_GPIO7INTR	
#define IRQ_USB		IRQ_USBINTR	

#ifdef CONFIG_MACH_KEV7A400
# define IRQ_TS		IRQ_GPIOFIQ	
# define IRQ_CPLD	IRQ_GPIO1INTR	
# define IRQ_PCC1_CD	IRQ_GPIO_F2	
# define IRQ_PCC2_CD	IRQ_GPIO_F3	
#endif

#if defined (CONFIG_MACH_LPD7A400) || defined (CONFIG_MACH_LPD7A404)
# define IRQ_CPLD_V28	IRQ_GPIO7INTR	
# define IRQ_CPLD_V34	IRQ_GPIO3INTR	
#endif

  

#define IRQ_BOARD_START NR_IRQ_CPU

#ifdef CONFIG_MACH_KEV7A400
# define IRQ_KEV7A400_CPLD	IRQ_BOARD_START
# define NR_IRQ_BOARD		5
# define IRQ_KEV7A400_MMC_CD	IRQ_KEV7A400_CPLD + 0	
# define IRQ_KEV7A400_RI2	IRQ_KEV7A400_CPLD + 1	
# define IRQ_KEV7A400_IDE_CF	IRQ_KEV7A400_CPLD + 2	
# define IRQ_KEV7A400_ETH_INT	IRQ_KEV7A400_CPLD + 3	
# define IRQ_KEV7A400_INT	IRQ_KEV7A400_CPLD + 4
#endif

#if defined (CONFIG_MACH_LPD7A400) || defined (CONFIG_MACH_LPD7A404)
# define IRQ_LPD7A40X_CPLD	IRQ_BOARD_START
# define NR_IRQ_BOARD		2
# define IRQ_LPD7A40X_ETH_INT	IRQ_LPD7A40X_CPLD + 0	
# define IRQ_LPD7A400_TS	IRQ_LPD7A40X_CPLD + 1	
#endif

#if defined (CONFIG_MACH_LPD7A400)
# define IRQ_TOUCH		IRQ_LPD7A400_TS
#endif

#define NR_IRQS		(NR_IRQ_CPU + NR_IRQ_BOARD)

#endif
