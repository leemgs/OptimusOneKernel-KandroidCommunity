

#include <linux/kernel.h>
#include <linux/cpuidle.h>
#include <linux/pm_qos_params.h>
#include <linux/time.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/tick.h>
#include <linux/sched.h>
#include <linux/math64.h>

#define BUCKETS 12
#define RESOLUTION 1024
#define DECAY 4
#define MAX_INTERESTING 50000



struct menu_device {
	int		last_state_idx;
	int             needs_update;

	unsigned int	expected_us;
	u64		predicted_us;
	unsigned int	measured_us;
	unsigned int	exit_us;
	unsigned int	bucket;
	u64		correction_factor[BUCKETS];
};


#define LOAD_INT(x) ((x) >> FSHIFT)
#define LOAD_FRAC(x) LOAD_INT(((x) & (FIXED_1-1)) * 100)

static int get_loadavg(void)
{
	unsigned long this = this_cpu_load();


	return LOAD_INT(this) * 10 + LOAD_FRAC(this) / 10;
}

static inline int which_bucket(unsigned int duration)
{
	int bucket = 0;

	
	if (nr_iowait_cpu())
		bucket = BUCKETS/2;

	if (duration < 10)
		return bucket;
	if (duration < 100)
		return bucket + 1;
	if (duration < 1000)
		return bucket + 2;
	if (duration < 10000)
		return bucket + 3;
	if (duration < 100000)
		return bucket + 4;
	return bucket + 5;
}


static inline int performance_multiplier(void)
{
	int mult = 1;

	

	mult += 2 * get_loadavg();

	
	mult += 10 * nr_iowait_cpu();

	return mult;
}

static DEFINE_PER_CPU(struct menu_device, menu_devices);

static void menu_update(struct cpuidle_device *dev);


static u64 div_round64(u64 dividend, u32 divisor)
{
	return div_u64(dividend + (divisor / 2), divisor);
}


static int menu_select(struct cpuidle_device *dev)
{
	struct menu_device *data = &__get_cpu_var(menu_devices);
	int latency_req = pm_qos_requirement(PM_QOS_CPU_DMA_LATENCY);
	int i;
	int multiplier;

	data->last_state_idx = 0;
	data->exit_us = 0;

	if (data->needs_update) {
		menu_update(dev);
		data->needs_update = 0;
	}

	
	if (unlikely(latency_req == 0))
		return 0;

	
	data->expected_us =
	    DIV_ROUND_UP((u32)ktime_to_ns(tick_nohz_get_sleep_length()), 1000);


	data->bucket = which_bucket(data->expected_us);

	multiplier = performance_multiplier();

	
	if (data->correction_factor[data->bucket] == 0)
		data->correction_factor[data->bucket] = RESOLUTION * DECAY;

	
	data->predicted_us = div_round64(data->expected_us * data->correction_factor[data->bucket],
					 RESOLUTION * DECAY);

	
	if (data->expected_us > 5)
		data->last_state_idx = CPUIDLE_DRIVER_STATE_START;


	
	for (i = CPUIDLE_DRIVER_STATE_START; i < dev->state_count; i++) {
		struct cpuidle_state *s = &dev->states[i];

		if (s->target_residency > data->predicted_us)
			break;
		if (s->exit_latency > latency_req)
			break;
		if (s->exit_latency * multiplier > data->predicted_us)
			break;
		data->exit_us = s->exit_latency;
		data->last_state_idx = i;
	}

	return data->last_state_idx;
}


static void menu_reflect(struct cpuidle_device *dev)
{
	struct menu_device *data = &__get_cpu_var(menu_devices);
	data->needs_update = 1;
}


static void menu_update(struct cpuidle_device *dev)
{
	struct menu_device *data = &__get_cpu_var(menu_devices);
	int last_idx = data->last_state_idx;
	unsigned int last_idle_us = cpuidle_get_last_residency(dev);
	struct cpuidle_state *target = &dev->states[last_idx];
	unsigned int measured_us;
	u64 new_factor;

	
	if (unlikely(!(target->flags & CPUIDLE_FLAG_TIME_VALID)))
		last_idle_us = data->expected_us;


	measured_us = last_idle_us;

	
	if (measured_us > data->exit_us)
		measured_us -= data->exit_us;


	

	new_factor = data->correction_factor[data->bucket]
			* (DECAY - 1) / DECAY;

	if (data->expected_us > 0 && data->measured_us < MAX_INTERESTING)
		new_factor += RESOLUTION * measured_us / data->expected_us;
	else
		
		new_factor += RESOLUTION;

	
	if (new_factor == 0)
		new_factor = 1;

	data->correction_factor[data->bucket] = new_factor;
}


static int menu_enable_device(struct cpuidle_device *dev)
{
	struct menu_device *data = &per_cpu(menu_devices, dev->cpu);

	memset(data, 0, sizeof(struct menu_device));

	return 0;
}

static struct cpuidle_governor menu_governor = {
	.name =		"menu",
	.rating =	20,
	.enable =	menu_enable_device,
	.select =	menu_select,
	.reflect =	menu_reflect,
	.owner =	THIS_MODULE,
};


static int __init init_menu(void)
{
	return cpuidle_register_governor(&menu_governor);
}


static void __exit exit_menu(void)
{
	cpuidle_unregister_governor(&menu_governor);
}

MODULE_LICENSE("GPL");
module_init(init_menu);
module_exit(exit_menu);
