#ifndef _ASM_X86_SYSTEM_H
#define _ASM_X86_SYSTEM_H

#include <asm/asm.h>
#include <asm/segment.h>
#include <asm/cpufeature.h>
#include <asm/cmpxchg.h>
#include <asm/nops.h>

#include <linux/kernel.h>
#include <linux/irqflags.h>


#ifdef CONFIG_IA32_EMULATION
# define AT_VECTOR_SIZE_ARCH 2
#else
# define AT_VECTOR_SIZE_ARCH 1
#endif

struct task_struct; 
struct task_struct *__switch_to(struct task_struct *prev,
				struct task_struct *next);
struct tss_struct;
void __switch_to_xtra(struct task_struct *prev_p, struct task_struct *next_p,
		      struct tss_struct *tss);

#ifdef CONFIG_X86_32

#ifdef CONFIG_CC_STACKPROTECTOR
#define __switch_canary							\
	"movl %P[task_canary](%[next]), %%ebx\n\t"			\
	"movl %%ebx, "__percpu_arg([stack_canary])"\n\t"
#define __switch_canary_oparam						\
	, [stack_canary] "=m" (per_cpu_var(stack_canary.canary))
#define __switch_canary_iparam						\
	, [task_canary] "i" (offsetof(struct task_struct, stack_canary))
#else	
#define __switch_canary
#define __switch_canary_oparam
#define __switch_canary_iparam
#endif	


#define switch_to(prev, next, last)					\
do {									\
									\
	unsigned long ebx, ecx, edx, esi, edi;				\
									\
	asm volatile("pushfl\n\t"			\
		     "pushl %%ebp\n\t"			\
		     "movl %%esp,%[prev_sp]\n\t"	 \
		     "movl %[next_sp],%%esp\n\t"	 \
		     "movl $1f,%[prev_ip]\n\t"		\
		     "pushl %[next_ip]\n\t"		\
		     __switch_canary					\
		     "jmp __switch_to\n"		\
		     "1:\t"						\
		     "popl %%ebp\n\t"			\
		     "popfl\n"				\
									\
		     				\
		     : [prev_sp] "=m" (prev->thread.sp),		\
		       [prev_ip] "=m" (prev->thread.ip),		\
		       "=a" (last),					\
									\
		       		\
		       "=b" (ebx), "=c" (ecx), "=d" (edx),		\
		       "=S" (esi), "=D" (edi)				\
		       							\
		       __switch_canary_oparam				\
									\
		       				\
		     : [next_sp]  "m" (next->thread.sp),		\
		       [next_ip]  "m" (next->thread.ip),		\
		       							\
		       	\
		       [prev]     "a" (prev),				\
		       [next]     "d" (next)				\
									\
		       __switch_canary_iparam				\
									\
		     : 			\
			"memory");					\
} while (0)


#define HAVE_DISABLE_HLT
#else
#define __SAVE(reg, offset) "movq %%" #reg ",(14-" #offset ")*8(%%rsp)\n\t"
#define __RESTORE(reg, offset) "movq (14-" #offset ")*8(%%rsp),%%" #reg "\n\t"


#define SAVE_CONTEXT    "pushf ; pushq %%rbp ; movq %%rsi,%%rbp\n\t"
#define RESTORE_CONTEXT "movq %%rbp,%%rsi ; popq %%rbp ; popf\t"

#define __EXTRA_CLOBBER  \
	, "rcx", "rbx", "rdx", "r8", "r9", "r10", "r11", \
	  "r12", "r13", "r14", "r15"

#ifdef CONFIG_CC_STACKPROTECTOR
#define __switch_canary							  \
	"movq %P[task_canary](%%rsi),%%r8\n\t"				  \
	"movq %%r8,"__percpu_arg([gs_canary])"\n\t"
#define __switch_canary_oparam						  \
	, [gs_canary] "=m" (per_cpu_var(irq_stack_union.stack_canary))
#define __switch_canary_iparam						  \
	, [task_canary] "i" (offsetof(struct task_struct, stack_canary))
#else	
#define __switch_canary
#define __switch_canary_oparam
#define __switch_canary_iparam
#endif	


