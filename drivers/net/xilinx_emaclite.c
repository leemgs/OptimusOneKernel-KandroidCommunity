

#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/io.h>

#include <linux/of_device.h>
#include <linux/of_platform.h>

#define DRIVER_NAME "xilinx_emaclite"


#define XEL_TXBUFF_OFFSET 	0x0		
#define XEL_GIER_OFFSET		0x07F8		
#define XEL_TSR_OFFSET		0x07FC		
#define XEL_TPLR_OFFSET		0x07F4		

#define XEL_RXBUFF_OFFSET	0x1000		
#define XEL_RPLR_OFFSET		0x100C		
#define XEL_RSR_OFFSET		0x17FC		

#define XEL_BUFFER_OFFSET	0x0800		


#define XEL_GIER_GIE_MASK	0x80000000 	


#define XEL_TSR_XMIT_BUSY_MASK	 0x00000001 	
#define XEL_TSR_PROGRAM_MASK	 0x00000002 	
#define XEL_TSR_XMIT_IE_MASK	 0x00000008 	
#define XEL_TSR_XMIT_ACTIVE_MASK 0x80000000 	


#define XEL_TSR_PROG_MAC_ADDR	(XEL_TSR_XMIT_BUSY_MASK | XEL_TSR_PROGRAM_MASK)


#define XEL_RSR_RECV_DONE_MASK	0x00000001 	
#define XEL_RSR_RECV_IE_MASK	0x00000008 	


#define XEL_TPLR_LENGTH_MASK	0x0000FFFF 	


#define XEL_RPLR_LENGTH_MASK	0x0000FFFF 	

#define XEL_HEADER_OFFSET	12 		
#define XEL_HEADER_SHIFT	16 		


#define XEL_ARP_PACKET_SIZE		28 	
#define XEL_HEADER_IP_LENGTH_OFFSET	16 	



#define TX_TIMEOUT		(60*HZ)		
#define ALIGNMENT		4


#define BUFFER_ALIGN(adr) ((ALIGNMENT - ((u32) adr)) % ALIGNMENT)


struct net_local {

	struct net_device *ndev;

	bool tx_ping_pong;
	bool rx_ping_pong;
	u32 next_tx_buf_to_use;
	u32 next_rx_buf_to_use;
	void __iomem *base_addr;

	spinlock_t reset_lock;
	struct sk_buff *deferred_skb;
};







static void xemaclite_enable_interrupts(struct net_local *drvdata)
{
	u32 reg_data;

	
	reg_data = in_be32(drvdata->base_addr + XEL_TSR_OFFSET);
	out_be32(drvdata->base_addr + XEL_TSR_OFFSET,
		 reg_data | XEL_TSR_XMIT_IE_MASK);

	
	if (drvdata->tx_ping_pong != 0) {
		reg_data = in_be32(drvdata->base_addr +
				   XEL_BUFFER_OFFSET + XEL_TSR_OFFSET);
		out_be32(drvdata->base_addr + XEL_BUFFER_OFFSET +
			 XEL_TSR_OFFSET,
			 reg_data | XEL_TSR_XMIT_IE_MASK);
	}

	
	out_be32(drvdata->base_addr + XEL_RSR_OFFSET,
		 XEL_RSR_RECV_IE_MASK);

	
	if (drvdata->rx_ping_pong != 0) {
		out_be32(drvdata->base_addr + XEL_BUFFER_OFFSET +
			 XEL_RSR_OFFSET,
			 XEL_RSR_RECV_IE_MASK);
	}

	
	out_be32(drvdata->base_addr + XEL_GIER_OFFSET, XEL_GIER_GIE_MASK);
}


