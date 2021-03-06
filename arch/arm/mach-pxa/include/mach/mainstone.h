

#ifndef ASM_ARCH_MAINSTONE_H
#define ASM_ARCH_MAINSTONE_H

#define MST_ETH_PHYS		PXA_CS4_PHYS

#define MST_FPGA_PHYS		PXA_CS2_PHYS
#define MST_FPGA_VIRT		(0xf0000000)
#define MST_P2V(x)		((x) - MST_FPGA_PHYS + MST_FPGA_VIRT)
#define MST_V2P(x)		((x) - MST_FPGA_VIRT + MST_FPGA_PHYS)

#ifndef __ASSEMBLY__
# define __MST_REG(x)		(*((volatile unsigned long *)MST_P2V(x)))
#else
# define __MST_REG(x)		MST_P2V(x)
#endif



#define MST_LEDDAT1		__MST_REG(0x08000010)
#define MST_LEDDAT2		__MST_REG(0x08000014)
#define MST_LEDCTRL		__MST_REG(0x08000040)
#define MST_GPSWR		__MST_REG(0x08000060)
#define MST_MSCWR1		__MST_REG(0x08000080)
#define MST_MSCWR2		__MST_REG(0x08000084)
#define MST_MSCWR3		__MST_REG(0x08000088)
#define MST_MSCRD		__MST_REG(0x08000090)
#define MST_INTMSKENA		__MST_REG(0x080000c0)
#define MST_INTSETCLR		__MST_REG(0x080000d0)
#define MST_PCMCIA0		__MST_REG(0x080000e0)
#define MST_PCMCIA1		__MST_REG(0x080000e4)

#define MST_MSCWR1_CAMERA_ON	(1 << 15)  
#define MST_MSCWR1_CAMERA_SEL	(1 << 14)  
#define MST_MSCWR1_LCD_CTL	(1 << 13)  
#define MST_MSCWR1_MS_ON	(1 << 12)  
#define MST_MSCWR1_MMC_ON	(1 << 11)  
#define MST_MSCWR1_MS_SEL	(1 << 10)  
#define MST_MSCWR1_BB_SEL	(1 << 9)   
#define MST_MSCWR1_BT_ON	(1 << 8)   
#define MST_MSCWR1_BTDTR	(1 << 7)   

#define MST_MSCWR1_IRDA_MASK	(3 << 5)   
#define MST_MSCWR1_IRDA_FULL	(0 << 5)   
#define MST_MSCWR1_IRDA_OFF	(1 << 5)   
#define MST_MSCWR1_IRDA_MED	(2 << 5)   
#define MST_MSCWR1_IRDA_LOW	(3 << 5)   

#define MST_MSCWR1_IRDA_FIR	(1 << 4)   
#define MST_MSCWR1_GREENLED	(1 << 3)   
#define MST_MSCWR1_PDC_CTL	(1 << 2)   
#define MST_MSCWR1_MTR_ON	(1 << 1)   
#define MST_MSCWR1_SYSRESET	(1 << 0)   

#define MST_MSCWR2_USB_OTG_RST	(1 << 6)   
#define MST_MSCWR2_USB_OTG_SEL	(1 << 5)   
#define MST_MSCWR2_nUSBC_SC	(1 << 4)   
#define MST_MSCWR2_I2S_SPKROFF	(1 << 3)   
#define MST_MSCWR2_AC97_SPKROFF	(1 << 2)   
#define MST_MSCWR2_RADIO_PWR	(1 << 1)   
#define MST_MSCWR2_RADIO_WAKE	(1 << 0)   

#define MST_MSCWR3_GPIO_RESET_EN	(1 << 2) 
#define MST_MSCWR3_GPIO_RESET		(1 << 1) 
#define MST_MSCWR3_COMMS_SW_RESET	(1 << 0) 

#define MST_MSCRD_nPENIRQ	(1 << 9)   
#define MST_MSCRD_nMEMSTK_CD	(1 << 8)   
#define MST_MSCRD_nMMC_CD	(1 << 7)   
#define MST_MSCRD_nUSIM_CD	(1 << 6)   
#define MST_MSCRD_USB_CBL	(1 << 5)   
#define MST_MSCRD_TS_BUSY	(1 << 4)   
#define MST_MSCRD_BTDSR		(1 << 3)   
#define MST_MSCRD_BTRI		(1 << 2)   
#define MST_MSCRD_BTDCD		(1 << 1)   
#define MST_MSCRD_nMMC_WP	(1 << 0)   

#define MST_INT_S1_IRQ		(1 << 15)  
#define MST_INT_S1_STSCHG	(1 << 14)  
#define MST_INT_S1_CD		(1 << 13)  
#define MST_INT_S0_IRQ		(1 << 11)  
#define MST_INT_S0_STSCHG	(1 << 10)  
#define MST_INT_S0_CD		(1 << 9)   
#define MST_INT_nEXBRD_INT	(1 << 7)   
#define MST_INT_MSINS		(1 << 6)   
#define MST_INT_PENIRQ		(1 << 5)   
#define MST_INT_AC97		(1 << 4)   
#define MST_INT_ETHERNET	(1 << 3)   
#define MST_INT_USBC		(1 << 2)   
#define MST_INT_USIM		(1 << 1)   
#define MST_INT_MMC		(1 << 0)   

#define MST_PCMCIA_nIRQ		(1 << 10)  
#define MST_PCMCIA_nSPKR_BVD2	(1 << 9)   
#define MST_PCMCIA_nSTSCHG_BVD1	(1 << 8)   
#define MST_PCMCIA_nVS2		(1 << 7)   
#define MST_PCMCIA_nVS1		(1 << 6)   
#define MST_PCMCIA_nCD		(1 << 5)   
#define MST_PCMCIA_RESET	(1 << 4)   
#define MST_PCMCIA_PWR_MASK	(0x000f)   

#define MST_PCMCIA_PWR_VPP_0    0x0	   
#define MST_PCMCIA_PWR_VPP_120  0x2 	   
#define MST_PCMCIA_PWR_VPP_VCC  0x1	   
#define MST_PCMCIA_PWR_VCC_0    0x0	   
#define MST_PCMCIA_PWR_VCC_33   0x8	   
#define MST_PCMCIA_PWR_VCC_50   0x4	   

#endif
