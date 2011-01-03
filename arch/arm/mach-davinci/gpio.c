

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/bitops.h>

#include <mach/cputype.h>
#include <mach/irqs.h>
#include <mach/hardware.h>
#include <mach/common.h>
#include <mach/gpio.h>

#include <asm/mach/irq.h>


static DEFINE_SPINLOCK(gpio_lock);

struct davinci_gpio {
	struct gpio_chip	chip;
	struct gpio_controller	*__iomem regs;
	int			irq_base;
};

static struct davinci_gpio chips[DIV_ROUND_UP(DAVINCI_N_GPIO, 32)];


static struct gpio_controller __iomem * __init gpio2controller(unsigned gpio)
{
	return __gpio_to_controller(gpio);
}

static int __init davinci_gpio_irq_setup(void);





static int davinci_direction_in(struct gpio_chip *chip, unsigned offset)
{
	struct davinci_gpio *d = container_of(chip, struct davinci_gpio, chip);
	struct gpio_controller *__iomem g = d->regs;
	u32 temp;

	spin_lock(&gpio_lock);
	temp = __raw_readl(&g->dir);
	temp |= (1 << offset);
	__raw_writel(temp, &g->dir);
	spin_unlock(&gpio_lock);

	return 0;
}


static int davinci_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct davinci_gpio *d = container_of(chip, struct davinci_gpio, chip);
	struct gpio_controller *__iomem g = d->regs;

	return (1 << offset) & __raw_readl(&g->in_data);
}

static int
davinci_direction_out(struct gpio_chip *chip, unsigned offset, int value)
{
	struct davinci_gpio *d = container_of(chip, struct davinci_gpio, chip);
	struct gpio_controller *__iomem g = d->regs;
	u32 temp;
	u32 mask = 1 << offset;

	spin_lock(&gpio_lock);
	temp = __raw_readl(&g->dir);
	temp &= ~mask;
	__raw_writel(mask, value ? &g->set_data : &g->clr_data);
	__raw_writel(temp, &g->dir);
	spin_unlock(&gpio_lock);
	return 0;
}


static void
davinci_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct davinci_gpio *d = container_of(chip, struct davinci_gpio, chip);
	struct gpio_controller *__iomem g = d->regs;

	__raw_writel((1 << offset), value ? &g->set_data : &g->clr_data);
}

static int __init davinci_gpio_setup(void)
{
	int i, base;
	unsigned ngpio;
	struct davinci_soc_info *soc_info = &davinci_soc_info;

	
	ngpio = soc_info->gpio_num;
	if (ngpio == 0) {
		pr_err("GPIO setup:  how many GPIOs?\n");
		return -EINVAL;
	}

	if (WARN_ON(DAVINCI_N_GPIO < ngpio))
		ngpio = DAVINCI_N_GPIO;

	for (i = 0, base = 0; base < ngpio; i++, base += 32) {
		chips[i].chip.label = "DaVinci";

		chips[i].chip.direction_input = davinci_direction_in;
		chips[i].chip.get = davinci_gpio_get;
		chips[i].chip.direction_output = davinci_direction_out;
		chips[i].chip.set = davinci_gpio_set;

		chips[i].chip.base = base;
		chips[i].chip.ngpio = ngpio - base;
		if (chips[i].chip.ngpio > 32)
			chips[i].chip.ngpio = 32;

		chips[i].regs = gpio2controller(base);

		gpiochip_add(&chips[i].chip);
	}

	davinci_gpio_irq_setup();
	return 0;
}
pure_initcall(davinci_gpio_setup);




static void gpio_irq_disable(unsigned irq)
{
	struct gpio_controller *__iomem g = get_irq_chip_data(irq);
	u32 mask = (u32) get_irq_data(irq);

	__raw_writel(mask, &g->clr_falling);
	__raw_writel(mask, &g->clr_rising);
}

static void gpio_irq_enable(unsigned irq)
{
	struct gpio_controller *__iomem g = get_irq_chip_data(irq);
	u32 mask = (u32) get_irq_data(irq);
	unsigned status = irq_desc[irq].status;

	status &= IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING;
	if (!status)
		status = IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING;

	if (status & IRQ_TYPE_EDGE_FALLING)
		__raw_writel(mask, &g->set_falling);
	if (status & IRQ_TYPE_EDGE_RISING)
		__raw_writel(mask, &g->set_rising);
}

static int gpio_irq_type(unsigned irq, unsigned trigger)
{
	struct gpio_controller *__iomem g = get_irq_chip_data(irq);
	u32 mask = (u32) get_irq_data(irq);

	if (trigger & ~(IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING))
		return -EINVAL;

	irq_desc[irq].status &= ~IRQ_TYPE_SENSE_MASK;
	irq_desc[irq].status |= trigger;

	
	if (irq_desc[irq].depth == 0) {
		__raw_writel(mask, (trigger & IRQ_TYPE_EDGE_FALLING)
			     ? &g->set_falling : &g->clr_falling);
		__raw_writel(mask, (trigger & IRQ_TYPE_EDGE_RISING)
			     ? &g->set_rising : &g->clr_rising);
	}
	return 0;
}

static struct irq_chip gpio_irqchip = {
	.name		= "GPIO",
	.enable		= gpio_irq_enable,
	.disable	= gpio_irq_disable,
	.set_type	= gpio_irq_type,
};

