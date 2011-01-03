

#include <linux/init.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/io.h>

#include <mach/ipu.h>

#include "ipu_intern.h"


static u32 ipu_read_reg(struct ipu *ipu, unsigned long reg)
{
	return __raw_readl(ipu->reg_ipu + reg);
}

static void ipu_write_reg(struct ipu *ipu, u32 value, unsigned long reg)
{
	__raw_writel(value, ipu->reg_ipu + reg);
}




#define IPU_IRQ_NR_FN_BANKS 3
#define IPU_IRQ_NR_ERR_BANKS 2
#define IPU_IRQ_NR_BANKS (IPU_IRQ_NR_FN_BANKS + IPU_IRQ_NR_ERR_BANKS)

struct ipu_irq_bank {
	unsigned int	control;
	unsigned int	status;
	spinlock_t	lock;
	struct ipu	*ipu;
};

static struct ipu_irq_bank irq_bank[IPU_IRQ_NR_BANKS] = {
	
	{
		.control	= IPU_INT_CTRL_1,
		.status		= IPU_INT_STAT_1,
	}, {
		.control	= IPU_INT_CTRL_2,
		.status		= IPU_INT_STAT_2,
	}, {
		.control	= IPU_INT_CTRL_3,
		.status		= IPU_INT_STAT_3,
	},
	
	{
		.control	= IPU_INT_CTRL_4,
		.status		= IPU_INT_STAT_4,
	}, {
		.control	= IPU_INT_CTRL_5,
		.status		= IPU_INT_STAT_5,
	},
};

struct ipu_irq_map {
	unsigned int		irq;
	int			source;
	struct ipu_irq_bank	*bank;
	struct ipu		*ipu;
};

static struct ipu_irq_map irq_map[CONFIG_MX3_IPU_IRQS];

static DEFINE_MUTEX(map_lock);

static DEFINE_SPINLOCK(bank_lock);

static struct ipu_irq_map *src2map(unsigned int src)
{
	int i;

	for (i = 0; i < CONFIG_MX3_IPU_IRQS; i++)
		if (irq_map[i].source == src)
			return irq_map + i;

	return NULL;
}

static void ipu_irq_unmask(unsigned int irq)
{
	struct ipu_irq_map *map = get_irq_chip_data(irq);
	struct ipu_irq_bank *bank;
	uint32_t reg;
	unsigned long lock_flags;

	spin_lock_irqsave(&bank_lock, lock_flags);

	bank = map->bank;
	if (!bank) {
		spin_unlock_irqrestore(&bank_lock, lock_flags);
		pr_err("IPU: %s(%u) - unmapped!\n", __func__, irq);
		return;
	}

	reg = ipu_read_reg(bank->ipu, bank->control);
	reg |= (1UL << (map->source & 31));
	ipu_write_reg(bank->ipu, reg, bank->control);

	spin_unlock_irqrestore(&bank_lock, lock_flags);
}

static void ipu_irq_mask(unsigned int irq)
{
	struct ipu_irq_map *map = get_irq_chip_data(irq);
	struct ipu_irq_bank *bank;
	uint32_t reg;
	unsigned long lock_flags;

	spin_lock_irqsave(&bank_lock, lock_flags);

	bank = map->bank;
	if (!bank) {
		spin_unlock_irqrestore(&bank_lock, lock_flags);
		pr_err("IPU: %s(%u) - unmapped!\n", __func__, irq);
		return;
	}

	reg = ipu_read_reg(bank->ipu, bank->control);
	reg &= ~(1UL << (map->source & 31));
	ipu_write_reg(bank->ipu, reg, bank->control);

	spin_unlock_irqrestore(&bank_lock, lock_flags);
}

static void ipu_irq_ack(unsigned int irq)
{
	struct ipu_irq_map *map = get_irq_chip_data(irq);
	struct ipu_irq_bank *bank;
	unsigned long lock_flags;

	spin_lock_irqsave(&bank_lock, lock_flags);

	bank = map->bank;
	if (!bank) {
		spin_unlock_irqrestore(&bank_lock, lock_flags);
		pr_err("IPU: %s(%u) - unmapped!\n", __func__, irq);
		return;
	}

	ipu_write_reg(bank->ipu, 1UL << (map->source & 31), bank->status);
	spin_unlock_irqrestore(&bank_lock, lock_flags);
}


bool ipu_irq_status(unsigned int irq)
{
	struct ipu_irq_map *map = get_irq_chip_data(irq);
	struct ipu_irq_bank *bank;
	unsigned long lock_flags;
	bool ret;

	spin_lock_irqsave(&bank_lock, lock_flags);
	bank = map->bank;
	ret = bank && ipu_read_reg(bank->ipu, bank->status) &
		(1UL << (map->source & 31));
	spin_unlock_irqrestore(&bank_lock, lock_flags);

	return ret;
}


int ipu_irq_map(unsigned int source)
{
	int i, ret = -ENOMEM;
	struct ipu_irq_map *map;

	might_sleep();

	mutex_lock(&map_lock);
	map = src2map(source);
	if (map) {
		pr_err("IPU: Source %u already mapped to IRQ %u\n", source, map->irq);
		ret = -EBUSY;
		goto out;
	}

	for (i = 0; i < CONFIG_MX3_IPU_IRQS; i++) {
		if (irq_map[i].source < 0) {
			unsigned long lock_flags;

			spin_lock_irqsave(&bank_lock, lock_flags);
			irq_map[i].source = source;
			irq_map[i].bank = irq_bank + source / 32;
			spin_unlock_irqrestore(&bank_lock, lock_flags);

			ret = irq_map[i].irq;
			pr_debug("IPU: mapped source %u to IRQ %u\n",
				 source, ret);
			break;
		}
	}
out:
	mutex_unlock(&map_lock);

	if (ret < 0)
		pr_err("IPU: couldn't map source %u: %d\n", source, ret);

	return ret;
}


