

static const char *version =
	"lne390.c: Driver revision v0.99.1, 01/09/2000\n";

#include <linux/module.h>
#include <linux/eisa.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#include <asm/io.h>
#include <asm/system.h>

#include "8390.h"

#define DRV_NAME "lne390"

static int lne390_probe1(struct net_device *dev, int ioaddr);

static void lne390_reset_8390(struct net_device *dev);

static void lne390_get_8390_hdr(struct net_device *dev, struct e8390_pkt_hdr *hdr, int ring_page);
static void lne390_block_input(struct net_device *dev, int count, struct sk_buff *skb, int ring_offset);
static void lne390_block_output(struct net_device *dev, int count, const unsigned char *buf, const int start_page);

#define LNE390_START_PG		0x00    
#define LNE390_STOP_PG		0x80    

#define LNE390_ID_PORT		0xc80	
#define LNE390_IO_EXTENT	0x20
#define LNE390_SA_PROM		0x16	
#define LNE390_RESET_PORT	0xc84	
#define LNE390_NIC_OFFSET	0x00	

#define LNE390_ADDR0		0x00	
#define LNE390_ADDR1		0x80
#define LNE390_ADDR2		0xe5

#define LNE390_ID0	0x10009835	
#define LNE390_ID1	0x11009835	

#define LNE390_CFG1		0xc84	
#define LNE390_CFG2		0xc90



#define LNE390_D_PROBE	0x01
#define LNE390_D_RX_PKT	0x02
#define LNE390_D_TX_PKT	0x04
#define LNE390_D_IRQ	0x08

#define LNE390_DEBUG	0

static unsigned char irq_map[] __initdata = {15, 12, 11, 10, 9, 7, 5, 3};
static unsigned int shmem_mapA[] __initdata = {0xff, 0xfe, 0xfd, 0xfff, 0xffe, 0xffc, 0x0d, 0x0};
static unsigned int shmem_mapB[] __initdata = {0xff, 0xfe, 0x0e, 0xfff, 0xffe, 0xffc, 0x0d, 0x0};



static int __init do_lne390_probe(struct net_device *dev)
{
	unsigned short ioaddr = dev->base_addr;
	int irq = dev->irq;
	int mem_start = dev->mem_start;
	int ret;

	if (ioaddr > 0x1ff) {		
		if (!request_region(ioaddr, LNE390_IO_EXTENT, DRV_NAME))
			return -EBUSY;
		ret = lne390_probe1(dev, ioaddr);
		if (ret)
			release_region(ioaddr, LNE390_IO_EXTENT);
		return ret;
	}
	else if (ioaddr > 0)		
		return -ENXIO;

	if (!EISA_bus) {
#if LNE390_DEBUG & LNE390_D_PROBE
		printk("lne390-debug: Not an EISA bus. Not probing high ports.\n");
#endif
		return -ENXIO;
	}

	
	for (ioaddr = 0x1000; ioaddr < 0x9000; ioaddr += 0x1000) {
		if (!request_region(ioaddr, LNE390_IO_EXTENT, DRV_NAME))
			continue;
		if (lne390_probe1(dev, ioaddr) == 0)
			return 0;
		release_region(ioaddr, LNE390_IO_EXTENT);
		dev->irq = irq;
		dev->mem_start = mem_start;
	}

	return -ENODEV;
}

#ifndef MODULE
struct net_device * __init lne390_probe(int unit)
{
	struct net_device *dev = alloc_ei_netdev();
	int err;

	if (!dev)
		return ERR_PTR(-ENOMEM);

	sprintf(dev->name, "eth%d", unit);
	netdev_boot_setup_check(dev);

	err = do_lne390_probe(dev);
	if (err)
		goto out;
	return dev;
out:
	free_netdev(dev);
	return ERR_PTR(err);
}
#endif

