


#define DRV_NAME	"via-rhine"
#define DRV_VERSION	"1.4.3"
#define DRV_RELDATE	"2007-03-06"




static int debug = 1;	
static int max_interrupt_work = 20;


#if defined(__alpha__) || defined(__arm__) || defined(__hppa__) \
       || defined(CONFIG_SPARC) || defined(__ia64__) \
       || defined(__sh__) || defined(__mips__)
static int rx_copybreak = 1518;
#else
static int rx_copybreak;
#endif


static int avoid_D3;




static const int multicast_filter_limit = 32;





#define TX_RING_SIZE	16
#define TX_QUEUE_LEN	10	
#define RX_RING_SIZE	64




#define TX_TIMEOUT	(2*HZ)

#define PKT_BUF_SZ	1536	

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/crc32.h>
#include <linux/bitops.h>
#include <asm/processor.h>	
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <linux/dmi.h>


static const char version[] __devinitconst =
	KERN_INFO DRV_NAME ".c:v1.10-LK" DRV_VERSION " " DRV_RELDATE
	" Written by Donald Becker\n";


#ifdef CONFIG_VIA_RHINE_MMIO
#define USE_MMIO
#else
#endif

MODULE_AUTHOR("Donald Becker <becker@scyld.com>");
MODULE_DESCRIPTION("VIA Rhine PCI Fast Ethernet driver");
MODULE_LICENSE("GPL");

module_param(max_interrupt_work, int, 0);
module_param(debug, int, 0);
module_param(rx_copybreak, int, 0);
module_param(avoid_D3, bool, 0);
MODULE_PARM_DESC(max_interrupt_work, "VIA Rhine maximum events handled per interrupt");
MODULE_PARM_DESC(debug, "VIA Rhine debug level (0-7)");
MODULE_PARM_DESC(rx_copybreak, "VIA Rhine copy breakpoint for copy-only-tiny-frames");
MODULE_PARM_DESC(avoid_D3, "Avoid power state D3 (work-around for broken BIOSes)");






enum rhine_revs {
	VT86C100A	= 0x00,
	VTunknown0	= 0x20,
	VT6102		= 0x40,
	VT8231		= 0x50,	
	VT8233		= 0x60,	
	VT8235		= 0x74,	
	VT8237		= 0x78,	
	VTunknown1	= 0x7C,
	VT6105		= 0x80,
	VT6105_B0	= 0x83,
	VT6105L		= 0x8A,
	VT6107		= 0x8C,
	VTunknown2	= 0x8E,
	VT6105M		= 0x90,	
};

enum rhine_quirks {
	rqWOL		= 0x0001,	
	rqForceReset	= 0x0002,
	rq6patterns	= 0x0040,	
	rqStatusWBRace	= 0x0080,	
	rqRhineI	= 0x0100,	
};



#define IOSYNC	do { ioread8(ioaddr + StationAddr); } while (0)

static const struct pci_device_id rhine_pci_tbl[] = {
	{ 0x1106, 0x3043, PCI_ANY_ID, PCI_ANY_ID, },	
	{ 0x1106, 0x3065, PCI_ANY_ID, PCI_ANY_ID, },	
	{ 0x1106, 0x3106, PCI_ANY_ID, PCI_ANY_ID, },	
	{ 0x1106, 0x3053, PCI_ANY_ID, PCI_ANY_ID, },	
	{ }	
};
MODULE_DEVICE_TABLE(pci, rhine_pci_tbl);



enum register_offsets {
	StationAddr=0x00, RxConfig=0x06, TxConfig=0x07, ChipCmd=0x08,
	ChipCmd1=0x09,
	IntrStatus=0x0C, IntrEnable=0x0E,
	MulticastFilter0=0x10, MulticastFilter1=0x14,
	RxRingPtr=0x18, TxRingPtr=0x1C, GFIFOTest=0x54,
	MIIPhyAddr=0x6C, MIIStatus=0x6D, PCIBusConfig=0x6E,
	MIICmd=0x70, MIIRegAddr=0x71, MIIData=0x72, MACRegEEcsr=0x74,
	ConfigA=0x78, ConfigB=0x79, ConfigC=0x7A, ConfigD=0x7B,
	RxMissed=0x7C, RxCRCErrs=0x7E, MiscCmd=0x81,
	StickyHW=0x83, IntrStatus2=0x84,
	WOLcrSet=0xA0, PwcfgSet=0xA1, WOLcgSet=0xA3, WOLcrClr=0xA4,
	WOLcrClr1=0xA6, WOLcgClr=0xA7,
	PwrcsrSet=0xA8, PwrcsrSet1=0xA9, PwrcsrClr=0xAC, PwrcsrClr1=0xAD,
};


enum backoff_bits {
	BackOptional=0x01, BackModify=0x02,
	BackCaptureEffect=0x04, BackRandom=0x08
};

#ifdef USE_MMIO

static const int mmio_verify_registers[] = {
	RxConfig, TxConfig, IntrEnable, ConfigA, ConfigB, ConfigC, ConfigD,
	0
};
#endif


enum intr_status_bits {
	IntrRxDone=0x0001, IntrRxErr=0x0004, IntrRxEmpty=0x0020,
	IntrTxDone=0x0002, IntrTxError=0x0008, IntrTxUnderrun=0x0210,
	IntrPCIErr=0x0040,
	IntrStatsMax=0x0080, IntrRxEarly=0x0100,
	IntrRxOverflow=0x0400, IntrRxDropped=0x0800, IntrRxNoBuf=0x1000,
	IntrTxAborted=0x2000, IntrLinkChange=0x4000,
	IntrRxWakeUp=0x8000,
	IntrNormalSummary=0x0003, IntrAbnormalSummary=0xC260,
	IntrTxDescRace=0x080000,	
	IntrTxErrSummary=0x082218,
};


enum wol_bits {
	WOLucast	= 0x10,
	WOLmagic	= 0x20,
	WOLbmcast	= 0x30,
	WOLlnkon	= 0x40,
	WOLlnkoff	= 0x80,
};


struct rx_desc {
	__le32 rx_status;
	__le32 desc_length; 
	__le32 addr;
	__le32 next_desc;
};
struct tx_desc {
	__le32 tx_status;
	__le32 desc_length; 
	__le32 addr;
	__le32 next_desc;
};


#define TXDESC		0x00e08000

enum rx_status_bits {
	RxOK=0x8000, RxWholePkt=0x0300, RxErr=0x008F
};


enum desc_status_bits {
	DescOwn=0x80000000
};


enum chip_cmd_bits {
	CmdInit=0x01, CmdStart=0x02, CmdStop=0x04, CmdRxOn=0x08,
	CmdTxOn=0x10, Cmd1TxDemand=0x20, CmdRxDemand=0x40,
	Cmd1EarlyRx=0x01, Cmd1EarlyTx=0x02, Cmd1FDuplex=0x04,
	Cmd1NoTxPoll=0x08, Cmd1Reset=0x80,
};

struct rhine_private {
	
	struct rx_desc *rx_ring;
	struct tx_desc *tx_ring;
	dma_addr_t rx_ring_dma;
	dma_addr_t tx_ring_dma;

	
	struct sk_buff *rx_skbuff[RX_RING_SIZE];
	dma_addr_t rx_skbuff_dma[RX_RING_SIZE];

	
	struct sk_buff *tx_skbuff[TX_RING_SIZE];
	dma_addr_t tx_skbuff_dma[TX_RING_SIZE];

	
	unsigned char *tx_buf[TX_RING_SIZE];
	unsigned char *tx_bufs;
	dma_addr_t tx_bufs_dma;

