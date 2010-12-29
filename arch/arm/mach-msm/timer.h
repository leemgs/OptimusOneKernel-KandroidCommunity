

#ifndef _ARCH_ARM_MACH_MSM_TIMER_H_
#define _ARCH_ARM_MACH_MSM_TIMER_H_

extern struct sys_timer msm_timer;

int64_t msm_timer_enter_idle(void);
void msm_timer_exit_idle(int low_power);
int64_t msm_timer_get_sclk_time(int64_t *period);
int msm_timer_init_time_sync(void (*timeout)(void));
#endif
