

static const char *version =
	"isa-skeleton.c:v1.51 9/24/94 Donald Becker (becker@cesdis.gsfc.nasa.gov)\n";



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/bitops.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/dma.h>


static const char* cardname = "netcard";




static unsigned int netcard_portlist[] __initdata =
   { 0x200, 0x240, 0x280, 0x2C0, 0x300, 0x320, 0x340, 0};


#ifndef NET_DEBUG
#define NET_DEBUG 2
#endif
static unsigned int net_debug = NET_DEBUG;


#define NETCARD_IO_EXTENT	32

#define MY_TX_TIMEOUT  ((400*HZ)/1000)


struct net_local {
	struct net_device_stats stats;
	long open_time;			

	
	spinlock_t lock;
};


#define SA_ADDR0 0x00
#define SA_ADDR1 0x42
#define SA_ADDR2 0x65



static int	netcard_probe1(struct net_device *dev, int ioaddr);
static int	net_open(struct net_device *dev);
static int	net_send_packet(struct sk_buff *skb, struct net_device *dev);
static irqreturn_t net_interrupt(int irq, void *dev_id);
static void	net_rx(struct net_device *dev);
static int	net_close(struct net_device *dev);
static struct	net_device_stats *net_get_stats(struct net_device *dev);
static void	set_multicast_list(struct net_device *dev);
static void     net_tx_timeout(struct net_device *dev);



#define tx_done(dev) 1
static void	hardware_send_packet(short ioaddr, char *buf, int length);
static void 	chipset_init(struct net_device *dev, int startp);


static int __init do_netcard_probe(struct net_device *dev)
{
	int i;
	int base_addr = dev->base_addr;
	int irq = dev->irq;

	if (base_addr > 0x1ff)    
		return netcard_probe1(dev, base_addr);
	else if (base_addr != 0)  
		return -ENXIO;

	for (i = 0; netcard_portlist[i]; i++) {
		int ioaddr = netcard_portlist[i];
		if (netcard_probe1(dev, ioaddr) == 0)
			return 0;
		dev->irq = irq;
	}

	return -ENODEV;
}

static void cleanup_card(struct net_device *dev)
{
#ifdef jumpered_dma
	free_dma(dev->dma);
#endif
#ifdef jumpered_interrupts
	free_irq(dev->irq, dev);
#endif
	release_region(dev->base_addr, NETCARD_IO_EXTENT);
}

#ifndef MODULE
struct net_device * __init netcard_probe(int unit)
{
	struct net_device *dev = alloc_etherdev(sizeof(struct net_local));
	int err;

	if (!dev)
		return ERR_PTR(-ENOMEM);

	sprintf(dev->name, "eth%d", unit);
	netdev_boot_setup_check(dev);

	err = do_netcard_probe(dev);
	if (err)
		goto out;
	return dev;
out:
	free_netdev(dev);
	return ERR_PTR(err);
}
#endif

static const struct net_device_ops netcard_netdev_ops = {
	.ndo_open		= net_open,
	.ndo_stop		= net_close,
	.ndo_start_xmit		= net_send_packet,
	.ndo_get_stats		= net_get_stats,
	.ndo_set_multicast_list	= set_multicast_list,
	.ndo_tx_timeout		= net_tx_timeout,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_change_mtu		= eth_change_mtu,
};