	struct pci_dev *pdev;
	long pioaddr;
	struct net_device *dev;
	struct napi_struct napi;
	spinlock_t lock;

	
	u32 quirks;
	struct rx_desc *rx_head_desc;
	unsigned int cur_rx, dirty_rx;	
	unsigned int cur_tx, dirty_tx;
	unsigned int rx_buf_sz;		
	u8 wolopts;

	u8 tx_thresh, rx_thresh;

	struct mii_if_info mii_if;
	void __iomem *base;
};

static int  mdio_read(struct net_device *dev, int phy_id, int location);
static void mdio_write(struct net_device *dev, int phy_id, int location, int value);
static int  rhine_open(struct net_device *dev);
static void rhine_tx_timeout(struct net_device *dev);
static netdev_tx_t rhine_start_tx(struct sk_buff *skb,
				  struct net_device *dev);
static irqreturn_t rhine_interrupt(int irq, void *dev_instance);
static void rhine_tx(struct net_device *dev);
static int rhine_rx(struct net_device *dev, int limit);
static void rhine_error(struct net_device *dev, int intr_status);
static void rhine_set_rx_mode(struct net_device *dev);
static struct net_device_stats *rhine_get_stats(struct net_device *dev);
static int netdev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
static const struct ethtool_ops netdev_ethtool_ops;
static int  rhine_close(struct net_device *dev);
static void rhine_shutdown (struct pci_dev *pdev);

#define RHINE_WAIT_FOR(condition) do {					\
	int i=1024;							\
	while (!(condition) && --i)					\
		;							\
	if (debug > 1 && i < 512)					\
		printk(KERN_INFO "%s: %4d cycles used @ %s:%d\n",	\
				DRV_NAME, 1024-i, __func__, __LINE__);	\
} while(0)

static inline u32 get_intr_status(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;
	u32 intr_status;

	intr_status = ioread16(ioaddr + IntrStatus);
	
	if (rp->quirks & rqStatusWBRace)
		intr_status |= ioread8(ioaddr + IntrStatus2) << 16;
	return intr_status;
}


static void rhine_power_init(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;
	u16 wolstat;

	if (rp->quirks & rqWOL) {
		
		iowrite8(ioread8(ioaddr + StickyHW) & 0xFC, ioaddr + StickyHW);

		
		iowrite8(0x80, ioaddr + WOLcgClr);

		
		iowrite8(0xFF, ioaddr + WOLcrClr);
		
		if (rp->quirks & rq6patterns)
			iowrite8(0x03, ioaddr + WOLcrClr1);

		
		wolstat = ioread8(ioaddr + PwrcsrSet);
		if (rp->quirks & rq6patterns)
			wolstat |= (ioread8(ioaddr + PwrcsrSet1) & 0x03) << 8;

		
		iowrite8(0xFF, ioaddr + PwrcsrClr);
		if (rp->quirks & rq6patterns)
			iowrite8(0x03, ioaddr + PwrcsrClr1);

		if (wolstat) {
			char *reason;
			switch (wolstat) {
			case WOLmagic:
				reason = "Magic packet";
				break;
			case WOLlnkon:
				reason = "Link went up";
				break;
			case WOLlnkoff:
				reason = "Link went down";
				break;
			case WOLucast:
				reason = "Unicast packet";
				break;
			case WOLbmcast:
				reason = "Multicast/broadcast packet";
				break;
			default:
				reason = "Unknown";
			}
			printk(KERN_INFO "%s: Woke system up. Reason: %s.\n",
			       DRV_NAME, reason);
		}
	}
}

static void rhine_chip_reset(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;

	iowrite8(Cmd1Reset, ioaddr + ChipCmd1);
	IOSYNC;

	if (ioread8(ioaddr + ChipCmd1) & Cmd1Reset) {
		printk(KERN_INFO "%s: Reset not complete yet. "
			"Trying harder.\n", DRV_NAME);

		
		if (rp->quirks & rqForceReset)
			iowrite8(0x40, ioaddr + MiscCmd);

		
		RHINE_WAIT_FOR(!(ioread8(ioaddr + ChipCmd1) & Cmd1Reset));
	}

	if (debug > 1)
		printk(KERN_INFO "%s: Reset %s.\n", dev->name,
			(ioread8(ioaddr + ChipCmd1) & Cmd1Reset) ?
			"failed" : "succeeded");
}

#ifdef USE_MMIO
static void enable_mmio(long pioaddr, u32 quirks)
{
	int n;
	if (quirks & rqRhineI) {
		
		n = inb(pioaddr + ConfigA) | 0x20;
		outb(n, pioaddr + ConfigA);
	} else {
		n = inb(pioaddr + ConfigD) | 0x80;
		outb(n, pioaddr + ConfigD);
	}
}
#endif


static void __devinit rhine_reload_eeprom(long pioaddr, struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;

	outb(0x20, pioaddr + MACRegEEcsr);
	RHINE_WAIT_FOR(!(inb(pioaddr + MACRegEEcsr) & 0x20));

#ifdef USE_MMIO
	
	enable_mmio(pioaddr, rp->quirks);
#endif

	
	if (rp->quirks & rqWOL)
		iowrite8(ioread8(ioaddr + ConfigA) & 0xFC, ioaddr + ConfigA);

}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void rhine_poll(struct net_device *dev)
{
	disable_irq(dev->irq);
	rhine_interrupt(dev->irq, (void *)dev);
	enable_irq(dev->irq);
}
#endif

static int rhine_napipoll(struct napi_struct *napi, int budget)
{
	struct rhine_private *rp = container_of(napi, struct rhine_private, napi);
	struct net_device *dev = rp->dev;
	void __iomem *ioaddr = rp->base;
	int work_done;

	work_done = rhine_rx(dev, budget);

	if (work_done < budget) {
		napi_complete(napi);

		iowrite16(IntrRxDone | IntrRxErr | IntrRxEmpty| IntrRxOverflow |
			  IntrRxDropped | IntrRxNoBuf | IntrTxAborted |
			  IntrTxDone | IntrTxError | IntrTxUnderrun |
			  IntrPCIErr | IntrStatsMax | IntrLinkChange,
			  ioaddr + IntrEnable);
	}
	return work_done;
}

static void __devinit rhine_hw_init(struct net_device *dev, long pioaddr)
{
	struct rhine_private *rp = netdev_priv(dev);

	
	rhine_chip_reset(dev);

	
	if (rp->quirks & rqRhineI)
		msleep(5);

	
	rhine_reload_eeprom(pioaddr, dev);
}

static const struct net_device_ops rhine_netdev_ops = {
	.ndo_open		 = rhine_open,
	.ndo_stop		 = rhine_close,
	.ndo_start_xmit		 = rhine_start_tx,
	.ndo_get_stats		 = rhine_get_stats,
	.ndo_set_multicast_list	 = rhine_set_rx_mode,
	.ndo_change_mtu		 = eth_change_mtu,
	.ndo_validate_addr	 = eth_validate_addr,
	.ndo_set_mac_address 	 = eth_mac_addr,
	.ndo_do_ioctl		 = netdev_ioctl,
	.ndo_tx_timeout 	 = rhine_tx_timeout,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	 = rhine_poll,
#endif
};

