

#include <linux/clocksource.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioport.h>
#include <linux/scx200.h>

#define NAME "scx200_hrt"

static int mhz27;
module_param(mhz27, int, 0);	
MODULE_PARM_DESC(mhz27, "count at 27.0 MHz (default is 1.0 MHz)");

static int ppm;
module_param(ppm, int, 0);	
MODULE_PARM_DESC(ppm, "+-adjust to actual XO freq (ppm)");


#define SCx200_TMCNFG_OFFSET (SCx200_TIMER_OFFSET + 5)


#define HR_TMEN (1 << 0)	
#define HR_TMCLKSEL (1 << 1)	
#define HR_TM27MPD (1 << 2)	


#define HRT_FREQ   1000000

static cycle_t read_hrt(struct clocksource *cs)
{
	
	return (cycle_t) inl(scx200_cb_base + SCx200_TIMER_OFFSET);
}

#define HRT_SHIFT_1	22
#define HRT_SHIFT_27	26

static struct clocksource cs_hrt = {
	.name		= "scx200_hrt",
	.rating		= 250,
	.read		= read_hrt,
	.mask		= CLOCKSOURCE_MASK(32),
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
	
};

static int __init init_hrt_clocksource(void)
{
	
	if (!scx200_cb_present())
		return -ENODEV;

	
	if (!request_region(scx200_cb_base + SCx200_TIMER_OFFSET,
			    SCx200_TIMER_SIZE,
			    "NatSemi SCx200 High-Resolution Timer")) {
		printk(KERN_WARNING NAME ": unable to lock timer region\n");
		return -ENODEV;
	}

	
	outb(HR_TMEN | (mhz27 ? HR_TMCLKSEL : 0),
	     scx200_cb_base + SCx200_TMCNFG_OFFSET);

	if (mhz27) {
		cs_hrt.shift = HRT_SHIFT_27;
		cs_hrt.mult = clocksource_hz2mult((HRT_FREQ + ppm) * 27,
						  cs_hrt.shift);
	} else {
		cs_hrt.shift = HRT_SHIFT_1;
		cs_hrt.mult = clocksource_hz2mult(HRT_FREQ + ppm,
						  cs_hrt.shift);
	}
	printk(KERN_INFO "enabling scx200 high-res timer (%s MHz +%d ppm)\n",
		mhz27 ? "27":"1", ppm);

	return clocksource_register(&cs_hrt);
}

module_init(init_hrt_clocksource);

MODULE_AUTHOR("Jim Cromie <jim.cromie@gmail.com>");
MODULE_DESCRIPTION("clocksource on SCx200 HiRes Timer");
MODULE_LICENSE("GPL");
