

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>

#ifdef CONFIG_USB_FUNCTION
#include <linux/usb/mass_storage_function.h>
#endif

#include <linux/i2c.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#ifdef CONFIG_CACHE_L2X0
#include <asm/hardware/cache-l2x0.h>
#endif

#include <mach/hardware.h>
#include <mach/msm_hsusb.h>
#ifdef CONFIG_USB_ANDROID
#include <linux/usb/android.h>
#endif
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/msm_serial_hs.h>
#include <mach/memory.h>
#include <mach/msm_battery.h>
#include <mach/gpio.h>
#include <mach/board_lge.h>

#include "devices.h"
#include "socinfo.h"
#include "clock.h"
#include "msm-keypad-devices.h"
#include "board-lu3100.h"
#include "pm.h"


#include <mach/mpp.h>





extern struct msm_pm_platform_data msm7x25_pm_data[MSM_PM_SLEEP_MODE_NR];
extern struct msm_pm_platform_data msm7x27_pm_data[MSM_PM_SLEEP_MODE_NR];
#if 0
struct msm_pm_platform_data msm7x27_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].supported = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].suspend_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].latency = 16000,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].residency = 20000,

	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].supported = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].suspend_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].latency = 12000,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].residency = 20000,

	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].supported = 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].suspend_enabled
		= 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].idle_enabled = 1,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency = 2000,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].residency = 0,
};
#endif




#ifdef CONFIG_USB_ANDROID



struct usb_composition usb_func_composition[] = {
	{
		
		.product_id         = 0x61C5,
		.functions	    	= 0x2,
		.adb_product_id     = 0x61C5,
		.adb_functions	    = 0x2,
	},
	{
		
		.product_id         = 0x618E,
		.functions	    	= 0x2743,
		.adb_product_id     = 0x618E,
		.adb_functions	    = 0x12743,
	},
	{
		
		
		.product_id         = 0x6000,
		.functions	    	= 0x43,
		.adb_product_id     = 0x6000,
		.adb_functions	    = 0x43,
	},
#ifdef CONFIG_USB_ANDROID_CDC_ECM
	{
		
		.product_id         = 0x61A2,
		.functions          = 0x27384,
		.adb_product_id     = 0x61A1,
		.adb_functions      = 0x127384,
	},
#endif	
#ifdef CONFIG_USB_ANDROID_RNDIS
	{
		
		.product_id         = 0xF00E,
		.functions	    	= 0xA,
		.adb_product_id     = 0xF00E,
		.adb_functions	    = 0xA,
	},
#endif
};

#define VENDOR_QCT	0x05C6
#define VENDOR_LGE	0x1004

struct android_usb_platform_data android_usb_pdata = {
	.vendor_id	= VENDOR_LGE,
	.version	= 0x0100,
	.compositions   = usb_func_composition,
	.num_compositions = ARRAY_SIZE(usb_func_composition),
	.product_name       = "LG Android USB Device",
	.manufacturer_name	= "LG Electronics Inc.",
	
	.serial_number		= "LG_ANDROID_P500****",
	.init_product_id	= 0x618E,
	.nluns = 1,
};

struct usb_mass_storage_platform_data mass_storage_pdata = {
	.nluns		= 1,
	.vendor		= "GOOGLE",
	.product	= "Mass Storage",
	.release	= 0xFFFF,
};

struct platform_device mass_storage_device = {
	.name           = "usb_mass_storage",
	.id             = -1,
	.dev            = {
		.platform_data          = &mass_storage_pdata,
	},
};

#endif 




#define DMB_POWER_GPIO    124
#define DMB_RESET_GPIO    100
#define DMB_INT_GPIO       99

#define DMB_POWER_GPIO_CONFIG     GPIO_CFG(DMB_POWER_GPIO, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA)
#define DMB_RESET_GPIO_CONFIG     GPIO_CFG(DMB_RESET_GPIO, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA)
#define DMB_INT_GPIO_CONFIG       GPIO_CFG(DMB_INT_GPIO, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA)

static const struct msm_gpio dmb_ebi2_gpios[] = {
	{ .gpio_cfg = DMB_POWER_GPIO_CONFIG,  .label =  "dmb_power_gpio", },
	{ .gpio_cfg = DMB_RESET_GPIO_CONFIG,   .label =  "dmb_reset_gpio", },
	{ .gpio_cfg = DMB_INT_GPIO_CONFIG, .label =  "dmb_int_gpio", },

};






#define EBI2_CS2_BASE_PHYS   0x98000000
#define EBI2_CR_REG_BASE_PHYS   0xA0D00000
#define EBI2_XM_REG_BASE_PHYS   0xA0D10000