static void xemaclite_disable_interrupts(struct net_local *drvdata)
{
	u32 reg_data;

	
	out_be32(drvdata->base_addr + XEL_GIER_OFFSET, XEL_GIER_GIE_MASK);

	
	reg_data = in_be32(drvdata->base_addr + XEL_TSR_OFFSET);
	out_be32(drvdata->base_addr + XEL_TSR_OFFSET,
		 reg_data & (~XEL_TSR_XMIT_IE_MASK));

	
	if (drvdata->tx_ping_pong != 0) {
		reg_data = in_be32(drvdata->base_addr + XEL_BUFFER_OFFSET +
				   XEL_TSR_OFFSET);
		out_be32(drvdata->base_addr + XEL_BUFFER_OFFSET +
			 XEL_TSR_OFFSET,
			 reg_data & (~XEL_TSR_XMIT_IE_MASK));
	}

	
	reg_data = in_be32(drvdata->base_addr + XEL_RSR_OFFSET);
	out_be32(drvdata->base_addr + XEL_RSR_OFFSET,
		 reg_data & (~XEL_RSR_RECV_IE_MASK));

	
	if (drvdata->rx_ping_pong != 0) {

		reg_data = in_be32(drvdata->base_addr + XEL_BUFFER_OFFSET +
				   XEL_RSR_OFFSET);
		out_be32(drvdata->base_addr + XEL_BUFFER_OFFSET +
			 XEL_RSR_OFFSET,
			 reg_data & (~XEL_RSR_RECV_IE_MASK));
	}
}


static void xemaclite_aligned_write(void *src_ptr, u32 *dest_ptr,
				    unsigned length)
{
	u32 align_buffer;
	u32 *to_u32_ptr;
	u16 *from_u16_ptr, *to_u16_ptr;

	to_u32_ptr = dest_ptr;
	from_u16_ptr = (u16 *) src_ptr;
	align_buffer = 0;

	for (; length > 3; length -= 4) {
		to_u16_ptr = (u16 *) ((void *) &align_buffer);
		*to_u16_ptr++ = *from_u16_ptr++;
		*to_u16_ptr++ = *from_u16_ptr++;

		
		*to_u32_ptr++ = align_buffer;
	}
	if (length) {
		u8 *from_u8_ptr, *to_u8_ptr;

		
		align_buffer = 0;
		to_u8_ptr = (u8 *) &align_buffer;
		from_u8_ptr = (u8 *) from_u16_ptr;

		
		for (; length > 0; length--)
			*to_u8_ptr++ = *from_u8_ptr++;

		*to_u32_ptr = align_buffer;
	}
}


static void xemaclite_aligned_read(u32 *src_ptr, u8 *dest_ptr,
				   unsigned length)
{
	u16 *to_u16_ptr, *from_u16_ptr;
	u32 *from_u32_ptr;
	u32 align_buffer;

	from_u32_ptr = src_ptr;
	to_u16_ptr = (u16 *) dest_ptr;

	for (; length > 3; length -= 4) {
		
		align_buffer = *from_u32_ptr++;
		from_u16_ptr = (u16 *)&align_buffer;

		
		*to_u16_ptr++ = *from_u16_ptr++;
		*to_u16_ptr++ = *from_u16_ptr++;
	}

	if (length) {
		u8 *to_u8_ptr, *from_u8_ptr;

		
		to_u8_ptr = (u8 *) to_u16_ptr;
		align_buffer = *from_u32_ptr++;
		from_u8_ptr = (u8 *) &align_buffer;

		
		for (; length > 0; length--)
			*to_u8_ptr = *from_u8_ptr;
	}
}


static int xemaclite_send_data(struct net_local *drvdata, u8 *data,
			       unsigned int byte_count)
{
	u32 reg_data;
	void __iomem *addr;

	
	addr = drvdata->base_addr + drvdata->next_tx_buf_to_use;

	
	if (byte_count > ETH_FRAME_LEN)
		byte_count = ETH_FRAME_LEN;

	
	reg_data = in_be32(addr + XEL_TSR_OFFSET);
	if ((reg_data & (XEL_TSR_XMIT_BUSY_MASK |
	     XEL_TSR_XMIT_ACTIVE_MASK)) == 0) {

		
		if (drvdata->tx_ping_pong != 0)
			drvdata->next_tx_buf_to_use ^= XEL_BUFFER_OFFSET;
	} else if (drvdata->tx_ping_pong != 0) {
		

		addr = (void __iomem __force *)((u32 __force)addr ^
						 XEL_BUFFER_OFFSET);
		reg_data = in_be32(addr + XEL_TSR_OFFSET);

		if ((reg_data & (XEL_TSR_XMIT_BUSY_MASK |
		     XEL_TSR_XMIT_ACTIVE_MASK)) != 0)
			return -1; 
	} else
		return -1; 

	
	xemaclite_aligned_write(data, (u32 __force *) addr, byte_count);

	out_be32(addr + XEL_TPLR_OFFSET, (byte_count & XEL_TPLR_LENGTH_MASK));

	
	reg_data = in_be32(addr + XEL_TSR_OFFSET);
	reg_data |= (XEL_TSR_XMIT_BUSY_MASK | XEL_TSR_XMIT_ACTIVE_MASK);
	out_be32(addr + XEL_TSR_OFFSET, reg_data);

	return 0;
}


