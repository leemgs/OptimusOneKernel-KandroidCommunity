

#include <linux/device.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/uio_driver.h>
#include <linux/spinlock.h>

#define DRIVER_VERSION	"0.01.0"
#define DRIVER_AUTHOR	"Michael S. Tsirkin <mst@redhat.com>"
#define DRIVER_DESC	"Generic UIO driver for PCI 2.3 devices"

struct uio_pci_generic_dev {
	struct uio_info info;
	struct pci_dev *pdev;
	spinlock_t lock; 
};

static inline struct uio_pci_generic_dev *
to_uio_pci_generic_dev(struct uio_info *info)
{
	return container_of(info, struct uio_pci_generic_dev, info);
}


static irqreturn_t irqhandler(int irq, struct uio_info *info)
{
	struct uio_pci_generic_dev *gdev = to_uio_pci_generic_dev(info);
	struct pci_dev *pdev = gdev->pdev;
	irqreturn_t ret = IRQ_NONE;
	u32 cmd_status_dword;
	u16 origcmd, newcmd, status;

	
	BUILD_BUG_ON(PCI_COMMAND % 4);
	BUILD_BUG_ON(PCI_COMMAND + 2 != PCI_STATUS);

	spin_lock_irq(&gdev->lock);
	pci_block_user_cfg_access(pdev);

	
	pci_read_config_dword(pdev, PCI_COMMAND, &cmd_status_dword);
	origcmd = cmd_status_dword;
	status = cmd_status_dword >> 16;

	
	if (!(status & PCI_STATUS_INTERRUPT))
		goto done;

	
	newcmd = origcmd | PCI_COMMAND_INTX_DISABLE;
	if (newcmd != origcmd)
		pci_write_config_word(pdev, PCI_COMMAND, newcmd);

	
	ret = IRQ_HANDLED;
done:

	pci_unblock_user_cfg_access(pdev);
	spin_unlock_irq(&gdev->lock);
	return ret;
}


static int __devinit verify_pci_2_3(struct pci_dev *pdev)
{
	u16 orig, new;
	int err = 0;

	pci_block_user_cfg_access(pdev);
	pci_read_config_word(pdev, PCI_COMMAND, &orig);
	pci_write_config_word(pdev, PCI_COMMAND,
			      orig ^ PCI_COMMAND_INTX_DISABLE);
	pci_read_config_word(pdev, PCI_COMMAND, &new);
	
	if ((new ^ orig) & ~PCI_COMMAND_INTX_DISABLE) {
		err = -EBUSY;
		dev_err(&pdev->dev, "Command changed from 0x%x to 0x%x: "
			"driver or HW bug?\n", orig, new);
		goto err;
	}
	if (!((new ^ orig) & PCI_COMMAND_INTX_DISABLE)) {
		dev_warn(&pdev->dev, "Device does not support "
			 "disabling interrupts: unable to bind.\n");
		err = -ENODEV;
		goto err;
	}
	
	pci_write_config_word(pdev, PCI_COMMAND, orig);
err:
	pci_unblock_user_cfg_access(pdev);
	return err;
}

static int __devinit probe(struct pci_dev *pdev,
			   const struct pci_device_id *id)
{
	struct uio_pci_generic_dev *gdev;
	int err;

	if (!pdev->irq) {
		dev_warn(&pdev->dev, "No IRQ assigned to device: "
			 "no support for interrupts?\n");
		return -ENODEV;
	}

	err = pci_enable_device(pdev);
	if (err) {
		dev_err(&pdev->dev, "%s: pci_enable_device failed: %d\n",
			__func__, err);
		return err;
	}

	err = verify_pci_2_3(pdev);
	if (err)
		goto err_verify;

	gdev = kzalloc(sizeof(struct uio_pci_generic_dev), GFP_KERNEL);
	if (!gdev) {
		err = -ENOMEM;
		goto err_alloc;
	}

	gdev->info.name = "uio_pci_generic";
	gdev->info.version = DRIVER_VERSION;
	gdev->info.irq = pdev->irq;
	gdev->info.irq_flags = IRQF_SHARED;
	gdev->info.handler = irqhandler;
	gdev->pdev = pdev;
	spin_lock_init(&gdev->lock);

	if (uio_register_device(&pdev->dev, &gdev->info))
		goto err_register;
	pci_set_drvdata(pdev, gdev);

	return 0;
err_register:
	kfree(gdev);
err_alloc:
err_verify:
	pci_disable_device(pdev);
	return err;
}

static void remove(struct pci_dev *pdev)
{
	struct uio_pci_generic_dev *gdev = pci_get_drvdata(pdev);

	uio_unregister_device(&gdev->info);
	pci_disable_device(pdev);
	kfree(gdev);
}

static struct pci_driver driver = {
	.name = "uio_pci_generic",
	.id_table = NULL, 
	.probe = probe,
	.remove = remove,
};

static int __init init(void)
{
	pr_info(DRIVER_DESC " version: " DRIVER_VERSION "\n");
	return pci_register_driver(&driver);
}

static void __exit cleanup(void)
{
	pci_unregister_driver(&driver);
}

module_init(init);
module_exit(cleanup);

MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
