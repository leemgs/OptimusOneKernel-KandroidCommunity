
#ifndef __LINUX_MSM7200A_GPIO_H
#define __LINUX_MSM7200A_GPIO_H

struct msm7200a_gpio_regs {
	void __iomem *in;
	void __iomem *out;
	void __iomem *oe;
	void __iomem *int_status;
	void __iomem *int_clear;
	void __iomem *int_en;
	void __iomem *int_edge;
	void __iomem *int_pos;
};

struct msm7200a_gpio_platform_data {
	unsigned gpio_base;
	unsigned ngpio;
	unsigned irq_base;
	unsigned irq_summary;
	struct msm7200a_gpio_regs regs;
};

#endif
