

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/nand.h>
#include <linux/mv643xx_eth.h>
#include <linux/gpio_keys.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/serial_reg.h>
#include <linux/ata_platform.h>
#include <asm/mach-types.h>
#include <asm/gpio.h>
#include <asm/mach/arch.h>
#include <asm/mach/pci.h>
#include <mach/orion5x.h>
#include "common.h"
#include "mpp.h"
#include "tsx09-common.h"

#define QNAP_TS209_NOR_BOOT_BASE 0xf4000000
#define QNAP_TS209_NOR_BOOT_SIZE SZ_8M


static struct mtd_partition qnap_ts209_partitions[] = {
	{
		.name		= "U-Boot",
		.size		= 0x00080000,
		.offset		= 0x00780000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "Kernel",
		.size		= 0x00200000,
		.offset		= 0,
	}, {
		.name		= "RootFS1",
		.size		= 0x00400000,
		.offset		= 0x00200000,
	}, {
		.name		= "RootFS2",
		.size		= 0x00100000,
		.offset		= 0x00600000,
	}, {
		.name		= "U-Boot Config",
		.size		= 0x00020000,
		.offset		= 0x00760000,
	}, {
		.name		= "NAS Config",
		.size		= 0x00060000,
		.offset		= 0x00700000,
		.mask_flags	= MTD_WRITEABLE,
	},
};

static struct physmap_flash_data qnap_ts209_nor_flash_data = {
	.width		= 1,
	.parts		= qnap_ts209_partitions,
	.nr_parts	= ARRAY_SIZE(qnap_ts209_partitions)
};

static struct resource qnap_ts209_nor_flash_resource = {
	.flags	= IORESOURCE_MEM,
	.start	= QNAP_TS209_NOR_BOOT_BASE,
	.end	= QNAP_TS209_NOR_BOOT_BASE + QNAP_TS209_NOR_BOOT_SIZE - 1,
};

static struct platform_device qnap_ts209_nor_flash = {
	.name		= "physmap-flash",
	.id		= 0,
	.dev		= {
		.platform_data	= &qnap_ts209_nor_flash_data,
	},
	.resource	= &qnap_ts209_nor_flash_resource,
	.num_resources	= 1,
};



#define QNAP_TS209_PCI_SLOT0_OFFS	7
#define QNAP_TS209_PCI_SLOT0_IRQ_PIN	6
#define QNAP_TS209_PCI_SLOT1_IRQ_PIN	7

void __init qnap_ts209_pci_preinit(void)
{
	int pin;

	
	pin = QNAP_TS209_PCI_SLOT0_IRQ_PIN;
	if (gpio_request(pin, "PCI Int1") == 0) {
		if (gpio_direction_input(pin) == 0) {
			set_irq_type(gpio_to_irq(pin), IRQ_TYPE_LEVEL_LOW);
		} else {
			printk(KERN_ERR "qnap_ts209_pci_preinit failed to "
					"set_irq_type pin %d\n", pin);
			gpio_free(pin);
		}
	} else {
		printk(KERN_ERR "qnap_ts209_pci_preinit failed to gpio_request "
				"%d\n", pin);
	}

	pin = QNAP_TS209_PCI_SLOT1_IRQ_PIN;
	if (gpio_request(pin, "PCI Int2") == 0) {
		if (gpio_direction_input(pin) == 0) {
			set_irq_type(gpio_to_irq(pin), IRQ_TYPE_LEVEL_LOW);
		} else {
			printk(KERN_ERR "qnap_ts209_pci_preinit failed "
					"to set_irq_type pin %d\n", pin);
			gpio_free(pin);
		}
	} else {
		printk(KERN_ERR "qnap_ts209_pci_preinit failed to gpio_request "
				"%d\n", pin);
	}
}

static int __init qnap_ts209_pci_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int irq;

	
	irq = orion5x_pci_map_irq(dev, slot, pin);
	if (irq != -1)
		return irq;

	
	switch (slot - QNAP_TS209_PCI_SLOT0_OFFS) {
	case 0:
		return gpio_to_irq(QNAP_TS209_PCI_SLOT0_IRQ_PIN);
	case 1:
		return gpio_to_irq(QNAP_TS209_PCI_SLOT1_IRQ_PIN);
	default:
		return -1;
	}
}

static struct hw_pci qnap_ts209_pci __initdata = {
	.nr_controllers	= 2,
	.preinit	= qnap_ts209_pci_preinit,
	.swizzle	= pci_std_swizzle,
	.setup		= orion5x_pci_sys_setup,
	.scan		= orion5x_pci_sys_scan_bus,
	.map_irq	= qnap_ts209_pci_map_irq,
};

