

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/trace_clock.h>

#include <asm/ds.h>

#include "ds_selftest.h"


struct ds_configuration {
	
	const char		*name;

	
	unsigned char		sizeof_ptr_field;

	
	unsigned char		sizeof_rec[2];

	
	unsigned char		nr_counter_reset;

	
	unsigned long		ctl[dsf_ctl_max];
};
static struct ds_configuration ds_cfg __read_mostly;



#define MAX_SIZEOF_DS		0x80


#define MAX_SIZEOF_BTS		(3 * 8)


#define DS_ALIGNMENT		(1 << 3)


#define NUM_DS_PTR_FIELDS	8


#define PEBS_RESET_FIELD_SIZE	8


#define BTS_CONTROL				  \
	( ds_cfg.ctl[dsf_bts]			| \
	  ds_cfg.ctl[dsf_bts_kernel]		| \
	  ds_cfg.ctl[dsf_bts_user]		| \
	  ds_cfg.ctl[dsf_bts_overflow] )


struct ds_tracer {
	
	struct ds_context	*context;
	
	void			*buffer;
	size_t			size;
};

struct bts_tracer {
	
	struct ds_tracer	ds;

	
	struct bts_trace	trace;

	
	bts_ovfl_callback_t	ovfl;

	
	unsigned int		flags;
};

struct pebs_tracer {
	
	struct ds_tracer	ds;

	
	struct pebs_trace	trace;

	
	pebs_ovfl_callback_t	ovfl;
};



enum ds_field {
	ds_buffer_base = 0,
	ds_index,
	ds_absolute_maximum,
	ds_interrupt_threshold,
};

enum ds_qualifier {
	ds_bts = 0,
	ds_pebs
};

static inline unsigned long
ds_get(const unsigned char *base, enum ds_qualifier qual, enum ds_field field)
{
	base += (ds_cfg.sizeof_ptr_field * (field + (4 * qual)));
	return *(unsigned long *)base;
}

static inline void
ds_set(unsigned char *base, enum ds_qualifier qual, enum ds_field field,
       unsigned long value)
{
	base += (ds_cfg.sizeof_ptr_field * (field + (4 * qual)));
	(*(unsigned long *)base) = value;
}



static DEFINE_SPINLOCK(ds_lock);


static atomic_t tracers = ATOMIC_INIT(0);

static inline int get_tracer(struct task_struct *task)
{
	int error;

	spin_lock_irq(&ds_lock);

	if (task) {
		error = -EPERM;
		if (atomic_read(&tracers) < 0)
			goto out;
		atomic_inc(&tracers);
	} else {
		error = -EPERM;
		if (atomic_read(&tracers) > 0)
			goto out;
		atomic_dec(&tracers);
	}

	error = 0;
out:
	spin_unlock_irq(&ds_lock);
	return error;
}

static inline void put_tracer(struct task_struct *task)
{
	if (task)
		atomic_dec(&tracers);
	else
		atomic_inc(&tracers);
}


struct ds_context {
	
	unsigned char		ds[MAX_SIZEOF_DS];

	
	struct bts_tracer	*bts_master;
	struct pebs_tracer	*pebs_master;

	
	unsigned long		count;

	
	struct ds_context	**this;

	
	struct task_struct	*task;

	
	int			cpu;
};

static DEFINE_PER_CPU(struct ds_context *, cpu_context);


static struct ds_context *ds_get_context(struct task_struct *task, int cpu)
{
	struct ds_context **p_context =
		(task ? &task->thread.ds_ctx : &per_cpu(cpu_context, cpu));
	struct ds_context *context = NULL;
	struct ds_context *new_context = NULL;

	
	new_context = kzalloc(sizeof(*new_context), GFP_KERNEL);
	if (!new_context)
		return NULL;

	spin_lock_irq(&ds_lock);

	context = *p_context;
	if (likely(!context)) {
		context = new_context;

		context->this = p_context;
		context->task = task;
		context->cpu = cpu;
		context->count = 0;

		*p_context = context;
	}

	context->count++;

	spin_unlock_irq(&ds_lock);

