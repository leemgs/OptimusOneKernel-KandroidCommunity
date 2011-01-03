

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/err.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/sh_timer.h>

struct sh_cmt_priv {
	void __iomem *mapbase;
	struct clk *clk;
	unsigned long width; 
	unsigned long overflow_bit;
	unsigned long clear_bits;
	struct irqaction irqaction;
	struct platform_device *pdev;

	unsigned long flags;
	unsigned long flags_suspend;
	unsigned long match_value;
	unsigned long next_match_value;
	unsigned long max_match_value;
	unsigned long rate;
	spinlock_t lock;
	struct clock_event_device ced;
	struct clocksource cs;
	unsigned long total_cycles;
};

static DEFINE_SPINLOCK(sh_cmt_lock);

#define CMSTR -1 
#define CMCSR 0 
#define CMCNT 1 
#define CMCOR 2 

static inline unsigned long sh_cmt_read(struct sh_cmt_priv *p, int reg_nr)
{
	struct sh_timer_config *cfg = p->pdev->dev.platform_data;
	void __iomem *base = p->mapbase;
	unsigned long offs;

	if (reg_nr == CMSTR) {
		offs = 0;
		base -= cfg->channel_offset;
	} else
		offs = reg_nr;

	if (p->width == 16)
		offs <<= 1;
	else {
		offs <<= 2;
		if ((reg_nr == CMCNT) || (reg_nr == CMCOR))
			return ioread32(base + offs);
	}

	return ioread16(base + offs);
}

static inline void sh_cmt_write(struct sh_cmt_priv *p, int reg_nr,
				unsigned long value)
{
	struct sh_timer_config *cfg = p->pdev->dev.platform_data;
	void __iomem *base = p->mapbase;
	unsigned long offs;

	if (reg_nr == CMSTR) {
		offs = 0;
		base -= cfg->channel_offset;
	} else
		offs = reg_nr;

	if (p->width == 16)
		offs <<= 1;
	else {
		offs <<= 2;
		if ((reg_nr == CMCNT) || (reg_nr == CMCOR)) {
			iowrite32(value, base + offs);
			return;
		}
	}

	iowrite16(value, base + offs);
}

static unsigned long sh_cmt_get_counter(struct sh_cmt_priv *p,
					int *has_wrapped)
{
	unsigned long v1, v2, v3;
	int o1, o2;

	o1 = sh_cmt_read(p, CMCSR) & p->overflow_bit;

	
	do {
		o2 = o1;
		v1 = sh_cmt_read(p, CMCNT);
		v2 = sh_cmt_read(p, CMCNT);
		v3 = sh_cmt_read(p, CMCNT);
		o1 = sh_cmt_read(p, CMCSR) & p->overflow_bit;
	} while (unlikely((o1 != o2) || (v1 > v2 && v1 < v3)
			  || (v2 > v3 && v2 < v1) || (v3 > v1 && v3 < v2)));

	*has_wrapped = o1;
	return v2;
}


static void sh_cmt_start_stop_ch(struct sh_cmt_priv *p, int start)
{
	struct sh_timer_config *cfg = p->pdev->dev.platform_data;
	unsigned long flags, value;

	
	spin_lock_irqsave(&sh_cmt_lock, flags);
	value = sh_cmt_read(p, CMSTR);

	if (start)
		value |= 1 << cfg->timer_bit;
	else
		value &= ~(1 << cfg->timer_bit);

	sh_cmt_write(p, CMSTR, value);
	spin_unlock_irqrestore(&sh_cmt_lock, flags);
}

