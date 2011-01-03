

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_device.h>
#include <linux/libata.h>

#define DRV_NAME			"sata_nv"
#define DRV_VERSION			"3.5"

#define NV_ADMA_DMA_BOUNDARY		0xffffffffUL

enum {
	NV_MMIO_BAR			= 5,

	NV_PORTS			= 2,
	NV_PIO_MASK			= ATA_PIO4,
	NV_MWDMA_MASK			= ATA_MWDMA2,
	NV_UDMA_MASK			= ATA_UDMA6,
	NV_PORT0_SCR_REG_OFFSET		= 0x00,
	NV_PORT1_SCR_REG_OFFSET		= 0x40,

	
	NV_INT_STATUS			= 0x10,
	NV_INT_ENABLE			= 0x11,
	NV_INT_STATUS_CK804		= 0x440,
	NV_INT_ENABLE_CK804		= 0x441,

	
	NV_INT_DEV			= 0x01,
	NV_INT_PM			= 0x02,
	NV_INT_ADDED			= 0x04,
	NV_INT_REMOVED			= 0x08,

	NV_INT_PORT_SHIFT		= 4,	

	NV_INT_ALL			= 0x0f,
	NV_INT_MASK			= NV_INT_DEV |
					  NV_INT_ADDED | NV_INT_REMOVED,

	
	NV_INT_CONFIG			= 0x12,
	NV_INT_CONFIG_METHD		= 0x01, 

	
	NV_MCP_SATA_CFG_20		= 0x50,
	NV_MCP_SATA_CFG_20_SATA_SPACE_EN = 0x04,
	NV_MCP_SATA_CFG_20_PORT0_EN	= (1 << 17),
	NV_MCP_SATA_CFG_20_PORT1_EN	= (1 << 16),
	NV_MCP_SATA_CFG_20_PORT0_PWB_EN	= (1 << 14),
	NV_MCP_SATA_CFG_20_PORT1_PWB_EN	= (1 << 12),

	NV_ADMA_MAX_CPBS		= 32,
	NV_ADMA_CPB_SZ			= 128,
	NV_ADMA_APRD_SZ			= 16,
	NV_ADMA_SGTBL_LEN		= (1024 - NV_ADMA_CPB_SZ) /
					   NV_ADMA_APRD_SZ,
	NV_ADMA_SGTBL_TOTAL_LEN		= NV_ADMA_SGTBL_LEN + 5,
	NV_ADMA_SGTBL_SZ                = NV_ADMA_SGTBL_LEN * NV_ADMA_APRD_SZ,
	NV_ADMA_PORT_PRIV_DMA_SZ        = NV_ADMA_MAX_CPBS *
					   (NV_ADMA_CPB_SZ + NV_ADMA_SGTBL_SZ),

	
	NV_ADMA_GEN			= 0x400,
	NV_ADMA_GEN_CTL			= 0x00,
	NV_ADMA_NOTIFIER_CLEAR		= 0x30,

	
	NV_ADMA_PORT			= 0x480,

	
	NV_ADMA_PORT_SIZE		= 0x100,

	
	NV_ADMA_CTL			= 0x40,
	NV_ADMA_CPB_COUNT		= 0x42,
	NV_ADMA_NEXT_CPB_IDX		= 0x43,
	NV_ADMA_STAT			= 0x44,
	NV_ADMA_CPB_BASE_LOW		= 0x48,
	NV_ADMA_CPB_BASE_HIGH		= 0x4C,
	NV_ADMA_APPEND			= 0x50,
	NV_ADMA_NOTIFIER		= 0x68,
	NV_ADMA_NOTIFIER_ERROR		= 0x6C,

	
	NV_ADMA_CTL_HOTPLUG_IEN		= (1 << 0),
	NV_ADMA_CTL_CHANNEL_RESET	= (1 << 5),
	NV_ADMA_CTL_GO			= (1 << 7),
	NV_ADMA_CTL_AIEN		= (1 << 8),
	NV_ADMA_CTL_READ_NON_COHERENT	= (1 << 11),
	NV_ADMA_CTL_WRITE_NON_COHERENT	= (1 << 12),

	
	NV_CPB_RESP_DONE		= (1 << 0),
	NV_CPB_RESP_ATA_ERR		= (1 << 3),
	NV_CPB_RESP_CMD_ERR		= (1 << 4),
	NV_CPB_RESP_CPB_ERR		= (1 << 7),

	
	NV_CPB_CTL_CPB_VALID		= (1 << 0),
	NV_CPB_CTL_QUEUE		= (1 << 1),
	NV_CPB_CTL_APRD_VALID		= (1 << 2),
	NV_CPB_CTL_IEN			= (1 << 3),
	NV_CPB_CTL_FPDMA		= (1 << 4),

	
	NV_APRD_WRITE			= (1 << 1),
	NV_APRD_END			= (1 << 2),
	NV_APRD_CONT			= (1 << 3),

	
	NV_ADMA_STAT_TIMEOUT		= (1 << 0),
	NV_ADMA_STAT_HOTUNPLUG		= (1 << 1),
	NV_ADMA_STAT_HOTPLUG		= (1 << 2),
	NV_ADMA_STAT_CPBERR		= (1 << 4),
	NV_ADMA_STAT_SERROR		= (1 << 5),
	NV_ADMA_STAT_CMD_COMPLETE	= (1 << 6),
	NV_ADMA_STAT_IDLE		= (1 << 8),
	NV_ADMA_STAT_LEGACY		= (1 << 9),
	NV_ADMA_STAT_STOPPED		= (1 << 10),
	NV_ADMA_STAT_DONE		= (1 << 12),
	NV_ADMA_STAT_ERR		= NV_ADMA_STAT_CPBERR |
					  NV_ADMA_STAT_TIMEOUT,

	
	NV_ADMA_PORT_REGISTER_MODE	= (1 << 0),
	NV_ADMA_ATAPI_SETUP_COMPLETE	= (1 << 1),

	
	NV_CTL_MCP55			= 0x400,
	NV_INT_STATUS_MCP55		= 0x440,
	NV_INT_ENABLE_MCP55		= 0x444,
	NV_NCQ_REG_MCP55		= 0x448,

	
	NV_INT_ALL_MCP55		= 0xffff,
	NV_INT_PORT_SHIFT_MCP55		= 16,	
	NV_INT_MASK_MCP55		= NV_INT_ALL_MCP55 & 0xfffd,

	
	NV_CTL_PRI_SWNCQ		= 0x02,
	NV_CTL_SEC_SWNCQ		= 0x04,

	
	NV_SWNCQ_IRQ_DEV		= (1 << 0),
	NV_SWNCQ_IRQ_PM			= (1 << 1),
	NV_SWNCQ_IRQ_ADDED		= (1 << 2),
	NV_SWNCQ_IRQ_REMOVED		= (1 << 3),

	NV_SWNCQ_IRQ_BACKOUT		= (1 << 4),
	NV_SWNCQ_IRQ_SDBFIS		= (1 << 5),
	NV_SWNCQ_IRQ_DHREGFIS		= (1 << 6),
	NV_SWNCQ_IRQ_DMASETUP		= (1 << 7),

	NV_SWNCQ_IRQ_HOTPLUG		= NV_SWNCQ_IRQ_ADDED |
					  NV_SWNCQ_IRQ_REMOVED,

};


struct nv_adma_prd {
	__le64			addr;
	__le32			len;
	u8			flags;
	u8			packet_len;
	__le16			reserved;
};

enum nv_adma_regbits {
	CMDEND	= (1 << 15),		
	WNB	= (1 << 14),		
	IGN	= (1 << 13),		
	CS1n	= (1 << (4 + 8)),	
	DA2	= (1 << (2 + 8)),
	DA1	= (1 << (1 + 8)),
	DA0	= (1 << (0 + 8)),
};


struct nv_adma_cpb {
	u8			resp_flags;    
	u8			reserved1;     
	u8			ctl_flags;     
	
	u8			len;		
	u8			tag;           
	u8			next_cpb_idx;  
	__le16			reserved2;     
	__le16			tf[12];        
	struct nv_adma_prd	aprd[5];       
	__le64			next_aprd;     
	__le64			reserved3;     
};


struct nv_adma_port_priv {
	struct nv_adma_cpb	*cpb;
	dma_addr_t		cpb_dma;
	struct nv_adma_prd	*aprd;
	dma_addr_t		aprd_dma;
	void __iomem		*ctl_block;
	void __iomem		*gen_block;
	void __iomem		*notifier_clear_block;
	u64			adma_dma_mask;
	u8			flags;
	int			last_issue_ncq;
};

struct nv_host_priv {
	unsigned long		type;
};

struct defer_queue {
	u32		defer_bits;
	unsigned int	head;
	unsigned int	tail;
	unsigned int	tag[ATA_MAX_QUEUE];
};

enum ncq_saw_flag_list {
	ncq_saw_d2h	= (1U << 0),
	ncq_saw_dmas	= (1U << 1),
	ncq_saw_sdb	= (1U << 2),
	ncq_saw_backout	= (1U << 3),
};

struct nv_swncq_port_priv {
	struct ata_prd	*prd;	 
	dma_addr_t	prd_dma; 
	void __iomem	*sactive_block;
	void __iomem	*irq_block;
	void __iomem	*tag_block;
	u32		qc_active;

	unsigned int	last_issue_tag;

	
	struct defer_queue defer_queue;

	
	u32		dhfis_bits;
	u32		dmafis_bits;
	u32		sdbfis_bits;

	unsigned int	ncq_flags;
};


#define NV_ADMA_CHECK_INTR(GCTL, PORT) ((GCTL) & (1 << (19 + (12 * (PORT)))))

static int nv_init_one(struct pci_dev *pdev, const struct pci_device_id *ent);
#ifdef CONFIG_PM
static int nv_pci_device_resume(struct pci_dev *pdev);
#endif
static void nv_ck804_host_stop(struct ata_host *host);
static irqreturn_t nv_generic_interrupt(int irq, void *dev_instance);
static irqreturn_t nv_nf2_interrupt(int irq, void *dev_instance);
static irqreturn_t nv_ck804_interrupt(int irq, void *dev_instance);
static int nv_scr_read(struct ata_link *link, unsigned int sc_reg, u32 *val);
static int nv_scr_write(struct ata_link *link, unsigned int sc_reg, u32 val);

static int nv_hardreset(struct ata_link *link, unsigned int *class,
			unsigned long deadline);
static void nv_nf2_freeze(struct ata_port *ap);
static void nv_nf2_thaw(struct ata_port *ap);
static void nv_ck804_freeze(struct ata_port *ap);
static void nv_ck804_thaw(struct ata_port *ap);
static int nv_adma_slave_config(struct scsi_device *sdev);
static int nv_adma_check_atapi_dma(struct ata_queued_cmd *qc);
static void nv_adma_qc_prep(struct ata_queued_cmd *qc);
static unsigned int nv_adma_qc_issue(struct ata_queued_cmd *qc);
static irqreturn_t nv_adma_interrupt(int irq, void *dev_instance);
static void nv_adma_irq_clear(struct ata_port *ap);
static int nv_adma_port_start(struct ata_port *ap);
static void nv_adma_port_stop(struct ata_port *ap);
#ifdef CONFIG_PM
static int nv_adma_port_suspend(struct ata_port *ap, pm_message_t mesg);
static int nv_adma_port_resume(struct ata_port *ap);
#endif
static void nv_adma_freeze(struct ata_port *ap);
static void nv_adma_thaw(struct ata_port *ap);
static void nv_adma_error_handler(struct ata_port *ap);
static void nv_adma_host_stop(struct ata_host *host);
static void nv_adma_post_internal_cmd(struct ata_queued_cmd *qc);
static void nv_adma_tf_read(struct ata_port *ap, struct ata_taskfile *tf);

