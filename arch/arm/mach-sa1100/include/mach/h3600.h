

#ifndef _INCLUDE_H3600_H_
#define _INCLUDE_H3600_H_

typedef int __bitwise pm_request_t;

#define PM_SUSPEND	((__force pm_request_t) 1)	
#define PM_RESUME	((__force pm_request_t) 2)	


#define machine_is_h3xxx() (machine_is_h3100() || machine_is_h3600())


#define H3600_EGPIO_PHYS	(SA1100_CS5_PHYS + 0x01000000)
#define H3600_BANK_2_PHYS	SA1100_CS2_PHYS
#define H3600_BANK_4_PHYS	SA1100_CS4_PHYS


#define H3600_EGPIO_VIRT	0xf0000000
#define H3600_BANK_2_VIRT	0xf1000000
#define H3600_BANK_4_VIRT	0xf3800000



#define GPIO_H3600_NPOWER_BUTTON	GPIO_GPIO (0)	

#define GPIO_H3600_PCMCIA_CD1		GPIO_GPIO (10)
#define GPIO_H3600_PCMCIA_IRQ1		GPIO_GPIO (11)


#define GPIO_H3600_L3_DATA		GPIO_GPIO (14)
#define GPIO_H3600_L3_MODE		GPIO_GPIO (15)
#define GPIO_H3600_L3_CLOCK		GPIO_GPIO (16)

#define GPIO_H3600_PCMCIA_CD0		GPIO_GPIO (17)
#define GPIO_H3600_SYS_CLK		GPIO_GPIO (19)
#define GPIO_H3600_PCMCIA_IRQ0		GPIO_GPIO (21)

#define GPIO_H3600_COM_DCD		GPIO_GPIO (23)
#define GPIO_H3600_OPT_IRQ		GPIO_GPIO (24)
#define GPIO_H3600_COM_CTS		GPIO_GPIO (25)
#define GPIO_H3600_COM_RTS		GPIO_GPIO (26)

#define IRQ_GPIO_H3600_NPOWER_BUTTON	IRQ_GPIO0
#define IRQ_GPIO_H3600_PCMCIA_CD1	IRQ_GPIO10
#define IRQ_GPIO_H3600_PCMCIA_IRQ1	IRQ_GPIO11
#define IRQ_GPIO_H3600_PCMCIA_CD0	IRQ_GPIO17
#define IRQ_GPIO_H3600_PCMCIA_IRQ0	IRQ_GPIO21
#define IRQ_GPIO_H3600_COM_DCD		IRQ_GPIO23
#define IRQ_GPIO_H3600_OPT_IRQ		IRQ_GPIO24
#define IRQ_GPIO_H3600_COM_CTS		IRQ_GPIO25


#ifndef __ASSEMBLY__

enum ipaq_egpio_type {
	IPAQ_EGPIO_LCD_POWER,	  
	IPAQ_EGPIO_CODEC_NRESET,  
	IPAQ_EGPIO_AUDIO_ON,	  
	IPAQ_EGPIO_QMUTE,	  
	IPAQ_EGPIO_OPT_NVRAM_ON,  
	IPAQ_EGPIO_OPT_ON,	  
	IPAQ_EGPIO_CARD_RESET,	  
	IPAQ_EGPIO_OPT_RESET,	  
	IPAQ_EGPIO_IR_ON,	  
	IPAQ_EGPIO_IR_FSEL,	  
	IPAQ_EGPIO_RS232_ON,	  
	IPAQ_EGPIO_VPP_ON,	  
	IPAQ_EGPIO_LCD_ENABLE,	  
};

extern void (*assign_h3600_egpio)(enum ipaq_egpio_type x, int level);

#endif 

#endif 