static int sh_cmt_enable(struct sh_cmt_priv *p, unsigned long *rate)
{
	struct sh_timer_config *cfg = p->pdev->dev.platform_data;
	int ret;

	
	ret = clk_enable(p->clk);
	if (ret) {
		pr_err("sh_cmt: cannot enable clock \"%s\"\n", cfg->clk);
		return ret;
	}

	
	sh_cmt_start_stop_ch(p, 0);

	
	if (p->width == 16) {
		*rate = clk_get_rate(p->clk) / 512;
		sh_cmt_write(p, CMCSR, 0x43);
	} else {
		*rate = clk_get_rate(p->clk) / 8;
		sh_cmt_write(p, CMCSR, 0x01a4);
	}

	sh_cmt_write(p, CMCOR, 0xffffffff);
	sh_cmt_write(p, CMCNT, 0);

	
	sh_cmt_start_stop_ch(p, 1);
	return 0;
}

static void sh_cmt_disable(struct sh_cmt_priv *p)
{
	
	sh_cmt_start_stop_ch(p, 0);

	
	sh_cmt_write(p, CMCSR, 0);

	
	clk_disable(p->clk);
}


#define FLAG_CLOCKEVENT (1 << 0)
#define FLAG_CLOCKSOURCE (1 << 1)
#define FLAG_REPROGRAM (1 << 2)
#define FLAG_SKIPEVENT (1 << 3)
#define FLAG_IRQCONTEXT (1 << 4)

static void sh_cmt_clock_event_program_verify(struct sh_cmt_priv *p,
					      int absolute)
{
	unsigned long new_match;
	unsigned long value = p->next_match_value;
	unsigned long delay = 0;
	unsigned long now = 0;
	int has_wrapped;

	now = sh_cmt_get_counter(p, &has_wrapped);
	p->flags |= FLAG_REPROGRAM; 

	if (has_wrapped) {
		
		p->flags |= FLAG_SKIPEVENT;
		return;
	}

	if (absolute)
		now = 0;

	do {
		
		new_match = now + value + delay;
		if (new_match > p->max_match_value)
			new_match = p->max_match_value;

		sh_cmt_write(p, CMCOR, new_match);

		now = sh_cmt_get_counter(p, &has_wrapped);
		if (has_wrapped && (new_match > p->match_value)) {
			
			p->flags |= FLAG_SKIPEVENT;
			break;
		}

		if (has_wrapped) {
			
			p->match_value = new_match;
			break;
		}

		
		if (now < new_match) {
			
			p->match_value = new_match;
			break;
		}

		
		if (delay)
			delay <<= 1;
		else
			delay = 1;

		if (!delay)
			pr_warning("sh_cmt: too long delay\n");

	} while (delay);
}

static void sh_cmt_set_next(struct sh_cmt_priv *p, unsigned long delta)
{
	unsigned long flags;

	if (delta > p->max_match_value)
		pr_warning("sh_cmt: delta out of range\n");

	spin_lock_irqsave(&p->lock, flags);
	p->next_match_value = delta;
	sh_cmt_clock_event_program_verify(p, 0);
	spin_unlock_irqrestore(&p->lock, flags);
}

static irqreturn_t sh_cmt_interrupt(int irq, void *dev_id)
{
	struct sh_cmt_priv *p = dev_id;

	
	sh_cmt_write(p, CMCSR, sh_cmt_read(p, CMCSR) & p->clear_bits);

	
	if (p->flags & FLAG_CLOCKSOURCE)
		p->total_cycles += p->match_value;

	if (!(p->flags & FLAG_REPROGRAM))
		p->next_match_value = p->max_match_value;

	p->flags |= FLAG_IRQCONTEXT;

	if (p->flags & FLAG_CLOCKEVENT) {
		if (!(p->flags & FLAG_SKIPEVENT)) {
			if (p->ced.mode == CLOCK_EVT_MODE_ONESHOT) {
				p->next_match_value = p->max_match_value;
				p->flags |= FLAG_REPROGRAM;
			}

			p->ced.event_handler(&p->ced);
		}
	}

	p->flags &= ~FLAG_SKIPEVENT;

	if (p->flags & FLAG_REPROGRAM) {
		p->flags &= ~FLAG_REPROGRAM;
		sh_cmt_clock_event_program_verify(p, 1);

		if (p->flags & FLAG_CLOCKEVENT)
			if ((p->ced.mode == CLOCK_EVT_MODE_SHUTDOWN)
			    || (p->match_value == p->next_match_value))
				p->flags &= ~FLAG_REPROGRAM;
	}

	p->flags &= ~FLAG_IRQCONTEXT;

	return IRQ_HANDLED;
}

