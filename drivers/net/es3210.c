

static const char version[] =
	"es3210.c: Driver revision v0.03, 14/09/96\n";

#include <linux/module.h>
#include <linux/eisa.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include <asm/io.h>
#include <asm/system.h>

#include "8390.h"

static int es_probe1(struct net_device *dev, int ioaddr);

static void es_reset_8390(struct net_device *dev);

static void es_get_8390_hdr(struct net_device *dev, struct e8390_pkt_hdr *hdr, int ring_page);
static void es_block_input(struct net_device *dev, int count, struct sk_buff *skb, int ring_offset);
static void es_block_output(struct net_device *dev, int count, const unsigned char *buf, int start_page);

#define ES_START_PG	0x00    
#define ES_STOP_PG	0x40    

#define ES_IO_EXTENT	0x37	
#define ES_ID_PORT	0xc80	
#define ES_SA_PROM	0xc90	
#define ES_RESET_PORT	0xc84	
#define ES_NIC_OFFSET	0xca0	

#define ES_ADDR0	0x02	
#define ES_ADDR1	0x07
#define ES_ADDR2	0x01


#define ES_EISA_ID1	0x01012949	
#define ES_EISA_ID2	0x02012949	

#define ES_CFG1		0xcc0	
#define ES_CFG2		0xcc1
#define ES_CFG3		0xcc2
#define ES_CFG4		0xcc3
#define ES_CFG5		0xcc4
#define ES_CFG6		0xc84	



#define ES_D_PROBE	0x01
#define ES_D_RX_PKT	0x02
#define ES_D_TX_PKT	0x04
#define ED_D_IRQ	0x08

#define ES_DEBUG	0

static unsigned char lo_irq_map[] __initdata = {3, 4, 5, 6, 7, 9, 10};
static unsigned char hi_irq_map[] __initdata = {11, 12, 0, 14, 0, 0, 0, 15};



static int __init do_es_probe(struct net_device *dev)
{
	unsigned short ioaddr = dev->base_addr;
	int irq = dev->irq;
	int mem_start = dev->mem_start;

	if (ioaddr > 0x1ff)		
		return es_probe1(dev, ioaddr);
	else if (ioaddr > 0)		
		return -ENXIO;

	if (!EISA_bus) {
#if ES_DEBUG & ES_D_PROBE
		printk("es3210.c: Not EISA bus. Not probing high ports.\n");
#endif
		return -ENXIO;
	}

	
	for (ioaddr = 0x1000; ioaddr < 0x9000; ioaddr += 0x1000) {
		if (es_probe1(dev, ioaddr) == 0)
			return 0;
		dev->irq = irq;
		dev->mem_start = mem_start;
	}

	return -ENODEV;
}

#ifndef MODULE
struct net_device * __init es_probe(int unit)
{
	struct net_device *dev = alloc_ei_netdev();
	int err;

	if (!dev)
		return ERR_PTR(-ENOMEM);

	sprintf(dev->name, "eth%d", unit);
	netdev_boot_setup_check(dev);

	err = do_es_probe(dev);
	if (err)
		goto out;
	return dev;
out:
	free_netdev(dev);
	return ERR_PTR(err);
}
#endif