static void nv_mcp55_thaw(struct ata_port *ap);
static void nv_mcp55_freeze(struct ata_port *ap);
static void nv_swncq_error_handler(struct ata_port *ap);
static int nv_swncq_slave_config(struct scsi_device *sdev);
static int nv_swncq_port_start(struct ata_port *ap);
static void nv_swncq_qc_prep(struct ata_queued_cmd *qc);
static void nv_swncq_fill_sg(struct ata_queued_cmd *qc);
static unsigned int nv_swncq_qc_issue(struct ata_queued_cmd *qc);
static void nv_swncq_irq_clear(struct ata_port *ap, u16 fis);
static irqreturn_t nv_swncq_interrupt(int irq, void *dev_instance);
#ifdef CONFIG_PM
static int nv_swncq_port_suspend(struct ata_port *ap, pm_message_t mesg);
static int nv_swncq_port_resume(struct ata_port *ap);
#endif

enum nv_host_type
{
	GENERIC,
	NFORCE2,
	NFORCE3 = NFORCE2,	
	CK804,
	ADMA,
	MCP5x,
	SWNCQ,
};

static const struct pci_device_id nv_pci_tbl[] = {
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE2S_SATA), NFORCE2 },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE3S_SATA), NFORCE3 },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE3S_SATA2), NFORCE3 },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_CK804_SATA), CK804 },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_CK804_SATA2), CK804 },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_MCP04_SATA), CK804 },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_MCP04_SATA2), CK804 },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_MCP51_SATA), MCP5x },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_MCP51_SATA2), MCP5x },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_MCP55_SATA), MCP5x },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_MCP55_SATA2), MCP5x },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_MCP61_SATA), GENERIC },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_MCP61_SATA2), GENERIC },
	{ PCI_VDEVICE(NVIDIA, PCI_DEVICE_ID_NVIDIA_NFORCE_MCP61_SATA3), GENERIC },

	{ } 
};

static struct pci_driver nv_pci_driver = {
	.name			= DRV_NAME,
	.id_table		= nv_pci_tbl,
	.probe			= nv_init_one,
#ifdef CONFIG_PM
	.suspend		= ata_pci_device_suspend,
	.resume			= nv_pci_device_resume,
#endif
	.remove			= ata_pci_remove_one,
};

static struct scsi_host_template nv_sht = {
	ATA_BMDMA_SHT(DRV_NAME),
};

static struct scsi_host_template nv_adma_sht = {
	ATA_NCQ_SHT(DRV_NAME),
	.can_queue		= NV_ADMA_MAX_CPBS,
	.sg_tablesize		= NV_ADMA_SGTBL_TOTAL_LEN,
	.dma_boundary		= NV_ADMA_DMA_BOUNDARY,
	.slave_configure	= nv_adma_slave_config,
};

static struct scsi_host_template nv_swncq_sht = {
	ATA_NCQ_SHT(DRV_NAME),
	.can_queue		= ATA_MAX_QUEUE,
	.sg_tablesize		= LIBATA_MAX_PRD,
	.dma_boundary		= ATA_DMA_BOUNDARY,
	.slave_configure	= nv_swncq_slave_config,
};


static struct ata_port_operations nv_generic_ops = {
	.inherits		= &ata_bmdma_port_ops,
	.lost_interrupt		= ATA_OP_NULL,
	.scr_read		= nv_scr_read,
	.scr_write		= nv_scr_write,
	.hardreset		= nv_hardreset,
};

static struct ata_port_operations nv_nf2_ops = {
	.inherits		= &nv_generic_ops,
	.freeze			= nv_nf2_freeze,
	.thaw			= nv_nf2_thaw,
};

static struct ata_port_operations nv_ck804_ops = {
	.inherits		= &nv_generic_ops,
	.freeze			= nv_ck804_freeze,
	.thaw			= nv_ck804_thaw,
	.host_stop		= nv_ck804_host_stop,
};

static struct ata_port_operations nv_adma_ops = {
	.inherits		= &nv_ck804_ops,

	.check_atapi_dma	= nv_adma_check_atapi_dma,
	.sff_tf_read		= nv_adma_tf_read,
	.qc_defer		= ata_std_qc_defer,
	.qc_prep		= nv_adma_qc_prep,
	.qc_issue		= nv_adma_qc_issue,
	.sff_irq_clear		= nv_adma_irq_clear,

	.freeze			= nv_adma_freeze,
	.thaw			= nv_adma_thaw,
	.error_handler		= nv_adma_error_handler,
	.post_internal_cmd	= nv_adma_post_internal_cmd,

	.port_start		= nv_adma_port_start,
	.port_stop		= nv_adma_port_stop,
#ifdef CONFIG_PM
	.port_suspend		= nv_adma_port_suspend,
	.port_resume		= nv_adma_port_resume,
#endif
	.host_stop		= nv_adma_host_stop,
};

static struct ata_port_operations nv_swncq_ops = {
	.inherits		= &nv_generic_ops,

	.qc_defer		= ata_std_qc_defer,
	.qc_prep		= nv_swncq_qc_prep,
	.qc_issue		= nv_swncq_qc_issue,

	.freeze			= nv_mcp55_freeze,
	.thaw			= nv_mcp55_thaw,
	.error_handler		= nv_swncq_error_handler,

#ifdef CONFIG_PM
	.port_suspend		= nv_swncq_port_suspend,
	.port_resume		= nv_swncq_port_resume,
#endif
	.port_start		= nv_swncq_port_start,
};

struct nv_pi_priv {
	irq_handler_t			irq_handler;
	struct scsi_host_template	*sht;
};

#define NV_PI_PRIV(_irq_handler, _sht) \
	&(struct nv_pi_priv){ .irq_handler = _irq_handler, .sht = _sht }

static const struct ata_port_info nv_port_info[] = {
	
	{
		.flags		= ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY,
		.pio_mask	= NV_PIO_MASK,
		.mwdma_mask	= NV_MWDMA_MASK,
		.udma_mask	= NV_UDMA_MASK,
		.port_ops	= &nv_generic_ops,
		.private_data	= NV_PI_PRIV(nv_generic_interrupt, &nv_sht),
	},
	
	{
		.flags		= ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY,
		.pio_mask	= NV_PIO_MASK,
		.mwdma_mask	= NV_MWDMA_MASK,
		.udma_mask	= NV_UDMA_MASK,
		.port_ops	= &nv_nf2_ops,
		.private_data	= NV_PI_PRIV(nv_nf2_interrupt, &nv_sht),
	},
	
	{
		.flags		= ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY,
		.pio_mask	= NV_PIO_MASK,
		.mwdma_mask	= NV_MWDMA_MASK,
		.udma_mask	= NV_UDMA_MASK,
		.port_ops	= &nv_ck804_ops,
		.private_data	= NV_PI_PRIV(nv_ck804_interrupt, &nv_sht),
	},
	
	{
		.flags		= ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY |
				  ATA_FLAG_MMIO | ATA_FLAG_NCQ,
		.pio_mask	= NV_PIO_MASK,
		.mwdma_mask	= NV_MWDMA_MASK,
		.udma_mask	= NV_UDMA_MASK,
		.port_ops	= &nv_adma_ops,
		.private_data	= NV_PI_PRIV(nv_adma_interrupt, &nv_adma_sht),
	},
	
	{
		.flags		= ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY,
		.pio_mask	= NV_PIO_MASK,
		.mwdma_mask	= NV_MWDMA_MASK,
		.udma_mask	= NV_UDMA_MASK,
		.port_ops	= &nv_generic_ops,
		.private_data	= NV_PI_PRIV(nv_generic_interrupt, &nv_sht),
	},
	
	{
		.flags	        = ATA_FLAG_SATA | ATA_FLAG_NO_LEGACY |
				  ATA_FLAG_NCQ,
		.pio_mask	= NV_PIO_MASK,
		.mwdma_mask	= NV_MWDMA_MASK,
		.udma_mask	= NV_UDMA_MASK,
		.port_ops	= &nv_swncq_ops,
		.private_data	= NV_PI_PRIV(nv_swncq_interrupt, &nv_swncq_sht),
	},
};

MODULE_AUTHOR("NVIDIA");
MODULE_DESCRIPTION("low-level driver for NVIDIA nForce SATA controller");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(pci, nv_pci_tbl);
MODULE_VERSION(DRV_VERSION);

static int adma_enabled;
static int swncq_enabled = 1;
static int msi_enabled;

static void nv_adma_register_mode(struct ata_port *ap)
{
	struct nv_adma_port_priv *pp = ap->private_data;
	void __iomem *mmio = pp->ctl_block;
	u16 tmp, status;
	int count = 0;

	if (pp->flags & NV_ADMA_PORT_REGISTER_MODE)
		return;

	status = readw(mmio + NV_ADMA_STAT);
	while (!(status & NV_ADMA_STAT_IDLE) && count < 20) {
		ndelay(50);
		status = readw(mmio + NV_ADMA_STAT);
		count++;
	}
	if (count == 20)
		ata_port_printk(ap, KERN_WARNING,
			"timeout waiting for ADMA IDLE, stat=0x%hx\n",
			status);

	tmp = readw(mmio + NV_ADMA_CTL);
	writew(tmp & ~NV_ADMA_CTL_GO, mmio + NV_ADMA_CTL);

	count = 0;
	status = readw(mmio + NV_ADMA_STAT);
	while (!(status & NV_ADMA_STAT_LEGACY) && count < 20) {
		ndelay(50);
		status = readw(mmio + NV_ADMA_STAT);
		count++;
	}
	if (count == 20)
		ata_port_printk(ap, KERN_WARNING,
			 "timeout waiting for ADMA LEGACY, stat=0x%hx\n",
			 status);

	pp->flags |= NV_ADMA_PORT_REGISTER_MODE;
}

static void nv_adma_mode(struct ata_port *ap)
{
	struct nv_adma_port_priv *pp = ap->private_data;
	void __iomem *mmio = pp->ctl_block;
	u16 tmp, status;
	int count = 0;

	if (!(pp->flags & NV_ADMA_PORT_REGISTER_MODE))
		return;

	WARN_ON(pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE);

	tmp = readw(mmio + NV_ADMA_CTL);
	writew(tmp | NV_ADMA_CTL_GO, mmio + NV_ADMA_CTL);

	status = readw(mmio + NV_ADMA_STAT);
	while (((status & NV_ADMA_STAT_LEGACY) ||
	      !(status & NV_ADMA_STAT_IDLE)) && count < 20) {
		ndelay(50);
		status = readw(mmio + NV_ADMA_STAT);
		count++;
	}
	if (count == 20)
		ata_port_printk(ap, KERN_WARNING,
			"timeout waiting for ADMA LEGACY clear and IDLE, stat=0x%hx\n",
			status);

	pp->flags &= ~NV_ADMA_PORT_REGISTER_MODE;
}

