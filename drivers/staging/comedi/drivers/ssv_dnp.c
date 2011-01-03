




#include "../comedidev.h"









#define CSCIR 0x22		
#define CSCDR 0x23		
#define PAMR  0xa5		
#define PADR  0xa9		
#define PBMR  0xa4		
#define PBDR  0xa8		
#define PCMR  0xa3		
#define PCDR  0xa7		



struct dnp_board {
	const char *name;
	int ai_chans;
	int ai_bits;
	int have_dio;
};

static const struct dnp_board dnp_boards[] = {	
	{			
	 .name = "dnp-1486",
	 .ai_chans = 16,
	 .ai_bits = 12,
	 .have_dio = 1,
	 },
};


#define thisboard ((const struct dnp_board *)dev->board_ptr)


struct dnp_private_data {

};


#define devpriv ((dnp_private *)dev->private)









static int dnp_attach(struct comedi_device *dev, struct comedi_devconfig *it);
static int dnp_detach(struct comedi_device *dev);

static struct comedi_driver driver_dnp = {
	.driver_name = "ssv_dnp",
	.module = THIS_MODULE,
	.attach = dnp_attach,
	.detach = dnp_detach,
	.board_name = &dnp_boards[0].name,
	
	.offset = sizeof(struct dnp_board),	
	.num_names = ARRAY_SIZE(dnp_boards),
};

COMEDI_INITCLEANUP(driver_dnp);

static int dnp_dio_insn_bits(struct comedi_device *dev,
			     struct comedi_subdevice *s,
			     struct comedi_insn *insn, unsigned int *data);

static int dnp_dio_insn_config(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data);







static int dnp_attach(struct comedi_device *dev, struct comedi_devconfig *it)
{

	struct comedi_subdevice *s;

	printk("comedi%d: dnp: ", dev->minor);

	
	
	

	
	dev->board_name = thisboard->name;

	
	
	if (alloc_private(dev, sizeof(struct dnp_private_data)) < 0)
		return -ENOMEM;

	
	

	if (alloc_subdevices(dev, 1) < 0)
		return -ENOMEM;

	s = dev->subdevices + 0;
	
	s->type = COMEDI_SUBD_DIO;
	s->subdev_flags = SDF_READABLE | SDF_WRITABLE;
	s->n_chan = 20;
	s->maxdata = 1;
	s->range_table = &range_digital;
	s->insn_bits = dnp_dio_insn_bits;
	s->insn_config = dnp_dio_insn_config;

	printk("attached\n");

	

	
	outb(PAMR, CSCIR);
	outb(0x00, CSCDR);
	outb(PBMR, CSCIR);
	outb(0x00, CSCDR);
	outb(PCMR, CSCIR);
	outb((inb(CSCDR) & 0xAA), CSCDR);

	return 1;

}









static int dnp_detach(struct comedi_device *dev)
{

	
	outb(PAMR, CSCIR);
	outb(0x00, CSCDR);
	outb(PBMR, CSCIR);
	outb(0x00, CSCDR);
	outb(PCMR, CSCIR);
	outb((inb(CSCDR) & 0xAA), CSCDR);

	
	printk("comedi%d: dnp: remove\n", dev->minor);

	return 0;

}







static int dnp_dio_insn_bits(struct comedi_device *dev,
			     struct comedi_subdevice *s,
			     struct comedi_insn *insn, unsigned int *data)
{

	if (insn->n != 2)
		return -EINVAL;	

	
	

	
	
	

	if (data[0]) {

		outb(PADR, CSCIR);
		outb((inb(CSCDR)
		      & ~(u8) (data[0] & 0x0000FF))
		     | (u8) (data[1] & 0x0000FF), CSCDR);

		outb(PBDR, CSCIR);
		outb((inb(CSCDR)
		      & ~(u8) ((data[0] & 0x00FF00) >> 8))
		     | (u8) ((data[1] & 0x00FF00) >> 8), CSCDR);

		outb(PCDR, CSCIR);
		outb((inb(CSCDR)
		      & ~(u8) ((data[0] & 0x0F0000) >> 12))
		     | (u8) ((data[1] & 0x0F0000) >> 12), CSCDR);
	}

	
	outb(PADR, CSCIR);
	data[0] = inb(CSCDR);
	outb(PBDR, CSCIR);
	data[0] += inb(CSCDR) << 8;
	outb(PCDR, CSCIR);
	data[0] += ((inb(CSCDR) & 0xF0) << 12);

	return 2;

}







static int dnp_dio_insn_config(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data)
{

	u8 register_buffer;

	int chan = CR_CHAN(insn->chanspec);	

	switch (data[0]) {
	case INSN_CONFIG_DIO_OUTPUT:
	case INSN_CONFIG_DIO_INPUT:
		break;
	case INSN_CONFIG_DIO_QUERY:
		data[1] =
		    (inb(CSCDR) & (1 << chan)) ? COMEDI_OUTPUT : COMEDI_INPUT;
		return insn->n;
		break;
	default:
		return -EINVAL;
		break;
	}
	

	
	
	

	if ((chan >= 0) && (chan <= 7)) {
		
		outb(PAMR, CSCIR);
	} else if ((chan >= 8) && (chan <= 15)) {
		
		chan -= 8;
		outb(PBMR, CSCIR);
	} else if ((chan >= 16) && (chan <= 19)) {
		
		
		chan -= 16;
		chan *= 2;
		outb(PCMR, CSCIR);
	} else {
		return -EINVAL;
	}

	
	register_buffer = inb(CSCDR);
	if (data[0] == COMEDI_OUTPUT) {
		register_buffer |= (1 << chan);
	} else {
		register_buffer &= ~(1 << chan);
	}
	outb(register_buffer, CSCDR);

	return 1;

}
