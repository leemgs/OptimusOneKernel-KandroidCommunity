
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/serial_core.h>

#include <asm/irq.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/setup.h>

#include <asm/mach/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/irda.h>
#include <asm/mach/map.h>
#include <asm/mach/serial_sa1100.h>

#include <mach/h3600.h>
#include <mach/h3600_gpio.h>

#include "generic.h"

void (*assign_h3600_egpio)(enum ipaq_egpio_type x, int level);
EXPORT_SYMBOL(assign_h3600_egpio);

static struct mtd_partition h3xxx_partitions[] = {
	{
		.name		= "H3XXX boot firmware",
		.size		= 0x00040000,
		.offset		= 0,
		.mask_flags	= MTD_WRITEABLE,  
	}, {
		.name		= "H3XXX rootfs",
		.size		= MTDPART_SIZ_FULL,
		.offset		= 0x00040000,
	}
};

static void h3xxx_set_vpp(int vpp)
{
	assign_h3600_egpio(IPAQ_EGPIO_VPP_ON, vpp);
}

static struct flash_platform_data h3xxx_flash_data = {
	.map_name	= "cfi_probe",
	.set_vpp	= h3xxx_set_vpp,
	.parts		= h3xxx_partitions,
	.nr_parts	= ARRAY_SIZE(h3xxx_partitions),
};

static struct resource h3xxx_flash_resource = {
	.start		= SA1100_CS0_PHYS,
	.end		= SA1100_CS0_PHYS + SZ_32M - 1,
	.flags		= IORESOURCE_MEM,
};


static int h3600_irda_set_power(struct device *dev, unsigned int state)
{
	assign_h3600_egpio( IPAQ_EGPIO_IR_ON, state );

	return 0;
}

static void h3600_irda_set_speed(struct device *dev, unsigned int speed)
{
	assign_h3600_egpio(IPAQ_EGPIO_IR_FSEL, !(speed < 4000000));
}

static struct irda_platform_data h3600_irda_data = {
	.set_power	= h3600_irda_set_power,
	.set_speed	= h3600_irda_set_speed,
};

static void h3xxx_mach_init(void)
{
	sa11x0_set_flash_data(&h3xxx_flash_data, &h3xxx_flash_resource, 1);
	sa11x0_set_irda_data(&h3600_irda_data);
}



static void h3600_uart_set_mctrl(struct uart_port *port, u_int mctrl)
{
	if (port->mapbase == _Ser3UTCR0) {
		if (mctrl & TIOCM_RTS)
			GPCR = GPIO_H3600_COM_RTS;
		else
			GPSR = GPIO_H3600_COM_RTS;
	}
}

static u_int h3600_uart_get_mctrl(struct uart_port *port)
{
	u_int ret = TIOCM_CD | TIOCM_CTS | TIOCM_DSR;

	if (port->mapbase == _Ser3UTCR0) {
		int gplr = GPLR;
		
		if (gplr & GPIO_H3600_COM_DCD)
			ret &= ~TIOCM_CD;
		if (gplr & GPIO_H3600_COM_CTS)
			ret &= ~TIOCM_CTS;
	}

	return ret;
}

static void h3600_uart_pm(struct uart_port *port, u_int state, u_int oldstate)
{
	if (port->mapbase == _Ser2UTCR0) { 
		assign_h3600_egpio(IPAQ_EGPIO_IR_ON, !state);
	} else if (port->mapbase == _Ser3UTCR0) {
		assign_h3600_egpio(IPAQ_EGPIO_RS232_ON, !state);
	}
}


static int h3600_uart_set_wake(struct uart_port *port, u_int enable)
{
	int err = -EINVAL;

	if (port->mapbase == _Ser3UTCR0) {
		if (enable)
			PWER |= PWER_GPIO23 | PWER_GPIO25; 
		else
			PWER &= ~(PWER_GPIO23 | PWER_GPIO25); 
		err = 0;
	}
	return err;
}

static struct sa1100_port_fns h3600_port_fns __initdata = {
	.set_mctrl	= h3600_uart_set_mctrl,
	.get_mctrl	= h3600_uart_get_mctrl,
	.pm		= h3600_uart_pm,
	.set_wake	= h3600_uart_set_wake,
};


static void h3xxx_lcd_power(int enable)
{
	assign_h3600_egpio(IPAQ_EGPIO_LCD_POWER, enable);
}

