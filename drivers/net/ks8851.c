

#define DEBUG

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/cache.h>
#include <linux/crc32.h>
#include <linux/mii.h>

#include <linux/spi/spi.h>

#include "ks8851.h"


struct ks8851_rxctrl {
	u16	mchash[4];
	u16	rxcr1;
	u16	rxcr2;
};


union ks8851_tx_hdr {
	u8	txb[6];
	__le16	txw[3];
};


struct ks8851_net {
	struct net_device	*netdev;
	struct spi_device	*spidev;
	struct mutex		lock;
	spinlock_t		statelock;

	union ks8851_tx_hdr	txh ____cacheline_aligned;
	u8			rxd[8];
	u8			txd[8];

	u32			msg_enable ____cacheline_aligned;
	u16			tx_space;
	u8			fid;

	u16			rc_ier;
	u16			rc_rxqcr;

	struct mii_if_info	mii;
	struct ks8851_rxctrl	rxctrl;

	struct work_struct	tx_work;
	struct work_struct	irq_work;
	struct work_struct	rxctrl_work;

	struct sk_buff_head	txq;

	struct spi_message	spi_msg1;
	struct spi_message	spi_msg2;
	struct spi_transfer	spi_xfer1;
	struct spi_transfer	spi_xfer2[2];
};

static int msg_enable;

#define ks_info(_ks, _msg...) dev_info(&(_ks)->spidev->dev, _msg)
#define ks_warn(_ks, _msg...) dev_warn(&(_ks)->spidev->dev, _msg)
#define ks_dbg(_ks, _msg...) dev_dbg(&(_ks)->spidev->dev, _msg)
#define ks_err(_ks, _msg...) dev_err(&(_ks)->spidev->dev, _msg)


#define BYTE_EN(_x)	((_x) << 2)


#define MK_OP(_byteen, _reg) (BYTE_EN(_byteen) | (_reg)  << (8+2) | (_reg) >> 6)




static void ks8851_wrreg16(struct ks8851_net *ks, unsigned reg, unsigned val)
{
	struct spi_transfer *xfer = &ks->spi_xfer1;
	struct spi_message *msg = &ks->spi_msg1;
	__le16 txb[2];
	int ret;

	txb[0] = cpu_to_le16(MK_OP(reg & 2 ? 0xC : 0x03, reg) | KS_SPIOP_WR);
	txb[1] = cpu_to_le16(val);

	xfer->tx_buf = txb;
	xfer->rx_buf = NULL;
	xfer->len = 4;

	ret = spi_sync(ks->spidev, msg);
	if (ret < 0)
		ks_err(ks, "spi_sync() failed\n");
}


static void ks8851_wrreg8(struct ks8851_net *ks, unsigned reg, unsigned val)
{
	struct spi_transfer *xfer = &ks->spi_xfer1;
	struct spi_message *msg = &ks->spi_msg1;
	__le16 txb[2];
	int ret;
	int bit;

	bit = 1 << (reg & 3);

	txb[0] = cpu_to_le16(MK_OP(bit, reg) | KS_SPIOP_WR);
	txb[1] = val;

	xfer->tx_buf = txb;
	xfer->rx_buf = NULL;
	xfer->len = 3;

	ret = spi_sync(ks->spidev, msg);
	if (ret < 0)
		ks_err(ks, "spi_sync() failed\n");
}


static inline bool ks8851_rx_1msg(struct ks8851_net *ks)
{
	return true;
}


