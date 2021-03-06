



#ifndef __ASM_ARCH_BOARD_H
#define __ASM_ARCH_BOARD_H

#include <linux/mtd/partitions.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/leds.h>
#include <linux/spi/spi.h>
#include <linux/usb/atmel_usba_udc.h>
#include <linux/atmel-mci.h>
#include <sound/atmel-ac97c.h>

 
struct at91_udc_data {
	u8	vbus_pin;		
	u8	pullup_pin;		
	u8	pullup_active_low;	
};
extern void __init at91_add_device_udc(struct at91_udc_data *data);

 
extern void __init at91_add_device_usba(struct usba_platform_data *data);

 
struct at91_cf_data {
	u8	irq_pin;		
	u8	det_pin;		
	u8	vcc_pin;		
	u8	rst_pin;		
	u8	chipselect;		
	u8	flags;
#define AT91_CF_TRUE_IDE	0x01
#define AT91_IDE_SWAP_A0_A2	0x02
};
extern void __init at91_add_device_cf(struct at91_cf_data *data);

 
  
struct at91_mmc_data {
	u8		det_pin;	
	unsigned	slot_b:1;	
	unsigned	wire4:1;	
	u8		wp_pin;		
	u8		vcc_pin;	
};
extern void __init at91_add_device_mmc(short mmc_id, struct at91_mmc_data *data);

  
extern void __init at91_add_device_mci(short mmc_id, struct mci_platform_data *data);

 
struct at91_eth_data {
	u32		phy_mask;
	u8		phy_irq_pin;	
	u8		is_rmii;	
};
extern void __init at91_add_device_eth(struct at91_eth_data *data);

#if defined(CONFIG_ARCH_AT91SAM9260) || defined(CONFIG_ARCH_AT91SAM9263) || defined(CONFIG_ARCH_AT91SAM9G20) || defined(CONFIG_ARCH_AT91CAP9) \
	|| defined(CONFIG_ARCH_AT91SAM9G45)
#define eth_platform_data	at91_eth_data
#endif

 
struct at91_usbh_data {
	u8		ports;		
	u8		vbus_pin[2];	
};
extern void __init at91_add_device_usbh(struct at91_usbh_data *data);
extern void __init at91_add_device_usbh_ohci(struct at91_usbh_data *data);

 
struct atmel_nand_data {
	u8		enable_pin;	
	u8		det_pin;	
	u8		rdy_pin;	
	u8              rdy_pin_active_low;     
	u8		ale;		
	u8		cle;		
	u8		bus_width_16;	
	struct mtd_partition* (*partition_info)(int, int*);
};
extern void __init at91_add_device_nand(struct atmel_nand_data *data);

 
#if defined(CONFIG_ARCH_AT91SAM9G45)
extern void __init at91_add_device_i2c(short i2c_id, struct i2c_board_info *devices, int nr_devices);
#else
extern void __init at91_add_device_i2c(struct i2c_board_info *devices, int nr_devices);
#endif

 
extern void __init at91_add_device_spi(struct spi_board_info *devices, int nr_devices);

 
#define ATMEL_UART_CTS	0x01
#define ATMEL_UART_RTS	0x02
#define ATMEL_UART_DSR	0x04
#define ATMEL_UART_DTR	0x08
#define ATMEL_UART_DCD	0x10
#define ATMEL_UART_RI	0x20

extern void __init at91_register_uart(unsigned id, unsigned portnr, unsigned pins);
extern void __init at91_set_serial_console(unsigned portnr);

struct at91_uart_config {
	unsigned short	console_tty;	
	unsigned short	nr_tty;		
	short		tty_map[];	
};
extern struct platform_device *atmel_default_console_device;
extern void __init __deprecated at91_init_serial(struct at91_uart_config *config);

struct atmel_uart_data {
	short		use_dma_tx;	
	short		use_dma_rx;	
	void __iomem	*regs;		
};
extern void __init at91_add_device_serial(void);


#define AT91_PWM0	0
#define AT91_PWM1	1
#define AT91_PWM2	2
#define AT91_PWM3	3

extern void __init at91_add_device_pwm(u32 mask);


#define ATMEL_SSC_TK	0x01
#define ATMEL_SSC_TF	0x02
#define ATMEL_SSC_TD	0x04
#define ATMEL_SSC_TX	(ATMEL_SSC_TK | ATMEL_SSC_TF | ATMEL_SSC_TD)

#define ATMEL_SSC_RK	0x10
#define ATMEL_SSC_RF	0x20
#define ATMEL_SSC_RD	0x40
#define ATMEL_SSC_RX	(ATMEL_SSC_RK | ATMEL_SSC_RF | ATMEL_SSC_RD)

extern void __init at91_add_device_ssc(unsigned id, unsigned pins);

 
struct atmel_lcdfb_info;
extern void __init at91_add_device_lcdc(struct atmel_lcdfb_info *data);

 
extern void __init at91_add_device_ac97(struct ac97c_platform_data *data);

 
extern void __init at91_add_device_isi(void);

 
extern void __init at91_add_device_tsadcc(void);


struct at91_can_data {
	void (*transceiver_switch)(int on);
};
extern void __init at91_add_device_can(struct at91_can_data *data);

 
extern void __init at91_init_leds(u8 cpu_led, u8 timer_led);
extern void __init at91_gpio_leds(struct gpio_led *leds, int nr);
extern void __init at91_pwm_leds(struct gpio_led *leds, int nr);


extern int at91_suspend_entering_slow_clock(void);

#endif
