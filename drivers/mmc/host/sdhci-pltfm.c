



#include <linux/delay.h>
#include <linux/highmem.h>
#include <linux/platform_device.h>

#include <linux/mmc/host.h>

#include <linux/io.h>

#include "sdhci.h"



static struct sdhci_ops sdhci_pltfm_ops = {
};



static int __devinit sdhci_pltfm_probe(struct platform_device *pdev)
{
	struct sdhci_host *host;
	struct resource *iomem;
	int ret;

	BUG_ON(pdev == NULL);

	iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!iomem) {
		ret = -ENOMEM;
		goto err;
	}

	if (resource_size(iomem) != 0x100)
		dev_err(&pdev->dev, "Invalid iomem size. You may "
			"experience problems.\n");

	if (pdev->dev.parent)
		host = sdhci_alloc_host(pdev->dev.parent, 0);
	else
		host = sdhci_alloc_host(&pdev->dev, 0);

	if (IS_ERR(host)) {
		ret = PTR_ERR(host);
		goto err;
	}

	host->hw_name = "platform";
	host->ops = &sdhci_pltfm_ops;
	host->irq = platform_get_irq(pdev, 0);

	if (!request_mem_region(iomem->start, resource_size(iomem),
		mmc_hostname(host->mmc))) {
		dev_err(&pdev->dev, "cannot request region\n");
		ret = -EBUSY;
		goto err_request;
	}

	host->ioaddr = ioremap(iomem->start, resource_size(iomem));
	if (!host->ioaddr) {
		dev_err(&pdev->dev, "failed to remap registers\n");
		ret = -ENOMEM;
		goto err_remap;
	}

	ret = sdhci_add_host(host);
	if (ret)
		goto err_add_host;

	platform_set_drvdata(pdev, host);

	return 0;

err_add_host:
	iounmap(host->ioaddr);
err_remap:
	release_mem_region(iomem->start, resource_size(iomem));
err_request:
	sdhci_free_host(host);
err:
	printk(KERN_ERR"Probing of sdhci-pltfm failed: %d\n", ret);
	return ret;
}

static int __devexit sdhci_pltfm_remove(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct resource *iomem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	int dead;
	u32 scratch;

	dead = 0;
	scratch = readl(host->ioaddr + SDHCI_INT_STATUS);
	if (scratch == (u32)-1)
		dead = 1;

	sdhci_remove_host(host, dead);
	iounmap(host->ioaddr);
	release_mem_region(iomem->start, resource_size(iomem));
	sdhci_free_host(host);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver sdhci_pltfm_driver = {
	.driver = {
		.name	= "sdhci",
		.owner	= THIS_MODULE,
	},
	.probe		= sdhci_pltfm_probe,
	.remove		= __devexit_p(sdhci_pltfm_remove),
};



static int __init sdhci_drv_init(void)
{
	return platform_driver_register(&sdhci_pltfm_driver);
}

static void __exit sdhci_drv_exit(void)
{
	platform_driver_unregister(&sdhci_pltfm_driver);
}

module_init(sdhci_drv_init);
module_exit(sdhci_drv_exit);

MODULE_DESCRIPTION("Secure Digital Host Controller Interface platform driver");
MODULE_AUTHOR("Mocean Laboratories <info@mocean-labs.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:sdhci");

