

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/if_ether.h>
#include <linux/crc32.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/if_vlan.h>
#include <linux/dma-mapping.h>
#include <linux/stm/soc.h>
#include "stmmac.h"

#define STMMAC_RESOURCE_NAME	"stmmaceth"
#define PHY_RESOURCE_NAME	"stmmacphy"

#undef STMMAC_DEBUG

#ifdef STMMAC_DEBUG
#define DBG(nlevel, klevel, fmt, args...) \
		((void)(netif_msg_##nlevel(priv) && \
		printk(KERN_##klevel fmt, ## args)))
#else
#define DBG(nlevel, klevel, fmt, args...) do { } while (0)
#endif

#undef STMMAC_RX_DEBUG

#ifdef STMMAC_RX_DEBUG
#define RX_DBG(fmt, args...)  printk(fmt, ## args)
#else
#define RX_DBG(fmt, args...)  do { } while (0)
#endif

#undef STMMAC_XMIT_DEBUG

#ifdef STMMAC_TX_DEBUG
#define TX_DBG(fmt, args...)  printk(fmt, ## args)
#else
#define TX_DBG(fmt, args...)  do { } while (0)
#endif

#define STMMAC_ALIGN(x)	L1_CACHE_ALIGN(x)
#define JUMBO_LEN	9000


#define TX_TIMEO 5000 
static int watchdog = TX_TIMEO;
module_param(watchdog, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(watchdog, "Transmit timeout in milliseconds");

static int debug = -1;		
module_param(debug, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Message Level (0: no output, 16: all)");

static int phyaddr = -1;
module_param(phyaddr, int, S_IRUGO);
MODULE_PARM_DESC(phyaddr, "Physical device address");

#define DMA_TX_SIZE 256
static int dma_txsize = DMA_TX_SIZE;
module_param(dma_txsize, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dma_txsize, "Number of descriptors in the TX list");

#define DMA_RX_SIZE 256
static int dma_rxsize = DMA_RX_SIZE;
module_param(dma_rxsize, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(dma_rxsize, "Number of descriptors in the RX list");

static int flow_ctrl = FLOW_OFF;
module_param(flow_ctrl, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(flow_ctrl, "Flow control ability [on/off]");

static int pause = PAUSE_TIME;
module_param(pause, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(pause, "Flow Control Pause Time");

#define TC_DEFAULT 64
static int tc = TC_DEFAULT;
module_param(tc, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(tc, "DMA threshold control value");

#define RX_NO_COALESCE	1	
#define TX_NO_COALESCE	-1	


#ifdef CONFIG_STMMAC_TIMER
#define DEFAULT_PERIODIC_RATE	256
static int tmrate = DEFAULT_PERIODIC_RATE;
module_param(tmrate, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(tmrate, "External timer freq. (default: 256Hz)");
#endif

#define DMA_BUFFER_SIZE	BUF_SIZE_2KiB
static int buf_sz = DMA_BUFFER_SIZE;
module_param(buf_sz, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(buf_sz, "DMA buffer size");


static int tx_coe = HW_CSUM;
module_param(tx_coe, int, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(tx_coe, "GMAC COE type 2 [on/off]");

static const u32 default_msg_level = (NETIF_MSG_DRV | NETIF_MSG_PROBE |
				      NETIF_MSG_LINK | NETIF_MSG_IFUP |
				      NETIF_MSG_IFDOWN | NETIF_MSG_TIMER);

static irqreturn_t stmmac_interrupt(int irq, void *dev_id);
static netdev_tx_t stmmac_xmit(struct sk_buff *skb, struct net_device *dev);


static void stmmac_verify_args(void)
{
	if (unlikely(watchdog < 0))
		watchdog = TX_TIMEO;
	if (unlikely(dma_rxsize < 0))
		dma_rxsize = DMA_RX_SIZE;
	if (unlikely(dma_txsize < 0))
		dma_txsize = DMA_TX_SIZE;
	if (unlikely((buf_sz < DMA_BUFFER_SIZE) || (buf_sz > BUF_SIZE_16KiB)))
		buf_sz = DMA_BUFFER_SIZE;
	if (unlikely(flow_ctrl > 1))
		flow_ctrl = FLOW_AUTO;
	else if (likely(flow_ctrl < 0))
		flow_ctrl = FLOW_OFF;
	if (unlikely((pause < 0) || (pause > 0xffff)))
		pause = PAUSE_TIME;

	return;
}

#if defined(STMMAC_XMIT_DEBUG) || defined(STMMAC_RX_DEBUG)
static void print_pkt(unsigned char *buf, int len)
{
	int j;
	pr_info("len = %d byte, buf addr: 0x%p", len, buf);
	for (j = 0; j < len; j++) {
		if ((j % 16) == 0)
			pr_info("\n %03x:", j);
		pr_info(" %02x", buf[j]);
	}
	pr_info("\n");
	return;
}
#endif


#define STMMAC_TX_THRESH(x)	(x->dma_tx_size/4)

static inline u32 stmmac_tx_avail(struct stmmac_priv *priv)
{
	return priv->dirty_tx + priv->dma_tx_size - priv->cur_tx - 1;
}


static void stmmac_adjust_link(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	struct phy_device *phydev = priv->phydev;
	unsigned long ioaddr = dev->base_addr;
	unsigned long flags;
	int new_state = 0;
	unsigned int fc = priv->flow_ctrl, pause_time = priv->pause;

	if (phydev == NULL)
		return;

	DBG(probe, DEBUG, "stmmac_adjust_link: called.  address %d link %d\n",
	    phydev->addr, phydev->link);

	spin_lock_irqsave(&priv->lock, flags);
	if (phydev->link) {
		u32 ctrl = readl(ioaddr + MAC_CTRL_REG);

		
		if (phydev->duplex != priv->oldduplex) {
			new_state = 1;
			if (!(phydev->duplex))
				ctrl &= ~priv->mac_type->hw.link.duplex;
			else
				ctrl |= priv->mac_type->hw.link.duplex;
			priv->oldduplex = phydev->duplex;
		}
		
		if (phydev->pause)
			priv->mac_type->ops->flow_ctrl(ioaddr, phydev->duplex,
						       fc, pause_time);

		if (phydev->speed != priv->speed) {
			new_state = 1;
			switch (phydev->speed) {
			case 1000:
				if (likely(priv->is_gmac))
					ctrl &= ~priv->mac_type->hw.link.port;
				break;
			case 100:
			case 10:
				if (priv->is_gmac) {
					ctrl |= priv->mac_type->hw.link.port;
					if (phydev->speed == SPEED_100) {
						ctrl |=
						    priv->mac_type->hw.link.
						    speed;
					} else {
						ctrl &=
						    ~(priv->mac_type->hw.
						      link.speed);
					}
				} else {
					ctrl &= ~priv->mac_type->hw.link.port;
				}
				priv->fix_mac_speed(priv->bsp_priv,
						    phydev->speed);
				break;
			default:
				if (netif_msg_link(priv))
					pr_warning("%s: Speed (%d) is not 10"
				       " or 100!\n", dev->name, phydev->speed);
				break;
			}

			priv->speed = phydev->speed;
		}

		writel(ctrl, ioaddr + MAC_CTRL_REG);

		if (!priv->oldlink) {
			new_state = 1;
			priv->oldlink = 1;
		}
	} else if (priv->oldlink) {
		new_state = 1;
		priv->oldlink = 0;
		priv->speed = 0;
		priv->oldduplex = -1;
	}

	if (new_state && netif_msg_link(priv))
		phy_print_status(phydev);

	spin_unlock_irqrestore(&priv->lock, flags);

	DBG(probe, DEBUG, "stmmac_adjust_link: exiting\n");
}


static int stmmac_init_phy(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	struct phy_device *phydev;
	char phy_id[BUS_ID_SIZE];	
	char bus_id[BUS_ID_SIZE];

	priv->oldlink = 0;
	priv->speed = 0;
	priv->oldduplex = -1;

	if (priv->phy_addr == -1) {
		
		return 0;
	}

	snprintf(bus_id, MII_BUS_ID_SIZE, "%x", priv->bus_id);
	snprintf(phy_id, BUS_ID_SIZE, PHY_ID_FMT, bus_id, priv->phy_addr);
	pr_debug("stmmac_init_phy:  trying to attach to %s\n", phy_id);

	phydev = phy_connect(dev, phy_id, &stmmac_adjust_link, 0,
			priv->phy_interface);

	if (IS_ERR(phydev)) {
		pr_err("%s: Could not attach to PHY\n", dev->name);
		return PTR_ERR(phydev);
	}

	
	if (phydev->phy_id == 0) {
		phy_disconnect(phydev);
		return -ENODEV;
	}
	pr_debug("stmmac_init_phy:  %s: attached to PHY (UID 0x%x)"
	       " Link = %d\n", dev->name, phydev->phy_id, phydev->link);

	priv->phydev = phydev;

	return 0;
}

static inline void stmmac_mac_enable_rx(unsigned long ioaddr)
{
	u32 value = readl(ioaddr + MAC_CTRL_REG);
	value |= MAC_RNABLE_RX;
	
	writel(value, ioaddr + MAC_CTRL_REG);
}

static inline void stmmac_mac_enable_tx(unsigned long ioaddr)
{
	u32 value = readl(ioaddr + MAC_CTRL_REG);
	value |= MAC_ENABLE_TX;
	
	writel(value, ioaddr + MAC_CTRL_REG);
}

static inline void stmmac_mac_disable_rx(unsigned long ioaddr)
{
	u32 value = readl(ioaddr + MAC_CTRL_REG);
	value &= ~MAC_RNABLE_RX;
	writel(value, ioaddr + MAC_CTRL_REG);
}

static inline void stmmac_mac_disable_tx(unsigned long ioaddr)
{
	u32 value = readl(ioaddr + MAC_CTRL_REG);
	value &= ~MAC_ENABLE_TX;
	writel(value, ioaddr + MAC_CTRL_REG);
}


static void display_ring(struct dma_desc *p, int size)
{
	struct tmp_s {
		u64 a;
		unsigned int b;
		unsigned int c;
	};
	int i;
	for (i = 0; i < size; i++) {
		struct tmp_s *x = (struct tmp_s *)(p + i);
		pr_info("\t%d [0x%x]: DES0=0x%x DES1=0x%x BUF1=0x%x BUF2=0x%x",
		       i, (unsigned int)virt_to_phys(&p[i]),
		       (unsigned int)(x->a), (unsigned int)((x->a) >> 32),
		       x->b, x->c);
		pr_info("\n");
	}
}


static void init_dma_desc_rings(struct net_device *dev)
{
	int i;
	struct stmmac_priv *priv = netdev_priv(dev);
	struct sk_buff *skb;
	unsigned int txsize = priv->dma_tx_size;
	unsigned int rxsize = priv->dma_rx_size;
	unsigned int bfsize = priv->dma_buf_sz;
	int buff2_needed = 0, dis_ic = 0;

	
	if (unlikely(dev->mtu >= BUF_SIZE_8KiB))
		bfsize = BUF_SIZE_16KiB;
	else if (unlikely(dev->mtu >= BUF_SIZE_4KiB))
		bfsize = BUF_SIZE_8KiB;
	else if (unlikely(dev->mtu >= BUF_SIZE_2KiB))
		bfsize = BUF_SIZE_4KiB;
	else if (unlikely(dev->mtu >= DMA_BUFFER_SIZE))
		bfsize = BUF_SIZE_2KiB;
	else
		bfsize = DMA_BUFFER_SIZE;

#ifdef CONFIG_STMMAC_TIMER
	
	if (likely(priv->tm->enable))
		dis_ic = 1;
#endif
	
	if (bfsize >= BUF_SIZE_8KiB)
		buff2_needed = 1;

	DBG(probe, INFO, "stmmac: txsize %d, rxsize %d, bfsize %d\n",
	    txsize, rxsize, bfsize);

	priv->rx_skbuff_dma = kmalloc(rxsize * sizeof(dma_addr_t), GFP_KERNEL);
	priv->rx_skbuff =
	    kmalloc(sizeof(struct sk_buff *) * rxsize, GFP_KERNEL);
	priv->dma_rx =
	    (struct dma_desc *)dma_alloc_coherent(priv->device,
						  rxsize *
						  sizeof(struct dma_desc),
						  &priv->dma_rx_phy,
						  GFP_KERNEL);
	priv->tx_skbuff = kmalloc(sizeof(struct sk_buff *) * txsize,
				       GFP_KERNEL);
	priv->dma_tx =
	    (struct dma_desc *)dma_alloc_coherent(priv->device,
						  txsize *
						  sizeof(struct dma_desc),
						  &priv->dma_tx_phy,
						  GFP_KERNEL);

	if ((priv->dma_rx == NULL) || (priv->dma_tx == NULL)) {
		pr_err("%s:ERROR allocating the DMA Tx/Rx desc\n", __func__);
		return;
	}

	DBG(probe, INFO, "stmmac (%s) DMA desc rings: virt addr (Rx %p, "
	    "Tx %p)\n\tDMA phy addr (Rx 0x%08x, Tx 0x%08x)\n",
	    dev->name, priv->dma_rx, priv->dma_tx,
	    (unsigned int)priv->dma_rx_phy, (unsigned int)priv->dma_tx_phy);

	
	DBG(probe, INFO, "stmmac: SKB addresses:\n"
			 "skb\t\tskb data\tdma data\n");

	for (i = 0; i < rxsize; i++) {
		struct dma_desc *p = priv->dma_rx + i;

		skb = netdev_alloc_skb_ip_align(dev, bfsize);
		if (unlikely(skb == NULL)) {
			pr_err("%s: Rx init fails; skb is NULL\n", __func__);
			break;
		}
		priv->rx_skbuff[i] = skb;
		priv->rx_skbuff_dma[i] = dma_map_single(priv->device, skb->data,
						bfsize, DMA_FROM_DEVICE);

		p->des2 = priv->rx_skbuff_dma[i];
		if (unlikely(buff2_needed))
			p->des3 = p->des2 + BUF_SIZE_8KiB;
		DBG(probe, INFO, "[%p]\t[%p]\t[%x]\n", priv->rx_skbuff[i],
			priv->rx_skbuff[i]->data, priv->rx_skbuff_dma[i]);
	}
	priv->cur_rx = 0;
	priv->dirty_rx = (unsigned int)(i - rxsize);
	priv->dma_buf_sz = bfsize;
	buf_sz = bfsize;

	
	for (i = 0; i < txsize; i++) {
		priv->tx_skbuff[i] = NULL;
		priv->dma_tx[i].des2 = 0;
	}
	priv->dirty_tx = 0;
	priv->cur_tx = 0;

	
	priv->mac_type->ops->init_rx_desc(priv->dma_rx, rxsize, dis_ic);
	priv->mac_type->ops->init_tx_desc(priv->dma_tx, txsize);

	if (netif_msg_hw(priv)) {
		pr_info("RX descriptor ring:\n");
		display_ring(priv->dma_rx, rxsize);
		pr_info("TX descriptor ring:\n");
		display_ring(priv->dma_tx, txsize);
	}
	return;
}

static void dma_free_rx_skbufs(struct stmmac_priv *priv)
{
	int i;

	for (i = 0; i < priv->dma_rx_size; i++) {
		if (priv->rx_skbuff[i]) {
			dma_unmap_single(priv->device, priv->rx_skbuff_dma[i],
					 priv->dma_buf_sz, DMA_FROM_DEVICE);
			dev_kfree_skb_any(priv->rx_skbuff[i]);
		}
		priv->rx_skbuff[i] = NULL;
	}
	return;
}

static void dma_free_tx_skbufs(struct stmmac_priv *priv)
{
	int i;

	for (i = 0; i < priv->dma_tx_size; i++) {
		if (priv->tx_skbuff[i] != NULL) {
			struct dma_desc *p = priv->dma_tx + i;
			if (p->des2)
				dma_unmap_single(priv->device, p->des2,
				 priv->mac_type->ops->get_tx_len(p),
				 DMA_TO_DEVICE);
			dev_kfree_skb_any(priv->tx_skbuff[i]);
			priv->tx_skbuff[i] = NULL;
		}
	}
	return;
}

static void free_dma_desc_resources(struct stmmac_priv *priv)
{
	
	dma_free_rx_skbufs(priv);
	dma_free_tx_skbufs(priv);

	
	dma_free_coherent(priv->device,
			  priv->dma_tx_size * sizeof(struct dma_desc),
			  priv->dma_tx, priv->dma_tx_phy);
	dma_free_coherent(priv->device,
			  priv->dma_rx_size * sizeof(struct dma_desc),
			  priv->dma_rx, priv->dma_rx_phy);
	kfree(priv->rx_skbuff_dma);
	kfree(priv->rx_skbuff);
	kfree(priv->tx_skbuff);

	return;
}


static void stmmac_dma_start_tx(unsigned long ioaddr)
{
	u32 value = readl(ioaddr + DMA_CONTROL);
	value |= DMA_CONTROL_ST;
	writel(value, ioaddr + DMA_CONTROL);
	return;
}

static void stmmac_dma_stop_tx(unsigned long ioaddr)
{
	u32 value = readl(ioaddr + DMA_CONTROL);
	value &= ~DMA_CONTROL_ST;
	writel(value, ioaddr + DMA_CONTROL);
	return;
}


static void stmmac_dma_start_rx(unsigned long ioaddr)
{
	u32 value = readl(ioaddr + DMA_CONTROL);
	value |= DMA_CONTROL_SR;
	writel(value, ioaddr + DMA_CONTROL);

	return;
}

static void stmmac_dma_stop_rx(unsigned long ioaddr)
{
	u32 value = readl(ioaddr + DMA_CONTROL);
	value &= ~DMA_CONTROL_SR;
	writel(value, ioaddr + DMA_CONTROL);

	return;
}


static void stmmac_dma_operation_mode(struct stmmac_priv *priv)
{
	if (!priv->is_gmac) {
		
		priv->mac_type->ops->dma_mode(priv->dev->base_addr, tc, 0);
		priv->tx_coe = NO_HW_CSUM;
	} else {
		if ((priv->dev->mtu <= ETH_DATA_LEN) && (tx_coe)) {
			priv->mac_type->ops->dma_mode(priv->dev->base_addr,
						      SF_DMA_MODE, SF_DMA_MODE);
			tc = SF_DMA_MODE;
			priv->tx_coe = HW_CSUM;
		} else {
			
			priv->mac_type->ops->dma_mode(priv->dev->base_addr, tc,
						      SF_DMA_MODE);
			priv->tx_coe = NO_HW_CSUM;
		}
	}
	tx_coe = priv->tx_coe;

	return;
}

#ifdef STMMAC_DEBUG

static void show_tx_process_state(unsigned int status)
{
	unsigned int state;
	state = (status & DMA_STATUS_TS_MASK) >> DMA_STATUS_TS_SHIFT;

	switch (state) {
	case 0:
		pr_info("- TX (Stopped): Reset or Stop command\n");
		break;
	case 1:
		pr_info("- TX (Running):Fetching the Tx desc\n");
		break;
	case 2:
		pr_info("- TX (Running): Waiting for end of tx\n");
		break;
	case 3:
		pr_info("- TX (Running): Reading the data "
		       "and queuing the data into the Tx buf\n");
		break;
	case 6:
		pr_info("- TX (Suspended): Tx Buff Underflow "
		       "or an unavailable Transmit descriptor\n");
		break;
	case 7:
		pr_info("- TX (Running): Closing Tx descriptor\n");
		break;
	default:
		break;
	}
	return;
}


static void show_rx_process_state(unsigned int status)
{
	unsigned int state;
	state = (status & DMA_STATUS_RS_MASK) >> DMA_STATUS_RS_SHIFT;

	switch (state) {
	case 0:
		pr_info("- RX (Stopped): Reset or Stop command\n");
		break;
	case 1:
		pr_info("- RX (Running): Fetching the Rx desc\n");
		break;
	case 2:
		pr_info("- RX (Running):Checking for end of pkt\n");
		break;
	case 3:
		pr_info("- RX (Running): Waiting for Rx pkt\n");
		break;
	case 4:
		pr_info("- RX (Suspended): Unavailable Rx buf\n");
		break;
	case 5:
		pr_info("- RX (Running): Closing Rx descriptor\n");
		break;
	case 6:
		pr_info("- RX(Running): Flushing the current frame"
		       " from the Rx buf\n");
		break;
	case 7:
		pr_info("- RX (Running): Queuing the Rx frame"
		       " from the Rx buf into memory\n");
		break;
	default:
		break;
	}
	return;
}
#endif


static void stmmac_tx(struct stmmac_priv *priv)
{
	unsigned int txsize = priv->dma_tx_size;
	unsigned long ioaddr = priv->dev->base_addr;

	while (priv->dirty_tx != priv->cur_tx) {
		int last;
		unsigned int entry = priv->dirty_tx % txsize;
		struct sk_buff *skb = priv->tx_skbuff[entry];
		struct dma_desc *p = priv->dma_tx + entry;

		
		if (priv->mac_type->ops->get_tx_owner(p))
			break;

		
		last = priv->mac_type->ops->get_tx_ls(p);
		if (likely(last)) {
			int tx_error =
			    priv->mac_type->ops->tx_status(&priv->dev->stats,
							   &priv->xstats,
							   p, ioaddr);
			if (likely(tx_error == 0)) {
				priv->dev->stats.tx_packets++;
				priv->xstats.tx_pkt_n++;
			} else
				priv->dev->stats.tx_errors++;
		}
		TX_DBG("%s: curr %d, dirty %d\n", __func__,
			priv->cur_tx, priv->dirty_tx);

		if (likely(p->des2))
			dma_unmap_single(priv->device, p->des2,
					 priv->mac_type->ops->get_tx_len(p),
					 DMA_TO_DEVICE);
		if (unlikely(p->des3))
			p->des3 = 0;

		if (likely(skb != NULL)) {
			
			if ((skb_queue_len(&priv->rx_recycle) <
				priv->dma_rx_size) &&
				skb_recycle_check(skb, priv->dma_buf_sz))
				__skb_queue_head(&priv->rx_recycle, skb);
			else
				dev_kfree_skb(skb);

			priv->tx_skbuff[entry] = NULL;
		}

		priv->mac_type->ops->release_tx_desc(p);

		entry = (++priv->dirty_tx) % txsize;
	}
	if (unlikely(netif_queue_stopped(priv->dev) &&
		     stmmac_tx_avail(priv) > STMMAC_TX_THRESH(priv))) {
		netif_tx_lock(priv->dev);
		if (netif_queue_stopped(priv->dev) &&
		     stmmac_tx_avail(priv) > STMMAC_TX_THRESH(priv)) {
			TX_DBG("%s: restart transmit\n", __func__);
			netif_wake_queue(priv->dev);
		}
		netif_tx_unlock(priv->dev);
	}
	return;
}

static inline void stmmac_enable_irq(struct stmmac_priv *priv)
{
#ifdef CONFIG_STMMAC_TIMER
	if (likely(priv->tm->enable))
		priv->tm->timer_start(tmrate);
	else
#endif
	writel(DMA_INTR_DEFAULT_MASK, priv->dev->base_addr + DMA_INTR_ENA);
}

static inline void stmmac_disable_irq(struct stmmac_priv *priv)
{
#ifdef CONFIG_STMMAC_TIMER
	if (likely(priv->tm->enable))
		priv->tm->timer_stop();
	else
#endif
	writel(0, priv->dev->base_addr + DMA_INTR_ENA);
}

static int stmmac_has_work(struct stmmac_priv *priv)
{
	unsigned int has_work = 0;
	int rxret, tx_work = 0;

	rxret = priv->mac_type->ops->get_rx_owner(priv->dma_rx +
		(priv->cur_rx % priv->dma_rx_size));

	if (priv->dirty_tx != priv->cur_tx)
		tx_work = 1;

	if (likely(!rxret || tx_work))
		has_work = 1;

	return has_work;
}

static inline void _stmmac_schedule(struct stmmac_priv *priv)
{
	if (likely(stmmac_has_work(priv))) {
		stmmac_disable_irq(priv);
		napi_schedule(&priv->napi);
	}
}

#ifdef CONFIG_STMMAC_TIMER
void stmmac_schedule(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);

	priv->xstats.sched_timer_n++;

	_stmmac_schedule(priv);

	return;
}

static void stmmac_no_timer_started(unsigned int x)
{;
};

static void stmmac_no_timer_stopped(void)
{;
};
#endif


static void stmmac_tx_err(struct stmmac_priv *priv)
{
	netif_stop_queue(priv->dev);

	stmmac_dma_stop_tx(priv->dev->base_addr);
	dma_free_tx_skbufs(priv);
	priv->mac_type->ops->init_tx_desc(priv->dma_tx, priv->dma_tx_size);
	priv->dirty_tx = 0;
	priv->cur_tx = 0;
	stmmac_dma_start_tx(priv->dev->base_addr);

	priv->dev->stats.tx_errors++;
	netif_wake_queue(priv->dev);

	return;
}


static void stmmac_dma_interrupt(struct net_device *dev)
{
	unsigned long ioaddr = dev->base_addr;
	struct stmmac_priv *priv = netdev_priv(dev);
	
	u32 intr_status = readl(ioaddr + DMA_STATUS);

	DBG(intr, INFO, "%s: [CSR5: 0x%08x]\n", __func__, intr_status);

#ifdef STMMAC_DEBUG
	
	if (netif_msg_tx_done(priv))
		show_tx_process_state(intr_status);
	if (netif_msg_rx_status(priv))
		show_rx_process_state(intr_status);
#endif
	
	if (unlikely(intr_status & DMA_STATUS_AIS)) {
		DBG(intr, INFO, "CSR5[15] DMA ABNORMAL IRQ: ");
		if (unlikely(intr_status & DMA_STATUS_UNF)) {
			DBG(intr, INFO, "transmit underflow\n");
			if (unlikely(tc != SF_DMA_MODE)
			    && (tc <= 256)) {
				
				tc += 64;
				priv->mac_type->ops->dma_mode(ioaddr, tc,
					      SF_DMA_MODE);
				priv->xstats.threshold = tc;
			}
			stmmac_tx_err(priv);
			priv->xstats.tx_undeflow_irq++;
		}
		if (unlikely(intr_status & DMA_STATUS_TJT)) {
			DBG(intr, INFO, "transmit jabber\n");
			priv->xstats.tx_jabber_irq++;
		}
		if (unlikely(intr_status & DMA_STATUS_OVF)) {
			DBG(intr, INFO, "recv overflow\n");
			priv->xstats.rx_overflow_irq++;
		}
		if (unlikely(intr_status & DMA_STATUS_RU)) {
			DBG(intr, INFO, "receive buffer unavailable\n");
			priv->xstats.rx_buf_unav_irq++;
		}
		if (unlikely(intr_status & DMA_STATUS_RPS)) {
			DBG(intr, INFO, "receive process stopped\n");
			priv->xstats.rx_process_stopped_irq++;
		}
		if (unlikely(intr_status & DMA_STATUS_RWT)) {
			DBG(intr, INFO, "receive watchdog\n");
			priv->xstats.rx_watchdog_irq++;
		}
		if (unlikely(intr_status & DMA_STATUS_ETI)) {
			DBG(intr, INFO, "transmit early interrupt\n");
			priv->xstats.tx_early_irq++;
		}
		if (unlikely(intr_status & DMA_STATUS_TPS)) {
			DBG(intr, INFO, "transmit process stopped\n");
			priv->xstats.tx_process_stopped_irq++;
			stmmac_tx_err(priv);
		}
		if (unlikely(intr_status & DMA_STATUS_FBI)) {
			DBG(intr, INFO, "fatal bus error\n");
			priv->xstats.fatal_bus_error_irq++;
			stmmac_tx_err(priv);
		}
	}

	
	if (intr_status & DMA_STATUS_NIS) {
		priv->xstats.normal_irq_n++;
		if (likely((intr_status & DMA_STATUS_RI) ||
			 (intr_status & (DMA_STATUS_TI))))
				_stmmac_schedule(priv);
	}

	
	if (unlikely(intr_status &
		     (DMA_STATUS_GPI | DMA_STATUS_GMI | DMA_STATUS_GLI)))
		pr_info("%s: unexpected status %08x\n", __func__, intr_status);

	
	writel((intr_status & 0x1ffff), ioaddr + DMA_STATUS);

	DBG(intr, INFO, "\n\n");

	return;
}


static int stmmac_open(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;
	int ret;

	
	if (!is_valid_ether_addr(dev->dev_addr)) {
		random_ether_addr(dev->dev_addr);
		pr_warning("%s: generated random MAC address %pM\n", dev->name,
			dev->dev_addr);
	}

	stmmac_verify_args();

	ret = stmmac_init_phy(dev);
	if (unlikely(ret)) {
		pr_err("%s: Cannot attach to PHY (error: %d)\n", __func__, ret);
		return ret;
	}

	
	ret = request_irq(dev->irq, &stmmac_interrupt,
			  IRQF_SHARED, dev->name, dev);
	if (unlikely(ret < 0)) {
		pr_err("%s: ERROR: allocating the IRQ %d (error: %d)\n",
		       __func__, dev->irq, ret);
		return ret;
	}

#ifdef CONFIG_STMMAC_TIMER
	priv->tm = kzalloc(sizeof(struct stmmac_timer *), GFP_KERNEL);
	if (unlikely(priv->tm == NULL)) {
		pr_err("%s: ERROR: timer memory alloc failed \n", __func__);
		return -ENOMEM;
	}
	priv->tm->freq = tmrate;

	
	if (unlikely((stmmac_open_ext_timer(dev, priv->tm)) < 0)) {
		pr_warning("stmmaceth: cannot attach the external timer.\n");
		tmrate = 0;
		priv->tm->freq = 0;
		priv->tm->timer_start = stmmac_no_timer_started;
		priv->tm->timer_stop = stmmac_no_timer_stopped;
	} else
		priv->tm->enable = 1;
#endif

	
	priv->dma_tx_size = STMMAC_ALIGN(dma_txsize);
	priv->dma_rx_size = STMMAC_ALIGN(dma_rxsize);
	priv->dma_buf_sz = STMMAC_ALIGN(buf_sz);
	init_dma_desc_rings(dev);

	
	if (unlikely(priv->mac_type->ops->dma_init(ioaddr,
		priv->pbl, priv->dma_tx_phy, priv->dma_rx_phy) < 0)) {

		pr_err("%s: DMA initialization failed\n", __func__);
		return -1;
	}

	
	priv->mac_type->ops->set_umac_addr(ioaddr, dev->dev_addr, 0);
	
	priv->mac_type->ops->core_init(ioaddr);

	priv->shutdown = 0;

	
	writel(0xffffffff, ioaddr + MMC_HIGH_INTR_MASK);
	writel(0xffffffff, ioaddr + MMC_LOW_INTR_MASK);

	
	stmmac_mac_enable_rx(ioaddr);
	stmmac_mac_enable_tx(ioaddr);

	
	stmmac_dma_operation_mode(priv);

	
	memset(&priv->xstats, 0, sizeof(struct stmmac_extra_stats));
	priv->xstats.threshold = tc;

	
	DBG(probe, DEBUG, "%s: DMA RX/TX processes started...\n", dev->name);
	stmmac_dma_start_tx(ioaddr);
	stmmac_dma_start_rx(ioaddr);

#ifdef CONFIG_STMMAC_TIMER
	priv->tm->timer_start(tmrate);
#endif
	
	if (netif_msg_hw(priv)) {
		priv->mac_type->ops->dump_mac_regs(ioaddr);
		priv->mac_type->ops->dump_dma_regs(ioaddr);
	}

	if (priv->phydev)
		phy_start(priv->phydev);

	napi_enable(&priv->napi);
	skb_queue_head_init(&priv->rx_recycle);
	netif_start_queue(dev);
	return 0;
}


static int stmmac_release(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);

	
	if (priv->phydev) {
		phy_stop(priv->phydev);
		phy_disconnect(priv->phydev);
		priv->phydev = NULL;
	}

	netif_stop_queue(dev);

#ifdef CONFIG_STMMAC_TIMER
	
	stmmac_close_ext_timer();
	if (priv->tm != NULL)
		kfree(priv->tm);
#endif
	napi_disable(&priv->napi);
	skb_queue_purge(&priv->rx_recycle);

	
	free_irq(dev->irq, dev);

	
	stmmac_dma_stop_tx(dev->base_addr);
	stmmac_dma_stop_rx(dev->base_addr);

	
	free_dma_desc_resources(priv);

	
	stmmac_mac_disable_tx(dev->base_addr);
	stmmac_mac_disable_rx(dev->base_addr);

	netif_carrier_off(dev);

	return 0;
}


static int stmmac_sw_tso(struct stmmac_priv *priv, struct sk_buff *skb)
{
	struct sk_buff *segs, *curr_skb;
	int gso_segs = skb_shinfo(skb)->gso_segs;

	
	if (unlikely(stmmac_tx_avail(priv) < gso_segs)) {
		netif_stop_queue(priv->dev);
		TX_DBG(KERN_ERR "%s: TSO BUG! Tx Ring full when queue awake\n",
		       __func__);
		if (stmmac_tx_avail(priv) < gso_segs)
			return NETDEV_TX_BUSY;

		netif_wake_queue(priv->dev);
	}
	TX_DBG("\tstmmac_sw_tso: segmenting: skb %p (len %d)\n",
	       skb, skb->len);

	segs = skb_gso_segment(skb, priv->dev->features & ~NETIF_F_TSO);
	if (unlikely(IS_ERR(segs)))
		goto sw_tso_end;

	do {
		curr_skb = segs;
		segs = segs->next;
		TX_DBG("\t\tcurrent skb->len: %d, *curr %p,"
		       "*next %p\n", curr_skb->len, curr_skb, segs);
		curr_skb->next = NULL;
		stmmac_xmit(curr_skb, priv->dev);
	} while (segs);

sw_tso_end:
	dev_kfree_skb(skb);

	return NETDEV_TX_OK;
}

static unsigned int stmmac_handle_jumbo_frames(struct sk_buff *skb,
					       struct net_device *dev,
					       int csum_insertion)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned int nopaged_len = skb_headlen(skb);
	unsigned int txsize = priv->dma_tx_size;
	unsigned int entry = priv->cur_tx % txsize;
	struct dma_desc *desc = priv->dma_tx + entry;

	if (nopaged_len > BUF_SIZE_8KiB) {

		int buf2_size = nopaged_len - BUF_SIZE_8KiB;

		desc->des2 = dma_map_single(priv->device, skb->data,
					    BUF_SIZE_8KiB, DMA_TO_DEVICE);
		desc->des3 = desc->des2 + BUF_SIZE_4KiB;
		priv->mac_type->ops->prepare_tx_desc(desc, 1, BUF_SIZE_8KiB,
						     csum_insertion);

		entry = (++priv->cur_tx) % txsize;
		desc = priv->dma_tx + entry;

		desc->des2 = dma_map_single(priv->device,
					skb->data + BUF_SIZE_8KiB,
					buf2_size, DMA_TO_DEVICE);
		desc->des3 = desc->des2 + BUF_SIZE_4KiB;
		priv->mac_type->ops->prepare_tx_desc(desc, 0,
						     buf2_size, csum_insertion);
		priv->mac_type->ops->set_tx_owner(desc);
		priv->tx_skbuff[entry] = NULL;
	} else {
		desc->des2 = dma_map_single(priv->device, skb->data,
					nopaged_len, DMA_TO_DEVICE);
		desc->des3 = desc->des2 + BUF_SIZE_4KiB;
		priv->mac_type->ops->prepare_tx_desc(desc, 1, nopaged_len,
						     csum_insertion);
	}
	return entry;
}


static netdev_tx_t stmmac_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned int txsize = priv->dma_tx_size;
	unsigned int entry;
	int i, csum_insertion = 0;
	int nfrags = skb_shinfo(skb)->nr_frags;
	struct dma_desc *desc, *first;

	if (unlikely(stmmac_tx_avail(priv) < nfrags + 1)) {
		if (!netif_queue_stopped(dev)) {
			netif_stop_queue(dev);
			
			pr_err("%s: BUG! Tx Ring full when queue awake\n",
				__func__);
		}
		return NETDEV_TX_BUSY;
	}

	entry = priv->cur_tx % txsize;

#ifdef STMMAC_XMIT_DEBUG
	if ((skb->len > ETH_FRAME_LEN) || nfrags)
		pr_info("stmmac xmit:\n"
		       "\tskb addr %p - len: %d - nopaged_len: %d\n"
		       "\tn_frags: %d - ip_summed: %d - %s gso\n",
		       skb, skb->len, skb_headlen(skb), nfrags, skb->ip_summed,
		       !skb_is_gso(skb) ? "isn't" : "is");
#endif

	if (unlikely(skb_is_gso(skb)))
		return stmmac_sw_tso(priv, skb);

	if (likely((skb->ip_summed == CHECKSUM_PARTIAL))) {
		if (likely(priv->tx_coe == NO_HW_CSUM))
			skb_checksum_help(skb);
		else
			csum_insertion = 1;
	}

	desc = priv->dma_tx + entry;
	first = desc;

#ifdef STMMAC_XMIT_DEBUG
	if ((nfrags > 0) || (skb->len > ETH_FRAME_LEN))
		pr_debug("stmmac xmit: skb len: %d, nopaged_len: %d,\n"
		       "\t\tn_frags: %d, ip_summed: %d\n",
		       skb->len, skb_headlen(skb), nfrags, skb->ip_summed);
#endif
	priv->tx_skbuff[entry] = skb;
	if (unlikely(skb->len >= BUF_SIZE_4KiB)) {
		entry = stmmac_handle_jumbo_frames(skb, dev, csum_insertion);
		desc = priv->dma_tx + entry;
	} else {
		unsigned int nopaged_len = skb_headlen(skb);
		desc->des2 = dma_map_single(priv->device, skb->data,
					nopaged_len, DMA_TO_DEVICE);
		priv->mac_type->ops->prepare_tx_desc(desc, 1, nopaged_len,
						     csum_insertion);
	}

	for (i = 0; i < nfrags; i++) {
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		int len = frag->size;

		entry = (++priv->cur_tx) % txsize;
		desc = priv->dma_tx + entry;

		TX_DBG("\t[entry %d] segment len: %d\n", entry, len);
		desc->des2 = dma_map_page(priv->device, frag->page,
					  frag->page_offset,
					  len, DMA_TO_DEVICE);
		priv->tx_skbuff[entry] = NULL;
		priv->mac_type->ops->prepare_tx_desc(desc, 0, len,
						     csum_insertion);
		priv->mac_type->ops->set_tx_owner(desc);
	}

	
	priv->mac_type->ops->close_tx_desc(desc);

#ifdef CONFIG_STMMAC_TIMER
	
	if (likely(priv->tm->enable))
		priv->mac_type->ops->clear_tx_ic(desc);
#endif
	
	priv->mac_type->ops->set_tx_owner(first);

	priv->cur_tx++;

#ifdef STMMAC_XMIT_DEBUG
	if (netif_msg_pktdata(priv)) {
		pr_info("stmmac xmit: current=%d, dirty=%d, entry=%d, "
		       "first=%p, nfrags=%d\n",
		       (priv->cur_tx % txsize), (priv->dirty_tx % txsize),
		       entry, first, nfrags);
		display_ring(priv->dma_tx, txsize);
		pr_info(">>> frame to be transmitted: ");
		print_pkt(skb->data, skb->len);
	}
#endif
	if (unlikely(stmmac_tx_avail(priv) <= (MAX_SKB_FRAGS + 1))) {
		TX_DBG("%s: stop transmitted packets\n", __func__);
		netif_stop_queue(dev);
	}

	dev->stats.tx_bytes += skb->len;

	
	writel(1, dev->base_addr + DMA_XMT_POLL_DEMAND);

	return NETDEV_TX_OK;
}

static inline void stmmac_rx_refill(struct stmmac_priv *priv)
{
	unsigned int rxsize = priv->dma_rx_size;
	int bfsize = priv->dma_buf_sz;
	struct dma_desc *p = priv->dma_rx;

	for (; priv->cur_rx - priv->dirty_rx > 0; priv->dirty_rx++) {
		unsigned int entry = priv->dirty_rx % rxsize;
		if (likely(priv->rx_skbuff[entry] == NULL)) {
			struct sk_buff *skb;

			skb = __skb_dequeue(&priv->rx_recycle);
			if (skb == NULL)
				skb = netdev_alloc_skb_ip_align(priv->dev,
								bfsize);

			if (unlikely(skb == NULL))
				break;

			priv->rx_skbuff[entry] = skb;
			priv->rx_skbuff_dma[entry] =
			    dma_map_single(priv->device, skb->data, bfsize,
					   DMA_FROM_DEVICE);

			(p + entry)->des2 = priv->rx_skbuff_dma[entry];
			if (unlikely(priv->is_gmac)) {
				if (bfsize >= BUF_SIZE_8KiB)
					(p + entry)->des3 =
					    (p + entry)->des2 + BUF_SIZE_8KiB;
			}
			RX_DBG(KERN_INFO "\trefill entry #%d\n", entry);
		}
		priv->mac_type->ops->set_rx_owner(p + entry);
	}
	return;
}

static int stmmac_rx(struct stmmac_priv *priv, int limit)
{
	unsigned int rxsize = priv->dma_rx_size;
	unsigned int entry = priv->cur_rx % rxsize;
	unsigned int next_entry;
	unsigned int count = 0;
	struct dma_desc *p = priv->dma_rx + entry;
	struct dma_desc *p_next;

#ifdef STMMAC_RX_DEBUG
	if (netif_msg_hw(priv)) {
		pr_debug(">>> stmmac_rx: descriptor ring:\n");
		display_ring(priv->dma_rx, rxsize);
	}
#endif
	count = 0;
	while (!priv->mac_type->ops->get_rx_owner(p)) {
		int status;

		if (count >= limit)
			break;

		count++;

		next_entry = (++priv->cur_rx) % rxsize;
		p_next = priv->dma_rx + next_entry;
		prefetch(p_next);

		
		status = (priv->mac_type->ops->rx_status(&priv->dev->stats,
							 &priv->xstats, p));
		if (unlikely(status == discard_frame))
			priv->dev->stats.rx_errors++;
		else {
			struct sk_buff *skb;
			
			int frame_len =
			    priv->mac_type->ops->get_rx_frame_len(p) - 4;

#ifdef STMMAC_RX_DEBUG
			if (frame_len > ETH_FRAME_LEN)
				pr_debug("\tRX frame size %d, COE status: %d\n",
					frame_len, status);

			if (netif_msg_hw(priv))
				pr_debug("\tdesc: %p [entry %d] buff=0x%x\n",
					p, entry, p->des2);
#endif
			skb = priv->rx_skbuff[entry];
			if (unlikely(!skb)) {
				pr_err("%s: Inconsistent Rx descriptor chain\n",
					priv->dev->name);
				priv->dev->stats.rx_dropped++;
				break;
			}
			prefetch(skb->data - NET_IP_ALIGN);
			priv->rx_skbuff[entry] = NULL;

			skb_put(skb, frame_len);
			dma_unmap_single(priv->device,
					 priv->rx_skbuff_dma[entry],
					 priv->dma_buf_sz, DMA_FROM_DEVICE);
#ifdef STMMAC_RX_DEBUG
			if (netif_msg_pktdata(priv)) {
				pr_info(" frame received (%dbytes)", frame_len);
				print_pkt(skb->data, frame_len);
			}
#endif
			skb->protocol = eth_type_trans(skb, priv->dev);

			if (unlikely(status == csum_none)) {
				
				skb->ip_summed = CHECKSUM_NONE;
				netif_receive_skb(skb);
			} else {
				skb->ip_summed = CHECKSUM_UNNECESSARY;
				napi_gro_receive(&priv->napi, skb);
			}

			priv->dev->stats.rx_packets++;
			priv->dev->stats.rx_bytes += frame_len;
			priv->dev->last_rx = jiffies;
		}
		entry = next_entry;
		p = p_next;	
	}

	stmmac_rx_refill(priv);

	priv->xstats.rx_pkt_n += count;

	return count;
}


static int stmmac_poll(struct napi_struct *napi, int budget)
{
	struct stmmac_priv *priv = container_of(napi, struct stmmac_priv, napi);
	int work_done = 0;

	priv->xstats.poll_n++;
	stmmac_tx(priv);
	work_done = stmmac_rx(priv, budget);

	if (work_done < budget) {
		napi_complete(napi);
		stmmac_enable_irq(priv);
	}
	return work_done;
}


static void stmmac_tx_timeout(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);

	
	stmmac_tx_err(priv);
	return;
}


static int stmmac_config(struct net_device *dev, struct ifmap *map)
{
	if (dev->flags & IFF_UP)	
		return -EBUSY;

	
	if (map->base_addr != dev->base_addr) {
		pr_warning("%s: can't change I/O address\n", dev->name);
		return -EOPNOTSUPP;
	}

	
	if (map->irq != dev->irq) {
		pr_warning("%s: can't change IRQ number %d\n",
		       dev->name, dev->irq);
		return -EOPNOTSUPP;
	}

	
	return 0;
}


static void stmmac_multicast_list(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);

	spin_lock(&priv->lock);
	priv->mac_type->ops->set_filter(dev);
	spin_unlock(&priv->lock);
	return;
}


static int stmmac_change_mtu(struct net_device *dev, int new_mtu)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	int max_mtu;

	if (netif_running(dev)) {
		pr_err("%s: must be stopped to change its MTU\n", dev->name);
		return -EBUSY;
	}

	if (priv->is_gmac)
		max_mtu = JUMBO_LEN;
	else
		max_mtu = ETH_DATA_LEN;

	if ((new_mtu < 46) || (new_mtu > max_mtu)) {
		pr_err("%s: invalid MTU, max MTU is: %d\n", dev->name, max_mtu);
		return -EINVAL;
	}

	dev->mtu = new_mtu;

	return 0;
}

static irqreturn_t stmmac_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct stmmac_priv *priv = netdev_priv(dev);

	if (unlikely(!dev)) {
		pr_err("%s: invalid dev pointer\n", __func__);
		return IRQ_NONE;
	}

	if (priv->is_gmac) {
		unsigned long ioaddr = dev->base_addr;
		
		priv->mac_type->ops->host_irq_status(ioaddr);
	}
	stmmac_dma_interrupt(dev);

	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER

static void stmmac_poll_controller(struct net_device *dev)
{
	disable_irq(dev->irq);
	stmmac_interrupt(dev->irq, dev);
	enable_irq(dev->irq);
}
#endif


static int stmmac_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	int ret = -EOPNOTSUPP;

	if (!netif_running(dev))
		return -EINVAL;

	switch (cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		if (!priv->phydev)
			return -EINVAL;

		spin_lock(&priv->lock);
		ret = phy_mii_ioctl(priv->phydev, if_mii(rq), cmd);
		spin_unlock(&priv->lock);
	default:
		break;
	}
	return ret;
}

#ifdef STMMAC_VLAN_TAG_USED
static void stmmac_vlan_rx_register(struct net_device *dev,
				    struct vlan_group *grp)
{
	struct stmmac_priv *priv = netdev_priv(dev);

	DBG(probe, INFO, "%s: Setting vlgrp to %p\n", dev->name, grp);

	spin_lock(&priv->lock);
	priv->vlgrp = grp;
	spin_unlock(&priv->lock);

	return;
}
#endif

static const struct net_device_ops stmmac_netdev_ops = {
	.ndo_open = stmmac_open,
	.ndo_start_xmit = stmmac_xmit,
	.ndo_stop = stmmac_release,
	.ndo_change_mtu = stmmac_change_mtu,
	.ndo_set_multicast_list = stmmac_multicast_list,
	.ndo_tx_timeout = stmmac_tx_timeout,
	.ndo_do_ioctl = stmmac_ioctl,
	.ndo_set_config = stmmac_config,
#ifdef STMMAC_VLAN_TAG_USED
	.ndo_vlan_rx_register = stmmac_vlan_rx_register,
#endif
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller = stmmac_poll_controller,
#endif
	.ndo_set_mac_address = eth_mac_addr,
};


static int stmmac_probe(struct net_device *dev)
{
	int ret = 0;
	struct stmmac_priv *priv = netdev_priv(dev);

	ether_setup(dev);

	dev->netdev_ops = &stmmac_netdev_ops;
	stmmac_set_ethtool_ops(dev);

	dev->features |= (NETIF_F_SG | NETIF_F_HW_CSUM | NETIF_F_HIGHDMA);
	dev->watchdog_timeo = msecs_to_jiffies(watchdog);
#ifdef STMMAC_VLAN_TAG_USED
	
	dev->features |= NETIF_F_HW_VLAN_RX;
#endif
	priv->msg_enable = netif_msg_init(debug, default_msg_level);

	if (priv->is_gmac)
		priv->rx_csum = 1;

	if (flow_ctrl)
		priv->flow_ctrl = FLOW_AUTO;	

	priv->pause = pause;
	netif_napi_add(dev, &priv->napi, stmmac_poll, 64);

	
	priv->mac_type->ops->get_umac_addr(dev->base_addr, dev->dev_addr, 0);

	if (!is_valid_ether_addr(dev->dev_addr))
		pr_warning("\tno valid MAC address;"
			"please, use ifconfig or nwhwconfig!\n");

	ret = register_netdev(dev);
	if (ret) {
		pr_err("%s: ERROR %i registering the device\n",
		       __func__, ret);
		return -ENODEV;
	}

	DBG(probe, DEBUG, "%s: Scatter/Gather: %s - HW checksums: %s\n",
	    dev->name, (dev->features & NETIF_F_SG) ? "on" : "off",
	    (dev->features & NETIF_F_HW_CSUM) ? "on" : "off");

	spin_lock_init(&priv->lock);

	return ret;
}


static int stmmac_mac_device_setup(struct net_device *dev)
{
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;

	struct mac_device_info *device;

	if (priv->is_gmac)
		device = gmac_setup(ioaddr);
	else
		device = mac100_setup(ioaddr);

	if (!device)
		return -ENOMEM;

	priv->mac_type = device;

	priv->wolenabled = priv->mac_type->hw.pmt;	
	if (priv->wolenabled == PMT_SUPPORTED)
		priv->wolopts = WAKE_MAGIC;		

	return 0;
}

static int stmmacphy_dvr_probe(struct platform_device *pdev)
{
	struct plat_stmmacphy_data *plat_dat;
	plat_dat = (struct plat_stmmacphy_data *)((pdev->dev).platform_data);

	pr_debug("stmmacphy_dvr_probe: added phy for bus %d\n",
	       plat_dat->bus_id);

	return 0;
}

static int stmmacphy_dvr_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver stmmacphy_driver = {
	.driver = {
		   .name = PHY_RESOURCE_NAME,
		   },
	.probe = stmmacphy_dvr_probe,
	.remove = stmmacphy_dvr_remove,
};


static int stmmac_associate_phy(struct device *dev, void *data)
{
	struct stmmac_priv *priv = (struct stmmac_priv *)data;
	struct plat_stmmacphy_data *plat_dat;

	plat_dat = (struct plat_stmmacphy_data *)(dev->platform_data);

	DBG(probe, DEBUG, "%s: checking phy for bus %d\n", __func__,
		plat_dat->bus_id);

	
	if (priv->bus_id != plat_dat->bus_id)
		return 0;

	
	DBG(probe, DEBUG, "%s: OK. Found PHY config\n", __func__);
	priv->phy_irq =
	    platform_get_irq_byname(to_platform_device(dev), "phyirq");
	DBG(probe, DEBUG, "%s: PHY irq on bus %d is %d\n", __func__,
	    plat_dat->bus_id, priv->phy_irq);

	
	if ((phyaddr >= 0) && (phyaddr <= 31))
		plat_dat->phy_addr = phyaddr;

	priv->phy_addr = plat_dat->phy_addr;
	priv->phy_mask = plat_dat->phy_mask;
	priv->phy_interface = plat_dat->interface;
	priv->phy_reset = plat_dat->phy_reset;

	DBG(probe, DEBUG, "%s: exiting\n", __func__);
	return 1;	
}


static int stmmac_dvr_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct resource *res;
	unsigned int *addr = NULL;
	struct net_device *ndev = NULL;
	struct stmmac_priv *priv;
	struct plat_stmmacenet_data *plat_dat;

	pr_info("STMMAC driver:\n\tplatform registration... ");
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		ret = -ENODEV;
		goto out;
	}
	pr_info("done!\n");

	if (!request_mem_region(res->start, (res->end - res->start),
				pdev->name)) {
		pr_err("%s: ERROR: memory allocation failed"
		       "cannot get the I/O addr 0x%x\n",
		       __func__, (unsigned int)res->start);
		ret = -EBUSY;
		goto out;
	}

	addr = ioremap(res->start, (res->end - res->start));
	if (!addr) {
		pr_err("%s: ERROR: memory mapping failed \n", __func__);
		ret = -ENOMEM;
		goto out;
	}

	ndev = alloc_etherdev(sizeof(struct stmmac_priv));
	if (!ndev) {
		pr_err("%s: ERROR: allocating the device\n", __func__);
		ret = -ENOMEM;
		goto out;
	}

	SET_NETDEV_DEV(ndev, &pdev->dev);

	
	ndev->irq = platform_get_irq_byname(pdev, "macirq");
	if (ndev->irq == -ENXIO) {
		pr_err("%s: ERROR: MAC IRQ configuration "
		       "information not found\n", __func__);
		ret = -ENODEV;
		goto out;
	}

	priv = netdev_priv(ndev);
	priv->device = &(pdev->dev);
	priv->dev = ndev;
	plat_dat = (struct plat_stmmacenet_data *)((pdev->dev).platform_data);
	priv->bus_id = plat_dat->bus_id;
	priv->pbl = plat_dat->pbl;	
	priv->is_gmac = plat_dat->has_gmac;	

	platform_set_drvdata(pdev, ndev);

	
	ndev->base_addr = (unsigned long)addr;

	
	ret = stmmac_mac_device_setup(ndev);
	if (ret < 0)
		goto out;

	
	ret = stmmac_probe(ndev);
	if (ret < 0)
		goto out;

	
	if (!driver_for_each_device
	    (&(stmmacphy_driver.driver), NULL, (void *)priv,
	     stmmac_associate_phy)) {
		pr_err("No PHY device is associated with this MAC!\n");
		ret = -ENODEV;
		goto out;
	}

	priv->fix_mac_speed = plat_dat->fix_mac_speed;
	priv->bsp_priv = plat_dat->bsp_priv;

	pr_info("\t%s - (dev. name: %s - id: %d, IRQ #%d\n"
	       "\tIO base addr: 0x%08x)\n", ndev->name, pdev->name,
	       pdev->id, ndev->irq, (unsigned int)addr);

	
	pr_debug("\tMDIO bus (id: %d)...", priv->bus_id);
	ret = stmmac_mdio_register(ndev);
	if (ret < 0)
		goto out;
	pr_debug("registered!\n");

out:
	if (ret < 0) {
		platform_set_drvdata(pdev, NULL);
		release_mem_region(res->start, (res->end - res->start));
		if (addr != NULL)
			iounmap(addr);
	}

	return ret;
}


static int stmmac_dvr_remove(struct platform_device *pdev)
{
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct resource *res;

	pr_info("%s:\n\tremoving driver", __func__);

	stmmac_dma_stop_rx(ndev->base_addr);
	stmmac_dma_stop_tx(ndev->base_addr);

	stmmac_mac_disable_rx(ndev->base_addr);
	stmmac_mac_disable_tx(ndev->base_addr);

	netif_carrier_off(ndev);

	stmmac_mdio_unregister(ndev);

	platform_set_drvdata(pdev, NULL);
	unregister_netdev(ndev);

	iounmap((void *)ndev->base_addr);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, (res->end - res->start));

	free_netdev(ndev);

	return 0;
}

#ifdef CONFIG_PM
static int stmmac_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct stmmac_priv *priv = netdev_priv(dev);
	int dis_ic = 0;

	if (!dev || !netif_running(dev))
		return 0;

	spin_lock(&priv->lock);

	if (state.event == PM_EVENT_SUSPEND) {
		netif_device_detach(dev);
		netif_stop_queue(dev);
		if (priv->phydev)
			phy_stop(priv->phydev);

#ifdef CONFIG_STMMAC_TIMER
		priv->tm->timer_stop();
		if (likely(priv->tm->enable))
			dis_ic = 1;
#endif
		napi_disable(&priv->napi);

		
		stmmac_dma_stop_tx(dev->base_addr);
		stmmac_dma_stop_rx(dev->base_addr);
		
		priv->mac_type->ops->init_rx_desc(priv->dma_rx,
						  priv->dma_rx_size, dis_ic);
		priv->mac_type->ops->init_tx_desc(priv->dma_tx,
						  priv->dma_tx_size);

		stmmac_mac_disable_tx(dev->base_addr);

		if (device_may_wakeup(&(pdev->dev))) {
			
			if (priv->wolenabled == PMT_SUPPORTED)
				priv->mac_type->ops->pmt(dev->base_addr,
							 priv->wolopts);
		} else {
			stmmac_mac_disable_rx(dev->base_addr);
		}
	} else {
		priv->shutdown = 1;
		
		stmmac_release(dev);
	}

	spin_unlock(&priv->lock);
	return 0;
}

