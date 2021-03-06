
#include <linux/init.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/sysdev.h>
#include <linux/device.h>
#include <linux/amba/bus.h>

#include <asm/mach/irq.h>
#include <asm/hardware/vic.h>

static void vic_ack_irq(unsigned int irq)
{
	void __iomem *base = get_irq_chip_data(irq);
	irq &= 31;
	writel(1 << irq, base + VIC_INT_ENABLE_CLEAR);
	
	writel(1 << irq, base + VIC_INT_SOFT_CLEAR);
}

static void vic_mask_irq(unsigned int irq)
{
	void __iomem *base = get_irq_chip_data(irq);
	irq &= 31;
	writel(1 << irq, base + VIC_INT_ENABLE_CLEAR);
}

static void vic_unmask_irq(unsigned int irq)
{
	void __iomem *base = get_irq_chip_data(irq);
	irq &= 31;
	writel(1 << irq, base + VIC_INT_ENABLE);
}


static void vic_init2(void __iomem *base)
{
	int i;

	for (i = 0; i < 16; i++) {
		void __iomem *reg = base + VIC_VECT_CNTL0 + (i * 4);
		writel(VIC_VECT_CNTL_ENABLE | i, reg);
	}

	writel(32, base + VIC_PL190_DEF_VECT_ADDR);
}

#if defined(CONFIG_PM)

struct vic_device {
	struct sys_device sysdev;

	void __iomem	*base;
	int		irq;
	u32		resume_sources;
	u32		resume_irqs;
	u32		int_select;
	u32		int_enable;
	u32		soft_int;
	u32		protect;
};


static struct vic_device vic_devices[CONFIG_ARM_VIC_NR];

static inline struct vic_device *to_vic(struct sys_device *sys)
{
	return container_of(sys, struct vic_device, sysdev);
}

static int vic_id;

static int vic_class_resume(struct sys_device *dev)
{
	struct vic_device *vic = to_vic(dev);
	void __iomem *base = vic->base;

	printk(KERN_DEBUG "%s: resuming vic at %p\n", __func__, base);

	
	vic_init2(base);

	writel(vic->int_select, base + VIC_INT_SELECT);
	writel(vic->protect, base + VIC_PROTECT);

	
	writel(vic->int_enable, base + VIC_INT_ENABLE);
	writel(~vic->int_enable, base + VIC_INT_ENABLE_CLEAR);

	

	writel(vic->soft_int, base + VIC_INT_SOFT);
	writel(~vic->soft_int, base + VIC_INT_SOFT_CLEAR);

	return 0;
}

static int vic_class_suspend(struct sys_device *dev, pm_message_t state)
{
	struct vic_device *vic = to_vic(dev);
	void __iomem *base = vic->base;

	printk(KERN_DEBUG "%s: suspending vic at %p\n", __func__, base);

	vic->int_select = readl(base + VIC_INT_SELECT);
	vic->int_enable = readl(base + VIC_INT_ENABLE);
	vic->soft_int = readl(base + VIC_INT_SOFT);
	vic->protect = readl(base + VIC_PROTECT);

	

	writel(vic->resume_irqs, base + VIC_INT_ENABLE);
	writel(~vic->resume_irqs, base + VIC_INT_ENABLE_CLEAR);

	return 0;
}

struct sysdev_class vic_class = {
	.name		= "vic",
	.suspend	= vic_class_suspend,
	.resume		= vic_class_resume,
};


static void __init vic_pm_register(void __iomem *base, unsigned int irq, u32 resume_sources)
{
	struct vic_device *v;

	if (vic_id >= ARRAY_SIZE(vic_devices))
		printk(KERN_ERR "%s: too few VICs, increase CONFIG_ARM_VIC_NR\n", __func__);
	else {
		v = &vic_devices[vic_id];
		v->base = base;
		v->resume_sources = resume_sources;
		v->irq = irq;
		vic_id++;
	}
}


static int __init vic_pm_init(void)
{
	struct vic_device *dev = vic_devices;
	int err;
	int id;

	if (vic_id == 0)
		return 0;

	err = sysdev_class_register(&vic_class);
	if (err) {
		printk(KERN_ERR "%s: cannot register class\n", __func__);
		return err;
	}

	for (id = 0; id < vic_id; id++, dev++) {
		dev->sysdev.id = id;
		dev->sysdev.cls = &vic_class;

		err = sysdev_register(&dev->sysdev);
		if (err) {
			printk(KERN_ERR "%s: failed to register device\n",
			       __func__);
			return err;
		}
	}

	return 0;
}

late_initcall(vic_pm_init);