int ipu_irq_unmap(unsigned int source)
{
	int i, ret = -EINVAL;

	might_sleep();

	mutex_lock(&map_lock);
	for (i = 0; i < CONFIG_MX3_IPU_IRQS; i++) {
		if (irq_map[i].source == source) {
			unsigned long lock_flags;

			pr_debug("IPU: unmapped source %u from IRQ %u\n",
				 source, irq_map[i].irq);

			spin_lock_irqsave(&bank_lock, lock_flags);
			irq_map[i].source = -EINVAL;
			irq_map[i].bank = NULL;
			spin_unlock_irqrestore(&bank_lock, lock_flags);

			ret = 0;
			break;
		}
	}
	mutex_unlock(&map_lock);

	return ret;
}


static void ipu_irq_err(unsigned int irq, struct irq_desc *desc)
{
	struct ipu *ipu = get_irq_data(irq);
	u32 status;
	int i, line;

	for (i = IPU_IRQ_NR_FN_BANKS; i < IPU_IRQ_NR_BANKS; i++) {
		struct ipu_irq_bank *bank = irq_bank + i;

		spin_lock(&bank_lock);
		status = ipu_read_reg(ipu, bank->status);
		
		status &= ipu_read_reg(ipu, bank->control);
		spin_unlock(&bank_lock);
		while ((line = ffs(status))) {
			struct ipu_irq_map *map;

			line--;
			status &= ~(1UL << line);

			spin_lock(&bank_lock);
			map = src2map(32 * i + line);
			if (map)
				irq = map->irq;
			spin_unlock(&bank_lock);

			if (!map) {
				pr_err("IPU: Interrupt on unmapped source %u bank %d\n",
				       line, i);
				continue;
			}
			generic_handle_irq(irq);
		}
	}
}


static void ipu_irq_fn(unsigned int irq, struct irq_desc *desc)
{
	struct ipu *ipu = get_irq_data(irq);
	u32 status;
	int i, line;

	for (i = 0; i < IPU_IRQ_NR_FN_BANKS; i++) {
		struct ipu_irq_bank *bank = irq_bank + i;

		spin_lock(&bank_lock);
		status = ipu_read_reg(ipu, bank->status);
		
		status &= ipu_read_reg(ipu, bank->control);
		spin_unlock(&bank_lock);
		while ((line = ffs(status))) {
			struct ipu_irq_map *map;

			line--;
			status &= ~(1UL << line);

			spin_lock(&bank_lock);
			map = src2map(32 * i + line);
			if (map)
				irq = map->irq;
			spin_unlock(&bank_lock);

			if (!map) {
				pr_err("IPU: Interrupt on unmapped source %u bank %d\n",
				       line, i);
				continue;
			}
			generic_handle_irq(irq);
		}
	}
}

static struct irq_chip ipu_irq_chip = {
	.name	= "ipu_irq",
	.ack	= ipu_irq_ack,
	.mask	= ipu_irq_mask,
	.unmask	= ipu_irq_unmask,
};


int __init ipu_irq_attach_irq(struct ipu *ipu, struct platform_device *dev)
{
	struct ipu_platform_data *pdata = dev->dev.platform_data;
	unsigned int irq, irq_base, i;

	irq_base = pdata->irq_base;

	for (i = 0; i < IPU_IRQ_NR_BANKS; i++)
		irq_bank[i].ipu = ipu;

	for (i = 0; i < CONFIG_MX3_IPU_IRQS; i++) {
		int ret;

		irq = irq_base + i;
		ret = set_irq_chip(irq, &ipu_irq_chip);
		if (ret < 0)
			return ret;
		ret = set_irq_chip_data(irq, irq_map + i);
		if (ret < 0)
			return ret;
		irq_map[i].ipu = ipu;
		irq_map[i].irq = irq;
		irq_map[i].source = -EINVAL;
		set_irq_handler(irq, handle_level_irq);
#ifdef CONFIG_ARM
		set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
#endif
	}

	set_irq_data(ipu->irq_fn, ipu);
	set_irq_chained_handler(ipu->irq_fn, ipu_irq_fn);

	set_irq_data(ipu->irq_err, ipu);
	set_irq_chained_handler(ipu->irq_err, ipu_irq_err);

	return 0;
}

void ipu_irq_detach_irq(struct ipu *ipu, struct platform_device *dev)
{
	struct ipu_platform_data *pdata = dev->dev.platform_data;
	unsigned int irq, irq_base;

	irq_base = pdata->irq_base;

	set_irq_chained_handler(ipu->irq_fn, NULL);
	set_irq_data(ipu->irq_fn, NULL);

	set_irq_chained_handler(ipu->irq_err, NULL);
	set_irq_data(ipu->irq_err, NULL);

	for (irq = irq_base; irq < irq_base + CONFIG_MX3_IPU_IRQS; irq++) {
#ifdef CONFIG_ARM
		set_irq_flags(irq, 0);
#endif
		set_irq_chip(irq, NULL);
		set_irq_chip_data(irq, NULL);
	}
}