static int stmmac_resume(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct stmmac_priv *priv = netdev_priv(dev);
	unsigned long ioaddr = dev->base_addr;

	if (!netif_running(dev))
		return 0;

	spin_lock(&priv->lock);

	if (priv->shutdown) {
		
		stmmac_open(dev);
		goto out_resume;
	}

	
	if (device_may_wakeup(&(pdev->dev)))
		if (priv->wolenabled == PMT_SUPPORTED)
			priv->mac_type->ops->pmt(dev->base_addr, 0);

	netif_device_attach(dev);

	
	stmmac_mac_enable_rx(ioaddr);
	stmmac_mac_enable_tx(ioaddr);
	stmmac_dma_start_tx(ioaddr);
	stmmac_dma_start_rx(ioaddr);

#ifdef CONFIG_STMMAC_TIMER
	priv->tm->timer_start(tmrate);
#endif
	napi_enable(&priv->napi);

	if (priv->phydev)
		phy_start(priv->phydev);

	netif_start_queue(dev);

out_resume:
	spin_unlock(&priv->lock);
	return 0;
}
#endif

static struct platform_driver stmmac_driver = {
	.driver = {
		   .name = STMMAC_RESOURCE_NAME,
		   },
	.probe = stmmac_dvr_probe,
	.remove = stmmac_dvr_remove,
#ifdef CONFIG_PM
	.suspend = stmmac_suspend,
	.resume = stmmac_resume,
#endif

};


