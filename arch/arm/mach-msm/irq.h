

#ifndef _ARCH_ARM_MACH_MSM_IRQ_H_
#define _ARCH_ARM_MACH_MSM_IRQ_H_

int msm_irq_idle_sleep_allowed(void);
unsigned int msm_irq_pending(void);
void msm_irq_enter_sleep1(bool arm9_wake, int from_idle, uint32_t *irq_mask);
int msm_irq_enter_sleep2(bool arm9_wake, int from_idle);
void msm_irq_exit_sleep1
	(uint32_t irq_mask, uint32_t wakeup_reason, uint32_t pending_irqs);
void msm_irq_exit_sleep2
	(uint32_t irq_mask, uint32_t wakeup_reason, uint32_t pending);
void msm_irq_exit_sleep3
	(uint32_t irq_mask, uint32_t wakeup_reason, uint32_t pending_irqs);

#endif