static int __devinit rhine_init_one(struct pci_dev *pdev,
				    const struct pci_device_id *ent)
{
	struct net_device *dev;
	struct rhine_private *rp;
	int i, rc;
	u32 quirks;
	long pioaddr;
	long memaddr;
	void __iomem *ioaddr;
	int io_size, phy_id;
	const char *name;
#ifdef USE_MMIO
	int bar = 1;
#else
	int bar = 0;
#endif


#ifndef MODULE
	static int printed_version;
	if (!printed_version++)
		printk(version);
#endif

	io_size = 256;
	phy_id = 0;
	quirks = 0;
	name = "Rhine";
	if (pdev->revision < VTunknown0) {
		quirks = rqRhineI;
		io_size = 128;
	}
	else if (pdev->revision >= VT6102) {
		quirks = rqWOL | rqForceReset;
		if (pdev->revision < VT6105) {
			name = "Rhine II";
			quirks |= rqStatusWBRace;	
		}
		else {
			phy_id = 1;	
			if (pdev->revision >= VT6105_B0)
				quirks |= rq6patterns;
			if (pdev->revision < VT6105M)
				name = "Rhine III";
			else
				name = "Rhine III (Management Adapter)";
		}
	}

	rc = pci_enable_device(pdev);
	if (rc)
		goto err_out;

	
	rc = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
	if (rc) {
		printk(KERN_ERR "32-bit PCI DMA addresses not supported by "
		       "the card!?\n");
		goto err_out;
	}

	
	if ((pci_resource_len(pdev, 0) < io_size) ||
	    (pci_resource_len(pdev, 1) < io_size)) {
		rc = -EIO;
		printk(KERN_ERR "Insufficient PCI resources, aborting\n");
		goto err_out;
	}

	pioaddr = pci_resource_start(pdev, 0);
	memaddr = pci_resource_start(pdev, 1);

	pci_set_master(pdev);

	dev = alloc_etherdev(sizeof(struct rhine_private));
	if (!dev) {
		rc = -ENOMEM;
		printk(KERN_ERR "alloc_etherdev failed\n");
		goto err_out;
	}
	SET_NETDEV_DEV(dev, &pdev->dev);

	rp = netdev_priv(dev);
	rp->dev = dev;
	rp->quirks = quirks;
	rp->pioaddr = pioaddr;
	rp->pdev = pdev;

	rc = pci_request_regions(pdev, DRV_NAME);
	if (rc)
		goto err_out_free_netdev;

	ioaddr = pci_iomap(pdev, bar, io_size);
	if (!ioaddr) {
		rc = -EIO;
		printk(KERN_ERR "ioremap failed for device %s, region 0x%X "
		       "@ 0x%lX\n", pci_name(pdev), io_size, memaddr);
		goto err_out_free_res;
	}

#ifdef USE_MMIO
	enable_mmio(pioaddr, quirks);

	
	i = 0;
	while (mmio_verify_registers[i]) {
		int reg = mmio_verify_registers[i++];
		unsigned char a = inb(pioaddr+reg);
		unsigned char b = readb(ioaddr+reg);
		if (a != b) {
			rc = -EIO;
			printk(KERN_ERR "MMIO do not match PIO [%02x] "
			       "(%02x != %02x)\n", reg, a, b);
			goto err_out_unmap;
		}
	}
#endif 

	dev->base_addr = (unsigned long)ioaddr;
	rp->base = ioaddr;

	
	rhine_power_init(dev);
	rhine_hw_init(dev, pioaddr);

	for (i = 0; i < 6; i++)
		dev->dev_addr[i] = ioread8(ioaddr + StationAddr + i);
	memcpy(dev->perm_addr, dev->dev_addr, dev->addr_len);

	if (!is_valid_ether_addr(dev->perm_addr)) {
		rc = -EIO;
		printk(KERN_ERR "Invalid MAC address\n");
		goto err_out_unmap;
	}

	
	if (!phy_id)
		phy_id = ioread8(ioaddr + 0x6C);

	dev->irq = pdev->irq;

	spin_lock_init(&rp->lock);
	rp->mii_if.dev = dev;
	rp->mii_if.mdio_read = mdio_read;
	rp->mii_if.mdio_write = mdio_write;
	rp->mii_if.phy_id_mask = 0x1f;
	rp->mii_if.reg_num_mask = 0x1f;

	
	dev->netdev_ops = &rhine_netdev_ops;
	dev->ethtool_ops = &netdev_ethtool_ops,
	dev->watchdog_timeo = TX_TIMEOUT;

	netif_napi_add(dev, &rp->napi, rhine_napipoll, 64);

	if (rp->quirks & rqRhineI)
		dev->features |= NETIF_F_SG|NETIF_F_HW_CSUM;

	
	rc = register_netdev(dev);
	if (rc)
		goto err_out_unmap;

	printk(KERN_INFO "%s: VIA %s at 0x%lx, %pM, IRQ %d.\n",
	       dev->name, name,
#ifdef USE_MMIO
	       memaddr,
#else
	       (long)ioaddr,
#endif
	       dev->dev_addr, pdev->irq);

	pci_set_drvdata(pdev, dev);

	{
		u16 mii_cmd;
		int mii_status = mdio_read(dev, phy_id, 1);
		mii_cmd = mdio_read(dev, phy_id, MII_BMCR) & ~BMCR_ISOLATE;
		mdio_write(dev, phy_id, MII_BMCR, mii_cmd);
		if (mii_status != 0xffff && mii_status != 0x0000) {
			rp->mii_if.advertising = mdio_read(dev, phy_id, 4);
			printk(KERN_INFO "%s: MII PHY found at address "
			       "%d, status 0x%4.4x advertising %4.4x "
			       "Link %4.4x.\n", dev->name, phy_id,
			       mii_status, rp->mii_if.advertising,
			       mdio_read(dev, phy_id, 5));

			
			if (mii_status & BMSR_LSTATUS)
				netif_carrier_on(dev);
			else
				netif_carrier_off(dev);

		}
	}
	rp->mii_if.phy_id = phy_id;
	if (debug > 1 && avoid_D3)
		printk(KERN_INFO "%s: No D3 power state at shutdown.\n",
		       dev->name);

	return 0;

err_out_unmap:
	pci_iounmap(pdev, ioaddr);
err_out_free_res:
	pci_release_regions(pdev);
err_out_free_netdev:
	free_netdev(dev);
err_out:
	return rc;
}

static int alloc_ring(struct net_device* dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void *ring;
	dma_addr_t ring_dma;

	ring = pci_alloc_consistent(rp->pdev,
				    RX_RING_SIZE * sizeof(struct rx_desc) +
				    TX_RING_SIZE * sizeof(struct tx_desc),
				    &ring_dma);
	if (!ring) {
		printk(KERN_ERR "Could not allocate DMA memory.\n");
		return -ENOMEM;
	}
	if (rp->quirks & rqRhineI) {
		rp->tx_bufs = pci_alloc_consistent(rp->pdev,
						   PKT_BUF_SZ * TX_RING_SIZE,
						   &rp->tx_bufs_dma);
		if (rp->tx_bufs == NULL) {
			pci_free_consistent(rp->pdev,
				    RX_RING_SIZE * sizeof(struct rx_desc) +
				    TX_RING_SIZE * sizeof(struct tx_desc),
				    ring, ring_dma);
			return -ENOMEM;
		}
	}

	rp->rx_ring = ring;
	rp->tx_ring = ring + RX_RING_SIZE * sizeof(struct rx_desc);
	rp->rx_ring_dma = ring_dma;
	rp->tx_ring_dma = ring_dma + RX_RING_SIZE * sizeof(struct rx_desc);

	return 0;
}

