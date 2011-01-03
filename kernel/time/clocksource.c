

#include <linux/clocksource.h>
#include <linux/sysdev.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h> 
#include <linux/tick.h>
#include <linux/kthread.h>

void timecounter_init(struct timecounter *tc,
		      const struct cyclecounter *cc,
		      u64 start_tstamp)
{
	tc->cc = cc;
	tc->cycle_last = cc->read(cc);
	tc->nsec = start_tstamp;
}
EXPORT_SYMBOL(timecounter_init);


static u64 timecounter_read_delta(struct timecounter *tc)
{
	cycle_t cycle_now, cycle_delta;
	u64 ns_offset;

	
	cycle_now = tc->cc->read(tc->cc);

	
	cycle_delta = (cycle_now - tc->cycle_last) & tc->cc->mask;

	
	ns_offset = cyclecounter_cyc2ns(tc->cc, cycle_delta);

	
	tc->cycle_last = cycle_now;

	return ns_offset;
}

u64 timecounter_read(struct timecounter *tc)
{
	u64 nsec;

	
	nsec = timecounter_read_delta(tc);
	nsec += tc->nsec;
	tc->nsec = nsec;

	return nsec;
}
EXPORT_SYMBOL(timecounter_read);

u64 timecounter_cyc2time(struct timecounter *tc,
			 cycle_t cycle_tstamp)
{
	u64 cycle_delta = (cycle_tstamp - tc->cycle_last) & tc->cc->mask;
	u64 nsec;

	
	if (cycle_delta > tc->cc->mask / 2) {
		cycle_delta = (tc->cycle_last - cycle_tstamp) & tc->cc->mask;
		nsec = tc->nsec - cyclecounter_cyc2ns(tc->cc, cycle_delta);
	} else {
		nsec = cyclecounter_cyc2ns(tc->cc, cycle_delta) + tc->nsec;
	}

	return nsec;
}
EXPORT_SYMBOL(timecounter_cyc2time);


static struct clocksource *curr_clocksource;
static LIST_HEAD(clocksource_list);
static DEFINE_MUTEX(clocksource_mutex);
static char override_name[32];
static int finished_booting;

#ifdef CONFIG_CLOCKSOURCE_WATCHDOG
static void clocksource_watchdog_work(struct work_struct *work);

static LIST_HEAD(watchdog_list);
static struct clocksource *watchdog;
static struct timer_list watchdog_timer;
static DECLARE_WORK(watchdog_work, clocksource_watchdog_work);
static DEFINE_SPINLOCK(watchdog_lock);
static cycle_t watchdog_last;
static int watchdog_running;

static int clocksource_watchdog_kthread(void *data);
static void __clocksource_change_rating(struct clocksource *cs, int rating);


#define WATCHDOG_INTERVAL (HZ >> 1)
#define WATCHDOG_THRESHOLD (NSEC_PER_SEC >> 4)

static void clocksource_watchdog_work(struct work_struct *work)
{
	
	kthread_run(clocksource_watchdog_kthread, NULL, "kwatchdog");
}

static void __clocksource_unstable(struct clocksource *cs)
{
	cs->flags &= ~(CLOCK_SOURCE_VALID_FOR_HRES | CLOCK_SOURCE_WATCHDOG);
	cs->flags |= CLOCK_SOURCE_UNSTABLE;
	if (finished_booting)
		schedule_work(&watchdog_work);
}

static void clocksource_unstable(struct clocksource *cs, int64_t delta)
{
	printk(KERN_WARNING "Clocksource %s unstable (delta = %Ld ns)\n",
	       cs->name, delta);
	__clocksource_unstable(cs);
}


void clocksource_mark_unstable(struct clocksource *cs)
{
	unsigned long flags;

	spin_lock_irqsave(&watchdog_lock, flags);
	if (!(cs->flags & CLOCK_SOURCE_UNSTABLE)) {
		if (list_empty(&cs->wd_list))
			list_add(&cs->wd_list, &watchdog_list);
		__clocksource_unstable(cs);
	}
	spin_unlock_irqrestore(&watchdog_lock, flags);
}

