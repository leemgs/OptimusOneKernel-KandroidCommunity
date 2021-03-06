

#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/parport.h>
#include <linux/parport_pc.h>
#include <linux/8250_pci.h>

enum parport_pc_pci_cards {
	titan_110l = 0,
	titan_210l,
	netmos_9xx5_combo,
	netmos_9855,
	netmos_9855_2p,
	avlab_1s1p,
	avlab_1s2p,
	avlab_2s1p,
	siig_1s1p_10x,
	siig_2s1p_10x,
	siig_2p1s_20x,
	siig_1s1p_20x,
	siig_2s1p_20x,
};


struct parport_pc_pci {
	int numports;
	struct { 
		int lo;
		int hi; 
	} addr[4];

	
	int (*preinit_hook) (struct pci_dev *pdev, struct parport_pc_pci *card,
				int autoirq, int autodma);

	
	void (*postinit_hook) (struct pci_dev *pdev,
				struct parport_pc_pci *card, int failed);
};

static int __devinit netmos_parallel_init(struct pci_dev *dev, struct parport_pc_pci *par, int autoirq, int autodma)
{
	
	if (dev->device == PCI_DEVICE_ID_NETMOS_9835 &&
			dev->subsystem_vendor == PCI_VENDOR_ID_IBM &&
			dev->subsystem_device == 0x0299)
		return -ENODEV;
	
	par->numports = (dev->subsystem_device & 0xf0) >> 4;
	if (par->numports > ARRAY_SIZE(par->addr))
		par->numports = ARRAY_SIZE(par->addr);
	
	if (par->addr[0].lo != 0)
		par->addr[0].lo = dev->subsystem_device & 0xf;
	return 0;
}

static struct parport_pc_pci cards[] __devinitdata = {
			{ 1, { { 3, -1 }, } },
			{ 1, { { 3, -1 }, } },
			{ 1, { { 2, -1 }, }, netmos_parallel_init },
			{ 1, { { 0, -1 }, }, netmos_parallel_init },
			{ 2, { { 0, -1 }, { 2, -1 }, } },
			{ 1, { { 1, 2}, } },
			{ 2, { { 1, 2}, { 3, 4 },} },
			{ 1, { { 2, 3}, } },
			{ 1, { { 3, 4 }, } },
			{ 1, { { 4, 5 }, } },
			{ 2, { { 1, 2 }, { 3, 4 }, } },
			{ 1, { { 1, 2 }, } },
			{ 1, { { 2, 3 }, } },
};

static struct pci_device_id parport_serial_pci_tbl[] = {
	
	{ PCI_VENDOR_ID_TITAN, PCI_DEVICE_ID_TITAN_110L,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, titan_110l },
	{ PCI_VENDOR_ID_TITAN, PCI_DEVICE_ID_TITAN_210L,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, titan_210l },
	{ PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9735,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, netmos_9xx5_combo },
	{ PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9745,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, netmos_9xx5_combo },
	{ PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9835,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, netmos_9xx5_combo },
	{ PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9845,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, netmos_9xx5_combo },
	{ PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9855,
	  0x1000, 0x0020, 0, 0, netmos_9855_2p },
	{ PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9855,
	  0x1000, 0x0022, 0, 0, netmos_9855_2p },
	{ PCI_VENDOR_ID_NETMOS, PCI_DEVICE_ID_NETMOS_9855,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, netmos_9855 },
	
	{ PCI_VENDOR_ID_AFAVLAB, 0x2110,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, avlab_1s1p },
	{ PCI_VENDOR_ID_AFAVLAB, 0x2111,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, avlab_1s1p },
	{ PCI_VENDOR_ID_AFAVLAB, 0x2112,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, avlab_1s1p },
	{ PCI_VENDOR_ID_AFAVLAB, 0x2140,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, avlab_1s2p },
	{ PCI_VENDOR_ID_AFAVLAB, 0x2141,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, avlab_1s2p },
	{ PCI_VENDOR_ID_AFAVLAB, 0x2142,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, avlab_1s2p },
	{ PCI_VENDOR_ID_AFAVLAB, 0x2160,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, avlab_2s1p },
	{ PCI_VENDOR_ID_AFAVLAB, 0x2161,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, avlab_2s1p },
	{ PCI_VENDOR_ID_AFAVLAB, 0x2162,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, avlab_2s1p },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_1S1P_10x_550,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_1s1p_10x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_1S1P_10x_650,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_1s1p_10x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_1S1P_10x_850,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_1s1p_10x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_2S1P_10x_550,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_2s1p_10x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_2S1P_10x_650,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_2s1p_10x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_2S1P_10x_850,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_2s1p_10x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_2P1S_20x_550,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_2p1s_20x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_2P1S_20x_650,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_2p1s_20x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_2P1S_20x_850,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_2p1s_20x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_1S1P_20x_550,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_2s1p_20x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_1S1P_20x_650,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_1s1p_20x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_1S1P_20x_850,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_1s1p_20x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_2S1P_20x_550,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_2s1p_20x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_2S1P_20x_650,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_2s1p_20x },
	{ PCI_VENDOR_ID_SIIG, PCI_DEVICE_ID_SIIG_2S1P_20x_850,
	  PCI_ANY_ID, PCI_ANY_ID, 0, 0, siig_2s1p_20x },

	{ 0, } 
};
MODULE_DEVICE_TABLE(pci,parport_serial_pci_tbl);


