



#include "../comedidev.h"

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>



#ifdef PCMCIA_DEBUG
static int pc_debug = PCMCIA_DEBUG;
module_param(pc_debug, int, 0644);
#define DEBUG(n, args...) if (pc_debug>(n)) printk(KERN_DEBUG args)
static char *version = "quatech_daqp_cs.c 1.10 2003/04/21 (Brent Baccala)";
#else
#define DEBUG(n, args...)
#endif


#define MAX_DEV         4

struct local_info_t {
	struct pcmcia_device *link;
	dev_node_t node;
	int stop;
	int table_index;
	char board_name[32];

	enum { semaphore, buffer } interrupt_mode;

	struct semaphore eos;

	struct comedi_device *dev;
	struct comedi_subdevice *s;
	int count;
};



static struct local_info_t *dev_table[MAX_DEV] = { NULL,   };



#define DAQP_FIFO_SIZE		4096

#define DAQP_FIFO		0
#define DAQP_SCANLIST		1
#define DAQP_CONTROL		2
#define DAQP_STATUS		2
#define DAQP_DIGITAL_IO		3
#define DAQP_PACER_LOW		4
#define DAQP_PACER_MID		5
#define DAQP_PACER_HIGH		6
#define DAQP_COMMAND		7
#define DAQP_DA			8
#define DAQP_TIMER		10
#define DAQP_AUX		15

#define DAQP_SCANLIST_DIFFERENTIAL	0x4000
#define DAQP_SCANLIST_GAIN(x)		((x)<<12)
#define DAQP_SCANLIST_CHANNEL(x)	((x)<<8)
#define DAQP_SCANLIST_START		0x0080
#define DAQP_SCANLIST_EXT_GAIN(x)	((x)<<4)
#define DAQP_SCANLIST_EXT_CHANNEL(x)	(x)

#define DAQP_CONTROL_PACER_100kHz	0xc0
#define DAQP_CONTROL_PACER_1MHz		0x80
#define DAQP_CONTROL_PACER_5MHz		0x40
#define DAQP_CONTROL_PACER_EXTERNAL	0x00
#define DAQP_CONTORL_EXPANSION		0x20
#define DAQP_CONTROL_EOS_INT_ENABLE	0x10
#define DAQP_CONTROL_FIFO_INT_ENABLE	0x08
#define DAQP_CONTROL_TRIGGER_ONESHOT	0x00
#define DAQP_CONTROL_TRIGGER_CONTINUOUS	0x04
#define DAQP_CONTROL_TRIGGER_INTERNAL	0x00
#define DAQP_CONTROL_TRIGGER_EXTERNAL	0x02
#define DAQP_CONTROL_TRIGGER_RISING	0x00
#define DAQP_CONTROL_TRIGGER_FALLING	0x01

#define DAQP_STATUS_IDLE		0x80
#define DAQP_STATUS_RUNNING		0x40
#define DAQP_STATUS_EVENTS		0x38
#define DAQP_STATUS_DATA_LOST		0x20
#define DAQP_STATUS_END_OF_SCAN		0x10
#define DAQP_STATUS_FIFO_THRESHOLD	0x08
#define DAQP_STATUS_FIFO_FULL		0x04
#define DAQP_STATUS_FIFO_NEARFULL	0x02
#define DAQP_STATUS_FIFO_EMPTY		0x01

#define DAQP_COMMAND_ARM		0x80
#define DAQP_COMMAND_RSTF		0x40
#define DAQP_COMMAND_RSTQ		0x20
#define DAQP_COMMAND_STOP		0x10
#define DAQP_COMMAND_LATCH		0x08
#define DAQP_COMMAND_100kHz		0x00
#define DAQP_COMMAND_50kHz		0x02
#define DAQP_COMMAND_25kHz		0x04
#define DAQP_COMMAND_FIFO_DATA		0x01
#define DAQP_COMMAND_FIFO_PROGRAM	0x00