static struct map_desc h3600_io_desc[] __initdata = {
  	{	
		.virtual	=  H3600_BANK_2_VIRT,
		.pfn		= __phys_to_pfn(SA1100_CS2_PHYS),
		.length		= 0x02800000,
		.type		= MT_DEVICE
	}, {	
		.virtual	=  H3600_BANK_4_VIRT,
		.pfn		= __phys_to_pfn(SA1100_CS4_PHYS),
		.length		= 0x00800000,
		.type		= MT_DEVICE
	}, {	
		.virtual	=  H3600_EGPIO_VIRT,
		.pfn		= __phys_to_pfn(H3600_EGPIO_PHYS),
		.length		= 0x01000000,
		.type		= MT_DEVICE
	}
};



static void __init h3xxx_map_io(void)
{
	sa1100_map_io();
	iotable_init(h3600_io_desc, ARRAY_SIZE(h3600_io_desc));

	sa1100_register_uart_fns(&h3600_port_fns);
	sa1100_register_uart(0, 3); 


	
	PPDR |= PPC_TXD4 | PPC_SCLK | PPC_SFRM;
	PPSR &= ~(PPC_TXD4 | PPC_SCLK | PPC_SFRM);

	
	PGSR = 0;
	PWER = PWER_GPIO0 | PWER_RTC;
	PCFR = PCFR_OPDE;
	PSDR = 0;

	sa1100fb_lcd_power = h3xxx_lcd_power;
}



#ifdef CONFIG_SA1100_H3100

#define H3100_EGPIO	(*(volatile unsigned int *)H3600_EGPIO_VIRT)
static unsigned int h3100_egpio = 0;

static void h3100_control_egpio(enum ipaq_egpio_type x, int setp)
{
	unsigned int egpio = 0;
	long	     gpio = 0;
	unsigned long flags;

	switch (x) {
	case IPAQ_EGPIO_LCD_POWER:
		egpio |= EGPIO_H3600_LCD_ON;
		gpio  |= GPIO_H3100_LCD_3V_ON;
		break;
	case IPAQ_EGPIO_LCD_ENABLE:
		break;
	case IPAQ_EGPIO_CODEC_NRESET:
		egpio |= EGPIO_H3600_CODEC_NRESET;
		break;
	case IPAQ_EGPIO_AUDIO_ON:
		gpio |= GPIO_H3100_AUD_PWR_ON
			| GPIO_H3100_AUD_ON;
		break;
	case IPAQ_EGPIO_QMUTE:
		gpio |= GPIO_H3100_QMUTE;
		break;
	case IPAQ_EGPIO_OPT_NVRAM_ON:
		egpio |= EGPIO_H3600_OPT_NVRAM_ON;
		break;
	case IPAQ_EGPIO_OPT_ON:
		egpio |= EGPIO_H3600_OPT_ON;
		break;
	case IPAQ_EGPIO_CARD_RESET:
		egpio |= EGPIO_H3600_CARD_RESET;
		break;
	case IPAQ_EGPIO_OPT_RESET:
		egpio |= EGPIO_H3600_OPT_RESET;
		break;
	case IPAQ_EGPIO_IR_ON:
		gpio |= GPIO_H3100_IR_ON;
		break;
	case IPAQ_EGPIO_IR_FSEL:
		gpio |= GPIO_H3100_IR_FSEL;
		break;
	case IPAQ_EGPIO_RS232_ON:
		egpio |= EGPIO_H3600_RS232_ON;
		break;
	case IPAQ_EGPIO_VPP_ON:
		egpio |= EGPIO_H3600_VPP_ON;
		break;
	}

	if (egpio || gpio) {
		local_irq_save(flags);
		if (setp) {
			h3100_egpio |= egpio;
			GPSR = gpio;
		} else {
			h3100_egpio &= ~egpio;
			GPCR = gpio;
		}
		H3100_EGPIO = h3100_egpio;
		local_irq_restore(flags);
	}
}

#define H3100_DIRECT_EGPIO (GPIO_H3100_BT_ON	  \
			  | GPIO_H3100_GPIO3	  \
			  | GPIO_H3100_QMUTE	  \
			  | GPIO_H3100_LCD_3V_ON  \
			  | GPIO_H3100_AUD_ON	  \
			  | GPIO_H3100_AUD_PWR_ON \
			  | GPIO_H3100_IR_ON	  \
			  | GPIO_H3100_IR_FSEL)