static void
gpio_irq_handler(unsigned irq, struct irq_desc *desc)
{
	struct gpio_controller *__iomem g = get_irq_chip_data(irq);
	u32 mask = 0xffff;

	
	if (irq & 1)
		mask <<= 16;

	
	desc->chip->mask(irq);
	desc->chip->ack(irq);
	while (1) {
		u32		status;
		int		n;
		int		res;

		
		status = __raw_readl(&g->intstat) & mask;
		if (!status)
			break;
		__raw_writel(status, &g->intstat);
		if (irq & 1)
			status >>= 16;

		
		n = (int)get_irq_data(irq);
		while (status) {
			res = ffs(status);
			n += res;
			generic_handle_irq(n - 1);
			status >>= res;
		}
	}
	desc->chip->unmask(irq);
	
}

static int gpio_to_irq_banked(struct gpio_chip *chip, unsigned offset)
{
	struct davinci_gpio *d = container_of(chip, struct davinci_gpio, chip);

	if (d->irq_base >= 0)
		return d->irq_base + offset;
	else
		return -ENODEV;
}

static int gpio_to_irq_unbanked(struct gpio_chip *chip, unsigned offset)
{
	struct davinci_soc_info *soc_info = &davinci_soc_info;

	
	if (offset < soc_info->gpio_unbanked)
		return soc_info->gpio_irq + offset;
	else
		return -ENODEV;
}

static int gpio_irq_type_unbanked(unsigned irq, unsigned trigger)
{
	struct gpio_controller *__iomem g = get_irq_chip_data(irq);
	u32 mask = (u32) get_irq_data(irq);

	if (trigger & ~(IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING))
		return -EINVAL;

	__raw_writel(mask, (trigger & IRQ_TYPE_EDGE_FALLING)
		     ? &g->set_falling : &g->clr_falling);
	__raw_writel(mask, (trigger & IRQ_TYPE_EDGE_RISING)
		     ? &g->set_rising : &g->clr_rising);

	return 0;
}



static int __init davinci_gpio_irq_setup(void)
{
	unsigned	gpio, irq, bank;
	struct clk	*clk;
	u32		binten = 0;
	unsigned	ngpio, bank_irq;
	struct davinci_soc_info *soc_info = &davinci_soc_info;
	struct gpio_controller	*__iomem g;

	ngpio = soc_info->gpio_num;

	bank_irq = soc_info->gpio_irq;
	if (bank_irq == 0) {
		printk(KERN_ERR "Don't know first GPIO bank IRQ.\n");
		return -EINVAL;
	}

	clk = clk_get(NULL, "gpio");
	if (IS_ERR(clk)) {
		printk(KERN_ERR "Error %ld getting gpio clock?\n",
		       PTR_ERR(clk));
		return PTR_ERR(clk);
	}
	clk_enable(clk);

	
	for (gpio = 0, bank = 0; gpio < ngpio; bank++, gpio += 32) {
		chips[bank].chip.to_irq = gpio_to_irq_banked;
		chips[bank].irq_base = soc_info->gpio_unbanked
			? -EINVAL
			: (soc_info->intc_irq_num + gpio);
	}

	
	if (soc_info->gpio_unbanked) {
		static struct irq_chip gpio_irqchip_unbanked;

		
		chips[0].chip.to_irq = gpio_to_irq_unbanked;
		binten = BIT(0);

		
		irq = bank_irq;
		gpio_irqchip_unbanked = *get_irq_desc_chip(irq_to_desc(irq));
		gpio_irqchip_unbanked.name = "GPIO-AINTC";
		gpio_irqchip_unbanked.set_type = gpio_irq_type_unbanked;

		
		g = gpio2controller(0);
		__raw_writel(~0, &g->set_falling);
		__raw_writel(~0, &g->set_rising);

		
		for (gpio = 0; gpio < soc_info->gpio_unbanked; gpio++, irq++) {
			set_irq_chip(irq, &gpio_irqchip_unbanked);
			set_irq_data(irq, (void *) __gpio_mask(gpio));
			set_irq_chip_data(irq, g);
			irq_desc[irq].status |= IRQ_TYPE_EDGE_BOTH;
		}

		goto done;
	}

	
	for (gpio = 0, irq = gpio_to_irq(0), bank = 0;
			gpio < ngpio;
			bank++, bank_irq++) {
		unsigned		i;

		
		g = gpio2controller(gpio);
		__raw_writel(~0, &g->clr_falling);
		__raw_writel(~0, &g->clr_rising);

		
		set_irq_chained_handler(bank_irq, gpio_irq_handler);
		set_irq_chip_data(bank_irq, g);
		set_irq_data(bank_irq, (void *)irq);

		for (i = 0; i < 16 && gpio < ngpio; i++, irq++, gpio++) {
			set_irq_chip(irq, &gpio_irqchip);
			set_irq_chip_data(irq, g);
			set_irq_data(irq, (void *) __gpio_mask(gpio));
			set_irq_handler(irq, handle_simple_irq);
			set_irq_flags(irq, IRQF_VALID);
		}

		binten |= BIT(bank);
	}

done:
	
	__raw_writel(binten, soc_info->gpio_base + 0x08);

	printk(KERN_INFO "DaVinci: %d gpio irqs\n", irq - gpio_to_irq(0));

	return 0;
}