	if (context != new_context)
		kfree(new_context);

	return context;
}

static void ds_put_context(struct ds_context *context)
{
	struct task_struct *task;
	unsigned long irq;

	if (!context)
		return;

	spin_lock_irqsave(&ds_lock, irq);

	if (--context->count) {
		spin_unlock_irqrestore(&ds_lock, irq);
		return;
	}

	*(context->this) = NULL;

	task = context->task;

	if (task)
		clear_tsk_thread_flag(task, TIF_DS_AREA_MSR);

	

	spin_unlock_irqrestore(&ds_lock, irq);

	
	if (task && (task != current))
		wait_task_context_switch(task);

	kfree(context);
}

static void ds_install_ds_area(struct ds_context *context)
{
	unsigned long ds;

	ds = (unsigned long)context->ds;

	
	if (context->task) {
		get_cpu();
		if (context->task == current)
			wrmsrl(MSR_IA32_DS_AREA, ds);
		set_tsk_thread_flag(context->task, TIF_DS_AREA_MSR);
		put_cpu();
	} else
		wrmsr_on_cpu(context->cpu, MSR_IA32_DS_AREA,
			     (u32)((u64)ds), (u32)((u64)ds >> 32));
}


static void ds_overflow(struct ds_context *context, enum ds_qualifier qual)
{
	switch (qual) {
	case ds_bts:
		if (context->bts_master &&
		    context->bts_master->ovfl)
			context->bts_master->ovfl(context->bts_master);
		break;
	case ds_pebs:
		if (context->pebs_master &&
		    context->pebs_master->ovfl)
			context->pebs_master->ovfl(context->pebs_master);
		break;
	}
}



static int ds_write(struct ds_context *context, enum ds_qualifier qual,
		    const void *record, size_t size)
{
	int bytes_written = 0;

	if (!record)
		return -EINVAL;

	while (size) {
		unsigned long base, index, end, write_end, int_th;
		unsigned long write_size, adj_write_size;

		
		base   = ds_get(context->ds, qual, ds_buffer_base);
		index  = ds_get(context->ds, qual, ds_index);
		end    = ds_get(context->ds, qual, ds_absolute_maximum);
		int_th = ds_get(context->ds, qual, ds_interrupt_threshold);

		write_end = min(end, int_th);

		
		if (write_end <= index)
			write_end = end;

		if (write_end <= index)
			break;

		write_size = min((unsigned long) size, write_end - index);
		memcpy((void *)index, record, write_size);

		record = (const char *)record + write_size;
		size -= write_size;
		bytes_written += write_size;

		adj_write_size = write_size / ds_cfg.sizeof_rec[qual];
		adj_write_size *= ds_cfg.sizeof_rec[qual];

		
		memset((char *)index + write_size, 0,
		       adj_write_size - write_size);
		index += adj_write_size;

		if (index >= end)
			index = base;
		ds_set(context->ds, qual, ds_index, index);

		if (index >= int_th)
			ds_overflow(context, qual);
	}

	return bytes_written;
}




enum bts_field {
	bts_from,
	bts_to,
	bts_flags,

	bts_qual		= bts_from,
	bts_clock		= bts_to,
	bts_pid			= bts_flags,

	bts_qual_mask		= (bts_qual_max - 1),
	bts_escape		= ((unsigned long)-1 & ~bts_qual_mask)
};

static inline unsigned long bts_get(const char *base, unsigned long field)
{
	base += (ds_cfg.sizeof_ptr_field * field);
	return *(unsigned long *)base;
}

static inline void bts_set(char *base, unsigned long field, unsigned long val)
{
	base += (ds_cfg.sizeof_ptr_field * field);
	(*(unsigned long *)base) = val;
}