static void ks8851_rdreg(struct ks8851_net *ks, unsigned op,
			 u8 *rxb, unsigned rxl)
{
	struct spi_transfer *xfer;
	struct spi_message *msg;
	__le16 *txb = (__le16 *)ks->txd;
	u8 *trx = ks->rxd;
	int ret;

	txb[0] = cpu_to_le16(op | KS_SPIOP_RD);

	if (ks8851_rx_1msg(ks)) {
		msg = &ks->spi_msg1;
		xfer = &ks->spi_xfer1;

		xfer->tx_buf = txb;
		xfer->rx_buf = trx;
		xfer->len = rxl + 2;
	} else {
		msg = &ks->spi_msg2;
		xfer = ks->spi_xfer2;

		xfer->tx_buf = txb;
		xfer->rx_buf = NULL;
		xfer->len = 2;

		xfer++;
		xfer->tx_buf = NULL;
		xfer->rx_buf = trx;
		xfer->len = rxl;
	}

	ret = spi_sync(ks->spidev, msg);
	if (ret < 0)
		ks_err(ks, "read: spi_sync() failed\n");
	else if (ks8851_rx_1msg(ks))
		memcpy(rxb, trx + 2, rxl);
	else
		memcpy(rxb, trx, rxl);
}


static unsigned ks8851_rdreg8(struct ks8851_net *ks, unsigned reg)
{
	u8 rxb[1];

	ks8851_rdreg(ks, MK_OP(1 << (reg & 3), reg), rxb, 1);
	return rxb[0];
}


static unsigned ks8851_rdreg16(struct ks8851_net *ks, unsigned reg)
{
	__le16 rx = 0;

	ks8851_rdreg(ks, MK_OP(reg & 2 ? 0xC : 0x3, reg), (u8 *)&rx, 2);
	return le16_to_cpu(rx);
}


static unsigned ks8851_rdreg32(struct ks8851_net *ks, unsigned reg)
{
	__le32 rx = 0;

	WARN_ON(reg & 3);

	ks8851_rdreg(ks, MK_OP(0xf, reg), (u8 *)&rx, 4);
	return le32_to_cpu(rx);
}


static void ks8851_soft_reset(struct ks8851_net *ks, unsigned op)
{
	ks8851_wrreg16(ks, KS_GRR, op);
	mdelay(1);	
	ks8851_wrreg16(ks, KS_GRR, 0);
	mdelay(1);	
}


static int ks8851_write_mac_addr(struct net_device *dev)
{
	struct ks8851_net *ks = netdev_priv(dev);
	int i;

	mutex_lock(&ks->lock);

	for (i = 0; i < ETH_ALEN; i++)
		ks8851_wrreg8(ks, KS_MAR(i), dev->dev_addr[i]);

	mutex_unlock(&ks->lock);

	return 0;
}


static void ks8851_init_mac(struct ks8851_net *ks)
{
	struct net_device *dev = ks->netdev;

	random_ether_addr(dev->dev_addr);
	ks8851_write_mac_addr(dev);
}


static irqreturn_t ks8851_irq(int irq, void *pw)
{
	struct ks8851_net *ks = pw;

	disable_irq_nosync(irq);
	schedule_work(&ks->irq_work);
	return IRQ_HANDLED;
}


static void ks8851_rdfifo(struct ks8851_net *ks, u8 *buff, unsigned len)
{
	struct spi_transfer *xfer = ks->spi_xfer2;
	struct spi_message *msg = &ks->spi_msg2;
	u8 txb[1];
	int ret;

	if (netif_msg_rx_status(ks))
		ks_dbg(ks, "%s: %d@%p\n", __func__, len, buff);

	
	txb[0] = KS_SPIOP_RXFIFO;

	xfer->tx_buf = txb;
	xfer->rx_buf = NULL;
	xfer->len = 1;

	xfer++;
	xfer->rx_buf = buff;
	xfer->tx_buf = NULL;
	xfer->len = len;

	ret = spi_sync(ks->spidev, msg);
	if (ret < 0)
		ks_err(ks, "%s: spi_sync() failed\n", __func__);
}


static void ks8851_dbg_dumpkkt(struct ks8851_net *ks, u8 *rxpkt)
{
	ks_dbg(ks, "pkt %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",
	       rxpkt[4], rxpkt[5], rxpkt[6], rxpkt[7],
	       rxpkt[8], rxpkt[9], rxpkt[10], rxpkt[11],
	       rxpkt[12], rxpkt[13], rxpkt[14], rxpkt[15]);
}