static int nv_adma_slave_config(struct scsi_device *sdev)
{
	struct ata_port *ap = ata_shost_to_port(sdev->host);
	struct nv_adma_port_priv *pp = ap->private_data;
	struct nv_adma_port_priv *port0, *port1;
	struct scsi_device *sdev0, *sdev1;
	struct pci_dev *pdev = to_pci_dev(ap->host->dev);
	unsigned long segment_boundary, flags;
	unsigned short sg_tablesize;
	int rc;
	int adma_enable;
	u32 current_reg, new_reg, config_mask;

	rc = ata_scsi_slave_config(sdev);

	if (sdev->id >= ATA_MAX_DEVICES || sdev->channel || sdev->lun)
		
		return rc;

	spin_lock_irqsave(ap->lock, flags);

	if (ap->link.device[sdev->id].class == ATA_DEV_ATAPI) {
		
		segment_boundary = ATA_DMA_BOUNDARY;
		
		sg_tablesize = LIBATA_MAX_PRD - 1;

		
		adma_enable = 0;
		nv_adma_register_mode(ap);
	} else {
		segment_boundary = NV_ADMA_DMA_BOUNDARY;
		sg_tablesize = NV_ADMA_SGTBL_TOTAL_LEN;
		adma_enable = 1;
	}

	pci_read_config_dword(pdev, NV_MCP_SATA_CFG_20, &current_reg);

	if (ap->port_no == 1)
		config_mask = NV_MCP_SATA_CFG_20_PORT1_EN |
			      NV_MCP_SATA_CFG_20_PORT1_PWB_EN;
	else
		config_mask = NV_MCP_SATA_CFG_20_PORT0_EN |
			      NV_MCP_SATA_CFG_20_PORT0_PWB_EN;

	if (adma_enable) {
		new_reg = current_reg | config_mask;
		pp->flags &= ~NV_ADMA_ATAPI_SETUP_COMPLETE;
	} else {
		new_reg = current_reg & ~config_mask;
		pp->flags |= NV_ADMA_ATAPI_SETUP_COMPLETE;
	}

	if (current_reg != new_reg)
		pci_write_config_dword(pdev, NV_MCP_SATA_CFG_20, new_reg);

	port0 = ap->host->ports[0]->private_data;
	port1 = ap->host->ports[1]->private_data;
	sdev0 = ap->host->ports[0]->link.device[0].sdev;
	sdev1 = ap->host->ports[1]->link.device[0].sdev;
	if ((port0->flags & NV_ADMA_ATAPI_SETUP_COMPLETE) ||
	    (port1->flags & NV_ADMA_ATAPI_SETUP_COMPLETE)) {
		
		if (sdev0)
			blk_queue_bounce_limit(sdev0->request_queue,
					       ATA_DMA_MASK);
		if (sdev1)
			blk_queue_bounce_limit(sdev1->request_queue,
					       ATA_DMA_MASK);

		pci_set_dma_mask(pdev, ATA_DMA_MASK);
	} else {
		
		pci_set_dma_mask(pdev, pp->adma_dma_mask);
		if (sdev0)
			blk_queue_bounce_limit(sdev0->request_queue,
					       pp->adma_dma_mask);
		if (sdev1)
			blk_queue_bounce_limit(sdev1->request_queue,
					       pp->adma_dma_mask);
	}

	blk_queue_segment_boundary(sdev->request_queue, segment_boundary);
	blk_queue_max_hw_segments(sdev->request_queue, sg_tablesize);
	ata_port_printk(ap, KERN_INFO,
		"DMA mask 0x%llX, segment boundary 0x%lX, hw segs %hu\n",
		(unsigned long long)*ap->host->dev->dma_mask,
		segment_boundary, sg_tablesize);

	spin_unlock_irqrestore(ap->lock, flags);

	return rc;
}

static int nv_adma_check_atapi_dma(struct ata_queued_cmd *qc)
{
	struct nv_adma_port_priv *pp = qc->ap->private_data;
	return !(pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE);
}

static void nv_adma_tf_read(struct ata_port *ap, struct ata_taskfile *tf)
{
	
	nv_adma_register_mode(ap);

	ata_sff_tf_read(ap, tf);
}

static unsigned int nv_adma_tf_to_cpb(struct ata_taskfile *tf, __le16 *cpb)
{
	unsigned int idx = 0;

	if (tf->flags & ATA_TFLAG_ISADDR) {
		if (tf->flags & ATA_TFLAG_LBA48) {
			cpb[idx++] = cpu_to_le16((ATA_REG_ERR   << 8) | tf->hob_feature | WNB);
			cpb[idx++] = cpu_to_le16((ATA_REG_NSECT << 8) | tf->hob_nsect);
			cpb[idx++] = cpu_to_le16((ATA_REG_LBAL  << 8) | tf->hob_lbal);
			cpb[idx++] = cpu_to_le16((ATA_REG_LBAM  << 8) | tf->hob_lbam);
			cpb[idx++] = cpu_to_le16((ATA_REG_LBAH  << 8) | tf->hob_lbah);
			cpb[idx++] = cpu_to_le16((ATA_REG_ERR    << 8) | tf->feature);
		} else
			cpb[idx++] = cpu_to_le16((ATA_REG_ERR    << 8) | tf->feature | WNB);

		cpb[idx++] = cpu_to_le16((ATA_REG_NSECT  << 8) | tf->nsect);
		cpb[idx++] = cpu_to_le16((ATA_REG_LBAL   << 8) | tf->lbal);
		cpb[idx++] = cpu_to_le16((ATA_REG_LBAM   << 8) | tf->lbam);
		cpb[idx++] = cpu_to_le16((ATA_REG_LBAH   << 8) | tf->lbah);
	}

	if (tf->flags & ATA_TFLAG_DEVICE)
		cpb[idx++] = cpu_to_le16((ATA_REG_DEVICE << 8) | tf->device);

	cpb[idx++] = cpu_to_le16((ATA_REG_CMD    << 8) | tf->command | CMDEND);

	while (idx < 12)
		cpb[idx++] = cpu_to_le16(IGN);

	return idx;
}

static int nv_adma_check_cpb(struct ata_port *ap, int cpb_num, int force_err)
{
	struct nv_adma_port_priv *pp = ap->private_data;
	u8 flags = pp->cpb[cpb_num].resp_flags;

	VPRINTK("CPB %d, flags=0x%x\n", cpb_num, flags);

	if (unlikely((force_err ||
		     flags & (NV_CPB_RESP_ATA_ERR |
			      NV_CPB_RESP_CMD_ERR |
			      NV_CPB_RESP_CPB_ERR)))) {
		struct ata_eh_info *ehi = &ap->link.eh_info;
		int freeze = 0;

		ata_ehi_clear_desc(ehi);
		__ata_ehi_push_desc(ehi, "CPB resp_flags 0x%x: ", flags);
		if (flags & NV_CPB_RESP_ATA_ERR) {
			ata_ehi_push_desc(ehi, "ATA error");
			ehi->err_mask |= AC_ERR_DEV;
		} else if (flags & NV_CPB_RESP_CMD_ERR) {
			ata_ehi_push_desc(ehi, "CMD error");
			ehi->err_mask |= AC_ERR_DEV;
		} else if (flags & NV_CPB_RESP_CPB_ERR) {
			ata_ehi_push_desc(ehi, "CPB error");
			ehi->err_mask |= AC_ERR_SYSTEM;
			freeze = 1;
		} else {
			
			ata_ehi_push_desc(ehi, "unknown");
			ehi->err_mask |= AC_ERR_OTHER;
			freeze = 1;
		}
		
		if (freeze)
			ata_port_freeze(ap);
		else
			ata_port_abort(ap);
		return 1;
	}

	if (likely(flags & NV_CPB_RESP_DONE)) {
		struct ata_queued_cmd *qc = ata_qc_from_tag(ap, cpb_num);
		VPRINTK("CPB flags done, flags=0x%x\n", flags);
		if (likely(qc)) {
			DPRINTK("Completing qc from tag %d\n", cpb_num);
			ata_qc_complete(qc);
		} else {
			struct ata_eh_info *ehi = &ap->link.eh_info;
			
			ata_port_printk(ap, KERN_ERR,
					"notifier for tag %d with no cmd?\n",
					cpb_num);
			ehi->err_mask |= AC_ERR_HSM;
			ehi->action |= ATA_EH_RESET;
			ata_port_freeze(ap);
			return 1;
		}
	}
	return 0;
}

static int nv_host_intr(struct ata_port *ap, u8 irq_stat)
{
	struct ata_queued_cmd *qc = ata_qc_from_tag(ap, ap->link.active_tag);

	
	if (unlikely(irq_stat & (NV_INT_ADDED | NV_INT_REMOVED))) {
		ata_port_freeze(ap);
		return 1;
	}

	
	if (!(irq_stat & NV_INT_DEV))
		return 0;

	
	if (unlikely(!qc || (qc->tf.flags & ATA_TFLAG_POLLING))) {
		ata_sff_check_status(ap);
		return 1;
	}

	
	return ata_sff_host_intr(ap, qc);
}