static struct vic_device *vic_from_irq(unsigned int irq)
{
        struct vic_device *v = vic_devices;
	unsigned int base_irq = irq & ~31;
	int id;

	for (id = 0; id < vic_id; id++, v++) {
		if (v->irq == base_irq)
			return v;
	}

	return NULL;
}

static int vic_set_wake(unsigned int irq, unsigned int on)
{
	struct vic_device *v = vic_from_irq(irq);
	unsigned int off = irq & 31;
	u32 bit = 1 << off;

	if (!v)
		return -EINVAL;

	if (!(bit & v->resume_sources))
		return -EINVAL;

	if (on)
		v->resume_irqs |= bit;
	else
		v->resume_irqs &= ~bit;

	return 0;
}

#else
static inline void vic_pm_register(void __iomem *base, unsigned int irq, u32 arg1) { }

#define vic_set_wake NULL
#endif 

static struct irq_chip vic_chip = {
	.name	= "VIC",
	.ack	= vic_ack_irq,
	.mask	= vic_mask_irq,
	.unmask	= vic_unmask_irq,
	.set_wake = vic_set_wake,
};


static void vik_init_st(void __iomem *base, unsigned int irq_start,
			 u32 vic_sources);


void __init vic_init(void __iomem *base, unsigned int irq_start,
		     u32 vic_sources, u32 resume_sources)
{
	unsigned int i;
	u32 cellid = 0;
	enum amba_vendor vendor;

	
	for (i = 0; i < 4; i++) {
		u32 addr = ((u32)base & PAGE_MASK) + 0xfe0 + (i * 4);
		cellid |= (readl(addr) & 0xff) << (8 * i);
	}
	vendor = (cellid >> 12) & 0xff;
	printk(KERN_INFO "VIC @%p: id 0x%08x, vendor 0x%02x\n",
	       base, cellid, vendor);

	switch(vendor) {
	case AMBA_VENDOR_ST:
		vik_init_st(base, irq_start, vic_sources);
		return;
	default:
		printk(KERN_WARNING "VIC: unknown vendor, continuing anyways\n");
		
	case AMBA_VENDOR_ARM:
		break;
	}

	

	writel(0, base + VIC_INT_SELECT);
	writel(0, base + VIC_INT_ENABLE);
	writel(~0, base + VIC_INT_ENABLE_CLEAR);
	writel(0, base + VIC_IRQ_STATUS);
	writel(0, base + VIC_ITCR);
	writel(~0, base + VIC_INT_SOFT_CLEAR);

	
	writel(0, base + VIC_PL190_VECT_ADDR);
	for (i = 0; i < 19; i++) {
		unsigned int value;

		value = readl(base + VIC_PL190_VECT_ADDR);
		writel(value, base + VIC_PL190_VECT_ADDR);
	}

	vic_init2(base);

	for (i = 0; i < 32; i++) {
		if (vic_sources & (1 << i)) {
			unsigned int irq = irq_start + i;

			set_irq_chip(irq, &vic_chip);
			set_irq_chip_data(irq, base);
			set_irq_handler(irq, handle_level_irq);
			set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
		}
	}

	vic_pm_register(base, irq_start, resume_sources);
}


static void __init vik_init_st(void __iomem *base, unsigned int irq_start,
				u32 vic_sources)
{
	unsigned int i;
	int vic_2nd_block = ((unsigned long)base & ~PAGE_MASK) != 0;

	

	writel(0, base + VIC_INT_SELECT);
	writel(0, base + VIC_INT_ENABLE);
	writel(~0, base + VIC_INT_ENABLE_CLEAR);
	writel(0, base + VIC_IRQ_STATUS);
	writel(0, base + VIC_ITCR);
	writel(~0, base + VIC_INT_SOFT_CLEAR);

	
	if (vic_2nd_block) {
		writel(0, base + VIC_PL190_VECT_ADDR);
		for (i = 0; i < 19; i++) {
			unsigned int value;

			value = readl(base + VIC_PL190_VECT_ADDR);
			writel(value, base + VIC_PL190_VECT_ADDR);
		}
		
		for (i = 0; i < 16; i++) {
			void __iomem *reg = base + VIC_VECT_CNTL0 + (i * 4);
			writel(0, reg);
		}

		writel(32, base + VIC_PL190_DEF_VECT_ADDR);
	}

	for (i = 0; i < 32; i++) {
		if (vic_sources & (1 << i)) {
			unsigned int irq = irq_start + i;

			set_irq_chip(irq, &vic_chip);
			set_irq_chip_data(irq, base);
			set_irq_handler(irq, handle_level_irq);
			set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
		}
	}
}