static int
bts_read(struct bts_tracer *tracer, const void *at, struct bts_struct *out)
{
	if (!tracer)
		return -EINVAL;

	if (at < tracer->trace.ds.begin)
		return -EINVAL;

	if (tracer->trace.ds.end < (at + tracer->trace.ds.size))
		return -EINVAL;

	memset(out, 0, sizeof(*out));
	if ((bts_get(at, bts_qual) & ~bts_qual_mask) == bts_escape) {
		out->qualifier = (bts_get(at, bts_qual) & bts_qual_mask);
		out->variant.event.clock = bts_get(at, bts_clock);
		out->variant.event.pid = bts_get(at, bts_pid);
	} else {
		out->qualifier = bts_branch;
		out->variant.lbr.from = bts_get(at, bts_from);
		out->variant.lbr.to   = bts_get(at, bts_to);

		if (!out->variant.lbr.from && !out->variant.lbr.to)
			out->qualifier = bts_invalid;
	}

	return ds_cfg.sizeof_rec[ds_bts];
}

static int bts_write(struct bts_tracer *tracer, const struct bts_struct *in)
{
	unsigned char raw[MAX_SIZEOF_BTS];

	if (!tracer)
		return -EINVAL;

	if (MAX_SIZEOF_BTS < ds_cfg.sizeof_rec[ds_bts])
		return -EOVERFLOW;

	switch (in->qualifier) {
	case bts_invalid:
		bts_set(raw, bts_from, 0);
		bts_set(raw, bts_to, 0);
		bts_set(raw, bts_flags, 0);
		break;
	case bts_branch:
		bts_set(raw, bts_from, in->variant.lbr.from);
		bts_set(raw, bts_to,   in->variant.lbr.to);
		bts_set(raw, bts_flags, 0);
		break;
	case bts_task_arrives:
	case bts_task_departs:
		bts_set(raw, bts_qual, (bts_escape | in->qualifier));
		bts_set(raw, bts_clock, in->variant.event.clock);
		bts_set(raw, bts_pid, in->variant.event.pid);
		break;
	default:
		return -EINVAL;
	}

	return ds_write(tracer->ds.context, ds_bts, raw,
			ds_cfg.sizeof_rec[ds_bts]);
}


static void ds_write_config(struct ds_context *context,
			    struct ds_trace *cfg, enum ds_qualifier qual)
{
	unsigned char *ds = context->ds;

	ds_set(ds, qual, ds_buffer_base, (unsigned long)cfg->begin);
	ds_set(ds, qual, ds_index, (unsigned long)cfg->top);
	ds_set(ds, qual, ds_absolute_maximum, (unsigned long)cfg->end);
	ds_set(ds, qual, ds_interrupt_threshold, (unsigned long)cfg->ith);
}

static void ds_read_config(struct ds_context *context,
			   struct ds_trace *cfg, enum ds_qualifier qual)
{
	unsigned char *ds = context->ds;

	cfg->begin = (void *)ds_get(ds, qual, ds_buffer_base);
	cfg->top = (void *)ds_get(ds, qual, ds_index);
	cfg->end = (void *)ds_get(ds, qual, ds_absolute_maximum);
	cfg->ith = (void *)ds_get(ds, qual, ds_interrupt_threshold);
}

static void ds_init_ds_trace(struct ds_trace *trace, enum ds_qualifier qual,
			     void *base, size_t size, size_t ith,
			     unsigned int flags) {
	unsigned long buffer, adj;

	
	buffer = (unsigned long)base;

	adj = ALIGN(buffer, DS_ALIGNMENT) - buffer;
	buffer += adj;
	size   -= adj;

	trace->n = size / ds_cfg.sizeof_rec[qual];
	trace->size = ds_cfg.sizeof_rec[qual];

	size = (trace->n * trace->size);

	trace->begin = (void *)buffer;
	trace->top = trace->begin;
	trace->end = (void *)(buffer + size);
	
	ith *= ds_cfg.sizeof_rec[qual];
	trace->ith = (void *)(buffer + size - ith);

	trace->flags = flags;
}


