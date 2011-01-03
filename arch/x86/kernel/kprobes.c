

#include <linux/kprobes.h>
#include <linux/ptrace.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/hardirq.h>
#include <linux/preempt.h>
#include <linux/module.h>
#include <linux/kdebug.h>

#include <asm/cacheflush.h>
#include <asm/desc.h>
#include <asm/pgtable.h>
#include <asm/uaccess.h>
#include <asm/alternative.h>

void jprobe_return_end(void);

DEFINE_PER_CPU(struct kprobe *, current_kprobe) = NULL;
DEFINE_PER_CPU(struct kprobe_ctlblk, kprobe_ctlblk);

#ifdef CONFIG_X86_64
#define stack_addr(regs) ((unsigned long *)regs->sp)
#else

#define stack_addr(regs) ((unsigned long *)&regs->sp)
#endif

#define W(row, b0, b1, b2, b3, b4, b5, b6, b7, b8, b9, ba, bb, bc, bd, be, bf)\
	(((b0##UL << 0x0)|(b1##UL << 0x1)|(b2##UL << 0x2)|(b3##UL << 0x3) |   \
	  (b4##UL << 0x4)|(b5##UL << 0x5)|(b6##UL << 0x6)|(b7##UL << 0x7) |   \
	  (b8##UL << 0x8)|(b9##UL << 0x9)|(ba##UL << 0xa)|(bb##UL << 0xb) |   \
	  (bc##UL << 0xc)|(bd##UL << 0xd)|(be##UL << 0xe)|(bf##UL << 0xf))    \
	 << (row % 32))
	
static const u32 twobyte_is_boostable[256 / 32] = {
	
	
	W(0x00, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0) | 
	W(0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) , 
	W(0x20, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) | 
	W(0x30, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) , 
	W(0x40, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) | 
	W(0x50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) , 
	W(0x60, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1) | 
	W(0x70, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1) , 
	W(0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) | 
	W(0x90, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) , 
	W(0xa0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1) | 
	W(0xb0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1) , 
	W(0xc0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1) | 
	W(0xd0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1) , 
	W(0xe0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1) | 
	W(0xf0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0)   
	
	
};
static const u32 onebyte_has_modrm[256 / 32] = {
	
	
	W(0x00, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0) | 
	W(0x10, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0) , 
	W(0x20, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0) | 
	W(0x30, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0) , 
	W(0x40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) | 
	W(0x50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) , 
	W(0x60, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0) | 
	W(0x70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) , 
	W(0x80, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) | 
	W(0x90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) , 
	W(0xa0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) | 
	W(0xb0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) , 
	W(0xc0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0) | 
	W(0xd0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1) , 
	W(0xe0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) | 
	W(0xf0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1)   
	
	
};
static const u32 twobyte_has_modrm[256 / 32] = {
	
	
	W(0x00, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1) | 
	W(0x10, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0) , 
	W(0x20, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1) | 
	W(0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) , 
	W(0x40, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) | 
	W(0x50, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) , 
	W(0x60, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) | 
	W(0x70, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1) , 
	W(0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) | 
	W(0x90, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) , 
	W(0xa0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1) | 
	W(0xb0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1) , 
	W(0xc0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0) | 
	W(0xd0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) , 
	W(0xe0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1) | 
	W(0xf0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)   
	
	
};
#undef W

struct kretprobe_blackpoint kretprobe_blacklist[] = {
	{"__switch_to", }, 
	{NULL, NULL}	
};
const int kretprobe_blacklist_size = ARRAY_SIZE(kretprobe_blacklist);


static void __kprobes set_jmp_op(void *from, void *to)
{
	struct __arch_jmp_op {
		char op;
		s32 raddr;
	} __attribute__((packed)) * jop;
	jop = (struct __arch_jmp_op *)from;
	jop->raddr = (s32)((long)(to) - ((long)(from) + 5));
	jop->op = RELATIVEJUMP_INSTRUCTION;
}


static int __kprobes is_REX_prefix(kprobe_opcode_t *insn)
{
#ifdef CONFIG_X86_64
	if ((*insn & 0xf0) == 0x40)
		return 1;
#endif
	return 0;
}


static int __kprobes can_boost(kprobe_opcode_t *opcodes)
{
	kprobe_opcode_t opcode;
	kprobe_opcode_t *orig_opcodes = opcodes;

	if (search_exception_tables((unsigned long)opcodes))
		return 0;	

retry:
	if (opcodes - orig_opcodes > MAX_INSN_SIZE - 1)
		return 0;
	opcode = *(opcodes++);

	
	if (opcode == 0x0f) {
		if (opcodes - orig_opcodes > MAX_INSN_SIZE - 1)
			return 0;
		return test_bit(*opcodes,
				(unsigned long *)twobyte_is_boostable);
	}

	switch (opcode & 0xf0) {
#ifdef CONFIG_X86_64
	case 0x40:
		goto retry; 
#endif
	case 0x60:
		if (0x63 < opcode && opcode < 0x67)
			goto retry; 
		
		return (opcode != 0x62 && opcode != 0x67);
	case 0x70:
		return 0; 
	case 0xc0:
		
		return (0xc1 < opcode && opcode < 0xcc) || opcode == 0xcf;
	case 0xd0:
		
		return (opcode == 0xd4 || opcode == 0xd5 || opcode == 0xd7);
	case 0xe0:
		
		return ((opcode & 0x04) || opcode == 0xea);
	case 0xf0:
		if ((opcode & 0x0c) == 0 && opcode != 0xf1)
			goto retry; 
		
		return (opcode == 0xf5 || (0xf7 < opcode && opcode < 0xfe));
	default:
		
		if (opcode == 0x26 || opcode == 0x36 || opcode == 0x3e)
			goto retry; 
		
		return (opcode != 0x2e && opcode != 0x9a);
	}
}


static int __kprobes is_IF_modifier(kprobe_opcode_t *insn)
{
	switch (*insn) {
	case 0xfa:		
	case 0xfb:		
	case 0xcf:		
	case 0x9d:		
		return 1;
	}

	
	if (is_REX_prefix(insn))
		return is_IF_modifier(++insn);

	return 0;
}


static void __kprobes fix_riprel(struct kprobe *p)
{
#ifdef CONFIG_X86_64
	u8 *insn = p->ainsn.insn;
	s64 disp;
	int need_modrm;

	
	while (1) {
		switch (*insn) {
		case 0x66:
		case 0x67:
		case 0x2e:
		case 0x3e:
		case 0x26:
		case 0x64:
		case 0x65:
		case 0x36:
		case 0xf0:
		case 0xf3:
		case 0xf2:
			++insn;
			continue;
		}
		break;
	}

	
	if (is_REX_prefix(insn))
		++insn;

	if (*insn == 0x0f) {
		
		++insn;
		need_modrm = test_bit(*insn,
				      (unsigned long *)twobyte_has_modrm);
	} else
		
		need_modrm = test_bit(*insn,
				      (unsigned long *)onebyte_has_modrm);

	if (need_modrm) {
		u8 modrm = *++insn;
		if ((modrm & 0xc7) == 0x05) {
			
			
			++insn;
			
			disp = (u8 *) p->addr + *((s32 *) insn) -
			       (u8 *) p->ainsn.insn;
			BUG_ON((s64) (s32) disp != disp); 
			*(s32 *)insn = (s32) disp;
		}
	}
#endif
}

static void __kprobes arch_copy_kprobe(struct kprobe *p)
{
	memcpy(p->ainsn.insn, p->addr, MAX_INSN_SIZE * sizeof(kprobe_opcode_t));

	fix_riprel(p);

	if (can_boost(p->addr))
		p->ainsn.boostable = 0;
	else
		p->ainsn.boostable = -1;

	p->opcode = *p->addr;
}

int __kprobes arch_prepare_kprobe(struct kprobe *p)
{
	
	p->ainsn.insn = get_insn_slot();
	if (!p->ainsn.insn)
		return -ENOMEM;
	arch_copy_kprobe(p);
	return 0;
}

void __kprobes arch_arm_kprobe(struct kprobe *p)
{
	text_poke(p->addr, ((unsigned char []){BREAKPOINT_INSTRUCTION}), 1);
}

void __kprobes arch_disarm_kprobe(struct kprobe *p)
{
	text_poke(p->addr, &p->opcode, 1);
}

void __kprobes arch_remove_kprobe(struct kprobe *p)
{
	if (p->ainsn.insn) {
		free_insn_slot(p->ainsn.insn, (p->ainsn.boostable == 1));
		p->ainsn.insn = NULL;
	}
}

static void __kprobes save_previous_kprobe(struct kprobe_ctlblk *kcb)
{
	kcb->prev_kprobe.kp = kprobe_running();
	kcb->prev_kprobe.status = kcb->kprobe_status;
	kcb->prev_kprobe.old_flags = kcb->kprobe_old_flags;
	kcb->prev_kprobe.saved_flags = kcb->kprobe_saved_flags;
}

static void __kprobes restore_previous_kprobe(struct kprobe_ctlblk *kcb)
{
	__get_cpu_var(current_kprobe) = kcb->prev_kprobe.kp;
	kcb->kprobe_status = kcb->prev_kprobe.status;
	kcb->kprobe_old_flags = kcb->prev_kprobe.old_flags;
	kcb->kprobe_saved_flags = kcb->prev_kprobe.saved_flags;
}

static void __kprobes set_current_kprobe(struct kprobe *p, struct pt_regs *regs,
				struct kprobe_ctlblk *kcb)
{
	__get_cpu_var(current_kprobe) = p;
	kcb->kprobe_saved_flags = kcb->kprobe_old_flags
		= (regs->flags & (X86_EFLAGS_TF | X86_EFLAGS_IF));
	if (is_IF_modifier(p->ainsn.insn))
		kcb->kprobe_saved_flags &= ~X86_EFLAGS_IF;
}

static void __kprobes clear_btf(void)
{
	if (test_thread_flag(TIF_DEBUGCTLMSR))
		update_debugctlmsr(0);
}

static void __kprobes restore_btf(void)
{
	if (test_thread_flag(TIF_DEBUGCTLMSR))
		update_debugctlmsr(current->thread.debugctlmsr);
}

static void __kprobes prepare_singlestep(struct kprobe *p, struct pt_regs *regs)
{
	clear_btf();
	regs->flags |= X86_EFLAGS_TF;
	regs->flags &= ~X86_EFLAGS_IF;
	
	if (p->opcode == BREAKPOINT_INSTRUCTION)
		regs->ip = (unsigned long)p->addr;
	else
		regs->ip = (unsigned long)p->ainsn.insn;
}

void __kprobes arch_prepare_kretprobe(struct kretprobe_instance *ri,
				      struct pt_regs *regs)
{
	unsigned long *sara = stack_addr(regs);

	ri->ret_addr = (kprobe_opcode_t *) *sara;

	
	*sara = (unsigned long) &kretprobe_trampoline;
}

static void __kprobes setup_singlestep(struct kprobe *p, struct pt_regs *regs,
				       struct kprobe_ctlblk *kcb)
{
#if !defined(CONFIG_PREEMPT) || defined(CONFIG_FREEZER)
	if (p->ainsn.boostable == 1 && !p->post_handler) {
		
		reset_current_kprobe();
		regs->ip = (unsigned long)p->ainsn.insn;
		preempt_enable_no_resched();
		return;
	}
#endif
	prepare_singlestep(p, regs);
	kcb->kprobe_status = KPROBE_HIT_SS;
}


static int __kprobes reenter_kprobe(struct kprobe *p, struct pt_regs *regs,
				    struct kprobe_ctlblk *kcb)
{
	switch (kcb->kprobe_status) {
	case KPROBE_HIT_SSDONE:
#ifdef CONFIG_X86_64
		
		arch_disarm_kprobe(p);
		regs->ip = (unsigned long)p->addr;
		reset_current_kprobe();
		preempt_enable_no_resched();
		break;
#endif
	case KPROBE_HIT_ACTIVE:
		save_previous_kprobe(kcb);
		set_current_kprobe(p, regs, kcb);
		kprobes_inc_nmissed_count(p);
		prepare_singlestep(p, regs);
		kcb->kprobe_status = KPROBE_REENTER;
		break;
	case KPROBE_HIT_SS:
		if (p == kprobe_running()) {
			regs->flags &= ~X86_EFLAGS_TF;
			regs->flags |= kcb->kprobe_saved_flags;
			return 0;
		} else {
			
		}
	default:
		
		WARN_ON(1);
		return 0;
	}

	return 1;
}


static int __kprobes kprobe_handler(struct pt_regs *regs)
{
	kprobe_opcode_t *addr;
	struct kprobe *p;
	struct kprobe_ctlblk *kcb;

	addr = (kprobe_opcode_t *)(regs->ip - sizeof(kprobe_opcode_t));
	if (*addr != BREAKPOINT_INSTRUCTION) {
		
		regs->ip = (unsigned long)addr;
		return 1;
	}

	
	preempt_disable();

	kcb = get_kprobe_ctlblk();
	p = get_kprobe(addr);

	if (p) {
		if (kprobe_running()) {
			if (reenter_kprobe(p, regs, kcb))
				return 1;
		} else {
			set_current_kprobe(p, regs, kcb);
			kcb->kprobe_status = KPROBE_HIT_ACTIVE;

			
			if (!p->pre_handler || !p->pre_handler(p, regs))
				setup_singlestep(p, regs, kcb);
			return 1;
		}
	} else if (kprobe_running()) {
		p = __get_cpu_var(current_kprobe);
		if (p->break_handler && p->break_handler(p, regs)) {
			setup_singlestep(p, regs, kcb);
			return 1;
		}
	} 

	preempt_enable_no_resched();
	return 0;
}


static void __used __kprobes kretprobe_trampoline_holder(void)
{
	asm volatile (
			".global kretprobe_trampoline\n"
			"kretprobe_trampoline: \n"
#ifdef CONFIG_X86_64
			
			"	pushq %rsp\n"
			"	pushfq\n"
			
			"	subq $24, %rsp\n"
			"	pushq %rdi\n"
			"	pushq %rsi\n"
			"	pushq %rdx\n"
			"	pushq %rcx\n"
			"	pushq %rax\n"
			"	pushq %r8\n"
			"	pushq %r9\n"
			"	pushq %r10\n"
			"	pushq %r11\n"
			"	pushq %rbx\n"
			"	pushq %rbp\n"
			"	pushq %r12\n"
			"	pushq %r13\n"
			"	pushq %r14\n"
			"	pushq %r15\n"
			"	movq %rsp, %rdi\n"
			"	call trampoline_handler\n"
			
			"	movq %rax, 152(%rsp)\n"
			"	popq %r15\n"
			"	popq %r14\n"
			"	popq %r13\n"
			"	popq %r12\n"
			"	popq %rbp\n"
			"	popq %rbx\n"
			"	popq %r11\n"
			"	popq %r10\n"
			"	popq %r9\n"
			"	popq %r8\n"
			"	popq %rax\n"
			"	popq %rcx\n"
			"	popq %rdx\n"
			"	popq %rsi\n"
			"	popq %rdi\n"
			
			"	addq $24, %rsp\n"
			"	popfq\n"
#else
			"	pushf\n"
			
			"	subl $16, %esp\n"
			"	pushl %fs\n"
			"	pushl %es\n"
			"	pushl %ds\n"
			"	pushl %eax\n"
			"	pushl %ebp\n"
			"	pushl %edi\n"
			"	pushl %esi\n"
			"	pushl %edx\n"
			"	pushl %ecx\n"
			"	pushl %ebx\n"
			"	movl %esp, %eax\n"
			"	call trampoline_handler\n"
			
			"	movl 56(%esp), %edx\n"
			"	movl %edx, 52(%esp)\n"
			
			"	movl %eax, 56(%esp)\n"
			"	popl %ebx\n"
			"	popl %ecx\n"
			"	popl %edx\n"
			"	popl %esi\n"
			"	popl %edi\n"
			"	popl %ebp\n"
			"	popl %eax\n"
			
			"	addl $24, %esp\n"
			"	popf\n"
#endif
			"	ret\n");
}


static __used __kprobes void *trampoline_handler(struct pt_regs *regs)
{
	struct kretprobe_instance *ri = NULL;
	struct hlist_head *head, empty_rp;
	struct hlist_node *node, *tmp;
	unsigned long flags, orig_ret_address = 0;
	unsigned long trampoline_address = (unsigned long)&kretprobe_trampoline;

	INIT_HLIST_HEAD(&empty_rp);
	kretprobe_hash_lock(current, &head, &flags);
	
#ifdef CONFIG_X86_64
	regs->cs = __KERNEL_CS;
#else
	regs->cs = __KERNEL_CS | get_kernel_rpl();
	regs->gs = 0;
#endif
	regs->ip = trampoline_address;
	regs->orig_ax = ~0UL;

	
	hlist_for_each_entry_safe(ri, node, tmp, head, hlist) {
		if (ri->task != current)
			
			continue;

		if (ri->rp && ri->rp->handler) {
			__get_cpu_var(current_kprobe) = &ri->rp->kp;
			get_kprobe_ctlblk()->kprobe_status = KPROBE_HIT_ACTIVE;
			ri->rp->handler(ri, regs);
			__get_cpu_var(current_kprobe) = NULL;
		}

		orig_ret_address = (unsigned long)ri->ret_addr;
		recycle_rp_inst(ri, &empty_rp);

		if (orig_ret_address != trampoline_address)
			
			break;
	}

	kretprobe_assert(ri, orig_ret_address, trampoline_address);

	kretprobe_hash_unlock(current, &flags);

	hlist_for_each_entry_safe(ri, node, tmp, &empty_rp, hlist) {
		hlist_del(&ri->hlist);
		kfree(ri);
	}
	return (void *)orig_ret_address;
}


static void __kprobes resume_execution(struct kprobe *p,
		struct pt_regs *regs, struct kprobe_ctlblk *kcb)
{
	unsigned long *tos = stack_addr(regs);
	unsigned long copy_ip = (unsigned long)p->ainsn.insn;
	unsigned long orig_ip = (unsigned long)p->addr;
	kprobe_opcode_t *insn = p->ainsn.insn;

	
	if (is_REX_prefix(insn))
		insn++;

	regs->flags &= ~X86_EFLAGS_TF;
	switch (*insn) {
	case 0x9c:	
		*tos &= ~(X86_EFLAGS_TF | X86_EFLAGS_IF);
		*tos |= kcb->kprobe_old_flags;
		break;
	case 0xc2:	
	case 0xc3:
	case 0xca:
	case 0xcb:
	case 0xcf:
	case 0xea:	
		
		p->ainsn.boostable = 1;
		goto no_change;
	case 0xe8:	
		*tos = orig_ip + (*tos - copy_ip);
		break;
#ifdef CONFIG_X86_32
	case 0x9a:	
		*tos = orig_ip + (*tos - copy_ip);
		goto no_change;
#endif
	case 0xff:
		if ((insn[1] & 0x30) == 0x10) {
			
			*tos = orig_ip + (*tos - copy_ip);
			goto no_change;
		} else if (((insn[1] & 0x31) == 0x20) ||
			   ((insn[1] & 0x31) == 0x21)) {
			
			p->ainsn.boostable = 1;
			goto no_change;
		}
	default:
		break;
	}

	if (p->ainsn.boostable == 0) {
		if ((regs->ip > copy_ip) &&
		    (regs->ip - copy_ip) + 5 < MAX_INSN_SIZE) {
			
			set_jmp_op((void *)regs->ip,
				   (void *)orig_ip + (regs->ip - copy_ip));
			p->ainsn.boostable = 1;
		} else {
			p->ainsn.boostable = -1;
		}
	}

	regs->ip += orig_ip - copy_ip;

no_change:
	restore_btf();
}


static int __kprobes post_kprobe_handler(struct pt_regs *regs)
{
	struct kprobe *cur = kprobe_running();
	struct kprobe_ctlblk *kcb = get_kprobe_ctlblk();

	if (!cur)
		return 0;

	resume_execution(cur, regs, kcb);
	regs->flags |= kcb->kprobe_saved_flags;

	if ((kcb->kprobe_status != KPROBE_REENTER) && cur->post_handler) {
		kcb->kprobe_status = KPROBE_HIT_SSDONE;
		cur->post_handler(cur, regs, 0);
	}

	
	if (kcb->kprobe_status == KPROBE_REENTER) {
		restore_previous_kprobe(kcb);
		goto out;
	}
	reset_current_kprobe();
out:
	preempt_enable_no_resched();

	
	if (regs->flags & X86_EFLAGS_TF)
		return 0;

	return 1;
}

int __kprobes kprobe_fault_handler(struct pt_regs *regs, int trapnr)
{
	struct kprobe *cur = kprobe_running();
	struct kprobe_ctlblk *kcb = get_kprobe_ctlblk();

	switch (kcb->kprobe_status) {
	case KPROBE_HIT_SS:
	case KPROBE_REENTER:
		
		regs->ip = (unsigned long)cur->addr;
		regs->flags |= kcb->kprobe_old_flags;
		if (kcb->kprobe_status == KPROBE_REENTER)
			restore_previous_kprobe(kcb);
		else
			reset_current_kprobe();
		preempt_enable_no_resched();
		break;
	case KPROBE_HIT_ACTIVE:
	case KPROBE_HIT_SSDONE:
		
		kprobes_inc_nmissed_count(cur);

		
		if (cur->fault_handler && cur->fault_handler(cur, regs, trapnr))
			return 1;

		
		if (fixup_exception(regs))
			return 1;

		
		break;
	default:
		break;
	}
	return 0;
}


int __kprobes kprobe_exceptions_notify(struct notifier_block *self,
				       unsigned long val, void *data)
{
	struct die_args *args = data;
	int ret = NOTIFY_DONE;

	if (args->regs && user_mode_vm(args->regs))
		return ret;

	switch (val) {
	case DIE_INT3:
		if (kprobe_handler(args->regs))
			ret = NOTIFY_STOP;
		break;
	case DIE_DEBUG:
		if (post_kprobe_handler(args->regs))
			ret = NOTIFY_STOP;
		break;
	case DIE_GPF:
		
		if (!preemptible() && kprobe_running() &&
		    kprobe_fault_handler(args->regs, args->trapnr))
			ret = NOTIFY_STOP;
		break;
	default:
		break;
	}
	return ret;
}

int __kprobes setjmp_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct jprobe *jp = container_of(p, struct jprobe, kp);
	unsigned long addr;
	struct kprobe_ctlblk *kcb = get_kprobe_ctlblk();

	kcb->jprobe_saved_regs = *regs;
	kcb->jprobe_saved_sp = stack_addr(regs);
	addr = (unsigned long)(kcb->jprobe_saved_sp);

	
	memcpy(kcb->jprobes_stack, (kprobe_opcode_t *)addr,
	       MIN_STACK_SIZE(addr));
	regs->flags &= ~X86_EFLAGS_IF;
	trace_hardirqs_off();
	regs->ip = (unsigned long)(jp->entry);
	return 1;
}

void __kprobes jprobe_return(void)
{
	struct kprobe_ctlblk *kcb = get_kprobe_ctlblk();

	asm volatile (
#ifdef CONFIG_X86_64
			"       xchg   %%rbx,%%rsp	\n"
#else
			"       xchgl   %%ebx,%%esp	\n"
#endif
			"       int3			\n"
			"       .globl jprobe_return_end\n"
			"       jprobe_return_end:	\n"
			"       nop			\n"::"b"
			(kcb->jprobe_saved_sp):"memory");
}

int __kprobes longjmp_break_handler(struct kprobe *p, struct pt_regs *regs)
{
	struct kprobe_ctlblk *kcb = get_kprobe_ctlblk();
	u8 *addr = (u8 *) (regs->ip - 1);
	struct jprobe *jp = container_of(p, struct jprobe, kp);

	if ((addr > (u8 *) jprobe_return) &&
	    (addr < (u8 *) jprobe_return_end)) {
		if (stack_addr(regs) != kcb->jprobe_saved_sp) {
			struct pt_regs *saved_regs = &kcb->jprobe_saved_regs;
			printk(KERN_ERR
			       "current sp %p does not match saved sp %p\n",
			       stack_addr(regs), kcb->jprobe_saved_sp);
			printk(KERN_ERR "Saved registers for jprobe %p\n", jp);
			show_registers(saved_regs);
			printk(KERN_ERR "Current registers\n");
			show_registers(regs);
			BUG();
		}
		*regs = kcb->jprobe_saved_regs;
		memcpy((kprobe_opcode_t *)(kcb->jprobe_saved_sp),
		       kcb->jprobes_stack,
		       MIN_STACK_SIZE(kcb->jprobe_saved_sp));
		preempt_enable_no_resched();
		return 1;
	}
	return 0;
}

int __init arch_init_kprobes(void)
{
	return 0;
}

int __kprobes arch_trampoline_kprobe(struct kprobe *p)
{
	return 0;
}
