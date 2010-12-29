

#include <linux/sched.h>
#include <linux/cpuidle.h>
#include <linux/memory.h>
#include <linux/smp.h>
#include <linux/cpumask.h>

#include "idle.h"


#define MSM_STATE_C1 0
#define MSM_STATE_C2 1
#define MSM_STATE_C3 2
#define MSM_MAX_STATES 3

struct msm_processor_cx {
	u8 type;
	u32 sleep_latency;
	u32 wakeup_latency;
	u32 threshold;
	u32 flags;
	int (*enter)    (struct cpuidle_device *dev,
				 struct cpuidle_state *state);
};

static int msm_idle_c1_enter(struct cpuidle_device *dev,
				struct cpuidle_state *state);
static int msm_idle_c2_enter(struct cpuidle_device *dev,
				struct cpuidle_state *state);
static int msm_idle_c3_enter(struct cpuidle_device *dev,
				struct cpuidle_state *state);

struct msm_processor_cx msm_power_states[MSM_MAX_STATES] = {

	[MSM_STATE_C1] = {
				.type = MSM_STATE_C1,
				.sleep_latency = 2,
				.wakeup_latency = 2,
				.threshold = 2,
				.flags = CPUIDLE_FLAG_TIME_VALID,
				.enter = msm_idle_c1_enter,
			},

	[MSM_STATE_C2] = {
				.type = MSM_STATE_C2,
				.sleep_latency = 10,
				.wakeup_latency = 10,
				.threshold = 10,
				.flags = CPUIDLE_FLAG_TIME_VALID,
				.enter = msm_idle_c2_enter,
			},

	[MSM_STATE_C3] = {
				.type = MSM_STATE_C3,
				.sleep_latency = 20,
				.wakeup_latency = 20,
				.threshold = 20,
				.flags = CPUIDLE_FLAG_TIME_VALID,
				.enter = msm_idle_c3_enter,
			},
};


static int msm_enter_idle(struct cpuidle_device *dev,
			struct cpuidle_state *state)
{
	struct timespec ts_preidle, ts_postidle, ts_idle;


	
	getnstimeofday(&ts_preidle);

	local_irq_disable();
	local_fiq_disable();

	
	msm_arch_idle();

	getnstimeofday(&ts_postidle);
	ts_idle = timespec_sub(ts_postidle, ts_preidle);

	local_irq_enable();
	local_fiq_enable();

	return (u32)timespec_to_ns(&ts_idle)/1000;
}

static int msm_idle_c1_enter(struct cpuidle_device *dev,
				struct cpuidle_state *state)
{
	return msm_enter_idle(dev, state);
}


static int msm_idle_c2_enter(struct cpuidle_device *dev,
				struct cpuidle_state *state)
{
	return msm_enter_idle(dev, state);
}

static int msm_idle_c3_enter(struct cpuidle_device *dev,
				struct cpuidle_state *state)
{
	return msm_enter_idle(dev, state);
}

DEFINE_PER_CPU(struct cpuidle_device, msm_idle_dev);

struct cpuidle_driver msm_idle_driver = {
	.name =	"msm_idle",
	.owner = THIS_MODULE,
};


int __init msm_idle_init(void)
{

	int i, cpu;
	struct msm_processor_cx *cx;
	struct cpuidle_state *state;
	struct cpuidle_device *dev;

	cpuidle_register_driver(&msm_idle_driver);

	for_each_possible_cpu(cpu) {
		dev = &per_cpu(msm_idle_dev, cpu);
		dev->cpu = cpu;

		for (i = MSM_STATE_C1; i < MSM_MAX_STATES; i++) {
			cx = &msm_power_states[i];
			state = &dev->states[i];

			cpuidle_set_statedata(state, cx);
			state->exit_latency =
				cx->sleep_latency + cx->wakeup_latency;
			state->target_residency = cx->threshold;
			state->flags = cx->flags;
			state->enter = cx->enter;
			if (cx->type == MSM_STATE_C1)
				dev->safe_state = state;
			sprintf(state->name, "C%d", i+1);
		}

		dev->state_count = MSM_MAX_STATES;

		if (cpuidle_register_device(dev)) {
			printk(KERN_ERR "%s: CPUidle register device failed\n",
			       __func__);
		}

		printk(KERN_INFO "cpuidle device registered for cpu=%d \n",
			cpu);
	}

	return 0;
}

late_initcall(msm_idle_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("msm cpuidle driver");
MODULE_VERSION("1.0");
MODULE_AUTHOR("Qualcomm Innovation Center, Inc.");