static int ds_request(struct ds_tracer *tracer, struct ds_trace *trace,
		      enum ds_qualifier qual, struct task_struct *task,
		      int cpu, void *base, size_t size, size_t th)
{
	struct ds_context *context;
	int error;
	size_t req_size;

	error = -EOPNOTSUPP;
	if (!ds_cfg.sizeof_rec[qual])
		goto out;

	error = -EINVAL;
	if (!base)
		goto out;

	req_size = ds_cfg.sizeof_rec[qual];
	
	if (!IS_ALIGNED((unsigned long)base, DS_ALIGNMENT))
		req_size += DS_ALIGNMENT;

	error = -EINVAL;
	if (size < req_size)
		goto out;

	if (th != (size_t)-1) {
		th *= ds_cfg.sizeof_rec[qual];

		error = -EINVAL;
		if (size <= th)
			goto out;
	}

	tracer->buffer = base;
	tracer->size = size;

	error = -ENOMEM;
	context = ds_get_context(task, cpu);
	if (!context)
		goto out;
	tracer->context = context;

	

	error = 0;
 out:
	return error;
}

static struct bts_tracer *ds_request_bts(struct task_struct *task, int cpu,
					 void *base, size_t size,
					 bts_ovfl_callback_t ovfl, size_t th,
					 unsigned int flags)
{
	struct bts_tracer *tracer;
	int error;

	
	error = -EOPNOTSUPP;
	if (ovfl)
		goto out;

	error = get_tracer(task);
	if (error < 0)
		goto out;

	error = -ENOMEM;
	tracer = kzalloc(sizeof(*tracer), GFP_KERNEL);
	if (!tracer)
		goto out_put_tracer;
	tracer->ovfl = ovfl;

	
	error = ds_request(&tracer->ds, &tracer->trace.ds,
			   ds_bts, task, cpu, base, size, th);
	if (error < 0)
		goto out_tracer;

	
	spin_lock_irq(&ds_lock);

	error = -EPERM;
	if (tracer->ds.context->bts_master)
		goto out_unlock;
	tracer->ds.context->bts_master = tracer;

	spin_unlock_irq(&ds_lock);

	
	ds_init_ds_trace(&tracer->trace.ds, ds_bts, base, size, th, flags);
	ds_write_config(tracer->ds.context, &tracer->trace.ds, ds_bts);
	ds_install_ds_area(tracer->ds.context);

	tracer->trace.read  = bts_read;
	tracer->trace.write = bts_write;

	
	ds_resume_bts(tracer);

	return tracer;

 out_unlock:
	spin_unlock_irq(&ds_lock);
	ds_put_context(tracer->ds.context);
 out_tracer:
	kfree(tracer);
 out_put_tracer:
	put_tracer(task);
 out:
	return ERR_PTR(error);
}

struct bts_tracer *ds_request_bts_task(struct task_struct *task,
				       void *base, size_t size,
				       bts_ovfl_callback_t ovfl,
				       size_t th, unsigned int flags)
{
	return ds_request_bts(task, 0, base, size, ovfl, th, flags);
}

struct bts_tracer *ds_request_bts_cpu(int cpu, void *base, size_t size,
				      bts_ovfl_callback_t ovfl,
				      size_t th, unsigned int flags)
{
	return ds_request_bts(NULL, cpu, base, size, ovfl, th, flags);
}

static struct pebs_tracer *ds_request_pebs(struct task_struct *task, int cpu,
					   void *base, size_t size,
					   pebs_ovfl_callback_t ovfl, size_t th,
					   unsigned int flags)
{
	struct pebs_tracer *tracer;
	int error;

	
	error = -EOPNOTSUPP;
	if (ovfl)
		goto out;

	error = get_tracer(task);
	if (error < 0)
		goto out;

	error = -ENOMEM;
	tracer = kzalloc(sizeof(*tracer), GFP_KERNEL);
	if (!tracer)
		goto out_put_tracer;
	tracer->ovfl = ovfl;

	
	error = ds_request(&tracer->ds, &tracer->trace.ds,
			   ds_pebs, task, cpu, base, size, th);
	if (error < 0)
		goto out_tracer;

	
	spin_lock_irq(&ds_lock);

	error = -EPERM;
	if (tracer->ds.context->pebs_master)
		goto out_unlock;
	tracer->ds.context->pebs_master = tracer;

	spin_unlock_irq(&ds_lock);

	
	ds_init_ds_trace(&tracer->trace.ds, ds_pebs, base, size, th, flags);
	ds_write_config(tracer->ds.context, &tracer->trace.ds, ds_pebs);
	ds_install_ds_area(tracer->ds.context);

	
	ds_resume_pebs(tracer);

	return tracer;

 out_unlock:
	spin_unlock_irq(&ds_lock);
	ds_put_context(tracer->ds.context);
 out_tracer:
	kfree(tracer);
 out_put_tracer:
	put_tracer(task);
 out:
	return ERR_PTR(error);
}