static void ks8851_rx_pkts(struct ks8851_net *ks)
{
	struct sk_buff *skb;
	unsigned rxfc;
	unsigned rxlen;
	unsigned rxstat;
	u32 rxh;
	u8 *rxpkt;

	rxfc = ks8851_rdreg8(ks, KS_RXFC);

	if (netif_msg_rx_status(ks))
		ks_dbg(ks, "%s: %d packets\n", __func__, rxfc);

	

	for (; rxfc != 0; rxfc--) {
		rxh = ks8851_rdreg32(ks, KS_RXFHSR);
		rxstat = rxh & 0xffff;
		rxlen = rxh >> 16;

		if (netif_msg_rx_status(ks))
			ks_dbg(ks, "rx: stat 0x%04x, len 0x%04x\n",
				rxstat, rxlen);

		

		
		ks8851_wrreg16(ks, KS_RXFDPR, RXFDPR_RXFPAI | 0x00);

		
		ks8851_wrreg16(ks, KS_RXQCR,
			       ks->rc_rxqcr | RXQCR_SDA | RXQCR_ADRFE);

		if (rxlen > 0) {
			skb = netdev_alloc_skb(ks->netdev, rxlen + 2 + 8);
			if (!skb) {
				
			}

			
			skb_reserve(skb, 2 + 4 + 4);

			rxpkt = skb_put(skb, rxlen - 4) - 8;

			
			ks8851_rdfifo(ks, rxpkt, ALIGN(rxlen, 4) + 8);

			if (netif_msg_pktdata(ks))
				ks8851_dbg_dumpkkt(ks, rxpkt);

			skb->protocol = eth_type_trans(skb, ks->netdev);
			netif_rx(skb);

			ks->netdev->stats.rx_packets++;
			ks->netdev->stats.rx_bytes += rxlen - 4;
		}

		ks8851_wrreg16(ks, KS_RXQCR, ks->rc_rxqcr);
	}
}


static void ks8851_irq_work(struct work_struct *work)
{
	struct ks8851_net *ks = container_of(work, struct ks8851_net, irq_work);
	unsigned status;
	unsigned handled = 0;

	mutex_lock(&ks->lock);

	status = ks8851_rdreg16(ks, KS_ISR);

	if (netif_msg_intr(ks))
		dev_dbg(&ks->spidev->dev, "%s: status 0x%04x\n",
			__func__, status);

	if (status & IRQ_LCI) {
		
		handled |= IRQ_LCI;
	}

	if (status & IRQ_LDI) {
		u16 pmecr = ks8851_rdreg16(ks, KS_PMECR);
		pmecr &= ~PMECR_WKEVT_MASK;
		ks8851_wrreg16(ks, KS_PMECR, pmecr | PMECR_WKEVT_LINK);

		handled |= IRQ_LDI;
	}

	if (status & IRQ_RXPSI)
		handled |= IRQ_RXPSI;

	if (status & IRQ_TXI) {
		handled |= IRQ_TXI;

		

		
		ks->tx_space = ks8851_rdreg16(ks, KS_TXMIR);

		if (netif_msg_intr(ks))
			ks_dbg(ks, "%s: txspace %d\n", __func__, ks->tx_space);
	}

	if (status & IRQ_RXI)
		handled |= IRQ_RXI;

	if (status & IRQ_SPIBEI) {
		dev_err(&ks->spidev->dev, "%s: spi bus error\n", __func__);
		handled |= IRQ_SPIBEI;
	}

	ks8851_wrreg16(ks, KS_ISR, handled);

	if (status & IRQ_RXI) {
		

		ks8851_rx_pkts(ks);
	}

	
	if (status & IRQ_RXPSI) {
		struct ks8851_rxctrl *rxc = &ks->rxctrl;

		
		ks8851_wrreg16(ks, KS_MAHTR0, rxc->mchash[0]);
		ks8851_wrreg16(ks, KS_MAHTR1, rxc->mchash[1]);
		ks8851_wrreg16(ks, KS_MAHTR2, rxc->mchash[2]);
		ks8851_wrreg16(ks, KS_MAHTR3, rxc->mchash[3]);

		ks8851_wrreg16(ks, KS_RXCR2, rxc->rxcr2);
		ks8851_wrreg16(ks, KS_RXCR1, rxc->rxcr1);
	}

	mutex_unlock(&ks->lock);

	if (status & IRQ_TXI)
		netif_wake_queue(ks->netdev);

	enable_irq(ks->netdev->irq);
}