#define DAQP_AUX_TRIGGER_TTL		0x00
#define DAQP_AUX_TRIGGER_ANALOG		0x80
#define DAQP_AUX_TRIGGER_PRETRIGGER	0x40
#define DAQP_AUX_TIMER_INT_ENABLE	0x20
#define DAQP_AUX_TIMER_RELOAD		0x00
#define DAQP_AUX_TIMER_PAUSE		0x08
#define DAQP_AUX_TIMER_GO		0x10
#define DAQP_AUX_TIMER_GO_EXTERNAL	0x18
#define DAQP_AUX_TIMER_EXTERNAL_SRC	0x04
#define DAQP_AUX_TIMER_INTERNAL_SRC	0x00
#define DAQP_AUX_DA_DIRECT		0x00
#define DAQP_AUX_DA_OVERFLOW		0x01
#define DAQP_AUX_DA_EXTERNAL		0x02
#define DAQP_AUX_DA_PACER		0x03

#define DAQP_AUX_RUNNING		0x80
#define DAQP_AUX_TRIGGERED		0x40
#define DAQP_AUX_DA_BUFFER		0x20
#define DAQP_AUX_TIMER_OVERFLOW		0x10
#define DAQP_AUX_CONVERSION		0x08
#define DAQP_AUX_DATA_LOST		0x04
#define DAQP_AUX_FIFO_NEARFULL		0x02
#define DAQP_AUX_FIFO_EMPTY		0x01



static const struct comedi_lrange range_daqp_ai = { 4, {
							BIP_RANGE(10),
							BIP_RANGE(5),
							BIP_RANGE(2.5),
							BIP_RANGE(1.25)
							}
};

static const struct comedi_lrange range_daqp_ao = { 1, {BIP_RANGE(5)} };





static int daqp_attach(struct comedi_device *dev, struct comedi_devconfig *it);
static int daqp_detach(struct comedi_device *dev);
static struct comedi_driver driver_daqp = {
	.driver_name = "quatech_daqp_cs",
	.module = THIS_MODULE,
	.attach = daqp_attach,
	.detach = daqp_detach,
};

#ifdef DAQP_DEBUG

static void daqp_dump(struct comedi_device *dev)
{
	printk("DAQP: status %02x; aux status %02x\n",
	       inb(dev->iobase + DAQP_STATUS), inb(dev->iobase + DAQP_AUX));
}

static void hex_dump(char *str, void *ptr, int len)
{
	unsigned char *cptr = ptr;
	int i;

	printk(str);

	for (i = 0; i < len; i++) {
		if (i % 16 == 0) {
			printk("\n0x%08x:", (unsigned int)cptr);
		}
		printk(" %02x", *(cptr++));
	}
	printk("\n");
}

#endif



static int daqp_ai_cancel(struct comedi_device *dev, struct comedi_subdevice *s)
{
	struct local_info_t *local = (struct local_info_t *)s->private;

	if (local->stop) {
		return -EIO;
	}

	outb(DAQP_COMMAND_STOP, dev->iobase + DAQP_COMMAND);

	
	

	local->interrupt_mode = semaphore;

	return 0;
}



