

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <scsi/scsi_host.h>
#include <linux/libata.h>

#define DRV_NAME	"pdc_adma"
#define DRV_VERSION	"1.0"


#define ADMA_ATA_REGS(base, port_no)	((base) + ((port_no) * 0x40))


#define ADMA_REGS(base, port_no)	((base) + 0x80 + ((port_no) * 0x20))


#define ADMA_PORT_REGS(ap) \
	ADMA_REGS((ap)->host->iomap[ADMA_MMIO_BAR], ap->port_no)

enum {
	ADMA_MMIO_BAR		= 4,

	ADMA_PORTS		= 2,
	ADMA_CPB_BYTES		= 40,
	ADMA_PRD_BYTES		= LIBATA_MAX_PRD * 16,
	ADMA_PKT_BYTES		= ADMA_CPB_BYTES + ADMA_PRD_BYTES,

	ADMA_DMA_BOUNDARY	= 0xffffffff,

	
	ADMA_MODE_LOCK		= 0x00c7,

	
	ADMA_CONTROL		= 0x0000, 
	ADMA_STATUS		= 0x0002, 
	ADMA_CPB_COUNT		= 0x0004, 
	ADMA_CPB_CURRENT	= 0x000c, 
	ADMA_CPB_NEXT		= 0x000c, 
	ADMA_CPB_LOOKUP		= 0x0010, 
	ADMA_FIFO_IN		= 0x0014, 
	ADMA_FIFO_OUT		= 0x0016, 

	
	aNIEN			= (1 << 8), 
	aGO			= (1 << 7), 
	aRSTADM			= (1 << 5), 
	aPIOMD4			= 0x0003,   

	
	aPSD			= (1 << 6),
	aUIRQ			= (1 << 4),
	aPERR			= (1 << 0),

	
	cDONE			= (1 << 0),
	cATERR			= (1 << 3),

	cVLD			= (1 << 0),
	cDAT			= (1 << 2),
	cIEN			= (1 << 3),

	
	pORD			= (1 << 4),
	pDIRO			= (1 << 5),
	pEND			= (1 << 7),

	
	rIGN			= (1 << 5),
	rEND			= (1 << 7),

	
	ADMA_REGS_CONTROL	= 0x0e,
	ADMA_REGS_SECTOR_COUNT	= 0x12,
	ADMA_REGS_LBA_LOW	= 0x13,
	ADMA_REGS_LBA_MID	= 0x14,
	ADMA_REGS_LBA_HIGH	= 0x15,
	ADMA_REGS_DEVICE	= 0x16,
	ADMA_REGS_COMMAND	= 0x17,

	
	board_1841_idx		= 0,	
};

typedef enum { adma_state_idle, adma_state_pkt, adma_state_mmio } adma_state_t;

struct adma_port_priv {
	u8			*pkt;
	dma_addr_t		pkt_dma;
	adma_state_t		state;
};

static int adma_ata_init_one(struct pci_dev *pdev,
				const struct pci_device_id *ent);
static int adma_port_start(struct ata_port *ap);
static void adma_port_stop(struct ata_port *ap);
static void adma_qc_prep(struct ata_queued_cmd *qc);
static unsigned int adma_qc_issue(struct ata_queued_cmd *qc);
static int adma_check_atapi_dma(struct ata_queued_cmd *qc);
static void adma_freeze(struct ata_port *ap);
static void adma_thaw(struct ata_port *ap);
static int adma_prereset(struct ata_link *link, unsigned long deadline);

static struct scsi_host_template adma_ata_sht = {
	ATA_BASE_SHT(DRV_NAME),
	.sg_tablesize		= LIBATA_MAX_PRD,
	.dma_boundary		= ADMA_DMA_BOUNDARY,
};

static struct ata_port_operations adma_ata_ops = {
	.inherits		= &ata_sff_port_ops,

	.lost_interrupt		= ATA_OP_NULL,

	.check_atapi_dma	= adma_check_atapi_dma,
	.qc_prep		= adma_qc_prep,
	.qc_issue		= adma_qc_issue,

	.freeze			= adma_freeze,
	.thaw			= adma_thaw,
	.prereset		= adma_prereset,

	.port_start		= adma_port_start,
	.port_stop		= adma_port_stop,
};