static inline unsigned calc_txlen(unsigned len)
{
	return ALIGN(len + 4, 4);
}


static void ks8851_wrpkt(struct ks8851_net *ks, struct sk_buff *txp, bool irq)
{
	struct spi_transfer *xfer = ks->spi_xfer2;
	struct spi_message *msg = &ks->spi_msg2;
	unsigned fid = 0;
	int ret;

	if (netif_msg_tx_queued(ks))
		dev_dbg(&ks->spidev->dev, "%s: skb %p, %d@%p, irq %d\n",
			__func__, txp, txp->len, txp->data, irq);

	fid = ks->fid++;
	fid &= TXFR_TXFID_MASK;

	if (irq)
		fid |= TXFR_TXIC;	

	
	ks->txh.txb[1] = KS_SPIOP_TXFIFO;
	ks->txh.txw[1] = cpu_to_le16(fid);
	ks->txh.txw[2] = cpu_to_le16(txp->len);

	xfer->tx_buf = &ks->txh.txb[1];
	xfer->rx_buf = NULL;
	xfer->len = 5;

	xfer++;
	xfer->tx_buf = txp->data;
	xfer->rx_buf = NULL;
	xfer->len = ALIGN(txp->len, 4);

	ret = spi_sync(ks->spidev, msg);
	if (ret < 0)
		ks_err(ks, "%s: spi_sync() failed\n", __func__);
}


static void ks8851_done_tx(struct ks8851_net *ks, struct sk_buff *txb)
{
	struct net_device *dev = ks->netdev;

	dev->stats.tx_bytes += txb->len;
	dev->stats.tx_packets++;

	dev_kfree_skb(txb);
}


static void ks8851_tx_work(struct work_struct *work)
{
	struct ks8851_net *ks = container_of(work, struct ks8851_net, tx_work);
	struct sk_buff *txb;
	bool last = false;

	mutex_lock(&ks->lock);

	while (!last) {
		txb = skb_dequeue(&ks->txq);
		last = skb_queue_empty(&ks->txq);

		ks8851_wrreg16(ks, KS_RXQCR, ks->rc_rxqcr | RXQCR_SDA);
		ks8851_wrpkt(ks, txb, last);
		ks8851_wrreg16(ks, KS_RXQCR, ks->rc_rxqcr);
		ks8851_wrreg16(ks, KS_TXQCR, TXQCR_METFE);

		ks8851_done_tx(ks, txb);
	}

	mutex_unlock(&ks->lock);
}


static void ks8851_set_powermode(struct ks8851_net *ks, unsigned pwrmode)
{
	unsigned pmecr;

	if (netif_msg_hw(ks))
		ks_dbg(ks, "setting power mode %d\n", pwrmode);

	pmecr = ks8851_rdreg16(ks, KS_PMECR);
	pmecr &= ~PMECR_PM_MASK;
	pmecr |= pwrmode;

	ks8851_wrreg16(ks, KS_PMECR, pmecr);
}


