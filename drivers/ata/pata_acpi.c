

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <scsi/scsi_host.h>
#include <acpi/acpi_bus.h>

#include <linux/libata.h>
#include <linux/ata.h>

#define DRV_NAME	"pata_acpi"
#define DRV_VERSION	"0.2.3"

struct pata_acpi {
	struct ata_acpi_gtm gtm;
	void *last;
	unsigned long mask[2];
};



static int pacpi_pre_reset(struct ata_link *link, unsigned long deadline)
{
	struct ata_port *ap = link->ap;
	struct pata_acpi *acpi = ap->private_data;
	if (ap->acpi_handle == NULL || ata_acpi_gtm(ap, &acpi->gtm) < 0)
		return -ENODEV;

	return ata_sff_prereset(link, deadline);
}



static int pacpi_cable_detect(struct ata_port *ap)
{
	struct pata_acpi *acpi = ap->private_data;

	if ((acpi->mask[0] | acpi->mask[1]) & (0xF8 << ATA_SHIFT_UDMA))
		return ATA_CBL_PATA80;
	else
		return ATA_CBL_PATA40;
}



static unsigned long pacpi_discover_modes(struct ata_port *ap, struct ata_device *adev)
{
	struct pata_acpi *acpi = ap->private_data;
	struct ata_acpi_gtm probe;
	unsigned int xfer_mask;

	probe = acpi->gtm;

	ata_acpi_gtm(ap, &probe);

	xfer_mask = ata_acpi_gtm_xfermask(adev, &probe);

	if (xfer_mask & (0xF8 << ATA_SHIFT_UDMA))
		ap->cbl = ATA_CBL_PATA80;

	return xfer_mask;
}



static unsigned long pacpi_mode_filter(struct ata_device *adev, unsigned long mask)
{
	struct pata_acpi *acpi = adev->link->ap->private_data;
	return ata_bmdma_mode_filter(adev, mask & acpi->mask[adev->devno]);
}



static void pacpi_set_piomode(struct ata_port *ap, struct ata_device *adev)
{
	int unit = adev->devno;
	struct pata_acpi *acpi = ap->private_data;
	const struct ata_timing *t;

	if (!(acpi->gtm.flags & 0x10))
		unit = 0;

	
	t = ata_timing_find_mode(adev->pio_mode);
	acpi->gtm.drive[unit].pio = t->cycle;
	ata_acpi_stm(ap, &acpi->gtm);
	
	ata_acpi_gtm(ap, &acpi->gtm);
}



static void pacpi_set_dmamode(struct ata_port *ap, struct ata_device *adev)
{
	int unit = adev->devno;
	struct pata_acpi *acpi = ap->private_data;
	const struct ata_timing *t;

	if (!(acpi->gtm.flags & 0x10))
		unit = 0;

	
	t = ata_timing_find_mode(adev->dma_mode);
	if (adev->dma_mode >= XFER_UDMA_0) {
		acpi->gtm.drive[unit].dma = t->udma;
		acpi->gtm.flags |= (1 << (2 * unit));
	} else {
		acpi->gtm.drive[unit].dma = t->cycle;
		acpi->gtm.flags &= ~(1 << (2 * unit));
	}
	ata_acpi_stm(ap, &acpi->gtm);
	
	ata_acpi_gtm(ap, &acpi->gtm);
}



static unsigned int pacpi_qc_issue(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct ata_device *adev = qc->dev;
	struct pata_acpi *acpi = ap->private_data;

	if (acpi->gtm.flags & 0x10)
		return ata_sff_qc_issue(qc);

	if (adev != acpi->last) {
		pacpi_set_piomode(ap, adev);
		if (ata_dma_enabled(adev))
			pacpi_set_dmamode(ap, adev);
		acpi->last = adev;
	}
	return ata_sff_qc_issue(qc);
}



static int pacpi_port_start(struct ata_port *ap)
{
	struct pci_dev *pdev = to_pci_dev(ap->host->dev);
	struct pata_acpi *acpi;

	int ret;

	if (ap->acpi_handle == NULL)
		return -ENODEV;

	acpi = ap->private_data = devm_kzalloc(&pdev->dev, sizeof(struct pata_acpi), GFP_KERNEL);
	if (ap->private_data == NULL)
		return -ENOMEM;
	acpi->mask[0] = pacpi_discover_modes(ap, &ap->link.device[0]);
	acpi->mask[1] = pacpi_discover_modes(ap, &ap->link.device[1]);
	ret = ata_sff_port_start(ap);
	if (ret < 0)
		return ret;

	return ret;
}

static struct scsi_host_template pacpi_sht = {
	ATA_BMDMA_SHT(DRV_NAME),
};

static struct ata_port_operations pacpi_ops = {
	.inherits		= &ata_bmdma_port_ops,
	.qc_issue		= pacpi_qc_issue,
	.cable_detect		= pacpi_cable_detect,
	.mode_filter		= pacpi_mode_filter,
	.set_piomode		= pacpi_set_piomode,
	.set_dmamode		= pacpi_set_dmamode,
	.prereset		= pacpi_pre_reset,
	.port_start		= pacpi_port_start,
};




static int pacpi_init_one (struct pci_dev *pdev, const struct pci_device_id *id)
{
	static const struct ata_port_info info = {
		.flags		= ATA_FLAG_SLAVE_POSS | ATA_FLAG_SRST,

		.pio_mask	= ATA_PIO4,
		.mwdma_mask	= ATA_MWDMA2,
		.udma_mask 	= ATA_UDMA6,

		.port_ops	= &pacpi_ops,
	};
	const struct ata_port_info *ppi[] = { &info, NULL };
	if (pdev->vendor == PCI_VENDOR_ID_ATI) {
		int rc = pcim_enable_device(pdev);
		if (rc < 0)
			return rc;
		pcim_pin_device(pdev);
	}
	return ata_pci_sff_init_one(pdev, ppi, &pacpi_sht, NULL);
}

static const struct pci_device_id pacpi_pci_tbl[] = {
	{ PCI_ANY_ID,		PCI_ANY_ID,			   PCI_ANY_ID, PCI_ANY_ID, PCI_CLASS_STORAGE_IDE << 8, 0xFFFFFF00UL, 1},
	{ }	
};

static struct pci_driver pacpi_pci_driver = {
	.name			= DRV_NAME,
	.id_table		= pacpi_pci_tbl,
	.probe			= pacpi_init_one,
	.remove			= ata_pci_remove_one,
#ifdef CONFIG_PM
	.suspend		= ata_pci_device_suspend,
	.resume			= ata_pci_device_resume,
#endif
};

static int __init pacpi_init(void)
{
	return pci_register_driver(&pacpi_pci_driver);
}

static void __exit pacpi_exit(void)
{
	pci_unregister_driver(&pacpi_pci_driver);
}

module_init(pacpi_init);
module_exit(pacpi_exit);

MODULE_AUTHOR("Alan Cox");
MODULE_DESCRIPTION("SCSI low-level driver for ATA in ACPI mode");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(pci, pacpi_pci_tbl);
MODULE_VERSION(DRV_VERSION);