static struct ata_port_info adma_port_info[] = {
	
	{
		.flags		= ATA_FLAG_SLAVE_POSS |
				  ATA_FLAG_NO_LEGACY | ATA_FLAG_MMIO |
				  ATA_FLAG_PIO_POLLING,
		.pio_mask	= ATA_PIO4_ONLY,
		.udma_mask	= ATA_UDMA4,
		.port_ops	= &adma_ata_ops,
	},
};

static const struct pci_device_id adma_ata_pci_tbl[] = {
	{ PCI_VDEVICE(PDC, 0x1841), board_1841_idx },

	{ }	
};

static struct pci_driver adma_ata_pci_driver = {
	.name			= DRV_NAME,
	.id_table		= adma_ata_pci_tbl,
	.probe			= adma_ata_init_one,
	.remove			= ata_pci_remove_one,
};

static int adma_check_atapi_dma(struct ata_queued_cmd *qc)
{
	return 1;	
}

static void adma_reset_engine(struct ata_port *ap)
{
	void __iomem *chan = ADMA_PORT_REGS(ap);

	
	writew(aPIOMD4 | aNIEN | aRSTADM, chan + ADMA_CONTROL);
	udelay(2);
	writew(aPIOMD4, chan + ADMA_CONTROL);
	udelay(2);
}

static void adma_reinit_engine(struct ata_port *ap)
{
	struct adma_port_priv *pp = ap->private_data;
	void __iomem *chan = ADMA_PORT_REGS(ap);

	
	writeb(ATA_NIEN, ap->ioaddr.ctl_addr);
	ata_sff_check_status(ap);

	
	adma_reset_engine(ap);

	
	writew(0x100, chan + ADMA_FIFO_IN);

	
	writel((u32)pp->pkt_dma, chan + ADMA_CPB_NEXT);

	
	writew(0x100, chan + ADMA_FIFO_OUT);

	
	writew(1, chan + ADMA_CPB_COUNT);

	
	readb(chan + ADMA_STATUS);
}

static inline void adma_enter_reg_mode(struct ata_port *ap)
{
	void __iomem *chan = ADMA_PORT_REGS(ap);

	writew(aPIOMD4, chan + ADMA_CONTROL);
	readb(chan + ADMA_STATUS);	
}

static void adma_freeze(struct ata_port *ap)
{
	void __iomem *chan = ADMA_PORT_REGS(ap);

	
	writeb(ATA_NIEN, ap->ioaddr.ctl_addr);
	ata_sff_check_status(ap);

	
	writew(aPIOMD4 | aNIEN | aRSTADM, chan + ADMA_CONTROL);
	udelay(2);
	writew(aPIOMD4 | aNIEN, chan + ADMA_CONTROL);
	udelay(2);
}

static void adma_thaw(struct ata_port *ap)
{
	adma_reinit_engine(ap);
}

static int adma_prereset(struct ata_link *link, unsigned long deadline)
{
	struct ata_port *ap = link->ap;
	struct adma_port_priv *pp = ap->private_data;

	if (pp->state != adma_state_idle) 
		pp->state = adma_state_mmio;
	adma_reinit_engine(ap);

	return ata_sff_prereset(link, deadline);
}

static int adma_fill_sg(struct ata_queued_cmd *qc)
{
	struct scatterlist *sg;
	struct ata_port *ap = qc->ap;
	struct adma_port_priv *pp = ap->private_data;
	u8  *buf = pp->pkt, *last_buf = NULL;
	int i = (2 + buf[3]) * 8;
	u8 pFLAGS = pORD | ((qc->tf.flags & ATA_TFLAG_WRITE) ? pDIRO : 0);
	unsigned int si;

	for_each_sg(qc->sg, sg, qc->n_elem, si) {
		u32 addr;
		u32 len;

		addr = (u32)sg_dma_address(sg);
		*(__le32 *)(buf + i) = cpu_to_le32(addr);
		i += 4;

		len = sg_dma_len(sg) >> 3;
		*(__le32 *)(buf + i) = cpu_to_le32(len);
		i += 4;

		last_buf = &buf[i];
		buf[i++] = pFLAGS;
		buf[i++] = qc->dev->dma_mode & 0xf;
		buf[i++] = 0;	
		buf[i++] = 0;	

		*(__le32 *)(buf + i) =
			(pFLAGS & pEND) ? 0 : cpu_to_le32(pp->pkt_dma + i + 4);
		i += 4;

		VPRINTK("PRD[%u] = (0x%lX, 0x%X)\n", i/4,
					(unsigned long)addr, len);
	}

	if (likely(last_buf))
		*last_buf |= pEND;

	return i;
}

