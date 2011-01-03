
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/smp.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/localtimer.h>
#include <asm/unified.h>

#include <mach/board-eb.h>
#include <mach/board-pb11mp.h>
#include <mach/board-pbx.h>
#include <asm/smp_scu.h>

#include "core.h"

extern void realview_secondary_startup(void);


volatile int __cpuinitdata pen_release = -1;

static void __iomem *scu_base_addr(void)
{
	if (machine_is_realview_eb_mp())
		return __io_address(REALVIEW_EB11MP_SCU_BASE);
	else if (machine_is_realview_pb11mp())
		return __io_address(REALVIEW_TC11MP_SCU_BASE);
	else if (machine_is_realview_pbx() &&
		 (core_tile_pbx11mp() || core_tile_pbxa9mp()))
		return __io_address(REALVIEW_PBX_TILE_SCU_BASE);
	else
		return (void __iomem *)0;
}

static inline unsigned int get_core_count(void)
{
	void __iomem *scu_base = scu_base_addr();
	if (scu_base)
		return scu_get_core_count(scu_base);
	return 1;
}

static DEFINE_SPINLOCK(boot_lock);

void __cpuinit platform_secondary_init(unsigned int cpu)
{
	trace_hardirqs_off();

	
	gic_cpu_init(0, gic_cpu_base_addr);

	
	pen_release = -1;
	smp_wmb();

	
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;

	
	spin_lock(&boot_lock);

	
	pen_release = cpu;
	flush_cache_all();

	
	smp_cross_call(cpumask_of(cpu));

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
		smp_rmb();
		if (pen_release == -1)
			break;

		udelay(10);
	}

	
	spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}

static void __init poke_milo(void)
{
	
	pen_release = -1;

	
	__raw_writel(BSYM(virt_to_phys(realview_secondary_startup)),
		     __io_address(REALVIEW_SYS_FLAGSSET));

	mb();
}


void __init smp_init_cpus(void)
{
	unsigned int i, ncores = get_core_count();

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);
}

void __init smp_prepare_cpus(unsigned int max_cpus)
{
	unsigned int ncores = get_core_count();
	unsigned int cpu = smp_processor_id();
	int i;

	
	if (ncores == 0) {
		printk(KERN_ERR
		       "Realview: strange CM count of 0? Default to 1\n");

		ncores = 1;
	}

	if (ncores > NR_CPUS) {
		printk(KERN_WARNING
		       "Realview: no. of cores (%d) greater than configured "
		       "maximum of %d - clipping\n",
		       ncores, NR_CPUS);
		ncores = NR_CPUS;
	}

	smp_store_cpu_info(cpu);

	
	if (max_cpus > ncores)
		max_cpus = ncores;

	
	for (i = 0; i < max_cpus; i++)
		set_cpu_present(i, true);

	
	if (max_cpus > 1) {
		
		percpu_timer_setup();

		scu_enable(scu_base_addr());
		poke_milo();
	}
}
