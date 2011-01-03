

#ifndef MSM_GEMINI_PLATFORM_H
#define MSM_GEMINI_PLATFORM_H

#include <linux/interrupt.h>
#include <linux/platform_device.h>

void msm_gemini_platform_p2v(struct file  *file);
uint32_t msm_gemini_platform_v2p(int fd, uint32_t len, struct file **file);

int msm_gemini_platform_clk_enable(void);
int msm_gemini_platform_clk_disable(void);

int msm_gemini_platform_init(struct platform_device *pdev,
	struct resource **mem,
	void **base,
	int *irq,
	irqreturn_t (*handler) (int, void *),
	void *context);
int msm_gemini_platform_release(struct resource *mem, void *base, int irq,
	void *context);

#endif 
