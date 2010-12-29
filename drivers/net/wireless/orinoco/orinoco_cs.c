

#define DRIVER_NAME "orinoco_cs"
#define PFX DRIVER_NAME ": "

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>

#include "orinoco.h"





MODULE_AUTHOR("David Gibson <hermes@gibson.dropbear.id.au>");
MODULE_DESCRIPTION("Driver for PCMCIA Lucent Orinoco,"
		   " Prism II based and similar wireless cards");
MODULE_LICENSE("Dual MPL/GPL");




static int ignore_cis_vcc; 
module_param(ignore_cis_vcc, int, 0);
MODULE_PARM_DESC(ignore_cis_vcc, "Allow voltage mismatch between card and socket");






struct orinoco_pccard {
	struct pcmcia_device	*p_dev;
	dev_node_t node;

	
	
	unsigned long hard_reset_in_progress;
};






static int orinoco_cs_config(struct pcmcia_device *link);
static void orinoco_cs_release(struct pcmcia_device *link);
static void orinoco_cs_detach(struct pcmcia_device *p_dev);





static int
orinoco_cs_hard_reset(struct orinoco_private *priv)
{
	struct orinoco_pccard *card = priv->card;
	struct pcmcia_device *link = card->p_dev;
	int err;

	
	set_bit(0, &card->hard_reset_in_progress);

	err = pcmcia_reset_card(link->socket);
	if (err)
		return err;

	msleep(100);
	clear_bit(0, &card->hard_reset_in_progress);

	return 0;
}






static int
orinoco_cs_probe(struct pcmcia_device *link)
{
	struct orinoco_private *priv;
	struct orinoco_pccard *card;

	priv = alloc_orinocodev(sizeof(*card), &handle_to_dev(link),
				orinoco_cs_hard_reset, NULL);
	if (!priv)
		return -ENOMEM;
	card = priv->card;

	
	card->p_dev = link;
	link->priv = priv;

	
	link->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING | IRQ_HANDLE_PRESENT;
	link->irq.IRQInfo1 = IRQ_LEVEL_ID;
	link->irq.Handler = orinoco_interrupt;
	link->irq.Instance = priv;

	
	link->conf.Attributes = 0;
	link->conf.IntType = INT_MEMORY_AND_IO;

	return orinoco_cs_config(link);
}				


static void orinoco_cs_detach(struct pcmcia_device *link)
{
	struct orinoco_private *priv = link->priv;

	if (link->dev_node)
		orinoco_if_del(priv);

	orinoco_cs_release(link);

	free_orinocodev(priv);
}				



#define CS_CHECK(fn, ret) do { \
	last_fn = (fn); \
	if ((last_ret = (ret)) != 0) \
		goto cs_failed; \
} while (0)

static int orinoco_cs_config_check(struct pcmcia_device *p_dev,
				   cistpl_cftable_entry_t *cfg,
				   cistpl_cftable_entry_t *dflt,
				   unsigned int vcc,
				   void *priv_data)
{
	if (cfg->index == 0)
		goto next_entry;

	
	
	if (cfg->vcc.present & (1 << CISTPL_POWER_VNOM)) {
		if (vcc != cfg->vcc.param[CISTPL_POWER_VNOM] / 10000) {
			DEBUG(2, "%s: Vcc mismatch (vcc = %d, CIS = %d)\n",
			      __func__, vcc,
			      cfg->vcc.param[CISTPL_POWER_VNOM] / 10000);
			if (!ignore_cis_vcc)
				goto next_entry;
		}
	} else if (dflt->vcc.present & (1 << CISTPL_POWER_VNOM)) {
		if (vcc != dflt->vcc.param[CISTPL_POWER_VNOM] / 10000) {
			DEBUG(2, "%s: Vcc mismatch (vcc = %d, CIS = %d)\n",
			      __func__, vcc,
			      dflt->vcc.param[CISTPL_POWER_VNOM] / 10000);
			if (!ignore_cis_vcc)
				goto next_entry;
		}
	}

	if (cfg->vpp1.present & (1 << CISTPL_POWER_VNOM))
		p_dev->conf.Vpp =
			cfg->vpp1.param[CISTPL_POWER_VNOM] / 10000;
	else if (dflt->vpp1.present & (1 << CISTPL_POWER_VNOM))
		p_dev->conf.Vpp =
			dflt->vpp1.param[CISTPL_POWER_VNOM] / 10000;

	
	p_dev->conf.Attributes |= CONF_ENABLE_IRQ;

	
	p_dev->io.NumPorts1 = p_dev->io.NumPorts2 = 0;
	if ((cfg->io.nwin > 0) || (dflt->io.nwin > 0)) {
		cistpl_io_t *io = (cfg->io.nwin) ? &cfg->io : &dflt->io;
		p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
		if (!(io->flags & CISTPL_IO_8BIT))
			p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_16;
		if (!(io->flags & CISTPL_IO_16BIT))
			p_dev->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
		p_dev->io.IOAddrLines = io->flags & CISTPL_IO_LINES_MASK;
		p_dev->io.BasePort1 = io->win[0].base;
		p_dev->io.NumPorts1 = io->win[0].len;
		if (io->nwin > 1) {
			p_dev->io.Attributes2 = p_dev->io.Attributes1;
			p_dev->io.BasePort2 = io->win[1].base;
			p_dev->io.NumPorts2 = io->win[1].len;
		}

		
		if (pcmcia_request_io(p_dev, &p_dev->io) != 0)
			goto next_entry;
	}
	return 0;

next_entry:
	pcmcia_disable_device(p_dev);
	return -ENODEV;
};

