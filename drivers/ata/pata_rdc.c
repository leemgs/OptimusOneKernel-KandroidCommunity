

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <scsi/scsi_host.h>
#include <linux/libata.h>
#include <linux/dmi.h>

#define DRV_NAME	"pata_rdc"
#define DRV_VERSION	"0.01"

struct rdc_host_priv {
	u32 saved_iocfg;
};



static int rdc_pata_cable_detect(struct ata_port *ap)
{
	struct rdc_host_priv *hpriv = ap->host->private_data;
	u8 mask;

	
	mask = 0x30 << (2 * ap->port_no);
	if ((hpriv->saved_iocfg & mask) == 0)
		return ATA_CBL_PATA40;
	return ATA_CBL_PATA80;
}


static int rdc_pata_prereset(struct ata_link *link, unsigned long deadline)
{
	struct ata_port *ap = link->ap;
	struct pci_dev *pdev = to_pci_dev(ap->host->dev);

	static const struct pci_bits rdc_enable_bits[] = {
		{ 0x41U, 1U, 0x80UL, 0x80UL },	
		{ 0x43U, 1U, 0x80UL, 0x80UL },	
	};

	if (!pci_test_config_bits(pdev, &rdc_enable_bits[ap->port_no]))
		return -ENOENT;
	return ata_sff_prereset(link, deadline);
}



static void rdc_set_piomode(struct ata_port *ap, struct ata_device *adev)
{
	unsigned int pio	= adev->pio_mode - XFER_PIO_0;
	struct pci_dev *dev	= to_pci_dev(ap->host->dev);
	unsigned int is_slave	= (adev->devno != 0);
	unsigned int master_port= ap->port_no ? 0x42 : 0x40;
	unsigned int slave_port	= 0x44;
	u16 master_data;
	u8 slave_data;
	u8 udma_enable;
	int control = 0;

	static const	 
	u8 timings[][2]	= { { 0, 0 },
			    { 0, 0 },
			    { 1, 0 },
			    { 2, 1 },
			    { 2, 3 }, };

	if (pio >= 2)
		control |= 1;	
	if (ata_pio_need_iordy(adev))
		control |= 2;	

	if (adev->class == ATA_DEV_ATA)
		control |= 4;	

	
	pci_read_config_word(dev, master_port, &master_data);
	if (is_slave) {
		
		master_data &= 0xff0f;
		
		master_data |= 0x4000;
		
		master_data |= (control << 4);
		pci_read_config_byte(dev, slave_port, &slave_data);
		slave_data &= (ap->port_no ? 0x0f : 0xf0);
		
		slave_data |= ((timings[pio][0] << 2) | timings[pio][1])
						<< (ap->port_no ? 4 : 0);
	} else {
		
		master_data &= 0xccf0;
		
		master_data |= control;
		
		master_data |=
			(timings[pio][0] << 12) |
			(timings[pio][1] << 8);
	}
	pci_write_config_word(dev, master_port, master_data);
	if (is_slave)
		pci_write_config_byte(dev, slave_port, slave_data);

	

	pci_read_config_byte(dev, 0x48, &udma_enable);
	udma_enable &= ~(1 << (2 * ap->port_no + adev->devno));
	pci_write_config_byte(dev, 0x48, udma_enable);
}



static void rdc_set_dmamode(struct ata_port *ap, struct ata_device *adev)
{
	struct pci_dev *dev	= to_pci_dev(ap->host->dev);
	u8 master_port		= ap->port_no ? 0x42 : 0x40;
	u16 master_data;
	u8 speed		= adev->dma_mode;
	int devid		= adev->devno + 2 * ap->port_no;
	u8 udma_enable		= 0;

	static const	 
	u8 timings[][2]	= { { 0, 0 },
			    { 0, 0 },
			    { 1, 0 },
			    { 2, 1 },
			    { 2, 3 }, };

	pci_read_config_word(dev, master_port, &master_data);
	pci_read_config_byte(dev, 0x48, &udma_enable);

	if (speed >= XFER_UDMA_0) {
		unsigned int udma = adev->dma_mode - XFER_UDMA_0;
		u16 udma_timing;
		u16 ideconf;
		int u_clock, u_speed;

		
		u_speed = min(2 - (udma & 1), udma);
		if (udma == 5)
			u_clock = 0x1000;	
		else if (udma > 2)
			u_clock = 1;		
		else
			u_clock = 0;		

		udma_enable |= (1 << devid);

		
		pci_read_config_word(dev, 0x4A, &udma_timing);
		udma_timing &= ~(3 << (4 * devid));
		udma_timing |= u_speed << (4 * devid);
		pci_write_config_word(dev, 0x4A, udma_timing);

		
		pci_read_config_word(dev, 0x54, &ideconf);
		ideconf &= ~(0x1001 << devid);
		ideconf |= u_clock << devid;
		pci_write_config_word(dev, 0x54, ideconf);
	} else {
		
		unsigned int mwdma	= adev->dma_mode - XFER_MW_DMA_0;
		unsigned int control;
		u8 slave_data;
		const unsigned int needed_pio[3] = {
			XFER_PIO_0, XFER_PIO_3, XFER_PIO_4
		};
		int pio = needed_pio[mwdma] - XFER_PIO_0;

		control = 3;	

		

		if (adev->pio_mode < needed_pio[mwdma])
			
			control |= 8;	

		if (adev->devno) {	
			master_data &= 0xFF4F;  
			master_data |= control << 4;
			pci_read_config_byte(dev, 0x44, &slave_data);
			slave_data &= (ap->port_no ? 0x0f : 0xf0);
			
			slave_data |= ((timings[pio][0] << 2) | timings[pio][1]) << (ap->port_no ? 4 : 0);
			pci_write_config_byte(dev, 0x44, slave_data);
		} else { 	
			master_data &= 0xCCF4;	
			master_data |= control;
			master_data |=
				(timings[pio][0] << 12) |
				(timings[pio][1] << 8);
		}

		udma_enable &= ~(1 << devid);
		pci_write_config_word(dev, master_port, master_data);
	}
	pci_write_config_byte(dev, 0x48, udma_enable);
}

