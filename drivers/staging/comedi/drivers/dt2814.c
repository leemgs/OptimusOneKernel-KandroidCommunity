


#include <linux/interrupt.h>
#include "../comedidev.h"

#include <linux/ioport.h>
#include <linux/delay.h>

#define DT2814_SIZE 2

#define DT2814_CSR 0
#define DT2814_DATA 1



#define DT2814_FINISH 0x80
#define DT2814_ERR 0x40
#define DT2814_BUSY 0x20
#define DT2814_ENB 0x10
#define DT2814_CHANMASK 0x0f

static int dt2814_attach(struct comedi_device *dev,
			 struct comedi_devconfig *it);
static int dt2814_detach(struct comedi_device *dev);
static struct comedi_driver driver_dt2814 = {
	.driver_name = "dt2814",
	.module = THIS_MODULE,
	.attach = dt2814_attach,
	.detach = dt2814_detach,
};

COMEDI_INITCLEANUP(driver_dt2814);

static irqreturn_t dt2814_interrupt(int irq, void *dev);

struct dt2814_private {

	int ntrig;
	int curadchan;
};

#define devpriv ((struct dt2814_private *)dev->private)

#define DT2814_TIMEOUT 10
#define DT2814_MAX_SPEED 100000	

static int dt2814_ai_insn_read(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data)
{
	int n, i, hi, lo;
	int chan;
	int status = 0;

	for (n = 0; n < insn->n; n++) {
		chan = CR_CHAN(insn->chanspec);

		outb(chan, dev->iobase + DT2814_CSR);
		for (i = 0; i < DT2814_TIMEOUT; i++) {
			status = inb(dev->iobase + DT2814_CSR);
			printk("dt2814: status: %02x\n", status);
			udelay(10);
			if (status & DT2814_FINISH)
				break;
		}
		if (i >= DT2814_TIMEOUT) {
			printk("dt2814: status: %02x\n", status);
			return -ETIMEDOUT;
		}

		hi = inb(dev->iobase + DT2814_DATA);
		lo = inb(dev->iobase + DT2814_DATA);

		data[n] = (hi << 4) | (lo >> 4);
	}

	return n;
}

static int dt2814_ns_to_timer(unsigned int *ns, unsigned int flags)
{
	int i;
	unsigned int f;

	

	f = 10000;		
	for (i = 0; i < 8; i++) {
		if ((2 * (*ns)) < (f * 11))
			break;
		f *= 10;
	}

	*ns = f;

	return i;
}

static int dt2814_ai_cmdtest(struct comedi_device *dev,
			     struct comedi_subdevice *s, struct comedi_cmd *cmd)
{
	int err = 0;
	int tmp;

	

	tmp = cmd->start_src;
	cmd->start_src &= TRIG_NOW;
	if (!cmd->start_src || tmp != cmd->start_src)
		err++;

	tmp = cmd->scan_begin_src;
	cmd->scan_begin_src &= TRIG_TIMER;
	if (!cmd->scan_begin_src || tmp != cmd->scan_begin_src)
		err++;

	tmp = cmd->convert_src;
	cmd->convert_src &= TRIG_NOW;
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

	

	
	if (cmd->stop_src != TRIG_TIMER && cmd->stop_src != TRIG_EXT)
		err++;

	if (err)
		return 2;

	

