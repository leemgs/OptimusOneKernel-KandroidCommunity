
#ifndef __MACH_MXC91231_H__
#define __MACH_MXC91231_H__


#define MXC91231_L2CC_BASE_ADDR		0x30000000
#define MXC91231_L2CC_BASE_ADDR_VIRT	0xF9000000
#define MXC91231_L2CC_SIZE		SZ_64K


#define MXC91231_AIPS1_BASE_ADDR	0x43F00000
#define MXC91231_AIPS1_BASE_ADDR_VIRT	0xFC000000
#define MXC91231_AIPS1_SIZE		SZ_1M

#define MXC91231_AIPS1_CTRL_BASE_ADDR	MXC91231_AIPS1_BASE_ADDR
#define MXC91231_MAX_BASE_ADDR		(MXC91231_AIPS1_BASE_ADDR + 0x04000)
#define MXC91231_EVTMON_BASE_ADDR	(MXC91231_AIPS1_BASE_ADDR + 0x08000)
#define MXC91231_CLKCTL_BASE_ADDR	(MXC91231_AIPS1_BASE_ADDR + 0x0C000)
#define MXC91231_ETB_SLOT4_BASE_ADDR	(MXC91231_AIPS1_BASE_ADDR + 0x10000)
#define MXC91231_ETB_SLOT5_BASE_ADDR	(MXC91231_AIPS1_BASE_ADDR + 0x14000)
#define MXC91231_ECT_CTIO_BASE_ADDR	(MXC91231_AIPS1_BASE_ADDR + 0x18000)
#define MXC91231_I2C_BASE_ADDR		(MXC91231_AIPS1_BASE_ADDR + 0x80000)
#define MXC91231_MU_BASE_ADDR		(MXC91231_AIPS1_BASE_ADDR + 0x88000)
#define MXC91231_UART1_BASE_ADDR	(MXC91231_AIPS1_BASE_ADDR + 0x90000)
#define MXC91231_UART2_BASE_ADDR	(MXC91231_AIPS1_BASE_ADDR + 0x94000)
#define MXC91231_DSM_BASE_ADDR		(MXC91231_AIPS1_BASE_ADDR + 0x98000)
#define MXC91231_OWIRE_BASE_ADDR	(MXC91231_AIPS1_BASE_ADDR + 0x9C000)
#define MXC91231_SSI1_BASE_ADDR		(MXC91231_AIPS1_BASE_ADDR + 0xA0000)
#define MXC91231_KPP_BASE_ADDR		(MXC91231_AIPS1_BASE_ADDR + 0xA8000)
#define MXC91231_IOMUX_AP_BASE_ADDR	(MXC91231_AIPS1_BASE_ADDR + 0xAC000)
#define MXC91231_CTI_AP_BASE_ADDR	(MXC91231_AIPS1_BASE_ADDR + 0xB8000)


#define MXC91231_AIPS2_BASE_ADDR	0x53F00000
#define MXC91231_AIPS2_BASE_ADDR_VIRT	0xFC100000
#define MXC91231_AIPS2_SIZE		SZ_1M

#define MXC91231_GEMK_BASE_ADDR		(MXC91231_AIPS2_BASE_ADDR + 0x8C000)
#define MXC91231_GPT1_BASE_ADDR		(MXC91231_AIPS2_BASE_ADDR + 0x90000)
#define MXC91231_EPIT1_AP_BASE_ADDR	(MXC91231_AIPS2_BASE_ADDR + 0x94000)
#define MXC91231_SCC_BASE_ADDR		(MXC91231_AIPS2_BASE_ADDR + 0xAC000)
#define MXC91231_RNGA_BASE_ADDR		(MXC91231_AIPS2_BASE_ADDR + 0xB0000)
#define MXC91231_IPU_CTRL_BASE_ADDR	(MXC91231_AIPS2_BASE_ADDR + 0xC0000)
#define MXC91231_AUDMUX_BASE_ADDR	(MXC91231_AIPS2_BASE_ADDR + 0xC4000)
#define MXC91231_EDIO_BASE_ADDR		(MXC91231_AIPS2_BASE_ADDR + 0xC8000)
#define MXC91231_GPIO1_AP_BASE_ADDR	(MXC91231_AIPS2_BASE_ADDR + 0xCC000)
#define MXC91231_GPIO2_AP_BASE_ADDR	(MXC91231_AIPS2_BASE_ADDR + 0xD0000)
#define MXC91231_SDMA_BASE_ADDR		(MXC91231_AIPS2_BASE_ADDR + 0xD4000)
#define MXC91231_RTC_BASE_ADDR		(MXC91231_AIPS2_BASE_ADDR + 0xD8000)
#define MXC91231_WDOG1_BASE_ADDR	(MXC91231_AIPS2_BASE_ADDR + 0xDC000)
#define MXC91231_PWM_BASE_ADDR		(MXC91231_AIPS2_BASE_ADDR + 0xE0000)
#define MXC91231_GPIO3_AP_BASE_ADDR	(MXC91231_AIPS2_BASE_ADDR + 0xE4000)
#define MXC91231_WDOG2_BASE_ADDR	(MXC91231_AIPS2_BASE_ADDR + 0xE8000)
#define MXC91231_RTIC_BASE_ADDR		(MXC91231_AIPS2_BASE_ADDR + 0xEC000)
#define MXC91231_LPMC_BASE_ADDR		(MXC91231_AIPS2_BASE_ADDR + 0xF0000)


