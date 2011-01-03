

#include <linux/spinlock.h>
#include <linux/hardirq.h>
#include <linux/uaccess.h>
#include <linux/ftrace.h>
#include <linux/percpu.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/list.h>

#include <trace/syscall.h>

#include <asm/cacheflush.h>
#include <asm/ftrace.h>
#include <asm/nops.h>
#include <asm/nmi.h>


#ifdef CONFIG_DYNAMIC_FTRACE

int ftrace_arch_code_modify_prepare(void)
{
	set_kernel_text_rw();
	return 0;
}

int ftrace_arch_code_modify_post_process(void)
{
	set_kernel_text_ro();
	return 0;
}

union ftrace_code_union {
	char code[MCOUNT_INSN_SIZE];
	struct {
		char e8;
		int offset;
	} __attribute__((packed));
};

static int ftrace_calc_offset(long ip, long addr)
{
	return (int)(addr - ip);
}

static unsigned char *ftrace_call_replace(unsigned long ip, unsigned long addr)
{
	static union ftrace_code_union calc;

	calc.e8		= 0xe8;
	calc.offset	= ftrace_calc_offset(ip + MCOUNT_INSN_SIZE, addr);

	
	return calc.code;
}



#define MOD_CODE_WRITE_FLAG (1 << 31)	
static atomic_t nmi_running = ATOMIC_INIT(0);
static int mod_code_status;		
static void *mod_code_ip;		
static void *mod_code_newcode;		

static unsigned nmi_wait_count;
static atomic_t nmi_update_count = ATOMIC_INIT(0);

int ftrace_arch_read_dyn_info(char *buf, int size)
{
	int r;

	r = snprintf(buf, size, "%u %u",
		     nmi_wait_count,
		     atomic_read(&nmi_update_count));
	return r;
}

static void clear_mod_flag(void)
{
	int old = atomic_read(&nmi_running);

	for (;;) {
		int new = old & ~MOD_CODE_WRITE_FLAG;

		if (old == new)
			break;

		old = atomic_cmpxchg(&nmi_running, old, new);
	}
}

static void ftrace_mod_code(void)
{
	
	mod_code_status = probe_kernel_write(mod_code_ip, mod_code_newcode,
					     MCOUNT_INSN_SIZE);

	
	if (mod_code_status)
		clear_mod_flag();
}

void ftrace_nmi_enter(void)
{
	if (atomic_inc_return(&nmi_running) & MOD_CODE_WRITE_FLAG) {
		smp_rmb();
		ftrace_mod_code();
		atomic_inc(&nmi_update_count);
	}
	
	smp_mb();
}

void ftrace_nmi_exit(void)
{
	
	smp_mb();
	atomic_dec(&nmi_running);
}

static void wait_for_nmi_and_set_mod_flag(void)
{
	if (!atomic_cmpxchg(&nmi_running, 0, MOD_CODE_WRITE_FLAG))
		return;

	do {
		cpu_relax();
	} while (atomic_cmpxchg(&nmi_running, 0, MOD_CODE_WRITE_FLAG));

	nmi_wait_count++;
}

static void wait_for_nmi(void)
{
	if (!atomic_read(&nmi_running))
		return;

	do {
		cpu_relax();
	} while (atomic_read(&nmi_running));

	nmi_wait_count++;
}

static int
do_ftrace_mod_code(unsigned long ip, void *new_code)
{
	mod_code_ip = (void *)ip;
	mod_code_newcode = new_code;

	
	smp_mb();

	wait_for_nmi_and_set_mod_flag();

	
	smp_mb();

	ftrace_mod_code();

	
	smp_mb();

	clear_mod_flag();
	wait_for_nmi();

	return mod_code_status;
}




static unsigned char ftrace_nop[MCOUNT_INSN_SIZE];

static unsigned char *ftrace_nop_replace(void)
{
	return ftrace_nop;
}

static int
ftrace_modify_code(unsigned long ip, unsigned char *old_code,
		   unsigned char *new_code)
{
	unsigned char replaced[MCOUNT_INSN_SIZE];

	

	
	if (probe_kernel_read(replaced, (void *)ip, MCOUNT_INSN_SIZE))
		return -EFAULT;

	
	if (memcmp(replaced, old_code, MCOUNT_INSN_SIZE) != 0)
		return -EINVAL;

	
	if (do_ftrace_mod_code(ip, new_code))
		return -EPERM;

	sync_core();

	return 0;
}