struct pebs_tracer *ds_request_pebs_task(struct task_struct *task,
					 void *base, size_t size,
					 pebs_ovfl_callback_t ovfl,
					 size_t th, unsigned int flags)
{
	return ds_request_pebs(task, 0, base, size, ovfl, th, flags);
}

struct pebs_tracer *ds_request_pebs_cpu(int cpu, void *base, size_t size,
					pebs_ovfl_callback_t ovfl,
					size_t th, unsigned int flags)
{
	return ds_request_pebs(NULL, cpu, base, size, ovfl, th, flags);
}

static void ds_free_bts(struct bts_tracer *tracer)
{
	struct task_struct *task;

	task = tracer->ds.context->task;

	WARN_ON_ONCE(tracer->ds.context->bts_master != tracer);
	tracer->ds.context->bts_master = NULL;

	
	if (task && (task != current))
		wait_task_context_switch(task);

	ds_put_context(tracer->ds.context);
	put_tracer(task);

	kfree(tracer);
}

void ds_release_bts(struct bts_tracer *tracer)
{
	might_sleep();

	if (!tracer)
		return;

	ds_suspend_bts(tracer);
	ds_free_bts(tracer);
}

int ds_release_bts_noirq(struct bts_tracer *tracer)
{
	struct task_struct *task;
	unsigned long irq;
	int error;

	if (!tracer)
		return 0;

	task = tracer->ds.context->task;

	local_irq_save(irq);

	error = -EPERM;
	if (!task &&
	    (tracer->ds.context->cpu != smp_processor_id()))
		goto out;

	error = -EPERM;
	if (task && (task != current))
		goto out;

	ds_suspend_bts_noirq(tracer);
	ds_free_bts(tracer);

	error = 0;
 out:
	local_irq_restore(irq);
	return error;
}

static void update_task_debugctlmsr(struct task_struct *task,
				    unsigned long debugctlmsr)
{
	task->thread.debugctlmsr = debugctlmsr;

	get_cpu();
	if (task == current)
		update_debugctlmsr(debugctlmsr);
	put_cpu();
}

void ds_suspend_bts(struct bts_tracer *tracer)
{
	struct task_struct *task;
	unsigned long debugctlmsr;
	int cpu;

	if (!tracer)
		return;

	tracer->flags = 0;

	task = tracer->ds.context->task;
	cpu  = tracer->ds.context->cpu;

	WARN_ON(!task && irqs_disabled());

	debugctlmsr = (task ?
		       task->thread.debugctlmsr :
		       get_debugctlmsr_on_cpu(cpu));
	debugctlmsr &= ~BTS_CONTROL;

	if (task)
		update_task_debugctlmsr(task, debugctlmsr);
	else
		update_debugctlmsr_on_cpu(cpu, debugctlmsr);
}

int ds_suspend_bts_noirq(struct bts_tracer *tracer)
{
	struct task_struct *task;
	unsigned long debugctlmsr, irq;
	int cpu, error = 0;

	if (!tracer)
		return 0;

	tracer->flags = 0;

	task = tracer->ds.context->task;
	cpu  = tracer->ds.context->cpu;

	local_irq_save(irq);

	error = -EPERM;
	if (!task && (cpu != smp_processor_id()))
		goto out;

	debugctlmsr = (task ?
		       task->thread.debugctlmsr :
		       get_debugctlmsr());
	debugctlmsr &= ~BTS_CONTROL;

	if (task)
		update_task_debugctlmsr(task, debugctlmsr);
	else
		update_debugctlmsr(debugctlmsr);

	error = 0;
 out:
	local_irq_restore(irq);
	return error;
}