static struct pciserial_board pci_parport_serial_boards[] __devinitdata = {
	[titan_110l] = {
		.flags		= FL_BASE1 | FL_BASE_BARS,
		.num_ports	= 1,
		.base_baud	= 921600,
		.uart_offset	= 8,
	},
	[titan_210l] = {
		.flags		= FL_BASE1 | FL_BASE_BARS,
		.num_ports	= 2,
		.base_baud	= 921600,
		.uart_offset	= 8,
	},
	[netmos_9xx5_combo] = {
		.flags		= FL_BASE0 | FL_BASE_BARS,
		.num_ports	= 1,
		.base_baud	= 115200,
		.uart_offset	= 8,
	},
	[netmos_9855] = {
		.flags		= FL_BASE2 | FL_BASE_BARS,
		.num_ports	= 1,
		.base_baud	= 115200,
		.uart_offset	= 8,
	},
	[netmos_9855_2p] = {
		.flags		= FL_BASE4 | FL_BASE_BARS,
		.num_ports	= 1,
		.base_baud	= 115200,
		.uart_offset	= 8,
	},
	[avlab_1s1p] = { 
		.flags		= FL_BASE0 | FL_BASE_BARS,
		.num_ports	= 1,
		.base_baud	= 115200,
		.uart_offset	= 8,
	},
	[avlab_1s2p] = { 
		.flags		= FL_BASE0 | FL_BASE_BARS,
		.num_ports	= 1,
		.base_baud	= 115200,
		.uart_offset	= 8,
	},
	[avlab_2s1p] = { 
		.flags		= FL_BASE0 | FL_BASE_BARS,
		.num_ports	= 2,
		.base_baud	= 115200,
		.uart_offset	= 8,
	},
	[siig_1s1p_10x] = {
		.flags		= FL_BASE2,
		.num_ports	= 1,
		.base_baud	= 460800,
		.uart_offset	= 8,
	},
	[siig_2s1p_10x] = {
		.flags		= FL_BASE2,
		.num_ports	= 1,
		.base_baud	= 921600,
		.uart_offset	= 8,
	},
	[siig_2p1s_20x] = {
		.flags		= FL_BASE0,
		.num_ports	= 1,
		.base_baud	= 921600,
		.uart_offset	= 8,
	},
	[siig_1s1p_20x] = {
		.flags		= FL_BASE0,
		.num_ports	= 1,
		.base_baud	= 921600,
		.uart_offset	= 8,
	},
	[siig_2s1p_20x] = {
		.flags		= FL_BASE0,
		.num_ports	= 1,
		.base_baud	= 921600,
		.uart_offset	= 8,
	},
};

struct parport_serial_private {
	struct serial_private	*serial;
	int num_par;
	struct parport *port[PARPORT_MAX];
	struct parport_pc_pci par;
};


static int __devinit serial_register (struct pci_dev *dev,
				      const struct pci_device_id *id)
{
	struct parport_serial_private *priv = pci_get_drvdata (dev);
	struct pciserial_board *board;
	struct serial_private *serial;

	board = &pci_parport_serial_boards[id->driver_data];
	serial = pciserial_init_ports(dev, board);

	if (IS_ERR(serial))
		return PTR_ERR(serial);

	priv->serial = serial;
	return 0;
}


