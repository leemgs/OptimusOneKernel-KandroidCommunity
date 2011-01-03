

#ifndef __MACH_U300_REGS_H
#define __MACH_U300_REGS_H




#define U300_NAND_CS0_PHYS_BASE		0x80000000
#define U300_NAND_CS0_VIRT_BASE		0xff040000


#define U300_NAND_IF_PHYS_BASE		0x9f800000
#define U300_NAND_IF_VIRT_BASE		0xff030000


#define U300_AHB_PER_PHYS_BASE		0xa0000000
#define U300_AHB_PER_VIRT_BASE		0xff010000


#define U300_FAST_PER_PHYS_BASE		0xc0000000
#define U300_FAST_PER_VIRT_BASE		0xff020000


#define U300_SLOW_PER_PHYS_BASE		0xc0010000
#define U300_SLOW_PER_VIRT_BASE		0xff000000


#define U300_BOOTROM_PHYS_BASE		0xffff0000
#define U300_BOOTROM_VIRT_BASE		0xffff0000


#ifdef CONFIG_MACH_U300_BS335
#define U300_SEMI_CONFIG_BASE		0x2FFE0000
#else
#define U300_SEMI_CONFIG_BASE		0x30000000
#endif






#define U300_AHB_BRIDGE_BASE		(U300_AHB_PER_PHYS_BASE+0x0000)


#define U300_INTCON0_BASE		(U300_AHB_PER_PHYS_BASE+0x1000)
#define U300_INTCON0_VBASE		(U300_AHB_PER_VIRT_BASE+0x1000)


#define U300_INTCON1_BASE		(U300_AHB_PER_PHYS_BASE+0x2000)
#define U300_INTCON1_VBASE		(U300_AHB_PER_VIRT_BASE+0x2000)


#define U300_MSPRO_BASE			(U300_AHB_PER_PHYS_BASE+0x3000)


#define U300_EMIF_CFG_BASE		(U300_AHB_PER_PHYS_BASE+0x4000)





#define U300_FAST_BRIDGE_BASE		(U300_FAST_PER_PHYS_BASE+0x0000)


#define U300_MMCSD_BASE			(U300_FAST_PER_PHYS_BASE+0x1000)


#define U300_PCM_I2S0_BASE		(U300_FAST_PER_PHYS_BASE+0x2000)


#define U300_PCM_I2S1_BASE		(U300_FAST_PER_PHYS_BASE+0x3000)


#define U300_I2C0_BASE			(U300_FAST_PER_PHYS_BASE+0x4000)


#define U300_I2C1_BASE			(U300_FAST_PER_PHYS_BASE+0x5000)


#define U300_SPI_BASE			(U300_FAST_PER_PHYS_BASE+0x6000)

#ifdef CONFIG_MACH_U300_BS335

#define U300_UART1_BASE			(U300_SLOW_PER_PHYS_BASE+0x7000)
#endif




#define U300_SLOW_BRIDGE_BASE		(U300_SLOW_PER_PHYS_BASE)


#define U300_SYSCON_BASE		(U300_SLOW_PER_PHYS_BASE+0x1000)
#define U300_SYSCON_VBASE		(U300_SLOW_PER_VIRT_BASE+0x1000)


#define U300_WDOG_BASE			(U300_SLOW_PER_PHYS_BASE+0x2000)


#define U300_UART0_BASE			(U300_SLOW_PER_PHYS_BASE+0x3000)


#define U300_TIMER_APP_BASE		(U300_SLOW_PER_PHYS_BASE+0x4000)
#define U300_TIMER_APP_VBASE		(U300_SLOW_PER_VIRT_BASE+0x4000)


#define U300_KEYPAD_BASE		(U300_SLOW_PER_PHYS_BASE+0x5000)


#define U300_GPIO_BASE			(U300_SLOW_PER_PHYS_BASE+0x6000)


#define U300_RTC_BASE			(U300_SLOW_PER_PHYS_BASE+0x7000)


#define U300_BUSTR_BASE			(U300_SLOW_PER_PHYS_BASE+0x8000)


#define U300_EVHIST_BASE		(U300_SLOW_PER_PHYS_BASE+0x9000)


#define U300_TIMER_BASE			(U300_SLOW_PER_PHYS_BASE+0xa000)


#define U300_PPM_BASE			(U300_SLOW_PER_PHYS_BASE+0xb000)





#ifdef CONFIG_MACH_U300_BS335
#define U300_ISP_BASE			(0xA0008000)
#endif


#define U300_DMAC_BASE			(0xC0020000)


#define U300_MSL_BASE			(0xc0022000)


#define U300_APEX_BASE			(0xc0030000)


#ifdef CONFIG_MACH_U300_BS335
#define U300_VIDEOENC_BASE		(0xc0080000)
#else
#define U300_VIDEOENC_BASE		(0xc0040000)
#endif


#define U300_XGAM_BASE			(0xd0000000)




#endif