#define MXC91231_SPBA0_BASE_ADDR	0x50000000
#define MXC91231_SPBA0_BASE_ADDR_VIRT	0xFC200000
#define MXC91231_SPBA0_SIZE		SZ_1M

#define MXC91231_MMC_SDHC1_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x04000)
#define MXC91231_MMC_SDHC2_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x08000)
#define MXC91231_UART3_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x0C000)
#define MXC91231_CSPI2_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x10000)
#define MXC91231_SSI2_BASE_ADDR		(MXC91231_SPBA0_BASE_ADDR + 0x14000)
#define MXC91231_SIM_BASE_ADDR		(MXC91231_SPBA0_BASE_ADDR + 0x18000)
#define MXC91231_IIM_BASE_ADDR		(MXC91231_SPBA0_BASE_ADDR + 0x1C000)
#define MXC91231_CTI_SDMA_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x20000)
#define MXC91231_USBOTG_CTRL_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x24000)
#define MXC91231_USBOTG_DATA_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x28000)
#define MXC91231_CSPI1_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x30000)
#define MXC91231_SPBA_CTRL_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x3C000)
#define MXC91231_IOMUX_COM_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x40000)
#define MXC91231_CRM_COM_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x44000)
#define MXC91231_CRM_AP_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x48000)
#define MXC91231_PLL0_BASE_ADDR		(MXC91231_SPBA0_BASE_ADDR + 0x4C000)
#define MXC91231_PLL1_BASE_ADDR		(MXC91231_SPBA0_BASE_ADDR + 0x50000)
#define MXC91231_PLL2_BASE_ADDR		(MXC91231_SPBA0_BASE_ADDR + 0x54000)
#define MXC91231_GPIO4_SH_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x58000)
#define MXC91231_HAC_BASE_ADDR		(MXC91231_SPBA0_BASE_ADDR + 0x5C000)
#define MXC91231_SAHARA_BASE_ADDR	(MXC91231_SPBA0_BASE_ADDR + 0x5C000)
#define MXC91231_PLL3_BASE_ADDR		(MXC91231_SPBA0_BASE_ADDR + 0x60000)


#define MXC91231_SPBA1_BASE_ADDR	0x52000000
#define MXC91231_SPBA1_BASE_ADDR_VIRT	0xFC300000
#define MXC91231_SPBA1_SIZE		SZ_1M

#define MXC91231_MQSPI_BASE_ADDR	(MXC91231_SPBA1_BASE_ADDR + 0x34000)
#define MXC91231_EL1T_BASE_ADDR		(MXC91231_SPBA1_BASE_ADDR + 0x38000)


#define MXC91231_SPBA_SDHC1		0x04
#define MXC91231_SPBA_SDHC2		0x08
#define MXC91231_SPBA_UART3		0x0C
#define MXC91231_SPBA_CSPI2		0x10
#define MXC91231_SPBA_SSI2		0x14
#define MXC91231_SPBA_SIM		0x18
#define MXC91231_SPBA_IIM		0x1C
#define MXC91231_SPBA_CTI_SDMA		0x20
#define MXC91231_SPBA_USBOTG_CTRL_REGS	0x24
#define MXC91231_SPBA_USBOTG_DATA_REGS	0x28
#define MXC91231_SPBA_CSPI1		0x30
#define MXC91231_SPBA_MQSPI		0x34
#define MXC91231_SPBA_EL1T		0x38
#define MXC91231_SPBA_IOMUX		0x40
#define MXC91231_SPBA_CRM_COM		0x44
#define MXC91231_SPBA_CRM_AP		0x48
#define MXC91231_SPBA_PLL0		0x4C
#define MXC91231_SPBA_PLL1		0x50
#define MXC91231_SPBA_PLL2		0x54
#define MXC91231_SPBA_GPIO4		0x58
#define MXC91231_SPBA_SAHARA		0x5C


