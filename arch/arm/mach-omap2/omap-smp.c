
#include <linux/init.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/smp.h>
#include <linux/io.h>

#include <asm/localtimer.h>
#include <asm/smp_scu.h>
#include <mach/hardware.h>


#define OMAP4_AUXCOREBOOT_REG0		(OMAP44XX_VA_WKUPGEN_BASE + 0x800)
#define OMAP4_AUXCOREBOOT_REG1		(OMAP44XX_VA_WKUPGEN_BASE + 0x804)


static void __iomem *scu_base = OMAP44XX_VA_SCU_BASE;


static inline unsigned int get_core_count(void)
{
	if (scu_base)
		return scu_get_core_count(scu_base);
	return 1;
}

static DEFINE_SPINLOCK(boot_lock);

void __cpuinit platform_secondary_init(unsigned int cpu)
{
	trace_hardirqs_off();

	

	gic_cpu_init(0, OMAP2_IO_ADDRESS(OMAP44XX_GIC_CPU_BASE));

	
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;

	
	spin_lock(&boot_lock);

	
	__raw_writel(cpu, OMAP4_AUXCOREBOOT_REG1);
	smp_wmb();

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout))
		;

	
	spin_unlock(&boot_lock);

	return 0;
}

static void __init wakeup_secondary(void)
{
	
	__raw_writel(virt_to_phys(omap_secondary_startup),	   \
					OMAP4_AUXCOREBOOT_REG0);
	smp_wmb();

	
	set_event();
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
		       "OMAP4: strange core count of 0? Default to 1\n");
		ncores = 1;
	}

	if (ncores > NR_CPUS) {
		printk(KERN_WARNING
		       "OMAP4: no. of cores (%d) greater than configured "
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

		
		scu_enable(scu_base);
		wakeup_secondary();
	}
}