static irqreturn_t nv_adma_interrupt(int irq, void *dev_instance)
{
	struct ata_host *host = dev_instance;
	int i, handled = 0;
	u32 notifier_clears[2];

	spin_lock(&host->lock);

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];
		notifier_clears[i] = 0;

		if (ap && !(ap->flags & ATA_FLAG_DISABLED)) {
			struct nv_adma_port_priv *pp = ap->private_data;
			void __iomem *mmio = pp->ctl_block;
			u16 status;
			u32 gen_ctl;
			u32 notifier, notifier_error;

			
			if (pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE) {
				u8 irq_stat = readb(host->iomap[NV_MMIO_BAR] + NV_INT_STATUS_CK804)
					>> (NV_INT_PORT_SHIFT * i);
				handled += nv_host_intr(ap, irq_stat);
				continue;
			}

			
			if (pp->flags & NV_ADMA_PORT_REGISTER_MODE) {
				u8 irq_stat = readb(host->iomap[NV_MMIO_BAR] + NV_INT_STATUS_CK804)
					>> (NV_INT_PORT_SHIFT * i);
				if (ata_tag_valid(ap->link.active_tag))
					
					irq_stat |= NV_INT_DEV;
				handled += nv_host_intr(ap, irq_stat);
			}

			notifier = readl(mmio + NV_ADMA_NOTIFIER);
			notifier_error = readl(mmio + NV_ADMA_NOTIFIER_ERROR);
			notifier_clears[i] = notifier | notifier_error;

			gen_ctl = readl(pp->gen_block + NV_ADMA_GEN_CTL);

			if (!NV_ADMA_CHECK_INTR(gen_ctl, ap->port_no) && !notifier &&
			    !notifier_error)
				
				continue;

			status = readw(mmio + NV_ADMA_STAT);

			
			writew(status, mmio + NV_ADMA_STAT);
			readw(mmio + NV_ADMA_STAT); 
			rmb();

			handled++; 

			
			if (unlikely(status & (NV_ADMA_STAT_HOTPLUG |
					       NV_ADMA_STAT_HOTUNPLUG |
					       NV_ADMA_STAT_TIMEOUT |
					       NV_ADMA_STAT_SERROR))) {
				struct ata_eh_info *ehi = &ap->link.eh_info;

				ata_ehi_clear_desc(ehi);
				__ata_ehi_push_desc(ehi, "ADMA status 0x%08x: ", status);
				if (status & NV_ADMA_STAT_TIMEOUT) {
					ehi->err_mask |= AC_ERR_SYSTEM;
					ata_ehi_push_desc(ehi, "timeout");
				} else if (status & NV_ADMA_STAT_HOTPLUG) {
					ata_ehi_hotplugged(ehi);
					ata_ehi_push_desc(ehi, "hotplug");
				} else if (status & NV_ADMA_STAT_HOTUNPLUG) {
					ata_ehi_hotplugged(ehi);
					ata_ehi_push_desc(ehi, "hot unplug");
				} else if (status & NV_ADMA_STAT_SERROR) {
					
					ata_ehi_push_desc(ehi, "SError");
				} else
					ata_ehi_push_desc(ehi, "unknown");
				ata_port_freeze(ap);
				continue;
			}

			if (status & (NV_ADMA_STAT_DONE |
				      NV_ADMA_STAT_CPBERR |
				      NV_ADMA_STAT_CMD_COMPLETE)) {
				u32 check_commands = notifier_clears[i];
				int pos, error = 0;

				if (status & NV_ADMA_STAT_CPBERR) {
					
					if (ata_tag_valid(ap->link.active_tag))
						check_commands = 1 <<
							ap->link.active_tag;
					else
						check_commands = ap->
							link.sactive;
				}

				
				while ((pos = ffs(check_commands)) && !error) {
					pos--;
					error = nv_adma_check_cpb(ap, pos,
						notifier_error & (1 << pos));
					check_commands &= ~(1 << pos);
				}
			}
		}
	}

	if (notifier_clears[0] || notifier_clears[1]) {
		
		struct nv_adma_port_priv *pp = host->ports[0]->private_data;
		writel(notifier_clears[0], pp->notifier_clear_block);
		pp = host->ports[1]->private_data;
		writel(notifier_clears[1], pp->notifier_clear_block);
	}

	spin_unlock(&host->lock);

	return IRQ_RETVAL(handled);
}

static void nv_adma_freeze(struct ata_port *ap)
{
	struct nv_adma_port_priv *pp = ap->private_data;
	void __iomem *mmio = pp->ctl_block;
	u16 tmp;

	nv_ck804_freeze(ap);

	if (pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE)
		return;

	
	writeb(NV_INT_ALL << (ap->port_no * NV_INT_PORT_SHIFT),
		ap->host->iomap[NV_MMIO_BAR] + NV_INT_STATUS_CK804);

	
	tmp = readw(mmio + NV_ADMA_CTL);
	writew(tmp & ~(NV_ADMA_CTL_AIEN | NV_ADMA_CTL_HOTPLUG_IEN),
		mmio + NV_ADMA_CTL);
	readw(mmio + NV_ADMA_CTL);	
}

static void nv_adma_thaw(struct ata_port *ap)
{
	struct nv_adma_port_priv *pp = ap->private_data;
	void __iomem *mmio = pp->ctl_block;
	u16 tmp;

	nv_ck804_thaw(ap);

	if (pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE)
		return;

	
	tmp = readw(mmio + NV_ADMA_CTL);
	writew(tmp | (NV_ADMA_CTL_AIEN | NV_ADMA_CTL_HOTPLUG_IEN),
		mmio + NV_ADMA_CTL);
	readw(mmio + NV_ADMA_CTL);	
}

static void nv_adma_irq_clear(struct ata_port *ap)
{
	struct nv_adma_port_priv *pp = ap->private_data;
	void __iomem *mmio = pp->ctl_block;
	u32 notifier_clears[2];

	if (pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE) {
		ata_sff_irq_clear(ap);
		return;
	}

	
	writeb(NV_INT_ALL << (ap->port_no * NV_INT_PORT_SHIFT),
		ap->host->iomap[NV_MMIO_BAR] + NV_INT_STATUS_CK804);

	
	writew(0xffff, mmio + NV_ADMA_STAT);

	
	if (ap->port_no == 0) {
		notifier_clears[0] = 0xFFFFFFFF;
		notifier_clears[1] = 0;
	} else {
		notifier_clears[0] = 0;
		notifier_clears[1] = 0xFFFFFFFF;
	}
	pp = ap->host->ports[0]->private_data;
	writel(notifier_clears[0], pp->notifier_clear_block);
	pp = ap->host->ports[1]->private_data;
	writel(notifier_clears[1], pp->notifier_clear_block);
}

static void nv_adma_post_internal_cmd(struct ata_queued_cmd *qc)
{
	struct nv_adma_port_priv *pp = qc->ap->private_data;

	if (pp->flags & NV_ADMA_PORT_REGISTER_MODE)
		ata_sff_post_internal_cmd(qc);
}

static int nv_adma_port_start(struct ata_port *ap)
{
	struct device *dev = ap->host->dev;
	struct nv_adma_port_priv *pp;
	int rc;
	void *mem;
	dma_addr_t mem_dma;
	void __iomem *mmio;
	struct pci_dev *pdev = to_pci_dev(dev);
	u16 tmp;

	VPRINTK("ENTER\n");

	
	rc = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
	if (rc)
		return rc;
	rc = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
	if (rc)
		return rc;

	rc = ata_port_start(ap);
	if (rc)
		return rc;

	pp = devm_kzalloc(dev, sizeof(*pp), GFP_KERNEL);
	if (!pp)
		return -ENOMEM;

	mmio = ap->host->iomap[NV_MMIO_BAR] + NV_ADMA_PORT +
	       ap->port_no * NV_ADMA_PORT_SIZE;
	pp->ctl_block = mmio;
	pp->gen_block = ap->host->iomap[NV_MMIO_BAR] + NV_ADMA_GEN;
	pp->notifier_clear_block = pp->gen_block +
	       NV_ADMA_NOTIFIER_CLEAR + (4 * ap->port_no);

	
	pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
	pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64));
	pp->adma_dma_mask = *dev->dma_mask;

	mem = dmam_alloc_coherent(dev, NV_ADMA_PORT_PRIV_DMA_SZ,
				  &mem_dma, GFP_KERNEL);
	if (!mem)
		return -ENOMEM;
	memset(mem, 0, NV_ADMA_PORT_PRIV_DMA_SZ);

	
	pp->cpb     = mem;
	pp->cpb_dma = mem_dma;

	writel(mem_dma & 0xFFFFFFFF, 	mmio + NV_ADMA_CPB_BASE_LOW);
	writel((mem_dma >> 16) >> 16,	mmio + NV_ADMA_CPB_BASE_HIGH);

	mem     += NV_ADMA_MAX_CPBS * NV_ADMA_CPB_SZ;
	mem_dma += NV_ADMA_MAX_CPBS * NV_ADMA_CPB_SZ;

	
	pp->aprd = mem;
	pp->aprd_dma = mem_dma;

	ap->private_data = pp;

	
	writew(0xffff, mmio + NV_ADMA_STAT);

	
	pp->flags = NV_ADMA_PORT_REGISTER_MODE;

	
	writew(0, mmio + NV_ADMA_CPB_COUNT);

	
	tmp = readw(mmio + NV_ADMA_CTL);
	writew((tmp & ~NV_ADMA_CTL_GO) | NV_ADMA_CTL_AIEN |
		NV_ADMA_CTL_HOTPLUG_IEN, mmio + NV_ADMA_CTL);

	tmp = readw(mmio + NV_ADMA_CTL);
	writew(tmp | NV_ADMA_CTL_CHANNEL_RESET, mmio + NV_ADMA_CTL);
	readw(mmio + NV_ADMA_CTL);	
	udelay(1);
	writew(tmp & ~NV_ADMA_CTL_CHANNEL_RESET, mmio + NV_ADMA_CTL);
	readw(mmio + NV_ADMA_CTL);	

	return 0;
}

static void nv_adma_port_stop(struct ata_port *ap)
{
	struct nv_adma_port_priv *pp = ap->private_data;
	void __iomem *mmio = pp->ctl_block;

	VPRINTK("ENTER\n");
	writew(0, mmio + NV_ADMA_CTL);
}

#ifdef CONFIG_PM
static int nv_adma_port_suspend(struct ata_port *ap, pm_message_t mesg)
{
	struct nv_adma_port_priv *pp = ap->private_data;
	void __iomem *mmio = pp->ctl_block;

	
	nv_adma_register_mode(ap);

	
	writew(0, mmio + NV_ADMA_CPB_COUNT);

	
	writew(0, mmio + NV_ADMA_CTL);

	return 0;
}

static int nv_adma_port_resume(struct ata_port *ap)
{
	struct nv_adma_port_priv *pp = ap->private_data;
	void __iomem *mmio = pp->ctl_block;
	u16 tmp;

	
	writel(pp->cpb_dma & 0xFFFFFFFF, 	mmio + NV_ADMA_CPB_BASE_LOW);
	writel((pp->cpb_dma >> 16) >> 16,	mmio + NV_ADMA_CPB_BASE_HIGH);

	
	writew(0xffff, mmio + NV_ADMA_STAT);

	
	pp->flags |= NV_ADMA_PORT_REGISTER_MODE;

	
	writew(0, mmio + NV_ADMA_CPB_COUNT);

	
	tmp = readw(mmio + NV_ADMA_CTL);
	writew((tmp & ~NV_ADMA_CTL_GO) | NV_ADMA_CTL_AIEN |
		NV_ADMA_CTL_HOTPLUG_IEN, mmio + NV_ADMA_CTL);

	tmp = readw(mmio + NV_ADMA_CTL);
	writew(tmp | NV_ADMA_CTL_CHANNEL_RESET, mmio + NV_ADMA_CTL);
	readw(mmio + NV_ADMA_CTL);	
	udelay(1);
	writew(tmp & ~NV_ADMA_CTL_CHANNEL_RESET, mmio + NV_ADMA_CTL);
	readw(mmio + NV_ADMA_CTL);	

	return 0;
}
#endif