static int
orinoco_cs_config(struct pcmcia_device *link)
{
	struct orinoco_private *priv = link->priv;
	struct orinoco_pccard *card = priv->card;
	hermes_t *hw = &priv->hw;
	int last_fn, last_ret;
	void __iomem *mem;

	
	last_ret = pcmcia_loop_config(link, orinoco_cs_config_check, NULL);
	if (last_ret) {
		if (!ignore_cis_vcc)
			printk(KERN_ERR PFX "GetNextTuple(): No matching "
			       "CIS configuration.  Maybe you need the "
			       "ignore_cis_vcc=1 parameter.\n");
		cs_error(link, RequestIO, last_ret);
		goto failed;
	}

	
	CS_CHECK(RequestIRQ, pcmcia_request_irq(link, &link->irq));

	
	mem = ioport_map(link->io.BasePort1, link->io.NumPorts1);
	if (!mem)
		goto cs_failed;

	hermes_struct_init(hw, mem, HERMES_16BIT_REGSPACING);

	
	CS_CHECK(RequestConfiguration,
		 pcmcia_request_configuration(link, &link->conf));

	
	card->node.major = card->node.minor = 0;

	
	if (orinoco_init(priv) != 0) {
		printk(KERN_ERR PFX "orinoco_init() failed\n");
		goto failed;
	}

	
	if (orinoco_if_add(priv, link->io.BasePort1,
			   link->irq.AssignedIRQ) != 0) {
		printk(KERN_ERR PFX "orinoco_if_add() failed\n");
		goto failed;
	}

	
	strcpy(card->node.dev_name, priv->ndev->name);
	link->dev_node = &card->node; 
	return 0;

 cs_failed:
	cs_error(link, last_fn, last_ret);

 failed:
	orinoco_cs_release(link);
	return -ENODEV;
}				


static void
orinoco_cs_release(struct pcmcia_device *link)
{
	struct orinoco_private *priv = link->priv;
	unsigned long flags;

	
	spin_lock_irqsave(&priv->lock, flags);
	priv->hw_unavailable++;
	spin_unlock_irqrestore(&priv->lock, flags);

	pcmcia_disable_device(link);
	if (priv->hw.iobase)
		ioport_unmap(priv->hw.iobase);
}				

static int orinoco_cs_suspend(struct pcmcia_device *link)
{
	struct orinoco_private *priv = link->priv;
	struct orinoco_pccard *card = priv->card;

	
	if (!test_bit(0, &card->hard_reset_in_progress))
		orinoco_down(priv);

	return 0;
}

static int orinoco_cs_resume(struct pcmcia_device *link)
{
	struct orinoco_private *priv = link->priv;
	struct orinoco_pccard *card = priv->card;
	int err = 0;

	if (!test_bit(0, &card->hard_reset_in_progress))
		err = orinoco_up(priv);

	return err;
}