static unsigned long ds_bts_control(struct bts_tracer *tracer)
{
	unsigned long control;

	control = ds_cfg.ctl[dsf_bts];
	if (!(tracer->trace.ds.flags & BTS_KERNEL))
		control |= ds_cfg.ctl[dsf_bts_kernel];
	if (!(tracer->trace.ds.flags & BTS_USER))
		control |= ds_cfg.ctl[dsf_bts_user];

	return control;
}

void ds_resume_bts(struct bts_tracer *tracer)
{
	struct task_struct *task;
	unsigned long debugctlmsr;
	int cpu;

	if (!tracer)
		return;

	tracer->flags = tracer->trace.ds.flags;

	task = tracer->ds.context->task;
	cpu  = tracer->ds.context->cpu;

	WARN_ON(!task && irqs_disabled());

	debugctlmsr = (task ?
		       task->thread.debugctlmsr :
		       get_debugctlmsr_on_cpu(cpu));
	debugctlmsr |= ds_bts_control(tracer);

	if (task)
		update_task_debugctlmsr(task, debugctlmsr);
	else
		update_debugctlmsr_on_cpu(cpu, debugctlmsr);
}

int ds_resume_bts_noirq(struct bts_tracer *tracer)
{
	struct task_struct *task;
	unsigned long debugctlmsr, irq;
	int cpu, error = 0;

	if (!tracer)
		return 0;

	tracer->flags = tracer->trace.ds.flags;

	task = tracer->ds.context->task;
	cpu  = tracer->ds.context->cpu;

	local_irq_save(irq);

	error = -EPERM;
	if (!task && (cpu != smp_processor_id()))
		goto out;

	debugctlmsr = (task ?
		       task->thread.debugctlmsr :
		       get_debugctlmsr());
	debugctlmsr |= ds_bts_control(tracer);

	if (task)
		update_task_debugctlmsr(task, debugctlmsr);
	else
		update_debugctlmsr(debugctlmsr);

	error = 0;
 out:
	local_irq_restore(irq);
	return error;
}

static void ds_free_pebs(struct pebs_tracer *tracer)
{
	struct task_struct *task;

	task = tracer->ds.context->task;

	WARN_ON_ONCE(tracer->ds.context->pebs_master != tracer);
	tracer->ds.context->pebs_master = NULL;

	ds_put_context(tracer->ds.context);
	put_tracer(task);

	kfree(tracer);
}

void ds_release_pebs(struct pebs_tracer *tracer)
{
	might_sleep();

	if (!tracer)
		return;

	ds_suspend_pebs(tracer);
	ds_free_pebs(tracer);
}

int ds_release_pebs_noirq(struct pebs_tracer *tracer)
{
	struct task_struct *task;
	unsigned long irq;
	int error;

	if (!tracer)
		return 0;

	task = tracer->ds.context->task;

	local_irq_save(irq);

	error = -EPERM;
	if (!task &&
	    (tracer->ds.context->cpu != smp_processor_id()))
		goto out;

	error = -EPERM;
	if (task && (task != current))
		goto out;

	ds_suspend_pebs_noirq(tracer);
	ds_free_pebs(tracer);

	error = 0;
 out:
	local_irq_restore(irq);
	return error;
}

void ds_suspend_pebs(struct pebs_tracer *tracer)
{

}

int ds_suspend_pebs_noirq(struct pebs_tracer *tracer)
{
	return 0;
}

void ds_resume_pebs(struct pebs_tracer *tracer)
{

}

int ds_resume_pebs_noirq(struct pebs_tracer *tracer)
{
	return 0;
}

const struct bts_trace *ds_read_bts(struct bts_tracer *tracer)
{
	if (!tracer)
		return NULL;

	ds_read_config(tracer->ds.context, &tracer->trace.ds, ds_bts);
	return &tracer->trace;
}

