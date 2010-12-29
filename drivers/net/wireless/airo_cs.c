

#ifdef __IN_PCMCIA_PACKAGE__
#include <pcmcia/k_compat.h>
#endif
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/netdevice.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>

#include <linux/io.h>
#include <asm/system.h>

#include "airo.h"


#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0);
static char *version = "$Revision: 1.2 $";
#define DEBUG(n, args...) if (pc_debug > (n)) printk(KERN_DEBUG args);
#else
#define DEBUG(n, args...)
#endif



MODULE_AUTHOR("Benjamin Reed");
MODULE_DESCRIPTION("Support for Cisco/Aironet 802.11 wireless ethernet "
		   "cards.  This is the module that links the PCMCIA card "
		   "with the airo module.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_SUPPORTED_DEVICE("Aironet 4500, 4800 and Cisco 340 PCMCIA cards");





static int airo_config(struct pcmcia_device *link);
static void airo_release(struct pcmcia_device *link);



static void airo_detach(struct pcmcia_device *p_dev);







typedef struct local_info_t {
	dev_node_t	node;
	struct net_device *eth_dev;
} local_info_t;



static int airo_probe(struct pcmcia_device *p_dev)
{
	local_info_t *local;

	DEBUG(0, "airo_attach()\n");

	
	p_dev->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING;
	p_dev->irq.IRQInfo1 = IRQ_LEVEL_ID;
	p_dev->irq.Handler = NULL;

	
	p_dev->conf.Attributes = 0;
	p_dev->conf.IntType = INT_MEMORY_AND_IO;

	
	local = kzalloc(sizeof(local_info_t), GFP_KERNEL);
	if (!local) {
		printk(KERN_ERR "airo_cs: no memory for new device\n");
		return -ENOMEM;
	}
	p_dev->priv = local;

	return airo_config(p_dev);
} 



static void airo_detach(struct pcmcia_device *link)
{
	DEBUG(0, "airo_detach(0x%p)\n", link);

	airo_release(link);

	if (((local_info_t *)link->priv)->eth_dev) {
		stop_airo_card(((local_info_t *)link->priv)->eth_dev, 0);
	}
	((local_info_t *)link->priv)->eth_dev = NULL;

	kfree(link->priv);
} 



#define CS_CHECK(fn, ret) \
do { last_fn = (fn); if ((last_ret = (ret)) != 0) goto cs_failed; } while (0)

static int airo_cs_config_check(struct pcmcia_device *p_dev,
				cistpl_cftable_entry_t *cfg,
				cistpl_cftable_entry_t *dflt,
				unsigned int vcc,
				void *priv_data)
{
	win_req_t *req = priv_data;

	if (cfg->index == 0)
		return -ENODEV;

	
	if (cfg->flags & CISTPL_CFTABLE_AUDIO) {
		p_dev->conf.Attributes |= CONF_ENABLE_SPKR;
		p_dev->conf.Status = CCSR_AUDIO_ENA;
	}

	
	
	if (cfg->vpp1.present & (1<<CISTPL_POWER_VNOM))
		p_dev->conf.Vpp = cfg->vpp1.param[CISTPL_POWER_VNOM]/10000;
	else if (dflt->vpp1.present & (1<<CISTPL_POWER_VNOM))
		p_dev->conf.Vpp = dflt->vpp1.param[CISTPL_POWER_VNOM]/10000;

	
	if (cfg->irq.IRQInfo1 || dflt->irq.IRQInfo1)
		p_dev->conf.Attributes |= CONF_ENABLE_IRQ;

	
	p_dev->io.NumPorts1 = p_dev->io.NumPorts2 = 0;
	if ((cfg->io.nwin > 0) || (dflt->io.nwin > 0)) {
		cistpl_io_t *io = (cfg->io.nwin) ? &cfg->io : &dflt->io;
		p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
		if (!(io->flags & CISTPL_IO_8BIT))
			p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_16;
		if (!(io->flags & CISTPL_IO_16BIT))
			p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
		p_dev->io.BasePort1 = io->win[0].base;
		p_dev->io.NumPorts1 = io->win[0].len;
		if (io->nwin > 1) {
			p_dev->io.Attributes2 = p_dev->io.Attributes1;
			p_dev->io.BasePort2 = io->win[1].base;
			p_dev->io.NumPorts2 = io->win[1].len;
		}
	}

	
	if (pcmcia_request_io(p_dev, &p_dev->io) != 0)
		return -ENODEV;

	
	if ((cfg->mem.nwin > 0) || (dflt->mem.nwin > 0)) {
		cistpl_mem_t *mem = (cfg->mem.nwin) ? &cfg->mem : &dflt->mem;
		memreq_t map;
		req->Attributes = WIN_DATA_WIDTH_16|WIN_MEMORY_TYPE_CM;
		req->Base = mem->win[0].host_addr;
		req->Size = mem->win[0].len;
		req->AccessSpeed = 0;
		if (pcmcia_request_window(&p_dev, req, &p_dev->win) != 0)
			return -ENODEV;
		map.Page = 0;
		map.CardOffset = mem->win[0].card_addr;
		if (pcmcia_map_mem_page(p_dev->win, &map) != 0)
			return -ENODEV;
	}
	
	return 0;
}


static int airo_config(struct pcmcia_device *link)
{
	local_info_t *dev;
	win_req_t *req;
	int last_fn, last_ret;

	dev = link->priv;

	DEBUG(0, "airo_config(0x%p)\n", link);

	req = kzalloc(sizeof(win_req_t), GFP_KERNEL);
	if (!req)
		return -ENOMEM;

	
	last_ret = pcmcia_loop_config(link, airo_cs_config_check, req);
	if (last_ret)
		goto failed;

	
	if (link->conf.Attributes & CONF_ENABLE_IRQ)
		CS_CHECK(RequestIRQ, pcmcia_request_irq(link, &link->irq));

	
	CS_CHECK(RequestConfiguration,
		 pcmcia_request_configuration(link, &link->conf));
	((local_info_t *)link->priv)->eth_dev =
		init_airo_card(link->irq.AssignedIRQ,
			       link->io.BasePort1, 1, &handle_to_dev(link));
	if (!((local_info_t *)link->priv)->eth_dev)
		goto cs_failed;

	
	strcpy(dev->node.dev_name, ((local_info_t *)link->priv)->eth_dev->name);
	dev->node.major = dev->node.minor = 0;
	link->dev_node = &dev->node;

	
	printk(KERN_INFO "%s: index 0x%02x: ",
	       dev->node.dev_name, link->conf.ConfigIndex);
	if (link->conf.Vpp)
		printk(", Vpp %d.%d", link->conf.Vpp/10, link->conf.Vpp%10);
	if (link->conf.Attributes & CONF_ENABLE_IRQ)
		printk(", irq %d", link->irq.AssignedIRQ);
	if (link->io.NumPorts1)
		printk(", io 0x%04x-0x%04x", link->io.BasePort1,
		       link->io.BasePort1+link->io.NumPorts1-1);
	if (link->io.NumPorts2)
		printk(" & 0x%04x-0x%04x", link->io.BasePort2,
		       link->io.BasePort2+link->io.NumPorts2-1);
	if (link->win)
		printk(", mem 0x%06lx-0x%06lx", req->Base,
		       req->Base+req->Size-1);
	printk("\n");
	kfree(req);
	return 0;

 cs_failed:
	cs_error(link, last_fn, last_ret);
 failed:
	airo_release(link);
	kfree(req);
	return -ENODEV;
} 



static void airo_release(struct pcmcia_device *link)
{
	DEBUG(0, "airo_release(0x%p)\n", link);
	pcmcia_disable_device(link);
}

static int airo_suspend(struct pcmcia_device *link)
{
	local_info_t *local = link->priv;

	netif_device_detach(local->eth_dev);

	return 0;
}

static int airo_resume(struct pcmcia_device *link)
{
	local_info_t *local = link->priv;

	if (link->open) {
		reset_airo_card(local->eth_dev);
		netif_device_attach(local->eth_dev);
	}

	return 0;
}

static struct pcmcia_device_id airo_ids[] = {
	PCMCIA_DEVICE_MANF_CARD(0x015f, 0x000a),
	PCMCIA_DEVICE_MANF_CARD(0x015f, 0x0005),
	PCMCIA_DEVICE_MANF_CARD(0x015f, 0x0007),
	PCMCIA_DEVICE_MANF_CARD(0x0105, 0x0007),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, airo_ids);

static struct pcmcia_driver airo_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= "airo_cs",
	},
	.probe		= airo_probe,
	.remove		= airo_detach,
	.id_table       = airo_ids,
	.suspend	= airo_suspend,
	.resume		= airo_resume,
};

static int airo_cs_init(void)
{
	return pcmcia_register_driver(&airo_driver);
}

static void airo_cs_cleanup(void)
{
	pcmcia_unregister_driver(&airo_driver);
}



module_init(airo_cs_init);
module_exit(airo_cs_cleanup);
