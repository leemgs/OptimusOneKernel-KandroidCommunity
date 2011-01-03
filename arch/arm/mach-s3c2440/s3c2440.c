

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/sysdev.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/irq.h>

#include <plat/s3c2440.h>
#include <plat/devs.h>
#include <plat/cpu.h>

static struct sys_device s3c2440_sysdev = {
	.cls		= &s3c2440_sysclass,
};

int __init s3c2440_init(void)
{
	printk("S3C2440: Initialising architecture\n");

	

	s3c_device_wdt.resource[1].start = IRQ_S3C2440_WDT;
	s3c_device_wdt.resource[1].end   = IRQ_S3C2440_WDT;

	

	return sysdev_register(&s3c2440_sysdev);
}
