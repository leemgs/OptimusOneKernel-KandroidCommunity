

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/module.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <asm/mach/irq.h>

#include <mach/regs-gpio.h>
#include <mach/gpio.h>


static void ks8695_gpio_mode(unsigned int pin, short gpio)
{
	unsigned int enable[] = { IOPC_IOEINT0EN, IOPC_IOEINT1EN, IOPC_IOEINT2EN, IOPC_IOEINT3EN, IOPC_IOTIM0EN, IOPC_IOTIM1EN };
	unsigned long x, flags;

	if (pin > KS8695_GPIO_5)	
		return;

	local_irq_save(flags);

	x = __raw_readl(KS8695_GPIO_VA + KS8695_IOPC);
	if (gpio)			
		x &= ~enable[pin];
	else				
		x |= enable[pin];
	__raw_writel(x, KS8695_GPIO_VA + KS8695_IOPC);

	local_irq_restore(flags);
}


static unsigned short gpio_irq[] = { KS8695_IRQ_EXTERN0, KS8695_IRQ_EXTERN1, KS8695_IRQ_EXTERN2, KS8695_IRQ_EXTERN3 };


int ks8695_gpio_interrupt(unsigned int pin, unsigned int type)
{
	unsigned long x, flags;

	if (pin > KS8695_GPIO_3)	
		return -EINVAL;

	local_irq_save(flags);

	
	x = __raw_readl(KS8695_GPIO_VA + KS8695_IOPM);
	x &= ~IOPM(pin);
	__raw_writel(x, KS8695_GPIO_VA + KS8695_IOPM);

	local_irq_restore(flags);

	
	set_irq_type(gpio_irq[pin], type);

	
	ks8695_gpio_mode(pin, 0);

	return 0;
}
EXPORT_SYMBOL(ks8695_gpio_interrupt);






static int ks8695_gpio_direction_input(struct gpio_chip *gc, unsigned int pin)
{
	unsigned long x, flags;

	if (pin > KS8695_GPIO_15)
		return -EINVAL;

	
	ks8695_gpio_mode(pin, 1);

	local_irq_save(flags);

	
	x = __raw_readl(KS8695_GPIO_VA + KS8695_IOPM);
	x &= ~IOPM(pin);
	__raw_writel(x, KS8695_GPIO_VA + KS8695_IOPM);

	local_irq_restore(flags);

	return 0;
}



static int ks8695_gpio_direction_output(struct gpio_chip *gc,
					unsigned int pin, int state)
{
	unsigned long x, flags;

	if (pin > KS8695_GPIO_15)
		return -EINVAL;

	
	ks8695_gpio_mode(pin, 1);

	local_irq_save(flags);

	
	x = __raw_readl(KS8695_GPIO_VA + KS8695_IOPD);
	if (state)
		x |= IOPD(pin);
	else
		x &= ~IOPD(pin);
	__raw_writel(x, KS8695_GPIO_VA + KS8695_IOPD);

	
	x = __raw_readl(KS8695_GPIO_VA + KS8695_IOPM);
	x |= IOPM(pin);
	__raw_writel(x, KS8695_GPIO_VA + KS8695_IOPM);

	local_irq_restore(flags);

	return 0;
}



static void ks8695_gpio_set_value(struct gpio_chip *gc,
				  unsigned int pin, int state)
{
	unsigned long x, flags;

	if (pin > KS8695_GPIO_15)
		return;

	local_irq_save(flags);

	
	x = __raw_readl(KS8695_GPIO_VA + KS8695_IOPD);
	if (state)
		x |= IOPD(pin);
	else
		x &= ~IOPD(pin);
	__raw_writel(x, KS8695_GPIO_VA + KS8695_IOPD);

	local_irq_restore(flags);
}



static int ks8695_gpio_get_value(struct gpio_chip *gc, unsigned int pin)
{
	unsigned long x;

	if (pin > KS8695_GPIO_15)
		return -EINVAL;

	x = __raw_readl(KS8695_GPIO_VA + KS8695_IOPD);
	return (x & IOPD(pin)) != 0;
}