const struct pebs_trace *ds_read_pebs(struct pebs_tracer *tracer)
{
	if (!tracer)
		return NULL;

	ds_read_config(tracer->ds.context, &tracer->trace.ds, ds_pebs);

	tracer->trace.counters = ds_cfg.nr_counter_reset;
	memcpy(tracer->trace.counter_reset,
	       tracer->ds.context->ds +
	       (NUM_DS_PTR_FIELDS * ds_cfg.sizeof_ptr_field),
	       ds_cfg.nr_counter_reset * PEBS_RESET_FIELD_SIZE);

	return &tracer->trace;
}

int ds_reset_bts(struct bts_tracer *tracer)
{
	if (!tracer)
		return -EINVAL;

	tracer->trace.ds.top = tracer->trace.ds.begin;

	ds_set(tracer->ds.context->ds, ds_bts, ds_index,
	       (unsigned long)tracer->trace.ds.top);

	return 0;
}

int ds_reset_pebs(struct pebs_tracer *tracer)
{
	if (!tracer)
		return -EINVAL;

	tracer->trace.ds.top = tracer->trace.ds.begin;

	ds_set(tracer->ds.context->ds, ds_pebs, ds_index,
	       (unsigned long)tracer->trace.ds.top);

	return 0;
}

int ds_set_pebs_reset(struct pebs_tracer *tracer,
		      unsigned int counter, u64 value)
{
	if (!tracer)
		return -EINVAL;

	if (ds_cfg.nr_counter_reset < counter)
		return -EINVAL;

	*(u64 *)(tracer->ds.context->ds +
		 (NUM_DS_PTR_FIELDS * ds_cfg.sizeof_ptr_field) +
		 (counter * PEBS_RESET_FIELD_SIZE)) = value;

	return 0;
}

static const struct ds_configuration ds_cfg_netburst = {
	.name = "Netburst",
	.ctl[dsf_bts]		= (1 << 2) | (1 << 3),
	.ctl[dsf_bts_kernel]	= (1 << 5),
	.ctl[dsf_bts_user]	= (1 << 6),
	.nr_counter_reset	= 1,
};
static const struct ds_configuration ds_cfg_pentium_m = {
	.name = "Pentium M",
	.ctl[dsf_bts]		= (1 << 6) | (1 << 7),
	.nr_counter_reset	= 1,
};
static const struct ds_configuration ds_cfg_core2_atom = {
	.name = "Core 2/Atom",
	.ctl[dsf_bts]		= (1 << 6) | (1 << 7),
	.ctl[dsf_bts_kernel]	= (1 << 9),
	.ctl[dsf_bts_user]	= (1 << 10),
	.nr_counter_reset	= 1,
};
static const struct ds_configuration ds_cfg_core_i7 = {
	.name = "Core i7",
	.ctl[dsf_bts]		= (1 << 6) | (1 << 7),
	.ctl[dsf_bts_kernel]	= (1 << 9),
	.ctl[dsf_bts_user]	= (1 << 10),
	.nr_counter_reset	= 4,
};

static void
ds_configure(const struct ds_configuration *cfg,
	     struct cpuinfo_x86 *cpu)
{
	unsigned long nr_pebs_fields = 0;

	printk(KERN_INFO "[ds] using %s configuration\n", cfg->name);

#ifdef __i386__
	nr_pebs_fields = 10;
#else
	nr_pebs_fields = 18;
#endif

	
	if ((cpuid_eax(0xa) & 0xff) > 1) {
		unsigned long perf_capabilities, format;

		rdmsrl(MSR_IA32_PERF_CAPABILITIES, perf_capabilities);

		format = (perf_capabilities >> 8) & 0xf;

		switch (format) {
		case 0:
			nr_pebs_fields = 18;
			break;
		case 1:
			nr_pebs_fields = 22;
			break;
		default:
			printk(KERN_INFO
			       "[ds] unknown PEBS format: %lu\n", format);
			nr_pebs_fields = 0;
			break;
		}
	}

	memset(&ds_cfg, 0, sizeof(ds_cfg));
	ds_cfg = *cfg;

