

#ifndef MSM_GEMINI_CORE_H
#define MSM_GEMINI_CORE_H

#include <linux/interrupt.h>
#include "msm_gemini_hw.h"

#define msm_gemini_core_buf msm_gemini_hw_buf

irqreturn_t msm_gemini_core_irq(int irq_num, void *context);

void msm_gemini_core_irq_install(int (*irq_handler) (int, void *, void *));
void msm_gemini_core_irq_remove(void);

int msm_gemini_core_fe_buf_update(struct msm_gemini_core_buf *buf);
int msm_gemini_core_we_buf_update(struct msm_gemini_core_buf *buf);

int msm_gemini_core_reset(uint8_t op_mode, void *base, int size);
int msm_gemini_core_fe_start(void);

#endif 
