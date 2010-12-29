

#ifndef __ASM_ARCH_MSM_FIQ_H
#define __ASM_ARCH_MSM_FIQ_H


void msm_fiq_select(int number);
void msm_fiq_unselect(int number);


void msm_fiq_enable(int number);
void msm_fiq_disable(int number);


int msm_fiq_set_handler(void (*func)(void *data, void *regs), void *data);


void msm_trigger_irq(int number);

#endif