static int ks8695_gpio_to_irq(struct gpio_chip *gc, unsigned int pin)
{
	if (pin > KS8695_GPIO_3)	
		return -EINVAL;

	return gpio_irq[pin];
}


int irq_to_gpio(unsigned int irq)
{
	if ((irq < KS8695_IRQ_EXTERN0) || (irq > KS8695_IRQ_EXTERN3))
		return -EINVAL;

	return (irq - KS8695_IRQ_EXTERN0);
}
EXPORT_SYMBOL(irq_to_gpio);



static struct gpio_chip ks8695_gpio_chip = {
	.label			= "KS8695",
	.direction_input	= ks8695_gpio_direction_input,
	.direction_output	= ks8695_gpio_direction_output,
	.get			= ks8695_gpio_get_value,
	.set			= ks8695_gpio_set_value,
	.to_irq			= ks8695_gpio_to_irq,
	.base			= 0,
	.ngpio			= 16,
	.can_sleep		= 0,
};


void ks8695_register_gpios(void)
{
	if (gpiochip_add(&ks8695_gpio_chip))
		printk(KERN_ERR "Unable to register core GPIOs\n");
}



#ifdef CONFIG_DEBUG_FS

static int ks8695_gpio_show(struct seq_file *s, void *unused)
{
	unsigned int enable[] = { IOPC_IOEINT0EN, IOPC_IOEINT1EN, IOPC_IOEINT2EN, IOPC_IOEINT3EN, IOPC_IOTIM0EN, IOPC_IOTIM1EN };
	unsigned int intmask[] = { IOPC_IOEINT0TM, IOPC_IOEINT1TM, IOPC_IOEINT2TM, IOPC_IOEINT3TM };
	unsigned long mode, ctrl, data;
	int i;

	mode = __raw_readl(KS8695_GPIO_VA + KS8695_IOPM);
	ctrl = __raw_readl(KS8695_GPIO_VA + KS8695_IOPC);
	data = __raw_readl(KS8695_GPIO_VA + KS8695_IOPD);

	seq_printf(s, "Pin\tI/O\tFunction\tState\n\n");

	for (i = KS8695_GPIO_0; i <= KS8695_GPIO_15 ; i++) {
		seq_printf(s, "%i:\t", i);

		seq_printf(s, "%s\t", (mode & IOPM(i)) ? "Output" : "Input");

		if (i <= KS8695_GPIO_3) {
			if (ctrl & enable[i]) {
				seq_printf(s, "EXT%i ", i);

				switch ((ctrl & intmask[i]) >> (4 * i)) {
					case IOPC_TM_LOW:
						seq_printf(s, "(Low)");		break;
					case IOPC_TM_HIGH:
						seq_printf(s, "(High)");	break;
					case IOPC_TM_RISING:
						seq_printf(s, "(Rising)");	break;
					case IOPC_TM_FALLING:
						seq_printf(s, "(Falling)");	break;
					case IOPC_TM_EDGE:
						seq_printf(s, "(Edges)");	break;
				}
			}
			else
				seq_printf(s, "GPIO\t");
		}
		else if (i <= KS8695_GPIO_5) {
			if (ctrl & enable[i])
				seq_printf(s, "TOUT%i\t", i - KS8695_GPIO_4);
			else
				seq_printf(s, "GPIO\t");
		}
		else
			seq_printf(s, "GPIO\t");

		seq_printf(s, "\t");

		seq_printf(s, "%i\n", (data & IOPD(i)) ? 1 : 0);
	}
	return 0;
}

static int ks8695_gpio_open(struct inode *inode, struct file *file)
{
	return single_open(file, ks8695_gpio_show, NULL);
}

static const struct file_operations ks8695_gpio_operations = {
	.open		= ks8695_gpio_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init ks8695_gpio_debugfs_init(void)
{
	
	(void) debugfs_create_file("ks8695_gpio", S_IFREG | S_IRUGO, NULL, NULL, &ks8695_gpio_operations);
	return 0;
}
postcore_initcall(ks8695_gpio_debugfs_init);

#endif
