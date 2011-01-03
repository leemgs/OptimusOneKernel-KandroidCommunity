



#include <linux/kernel.h>
#include <linux/kprobes.h>

#define sign_extend(x, signbit) ((x) | (0 - ((x) & (1 << (signbit)))))

#define branch_displacement(insn) sign_extend(((insn) & 0xffffff) << 2, 25)

#define PSR_fs	(PSR_f|PSR_s)

#define KPROBE_RETURN_INSTRUCTION	0xe1a0f00e	
#define SET_R0_TRUE_INSTRUCTION		0xe3a00001	

#define	truecc_insn(insn)	(((insn) & 0xf0000000) | \
				 (SET_R0_TRUE_INSTRUCTION & 0x0fffffff))

typedef long (insn_0arg_fn_t)(void);
typedef long (insn_1arg_fn_t)(long);
typedef long (insn_2arg_fn_t)(long, long);
typedef long (insn_3arg_fn_t)(long, long, long);
typedef long (insn_4arg_fn_t)(long, long, long, long);
typedef long long (insn_llret_0arg_fn_t)(void);
typedef long long (insn_llret_3arg_fn_t)(long, long, long);
typedef long long (insn_llret_4arg_fn_t)(long, long, long, long);

union reg_pair {
	long long	dr;
#ifdef __LITTLE_ENDIAN
	struct { long	r0, r1; };
#else
	struct { long	r1, r0; };
#endif
};



static int str_pc_offset;

static void __init find_str_pc_offset(void)
{
	int addr, scratch, ret;

	__asm__ (
		"sub	%[ret], pc, #4		\n\t"
		"str	pc, %[addr]		\n\t"
		"ldr	%[scr], %[addr]		\n\t"
		"sub	%[ret], %[scr], %[ret]	\n\t"
		: [ret] "=r" (ret), [scr] "=r" (scratch), [addr] "+m" (addr));

	str_pc_offset = ret;
}



