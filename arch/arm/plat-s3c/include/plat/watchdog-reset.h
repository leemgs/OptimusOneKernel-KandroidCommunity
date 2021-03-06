

#include <plat/regs-watchdog.h>
#include <mach/map.h>

#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>

static inline void arch_wdt_reset(void)
{
	struct clk *wdtclk;

	printk("arch_reset: attempting watchdog reset\n");

	__raw_writel(0, S3C2410_WTCON);	  

	wdtclk = clk_get(NULL, "watchdog");
	if (!IS_ERR(wdtclk)) {
		clk_enable(wdtclk);
	} else
		printk(KERN_WARNING "%s: warning: cannot get watchdog clock\n", __func__);

	
	__raw_writel(0x80, S3C2410_WTCNT);
	__raw_writel(0x80, S3C2410_WTDAT);

	
	__raw_writel(S3C2410_WTCON_ENABLE|S3C2410_WTCON_DIV16|S3C2410_WTCON_RSTEN |
		     S3C2410_WTCON_PRESCALE(0x20), S3C2410_WTCON);

	
	mdelay(500);

	printk(KERN_ERR "Watchdog reset failed to assert reset\n");

	
	mdelay(50);
}
