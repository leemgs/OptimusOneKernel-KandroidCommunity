




#undef LABPC_DEBUG
			    

#include "../comedidev.h"

#include <linux/delay.h>

#include "8253.h"
#include "8255.h"
#include "comedi_fc.h"
#include "ni_labpc.h"

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>

static struct pcmcia_device *pcmcia_cur_dev = NULL;

static int labpc_attach(struct comedi_device *dev, struct comedi_devconfig *it);

static const struct labpc_board_struct labpc_cs_boards[] = {
	{
	 .name = "daqcard-1200",
	 .device_id = 0x103,	
	 .ai_speed = 10000,
	 .bustype = pcmcia_bustype,
	 .register_layout = labpc_1200_layout,
	 .has_ao = 1,
	 .ai_range_table = &range_labpc_1200_ai,
	 .ai_range_code = labpc_1200_ai_gain_bits,
	 .ai_range_is_unipolar = labpc_1200_is_unipolar,
	 .ai_scan_up = 0,
	 .memory_mapped_io = 0,
	 },
	
	{
	 .name = "ni_labpc_cs",
	 .device_id = 0x103,
	 .ai_speed = 10000,
	 .bustype = pcmcia_bustype,
	 .register_layout = labpc_1200_layout,
	 .has_ao = 1,
	 .ai_range_table = &range_labpc_1200_ai,
	 .ai_range_code = labpc_1200_ai_gain_bits,
	 .ai_range_is_unipolar = labpc_1200_is_unipolar,
	 .ai_scan_up = 0,
	 .memory_mapped_io = 0,
	 },
};


#define thisboard ((const struct labpc_board_struct *)dev->board_ptr)

static struct comedi_driver driver_labpc_cs = {
	.driver_name = "ni_labpc_cs",
	.module = THIS_MODULE,
	.attach = &labpc_attach,
	.detach = &labpc_common_detach,
	.num_names = ARRAY_SIZE(labpc_cs_boards),
	.board_name = &labpc_cs_boards[0].name,
	.offset = sizeof(struct labpc_board_struct),
};

static int labpc_attach(struct comedi_device *dev, struct comedi_devconfig *it)
{
	unsigned long iobase = 0;
	unsigned int irq = 0;
	struct pcmcia_device *link;

	
	if (alloc_private(dev, sizeof(struct labpc_private)) < 0)
		return -ENOMEM;

	
	switch (thisboard->bustype) {
	case pcmcia_bustype:
		link = pcmcia_cur_dev;	
		if (!link)
			return -EIO;
		iobase = link->io.BasePort1;
		irq = link->irq.AssignedIRQ;
		break;
	default:
		printk("bug! couldn't determine board type\n");
		return -EINVAL;
		break;
	}
	return labpc_common_attach(dev, iobase, irq, 0);
}


#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0644);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
static const char *version =
    "ni_labpc.c, based on dummy_cs.c 1.31 2001/08/24 12:13:13";
#else
#define DEBUG(n, args...)
#endif





static void labpc_config(struct pcmcia_device *link);
static void labpc_release(struct pcmcia_device *link);
static int labpc_cs_suspend(struct pcmcia_device *p_dev);
static int labpc_cs_resume(struct pcmcia_device *p_dev);



static int labpc_cs_attach(struct pcmcia_device *);
static void labpc_cs_detach(struct pcmcia_device *);





static const dev_info_t dev_info = "daqcard-1200";

struct local_info_t {
	struct pcmcia_device *link;
	dev_node_t node;
	int stop;
	struct bus_operations *bus;
};



static int labpc_cs_attach(struct pcmcia_device *link)
{
	struct local_info_t *local;

	DEBUG(0, "labpc_cs_attach()\n");

	
	local = kzalloc(sizeof(struct local_info_t), GFP_KERNEL);
	if (!local)
		return -ENOMEM;
	local->link = link;
	link->priv = local;

	
	link->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING | IRQ_FORCED_PULSE;
	link->irq.IRQInfo1 = IRQ_INFO2_VALID | IRQ_PULSE_ID;
	link->irq.Handler = NULL;

	
	link->conf.Attributes = 0;
	link->conf.IntType = INT_MEMORY_AND_IO;

	pcmcia_cur_dev = link;

	labpc_config(link);

	return 0;
}				