static void clocksource_watchdog(unsigned long data)
{
	struct clocksource *cs;
	cycle_t csnow, wdnow;
	int64_t wd_nsec, cs_nsec;
	int next_cpu;

	spin_lock(&watchdog_lock);
	if (!watchdog_running)
		goto out;

	wdnow = watchdog->read(watchdog);
	wd_nsec = clocksource_cyc2ns((wdnow - watchdog_last) & watchdog->mask,
				     watchdog->mult, watchdog->shift);
	watchdog_last = wdnow;

	list_for_each_entry(cs, &watchdog_list, wd_list) {

		
		if (cs->flags & CLOCK_SOURCE_UNSTABLE) {
			if (finished_booting)
				schedule_work(&watchdog_work);
			continue;
		}

		csnow = cs->read(cs);

		
		if (!(cs->flags & CLOCK_SOURCE_WATCHDOG)) {
			cs->flags |= CLOCK_SOURCE_WATCHDOG;
			cs->wd_last = csnow;
			continue;
		}

		
		cs_nsec = clocksource_cyc2ns((csnow - cs->wd_last) &
					     cs->mask, cs->mult, cs->shift);
		cs->wd_last = csnow;
		if (abs(cs_nsec - wd_nsec) > WATCHDOG_THRESHOLD) {
			clocksource_unstable(cs, cs_nsec - wd_nsec);
			continue;
		}

		if (!(cs->flags & CLOCK_SOURCE_VALID_FOR_HRES) &&
		    (cs->flags & CLOCK_SOURCE_IS_CONTINUOUS) &&
		    (watchdog->flags & CLOCK_SOURCE_IS_CONTINUOUS)) {
			cs->flags |= CLOCK_SOURCE_VALID_FOR_HRES;
			
			tick_clock_notify();
		}
	}

	
	next_cpu = cpumask_next(raw_smp_processor_id(), cpu_online_mask);
	if (next_cpu >= nr_cpu_ids)
		next_cpu = cpumask_first(cpu_online_mask);
	watchdog_timer.expires += WATCHDOG_INTERVAL;
	add_timer_on(&watchdog_timer, next_cpu);
out:
	spin_unlock(&watchdog_lock);
}

static inline void clocksource_start_watchdog(void)
{
	if (watchdog_running || !watchdog || list_empty(&watchdog_list))
		return;
	init_timer(&watchdog_timer);
	watchdog_timer.function = clocksource_watchdog;
	watchdog_last = watchdog->read(watchdog);
	watchdog_timer.expires = jiffies + WATCHDOG_INTERVAL;
	add_timer_on(&watchdog_timer, cpumask_first(cpu_online_mask));
	watchdog_running = 1;
}

static inline void clocksource_stop_watchdog(void)
{
	if (!watchdog_running || (watchdog && !list_empty(&watchdog_list)))
		return;
	del_timer(&watchdog_timer);
	watchdog_running = 0;
}

static inline void clocksource_reset_watchdog(void)
{
	struct clocksource *cs;

	list_for_each_entry(cs, &watchdog_list, wd_list)
		cs->flags &= ~CLOCK_SOURCE_WATCHDOG;
}

static void clocksource_resume_watchdog(void)
{
	unsigned long flags;

	spin_lock_irqsave(&watchdog_lock, flags);
	clocksource_reset_watchdog();
	spin_unlock_irqrestore(&watchdog_lock, flags);
}

static void clocksource_enqueue_watchdog(struct clocksource *cs)
{
	unsigned long flags;

	spin_lock_irqsave(&watchdog_lock, flags);
	if (cs->flags & CLOCK_SOURCE_MUST_VERIFY) {
		
		list_add(&cs->wd_list, &watchdog_list);
		cs->flags &= ~CLOCK_SOURCE_WATCHDOG;
	} else {
		
		if (cs->flags & CLOCK_SOURCE_IS_CONTINUOUS)
			cs->flags |= CLOCK_SOURCE_VALID_FOR_HRES;
		
		if (!watchdog || cs->rating > watchdog->rating) {
			watchdog = cs;
			
			clocksource_reset_watchdog();
		}
	}
	
	clocksource_start_watchdog();
	spin_unlock_irqrestore(&watchdog_lock, flags);
}

static void clocksource_dequeue_watchdog(struct clocksource *cs)
{
	struct clocksource *tmp;
	unsigned long flags;

	spin_lock_irqsave(&watchdog_lock, flags);
	if (cs->flags & CLOCK_SOURCE_MUST_VERIFY) {
		
		list_del_init(&cs->wd_list);
	} else if (cs == watchdog) {
		
		clocksource_reset_watchdog();
		
		watchdog = NULL;
		list_for_each_entry(tmp, &clocksource_list, list) {
			if (tmp == cs || tmp->flags & CLOCK_SOURCE_MUST_VERIFY)
				continue;
			if (!watchdog || tmp->rating > watchdog->rating)
				watchdog = tmp;
		}
	}
	cs->flags &= ~CLOCK_SOURCE_WATCHDOG;
	
	clocksource_stop_watchdog();
	spin_unlock_irqrestore(&watchdog_lock, flags);
}

