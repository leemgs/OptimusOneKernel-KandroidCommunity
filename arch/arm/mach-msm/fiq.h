

#ifndef _ARCH_ARM_MACH_MSM_FIQ_H
#define _ARCH_ARM_MACH_MSM_FIQ_H

extern unsigned char fiq_glue, fiq_glue_end;
void fiq_glue_setup(void *func, void *data, void *sp);

#endif