#define MXC91231_ROMP_BASE_ADDR		0x60000000
#define MXC91231_ROMP_BASE_ADDR_VIRT	0xFC400000
#define MXC91231_ROMP_SIZE		SZ_64K

#define MXC91231_AVIC_BASE_ADDR		0x68000000
#define MXC91231_AVIC_BASE_ADDR_VIRT	0xFC410000
#define MXC91231_AVIC_SIZE		SZ_64K


#define MXC91231_X_MEMC_BASE_ADDR	0xB8000000
#define MXC91231_X_MEMC_BASE_ADDR_VIRT	0xFC420000
#define MXC91231_X_MEMC_SIZE		SZ_64K

#define MXC91231_NFC_BASE_ADDR		(MXC91231_X_MEMC_BASE_ADDR + 0x0000)
#define MXC91231_ESDCTL_BASE_ADDR	(MXC91231_X_MEMC_BASE_ADDR + 0x1000)
#define MXC91231_WEIM_BASE_ADDR		(MXC91231_X_MEMC_BASE_ADDR + 0x2000)
#define MXC91231_M3IF_BASE_ADDR		(MXC91231_X_MEMC_BASE_ADDR + 0x3000)
#define MXC91231_EMI_CTL_BASE_ADDR	(MXC91231_X_MEMC_BASE_ADDR + 0x4000)


#define MXC91231_FB_RAM_BASE_ADDR	0x78000000
#define MXC91231_FB_RAM_SIZE		SZ_256K
#define MXC91231_CSD0_BASE_ADDR		0x80000000
#define MXC91231_CSD1_BASE_ADDR		0x90000000
#define MXC91231_CS0_BASE_ADDR		0xA0000000
#define MXC91231_CS1_BASE_ADDR		0xA8000000
#define MXC91231_CS2_BASE_ADDR		0xB0000000
#define MXC91231_CS3_BASE_ADDR		0xB2000000
#define MXC91231_CS4_BASE_ADDR		0xB4000000
#define MXC91231_CS5_BASE_ADDR		0xB6000000


#define ADDRESS_IN_REGION(addr, start, size) \
	(((addr) >= (start)) && ((addr) < (start)+(size)))