static int clocksource_watchdog_kthread(void *data)
{
	struct clocksource *cs, *tmp;
	unsigned long flags;
	LIST_HEAD(unstable);

	mutex_lock(&clocksource_mutex);
	spin_lock_irqsave(&watchdog_lock, flags);
	list_for_each_entry_safe(cs, tmp, &watchdog_list, wd_list)
		if (cs->flags & CLOCK_SOURCE_UNSTABLE) {
			list_del_init(&cs->wd_list);
			list_add(&cs->wd_list, &unstable);
		}
	
	clocksource_stop_watchdog();
	spin_unlock_irqrestore(&watchdog_lock, flags);

	
	list_for_each_entry_safe(cs, tmp, &unstable, wd_list) {
		list_del_init(&cs->wd_list);
		__clocksource_change_rating(cs, 0);
	}
	mutex_unlock(&clocksource_mutex);
	return 0;
}

#else 

static void clocksource_enqueue_watchdog(struct clocksource *cs)
{
	if (cs->flags & CLOCK_SOURCE_IS_CONTINUOUS)
		cs->flags |= CLOCK_SOURCE_VALID_FOR_HRES;
}

static inline void clocksource_dequeue_watchdog(struct clocksource *cs) { }
static inline void clocksource_resume_watchdog(void) { }
static inline int clocksource_watchdog_kthread(void *data) { return 0; }

#endif 


void clocksource_resume(void)
{
	struct clocksource *cs;

	list_for_each_entry(cs, &clocksource_list, list)
		if (cs->resume)
			cs->resume();

	clocksource_resume_watchdog();
}


void clocksource_touch_watchdog(void)
{
	clocksource_resume_watchdog();
}


static u64 clocksource_max_deferment(struct clocksource *cs)
{
	u64 max_nsecs, max_cycles;

	
	max_cycles = 1ULL << (63 - (ilog2(cs->mult) + 1));

	
	max_cycles = min_t(u64, max_cycles, (u64) cs->mask);
	max_nsecs = clocksource_cyc2ns(max_cycles, cs->mult, cs->shift);

	
	return max_nsecs - (max_nsecs >> 5);
}

#ifdef CONFIG_GENERIC_TIME


static void clocksource_select(void)
{
	struct clocksource *best, *cs;

	if (!finished_booting || list_empty(&clocksource_list))
		return;
	
	best = list_first_entry(&clocksource_list, struct clocksource, list);
	
	list_for_each_entry(cs, &clocksource_list, list) {
		if (strcmp(cs->name, override_name) != 0)
			continue;
		
		if (!(cs->flags & CLOCK_SOURCE_VALID_FOR_HRES) &&
		    tick_oneshot_mode_active()) {
			
			printk(KERN_WARNING "Override clocksource %s is not "
			       "HRT compatible. Cannot switch while in "
			       "HRT/NOHZ mode\n", cs->name);
			override_name[0] = 0;
		} else
			
			best = cs;
		break;
	}
	if (curr_clocksource != best) {
		printk(KERN_INFO "Switching to clocksource %s\n", best->name);
		curr_clocksource = best;
		timekeeping_notify(curr_clocksource);
	}
}

#else 

static inline void clocksource_select(void) { }

#endif


static int __init clocksource_done_booting(void)
{
	finished_booting = 1;

	
	clocksource_watchdog_kthread(NULL);

	mutex_lock(&clocksource_mutex);
	clocksource_select();
	mutex_unlock(&clocksource_mutex);
	return 0;
}
fs_initcall(clocksource_done_booting);


static void clocksource_enqueue(struct clocksource *cs)
{
	struct list_head *entry = &clocksource_list;
	struct clocksource *tmp;

	list_for_each_entry(tmp, &clocksource_list, list)
		
		if (tmp->rating >= cs->rating)
			entry = &tmp->list;
	list_add(&cs->list, entry);
}


int clocksource_register(struct clocksource *cs)
{
	
	cs->max_idle_ns = clocksource_max_deferment(cs);

	mutex_lock(&clocksource_mutex);
	clocksource_enqueue(cs);
	clocksource_select();
	clocksource_enqueue_watchdog(cs);
	mutex_unlock(&clocksource_mutex);
	return 0;
}
EXPORT_SYMBOL(clocksource_register);