static void free_ring(struct net_device* dev)
{
	struct rhine_private *rp = netdev_priv(dev);

	pci_free_consistent(rp->pdev,
			    RX_RING_SIZE * sizeof(struct rx_desc) +
			    TX_RING_SIZE * sizeof(struct tx_desc),
			    rp->rx_ring, rp->rx_ring_dma);
	rp->tx_ring = NULL;

	if (rp->tx_bufs)
		pci_free_consistent(rp->pdev, PKT_BUF_SZ * TX_RING_SIZE,
				    rp->tx_bufs, rp->tx_bufs_dma);

	rp->tx_bufs = NULL;

}

static void alloc_rbufs(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	dma_addr_t next;
	int i;

	rp->dirty_rx = rp->cur_rx = 0;

	rp->rx_buf_sz = (dev->mtu <= 1500 ? PKT_BUF_SZ : dev->mtu + 32);
	rp->rx_head_desc = &rp->rx_ring[0];
	next = rp->rx_ring_dma;

	
	for (i = 0; i < RX_RING_SIZE; i++) {
		rp->rx_ring[i].rx_status = 0;
		rp->rx_ring[i].desc_length = cpu_to_le32(rp->rx_buf_sz);
		next += sizeof(struct rx_desc);
		rp->rx_ring[i].next_desc = cpu_to_le32(next);
		rp->rx_skbuff[i] = NULL;
	}
	
	rp->rx_ring[i-1].next_desc = cpu_to_le32(rp->rx_ring_dma);

	
	for (i = 0; i < RX_RING_SIZE; i++) {
		struct sk_buff *skb = netdev_alloc_skb(dev, rp->rx_buf_sz);
		rp->rx_skbuff[i] = skb;
		if (skb == NULL)
			break;
		skb->dev = dev;                 

		rp->rx_skbuff_dma[i] =
			pci_map_single(rp->pdev, skb->data, rp->rx_buf_sz,
				       PCI_DMA_FROMDEVICE);

		rp->rx_ring[i].addr = cpu_to_le32(rp->rx_skbuff_dma[i]);
		rp->rx_ring[i].rx_status = cpu_to_le32(DescOwn);
	}
	rp->dirty_rx = (unsigned int)(i - RX_RING_SIZE);
}

static void free_rbufs(struct net_device* dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	int i;

	
	for (i = 0; i < RX_RING_SIZE; i++) {
		rp->rx_ring[i].rx_status = 0;
		rp->rx_ring[i].addr = cpu_to_le32(0xBADF00D0); 
		if (rp->rx_skbuff[i]) {
			pci_unmap_single(rp->pdev,
					 rp->rx_skbuff_dma[i],
					 rp->rx_buf_sz, PCI_DMA_FROMDEVICE);
			dev_kfree_skb(rp->rx_skbuff[i]);
		}
		rp->rx_skbuff[i] = NULL;
	}
}

static void alloc_tbufs(struct net_device* dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	dma_addr_t next;
	int i;

	rp->dirty_tx = rp->cur_tx = 0;
	next = rp->tx_ring_dma;
	for (i = 0; i < TX_RING_SIZE; i++) {
		rp->tx_skbuff[i] = NULL;
		rp->tx_ring[i].tx_status = 0;
		rp->tx_ring[i].desc_length = cpu_to_le32(TXDESC);
		next += sizeof(struct tx_desc);
		rp->tx_ring[i].next_desc = cpu_to_le32(next);
		if (rp->quirks & rqRhineI)
			rp->tx_buf[i] = &rp->tx_bufs[i * PKT_BUF_SZ];
	}
	rp->tx_ring[i-1].next_desc = cpu_to_le32(rp->tx_ring_dma);

}

static void free_tbufs(struct net_device* dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	int i;

	for (i = 0; i < TX_RING_SIZE; i++) {
		rp->tx_ring[i].tx_status = 0;
		rp->tx_ring[i].desc_length = cpu_to_le32(TXDESC);
		rp->tx_ring[i].addr = cpu_to_le32(0xBADF00D0); 
		if (rp->tx_skbuff[i]) {
			if (rp->tx_skbuff_dma[i]) {
				pci_unmap_single(rp->pdev,
						 rp->tx_skbuff_dma[i],
						 rp->tx_skbuff[i]->len,
						 PCI_DMA_TODEVICE);
			}
			dev_kfree_skb(rp->tx_skbuff[i]);
		}
		rp->tx_skbuff[i] = NULL;
		rp->tx_buf[i] = NULL;
	}
}

static void rhine_check_media(struct net_device *dev, unsigned int init_media)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;

	mii_check_media(&rp->mii_if, debug, init_media);

	if (rp->mii_if.full_duplex)
	    iowrite8(ioread8(ioaddr + ChipCmd1) | Cmd1FDuplex,
		   ioaddr + ChipCmd1);
	else
	    iowrite8(ioread8(ioaddr + ChipCmd1) & ~Cmd1FDuplex,
		   ioaddr + ChipCmd1);
	if (debug > 1)
		printk(KERN_INFO "%s: force_media %d, carrier %d\n", dev->name,
			rp->mii_if.force_media, netif_carrier_ok(dev));
}


static void rhine_set_carrier(struct mii_if_info *mii)
{
	if (mii->force_media) {
		
		if (!netif_carrier_ok(mii->dev))
			netif_carrier_on(mii->dev);
	}
	else	
		rhine_check_media(mii->dev, 0);
	if (debug > 1)
		printk(KERN_INFO "%s: force_media %d, carrier %d\n",
		       mii->dev->name, mii->force_media,
		       netif_carrier_ok(mii->dev));
}

static void init_registers(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;
	int i;

	for (i = 0; i < 6; i++)
		iowrite8(dev->dev_addr[i], ioaddr + StationAddr + i);

	
	iowrite16(0x0006, ioaddr + PCIBusConfig);	
	
	iowrite8(0x20, ioaddr + TxConfig);
	rp->tx_thresh = 0x20;
	rp->rx_thresh = 0x60;		

	iowrite32(rp->rx_ring_dma, ioaddr + RxRingPtr);
	iowrite32(rp->tx_ring_dma, ioaddr + TxRingPtr);

	rhine_set_rx_mode(dev);

	napi_enable(&rp->napi);

	
	iowrite16(IntrRxDone | IntrRxErr | IntrRxEmpty| IntrRxOverflow |
	       IntrRxDropped | IntrRxNoBuf | IntrTxAborted |
	       IntrTxDone | IntrTxError | IntrTxUnderrun |
	       IntrPCIErr | IntrStatsMax | IntrLinkChange,
	       ioaddr + IntrEnable);

	iowrite16(CmdStart | CmdTxOn | CmdRxOn | (Cmd1NoTxPoll << 8),
	       ioaddr + ChipCmd);
	rhine_check_media(dev, 1);
}


static void rhine_enable_linkmon(void __iomem *ioaddr)
{
	iowrite8(0, ioaddr + MIICmd);
	iowrite8(MII_BMSR, ioaddr + MIIRegAddr);
	iowrite8(0x80, ioaddr + MIICmd);

	RHINE_WAIT_FOR((ioread8(ioaddr + MIIRegAddr) & 0x20));

	iowrite8(MII_BMSR | 0x40, ioaddr + MIIRegAddr);
}