static int ks8851_net_open(struct net_device *dev)
{
	struct ks8851_net *ks = netdev_priv(dev);

	
	mutex_lock(&ks->lock);

	if (netif_msg_ifup(ks))
		ks_dbg(ks, "opening %s\n", dev->name);

	
	ks8851_set_powermode(ks, PMECR_PM_NORMAL);

	
	ks8851_soft_reset(ks, GRR_QMU);

	

	ks8851_wrreg16(ks, KS_TXCR, (TXCR_TXE | 
				     TXCR_TXPE | 
				     TXCR_TXCRC | 
				     TXCR_TXFCE)); 

	
	ks8851_wrreg16(ks, KS_TXFDPR, TXFDPR_TXFPAI);

	

	ks8851_wrreg16(ks, KS_RXCR1, (RXCR1_RXPAFMA | 
				      RXCR1_RXFCE | 
				      RXCR1_RXBE | 
				      RXCR1_RXUE | 
				      RXCR1_RXE)); 

	
	ks8851_wrreg16(ks, KS_RXCR2, RXCR2_SRDBL_FRAME);

	
	ks8851_wrreg16(ks, KS_RXDTTR, 1000); 
	ks8851_wrreg16(ks, KS_RXDBCTR, 4096); 
	ks8851_wrreg16(ks, KS_RXFCTR, 10);  

	ks->rc_rxqcr = (RXQCR_RXFCTE |  
			RXQCR_RXDBCTE | 
			RXQCR_RXDTTE);  

	ks8851_wrreg16(ks, KS_RXQCR, ks->rc_rxqcr);

	

#define STD_IRQ (IRQ_LCI |		\
		 IRQ_TXI |			\
		 IRQ_RXI |			\
		 IRQ_SPIBEI |		\
		 IRQ_TXPSI |		\
		 IRQ_RXPSI)	

	ks->rc_ier = STD_IRQ;
	ks8851_wrreg16(ks, KS_ISR, STD_IRQ);
	ks8851_wrreg16(ks, KS_IER, STD_IRQ);

	netif_start_queue(ks->netdev);

	if (netif_msg_ifup(ks))
		ks_dbg(ks, "network device %s up\n", dev->name);

	mutex_unlock(&ks->lock);
	return 0;
}


static int ks8851_net_stop(struct net_device *dev)
{
	struct ks8851_net *ks = netdev_priv(dev);

	if (netif_msg_ifdown(ks))
		ks_info(ks, "%s: shutting down\n", dev->name);

	netif_stop_queue(dev);

	mutex_lock(&ks->lock);

	
	flush_work(&ks->irq_work);
	flush_work(&ks->tx_work);
	flush_work(&ks->rxctrl_work);

	
	ks8851_wrreg16(ks, KS_IER, 0x0000);
	ks8851_wrreg16(ks, KS_ISR, 0xffff);

	
	ks8851_wrreg16(ks, KS_RXCR1, 0x0000);

	
	ks8851_wrreg16(ks, KS_TXCR, 0x0000);

	
	ks8851_set_powermode(ks, PMECR_PM_SOFTDOWN);

	
	while (!skb_queue_empty(&ks->txq)) {
		struct sk_buff *txb = skb_dequeue(&ks->txq);

		if (netif_msg_ifdown(ks))
			ks_dbg(ks, "%s: freeing txb %p\n", __func__, txb);

		dev_kfree_skb(txb);
	}

	mutex_unlock(&ks->lock);
	return 0;
}


static netdev_tx_t ks8851_start_xmit(struct sk_buff *skb,
				     struct net_device *dev)
{
	struct ks8851_net *ks = netdev_priv(dev);
	unsigned needed = calc_txlen(skb->len);
	netdev_tx_t ret = NETDEV_TX_OK;

	if (netif_msg_tx_queued(ks))
		ks_dbg(ks, "%s: skb %p, %d@%p\n", __func__,
		       skb, skb->len, skb->data);

	spin_lock(&ks->statelock);

	if (needed > ks->tx_space) {
		netif_stop_queue(dev);
		ret = NETDEV_TX_BUSY;
	} else {
		ks->tx_space -= needed;
		skb_queue_tail(&ks->txq, skb);
	}

	spin_unlock(&ks->statelock);
	schedule_work(&ks->tx_work);

