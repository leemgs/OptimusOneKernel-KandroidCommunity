
#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H

#include <mach/hardware.h>

#define IRQ_VIC_START		0	


#define IRQ_WATCHDOG			0
#define IRQ_SOFTINT			1
#define IRQ_CRYPTO			2
#define IRQ_OWM				3
#define IRQ_MTU0			4
#define IRQ_MTU1			5
#define IRQ_GPIO0			6
#define IRQ_GPIO1			7
#define IRQ_GPIO2			8
#define IRQ_GPIO3			9
#define IRQ_RTC_RTT			10
#define IRQ_SSP				11
#define IRQ_UART0			12
#define IRQ_DMA1			13
#define IRQ_CLCD_MDIF			14
#define IRQ_DMA0			15
#define IRQ_PWRFAIL			16
#define IRQ_UART1			17
#define IRQ_FIRDA			18
#define IRQ_MSP0			19
#define IRQ_I2C0			20
#define IRQ_I2C1			21
#define IRQ_SDMMC			22
#define IRQ_USBOTG			23
#define IRQ_SVA_IT0			24
#define IRQ_SVA_IT1			25
#define IRQ_SAA_IT0			26
#define IRQ_SAA_IT1			27
#define IRQ_UART2			28
#define IRQ_MSP2			31
#define IRQ_L2CC			48
#define IRQ_HPI				49
#define IRQ_SKE				50
#define IRQ_KP				51
#define IRQ_MEMST			54
#define IRQ_SGA_IT			58
#define IRQ_USBM			60
#define IRQ_MSP1			62

#define NOMADIK_SOC_NR_IRQS		64


#define NOMADIK_NR_GPIO			128 
#define NOMADIK_GPIO_TO_IRQ(gpio)	((gpio) + NOMADIK_SOC_NR_IRQS)
#define NOMADIK_IRQ_TO_GPIO(irq)	((irq) - NOMADIK_SOC_NR_IRQS)
#define NR_IRQS				NOMADIK_GPIO_TO_IRQ(NOMADIK_NR_GPIO)


#define VIC_REG_IRQSR0		0
#define VIC_REG_IRQSR1		0x20

#endif 

