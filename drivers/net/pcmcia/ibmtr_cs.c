

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/trdevice.h>
#include <linux/ibmtr.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ds.h>

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/system.h>

#define PCMCIA
#include "../tokenring/ibmtr.c"

#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
static char *version =
"ibmtr_cs.c 1.10   1996/01/06 05:19:00 (Steve Kipisz)\n"
"           2.2.7  1999/05/03 12:00:00 (Mike Phillips)\n"
"           2.4.2  2001/30/28 Midnight (Burt Silverman)\n";
#else
#define DEBUG(n, args...)
#endif






static u_long mmiobase = 0xce000;


static u_long srambase = 0xd0000;


static u_long sramsize = 64;


static int ringspeed = 16;

module_param(mmiobase, ulong, 0);
module_param(srambase, ulong, 0);
module_param(sramsize, ulong, 0);
module_param(ringspeed, int, 0);
MODULE_LICENSE("GPL");



static int ibmtr_config(struct pcmcia_device *link);
static void ibmtr_hw_setup(struct net_device *dev, u_int mmiobase);
static void ibmtr_release(struct pcmcia_device *link);
static void ibmtr_detach(struct pcmcia_device *p_dev);



typedef struct ibmtr_dev_t {
	struct pcmcia_device	*p_dev;
    struct net_device	*dev;
    dev_node_t          node;
    window_handle_t     sram_win_handle;
    struct tok_info	*ti;
} ibmtr_dev_t;

static void netdev_get_drvinfo(struct net_device *dev,
			       struct ethtool_drvinfo *info)
{
	strcpy(info->driver, "ibmtr_cs");
}

static const struct ethtool_ops netdev_ethtool_ops = {
	.get_drvinfo		= netdev_get_drvinfo,
};



static int __devinit ibmtr_attach(struct pcmcia_device *link)
{
    ibmtr_dev_t *info;
    struct net_device *dev;

    DEBUG(0, "ibmtr_attach()\n");

    
    info = kzalloc(sizeof(*info), GFP_KERNEL);
    if (!info) return -ENOMEM;
    dev = alloc_trdev(sizeof(struct tok_info));
    if (!dev) {
	kfree(info);
	return -ENOMEM;
    }

    info->p_dev = link;
    link->priv = info;
    info->ti = netdev_priv(dev);

    link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
    link->io.NumPorts1 = 4;
    link->io.IOAddrLines = 16;
    link->irq.Attributes = IRQ_TYPE_EXCLUSIVE | IRQ_HANDLE_PRESENT;
    link->irq.IRQInfo1 = IRQ_LEVEL_ID;
    link->irq.Handler = &tok_interrupt;
    link->conf.Attributes = CONF_ENABLE_IRQ;
    link->conf.IntType = INT_MEMORY_AND_IO;
    link->conf.Present = PRESENT_OPTION;

    link->irq.Instance = info->dev = dev;

    SET_ETHTOOL_OPS(dev, &netdev_ethtool_ops);

    return ibmtr_config(link);
} 



static void ibmtr_detach(struct pcmcia_device *link)
{
    struct ibmtr_dev_t *info = link->priv;
    struct net_device *dev = info->dev;
     struct tok_info *ti = netdev_priv(dev);

    DEBUG(0, "ibmtr_detach(0x%p)\n", link);
    
    
    ti->sram_phys |= 1;

    if (link->dev_node)
	unregister_netdev(dev);
    
    del_timer_sync(&(ti->tr_timer));

    ibmtr_release(link);

    free_netdev(dev);
    kfree(info);
} 



#define CS_CHECK(fn, ret) \
do { last_fn = (fn); if ((last_ret = (ret)) != 0) goto cs_failed; } while (0)

