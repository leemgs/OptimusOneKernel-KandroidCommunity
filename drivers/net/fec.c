

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ptrace.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <asm/cacheflush.h>

#ifndef CONFIG_ARCH_MXC
#include <asm/coldfire.h>
#include <asm/mcfsim.h>
#endif

#include "fec.h"

#ifdef CONFIG_ARCH_MXC
#include <mach/hardware.h>
#define FEC_ALIGNMENT	0xf
#else
#define FEC_ALIGNMENT	0x3
#endif


#if defined(CONFIG_M5272)
#define HAVE_mii_link_interrupt

static unsigned char	fec_mac_default[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


#if defined(CONFIG_NETtel)
#define	FEC_FLASHMAC	0xf0006006
#elif defined(CONFIG_GILBARCONAP) || defined(CONFIG_SCALES)
#define	FEC_FLASHMAC	0xf0006000
#elif defined(CONFIG_CANCam)
#define	FEC_FLASHMAC	0xf0020000
#elif defined (CONFIG_M5272C3)
#define	FEC_FLASHMAC	(0xffe04000 + 4)
#elif defined(CONFIG_MOD5272)
#define FEC_FLASHMAC 	0xffc0406b
#else
#define	FEC_FLASHMAC	0
#endif
#endif 



typedef struct {
	uint mii_data;
	void (*funct)(uint mii_reg, struct net_device *dev);
} phy_cmd_t;

typedef struct {
	uint id;
	char *name;

	const phy_cmd_t *config;
	const phy_cmd_t *startup;
	const phy_cmd_t *ack_int;
	const phy_cmd_t *shutdown;
} phy_info_t;


#define FEC_ENET_RX_PAGES	8
#define FEC_ENET_RX_FRSIZE	2048
#define FEC_ENET_RX_FRPPG	(PAGE_SIZE / FEC_ENET_RX_FRSIZE)
#define RX_RING_SIZE		(FEC_ENET_RX_FRPPG * FEC_ENET_RX_PAGES)
#define FEC_ENET_TX_FRSIZE	2048
#define FEC_ENET_TX_FRPPG	(PAGE_SIZE / FEC_ENET_TX_FRSIZE)
#define TX_RING_SIZE		16	
#define TX_RING_MOD_MASK	15	

#if (((RX_RING_SIZE + TX_RING_SIZE) * 8) > PAGE_SIZE)
#error "FEC: descriptor ring size constants too large"
#endif


#define FEC_ENET_HBERR	((uint)0x80000000)	
#define FEC_ENET_BABR	((uint)0x40000000)	
#define FEC_ENET_BABT	((uint)0x20000000)	
#define FEC_ENET_GRA	((uint)0x10000000)	
#define FEC_ENET_TXF	((uint)0x08000000)	
#define FEC_ENET_TXB	((uint)0x04000000)	
#define FEC_ENET_RXF	((uint)0x02000000)	
#define FEC_ENET_RXB	((uint)0x01000000)	
#define FEC_ENET_MII	((uint)0x00800000)	
#define FEC_ENET_EBERR	((uint)0x00400000)	


#define PKT_MAXBUF_SIZE		1518
#define PKT_MINBUF_SIZE		64
#define PKT_MAXBLR_SIZE		1520



#if defined(CONFIG_M523x) || defined(CONFIG_M527x) || defined(CONFIG_M528x) || \
    defined(CONFIG_M520x) || defined(CONFIG_M532x) || defined(CONFIG_ARCH_MXC)
#define	OPT_FRAME_SIZE	(PKT_MAXBUF_SIZE << 16)
#else
#define	OPT_FRAME_SIZE	0
#endif


struct fec_enet_private {
	
	void __iomem *hwp;

	struct net_device *netdev;

	struct clk *clk;

	
	unsigned char *tx_bounce[TX_RING_SIZE];
	struct	sk_buff* tx_skbuff[TX_RING_SIZE];
	struct	sk_buff* rx_skbuff[RX_RING_SIZE];
	ushort	skb_cur;
	ushort	skb_dirty;

	
	dma_addr_t	bd_dma;
	
	struct bufdesc	*rx_bd_base;
	struct bufdesc	*tx_bd_base;
	
	struct bufdesc	*cur_rx, *cur_tx; 
	
	struct bufdesc	*dirty_tx;

	uint	tx_full;
	
	spinlock_t hw_lock;
	
	spinlock_t mii_lock;

	uint	phy_id;
	uint	phy_id_done;
	uint	phy_status;
	uint	phy_speed;
	phy_info_t const	*phy;
	struct work_struct phy_task;

	uint	sequence_done;
	uint	mii_phy_task_queued;

	uint	phy_addr;

	int	index;
	int	opened;
	int	link;
	int	old_link;
	int	full_duplex;
};

static void fec_enet_mii(struct net_device *dev);
static irqreturn_t fec_enet_interrupt(int irq, void * dev_id);
static void fec_enet_tx(struct net_device *dev);
static void fec_enet_rx(struct net_device *dev);
static int fec_enet_close(struct net_device *dev);
static void fec_restart(struct net_device *dev, int duplex);
static void fec_stop(struct net_device *dev);



typedef struct mii_list {
	uint	mii_regval;
	void	(*mii_func)(uint val, struct net_device *dev);
	struct	mii_list *mii_next;
} mii_list_t;

#define		NMII	20
static mii_list_t	mii_cmds[NMII];
static mii_list_t	*mii_free;
static mii_list_t	*mii_head;
static mii_list_t	*mii_tail;

static int	mii_queue(struct net_device *dev, int request,
				void (*func)(uint, struct net_device *));


#define mk_mii_read(REG)	(0x60020000 | ((REG & 0x1f) << 18))
#define mk_mii_write(REG, VAL)	(0x50020000 | ((REG & 0x1f) << 18) | \
						(VAL & 0xffff))
#define mk_mii_end	0


#define TX_TIMEOUT (2 * HZ)



#define MII_REG_CR          0  
#define MII_REG_SR          1  
#define MII_REG_PHYIR1      2  
#define MII_REG_PHYIR2      3  
#define MII_REG_ANAR        4  
#define MII_REG_ANLPAR      5  
#define MII_REG_ANER        6  
#define MII_REG_ANNPTR      7  
#define MII_REG_ANLPRNPR    8  



#define PHY_CONF_ANE	0x0001  
#define PHY_CONF_LOOP	0x0002  
#define PHY_CONF_SPMASK	0x00f0  
#define PHY_CONF_10HDX	0x0010  
#define PHY_CONF_10FDX	0x0020  
#define PHY_CONF_100HDX	0x0040  
#define PHY_CONF_100FDX	0x0080  

