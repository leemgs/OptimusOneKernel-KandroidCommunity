

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sysdev.h>

#include <asm/uaccess.h>

#include <acpi/acpi_bus.h>
#include <acpi/processor.h>
#include <acpi/acpi_drivers.h>

#define PREFIX "ACPI: "

#define ACPI_PROCESSOR_CLASS            "processor"
#define _COMPONENT              ACPI_PROCESSOR_COMPONENT
ACPI_MODULE_NAME("processor_thermal");


static int acpi_processor_apply_limit(struct acpi_processor *pr)
{
	int result = 0;
	u16 px = 0;
	u16 tx = 0;


	if (!pr)
		return -EINVAL;

	if (!pr->flags.limit)
		return -ENODEV;

	if (pr->flags.throttling) {
		if (pr->limit.user.tx > tx)
			tx = pr->limit.user.tx;
		if (pr->limit.thermal.tx > tx)
			tx = pr->limit.thermal.tx;

		result = acpi_processor_set_throttling(pr, tx, false);
		if (result)
			goto end;
	}

	pr->limit.state.px = px;
	pr->limit.state.tx = tx;

	ACPI_DEBUG_PRINT((ACPI_DB_INFO,
			  "Processor [%d] limit set to (P%d:T%d)\n", pr->id,
			  pr->limit.state.px, pr->limit.state.tx));

      end:
	if (result)
		printk(KERN_ERR PREFIX "Unable to set limit\n");

	return result;
}

#ifdef CONFIG_CPU_FREQ



#define CPUFREQ_THERMAL_MIN_STEP 0
#define CPUFREQ_THERMAL_MAX_STEP 3

static DEFINE_PER_CPU(unsigned int, cpufreq_thermal_reduction_pctg);
static unsigned int acpi_thermal_cpufreq_is_init = 0;

static int cpu_has_cpufreq(unsigned int cpu)
{
	struct cpufreq_policy policy;
	if (!acpi_thermal_cpufreq_is_init || cpufreq_get_policy(&policy, cpu))
		return 0;
	return 1;
}

static int acpi_thermal_cpufreq_increase(unsigned int cpu)
{
	if (!cpu_has_cpufreq(cpu))
		return -ENODEV;

	if (per_cpu(cpufreq_thermal_reduction_pctg, cpu) <
		CPUFREQ_THERMAL_MAX_STEP) {
		per_cpu(cpufreq_thermal_reduction_pctg, cpu)++;
		cpufreq_update_policy(cpu);
		return 0;
	}

	return -ERANGE;
}

static int acpi_thermal_cpufreq_decrease(unsigned int cpu)
{
	if (!cpu_has_cpufreq(cpu))
		return -ENODEV;

	if (per_cpu(cpufreq_thermal_reduction_pctg, cpu) >
		(CPUFREQ_THERMAL_MIN_STEP + 1))
		per_cpu(cpufreq_thermal_reduction_pctg, cpu)--;
	else
		per_cpu(cpufreq_thermal_reduction_pctg, cpu) = 0;
	cpufreq_update_policy(cpu);
	
	return !per_cpu(cpufreq_thermal_reduction_pctg, cpu);
}

static int acpi_thermal_cpufreq_notifier(struct notifier_block *nb,
					 unsigned long event, void *data)
{
	struct cpufreq_policy *policy = data;
	unsigned long max_freq = 0;

	if (event != CPUFREQ_ADJUST)
		goto out;

	max_freq = (
	    policy->cpuinfo.max_freq *
	    (100 - per_cpu(cpufreq_thermal_reduction_pctg, policy->cpu) * 20)
	) / 100;

	cpufreq_verify_within_limits(policy, 0, max_freq);

      out:
	return 0;
}

static struct notifier_block acpi_thermal_cpufreq_notifier_block = {
	.notifier_call = acpi_thermal_cpufreq_notifier,
};

static int cpufreq_get_max_state(unsigned int cpu)
{
	if (!cpu_has_cpufreq(cpu))
		return 0;

	return CPUFREQ_THERMAL_MAX_STEP;
}

static int cpufreq_get_cur_state(unsigned int cpu)
{
	if (!cpu_has_cpufreq(cpu))
		return 0;

	return per_cpu(cpufreq_thermal_reduction_pctg, cpu);
}

