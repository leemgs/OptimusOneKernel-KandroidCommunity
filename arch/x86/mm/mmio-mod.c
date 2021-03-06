
#define DEBUG 1

#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/version.h>
#include <linux/kallsyms.h>
#include <asm/pgtable.h>
#include <linux/mmiotrace.h>
#include <asm/e820.h> 
#include <asm/atomic.h>
#include <linux/percpu.h>
#include <linux/cpu.h>

#include "pf_in.h"

#define NAME "mmiotrace: "

struct trap_reason {
	unsigned long addr;
	unsigned long ip;
	enum reason_type type;
	int active_traces;
};

struct remap_trace {
	struct list_head list;
	struct kmmio_probe probe;
	resource_size_t phys;
	unsigned long id;
};


static DEFINE_PER_CPU(struct trap_reason, pf_reason);
static DEFINE_PER_CPU(struct mmiotrace_rw, cpu_trace);

static DEFINE_MUTEX(mmiotrace_mutex);
static DEFINE_SPINLOCK(trace_lock);
static atomic_t mmiotrace_enabled;
static LIST_HEAD(trace_list);		




static unsigned long	filter_offset;
static int		nommiotrace;
static int		trace_pc;

module_param(filter_offset, ulong, 0);
module_param(nommiotrace, bool, 0);
module_param(trace_pc, bool, 0);

MODULE_PARM_DESC(filter_offset, "Start address of traced mappings.");
MODULE_PARM_DESC(nommiotrace, "Disable actual MMIO tracing.");
MODULE_PARM_DESC(trace_pc, "Record address of faulting instructions.");

static bool is_enabled(void)
{
	return atomic_read(&mmiotrace_enabled);
}

static void print_pte(unsigned long address)
{
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);

	if (!pte) {
		pr_err(NAME "Error in %s: no pte for page 0x%08lx\n",
							__func__, address);
		return;
	}

	if (level == PG_LEVEL_2M) {
		pr_emerg(NAME "4MB pages are not currently supported: "
							"0x%08lx\n", address);
		BUG();
	}
	pr_info(NAME "pte for 0x%lx: 0x%llx 0x%llx\n", address,
		(unsigned long long)pte_val(*pte),
		(unsigned long long)pte_val(*pte) & _PAGE_PRESENT);
}


static void die_kmmio_nesting_error(struct pt_regs *regs, unsigned long addr)
{
	const struct trap_reason *my_reason = &get_cpu_var(pf_reason);
	pr_emerg(NAME "unexpected fault for address: 0x%08lx, "
					"last fault for address: 0x%08lx\n",
					addr, my_reason->addr);
	print_pte(addr);
	print_symbol(KERN_EMERG "faulting IP is at %s\n", regs->ip);
	print_symbol(KERN_EMERG "last faulting IP was at %s\n", my_reason->ip);
#ifdef __i386__
	pr_emerg("eax: %08lx   ebx: %08lx   ecx: %08lx   edx: %08lx\n",
			regs->ax, regs->bx, regs->cx, regs->dx);
	pr_emerg("esi: %08lx   edi: %08lx   ebp: %08lx   esp: %08lx\n",
			regs->si, regs->di, regs->bp, regs->sp);
#else
	pr_emerg("rax: %016lx   rcx: %016lx   rdx: %016lx\n",
					regs->ax, regs->cx, regs->dx);
	pr_emerg("rsi: %016lx   rdi: %016lx   rbp: %016lx   rsp: %016lx\n",
				regs->si, regs->di, regs->bp, regs->sp);
#endif
	put_cpu_var(pf_reason);
	BUG();
}

