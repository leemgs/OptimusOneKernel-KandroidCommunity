

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/platform_device.h>
#include <linux/mtd/partitions.h>
#include <linux/io.h>
#include <mach/nand.h>
#include <mach/fsmc.h>

#include <mtd/mtd-abi.h>

struct nomadik_nand_host {
	struct mtd_info		mtd;
	struct nand_chip	nand;
	void __iomem *data_va;
	void __iomem *cmd_va;
	void __iomem *addr_va;
	struct nand_bbt_descr *bbt_desc;
};

static struct nand_ecclayout nomadik_ecc_layout = {
	.eccbytes = 3 * 4,
	.eccpos = { 
		0x02, 0x03, 0x04,
		0x12, 0x13, 0x14,
		0x22, 0x23, 0x24,
		0x32, 0x33, 0x34},
	
	.oobfree = { {0x08, 0x08}, {0x18, 0x08}, {0x28, 0x08}, {0x38, 0x08} },
};

static void nomadik_ecc_control(struct mtd_info *mtd, int mode)
{
	
}

static void nomadik_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *nand = mtd->priv;
	struct nomadik_nand_host *host = nand->priv;

	if (cmd == NAND_CMD_NONE)
		return;

	if (ctrl & NAND_CLE)
		writeb(cmd, host->cmd_va);
	else
		writeb(cmd, host->addr_va);
}

static int nomadik_nand_probe(struct platform_device *pdev)
{
	struct nomadik_nand_platform_data *pdata = pdev->dev.platform_data;
	struct nomadik_nand_host *host;
	struct mtd_info *mtd;
	struct nand_chip *nand;
	struct resource *res;
	int ret = 0;

	
	host = kzalloc(sizeof(struct nomadik_nand_host), GFP_KERNEL);
	if (!host) {
		dev_err(&pdev->dev, "Failed to allocate device structure.\n");
		return -ENOMEM;
	}

	
	if (pdata->init)
		ret = pdata->init();
	if (ret < 0) {
		dev_err(&pdev->dev, "Init function failed\n");
		goto err;
	}

	
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "nand_addr");
	if (!res) {
		ret = -EIO;
		goto err_unmap;
	}
	host->addr_va = ioremap(res->start, res->end - res->start + 1);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "nand_data");
	if (!res) {
		ret = -EIO;
		goto err_unmap;
	}
	host->data_va = ioremap(res->start, res->end - res->start + 1);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "nand_cmd");
	if (!res) {
		ret = -EIO;
		goto err_unmap;
	}
	host->cmd_va = ioremap(res->start, res->end - res->start + 1);

	if (!host->addr_va || !host->data_va || !host->cmd_va) {
		ret = -ENOMEM;
		goto err_unmap;
	}

	
	mtd = &host->mtd;
	nand = &host->nand;
	mtd->priv = nand;
	nand->priv = host;

	host->mtd.owner = THIS_MODULE;
	nand->IO_ADDR_R = host->data_va;
	nand->IO_ADDR_W = host->data_va;
	nand->cmd_ctrl = nomadik_cmd_ctrl;

	
	nand->ecc.mode = NAND_ECC_SOFT;
	nand->ecc.layout = &nomadik_ecc_layout;
	nand->ecc.hwctl = nomadik_ecc_control;
	nand->ecc.size = 512;
	nand->ecc.bytes = 3;

	nand->options = pdata->options;

	
	if (nand_scan(&host->mtd, 1)) {
		ret = -ENXIO;
		goto err_unmap;
	}

#ifdef CONFIG_MTD_PARTITIONS
	add_mtd_partitions(&host->mtd, pdata->parts, pdata->nparts);
#else
	pr_info("Registering %s as whole device\n", mtd->name);
	add_mtd_device(mtd);
#endif

	platform_set_drvdata(pdev, host);
	return 0;

 err_unmap:
	if (host->cmd_va)
		iounmap(host->cmd_va);
	if (host->data_va)
		iounmap(host->data_va);
	if (host->addr_va)
		iounmap(host->addr_va);
 err:
	kfree(host);
	return ret;
}


static int nomadik_nand_remove(struct platform_device *pdev)
{
	struct nomadik_nand_host *host = platform_get_drvdata(pdev);
	struct nomadik_nand_platform_data *pdata = pdev->dev.platform_data;

	if (pdata->exit)
		pdata->exit();

	if (host) {
		iounmap(host->cmd_va);
		iounmap(host->data_va);
		iounmap(host->addr_va);
		kfree(host);
	}
	return 0;
}

static int nomadik_nand_suspend(struct device *dev)
{
	struct nomadik_nand_host *host = dev_get_drvdata(dev);
	int ret = 0;
	if (host)
		ret = host->mtd.suspend(&host->mtd);
	return ret;
}

static int nomadik_nand_resume(struct device *dev)
{
	struct nomadik_nand_host *host = dev_get_drvdata(dev);
	if (host)
		host->mtd.resume(&host->mtd);
	return 0;
}

static struct dev_pm_ops nomadik_nand_pm_ops = {
	.suspend = nomadik_nand_suspend,
	.resume = nomadik_nand_resume,
};

static struct platform_driver nomadik_nand_driver = {
	.probe = nomadik_nand_probe,
	.remove = nomadik_nand_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "nomadik_nand",
		.pm = &nomadik_nand_pm_ops,
	},
};

static int __init nand_nomadik_init(void)
{
	pr_info("Nomadik NAND driver\n");
	return platform_driver_register(&nomadik_nand_driver);
}

static void __exit nand_nomadik_exit(void)
{
	platform_driver_unregister(&nomadik_nand_driver);
}

module_init(nand_nomadik_init);
module_exit(nand_nomadik_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ST Microelectronics (sachin.verma@st.com)");
MODULE_DESCRIPTION("NAND driver for Nomadik Platform");