static void daqp_interrupt(int irq, void *dev_id)
{
	struct local_info_t *local = (struct local_info_t *)dev_id;
	struct comedi_device *dev;
	struct comedi_subdevice *s;
	int loop_limit = 10000;
	int status;

	if (local == NULL) {
		printk(KERN_WARNING
		       "daqp_interrupt(): irq %d for unknown device.\n", irq);
		return;
	}

	dev = local->dev;
	if (dev == NULL) {
		printk(KERN_WARNING "daqp_interrupt(): NULL comedi_device.\n");
		return;
	}

	if (!dev->attached) {
		printk(KERN_WARNING
		       "daqp_interrupt(): struct comedi_device not yet attached.\n");
		return;
	}

	s = local->s;
	if (s == NULL) {
		printk(KERN_WARNING
		       "daqp_interrupt(): NULL comedi_subdevice.\n");
		return;
	}

	if ((struct local_info_t *)s->private != local) {
		printk(KERN_WARNING
		       "daqp_interrupt(): invalid comedi_subdevice.\n");
		return;
	}

	switch (local->interrupt_mode) {

	case semaphore:

		up(&local->eos);
		break;

	case buffer:

		while (!((status = inb(dev->iobase + DAQP_STATUS))
			 & DAQP_STATUS_FIFO_EMPTY)) {

			short data;

			if (status & DAQP_STATUS_DATA_LOST) {
				s->async->events |=
				    COMEDI_CB_EOA | COMEDI_CB_OVERFLOW;
				printk("daqp: data lost\n");
				daqp_ai_cancel(dev, s);
				break;
			}

			data = inb(dev->iobase + DAQP_FIFO);
			data |= inb(dev->iobase + DAQP_FIFO) << 8;
			data ^= 0x8000;

			comedi_buf_put(s->async, data);

			

			if (local->count > 0) {
				local->count--;
				if (local->count == 0) {
					daqp_ai_cancel(dev, s);
					s->async->events |= COMEDI_CB_EOA;
					break;
				}
			}

			if ((loop_limit--) <= 0)
				break;
		}

		if (loop_limit <= 0) {
			printk(KERN_WARNING
			       "loop_limit reached in daqp_interrupt()\n");
			daqp_ai_cancel(dev, s);
			s->async->events |= COMEDI_CB_EOA | COMEDI_CB_ERROR;
		}

		s->async->events |= COMEDI_CB_BLOCK;

		comedi_event(dev, s);
	}
}



static int daqp_ai_insn_read(struct comedi_device *dev,
			     struct comedi_subdevice *s,
			     struct comedi_insn *insn, unsigned int *data)
{
	struct local_info_t *local = (struct local_info_t *)s->private;
	int i;
	int v;
	int counter = 10000;

	if (local->stop) {
		return -EIO;
	}

	
	daqp_ai_cancel(dev, s);

	outb(0, dev->iobase + DAQP_AUX);

	
	outb(DAQP_COMMAND_RSTQ, dev->iobase + DAQP_COMMAND);

	

	v = DAQP_SCANLIST_CHANNEL(CR_CHAN(insn->chanspec))
	    | DAQP_SCANLIST_GAIN(CR_RANGE(insn->chanspec));

	if (CR_AREF(insn->chanspec) == AREF_DIFF) {
		v |= DAQP_SCANLIST_DIFFERENTIAL;
	}

	v |= DAQP_SCANLIST_START;

	outb(v & 0xff, dev->iobase + DAQP_SCANLIST);
	outb(v >> 8, dev->iobase + DAQP_SCANLIST);

	

	outb(DAQP_COMMAND_RSTF, dev->iobase + DAQP_COMMAND);

	

	v = DAQP_CONTROL_TRIGGER_ONESHOT | DAQP_CONTROL_TRIGGER_INTERNAL
	    | DAQP_CONTROL_PACER_100kHz | DAQP_CONTROL_EOS_INT_ENABLE;

	outb(v, dev->iobase + DAQP_CONTROL);

	

	while (--counter
	       && (inb(dev->iobase + DAQP_STATUS) & DAQP_STATUS_EVENTS)) ;
	if (!counter) {
		printk("daqp: couldn't clear interrupts in status register\n");
		return -1;
	}

	
	sema_init(&local->eos, 0);
	local->interrupt_mode = semaphore;
	local->dev = dev;
	local->s = s;

	for (i = 0; i < insn->n; i++) {

		
		outb(DAQP_COMMAND_ARM | DAQP_COMMAND_FIFO_DATA,
		     dev->iobase + DAQP_COMMAND);

		
		
		if (down_interruptible(&local->eos))
			return -EINTR;

		data[i] = inb(dev->iobase + DAQP_FIFO);
		data[i] |= inb(dev->iobase + DAQP_FIFO) << 8;
		data[i] ^= 0x8000;
	}

	return insn->n;
}



static int daqp_ns_to_timer(unsigned int *ns, int round)
{
	int timer;

	timer = *ns / 200;
	*ns = timer * 200;

	return timer;
}



static int daqp_ai_cmdtest(struct comedi_device *dev,
			   struct comedi_subdevice *s, struct comedi_cmd *cmd)
{
	int err = 0;
	int tmp;

	