static int sh_cmt_start(struct sh_cmt_priv *p, unsigned long flag)
{
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&p->lock, flags);

	if (!(p->flags & (FLAG_CLOCKEVENT | FLAG_CLOCKSOURCE)))
		ret = sh_cmt_enable(p, &p->rate);

	if (ret)
		goto out;
	p->flags |= flag;

	
	if ((flag == FLAG_CLOCKSOURCE) && (!(p->flags & FLAG_CLOCKEVENT)))
		sh_cmt_set_next(p, p->max_match_value);
 out:
	spin_unlock_irqrestore(&p->lock, flags);

	return ret;
}

static void sh_cmt_stop(struct sh_cmt_priv *p, unsigned long flag)
{
	unsigned long flags;
	unsigned long f;

	spin_lock_irqsave(&p->lock, flags);

	f = p->flags & (FLAG_CLOCKEVENT | FLAG_CLOCKSOURCE);
	p->flags &= ~flag;

	if (f && !(p->flags & (FLAG_CLOCKEVENT | FLAG_CLOCKSOURCE)))
		sh_cmt_disable(p);

	
	if ((flag == FLAG_CLOCKEVENT) && (p->flags & FLAG_CLOCKSOURCE))
		sh_cmt_set_next(p, p->max_match_value);

	spin_unlock_irqrestore(&p->lock, flags);
}

static struct sh_cmt_priv *cs_to_sh_cmt(struct clocksource *cs)
{
	return container_of(cs, struct sh_cmt_priv, cs);
}

static cycle_t sh_cmt_clocksource_read(struct clocksource *cs)
{
	struct sh_cmt_priv *p = cs_to_sh_cmt(cs);
	unsigned long flags, raw;
	unsigned long value;
	int has_wrapped;

	spin_lock_irqsave(&p->lock, flags);
	value = p->total_cycles;
	raw = sh_cmt_get_counter(p, &has_wrapped);

	if (unlikely(has_wrapped))
		raw += p->match_value;
	spin_unlock_irqrestore(&p->lock, flags);

	return value + raw;
}

static int sh_cmt_clocksource_enable(struct clocksource *cs)
{
	struct sh_cmt_priv *p = cs_to_sh_cmt(cs);
	int ret;

	p->total_cycles = 0;

	ret = sh_cmt_start(p, FLAG_CLOCKSOURCE);
	if (ret)
		return ret;

	
	cs->shift = 0;
	cs->mult = clocksource_hz2mult(p->rate, cs->shift);
	return 0;
}

static void sh_cmt_clocksource_disable(struct clocksource *cs)
{
	sh_cmt_stop(cs_to_sh_cmt(cs), FLAG_CLOCKSOURCE);
}

static int sh_cmt_register_clocksource(struct sh_cmt_priv *p,
				       char *name, unsigned long rating)
{
	struct clocksource *cs = &p->cs;

	cs->name = name;
	cs->rating = rating;
	cs->read = sh_cmt_clocksource_read;
	cs->enable = sh_cmt_clocksource_enable;
	cs->disable = sh_cmt_clocksource_disable;
	cs->mask = CLOCKSOURCE_MASK(sizeof(unsigned long) * 8);
	cs->flags = CLOCK_SOURCE_IS_CONTINUOUS;
	pr_info("sh_cmt: %s used as clock source\n", cs->name);
	clocksource_register(cs);
	return 0;
}

static struct sh_cmt_priv *ced_to_sh_cmt(struct clock_event_device *ced)
{
	return container_of(ced, struct sh_cmt_priv, ced);
}