static u16 xemaclite_recv_data(struct net_local *drvdata, u8 *data)
{
	void __iomem *addr;
	u16 length, proto_type;
	u32 reg_data;

	
	addr = (drvdata->base_addr + drvdata->next_rx_buf_to_use);

	
	reg_data = in_be32(addr + XEL_RSR_OFFSET);

	if ((reg_data & XEL_RSR_RECV_DONE_MASK) == XEL_RSR_RECV_DONE_MASK) {
		if (drvdata->rx_ping_pong != 0)
			drvdata->next_rx_buf_to_use ^= XEL_BUFFER_OFFSET;
	} else {
		
		if (drvdata->rx_ping_pong != 0)
			addr = (void __iomem __force *)((u32 __force)addr ^
							 XEL_BUFFER_OFFSET);
		else
			return 0;	

		
		reg_data = in_be32(addr + XEL_RSR_OFFSET);
		if ((reg_data & XEL_RSR_RECV_DONE_MASK) !=
		     XEL_RSR_RECV_DONE_MASK)
			return 0;	
	}

	
	proto_type = ((in_be32(addr + XEL_HEADER_OFFSET +
			XEL_RXBUFF_OFFSET) >> XEL_HEADER_SHIFT) &
			XEL_RPLR_LENGTH_MASK);

	
	if (proto_type > (ETH_FRAME_LEN + ETH_FCS_LEN)) {

		if (proto_type == ETH_P_IP) {
			length = ((in_be32(addr +
					XEL_HEADER_IP_LENGTH_OFFSET +
					XEL_RXBUFF_OFFSET) >>
					XEL_HEADER_SHIFT) &
					XEL_RPLR_LENGTH_MASK);
			length += ETH_HLEN + ETH_FCS_LEN;

		} else if (proto_type == ETH_P_ARP)
			length = XEL_ARP_PACKET_SIZE + ETH_HLEN + ETH_FCS_LEN;
		else
			
			length = ETH_FRAME_LEN + ETH_FCS_LEN;
	} else
		
		length = proto_type + ETH_HLEN + ETH_FCS_LEN;

	
	xemaclite_aligned_read((u32 __force *) (addr + XEL_RXBUFF_OFFSET),
				data, length);

	
	reg_data = in_be32(addr + XEL_RSR_OFFSET);
	reg_data &= ~XEL_RSR_RECV_DONE_MASK;
	out_be32(addr + XEL_RSR_OFFSET, reg_data);

	return length;
}


static void xemaclite_set_mac_address(struct net_local *drvdata,
				      u8 *address_ptr)
{
	void __iomem *addr;
	u32 reg_data;

	
	addr = drvdata->base_addr + drvdata->next_tx_buf_to_use;

	xemaclite_aligned_write(address_ptr, (u32 __force *) addr, ETH_ALEN);

	out_be32(addr + XEL_TPLR_OFFSET, ETH_ALEN);

	
	reg_data = in_be32(addr + XEL_TSR_OFFSET);
	out_be32(addr + XEL_TSR_OFFSET, reg_data | XEL_TSR_PROG_MAC_ADDR);

	
	while ((in_be32(addr + XEL_TSR_OFFSET) &
		XEL_TSR_PROG_MAC_ADDR) != 0)
		;
}


static void xemaclite_tx_timeout(struct net_device *dev)
{
	struct net_local *lp = (struct net_local *) netdev_priv(dev);
	unsigned long flags;

	dev_err(&lp->ndev->dev, "Exceeded transmit timeout of %lu ms\n",
		TX_TIMEOUT * 1000UL / HZ);

	dev->stats.tx_errors++;

	
	spin_lock_irqsave(&lp->reset_lock, flags);

	
	netif_stop_queue(dev);

	xemaclite_disable_interrupts(lp);
	xemaclite_enable_interrupts(lp);

	if (lp->deferred_skb) {
		dev_kfree_skb(lp->deferred_skb);
		lp->deferred_skb = NULL;
		dev->stats.tx_errors++;
	}

	
	dev->trans_start = 0xffffffff - TX_TIMEOUT - TX_TIMEOUT;

	
	netif_wake_queue(dev);
	spin_unlock_irqrestore(&lp->reset_lock, flags);
}






