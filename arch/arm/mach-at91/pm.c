

#include <linux/suspend.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <asm/atomic.h>
#include <asm/mach/time.h>
#include <asm/mach/irq.h>

#include <mach/at91_pmc.h>
#include <mach/gpio.h>
#include <mach/cpu.h>

#include "generic.h"

#ifdef CONFIG_ARCH_AT91RM9200
#include <mach/at91rm9200_mc.h>


#define sdram_selfrefresh_enable()	at91_sys_write(AT91_SDRAMC_SRR, 1)
#define sdram_selfrefresh_disable()	do {} while (0)

#elif defined(CONFIG_ARCH_AT91CAP9)
#include <mach/at91cap9_ddrsdr.h>

static u32 saved_lpr;

static inline void sdram_selfrefresh_enable(void)
{
	u32 lpr;

	saved_lpr = at91_sys_read(AT91_DDRSDRC_LPR);

	lpr = saved_lpr & ~AT91_DDRSDRC_LPCB;
	at91_sys_write(AT91_DDRSDRC_LPR, lpr | AT91_DDRSDRC_LPCB_SELF_REFRESH);
}

#define sdram_selfrefresh_disable()	at91_sys_write(AT91_DDRSDRC_LPR, saved_lpr)

#else
#include <mach/at91sam9_sdramc.h>

#ifdef CONFIG_ARCH_AT91SAM9263

#define	AT91_SDRAMC	AT91_SDRAMC0
#warning Assuming EB1 SDRAM controller is *NOT* used
#endif

static u32 saved_lpr;

static inline void sdram_selfrefresh_enable(void)
{
	u32 lpr;

	saved_lpr = at91_sys_read(AT91_SDRAMC_LPR);

	lpr = saved_lpr & ~AT91_SDRAMC_LPCB;
	at91_sys_write(AT91_SDRAMC_LPR, lpr | AT91_SDRAMC_LPCB_SELF_REFRESH);
}

#define sdram_selfrefresh_disable()	at91_sys_write(AT91_SDRAMC_LPR, saved_lpr)

#endif



#if defined(AT91_SHDWC)

#include <mach/at91_rstc.h>
#include <mach/at91_shdwc.h>

static void __init show_reset_status(void)
{
	static char reset[] __initdata = "reset";

	static char general[] __initdata = "general";
	static char wakeup[] __initdata = "wakeup";
	static char watchdog[] __initdata = "watchdog";
	static char software[] __initdata = "software";
	static char user[] __initdata = "user";
	static char unknown[] __initdata = "unknown";

	static char signal[] __initdata = "signal";
	static char rtc[] __initdata = "rtc";
	static char rtt[] __initdata = "rtt";
	static char restore[] __initdata = "power-restored";

	char *reason, *r2 = reset;
	u32 reset_type, wake_type;

	reset_type = at91_sys_read(AT91_RSTC_SR) & AT91_RSTC_RSTTYP;
	wake_type = at91_sys_read(AT91_SHDW_SR);

	switch (reset_type) {
	case AT91_RSTC_RSTTYP_GENERAL:
		reason = general;
		break;
	case AT91_RSTC_RSTTYP_WAKEUP:
		
		reason = wakeup;

		
		if (wake_type & AT91_SHDW_WAKEUP0)
			r2 = signal;
		else {
			r2 = reason;
			if (wake_type & AT91_SHDW_RTTWK)	
				reason = rtt;
			else if (wake_type & AT91_SHDW_RTCWK)	
				reason = rtc;
			else if (wake_type == 0)	
				reason = restore;
			else				
				reason = unknown;
		}
		break;
	case AT91_RSTC_RSTTYP_WATCHDOG:
		reason = watchdog;
		break;
	case AT91_RSTC_RSTTYP_SOFTWARE:
		reason = software;
		break;
	case AT91_RSTC_RSTTYP_USER:
		reason = user;
		break;
	default:
		reason = unknown;
		break;
	}
	pr_info("AT91: Starting after %s %s\n", reason, r2);
}
#else
static void __init show_reset_status(void) {}
#endif


static int at91_pm_valid_state(suspend_state_t state)
{
	switch (state) {
		case PM_SUSPEND_ON:
		case PM_SUSPEND_STANDBY:
		case PM_SUSPEND_MEM:
			return 1;

		default:
			return 0;
	}
}


static suspend_state_t target_state;


static int at91_pm_begin(suspend_state_t state)
{
	target_state = state;
	return 0;
}