static void sh_cmt_clock_event_start(struct sh_cmt_priv *p, int periodic)
{
	struct clock_event_device *ced = &p->ced;

	sh_cmt_start(p, FLAG_CLOCKEVENT);

	

	ced->shift = 32;
	ced->mult = div_sc(p->rate, NSEC_PER_SEC, ced->shift);
	ced->max_delta_ns = clockevent_delta2ns(p->max_match_value, ced);
	ced->min_delta_ns = clockevent_delta2ns(0x1f, ced);

	if (periodic)
		sh_cmt_set_next(p, (p->rate + HZ/2) / HZ);
	else
		sh_cmt_set_next(p, p->max_match_value);
}

static void sh_cmt_clock_event_mode(enum clock_event_mode mode,
				    struct clock_event_device *ced)
{
	struct sh_cmt_priv *p = ced_to_sh_cmt(ced);

	
	switch (ced->mode) {
	case CLOCK_EVT_MODE_PERIODIC:
	case CLOCK_EVT_MODE_ONESHOT:
		sh_cmt_stop(p, FLAG_CLOCKEVENT);
		break;
	default:
		break;
	}

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		pr_info("sh_cmt: %s used for periodic clock events\n",
			ced->name);
		sh_cmt_clock_event_start(p, 1);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		pr_info("sh_cmt: %s used for oneshot clock events\n",
			ced->name);
		sh_cmt_clock_event_start(p, 0);
		break;
	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_UNUSED:
		sh_cmt_stop(p, FLAG_CLOCKEVENT);
		break;
	default:
		break;
	}
}

static int sh_cmt_clock_event_next(unsigned long delta,
				   struct clock_event_device *ced)
{
	struct sh_cmt_priv *p = ced_to_sh_cmt(ced);

	BUG_ON(ced->mode != CLOCK_EVT_MODE_ONESHOT);
	if (likely(p->flags & FLAG_IRQCONTEXT))
		p->next_match_value = delta;
	else
		sh_cmt_set_next(p, delta);

	return 0;
}

static void sh_cmt_register_clockevent(struct sh_cmt_priv *p,
				       char *name, unsigned long rating)
{
	struct clock_event_device *ced = &p->ced;

	memset(ced, 0, sizeof(*ced));

	ced->name = name;
	ced->features = CLOCK_EVT_FEAT_PERIODIC;
	ced->features |= CLOCK_EVT_FEAT_ONESHOT;
	ced->rating = rating;
	ced->cpumask = cpumask_of(0);
	ced->set_next_event = sh_cmt_clock_event_next;
	ced->set_mode = sh_cmt_clock_event_mode;

	pr_info("sh_cmt: %s used for clock events\n", ced->name);
	clockevents_register_device(ced);
}

static int sh_cmt_register(struct sh_cmt_priv *p, char *name,
			   unsigned long clockevent_rating,
			   unsigned long clocksource_rating)
{
	if (p->width == (sizeof(p->max_match_value) * 8))
		p->max_match_value = ~0;
	else
		p->max_match_value = (1 << p->width) - 1;

	p->match_value = p->max_match_value;
	spin_lock_init(&p->lock);

	if (clockevent_rating)
		sh_cmt_register_clockevent(p, name, clockevent_rating);

	if (clocksource_rating)
		sh_cmt_register_clocksource(p, name, clocksource_rating);

	return 0;
}