static void xemaclite_tx_handler(struct net_device *dev)
{
	struct net_local *lp = (struct net_local *) netdev_priv(dev);

	dev->stats.tx_packets++;
	if (lp->deferred_skb) {
		if (xemaclite_send_data(lp,
					(u8 *) lp->deferred_skb->data,
					lp->deferred_skb->len) != 0)
			return;
		else {
			dev->stats.tx_bytes += lp->deferred_skb->len;
			dev_kfree_skb_irq(lp->deferred_skb);
			lp->deferred_skb = NULL;
			dev->trans_start = jiffies;
			netif_wake_queue(dev);
		}
	}
}


static void xemaclite_rx_handler(struct net_device *dev)
{
	struct net_local *lp = (struct net_local *) netdev_priv(dev);
	struct sk_buff *skb;
	unsigned int align;
	u32 len;

	len = ETH_FRAME_LEN + ETH_FCS_LEN;
	skb = dev_alloc_skb(len + ALIGNMENT);
	if (!skb) {
		
		dev->stats.rx_dropped++;
		dev_err(&lp->ndev->dev, "Could not allocate receive buffer\n");
		return;
	}

	
	align = BUFFER_ALIGN(skb->data);
	if (align)
		skb_reserve(skb, align);

	skb_reserve(skb, 2);

	len = xemaclite_recv_data(lp, (u8 *) skb->data);

	if (!len) {
		dev->stats.rx_errors++;
		dev_kfree_skb_irq(skb);
		return;
	}

	skb_put(skb, len);	
	skb->dev = dev;		

	skb->protocol = eth_type_trans(skb, dev);
	skb->ip_summed = CHECKSUM_NONE;

	dev->stats.rx_packets++;
	dev->stats.rx_bytes += len;

	netif_rx(skb);		
}


static irqreturn_t xemaclite_interrupt(int irq, void *dev_id)
{
	bool tx_complete = 0;
	struct net_device *dev = dev_id;
	struct net_local *lp = (struct net_local *) netdev_priv(dev);
	void __iomem *base_addr = lp->base_addr;
	u32 tx_status;

	
	if ((in_be32(base_addr + XEL_RSR_OFFSET) & XEL_RSR_RECV_DONE_MASK) ||
			(in_be32(base_addr + XEL_BUFFER_OFFSET + XEL_RSR_OFFSET)
			 & XEL_RSR_RECV_DONE_MASK))

		xemaclite_rx_handler(dev);

	
	tx_status = in_be32(base_addr + XEL_TSR_OFFSET);
	if (((tx_status & XEL_TSR_XMIT_BUSY_MASK) == 0) &&
		(tx_status & XEL_TSR_XMIT_ACTIVE_MASK) != 0) {

		tx_status &= ~XEL_TSR_XMIT_ACTIVE_MASK;
		out_be32(base_addr + XEL_TSR_OFFSET, tx_status);

		tx_complete = 1;
	}

	
	tx_status = in_be32(base_addr + XEL_BUFFER_OFFSET + XEL_TSR_OFFSET);
	if (((tx_status & XEL_TSR_XMIT_BUSY_MASK) == 0) &&
		(tx_status & XEL_TSR_XMIT_ACTIVE_MASK) != 0) {

		tx_status &= ~XEL_TSR_XMIT_ACTIVE_MASK;
		out_be32(base_addr + XEL_BUFFER_OFFSET + XEL_TSR_OFFSET,
			 tx_status);

		tx_complete = 1;
	}

	
	if (tx_complete != 0)
		xemaclite_tx_handler(dev);

	return IRQ_HANDLED;
}


static int xemaclite_open(struct net_device *dev)
{
	struct net_local *lp = (struct net_local *) netdev_priv(dev);
	int retval;

	
	xemaclite_disable_interrupts(lp);

	
	xemaclite_set_mac_address(lp, dev->dev_addr);

	
	retval = request_irq(dev->irq, &xemaclite_interrupt, 0, dev->name, dev);
	if (retval) {
		dev_err(&lp->ndev->dev, "Could not allocate interrupt %d\n",
			dev->irq);
		return retval;
	}

	
	xemaclite_enable_interrupts(lp);

	
	netif_start_queue(dev);

	return 0;
}


