


#include "../comedidev.h"


#define ADQ12B_SIZE     16

#define ADQ12B_CTREG    0x00
#define ADQ12B_STINR    0x00
#define ADQ12B_OUTBR    0x04
#define ADQ12B_ADLOW    0x08
#define ADQ12B_ADHIG    0x09
#define ADQ12B_CONT0    0x0c
#define ADQ12B_CONT1    0x0d
#define ADQ12B_CONT2    0x0e
#define ADQ12B_COWORD   0x0f


#define ADQ12B_EOC     0x20

#define TIMEOUT        20


static const struct comedi_lrange range_adq12b_ai_bipolar = { 4, {
								  BIP_RANGE(5),
								  BIP_RANGE(2),
								  BIP_RANGE(1),
								  BIP_RANGE(0.5)
								  }
};

static const struct comedi_lrange range_adq12b_ai_unipolar = { 4, {
								   UNI_RANGE(5),
								   UNI_RANGE(2),
								   UNI_RANGE(1),
								   UNI_RANGE
								   (0.5)
								   }
};

struct adq12b_board {
	const char *name;
	int ai_se_chans;
	int ai_diff_chans;
	int ai_bits;
	int di_chans;
	int do_chans;
};

static const struct adq12b_board adq12b_boards[] = {
	{
	 .name = "adq12b",
	 .ai_se_chans = 16,
	 .ai_diff_chans = 8,
	 .ai_bits = 12,
	 .di_chans = 5,
	 .do_chans = 8}


};

#define thisboard ((const struct adq12b_board *)dev->board_ptr)

struct adq12b_private {
	int unipolar;		
	int differential;	
	int last_channel;
	int last_range;
	unsigned int digital_state;
};

#define devpriv ((struct adq12b_private *)dev->private)


static int adq12b_attach(struct comedi_device *dev,
			 struct comedi_devconfig *it);
static int adq12b_detach(struct comedi_device *dev);
static struct comedi_driver driver_adq12b = {
driver_name:"adq12b",
module:THIS_MODULE,
attach:adq12b_attach,
detach:adq12b_detach,
board_name:&adq12b_boards[0].name,
offset:sizeof(struct adq12b_board),
num_names:ARRAY_SIZE(adq12b_boards),
};

static int adq12b_ai_rinsn(struct comedi_device *dev,
			   struct comedi_subdevice *s, struct comedi_insn *insn,
			   unsigned int *data);
static int adq12b_di_insn_bits(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data);
static int adq12b_do_insn_bits(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data);


static int adq12b_attach(struct comedi_device *dev, struct comedi_devconfig *it)
{
	struct comedi_subdevice *s;
	unsigned long iobase;
	int unipolar, differential;

	iobase = it->options[0];
	unipolar = it->options[1];
	differential = it->options[2];

	printk("comedi%d: adq12b called with options base=0x%03lx, %s and %s\n",
	       dev->minor, iobase, (unipolar == 1) ? "unipolar" : "bipolar",
	       (differential == 1) ? "differential" : "single-ended");

	
	if (iobase == 0) {
		printk
		    ("comedi%d: adq12b warning: I/O base address not specified. Trying the default 0x300.\n",
		     dev->minor);
		iobase = 0x300;
	}

	printk("comedi%d: adq12b: 0x%04lx ", dev->minor, iobase);
	if (!request_region(iobase, ADQ12B_SIZE, "adq12b")) {
		printk("I/O port conflict\n");
		return -EIO;
	}
	dev->iobase = iobase;


	dev->board_name = thisboard->name;


	if (alloc_private(dev, sizeof(struct adq12b_private)) < 0)
		return -ENOMEM;


	devpriv->unipolar = unipolar;
	devpriv->differential = differential;
	devpriv->digital_state = 0;

	devpriv->last_channel = -1;
	devpriv->last_range = -1;


	if (alloc_subdevices(dev, 3) < 0)
		return -ENOMEM;

	s = dev->subdevices + 0;
	
	s->type = COMEDI_SUBD_AI;
	if (differential) {
		s->subdev_flags = SDF_READABLE | SDF_GROUND | SDF_DIFF;
		s->n_chan = thisboard->ai_diff_chans;
	} else {
		s->subdev_flags = SDF_READABLE | SDF_GROUND;
		s->n_chan = thisboard->ai_se_chans;
	}

	if (unipolar) {
		s->range_table = &range_adq12b_ai_unipolar;
	} else {
		s->range_table = &range_adq12b_ai_bipolar;
	}

	s->maxdata = (1 << thisboard->ai_bits) - 1;

	s->len_chanlist = 4;	
	s->insn_read = adq12b_ai_rinsn;

	s = dev->subdevices + 1;
	
	s->type = COMEDI_SUBD_DI;
	s->subdev_flags = SDF_READABLE;
	s->n_chan = thisboard->di_chans;
	s->maxdata = 1;
	s->range_table = &range_digital;
	s->insn_bits = adq12b_di_insn_bits;

	s = dev->subdevices + 2;
	
	s->type = COMEDI_SUBD_DO;
	s->subdev_flags = SDF_WRITABLE;
	s->n_chan = thisboard->do_chans;
	s->maxdata = 1;
	s->range_table = &range_digital;
	s->insn_bits = adq12b_do_insn_bits;

	printk("attached\n");

	return 0;
}


static int adq12b_detach(struct comedi_device *dev)
{
	if (dev->iobase)
		release_region(dev->iobase, ADQ12B_SIZE);

	kfree(devpriv);

	printk("comedi%d: adq12b: removed\n", dev->minor);

	return 0;
}



static int adq12b_ai_rinsn(struct comedi_device *dev,
			   struct comedi_subdevice *s, struct comedi_insn *insn,
			   unsigned int *data)
{
	int n, i;
	int range, channel;
	unsigned char hi, lo, status;

	
	range = CR_RANGE(insn->chanspec);
	channel = CR_CHAN(insn->chanspec);
	if (channel != devpriv->last_channel || range != devpriv->last_range) {
		outb((range << 4) | channel, dev->iobase + ADQ12B_CTREG);
		udelay(50);	
	}

	
	status = inb(dev->iobase + ADQ12B_ADLOW);

	
	for (n = 0; n < insn->n; n++) {

		
		i = 0;
		do {

			status = inb(dev->iobase + ADQ12B_STINR);
			status = status & ADQ12B_EOC;
		} while (status == 0 && ++i < TIMEOUT);


		
		hi = inb(dev->iobase + ADQ12B_ADHIG);
		lo = inb(dev->iobase + ADQ12B_ADLOW);

		
		data[n] = (hi << 8) | lo;

	}

	
	return n;
}

static int adq12b_di_insn_bits(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data)
{

	
	data[1] = (inb(dev->iobase + ADQ12B_STINR) & (0x1f));

	return 2;
}

static int adq12b_do_insn_bits(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data)
{
	int channel;

	for (channel = 0; channel < 8; channel++)
		if (((data[0] >> channel) & 0x01) != 0)
			outb((((data[1] >> channel) & 0x01) << 3) | channel,
			     dev->iobase + ADQ12B_OUTBR);

	
	if (data[0]) {
		devpriv->digital_state &= ~data[0];
		devpriv->digital_state |= (data[0] & data[1]);
	}

	data[1] = devpriv->digital_state;

	return 2;
}


COMEDI_INITCLEANUP(driver_adq12b);