#define switch_to(prev, next, last) \
	asm volatile(SAVE_CONTEXT					  \
	     "movq %%rsp,%P[threadrsp](%[prev])\n\t" 	  \
	     "movq %P[threadrsp](%[next]),%%rsp\n\t" 	  \
	     "call __switch_to\n\t"					  \
	     ".globl thread_return\n"					  \
	     "thread_return:\n\t"					  \
	     "movq "__percpu_arg([current_task])",%%rsi\n\t"		  \
	     __switch_canary						  \
	     "movq %P[thread_info](%%rsi),%%r8\n\t"			  \
	     "movq %%rax,%%rdi\n\t" 					  \
	     "testl  %[_tif_fork],%P[ti_flags](%%r8)\n\t"	  \
	     "jnz   ret_from_fork\n\t"					  \
	     RESTORE_CONTEXT						  \
	     : "=a" (last)					  	  \
	       __switch_canary_oparam					  \
	     : [next] "S" (next), [prev] "D" (prev),			  \
	       [threadrsp] "i" (offsetof(struct task_struct, thread.sp)), \
	       [ti_flags] "i" (offsetof(struct thread_info, flags)),	  \
	       [_tif_fork] "i" (_TIF_FORK),			  	  \
	       [thread_info] "i" (offsetof(struct task_struct, stack)),   \
	       [current_task] "m" (per_cpu_var(current_task))		  \
	       __switch_canary_iparam					  \
	     : "memory", "cc" __EXTRA_CLOBBER)
#endif

#ifdef __KERNEL__

extern void native_load_gs_index(unsigned);


#define loadsegment(seg, value)			\
	asm volatile("\n"			\
		     "1:\t"			\
		     "movl %k0,%%" #seg "\n"	\
		     "2:\n"			\
		     ".section .fixup,\"ax\"\n"	\
		     "3:\t"			\
		     "movl %k1, %%" #seg "\n\t"	\
		     "jmp 2b\n"			\
		     ".previous\n"		\
		     _ASM_EXTABLE(1b,3b)	\
		     : :"r" (value), "r" (0) : "memory")



