

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <mach/hardware.h>
#include <mach/ssp.h>
#include <mach/regs-ssp.h>

#define TIMEOUT 100000

static irqreturn_t ssp_interrupt(int irq, void *dev_id)
{
	struct ssp_dev *dev = dev_id;
	struct ssp_device *ssp = dev->ssp;
	unsigned int status;

	status = __raw_readl(ssp->mmio_base + SSSR);
	__raw_writel(status, ssp->mmio_base + SSSR);

	if (status & SSSR_ROR)
		printk(KERN_WARNING "SSP(%d): receiver overrun\n", dev->port);

	if (status & SSSR_TUR)
		printk(KERN_WARNING "SSP(%d): transmitter underrun\n", dev->port);

	if (status & SSSR_BCE)
		printk(KERN_WARNING "SSP(%d): bit count error\n", dev->port);

	return IRQ_HANDLED;
}


int ssp_write_word(struct ssp_dev *dev, u32 data)
{
	struct ssp_device *ssp = dev->ssp;
	int timeout = TIMEOUT;

	while (!(__raw_readl(ssp->mmio_base + SSSR) & SSSR_TNF)) {
	        if (!--timeout)
	        	return -ETIMEDOUT;
		cpu_relax();
	}

	__raw_writel(data, ssp->mmio_base + SSDR);

	return 0;
}


int ssp_read_word(struct ssp_dev *dev, u32 *data)
{
	struct ssp_device *ssp = dev->ssp;
	int timeout = TIMEOUT;

	while (!(__raw_readl(ssp->mmio_base + SSSR) & SSSR_RNE)) {
	        if (!--timeout)
	        	return -ETIMEDOUT;
		cpu_relax();
	}

	*data = __raw_readl(ssp->mmio_base + SSDR);
	return 0;
}


int ssp_flush(struct ssp_dev *dev)
{
	struct ssp_device *ssp = dev->ssp;
	int timeout = TIMEOUT * 2;

	
	if (cpu_is_pxa3xx()) {
		while (__raw_readl(ssp->mmio_base + SSSR) & 0xf00) {
			if (!--timeout)
				return -ETIMEDOUT;
			cpu_relax();
		}
		timeout = TIMEOUT * 2;
	}

	do {
		while (__raw_readl(ssp->mmio_base + SSSR) & SSSR_RNE) {
		        if (!--timeout)
		        	return -ETIMEDOUT;
			(void)__raw_readl(ssp->mmio_base + SSDR);
		}
	        if (!--timeout)
	        	return -ETIMEDOUT;
	} while (__raw_readl(ssp->mmio_base + SSSR) & SSSR_BSY);

	return 0;
}


void ssp_enable(struct ssp_dev *dev)
{
	struct ssp_device *ssp = dev->ssp;
	uint32_t sscr0;

	sscr0 = __raw_readl(ssp->mmio_base + SSCR0);
	sscr0 |= SSCR0_SSE;
	__raw_writel(sscr0, ssp->mmio_base + SSCR0);
}


void ssp_disable(struct ssp_dev *dev)
{
	struct ssp_device *ssp = dev->ssp;
	uint32_t sscr0;

	sscr0 = __raw_readl(ssp->mmio_base + SSCR0);
	sscr0 &= ~SSCR0_SSE;
	__raw_writel(sscr0, ssp->mmio_base + SSCR0);
}


void ssp_save_state(struct ssp_dev *dev, struct ssp_state *state)
{
	struct ssp_device *ssp = dev->ssp;

	state->cr0 = __raw_readl(ssp->mmio_base + SSCR0);
	state->cr1 = __raw_readl(ssp->mmio_base + SSCR1);
	state->to  = __raw_readl(ssp->mmio_base + SSTO);
	state->psp = __raw_readl(ssp->mmio_base + SSPSP);

	ssp_disable(dev);
}


void ssp_restore_state(struct ssp_dev *dev, struct ssp_state *state)
{
	struct ssp_device *ssp = dev->ssp;
	uint32_t sssr = SSSR_ROR | SSSR_TUR | SSSR_BCE;

	__raw_writel(sssr, ssp->mmio_base + SSSR);

	__raw_writel(state->cr0 & ~SSCR0_SSE, ssp->mmio_base + SSCR0);
	__raw_writel(state->cr1, ssp->mmio_base + SSCR1);
	__raw_writel(state->to,  ssp->mmio_base + SSTO);
	__raw_writel(state->psp, ssp->mmio_base + SSPSP);
	__raw_writel(state->cr0, ssp->mmio_base + SSCR0);
}


