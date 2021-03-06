



#include <linux/interrupt.h>
#include "../comedidev.h"

#include <linux/ioport.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>

static struct pcmcia_device *pcmcia_cur_dev = NULL;

#define DIO700_SIZE 8		

static int dio700_attach(struct comedi_device *dev,
			 struct comedi_devconfig *it);
static int dio700_detach(struct comedi_device *dev);

enum dio700_bustype { pcmcia_bustype };

struct dio700_board {
	const char *name;
	int device_id;		
	enum dio700_bustype bustype;	
	int have_dio;		
	
	
	unsigned int (*read_byte) (unsigned int address);
	void (*write_byte) (unsigned int byte, unsigned int address);
};

static const struct dio700_board dio700_boards[] = {
	{
	 .name = "daqcard-700",
	 .device_id = 0x4743,	
	 .bustype = pcmcia_bustype,
	 .have_dio = 1,
	 },
	{
	 .name = "ni_daq_700",
	 .device_id = 0x4743,	
	 .bustype = pcmcia_bustype,
	 .have_dio = 1,
	 },
};


#define thisboard ((const struct dio700_board *)dev->board_ptr)

struct dio700_private {

	int data;		
};

#define devpriv ((struct dio700_private *)dev->private)

static struct comedi_driver driver_dio700 = {
	.driver_name = "ni_daq_700",
	.module = THIS_MODULE,
	.attach = dio700_attach,
	.detach = dio700_detach,
	.num_names = ARRAY_SIZE(dio700_boards),
	.board_name = &dio700_boards[0].name,
	.offset = sizeof(struct dio700_board),
};



#define _700_SIZE 8

#define _700_DATA 0

#define DIO_W		0x04
#define DIO_R		0x05

struct subdev_700_struct {
	unsigned long cb_arg;
	int (*cb_func) (int, int, int, unsigned long);
	int have_irq;
};

#define CALLBACK_ARG	(((struct subdev_700_struct *)s->private)->cb_arg)
#define CALLBACK_FUNC	(((struct subdev_700_struct *)s->private)->cb_func)
#define subdevpriv	((struct subdev_700_struct *)s->private)

static void do_config(struct comedi_device *dev, struct comedi_subdevice *s);

void subdev_700_interrupt(struct comedi_device *dev, struct comedi_subdevice *s)
{
	short d;

	d = CALLBACK_FUNC(0, _700_DATA, 0, CALLBACK_ARG);

	comedi_buf_put(s->async, d);
	s->async->events |= COMEDI_CB_EOS;

	comedi_event(dev, s);
}

static int subdev_700_cb(int dir, int port, int data, unsigned long arg)
{
	
	unsigned long iobase = arg;

	if (dir) {
		outb(data, iobase + DIO_W);
		return 0;
	} else {
		return inb(iobase + DIO_R);
	}
}

static int subdev_700_insn(struct comedi_device *dev,
			   struct comedi_subdevice *s, struct comedi_insn *insn,
			   unsigned int *data)
{
	if (data[0]) {
		s->state &= ~data[0];
		s->state |= (data[0] & data[1]);

		if (data[0] & 0xff)
			CALLBACK_FUNC(1, _700_DATA, s->state & 0xff,
				      CALLBACK_ARG);
	}

	data[1] = s->state & 0xff;
	data[1] |= CALLBACK_FUNC(0, _700_DATA, 0, CALLBACK_ARG) << 8;

	return 2;
}

static int subdev_700_insn_config(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data)
{

	switch (data[0]) {
	case INSN_CONFIG_DIO_INPUT:
		break;
	case INSN_CONFIG_DIO_OUTPUT:
		break;
	case INSN_CONFIG_DIO_QUERY:
		data[1] =
		    (s->
		     io_bits & (1 << CR_CHAN(insn->chanspec))) ? COMEDI_OUTPUT :
		    COMEDI_INPUT;
		return insn->n;
		break;
	default:
		return -EINVAL;
	}

	return 1;
}

static void do_config(struct comedi_device *dev, struct comedi_subdevice *s)
{				
	return;
}