static void pre(struct kmmio_probe *p, struct pt_regs *regs,
						unsigned long addr)
{
	struct trap_reason *my_reason = &get_cpu_var(pf_reason);
	struct mmiotrace_rw *my_trace = &get_cpu_var(cpu_trace);
	const unsigned long instptr = instruction_pointer(regs);
	const enum reason_type type = get_ins_type(instptr);
	struct remap_trace *trace = p->private;

	
	if (my_reason->active_traces)
		die_kmmio_nesting_error(regs, addr);
	else
		my_reason->active_traces++;

	my_reason->type = type;
	my_reason->addr = addr;
	my_reason->ip = instptr;

	my_trace->phys = addr - trace->probe.addr + trace->phys;
	my_trace->map_id = trace->id;

	
	if (trace_pc)
		my_trace->pc = instptr;
	else
		my_trace->pc = 0;

	

	switch (type) {
	case REG_READ:
		my_trace->opcode = MMIO_READ;
		my_trace->width = get_ins_mem_width(instptr);
		break;
	case REG_WRITE:
		my_trace->opcode = MMIO_WRITE;
		my_trace->width = get_ins_mem_width(instptr);
		my_trace->value = get_ins_reg_val(instptr, regs);
		break;
	case IMM_WRITE:
		my_trace->opcode = MMIO_WRITE;
		my_trace->width = get_ins_mem_width(instptr);
		my_trace->value = get_ins_imm_val(instptr);
		break;
	default:
		{
			unsigned char *ip = (unsigned char *)instptr;
			my_trace->opcode = MMIO_UNKNOWN_OP;
			my_trace->width = 0;
			my_trace->value = (*ip) << 16 | *(ip + 1) << 8 |
								*(ip + 2);
		}
	}
	put_cpu_var(cpu_trace);
	put_cpu_var(pf_reason);
}

static void post(struct kmmio_probe *p, unsigned long condition,
							struct pt_regs *regs)
{
	struct trap_reason *my_reason = &get_cpu_var(pf_reason);
	struct mmiotrace_rw *my_trace = &get_cpu_var(cpu_trace);

	
	my_reason->active_traces--;
	if (my_reason->active_traces) {
		pr_emerg(NAME "unexpected post handler");
		BUG();
	}

	switch (my_reason->type) {
	case REG_READ:
		my_trace->value = get_ins_reg_val(my_reason->ip, regs);
		break;
	default:
		break;
	}

	mmio_trace_rw(my_trace);
	put_cpu_var(cpu_trace);
	put_cpu_var(pf_reason);
}

static void ioremap_trace_core(resource_size_t offset, unsigned long size,
							void __iomem *addr)
{
	static atomic_t next_id;
	struct remap_trace *trace = kmalloc(sizeof(*trace), GFP_KERNEL);
	
	struct mmiotrace_map map = {
		.phys = offset,
		.virt = (unsigned long)addr,
		.len = size,
		.opcode = MMIO_PROBE
	};

	if (!trace) {
		pr_err(NAME "kmalloc failed in ioremap\n");
		return;
	}

	*trace = (struct remap_trace) {
		.probe = {
			.addr = (unsigned long)addr,
			.len = size,
			.pre_handler = pre,
			.post_handler = post,
			.private = trace
		},
		.phys = offset,
		.id = atomic_inc_return(&next_id)
	};
	map.map_id = trace->id;

	spin_lock_irq(&trace_lock);
	if (!is_enabled()) {
		kfree(trace);
		goto not_enabled;
	}

	mmio_trace_mapping(&map);
	list_add_tail(&trace->list, &trace_list);
	if (!nommiotrace)
		register_kmmio_probe(&trace->probe);

not_enabled:
	spin_unlock_irq(&trace_lock);
}

void mmiotrace_ioremap(resource_size_t offset, unsigned long size,
						void __iomem *addr)
{
	if (!is_enabled()) 
		return;

	pr_debug(NAME "ioremap_*(0x%llx, 0x%lx) = %p\n",
				(unsigned long long)offset, size, addr);
	if ((filter_offset) && (offset != filter_offset))
		return;
	ioremap_trace_core(offset, size, addr);
}

static void iounmap_trace_core(volatile void __iomem *addr)
{
	struct mmiotrace_map map = {
		.phys = 0,
		.virt = (unsigned long)addr,
		.len = 0,
		.opcode = MMIO_UNPROBE
	};
	struct remap_trace *trace;
	struct remap_trace *tmp;
	struct remap_trace *found_trace = NULL;

	pr_debug(NAME "Unmapping %p.\n", addr);

	spin_lock_irq(&trace_lock);
	if (!is_enabled())
		goto not_enabled;

	list_for_each_entry_safe(trace, tmp, &trace_list, list) {
		if ((unsigned long)addr == trace->probe.addr) {
			if (!nommiotrace)
				unregister_kmmio_probe(&trace->probe);
			list_del(&trace->list);
			found_trace = trace;
			break;
		}
	}
	map.map_id = (found_trace) ? found_trace->id : -1;
	mmio_trace_mapping(&map);

not_enabled:
	spin_unlock_irq(&trace_lock);
	if (found_trace) {
		synchronize_rcu(); 
		kfree(found_trace);
	}
}

