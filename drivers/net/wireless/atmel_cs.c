

#ifdef __IN_PCMCIA_PACKAGE__
#include <pcmcia/k_compat.h>
#endif
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/netdevice.h>
#include <linux/moduleparam.h>
#include <linux/device.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>
#include <pcmcia/ciscode.h>

#include <asm/io.h>
#include <asm/system.h>
#include <linux/wireless.h>

#include "atmel.h"



#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0);
static char *version = "$Revision: 1.2 $";
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args);
#else
#define DEBUG(n, args...)
#endif



MODULE_AUTHOR("Simon Kelley");
MODULE_DESCRIPTION("Support for Atmel at76c50x 802.11 wireless ethernet cards.");
MODULE_LICENSE("GPL");
MODULE_SUPPORTED_DEVICE("Atmel at76c50x PCMCIA cards");





static int atmel_config(struct pcmcia_device *link);
static void atmel_release(struct pcmcia_device *link);



static void atmel_detach(struct pcmcia_device *p_dev);







typedef struct local_info_t {
	dev_node_t	node;
	struct net_device *eth_dev;
} local_info_t;



static int atmel_probe(struct pcmcia_device *p_dev)
{
	local_info_t *local;

	DEBUG(0, "atmel_attach()\n");

	
	p_dev->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING;
	p_dev->irq.IRQInfo1 = IRQ_LEVEL_ID;
	p_dev->irq.Handler = NULL;

	
	p_dev->conf.Attributes = 0;
	p_dev->conf.IntType = INT_MEMORY_AND_IO;

	
	local = kzalloc(sizeof(local_info_t), GFP_KERNEL);
	if (!local) {
		printk(KERN_ERR "atmel_cs: no memory for new device\n");
		return -ENOMEM;
	}
	p_dev->priv = local;

	return atmel_config(p_dev);
} 



static void atmel_detach(struct pcmcia_device *link)
{
	DEBUG(0, "atmel_detach(0x%p)\n", link);

	atmel_release(link);

	kfree(link->priv);
}



#define CS_CHECK(fn, ret) \
do { last_fn = (fn); if ((last_ret = (ret)) != 0) goto cs_failed; } while (0)


static int card_present(void *arg)
{
	struct pcmcia_device *link = (struct pcmcia_device *)arg;

	if (pcmcia_dev_present(link))
		return 1;

	return 0;
}

static int atmel_config_check(struct pcmcia_device *p_dev,
			      cistpl_cftable_entry_t *cfg,
			      cistpl_cftable_entry_t *dflt,
			      unsigned int vcc,
			      void *priv_data)
{
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

	
	return pcmcia_request_io(p_dev, &p_dev->io);
}

static int atmel_config(struct pcmcia_device *link)
{
	local_info_t *dev;
	int last_fn, last_ret;
	struct pcmcia_device_id *did;

	dev = link->priv;
	did = dev_get_drvdata(&handle_to_dev(link));

	DEBUG(0, "atmel_config(0x%p)\n", link);

	
	if (pcmcia_loop_config(link, atmel_config_check, NULL))
		goto failed;

	
	if (link->conf.Attributes & CONF_ENABLE_IRQ)
		CS_CHECK(RequestIRQ, pcmcia_request_irq(link, &link->irq));

	
	CS_CHECK(RequestConfiguration, pcmcia_request_configuration(link, &link->conf));

	if (link->irq.AssignedIRQ == 0) {
		printk(KERN_ALERT
		       "atmel: cannot assign IRQ: check that CONFIG_ISA is set in kernel config.");
		goto cs_failed;
	}

	((local_info_t*)link->priv)->eth_dev =
		init_atmel_card(link->irq.AssignedIRQ,
				link->io.BasePort1,
				did ? did->driver_info : ATMEL_FW_TYPE_NONE,
				&handle_to_dev(link),
				card_present,
				link);
	if (!((local_info_t*)link->priv)->eth_dev)
			goto cs_failed;


	
	strcpy(dev->node.dev_name, ((local_info_t*)link->priv)->eth_dev->name );
	dev->node.major = dev->node.minor = 0;
	link->dev_node = &dev->node;

	return 0;

 cs_failed:
	cs_error(link, last_fn, last_ret);
 failed:
	atmel_release(link);
	return -ENODEV;
}



static void atmel_release(struct pcmcia_device *link)
{
	struct net_device *dev = ((local_info_t*)link->priv)->eth_dev;

	DEBUG(0, "atmel_release(0x%p)\n", link);

	if (dev)
		stop_atmel_card(dev);
	((local_info_t*)link->priv)->eth_dev = NULL;

	pcmcia_disable_device(link);
}

static int atmel_suspend(struct pcmcia_device *link)
{
	local_info_t *local = link->priv;

	netif_device_detach(local->eth_dev);

	return 0;
}

static int atmel_resume(struct pcmcia_device *link)
{
	local_info_t *local = link->priv;

	atmel_open(local->eth_dev);
	netif_device_attach(local->eth_dev);

	return 0;
}




#define PCMCIA_DEVICE_MANF_CARD_INFO(manf, card, info) { \
	.match_flags = PCMCIA_DEV_ID_MATCH_MANF_ID| \
			PCMCIA_DEV_ID_MATCH_CARD_ID, \
	.manf_id = (manf), \
	.card_id = (card), \
        .driver_info = (kernel_ulong_t)(info), }