static void nv_adma_setup_port(struct ata_port *ap)
{
	void __iomem *mmio = ap->host->iomap[NV_MMIO_BAR];
	struct ata_ioports *ioport = &ap->ioaddr;

	VPRINTK("ENTER\n");

	mmio += NV_ADMA_PORT + ap->port_no * NV_ADMA_PORT_SIZE;

	ioport->cmd_addr	= mmio;
	ioport->data_addr	= mmio + (ATA_REG_DATA * 4);
	ioport->error_addr	=
	ioport->feature_addr	= mmio + (ATA_REG_ERR * 4);
	ioport->nsect_addr	= mmio + (ATA_REG_NSECT * 4);
	ioport->lbal_addr	= mmio + (ATA_REG_LBAL * 4);
	ioport->lbam_addr	= mmio + (ATA_REG_LBAM * 4);
	ioport->lbah_addr	= mmio + (ATA_REG_LBAH * 4);
	ioport->device_addr	= mmio + (ATA_REG_DEVICE * 4);
	ioport->status_addr	=
	ioport->command_addr	= mmio + (ATA_REG_STATUS * 4);
	ioport->altstatus_addr	=
	ioport->ctl_addr	= mmio + 0x20;
}

static int nv_adma_host_init(struct ata_host *host)
{
	struct pci_dev *pdev = to_pci_dev(host->dev);
	unsigned int i;
	u32 tmp32;

	VPRINTK("ENTER\n");

	
	pci_read_config_dword(pdev, NV_MCP_SATA_CFG_20, &tmp32);
	tmp32 |= NV_MCP_SATA_CFG_20_PORT0_EN |
		 NV_MCP_SATA_CFG_20_PORT0_PWB_EN |
		 NV_MCP_SATA_CFG_20_PORT1_EN |
		 NV_MCP_SATA_CFG_20_PORT1_PWB_EN;

	pci_write_config_dword(pdev, NV_MCP_SATA_CFG_20, tmp32);

	for (i = 0; i < host->n_ports; i++)
		nv_adma_setup_port(host->ports[i]);

	return 0;
}

static void nv_adma_fill_aprd(struct ata_queued_cmd *qc,
			      struct scatterlist *sg,
			      int idx,
			      struct nv_adma_prd *aprd)
{
	u8 flags = 0;
	if (qc->tf.flags & ATA_TFLAG_WRITE)
		flags |= NV_APRD_WRITE;
	if (idx == qc->n_elem - 1)
		flags |= NV_APRD_END;
	else if (idx != 4)
		flags |= NV_APRD_CONT;

	aprd->addr  = cpu_to_le64(((u64)sg_dma_address(sg)));
	aprd->len   = cpu_to_le32(((u32)sg_dma_len(sg))); 
	aprd->flags = flags;
	aprd->packet_len = 0;
}

static void nv_adma_fill_sg(struct ata_queued_cmd *qc, struct nv_adma_cpb *cpb)
{
	struct nv_adma_port_priv *pp = qc->ap->private_data;
	struct nv_adma_prd *aprd;
	struct scatterlist *sg;
	unsigned int si;

	VPRINTK("ENTER\n");

	for_each_sg(qc->sg, sg, qc->n_elem, si) {
		aprd = (si < 5) ? &cpb->aprd[si] :
			       &pp->aprd[NV_ADMA_SGTBL_LEN * qc->tag + (si-5)];
		nv_adma_fill_aprd(qc, sg, si, aprd);
	}
	if (si > 5)
		cpb->next_aprd = cpu_to_le64(((u64)(pp->aprd_dma + NV_ADMA_SGTBL_SZ * qc->tag)));
	else
		cpb->next_aprd = cpu_to_le64(0);
}

static int nv_adma_use_reg_mode(struct ata_queued_cmd *qc)
{
	struct nv_adma_port_priv *pp = qc->ap->private_data;

	
	if ((pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE) ||
	   (qc->tf.flags & ATA_TFLAG_POLLING))
		return 1;

	if ((qc->flags & ATA_QCFLAG_DMAMAP) ||
	   (qc->tf.protocol == ATA_PROT_NODATA))
		return 0;

	return 1;
}

static void nv_adma_qc_prep(struct ata_queued_cmd *qc)
{
	struct nv_adma_port_priv *pp = qc->ap->private_data;
	struct nv_adma_cpb *cpb = &pp->cpb[qc->tag];
	u8 ctl_flags = NV_CPB_CTL_CPB_VALID |
		       NV_CPB_CTL_IEN;

	if (nv_adma_use_reg_mode(qc)) {
		BUG_ON(!(pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE) &&
			(qc->flags & ATA_QCFLAG_DMAMAP));
		nv_adma_register_mode(qc->ap);
		ata_sff_qc_prep(qc);
		return;
	}

	cpb->resp_flags = NV_CPB_RESP_DONE;
	wmb();
	cpb->ctl_flags = 0;
	wmb();

	cpb->len		= 3;
	cpb->tag		= qc->tag;
	cpb->next_cpb_idx	= 0;

	
	if (qc->tf.protocol == ATA_PROT_NCQ)
		ctl_flags |= NV_CPB_CTL_QUEUE | NV_CPB_CTL_FPDMA;

	VPRINTK("qc->flags = 0x%lx\n", qc->flags);

	nv_adma_tf_to_cpb(&qc->tf, cpb->tf);

	if (qc->flags & ATA_QCFLAG_DMAMAP) {
		nv_adma_fill_sg(qc, cpb);
		ctl_flags |= NV_CPB_CTL_APRD_VALID;
	} else
		memset(&cpb->aprd[0], 0, sizeof(struct nv_adma_prd) * 5);

	
	wmb();
	cpb->ctl_flags = ctl_flags;
	wmb();
	cpb->resp_flags = 0;
}

static unsigned int nv_adma_qc_issue(struct ata_queued_cmd *qc)
{
	struct nv_adma_port_priv *pp = qc->ap->private_data;
	void __iomem *mmio = pp->ctl_block;
	int curr_ncq = (qc->tf.protocol == ATA_PROT_NCQ);

	VPRINTK("ENTER\n");

	
	if (unlikely(qc->tf.protocol == ATA_PROT_NCQ &&
		     (qc->flags & ATA_QCFLAG_RESULT_TF))) {
		ata_dev_printk(qc->dev, KERN_ERR,
			"NCQ w/ RESULT_TF not allowed\n");
		return AC_ERR_SYSTEM;
	}

	if (nv_adma_use_reg_mode(qc)) {
		
		VPRINTK("using ATA register mode: 0x%lx\n", qc->flags);
		BUG_ON(!(pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE) &&
			(qc->flags & ATA_QCFLAG_DMAMAP));
		nv_adma_register_mode(qc->ap);
		return ata_sff_qc_issue(qc);
	} else
		nv_adma_mode(qc->ap);

	
	wmb();

	if (curr_ncq != pp->last_issue_ncq) {
		
		udelay(20);
		pp->last_issue_ncq = curr_ncq;
	}

	writew(qc->tag, mmio + NV_ADMA_APPEND);

	DPRINTK("Issued tag %u\n", qc->tag);

	return 0;
}

static irqreturn_t nv_generic_interrupt(int irq, void *dev_instance)
{
	struct ata_host *host = dev_instance;
	unsigned int i;
	unsigned int handled = 0;
	unsigned long flags;

	spin_lock_irqsave(&host->lock, flags);

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap;

		ap = host->ports[i];
		if (ap &&
		    !(ap->flags & ATA_FLAG_DISABLED)) {
			struct ata_queued_cmd *qc;

			qc = ata_qc_from_tag(ap, ap->link.active_tag);
			if (qc && (!(qc->tf.flags & ATA_TFLAG_POLLING)))
				handled += ata_sff_host_intr(ap, qc);
			else
				
				
				ap->ops->sff_check_status(ap);
		}

	}

	spin_unlock_irqrestore(&host->lock, flags);

	return IRQ_RETVAL(handled);
}

static irqreturn_t nv_do_interrupt(struct ata_host *host, u8 irq_stat)
{
	int i, handled = 0;

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];

		if (ap && !(ap->flags & ATA_FLAG_DISABLED))
			handled += nv_host_intr(ap, irq_stat);

		irq_stat >>= NV_INT_PORT_SHIFT;
	}

	return IRQ_RETVAL(handled);
}

static irqreturn_t nv_nf2_interrupt(int irq, void *dev_instance)
{
	struct ata_host *host = dev_instance;
	u8 irq_stat;
	irqreturn_t ret;

	spin_lock(&host->lock);
	irq_stat = ioread8(host->ports[0]->ioaddr.scr_addr + NV_INT_STATUS);
	ret = nv_do_interrupt(host, irq_stat);
	spin_unlock(&host->lock);

	return ret;
}

static irqreturn_t nv_ck804_interrupt(int irq, void *dev_instance)
{
	struct ata_host *host = dev_instance;
	u8 irq_stat;
	irqreturn_t ret;

	spin_lock(&host->lock);
	irq_stat = readb(host->iomap[NV_MMIO_BAR] + NV_INT_STATUS_CK804);
	ret = nv_do_interrupt(host, irq_stat);
	spin_unlock(&host->lock);

	return ret;
}

static int nv_scr_read(struct ata_link *link, unsigned int sc_reg, u32 *val)
{
	if (sc_reg > SCR_CONTROL)
		return -EINVAL;

	*val = ioread32(link->ap->ioaddr.scr_addr + (sc_reg * 4));
	return 0;
}

static int nv_scr_write(struct ata_link *link, unsigned int sc_reg, u32 val)
{
	if (sc_reg > SCR_CONTROL)
		return -EINVAL;

	iowrite32(val, link->ap->ioaddr.scr_addr + (sc_reg * 4));
	return 0;
}

static int nv_hardreset(struct ata_link *link, unsigned int *class,
			unsigned long deadline)
{
	struct ata_eh_context *ehc = &link->eh_context;

	
	if (!(link->ap->pflags & ATA_PFLAG_LOADING) &&
	    !ata_dev_enabled(link->device))
		sata_link_hardreset(link, sata_deb_timing_hotplug, deadline,
				    NULL, NULL);
	else {
		const unsigned long *timing = sata_ehc_deb_timing(ehc);
		int rc;

		if (!(ehc->i.flags & ATA_EHI_QUIET))
			ata_link_printk(link, KERN_INFO, "nv: skipping "
					"hardreset on occupied port\n");

		
		rc = sata_link_resume(link, timing, deadline);
		
		if (rc && rc != -EOPNOTSUPP)
			ata_link_printk(link, KERN_WARNING, "failed to resume "
					"link (errno=%d)\n", rc);
	}

	
	return -EAGAIN;
}

static void nv_nf2_freeze(struct ata_port *ap)
{
	void __iomem *scr_addr = ap->host->ports[0]->ioaddr.scr_addr;
	int shift = ap->port_no * NV_INT_PORT_SHIFT;
	u8 mask;

	mask = ioread8(scr_addr + NV_INT_ENABLE);
	mask &= ~(NV_INT_ALL << shift);
	iowrite8(mask, scr_addr + NV_INT_ENABLE);
}

static void nv_nf2_thaw(struct ata_port *ap)
{
	void __iomem *scr_addr = ap->host->ports[0]->ioaddr.scr_addr;
	int shift = ap->port_no * NV_INT_PORT_SHIFT;
	u8 mask;

	iowrite8(NV_INT_ALL << shift, scr_addr + NV_INT_STATUS);

	mask = ioread8(scr_addr + NV_INT_ENABLE);
	mask |= (NV_INT_MASK << shift);
	iowrite8(mask, scr_addr + NV_INT_ENABLE);
}

