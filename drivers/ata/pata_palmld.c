

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/libata.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/gpio.h>

#include <scsi/scsi_host.h>
#include <mach/palmld.h>

#define DRV_NAME "pata_palmld"

static struct scsi_host_template palmld_sht = {
	ATA_PIO_SHT(DRV_NAME),
};

static struct ata_port_operations palmld_port_ops = {
	.inherits		= &ata_sff_port_ops,
	.sff_data_xfer		= ata_sff_data_xfer_noirq,
	.cable_detect		= ata_cable_40wire,
};

static __devinit int palmld_pata_probe(struct platform_device *pdev)
{
	struct ata_host *host;
	struct ata_port *ap;
	void __iomem *mem;
	int ret;

	
	host = ata_host_alloc(&pdev->dev, 1);
	if (!host)
		return -ENOMEM;

	
	mem = devm_ioremap(&pdev->dev, PALMLD_IDE_PHYS, 0x1000);
	if (!mem)
		return -ENOMEM;

	
	ret = gpio_request(GPIO_NR_PALMLD_IDE_PWEN, "HDD PWR");
	if (ret)
		goto err1;
	ret = gpio_direction_output(GPIO_NR_PALMLD_IDE_PWEN, 1);
	if (ret)
		goto err2;

	ret = gpio_request(GPIO_NR_PALMLD_IDE_RESET, "HDD RST");
	if (ret)
		goto err2;
	ret = gpio_direction_output(GPIO_NR_PALMLD_IDE_RESET, 0);
	if (ret)
		goto err3;

	
	gpio_set_value(GPIO_NR_PALMLD_IDE_RESET, 0);
	msleep(30);
	gpio_set_value(GPIO_NR_PALMLD_IDE_RESET, 1);
	msleep(30);

	
	ap = host->ports[0];
	ap->ops	= &palmld_port_ops;
	ap->pio_mask = ATA_PIO4;
	ap->flags |= ATA_FLAG_MMIO | ATA_FLAG_NO_LEGACY | ATA_FLAG_PIO_POLLING;

	
	ap->ioaddr.cmd_addr = mem + 0x10;
	ap->ioaddr.altstatus_addr = mem + 0xe;
	ap->ioaddr.ctl_addr = mem + 0xe;

	
	ata_sff_std_ports(&ap->ioaddr);

	
	return ata_host_activate(host, 0, NULL, IRQF_TRIGGER_RISING,
					&palmld_sht);

err3:
	gpio_free(GPIO_NR_PALMLD_IDE_RESET);
err2:
	gpio_free(GPIO_NR_PALMLD_IDE_PWEN);
err1:
	return ret;
}

static __devexit int palmld_pata_remove(struct platform_device *dev)
{
	struct ata_host *host = platform_get_drvdata(dev);

	ata_host_detach(host);

	
	gpio_set_value(GPIO_NR_PALMLD_IDE_PWEN, 0);

	gpio_free(GPIO_NR_PALMLD_IDE_RESET);
	gpio_free(GPIO_NR_PALMLD_IDE_PWEN);

	return 0;
}

static struct platform_driver palmld_pata_platform_driver = {
	.driver	 = {
		.name   = DRV_NAME,
		.owner  = THIS_MODULE,
	},
	.probe		= palmld_pata_probe,
	.remove		= __devexit_p(palmld_pata_remove),
};

static int __init palmld_pata_init(void)
{
	return platform_driver_register(&palmld_pata_platform_driver);
}

static void __exit palmld_pata_exit(void)
{
	platform_driver_unregister(&palmld_pata_platform_driver);
}

MODULE_AUTHOR("Marek Vasut <marek.vasut@gmail.com>");
MODULE_DESCRIPTION("PalmLD PATA driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);

module_init(palmld_pata_init);
module_exit(palmld_pata_exit);
