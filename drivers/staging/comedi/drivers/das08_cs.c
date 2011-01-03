


#include "../comedidev.h"

#include <linux/delay.h>
#include <linux/pci.h>

#include "das08.h"


#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/ds.h>

static struct pcmcia_device *cur_dev = NULL;

#define thisboard ((const struct das08_board_struct *)dev->board_ptr)

static int das08_cs_attach(struct comedi_device *dev,
			   struct comedi_devconfig *it);

static struct comedi_driver driver_das08_cs = {
	.driver_name = "das08_cs",
	.module = THIS_MODULE,
	.attach = das08_cs_attach,
	.detach = das08_common_detach,
	.board_name = &das08_cs_boards[0].name,
	.num_names = ARRAY_SIZE(das08_cs_boards),
	.offset = sizeof(struct das08_board_struct),
};

static int das08_cs_attach(struct comedi_device *dev,
			   struct comedi_devconfig *it)
{
	int ret;
	unsigned long iobase;
	struct pcmcia_device *link = cur_dev;	

	ret = alloc_private(dev, sizeof(struct das08_private_struct));
	if (ret < 0)
		return ret;

	printk("comedi%d: das08_cs: ", dev->minor);
	

	if (thisboard->bustype == pcmcia) {
		if (link == NULL) {
			printk(" no pcmcia cards found\n");
			return -EIO;
		}
		iobase = link->io.BasePort1;
	} else {
		printk(" bug! board does not have PCMCIA bustype\n");
		return -EINVAL;
	}

	printk("\n");

	return das08_common_attach(dev, iobase);
}





#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0644);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
static const char *version =
    "das08.c pcmcia code (Frank Hess), modified from dummy_cs.c 1.31 2001/08/24 12:13:13 (David Hinds)";
#else
#define DEBUG(n, args...)
#endif


static void das08_pcmcia_config(struct pcmcia_device *link);
static void das08_pcmcia_release(struct pcmcia_device *link);
static int das08_pcmcia_suspend(struct pcmcia_device *p_dev);
static int das08_pcmcia_resume(struct pcmcia_device *p_dev);



static int das08_pcmcia_attach(struct pcmcia_device *);
static void das08_pcmcia_detach(struct pcmcia_device *);





static const dev_info_t dev_info = "pcm-das08";

struct local_info_t {
	struct pcmcia_device *link;
	dev_node_t node;
	int stop;
	struct bus_operations *bus;
};



static int das08_pcmcia_attach(struct pcmcia_device *link)
{
	struct local_info_t *local;

	DEBUG(0, "das08_pcmcia_attach()\n");

	
	local = kzalloc(sizeof(struct local_info_t), GFP_KERNEL);
	if (!local)
		return -ENOMEM;
	local->link = link;
	link->priv = local;

	
	link->irq.Attributes = IRQ_TYPE_EXCLUSIVE;
	link->irq.IRQInfo1 = IRQ_LEVEL_ID;
	link->irq.Handler = NULL;

	
	link->conf.Attributes = 0;
	link->conf.IntType = INT_MEMORY_AND_IO;

	cur_dev = link;

	das08_pcmcia_config(link);

	return 0;
}				



static void das08_pcmcia_detach(struct pcmcia_device *link)
{

	DEBUG(0, "das08_pcmcia_detach(0x%p)\n", link);

	if (link->dev_node) {
		((struct local_info_t *)link->priv)->stop = 1;
		das08_pcmcia_release(link);
	}

	
	if (link->priv)
		kfree(link->priv);

}				