int ssp_config(struct ssp_dev *dev, u32 mode, u32 flags, u32 psp_flags, u32 speed)
{
	struct ssp_device *ssp = dev->ssp;

	dev->mode = mode;
	dev->flags = flags;
	dev->psp_flags = psp_flags;
	dev->speed = speed;

	
	__raw_writel((dev->speed | dev->mode), ssp->mmio_base + SSCR0);
	__raw_writel(dev->flags, ssp->mmio_base + SSCR1);
	__raw_writel(dev->psp_flags, ssp->mmio_base + SSPSP);

	return 0;
}


int ssp_init(struct ssp_dev *dev, u32 port, u32 init_flags)
{
	struct ssp_device *ssp;
	int ret;

	ssp = ssp_request(port, "SSP");
	if (ssp == NULL)
		return -ENODEV;

	dev->ssp = ssp;
	dev->port = port;

	
	if (!(init_flags & SSP_NO_IRQ)) {
		ret = request_irq(ssp->irq, ssp_interrupt,
				0, "SSP", dev);
	    	if (ret)
			goto out_region;
		dev->irq = ssp->irq;
	} else
		dev->irq = NO_IRQ;

	
	clk_enable(ssp->clk);
	return 0;

out_region:
	ssp_free(ssp);
	return ret;
}


void ssp_exit(struct ssp_dev *dev)
{
	struct ssp_device *ssp = dev->ssp;

	ssp_disable(dev);
	if (dev->irq != NO_IRQ)
		free_irq(dev->irq, dev);
	clk_disable(ssp->clk);
	ssp_free(ssp);
}

static DEFINE_MUTEX(ssp_lock);
static LIST_HEAD(ssp_list);

struct ssp_device *ssp_request(int port, const char *label)
{
	struct ssp_device *ssp = NULL;

	mutex_lock(&ssp_lock);

	list_for_each_entry(ssp, &ssp_list, node) {
		if (ssp->port_id == port && ssp->use_count == 0) {
			ssp->use_count++;
			ssp->label = label;
			break;
		}
	}

	mutex_unlock(&ssp_lock);

	if (&ssp->node == &ssp_list)
		return NULL;

	return ssp;
}
EXPORT_SYMBOL(ssp_request);

void ssp_free(struct ssp_device *ssp)
{
	mutex_lock(&ssp_lock);
	if (ssp->use_count) {
		ssp->use_count--;
		ssp->label = NULL;
	} else
		dev_err(&ssp->pdev->dev, "device already free\n");
	mutex_unlock(&ssp_lock);
}
EXPORT_SYMBOL(ssp_free);

static int __devinit ssp_probe(struct platform_device *pdev, int type)
{
	struct resource *res;
	struct ssp_device *ssp;
	int ret = 0;

	ssp = kzalloc(sizeof(struct ssp_device), GFP_KERNEL);
	if (ssp == NULL) {
		dev_err(&pdev->dev, "failed to allocate memory");
		return -ENOMEM;
	}
	ssp->pdev = pdev;

	ssp->clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(ssp->clk)) {
		ret = PTR_ERR(ssp->clk);
		goto err_free;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		ret = -ENODEV;
		goto err_free_clk;
	}

	res = request_mem_region(res->start, res->end - res->start + 1,
			pdev->name);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to request memory resource\n");
		ret = -EBUSY;
		goto err_free_clk;
	}

	ssp->phys_base = res->start;

	ssp->mmio_base = ioremap(res->start, res->end - res->start + 1);
	if (ssp->mmio_base == NULL) {
		dev_err(&pdev->dev, "failed to ioremap() registers\n");
		ret = -ENODEV;
		goto err_free_mem;
	}

	ssp->irq = platform_get_irq(pdev, 0);
	if (ssp->irq < 0) {
		dev_err(&pdev->dev, "no IRQ resource defined\n");
		ret = -ENODEV;
		goto err_free_io;
	}

	res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "no SSP RX DRCMR defined\n");
		ret = -ENODEV;
		goto err_free_io;
	}
	ssp->drcmr_rx = res->start;

	res = platform_get_resource(pdev, IORESOURCE_DMA, 1);
	if (res == NULL) {
		dev_err(&pdev->dev, "no SSP TX DRCMR defined\n");
		ret = -ENODEV;
		goto err_free_io;
	}
	ssp->drcmr_tx = res->start;

	
	ssp->port_id = pdev->id + 1;
	ssp->use_count = 0;
	ssp->type = type;

	mutex_lock(&ssp_lock);
	list_add(&ssp->node, &ssp_list);
	mutex_unlock(&ssp_lock);

	platform_set_drvdata(pdev, ssp);
	return 0;