static inline long __kprobes
insnslot_0arg_rflags(long cpsr, insn_0arg_fn_t *fn)
{
	register long ret asm("r0");

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[cpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		: "=r" (ret)
		: [cpsr] "r" (cpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	return ret;
}

static inline long long __kprobes
insnslot_llret_0arg_rflags(long cpsr, insn_llret_0arg_fn_t *fn)
{
	register long ret0 asm("r0");
	register long ret1 asm("r1");
	union reg_pair fnr;

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[cpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		: "=r" (ret0), "=r" (ret1)
		: [cpsr] "r" (cpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	fnr.r0 = ret0;
	fnr.r1 = ret1;
	return fnr.dr;
}

static inline long __kprobes
insnslot_1arg_rflags(long r0, long cpsr, insn_1arg_fn_t *fn)
{
	register long rr0 asm("r0") = r0;
	register long ret asm("r0");

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[cpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		: "=r" (ret)
		: "0" (rr0), [cpsr] "r" (cpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	return ret;
}

static inline long __kprobes
insnslot_2arg_rflags(long r0, long r1, long cpsr, insn_2arg_fn_t *fn)
{
	register long rr0 asm("r0") = r0;
	register long rr1 asm("r1") = r1;
	register long ret asm("r0");

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[cpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		: "=r" (ret)
		: "0" (rr0), "r" (rr1),
		  [cpsr] "r" (cpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	return ret;
}

static inline long __kprobes
insnslot_3arg_rflags(long r0, long r1, long r2, long cpsr, insn_3arg_fn_t *fn)
{
	register long rr0 asm("r0") = r0;
	register long rr1 asm("r1") = r1;
	register long rr2 asm("r2") = r2;
	register long ret asm("r0");

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[cpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		: "=r" (ret)
		: "0" (rr0), "r" (rr1), "r" (rr2),
		  [cpsr] "r" (cpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	return ret;
}

static inline long long __kprobes
insnslot_llret_3arg_rflags(long r0, long r1, long r2, long cpsr,
			   insn_llret_3arg_fn_t *fn)
{
	register long rr0 asm("r0") = r0;
	register long rr1 asm("r1") = r1;
	register long rr2 asm("r2") = r2;
	register long ret0 asm("r0");
	register long ret1 asm("r1");
	union reg_pair fnr;

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[cpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		: "=r" (ret0), "=r" (ret1)
		: "0" (rr0), "r" (rr1), "r" (rr2),
		  [cpsr] "r" (cpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	fnr.r0 = ret0;
	fnr.r1 = ret1;
	return fnr.dr;
}

static inline long __kprobes
insnslot_4arg_rflags(long r0, long r1, long r2, long r3, long cpsr,
		     insn_4arg_fn_t *fn)
{
	register long rr0 asm("r0") = r0;
	register long rr1 asm("r1") = r1;
	register long rr2 asm("r2") = r2;
	register long rr3 asm("r3") = r3;
	register long ret asm("r0");

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[cpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		: "=r" (ret)
		: "0" (rr0), "r" (rr1), "r" (rr2), "r" (rr3),
		  [cpsr] "r" (cpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	return ret;
}

static inline long __kprobes
insnslot_1arg_rwflags(long r0, long *cpsr, insn_1arg_fn_t *fn)
{
	register long rr0 asm("r0") = r0;
	register long ret asm("r0");
	long oldcpsr = *cpsr;
	long newcpsr;

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[oldcpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		"mrs	%[newcpsr], cpsr	\n\t"
		: "=r" (ret), [newcpsr] "=r" (newcpsr)
		: "0" (rr0), [oldcpsr] "r" (oldcpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	*cpsr = (oldcpsr & ~PSR_fs) | (newcpsr & PSR_fs);
	return ret;
}

static inline long __kprobes
insnslot_2arg_rwflags(long r0, long r1, long *cpsr, insn_2arg_fn_t *fn)
{
	register long rr0 asm("r0") = r0;
	register long rr1 asm("r1") = r1;
	register long ret asm("r0");
	long oldcpsr = *cpsr;
	long newcpsr;

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[oldcpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		"mrs	%[newcpsr], cpsr	\n\t"
		: "=r" (ret), [newcpsr] "=r" (newcpsr)
		: "0" (rr0), "r" (rr1), [oldcpsr] "r" (oldcpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	*cpsr = (oldcpsr & ~PSR_fs) | (newcpsr & PSR_fs);
	return ret;
}

static inline long __kprobes
insnslot_3arg_rwflags(long r0, long r1, long r2, long *cpsr,
		      insn_3arg_fn_t *fn)
{
	register long rr0 asm("r0") = r0;
	register long rr1 asm("r1") = r1;
	register long rr2 asm("r2") = r2;
	register long ret asm("r0");
	long oldcpsr = *cpsr;
	long newcpsr;

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[oldcpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		"mrs	%[newcpsr], cpsr	\n\t"
		: "=r" (ret), [newcpsr] "=r" (newcpsr)
		: "0" (rr0), "r" (rr1), "r" (rr2),
		  [oldcpsr] "r" (oldcpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	*cpsr = (oldcpsr & ~PSR_fs) | (newcpsr & PSR_fs);
	return ret;
}

static inline long __kprobes
insnslot_4arg_rwflags(long r0, long r1, long r2, long r3, long *cpsr,
		      insn_4arg_fn_t *fn)
{
	register long rr0 asm("r0") = r0;
	register long rr1 asm("r1") = r1;
	register long rr2 asm("r2") = r2;
	register long rr3 asm("r3") = r3;
	register long ret asm("r0");
	long oldcpsr = *cpsr;
	long newcpsr;

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[oldcpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		"mrs	%[newcpsr], cpsr	\n\t"
		: "=r" (ret), [newcpsr] "=r" (newcpsr)
		: "0" (rr0), "r" (rr1), "r" (rr2), "r" (rr3),
		  [oldcpsr] "r" (oldcpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	*cpsr = (oldcpsr & ~PSR_fs) | (newcpsr & PSR_fs);
	return ret;
}

static inline long long __kprobes
insnslot_llret_4arg_rwflags(long r0, long r1, long r2, long r3, long *cpsr,
			    insn_llret_4arg_fn_t *fn)
{
	register long rr0 asm("r0") = r0;
	register long rr1 asm("r1") = r1;
	register long rr2 asm("r2") = r2;
	register long rr3 asm("r3") = r3;
	register long ret0 asm("r0");
	register long ret1 asm("r1");
	long oldcpsr = *cpsr;
	long newcpsr;
	union reg_pair fnr;

	__asm__ __volatile__ (
		"msr	cpsr_fs, %[oldcpsr]	\n\t"
		"mov	lr, pc			\n\t"
		"mov	pc, %[fn]		\n\t"
		"mrs	%[newcpsr], cpsr	\n\t"
		: "=r" (ret0), "=r" (ret1), [newcpsr] "=r" (newcpsr)
		: "0" (rr0), "r" (rr1), "r" (rr2), "r" (rr3),
		  [oldcpsr] "r" (oldcpsr), [fn] "r" (fn)
		: "lr", "cc"
	);
	*cpsr = (oldcpsr & ~PSR_fs) | (newcpsr & PSR_fs);
	fnr.r0 = ret0;
	fnr.r1 = ret1;
	return fnr.dr;
}



static void __kprobes simulate_bbl(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	long iaddr = (long)p->addr;
	int disp  = branch_displacement(insn);

	if (!insnslot_1arg_rflags(0, regs->ARM_cpsr, i_fn))
		return;

	if (insn & (1 << 24))
		regs->ARM_lr = iaddr + 4;

	regs->ARM_pc = iaddr + 8 + disp;
}

static void __kprobes simulate_blx1(struct kprobe *p, struct pt_regs *regs)
{
	kprobe_opcode_t insn = p->opcode;
	long iaddr = (long)p->addr;
	int disp = branch_displacement(insn);

	regs->ARM_lr = iaddr + 4;
	regs->ARM_pc = iaddr + 8 + disp + ((insn >> 23) & 0x2);
	regs->ARM_cpsr |= PSR_T_BIT;
}

static void __kprobes simulate_blx2bx(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rm = insn & 0xf;
	long rmv = regs->uregs[rm];

	if (!insnslot_1arg_rflags(0, regs->ARM_cpsr, i_fn))
		return;

	if (insn & (1 << 5))
		regs->ARM_lr = (long)p->addr + 4;

	regs->ARM_pc = rmv & ~0x1;
	regs->ARM_cpsr &= ~PSR_T_BIT;
	if (rmv & 0x1)
		regs->ARM_cpsr |= PSR_T_BIT;
}

static void __kprobes simulate_ldm1stm1(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rn = (insn >> 16) & 0xf;
	int lbit = insn & (1 << 20);
	int wbit = insn & (1 << 21);
	int ubit = insn & (1 << 23);
	int pbit = insn & (1 << 24);
	long *addr = (long *)regs->uregs[rn];
	int reg_bit_vector;
	int reg_count;

	if (!insnslot_1arg_rflags(0, regs->ARM_cpsr, i_fn))
		return;

	reg_count = 0;
	reg_bit_vector = insn & 0xffff;
	while (reg_bit_vector) {
		reg_bit_vector &= (reg_bit_vector - 1);
		++reg_count;
	}

	if (!ubit)
		addr -= reg_count;
	addr += (!pbit == !ubit);

	reg_bit_vector = insn & 0xffff;
	while (reg_bit_vector) {
		int reg = __ffs(reg_bit_vector);
		reg_bit_vector &= (reg_bit_vector - 1);
		if (lbit)
			regs->uregs[reg] = *addr++;
		else
			*addr++ = regs->uregs[reg];
	}

	if (wbit) {
		if (!ubit)
			addr -= reg_count;
		addr -= (!pbit == !ubit);
		regs->uregs[rn] = (long)addr;
	}
}

static void __kprobes simulate_stm1_pc(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];

	if (!insnslot_1arg_rflags(0, regs->ARM_cpsr, i_fn))
		return;

	regs->ARM_pc = (long)p->addr + str_pc_offset;
	simulate_ldm1stm1(p, regs);
	regs->ARM_pc = (long)p->addr + 4;
}

static void __kprobes simulate_mov_ipsp(struct kprobe *p, struct pt_regs *regs)
{
	regs->uregs[12] = regs->uregs[13];
}

static void __kprobes emulate_ldcstc(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rn = (insn >> 16) & 0xf;
	long rnv = regs->uregs[rn];

	
	regs->uregs[rn] = insnslot_1arg_rflags(rnv, regs->ARM_cpsr, i_fn);
}

static void __kprobes emulate_ldrd(struct kprobe *p, struct pt_regs *regs)
{
	insn_2arg_fn_t *i_fn = (insn_2arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;
	int rm = insn & 0xf;  

	
	__asm__ __volatile__ (
		"ldr	r0, %[rn]	\n\t"
		"ldr	r1, %[rm]	\n\t"
		"msr	cpsr_fs, %[cpsr]\n\t"
		"mov	lr, pc		\n\t"
		"mov	pc, %[i_fn]	\n\t"
		"str	r0, %[rn]	\n\t"	
		"str	r2, %[rd0]	\n\t"
		"str	r3, %[rd1]	\n\t"
		: [rn]  "+m" (regs->uregs[rn]),
		  [rd0] "=m" (regs->uregs[rd]),
		  [rd1] "=m" (regs->uregs[rd+1])
		: [rm]   "m" (regs->uregs[rm]),
		  [cpsr] "r" (regs->ARM_cpsr),
		  [i_fn] "r" (i_fn)
		: "r0", "r1", "r2", "r3", "lr", "cc"
	);
}

static void __kprobes emulate_strd(struct kprobe *p, struct pt_regs *regs)
{
	insn_4arg_fn_t *i_fn = (insn_4arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;
	int rm  = insn & 0xf;
	long rnv = regs->uregs[rn];
	long rmv = regs->uregs[rm];  

	regs->uregs[rn] = insnslot_4arg_rflags(rnv, rmv, regs->uregs[rd],
					       regs->uregs[rd+1],
					       regs->ARM_cpsr, i_fn);
}

static void __kprobes emulate_ldr(struct kprobe *p, struct pt_regs *regs)
{
	insn_llret_3arg_fn_t *i_fn = (insn_llret_3arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	union reg_pair fnr;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;
	int rm = insn & 0xf;
	long rdv;
	long rnv  = regs->uregs[rn];
	long rmv  = regs->uregs[rm]; 
	long cpsr = regs->ARM_cpsr;

	fnr.dr = insnslot_llret_3arg_rflags(rnv, 0, rmv, cpsr, i_fn);
	regs->uregs[rn] = fnr.r0;  
	rdv = fnr.r1;

	if (rd == 15) {
#if __LINUX_ARM_ARCH__ >= 5
		cpsr &= ~PSR_T_BIT;
		if (rdv & 0x1)
			cpsr |= PSR_T_BIT;
		regs->ARM_cpsr = cpsr;
		rdv &= ~0x1;
#else
		rdv &= ~0x2;
#endif
	}
	regs->uregs[rd] = rdv;
}

static void __kprobes emulate_str(struct kprobe *p, struct pt_regs *regs)
{
	insn_3arg_fn_t *i_fn = (insn_3arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	long iaddr = (long)p->addr;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;
	int rm = insn & 0xf;
	long rdv = (rd == 15) ? iaddr + str_pc_offset : regs->uregs[rd];
	long rnv = (rn == 15) ? iaddr +  8 : regs->uregs[rn];
	long rmv = regs->uregs[rm];  

	
	regs->uregs[rn] =
		insnslot_3arg_rflags(rnv, rdv, rmv, regs->ARM_cpsr, i_fn);
}

static void __kprobes emulate_mrrc(struct kprobe *p, struct pt_regs *regs)
{
	insn_llret_0arg_fn_t *i_fn = (insn_llret_0arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	union reg_pair fnr;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;

	fnr.dr = insnslot_llret_0arg_rflags(regs->ARM_cpsr, i_fn);
	regs->uregs[rn] = fnr.r0;
	regs->uregs[rd] = fnr.r1;
}

static void __kprobes emulate_mcrr(struct kprobe *p, struct pt_regs *regs)
{
	insn_2arg_fn_t *i_fn = (insn_2arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;
	long rnv = regs->uregs[rn];
	long rdv = regs->uregs[rd];

	insnslot_2arg_rflags(rnv, rdv, regs->ARM_cpsr, i_fn);
}

static void __kprobes emulate_sat(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 12) & 0xf;
	int rm = insn & 0xf;
	long rmv = regs->uregs[rm];

	
	regs->uregs[rd] = insnslot_1arg_rwflags(rmv, &regs->ARM_cpsr, i_fn);
}

static void __kprobes emulate_sel(struct kprobe *p, struct pt_regs *regs)
{
	insn_2arg_fn_t *i_fn = (insn_2arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;
	int rm = insn & 0xf;
	long rnv = regs->uregs[rn];
	long rmv = regs->uregs[rm];

	
	regs->uregs[rd] = insnslot_2arg_rflags(rnv, rmv, regs->ARM_cpsr, i_fn);
}

static void __kprobes emulate_none(struct kprobe *p, struct pt_regs *regs)
{
	insn_0arg_fn_t *i_fn = (insn_0arg_fn_t *)&p->ainsn.insn[0];

	insnslot_0arg_rflags(regs->ARM_cpsr, i_fn);
}

static void __kprobes emulate_rd12(struct kprobe *p, struct pt_regs *regs)
{
	insn_0arg_fn_t *i_fn = (insn_0arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 12) & 0xf;

	regs->uregs[rd] = insnslot_0arg_rflags(regs->ARM_cpsr, i_fn);
}

static void __kprobes emulate_ird12(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int ird = (insn >> 12) & 0xf;

	insnslot_1arg_rflags(regs->uregs[ird], regs->ARM_cpsr, i_fn);
}

static void __kprobes emulate_rn16(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rn = (insn >> 16) & 0xf;
	long rnv = regs->uregs[rn];

	insnslot_1arg_rflags(rnv, regs->ARM_cpsr, i_fn);
}

static void __kprobes emulate_rd12rm0(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 12) & 0xf;
	int rm = insn & 0xf;
	long rmv = regs->uregs[rm];

	regs->uregs[rd] = insnslot_1arg_rflags(rmv, regs->ARM_cpsr, i_fn);
}

static void __kprobes
emulate_rd12rn16rm0_rwflags(struct kprobe *p, struct pt_regs *regs)
{
	insn_2arg_fn_t *i_fn = (insn_2arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;
	int rm = insn & 0xf;
	long rnv = regs->uregs[rn];
	long rmv = regs->uregs[rm];

	regs->uregs[rd] =
		insnslot_2arg_rwflags(rnv, rmv, &regs->ARM_cpsr, i_fn);
}

static void __kprobes
emulate_rd16rn12rs8rm0_rwflags(struct kprobe *p, struct pt_regs *regs)
{
	insn_3arg_fn_t *i_fn = (insn_3arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 16) & 0xf;
	int rn = (insn >> 12) & 0xf;
	int rs = (insn >> 8) & 0xf;
	int rm = insn & 0xf;
	long rnv = regs->uregs[rn];
	long rsv = regs->uregs[rs];
	long rmv = regs->uregs[rm];

	regs->uregs[rd] =
		insnslot_3arg_rwflags(rnv, rsv, rmv, &regs->ARM_cpsr, i_fn);
}

static void __kprobes
emulate_rd16rs8rm0_rwflags(struct kprobe *p, struct pt_regs *regs)
{
	insn_2arg_fn_t *i_fn = (insn_2arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 16) & 0xf;
	int rs = (insn >> 8) & 0xf;
	int rm = insn & 0xf;
	long rsv = regs->uregs[rs];
	long rmv = regs->uregs[rm];

	regs->uregs[rd] =
		insnslot_2arg_rwflags(rsv, rmv, &regs->ARM_cpsr, i_fn);
}

static void __kprobes
emulate_rdhi16rdlo12rs8rm0_rwflags(struct kprobe *p, struct pt_regs *regs)
{
	insn_llret_4arg_fn_t *i_fn = (insn_llret_4arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	union reg_pair fnr;
	int rdhi = (insn >> 16) & 0xf;
	int rdlo = (insn >> 12) & 0xf;
	int rs   = (insn >> 8) & 0xf;
	int rm   = insn & 0xf;
	long rsv = regs->uregs[rs];
	long rmv = regs->uregs[rm];

	fnr.dr = insnslot_llret_4arg_rwflags(regs->uregs[rdhi],
					     regs->uregs[rdlo], rsv, rmv,
					     &regs->ARM_cpsr, i_fn);
	regs->uregs[rdhi] = fnr.r0;
	regs->uregs[rdlo] = fnr.r1;
}

static void __kprobes
emulate_alu_imm_rflags(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;
	long rnv = (rn == 15) ? (long)p->addr + 8 : regs->uregs[rn];

	regs->uregs[rd] = insnslot_1arg_rflags(rnv, regs->ARM_cpsr, i_fn);
}

static void __kprobes
emulate_alu_imm_rwflags(struct kprobe *p, struct pt_regs *regs)
{
	insn_1arg_fn_t *i_fn = (insn_1arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;
	long rnv = (rn == 15) ? (long)p->addr + 8 : regs->uregs[rn];

	regs->uregs[rd] = insnslot_1arg_rwflags(rnv, &regs->ARM_cpsr, i_fn);
}

static void __kprobes
emulate_alu_rflags(struct kprobe *p, struct pt_regs *regs)
{
	insn_3arg_fn_t *i_fn = (insn_3arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	long ppc = (long)p->addr + 8;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;	
	int rs = (insn >> 8) & 0xf;	
	int rm = insn & 0xf;
	long rnv = (rn == 15) ? ppc : regs->uregs[rn];
	long rmv = (rm == 15) ? ppc : regs->uregs[rm];
	long rsv = regs->uregs[rs];

	regs->uregs[rd] =
		insnslot_3arg_rflags(rnv, rmv, rsv, regs->ARM_cpsr, i_fn);
}

static void __kprobes
emulate_alu_rwflags(struct kprobe *p, struct pt_regs *regs)
{
	insn_3arg_fn_t *i_fn = (insn_3arg_fn_t *)&p->ainsn.insn[0];
	kprobe_opcode_t insn = p->opcode;
	long ppc = (long)p->addr + 8;
	int rd = (insn >> 12) & 0xf;
	int rn = (insn >> 16) & 0xf;	
	int rs = (insn >> 8) & 0xf;	
	int rm = insn & 0xf;
	long rnv = (rn == 15) ? ppc : regs->uregs[rn];
	long rmv = (rm == 15) ? ppc : regs->uregs[rm];
	long rsv = regs->uregs[rs];

	regs->uregs[rd] =
		insnslot_3arg_rwflags(rnv, rmv, rsv, &regs->ARM_cpsr, i_fn);
}

static enum kprobe_insn __kprobes
prep_emulate_ldr_str(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	int ibit = (insn & (1 << 26)) ? 25 : 22;

	insn &= 0xfff00fff;
	insn |= 0x00001000;	
	if (insn & (1 << ibit)) {
		insn &= ~0xf;
		insn |= 2;	
	}
	asi->insn[0] = insn;
	asi->insn_handler = (insn & (1 << 20)) ? emulate_ldr : emulate_str;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
prep_emulate_rd12rm0(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	insn &= 0xffff0ff0;	
	asi->insn[0] = insn;
	asi->insn_handler = emulate_rd12rm0;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
prep_emulate_rd12(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	insn &= 0xffff0fff;	
	asi->insn[0] = insn;
	asi->insn_handler = emulate_rd12;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
prep_emulate_rd12rn16rm0_wflags(kprobe_opcode_t insn,
				struct arch_specific_insn *asi)
{
	insn &= 0xfff00ff0;	
	insn |= 0x00000001;	
	asi->insn[0] = insn;
	asi->insn_handler = emulate_rd12rn16rm0_rwflags;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
prep_emulate_rd16rs8rm0_wflags(kprobe_opcode_t insn,
			       struct arch_specific_insn *asi)
{
	insn &= 0xfff0f0f0;	
	insn |= 0x00000001;	
	asi->insn[0] = insn;
	asi->insn_handler = emulate_rd16rs8rm0_rwflags;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
prep_emulate_rd16rn12rs8rm0_wflags(kprobe_opcode_t insn,
				   struct arch_specific_insn *asi)
{
	insn &= 0xfff000f0;	
	insn |= 0x00000102;	
	asi->insn[0] = insn;
	asi->insn_handler = emulate_rd16rn12rs8rm0_rwflags;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
prep_emulate_rdhi16rdlo12rs8rm0_wflags(kprobe_opcode_t insn,
				       struct arch_specific_insn *asi)
{
	insn &= 0xfff000f0;	
	insn |= 0x00001203;	
	asi->insn[0] = insn;
	asi->insn_handler = emulate_rdhi16rdlo12rs8rm0_rwflags;
	return INSN_GOOD;
}



static enum kprobe_insn __kprobes
space_1111(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	
	
	if ((insn & 0xfff30020) == 0xf1020000 ||
	    (insn & 0xfe500f00) == 0xf8100a00 ||
	    (insn & 0xfe5f0f00) == 0xf84d0500)
		return INSN_REJECTED;

	
	if ((insn & 0xfd700000) == 0xf4500000) {
		insn &= 0xfff0ffff;	
		asi->insn[0] = insn;
		asi->insn_handler = emulate_rn16;
		return INSN_GOOD;
	}

	
	if ((insn & 0xfe000000) == 0xfa000000) {
		asi->insn_handler = simulate_blx1;
		return INSN_GOOD_NO_SLOT;
	}

	
	
	if ((insn & 0xffff00f0) == 0xf1010000 ||
	    (insn & 0xff000010) == 0xfe000000) {
		asi->insn[0] = insn;
		asi->insn_handler = emulate_none;
		return INSN_GOOD;
	}

	
	
	if ((insn & 0xffe00000) == 0xfc400000) {
		insn &= 0xfff00fff;	
		insn |= 0x00001000;	
		asi->insn[0] = insn;
		asi->insn_handler =
			(insn & (1 << 20)) ? emulate_mrrc : emulate_mcrr;
		return INSN_GOOD;
	}

	
	
	if ((insn & 0xfe000000) == 0xfc000000) {
		insn &= 0xfff0ffff;      
		asi->insn[0] = insn;
		asi->insn_handler = emulate_ldcstc;
		return INSN_GOOD;
	}

	
	
	insn &= 0xffff0fff;	
	asi->insn[0]      = insn;
	asi->insn_handler = (insn & (1 << 20)) ? emulate_rd12 : emulate_ird12;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
space_cccc_000x(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	if ((insn & 0x0f900010) == 0x01000000) {

		
		
		if ((insn & 0x0ff000f0) == 0x01200020 ||
		    (insn & 0x0fb000f0) == 0x01200000)
			return INSN_REJECTED;

		
		if ((insn & 0x0fb00010) == 0x01000000)
			return prep_emulate_rd12(insn, asi);

		
		if ((insn & 0x0ff00090) == 0x01400080)
			return prep_emulate_rdhi16rdlo12rs8rm0_wflags(insn, asi);

		
		
		if ((insn & 0x0ff000b0) == 0x012000a0 ||
		    (insn & 0x0ff00090) == 0x01600080)
			return prep_emulate_rd16rs8rm0_wflags(insn, asi);

		
		
		return prep_emulate_rd16rn12rs8rm0_wflags(insn, asi);

	}

	
	else if ((insn & 0x0f900090) == 0x01000010) {

		
		if ((insn & 0xfff000f0) == 0xe1200070)
			return INSN_REJECTED;

		
		
		if ((insn & 0x0ff000d0) == 0x01200010) {
			asi->insn[0] = truecc_insn(insn);
			asi->insn_handler = simulate_blx2bx;
			return INSN_GOOD;
		}

		
		if ((insn & 0x0ff000f0) == 0x01600010)
			return prep_emulate_rd12rm0(insn, asi);

		
		
		
		
		return prep_emulate_rd12rn16rm0_wflags(insn, asi);
	}

	
	else if ((insn & 0x0f000090) == 0x00000090) {

		
		
		
		
		
		
		
		
		
		
		
		
		
		if ((insn & 0x0fe000f0) == 0x00000090) {
		       return prep_emulate_rd16rs8rm0_wflags(insn, asi);
		} else if  ((insn & 0x0fe000f0) == 0x00200090) {
		       return prep_emulate_rd16rn12rs8rm0_wflags(insn, asi);
		} else {
		       return prep_emulate_rdhi16rdlo12rs8rm0_wflags(insn, asi);
		}
	}

	
	else if ((insn & 0x0e000090) == 0x00000090) {

		
		
		
		
		
		
		
		
		
		
		if ((insn & 0x0fb000f0) == 0x01000090) {
			
			return prep_emulate_rd12rn16rm0_wflags(insn, asi);
		} else if ((insn & 0x0e1000d0) == 0x00000d0) {
			
			insn &= 0xfff00fff;
			insn |= 0x00002000;	
			if (insn & (1 << 22)) {
				
				insn &= ~0xf;
				insn |= 1;	
			}
			asi->insn[0] = insn;
			asi->insn_handler =
				(insn & (1 << 5)) ? emulate_strd : emulate_ldrd;
			return INSN_GOOD;
		}

		return prep_emulate_ldr_str(insn, asi);
	}

	

	
	if ((insn & 0x0e10f000) == 0x0010f000)
		return INSN_REJECTED;

	
	if (insn == 0xe1a0c00d) {
		asi->insn_handler = simulate_mov_ipsp;
		return INSN_GOOD_NO_SLOT;
	}

	
	insn &= 0xfff00ff0;	
	insn |= 0x00000001;	
	if (insn & 0x010) {
		insn &= 0xfffff0ff;     
		insn |= 0x00000200;     
	}
	asi->insn[0] = insn;
	asi->insn_handler = (insn & (1 << 20)) ?  
				emulate_alu_rwflags : emulate_alu_rflags;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
space_cccc_001x(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	if ((insn & 0x0f900000) == 0x03200000 ||	
	    (insn & 0x0e10f000) == 0x0210f000)		
		return INSN_REJECTED;

	
	insn &= 0xfff00fff;	
	asi->insn[0] = insn;
	asi->insn_handler = (insn & (1 << 20)) ?  
			emulate_alu_imm_rwflags : emulate_alu_imm_rflags;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
space_cccc_0110__1(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	if ((insn & 0x0ff000f0) == 0x068000b0) {
		insn &= 0xfff00ff0;	
		insn |= 0x00000001;	
		asi->insn[0] = insn;
		asi->insn_handler = emulate_sel;
		return INSN_GOOD;
	}

	
	
	
	
	if ((insn & 0x0fa00030) == 0x06a00010 ||
	    (insn & 0x0fb000f0) == 0x06a00030) {
		insn &= 0xffff0ff0;	
		asi->insn[0] = insn;
		asi->insn_handler = emulate_sat;
		return INSN_GOOD;
	}

	
	
	
	if ((insn & 0x0ff00070) == 0x06b00030 ||
	    (insn & 0x0ff000f0) == 0x06f000b0)
		return prep_emulate_rd12rm0(insn, asi);

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	return prep_emulate_rd12rn16rm0_wflags(insn, asi);
}

static enum kprobe_insn __kprobes
space_cccc_0111__1(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	if ((insn & 0x0ff000f0) == 0x03f000f0)
		return INSN_REJECTED;

	
	
	if ((insn & 0x0ff000f0) == 0x07800010)
		 return prep_emulate_rd16rn12rs8rm0_wflags(insn, asi);

	
	
	if ((insn & 0x0ff00090) == 0x07400010)
		return prep_emulate_rdhi16rdlo12rs8rm0_wflags(insn, asi);

	
	
	
	
	if ((insn & 0x0ff00090) == 0x07000010 ||
	    (insn & 0x0ff000d0) == 0x07500010 ||
	    (insn & 0x0ff000d0) == 0x075000d0)
		return prep_emulate_rd16rn12rs8rm0_wflags(insn, asi);

	
	
	
	return prep_emulate_rd16rs8rm0_wflags(insn, asi);
}

static enum kprobe_insn __kprobes
space_cccc_01xx(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	
	
	
	
	
	
	
	return prep_emulate_ldr_str(insn, asi);
}

static enum kprobe_insn __kprobes
space_cccc_100x(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	
	if ((insn & 0x0e708000) == 0x85000000 ||
	    (insn & 0x0e508000) == 0x85010000)
		return INSN_REJECTED;

	
	
	asi->insn[0] = truecc_insn(insn);
	asi->insn_handler = ((insn & 0x108000) == 0x008000) ? 
				simulate_stm1_pc : simulate_ldm1stm1;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
space_cccc_101x(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	
	asi->insn[0] = truecc_insn(insn);
	asi->insn_handler = simulate_bbl;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
space_cccc_1100_010x(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	
	insn &= 0xfff00fff;
	insn |= 0x00001000;	
	asi->insn[0] = insn;
	asi->insn_handler = (insn & (1 << 20)) ? emulate_mrrc : emulate_mcrr;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
space_cccc_110x(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	
	insn &= 0xfff0ffff;	
	asi->insn[0] = insn;
	asi->insn_handler = emulate_ldcstc;
	return INSN_GOOD;
}

static enum kprobe_insn __kprobes
space_cccc_111x(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	
	
	if ((insn & 0xfff000f0) == 0xe1200070 ||
	    (insn & 0x0f000000) == 0x0f000000)
		return INSN_REJECTED;

	
	if ((insn & 0x0f000010) == 0x0e000000) {
		asi->insn[0] = insn;
		asi->insn_handler = emulate_none;
		return INSN_GOOD;
	}

	
	
	insn &= 0xffff0fff;	
	asi->insn[0] = insn;
	asi->insn_handler = (insn & (1 << 20)) ? emulate_rd12 : emulate_ird12;
	return INSN_GOOD;
}


enum kprobe_insn __kprobes
arm_kprobe_decode_insn(kprobe_opcode_t insn, struct arch_specific_insn *asi)
{
	asi->insn[1] = KPROBE_RETURN_INSTRUCTION;

	if ((insn & 0xf0000000) == 0xf0000000) {

		return space_1111(insn, asi);

	} else if ((insn & 0x0e000000) == 0x00000000) {

		return space_cccc_000x(insn, asi);

	} else if ((insn & 0x0e000000) == 0x02000000) {

		return space_cccc_001x(insn, asi);

	} else if ((insn & 0x0f000010) == 0x06000010) {

		return space_cccc_0110__1(insn, asi);

	} else if ((insn & 0x0f000010) == 0x07000010) {

		return space_cccc_0111__1(insn, asi);

	} else if ((insn & 0x0c000000) == 0x04000000) {

		return space_cccc_01xx(insn, asi);

	} else if ((insn & 0x0e000000) == 0x08000000) {

		return space_cccc_100x(insn, asi);

	} else if ((insn & 0x0e000000) == 0x0a000000) {

		return space_cccc_101x(insn, asi);

	} else if ((insn & 0x0fe00000) == 0x0c400000) {

		return space_cccc_1100_010x(insn, asi);

	} else if ((insn & 0x0e000000) == 0x0c400000) {

		return space_cccc_110x(insn, asi);

	}

	return space_cccc_111x(insn, asi);
}

void __init arm_kprobe_decode_init(void)
{
	find_str_pc_offset();
}