int ftrace_make_nop(struct module *mod,
		    struct dyn_ftrace *rec, unsigned long addr)
{
	unsigned char *new, *old;
	unsigned long ip = rec->ip;

	old = ftrace_call_replace(ip, addr);
	new = ftrace_nop_replace();

	return ftrace_modify_code(rec->ip, old, new);
}

int ftrace_make_call(struct dyn_ftrace *rec, unsigned long addr)
{
	unsigned char *new, *old;
	unsigned long ip = rec->ip;

	old = ftrace_nop_replace();
	new = ftrace_call_replace(ip, addr);

	return ftrace_modify_code(rec->ip, old, new);
}

int ftrace_update_ftrace_func(ftrace_func_t func)
{
	unsigned long ip = (unsigned long)(&ftrace_call);
	unsigned char old[MCOUNT_INSN_SIZE], *new;
	int ret;

	memcpy(old, &ftrace_call, MCOUNT_INSN_SIZE);
	new = ftrace_call_replace(ip, (unsigned long)func);
	ret = ftrace_modify_code(ip, old, new);

	return ret;
}

int __init ftrace_dyn_arch_init(void *data)
{
	extern const unsigned char ftrace_test_p6nop[];
	extern const unsigned char ftrace_test_nop5[];
	extern const unsigned char ftrace_test_jmp[];
	int faulted = 0;

	
	asm volatile (
		"ftrace_test_jmp:"
		"jmp ftrace_test_p6nop\n"
		"nop\n"
		"nop\n"
		"nop\n"  
		"ftrace_test_p6nop:"
		P6_NOP5
		"jmp 1f\n"
		"ftrace_test_nop5:"
		".byte 0x66,0x66,0x66,0x66,0x90\n"
		"1:"
		".section .fixup, \"ax\"\n"
		"2:	movl $1, %0\n"
		"	jmp ftrace_test_nop5\n"
		"3:	movl $2, %0\n"
		"	jmp 1b\n"
		".previous\n"
		_ASM_EXTABLE(ftrace_test_p6nop, 2b)
		_ASM_EXTABLE(ftrace_test_nop5, 3b)
		: "=r"(faulted) : "0" (faulted));

	switch (faulted) {
	case 0:
		pr_info("ftrace: converting mcount calls to 0f 1f 44 00 00\n");
		memcpy(ftrace_nop, ftrace_test_p6nop, MCOUNT_INSN_SIZE);
		break;
	case 1:
		pr_info("ftrace: converting mcount calls to 66 66 66 66 90\n");
		memcpy(ftrace_nop, ftrace_test_nop5, MCOUNT_INSN_SIZE);
		break;
	case 2:
		pr_info("ftrace: converting mcount calls to jmp . + 5\n");
		memcpy(ftrace_nop, ftrace_test_jmp, MCOUNT_INSN_SIZE);
		break;
	}

	
	*(unsigned long *)data = 0;

	return 0;
}
#endif

#ifdef CONFIG_FUNCTION_GRAPH_TRACER

#ifdef CONFIG_DYNAMIC_FTRACE
extern void ftrace_graph_call(void);

static int ftrace_mod_jmp(unsigned long ip,
			  int old_offset, int new_offset)
{
	unsigned char code[MCOUNT_INSN_SIZE];

	if (probe_kernel_read(code, (void *)ip, MCOUNT_INSN_SIZE))
		return -EFAULT;

	if (code[0] != 0xe9 || old_offset != *(int *)(&code[1]))
		return -EINVAL;

	*(int *)(&code[1]) = new_offset;

	if (do_ftrace_mod_code(ip, &code))
		return -EPERM;

	return 0;
}

int ftrace_enable_ftrace_graph_caller(void)
{
	unsigned long ip = (unsigned long)(&ftrace_graph_call);
	int old_offset, new_offset;

	old_offset = (unsigned long)(&ftrace_stub) - (ip + MCOUNT_INSN_SIZE);
	new_offset = (unsigned long)(&ftrace_graph_caller) - (ip + MCOUNT_INSN_SIZE);

	return ftrace_mod_jmp(ip, old_offset, new_offset);
}