static int __init es_probe1(struct net_device *dev, int ioaddr)
{
	int i, retval;
	unsigned long eisa_id;

	if (!request_region(ioaddr + ES_SA_PROM, ES_IO_EXTENT, "es3210"))
		return -ENODEV;

#if ES_DEBUG & ES_D_PROBE
	printk("es3210.c: probe at %#x, ID %#8x\n", ioaddr, inl(ioaddr + ES_ID_PORT));
	printk("es3210.c: config regs: %#x %#x %#x %#x %#x %#x\n",
		inb(ioaddr + ES_CFG1), inb(ioaddr + ES_CFG2), inb(ioaddr + ES_CFG3),
		inb(ioaddr + ES_CFG4), inb(ioaddr + ES_CFG5), inb(ioaddr + ES_CFG6));
#endif


	eisa_id = inl(ioaddr + ES_ID_PORT);
	if ((eisa_id != ES_EISA_ID1) && (eisa_id != ES_EISA_ID2)) {
		retval = -ENODEV;
		goto out;
	}

	for (i = 0; i < ETHER_ADDR_LEN ; i++)
		dev->dev_addr[i] = inb(ioaddr + ES_SA_PROM + i);


	if (dev->dev_addr[0] != ES_ADDR0 ||
	    dev->dev_addr[1] != ES_ADDR1 ||
	    dev->dev_addr[2] != ES_ADDR2) {
		printk("es3210.c: card not found %pM (invalid_prefix).\n",
		       dev->dev_addr);
		retval = -ENODEV;
		goto out;
	}

	printk("es3210.c: ES3210 rev. %ld at %#x, node %pM",
	       eisa_id>>24, ioaddr, dev->dev_addr);

	
	if (dev->irq == 0) {
		unsigned char hi_irq = inb(ioaddr + ES_CFG2) & 0x07;
		unsigned char lo_irq = inb(ioaddr + ES_CFG1) & 0xfe;

		if (hi_irq != 0) {
			dev->irq = hi_irq_map[hi_irq - 1];
		} else {
			int i = 0;
			while (lo_irq > (1<<i)) i++;
			dev->irq = lo_irq_map[i];
		}
		printk(" using IRQ %d", dev->irq);
#if ES_DEBUG & ES_D_PROBE
		printk("es3210.c: hi_irq %#x, lo_irq %#x, dev->irq = %d\n",
					hi_irq, lo_irq, dev->irq);
#endif
	} else {
		if (dev->irq == 2)
			dev->irq = 9;			
		printk(" assigning IRQ %d", dev->irq);
	}

	if (request_irq(dev->irq, ei_interrupt, 0, "es3210", dev)) {
		printk (" unable to get IRQ %d.\n", dev->irq);
		retval = -EAGAIN;
		goto out;
	}

	if (dev->mem_start == 0) {
		unsigned char mem_enabled = inb(ioaddr + ES_CFG2) & 0xc0;
		unsigned char mem_bits = inb(ioaddr + ES_CFG3) & 0x07;

		if (mem_enabled != 0x80) {
			printk(" shared mem disabled - giving up\n");
			retval = -ENXIO;
			goto out1;
		}
		dev->mem_start = 0xC0000 + mem_bits*0x4000;
		printk(" using ");
	} else {
		printk(" assigning ");
	}

	ei_status.mem = ioremap(dev->mem_start, (ES_STOP_PG - ES_START_PG)*256);
	if (!ei_status.mem) {
		printk("ioremap failed - giving up\n");
		retval = -ENXIO;
		goto out1;
	}

	dev->mem_end = dev->mem_start + (ES_STOP_PG - ES_START_PG)*256;

	printk("mem %#lx-%#lx\n", dev->mem_start, dev->mem_end-1);

#if ES_DEBUG & ES_D_PROBE
	if (inb(ioaddr + ES_CFG5))
		printk("es3210: Warning - DMA channel enabled, but not used here.\n");
#endif
	
	dev->base_addr = ioaddr + ES_NIC_OFFSET;

	ei_status.name = "ES3210";
	ei_status.tx_start_page = ES_START_PG;
	ei_status.rx_start_page = ES_START_PG + TX_PAGES;
	ei_status.stop_page = ES_STOP_PG;
	ei_status.word16 = 1;

	if (ei_debug > 0)
		printk(version);

	ei_status.reset_8390 = &es_reset_8390;
	ei_status.block_input = &es_block_input;
	ei_status.block_output = &es_block_output;
	ei_status.get_8390_hdr = &es_get_8390_hdr;

	dev->netdev_ops = &ei_netdev_ops;
	NS8390_init(dev, 0);

	retval = register_netdev(dev);
	if (retval)
		goto out1;
	return 0;
out1:
	free_irq(dev->irq, dev);
out:
	release_region(ioaddr + ES_SA_PROM, ES_IO_EXTENT);
	return retval;
}