static struct resource dmb_ebi2_resources[] = {
	{
		.name	= "dmb_ebi2_cr_base",
		.start	= EBI2_CR_REG_BASE_PHYS,
		.end	       = EBI2_CR_REG_BASE_PHYS + 0x100 - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "dmb_ebi2_xm_base",
		.start	= EBI2_XM_REG_BASE_PHYS,
		.end	       = EBI2_XM_REG_BASE_PHYS + 0x100 - 1,
		.flags	= IORESOURCE_MEM,
	},	
	{
		.name	= "dmb_ebi2_phys_memory",
		.start	= EBI2_CS2_BASE_PHYS,
		.end	       = EBI2_CS2_BASE_PHYS + 0xFFFF,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "dmb_ebi2_int",
		.start	= MSM_GPIO_TO_INT(DMB_INT_GPIO),
		.end	= MSM_GPIO_TO_INT(DMB_INT_GPIO),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device msm_dmb_ebi2_device = {
	.name = "tdmb_lg2102",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(dmb_ebi2_resources),
	.resource	= dmb_ebi2_resources,
};



int usb_power_on( int on_off )
{
	int rc = 0;

    unsigned usb_en_mpp = 6; 

	if( on_off ) 
		rc = mpp_config_digital_out(usb_en_mpp,
							MPP_CFG(MPP_DLOGIC_LVL_MSMP,
							MPP_DLOGIC_OUT_CTRL_HIGH));
	else
		rc = mpp_config_digital_out(usb_en_mpp,
							MPP_CFG(MPP_DLOGIC_LVL_MSMP,
							MPP_DLOGIC_OUT_CTRL_LOW));
	return rc ;
}

static int usb_gpio_en( int on_off )
{
	int rc = 0;
	return rc ;
}



#if 0 
static int lu3100_reboot_key_detect(void)
{
	if (gpio_get_value(GPIO_PP2106_IRQ) == 0)
		return REBOOT_KEY_PRESS;
	else
		return REBOOT_KEY_NOT_PRESS;
}
struct lge_panic_handler_platform_data panic_handler_data = {
	.reboot_key_detect = lu3100_reboot_key_detect,
};
#endif

static struct platform_device *devices[] __initdata = {
	&msm_device_smd,
	&msm_device_dmov,
	&msm_device_nand,
	&msm_device_i2c,
	&msm_device_uart_dm1,
	&msm_device_snd,
	&msm_device_adspdec,
	
	&msm_dmb_ebi2_device,
	
};

extern struct sys_timer msm_timer;

static void __init msm7x2x_init_irq(void)
{
	msm_init_irq();
}

static struct msm_acpu_clock_platform_data msm7x2x_clock_data = {
	.acpu_switch_time_us = 50,
	.max_speed_delta_khz = 256000,
	.vdd_switch_time_us = 62,
	.max_axi_khz = 160000,
};

void msm_serial_debug_init(unsigned int base, int irq,
			   struct device *clk_device, int signal_irq);


unsigned pmem_fb_size = 	0x96000;
unsigned pmem_adsp_size =	0xAE4000; 

static void __init msm7x2x_init(void)
{
	if (socinfo_init() < 0)
		BUG();

	msm_clock_init(msm_clocks_7x27, msm_num_clocks_7x27);

#if defined(CONFIG_MSM_SERIAL_DEBUGGER)
	msm_serial_debug_init(MSM_UART1_PHYS, INT_UART1,
			&msm_device_uart1.dev, 1);
#endif

	if (cpu_is_msm7x27())
		msm7x2x_clock_data.max_axi_khz = 200000;

	msm_acpu_clock_init(&msm7x2x_clock_data);

	msm_add_pmem_devices();
	msm_add_fb_device();
#if !defined(CONFIG_MSM_SERIAL_DEBUGGER)
	if (lge_get_uart_mode())
		platform_device_register(&msm_device_uart3);
#endif
	platform_add_devices(devices, ARRAY_SIZE(devices));
#ifdef CONFIG_ARCH_MSM7X27
	msm_add_kgsl_device();
#endif
	msm_add_usb_devices();

#ifdef CONFIG_MSM_CAMERA
	config_camera_off_gpios(); 
#endif
	msm_device_i2c_init();
	i2c_register_board_info(0, i2c_devices, ARRAY_SIZE(i2c_devices));

	if (cpu_is_msm7x27())
		msm_pm_set_platform_data(msm7x27_pm_data,
					ARRAY_SIZE(msm7x27_pm_data));
	else
		msm_pm_set_platform_data(msm7x25_pm_data,
					ARRAY_SIZE(msm7x25_pm_data));

#ifdef CONFIG_ANDROID_RAM_CONSOLE
	lge_add_ramconsole_devices();
	lge_add_ers_devices();
	lge_add_panic_handler_devices();
#endif
	lge_add_camera_devices();
	lge_add_lcd_devices();
	lge_add_btpower_devices();
	lge_add_mmc_devices();
	lge_add_input_devices();
	lge_add_misc_devices();
	lge_add_pm_devices();
	
	
	lge_add_gpio_i2c_devices();
	create_testmode_status();  
	create_sw_version_status();  
	create_smpl_reset_status();    
	create_qpst_enable_status(); 
}

static void __init msm7x2x_map_io(void)
{
	msm_map_common_io();

	msm_msm7x2x_allocate_memory_regions();

#ifdef CONFIG_CACHE_L2X0
	
	l2x0_init(MSM_L2CC_BASE, 0x00068012, 0xfe000000);
#endif
}

MACHINE_START(MSM7X27_LU3100, "LU3100 Global board (LGE LU3100)")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io        = MSM_DEBUG_UART_PHYS,
	.io_pg_offst    = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= PHYS_OFFSET + 0x100,
	.map_io			= msm7x2x_map_io,
	.init_irq		= msm7x2x_init_irq,
	.init_machine	= msm7x2x_init,
	.timer			= &msm_timer,
MACHINE_END
