#ifndef _ASM_ARM_FTRACE
#define _ASM_ARM_FTRACE

#ifdef CONFIG_FUNCTION_TRACER
#define MCOUNT_ADDR		((long)(mcount))
#define MCOUNT_INSN_SIZE	4 

#ifndef __ASSEMBLY__
extern void mcount(void);
extern void __gnu_mcount_nc(void);
#endif

#endif

#ifndef __ASSEMBLY__

#if defined(CONFIG_FRAME_POINTER) && !defined(CONFIG_ARM_UNWIND)


void *return_address(unsigned int);

#else

extern inline void *return_address(unsigned int level)
{
	return NULL;
}

#endif

#define HAVE_ARCH_CALLER_ADDR

#define CALLER_ADDR0 ((unsigned long)__builtin_return_address(0))
#define CALLER_ADDR1 ((unsigned long)return_address(1))
#define CALLER_ADDR2 ((unsigned long)return_address(2))
#define CALLER_ADDR3 ((unsigned long)return_address(3))
#define CALLER_ADDR4 ((unsigned long)return_address(4))
#define CALLER_ADDR5 ((unsigned long)return_address(5))
#define CALLER_ADDR6 ((unsigned long)return_address(6))

#endif 

#endif 