static void nv_ck804_freeze(struct ata_port *ap)
{
	void __iomem *mmio_base = ap->host->iomap[NV_MMIO_BAR];
	int shift = ap->port_no * NV_INT_PORT_SHIFT;
	u8 mask;

	mask = readb(mmio_base + NV_INT_ENABLE_CK804);
	mask &= ~(NV_INT_ALL << shift);
	writeb(mask, mmio_base + NV_INT_ENABLE_CK804);
}

static void nv_ck804_thaw(struct ata_port *ap)
{
	void __iomem *mmio_base = ap->host->iomap[NV_MMIO_BAR];
	int shift = ap->port_no * NV_INT_PORT_SHIFT;
	u8 mask;

	writeb(NV_INT_ALL << shift, mmio_base + NV_INT_STATUS_CK804);

	mask = readb(mmio_base + NV_INT_ENABLE_CK804);
	mask |= (NV_INT_MASK << shift);
	writeb(mask, mmio_base + NV_INT_ENABLE_CK804);
}

static void nv_mcp55_freeze(struct ata_port *ap)
{
	void __iomem *mmio_base = ap->host->iomap[NV_MMIO_BAR];
	int shift = ap->port_no * NV_INT_PORT_SHIFT_MCP55;
	u32 mask;

	writel(NV_INT_ALL_MCP55 << shift, mmio_base + NV_INT_STATUS_MCP55);

	mask = readl(mmio_base + NV_INT_ENABLE_MCP55);
	mask &= ~(NV_INT_ALL_MCP55 << shift);
	writel(mask, mmio_base + NV_INT_ENABLE_MCP55);
	ata_sff_freeze(ap);
}

static void nv_mcp55_thaw(struct ata_port *ap)
{
	void __iomem *mmio_base = ap->host->iomap[NV_MMIO_BAR];
	int shift = ap->port_no * NV_INT_PORT_SHIFT_MCP55;
	u32 mask;

	writel(NV_INT_ALL_MCP55 << shift, mmio_base + NV_INT_STATUS_MCP55);

	mask = readl(mmio_base + NV_INT_ENABLE_MCP55);
	mask |= (NV_INT_MASK_MCP55 << shift);
	writel(mask, mmio_base + NV_INT_ENABLE_MCP55);
	ata_sff_thaw(ap);
}

static void nv_adma_error_handler(struct ata_port *ap)
{
	struct nv_adma_port_priv *pp = ap->private_data;
	if (!(pp->flags & NV_ADMA_PORT_REGISTER_MODE)) {
		void __iomem *mmio = pp->ctl_block;
		int i;
		u16 tmp;

		if (ata_tag_valid(ap->link.active_tag) || ap->link.sactive) {
			u32 notifier = readl(mmio + NV_ADMA_NOTIFIER);
			u32 notifier_error = readl(mmio + NV_ADMA_NOTIFIER_ERROR);
			u32 gen_ctl = readl(pp->gen_block + NV_ADMA_GEN_CTL);
			u32 status = readw(mmio + NV_ADMA_STAT);
			u8 cpb_count = readb(mmio + NV_ADMA_CPB_COUNT);
			u8 next_cpb_idx = readb(mmio + NV_ADMA_NEXT_CPB_IDX);

			ata_port_printk(ap, KERN_ERR,
				"EH in ADMA mode, notifier 0x%X "
				"notifier_error 0x%X gen_ctl 0x%X status 0x%X "
				"next cpb count 0x%X next cpb idx 0x%x\n",
				notifier, notifier_error, gen_ctl, status,
				cpb_count, next_cpb_idx);

			for (i = 0; i < NV_ADMA_MAX_CPBS; i++) {
				struct nv_adma_cpb *cpb = &pp->cpb[i];
				if ((ata_tag_valid(ap->link.active_tag) && i == ap->link.active_tag) ||
				    ap->link.sactive & (1 << i))
					ata_port_printk(ap, KERN_ERR,
						"CPB %d: ctl_flags 0x%x, resp_flags 0x%x\n",
						i, cpb->ctl_flags, cpb->resp_flags);
			}
		}

		
		nv_adma_register_mode(ap);

		
		for (i = 0; i < NV_ADMA_MAX_CPBS; i++)
			pp->cpb[i].ctl_flags &= ~NV_CPB_CTL_CPB_VALID;

		
		writew(0, mmio + NV_ADMA_CPB_COUNT);

		
		tmp = readw(mmio + NV_ADMA_CTL);
		writew(tmp | NV_ADMA_CTL_CHANNEL_RESET, mmio + NV_ADMA_CTL);
		readw(mmio + NV_ADMA_CTL);	
		udelay(1);
		writew(tmp & ~NV_ADMA_CTL_CHANNEL_RESET, mmio + NV_ADMA_CTL);
		readw(mmio + NV_ADMA_CTL);	
	}

	ata_sff_error_handler(ap);
}

static void nv_swncq_qc_to_dq(struct ata_port *ap, struct ata_queued_cmd *qc)
{
	struct nv_swncq_port_priv *pp = ap->private_data;
	struct defer_queue *dq = &pp->defer_queue;

	
	WARN_ON(dq->tail - dq->head == ATA_MAX_QUEUE);
	dq->defer_bits |= (1 << qc->tag);
	dq->tag[dq->tail++ & (ATA_MAX_QUEUE - 1)] = qc->tag;
}

static struct ata_queued_cmd *nv_swncq_qc_from_dq(struct ata_port *ap)
{
	struct nv_swncq_port_priv *pp = ap->private_data;
	struct defer_queue *dq = &pp->defer_queue;
	unsigned int tag;

	if (dq->head == dq->tail)	
		return NULL;

	tag = dq->tag[dq->head & (ATA_MAX_QUEUE - 1)];
	dq->tag[dq->head++ & (ATA_MAX_QUEUE - 1)] = ATA_TAG_POISON;
	WARN_ON(!(dq->defer_bits & (1 << tag)));
	dq->defer_bits &= ~(1 << tag);

	return ata_qc_from_tag(ap, tag);
}

static void nv_swncq_fis_reinit(struct ata_port *ap)
{
	struct nv_swncq_port_priv *pp = ap->private_data;

	pp->dhfis_bits = 0;
	pp->dmafis_bits = 0;
	pp->sdbfis_bits = 0;
	pp->ncq_flags = 0;
}

static void nv_swncq_pp_reinit(struct ata_port *ap)
{
	struct nv_swncq_port_priv *pp = ap->private_data;
	struct defer_queue *dq = &pp->defer_queue;

	dq->head = 0;
	dq->tail = 0;
	dq->defer_bits = 0;
	pp->qc_active = 0;
	pp->last_issue_tag = ATA_TAG_POISON;
	nv_swncq_fis_reinit(ap);
}

static void nv_swncq_irq_clear(struct ata_port *ap, u16 fis)
{
	struct nv_swncq_port_priv *pp = ap->private_data;

	writew(fis, pp->irq_block);
}

static void __ata_bmdma_stop(struct ata_port *ap)
{
	struct ata_queued_cmd qc;

	qc.ap = ap;
	ata_bmdma_stop(&qc);
}

static void nv_swncq_ncq_stop(struct ata_port *ap)
{
	struct nv_swncq_port_priv *pp = ap->private_data;
	unsigned int i;
	u32 sactive;
	u32 done_mask;

	ata_port_printk(ap, KERN_ERR,
			"EH in SWNCQ mode,QC:qc_active 0x%X sactive 0x%X\n",
			ap->qc_active, ap->link.sactive);
	ata_port_printk(ap, KERN_ERR,
		"SWNCQ:qc_active 0x%X defer_bits 0x%X last_issue_tag 0x%x\n  "
		"dhfis 0x%X dmafis 0x%X sdbfis 0x%X\n",
		pp->qc_active, pp->defer_queue.defer_bits, pp->last_issue_tag,
		pp->dhfis_bits, pp->dmafis_bits, pp->sdbfis_bits);

	ata_port_printk(ap, KERN_ERR, "ATA_REG 0x%X ERR_REG 0x%X\n",
			ap->ops->sff_check_status(ap),
			ioread8(ap->ioaddr.error_addr));

	sactive = readl(pp->sactive_block);
	done_mask = pp->qc_active ^ sactive;

	ata_port_printk(ap, KERN_ERR, "tag : dhfis dmafis sdbfis sacitve\n");
	for (i = 0; i < ATA_MAX_QUEUE; i++) {
		u8 err = 0;
		if (pp->qc_active & (1 << i))
			err = 0;
		else if (done_mask & (1 << i))
			err = 1;
		else
			continue;

		ata_port_printk(ap, KERN_ERR,
				"tag 0x%x: %01x %01x %01x %01x %s\n", i,
				(pp->dhfis_bits >> i) & 0x1,
				(pp->dmafis_bits >> i) & 0x1,
				(pp->sdbfis_bits >> i) & 0x1,
				(sactive >> i) & 0x1,
				(err ? "error! tag doesn't exit" : " "));
	}

	nv_swncq_pp_reinit(ap);
	ap->ops->sff_irq_clear(ap);
	__ata_bmdma_stop(ap);
	nv_swncq_irq_clear(ap, 0xffff);
}

static void nv_swncq_error_handler(struct ata_port *ap)
{
	struct ata_eh_context *ehc = &ap->link.eh_context;

	if (ap->link.sactive) {
		nv_swncq_ncq_stop(ap);
		ehc->i.action |= ATA_EH_RESET;
	}

	ata_sff_error_handler(ap);
}

#ifdef CONFIG_PM
static int nv_swncq_port_suspend(struct ata_port *ap, pm_message_t mesg)
{
	void __iomem *mmio = ap->host->iomap[NV_MMIO_BAR];
	u32 tmp;

	
	writel(~0, mmio + NV_INT_STATUS_MCP55);

	
	writel(0, mmio + NV_INT_ENABLE_MCP55);

	
	tmp = readl(mmio + NV_CTL_MCP55);
	tmp &= ~(NV_CTL_PRI_SWNCQ | NV_CTL_SEC_SWNCQ);
	writel(tmp, mmio + NV_CTL_MCP55);

	return 0;
}

static int nv_swncq_port_resume(struct ata_port *ap)
{
	void __iomem *mmio = ap->host->iomap[NV_MMIO_BAR];
	u32 tmp;

	
	writel(~0, mmio + NV_INT_STATUS_MCP55);

	
	writel(0x00fd00fd, mmio + NV_INT_ENABLE_MCP55);

	
	tmp = readl(mmio + NV_CTL_MCP55);
	writel(tmp | NV_CTL_PRI_SWNCQ | NV_CTL_SEC_SWNCQ, mmio + NV_CTL_MCP55);

	return 0;
}
#endif