	ds_cfg.sizeof_ptr_field =
		(cpu_has(cpu, X86_FEATURE_DTES64) ? 8 : 4);

	ds_cfg.sizeof_rec[ds_bts]  = ds_cfg.sizeof_ptr_field * 3;
	ds_cfg.sizeof_rec[ds_pebs] = ds_cfg.sizeof_ptr_field * nr_pebs_fields;

	if (!cpu_has(cpu, X86_FEATURE_BTS)) {
		ds_cfg.sizeof_rec[ds_bts] = 0;
		printk(KERN_INFO "[ds] bts not available\n");
	}
	if (!cpu_has(cpu, X86_FEATURE_PEBS)) {
		ds_cfg.sizeof_rec[ds_pebs] = 0;
		printk(KERN_INFO "[ds] pebs not available\n");
	}

	printk(KERN_INFO "[ds] sizes: address: %u bit, ",
	       8 * ds_cfg.sizeof_ptr_field);
	printk("bts/pebs record: %u/%u bytes\n",
	       ds_cfg.sizeof_rec[ds_bts], ds_cfg.sizeof_rec[ds_pebs]);

	WARN_ON_ONCE(MAX_PEBS_COUNTERS < ds_cfg.nr_counter_reset);
}

void __cpuinit ds_init_intel(struct cpuinfo_x86 *c)
{
	
	if (ds_cfg.name)
		return;

	switch (c->x86) {
	case 0x6:
		switch (c->x86_model) {
		case 0x9:
		case 0xd: 
			ds_configure(&ds_cfg_pentium_m, c);
			break;
		case 0xf:
		case 0x17: 
		case 0x1c: 
			ds_configure(&ds_cfg_core2_atom, c);
			break;
		case 0x1a: 
			ds_configure(&ds_cfg_core_i7, c);
			break;
		default:
			
			break;
		}
		break;
	case 0xf:
		switch (c->x86_model) {
		case 0x0:
		case 0x1:
		case 0x2: 
			ds_configure(&ds_cfg_netburst, c);
			break;
		default:
			
			break;
		}
		break;
	default:
		
		break;
	}
}

static inline void ds_take_timestamp(struct ds_context *context,
				     enum bts_qualifier qualifier,
				     struct task_struct *task)
{
	struct bts_tracer *tracer = context->bts_master;
	struct bts_struct ts;

	
	barrier();

	if (!tracer || !(tracer->flags & BTS_TIMESTAMPS))
		return;

	memset(&ts, 0, sizeof(ts));
	ts.qualifier		= qualifier;
	ts.variant.event.clock	= trace_clock_global();
	ts.variant.event.pid	= task->pid;

	bts_write(tracer, &ts);
}


void ds_switch_to(struct task_struct *prev, struct task_struct *next)
{
	struct ds_context *prev_ctx	= prev->thread.ds_ctx;
	struct ds_context *next_ctx	= next->thread.ds_ctx;
	unsigned long debugctlmsr	= next->thread.debugctlmsr;

	
	barrier();

	if (prev_ctx) {
		update_debugctlmsr(0);

		ds_take_timestamp(prev_ctx, bts_task_departs, prev);
	}

	if (next_ctx) {
		ds_take_timestamp(next_ctx, bts_task_arrives, next);

		wrmsrl(MSR_IA32_DS_AREA, (unsigned long)next_ctx->ds);
	}

	update_debugctlmsr(debugctlmsr);
}

static __init int ds_selftest(void)
{
	if (ds_cfg.sizeof_rec[ds_bts]) {
		int error;

		error = ds_selftest_bts();
		if (error) {
			WARN(1, "[ds] selftest failed. disabling bts.\n");
			ds_cfg.sizeof_rec[ds_bts] = 0;
		}
	}

	if (ds_cfg.sizeof_rec[ds_pebs]) {
		int error;

		error = ds_selftest_pebs();
		if (error) {
			WARN(1, "[ds] selftest failed. disabling pebs.\n");
			ds_cfg.sizeof_rec[ds_pebs] = 0;
		}
	}

	return 0;
}
device_initcall(ds_selftest);