static int sh_cmt_setup(struct sh_cmt_priv *p, struct platform_device *pdev)
{
	struct sh_timer_config *cfg = pdev->dev.platform_data;
	struct resource *res;
	int irq, ret;
	ret = -ENXIO;

	memset(p, 0, sizeof(*p));
	p->pdev = pdev;

	if (!cfg) {
		dev_err(&p->pdev->dev, "missing platform data\n");
		goto err0;
	}

	platform_set_drvdata(pdev, p);

	res = platform_get_resource(p->pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&p->pdev->dev, "failed to get I/O memory\n");
		goto err0;
	}

	irq = platform_get_irq(p->pdev, 0);
	if (irq < 0) {
		dev_err(&p->pdev->dev, "failed to get irq\n");
		goto err0;
	}

	
	p->mapbase = ioremap_nocache(res->start, resource_size(res));
	if (p->mapbase == NULL) {
		pr_err("sh_cmt: failed to remap I/O memory\n");
		goto err0;
	}

	
	p->irqaction.name = cfg->name;
	p->irqaction.handler = sh_cmt_interrupt;
	p->irqaction.dev_id = p;
	p->irqaction.flags = IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL;
	ret = setup_irq(irq, &p->irqaction);
	if (ret) {
		pr_err("sh_cmt: failed to request irq %d\n", irq);
		goto err1;
	}

	
	p->clk = clk_get(&p->pdev->dev, cfg->clk);
	if (IS_ERR(p->clk)) {
		pr_err("sh_cmt: cannot get clock \"%s\"\n", cfg->clk);
		ret = PTR_ERR(p->clk);
		goto err2;
	}

	if (resource_size(res) == 6) {
		p->width = 16;
		p->overflow_bit = 0x80;
		p->clear_bits = ~0x80;
	} else {
		p->width = 32;
		p->overflow_bit = 0x8000;
		p->clear_bits = ~0xc000;
	}

	return sh_cmt_register(p, cfg->name,
			       cfg->clockevent_rating,
			       cfg->clocksource_rating);
 err2:
	remove_irq(irq, &p->irqaction);
 err1:
	iounmap(p->mapbase);
 err0:
	return ret;
}

static int __devinit sh_cmt_probe(struct platform_device *pdev)
{
	struct sh_cmt_priv *p = platform_get_drvdata(pdev);
	struct sh_timer_config *cfg = pdev->dev.platform_data;
	int ret;

	if (p) {
		pr_info("sh_cmt: %s kept as earlytimer\n", cfg->name);
		return 0;
	}

	p = kmalloc(sizeof(*p), GFP_KERNEL);
	if (p == NULL) {
		dev_err(&pdev->dev, "failed to allocate driver data\n");
		return -ENOMEM;
	}

	ret = sh_cmt_setup(p, pdev);
	if (ret) {
		kfree(p);
		platform_set_drvdata(pdev, NULL);
	}
	return ret;
}

static int __devexit sh_cmt_remove(struct platform_device *pdev)
{
	return -EBUSY; 
}

static int sh_cmt_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sh_cmt_priv *p = platform_get_drvdata(pdev);

	
	p->flags_suspend = p->flags;
	sh_cmt_stop(p, p->flags);
	return 0;
}

static int sh_cmt_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sh_cmt_priv *p = platform_get_drvdata(pdev);

	
	sh_cmt_start(p, p->flags_suspend);
	return 0;
}

static struct dev_pm_ops sh_cmt_dev_pm_ops = {
	.suspend = sh_cmt_suspend,
	.resume = sh_cmt_resume,
};

static struct platform_driver sh_cmt_device_driver = {
	.probe		= sh_cmt_probe,
	.remove		= __devexit_p(sh_cmt_remove),
	.driver		= {
		.name	= "sh_cmt",
		.pm	= &sh_cmt_dev_pm_ops,
	}
};

static int __init sh_cmt_init(void)
{
	return platform_driver_register(&sh_cmt_device_driver);
}

static void __exit sh_cmt_exit(void)
{
	platform_driver_unregister(&sh_cmt_device_driver);
}

early_platform_init("earlytimer", &sh_cmt_device_driver);
module_init(sh_cmt_init);
module_exit(sh_cmt_exit);

MODULE_AUTHOR("Magnus Damm");
MODULE_DESCRIPTION("SuperH CMT Timer Driver");
MODULE_LICENSE("GPL v2");