static void labpc_cs_detach(struct pcmcia_device *link)
{
	DEBUG(0, "labpc_cs_detach(0x%p)\n", link);

	
	if (link->dev_node) {
		((struct local_info_t *)link->priv)->stop = 1;
		labpc_release(link);
	}

	
	if (link->priv)
		kfree(link->priv);

}				



static void labpc_config(struct pcmcia_device *link)
{
	struct local_info_t *dev = link->priv;
	tuple_t tuple;
	cisparse_t parse;
	int last_ret;
	u_char buf[64];
	win_req_t req;
	memreq_t map;
	cistpl_cftable_entry_t dflt = { 0 };

	DEBUG(0, "labpc_config(0x%p)\n", link);

	
	tuple.DesiredTuple = CISTPL_CONFIG;
	tuple.Attributes = 0;
	tuple.TupleData = buf;
	tuple.TupleDataMax = sizeof(buf);
	tuple.TupleOffset = 0;

	last_ret = pcmcia_get_first_tuple(link, &tuple);
	if (last_ret) {
		cs_error(link, GetFirstTuple, last_ret);
		goto cs_failed;
	}

	last_ret = pcmcia_get_tuple_data(link, &tuple);
	if (last_ret) {
		cs_error(link, GetTupleData, last_ret);
		goto cs_failed;
	}

	last_ret = pcmcia_parse_tuple(&tuple, &parse);
	if (last_ret) {
		cs_error(link, ParseTuple, last_ret);
		goto cs_failed;
	}
	link->conf.ConfigBase = parse.config.base;
	link->conf.Present = parse.config.rmask[0];

	
	tuple.DesiredTuple = CISTPL_CFTABLE_ENTRY;
	last_ret = pcmcia_get_first_tuple(link, &tuple);
	if (last_ret) {
		cs_error(link, GetFirstTuple, last_ret);
		goto cs_failed;
	}
	while (1) {
		cistpl_cftable_entry_t *cfg = &(parse.cftable_entry);
		if (pcmcia_get_tuple_data(link, &tuple))
			goto next_entry;
		if (pcmcia_parse_tuple(&tuple, &parse))
			goto next_entry;

		if (cfg->flags & CISTPL_CFTABLE_DEFAULT)
			dflt = *cfg;
		if (cfg->index == 0)
			goto next_entry;
		link->conf.ConfigIndex = cfg->index;

		
		if (cfg->flags & CISTPL_CFTABLE_AUDIO) {
			link->conf.Attributes |= CONF_ENABLE_SPKR;
			link->conf.Status = CCSR_AUDIO_ENA;
		}

		
		if (cfg->irq.IRQInfo1 || dflt.irq.IRQInfo1)
			link->conf.Attributes |= CONF_ENABLE_IRQ;

		
		link->io.NumPorts1 = link->io.NumPorts2 = 0;
		if ((cfg->io.nwin > 0) || (dflt.io.nwin > 0)) {
			cistpl_io_t *io = (cfg->io.nwin) ? &cfg->io : &dflt.io;
			link->io.Attributes1 = IO_DATA_PATH_WIDTH_8;
			link->io.IOAddrLines = io->flags & CISTPL_IO_LINES_MASK;
			link->io.BasePort1 = io->win[0].base;
			link->io.NumPorts1 = io->win[0].len;
			if (io->nwin > 1) {
				link->io.Attributes2 = link->io.Attributes1;
				link->io.BasePort2 = io->win[1].base;
				link->io.NumPorts2 = io->win[1].len;
			}
			
			if (pcmcia_request_io(link, &link->io))
				goto next_entry;
		}

		if ((cfg->mem.nwin > 0) || (dflt.mem.nwin > 0)) {
			cistpl_mem_t *mem =
			    (cfg->mem.nwin) ? &cfg->mem : &dflt.mem;
			req.Attributes = WIN_DATA_WIDTH_16 | WIN_MEMORY_TYPE_CM;
			req.Attributes |= WIN_ENABLE;
			req.Base = mem->win[0].host_addr;
			req.Size = mem->win[0].len;
			if (req.Size < 0x1000)
				req.Size = 0x1000;
			req.AccessSpeed = 0;
			link->win = (window_handle_t) link;
			if (pcmcia_request_window(&link, &req, &link->win))
				goto next_entry;
			map.Page = 0;
			map.CardOffset = mem->win[0].card_addr;
			if (pcmcia_map_mem_page(link->win, &map))
				goto next_entry;
		}
		
		break;

next_entry:
		last_ret = pcmcia_get_next_tuple(link, &tuple);
		if (last_ret) {
			cs_error(link, GetNextTuple, last_ret);
			goto cs_failed;
		}
	}

	
	if (link->conf.Attributes & CONF_ENABLE_IRQ) {
		last_ret = pcmcia_request_irq(link, &link->irq);
		if (last_ret) {
			cs_error(link, RequestIRQ, last_ret);
			goto cs_failed;
		}
	}

	
	last_ret = pcmcia_request_configuration(link, &link->conf);
	if (last_ret) {
		cs_error(link, RequestConfiguration, last_ret);
		goto cs_failed;
	}

	
	sprintf(dev->node.dev_name, "daqcard-1200");
	dev->node.major = dev->node.minor = 0;
	link->dev_node = &dev->node;

	
	printk(KERN_INFO "%s: index 0x%02x",
	       dev->node.dev_name, link->conf.ConfigIndex);
	if (link->conf.Attributes & CONF_ENABLE_IRQ)
		printk(", irq %d", link->irq.AssignedIRQ);
	if (link->io.NumPorts1)
		printk(", io 0x%04x-0x%04x", link->io.BasePort1,
		       link->io.BasePort1 + link->io.NumPorts1 - 1);
	if (link->io.NumPorts2)
		printk(" & 0x%04x-0x%04x", link->io.BasePort2,
		       link->io.BasePort2 + link->io.NumPorts2 - 1);
	if (link->win)
		printk(", mem 0x%06lx-0x%06lx", req.Base,
		       req.Base + req.Size - 1);
	printk("\n");

	return;

cs_failed:
	labpc_release(link);

}				