static int cpufreq_set_cur_state(unsigned int cpu, int state)
{
	if (!cpu_has_cpufreq(cpu))
		return 0;

	per_cpu(cpufreq_thermal_reduction_pctg, cpu) = state;
	cpufreq_update_policy(cpu);
	return 0;
}

void acpi_thermal_cpufreq_init(void)
{
	int i;

	for (i = 0; i < nr_cpu_ids; i++)
		if (cpu_present(i))
			per_cpu(cpufreq_thermal_reduction_pctg, i) = 0;

	i = cpufreq_register_notifier(&acpi_thermal_cpufreq_notifier_block,
				      CPUFREQ_POLICY_NOTIFIER);
	if (!i)
		acpi_thermal_cpufreq_is_init = 1;
}

void acpi_thermal_cpufreq_exit(void)
{
	if (acpi_thermal_cpufreq_is_init)
		cpufreq_unregister_notifier
		    (&acpi_thermal_cpufreq_notifier_block,
		     CPUFREQ_POLICY_NOTIFIER);

	acpi_thermal_cpufreq_is_init = 0;
}

#else				
static int cpufreq_get_max_state(unsigned int cpu)
{
	return 0;
}

static int cpufreq_get_cur_state(unsigned int cpu)
{
	return 0;
}

static int cpufreq_set_cur_state(unsigned int cpu, int state)
{
	return 0;
}

static int acpi_thermal_cpufreq_increase(unsigned int cpu)
{
	return -ENODEV;
}
static int acpi_thermal_cpufreq_decrease(unsigned int cpu)
{
	return -ENODEV;
}

#endif

int acpi_processor_set_thermal_limit(acpi_handle handle, int type)
{
	int result = 0;
	struct acpi_processor *pr = NULL;
	struct acpi_device *device = NULL;
	int tx = 0, max_tx_px = 0;


	if ((type < ACPI_PROCESSOR_LIMIT_NONE)
	    || (type > ACPI_PROCESSOR_LIMIT_DECREMENT))
		return -EINVAL;

	result = acpi_bus_get_device(handle, &device);
	if (result)
		return result;

	pr = acpi_driver_data(device);
	if (!pr)
		return -ENODEV;

	
	if (pr->flags.throttling)
		pr->limit.thermal.tx = pr->throttling.state;

	

	tx = pr->limit.thermal.tx;

	switch (type) {

	case ACPI_PROCESSOR_LIMIT_NONE:
		do {
			result = acpi_thermal_cpufreq_decrease(pr->id);
		} while (!result);
		tx = 0;
		break;

	case ACPI_PROCESSOR_LIMIT_INCREMENT:
		

		result = acpi_thermal_cpufreq_increase(pr->id);
		if (!result)
			goto end;
		else if (result == -ERANGE)
			ACPI_DEBUG_PRINT((ACPI_DB_INFO,
					  "At maximum performance state\n"));

		if (pr->flags.throttling) {
			if (tx == (pr->throttling.state_count - 1))
				ACPI_DEBUG_PRINT((ACPI_DB_INFO,
						  "At maximum throttling state\n"));
			else
				tx++;
		}
		break;

	case ACPI_PROCESSOR_LIMIT_DECREMENT:
		

		if (pr->flags.throttling) {
			if (tx == 0) {
				max_tx_px = 1;
				ACPI_DEBUG_PRINT((ACPI_DB_INFO,
						  "At minimum throttling state\n"));
			} else {
				tx--;
				goto end;
			}
		}

		result = acpi_thermal_cpufreq_decrease(pr->id);
		if (result) {
			
			ACPI_DEBUG_PRINT((ACPI_DB_INFO,
					  "At minimum performance state\n"));
			max_tx_px = 1;
		} else
			max_tx_px = 0;

		break;
	}

      end:
	if (pr->flags.throttling) {
		pr->limit.thermal.px = 0;
		pr->limit.thermal.tx = tx;

		result = acpi_processor_apply_limit(pr);
		if (result)
			printk(KERN_ERR PREFIX "Unable to set thermal limit\n");

		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Thermal limit now (P%d:T%d)\n",
				  pr->limit.thermal.px, pr->limit.thermal.tx));
	} else
		result = 0;
	if (max_tx_px)
		return 1;
	else
		return result;
}

int acpi_processor_get_limit_info(struct acpi_processor *pr)
{

	if (!pr)
		return -EINVAL;

	if (pr->flags.throttling)
		pr->flags.limit = 1;

	return 0;
}


