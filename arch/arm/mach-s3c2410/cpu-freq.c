

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/cpufreq.h>
#include <linux/sysdev.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>

#include <mach/regs-clock.h>

#include <plat/cpu.h>
#include <plat/clock.h>
#include <plat/cpu-freq-core.h>



static void s3c2410_cpufreq_setdivs(struct s3c_cpufreq_config *cfg)
{
	u32 clkdiv = 0;

	if (cfg->divs.h_divisor == 2)
		clkdiv |= S3C2410_CLKDIVN_HDIVN;

	if (cfg->divs.p_divisor != cfg->divs.h_divisor)
		clkdiv |= S3C2410_CLKDIVN_PDIVN;

	__raw_writel(clkdiv, S3C2410_CLKDIVN);
}

static int s3c2410_cpufreq_calcdivs(struct s3c_cpufreq_config *cfg)
{
	unsigned long hclk, fclk, pclk;
	unsigned int hdiv, pdiv;
	unsigned long hclk_max;

	fclk = cfg->freq.fclk;
	hclk_max = cfg->max.hclk;

	cfg->freq.armclk = fclk;

	s3c_freq_dbg("%s: fclk is %lu, max hclk %lu\n",
		      __func__, fclk, hclk_max);

	hdiv = (fclk > cfg->max.hclk) ? 2 : 1;
	hclk = fclk / hdiv;

	if (hclk > cfg->max.hclk) {
		s3c_freq_dbg("%s: hclk too big\n", __func__);
		return -EINVAL;
	}

	pdiv = (hclk > cfg->max.pclk) ? 2 : 1;
	pclk = hclk / pdiv;

	if (pclk > cfg->max.pclk) {
		s3c_freq_dbg("%s: pclk too big\n", __func__);
		return -EINVAL;
	}

	pdiv *= hdiv;

	
	cfg->divs.p_divisor = pdiv;
	cfg->divs.h_divisor = hdiv;

	return 0      ;
}

static struct s3c_cpufreq_info s3c2410_cpufreq_info = {
	.max		= {
		.fclk	= 200000000,
		.hclk	= 100000000,
		.pclk	=  50000000,
	},

	
	.latency	= 10000000,

	.locktime_m	= 150,
	.locktime_u	= 150,
	.locktime_bits	= 12,

	.need_pll	= 1,

	.name		= "s3c2410",
	.calc_iotiming	= s3c2410_iotiming_calc,
	.set_iotiming	= s3c2410_iotiming_set,
	.get_iotiming	= s3c2410_iotiming_get,
	.resume_clocks	= s3c2410_setup_clocks,

	.set_fvco	= s3c2410_set_fvco,
	.set_refresh	= s3c2410_cpufreq_setrefresh,
	.set_divs	= s3c2410_cpufreq_setdivs,
	.calc_divs	= s3c2410_cpufreq_calcdivs,

	.debug_io_show	= s3c_cpufreq_debugfs_call(s3c2410_iotiming_debugfs),
};

static int s3c2410_cpufreq_add(struct sys_device *sysdev)
{
	return s3c_cpufreq_register(&s3c2410_cpufreq_info);
}

static struct sysdev_driver s3c2410_cpufreq_driver = {
	.add		= s3c2410_cpufreq_add,
};

static int __init s3c2410_cpufreq_init(void)
{
	return sysdev_driver_register(&s3c2410_sysclass,
				      &s3c2410_cpufreq_driver);
}

arch_initcall(s3c2410_cpufreq_init);

static int s3c2410a_cpufreq_add(struct sys_device *sysdev)
{
	

	s3c2410_cpufreq_info.max.fclk = 266000000;
	s3c2410_cpufreq_info.max.hclk = 133000000;
	s3c2410_cpufreq_info.max.pclk =  66500000;
	s3c2410_cpufreq_info.name = "s3c2410a";

	return s3c2410_cpufreq_add(sysdev);
}

static struct sysdev_driver s3c2410a_cpufreq_driver = {
	.add		= s3c2410a_cpufreq_add,
};

static int __init s3c2410a_cpufreq_init(void)
{
	return sysdev_driver_register(&s3c2410a_sysclass,
				      &s3c2410a_cpufreq_driver);
}

arch_initcall(s3c2410a_cpufreq_init);