static void rhine_disable_linkmon(void __iomem *ioaddr, u32 quirks)
{
	iowrite8(0, ioaddr + MIICmd);

	if (quirks & rqRhineI) {
		iowrite8(0x01, ioaddr + MIIRegAddr);	

		
		mdelay(1);

		
		iowrite8(0x80, ioaddr + MIICmd);

		RHINE_WAIT_FOR(ioread8(ioaddr + MIIRegAddr) & 0x20);

		
		iowrite8(0, ioaddr + MIICmd);
	}
	else
		RHINE_WAIT_FOR(ioread8(ioaddr + MIIRegAddr) & 0x80);
}



static int mdio_read(struct net_device *dev, int phy_id, int regnum)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;
	int result;

	rhine_disable_linkmon(ioaddr, rp->quirks);

	
	iowrite8(phy_id, ioaddr + MIIPhyAddr);
	iowrite8(regnum, ioaddr + MIIRegAddr);
	iowrite8(0x40, ioaddr + MIICmd);		
	RHINE_WAIT_FOR(!(ioread8(ioaddr + MIICmd) & 0x40));
	result = ioread16(ioaddr + MIIData);

	rhine_enable_linkmon(ioaddr);
	return result;
}

static void mdio_write(struct net_device *dev, int phy_id, int regnum, int value)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;

	rhine_disable_linkmon(ioaddr, rp->quirks);

	
	iowrite8(phy_id, ioaddr + MIIPhyAddr);
	iowrite8(regnum, ioaddr + MIIRegAddr);
	iowrite16(value, ioaddr + MIIData);
	iowrite8(0x20, ioaddr + MIICmd);		
	RHINE_WAIT_FOR(!(ioread8(ioaddr + MIICmd) & 0x20));

	rhine_enable_linkmon(ioaddr);
}

static int rhine_open(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;
	int rc;

	rc = request_irq(rp->pdev->irq, &rhine_interrupt, IRQF_SHARED, dev->name,
			dev);
	if (rc)
		return rc;

	if (debug > 1)
		printk(KERN_DEBUG "%s: rhine_open() irq %d.\n",
		       dev->name, rp->pdev->irq);

	rc = alloc_ring(dev);
	if (rc) {
		free_irq(rp->pdev->irq, dev);
		return rc;
	}
	alloc_rbufs(dev);
	alloc_tbufs(dev);
	rhine_chip_reset(dev);
	init_registers(dev);
	if (debug > 2)
		printk(KERN_DEBUG "%s: Done rhine_open(), status %4.4x "
		       "MII status: %4.4x.\n",
		       dev->name, ioread16(ioaddr + ChipCmd),
		       mdio_read(dev, rp->mii_if.phy_id, MII_BMSR));

	netif_start_queue(dev);

	return 0;
}

static void rhine_tx_timeout(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;

	printk(KERN_WARNING "%s: Transmit timed out, status %4.4x, PHY status "
	       "%4.4x, resetting...\n",
	       dev->name, ioread16(ioaddr + IntrStatus),
	       mdio_read(dev, rp->mii_if.phy_id, MII_BMSR));

	
	disable_irq(rp->pdev->irq);

	napi_disable(&rp->napi);

	spin_lock(&rp->lock);

	
	free_tbufs(dev);
	free_rbufs(dev);
	alloc_tbufs(dev);
	alloc_rbufs(dev);

	
	rhine_chip_reset(dev);
	init_registers(dev);

	spin_unlock(&rp->lock);
	enable_irq(rp->pdev->irq);

	dev->trans_start = jiffies;
	dev->stats.tx_errors++;
	netif_wake_queue(dev);
}

static netdev_tx_t rhine_start_tx(struct sk_buff *skb,
				  struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;
	unsigned entry;
	unsigned long flags;

	

	
	entry = rp->cur_tx % TX_RING_SIZE;

	if (skb_padto(skb, ETH_ZLEN))
		return NETDEV_TX_OK;

	rp->tx_skbuff[entry] = skb;

	if ((rp->quirks & rqRhineI) &&
	    (((unsigned long)skb->data & 3) || skb_shinfo(skb)->nr_frags != 0 || skb->ip_summed == CHECKSUM_PARTIAL)) {
		
		if (skb->len > PKT_BUF_SZ) {
			
			dev_kfree_skb(skb);
			rp->tx_skbuff[entry] = NULL;
			dev->stats.tx_dropped++;
			return NETDEV_TX_OK;
		}

		
		skb_copy_and_csum_dev(skb, rp->tx_buf[entry]);
		if (skb->len < ETH_ZLEN)
			memset(rp->tx_buf[entry] + skb->len, 0,
			       ETH_ZLEN - skb->len);
		rp->tx_skbuff_dma[entry] = 0;
		rp->tx_ring[entry].addr = cpu_to_le32(rp->tx_bufs_dma +
						      (rp->tx_buf[entry] -
						       rp->tx_bufs));
	} else {
		rp->tx_skbuff_dma[entry] =
			pci_map_single(rp->pdev, skb->data, skb->len,
				       PCI_DMA_TODEVICE);
		rp->tx_ring[entry].addr = cpu_to_le32(rp->tx_skbuff_dma[entry]);
	}

	rp->tx_ring[entry].desc_length =
		cpu_to_le32(TXDESC | (skb->len >= ETH_ZLEN ? skb->len : ETH_ZLEN));

	
	spin_lock_irqsave(&rp->lock, flags);
	wmb();
	rp->tx_ring[entry].tx_status = cpu_to_le32(DescOwn);
	wmb();

	rp->cur_tx++;

	

	
	iowrite8(ioread8(ioaddr + ChipCmd1) | Cmd1TxDemand,
	       ioaddr + ChipCmd1);
	IOSYNC;

	if (rp->cur_tx == rp->dirty_tx + TX_QUEUE_LEN)
		netif_stop_queue(dev);

	dev->trans_start = jiffies;

	spin_unlock_irqrestore(&rp->lock, flags);

	if (debug > 4) {
		printk(KERN_DEBUG "%s: Transmit frame #%d queued in slot %d.\n",
		       dev->name, rp->cur_tx-1, entry);
	}
	return NETDEV_TX_OK;
}


static irqreturn_t rhine_interrupt(int irq, void *dev_instance)
{
	struct net_device *dev = dev_instance;
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;
	u32 intr_status;
	int boguscnt = max_interrupt_work;
	int handled = 0;

	while ((intr_status = get_intr_status(dev))) {
		handled = 1;

		
		if (intr_status & IntrTxDescRace)
			iowrite8(0x08, ioaddr + IntrStatus2);
		iowrite16(intr_status & 0xffff, ioaddr + IntrStatus);
		IOSYNC;

		if (debug > 4)
			printk(KERN_DEBUG "%s: Interrupt, status %8.8x.\n",
			       dev->name, intr_status);

		if (intr_status & (IntrRxDone | IntrRxErr | IntrRxDropped |
				   IntrRxWakeUp | IntrRxEmpty | IntrRxNoBuf)) {
			iowrite16(IntrTxAborted |
				  IntrTxDone | IntrTxError | IntrTxUnderrun |
				  IntrPCIErr | IntrStatsMax | IntrLinkChange,
				  ioaddr + IntrEnable);

			napi_schedule(&rp->napi);
		}

		if (intr_status & (IntrTxErrSummary | IntrTxDone)) {
			if (intr_status & IntrTxErrSummary) {
				
				RHINE_WAIT_FOR(!(ioread8(ioaddr+ChipCmd) & CmdTxOn));
				if (debug > 2 &&
				    ioread8(ioaddr+ChipCmd) & CmdTxOn)
					printk(KERN_WARNING "%s: "
					       "rhine_interrupt() Tx engine "
					       "still on.\n", dev->name);
			}
			rhine_tx(dev);
		}

		
		if (intr_status & (IntrPCIErr | IntrLinkChange |
				   IntrStatsMax | IntrTxError | IntrTxAborted |
				   IntrTxUnderrun | IntrTxDescRace))
			rhine_error(dev, intr_status);

		if (--boguscnt < 0) {
			printk(KERN_WARNING "%s: Too much work at interrupt, "
			       "status=%#8.8x.\n",
			       dev->name, intr_status);
			break;
		}
	}

	if (debug > 3)
		printk(KERN_DEBUG "%s: exiting interrupt, status=%8.8x.\n",
		       dev->name, ioread16(ioaddr + IntrStatus));
	return IRQ_RETVAL(handled);
}


