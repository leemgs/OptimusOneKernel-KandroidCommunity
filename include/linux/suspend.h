#ifndef _LINUX_SUSPEND_H
#define _LINUX_SUSPEND_H

#include <linux/swap.h>
#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/pm.h>
#include <linux/mm.h>
#include <asm/errno.h>

#if defined(CONFIG_PM_SLEEP) && defined(CONFIG_VT) && defined(CONFIG_VT_CONSOLE)
extern void pm_set_vt_switch(int);
extern int pm_prepare_console(void);
extern void pm_restore_console(void);
#else
static inline void pm_set_vt_switch(int do_switch)
{
}

static inline int pm_prepare_console(void)
{
	return 0;
}

static inline void pm_restore_console(void)
{
}
#endif

typedef int __bitwise suspend_state_t;

#define PM_SUSPEND_ON		((__force suspend_state_t) 0)
#define PM_SUSPEND_STANDBY	((__force suspend_state_t) 1)
#define PM_SUSPEND_MEM		((__force suspend_state_t) 3)
#define PM_SUSPEND_MAX		((__force suspend_state_t) 4)


struct platform_suspend_ops {
	int (*valid)(suspend_state_t state);
	int (*begin)(suspend_state_t state);
	int (*prepare)(void);
	int (*prepare_late)(void);
	int (*enter)(suspend_state_t state);
	void (*wake)(void);
	void (*finish)(void);
	void (*end)(void);
	void (*recover)(void);
};

#ifdef CONFIG_SUSPEND

extern void suspend_set_ops(struct platform_suspend_ops *ops);
extern int suspend_valid_only_mem(suspend_state_t state);


extern void arch_suspend_disable_irqs(void);


extern void arch_suspend_enable_irqs(void);

extern int pm_suspend(suspend_state_t state);
#else 
#define suspend_valid_only_mem	NULL

static inline void suspend_set_ops(struct platform_suspend_ops *ops) {}
static inline int pm_suspend(suspend_state_t state) { return -ENOSYS; }
#endif 


struct pbe {
	void *address;		
	void *orig_address;	
	struct pbe *next;
};


extern void mark_free_pages(struct zone *zone);


struct platform_hibernation_ops {
	int (*begin)(void);
	void (*end)(void);
	int (*pre_snapshot)(void);
	void (*finish)(void);
	int (*prepare)(void);
	int (*enter)(void);
	void (*leave)(void);
	int (*pre_restore)(void);
	void (*restore_cleanup)(void);
	void (*recover)(void);
};

#ifdef CONFIG_HIBERNATION

extern void __register_nosave_region(unsigned long b, unsigned long e, int km);
static inline void __init register_nosave_region(unsigned long b, unsigned long e)
{
	__register_nosave_region(b, e, 0);
}
static inline void __init register_nosave_region_late(unsigned long b, unsigned long e)
{
	__register_nosave_region(b, e, 1);
}
extern int swsusp_page_is_forbidden(struct page *);
extern void swsusp_set_page_free(struct page *);
extern void swsusp_unset_page_free(struct page *);
extern unsigned long get_safe_page(gfp_t gfp_mask);

extern void hibernation_set_ops(struct platform_hibernation_ops *ops);
extern int hibernate(void);
extern bool system_entering_hibernation(void);
#else 
static inline int swsusp_page_is_forbidden(struct page *p) { return 0; }
static inline void swsusp_set_page_free(struct page *p) {}
static inline void swsusp_unset_page_free(struct page *p) {}

static inline void hibernation_set_ops(struct platform_hibernation_ops *ops) {}
static inline int hibernate(void) { return -ENOSYS; }
static inline bool system_entering_hibernation(void) { return false; }
#endif 

#ifdef CONFIG_HIBERNATION_NVS
extern int hibernate_nvs_register(unsigned long start, unsigned long size);
extern int hibernate_nvs_alloc(void);
extern void hibernate_nvs_free(void);
extern void hibernate_nvs_save(void);
extern void hibernate_nvs_restore(void);
#else 
static inline int hibernate_nvs_register(unsigned long a, unsigned long b)
{
	return 0;
}
static inline int hibernate_nvs_alloc(void) { return 0; }
static inline void hibernate_nvs_free(void) {}
static inline void hibernate_nvs_save(void) {}
static inline void hibernate_nvs_restore(void) {}
#endif 

#ifdef CONFIG_PM_SLEEP
void save_processor_state(void);
void restore_processor_state(void);


extern int register_pm_notifier(struct notifier_block *nb);
extern int unregister_pm_notifier(struct notifier_block *nb);

#define pm_notifier(fn, pri) {				\
	static struct notifier_block fn##_nb =			\
		{ .notifier_call = fn, .priority = pri };	\
	register_pm_notifier(&fn##_nb);			\
}
#else 

static inline int register_pm_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline int unregister_pm_notifier(struct notifier_block *nb)
{
	return 0;
}

#define pm_notifier(fn, pri)	do { (void)(fn); } while (0)
#endif 

extern struct mutex pm_mutex;

#ifndef CONFIG_HIBERNATION
static inline void register_nosave_region(unsigned long b, unsigned long e)
{
}
static inline void register_nosave_region_late(unsigned long b, unsigned long e)
{
}

static inline void lock_system_sleep(void) {}
static inline void unlock_system_sleep(void) {}

#else



static inline void lock_system_sleep(void)
{
	mutex_lock(&pm_mutex);
}

static inline void unlock_system_sleep(void)
{
	mutex_unlock(&pm_mutex);
}
#endif

#endif 