	tmp = cmd->start_src;
	cmd->start_src &= TRIG_NOW;
	if (!cmd->start_src || tmp != cmd->start_src)
		err++;

	tmp = cmd->scan_begin_src;
	cmd->scan_begin_src &= TRIG_TIMER | TRIG_FOLLOW;
	if (!cmd->scan_begin_src || tmp != cmd->scan_begin_src)
		err++;

	tmp = cmd->convert_src;
	cmd->convert_src &= TRIG_TIMER | TRIG_NOW;
	if (!cmd->convert_src || tmp != cmd->convert_src)
		err++;

	tmp = cmd->scan_end_src;
	cmd->scan_end_src &= TRIG_COUNT;
	if (!cmd->scan_end_src || tmp != cmd->scan_end_src)
		err++;

	tmp = cmd->stop_src;
	cmd->stop_src &= TRIG_COUNT | TRIG_NONE;
	if (!cmd->stop_src || tmp != cmd->stop_src)
		err++;

	if (err)
		return 1;

	

	
	if (cmd->scan_begin_src != TRIG_TIMER &&
	    cmd->scan_begin_src != TRIG_FOLLOW)
		err++;
	if (cmd->convert_src != TRIG_NOW && cmd->convert_src != TRIG_TIMER)
		err++;
	if (cmd->scan_begin_src == TRIG_FOLLOW && cmd->convert_src == TRIG_NOW)
		err++;
	if (cmd->stop_src != TRIG_COUNT && cmd->stop_src != TRIG_NONE)
		err++;

	if (err)
		return 2;

	

	if (cmd->start_arg != 0) {
		cmd->start_arg = 0;
		err++;
	}
#define MAX_SPEED	10000	

	if (cmd->scan_begin_src == TRIG_TIMER
	    && cmd->scan_begin_arg < MAX_SPEED) {
		cmd->scan_begin_arg = MAX_SPEED;
		err++;
	}

	

	if (cmd->scan_begin_src == TRIG_TIMER && cmd->convert_src == TRIG_TIMER
	    && cmd->scan_begin_arg != cmd->convert_arg * cmd->scan_end_arg) {
		err++;
	}

	if (cmd->convert_src == TRIG_TIMER && cmd->convert_arg < MAX_SPEED) {
		cmd->convert_arg = MAX_SPEED;
		err++;
	}

	if (cmd->scan_end_arg != cmd->chanlist_len) {
		cmd->scan_end_arg = cmd->chanlist_len;
		err++;
	}
	if (cmd->stop_src == TRIG_COUNT) {
		if (cmd->stop_arg > 0x00ffffff) {
			cmd->stop_arg = 0x00ffffff;
			err++;
		}
	} else {
		
		if (cmd->stop_arg != 0) {
			cmd->stop_arg = 0;
			err++;
		}
	}

	if (err)
		return 3;

	

	if (cmd->scan_begin_src == TRIG_TIMER) {
		tmp = cmd->scan_begin_arg;
		daqp_ns_to_timer(&cmd->scan_begin_arg,
				 cmd->flags & TRIG_ROUND_MASK);
		if (tmp != cmd->scan_begin_arg)
			err++;
	}

	if (cmd->convert_src == TRIG_TIMER) {
		tmp = cmd->convert_arg;
		daqp_ns_to_timer(&cmd->convert_arg,
				 cmd->flags & TRIG_ROUND_MASK);
		if (tmp != cmd->convert_arg)
			err++;
	}

	if (err)
		return 4;

	return 0;
}