static void rhine_tx(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	int txstatus = 0, entry = rp->dirty_tx % TX_RING_SIZE;

	spin_lock(&rp->lock);

	
	while (rp->dirty_tx != rp->cur_tx) {
		txstatus = le32_to_cpu(rp->tx_ring[entry].tx_status);
		if (debug > 6)
			printk(KERN_DEBUG "Tx scavenge %d status %8.8x.\n",
			       entry, txstatus);
		if (txstatus & DescOwn)
			break;
		if (txstatus & 0x8000) {
			if (debug > 1)
				printk(KERN_DEBUG "%s: Transmit error, "
				       "Tx status %8.8x.\n",
				       dev->name, txstatus);
			dev->stats.tx_errors++;
			if (txstatus & 0x0400)
				dev->stats.tx_carrier_errors++;
			if (txstatus & 0x0200)
				dev->stats.tx_window_errors++;
			if (txstatus & 0x0100)
				dev->stats.tx_aborted_errors++;
			if (txstatus & 0x0080)
				dev->stats.tx_heartbeat_errors++;
			if (((rp->quirks & rqRhineI) && txstatus & 0x0002) ||
			    (txstatus & 0x0800) || (txstatus & 0x1000)) {
				dev->stats.tx_fifo_errors++;
				rp->tx_ring[entry].tx_status = cpu_to_le32(DescOwn);
				break; 
			}
			
		} else {
			if (rp->quirks & rqRhineI)
				dev->stats.collisions += (txstatus >> 3) & 0x0F;
			else
				dev->stats.collisions += txstatus & 0x0F;
			if (debug > 6)
				printk(KERN_DEBUG "collisions: %1.1x:%1.1x\n",
				       (txstatus >> 3) & 0xF,
				       txstatus & 0xF);
			dev->stats.tx_bytes += rp->tx_skbuff[entry]->len;
			dev->stats.tx_packets++;
		}
		
		if (rp->tx_skbuff_dma[entry]) {
			pci_unmap_single(rp->pdev,
					 rp->tx_skbuff_dma[entry],
					 rp->tx_skbuff[entry]->len,
					 PCI_DMA_TODEVICE);
		}
		dev_kfree_skb_irq(rp->tx_skbuff[entry]);
		rp->tx_skbuff[entry] = NULL;
		entry = (++rp->dirty_tx) % TX_RING_SIZE;
	}
	if ((rp->cur_tx - rp->dirty_tx) < TX_QUEUE_LEN - 4)
		netif_wake_queue(dev);

	spin_unlock(&rp->lock);
}


static int rhine_rx(struct net_device *dev, int limit)
{
	struct rhine_private *rp = netdev_priv(dev);
	int count;
	int entry = rp->cur_rx % RX_RING_SIZE;

	if (debug > 4) {
		printk(KERN_DEBUG "%s: rhine_rx(), entry %d status %8.8x.\n",
		       dev->name, entry,
		       le32_to_cpu(rp->rx_head_desc->rx_status));
	}

	
	for (count = 0; count < limit; ++count) {
		struct rx_desc *desc = rp->rx_head_desc;
		u32 desc_status = le32_to_cpu(desc->rx_status);
		int data_size = desc_status >> 16;

		if (desc_status & DescOwn)
			break;

		if (debug > 4)
			printk(KERN_DEBUG "rhine_rx() status is %8.8x.\n",
			       desc_status);

		if ((desc_status & (RxWholePkt | RxErr)) != RxWholePkt) {
			if ((desc_status & RxWholePkt) != RxWholePkt) {
				printk(KERN_WARNING "%s: Oversized Ethernet "
				       "frame spanned multiple buffers, entry "
				       "%#x length %d status %8.8x!\n",
				       dev->name, entry, data_size,
				       desc_status);
				printk(KERN_WARNING "%s: Oversized Ethernet "
				       "frame %p vs %p.\n", dev->name,
				       rp->rx_head_desc, &rp->rx_ring[entry]);
				dev->stats.rx_length_errors++;
			} else if (desc_status & RxErr) {
				
				if (debug > 2)
					printk(KERN_DEBUG "rhine_rx() Rx "
					       "error was %8.8x.\n",
					       desc_status);
				dev->stats.rx_errors++;
				if (desc_status & 0x0030)
					dev->stats.rx_length_errors++;
				if (desc_status & 0x0048)
					dev->stats.rx_fifo_errors++;
				if (desc_status & 0x0004)
					dev->stats.rx_frame_errors++;
				if (desc_status & 0x0002) {
					
					spin_lock(&rp->lock);
					dev->stats.rx_crc_errors++;
					spin_unlock(&rp->lock);
				}
			}
		} else {
			struct sk_buff *skb;
			
			int pkt_len = data_size - 4;

			
			if (pkt_len < rx_copybreak &&
				(skb = netdev_alloc_skb(dev, pkt_len + NET_IP_ALIGN)) != NULL) {
				skb_reserve(skb, NET_IP_ALIGN);	
				pci_dma_sync_single_for_cpu(rp->pdev,
							    rp->rx_skbuff_dma[entry],
							    rp->rx_buf_sz,
							    PCI_DMA_FROMDEVICE);

				skb_copy_to_linear_data(skb,
						 rp->rx_skbuff[entry]->data,
						 pkt_len);
				skb_put(skb, pkt_len);
				pci_dma_sync_single_for_device(rp->pdev,
							       rp->rx_skbuff_dma[entry],
							       rp->rx_buf_sz,
							       PCI_DMA_FROMDEVICE);
			} else {
				skb = rp->rx_skbuff[entry];
				if (skb == NULL) {
					printk(KERN_ERR "%s: Inconsistent Rx "
					       "descriptor chain.\n",
					       dev->name);
					break;
				}
				rp->rx_skbuff[entry] = NULL;
				skb_put(skb, pkt_len);
				pci_unmap_single(rp->pdev,
						 rp->rx_skbuff_dma[entry],
						 rp->rx_buf_sz,
						 PCI_DMA_FROMDEVICE);
			}
			skb->protocol = eth_type_trans(skb, dev);
			netif_receive_skb(skb);
			dev->stats.rx_bytes += pkt_len;
			dev->stats.rx_packets++;
		}
		entry = (++rp->cur_rx) % RX_RING_SIZE;
		rp->rx_head_desc = &rp->rx_ring[entry];
	}

	
	for (; rp->cur_rx - rp->dirty_rx > 0; rp->dirty_rx++) {
		struct sk_buff *skb;
		entry = rp->dirty_rx % RX_RING_SIZE;
		if (rp->rx_skbuff[entry] == NULL) {
			skb = netdev_alloc_skb(dev, rp->rx_buf_sz);
			rp->rx_skbuff[entry] = skb;
			if (skb == NULL)
				break;	
			skb->dev = dev;	
			rp->rx_skbuff_dma[entry] =
				pci_map_single(rp->pdev, skb->data,
					       rp->rx_buf_sz,
					       PCI_DMA_FROMDEVICE);
			rp->rx_ring[entry].addr = cpu_to_le32(rp->rx_skbuff_dma[entry]);
		}
		rp->rx_ring[entry].rx_status = cpu_to_le32(DescOwn);
	}

	return count;
}