err_free_io:
	iounmap(ssp->mmio_base);
err_free_mem:
	release_mem_region(res->start, res->end - res->start + 1);
err_free_clk:
	clk_put(ssp->clk);
err_free:
	kfree(ssp);
	return ret;
}

static int __devexit ssp_remove(struct platform_device *pdev)
{
	struct resource *res;
	struct ssp_device *ssp;

	ssp = platform_get_drvdata(pdev);
	if (ssp == NULL)
		return -ENODEV;

	iounmap(ssp->mmio_base);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, res->end - res->start + 1);

	clk_put(ssp->clk);

	mutex_lock(&ssp_lock);
	list_del(&ssp->node);
	mutex_unlock(&ssp_lock);

	kfree(ssp);
	return 0;
}

static int __devinit pxa25x_ssp_probe(struct platform_device *pdev)
{
	return ssp_probe(pdev, PXA25x_SSP);
}

static int __devinit pxa25x_nssp_probe(struct platform_device *pdev)
{
	return ssp_probe(pdev, PXA25x_NSSP);
}

static int __devinit pxa27x_ssp_probe(struct platform_device *pdev)
{
	return ssp_probe(pdev, PXA27x_SSP);
}

static struct platform_driver pxa25x_ssp_driver = {
	.driver		= {
		.name	= "pxa25x-ssp",
	},
	.probe		= pxa25x_ssp_probe,
	.remove		= __devexit_p(ssp_remove),
};

static struct platform_driver pxa25x_nssp_driver = {
	.driver		= {
		.name	= "pxa25x-nssp",
	},
	.probe		= pxa25x_nssp_probe,
	.remove		= __devexit_p(ssp_remove),
};

static struct platform_driver pxa27x_ssp_driver = {
	.driver		= {
		.name	= "pxa27x-ssp",
	},
	.probe		= pxa27x_ssp_probe,
	.remove		= __devexit_p(ssp_remove),
};

static int __init pxa_ssp_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&pxa25x_ssp_driver);
	if (ret) {
		printk(KERN_ERR "failed to register pxa25x_ssp_driver");
		return ret;
	}

	ret = platform_driver_register(&pxa25x_nssp_driver);
	if (ret) {
		printk(KERN_ERR "failed to register pxa25x_nssp_driver");
		return ret;
	}

	ret = platform_driver_register(&pxa27x_ssp_driver);
	if (ret) {
		printk(KERN_ERR "failed to register pxa27x_ssp_driver");
		return ret;
	}

	return ret;
}

static void __exit pxa_ssp_exit(void)
{
	platform_driver_unregister(&pxa25x_ssp_driver);
	platform_driver_unregister(&pxa25x_nssp_driver);
	platform_driver_unregister(&pxa27x_ssp_driver);
}

arch_initcall(pxa_ssp_init);
module_exit(pxa_ssp_exit);

EXPORT_SYMBOL(ssp_write_word);
EXPORT_SYMBOL(ssp_read_word);
EXPORT_SYMBOL(ssp_flush);
EXPORT_SYMBOL(ssp_enable);
EXPORT_SYMBOL(ssp_disable);
EXPORT_SYMBOL(ssp_save_state);
EXPORT_SYMBOL(ssp_restore_state);
EXPORT_SYMBOL(ssp_init);
EXPORT_SYMBOL(ssp_exit);
EXPORT_SYMBOL(ssp_config);

MODULE_DESCRIPTION("PXA SSP driver");
MODULE_AUTHOR("Liam Girdwood");
MODULE_LICENSE("GPL");