static void adma_qc_prep(struct ata_queued_cmd *qc)
{
	struct adma_port_priv *pp = qc->ap->private_data;
	u8  *buf = pp->pkt;
	u32 pkt_dma = (u32)pp->pkt_dma;
	int i = 0;

	VPRINTK("ENTER\n");

	adma_enter_reg_mode(qc->ap);
	if (qc->tf.protocol != ATA_PROT_DMA) {
		ata_sff_qc_prep(qc);
		return;
	}

	buf[i++] = 0;	
	buf[i++] = 0;	
	buf[i++] = cVLD | cDAT | cIEN;
	i++;		

	*(__le32 *)(buf+i) = cpu_to_le32(pkt_dma);	
	i += 4;		
	i += 4;		

	buf[i++] = 0;	
	buf[i++] = 0;	
	buf[i++] = 0;	
	buf[i++] = 0;	

	
	buf[i++] = qc->tf.device;
	buf[i++] = ADMA_REGS_DEVICE;
	if ((qc->tf.flags & ATA_TFLAG_LBA48)) {
		buf[i++] = qc->tf.hob_nsect;
		buf[i++] = ADMA_REGS_SECTOR_COUNT;
		buf[i++] = qc->tf.hob_lbal;
		buf[i++] = ADMA_REGS_LBA_LOW;
		buf[i++] = qc->tf.hob_lbam;
		buf[i++] = ADMA_REGS_LBA_MID;
		buf[i++] = qc->tf.hob_lbah;
		buf[i++] = ADMA_REGS_LBA_HIGH;
	}
	buf[i++] = qc->tf.nsect;
	buf[i++] = ADMA_REGS_SECTOR_COUNT;
	buf[i++] = qc->tf.lbal;
	buf[i++] = ADMA_REGS_LBA_LOW;
	buf[i++] = qc->tf.lbam;
	buf[i++] = ADMA_REGS_LBA_MID;
	buf[i++] = qc->tf.lbah;
	buf[i++] = ADMA_REGS_LBA_HIGH;
	buf[i++] = 0;
	buf[i++] = ADMA_REGS_CONTROL;
	buf[i++] = rIGN;
	buf[i++] = 0;
	buf[i++] = qc->tf.command;
	buf[i++] = ADMA_REGS_COMMAND | rEND;

	buf[3] = (i >> 3) - 2;				
	*(__le32 *)(buf+8) = cpu_to_le32(pkt_dma + i);	

	i = adma_fill_sg(qc);
	wmb();	
#if 0
	
	{
		int j, len = 0;
		static char obuf[2048];
		for (j = 0; j < i; ++j) {
			len += sprintf(obuf+len, "%02x ", buf[j]);
			if ((j & 7) == 7) {
				printk("%s\n", obuf);
				len = 0;
			}
		}
		if (len)
			printk("%s\n", obuf);
	}
#endif
}

static inline void adma_packet_start(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	void __iomem *chan = ADMA_PORT_REGS(ap);

	VPRINTK("ENTER, ap %p\n", ap);

	
	writew(aPIOMD4 | aGO, chan + ADMA_CONTROL);
}

static unsigned int adma_qc_issue(struct ata_queued_cmd *qc)
{
	struct adma_port_priv *pp = qc->ap->private_data;

	switch (qc->tf.protocol) {
	case ATA_PROT_DMA:
		pp->state = adma_state_pkt;
		adma_packet_start(qc);
		return 0;

	case ATAPI_PROT_DMA:
		BUG();
		break;

	default:
		break;
	}

	pp->state = adma_state_mmio;
	return ata_sff_qc_issue(qc);
}