static void __clocksource_change_rating(struct clocksource *cs, int rating)
{
	list_del(&cs->list);
	cs->rating = rating;
	clocksource_enqueue(cs);
	clocksource_select();
}


void clocksource_change_rating(struct clocksource *cs, int rating)
{
	mutex_lock(&clocksource_mutex);
	__clocksource_change_rating(cs, rating);
	mutex_unlock(&clocksource_mutex);
}
EXPORT_SYMBOL(clocksource_change_rating);


void clocksource_unregister(struct clocksource *cs)
{
	mutex_lock(&clocksource_mutex);
	clocksource_dequeue_watchdog(cs);
	list_del(&cs->list);
	clocksource_select();
	mutex_unlock(&clocksource_mutex);
}
EXPORT_SYMBOL(clocksource_unregister);

#ifdef CONFIG_SYSFS

static ssize_t
sysfs_show_current_clocksources(struct sys_device *dev,
				struct sysdev_attribute *attr, char *buf)
{
	ssize_t count = 0;

	mutex_lock(&clocksource_mutex);
	count = snprintf(buf, PAGE_SIZE, "%s\n", curr_clocksource->name);
	mutex_unlock(&clocksource_mutex);

	return count;
}


static ssize_t sysfs_override_clocksource(struct sys_device *dev,
					  struct sysdev_attribute *attr,
					  const char *buf, size_t count)
{
	size_t ret = count;

	
	if (count >= sizeof(override_name))
		return -EINVAL;

	
	if (buf[count-1] == '\n')
		count--;

	mutex_lock(&clocksource_mutex);

	if (count > 0)
		memcpy(override_name, buf, count);
	override_name[count] = 0;
	clocksource_select();

	mutex_unlock(&clocksource_mutex);

	return ret;
}


static ssize_t
sysfs_show_available_clocksources(struct sys_device *dev,
				  struct sysdev_attribute *attr,
				  char *buf)
{
	struct clocksource *src;
	ssize_t count = 0;

	mutex_lock(&clocksource_mutex);
	list_for_each_entry(src, &clocksource_list, list) {
		
		if (!tick_oneshot_mode_active() ||
		    (src->flags & CLOCK_SOURCE_VALID_FOR_HRES))
			count += snprintf(buf + count,
				  max((ssize_t)PAGE_SIZE - count, (ssize_t)0),
				  "%s ", src->name);
	}
	mutex_unlock(&clocksource_mutex);

	count += snprintf(buf + count,
			  max((ssize_t)PAGE_SIZE - count, (ssize_t)0), "\n");

	return count;
}


static SYSDEV_ATTR(current_clocksource, 0644, sysfs_show_current_clocksources,
		   sysfs_override_clocksource);

static SYSDEV_ATTR(available_clocksource, 0444,
		   sysfs_show_available_clocksources, NULL);

static struct sysdev_class clocksource_sysclass = {
	.name = "clocksource",
};

static struct sys_device device_clocksource = {
	.id	= 0,
	.cls	= &clocksource_sysclass,
};

static int __init init_clocksource_sysfs(void)
{
	int error = sysdev_class_register(&clocksource_sysclass);

	if (!error)
		error = sysdev_register(&device_clocksource);
	if (!error)
		error = sysdev_create_file(
				&device_clocksource,
				&attr_current_clocksource);
	if (!error)
		error = sysdev_create_file(
				&device_clocksource,
				&attr_available_clocksource);
	return error;
}

device_initcall(init_clocksource_sysfs);
#endif 


static int __init boot_override_clocksource(char* str)
{
	mutex_lock(&clocksource_mutex);
	if (str)
		strlcpy(override_name, str, sizeof(override_name));
	mutex_unlock(&clocksource_mutex);
	return 1;
}

__setup("clocksource=", boot_override_clocksource);


static int __init boot_override_clock(char* str)
{
	if (!strcmp(str, "pmtmr")) {
		printk("Warning: clock=pmtmr is deprecated. "
			"Use clocksource=acpi_pm.\n");
		return boot_override_clocksource("acpi_pm");
	}
	printk("Warning! clock= boot option is deprecated. "
		"Use clocksource=xyz\n");
	return boot_override_clocksource(str);
}

__setup("clock=", boot_override_clock);