static void labpc_release(struct pcmcia_device *link)
{
	DEBUG(0, "labpc_release(0x%p)\n", link);

	pcmcia_disable_device(link);
}				



static int labpc_cs_suspend(struct pcmcia_device *link)
{
	struct local_info_t *local = link->priv;

	
	local->stop = 1;
	return 0;
}				

static int labpc_cs_resume(struct pcmcia_device *link)
{
	struct local_info_t *local = link->priv;

	local->stop = 0;
	return 0;
}				



static struct pcmcia_device_id labpc_cs_ids[] = {
	
	PCMCIA_DEVICE_MANF_CARD(0x010b, 0x0103),	
	PCMCIA_DEVICE_NULL
};

MODULE_DEVICE_TABLE(pcmcia, labpc_cs_ids);

struct pcmcia_driver labpc_cs_driver = {
	.probe = labpc_cs_attach,
	.remove = labpc_cs_detach,
	.suspend = labpc_cs_suspend,
	.resume = labpc_cs_resume,
	.id_table = labpc_cs_ids,
	.owner = THIS_MODULE,
	.drv = {
		.name = dev_info,
		},
};

static int __init init_labpc_cs(void)
{
	DEBUG(0, "%s\n", version);
	pcmcia_register_driver(&labpc_cs_driver);
	return 0;
}

static void __exit exit_labpc_cs(void)
{
	DEBUG(0, "ni_labpc: unloading\n");
	pcmcia_unregister_driver(&labpc_cs_driver);
}

int __init labpc_init_module(void)
{
	int ret;

	ret = init_labpc_cs();
	if (ret < 0)
		return ret;

	return comedi_driver_register(&driver_labpc_cs);
}

void __exit labpc_exit_module(void)
{
	exit_labpc_cs();
	comedi_driver_unregister(&driver_labpc_cs);
}

MODULE_LICENSE("GPL");
module_init(labpc_init_module);
module_exit(labpc_exit_module);
