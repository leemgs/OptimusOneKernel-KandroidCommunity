
#include <linux/cpu.h>
#include <linux/err.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/stop_machine.h>
#include <linux/syscalls.h>
#include <linux/interrupt.h>

#include <asm/atomic.h>
#include <asm/uaccess.h>


enum stopmachine_state {
	
	STOPMACHINE_NONE,
	
	STOPMACHINE_PREPARE,
	
	STOPMACHINE_DISABLE_IRQ,
	
	STOPMACHINE_RUN,
	
	STOPMACHINE_EXIT,
};
static enum stopmachine_state state;

struct stop_machine_data {
	int (*fn)(void *);
	void *data;
	int fnret;
};


static unsigned int num_threads;
static atomic_t thread_ack;
static DEFINE_MUTEX(lock);

static DEFINE_MUTEX(setup_lock);

static int refcount;
static struct workqueue_struct *stop_machine_wq;
static struct stop_machine_data active, idle;
static const struct cpumask *active_cpus;
static void *stop_machine_work;

static void set_state(enum stopmachine_state newstate)
{
	
	atomic_set(&thread_ack, num_threads);
	smp_wmb();
	state = newstate;
}


static void ack_state(void)
{
	if (atomic_dec_and_test(&thread_ack))
		set_state(state + 1);
}


static void stop_cpu(struct work_struct *unused)
{
	enum stopmachine_state curstate = STOPMACHINE_NONE;
	struct stop_machine_data *smdata = &idle;
	int cpu = smp_processor_id();
	int err;

	if (!active_cpus) {
		if (cpu == cpumask_first(cpu_online_mask))
			smdata = &active;
	} else {
		if (cpumask_test_cpu(cpu, active_cpus))
			smdata = &active;
	}
	
	do {
		
		cpu_relax();
		if (state != curstate) {
			curstate = state;
			switch (curstate) {
			case STOPMACHINE_DISABLE_IRQ:
				local_irq_disable();
				hard_irq_disable();
				break;
			case STOPMACHINE_RUN:
				
				err = smdata->fn(smdata->data);
				if (err)
					smdata->fnret = err;
				break;
			default:
				break;
			}
			ack_state();
		}
	} while (curstate != STOPMACHINE_EXIT);

	local_irq_enable();
}


static int chill(void *unused)
{
	return 0;
}

int stop_machine_create(void)
{
	mutex_lock(&setup_lock);
	if (refcount)
		goto done;
	stop_machine_wq = create_rt_workqueue("kstop");
	if (!stop_machine_wq)
		goto err_out;
	stop_machine_work = alloc_percpu(struct work_struct);
	if (!stop_machine_work)
		goto err_out;
done:
	refcount++;
	mutex_unlock(&setup_lock);
	return 0;

err_out:
	if (stop_machine_wq)
		destroy_workqueue(stop_machine_wq);
	mutex_unlock(&setup_lock);
	return -ENOMEM;
}
EXPORT_SYMBOL_GPL(stop_machine_create);

void stop_machine_destroy(void)
{
	mutex_lock(&setup_lock);
	refcount--;
	if (refcount)
		goto done;
	destroy_workqueue(stop_machine_wq);
	free_percpu(stop_machine_work);
done:
	mutex_unlock(&setup_lock);
}
EXPORT_SYMBOL_GPL(stop_machine_destroy);

int __stop_machine(int (*fn)(void *), void *data, const struct cpumask *cpus)
{
	struct work_struct *sm_work;
	int i, ret;

	
	mutex_lock(&lock);
	num_threads = num_online_cpus();
	active_cpus = cpus;
	active.fn = fn;
	active.data = data;
	active.fnret = 0;
	idle.fn = chill;
	idle.data = NULL;

	set_state(STOPMACHINE_PREPARE);

	
	get_cpu();
	for_each_online_cpu(i) {
		sm_work = per_cpu_ptr(stop_machine_work, i);
		INIT_WORK(sm_work, stop_cpu);
		queue_work_on(i, stop_machine_wq, sm_work);
	}
	
	put_cpu();
	flush_workqueue(stop_machine_wq);
	ret = active.fnret;
	mutex_unlock(&lock);
	return ret;
}

int stop_machine(int (*fn)(void *), void *data, const struct cpumask *cpus)
{
	int ret;

	ret = stop_machine_create();
	if (ret)
		return ret;
	
	get_online_cpus();
	ret = __stop_machine(fn, data, cpus);
	put_online_cpus();
	stop_machine_destroy();
	return ret;
}
EXPORT_SYMBOL_GPL(stop_machine);