static int daqp_ai_cmd(struct comedi_device *dev, struct comedi_subdevice *s)
{
	struct local_info_t *local = (struct local_info_t *)s->private;
	struct comedi_cmd *cmd = &s->async->cmd;
	int counter = 100;
	int scanlist_start_on_every_entry;
	int threshold;

	int i;
	int v;

	if (local->stop) {
		return -EIO;
	}

	
	daqp_ai_cancel(dev, s);

	outb(0, dev->iobase + DAQP_AUX);

	
	outb(DAQP_COMMAND_RSTQ, dev->iobase + DAQP_COMMAND);

	

	if (cmd->convert_src == TRIG_TIMER) {
		int counter = daqp_ns_to_timer(&cmd->convert_arg,
					       cmd->flags & TRIG_ROUND_MASK);
		outb(counter & 0xff, dev->iobase + DAQP_PACER_LOW);
		outb((counter >> 8) & 0xff, dev->iobase + DAQP_PACER_MID);
		outb((counter >> 16) & 0xff, dev->iobase + DAQP_PACER_HIGH);
		scanlist_start_on_every_entry = 1;
	} else {
		int counter = daqp_ns_to_timer(&cmd->scan_begin_arg,
					       cmd->flags & TRIG_ROUND_MASK);
		outb(counter & 0xff, dev->iobase + DAQP_PACER_LOW);
		outb((counter >> 8) & 0xff, dev->iobase + DAQP_PACER_MID);
		outb((counter >> 16) & 0xff, dev->iobase + DAQP_PACER_HIGH);
		scanlist_start_on_every_entry = 0;
	}

	

	for (i = 0; i < cmd->chanlist_len; i++) {

		int chanspec = cmd->chanlist[i];

		

		v = DAQP_SCANLIST_CHANNEL(CR_CHAN(chanspec))
		    | DAQP_SCANLIST_GAIN(CR_RANGE(chanspec));

		if (CR_AREF(chanspec) == AREF_DIFF) {
			v |= DAQP_SCANLIST_DIFFERENTIAL;
		}

		if (i == 0 || scanlist_start_on_every_entry) {
			v |= DAQP_SCANLIST_START;
		}

		outb(v & 0xff, dev->iobase + DAQP_SCANLIST);
		outb(v >> 8, dev->iobase + DAQP_SCANLIST);
	}

	

	

	if (cmd->stop_src == TRIG_COUNT) {
		local->count = cmd->stop_arg * cmd->scan_end_arg;
		threshold = 2 * local->count;
		while (threshold > DAQP_FIFO_SIZE * 3 / 4)
			threshold /= 2;
	} else {
		local->count = -1;
		threshold = DAQP_FIFO_SIZE / 2;
	}

	

	outb(DAQP_COMMAND_RSTF, dev->iobase + DAQP_COMMAND);

	

	outb(0x00, dev->iobase + DAQP_FIFO);
	outb(0x00, dev->iobase + DAQP_FIFO);

	outb((DAQP_FIFO_SIZE - threshold) & 0xff, dev->iobase + DAQP_FIFO);
	outb((DAQP_FIFO_SIZE - threshold) >> 8, dev->iobase + DAQP_FIFO);

	

	v = DAQP_CONTROL_TRIGGER_CONTINUOUS | DAQP_CONTROL_TRIGGER_INTERNAL
	    | DAQP_CONTROL_PACER_5MHz | DAQP_CONTROL_FIFO_INT_ENABLE;

	outb(v, dev->iobase + DAQP_CONTROL);

	

	while (--counter
	       && (inb(dev->iobase + DAQP_STATUS) & DAQP_STATUS_EVENTS)) ;
	if (!counter) {
		printk("daqp: couldn't clear interrupts in status register\n");
		return -1;
	}

	local->interrupt_mode = buffer;
	local->dev = dev;
	local->s = s;

	
	outb(DAQP_COMMAND_ARM | DAQP_COMMAND_FIFO_DATA,
	     dev->iobase + DAQP_COMMAND);

	return 0;
}



static int daqp_ao_insn_write(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      struct comedi_insn *insn, unsigned int *data)
{
	struct local_info_t *local = (struct local_info_t *)s->private;
	int d;
	unsigned int chan;

	if (local->stop) {
		return -EIO;
	}

	chan = CR_CHAN(insn->chanspec);
	d = data[0];
	d &= 0x0fff;
	d ^= 0x0800;		
	d |= chan << 12;

	
	outb(0, dev->iobase + DAQP_AUX);

	outw(d, dev->iobase + DAQP_DA);

	return 1;
}



static int daqp_di_insn_read(struct comedi_device *dev,
			     struct comedi_subdevice *s,
			     struct comedi_insn *insn, unsigned int *data)
{
	struct local_info_t *local = (struct local_info_t *)s->private;