#define MXC91231_IS_MODULE(addr, module) \
	ADDRESS_IN_REGION(addr, MXC91231_ ## module ## _BASE_ADDR, \
	                        MXC91231_ ## module ## _SIZE)


#define MXC91231_IO_ADDRESS(x) \
	(void __iomem *) \
	(MXC91231_IS_MODULE(x, L2CC) ? MXC91231_L2CC_IO_ADDRESS(x) : \
	 MXC91231_IS_MODULE(x, AIPS1) ? MXC91231_AIPS1_IO_ADDRESS(x) : \
	 MXC91231_IS_MODULE(x, AIPS2) ? MXC91231_AIPS2_IO_ADDRESS(x) : \
	 MXC91231_IS_MODULE(x, SPBA0) ? MXC91231_SPBA0_IO_ADDRESS(x) : \
	 MXC91231_IS_MODULE(x, SPBA1) ? MXC91231_SPBA1_IO_ADDRESS(x) : \
	 MXC91231_IS_MODULE(x, ROMP) ? MXC91231_ROMP_IO_ADDRESS(x) : \
	 MXC91231_IS_MODULE(x, AVIC) ? MXC91231_AVIC_IO_ADDRESS(x) : \
	 MXC91231_IS_MODULE(x, X_MEMC) ? MXC91231_X_MEMC_IO_ADDRESS(x) : \
	 0xDEADBEEF)



#define MXC91231_L2CC_IO_ADDRESS(x)  \
	(((x) - MXC91231_L2CC_BASE_ADDR) + MXC91231_L2CC_BASE_ADDR_VIRT)

#define MXC91231_AIPS1_IO_ADDRESS(x)  \
	(((x) - MXC91231_AIPS1_BASE_ADDR) + MXC91231_AIPS1_BASE_ADDR_VIRT)

#define MXC91231_SPBA0_IO_ADDRESS(x)  \
	(((x) - MXC91231_SPBA0_BASE_ADDR) + MXC91231_SPBA0_BASE_ADDR_VIRT)

#define MXC91231_SPBA1_IO_ADDRESS(x)  \
	(((x) - MXC91231_SPBA1_BASE_ADDR) + MXC91231_SPBA1_BASE_ADDR_VIRT)

#define MXC91231_AIPS2_IO_ADDRESS(x)  \
	(((x) - MXC91231_AIPS2_BASE_ADDR) + MXC91231_AIPS2_BASE_ADDR_VIRT)

#define MXC91231_ROMP_IO_ADDRESS(x)  \
	(((x) - MXC91231_ROMP_BASE_ADDR) + MXC91231_ROMP_BASE_ADDR_VIRT)

#define MXC91231_AVIC_IO_ADDRESS(x)  \
	(((x) - MXC91231_AVIC_BASE_ADDR) + MXC91231_AVIC_BASE_ADDR_VIRT)

#define MXC91231_X_MEMC_IO_ADDRESS(x)  \
	(((x) - MXC91231_X_MEMC_BASE_ADDR) + MXC91231_X_MEMC_BASE_ADDR_VIRT)


#define MXC91231_INT_GPIO3		0
#define MXC91231_INT_EL1T_CI		1
#define MXC91231_INT_EL1T_RFCI		2
#define MXC91231_INT_EL1T_RFI		3
#define MXC91231_INT_EL1T_MCU		4
#define MXC91231_INT_EL1T_IPI		5
#define MXC91231_INT_MU_GEN		6
#define MXC91231_INT_GPIO4		7
#define MXC91231_INT_MMC_SDHC2		8
#define MXC91231_INT_MMC_SDHC1		9
#define MXC91231_INT_I2C		10
#define MXC91231_INT_SSI2		11
#define MXC91231_INT_SSI1		12
#define MXC91231_INT_CSPI2		13
#define MXC91231_INT_CSPI1		14
#define MXC91231_INT_RTIC		15
#define MXC91231_INT_SAHARA		15
#define MXC91231_INT_HAC		15
#define MXC91231_INT_UART3_RX		16
#define MXC91231_INT_UART3_TX		17
#define MXC91231_INT_UART3_MINT		18
#define MXC91231_INT_ECT		19
#define MXC91231_INT_SIM_IPB		20
#define MXC91231_INT_SIM_DATA		21
#define MXC91231_INT_RNGA		22
#define MXC91231_INT_DSM_AP		23
#define MXC91231_INT_KPP		24
#define MXC91231_INT_RTC		25
#define MXC91231_INT_PWM		26
#define MXC91231_INT_GEMK_AP		27
#define MXC91231_INT_EPIT		28
#define MXC91231_INT_GPT		29
#define MXC91231_INT_UART2_RX		30
#define MXC91231_INT_UART2_TX		31
#define MXC91231_INT_UART2_MINT		32
#define MXC91231_INT_NANDFC		33
#define MXC91231_INT_SDMA		34
#define MXC91231_INT_USB_WAKEUP		35
#define MXC91231_INT_USB_SOF		36
#define MXC91231_INT_PMU_EVTMON		37
#define MXC91231_INT_USB_FUNC		38
#define MXC91231_INT_USB_DMA		39
#define MXC91231_INT_USB_CTRL		40
#define MXC91231_INT_IPU_ERR		41
#define MXC91231_INT_IPU_SYN		42
#define MXC91231_INT_UART1_RX		43
#define MXC91231_INT_UART1_TX		44
#define MXC91231_INT_UART1_MINT		45
#define MXC91231_INT_IIM		46
#define MXC91231_INT_MU_RX_OR		47
#define MXC91231_INT_MU_TX_OR		48
#define MXC91231_INT_SCC_SCM		49
#define MXC91231_INT_SCC_SMN		50
#define MXC91231_INT_GPIO2		51
#define MXC91231_INT_GPIO1		52
#define MXC91231_INT_MQSPI1		53
#define MXC91231_INT_MQSPI2		54
#define MXC91231_INT_WDOG2		55
#define MXC91231_INT_EXT_INT7		56
#define MXC91231_INT_EXT_INT6		57
#define MXC91231_INT_EXT_INT5		58
#define MXC91231_INT_EXT_INT4		59
#define MXC91231_INT_EXT_INT3		60
#define MXC91231_INT_EXT_INT2		61
#define MXC91231_INT_EXT_INT1		62
#define MXC91231_INT_EXT_INT0		63

#define MXC91231_MAX_INT_LINES		63
#define MXC91231_MAX_EXT_LINES		8

#endif 
