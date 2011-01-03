

#undef DEBUG

#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/pci.h>

#include <asm/io.h>

#define LBICTRL_LPCCTL_NR		0x00004000
#define CLE_PIN_CTL			15
#define ALE_PIN_CTL			14

static unsigned int lpcctl;
static struct mtd_info *pasemi_nand_mtd;
static const char driver_name[] = "pasemi-nand";

static void pasemi_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	struct nand_chip *chip = mtd->priv;

	while (len > 0x800) {
		memcpy_fromio(buf, chip->IO_ADDR_R, 0x800);
		buf += 0x800;
		len -= 0x800;
	}
	memcpy_fromio(buf, chip->IO_ADDR_R, len);
}

static void pasemi_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	struct nand_chip *chip = mtd->priv;

	while (len > 0x800) {
		memcpy_toio(chip->IO_ADDR_R, buf, 0x800);
		buf += 0x800;
		len -= 0x800;
	}
	memcpy_toio(chip->IO_ADDR_R, buf, len);
}

static void pasemi_hwcontrol(struct mtd_info *mtd, int cmd,
			     unsigned int ctrl)
{
	struct nand_chip *chip = mtd->priv;

	if (cmd == NAND_CMD_NONE)
		return;

	if (ctrl & NAND_CLE)
		out_8(chip->IO_ADDR_W + (1 << CLE_PIN_CTL), cmd);
	else
		out_8(chip->IO_ADDR_W + (1 << ALE_PIN_CTL), cmd);

	
	eieio();
	inl(lpcctl);
}

int pasemi_device_ready(struct mtd_info *mtd)
{
	return !!(inl(lpcctl) & LBICTRL_LPCCTL_NR);
}

static int __devinit pasemi_nand_probe(struct of_device *ofdev,
				      const struct of_device_id *match)
{
	struct pci_dev *pdev;
	struct device_node *np = ofdev->node;
	struct resource res;
	struct nand_chip *chip;
	int err = 0;

	err = of_address_to_resource(np, 0, &res);

	if (err)
		return -EINVAL;

	
	if (pasemi_nand_mtd)
		return -ENODEV;

	pr_debug("pasemi_nand at %llx-%llx\n", res.start, res.end);

	
	pasemi_nand_mtd = kzalloc(sizeof(struct mtd_info) +
				  sizeof(struct nand_chip), GFP_KERNEL);
	if (!pasemi_nand_mtd) {
		printk(KERN_WARNING
		       "Unable to allocate PASEMI NAND MTD device structure\n");
		err = -ENOMEM;
		goto out;
	}

	
	chip = (struct nand_chip *)&pasemi_nand_mtd[1];

	
	pasemi_nand_mtd->priv = chip;
	pasemi_nand_mtd->owner = THIS_MODULE;

	chip->IO_ADDR_R = of_iomap(np, 0);
	chip->IO_ADDR_W = chip->IO_ADDR_R;

	if (!chip->IO_ADDR_R) {
		err = -EIO;
		goto out_mtd;
	}

	pdev = pci_get_device(PCI_VENDOR_ID_PASEMI, 0xa008, NULL);
	if (!pdev) {
		err = -ENODEV;
		goto out_ior;
	}

	lpcctl = pci_resource_start(pdev, 0);
	pci_dev_put(pdev);

	if (!request_region(lpcctl, 4, driver_name)) {
		err = -EBUSY;
		goto out_ior;
	}

	chip->cmd_ctrl = pasemi_hwcontrol;
	chip->dev_ready = pasemi_device_ready;
	chip->read_buf = pasemi_read_buf;
	chip->write_buf = pasemi_write_buf;
	chip->chip_delay = 0;
	chip->ecc.mode = NAND_ECC_SOFT;

	
	chip->options = NAND_USE_FLASH_BBT | NAND_NO_AUTOINCR;

	
	if (nand_scan(pasemi_nand_mtd, 1)) {
		err = -ENXIO;
		goto out_lpc;
	}

	if (add_mtd_device(pasemi_nand_mtd)) {
		printk(KERN_ERR "pasemi_nand: Unable to register MTD device\n");
		err = -ENODEV;
		goto out_lpc;
	}

	printk(KERN_INFO "PA Semi NAND flash at %08llx, control at I/O %x\n",
	       res.start, lpcctl);

	return 0;

 out_lpc:
	release_region(lpcctl, 4);
 out_ior:
	iounmap(chip->IO_ADDR_R);
 out_mtd:
	kfree(pasemi_nand_mtd);
 out:
	return err;
}

static int __devexit pasemi_nand_remove(struct of_device *ofdev)
{
	struct nand_chip *chip;

	if (!pasemi_nand_mtd)
		return 0;

	chip = pasemi_nand_mtd->priv;

	
	nand_release(pasemi_nand_mtd);

	release_region(lpcctl, 4);

	iounmap(chip->IO_ADDR_R);

	
	kfree(pasemi_nand_mtd);

	pasemi_nand_mtd = NULL;

	return 0;
}

static struct of_device_id pasemi_nand_match[] =
{
	{
		.compatible   = "pasemi,localbus-nand",
	},
	{},
};

MODULE_DEVICE_TABLE(of, pasemi_nand_match);

static struct of_platform_driver pasemi_nand_driver =
{
	.name		= (char*)driver_name,
	.match_table	= pasemi_nand_match,
	.probe		= pasemi_nand_probe,
	.remove		= pasemi_nand_remove,
};

static int __init pasemi_nand_init(void)
{
	return of_register_platform_driver(&pasemi_nand_driver);
}
module_init(pasemi_nand_init);

static void __exit pasemi_nand_exit(void)
{
	of_unregister_platform_driver(&pasemi_nand_driver);
}
module_exit(pasemi_nand_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Egor Martovetsky <egor@pasemi.com>");
MODULE_DESCRIPTION("NAND flash interface driver for PA Semi PWRficient");
