

#include <linux/init.h>
#include <linux/cpumask.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#include <asm/hardware/gic.h>
#include <asm/cacheflush.h>
#include <asm/mach-types.h>

#include <mach/smp.h>
#include <mach/hardware.h>
#include <mach/msm_iomap.h>

#define SECONDARY_CPU_WAIT_MS 10

int pen_release = -1;

int get_core_count(void)
{
#ifdef CONFIG_NR_CPUS
	return CONFIG_NR_CPUS;
#else
	return 1;
#endif
}


void smp_prepare_cpus(unsigned int max_cpus)
{
	int i;

	for (i = 0; i < max_cpus; i++)
		cpu_set(i, cpu_present_map);
}

void smp_init_cpus(void)
{
	unsigned int i, ncores = get_core_count();

	for (i = 0; i < ncores; i++)
		cpu_set(i, cpu_possible_map);
}


int boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	int cnt = 0;
	printk(KERN_DEBUG "Starting secondary CPU %d\n", cpu);

	
	pen_release = cpu;
	dmac_clean_range((void *)&pen_release,
			 (void *)(&pen_release + sizeof(pen_release)));
	dmac_clean_range((void *)&secondary_data,
			 (void *)(&secondary_data + sizeof(secondary_data)));
	sev();
	dsb();

	
	while (pen_release != 0xFFFFFFFF) {
		dmac_inv_range((void *)&pen_release,
			       (void *)(&pen_release+sizeof(pen_release)));
		msleep_interruptible(1);
		if (cnt++ >= SECONDARY_CPU_WAIT_MS)
			break;
	}

	if (pen_release == 0xFFFFFFFF)
		printk(KERN_DEBUG "Secondary CPU start acked %d\n", cpu);
	else
		printk(KERN_ERR "Secondary CPU failed to start..." \
		       "continuing\n");

	return 0;
}


void platform_secondary_init(unsigned int cpu)
{
	printk(KERN_DEBUG "%s: cpu:%d\n", __func__, cpu);

	trace_hardirqs_off();

	
	writel(0xFFFFD7FF, MSM_QGIC_DIST_BASE + GIC_DIST_CONFIG + 4);

	
	if (machine_is_msm8x60_surf() ||
	    machine_is_msm8x60_ffa()  ||
	    machine_is_msm8x60_rumi3())
		writel(0x0000FFFF, MSM_QGIC_DIST_BASE + GIC_DIST_ENABLE_SET);

	
	gic_cpu_init(0, MSM_QGIC_CPU_BASE);
}