static void nv_swncq_host_init(struct ata_host *host)
{
	u32 tmp;
	void __iomem *mmio = host->iomap[NV_MMIO_BAR];
	struct pci_dev *pdev = to_pci_dev(host->dev);
	u8 regval;

	
	pci_read_config_byte(pdev, 0x7f, &regval);
	regval &= ~(1 << 7);
	pci_write_config_byte(pdev, 0x7f, regval);

	
	tmp = readl(mmio + NV_CTL_MCP55);
	VPRINTK("HOST_CTL:0x%X\n", tmp);
	writel(tmp | NV_CTL_PRI_SWNCQ | NV_CTL_SEC_SWNCQ, mmio + NV_CTL_MCP55);

	
	tmp = readl(mmio + NV_INT_ENABLE_MCP55);
	VPRINTK("HOST_ENABLE:0x%X\n", tmp);
	writel(tmp | 0x00fd00fd, mmio + NV_INT_ENABLE_MCP55);

	
	writel(~0x0, mmio + NV_INT_STATUS_MCP55);
}

static int nv_swncq_slave_config(struct scsi_device *sdev)
{
	struct ata_port *ap = ata_shost_to_port(sdev->host);
	struct pci_dev *pdev = to_pci_dev(ap->host->dev);
	struct ata_device *dev;
	int rc;
	u8 rev;
	u8 check_maxtor = 0;
	unsigned char model_num[ATA_ID_PROD_LEN + 1];

	rc = ata_scsi_slave_config(sdev);
	if (sdev->id >= ATA_MAX_DEVICES || sdev->channel || sdev->lun)
		
		return rc;

	dev = &ap->link.device[sdev->id];
	if (!(ap->flags & ATA_FLAG_NCQ) || dev->class == ATA_DEV_ATAPI)
		return rc;

	
	if (pdev->device == PCI_DEVICE_ID_NVIDIA_NFORCE_MCP51_SATA ||
		pdev->device == PCI_DEVICE_ID_NVIDIA_NFORCE_MCP51_SATA2)
		check_maxtor = 1;

	
	if (pdev->device == PCI_DEVICE_ID_NVIDIA_NFORCE_MCP55_SATA ||
		pdev->device == PCI_DEVICE_ID_NVIDIA_NFORCE_MCP55_SATA2) {
		pci_read_config_byte(pdev, 0x8, &rev);
		if (rev <= 0xa2)
			check_maxtor = 1;
	}

	if (!check_maxtor)
		return rc;

	ata_id_c_string(dev->id, model_num, ATA_ID_PROD, sizeof(model_num));

	if (strncmp(model_num, "Maxtor", 6) == 0) {
		ata_scsi_change_queue_depth(sdev, 1);
		ata_dev_printk(dev, KERN_NOTICE,
			"Disabling SWNCQ mode (depth %x)\n", sdev->queue_depth);
	}

	return rc;
}

static int nv_swncq_port_start(struct ata_port *ap)
{
	struct device *dev = ap->host->dev;
	void __iomem *mmio = ap->host->iomap[NV_MMIO_BAR];
	struct nv_swncq_port_priv *pp;
	int rc;

	rc = ata_port_start(ap);
	if (rc)
		return rc;

	pp = devm_kzalloc(dev, sizeof(*pp), GFP_KERNEL);
	if (!pp)
		return -ENOMEM;

	pp->prd = dmam_alloc_coherent(dev, ATA_PRD_TBL_SZ * ATA_MAX_QUEUE,
				      &pp->prd_dma, GFP_KERNEL);
	if (!pp->prd)
		return -ENOMEM;
	memset(pp->prd, 0, ATA_PRD_TBL_SZ * ATA_MAX_QUEUE);

	ap->private_data = pp;
	pp->sactive_block = ap->ioaddr.scr_addr + 4 * SCR_ACTIVE;
	pp->irq_block = mmio + NV_INT_STATUS_MCP55 + ap->port_no * 2;
	pp->tag_block = mmio + NV_NCQ_REG_MCP55 + ap->port_no * 2;

	return 0;
}

static void nv_swncq_qc_prep(struct ata_queued_cmd *qc)
{
	if (qc->tf.protocol != ATA_PROT_NCQ) {
		ata_sff_qc_prep(qc);
		return;
	}

	if (!(qc->flags & ATA_QCFLAG_DMAMAP))
		return;

	nv_swncq_fill_sg(qc);
}

static void nv_swncq_fill_sg(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct scatterlist *sg;
	struct nv_swncq_port_priv *pp = ap->private_data;
	struct ata_prd *prd;
	unsigned int si, idx;

	prd = pp->prd + ATA_MAX_PRD * qc->tag;

	idx = 0;
	for_each_sg(qc->sg, sg, qc->n_elem, si) {
		u32 addr, offset;
		u32 sg_len, len;

		addr = (u32)sg_dma_address(sg);
		sg_len = sg_dma_len(sg);

		while (sg_len) {
			offset = addr & 0xffff;
			len = sg_len;
			if ((offset + sg_len) > 0x10000)
				len = 0x10000 - offset;

			prd[idx].addr = cpu_to_le32(addr);
			prd[idx].flags_len = cpu_to_le32(len & 0xffff);

			idx++;
			sg_len -= len;
			addr += len;
		}
	}

	prd[idx - 1].flags_len |= cpu_to_le32(ATA_PRD_EOT);
}

static unsigned int nv_swncq_issue_atacmd(struct ata_port *ap,
					  struct ata_queued_cmd *qc)
{
	struct nv_swncq_port_priv *pp = ap->private_data;

	if (qc == NULL)
		return 0;

	DPRINTK("Enter\n");

	writel((1 << qc->tag), pp->sactive_block);
	pp->last_issue_tag = qc->tag;
	pp->dhfis_bits &= ~(1 << qc->tag);
	pp->dmafis_bits &= ~(1 << qc->tag);
	pp->qc_active |= (0x1 << qc->tag);

	ap->ops->sff_tf_load(ap, &qc->tf);	 
	ap->ops->sff_exec_command(ap, &qc->tf);

	DPRINTK("Issued tag %u\n", qc->tag);

	return 0;
}

static unsigned int nv_swncq_qc_issue(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct nv_swncq_port_priv *pp = ap->private_data;

	if (qc->tf.protocol != ATA_PROT_NCQ)
		return ata_sff_qc_issue(qc);

	DPRINTK("Enter\n");

	if (!pp->qc_active)
		nv_swncq_issue_atacmd(ap, qc);
	else
		nv_swncq_qc_to_dq(ap, qc);	

	return 0;
}

static void nv_swncq_hotplug(struct ata_port *ap, u32 fis)
{
	u32 serror;
	struct ata_eh_info *ehi = &ap->link.eh_info;

	ata_ehi_clear_desc(ehi);

	
	sata_scr_read(&ap->link, SCR_ERROR, &serror);
	sata_scr_write(&ap->link, SCR_ERROR, serror);

	
	if (fis & NV_SWNCQ_IRQ_ADDED)
		ata_ehi_push_desc(ehi, "hot plug");
	else if (fis & NV_SWNCQ_IRQ_REMOVED)
		ata_ehi_push_desc(ehi, "hot unplug");

	ata_ehi_hotplugged(ehi);

	
	ehi->serror |= serror;

	ata_port_freeze(ap);
}

static int nv_swncq_sdbfis(struct ata_port *ap)
{
	struct ata_queued_cmd *qc;
	struct nv_swncq_port_priv *pp = ap->private_data;
	struct ata_eh_info *ehi = &ap->link.eh_info;
	u32 sactive;
	int nr_done = 0;
	u32 done_mask;
	int i;
	u8 host_stat;
	u8 lack_dhfis = 0;

	host_stat = ap->ops->bmdma_status(ap);
	if (unlikely(host_stat & ATA_DMA_ERR)) {
		
		ata_ehi_clear_desc(ehi);
		ata_ehi_push_desc(ehi, "BMDMA stat 0x%x", host_stat);
		ehi->err_mask |= AC_ERR_HOST_BUS;
		ehi->action |= ATA_EH_RESET;
		return -EINVAL;
	}

	ap->ops->sff_irq_clear(ap);
	__ata_bmdma_stop(ap);

	sactive = readl(pp->sactive_block);
	done_mask = pp->qc_active ^ sactive;

	if (unlikely(done_mask & sactive)) {
		ata_ehi_clear_desc(ehi);
		ata_ehi_push_desc(ehi, "illegal SWNCQ:qc_active transition"
				  "(%08x->%08x)", pp->qc_active, sactive);
		ehi->err_mask |= AC_ERR_HSM;
		ehi->action |= ATA_EH_RESET;
		return -EINVAL;
	}
	for (i = 0; i < ATA_MAX_QUEUE; i++) {
		if (!(done_mask & (1 << i)))
			continue;

		qc = ata_qc_from_tag(ap, i);
		if (qc) {
			ata_qc_complete(qc);
			pp->qc_active &= ~(1 << i);
			pp->dhfis_bits &= ~(1 << i);
			pp->dmafis_bits &= ~(1 << i);
			pp->sdbfis_bits |= (1 << i);
			nr_done++;
		}
	}

	if (!ap->qc_active) {
		DPRINTK("over\n");
		nv_swncq_pp_reinit(ap);
		return nr_done;
	}

	if (pp->qc_active & pp->dhfis_bits)
		return nr_done;

	if ((pp->ncq_flags & ncq_saw_backout) ||
	    (pp->qc_active ^ pp->dhfis_bits))
		
		lack_dhfis = 1;

	DPRINTK("id 0x%x QC: qc_active 0x%x,"
		"SWNCQ:qc_active 0x%X defer_bits %X "
		"dhfis 0x%X dmafis 0x%X last_issue_tag %x\n",
		ap->print_id, ap->qc_active, pp->qc_active,
		pp->defer_queue.defer_bits, pp->dhfis_bits,
		pp->dmafis_bits, pp->last_issue_tag);

	nv_swncq_fis_reinit(ap);

	if (lack_dhfis) {
		qc = ata_qc_from_tag(ap, pp->last_issue_tag);
		nv_swncq_issue_atacmd(ap, qc);
		return nr_done;
	}

	if (pp->defer_queue.defer_bits) {
		
		qc = nv_swncq_qc_from_dq(ap);
		WARN_ON(qc == NULL);
		nv_swncq_issue_atacmd(ap, qc);
	}

	return nr_done;
}

static inline u32 nv_swncq_tag(struct ata_port *ap)
{
	struct nv_swncq_port_priv *pp = ap->private_data;
	u32 tag;

	tag = readb(pp->tag_block) >> 2;
	return (tag & 0x1f);
}

static int nv_swncq_dmafis(struct ata_port *ap)
{
	struct ata_queued_cmd *qc;
	unsigned int rw;
	u8 dmactl;
	u32 tag;
	struct nv_swncq_port_priv *pp = ap->private_data;

	__ata_bmdma_stop(ap);
	tag = nv_swncq_tag(ap);

	DPRINTK("dma setup tag 0x%x\n", tag);
	qc = ata_qc_from_tag(ap, tag);

	if (unlikely(!qc))
		return 0;

	rw = qc->tf.flags & ATA_TFLAG_WRITE;

	
	iowrite32(pp->prd_dma + ATA_PRD_TBL_SZ * qc->tag,
		  ap->ioaddr.bmdma_addr + ATA_DMA_TABLE_OFS);

	
	dmactl = ioread8(ap->ioaddr.bmdma_addr + ATA_DMA_CMD);
	dmactl &= ~ATA_DMA_WR;
	if (!rw)
		dmactl |= ATA_DMA_WR;

	iowrite8(dmactl | ATA_DMA_START, ap->ioaddr.bmdma_addr + ATA_DMA_CMD);

	return 1;
}

