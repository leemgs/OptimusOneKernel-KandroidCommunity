

#include <linux/io.h>
#include <mach/hardware.h>

#include <mach/map.h>
#include <mach/idle.h>
#include <mach/reset.h>

#include <mach/regs-clock.h>

void (*s3c24xx_idle)(void);
void (*s3c24xx_reset_hook)(void);

void s3c24xx_default_idle(void)
{
	unsigned long tmp;
	int i;

	

	

	__raw_writel(__raw_readl(S3C2410_CLKCON) | S3C2410_CLKCON_IDLE,
		     S3C2410_CLKCON);

	
	for (i = 0; i < 50; i++) {
		tmp += __raw_readl(S3C2410_CLKCON); 
	}

	

	__raw_writel(__raw_readl(S3C2410_CLKCON) & ~S3C2410_CLKCON_IDLE,
		     S3C2410_CLKCON);
}

static void arch_idle(void)
{
	if (s3c24xx_idle != NULL)
		(s3c24xx_idle)();
	else
		s3c24xx_default_idle();
}

#include <mach/system-reset.h>