static int __init netcard_probe1(struct net_device *dev, int ioaddr)
{
	struct net_local *np;
	static unsigned version_printed;
	int i;
	int err = -ENODEV;

	
	if (!request_region(ioaddr, NETCARD_IO_EXTENT, cardname))
		return -EBUSY;

	
	if (inb(ioaddr + 0) != SA_ADDR0
		||	 inb(ioaddr + 1) != SA_ADDR1
		||	 inb(ioaddr + 2) != SA_ADDR2)
		goto out;

	if (net_debug  &&  version_printed++ == 0)
		printk(KERN_DEBUG "%s", version);

	printk(KERN_INFO "%s: %s found at %#3x, ", dev->name, cardname, ioaddr);

	
	dev->base_addr = ioaddr;

	
	for (i = 0; i < 6; i++)
		dev->dev_addr[i] = inb(ioaddr + i);

	printk("%pM", dev->dev_addr);

	err = -EAGAIN;
#ifdef jumpered_interrupts
	

	if (dev->irq == -1)
		;	
	else if (dev->irq < 2) {	
		unsigned long irq_mask = probe_irq_on();
		

		dev->irq = probe_irq_off(irq_mask);
		if (net_debug >= 2)
			printk(" autoirq is %d", dev->irq);
	} else if (dev->irq == 2)
		
		dev->irq = 9;

	{
		int irqval = request_irq(dev->irq, &net_interrupt, 0, cardname, dev);
		if (irqval) {
			printk("%s: unable to get IRQ %d (irqval=%d).\n",
				   dev->name, dev->irq, irqval);
			goto out;
		}
	}
#endif	
#ifdef jumpered_dma
	
	if (dev->dma == 0) {
		if (request_dma(dev->dma, cardname)) {
			printk("DMA %d allocation failed.\n", dev->dma);
			goto out1;
		} else
			printk(", assigned DMA %d.\n", dev->dma);
	} else {
		short dma_status, new_dma_status;

		
		dma_status = ((inb(DMA1_STAT_REG) >> 4) & 0x0f) |
			(inb(DMA2_STAT_REG) & 0xf0);
		
		outw(0x1234, ioaddr + 8);
		
		new_dma_status = ((inb(DMA1_STAT_REG) >> 4) & 0x0f) |
			(inb(DMA2_STAT_REG) & 0xf0);
		
		new_dma_status ^= dma_status;
		new_dma_status &= ~0x10;
		for (i = 7; i > 0; i--)
			if (test_bit(i, &new_dma_status)) {
				dev->dma = i;
				break;
			}
		if (i <= 0) {
			printk("DMA probe failed.\n");
			goto out1;
		}
		if (request_dma(dev->dma, cardname)) {
			printk("probed DMA %d allocation failed.\n", dev->dma);
			goto out1;
		}
	}
#endif	

	np = netdev_priv(dev);
	spin_lock_init(&np->lock);

        dev->netdev_ops		= &netcard_netdev_ops;
        dev->watchdog_timeo	= MY_TX_TIMEOUT;

	err = register_netdev(dev);
	if (err)
		goto out2;
	return 0;
out2:
#ifdef jumpered_dma
	free_dma(dev->dma);
#endif
out1:
#ifdef jumpered_interrupts
	free_irq(dev->irq, dev);
#endif
out:
	release_region(base_addr, NETCARD_IO_EXTENT);
	return err;
}

static void net_tx_timeout(struct net_device *dev)
{
	struct net_local *np = netdev_priv(dev);

	printk(KERN_WARNING "%s: transmit timed out, %s?\n", dev->name,
	       tx_done(dev) ? "IRQ conflict" : "network cable problem");

	
	chipset_init(dev, 1);

	np->stats.tx_errors++;

	
	if (!tx_full(dev))
		netif_wake_queue(dev);
}


static int
net_open(struct net_device *dev)
{
	struct net_local *np = netdev_priv(dev);
	int ioaddr = dev->base_addr;
	
	if (request_irq(dev->irq, &net_interrupt, 0, cardname, dev)) {
		return -EAGAIN;
	}
	
	if (request_dma(dev->dma, cardname)) {
		free_irq(dev->irq, dev);
		return -EAGAIN;
	}

	
	chipset_init(dev, 1);
	outb(0x00, ioaddr);
	np->open_time = jiffies;

	
	netif_start_queue(dev);

	return 0;
}


static int net_send_packet(struct sk_buff *skb, struct net_device *dev)
{
	struct net_local *np = netdev_priv(dev);
	int ioaddr = dev->base_addr;
	short length = ETH_ZLEN < skb->len ? skb->len : ETH_ZLEN;
	unsigned char *buf = skb->data;

	

#if TX_RING
	
	unsigned long flags;
	spin_lock_irqsave(&np->lock, flags);

	add_to_tx_ring(np, skb, length);
	dev->trans_start = jiffies;

	
	if (tx_full(dev))
		netif_stop_queue(dev);

	

	spin_unlock_irqrestore(&np->lock, flags);
#else
	
	hardware_send_packet(ioaddr, buf, length);
	np->stats.tx_bytes += skb->len;

	dev->trans_start = jiffies;

	
	if (inw(ioaddr) == 81)
		np->stats.tx_aborted_errors++;
	dev_kfree_skb (skb);
#endif

	return NETDEV_TX_OK;
}

#if TX_RING

void net_tx(struct net_device *dev)
{
	struct net_local *np = netdev_priv(dev);
	int entry;

	
	spin_lock(&np->lock);

	entry = np->tx_old;
	while (tx_entry_is_sent(np, entry)) {
		struct sk_buff *skb = np->skbs[entry];

		np->stats.tx_bytes += skb->len;
		dev_kfree_skb_irq (skb);

		entry = next_tx_entry(np, entry);
	}
	np->tx_old = entry;

	
	if (netif_queue_stopped(dev) && ! tx_full(dev))
		netif_wake_queue(dev);

	spin_unlock(&np->lock);
}
#endif


