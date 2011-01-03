#ifndef _ASM_X86_TRAMPOLINE_H
#define _ASM_X86_TRAMPOLINE_H

#ifndef __ASSEMBLY__

#ifdef CONFIG_X86_TRAMPOLINE

extern const unsigned char trampoline_data [];
extern const unsigned char trampoline_end  [];
extern unsigned char *trampoline_base;

extern unsigned long init_rsp;
extern unsigned long initial_code;
extern unsigned long initial_gs;

#define TRAMPOLINE_SIZE roundup(trampoline_end - trampoline_data, PAGE_SIZE)
#define TRAMPOLINE_BASE 0x6000

extern unsigned long setup_trampoline(void);
extern void __init reserve_trampoline_memory(void);
#else
static inline void reserve_trampoline_memory(void) {};
#endif 

#endif 

#endif 