static int __devinit ibmtr_config(struct pcmcia_device *link)
{
    ibmtr_dev_t *info = link->priv;
    struct net_device *dev = info->dev;
    struct tok_info *ti = netdev_priv(dev);
    win_req_t req;
    memreq_t mem;
    int i, last_ret, last_fn;

    DEBUG(0, "ibmtr_config(0x%p)\n", link);

    link->conf.ConfigIndex = 0x61;

    

    
    link->io.BasePort1 = 0xA20;
    i = pcmcia_request_io(link, &link->io);
    if (i != 0) {
	
	link->io.BasePort1 = 0xA24;
	CS_CHECK(RequestIO, pcmcia_request_io(link, &link->io));
    }
    dev->base_addr = link->io.BasePort1;

    CS_CHECK(RequestIRQ, pcmcia_request_irq(link, &link->irq));
    dev->irq = link->irq.AssignedIRQ;
    ti->irq = link->irq.AssignedIRQ;
    ti->global_int_enable=GLOBAL_INT_ENABLE+((dev->irq==9) ? 2 : dev->irq);

    
    req.Attributes = WIN_DATA_WIDTH_16|WIN_MEMORY_TYPE_CM|WIN_ENABLE;
    req.Attributes |= WIN_USE_WAIT;
    req.Base = 0; 
    req.Size = 0x2000;
    req.AccessSpeed = 250;
    CS_CHECK(RequestWindow, pcmcia_request_window(&link, &req, &link->win));

    mem.CardOffset = mmiobase;
    mem.Page = 0;
    CS_CHECK(MapMemPage, pcmcia_map_mem_page(link->win, &mem));
    ti->mmio = ioremap(req.Base, req.Size);

    
    req.Attributes = WIN_DATA_WIDTH_16|WIN_MEMORY_TYPE_CM|WIN_ENABLE;
    req.Attributes |= WIN_USE_WAIT;
    req.Base = 0;
    req.Size = sramsize * 1024;
    req.AccessSpeed = 250;
    CS_CHECK(RequestWindow, pcmcia_request_window(&link, &req, &info->sram_win_handle));

    mem.CardOffset = srambase;
    mem.Page = 0;
    CS_CHECK(MapMemPage, pcmcia_map_mem_page(info->sram_win_handle, &mem));

    ti->sram_base = mem.CardOffset >> 12;
    ti->sram_virt = ioremap(req.Base, req.Size);
    ti->sram_phys = req.Base;

    CS_CHECK(RequestConfiguration, pcmcia_request_configuration(link, &link->conf));

    
    ibmtr_hw_setup(dev, mmiobase);

    link->dev_node = &info->node;
    SET_NETDEV_DEV(dev, &handle_to_dev(link));

    i = ibmtr_probe_card(dev);
    if (i != 0) {
	printk(KERN_NOTICE "ibmtr_cs: register_netdev() failed\n");
	link->dev_node = NULL;
	goto failed;
    }

    strcpy(info->node.dev_name, dev->name);

    printk(KERN_INFO
	   "%s: port %#3lx, irq %d,  mmio %#5lx, sram %#5lx, hwaddr=%pM\n",
           dev->name, dev->base_addr, dev->irq,
	   (u_long)ti->mmio, (u_long)(ti->sram_base << 12),
	   dev->dev_addr);
    return 0;

cs_failed:
    cs_error(link, last_fn, last_ret);
failed:
    ibmtr_release(link);
    return -ENODEV;
} 



static void ibmtr_release(struct pcmcia_device *link)
{
	ibmtr_dev_t *info = link->priv;
	struct net_device *dev = info->dev;

	DEBUG(0, "ibmtr_release(0x%p)\n", link);

	if (link->win) {
		struct tok_info *ti = netdev_priv(dev);
		iounmap(ti->mmio);
		pcmcia_release_window(info->sram_win_handle);
	}
	pcmcia_disable_device(link);
}

static int ibmtr_suspend(struct pcmcia_device *link)
{
	ibmtr_dev_t *info = link->priv;
	struct net_device *dev = info->dev;

	if (link->open)
		netif_device_detach(dev);

	return 0;
}

static int __devinit ibmtr_resume(struct pcmcia_device *link)
{
	ibmtr_dev_t *info = link->priv;
	struct net_device *dev = info->dev;

	if (link->open) {
		ibmtr_probe(dev);	
		netif_device_attach(dev);
	}

	return 0;
}




static void ibmtr_hw_setup(struct net_device *dev, u_int mmiobase)
{
    int i;

    

    
    i = (mmiobase >> 16) & 0x0F;
    outb(i, dev->base_addr);

    
    i = 0x10 | ((mmiobase >> 12) & 0x0E);
    outb(i, dev->base_addr);

    
    i = 0x26;
    outb(i, dev->base_addr);

    

              
    i = (sramsize >> 4) & 0x07;
    i = ((i == 4) ? 3 : i) << 2;
    i |= 0x30;

    if (ringspeed == 16)
	i |= 2;
    if (dev->base_addr == 0xA24)
	i |= 1;
    outb(i, dev->base_addr);

    
    outb(0x40, dev->base_addr);

    return;
}

static struct pcmcia_device_id ibmtr_ids[] = {
	PCMCIA_DEVICE_PROD_ID12("3Com", "TokenLink Velocity PC Card", 0x41240e5b, 0x82c3734e),
	PCMCIA_DEVICE_PROD_ID12("IBM", "TOKEN RING", 0xb569a6e5, 0xbf8eed47),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, ibmtr_ids);

static struct pcmcia_driver ibmtr_cs_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= "ibmtr_cs",
	},
	.probe		= ibmtr_attach,
	.remove		= ibmtr_detach,
	.id_table       = ibmtr_ids,
	.suspend	= ibmtr_suspend,
	.resume		= ibmtr_resume,
};

static int __init init_ibmtr_cs(void)
{
	return pcmcia_register_driver(&ibmtr_cs_driver);
}

static void __exit exit_ibmtr_cs(void)
{
	pcmcia_unregister_driver(&ibmtr_cs_driver);
}

module_init(init_ibmtr_cs);
module_exit(exit_ibmtr_cs);