static int subdev_700_cmdtest(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      struct comedi_cmd *cmd)
{
	int err = 0;
	unsigned int tmp;

	

	tmp = cmd->start_src;
	cmd->start_src &= TRIG_NOW;
	if (!cmd->start_src || tmp != cmd->start_src)
		err++;

	tmp = cmd->scan_begin_src;
	cmd->scan_begin_src &= TRIG_EXT;
	if (!cmd->scan_begin_src || tmp != cmd->scan_begin_src)
		err++;

	tmp = cmd->convert_src;
	cmd->convert_src &= TRIG_FOLLOW;
	if (!cmd->convert_src || tmp != cmd->convert_src)
		err++;

	tmp = cmd->scan_end_src;
	cmd->scan_end_src &= TRIG_COUNT;
	if (!cmd->scan_end_src || tmp != cmd->scan_end_src)
		err++;

	tmp = cmd->stop_src;
	cmd->stop_src &= TRIG_NONE;
	if (!cmd->stop_src || tmp != cmd->stop_src)
		err++;

	if (err)
		return 1;

	

	if (err)
		return 2;

	

	if (cmd->start_arg != 0) {
		cmd->start_arg = 0;
		err++;
	}
	if (cmd->scan_begin_arg != 0) {
		cmd->scan_begin_arg = 0;
		err++;
	}
	if (cmd->convert_arg != 0) {
		cmd->convert_arg = 0;
		err++;
	}
	if (cmd->scan_end_arg != 1) {
		cmd->scan_end_arg = 1;
		err++;
	}
	if (cmd->stop_arg != 0) {
		cmd->stop_arg = 0;
		err++;
	}

	if (err)
		return 3;

	

	if (err)
		return 4;

	return 0;
}

static int subdev_700_cmd(struct comedi_device *dev, struct comedi_subdevice *s)
{
	

	return 0;
}

static int subdev_700_cancel(struct comedi_device *dev,
			     struct comedi_subdevice *s)
{
	

	return 0;
}

int subdev_700_init(struct comedi_device *dev, struct comedi_subdevice *s,
		    int (*cb) (int, int, int, unsigned long), unsigned long arg)
{
	s->type = COMEDI_SUBD_DIO;
	s->subdev_flags = SDF_READABLE | SDF_WRITABLE;
	s->n_chan = 16;
	s->range_table = &range_digital;
	s->maxdata = 1;

	s->private = kmalloc(sizeof(struct subdev_700_struct), GFP_KERNEL);
	if (!s->private)
		return -ENOMEM;

	CALLBACK_ARG = arg;
	if (cb == NULL) {
		CALLBACK_FUNC = subdev_700_cb;
	} else {
		CALLBACK_FUNC = cb;
	}
	s->insn_bits = subdev_700_insn;
	s->insn_config = subdev_700_insn_config;

	s->state = 0;
	s->io_bits = 0x00ff;
	do_config(dev, s);

	return 0;
}

int subdev_700_init_irq(struct comedi_device *dev, struct comedi_subdevice *s,
			int (*cb) (int, int, int, unsigned long),
			unsigned long arg)
{
	int ret;

	ret = subdev_700_init(dev, s, cb, arg);
	if (ret < 0)
		return ret;

	s->do_cmdtest = subdev_700_cmdtest;
	s->do_cmd = subdev_700_cmd;
	s->cancel = subdev_700_cancel;

	subdevpriv->have_irq = 1;

	return 0;
}

void subdev_700_cleanup(struct comedi_device *dev, struct comedi_subdevice *s)
{
	if (s->private) {
		if (subdevpriv->have_irq) {
		}

		kfree(s->private);
	}
}

EXPORT_SYMBOL(subdev_700_init);
EXPORT_SYMBOL(subdev_700_init_irq);
EXPORT_SYMBOL(subdev_700_cleanup);
EXPORT_SYMBOL(subdev_700_interrupt);

static int dio700_attach(struct comedi_device *dev, struct comedi_devconfig *it)
{
	struct comedi_subdevice *s;
	unsigned long iobase = 0;
#ifdef incomplete
	unsigned int irq = 0;
#endif
	struct pcmcia_device *link;

	
	if (alloc_private(dev, sizeof(struct dio700_private)) < 0)
		return -ENOMEM;

	
	switch (thisboard->bustype) {
	case pcmcia_bustype:
		link = pcmcia_cur_dev;	
		if (!link)
			return -EIO;
		iobase = link->io.BasePort1;
#ifdef incomplete
		irq = link->irq.AssignedIRQ;
#endif
		break;
	default:
		printk("bug! couldn't determine board type\n");
		return -EINVAL;
		break;
	}
	printk("comedi%d: ni_daq_700: %s, io 0x%lx", dev->minor,
	       thisboard->name, iobase);
#ifdef incomplete
	if (irq) {
		printk(", irq %u", irq);
	}
#endif

	printk("\n");

	if (iobase == 0) {
		printk("io base address is zero!\n");
		return -EINVAL;
	}

	dev->iobase = iobase;

#ifdef incomplete
	
	dev->irq = irq;
#endif

	dev->board_name = thisboard->name;

	if (alloc_subdevices(dev, 1) < 0)
		return -ENOMEM;

	
	s = dev->subdevices + 0;
	subdev_700_init(dev, s, NULL, dev->iobase);

	return 0;
};

static int dio700_detach(struct comedi_device *dev)
{
	printk("comedi%d: ni_daq_700: cs-remove\n", dev->minor);

	if (dev->subdevices)
		subdev_700_cleanup(dev, dev->subdevices + 0);

	if (thisboard->bustype != pcmcia_bustype && dev->iobase)
		release_region(dev->iobase, DIO700_SIZE);
	if (dev->irq)
		free_irq(dev->irq, dev);

	return 0;
};