static struct ata_port_operations rdc_pata_ops = {
	.inherits		= &ata_bmdma32_port_ops,
	.cable_detect		= rdc_pata_cable_detect,
	.set_piomode		= rdc_set_piomode,
	.set_dmamode		= rdc_set_dmamode,
	.prereset		= rdc_pata_prereset,
};

static struct ata_port_info rdc_port_info = {

	.flags		= ATA_FLAG_SLAVE_POSS,
	.pio_mask	= ATA_PIO4,
	.mwdma_mask	= ATA_MWDMA2,
	.udma_mask	= ATA_UDMA5,
	.port_ops	= &rdc_pata_ops,
};

static struct scsi_host_template rdc_sht = {
	ATA_BMDMA_SHT(DRV_NAME),
};



static int __devinit rdc_init_one(struct pci_dev *pdev,
				   const struct pci_device_id *ent)
{
	static int printed_version;
	struct device *dev = &pdev->dev;
	struct ata_port_info port_info[2];
	const struct ata_port_info *ppi[] = { &port_info[0], &port_info[1] };
	unsigned long port_flags;
	struct ata_host *host;
	struct rdc_host_priv *hpriv;
	int rc;

	if (!printed_version++)
		dev_printk(KERN_DEBUG, &pdev->dev,
			   "version " DRV_VERSION "\n");

	port_info[0] = rdc_port_info;
	port_info[1] = rdc_port_info;

	port_flags = port_info[0].flags;

	
	rc = pcim_enable_device(pdev);
	if (rc)
		return rc;

	hpriv = devm_kzalloc(dev, sizeof(*hpriv), GFP_KERNEL);
	if (!hpriv)
		return -ENOMEM;

	
	pci_read_config_dword(pdev, 0x54, &hpriv->saved_iocfg);

	rc = ata_pci_sff_prepare_host(pdev, ppi, &host);
	if (rc)
		return rc;
	host->private_data = hpriv;

	pci_intx(pdev, 1);

	host->flags |= ATA_HOST_PARALLEL_SCAN;

	pci_set_master(pdev);
	return ata_pci_sff_activate_host(host, ata_sff_interrupt, &rdc_sht);
}

static void rdc_remove_one(struct pci_dev *pdev)
{
	struct ata_host *host = dev_get_drvdata(&pdev->dev);
	struct rdc_host_priv *hpriv = host->private_data;

	pci_write_config_dword(pdev, 0x54, hpriv->saved_iocfg);

	ata_pci_remove_one(pdev);
}

static const struct pci_device_id rdc_pci_tbl[] = {
	{ PCI_DEVICE(0x17F3, 0x1011), },
	{ PCI_DEVICE(0x17F3, 0x1012), },
	{ }	
};

static struct pci_driver rdc_pci_driver = {
	.name			= DRV_NAME,
	.id_table		= rdc_pci_tbl,
	.probe			= rdc_init_one,
	.remove			= rdc_remove_one,
};


static int __init rdc_init(void)
{
	return pci_register_driver(&rdc_pci_driver);
}

static void __exit rdc_exit(void)
{
	pci_unregister_driver(&rdc_pci_driver);
}

module_init(rdc_init);
module_exit(rdc_exit);

MODULE_AUTHOR("Alan Cox (based on ata_piix)");
MODULE_DESCRIPTION("SCSI low-level driver for RDC PATA controllers");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(pci, rdc_pci_tbl);
MODULE_VERSION(DRV_VERSION);
