
#ifndef __PMIC8901_H__
#define __PMIC8901_H__


#include <linux/irq.h>
#include <linux/mfd/core.h>



#define PM8901_MPPS		4

#define PM8901_IRQ_BLOCK_BIT(block, bit) ((block) * 8 + (bit))


#define PM8901_MPP_IRQ(base, mpp)	((base) + \
					PM8901_IRQ_BLOCK_BIT(6, (mpp)))

struct pm8901_chip;

struct pm8901_platform_data {
	
	int		irq_base;

	int		num_subdevs;
	struct mfd_cell *sub_devices;
};

struct pm8901_gpio_platform_data {
	int	gpio_base;
	int	irq_base;
};

int pm8901_read(struct pm8901_chip *pm_chip, u16 addr, u8 *values,
		unsigned int len);
int pm8901_write(struct pm8901_chip *pm_chip, u16 addr, u8 *values,
		 unsigned int len);

int pm8901_irq_get_rt_status(struct pm8901_chip *pm_chip, int irq);

#endif 