static void __init h3100_map_io(void)
{
	h3xxx_map_io();

	
	GPCR = 0x0fffffff;	 
	GPDR = GPIO_H3600_COM_RTS  | GPIO_H3600_L3_CLOCK |
	       GPIO_H3600_L3_MODE  | GPIO_H3600_L3_DATA  |
	       GPIO_H3600_CLK_SET1 | GPIO_H3600_CLK_SET0 |
	       H3100_DIRECT_EGPIO;

	
	GAFR &= ~H3100_DIRECT_EGPIO;

	H3100_EGPIO = h3100_egpio;
	assign_h3600_egpio = h3100_control_egpio;
}

MACHINE_START(H3100, "Compaq iPAQ H3100")
	.phys_io	= 0x80000000,
	.io_pg_offst	= ((0xf8000000) >> 18) & 0xfffc,
	.boot_params	= 0xc0000100,
	.map_io		= h3100_map_io,
	.init_irq	= sa1100_init_irq,
	.timer		= &sa1100_timer,
	.init_machine	= h3xxx_mach_init,
MACHINE_END

#endif 



#ifdef CONFIG_SA1100_H3600

#define H3600_EGPIO	(*(volatile unsigned int *)H3600_EGPIO_VIRT)
static unsigned int h3600_egpio = EGPIO_H3600_RS232_ON;

static void h3600_control_egpio(enum ipaq_egpio_type x, int setp)
{
	unsigned int egpio = 0;
	unsigned long flags;

	switch (x) {
	case IPAQ_EGPIO_LCD_POWER:
		egpio |= EGPIO_H3600_LCD_ON |
			 EGPIO_H3600_LCD_PCI |
			 EGPIO_H3600_LCD_5V_ON |
			 EGPIO_H3600_LVDD_ON;
		break;
	case IPAQ_EGPIO_LCD_ENABLE:
		break;
	case IPAQ_EGPIO_CODEC_NRESET:
		egpio |= EGPIO_H3600_CODEC_NRESET;
		break;
	case IPAQ_EGPIO_AUDIO_ON:
		egpio |= EGPIO_H3600_AUD_AMP_ON |
			 EGPIO_H3600_AUD_PWR_ON;
		break;
	case IPAQ_EGPIO_QMUTE:
		egpio |= EGPIO_H3600_QMUTE;
		break;
	case IPAQ_EGPIO_OPT_NVRAM_ON:
		egpio |= EGPIO_H3600_OPT_NVRAM_ON;
		break;
	case IPAQ_EGPIO_OPT_ON:
		egpio |= EGPIO_H3600_OPT_ON;
		break;
	case IPAQ_EGPIO_CARD_RESET:
		egpio |= EGPIO_H3600_CARD_RESET;
		break;
	case IPAQ_EGPIO_OPT_RESET:
		egpio |= EGPIO_H3600_OPT_RESET;
		break;
	case IPAQ_EGPIO_IR_ON:
		egpio |= EGPIO_H3600_IR_ON;
		break;
	case IPAQ_EGPIO_IR_FSEL:
		egpio |= EGPIO_H3600_IR_FSEL;
		break;
	case IPAQ_EGPIO_RS232_ON:
		egpio |= EGPIO_H3600_RS232_ON;
		break;
	case IPAQ_EGPIO_VPP_ON:
		egpio |= EGPIO_H3600_VPP_ON;
		break;
	}

	if (egpio) {
		local_irq_save(flags);
		if (setp)
			h3600_egpio |= egpio;
		else
			h3600_egpio &= ~egpio;
		H3600_EGPIO = h3600_egpio;
		local_irq_restore(flags);
	}
}

static void __init h3600_map_io(void)
{
	h3xxx_map_io();

	

	GPCR = 0x0fffffff;	 
	GPDR = GPIO_H3600_COM_RTS  | GPIO_H3600_L3_CLOCK |
	       GPIO_H3600_L3_MODE  | GPIO_H3600_L3_DATA  |
	       GPIO_H3600_CLK_SET1 | GPIO_H3600_CLK_SET0 |
	       GPIO_LDD15 | GPIO_LDD14 | GPIO_LDD13 | GPIO_LDD12 |
	       GPIO_LDD11 | GPIO_LDD10 | GPIO_LDD9  | GPIO_LDD8;

	H3600_EGPIO = h3600_egpio;	   
	assign_h3600_egpio = h3600_control_egpio;
}

MACHINE_START(H3600, "Compaq iPAQ H3600")
	.phys_io	= 0x80000000,
	.io_pg_offst	= ((0xf8000000) >> 18) & 0xfffc,
	.boot_params	= 0xc0000100,
	.map_io		= h3600_map_io,
	.init_irq	= sa1100_init_irq,
	.timer		= &sa1100_timer,
	.init_machine	= h3xxx_mach_init,
MACHINE_END

#endif 

