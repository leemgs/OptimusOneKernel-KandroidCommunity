

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/reboot.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/pm.h>

#include <mach/msm_iomap.h>

#define TCSR_BASE 0x16B00000
#define TCSR_WDT_CFG 0x30

#define WDT0_RST       (MSM_TMR_BASE + 0x38)
#define WDT0_EN        (MSM_TMR_BASE + 0x40)
#define WDT0_BARK_TIME (MSM_TMR_BASE + 0x4C)

#define PSHOLD_CTL_SU (MSM_TLMM_BASE + 0x810)

static void *tcsr_base;

static void msm_power_off(void)
{
	printk(KERN_NOTICE "Powering off the SoC\n");
	writel(0, PSHOLD_CTL_SU);
	mdelay(10000);
	printk(KERN_NOTICE "Powering off has failed\n");
	return;
}

static void msm_restart(char str, const char *cmd)
{
	printk(KERN_NOTICE "Going down for restart now\n");
	writel(1, WDT0_RST);
	writel(0, WDT0_EN);
	writel(0x31F3, WDT0_BARK_TIME);
	writel(3, WDT0_EN);
	dmb();
	if (tcsr_base != NULL)
		writel(3, tcsr_base + TCSR_WDT_CFG);

	mdelay(10000);
	printk(KERN_ERR "Restarting has failed\n");
	return;
}

static int __init msm_restart_init(void)
{
	pm_power_off = msm_power_off;
	arm_pm_restart = msm_restart;

	tcsr_base = ioremap_nocache(TCSR_BASE, SZ_4K);
	if (tcsr_base == NULL)
		return -ENOMEM;
	return 0;
}

late_initcall(msm_restart_init);