static int at91_pm_verify_clocks(void)
{
	unsigned long scsr;
	int i;

	scsr = at91_sys_read(AT91_PMC_SCSR);

	
	if (cpu_is_at91rm9200()) {
		if ((scsr & (AT91RM9200_PMC_UHP | AT91RM9200_PMC_UDP)) != 0) {
			pr_err("AT91: PM - Suspend-to-RAM with USB still active\n");
			return 0;
		}
	} else if (cpu_is_at91sam9260() || cpu_is_at91sam9261() || cpu_is_at91sam9263()
			|| cpu_is_at91sam9g20() || cpu_is_at91sam9g10()) {
		if ((scsr & (AT91SAM926x_PMC_UHP | AT91SAM926x_PMC_UDP)) != 0) {
			pr_err("AT91: PM - Suspend-to-RAM with USB still active\n");
			return 0;
		}
	} else if (cpu_is_at91cap9()) {
		if ((scsr & AT91CAP9_PMC_UHP) != 0) {
			pr_err("AT91: PM - Suspend-to-RAM with USB still active\n");
			return 0;
		}
	}

#ifdef CONFIG_AT91_PROGRAMMABLE_CLOCKS
	
	for (i = 0; i < 4; i++) {
		u32 css;

		if ((scsr & (AT91_PMC_PCK0 << i)) == 0)
			continue;

		css = at91_sys_read(AT91_PMC_PCKR(i)) & AT91_PMC_CSS;
		if (css != AT91_PMC_CSS_SLOW) {
			pr_err("AT91: PM - Suspend-to-RAM with PCK%d src %d\n", i, css);
			return 0;
		}
	}
#endif

	return 1;
}


int at91_suspend_entering_slow_clock(void)
{
	return (target_state == PM_SUSPEND_MEM);
}
EXPORT_SYMBOL(at91_suspend_entering_slow_clock);


static void (*slow_clock)(void);

#ifdef CONFIG_AT91_SLOW_CLOCK
extern void at91_slow_clock(void);
extern u32 at91_slow_clock_sz;
#endif


static int at91_pm_enter(suspend_state_t state)
{
	at91_gpio_suspend();
	at91_irq_suspend();

	pr_debug("AT91: PM - wake mask %08x, pm state %d\n",
			
			(at91_sys_read(AT91_PMC_PCSR)
					| (1 << AT91_ID_FIQ)
					| (1 << AT91_ID_SYS)
					| (at91_extern_irq))
				& at91_sys_read(AT91_AIC_IMR),
			state);

	switch (state) {
		
		case PM_SUSPEND_MEM:
			
			if (!at91_pm_verify_clocks())
				goto error;

			
			if (slow_clock) {
#ifdef CONFIG_AT91_SLOW_CLOCK
				
				memcpy(slow_clock, at91_slow_clock, at91_slow_clock_sz);
#endif
				slow_clock();
				break;
			} else {
				pr_info("AT91: PM - no slow clock mode enabled ...\n");
				
			}

		
		case PM_SUSPEND_STANDBY:
			
			asm("b 1f; .align 5; 1:");
			asm("mcr p15, 0, r0, c7, c10, 4");	
			sdram_selfrefresh_enable();
			asm("mcr p15, 0, r0, c7, c0, 4");	
			sdram_selfrefresh_disable();
			break;

		case PM_SUSPEND_ON:
			asm("mcr p15, 0, r0, c7, c0, 4");	
			break;

		default:
			pr_debug("AT91: PM - bogus suspend state %d\n", state);
			goto error;
	}

	pr_debug("AT91: PM - wakeup %08x\n",
			at91_sys_read(AT91_AIC_IPR) & at91_sys_read(AT91_AIC_IMR));

error:
	target_state = PM_SUSPEND_ON;
	at91_irq_resume();
	at91_gpio_resume();
	return 0;
}


static void at91_pm_end(void)
{
	target_state = PM_SUSPEND_ON;
}


static struct platform_suspend_ops at91_pm_ops ={
	.valid	= at91_pm_valid_state,
	.begin	= at91_pm_begin,
	.enter	= at91_pm_enter,
	.end	= at91_pm_end,
};

static int __init at91_pm_init(void)
{
#ifdef CONFIG_AT91_SLOW_CLOCK
	slow_clock = (void *) (AT91_IO_VIRT_BASE - at91_slow_clock_sz);
#endif

	pr_info("AT91: Power Management%s\n", (slow_clock ? " (with slow clock mode)" : ""));

#ifdef CONFIG_ARCH_AT91RM9200
	
	at91_sys_write(AT91_SDRAMC_LPR, 0);
#endif

	suspend_set_ops(&at91_pm_ops);

	show_reset_status();
	return 0;
}
arch_initcall(at91_pm_init);
