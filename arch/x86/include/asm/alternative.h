#ifndef _ASM_X86_ALTERNATIVE_H
#define _ASM_X86_ALTERNATIVE_H

#include <linux/types.h>
#include <linux/stddef.h>
#include <linux/stringify.h>
#include <asm/asm.h>



#ifdef CONFIG_SMP
#define LOCK_PREFIX \
		".section .smp_locks,\"a\"\n"	\
		_ASM_ALIGN "\n"			\
		_ASM_PTR "661f\n" 	\
		".previous\n"			\
		"661:\n\tlock; "

#else 
#define LOCK_PREFIX ""
#endif


#include <asm/cpufeature.h>

struct alt_instr {
	u8 *instr;		
	u8 *replacement;
	u8  cpuid;		
	u8  instrlen;		
	u8  replacementlen;	
	u8  pad1;
#ifdef CONFIG_X86_64
	u32 pad2;
#endif
};

extern void alternative_instructions(void);
extern void apply_alternatives(struct alt_instr *start, struct alt_instr *end);

struct module;

#ifdef CONFIG_SMP
extern void alternatives_smp_module_add(struct module *mod, char *name,
					void *locks, void *locks_end,
					void *text, void *text_end);
extern void alternatives_smp_module_del(struct module *mod);
extern void alternatives_smp_switch(int smp);
#else
static inline void alternatives_smp_module_add(struct module *mod, char *name,
					       void *locks, void *locks_end,
					       void *text, void *text_end) {}
static inline void alternatives_smp_module_del(struct module *mod) {}
static inline void alternatives_smp_switch(int smp) {}
#endif	


#define ALTERNATIVE(oldinstr, newinstr, feature)			\
									\
      "661:\n\t" oldinstr "\n662:\n"					\
      ".section .altinstructions,\"a\"\n"				\
      _ASM_ALIGN "\n"							\
      _ASM_PTR "661b\n"					\
      _ASM_PTR "663f\n"					\
      "	 .byte " __stringify(feature) "\n"		\
      "	 .byte 662b-661b\n"				\
      "	 .byte 664f-663f\n"				\
      ".previous\n"							\
      ".section .altinstr_replacement, \"ax\"\n"			\
      "663:\n\t" newinstr "\n664:\n"			\
      ".previous"


#define alternative(oldinstr, newinstr, feature)			\
	asm volatile (ALTERNATIVE(oldinstr, newinstr, feature) : : : "memory")


#define alternative_input(oldinstr, newinstr, feature, input...)	\
	asm volatile (ALTERNATIVE(oldinstr, newinstr, feature)		\
		: : "i" (0), ## input)


#define alternative_io(oldinstr, newinstr, feature, output, input...)	\
	asm volatile (ALTERNATIVE(oldinstr, newinstr, feature)		\
		: output : "i" (0), ## input)


#define ASM_OUTPUT2(a, b) a, b

struct paravirt_patch_site;
#ifdef CONFIG_PARAVIRT
void apply_paravirt(struct paravirt_patch_site *start,
		    struct paravirt_patch_site *end);
#else
static inline void apply_paravirt(struct paravirt_patch_site *start,
				  struct paravirt_patch_site *end)
{}
#define __parainstructions	NULL
#define __parainstructions_end	NULL
#endif


extern void *text_poke(void *addr, const void *opcode, size_t len);

#endif 
