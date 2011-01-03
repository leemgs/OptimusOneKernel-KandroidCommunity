
#ifndef _LINUX_CPU_H_
#define _LINUX_CPU_H_

#include <linux/sysdev.h>
#include <linux/node.h>
#include <linux/compiler.h>
#include <linux/cpumask.h>

struct cpu {
	int node_id;		
	int hotpluggable;	
	struct sys_device sysdev;
};

extern int register_cpu(struct cpu *cpu, int num);
extern struct sys_device *get_cpu_sysdev(unsigned cpu);

extern int cpu_add_sysdev_attr(struct sysdev_attribute *attr);
extern void cpu_remove_sysdev_attr(struct sysdev_attribute *attr);

extern int cpu_add_sysdev_attr_group(struct attribute_group *attrs);
extern void cpu_remove_sysdev_attr_group(struct attribute_group *attrs);

extern int sched_create_sysfs_power_savings_entries(struct sysdev_class *cls);

#ifdef CONFIG_HOTPLUG_CPU
extern void unregister_cpu(struct cpu *cpu);
#endif
struct notifier_block;

#ifdef CONFIG_SMP

#if defined(CONFIG_HOTPLUG_CPU) || !defined(MODULE)
#define cpu_notifier(fn, pri) {					\
	static struct notifier_block fn##_nb __cpuinitdata =	\
		{ .notifier_call = fn, .priority = pri };	\
	register_cpu_notifier(&fn##_nb);			\
}
#else 
#define cpu_notifier(fn, pri)	do { (void)(fn); } while (0)
#endif 
#ifdef CONFIG_HOTPLUG_CPU
extern int register_cpu_notifier(struct notifier_block *nb);
extern void unregister_cpu_notifier(struct notifier_block *nb);
#else

#ifndef MODULE
extern int register_cpu_notifier(struct notifier_block *nb);
#else
static inline int register_cpu_notifier(struct notifier_block *nb)
{
	return 0;
}
#endif

static inline void unregister_cpu_notifier(struct notifier_block *nb)
{
}
#endif

int cpu_up(unsigned int cpu);
void notify_cpu_starting(unsigned int cpu);
extern void cpu_maps_update_begin(void);
extern void cpu_maps_update_done(void);

#else	

#define cpu_notifier(fn, pri)	do { (void)(fn); } while (0)

static inline int register_cpu_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline void unregister_cpu_notifier(struct notifier_block *nb)
{
}

static inline void cpu_maps_update_begin(void)
{
}

static inline void cpu_maps_update_done(void)
{
}

#endif 
extern struct sysdev_class cpu_sysdev_class;

#ifdef CONFIG_HOTPLUG_CPU


extern void get_online_cpus(void);
extern void put_online_cpus(void);
#define hotcpu_notifier(fn, pri)	cpu_notifier(fn, pri)
#define register_hotcpu_notifier(nb)	register_cpu_notifier(nb)
#define unregister_hotcpu_notifier(nb)	unregister_cpu_notifier(nb)
int cpu_down(unsigned int cpu);

#else		

#define get_online_cpus()	do { } while (0)
#define put_online_cpus()	do { } while (0)
#define hotcpu_notifier(fn, pri)	do { (void)(fn); } while (0)

#define register_hotcpu_notifier(nb)	({ (void)(nb); 0; })
#define unregister_hotcpu_notifier(nb)	({ (void)(nb); })
#endif		

#ifdef CONFIG_PM_SLEEP_SMP
extern int suspend_cpu_hotplug;

extern int disable_nonboot_cpus(void);
extern void enable_nonboot_cpus(void);
#else 
#define suspend_cpu_hotplug	0

static inline int disable_nonboot_cpus(void) { return 0; }
static inline void enable_nonboot_cpus(void) {}
#endif 

#endif 
