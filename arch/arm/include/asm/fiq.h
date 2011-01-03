

#ifndef __ASM_FIQ_H
#define __ASM_FIQ_H

#include <asm/ptrace.h>

struct fiq_handler {
	struct fiq_handler *next;
	
	const char *name;
	
	int (*fiq_op)(void *, int relinquish);
	
	void *dev_id;
};

extern int claim_fiq(struct fiq_handler *f);
extern void release_fiq(struct fiq_handler *f);
extern void set_fiq_handler(void *start, unsigned int length);
extern void set_fiq_regs(struct pt_regs *regs);
extern void get_fiq_regs(struct pt_regs *regs);
extern void enable_fiq(int fiq);
extern void disable_fiq(int fiq);

#endif
