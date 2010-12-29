
#ifndef _ARCH_ARM_MACH_MSM_TLMM_MSM8660_H
#define _ARCH_ARM_MACH_MSM_TLMM_MSM8660_H


enum {
	GPIO_OE_BIT = 9,
};


enum {
	GPIO_IN_BIT  = 0,
	GPIO_OUT_BIT = 1
};


enum {
	INTR_ENABLE_BIT        = 0,
	INTR_POL_CTL_BIT       = 1,
	INTR_DECT_CTL_BIT      = 2,
	INTR_RAW_STATUS_EN_BIT = 3,
};


enum {
	INTR_STATUS_BIT = 0,
};


enum {
	TARGET_PROC_SCORPION = 4,
	TARGET_PROC_NONE     = 7,
};


#define INTR_RAW_STATUS_EN BIT(INTR_RAW_STATUS_EN_BIT)
#define INTR_DECT_CTL_EDGE BIT(INTR_DECT_CTL_BIT)
#define INTR_POL_CTL_HI    BIT(INTR_POL_CTL_BIT)
#define INTR_ENABLE        BIT(INTR_ENABLE_BIT)

#define DC_IRQ_DISABLE (0 << 3)
#define DC_IRQ_ENABLE  (1 << 3)


#define DC_POLARITY_HI (1 << 11)

#define GPIO_INTR_CFG_SU(gpio)    (MSM_TLMM_BASE + 0x0400 + (0x04 * (gpio)))
#define DIR_CONN_INTR_CFG_SU(irq) (MSM_TLMM_BASE + 0x0700 + (0x04 * (irq)))
#define GPIO_CONFIG(gpio)         (MSM_TLMM_BASE + 0x1000 + (0x10 * (gpio)))
#define GPIO_IN_OUT(gpio)         (MSM_TLMM_BASE + 0x1004 + (0x10 * (gpio)))
#define GPIO_INTR_CFG(gpio)       (MSM_TLMM_BASE + 0x1008 + (0x10 * (gpio)))
#define GPIO_INTR_STATUS(gpio)    (MSM_TLMM_BASE + 0x100c + (0x10 * (gpio)))
#define GPIO_OE_CLR(gpio)  (MSM_TLMM_BASE + 0x3100 + (0x04 * ((gpio) / 32)))
#define GPIO_OE_SET(gpio)  (MSM_TLMM_BASE + 0x3120 + (0x04 * ((gpio) / 32)))

#endif