#define PHY_STAT_LINK	0x0100  
#define PHY_STAT_FAULT	0x0200  
#define PHY_STAT_ANC	0x0400  
#define PHY_STAT_SPMASK	0xf000  
#define PHY_STAT_10HDX	0x1000  
#define PHY_STAT_10FDX	0x2000  
#define PHY_STAT_100HDX	0x4000  
#define PHY_STAT_100FDX	0x8000  


static int
fec_enet_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	struct bufdesc *bdp;
	void *bufaddr;
	unsigned short	status;
	unsigned long flags;

	if (!fep->link) {
		
		return NETDEV_TX_BUSY;
	}

	spin_lock_irqsave(&fep->hw_lock, flags);
	
	bdp = fep->cur_tx;

	status = bdp->cbd_sc;

	if (status & BD_ENET_TX_READY) {
		
		printk("%s: tx queue full!.\n", dev->name);
		spin_unlock_irqrestore(&fep->hw_lock, flags);
		return NETDEV_TX_BUSY;
	}

	
	status &= ~BD_ENET_TX_STATS;

	
	bufaddr = skb->data;
	bdp->cbd_datlen = skb->len;

	
	if (((unsigned long) bufaddr) & FEC_ALIGNMENT) {
		unsigned int index;
		index = bdp - fep->tx_bd_base;
		memcpy(fep->tx_bounce[index], (void *)skb->data, skb->len);
		bufaddr = fep->tx_bounce[index];
	}

	
	fep->tx_skbuff[fep->skb_cur] = skb;

	dev->stats.tx_bytes += skb->len;
	fep->skb_cur = (fep->skb_cur+1) & TX_RING_MOD_MASK;

	
	bdp->cbd_bufaddr = dma_map_single(&dev->dev, bufaddr,
			FEC_ENET_TX_FRSIZE, DMA_TO_DEVICE);

	
	status |= (BD_ENET_TX_READY | BD_ENET_TX_INTR
			| BD_ENET_TX_LAST | BD_ENET_TX_TC);
	bdp->cbd_sc = status;

	dev->trans_start = jiffies;

	
	writel(0, fep->hwp + FEC_X_DES_ACTIVE);

	
	if (status & BD_ENET_TX_WRAP)
		bdp = fep->tx_bd_base;
	else
		bdp++;

	if (bdp == fep->dirty_tx) {
		fep->tx_full = 1;
		netif_stop_queue(dev);
	}

	fep->cur_tx = bdp;

	spin_unlock_irqrestore(&fep->hw_lock, flags);

	return NETDEV_TX_OK;
}

static void
fec_timeout(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);

	dev->stats.tx_errors++;

	fec_restart(dev, fep->full_duplex);
	netif_wake_queue(dev);
}

static irqreturn_t
fec_enet_interrupt(int irq, void * dev_id)
{
	struct	net_device *dev = dev_id;
	struct fec_enet_private *fep = netdev_priv(dev);
	uint	int_events;
	irqreturn_t ret = IRQ_NONE;

	do {
		int_events = readl(fep->hwp + FEC_IEVENT);
		writel(int_events, fep->hwp + FEC_IEVENT);

		if (int_events & FEC_ENET_RXF) {
			ret = IRQ_HANDLED;
			fec_enet_rx(dev);
		}

		
		if (int_events & FEC_ENET_TXF) {
			ret = IRQ_HANDLED;
			fec_enet_tx(dev);
		}

		if (int_events & FEC_ENET_MII) {
			ret = IRQ_HANDLED;
			fec_enet_mii(dev);
		}

	} while (int_events);

	return ret;
}


static void
fec_enet_tx(struct net_device *dev)
{
	struct	fec_enet_private *fep;
	struct bufdesc *bdp;
	unsigned short status;
	struct	sk_buff	*skb;

	fep = netdev_priv(dev);
	spin_lock(&fep->hw_lock);
	bdp = fep->dirty_tx;

	while (((status = bdp->cbd_sc) & BD_ENET_TX_READY) == 0) {
		if (bdp == fep->cur_tx && fep->tx_full == 0)
			break;

		dma_unmap_single(&dev->dev, bdp->cbd_bufaddr, FEC_ENET_TX_FRSIZE, DMA_TO_DEVICE);
		bdp->cbd_bufaddr = 0;

		skb = fep->tx_skbuff[fep->skb_dirty];
		
		if (status & (BD_ENET_TX_HB | BD_ENET_TX_LC |
				   BD_ENET_TX_RL | BD_ENET_TX_UN |
				   BD_ENET_TX_CSL)) {
			dev->stats.tx_errors++;
			if (status & BD_ENET_TX_HB)  
				dev->stats.tx_heartbeat_errors++;
			if (status & BD_ENET_TX_LC)  
				dev->stats.tx_window_errors++;
			if (status & BD_ENET_TX_RL)  
				dev->stats.tx_aborted_errors++;
			if (status & BD_ENET_TX_UN)  
				dev->stats.tx_fifo_errors++;
			if (status & BD_ENET_TX_CSL) 
				dev->stats.tx_carrier_errors++;
		} else {
			dev->stats.tx_packets++;
		}

		if (status & BD_ENET_TX_READY)
			printk("HEY! Enet xmit interrupt and TX_READY.\n");

		
		if (status & BD_ENET_TX_DEF)
			dev->stats.collisions++;

		
		dev_kfree_skb_any(skb);
		fep->tx_skbuff[fep->skb_dirty] = NULL;
		fep->skb_dirty = (fep->skb_dirty + 1) & TX_RING_MOD_MASK;

		
		if (status & BD_ENET_TX_WRAP)
			bdp = fep->tx_bd_base;
		else
			bdp++;

		
		if (fep->tx_full) {
			fep->tx_full = 0;
			if (netif_queue_stopped(dev))
				netif_wake_queue(dev);
		}
	}
	fep->dirty_tx = bdp;
	spin_unlock(&fep->hw_lock);
}