void mmiotrace_iounmap(volatile void __iomem *addr)
{
	might_sleep();
	if (is_enabled()) 
		iounmap_trace_core(addr);
}

int mmiotrace_printk(const char *fmt, ...)
{
	int ret = 0;
	va_list args;
	unsigned long flags;
	va_start(args, fmt);

	spin_lock_irqsave(&trace_lock, flags);
	if (is_enabled())
		ret = mmio_trace_printk(fmt, args);
	spin_unlock_irqrestore(&trace_lock, flags);

	va_end(args);
	return ret;
}
EXPORT_SYMBOL(mmiotrace_printk);

static void clear_trace_list(void)
{
	struct remap_trace *trace;
	struct remap_trace *tmp;

	
	list_for_each_entry(trace, &trace_list, list) {
		pr_notice(NAME "purging non-iounmapped "
					"trace @0x%08lx, size 0x%lx.\n",
					trace->probe.addr, trace->probe.len);
		if (!nommiotrace)
			unregister_kmmio_probe(&trace->probe);
	}
	synchronize_rcu(); 

	list_for_each_entry_safe(trace, tmp, &trace_list, list) {
		list_del(&trace->list);
		kfree(trace);
	}
}

#ifdef CONFIG_HOTPLUG_CPU
static cpumask_var_t downed_cpus;

static void enter_uniprocessor(void)
{
	int cpu;
	int err;

	if (downed_cpus == NULL &&
	    !alloc_cpumask_var(&downed_cpus, GFP_KERNEL)) {
		pr_notice(NAME "Failed to allocate mask\n");
		goto out;
	}

	get_online_cpus();
	cpumask_copy(downed_cpus, cpu_online_mask);
	cpumask_clear_cpu(cpumask_first(cpu_online_mask), downed_cpus);
	if (num_online_cpus() > 1)
		pr_notice(NAME "Disabling non-boot CPUs...\n");
	put_online_cpus();

	for_each_cpu(cpu, downed_cpus) {
		err = cpu_down(cpu);
		if (!err)
			pr_info(NAME "CPU%d is down.\n", cpu);
		else
			pr_err(NAME "Error taking CPU%d down: %d\n", cpu, err);
	}
out:
	if (num_online_cpus() > 1)
		pr_warning(NAME "multiple CPUs still online, "
						"may miss events.\n");
}


static void __ref leave_uniprocessor(void)
{
	int cpu;
	int err;

	if (downed_cpus == NULL || cpumask_weight(downed_cpus) == 0)
		return;
	pr_notice(NAME "Re-enabling CPUs...\n");
	for_each_cpu(cpu, downed_cpus) {
		err = cpu_up(cpu);
		if (!err)
			pr_info(NAME "enabled CPU%d.\n", cpu);
		else
			pr_err(NAME "cannot re-enable CPU%d: %d\n", cpu, err);
	}
}

#else 
static void enter_uniprocessor(void)
{
	if (num_online_cpus() > 1)
		pr_warning(NAME "multiple CPUs are online, may miss events. "
			"Suggest booting with maxcpus=1 kernel argument.\n");
}

static void leave_uniprocessor(void)
{
}
#endif

void enable_mmiotrace(void)
{
	mutex_lock(&mmiotrace_mutex);
	if (is_enabled())
		goto out;

	if (nommiotrace)
		pr_info(NAME "MMIO tracing disabled.\n");
	kmmio_init();
	enter_uniprocessor();
	spin_lock_irq(&trace_lock);
	atomic_inc(&mmiotrace_enabled);
	spin_unlock_irq(&trace_lock);
	pr_info(NAME "enabled.\n");
out:
	mutex_unlock(&mmiotrace_mutex);
}

void disable_mmiotrace(void)
{
	mutex_lock(&mmiotrace_mutex);
	if (!is_enabled())
		goto out;

	spin_lock_irq(&trace_lock);
	atomic_dec(&mmiotrace_enabled);
	BUG_ON(is_enabled());
	spin_unlock_irq(&trace_lock);

	clear_trace_list(); 
	leave_uniprocessor();
	kmmio_cleanup();
	pr_info(NAME "disabled.\n");
out:
	mutex_unlock(&mmiotrace_mutex);
}