	return ret;
}


static void ks8851_rxctrl_work(struct work_struct *work)
{
	struct ks8851_net *ks = container_of(work, struct ks8851_net, rxctrl_work);

	mutex_lock(&ks->lock);

	
	ks8851_wrreg16(ks, KS_RXCR1, 0x00);

	mutex_unlock(&ks->lock);
}

static void ks8851_set_rx_mode(struct net_device *dev)
{
	struct ks8851_net *ks = netdev_priv(dev);
	struct ks8851_rxctrl rxctrl;

	memset(&rxctrl, 0, sizeof(rxctrl));

	if (dev->flags & IFF_PROMISC) {
		

		rxctrl.rxcr1 = RXCR1_RXAE | RXCR1_RXINVF;
	} else if (dev->flags & IFF_ALLMULTI) {
		

		rxctrl.rxcr1 = (RXCR1_RXME | RXCR1_RXAE |
				RXCR1_RXPAFMA | RXCR1_RXMAFMA);
	} else if (dev->flags & IFF_MULTICAST && dev->mc_count > 0) {
		struct dev_mc_list *mcptr = dev->mc_list;
		u32 crc;
		int i;

		

		for (i = dev->mc_count; i > 0; i--) {
			crc = ether_crc(ETH_ALEN, mcptr->dmi_addr);
			crc >>= (32 - 6);  

			rxctrl.mchash[crc >> 4] |= (1 << (crc & 0xf));
			mcptr = mcptr->next;
		}

		rxctrl.rxcr1 = RXCR1_RXME | RXCR1_RXPAFMA;
	} else {
		
		rxctrl.rxcr1 = RXCR1_RXPAFMA;
	}

	rxctrl.rxcr1 |= (RXCR1_RXUE | 
			 RXCR1_RXBE | 
			 RXCR1_RXE | 
			 RXCR1_RXFCE); 

	rxctrl.rxcr2 |= RXCR2_SRDBL_FRAME;

	

	spin_lock(&ks->statelock);

	if (memcmp(&rxctrl, &ks->rxctrl, sizeof(rxctrl)) != 0) {
		memcpy(&ks->rxctrl, &rxctrl, sizeof(ks->rxctrl));
		schedule_work(&ks->rxctrl_work);
	}

	spin_unlock(&ks->statelock);
}

static int ks8851_set_mac_address(struct net_device *dev, void *addr)
{
	struct sockaddr *sa = addr;

	if (netif_running(dev))
		return -EBUSY;

	if (!is_valid_ether_addr(sa->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(dev->dev_addr, sa->sa_data, ETH_ALEN);
	return ks8851_write_mac_addr(dev);
}

static int ks8851_net_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
	struct ks8851_net *ks = netdev_priv(dev);

	if (!netif_running(dev))
		return -EINVAL;

	return generic_mii_ioctl(&ks->mii, if_mii(req), cmd, NULL);
}

static const struct net_device_ops ks8851_netdev_ops = {
	.ndo_open		= ks8851_net_open,
	.ndo_stop		= ks8851_net_stop,
	.ndo_do_ioctl		= ks8851_net_ioctl,
	.ndo_start_xmit		= ks8851_start_xmit,
	.ndo_set_mac_address	= ks8851_set_mac_address,
	.ndo_set_rx_mode	= ks8851_set_rx_mode,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
};



static void ks8851_get_drvinfo(struct net_device *dev,
			       struct ethtool_drvinfo *di)
{
	strlcpy(di->driver, "KS8851", sizeof(di->driver));
	strlcpy(di->version, "1.00", sizeof(di->version));
	strlcpy(di->bus_info, dev_name(dev->dev.parent), sizeof(di->bus_info));
}

static u32 ks8851_get_msglevel(struct net_device *dev)
{
	struct ks8851_net *ks = netdev_priv(dev);
	return ks->msg_enable;
}

static void ks8851_set_msglevel(struct net_device *dev, u32 to)
{
	struct ks8851_net *ks = netdev_priv(dev);
	ks->msg_enable = to;
}

static int ks8851_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct ks8851_net *ks = netdev_priv(dev);
	return mii_ethtool_gset(&ks->mii, cmd);
}