static void es_reset_8390(struct net_device *dev)
{
	unsigned short ioaddr = dev->base_addr;
	unsigned long end;

	outb(0x04, ioaddr + ES_RESET_PORT);
	if (ei_debug > 1) printk("%s: resetting the ES3210...", dev->name);

	end = jiffies + 2*HZ/100;
        while ((signed)(end - jiffies) > 0) continue;

	ei_status.txing = 0;
	outb(0x01, ioaddr + ES_RESET_PORT);
	if (ei_debug > 1) printk("reset done\n");

	return;
}





static void
es_get_8390_hdr(struct net_device *dev, struct e8390_pkt_hdr *hdr, int ring_page)
{
	void __iomem *hdr_start = ei_status.mem + ((ring_page - ES_START_PG)<<8);
	memcpy_fromio(hdr, hdr_start, sizeof(struct e8390_pkt_hdr));
	hdr->count = (hdr->count + 3) & ~3;     
}



static void es_block_input(struct net_device *dev, int count, struct sk_buff *skb,
						  int ring_offset)
{
	void __iomem *xfer_start = ei_status.mem + ring_offset - ES_START_PG*256;

	if (ring_offset + count > ES_STOP_PG*256) {
		
		int semi_count = ES_STOP_PG*256 - ring_offset;
		memcpy_fromio(skb->data, xfer_start, semi_count);
		count -= semi_count;
		memcpy_fromio(skb->data + semi_count, ei_status.mem, count);
	} else {
		
		memcpy_fromio(skb->data, xfer_start, count);
	}
}

static void es_block_output(struct net_device *dev, int count,
				const unsigned char *buf, int start_page)
{
	void __iomem *shmem = ei_status.mem + ((start_page - ES_START_PG)<<8);

	count = (count + 3) & ~3;     
	memcpy_toio(shmem, buf, count);
}

#ifdef MODULE
#define MAX_ES_CARDS	4	
#define NAMELEN		8	
static struct net_device *dev_es3210[MAX_ES_CARDS];
static int io[MAX_ES_CARDS];
static int irq[MAX_ES_CARDS];
static int mem[MAX_ES_CARDS];

module_param_array(io, int, NULL, 0);
module_param_array(irq, int, NULL, 0);
module_param_array(mem, int, NULL, 0);
MODULE_PARM_DESC(io, "I/O base address(es)");
MODULE_PARM_DESC(irq, "IRQ number(s)");
MODULE_PARM_DESC(mem, "memory base address(es)");
MODULE_DESCRIPTION("Racal-Interlan ES3210 EISA ethernet driver");
MODULE_LICENSE("GPL");

int __init init_module(void)
{
	struct net_device *dev;
	int this_dev, found = 0;

	for (this_dev = 0; this_dev < MAX_ES_CARDS; this_dev++) {
		if (io[this_dev] == 0 && this_dev != 0)
			break;
		dev = alloc_ei_netdev();
		if (!dev)
			break;
		dev->irq = irq[this_dev];
		dev->base_addr = io[this_dev];
		dev->mem_start = mem[this_dev];
		if (do_es_probe(dev) == 0) {
			dev_es3210[found++] = dev;
			continue;
		}
		free_netdev(dev);
		printk(KERN_WARNING "es3210.c: No es3210 card found (i/o = 0x%x).\n", io[this_dev]);
		break;
	}
	if (found)
		return 0;
	return -ENXIO;
}

static void cleanup_card(struct net_device *dev)
{
	free_irq(dev->irq, dev);
	release_region(dev->base_addr, ES_IO_EXTENT);
	iounmap(ei_status.mem);
}

void __exit
cleanup_module(void)
{
	int this_dev;

	for (this_dev = 0; this_dev < MAX_ES_CARDS; this_dev++) {
		struct net_device *dev = dev_es3210[this_dev];
		if (dev) {
			unregister_netdev(dev);
			cleanup_card(dev);
			free_netdev(dev);
		}
	}
}
#endif 