static inline void clear_tally_counters(void __iomem *ioaddr)
{
	iowrite32(0, ioaddr + RxMissed);
	ioread16(ioaddr + RxCRCErrs);
	ioread16(ioaddr + RxMissed);
}

static void rhine_restart_tx(struct net_device *dev) {
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;
	int entry = rp->dirty_tx % TX_RING_SIZE;
	u32 intr_status;

	
	intr_status = get_intr_status(dev);

	if ((intr_status & IntrTxErrSummary) == 0) {

		
		iowrite32(rp->tx_ring_dma + entry * sizeof(struct tx_desc),
		       ioaddr + TxRingPtr);

		iowrite8(ioread8(ioaddr + ChipCmd) | CmdTxOn,
		       ioaddr + ChipCmd);
		iowrite8(ioread8(ioaddr + ChipCmd1) | Cmd1TxDemand,
		       ioaddr + ChipCmd1);
		IOSYNC;
	}
	else {
		
		if (debug > 1)
			printk(KERN_WARNING "%s: rhine_restart_tx() "
			       "Another error occured %8.8x.\n",
			       dev->name, intr_status);
	}

}

static void rhine_error(struct net_device *dev, int intr_status)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;

	spin_lock(&rp->lock);

	if (intr_status & IntrLinkChange)
		rhine_check_media(dev, 0);
	if (intr_status & IntrStatsMax) {
		dev->stats.rx_crc_errors += ioread16(ioaddr + RxCRCErrs);
		dev->stats.rx_missed_errors += ioread16(ioaddr + RxMissed);
		clear_tally_counters(ioaddr);
	}
	if (intr_status & IntrTxAborted) {
		if (debug > 1)
			printk(KERN_INFO "%s: Abort %8.8x, frame dropped.\n",
			       dev->name, intr_status);
	}
	if (intr_status & IntrTxUnderrun) {
		if (rp->tx_thresh < 0xE0)
			iowrite8(rp->tx_thresh += 0x20, ioaddr + TxConfig);
		if (debug > 1)
			printk(KERN_INFO "%s: Transmitter underrun, Tx "
			       "threshold now %2.2x.\n",
			       dev->name, rp->tx_thresh);
	}
	if (intr_status & IntrTxDescRace) {
		if (debug > 2)
			printk(KERN_INFO "%s: Tx descriptor write-back race.\n",
			       dev->name);
	}
	if ((intr_status & IntrTxError) &&
	    (intr_status & (IntrTxAborted |
	     IntrTxUnderrun | IntrTxDescRace)) == 0) {
		if (rp->tx_thresh < 0xE0) {
			iowrite8(rp->tx_thresh += 0x20, ioaddr + TxConfig);
		}
		if (debug > 1)
			printk(KERN_INFO "%s: Unspecified error. Tx "
			       "threshold now %2.2x.\n",
			       dev->name, rp->tx_thresh);
	}
	if (intr_status & (IntrTxAborted | IntrTxUnderrun | IntrTxDescRace |
			   IntrTxError))
		rhine_restart_tx(dev);

	if (intr_status & ~(IntrLinkChange | IntrStatsMax | IntrTxUnderrun |
			    IntrTxError | IntrTxAborted | IntrNormalSummary |
			    IntrTxDescRace)) {
		if (debug > 1)
			printk(KERN_ERR "%s: Something Wicked happened! "
			       "%8.8x.\n", dev->name, intr_status);
	}

	spin_unlock(&rp->lock);
}

static struct net_device_stats *rhine_get_stats(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;
	unsigned long flags;

	spin_lock_irqsave(&rp->lock, flags);
	dev->stats.rx_crc_errors += ioread16(ioaddr + RxCRCErrs);
	dev->stats.rx_missed_errors += ioread16(ioaddr + RxMissed);
	clear_tally_counters(ioaddr);
	spin_unlock_irqrestore(&rp->lock, flags);

	return &dev->stats;
}

static void rhine_set_rx_mode(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;
	u32 mc_filter[2];	
	u8 rx_mode;		

	if (dev->flags & IFF_PROMISC) {		
		rx_mode = 0x1C;
		iowrite32(0xffffffff, ioaddr + MulticastFilter0);
		iowrite32(0xffffffff, ioaddr + MulticastFilter1);
	} else if ((dev->mc_count > multicast_filter_limit)
		   || (dev->flags & IFF_ALLMULTI)) {
		
		iowrite32(0xffffffff, ioaddr + MulticastFilter0);
		iowrite32(0xffffffff, ioaddr + MulticastFilter1);
		rx_mode = 0x0C;
	} else {
		struct dev_mc_list *mclist;
		int i;
		memset(mc_filter, 0, sizeof(mc_filter));
		for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;
		     i++, mclist = mclist->next) {
			int bit_nr = ether_crc(ETH_ALEN, mclist->dmi_addr) >> 26;

			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 31);
		}
		iowrite32(mc_filter[0], ioaddr + MulticastFilter0);
		iowrite32(mc_filter[1], ioaddr + MulticastFilter1);
		rx_mode = 0x0C;
	}
	iowrite8(rp->rx_thresh | rx_mode, ioaddr + RxConfig);
}

static void netdev_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct rhine_private *rp = netdev_priv(dev);

	strcpy(info->driver, DRV_NAME);
	strcpy(info->version, DRV_VERSION);
	strcpy(info->bus_info, pci_name(rp->pdev));
}

static int netdev_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct rhine_private *rp = netdev_priv(dev);
	int rc;

	spin_lock_irq(&rp->lock);
	rc = mii_ethtool_gset(&rp->mii_if, cmd);
	spin_unlock_irq(&rp->lock);

	return rc;
}

static int netdev_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct rhine_private *rp = netdev_priv(dev);
	int rc;

	spin_lock_irq(&rp->lock);
	rc = mii_ethtool_sset(&rp->mii_if, cmd);
	spin_unlock_irq(&rp->lock);
	rhine_set_carrier(&rp->mii_if);

	return rc;
}

static int netdev_nway_reset(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);

	return mii_nway_restart(&rp->mii_if);
}

static u32 netdev_get_link(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);

	return mii_link_ok(&rp->mii_if);
}

static u32 netdev_get_msglevel(struct net_device *dev)
{
	return debug;
}

static void netdev_set_msglevel(struct net_device *dev, u32 value)
{
	debug = value;
}

static void rhine_get_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
	struct rhine_private *rp = netdev_priv(dev);

	if (!(rp->quirks & rqWOL))
		return;

	spin_lock_irq(&rp->lock);
	wol->supported = WAKE_PHY | WAKE_MAGIC |
			 WAKE_UCAST | WAKE_MCAST | WAKE_BCAST;	
	wol->wolopts = rp->wolopts;
	spin_unlock_irq(&rp->lock);
}

