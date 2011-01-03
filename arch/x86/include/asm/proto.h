#ifndef _ASM_X86_PROTO_H
#define _ASM_X86_PROTO_H

#include <asm/ldt.h>



extern void early_idt_handler(void);

extern void system_call(void);
extern void syscall_init(void);

extern void ia32_syscall(void);
extern void ia32_cstar_target(void);
extern void ia32_sysenter_target(void);

extern void syscall32_cpu_init(void);

extern void check_efer(void);

extern int reboot_force;

long do_arch_prctl(struct task_struct *task, int code, unsigned long addr);


#define __round_mask(x,y) ((__typeof__(x))((y)-1))
#define round_up(x,y) ((((x)-1) | __round_mask(x,y))+1)
#define round_down(x,y) ((x) & ~__round_mask(x,y))

#endif 