int ftrace_disable_ftrace_graph_caller(void)
{
	unsigned long ip = (unsigned long)(&ftrace_graph_call);
	int old_offset, new_offset;

	old_offset = (unsigned long)(&ftrace_graph_caller) - (ip + MCOUNT_INSN_SIZE);
	new_offset = (unsigned long)(&ftrace_stub) - (ip + MCOUNT_INSN_SIZE);

	return ftrace_mod_jmp(ip, old_offset, new_offset);
}

#endif 


void prepare_ftrace_return(unsigned long *parent, unsigned long self_addr,
			   unsigned long frame_pointer)
{
	unsigned long old;
	int faulted;
	struct ftrace_graph_ent trace;
	unsigned long return_hooker = (unsigned long)
				&return_to_handler;

	if (unlikely(atomic_read(&current->tracing_graph_pause)))
		return;

	
	asm volatile(
		"1: " _ASM_MOV " (%[parent]), %[old]\n"
		"2: " _ASM_MOV " %[return_hooker], (%[parent])\n"
		"   movl $0, %[faulted]\n"
		"3:\n"

		".section .fixup, \"ax\"\n"
		"4: movl $1, %[faulted]\n"
		"   jmp 3b\n"
		".previous\n"

		_ASM_EXTABLE(1b, 4b)
		_ASM_EXTABLE(2b, 4b)

		: [old] "=&r" (old), [faulted] "=r" (faulted)
		: [parent] "r" (parent), [return_hooker] "r" (return_hooker)
		: "memory"
	);

	if (unlikely(faulted)) {
		ftrace_graph_stop();
		WARN_ON(1);
		return;
	}

	if (ftrace_push_return_trace(old, self_addr, &trace.depth,
		    frame_pointer) == -EBUSY) {
		*parent = old;
		return;
	}

	trace.func = self_addr;

	
	if (!ftrace_graph_entry(&trace)) {
		current->curr_ret_stack--;
		*parent = old;
	}
}
#endif 

#ifdef CONFIG_FTRACE_SYSCALLS

extern unsigned long __start_syscalls_metadata[];
extern unsigned long __stop_syscalls_metadata[];
extern unsigned long *sys_call_table;

static struct syscall_metadata **syscalls_metadata;

static struct syscall_metadata *find_syscall_meta(unsigned long *syscall)
{
	struct syscall_metadata *start;
	struct syscall_metadata *stop;
	char str[KSYM_SYMBOL_LEN];


	start = (struct syscall_metadata *)__start_syscalls_metadata;
	stop = (struct syscall_metadata *)__stop_syscalls_metadata;
	kallsyms_lookup((unsigned long) syscall, NULL, NULL, NULL, str);

	for ( ; start < stop; start++) {
		if (start->name && !strcmp(start->name, str))
			return start;
	}
	return NULL;
}

struct syscall_metadata *syscall_nr_to_meta(int nr)
{
	if (!syscalls_metadata || nr >= NR_syscalls || nr < 0)
		return NULL;

	return syscalls_metadata[nr];
}

int syscall_name_to_nr(char *name)
{
	int i;

	if (!syscalls_metadata)
		return -1;

	for (i = 0; i < NR_syscalls; i++) {
		if (syscalls_metadata[i]) {
			if (!strcmp(syscalls_metadata[i]->name, name))
				return i;
		}
	}
	return -1;
}

void set_syscall_enter_id(int num, int id)
{
	syscalls_metadata[num]->enter_id = id;
}

void set_syscall_exit_id(int num, int id)
{
	syscalls_metadata[num]->exit_id = id;
}

static int __init arch_init_ftrace_syscalls(void)
{
	int i;
	struct syscall_metadata *meta;
	unsigned long **psys_syscall_table = &sys_call_table;

	syscalls_metadata = kzalloc(sizeof(*syscalls_metadata) *
					NR_syscalls, GFP_KERNEL);
	if (!syscalls_metadata) {
		WARN_ON(1);
		return -ENOMEM;
	}

	for (i = 0; i < NR_syscalls; i++) {
		meta = find_syscall_meta(psys_syscall_table[i]);
		syscalls_metadata[i] = meta;
	}
	return 0;
}
arch_initcall(arch_init_ftrace_syscalls);
#endif