	if (local->stop) {
		return -EIO;
	}

	data[0] = inb(dev->iobase + DAQP_DIGITAL_IO);

	return 1;
}



static int daqp_do_insn_write(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      struct comedi_insn *insn, unsigned int *data)
{
	struct local_info_t *local = (struct local_info_t *)s->private;

	if (local->stop) {
		return -EIO;
	}

	outw(data[0] & 0xf, dev->iobase + DAQP_DIGITAL_IO);

	return 1;
}



static int daqp_attach(struct comedi_device *dev, struct comedi_devconfig *it)
{
	int ret;
	struct local_info_t *local = dev_table[it->options[0]];
	tuple_t tuple;
	int i;
	struct comedi_subdevice *s;

	if (it->options[0] < 0 || it->options[0] >= MAX_DEV || !local) {
		printk("comedi%d: No such daqp device %d\n",
		       dev->minor, it->options[0]);
		return -EIO;
	}

	

	strcpy(local->board_name, "DAQP");
	dev->board_name = local->board_name;

	tuple.DesiredTuple = CISTPL_VERS_1;
	if (pcmcia_get_first_tuple(local->link, &tuple) == 0) {
		u_char buf[128];

		buf[0] = buf[sizeof(buf) - 1] = 0;
		tuple.TupleData = buf;
		tuple.TupleDataMax = sizeof(buf);
		tuple.TupleOffset = 2;
		if (pcmcia_get_tuple_data(local->link, &tuple) == 0) {

			for (i = 0; i < tuple.TupleDataLen - 4; i++)
				if (buf[i] == 0)
					break;
			for (i++; i < tuple.TupleDataLen - 4; i++)
				if (buf[i] == 0)
					break;
			i++;
			if ((i < tuple.TupleDataLen - 4)
			    && (strncmp(buf + i, "DAQP", 4) == 0)) {
				strncpy(local->board_name, buf + i,
					sizeof(local->board_name));
			}
		}
	}

	dev->iobase = local->link->io.BasePort1;

	ret = alloc_subdevices(dev, 4);
	if (ret < 0)
		return ret;

	printk("comedi%d: attaching daqp%d (io 0x%04lx)\n",
	       dev->minor, it->options[0], dev->iobase);

	s = dev->subdevices + 0;
	dev->read_subdev = s;
	s->private = local;
	s->type = COMEDI_SUBD_AI;
	s->subdev_flags = SDF_READABLE | SDF_GROUND | SDF_DIFF | SDF_CMD_READ;
	s->n_chan = 8;
	s->len_chanlist = 2048;
	s->maxdata = 0xffff;
	s->range_table = &range_daqp_ai;
	s->insn_read = daqp_ai_insn_read;
	s->do_cmdtest = daqp_ai_cmdtest;
	s->do_cmd = daqp_ai_cmd;
	s->cancel = daqp_ai_cancel;

	s = dev->subdevices + 1;
	dev->write_subdev = s;
	s->private = local;
	s->type = COMEDI_SUBD_AO;
	s->subdev_flags = SDF_WRITEABLE;
	s->n_chan = 2;
	s->len_chanlist = 1;
	s->maxdata = 0x0fff;
	s->range_table = &range_daqp_ao;
	s->insn_write = daqp_ao_insn_write;

	s = dev->subdevices + 2;
	s->private = local;
	s->type = COMEDI_SUBD_DI;
	s->subdev_flags = SDF_READABLE;
	s->n_chan = 1;
	s->len_chanlist = 1;
	s->insn_read = daqp_di_insn_read;

	s = dev->subdevices + 3;
	s->private = local;
	s->type = COMEDI_SUBD_DO;
	s->subdev_flags = SDF_WRITEABLE;
	s->n_chan = 1;
	s->len_chanlist = 1;
	s->insn_write = daqp_do_insn_write;

	return 1;
}



static int daqp_detach(struct comedi_device *dev)
{
	printk("comedi%d: detaching daqp\n", dev->minor);

	return 0;
}