#define savesegment(seg, value)				\
	asm("mov %%" #seg ",%0":"=r" (value) : : "memory")


#ifdef CONFIG_X86_32
#ifdef CONFIG_X86_32_LAZY_GS
#define get_user_gs(regs)	(u16)({unsigned long v; savesegment(gs, v); v;})
#define set_user_gs(regs, v)	loadsegment(gs, (unsigned long)(v))
#define task_user_gs(tsk)	((tsk)->thread.gs)
#define lazy_save_gs(v)		savesegment(gs, (v))
#define lazy_load_gs(v)		loadsegment(gs, (v))
#else	
#define get_user_gs(regs)	(u16)((regs)->gs)
#define set_user_gs(regs, v)	do { (regs)->gs = (v); } while (0)
#define task_user_gs(tsk)	(task_pt_regs(tsk)->gs)
#define lazy_save_gs(v)		do { } while (0)
#define lazy_load_gs(v)		do { } while (0)
#endif	
#endif	

static inline unsigned long get_limit(unsigned long segment)
{
	unsigned long __limit;
	asm("lsll %1,%0" : "=r" (__limit) : "r" (segment));
	return __limit + 1;
}

static inline void native_clts(void)
{
	asm volatile("clts");
}


static unsigned long __force_order;

static inline unsigned long native_read_cr0(void)
{
	unsigned long val;
	asm volatile("mov %%cr0,%0\n\t" : "=r" (val), "=m" (__force_order));
	return val;
}

static inline void native_write_cr0(unsigned long val)
{
	asm volatile("mov %0,%%cr0": : "r" (val), "m" (__force_order));
}

static inline unsigned long native_read_cr2(void)
{
	unsigned long val;
	asm volatile("mov %%cr2,%0\n\t" : "=r" (val), "=m" (__force_order));
	return val;
}

static inline void native_write_cr2(unsigned long val)
{
	asm volatile("mov %0,%%cr2": : "r" (val), "m" (__force_order));
}

static inline unsigned long native_read_cr3(void)
{
	unsigned long val;
	asm volatile("mov %%cr3,%0\n\t" : "=r" (val), "=m" (__force_order));
	return val;
}

static inline void native_write_cr3(unsigned long val)
{
	asm volatile("mov %0,%%cr3": : "r" (val), "m" (__force_order));
}

static inline unsigned long native_read_cr4(void)
{
	unsigned long val;
	asm volatile("mov %%cr4,%0\n\t" : "=r" (val), "=m" (__force_order));
	return val;
}

static inline unsigned long native_read_cr4_safe(void)
{
	unsigned long val;
	
#ifdef CONFIG_X86_32
	asm volatile("1: mov %%cr4, %0\n"
		     "2:\n"
		     _ASM_EXTABLE(1b, 2b)
		     : "=r" (val), "=m" (__force_order) : "0" (0));
#else
	val = native_read_cr4();
#endif
	return val;
}

static inline void native_write_cr4(unsigned long val)
{
	asm volatile("mov %0,%%cr4": : "r" (val), "m" (__force_order));
}

#ifdef CONFIG_X86_64
static inline unsigned long native_read_cr8(void)
{
	unsigned long cr8;
	asm volatile("movq %%cr8,%0" : "=r" (cr8));
	return cr8;
}

static inline void native_write_cr8(unsigned long val)
{
	asm volatile("movq %0,%%cr8" :: "r" (val) : "memory");
}
#endif

static inline void native_wbinvd(void)
{
	asm volatile("wbinvd": : :"memory");
}

#ifdef CONFIG_PARAVIRT
#include <asm/paravirt.h>
#else
#define read_cr0()	(native_read_cr0())
#define write_cr0(x)	(native_write_cr0(x))
#define read_cr2()	(native_read_cr2())
#define write_cr2(x)	(native_write_cr2(x))
#define read_cr3()	(native_read_cr3())
#define write_cr3(x)	(native_write_cr3(x))
#define read_cr4()	(native_read_cr4())
#define read_cr4_safe()	(native_read_cr4_safe())
#define write_cr4(x)	(native_write_cr4(x))
#define wbinvd()	(native_wbinvd())
#ifdef CONFIG_X86_64
#define read_cr8()	(native_read_cr8())
#define write_cr8(x)	(native_write_cr8(x))
#define load_gs_index   native_load_gs_index
#endif


#define clts()		(native_clts())

#endif

#define stts() write_cr0(read_cr0() | X86_CR0_TS)

#endif 

static inline void clflush(volatile void *__p)
{
	asm volatile("clflush %0" : "+m" (*(volatile char __force *)__p));
}

#define nop() asm volatile ("nop")

void disable_hlt(void);
void enable_hlt(void);

void cpu_idle_wait(void);

extern unsigned long arch_align_stack(unsigned long sp);
extern void free_init_pages(char *what, unsigned long begin, unsigned long end);

void default_idle(void);

void stop_this_cpu(void *dummy);


#ifdef CONFIG_X86_32

#define mb() alternative("lock; addl $0,0(%%esp)", "mfence", X86_FEATURE_XMM2)
#define rmb() alternative("lock; addl $0,0(%%esp)", "lfence", X86_FEATURE_XMM2)
#define wmb() alternative("lock; addl $0,0(%%esp)", "sfence", X86_FEATURE_XMM)
#else
#define mb() 	asm volatile("mfence":::"memory")
#define rmb()	asm volatile("lfence":::"memory")
#define wmb()	asm volatile("sfence" ::: "memory")
#endif



#define read_barrier_depends()	do { } while (0)

#ifdef CONFIG_SMP
#define smp_mb()	mb()
#ifdef CONFIG_X86_PPRO_FENCE
# define smp_rmb()	rmb()
#else
# define smp_rmb()	barrier()
#endif
#ifdef CONFIG_X86_OOSTORE
# define smp_wmb() 	wmb()
#else
# define smp_wmb()	barrier()
#endif
#define smp_read_barrier_depends()	read_barrier_depends()
#define set_mb(var, value) do { (void)xchg(&var, value); } while (0)
#else
#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()
#define smp_read_barrier_depends()	do { } while (0)
#define set_mb(var, value) do { var = value; barrier(); } while (0)
#endif


static inline void rdtsc_barrier(void)
{
	alternative(ASM_NOP3, "mfence", X86_FEATURE_MFENCE_RDTSC);
	alternative(ASM_NOP3, "lfence", X86_FEATURE_LFENCE_RDTSC);
}

#endif 
