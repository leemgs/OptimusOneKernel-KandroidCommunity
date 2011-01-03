

#include <linux/init.h>
#include <linux/suspend.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/sysdev.h>
#include <linux/gpio.h>
#include <linux/io.h>

#include <mach/hardware.h>

#include <asm/mach-types.h>

#include <mach/regs-gpio.h>
#include <mach/h1940.h>

#include <plat/cpu.h>
#include <plat/pm.h>

static void s3c2410_pm_prepare(void)
{
	

	__raw_writel(virt_to_phys(s3c_cpu_resume), S3C2410_GSTATUS3);

	S3C_PMDBG("GSTATUS3 0x%08x\n", __raw_readl(S3C2410_GSTATUS3));
	S3C_PMDBG("GSTATUS4 0x%08x\n", __raw_readl(S3C2410_GSTATUS4));

	if (machine_is_h1940()) {
		void *base = phys_to_virt(H1940_SUSPEND_CHECK);
		unsigned long ptr;
		unsigned long calc = 0;

		

		for (ptr = 0; ptr < 0x40000; ptr += 0x400)
			calc += __raw_readl(base+ptr);

		__raw_writel(calc, phys_to_virt(H1940_SUSPEND_CHECKSUM));
	}

	

	if (machine_is_rx3715()) {
		void *base = phys_to_virt(H1940_SUSPEND_CHECK);
		unsigned long ptr;
		unsigned long calc = 0;

		

		for (ptr = 0; ptr < 0x40000; ptr += 0x4)
			calc += __raw_readl(base+ptr);

		__raw_writel(calc, phys_to_virt(H1940_SUSPEND_CHECKSUM));
	}

	if ( machine_is_aml_m5900() )
		s3c2410_gpio_setpin(S3C2410_GPF(2), 1);

}

static int s3c2410_pm_resume(struct sys_device *dev)
{
	unsigned long tmp;

	

	tmp = __raw_readl(S3C2410_GSTATUS2);
	tmp &= S3C2410_GSTATUS2_OFFRESET;
	__raw_writel(tmp, S3C2410_GSTATUS2);

	if ( machine_is_aml_m5900() )
		s3c2410_gpio_setpin(S3C2410_GPF(2), 0);

	return 0;
}

static int s3c2410_pm_add(struct sys_device *dev)
{
	pm_cpu_prep = s3c2410_pm_prepare;
	pm_cpu_sleep = s3c2410_cpu_suspend;

	return 0;
}

#if defined(CONFIG_CPU_S3C2410)
static struct sysdev_driver s3c2410_pm_driver = {
	.add		= s3c2410_pm_add,
	.resume		= s3c2410_pm_resume,
};



static int __init s3c2410_pm_drvinit(void)
{
	return sysdev_driver_register(&s3c2410_sysclass, &s3c2410_pm_driver);
}

arch_initcall(s3c2410_pm_drvinit);

static struct sysdev_driver s3c2410a_pm_driver = {
	.add		= s3c2410_pm_add,
	.resume		= s3c2410_pm_resume,
};

static int __init s3c2410a_pm_drvinit(void)
{
	return sysdev_driver_register(&s3c2410a_sysclass, &s3c2410a_pm_driver);
}

arch_initcall(s3c2410a_pm_drvinit);
#endif

#if defined(CONFIG_CPU_S3C2440)
static struct sysdev_driver s3c2440_pm_driver = {
	.add		= s3c2410_pm_add,
	.resume		= s3c2410_pm_resume,
};

static int __init s3c2440_pm_drvinit(void)
{
	return sysdev_driver_register(&s3c2440_sysclass, &s3c2440_pm_driver);
}

arch_initcall(s3c2440_pm_drvinit);
#endif

#if defined(CONFIG_CPU_S3C2442)
static struct sysdev_driver s3c2442_pm_driver = {
	.add		= s3c2410_pm_add,
	.resume		= s3c2410_pm_resume,
};

static int __init s3c2442_pm_drvinit(void)
{
	return sysdev_driver_register(&s3c2442_sysclass, &s3c2442_pm_driver);
}

arch_initcall(s3c2442_pm_drvinit);
#endif