static irqreturn_t net_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct net_local *np;
	int ioaddr, status;
	int handled = 0;

	ioaddr = dev->base_addr;

	np = netdev_priv(dev);
	status = inw(ioaddr + 0);

	if (status == 0)
		goto out;
	handled = 1;

	if (status & RX_INTR) {
		
		net_rx(dev);
	}
#if TX_RING
	if (status & TX_INTR) {
		
		net_tx(dev);
		np->stats.tx_packets++;
		netif_wake_queue(dev);
	}
#endif
	if (status & COUNTERS_INTR) {
		
		np->stats.tx_window_errors++;
	}
out:
	return IRQ_RETVAL(handled);
}


static void
net_rx(struct net_device *dev)
{
	struct net_local *lp = netdev_priv(dev);
	int ioaddr = dev->base_addr;
	int boguscount = 10;

	do {
		int status = inw(ioaddr);
		int pkt_len = inw(ioaddr);

		if (pkt_len == 0)		
			break;			

		if (status & 0x40) {	
			lp->stats.rx_errors++;
			if (status & 0x20) lp->stats.rx_frame_errors++;
			if (status & 0x10) lp->stats.rx_over_errors++;
			if (status & 0x08) lp->stats.rx_crc_errors++;
			if (status & 0x04) lp->stats.rx_fifo_errors++;
		} else {
			
			struct sk_buff *skb;

			lp->stats.rx_bytes+=pkt_len;

			skb = dev_alloc_skb(pkt_len);
			if (skb == NULL) {
				printk(KERN_NOTICE "%s: Memory squeeze, dropping packet.\n",
					   dev->name);
				lp->stats.rx_dropped++;
				break;
			}
			skb->dev = dev;

			
			memcpy(skb_put(skb,pkt_len), (void*)dev->rmem_start,
				   pkt_len);
			
			insw(ioaddr, skb->data, (pkt_len + 1) >> 1);

			netif_rx(skb);
			lp->stats.rx_packets++;
			lp->stats.rx_bytes += pkt_len;
		}
	} while (--boguscount);

	return;
}


static int
net_close(struct net_device *dev)
{
	struct net_local *lp = netdev_priv(dev);
	int ioaddr = dev->base_addr;

	lp->open_time = 0;

	netif_stop_queue(dev);

	

	disable_dma(dev->dma);

	
	outw(0x00, ioaddr+0);	

	free_irq(dev->irq, dev);
	free_dma(dev->dma);

	

	return 0;

}


static struct net_device_stats *net_get_stats(struct net_device *dev)
{
	struct net_local *lp = netdev_priv(dev);
	short ioaddr = dev->base_addr;

	
	lp->stats.rx_missed_errors = inw(ioaddr+1);
	return &lp->stats;
}


static void
set_multicast_list(struct net_device *dev)
{
	short ioaddr = dev->base_addr;
	if (dev->flags&IFF_PROMISC)
	{
		
		outw(MULTICAST|PROMISC, ioaddr);
	}
	else if((dev->flags&IFF_ALLMULTI) || dev->mc_count > HW_MAX_ADDRS)
	{
		
		hardware_set_filter(NULL);

		outw(MULTICAST, ioaddr);
	}
	else if(dev->mc_count)
	{
		
		hardware_set_filter(dev->mc_list);

		outw(MULTICAST, ioaddr);
	}
	else
		outw(0, ioaddr);
}

#ifdef MODULE

static struct net_device *this_device;
static int io = 0x300;
static int irq;
static int dma;
static int mem;
MODULE_LICENSE("GPL");

int init_module(void)
{
	struct net_device *dev;
	int result;

	if (io == 0)
		printk(KERN_WARNING "%s: You shouldn't use auto-probing with insmod!\n",
			   cardname);
	dev = alloc_etherdev(sizeof(struct net_local));
	if (!dev)
		return -ENOMEM;

	
	dev->base_addr = io;
	dev->irq       = irq;
	dev->dma       = dma;
	dev->mem_start = mem;
	if (do_netcard_probe(dev) == 0) {
		this_device = dev;
		return 0;
	}
	free_netdev(dev);
	return -ENXIO;
}

void
cleanup_module(void)
{
	unregister_netdev(this_device);
	cleanup_card(this_device);
	free_netdev(this_device);
}

#endif 