#define PCMCIA_DEVICE_PROD_ID12_INFO(v1, v2, vh1, vh2, info) { \
	.match_flags = PCMCIA_DEV_ID_MATCH_PROD_ID1| \
			PCMCIA_DEV_ID_MATCH_PROD_ID2, \
	.prod_id = { (v1), (v2), NULL, NULL }, \
	.prod_id_hash = { (vh1), (vh2), 0, 0 }, \
        .driver_info = (kernel_ulong_t)(info), }

static struct pcmcia_device_id atmel_ids[] = {
	PCMCIA_DEVICE_MANF_CARD_INFO(0x0101, 0x0620, ATMEL_FW_TYPE_502_3COM),
	PCMCIA_DEVICE_MANF_CARD_INFO(0x0101, 0x0696, ATMEL_FW_TYPE_502_3COM),
	PCMCIA_DEVICE_MANF_CARD_INFO(0x01bf, 0x3302, ATMEL_FW_TYPE_502E),
	PCMCIA_DEVICE_MANF_CARD_INFO(0xd601, 0x0007, ATMEL_FW_TYPE_502),
	PCMCIA_DEVICE_PROD_ID12_INFO("11WAVE", "11WP611AL-E", 0x9eb2da1f, 0xc9a0d3f9, ATMEL_FW_TYPE_502E),
	PCMCIA_DEVICE_PROD_ID12_INFO("ATMEL", "AT76C502AR", 0xabda4164, 0x41b37e1f, ATMEL_FW_TYPE_502),
	PCMCIA_DEVICE_PROD_ID12_INFO("ATMEL", "AT76C502AR_D", 0xabda4164, 0x3675d704, ATMEL_FW_TYPE_502D),
	PCMCIA_DEVICE_PROD_ID12_INFO("ATMEL", "AT76C502AR_E", 0xabda4164, 0x4172e792, ATMEL_FW_TYPE_502E),
	PCMCIA_DEVICE_PROD_ID12_INFO("ATMEL", "AT76C504_R", 0xabda4164, 0x917f3d72, ATMEL_FW_TYPE_504_2958),
	PCMCIA_DEVICE_PROD_ID12_INFO("ATMEL", "AT76C504", 0xabda4164, 0x5040670a, ATMEL_FW_TYPE_504),
	PCMCIA_DEVICE_PROD_ID12_INFO("ATMEL", "AT76C504A", 0xabda4164, 0xe15ed87f, ATMEL_FW_TYPE_504A_2958),
	PCMCIA_DEVICE_PROD_ID12_INFO("BT", "Voyager 1020 Laptop Adapter", 0xae49b86a, 0x1e957cd5, ATMEL_FW_TYPE_502),
	PCMCIA_DEVICE_PROD_ID12_INFO("CNet", "CNWLC 11Mbps Wireless PC Card V-5", 0xbc477dde, 0x502fae6b, ATMEL_FW_TYPE_502E),
	PCMCIA_DEVICE_PROD_ID12_INFO("IEEE 802.11b", "Wireless LAN PC Card", 0x5b878724, 0x122f1df6, ATMEL_FW_TYPE_502),
	PCMCIA_DEVICE_PROD_ID12_INFO("IEEE 802.11b", "Wireless LAN Card S", 0x5b878724, 0x5fba533a, ATMEL_FW_TYPE_504_2958),
	PCMCIA_DEVICE_PROD_ID12_INFO("OEM", "11Mbps Wireless LAN PC Card V-3", 0xfea54c90, 0x1c5b0f68, ATMEL_FW_TYPE_502),
	PCMCIA_DEVICE_PROD_ID12_INFO("SMC", "2632W", 0xc4f8b18b, 0x30f38774, ATMEL_FW_TYPE_502D),
	PCMCIA_DEVICE_PROD_ID12_INFO("SMC", "2632W-V2", 0xc4f8b18b, 0x172d1377, ATMEL_FW_TYPE_502),
	PCMCIA_DEVICE_PROD_ID12_INFO("Wireless", "PC_CARD", 0xa407ecdd, 0x119f6314, ATMEL_FW_TYPE_502D),
	PCMCIA_DEVICE_PROD_ID12_INFO("WLAN", "802.11b PC CARD", 0x575c516c, 0xb1f6dbc4, ATMEL_FW_TYPE_502D),
	PCMCIA_DEVICE_PROD_ID12_INFO("LG", "LW2100N", 0xb474d43a, 0x6b1fec94, ATMEL_FW_TYPE_502E),
	PCMCIA_DEVICE_NULL
};

MODULE_DEVICE_TABLE(pcmcia, atmel_ids);

static struct pcmcia_driver atmel_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= "atmel_cs",
        },
	.probe          = atmel_probe,
	.remove		= atmel_detach,
	.id_table	= atmel_ids,
	.suspend	= atmel_suspend,
	.resume		= atmel_resume,
};

static int atmel_cs_init(void)
{
        return pcmcia_register_driver(&atmel_driver);
}

static void atmel_cs_cleanup(void)
{
        pcmcia_unregister_driver(&atmel_driver);
}



module_init(atmel_cs_init);
module_exit(atmel_cs_cleanup);