static int __init lne390_probe1(struct net_device *dev, int ioaddr)
{
	int i, revision, ret;
	unsigned long eisa_id;

	if (inb_p(ioaddr + LNE390_ID_PORT) == 0xff) return -ENODEV;

#if LNE390_DEBUG & LNE390_D_PROBE
	printk("lne390-debug: probe at %#x, ID %#8x\n", ioaddr, inl(ioaddr + LNE390_ID_PORT));
	printk("lne390-debug: config regs: %#x %#x\n",
		inb(ioaddr + LNE390_CFG1), inb(ioaddr + LNE390_CFG2));
#endif



	eisa_id = inl(ioaddr + LNE390_ID_PORT);
	if ((eisa_id != LNE390_ID0) && (eisa_id != LNE390_ID1)) {
		return -ENODEV;
	}

	revision = (eisa_id >> 24) & 0x01;	

#if 0

	if (inb(ioaddr + LNE390_SA_PROM + 0) != LNE390_ADDR0
		|| inb(ioaddr + LNE390_SA_PROM + 1) != LNE390_ADDR1
		|| inb(ioaddr + LNE390_SA_PROM + 2) != LNE390_ADDR2 ) {
		printk("lne390.c: card not found");
		for(i = 0; i < ETHER_ADDR_LEN; i++)
			printk(" %02x", inb(ioaddr + LNE390_SA_PROM + i));
		printk(" (invalid prefix).\n");
		return -ENODEV;
	}
#endif

	for(i = 0; i < ETHER_ADDR_LEN; i++)
		dev->dev_addr[i] = inb(ioaddr + LNE390_SA_PROM + i);
	printk("lne390.c: LNE390%X in EISA slot %d, address %pM.\n",
	       0xa+revision, ioaddr/0x1000, dev->dev_addr);

	printk("lne390.c: ");

	
	if (dev->irq == 0) {
		unsigned char irq_reg = inb(ioaddr + LNE390_CFG2) >> 3;
		dev->irq = irq_map[irq_reg & 0x07];
		printk("using");
	} else {
		
		if (dev->irq == 2) dev->irq = 9;	
		printk("assigning");
	}
	printk(" IRQ %d,", dev->irq);

	if ((ret = request_irq(dev->irq, ei_interrupt, 0, DRV_NAME, dev))) {
		printk (" unable to get IRQ %d.\n", dev->irq);
		return ret;
	}

	if (dev->mem_start == 0) {
		unsigned char mem_reg = inb(ioaddr + LNE390_CFG2) & 0x07;

		if (revision)	
			dev->mem_start = shmem_mapB[mem_reg] * 0x10000;
		else		
			dev->mem_start = shmem_mapA[mem_reg] * 0x10000;
		printk(" using ");
	} else {
		
		dev->mem_start &= 0xfff0000;
		printk(" assigning ");
	}

	printk("%dkB memory at physical address %#lx\n",
			LNE390_STOP_PG/4, dev->mem_start);

	
	ei_status.mem = ioremap(dev->mem_start, LNE390_STOP_PG*0x100);
	if (!ei_status.mem) {
		printk(KERN_ERR "lne390.c: Unable to remap card memory above 1MB !!\n");
		printk(KERN_ERR "lne390.c: Try using EISA SCU to set memory below 1MB.\n");
		printk(KERN_ERR "lne390.c: Driver NOT installed.\n");
		ret = -EAGAIN;
		goto cleanup;
	}
	printk("lne390.c: remapped %dkB card memory to virtual address %p\n",
			LNE390_STOP_PG/4, ei_status.mem);

	dev->mem_start = (unsigned long)ei_status.mem;
	dev->mem_end = dev->mem_start + (LNE390_STOP_PG - LNE390_START_PG)*256;

	
	dev->base_addr = ioaddr;

	ei_status.name = "LNE390";
	ei_status.tx_start_page = LNE390_START_PG;
	ei_status.rx_start_page = LNE390_START_PG + TX_PAGES;
	ei_status.stop_page = LNE390_STOP_PG;
	ei_status.word16 = 1;

	if (ei_debug > 0)
		printk(version);

	ei_status.reset_8390 = &lne390_reset_8390;
	ei_status.block_input = &lne390_block_input;
	ei_status.block_output = &lne390_block_output;
	ei_status.get_8390_hdr = &lne390_get_8390_hdr;

	dev->netdev_ops = &ei_netdev_ops;
	NS8390_init(dev, 0);

	ret = register_netdev(dev);
	if (ret)
		goto unmap;
	return 0;
unmap:
	if (ei_status.reg0)
		iounmap(ei_status.mem);
cleanup:
	free_irq(dev->irq, dev);
	return ret;
}