static void das08_pcmcia_config(struct pcmcia_device *link)
{
	struct local_info_t *dev = link->priv;
	tuple_t tuple;
	cisparse_t parse;
	int last_fn, last_ret;
	u_char buf[64];
	cistpl_cftable_entry_t dflt = { 0 };

	DEBUG(0, "das08_pcmcia_config(0x%p)\n", link);

	
	tuple.DesiredTuple = CISTPL_CONFIG;
	tuple.Attributes = 0;
	tuple.TupleData = buf;
	tuple.TupleDataMax = sizeof(buf);
	tuple.TupleOffset = 0;
	last_fn = GetFirstTuple;

	last_ret = pcmcia_get_first_tuple(link, &tuple);
	if (last_ret)
		goto cs_failed;

	last_fn = GetTupleData;

	last_ret = pcmcia_get_tuple_data(link, &tuple);
	if (last_ret)
		goto cs_failed;

	last_fn = ParseTuple;

	last_ret = pcmcia_parse_tuple(&tuple, &parse);
	if (last_ret)
		goto cs_failed;

	link->conf.ConfigBase = parse.config.base;
	link->conf.Present = parse.config.rmask[0];

	
	tuple.DesiredTuple = CISTPL_CFTABLE_ENTRY;
	last_fn = GetFirstTuple;

	last_ret = pcmcia_get_first_tuple(link, &tuple);
	if (last_ret)
		goto cs_failed;

	while (1) {
		cistpl_cftable_entry_t *cfg = &(parse.cftable_entry);

		last_ret = pcmcia_get_tuple_data(link, &tuple);
		if (last_ret)
			goto next_entry;

		last_ret = pcmcia_parse_tuple(&tuple, &parse);
		if (last_ret)
			goto next_entry;

		if (cfg->flags & CISTPL_CFTABLE_DEFAULT)
			dflt = *cfg;
		if (cfg->index == 0)
			goto next_entry;
		link->conf.ConfigIndex = cfg->index;

		

		
		if (cfg->irq.IRQInfo1 || dflt.irq.IRQInfo1)
			link->conf.Attributes |= CONF_ENABLE_IRQ;

		
		link->io.NumPorts1 = link->io.NumPorts2 = 0;
		if ((cfg->io.nwin > 0) || (dflt.io.nwin > 0)) {
			cistpl_io_t *io = (cfg->io.nwin) ? &cfg->io : &dflt.io;
			link->io.Attributes1 = IO_DATA_PATH_WIDTH_AUTO;
			if (!(io->flags & CISTPL_IO_8BIT))
				link->io.Attributes1 = IO_DATA_PATH_WIDTH_16;
			if (!(io->flags & CISTPL_IO_16BIT))
				link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
			link->io.IOAddrLines = io->flags & CISTPL_IO_LINES_MASK;
			link->io.BasePort1 = io->win[0].base;
			link->io.NumPorts1 = io->win[0].len;
			if (io->nwin > 1) {
				link->io.Attributes2 = link->io.Attributes1;
				link->io.BasePort2 = io->win[1].base;
				link->io.NumPorts2 = io->win[1].len;
			}
			
			if (pcmcia_request_io(link, &link->io) != 0)
				goto next_entry;
		}

		
		break;

next_entry:
		last_fn = GetNextTuple;

		last_ret = pcmcia_get_next_tuple(link, &tuple);
		if (last_ret)
			goto cs_failed;
	}

	if (link->conf.Attributes & CONF_ENABLE_IRQ) {
		last_fn = RequestIRQ;
		last_ret = pcmcia_request_irq(link, &link->irq);
		if (last_ret)
			goto cs_failed;
	}

	
	last_fn = RequestConfiguration;
	last_ret = pcmcia_request_configuration(link, &link->conf);
	if (last_ret)
		goto cs_failed;

	
	sprintf(dev->node.dev_name, "pcm-das08");
	dev->node.major = dev->node.minor = 0;
	link->dev_node = &dev->node;

	
	printk(KERN_INFO "%s: index 0x%02x",
	       dev->node.dev_name, link->conf.ConfigIndex);
	if (link->conf.Attributes & CONF_ENABLE_IRQ)
		printk(", irq %u", link->irq.AssignedIRQ);
	if (link->io.NumPorts1)
		printk(", io 0x%04x-0x%04x", link->io.BasePort1,
		       link->io.BasePort1 + link->io.NumPorts1 - 1);
	if (link->io.NumPorts2)
		printk(" & 0x%04x-0x%04x", link->io.BasePort2,
		       link->io.BasePort2 + link->io.NumPorts2 - 1);
	printk("\n");

	return;

cs_failed:
	cs_error(link, last_fn, last_ret);
	das08_pcmcia_release(link);

}				



static void das08_pcmcia_release(struct pcmcia_device *link)
{
	DEBUG(0, "das08_pcmcia_release(0x%p)\n", link);
	pcmcia_disable_device(link);
}				



static int das08_pcmcia_suspend(struct pcmcia_device *link)
{
	struct local_info_t *local = link->priv;
	
	local->stop = 1;

	return 0;
}				

static int das08_pcmcia_resume(struct pcmcia_device *link)
{
	struct local_info_t *local = link->priv;

	local->stop = 0;
	return 0;
}				



static struct pcmcia_device_id das08_cs_id_table[] = {
	PCMCIA_DEVICE_MANF_CARD(0x01c5, 0x4001),
	PCMCIA_DEVICE_NULL
};

MODULE_DEVICE_TABLE(pcmcia, das08_cs_id_table);

struct pcmcia_driver das08_cs_driver = {
	.probe = das08_pcmcia_attach,
	.remove = das08_pcmcia_detach,
	.suspend = das08_pcmcia_suspend,
	.resume = das08_pcmcia_resume,
	.id_table = das08_cs_id_table,
	.owner = THIS_MODULE,
	.drv = {
		.name = dev_info,
		},
};

static int __init init_das08_pcmcia_cs(void)
{
	DEBUG(0, "%s\n", version);
	pcmcia_register_driver(&das08_cs_driver);
	return 0;
}

static void __exit exit_das08_pcmcia_cs(void)
{
	DEBUG(0, "das08_pcmcia_cs: unloading\n");
	pcmcia_unregister_driver(&das08_cs_driver);
}

static int __init das08_cs_init_module(void)
{
	int ret;

	ret = init_das08_pcmcia_cs();
	if (ret < 0)
		return ret;

	return comedi_driver_register(&driver_das08_cs);
}

static void __exit das08_cs_exit_module(void)
{
	exit_das08_pcmcia_cs();
	comedi_driver_unregister(&driver_das08_cs);
}

MODULE_LICENSE("GPL");
module_init(das08_cs_init_module);
module_exit(das08_cs_exit_module);