static int __devinit parport_register (struct pci_dev *dev,
				       const struct pci_device_id *id)
{
	struct parport_pc_pci *card;
	struct parport_serial_private *priv = pci_get_drvdata (dev);
	int n, success = 0;

	priv->par = cards[id->driver_data];
	card = &priv->par;
	if (card->preinit_hook &&
	    card->preinit_hook (dev, card, PARPORT_IRQ_NONE, PARPORT_DMA_NONE))
		return -ENODEV;

	for (n = 0; n < card->numports; n++) {
		struct parport *port;
		int lo = card->addr[n].lo;
		int hi = card->addr[n].hi;
		unsigned long io_lo, io_hi;
		int irq;

		if (priv->num_par == ARRAY_SIZE (priv->port)) {
			printk (KERN_WARNING
				"parport_serial: %s: only %zu parallel ports "
				"supported (%d reported)\n", pci_name (dev),
				ARRAY_SIZE(priv->port), card->numports);
			break;
		}

		io_lo = pci_resource_start (dev, lo);
		io_hi = 0;
		if ((hi >= 0) && (hi <= 6))
			io_hi = pci_resource_start (dev, hi);
		else if (hi > 6)
			io_lo += hi; 
		
		irq = dev->irq;
		if (irq == IRQ_NONE) {
			dev_dbg(&dev->dev,
			"PCI parallel port detected: I/O at %#lx(%#lx)\n",
				io_lo, io_hi);
			irq = PARPORT_IRQ_NONE;
		} else {
			dev_dbg(&dev->dev,
		"PCI parallel port detected: I/O at %#lx(%#lx), IRQ %d\n",
				io_lo, io_hi, irq);
			irq = PARPORT_IRQ_NONE;
		}
		port = parport_pc_probe_port (io_lo, io_hi, irq,
			      PARPORT_DMA_NONE, &dev->dev, IRQF_SHARED);
		if (port) {
			priv->port[priv->num_par++] = port;
			success = 1;
		}
	}

	if (card->postinit_hook)
		card->postinit_hook (dev, card, !success);

	return 0;
}

static int __devinit parport_serial_pci_probe (struct pci_dev *dev,
					       const struct pci_device_id *id)
{
	struct parport_serial_private *priv;
	int err;

	priv = kzalloc (sizeof *priv, GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	pci_set_drvdata (dev, priv);

	err = pci_enable_device (dev);
	if (err) {
		pci_set_drvdata (dev, NULL);
		kfree (priv);
		return err;
	}

	if (parport_register (dev, id)) {
		pci_set_drvdata (dev, NULL);
		kfree (priv);
		return -ENODEV;
	}

	if (serial_register (dev, id)) {
		int i;
		for (i = 0; i < priv->num_par; i++)
			parport_pc_unregister_port (priv->port[i]);
		pci_set_drvdata (dev, NULL);
		kfree (priv);
		return -ENODEV;
	}

	return 0;
}

static void __devexit parport_serial_pci_remove (struct pci_dev *dev)
{
	struct parport_serial_private *priv = pci_get_drvdata (dev);
	int i;

	pci_set_drvdata(dev, NULL);

	
	if (priv->serial)
		pciserial_remove_ports(priv->serial);

	
	for (i = 0; i < priv->num_par; i++)
		parport_pc_unregister_port (priv->port[i]);

	kfree (priv);
	return;
}

#ifdef CONFIG_PM
static int parport_serial_pci_suspend(struct pci_dev *dev, pm_message_t state)
{
	struct parport_serial_private *priv = pci_get_drvdata(dev);

	if (priv->serial)
		pciserial_suspend_ports(priv->serial);

	

	pci_save_state(dev);
	pci_set_power_state(dev, pci_choose_state(dev, state));
	return 0;
}

static int parport_serial_pci_resume(struct pci_dev *dev)
{
	struct parport_serial_private *priv = pci_get_drvdata(dev);
	int err;

	pci_set_power_state(dev, PCI_D0);
	pci_restore_state(dev);

	
	err = pci_enable_device(dev);
	if (err) {
		printk(KERN_ERR "parport_serial: %s: error enabling "
			"device for resume (%d)\n", pci_name(dev), err);
		return err;
	}

	if (priv->serial)
		pciserial_resume_ports(priv->serial);

	

	return 0;
}
#endif

static struct pci_driver parport_serial_pci_driver = {
	.name		= "parport_serial",
	.id_table	= parport_serial_pci_tbl,
	.probe		= parport_serial_pci_probe,
	.remove		= __devexit_p(parport_serial_pci_remove),
#ifdef CONFIG_PM
	.suspend	= parport_serial_pci_suspend,
	.resume		= parport_serial_pci_resume,
#endif
};


static int __init parport_serial_init (void)
{
	return pci_register_driver (&parport_serial_pci_driver);
}

static void __exit parport_serial_exit (void)
{
	pci_unregister_driver (&parport_serial_pci_driver);
	return;
}

MODULE_AUTHOR("Tim Waugh <twaugh@redhat.com>");
MODULE_DESCRIPTION("Driver for common parallel+serial multi-I/O PCI cards");
MODULE_LICENSE("GPL");

module_init(parport_serial_init);
module_exit(parport_serial_exit);