	if (cmd->start_arg != 0) {
		cmd->start_arg = 0;
		err++;
	}
	if (cmd->scan_begin_arg > 1000000000) {
		cmd->scan_begin_arg = 1000000000;
		err++;
	}
	if (cmd->scan_begin_arg < DT2814_MAX_SPEED) {
		cmd->scan_begin_arg = DT2814_MAX_SPEED;
		err++;
	}
	if (cmd->scan_end_arg != cmd->chanlist_len) {
		cmd->scan_end_arg = cmd->chanlist_len;
		err++;
	}
	if (cmd->stop_src == TRIG_COUNT) {
		if (cmd->stop_arg < 2) {
			cmd->stop_arg = 2;
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

	

	tmp = cmd->scan_begin_arg;
	dt2814_ns_to_timer(&cmd->scan_begin_arg, cmd->flags & TRIG_ROUND_MASK);
	if (tmp != cmd->scan_begin_arg)
		err++;

	if (err)
		return 4;

	return 0;
}

static int dt2814_ai_cmd(struct comedi_device *dev, struct comedi_subdevice *s)
{
	struct comedi_cmd *cmd = &s->async->cmd;
	int chan;
	int trigvar;

	trigvar =
	    dt2814_ns_to_timer(&cmd->scan_begin_arg,
			       cmd->flags & TRIG_ROUND_MASK);

	chan = CR_CHAN(cmd->chanlist[0]);

	devpriv->ntrig = cmd->stop_arg;
	outb(chan | DT2814_ENB | (trigvar << 5), dev->iobase + DT2814_CSR);

	return 0;

}

static int dt2814_attach(struct comedi_device *dev, struct comedi_devconfig *it)
{
	int i, irq;
	int ret;
	struct comedi_subdevice *s;
	unsigned long iobase;

	iobase = it->options[0];
	printk("comedi%d: dt2814: 0x%04lx ", dev->minor, iobase);
	if (!request_region(iobase, DT2814_SIZE, "dt2814")) {
		printk("I/O port conflict\n");
		return -EIO;
	}
	dev->iobase = iobase;
	dev->board_name = "dt2814";

	outb(0, dev->iobase + DT2814_CSR);
	udelay(100);
	if (inb(dev->iobase + DT2814_CSR) & DT2814_ERR) {
		printk("reset error (fatal)\n");
		return -EIO;
	}
	i = inb(dev->iobase + DT2814_DATA);
	i = inb(dev->iobase + DT2814_DATA);

	irq = it->options[1];
#if 0
	if (irq < 0) {
		save_flags(flags);
		sti();
		irqs = probe_irq_on();

		outb(0, dev->iobase + DT2814_CSR);

		udelay(100);

		irq = probe_irq_off(irqs);
		restore_flags(flags);
		if (inb(dev->iobase + DT2814_CSR) & DT2814_ERR) {
			printk("error probing irq (bad) \n");
		}

		i = inb(dev->iobase + DT2814_DATA);
		i = inb(dev->iobase + DT2814_DATA);
	}
#endif
	dev->irq = 0;
	if (irq > 0) {
		if (request_irq(irq, dt2814_interrupt, 0, "dt2814", dev)) {
			printk("(irq %d unavailable)\n", irq);
		} else {
			printk("( irq = %d )\n", irq);
			dev->irq = irq;
		}
	} else if (irq == 0) {
		printk("(no irq)\n");
	} else {
#if 0
		printk("(probe returned multiple irqs--bad)\n");
#else
		printk("(irq probe not implemented)\n");
#endif
	}

	ret = alloc_subdevices(dev, 1);
	if (ret < 0)
		return ret;

	ret = alloc_private(dev, sizeof(struct dt2814_private));
	if (ret < 0)
		return ret;

	s = dev->subdevices + 0;
	dev->read_subdev = s;
	s->type = COMEDI_SUBD_AI;
	s->subdev_flags = SDF_READABLE | SDF_GROUND | SDF_CMD_READ;
	s->n_chan = 16;		
	s->len_chanlist = 1;
	s->insn_read = dt2814_ai_insn_read;
	s->do_cmd = dt2814_ai_cmd;
	s->do_cmdtest = dt2814_ai_cmdtest;
	s->maxdata = 0xfff;
	s->range_table = &range_unknown;	

	return 0;
}

static int dt2814_detach(struct comedi_device *dev)
{
	printk("comedi%d: dt2814: remove\n", dev->minor);

	if (dev->irq) {
		free_irq(dev->irq, dev);
	}
	if (dev->iobase) {
		release_region(dev->iobase, DT2814_SIZE);
	}

	return 0;
}

static irqreturn_t dt2814_interrupt(int irq, void *d)
{
	int lo, hi;
	struct comedi_device *dev = d;
	struct comedi_subdevice *s;
	int data;

	if (!dev->attached) {
		comedi_error(dev, "spurious interrupt");
		return IRQ_HANDLED;
	}

	s = dev->subdevices + 0;

	hi = inb(dev->iobase + DT2814_DATA);
	lo = inb(dev->iobase + DT2814_DATA);

	data = (hi << 4) | (lo >> 4);

	if (!(--devpriv->ntrig)) {
		int i;

		outb(0, dev->iobase + DT2814_CSR);
		

		for (i = 0; i < DT2814_TIMEOUT; i++) {
			if (inb(dev->iobase + DT2814_CSR) & DT2814_FINISH)
				break;
		}
		inb(dev->iobase + DT2814_DATA);
		inb(dev->iobase + DT2814_DATA);

		s->async->events |= COMEDI_CB_EOA;
	}
	comedi_event(dev, s);
	return IRQ_HANDLED;
}