static inline unsigned int adma_intr_pkt(struct ata_host *host)
{
	unsigned int handled = 0, port_no;

	for (port_no = 0; port_no < host->n_ports; ++port_no) {
		struct ata_port *ap = host->ports[port_no];
		struct adma_port_priv *pp;
		struct ata_queued_cmd *qc;
		void __iomem *chan = ADMA_PORT_REGS(ap);
		u8 status = readb(chan + ADMA_STATUS);

		if (status == 0)
			continue;
		handled = 1;
		adma_enter_reg_mode(ap);
		if (ap->flags & ATA_FLAG_DISABLED)
			continue;
		pp = ap->private_data;
		if (!pp || pp->state != adma_state_pkt)
			continue;
		qc = ata_qc_from_tag(ap, ap->link.active_tag);
		if (qc && (!(qc->tf.flags & ATA_TFLAG_POLLING))) {
			if (status & aPERR)
				qc->err_mask |= AC_ERR_HOST_BUS;
			else if ((status & (aPSD | aUIRQ)))
				qc->err_mask |= AC_ERR_OTHER;

			if (pp->pkt[0] & cATERR)
				qc->err_mask |= AC_ERR_DEV;
			else if (pp->pkt[0] != cDONE)
				qc->err_mask |= AC_ERR_OTHER;

			if (!qc->err_mask)
				ata_qc_complete(qc);
			else {
				struct ata_eh_info *ehi = &ap->link.eh_info;
				ata_ehi_clear_desc(ehi);
				ata_ehi_push_desc(ehi,
					"ADMA-status 0x%02X", status);
				ata_ehi_push_desc(ehi,
					"pkt[0] 0x%02X", pp->pkt[0]);

				if (qc->err_mask == AC_ERR_DEV)
					ata_port_abort(ap);
				else
					ata_port_freeze(ap);
			}
		}
	}
	return handled;
}

static inline unsigned int adma_intr_mmio(struct ata_host *host)
{
	unsigned int handled = 0, port_no;

	for (port_no = 0; port_no < host->n_ports; ++port_no) {
		struct ata_port *ap;
		ap = host->ports[port_no];
		if (ap && (!(ap->flags & ATA_FLAG_DISABLED))) {
			struct ata_queued_cmd *qc;
			struct adma_port_priv *pp = ap->private_data;
			if (!pp || pp->state != adma_state_mmio)
				continue;
			qc = ata_qc_from_tag(ap, ap->link.active_tag);
			if (qc && (!(qc->tf.flags & ATA_TFLAG_POLLING))) {

				
				u8 status = ata_sff_check_status(ap);
				if ((status & ATA_BUSY))
					continue;
				DPRINTK("ata%u: protocol %d (dev_stat 0x%X)\n",
					ap->print_id, qc->tf.protocol, status);

				
				pp->state = adma_state_idle;
				qc->err_mask |= ac_err_mask(status);
				if (!qc->err_mask)
					ata_qc_complete(qc);
				else {
					struct ata_eh_info *ehi =
						&ap->link.eh_info;
					ata_ehi_clear_desc(ehi);
					ata_ehi_push_desc(ehi,
						"status 0x%02X", status);

					if (qc->err_mask == AC_ERR_DEV)
						ata_port_abort(ap);
					else
						ata_port_freeze(ap);
				}
				handled = 1;
			}
		}
	}
	return handled;
}

static irqreturn_t adma_intr(int irq, void *dev_instance)
{
	struct ata_host *host = dev_instance;
	unsigned int handled = 0;

	VPRINTK("ENTER\n");

	spin_lock(&host->lock);
	handled  = adma_intr_pkt(host) | adma_intr_mmio(host);
	spin_unlock(&host->lock);

	VPRINTK("EXIT\n");

	return IRQ_RETVAL(handled);
}

static void adma_ata_setup_port(struct ata_ioports *port, void __iomem *base)
{
	port->cmd_addr		=
	port->data_addr		= base + 0x000;
	port->error_addr	=
	port->feature_addr	= base + 0x004;
	port->nsect_addr	= base + 0x008;
	port->lbal_addr		= base + 0x00c;
	port->lbam_addr		= base + 0x010;
	port->lbah_addr		= base + 0x014;
	port->device_addr	= base + 0x018;
	port->status_addr	=
	port->command_addr	= base + 0x01c;
	port->altstatus_addr	=
	port->ctl_addr		= base + 0x038;
}