static void daqp_cs_config(struct pcmcia_device *link);
static void daqp_cs_release(struct pcmcia_device *link);
static int daqp_cs_suspend(struct pcmcia_device *p_dev);
static int daqp_cs_resume(struct pcmcia_device *p_dev);



static int daqp_cs_attach(struct pcmcia_device *);
static void daqp_cs_detach(struct pcmcia_device *);



static const dev_info_t dev_info = "quatech_daqp_cs";



static int daqp_cs_attach(struct pcmcia_device *link)
{
	struct local_info_t *local;
	int i;

	DEBUG(0, "daqp_cs_attach()\n");

	for (i = 0; i < MAX_DEV; i++)
		if (dev_table[i] == NULL)
			break;
	if (i == MAX_DEV) {
		printk(KERN_NOTICE "daqp_cs: no devices available\n");
		return -ENODEV;
	}

	
	local = kzalloc(sizeof(struct local_info_t), GFP_KERNEL);
	if (!local)
		return -ENOMEM;

	local->table_index = i;
	dev_table[i] = local;
	local->link = link;
	link->priv = local;

	
	link->irq.Attributes = IRQ_TYPE_DYNAMIC_SHARING | IRQ_HANDLE_PRESENT;
	link->irq.IRQInfo1 = IRQ_LEVEL_ID;
	link->irq.Handler = daqp_interrupt;
	link->irq.Instance = local;

	
	link->conf.Attributes = 0;
	link->conf.IntType = INT_MEMORY_AND_IO;

	daqp_cs_config(link);

	return 0;
}				



static void daqp_cs_detach(struct pcmcia_device *link)
{
	struct local_info_t *dev = link->priv;

	DEBUG(0, "daqp_cs_detach(0x%p)\n", link);

	if (link->dev_node) {
		dev->stop = 1;
		daqp_cs_release(link);
	}

	
	dev_table[dev->table_index] = NULL;
	if (dev)
		kfree(dev);

}				



static void daqp_cs_config(struct pcmcia_device *link)
{
	struct local_info_t *dev = link->priv;
	tuple_t tuple;
	cisparse_t parse;
	int last_ret;
	u_char buf[64];

	DEBUG(0, "daqp_cs_config(0x%p)\n", link);

	
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
		cistpl_cftable_entry_t dflt = { 0 };
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
		}

		
		if (pcmcia_request_io(link, &link->io))
			goto next_entry;

		
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

	
	
	
	sprintf(dev->node.dev_name, "quatech_daqp_cs");
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
	daqp_cs_release(link);

}				

static void daqp_cs_release(struct pcmcia_device *link)
{
	DEBUG(0, "daqp_cs_release(0x%p)\n", link);

	pcmcia_disable_device(link);
}				



static int daqp_cs_suspend(struct pcmcia_device *link)
{
	struct local_info_t *local = link->priv;

	
	local->stop = 1;
	return 0;
}

static int daqp_cs_resume(struct pcmcia_device *link)
{
	struct local_info_t *local = link->priv;

	local->stop = 0;

	return 0;
}



#ifdef MODULE

static struct pcmcia_device_id daqp_cs_id_table[] = {
	PCMCIA_DEVICE_MANF_CARD(0x0137, 0x0027),
	PCMCIA_DEVICE_NULL
};

MODULE_DEVICE_TABLE(pcmcia, daqp_cs_id_table);

struct pcmcia_driver daqp_cs_driver = {
	.probe = daqp_cs_attach,
	.remove = daqp_cs_detach,
	.suspend = daqp_cs_suspend,
	.resume = daqp_cs_resume,
	.id_table = daqp_cs_id_table,
	.owner = THIS_MODULE,
	.drv = {
		.name = dev_info,
		},
};

int __init init_module(void)
{
	DEBUG(0, "%s\n", version);
	pcmcia_register_driver(&daqp_cs_driver);
	comedi_driver_register(&driver_daqp);
	return 0;
}

void __exit cleanup_module(void)
{
	DEBUG(0, "daqp_cs: unloading\n");
	comedi_driver_unregister(&driver_daqp);
	pcmcia_unregister_driver(&daqp_cs_driver);
}

#endif