static int rhine_set_wol(struct net_device *dev, struct ethtool_wolinfo *wol)
{
	struct rhine_private *rp = netdev_priv(dev);
	u32 support = WAKE_PHY | WAKE_MAGIC |
		      WAKE_UCAST | WAKE_MCAST | WAKE_BCAST;	

	if (!(rp->quirks & rqWOL))
		return -EINVAL;

	if (wol->wolopts & ~support)
		return -EINVAL;

	spin_lock_irq(&rp->lock);
	rp->wolopts = wol->wolopts;
	spin_unlock_irq(&rp->lock);

	return 0;
}

static const struct ethtool_ops netdev_ethtool_ops = {
	.get_drvinfo		= netdev_get_drvinfo,
	.get_settings		= netdev_get_settings,
	.set_settings		= netdev_set_settings,
	.nway_reset		= netdev_nway_reset,
	.get_link		= netdev_get_link,
	.get_msglevel		= netdev_get_msglevel,
	.set_msglevel		= netdev_set_msglevel,
	.get_wol		= rhine_get_wol,
	.set_wol		= rhine_set_wol,
};

static int netdev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct rhine_private *rp = netdev_priv(dev);
	int rc;

	if (!netif_running(dev))
		return -EINVAL;

	spin_lock_irq(&rp->lock);
	rc = generic_mii_ioctl(&rp->mii_if, if_mii(rq), cmd, NULL);
	spin_unlock_irq(&rp->lock);
	rhine_set_carrier(&rp->mii_if);

	return rc;
}

static int rhine_close(struct net_device *dev)
{
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;

	spin_lock_irq(&rp->lock);

	netif_stop_queue(dev);
	napi_disable(&rp->napi);

	if (debug > 1)
		printk(KERN_DEBUG "%s: Shutting down ethercard, "
		       "status was %4.4x.\n",
		       dev->name, ioread16(ioaddr + ChipCmd));

	
	iowrite8(rp->tx_thresh | 0x02, ioaddr + TxConfig);

	
	iowrite16(0x0000, ioaddr + IntrEnable);

	
	iowrite16(CmdStop, ioaddr + ChipCmd);

	spin_unlock_irq(&rp->lock);

	free_irq(rp->pdev->irq, dev);
	free_rbufs(dev);
	free_tbufs(dev);
	free_ring(dev);

	return 0;
}


static void __devexit rhine_remove_one(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct rhine_private *rp = netdev_priv(dev);

	unregister_netdev(dev);

	pci_iounmap(pdev, rp->base);
	pci_release_regions(pdev);

	free_netdev(dev);
	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);
}

static void rhine_shutdown (struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct rhine_private *rp = netdev_priv(dev);
	void __iomem *ioaddr = rp->base;

	if (!(rp->quirks & rqWOL))
		return; 

	rhine_power_init(dev);

	
	if (rp->quirks & rq6patterns)
		iowrite8(0x04, ioaddr + WOLcgClr);

	if (rp->wolopts & WAKE_MAGIC) {
		iowrite8(WOLmagic, ioaddr + WOLcrSet);
		
		iowrite8(ioread8(ioaddr + ConfigA) | 0x03, ioaddr + ConfigA);
	}

	if (rp->wolopts & (WAKE_BCAST|WAKE_MCAST))
		iowrite8(WOLbmcast, ioaddr + WOLcgSet);

	if (rp->wolopts & WAKE_PHY)
		iowrite8(WOLlnkon | WOLlnkoff, ioaddr + WOLcrSet);

	if (rp->wolopts & WAKE_UCAST)
		iowrite8(WOLucast, ioaddr + WOLcrSet);

	if (rp->wolopts) {
		
		iowrite8(0x01, ioaddr + PwcfgSet);
		iowrite8(ioread8(ioaddr + StickyHW) | 0x04, ioaddr + StickyHW);
	}

	
	if (!avoid_D3)
		iowrite8(ioread8(ioaddr + StickyHW) | 0x03, ioaddr + StickyHW);

	

}

#ifdef CONFIG_PM
static int rhine_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct rhine_private *rp = netdev_priv(dev);
	unsigned long flags;

	if (!netif_running(dev))
		return 0;

	napi_disable(&rp->napi);

	netif_device_detach(dev);
	pci_save_state(pdev);

	spin_lock_irqsave(&rp->lock, flags);
	rhine_shutdown(pdev);
	spin_unlock_irqrestore(&rp->lock, flags);

	free_irq(dev->irq, dev);
	return 0;
}

static int rhine_resume(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct rhine_private *rp = netdev_priv(dev);
	unsigned long flags;
	int ret;

	if (!netif_running(dev))
		return 0;

        if (request_irq(dev->irq, rhine_interrupt, IRQF_SHARED, dev->name, dev))
		printk(KERN_ERR "via-rhine %s: request_irq failed\n", dev->name);

	ret = pci_set_power_state(pdev, PCI_D0);
	if (debug > 1)
		printk(KERN_INFO "%s: Entering power state D0 %s (%d).\n",
			dev->name, ret ? "failed" : "succeeded", ret);

	pci_restore_state(pdev);

	spin_lock_irqsave(&rp->lock, flags);
#ifdef USE_MMIO
	enable_mmio(rp->pioaddr, rp->quirks);
#endif
	rhine_power_init(dev);
	free_tbufs(dev);
	free_rbufs(dev);
	alloc_tbufs(dev);
	alloc_rbufs(dev);
	init_registers(dev);
	spin_unlock_irqrestore(&rp->lock, flags);

	netif_device_attach(dev);

	return 0;
}
#endif 

static struct pci_driver rhine_driver = {
	.name		= DRV_NAME,
	.id_table	= rhine_pci_tbl,
	.probe		= rhine_init_one,
	.remove		= __devexit_p(rhine_remove_one),
#ifdef CONFIG_PM
	.suspend	= rhine_suspend,
	.resume		= rhine_resume,
#endif 
	.shutdown =	rhine_shutdown,
};

static struct dmi_system_id __initdata rhine_dmi_table[] = {
	{
		.ident = "EPIA-M",
		.matches = {
			DMI_MATCH(DMI_BIOS_VENDOR, "Award Software International, Inc."),
			DMI_MATCH(DMI_BIOS_VERSION, "6.00 PG"),
		},
	},
	{
		.ident = "KV7",
		.matches = {
			DMI_MATCH(DMI_BIOS_VENDOR, "Phoenix Technologies, LTD"),
			DMI_MATCH(DMI_BIOS_VERSION, "6.00 PG"),
		},
	},
	{ NULL }
};

static int __init rhine_init(void)
{

#ifdef MODULE
	printk(version);
#endif
	if (dmi_check_system(rhine_dmi_table)) {
		
		avoid_D3 = 1;
		printk(KERN_WARNING "%s: Broken BIOS detected, avoid_D3 "
				    "enabled.\n",
		       DRV_NAME);
	}
	else if (avoid_D3)
		printk(KERN_INFO "%s: avoid_D3 set.\n", DRV_NAME);

	return pci_register_driver(&rhine_driver);
}


static void __exit rhine_cleanup(void)
{
	pci_unregister_driver(&rhine_driver);
}


module_init(rhine_init);
module_exit(rhine_cleanup);