static int __init stmmac_init_module(void)
{
	int ret;

	if (platform_driver_register(&stmmacphy_driver)) {
		pr_err("No PHY devices registered!\n");
		return -ENODEV;
	}

	ret = platform_driver_register(&stmmac_driver);
	return ret;
}


static void __exit stmmac_cleanup_module(void)
{
	platform_driver_unregister(&stmmacphy_driver);
	platform_driver_unregister(&stmmac_driver);
}

#ifndef MODULE
static int __init stmmac_cmdline_opt(char *str)
{
	char *opt;

	if (!str || !*str)
		return -EINVAL;
	while ((opt = strsep(&str, ",")) != NULL) {
		if (!strncmp(opt, "debug:", 6))
			strict_strtoul(opt + 6, 0, (unsigned long *)&debug);
		else if (!strncmp(opt, "phyaddr:", 8))
			strict_strtoul(opt + 8, 0, (unsigned long *)&phyaddr);
		else if (!strncmp(opt, "dma_txsize:", 11))
			strict_strtoul(opt + 11, 0,
				       (unsigned long *)&dma_txsize);
		else if (!strncmp(opt, "dma_rxsize:", 11))
			strict_strtoul(opt + 11, 0,
				       (unsigned long *)&dma_rxsize);
		else if (!strncmp(opt, "buf_sz:", 7))
			strict_strtoul(opt + 7, 0, (unsigned long *)&buf_sz);
		else if (!strncmp(opt, "tc:", 3))
			strict_strtoul(opt + 3, 0, (unsigned long *)&tc);
		else if (!strncmp(opt, "tx_coe:", 7))
			strict_strtoul(opt + 7, 0, (unsigned long *)&tx_coe);
		else if (!strncmp(opt, "watchdog:", 9))
			strict_strtoul(opt + 9, 0, (unsigned long *)&watchdog);
		else if (!strncmp(opt, "flow_ctrl:", 10))
			strict_strtoul(opt + 10, 0,
				       (unsigned long *)&flow_ctrl);
		else if (!strncmp(opt, "pause:", 6))
			strict_strtoul(opt + 6, 0, (unsigned long *)&pause);
#ifdef CONFIG_STMMAC_TIMER
		else if (!strncmp(opt, "tmrate:", 7))
			strict_strtoul(opt + 7, 0, (unsigned long *)&tmrate);
#endif
	}
	return 0;
}

__setup("stmmaceth=", stmmac_cmdline_opt);
#endif

module_init(stmmac_init_module);
module_exit(stmmac_cleanup_module);

MODULE_DESCRIPTION("STMMAC 10/100/1000 Ethernet driver");
MODULE_AUTHOR("Giuseppe Cavallaro <peppe.cavallaro@st.com>");
MODULE_LICENSE("GPL");
