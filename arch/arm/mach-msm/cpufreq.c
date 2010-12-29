

#include <linux/earlysuspend.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include "acpuclock.h"

#define dprintk(msg...) \
		cpufreq_debug_printk(CPUFREQ_DEBUG_DRIVER, "cpufreq-msm", msg)

static int msm_cpufreq_target(struct cpufreq_policy *policy,
				unsigned int target_freq,
				unsigned int relation)
{
	int index;
	int ret = 0;
	struct cpufreq_freqs freqs;
	struct cpufreq_frequency_table *table;

	table = cpufreq_frequency_get_table(policy->cpu);
	if (cpufreq_frequency_table_target(policy, table, target_freq, relation,
			&index)) {
		pr_err("cpufreq: invalid target_freq: %d\n", target_freq);
		return -EINVAL;
	}

#ifdef CONFIG_CPU_FREQ_DEBUG
	dprintk("target %d r %d (%d-%d) selected %d\n", target_freq,
		relation, policy->min, policy->max, table[index].frequency);
#endif
	freqs.old = policy->cur;
	freqs.new = table[index].frequency;
	freqs.cpu = policy->cpu;
	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
	ret = acpuclk_set_rate(policy->cpu, table[index].frequency,
				SETRATE_CPUFREQ);
	if (!ret)
		cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
	return ret;
}

static int msm_cpufreq_verify(struct cpufreq_policy *policy)
{
	cpufreq_verify_within_limits(policy, policy->cpuinfo.min_freq,
			policy->cpuinfo.max_freq);
	return 0;
}

static int __cpuinit msm_cpufreq_init(struct cpufreq_policy *policy)
{
	struct cpufreq_frequency_table *table;

	table = cpufreq_frequency_get_table(policy->cpu);
	policy->cur = acpuclk_get_rate(policy->cpu);
	if (cpufreq_frequency_table_cpuinfo(policy, table)) {
#ifdef CONFIG_MSM_CPU_FREQ_SET_MIN_MAX
		policy->cpuinfo.min_freq = CONFIG_MSM_CPU_FREQ_MIN;
		policy->cpuinfo.max_freq = CONFIG_MSM_CPU_FREQ_MAX;
#endif
	}
#ifdef CONFIG_MSM_CPU_FREQ_SET_MIN_MAX
	policy->min = CONFIG_MSM_CPU_FREQ_MIN;
	policy->max = CONFIG_MSM_CPU_FREQ_MAX;
#endif

	policy->cpuinfo.transition_latency =
		acpuclk_get_switch_time() * NSEC_PER_USEC;
	return 0;
}

static struct cpufreq_driver msm_cpufreq_driver = {
	
	.flags		= CPUFREQ_STICKY | CPUFREQ_CONST_LOOPS,
	.init		= msm_cpufreq_init,
	.verify		= msm_cpufreq_verify,
	.target		= msm_cpufreq_target,
	.name		= "msm",
};

static int __init msm_cpufreq_register(void)
{
	return cpufreq_register_driver(&msm_cpufreq_driver);
}

late_initcall(msm_cpufreq_register);