static int xemaclite_close(struct net_device *dev)
{
	struct net_local *lp = (struct net_local *) netdev_priv(dev);

	netif_stop_queue(dev);
	xemaclite_disable_interrupts(lp);
	free_irq(dev->irq, dev);

	return 0;
}


static struct net_device_stats *xemaclite_get_stats(struct net_device *dev)
{
	return &dev->stats;
}


static int xemaclite_send(struct sk_buff *orig_skb, struct net_device *dev)
{
	struct net_local *lp = (struct net_local *) netdev_priv(dev);
	struct sk_buff *new_skb;
	unsigned int len;
	unsigned long flags;

	len = orig_skb->len;

	new_skb = orig_skb;

	spin_lock_irqsave(&lp->reset_lock, flags);
	if (xemaclite_send_data(lp, (u8 *) new_skb->data, len) != 0) {
		
		netif_stop_queue(dev);
		lp->deferred_skb = new_skb;
		spin_unlock_irqrestore(&lp->reset_lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(&lp->reset_lock, flags);

	dev->stats.tx_bytes += len;
	dev_kfree_skb(new_skb);
	dev->trans_start = jiffies;

	return 0;
}


static int xemaclite_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct net_local *lp = (struct net_local *) netdev_priv(dev);
	struct hw_addr_data *hw_addr = (struct hw_addr_data *) &rq->ifr_hwaddr;

	switch (cmd) {
	case SIOCETHTOOL:
		return -EIO;

	case SIOCSIFHWADDR:
		dev_err(&lp->ndev->dev, "SIOCSIFHWADDR\n");

		
		copy_from_user((void __force *) dev->dev_addr,
			       (void __user __force *) hw_addr,
			       IFHWADDRLEN);
		xemaclite_set_mac_address(lp, dev->dev_addr);
		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}


static void xemaclite_remove_ndev(struct net_device *ndev)
{
	if (ndev) {
		struct net_local *lp = (struct net_local *) netdev_priv(ndev);

		if (lp->base_addr)
			iounmap((void __iomem __force *) (lp->base_addr));
		free_netdev(ndev);
	}
}


static bool get_bool(struct of_device *ofdev, const char *s)
{
	u32 *p = (u32 *)of_get_property(ofdev->node, s, NULL);

	if (p) {
		return (bool)*p;
	} else {
		dev_warn(&ofdev->dev, "Parameter %s not found,"
			"defaulting to false\n", s);
		return 0;
	}
}

static struct net_device_ops xemaclite_netdev_ops;


static int __devinit xemaclite_of_probe(struct of_device *ofdev,
					const struct of_device_id *match)
{
	struct resource r_irq; 
	struct resource r_mem; 
	struct net_device *ndev = NULL;
	struct net_local *lp = NULL;
	struct device *dev = &ofdev->dev;
	const void *mac_address;

	int rc = 0;

	dev_info(dev, "Device Tree Probing\n");

	
	rc = of_address_to_resource(ofdev->node, 0, &r_mem);
	if (rc) {
		dev_err(dev, "invalid address\n");
		return rc;
	}

	
	rc = of_irq_to_resource(ofdev->node, 0, &r_irq);
	if (rc == NO_IRQ) {
		dev_err(dev, "no IRQ found\n");
		return rc;
	}

	
	ndev = alloc_etherdev(sizeof(struct net_local));
	if (!ndev) {
		dev_err(dev, "Could not allocate network device\n");
		return -ENOMEM;
	}

	dev_set_drvdata(dev, ndev);

	ndev->irq = r_irq.start;
	ndev->mem_start = r_mem.start;
	ndev->mem_end = r_mem.end;

	lp = netdev_priv(ndev);
	lp->ndev = ndev;

	if (!request_mem_region(ndev->mem_start,
				ndev->mem_end - ndev->mem_start + 1,
				DRIVER_NAME)) {
		dev_err(dev, "Couldn't lock memory region at %p\n",
			(void *)ndev->mem_start);
		rc = -EBUSY;
		goto error2;
	}

	
	lp->base_addr = ioremap(r_mem.start, r_mem.end - r_mem.start + 1);
	if (NULL == lp->base_addr) {
		dev_err(dev, "EmacLite: Could not allocate iomem\n");
		rc = -EIO;
		goto error1;
	}

	spin_lock_init(&lp->reset_lock);
	lp->next_tx_buf_to_use = 0x0;
	lp->next_rx_buf_to_use = 0x0;
	lp->tx_ping_pong = get_bool(ofdev, "xlnx,tx-ping-pong");
	lp->rx_ping_pong = get_bool(ofdev, "xlnx,rx-ping-pong");
	mac_address = of_get_mac_address(ofdev->node);

	if (mac_address)
		
		memcpy(ndev->dev_addr, mac_address, 6);
	else
		dev_warn(dev, "No MAC address found\n");

	
	out_be32(lp->base_addr + XEL_TSR_OFFSET, 0);
	out_be32(lp->base_addr + XEL_BUFFER_OFFSET + XEL_TSR_OFFSET, 0);

	
	xemaclite_set_mac_address(lp, ndev->dev_addr);

	dev_info(dev,
		 "MAC address is now %2x:%2x:%2x:%2x:%2x:%2x\n",
		 ndev->dev_addr[0], ndev->dev_addr[1],
		 ndev->dev_addr[2], ndev->dev_addr[3],
		 ndev->dev_addr[4], ndev->dev_addr[5]);

	ndev->netdev_ops = &xemaclite_netdev_ops;
	ndev->flags &= ~IFF_MULTICAST;
	ndev->watchdog_timeo = TX_TIMEOUT;

	
	rc = register_netdev(ndev);
	if (rc) {
		dev_err(dev,
			"Cannot register network device, aborting\n");
		goto error1;
	}

	dev_info(dev,
		 "Xilinx EmacLite at 0x%08X mapped to 0x%08X, irq=%d\n",
		 (unsigned int __force)ndev->mem_start,
		 (unsigned int __force)lp->base_addr, ndev->irq);
	return 0;

error1:
	release_mem_region(ndev->mem_start, r_mem.end - r_mem.start + 1);

error2:
	xemaclite_remove_ndev(ndev);
	return rc;
}


static int __devexit xemaclite_of_remove(struct of_device *of_dev)
{
	struct device *dev = &of_dev->dev;
	struct net_device *ndev = dev_get_drvdata(dev);

	unregister_netdev(ndev);

	release_mem_region(ndev->mem_start, ndev->mem_end-ndev->mem_start + 1);

	xemaclite_remove_ndev(ndev);

	dev_set_drvdata(dev, NULL);

	return 0;
}

static struct net_device_ops xemaclite_netdev_ops = {
	.ndo_open		= xemaclite_open,
	.ndo_stop		= xemaclite_close,
	.ndo_start_xmit		= xemaclite_send,
	.ndo_do_ioctl		= xemaclite_ioctl,
	.ndo_tx_timeout		= xemaclite_tx_timeout,
	.ndo_get_stats		= xemaclite_get_stats,
};


static struct of_device_id xemaclite_of_match[] __devinitdata = {
	{ .compatible = "xlnx,opb-ethernetlite-1.01.a", },
	{ .compatible = "xlnx,opb-ethernetlite-1.01.b", },
	{ .compatible = "xlnx,xps-ethernetlite-1.00.a", },
	{ .compatible = "xlnx,xps-ethernetlite-2.00.a", },
	{ .compatible = "xlnx,xps-ethernetlite-2.01.a", },
	{  },
};
MODULE_DEVICE_TABLE(of, xemaclite_of_match);

static struct of_platform_driver xemaclite_of_driver = {
	.name		= DRIVER_NAME,
	.match_table	= xemaclite_of_match,
	.probe		= xemaclite_of_probe,
	.remove		= __devexit_p(xemaclite_of_remove),
};


static int __init xemaclite_init(void)
{
	
	return of_register_platform_driver(&xemaclite_of_driver);
}


static void __exit xemaclite_cleanup(void)
{
	of_unregister_platform_driver(&xemaclite_of_driver);
}

module_init(xemaclite_init);
module_exit(xemaclite_cleanup);

MODULE_AUTHOR("Xilinx, Inc.");
MODULE_DESCRIPTION("Xilinx Ethernet MAC Lite driver");
MODULE_LICENSE("GPL");