#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0644);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
static char *version = "ni_daq_700.c, based on dummy_cs.c";
#else
#define DEBUG(n, args...)
#endif



static void dio700_config(struct pcmcia_device *link);
static void dio700_release(struct pcmcia_device *link);
static int dio700_cs_suspend(struct pcmcia_device *p_dev);
static int dio700_cs_resume(struct pcmcia_device *p_dev);



static int dio700_cs_attach(struct pcmcia_device *);
static void dio700_cs_detach(struct pcmcia_device *);





static const dev_info_t dev_info = "ni_daq_700";

struct local_info_t {
	struct pcmcia_device *link;
	dev_node_t node;
	int stop;
	struct bus_operations *bus;
};



static int dio700_cs_attach(struct pcmcia_device *link)
{
	struct local_info_t *local;

	printk(KERN_INFO "ni_daq_700:  cs-attach\n");

	DEBUG(0, "dio700_cs_attach()\n");

	
	local = kzalloc(sizeof(struct local_info_t), GFP_KERNEL);
	if (!local)
		return -ENOMEM;
	local->link = link;
	link->priv = local;

	
	link->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING;
	link->irq.IRQInfo1 = IRQ_LEVEL_ID;
	link->irq.Handler = NULL;

	
	link->conf.Attributes = 0;
	link->conf.IntType = INT_MEMORY_AND_IO;

	pcmcia_cur_dev = link;

	dio700_config(link);

	return 0;
}				



static void dio700_cs_detach(struct pcmcia_device *link)
{

	printk(KERN_INFO "ni_daq_700: cs-detach!\n");

	DEBUG(0, "dio700_cs_detach(0x%p)\n", link);

	if (link->dev_node) {
		((struct local_info_t *)link->priv)->stop = 1;
		dio700_release(link);
	}

	
	if (link->priv)
		kfree(link->priv);

}				



static void dio700_config(struct pcmcia_device *link)
{
	struct local_info_t *dev = link->priv;
	tuple_t tuple;
	cisparse_t parse;
	int last_ret;
	u_char buf[64];
	win_req_t req;
	memreq_t map;
	cistpl_cftable_entry_t dflt = { 0 };

	printk(KERN_INFO "ni_daq_700:  cs-config\n");

	DEBUG(0, "dio700_config(0x%p)\n", link);

	
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
	if (last_ret != 0) {
		cs_error(link, GetFirstTuple, last_ret);
		goto cs_failed;
	}
	while (1) {
		cistpl_cftable_entry_t *cfg = &(parse.cftable_entry);
		if (pcmcia_get_tuple_data(link, &tuple) != 0)
			goto next_entry;
		if (pcmcia_parse_tuple(&tuple, &parse) != 0)
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
	if (last_ret != 0) {
		cs_error(link, RequestConfiguration, last_ret);
		goto cs_failed;
	}

	
	sprintf(dev->node.dev_name, "ni_daq_700");
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
	printk(KERN_INFO "ni_daq_700 cs failed");
	dio700_release(link);

}				

static void dio700_release(struct pcmcia_device *link)
{
	DEBUG(0, "dio700_release(0x%p)\n", link);

	pcmcia_disable_device(link);
}				



static int dio700_cs_suspend(struct pcmcia_device *link)
{
	struct local_info_t *local = link->priv;

	
	local->stop = 1;
	return 0;
}				

static int dio700_cs_resume(struct pcmcia_device *link)
{
	struct local_info_t *local = link->priv;

	local->stop = 0;
	return 0;
}				



static struct pcmcia_device_id dio700_cs_ids[] = {
	
	PCMCIA_DEVICE_MANF_CARD(0x010b, 0x4743),	
	PCMCIA_DEVICE_NULL
};

MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(pcmcia, dio700_cs_ids);

struct pcmcia_driver dio700_cs_driver = {
	.probe = dio700_cs_attach,
	.remove = dio700_cs_detach,
	.suspend = dio700_cs_suspend,
	.resume = dio700_cs_resume,
	.id_table = dio700_cs_ids,
	.owner = THIS_MODULE,
	.drv = {
		.name = dev_info,
		},
};

static int __init init_dio700_cs(void)
{
	printk("ni_daq_700:  cs-init \n");
	DEBUG(0, "%s\n", version);
	pcmcia_register_driver(&dio700_cs_driver);
	return 0;
}

static void __exit exit_dio700_cs(void)
{
	DEBUG(0, "ni_daq_700: unloading\n");
	pcmcia_unregister_driver(&dio700_cs_driver);
}

int __init init_module(void)
{
	int ret;

	ret = init_dio700_cs();
	if (ret < 0)
		return ret;

	return comedi_driver_register(&driver_dio700);
}

void __exit cleanup_module(void)
{
	exit_dio700_cs();
	comedi_driver_unregister(&driver_dio700);
}