static int __init qnap_ts209_pci_init(void)
{
	if (machine_is_ts_x09())
		pci_common_init(&qnap_ts209_pci);

	return 0;
}

subsys_initcall(qnap_ts209_pci_init);



#define TS209_RTC_GPIO	3

static struct i2c_board_info __initdata qnap_ts209_i2c_rtc = {
	I2C_BOARD_INFO("s35390a", 0x30),
	.irq	= 0,
};



#define QNAP_TS209_GPIO_KEY_MEDIA	1
#define QNAP_TS209_GPIO_KEY_RESET	2

static struct gpio_keys_button qnap_ts209_buttons[] = {
	{
		.code		= KEY_COPY,
		.gpio		= QNAP_TS209_GPIO_KEY_MEDIA,
		.desc		= "USB Copy Button",
		.active_low	= 1,
	}, {
		.code		= KEY_RESTART,
		.gpio		= QNAP_TS209_GPIO_KEY_RESET,
		.desc		= "Reset Button",
		.active_low	= 1,
	},
};

static struct gpio_keys_platform_data qnap_ts209_button_data = {
	.buttons	= qnap_ts209_buttons,
	.nbuttons	= ARRAY_SIZE(qnap_ts209_buttons),
};

static struct platform_device qnap_ts209_button_device = {
	.name		= "gpio-keys",
	.id		= -1,
	.num_resources	= 0,
	.dev		= {
		.platform_data	= &qnap_ts209_button_data,
	},
};


static struct mv_sata_platform_data qnap_ts209_sata_data = {
	.n_ports	= 2,
};


static struct orion5x_mpp_mode ts209_mpp_modes[] __initdata = {
	{  0, MPP_UNUSED },
	{  1, MPP_GPIO },		
	{  2, MPP_GPIO },		
	{  3, MPP_GPIO },		
	{  4, MPP_UNUSED },
	{  5, MPP_UNUSED },
	{  6, MPP_GPIO },		
	{  7, MPP_GPIO },		
	{  8, MPP_UNUSED },
	{  9, MPP_UNUSED },
	{ 10, MPP_UNUSED },
	{ 11, MPP_UNUSED },
	{ 12, MPP_SATA_LED },		
	{ 13, MPP_SATA_LED },		
	{ 14, MPP_SATA_LED },		
	{ 15, MPP_SATA_LED },		
	{ 16, MPP_UART },		
	{ 17, MPP_UART },		
	{ 18, MPP_GPIO },		
	{ 19, MPP_UNUSED },
	{ -1 },
};

static void __init qnap_ts209_init(void)
{
	
	orion5x_init();

	orion5x_mpp_conf(ts209_mpp_modes);

	

	
	orion5x_setup_dev_boot_win(QNAP_TS209_NOR_BOOT_BASE,
				   QNAP_TS209_NOR_BOOT_SIZE);
	platform_device_register(&qnap_ts209_nor_flash);

	orion5x_ehci0_init();
	orion5x_ehci1_init();
	qnap_tsx09_find_mac_addr(QNAP_TS209_NOR_BOOT_BASE +
				 qnap_ts209_partitions[5].offset,
				 qnap_ts209_partitions[5].size);
	orion5x_eth_init(&qnap_tsx09_eth_data);
	orion5x_i2c_init();
	orion5x_sata_init(&qnap_ts209_sata_data);
	orion5x_uart0_init();
	orion5x_uart1_init();
	orion5x_xor_init();

	platform_device_register(&qnap_ts209_button_device);

	
	if (gpio_request(TS209_RTC_GPIO, "rtc") == 0) {
		if (gpio_direction_input(TS209_RTC_GPIO) == 0)
			qnap_ts209_i2c_rtc.irq = gpio_to_irq(TS209_RTC_GPIO);
		else
			gpio_free(TS209_RTC_GPIO);
	}
	if (qnap_ts209_i2c_rtc.irq == 0)
		pr_warning("qnap_ts209_init: failed to get RTC IRQ\n");
	i2c_register_board_info(0, &qnap_ts209_i2c_rtc, 1);

	
	pm_power_off = qnap_tsx09_power_off;
}

MACHINE_START(TS209, "QNAP TS-109/TS-209")
	
	.phys_io	= ORION5X_REGS_PHYS_BASE,
	.io_pg_offst	= ((ORION5X_REGS_VIRT_BASE) >> 18) & 0xFFFC,
	.boot_params	= 0x00000100,
	.init_machine	= qnap_ts209_init,
	.map_io		= orion5x_map_io,
	.init_irq	= orion5x_init_irq,
	.timer		= &orion5x_timer,
	.fixup		= tag_fixup_mem32,
MACHINE_END