static int ks8851_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct ks8851_net *ks = netdev_priv(dev);
	return mii_ethtool_sset(&ks->mii, cmd);
}

static u32 ks8851_get_link(struct net_device *dev)
{
	struct ks8851_net *ks = netdev_priv(dev);
	return mii_link_ok(&ks->mii);
}

static int ks8851_nway_reset(struct net_device *dev)
{
	struct ks8851_net *ks = netdev_priv(dev);
	return mii_nway_restart(&ks->mii);
}

static const struct ethtool_ops ks8851_ethtool_ops = {
	.get_drvinfo	= ks8851_get_drvinfo,
	.get_msglevel	= ks8851_get_msglevel,
	.set_msglevel	= ks8851_set_msglevel,
	.get_settings	= ks8851_get_settings,
	.set_settings	= ks8851_set_settings,
	.get_link	= ks8851_get_link,
	.nway_reset	= ks8851_nway_reset,
};




static int ks8851_phy_reg(int reg)
{
	switch (reg) {
	case MII_BMCR:
		return KS_P1MBCR;
	case MII_BMSR:
		return KS_P1MBSR;
	case MII_PHYSID1:
		return KS_PHY1ILR;
	case MII_PHYSID2:
		return KS_PHY1IHR;
	case MII_ADVERTISE:
		return KS_P1ANAR;
	case MII_LPA:
		return KS_P1ANLPR;
	}

	return 0x0;
}


static int ks8851_phy_read(struct net_device *dev, int phy_addr, int reg)
{
	struct ks8851_net *ks = netdev_priv(dev);
	int ksreg;
	int result;

	ksreg = ks8851_phy_reg(reg);
	if (!ksreg)
		return 0x0;	

	mutex_lock(&ks->lock);
	result = ks8851_rdreg16(ks, ksreg);
	mutex_unlock(&ks->lock);

	return result;
}

static void ks8851_phy_write(struct net_device *dev,
			     int phy, int reg, int value)
{
	struct ks8851_net *ks = netdev_priv(dev);
	int ksreg;

	ksreg = ks8851_phy_reg(reg);
	if (ksreg) {
		mutex_lock(&ks->lock);
		ks8851_wrreg16(ks, ksreg, value);
		mutex_unlock(&ks->lock);
	}
}


static int ks8851_read_selftest(struct ks8851_net *ks)
{
	unsigned both_done = MBIR_TXMBF | MBIR_RXMBF;
	int ret = 0;
	unsigned rd;

	rd = ks8851_rdreg16(ks, KS_MBIR);

	if ((rd & both_done) != both_done) {
		ks_warn(ks, "Memory selftest not finished\n");
		return 0;
	}

	if (rd & MBIR_TXMBFA) {
		ks_err(ks, "TX memory selftest fail\n");
		ret |= 1;
	}

	if (rd & MBIR_RXMBFA) {
		ks_err(ks, "RX memory selftest fail\n");
		ret |= 2;
	}

	return 0;
}