static int adma_port_start(struct ata_port *ap)
{
	struct device *dev = ap->host->dev;
	struct adma_port_priv *pp;
	int rc;

	rc = ata_port_start(ap);
	if (rc)
		return rc;
	adma_enter_reg_mode(ap);
	pp = devm_kzalloc(dev, sizeof(*pp), GFP_KERNEL);
	if (!pp)
		return -ENOMEM;
	pp->pkt = dmam_alloc_coherent(dev, ADMA_PKT_BYTES, &pp->pkt_dma,
				      GFP_KERNEL);
	if (!pp->pkt)
		return -ENOMEM;
	
	if ((pp->pkt_dma & 7) != 0) {
		printk(KERN_ERR "bad alignment for pp->pkt_dma: %08x\n",
						(u32)pp->pkt_dma);
		return -ENOMEM;
	}
	memset(pp->pkt, 0, ADMA_PKT_BYTES);
	ap->private_data = pp;
	adma_reinit_engine(ap);
	return 0;
}

static void adma_port_stop(struct ata_port *ap)
{
	adma_reset_engine(ap);
}

static void adma_host_init(struct ata_host *host, unsigned int chip_id)
{
	unsigned int port_no;

	
	writeb(7, host->iomap[ADMA_MMIO_BAR] + ADMA_MODE_LOCK);

	
	for (port_no = 0; port_no < ADMA_PORTS; ++port_no)
		adma_reset_engine(host->ports[port_no]);
}

static int adma_set_dma_masks(struct pci_dev *pdev, void __iomem *mmio_base)
{
	int rc;

	rc = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
	if (rc) {
		dev_printk(KERN_ERR, &pdev->dev,
			"32-bit DMA enable failed\n");
		return rc;
	}
	rc = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
	if (rc) {
		dev_printk(KERN_ERR, &pdev->dev,
			"32-bit consistent DMA enable failed\n");
		return rc;
	}
	return 0;
}

static int adma_ata_init_one(struct pci_dev *pdev,
			     const struct pci_device_id *ent)
{
	static int printed_version;
	unsigned int board_idx = (unsigned int) ent->driver_data;
	const struct ata_port_info *ppi[] = { &adma_port_info[board_idx], NULL };
	struct ata_host *host;
	void __iomem *mmio_base;
	int rc, port_no;

	if (!printed_version++)
		dev_printk(KERN_DEBUG, &pdev->dev, "version " DRV_VERSION "\n");

	
	host = ata_host_alloc_pinfo(&pdev->dev, ppi, ADMA_PORTS);
	if (!host)
		return -ENOMEM;

	
	rc = pcim_enable_device(pdev);
	if (rc)
		return rc;

	if ((pci_resource_flags(pdev, 4) & IORESOURCE_MEM) == 0)
		return -ENODEV;

	rc = pcim_iomap_regions(pdev, 1 << ADMA_MMIO_BAR, DRV_NAME);
	if (rc)
		return rc;
	host->iomap = pcim_iomap_table(pdev);
	mmio_base = host->iomap[ADMA_MMIO_BAR];

	rc = adma_set_dma_masks(pdev, mmio_base);
	if (rc)
		return rc;

	for (port_no = 0; port_no < ADMA_PORTS; ++port_no) {
		struct ata_port *ap = host->ports[port_no];
		void __iomem *port_base = ADMA_ATA_REGS(mmio_base, port_no);
		unsigned int offset = port_base - mmio_base;

		adma_ata_setup_port(&ap->ioaddr, port_base);

		ata_port_pbar_desc(ap, ADMA_MMIO_BAR, -1, "mmio");
		ata_port_pbar_desc(ap, ADMA_MMIO_BAR, offset, "port");
	}

	
	adma_host_init(host, board_idx);

	pci_set_master(pdev);
	return ata_host_activate(host, pdev->irq, adma_intr, IRQF_SHARED,
				 &adma_ata_sht);
}

static int __init adma_ata_init(void)
{
	return pci_register_driver(&adma_ata_pci_driver);
}

static void __exit adma_ata_exit(void)
{
	pci_unregister_driver(&adma_ata_pci_driver);
}

MODULE_AUTHOR("Mark Lord");
MODULE_DESCRIPTION("Pacific Digital Corporation ADMA low-level driver");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(pci, adma_ata_pci_tbl);
MODULE_VERSION(DRV_VERSION);

module_init(adma_ata_init);
module_exit(adma_ata_exit);