static char version[] __initdata = DRIVER_NAME " " DRIVER_VERSION
	" (David Gibson <hermes@gibson.dropbear.id.au>, "
	"Pavel Roskin <proski@gnu.org>, et al)";

static struct pcmcia_device_id orinoco_cs_ids[] = {
	PCMCIA_DEVICE_MANF_CARD(0x000b, 0x7100), 
	PCMCIA_DEVICE_MANF_CARD(0x000b, 0x7300), 
	PCMCIA_DEVICE_MANF_CARD(0x0089, 0x0002), 
	PCMCIA_DEVICE_MANF_CARD(0x0101, 0x0777), 
	PCMCIA_DEVICE_MANF_CARD(0x0126, 0x8000), 
	PCMCIA_DEVICE_MANF_CARD(0x0138, 0x0002), 
	PCMCIA_DEVICE_MANF_CARD(0x0156, 0x0002), 
	PCMCIA_DEVICE_MANF_CARD(0x016b, 0x0001), 
	PCMCIA_DEVICE_MANF_CARD(0x01eb, 0x080a), 
	PCMCIA_DEVICE_MANF_CARD(0x01ff, 0x0008), 
	PCMCIA_DEVICE_MANF_CARD(0x0250, 0x0002), 
	PCMCIA_DEVICE_MANF_CARD(0x0261, 0x0002), 
	PCMCIA_DEVICE_MANF_CARD(0x0268, 0x0001), 
	PCMCIA_DEVICE_MANF_CARD(0x0268, 0x0003), 
	PCMCIA_DEVICE_MANF_CARD(0x026f, 0x0305), 
	PCMCIA_DEVICE_MANF_CARD(0x0274, 0x1612), 
	PCMCIA_DEVICE_MANF_CARD(0x0274, 0x1613), 
	PCMCIA_DEVICE_MANF_CARD(0x028a, 0x0002), 
	PCMCIA_DEVICE_MANF_CARD(0x028a, 0x0673), 
	PCMCIA_DEVICE_MANF_CARD(0x02aa, 0x0002), 
	PCMCIA_DEVICE_MANF_CARD(0x02ac, 0x0002), 
	PCMCIA_DEVICE_MANF_CARD(0x02ac, 0x3021), 
	PCMCIA_DEVICE_MANF_CARD(0x14ea, 0xb001), 
	PCMCIA_DEVICE_MANF_CARD(0x50c2, 0x7300), 
	PCMCIA_DEVICE_MANF_CARD(0x9005, 0x0021), 
	PCMCIA_DEVICE_MANF_CARD(0xc001, 0x0008), 
	PCMCIA_DEVICE_MANF_CARD(0xc250, 0x0002), 
	PCMCIA_DEVICE_MANF_CARD(0xd601, 0x0002), 
	PCMCIA_DEVICE_MANF_CARD(0xd601, 0x0005), 
	PCMCIA_DEVICE_PROD_ID12(" ", "IEEE 802.11 Wireless LAN/PC Card", 0x3b6e20c8, 0xefccafe9),
	PCMCIA_DEVICE_PROD_ID12("3Com", "3CRWE737A AirConnect Wireless LAN PC Card", 0x41240e5b, 0x56010af3),
	PCMCIA_DEVICE_PROD_ID12("ACTIONTEC", "PRISM Wireless LAN PC Card", 0x393089da, 0xa71e69d5),
	PCMCIA_DEVICE_PROD_ID12("Addtron", "AWP-100 Wireless PCMCIA", 0xe6ec52ce, 0x08649af2),
	PCMCIA_DEVICE_PROD_ID123("AIRVAST", "IEEE 802.11b Wireless PCMCIA Card", "HFA3863", 0xea569531, 0x4bcb9645, 0x355cb092),
	PCMCIA_DEVICE_PROD_ID12("Allied Telesyn", "AT-WCL452 Wireless PCMCIA Radio", 0x5cd01705, 0x4271660f),
	PCMCIA_DEVICE_PROD_ID12("ASUS", "802_11b_PC_CARD_25", 0x78fc06ee, 0xdb9aa842),
	PCMCIA_DEVICE_PROD_ID12("ASUS", "802_11B_CF_CARD_25", 0x78fc06ee, 0x45a50c1e),
	PCMCIA_DEVICE_PROD_ID12("Avaya Communication", "Avaya Wireless PC Card", 0xd8a43b78, 0x0d341169),
	PCMCIA_DEVICE_PROD_ID12("BENQ", "AWL100 PCMCIA ADAPTER", 0x35dadc74, 0x01f7fedb),
	PCMCIA_DEVICE_PROD_ID12("BUFFALO", "WLI-PCM-L11G", 0x2decece3, 0xf57ca4b3),
	PCMCIA_DEVICE_PROD_ID12("BUFFALO", "WLI-CF-S11G", 0x2decece3, 0x82067c18),
	PCMCIA_DEVICE_PROD_ID12("Cabletron", "RoamAbout 802.11 DS", 0x32d445f5, 0xedeffd90),
	PCMCIA_DEVICE_PROD_ID12("Compaq", "WL200_11Mbps_Wireless_PCI_Card", 0x54f7c49c, 0x15a75e5b),
	PCMCIA_DEVICE_PROD_ID123("corega", "WL PCCL-11", "ISL37300P", 0x0a21501a, 0x59868926, 0xc9049a39),
	PCMCIA_DEVICE_PROD_ID12("corega K.K.", "Wireless LAN PCC-11", 0x5261440f, 0xa6405584),
	PCMCIA_DEVICE_PROD_ID12("corega K.K.", "Wireless LAN PCCA-11", 0x5261440f, 0xdf6115f9),
	PCMCIA_DEVICE_PROD_ID12("corega_K.K.", "Wireless_LAN_PCCB-11", 0x29e33311, 0xee7a27ae),
	PCMCIA_DEVICE_PROD_ID12("D", "Link DRC-650 11Mbps WLAN Card", 0x71b18589, 0xf144e3ac),
	PCMCIA_DEVICE_PROD_ID12("D", "Link DWL-650 11Mbps WLAN Card", 0x71b18589, 0xb6f1b0ab),
	PCMCIA_DEVICE_PROD_ID12("D-Link Corporation", "D-Link DWL-650H 11Mbps WLAN Adapter", 0xef544d24, 0xcd8ea916),
	PCMCIA_DEVICE_PROD_ID12("Digital Data Communications", "WPC-0100", 0xfdd73470, 0xe0b6f146),
	PCMCIA_DEVICE_PROD_ID12("ELSA", "AirLancer MC-11", 0x4507a33a, 0xef54f0e3),
	PCMCIA_DEVICE_PROD_ID12("HyperLink", "Wireless PC Card 11Mbps", 0x56cc3f1a, 0x0bcf220c),
	PCMCIA_DEVICE_PROD_ID123("Instant Wireless ", " Network PC CARD", "Version 01.02", 0x11d901af, 0x6e9bd926, 0x4b74baa0),
	PCMCIA_DEVICE_PROD_ID12("Intel", "PRO/Wireless 2011 LAN PC Card", 0x816cc815, 0x07f58077),
	PCMCIA_DEVICE_PROD_ID12("INTERSIL", "HFA384x/IEEE", 0x74c5e40d, 0xdb472a18),
	PCMCIA_DEVICE_PROD_ID12("INTERSIL", "I-GATE 11M PC Card / PC Card plus", 0x74c5e40d, 0x8304ff77),
	PCMCIA_DEVICE_PROD_ID12("Intersil", "PRISM 2_5 PCMCIA ADAPTER", 0x4b801a17, 0x6345a0bf),
	PCMCIA_DEVICE_PROD_ID123("Intersil", "PRISM Freedom PCMCIA Adapter", "ISL37100P", 0x4b801a17, 0xf222ec2d, 0x630d52b2),
	PCMCIA_DEVICE_PROD_ID12("LeArtery", "SYNCBYAIR 11Mbps Wireless LAN PC Card", 0x7e3b326a, 0x49893e92),
	PCMCIA_DEVICE_PROD_ID12("Linksys", "Wireless CompactFlash Card", 0x0733cc81, 0x0c52f395),
	PCMCIA_DEVICE_PROD_ID12("Lucent Technologies", "WaveLAN/IEEE", 0x23eb9949, 0xc562e72a),
	PCMCIA_DEVICE_PROD_ID12("MELCO", "WLI-PCM-L11", 0x481e0094, 0x7360e410),
	PCMCIA_DEVICE_PROD_ID12("MELCO", "WLI-PCM-L11G", 0x481e0094, 0xf57ca4b3),
	PCMCIA_DEVICE_PROD_ID12("Microsoft", "Wireless Notebook Adapter MN-520", 0x5961bf85, 0x6eec8c01),
	PCMCIA_DEVICE_PROD_ID12("NCR", "WaveLAN/IEEE", 0x24358cd4, 0xc562e72a),
	PCMCIA_DEVICE_PROD_ID12("NETGEAR MA401 Wireless PC", "Card", 0xa37434e9, 0x9762e8f1),
	PCMCIA_DEVICE_PROD_ID12("NETGEAR MA401RA Wireless PC", "Card", 0x0306467f, 0x9762e8f1),
	PCMCIA_DEVICE_PROD_ID12("Nortel Networks", "emobility 802.11 Wireless LAN PC Card", 0x2d617ea0, 0x88cd5767),
	PCMCIA_DEVICE_PROD_ID12("OEM", "PRISM2 IEEE 802.11 PC-Card", 0xfea54c90, 0x48f2bdd6),
	PCMCIA_DEVICE_PROD_ID12("OTC", "Wireless AirEZY 2411-PCC WLAN Card", 0x4ac44287, 0x235a6bed),
	PCMCIA_DEVICE_PROD_ID123("PCMCIA", "11M WLAN Card v2.5", "ISL37300P", 0x281f1c5d, 0x6e440487, 0xc9049a39),
	PCMCIA_DEVICE_PROD_ID12("PLANEX", "GeoWave/GW-CF110", 0x209f40ab, 0xd9715264),
	PCMCIA_DEVICE_PROD_ID12("PLANEX", "GeoWave/GW-NS110", 0x209f40ab, 0x46263178),
	PCMCIA_DEVICE_PROD_ID12("PROXIM", "LAN PC CARD HARMONY 80211B", 0xc6536a5e, 0x090c3cd9),
	PCMCIA_DEVICE_PROD_ID12("PROXIM", "LAN PCI CARD HARMONY 80211B", 0xc6536a5e, 0x9f494e26),
	PCMCIA_DEVICE_PROD_ID12("SAMSUNG", "11Mbps WLAN Card", 0x43d74cb4, 0x579bd91b),
	PCMCIA_DEVICE_PROD_ID12("SMC", "SMC2532W-B EliteConnect Wireless Adapter", 0xc4f8b18b, 0x196bd757),
	PCMCIA_DEVICE_PROD_ID12("SMC", "SMC2632W", 0xc4f8b18b, 0x474a1f2a),
	PCMCIA_DEVICE_PROD_ID12("Symbol Technologies", "LA4111 Spectrum24 Wireless LAN PC Card", 0x3f02b4d6, 0x3663cb0e),
	PCMCIA_DEVICE_PROD_ID123("The Linksys Group, Inc.", "Instant Wireless Network PC Card", "ISL37300P", 0xa5f472c2, 0x590eb502, 0xc9049a39),
	PCMCIA_DEVICE_PROD_ID12("ZoomAir 11Mbps High", "Rate wireless Networking", 0x273fe3db, 0x32a1eaee),
	PCMCIA_DEVICE_NULL,
};
MODULE_DEVICE_TABLE(pcmcia, orinoco_cs_ids);

static struct pcmcia_driver orinoco_driver = {
	.owner		= THIS_MODULE,
	.drv		= {
		.name	= DRIVER_NAME,
	},
	.probe		= orinoco_cs_probe,
	.remove		= orinoco_cs_detach,
	.id_table       = orinoco_cs_ids,
	.suspend	= orinoco_cs_suspend,
	.resume		= orinoco_cs_resume,
};

static int __init
init_orinoco_cs(void)
{
	printk(KERN_DEBUG "%s\n", version);

	return pcmcia_register_driver(&orinoco_driver);
}

static void __exit
exit_orinoco_cs(void)
{
	pcmcia_unregister_driver(&orinoco_driver);
}

module_init(init_orinoco_cs);
module_exit(exit_orinoco_cs);