static int __devinit ks8851_probe(struct spi_device *spi)
{
	struct net_device *ndev;
	struct ks8851_net *ks;
	int ret;

	ndev = alloc_etherdev(sizeof(struct ks8851_net));
	if (!ndev) {
		dev_err(&spi->dev, "failed to alloc ethernet device\n");
		return -ENOMEM;
	}

	spi->bits_per_word = 8;

	ks = netdev_priv(ndev);

	ks->netdev = ndev;
	ks->spidev = spi;
	ks->tx_space = 6144;

	mutex_init(&ks->lock);
	spin_lock_init(&ks->statelock);

	INIT_WORK(&ks->tx_work, ks8851_tx_work);
	INIT_WORK(&ks->irq_work, ks8851_irq_work);
	INIT_WORK(&ks->rxctrl_work, ks8851_rxctrl_work);

	

	spi_message_init(&ks->spi_msg1);
	spi_message_add_tail(&ks->spi_xfer1, &ks->spi_msg1);

	spi_message_init(&ks->spi_msg2);
	spi_message_add_tail(&ks->spi_xfer2[0], &ks->spi_msg2);
	spi_message_add_tail(&ks->spi_xfer2[1], &ks->spi_msg2);

	
	ks->mii.dev		= ndev;
	ks->mii.phy_id		= 1,
	ks->mii.phy_id_mask	= 1;
	ks->mii.reg_num_mask	= 0xf;
	ks->mii.mdio_read	= ks8851_phy_read;
	ks->mii.mdio_write	= ks8851_phy_write;

	dev_info(&spi->dev, "message enable is %d\n", msg_enable);

	
	ks->msg_enable = netif_msg_init(msg_enable, (NETIF_MSG_DRV |
						     NETIF_MSG_PROBE |
						     NETIF_MSG_LINK));

	skb_queue_head_init(&ks->txq);

	SET_ETHTOOL_OPS(ndev, &ks8851_ethtool_ops);
	SET_NETDEV_DEV(ndev, &spi->dev);

	dev_set_drvdata(&spi->dev, ks);

	ndev->if_port = IF_PORT_100BASET;
	ndev->netdev_ops = &ks8851_netdev_ops;
	ndev->irq = spi->irq;

	
	ks8851_soft_reset(ks, GRR_GSR);

	

	if ((ks8851_rdreg16(ks, KS_CIDER) & ~CIDER_REV_MASK) != CIDER_ID) {
		dev_err(&spi->dev, "failed to read device ID\n");
		ret = -ENODEV;
		goto err_id;
	}

	ks8851_read_selftest(ks);
	ks8851_init_mac(ks);

	ret = request_irq(spi->irq, ks8851_irq, IRQF_TRIGGER_LOW,
			  ndev->name, ks);
	if (ret < 0) {
		dev_err(&spi->dev, "failed to get irq\n");
		goto err_irq;
	}

	ret = register_netdev(ndev);
	if (ret) {
		dev_err(&spi->dev, "failed to register network device\n");
		goto err_netdev;
	}

	dev_info(&spi->dev, "revision %d, MAC %pM, IRQ %d\n",
		 CIDER_REV_GET(ks8851_rdreg16(ks, KS_CIDER)),
		 ndev->dev_addr, ndev->irq);

	return 0;


err_netdev:
	free_irq(ndev->irq, ndev);

err_id:
err_irq:
	free_netdev(ndev);
	return ret;
}

static int __devexit ks8851_remove(struct spi_device *spi)
{
	struct ks8851_net *priv = dev_get_drvdata(&spi->dev);

	if (netif_msg_drv(priv))
		dev_info(&spi->dev, "remove");

	unregister_netdev(priv->netdev);
	free_irq(spi->irq, priv);
	free_netdev(priv->netdev);

	return 0;
}

static struct spi_driver ks8851_driver = {
	.driver = {
		.name = "ks8851",
		.owner = THIS_MODULE,
	},
	.probe = ks8851_probe,
	.remove = __devexit_p(ks8851_remove),
};

static int __init ks8851_init(void)
{
	return spi_register_driver(&ks8851_driver);
}

static void __exit ks8851_exit(void)
{
	spi_unregister_driver(&ks8851_driver);
}

module_init(ks8851_init);
module_exit(ks8851_exit);

MODULE_DESCRIPTION("KS8851 Network driver");
MODULE_AUTHOR("Ben Dooks <ben@simtec.co.uk>");
MODULE_LICENSE("GPL");

module_param_named(message, msg_enable, int, 0);
MODULE_PARM_DESC(message, "Message verbosity level (0=none, 31=all)");
MODULE_ALIAS("spi:ks8851");