static void lne390_reset_8390(struct net_device *dev)
{
	unsigned short ioaddr = dev->base_addr;

	outb(0x04, ioaddr + LNE390_RESET_PORT);
	if (ei_debug > 1) printk("%s: resetting the LNE390...", dev->name);

	mdelay(2);

	ei_status.txing = 0;
	outb(0x01, ioaddr + LNE390_RESET_PORT);
	if (ei_debug > 1) printk("reset done\n");

	return;
}





static void
lne390_get_8390_hdr(struct net_device *dev, struct e8390_pkt_hdr *hdr, int ring_page)
{
	void __iomem *hdr_start = ei_status.mem + ((ring_page - LNE390_START_PG)<<8);
	memcpy_fromio(hdr, hdr_start, sizeof(struct e8390_pkt_hdr));
	hdr->count = (hdr->count + 3) & ~3;     
}



static void lne390_block_input(struct net_device *dev, int count, struct sk_buff *skb,
						  int ring_offset)
{
	void __iomem *xfer_start = ei_status.mem + ring_offset - (LNE390_START_PG<<8);

	if (ring_offset + count > (LNE390_STOP_PG<<8)) {
		
		int semi_count = (LNE390_STOP_PG<<8) - ring_offset;
		memcpy_fromio(skb->data, xfer_start, semi_count);
		count -= semi_count;
		memcpy_fromio(skb->data + semi_count,
			ei_status.mem + (TX_PAGES<<8), count);
	} else {
		
		memcpy_fromio(skb->data, xfer_start, count);
	}
}

static void lne390_block_output(struct net_device *dev, int count,
				const unsigned char *buf, int start_page)
{
	void __iomem *shmem = ei_status.mem + ((start_page - LNE390_START_PG)<<8);

	count = (count + 3) & ~3;     
	memcpy_toio(shmem, buf, count);
}


#ifdef MODULE
#define MAX_LNE_CARDS	4	
static struct net_device *dev_lne[MAX_LNE_CARDS];
static int io[MAX_LNE_CARDS];
static int irq[MAX_LNE_CARDS];
static int mem[MAX_LNE_CARDS];

module_param_array(io, int, NULL, 0);
module_param_array(irq, int, NULL, 0);
module_param_array(mem, int, NULL, 0);
MODULE_PARM_DESC(io, "I/O base address(es)");
MODULE_PARM_DESC(irq, "IRQ number(s)");
MODULE_PARM_DESC(mem, "memory base address(es)");
MODULE_DESCRIPTION("Mylex LNE390A/B EISA Ethernet driver");
MODULE_LICENSE("GPL");

int __init init_module(void)
{
	struct net_device *dev;
	int this_dev, found = 0;

	for (this_dev = 0; this_dev < MAX_LNE_CARDS; this_dev++) {
		if (io[this_dev] == 0 && this_dev != 0)
			break;
		dev = alloc_ei_netdev();
		if (!dev)
			break;
		dev->irq = irq[this_dev];
		dev->base_addr = io[this_dev];
		dev->mem_start = mem[this_dev];
		if (do_lne390_probe(dev) == 0) {
			dev_lne[found++] = dev;
			continue;
		}
		free_netdev(dev);
		printk(KERN_WARNING "lne390.c: No LNE390 card found (i/o = 0x%x).\n", io[this_dev]);
		break;
	}
	if (found)
		return 0;
	return -ENXIO;
}

static void cleanup_card(struct net_device *dev)
{
	free_irq(dev->irq, dev);
	release_region(dev->base_addr, LNE390_IO_EXTENT);
	iounmap(ei_status.mem);
}

void __exit cleanup_module(void)
{
	int this_dev;

	for (this_dev = 0; this_dev < MAX_LNE_CARDS; this_dev++) {
		struct net_device *dev = dev_lne[this_dev];
		if (dev) {
			unregister_netdev(dev);
			cleanup_card(dev);
			free_netdev(dev);
		}
	}
}
#endif 