static void
fec_enet_rx(struct net_device *dev)
{
	struct	fec_enet_private *fep = netdev_priv(dev);
	struct bufdesc *bdp;
	unsigned short status;
	struct	sk_buff	*skb;
	ushort	pkt_len;
	__u8 *data;

#ifdef CONFIG_M532x
	flush_cache_all();
#endif

	spin_lock(&fep->hw_lock);

	
	bdp = fep->cur_rx;

	while (!((status = bdp->cbd_sc) & BD_ENET_RX_EMPTY)) {

		
		if ((status & BD_ENET_RX_LAST) == 0)
			printk("FEC ENET: rcv is not +last\n");

		if (!fep->opened)
			goto rx_processing_done;

		
		if (status & (BD_ENET_RX_LG | BD_ENET_RX_SH | BD_ENET_RX_NO |
			   BD_ENET_RX_CR | BD_ENET_RX_OV)) {
			dev->stats.rx_errors++;
			if (status & (BD_ENET_RX_LG | BD_ENET_RX_SH)) {
				
				dev->stats.rx_length_errors++;
			}
			if (status & BD_ENET_RX_NO)	
				dev->stats.rx_frame_errors++;
			if (status & BD_ENET_RX_CR)	
				dev->stats.rx_crc_errors++;
			if (status & BD_ENET_RX_OV)	
				dev->stats.rx_fifo_errors++;
		}

		
		if (status & BD_ENET_RX_CL) {
			dev->stats.rx_errors++;
			dev->stats.rx_frame_errors++;
			goto rx_processing_done;
		}

		
		dev->stats.rx_packets++;
		pkt_len = bdp->cbd_datlen;
		dev->stats.rx_bytes += pkt_len;
		data = (__u8*)__va(bdp->cbd_bufaddr);

	        dma_unmap_single(NULL, bdp->cbd_bufaddr, bdp->cbd_datlen,
        			DMA_FROM_DEVICE);

		
		skb = dev_alloc_skb(pkt_len - 4 + NET_IP_ALIGN);

		if (unlikely(!skb)) {
			printk("%s: Memory squeeze, dropping packet.\n",
					dev->name);
			dev->stats.rx_dropped++;
		} else {
			skb_reserve(skb, NET_IP_ALIGN);
			skb_put(skb, pkt_len - 4);	
			skb_copy_to_linear_data(skb, data, pkt_len - 4);
			skb->protocol = eth_type_trans(skb, dev);
			netif_rx(skb);
		}

        	bdp->cbd_bufaddr = dma_map_single(NULL, data, bdp->cbd_datlen,
			DMA_FROM_DEVICE);
rx_processing_done:
		
		status &= ~BD_ENET_RX_STATS;

		
		status |= BD_ENET_RX_EMPTY;
		bdp->cbd_sc = status;

		
		if (status & BD_ENET_RX_WRAP)
			bdp = fep->rx_bd_base;
		else
			bdp++;
		
		writel(0, fep->hwp + FEC_R_DES_ACTIVE);
	}
	fep->cur_rx = bdp;

	spin_unlock(&fep->hw_lock);
}


static void
fec_enet_mii(struct net_device *dev)
{
	struct	fec_enet_private *fep;
	mii_list_t	*mip;

	fep = netdev_priv(dev);
	spin_lock(&fep->mii_lock);

	if ((mip = mii_head) == NULL) {
		printk("MII and no head!\n");
		goto unlock;
	}

	if (mip->mii_func != NULL)
		(*(mip->mii_func))(readl(fep->hwp + FEC_MII_DATA), dev);

	mii_head = mip->mii_next;
	mip->mii_next = mii_free;
	mii_free = mip;

	if ((mip = mii_head) != NULL)
		writel(mip->mii_regval, fep->hwp + FEC_MII_DATA);

unlock:
	spin_unlock(&fep->mii_lock);
}

static int
mii_queue_unlocked(struct net_device *dev, int regval,
		void (*func)(uint, struct net_device *))
{
	struct fec_enet_private *fep;
	mii_list_t	*mip;
	int		retval;

	
	fep = netdev_priv(dev);

	regval |= fep->phy_addr << 23;
	retval = 0;

	if ((mip = mii_free) != NULL) {
		mii_free = mip->mii_next;
		mip->mii_regval = regval;
		mip->mii_func = func;
		mip->mii_next = NULL;
		if (mii_head) {
			mii_tail->mii_next = mip;
			mii_tail = mip;
		} else {
			mii_head = mii_tail = mip;
			writel(regval, fep->hwp + FEC_MII_DATA);
		}
	} else {
		retval = 1;
	}

	return retval;
}

static int
mii_queue(struct net_device *dev, int regval,
		void (*func)(uint, struct net_device *))
{
	struct fec_enet_private *fep;
	unsigned long   flags;
	int             retval;
	fep = netdev_priv(dev);
	spin_lock_irqsave(&fep->mii_lock, flags);
	retval = mii_queue_unlocked(dev, regval, func);
	spin_unlock_irqrestore(&fep->mii_lock, flags);
	return retval;
}

static void mii_do_cmd(struct net_device *dev, const phy_cmd_t *c)
{
	if(!c)
		return;

	for (; c->mii_data != mk_mii_end; c++)
		mii_queue(dev, c->mii_data, c->funct);
}

static void mii_parse_sr(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile uint *s = &(fep->phy_status);
	uint status;

	status = *s & ~(PHY_STAT_LINK | PHY_STAT_FAULT | PHY_STAT_ANC);

	if (mii_reg & 0x0004)
		status |= PHY_STAT_LINK;
	if (mii_reg & 0x0010)
		status |= PHY_STAT_FAULT;
	if (mii_reg & 0x0020)
		status |= PHY_STAT_ANC;
	*s = status;
}

static void mii_parse_cr(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile uint *s = &(fep->phy_status);
	uint status;

	status = *s & ~(PHY_CONF_ANE | PHY_CONF_LOOP);

	if (mii_reg & 0x1000)
		status |= PHY_CONF_ANE;
	if (mii_reg & 0x4000)
		status |= PHY_CONF_LOOP;
	*s = status;
}

static void mii_parse_anar(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile uint *s = &(fep->phy_status);
	uint status;

	status = *s & ~(PHY_CONF_SPMASK);

	if (mii_reg & 0x0020)
		status |= PHY_CONF_10HDX;
	if (mii_reg & 0x0040)
		status |= PHY_CONF_10FDX;
	if (mii_reg & 0x0080)
		status |= PHY_CONF_100HDX;
	if (mii_reg & 0x00100)
		status |= PHY_CONF_100FDX;
	*s = status;
}




#define MII_LXT970_MIRROR    16  
#define MII_LXT970_IER       17  
#define MII_LXT970_ISR       18  
#define MII_LXT970_CONFIG    19  
#define MII_LXT970_CSR       20  

static void mii_parse_lxt970_csr(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile uint *s = &(fep->phy_status);
	uint status;

	status = *s & ~(PHY_STAT_SPMASK);
	if (mii_reg & 0x0800) {
		if (mii_reg & 0x1000)
			status |= PHY_STAT_100FDX;
		else
			status |= PHY_STAT_100HDX;
	} else {
		if (mii_reg & 0x1000)
			status |= PHY_STAT_10FDX;
		else
			status |= PHY_STAT_10HDX;
	}
	*s = status;
}