static void nv_swncq_host_interrupt(struct ata_port *ap, u16 fis)
{
	struct nv_swncq_port_priv *pp = ap->private_data;
	struct ata_queued_cmd *qc;
	struct ata_eh_info *ehi = &ap->link.eh_info;
	u32 serror;
	u8 ata_stat;
	int rc = 0;

	ata_stat = ap->ops->sff_check_status(ap);
	nv_swncq_irq_clear(ap, fis);
	if (!fis)
		return;

	if (ap->pflags & ATA_PFLAG_FROZEN)
		return;

	if (fis & NV_SWNCQ_IRQ_HOTPLUG) {
		nv_swncq_hotplug(ap, fis);
		return;
	}

	if (!pp->qc_active)
		return;

	if (ap->ops->scr_read(&ap->link, SCR_ERROR, &serror))
		return;
	ap->ops->scr_write(&ap->link, SCR_ERROR, serror);

	if (ata_stat & ATA_ERR) {
		ata_ehi_clear_desc(ehi);
		ata_ehi_push_desc(ehi, "Ata error. fis:0x%X", fis);
		ehi->err_mask |= AC_ERR_DEV;
		ehi->serror |= serror;
		ehi->action |= ATA_EH_RESET;
		ata_port_freeze(ap);
		return;
	}

	if (fis & NV_SWNCQ_IRQ_BACKOUT) {
		
		pp->ncq_flags |= ncq_saw_backout;
	}

	if (fis & NV_SWNCQ_IRQ_SDBFIS) {
		pp->ncq_flags |= ncq_saw_sdb;
		DPRINTK("id 0x%x SWNCQ: qc_active 0x%X "
			"dhfis 0x%X dmafis 0x%X sactive 0x%X\n",
			ap->print_id, pp->qc_active, pp->dhfis_bits,
			pp->dmafis_bits, readl(pp->sactive_block));
		rc = nv_swncq_sdbfis(ap);
		if (rc < 0)
			goto irq_error;
	}

	if (fis & NV_SWNCQ_IRQ_DHREGFIS) {
		
		pp->dhfis_bits |= (0x1 << pp->last_issue_tag);
		pp->ncq_flags |= ncq_saw_d2h;
		if (pp->ncq_flags & (ncq_saw_sdb | ncq_saw_backout)) {
			ata_ehi_push_desc(ehi, "illegal fis transaction");
			ehi->err_mask |= AC_ERR_HSM;
			ehi->action |= ATA_EH_RESET;
			goto irq_error;
		}

		if (!(fis & NV_SWNCQ_IRQ_DMASETUP) &&
		    !(pp->ncq_flags & ncq_saw_dmas)) {
			ata_stat = ap->ops->sff_check_status(ap);
			if (ata_stat & ATA_BUSY)
				goto irq_exit;

			if (pp->defer_queue.defer_bits) {
				DPRINTK("send next command\n");
				qc = nv_swncq_qc_from_dq(ap);
				nv_swncq_issue_atacmd(ap, qc);
			}
		}
	}

	if (fis & NV_SWNCQ_IRQ_DMASETUP) {
		
		pp->dmafis_bits |= (0x1 << nv_swncq_tag(ap));
		pp->ncq_flags |= ncq_saw_dmas;
		rc = nv_swncq_dmafis(ap);
	}

irq_exit:
	return;
irq_error:
	ata_ehi_push_desc(ehi, "fis:0x%x", fis);
	ata_port_freeze(ap);
	return;
}

static irqreturn_t nv_swncq_interrupt(int irq, void *dev_instance)
{
	struct ata_host *host = dev_instance;
	unsigned int i;
	unsigned int handled = 0;
	unsigned long flags;
	u32 irq_stat;

	spin_lock_irqsave(&host->lock, flags);

	irq_stat = readl(host->iomap[NV_MMIO_BAR] + NV_INT_STATUS_MCP55);

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];

		if (ap && !(ap->flags & ATA_FLAG_DISABLED)) {
			if (ap->link.sactive) {
				nv_swncq_host_interrupt(ap, (u16)irq_stat);
				handled = 1;
			} else {
				if (irq_stat)	
					nv_swncq_irq_clear(ap, 0xfff0);

				handled += nv_host_intr(ap, (u8)irq_stat);
			}
		}
		irq_stat >>= NV_INT_PORT_SHIFT_MCP55;
	}

	spin_unlock_irqrestore(&host->lock, flags);

	return IRQ_RETVAL(handled);
}

static int nv_init_one(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	static int printed_version;
	const struct ata_port_info *ppi[] = { NULL, NULL };
	struct nv_pi_priv *ipriv;
	struct ata_host *host;
	struct nv_host_priv *hpriv;
	int rc;
	u32 bar;
	void __iomem *base;
	unsigned long type = ent->driver_data;

        
        
        
	for (bar = 0; bar < 6; bar++)
		if (pci_resource_start(pdev, bar) == 0)
			return -ENODEV;

	if (!printed_version++)
		dev_printk(KERN_DEBUG, &pdev->dev, "version " DRV_VERSION "\n");

	rc = pcim_enable_device(pdev);
	if (rc)
		return rc;

	
	if (type == CK804 && adma_enabled) {
		dev_printk(KERN_NOTICE, &pdev->dev, "Using ADMA mode\n");
		type = ADMA;
	} else if (type == MCP5x && swncq_enabled) {
		dev_printk(KERN_NOTICE, &pdev->dev, "Using SWNCQ mode\n");
		type = SWNCQ;
	}

	ppi[0] = &nv_port_info[type];
	ipriv = ppi[0]->private_data;
	rc = ata_pci_sff_prepare_host(pdev, ppi, &host);
	if (rc)
		return rc;

	hpriv = devm_kzalloc(&pdev->dev, sizeof(*hpriv), GFP_KERNEL);
	if (!hpriv)
		return -ENOMEM;
	hpriv->type = type;
	host->private_data = hpriv;

	
	rc = pcim_iomap_regions(pdev, 1 << NV_MMIO_BAR, DRV_NAME);
	if (rc)
		return rc;

	
	base = host->iomap[NV_MMIO_BAR];
	host->ports[0]->ioaddr.scr_addr = base + NV_PORT0_SCR_REG_OFFSET;
	host->ports[1]->ioaddr.scr_addr = base + NV_PORT1_SCR_REG_OFFSET;

	
	if (type >= CK804) {
		u8 regval;

		pci_read_config_byte(pdev, NV_MCP_SATA_CFG_20, &regval);
		regval |= NV_MCP_SATA_CFG_20_SATA_SPACE_EN;
		pci_write_config_byte(pdev, NV_MCP_SATA_CFG_20, regval);
	}

	
	if (type == ADMA) {
		rc = nv_adma_host_init(host);
		if (rc)
			return rc;
	} else if (type == SWNCQ)
		nv_swncq_host_init(host);

	if (msi_enabled) {
		dev_printk(KERN_NOTICE, &pdev->dev, "Using MSI\n");
		pci_enable_msi(pdev);
	}

	pci_set_master(pdev);
	return ata_host_activate(host, pdev->irq, ipriv->irq_handler,
				 IRQF_SHARED, ipriv->sht);
}

#ifdef CONFIG_PM
static int nv_pci_device_resume(struct pci_dev *pdev)
{
	struct ata_host *host = dev_get_drvdata(&pdev->dev);
	struct nv_host_priv *hpriv = host->private_data;
	int rc;

	rc = ata_pci_device_do_resume(pdev);
	if (rc)
		return rc;

	if (pdev->dev.power.power_state.event == PM_EVENT_SUSPEND) {
		if (hpriv->type >= CK804) {
			u8 regval;

			pci_read_config_byte(pdev, NV_MCP_SATA_CFG_20, &regval);
			regval |= NV_MCP_SATA_CFG_20_SATA_SPACE_EN;
			pci_write_config_byte(pdev, NV_MCP_SATA_CFG_20, regval);
		}
		if (hpriv->type == ADMA) {
			u32 tmp32;
			struct nv_adma_port_priv *pp;
			
			pci_read_config_dword(pdev, NV_MCP_SATA_CFG_20, &tmp32);

			pp = host->ports[0]->private_data;
			if (pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE)
				tmp32 &= ~(NV_MCP_SATA_CFG_20_PORT0_EN |
					   NV_MCP_SATA_CFG_20_PORT0_PWB_EN);
			else
				tmp32 |=  (NV_MCP_SATA_CFG_20_PORT0_EN |
					   NV_MCP_SATA_CFG_20_PORT0_PWB_EN);
			pp = host->ports[1]->private_data;
			if (pp->flags & NV_ADMA_ATAPI_SETUP_COMPLETE)
				tmp32 &= ~(NV_MCP_SATA_CFG_20_PORT1_EN |
					   NV_MCP_SATA_CFG_20_PORT1_PWB_EN);
			else
				tmp32 |=  (NV_MCP_SATA_CFG_20_PORT1_EN |
					   NV_MCP_SATA_CFG_20_PORT1_PWB_EN);

			pci_write_config_dword(pdev, NV_MCP_SATA_CFG_20, tmp32);
		}
	}

	ata_host_resume(host);

	return 0;
}
#endif

static void nv_ck804_host_stop(struct ata_host *host)
{
	struct pci_dev *pdev = to_pci_dev(host->dev);
	u8 regval;

	
	pci_read_config_byte(pdev, NV_MCP_SATA_CFG_20, &regval);
	regval &= ~NV_MCP_SATA_CFG_20_SATA_SPACE_EN;
	pci_write_config_byte(pdev, NV_MCP_SATA_CFG_20, regval);
}

static void nv_adma_host_stop(struct ata_host *host)
{
	struct pci_dev *pdev = to_pci_dev(host->dev);
	u32 tmp32;

	
	pci_read_config_dword(pdev, NV_MCP_SATA_CFG_20, &tmp32);
	tmp32 &= ~(NV_MCP_SATA_CFG_20_PORT0_EN |
		   NV_MCP_SATA_CFG_20_PORT0_PWB_EN |
		   NV_MCP_SATA_CFG_20_PORT1_EN |
		   NV_MCP_SATA_CFG_20_PORT1_PWB_EN);

	pci_write_config_dword(pdev, NV_MCP_SATA_CFG_20, tmp32);

	nv_ck804_host_stop(host);
}

static int __init nv_init(void)
{
	return pci_register_driver(&nv_pci_driver);
}

static void __exit nv_exit(void)
{
	pci_unregister_driver(&nv_pci_driver);
}

module_init(nv_init);
module_exit(nv_exit);
module_param_named(adma, adma_enabled, bool, 0444);
MODULE_PARM_DESC(adma, "Enable use of ADMA (Default: false)");
module_param_named(swncq, swncq_enabled, bool, 0444);
MODULE_PARM_DESC(swncq, "Enable use of SWNCQ (Default: true)");
module_param_named(msi, msi_enabled, bool, 0444);
MODULE_PARM_DESC(msi, "Enable use of MSI (Default: false)");

