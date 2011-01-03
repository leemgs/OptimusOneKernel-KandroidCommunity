

#include <linux/init.h>
#include <linux/errno.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/io.h>
#include <linux/gpio.h>


#define XGPIO_DATA_OFFSET   (0x0)	
#define XGPIO_TRI_OFFSET    (0x4)	

struct xgpio_instance {
	struct of_mm_gpio_chip mmchip;
	u32 gpio_state;		
	u32 gpio_dir;		
	spinlock_t gpio_lock;	
};


static int xgpio_get(struct gpio_chip *gc, unsigned int gpio)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);

	return (in_be32(mm_gc->regs + XGPIO_DATA_OFFSET) >> gpio) & 1;
}


static void xgpio_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	unsigned long flags;
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct xgpio_instance *chip =
	    container_of(mm_gc, struct xgpio_instance, mmchip);

	spin_lock_irqsave(&chip->gpio_lock, flags);

	
	if (val)
		chip->gpio_state |= 1 << gpio;
	else
		chip->gpio_state &= ~(1 << gpio);
	out_be32(mm_gc->regs + XGPIO_DATA_OFFSET, chip->gpio_state);

	spin_unlock_irqrestore(&chip->gpio_lock, flags);
}


static int xgpio_dir_in(struct gpio_chip *gc, unsigned int gpio)
{
	unsigned long flags;
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct xgpio_instance *chip =
	    container_of(mm_gc, struct xgpio_instance, mmchip);

	spin_lock_irqsave(&chip->gpio_lock, flags);

	
	chip->gpio_dir |= (1 << gpio);
	out_be32(mm_gc->regs + XGPIO_TRI_OFFSET, chip->gpio_dir);

	spin_unlock_irqrestore(&chip->gpio_lock, flags);

	return 0;
}


static int xgpio_dir_out(struct gpio_chip *gc, unsigned int gpio, int val)
{
	unsigned long flags;
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct xgpio_instance *chip =
	    container_of(mm_gc, struct xgpio_instance, mmchip);

	spin_lock_irqsave(&chip->gpio_lock, flags);

	
	if (val)
		chip->gpio_state |= 1 << gpio;
	else
		chip->gpio_state &= ~(1 << gpio);
	out_be32(mm_gc->regs + XGPIO_DATA_OFFSET, chip->gpio_state);

	
	chip->gpio_dir &= (~(1 << gpio));
	out_be32(mm_gc->regs + XGPIO_TRI_OFFSET, chip->gpio_dir);

	spin_unlock_irqrestore(&chip->gpio_lock, flags);

	return 0;
}


static void xgpio_save_regs(struct of_mm_gpio_chip *mm_gc)
{
	struct xgpio_instance *chip =
	    container_of(mm_gc, struct xgpio_instance, mmchip);

	out_be32(mm_gc->regs + XGPIO_DATA_OFFSET, chip->gpio_state);
	out_be32(mm_gc->regs + XGPIO_TRI_OFFSET, chip->gpio_dir);
}


static int __devinit xgpio_of_probe(struct device_node *np)
{
	struct xgpio_instance *chip;
	struct of_gpio_chip *ofchip;
	int status = 0;
	const u32 *tree_info;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;
	ofchip = &chip->mmchip.of_gc;

	
	tree_info = of_get_property(np, "xlnx,dout-default", NULL);
	if (tree_info)
		chip->gpio_state = *tree_info;

	
	chip->gpio_dir = 0xFFFFFFFF; 
	tree_info = of_get_property(np, "xlnx,tri-default", NULL);
	if (tree_info)
		chip->gpio_dir = *tree_info;

	
	ofchip->gc.ngpio = 32; 
	tree_info = of_get_property(np, "xlnx,gpio-width", NULL);
	if (!tree_info)
		tree_info = of_get_property(np->parent,
					    "xlnx,gpio-width", NULL);
	if (tree_info)
		ofchip->gc.ngpio = *tree_info;

	spin_lock_init(&chip->gpio_lock);

	ofchip->gpio_cells = 2;
	ofchip->gc.direction_input = xgpio_dir_in;
	ofchip->gc.direction_output = xgpio_dir_out;
	ofchip->gc.get = xgpio_get;
	ofchip->gc.set = xgpio_set;

	chip->mmchip.save_regs = xgpio_save_regs;

	
	status = of_mm_gpiochip_add(np, &chip->mmchip);
	if (status) {
		kfree(chip);
		pr_err("%s: error in probe function with status %d\n",
		       np->full_name, status);
		return status;
	}
	pr_info("XGpio: %s: registered\n", np->full_name);
	return 0;
}

static struct of_device_id xgpio_of_match[] __devinitdata = {
	{ .compatible = "xlnx,xps-gpio-1.00.a", },
	{  },
};

static int __init xgpio_init(void)
{
	struct device_node *np;

	for_each_matching_node(np, xgpio_of_match)
		xgpio_of_probe(np);

	return 0;
}


subsys_initcall(xgpio_init);


MODULE_AUTHOR("Xilinx, Inc.");
MODULE_DESCRIPTION("Xilinx GPIO driver");
MODULE_LICENSE("GPL");