static phy_cmd_t const phy_cmd_lxt970_config[] = {
		{ mk_mii_read(MII_REG_CR), mii_parse_cr },
		{ mk_mii_read(MII_REG_ANAR), mii_parse_anar },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_lxt970_startup[] = { 
		{ mk_mii_write(MII_LXT970_IER, 0x0002), NULL },
		{ mk_mii_write(MII_REG_CR, 0x1200), NULL }, 
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_lxt970_ack_int[] = {
		
		{ mk_mii_read(MII_REG_SR), mii_parse_sr },
		{ mk_mii_read(MII_LXT970_ISR), NULL },

		
		{ mk_mii_read(MII_LXT970_CSR), mii_parse_lxt970_csr },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_lxt970_shutdown[] = { 
		{ mk_mii_write(MII_LXT970_IER, 0x0000), NULL },
		{ mk_mii_end, }
	};
static phy_info_t const phy_info_lxt970 = {
	.id = 0x07810000,
	.name = "LXT970",
	.config = phy_cmd_lxt970_config,
	.startup = phy_cmd_lxt970_startup,
	.ack_int = phy_cmd_lxt970_ack_int,
	.shutdown = phy_cmd_lxt970_shutdown
};






#define MII_LXT971_PCR       16  
#define MII_LXT971_SR2       17  
#define MII_LXT971_IER       18  
#define MII_LXT971_ISR       19  
#define MII_LXT971_LCR       20  
#define MII_LXT971_TCR       30  



static void mii_parse_lxt971_sr2(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile uint *s = &(fep->phy_status);
	uint status;

	status = *s & ~(PHY_STAT_SPMASK | PHY_STAT_LINK | PHY_STAT_ANC);

	if (mii_reg & 0x0400) {
		fep->link = 1;
		status |= PHY_STAT_LINK;
	} else {
		fep->link = 0;
	}
	if (mii_reg & 0x0080)
		status |= PHY_STAT_ANC;
	if (mii_reg & 0x4000) {
		if (mii_reg & 0x0200)
			status |= PHY_STAT_100FDX;
		else
			status |= PHY_STAT_100HDX;
	} else {
		if (mii_reg & 0x0200)
			status |= PHY_STAT_10FDX;
		else
			status |= PHY_STAT_10HDX;
	}
	if (mii_reg & 0x0008)
		status |= PHY_STAT_FAULT;

	*s = status;
}

static phy_cmd_t const phy_cmd_lxt971_config[] = {
		
		{ mk_mii_read(MII_REG_CR), mii_parse_cr },
		{ mk_mii_read(MII_REG_ANAR), mii_parse_anar },
		{ mk_mii_read(MII_LXT971_SR2), mii_parse_lxt971_sr2 },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_lxt971_startup[] = {  
		{ mk_mii_write(MII_LXT971_IER, 0x00f2), NULL },
		{ mk_mii_write(MII_REG_CR, 0x1200), NULL }, 
		{ mk_mii_write(MII_LXT971_LCR, 0xd422), NULL }, 
		
		{ mk_mii_read(MII_REG_SR), mii_parse_sr },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_lxt971_ack_int[] = {
		
		{ mk_mii_read(MII_LXT971_ISR), NULL },
		
		{ mk_mii_read(MII_REG_SR), mii_parse_sr },
		{ mk_mii_read(MII_LXT971_SR2), mii_parse_lxt971_sr2 },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_lxt971_shutdown[] = { 
		{ mk_mii_write(MII_LXT971_IER, 0x0000), NULL },
		{ mk_mii_end, }
	};
static phy_info_t const phy_info_lxt971 = {
	.id = 0x0001378e,
	.name = "LXT971",
	.config = phy_cmd_lxt971_config,
	.startup = phy_cmd_lxt971_startup,
	.ack_int = phy_cmd_lxt971_ack_int,
	.shutdown = phy_cmd_lxt971_shutdown
};






#define MII_QS6612_MCR       17  
#define MII_QS6612_FTR       27  
#define MII_QS6612_MCO       28  
#define MII_QS6612_ISR       29  
#define MII_QS6612_IMR       30  
#define MII_QS6612_PCR       31  

static void mii_parse_qs6612_pcr(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile uint *s = &(fep->phy_status);
	uint status;

	status = *s & ~(PHY_STAT_SPMASK);

	switch((mii_reg >> 2) & 7) {
	case 1: status |= PHY_STAT_10HDX; break;
	case 2: status |= PHY_STAT_100HDX; break;
	case 5: status |= PHY_STAT_10FDX; break;
	case 6: status |= PHY_STAT_100FDX; break;
}

	*s = status;
}

static phy_cmd_t const phy_cmd_qs6612_config[] = {
		
		{ mk_mii_write(MII_QS6612_PCR, 0x0dc0), NULL },

		
		{ mk_mii_read(MII_REG_CR), mii_parse_cr },
		{ mk_mii_read(MII_REG_ANAR), mii_parse_anar },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_qs6612_startup[] = {  
		{ mk_mii_write(MII_QS6612_IMR, 0x003a), NULL },
		{ mk_mii_write(MII_REG_CR, 0x1200), NULL }, 
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_qs6612_ack_int[] = {
		
		{ mk_mii_read(MII_QS6612_ISR), NULL },
		{ mk_mii_read(MII_REG_SR), mii_parse_sr },
		{ mk_mii_read(MII_REG_ANER), NULL },

		
		{ mk_mii_read(MII_QS6612_PCR), mii_parse_qs6612_pcr },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_qs6612_shutdown[] = { 
		{ mk_mii_write(MII_QS6612_IMR, 0x0000), NULL },
		{ mk_mii_end, }
	};
static phy_info_t const phy_info_qs6612 = {
	.id = 0x00181440,
	.name = "QS6612",
	.config = phy_cmd_qs6612_config,
	.startup = phy_cmd_qs6612_startup,
	.ack_int = phy_cmd_qs6612_ack_int,
	.shutdown = phy_cmd_qs6612_shutdown
};






#define MII_AM79C874_MFR       16  
#define MII_AM79C874_ICSR      17  
#define MII_AM79C874_DR        18  
#define MII_AM79C874_PMLR      19  
#define MII_AM79C874_MCR       21  
#define MII_AM79C874_DC        23  
#define MII_AM79C874_REC       24  

static void mii_parse_am79c874_dr(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile uint *s = &(fep->phy_status);
	uint status;

	status = *s & ~(PHY_STAT_SPMASK | PHY_STAT_ANC);

	if (mii_reg & 0x0080)
		status |= PHY_STAT_ANC;
	if (mii_reg & 0x0400)
		status |= ((mii_reg & 0x0800) ? PHY_STAT_100FDX : PHY_STAT_100HDX);
	else
		status |= ((mii_reg & 0x0800) ? PHY_STAT_10FDX : PHY_STAT_10HDX);

	*s = status;
}

static phy_cmd_t const phy_cmd_am79c874_config[] = {
		{ mk_mii_read(MII_REG_CR), mii_parse_cr },
		{ mk_mii_read(MII_REG_ANAR), mii_parse_anar },
		{ mk_mii_read(MII_AM79C874_DR), mii_parse_am79c874_dr },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_am79c874_startup[] = {  
		{ mk_mii_write(MII_AM79C874_ICSR, 0xff00), NULL },
		{ mk_mii_write(MII_REG_CR, 0x1200), NULL }, 
		{ mk_mii_read(MII_REG_SR), mii_parse_sr },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_am79c874_ack_int[] = {
		
		{ mk_mii_read(MII_REG_SR), mii_parse_sr },
		{ mk_mii_read(MII_AM79C874_DR), mii_parse_am79c874_dr },
		
		{ mk_mii_read(MII_AM79C874_ICSR), NULL },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_am79c874_shutdown[] = { 
		{ mk_mii_write(MII_AM79C874_ICSR, 0x0000), NULL },
		{ mk_mii_end, }
	};
static phy_info_t const phy_info_am79c874 = {
	.id = 0x00022561,
	.name = "AM79C874",
	.config = phy_cmd_am79c874_config,
	.startup = phy_cmd_am79c874_startup,
	.ack_int = phy_cmd_am79c874_ack_int,
	.shutdown = phy_cmd_am79c874_shutdown
};







#define MII_KS8721BL_RXERCR	21
#define MII_KS8721BL_ICSR	27
#define	MII_KS8721BL_PHYCR	31

static phy_cmd_t const phy_cmd_ks8721bl_config[] = {
		{ mk_mii_read(MII_REG_CR), mii_parse_cr },
		{ mk_mii_read(MII_REG_ANAR), mii_parse_anar },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_ks8721bl_startup[] = {  
		{ mk_mii_write(MII_KS8721BL_ICSR, 0xff00), NULL },
		{ mk_mii_write(MII_REG_CR, 0x1200), NULL }, 
		{ mk_mii_read(MII_REG_SR), mii_parse_sr },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_ks8721bl_ack_int[] = {
		
		{ mk_mii_read(MII_REG_SR), mii_parse_sr },
		
		{ mk_mii_read(MII_KS8721BL_ICSR), NULL },
		{ mk_mii_end, }
	};
static phy_cmd_t const phy_cmd_ks8721bl_shutdown[] = { 
		{ mk_mii_write(MII_KS8721BL_ICSR, 0x0000), NULL },
		{ mk_mii_end, }
	};
static phy_info_t const phy_info_ks8721bl = {
	.id = 0x00022161,
	.name = "KS8721BL",
	.config = phy_cmd_ks8721bl_config,
	.startup = phy_cmd_ks8721bl_startup,
	.ack_int = phy_cmd_ks8721bl_ack_int,
	.shutdown = phy_cmd_ks8721bl_shutdown
};




#define MII_DP8384X_PHYSTST    16  

static void mii_parse_dp8384x_sr2(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile uint *s = &(fep->phy_status);

	*s &= ~(PHY_STAT_SPMASK | PHY_STAT_LINK | PHY_STAT_ANC);

	
	if (mii_reg & 0x0001) {
		fep->link = 1;
		*s |= PHY_STAT_LINK;
	} else
		fep->link = 0;
	
	if (mii_reg & 0x0010)   
		*s |= PHY_STAT_ANC;
	if (mii_reg & 0x0002) {   
		if (mii_reg & 0x0004)   
			*s |= PHY_STAT_10FDX;
		else
			*s |= PHY_STAT_10HDX;
	} else {                  
		if (mii_reg & 0x0004)   
			*s |= PHY_STAT_100FDX;
		else
			*s |= PHY_STAT_100HDX;
	}
	if (mii_reg & 0x0008)
		*s |= PHY_STAT_FAULT;
}

static phy_info_t phy_info_dp83848= {
	0x020005c9,
	"DP83848",

	(const phy_cmd_t []) {  
		{ mk_mii_read(MII_REG_CR), mii_parse_cr },
		{ mk_mii_read(MII_REG_ANAR), mii_parse_anar },
		{ mk_mii_read(MII_DP8384X_PHYSTST), mii_parse_dp8384x_sr2 },
		{ mk_mii_end, }
	},
	(const phy_cmd_t []) {  
		{ mk_mii_write(MII_REG_CR, 0x1200), NULL }, 
		{ mk_mii_read(MII_REG_SR), mii_parse_sr },
		{ mk_mii_end, }
	},
	(const phy_cmd_t []) { 
		{ mk_mii_end, }
	},
	(const phy_cmd_t []) {  
		{ mk_mii_end, }
	},
};



static phy_info_t const * const phy_info[] = {
	&phy_info_lxt970,
	&phy_info_lxt971,
	&phy_info_qs6612,
	&phy_info_am79c874,
	&phy_info_ks8721bl,
	&phy_info_dp83848,
	NULL
};


#ifdef HAVE_mii_link_interrupt
static irqreturn_t
mii_link_interrupt(int irq, void * dev_id);


static void __inline__ fec_request_mii_intr(struct net_device *dev)
{
	if (request_irq(66, mii_link_interrupt, IRQF_DISABLED, "fec(MII)", dev) != 0)
		printk("FEC: Could not allocate fec(MII) IRQ(66)!\n");
}

static void __inline__ fec_disable_phy_intr(struct net_device *dev)
{
	free_irq(66, dev);
}
#endif

#ifdef CONFIG_M5272
static void __inline__ fec_get_mac(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	unsigned char *iap, tmpaddr[ETH_ALEN];

	if (FEC_FLASHMAC) {
		
		iap = (unsigned char *)FEC_FLASHMAC;
		if ((iap[0] == 0) && (iap[1] == 0) && (iap[2] == 0) &&
		    (iap[3] == 0) && (iap[4] == 0) && (iap[5] == 0))
			iap = fec_mac_default;
		if ((iap[0] == 0xff) && (iap[1] == 0xff) && (iap[2] == 0xff) &&
		    (iap[3] == 0xff) && (iap[4] == 0xff) && (iap[5] == 0xff))
			iap = fec_mac_default;
	} else {
		*((unsigned long *) &tmpaddr[0]) = readl(fep->hwp + FEC_ADDR_LOW);
		*((unsigned short *) &tmpaddr[4]) = (readl(fep->hwp + FEC_ADDR_HIGH) >> 16);
		iap = &tmpaddr[0];
	}

	memcpy(dev->dev_addr, iap, ETH_ALEN);

	
	if (iap == fec_mac_default)
		 dev->dev_addr[ETH_ALEN-1] = fec_mac_default[ETH_ALEN-1] + fep->index;
}
#endif



static void mii_display_status(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	volatile uint *s = &(fep->phy_status);

	if (!fep->link && !fep->old_link) {
		
		return;
	}

	printk("%s: status: ", dev->name);

	if (!fep->link) {
		printk("link down");
	} else {
		printk("link up");

		switch(*s & PHY_STAT_SPMASK) {
		case PHY_STAT_100FDX: printk(", 100MBit Full Duplex"); break;
		case PHY_STAT_100HDX: printk(", 100MBit Half Duplex"); break;
		case PHY_STAT_10FDX: printk(", 10MBit Full Duplex"); break;
		case PHY_STAT_10HDX: printk(", 10MBit Half Duplex"); break;
		default:
			printk(", Unknown speed/duplex");
		}

		if (*s & PHY_STAT_ANC)
			printk(", auto-negotiation complete");
	}

	if (*s & PHY_STAT_FAULT)
		printk(", remote fault");

	printk(".\n");
}

static void mii_display_config(struct work_struct *work)
{
	struct fec_enet_private *fep = container_of(work, struct fec_enet_private, phy_task);
	struct net_device *dev = fep->netdev;
	uint status = fep->phy_status;

	
	fep->mii_phy_task_queued = 0;
	printk("%s: config: auto-negotiation ", dev->name);

	if (status & PHY_CONF_ANE)
		printk("on");
	else
		printk("off");

	if (status & PHY_CONF_100FDX)
		printk(", 100FDX");
	if (status & PHY_CONF_100HDX)
		printk(", 100HDX");
	if (status & PHY_CONF_10FDX)
		printk(", 10FDX");
	if (status & PHY_CONF_10HDX)
		printk(", 10HDX");
	if (!(status & PHY_CONF_SPMASK))
		printk(", No speed/duplex selected?");

	if (status & PHY_CONF_LOOP)
		printk(", loopback enabled");

	printk(".\n");

	fep->sequence_done = 1;
}

static void mii_relink(struct work_struct *work)
{
	struct fec_enet_private *fep = container_of(work, struct fec_enet_private, phy_task);
	struct net_device *dev = fep->netdev;
	int duplex;

	
	fep->mii_phy_task_queued = 0;
	fep->link = (fep->phy_status & PHY_STAT_LINK) ? 1 : 0;
	mii_display_status(dev);
	fep->old_link = fep->link;

	if (fep->link) {
		duplex = 0;
		if (fep->phy_status
		    & (PHY_STAT_100FDX | PHY_STAT_10FDX))
			duplex = 1;
		fec_restart(dev, duplex);
	} else
		fec_stop(dev);
}


static void mii_queue_relink(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);

	
	if (fep->mii_phy_task_queued)
		return;

	fep->mii_phy_task_queued = 1;
	INIT_WORK(&fep->phy_task, mii_relink);
	schedule_work(&fep->phy_task);
}


static void mii_queue_config(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);

	if (fep->mii_phy_task_queued)
		return;

	fep->mii_phy_task_queued = 1;
	INIT_WORK(&fep->phy_task, mii_display_config);
	schedule_work(&fep->phy_task);
}

phy_cmd_t const phy_cmd_relink[] = {
	{ mk_mii_read(MII_REG_CR), mii_queue_relink },
	{ mk_mii_end, }
	};
phy_cmd_t const phy_cmd_config[] = {
	{ mk_mii_read(MII_REG_CR), mii_queue_config },
	{ mk_mii_end, }
	};


static void
mii_discover_phy3(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep;
	int i;

	fep = netdev_priv(dev);
	fep->phy_id |= (mii_reg & 0xffff);
	printk("fec: PHY @ 0x%x, ID 0x%08x", fep->phy_addr, fep->phy_id);

	for(i = 0; phy_info[i]; i++) {
		if(phy_info[i]->id == (fep->phy_id >> 4))
			break;
	}

	if (phy_info[i])
		printk(" -- %s\n", phy_info[i]->name);
	else
		printk(" -- unknown PHY!\n");

	fep->phy = phy_info[i];
	fep->phy_id_done = 1;
}


static void
mii_discover_phy(uint mii_reg, struct net_device *dev)
{
	struct fec_enet_private *fep;
	uint phytype;

	fep = netdev_priv(dev);

	if (fep->phy_addr < 32) {
		if ((phytype = (mii_reg & 0xffff)) != 0xffff && phytype != 0) {

			
			fep->phy_id = phytype << 16;
			mii_queue_unlocked(dev, mk_mii_read(MII_REG_PHYIR2),
							mii_discover_phy3);
		} else {
			fep->phy_addr++;
			mii_queue_unlocked(dev, mk_mii_read(MII_REG_PHYIR1),
							mii_discover_phy);
		}
	} else {
		printk("FEC: No PHY device found.\n");
		
		writel(0, fep->hwp + FEC_MII_SPEED);
		fep->phy_speed = 0;
#ifdef HAVE_mii_link_interrupt
		fec_disable_phy_intr(dev);
#endif
	}
}


#ifdef HAVE_mii_link_interrupt
static irqreturn_t
mii_link_interrupt(int irq, void * dev_id)
{
	struct	net_device *dev = dev_id;
	struct fec_enet_private *fep = netdev_priv(dev);

	mii_do_cmd(dev, fep->phy->ack_int);
	mii_do_cmd(dev, phy_cmd_relink);  

	return IRQ_HANDLED;
}
#endif

static void fec_enet_free_buffers(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	int i;
	struct sk_buff *skb;
	struct bufdesc	*bdp;

	bdp = fep->rx_bd_base;
	for (i = 0; i < RX_RING_SIZE; i++) {
		skb = fep->rx_skbuff[i];

		if (bdp->cbd_bufaddr)
			dma_unmap_single(&dev->dev, bdp->cbd_bufaddr,
					FEC_ENET_RX_FRSIZE, DMA_FROM_DEVICE);
		if (skb)
			dev_kfree_skb(skb);
		bdp++;
	}

	bdp = fep->tx_bd_base;
	for (i = 0; i < TX_RING_SIZE; i++)
		kfree(fep->tx_bounce[i]);
}

static int fec_enet_alloc_buffers(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	int i;
	struct sk_buff *skb;
	struct bufdesc	*bdp;

	bdp = fep->rx_bd_base;
	for (i = 0; i < RX_RING_SIZE; i++) {
		skb = dev_alloc_skb(FEC_ENET_RX_FRSIZE);
		if (!skb) {
			fec_enet_free_buffers(dev);
			return -ENOMEM;
		}
		fep->rx_skbuff[i] = skb;

		bdp->cbd_bufaddr = dma_map_single(&dev->dev, skb->data,
				FEC_ENET_RX_FRSIZE, DMA_FROM_DEVICE);
		bdp->cbd_sc = BD_ENET_RX_EMPTY;
		bdp++;
	}

	
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	bdp = fep->tx_bd_base;
	for (i = 0; i < TX_RING_SIZE; i++) {
		fep->tx_bounce[i] = kmalloc(FEC_ENET_TX_FRSIZE, GFP_KERNEL);

		bdp->cbd_sc = 0;
		bdp->cbd_bufaddr = 0;
		bdp++;
	}

	
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	return 0;
}

static int
fec_enet_open(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	int ret;

	

	ret = fec_enet_alloc_buffers(dev);
	if (ret)
		return ret;

	fep->sequence_done = 0;
	fep->link = 0;

	fec_restart(dev, 1);

	if (fep->phy) {
		mii_do_cmd(dev, fep->phy->ack_int);
		mii_do_cmd(dev, fep->phy->config);
		mii_do_cmd(dev, phy_cmd_config);  

		
		while(!fep->sequence_done)
			schedule();

		mii_do_cmd(dev, fep->phy->startup);
	}

	
	fep->link = 1;

	netif_start_queue(dev);
	fep->opened = 1;
	return 0;
}

static int
fec_enet_close(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);

	
	fep->opened = 0;
	netif_stop_queue(dev);
	fec_stop(dev);

        fec_enet_free_buffers(dev);

	return 0;
}



#define HASH_BITS	6		
#define CRC32_POLY	0xEDB88320

static void set_multicast_list(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	struct dev_mc_list *dmi;
	unsigned int i, j, bit, data, crc, tmp;
	unsigned char hash;

	if (dev->flags & IFF_PROMISC) {
		tmp = readl(fep->hwp + FEC_R_CNTRL);
		tmp |= 0x8;
		writel(tmp, fep->hwp + FEC_R_CNTRL);
		return;
	}

	tmp = readl(fep->hwp + FEC_R_CNTRL);
	tmp &= ~0x8;
	writel(tmp, fep->hwp + FEC_R_CNTRL);

	if (dev->flags & IFF_ALLMULTI) {
		
		writel(0xffffffff, fep->hwp + FEC_GRP_HASH_TABLE_HIGH);
		writel(0xffffffff, fep->hwp + FEC_GRP_HASH_TABLE_LOW);

		return;
	}

	
	writel(0, fep->hwp + FEC_GRP_HASH_TABLE_HIGH);
	writel(0, fep->hwp + FEC_GRP_HASH_TABLE_LOW);

	dmi = dev->mc_list;

	for (j = 0; j < dev->mc_count; j++, dmi = dmi->next) {
		
		if (!(dmi->dmi_addr[0] & 1))
			continue;

		
		crc = 0xffffffff;

		for (i = 0; i < dmi->dmi_addrlen; i++) {
			data = dmi->dmi_addr[i];
			for (bit = 0; bit < 8; bit++, data >>= 1) {
				crc = (crc >> 1) ^
				(((crc ^ data) & 1) ? CRC32_POLY : 0);
			}
		}

		
		hash = (crc >> (32 - HASH_BITS)) & 0x3f;

		if (hash > 31) {
			tmp = readl(fep->hwp + FEC_GRP_HASH_TABLE_HIGH);
			tmp |= 1 << (hash - 32);
			writel(tmp, fep->hwp + FEC_GRP_HASH_TABLE_HIGH);
		} else {
			tmp = readl(fep->hwp + FEC_GRP_HASH_TABLE_LOW);
			tmp |= 1 << hash;
			writel(tmp, fep->hwp + FEC_GRP_HASH_TABLE_LOW);
		}
	}
}


static int
fec_set_mac_address(struct net_device *dev, void *p)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	struct sockaddr *addr = p;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

	writel(dev->dev_addr[3] | (dev->dev_addr[2] << 8) |
		(dev->dev_addr[1] << 16) | (dev->dev_addr[0] << 24),
		fep->hwp + FEC_ADDR_LOW);
	writel((dev->dev_addr[5] << 16) | (dev->dev_addr[4] << 24),
		fep + FEC_ADDR_HIGH);
	return 0;
}

static const struct net_device_ops fec_netdev_ops = {
	.ndo_open		= fec_enet_open,
	.ndo_stop		= fec_enet_close,
	.ndo_start_xmit		= fec_enet_start_xmit,
	.ndo_set_multicast_list = set_multicast_list,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_tx_timeout		= fec_timeout,
	.ndo_set_mac_address	= fec_set_mac_address,
};

 
static int fec_enet_init(struct net_device *dev, int index)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	struct bufdesc *cbd_base;
	int i;

	
	cbd_base = dma_alloc_coherent(NULL, PAGE_SIZE, &fep->bd_dma,
			GFP_KERNEL);
	if (!cbd_base) {
		printk("FEC: allocate descriptor memory failed?\n");
		return -ENOMEM;
	}

	spin_lock_init(&fep->hw_lock);
	spin_lock_init(&fep->mii_lock);

	fep->index = index;
	fep->hwp = (void __iomem *)dev->base_addr;
	fep->netdev = dev;

	
#ifdef CONFIG_M5272
	fec_get_mac(dev);
#else
	{
		unsigned long l;
		l = readl(fep->hwp + FEC_ADDR_LOW);
		dev->dev_addr[0] = (unsigned char)((l & 0xFF000000) >> 24);
		dev->dev_addr[1] = (unsigned char)((l & 0x00FF0000) >> 16);
		dev->dev_addr[2] = (unsigned char)((l & 0x0000FF00) >> 8);
		dev->dev_addr[3] = (unsigned char)((l & 0x000000FF) >> 0);
		l = readl(fep->hwp + FEC_ADDR_HIGH);
		dev->dev_addr[4] = (unsigned char)((l & 0xFF000000) >> 24);
		dev->dev_addr[5] = (unsigned char)((l & 0x00FF0000) >> 16);
	}
#endif

	
	fep->rx_bd_base = cbd_base;
	fep->tx_bd_base = cbd_base + RX_RING_SIZE;

#ifdef HAVE_mii_link_interrupt
	fec_request_mii_intr(dev);
#endif
	
	dev->watchdog_timeo = TX_TIMEOUT;
	dev->netdev_ops = &fec_netdev_ops;

	for (i=0; i<NMII-1; i++)
		mii_cmds[i].mii_next = &mii_cmds[i+1];
	mii_free = mii_cmds;

	
	fep->phy_speed = ((((clk_get_rate(fep->clk) / 2 + 4999999)
					/ 2500000) / 2) & 0x3F) << 1;
	fec_restart(dev, 0);

	
	fep->phy_id_done = 0;
	fep->phy_addr = 0;
	mii_queue(dev, mk_mii_read(MII_REG_PHYIR1), mii_discover_phy);

	return 0;
}


static void
fec_restart(struct net_device *dev, int duplex)
{
	struct fec_enet_private *fep = netdev_priv(dev);
	struct bufdesc *bdp;
	int i;

	
	writel(1, fep->hwp + FEC_ECNTRL);
	udelay(10);

	
	writel(0xffc00000, fep->hwp + FEC_IEVENT);

	
	writel(0, fep->hwp + FEC_GRP_HASH_TABLE_HIGH);
	writel(0, fep->hwp + FEC_GRP_HASH_TABLE_LOW);
#ifndef CONFIG_M5272
	writel(0, fep->hwp + FEC_HASH_TABLE_HIGH);
	writel(0, fep->hwp + FEC_HASH_TABLE_LOW);
#endif

	
	writel(PKT_MAXBLR_SIZE, fep->hwp + FEC_R_BUFF_SIZE);

	
	writel(fep->bd_dma, fep->hwp + FEC_R_DES_START);
	writel((unsigned long)fep->bd_dma + sizeof(struct bufdesc) * RX_RING_SIZE,
			fep->hwp + FEC_X_DES_START);

	fep->dirty_tx = fep->cur_tx = fep->tx_bd_base;
	fep->cur_rx = fep->rx_bd_base;

	
	fep->skb_cur = fep->skb_dirty = 0;
	for (i = 0; i <= TX_RING_MOD_MASK; i++) {
		if (fep->tx_skbuff[i]) {
			dev_kfree_skb_any(fep->tx_skbuff[i]);
			fep->tx_skbuff[i] = NULL;
		}
	}

	
	bdp = fep->rx_bd_base;
	for (i = 0; i < RX_RING_SIZE; i++) {

		
		bdp->cbd_sc = BD_ENET_RX_EMPTY;
		bdp++;
	}

	
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	
	bdp = fep->tx_bd_base;
	for (i = 0; i < TX_RING_SIZE; i++) {

		
		bdp->cbd_sc = 0;
		bdp->cbd_bufaddr = 0;
		bdp++;
	}

	
	bdp--;
	bdp->cbd_sc |= BD_SC_WRAP;

	
	if (duplex) {
		
		writel(OPT_FRAME_SIZE | 0x04, fep->hwp + FEC_R_CNTRL);
		writel(0x04, fep->hwp + FEC_X_CNTRL);
	} else {
		
		writel(OPT_FRAME_SIZE | 0x06, fep->hwp + FEC_R_CNTRL);
		writel(0x0, fep->hwp + FEC_X_CNTRL);
	}
	fep->full_duplex = duplex;

	
	writel(fep->phy_speed, fep->hwp + FEC_MII_SPEED);

	
	writel(2, fep->hwp + FEC_ECNTRL);
	writel(0, fep->hwp + FEC_R_DES_ACTIVE);

	
	writel(FEC_ENET_TXF | FEC_ENET_RXF | FEC_ENET_MII,
			fep->hwp + FEC_IMASK);
}

static void
fec_stop(struct net_device *dev)
{
	struct fec_enet_private *fep = netdev_priv(dev);

	
	if (fep->link) {
		writel(1, fep->hwp + FEC_X_CNTRL); 
		udelay(10);
		if (!(readl(fep->hwp + FEC_IEVENT) & FEC_ENET_GRA))
			printk("fec_stop : Graceful transmit stop did not complete !\n");
	}

	
	writel(1, fep->hwp + FEC_ECNTRL);
	udelay(10);

	
	writel(FEC_ENET_MII, fep->hwp + FEC_IEVENT);

	writel(FEC_ENET_MII, fep->hwp + FEC_IMASK);
	writel(fep->phy_speed, fep->hwp + FEC_MII_SPEED);
}

static int __devinit
fec_probe(struct platform_device *pdev)
{
	struct fec_enet_private *fep;
	struct net_device *ndev;
	int i, irq, ret = 0;
	struct resource *r;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r)
		return -ENXIO;

	r = request_mem_region(r->start, resource_size(r), pdev->name);
	if (!r)
		return -EBUSY;

	
	ndev = alloc_etherdev(sizeof(struct fec_enet_private));
	if (!ndev)
		return -ENOMEM;

	SET_NETDEV_DEV(ndev, &pdev->dev);

	
	fep = netdev_priv(ndev);
	memset(fep, 0, sizeof(*fep));

	ndev->base_addr = (unsigned long)ioremap(r->start, resource_size(r));

	if (!ndev->base_addr) {
		ret = -ENOMEM;
		goto failed_ioremap;
	}

	platform_set_drvdata(pdev, ndev);

	
	for (i = 0; i < 3; i++) {
		irq = platform_get_irq(pdev, i);
		if (i && irq < 0)
			break;
		ret = request_irq(irq, fec_enet_interrupt, IRQF_DISABLED, pdev->name, ndev);
		if (ret) {
			while (i >= 0) {
				irq = platform_get_irq(pdev, i);
				free_irq(irq, ndev);
				i--;
			}
			goto failed_irq;
		}
	}

	fep->clk = clk_get(&pdev->dev, "fec_clk");
	if (IS_ERR(fep->clk)) {
		ret = PTR_ERR(fep->clk);
		goto failed_clk;
	}
	clk_enable(fep->clk);

	ret = fec_enet_init(ndev, 0);
	if (ret)
		goto failed_init;

	ret = register_netdev(ndev);
	if (ret)
		goto failed_register;

	return 0;

failed_register:
failed_init:
	clk_disable(fep->clk);
	clk_put(fep->clk);
failed_clk:
	for (i = 0; i < 3; i++) {
		irq = platform_get_irq(pdev, i);
		if (irq > 0)
			free_irq(irq, ndev);
	}
failed_irq:
	iounmap((void __iomem *)ndev->base_addr);
failed_ioremap:
	free_netdev(ndev);

	return ret;
}

static int __devexit
fec_drv_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct fec_enet_private *fep = netdev_priv(ndev);

	platform_set_drvdata(pdev, NULL);

	fec_stop(ndev);
	clk_disable(fep->clk);
	clk_put(fep->clk);
	iounmap((void __iomem *)ndev->base_addr);
	unregister_netdev(ndev);
	free_netdev(ndev);
	return 0;
}

static int
fec_suspend(struct platform_device *dev, pm_message_t state)
{
	struct net_device *ndev = platform_get_drvdata(dev);
	struct fec_enet_private *fep;

	if (ndev) {
		fep = netdev_priv(ndev);
		if (netif_running(ndev)) {
			netif_device_detach(ndev);
			fec_stop(ndev);
		}
	}
	return 0;
}

static int
fec_resume(struct platform_device *dev)
{
	struct net_device *ndev = platform_get_drvdata(dev);

	if (ndev) {
		if (netif_running(ndev)) {
			fec_enet_init(ndev, 0);
			netif_device_attach(ndev);
		}
	}
	return 0;
}

static struct platform_driver fec_driver = {
	.driver	= {
		.name    = "fec",
		.owner	 = THIS_MODULE,
	},
	.probe   = fec_probe,
	.remove  = __devexit_p(fec_drv_remove),
	.suspend = fec_suspend,
	.resume  = fec_resume,
};

static int __init
fec_enet_module_init(void)
{
	printk(KERN_INFO "FEC Ethernet Driver\n");

	return platform_driver_register(&fec_driver);
}

static void __exit
fec_enet_cleanup(void)
{
	platform_driver_unregister(&fec_driver);
}

module_exit(fec_enet_cleanup);
module_init(fec_enet_module_init);

MODULE_LICENSE("GPL");