static int acpi_processor_max_state(struct acpi_processor *pr)
{
	int max_state = 0;

	
	max_state += cpufreq_get_max_state(pr->id);
	if (pr->flags.throttling)
		max_state += (pr->throttling.state_count -1);

	return max_state;
}
static int
processor_get_max_state(struct thermal_cooling_device *cdev,
			unsigned long *state)
{
	struct acpi_device *device = cdev->devdata;
	struct acpi_processor *pr = acpi_driver_data(device);

	if (!device || !pr)
		return -EINVAL;

	*state = acpi_processor_max_state(pr);
	return 0;
}

static int
processor_get_cur_state(struct thermal_cooling_device *cdev,
			unsigned long *cur_state)
{
	struct acpi_device *device = cdev->devdata;
	struct acpi_processor *pr = acpi_driver_data(device);

	if (!device || !pr)
		return -EINVAL;

	*cur_state = cpufreq_get_cur_state(pr->id);
	if (pr->flags.throttling)
		*cur_state += pr->throttling.state;
	return 0;
}

static int
processor_set_cur_state(struct thermal_cooling_device *cdev,
			unsigned long state)
{
	struct acpi_device *device = cdev->devdata;
	struct acpi_processor *pr = acpi_driver_data(device);
	int result = 0;
	int max_pstate;

	if (!device || !pr)
		return -EINVAL;

	max_pstate = cpufreq_get_max_state(pr->id);

	if (state > acpi_processor_max_state(pr))
		return -EINVAL;

	if (state <= max_pstate) {
		if (pr->flags.throttling && pr->throttling.state)
			result = acpi_processor_set_throttling(pr, 0, false);
		cpufreq_set_cur_state(pr->id, state);
	} else {
		cpufreq_set_cur_state(pr->id, max_pstate);
		result = acpi_processor_set_throttling(pr,
				state - max_pstate, false);
	}
	return result;
}

struct thermal_cooling_device_ops processor_cooling_ops = {
	.get_max_state = processor_get_max_state,
	.get_cur_state = processor_get_cur_state,
	.set_cur_state = processor_set_cur_state,
};


#ifdef CONFIG_ACPI_PROCFS
static int acpi_processor_limit_seq_show(struct seq_file *seq, void *offset)
{
	struct acpi_processor *pr = (struct acpi_processor *)seq->private;


	if (!pr)
		goto end;

	if (!pr->flags.limit) {
		seq_puts(seq, "<not supported>\n");
		goto end;
	}

	seq_printf(seq, "active limit:            P%d:T%d\n"
		   "user limit:              P%d:T%d\n"
		   "thermal limit:           P%d:T%d\n",
		   pr->limit.state.px, pr->limit.state.tx,
		   pr->limit.user.px, pr->limit.user.tx,
		   pr->limit.thermal.px, pr->limit.thermal.tx);

      end:
	return 0;
}

static int acpi_processor_limit_open_fs(struct inode *inode, struct file *file)
{
	return single_open(file, acpi_processor_limit_seq_show,
			   PDE(inode)->data);
}

static ssize_t acpi_processor_write_limit(struct file * file,
					  const char __user * buffer,
					  size_t count, loff_t * data)
{
	int result = 0;
	struct seq_file *m = file->private_data;
	struct acpi_processor *pr = m->private;
	char limit_string[25] = { '\0' };
	int px = 0;
	int tx = 0;


	if (!pr || (count > sizeof(limit_string) - 1)) {
		return -EINVAL;
	}

	if (copy_from_user(limit_string, buffer, count)) {
		return -EFAULT;
	}

	limit_string[count] = '\0';

	if (sscanf(limit_string, "%d:%d", &px, &tx) != 2) {
		printk(KERN_ERR PREFIX "Invalid data format\n");
		return -EINVAL;
	}

	if (pr->flags.throttling) {
		if ((tx < 0) || (tx > (pr->throttling.state_count - 1))) {
			printk(KERN_ERR PREFIX "Invalid tx\n");
			return -EINVAL;
		}
		pr->limit.user.tx = tx;
	}

	result = acpi_processor_apply_limit(pr);

	return count;
}

const struct file_operations acpi_processor_limit_fops = {
	.owner = THIS_MODULE,
	.open = acpi_processor_limit_open_fs,
	.read = seq_read,
	.write = acpi_processor_write_limit,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif
