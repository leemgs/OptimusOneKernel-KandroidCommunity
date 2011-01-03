



#include <linux/interrupt.h>
#include "../comedidev.h"

#include <linux/delay.h>
#include <linux/pci.h>

#include "icp_multi.h"

#define DEVICE_ID	0x8000	

#define ICP_MULTI_EXTDEBUG


#define TYPE_ICP_MULTI	0

#define IORANGE_ICP_MULTI 	32

#define ICP_MULTI_ADC_CSR	0	
#define ICP_MULTI_AI		2	
#define ICP_MULTI_DAC_CSR	4	
#define ICP_MULTI_AO		6	
#define ICP_MULTI_DI		8	
#define ICP_MULTI_DO		0x0A	
#define ICP_MULTI_INT_EN	0x0C	
#define ICP_MULTI_INT_STAT	0x0E	
#define ICP_MULTI_CNTR0		0x10	
#define ICP_MULTI_CNTR1		0x12	
#define ICP_MULTI_CNTR2		0x14	
#define ICP_MULTI_CNTR3		0x16	

#define ICP_MULTI_SIZE		0x20	


#define	ADC_ST		0x0001	
#define	ADC_BSY		0x0001	
#define ADC_BI		0x0010	
#define ADC_RA		0x0020	
#define	ADC_DI		0x0040	


#define	DAC_ST		0x0001	
#define DAC_BSY		0x0001	
#define	DAC_BI		0x0010	
#define	DAC_RA		0x0020	


#define	ADC_READY	0x0001	
#define	DAC_READY	0x0002	
#define	DOUT_ERROR	0x0004	
#define	DIN_STATUS	0x0008	
#define	CIE0		0x0010	
#define	CIE1		0x0020	
#define	CIE2		0x0040	
#define	CIE3		0x0080	


#define	Status_IRQ	0x00ff	


static const struct comedi_lrange range_analog = { 4, {
						       UNI_RANGE(5),
						       UNI_RANGE(10),
						       BIP_RANGE(5),
						       BIP_RANGE(10)
						       }
};

static const char range_codes_analog[] = { 0x00, 0x20, 0x10, 0x30 };


static int icp_multi_attach(struct comedi_device *dev,
			    struct comedi_devconfig *it);
static int icp_multi_detach(struct comedi_device *dev);


static unsigned short pci_list_builded = 0;	

struct boardtype {
	const char *name;	
	int device_id;
	int iorange;		
	char have_irq;		
	char cardtype;		
	int n_aichan;		
	int n_aichand;		
	int n_aochan;		
	int n_dichan;		
	int n_dochan;		
	int n_ctrs;		
	int ai_maxdata;		
	int ao_maxdata;		
	const struct comedi_lrange *rangelist_ai;	
	const char *rangecode;	
	const struct comedi_lrange *rangelist_ao;	
};

static const struct boardtype boardtypes[] = {
	{"icp_multi",		
	 DEVICE_ID,		
	 IORANGE_ICP_MULTI,	
	 1,			
	 TYPE_ICP_MULTI,	
	 16,			
	 8,			
	 4,			
	 16,			
	 8,			
	 4,			
	 0x0fff,		
	 0x0fff,		
	 &range_analog,		
	 range_codes_analog,	
	 &range_analog},	
};

#define n_boardtypes (sizeof(boardtypes)/sizeof(struct boardtype))

static struct comedi_driver driver_icp_multi = {
driver_name:"icp_multi",
module:THIS_MODULE,
attach:icp_multi_attach,
detach:icp_multi_detach,
num_names:n_boardtypes,
board_name:&boardtypes[0].name,
offset:sizeof(struct boardtype),
};

COMEDI_INITCLEANUP(driver_icp_multi);

struct icp_multi_private {
	struct pcilst_struct *card;	
	char valid;		
	void *io_addr;		
	resource_size_t phys_iobase;	
	unsigned int AdcCmdStatus;	
	unsigned int DacCmdStatus;	
	unsigned int IntEnable;	
	unsigned int IntStatus;	
	unsigned int act_chanlist[32];	
	unsigned char act_chanlist_len;	
	unsigned char act_chanlist_pos;	
	unsigned int *ai_chanlist;	
	short *ai_data;		
	short ao_data[4];	
	short di_data;		
	unsigned int do_data;	
};

#define devpriv ((struct icp_multi_private *)dev->private)
#define this_board ((const struct boardtype *)dev->board_ptr)



#if 0
static int check_channel_list(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      unsigned int *chanlist, unsigned int n_chan);
#endif
static void setup_channel_list(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       unsigned int *chanlist, unsigned int n_chan);
static int icp_multi_reset(struct comedi_device *dev);




static int icp_multi_insn_read_ai(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data)
{
	int n, timeout;

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: BGN: icp_multi_insn_read_ai(...)\n");
#endif
	
	devpriv->IntEnable &= ~ADC_READY;
	writew(devpriv->IntEnable, devpriv->io_addr + ICP_MULTI_INT_EN);

	
	devpriv->IntStatus |= ADC_READY;
	writew(devpriv->IntStatus, devpriv->io_addr + ICP_MULTI_INT_STAT);

	
	setup_channel_list(dev, s, &insn->chanspec, 1);

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp_multi A ST=%4x IO=%p\n",
	       readw(devpriv->io_addr + ICP_MULTI_ADC_CSR),
	       devpriv->io_addr + ICP_MULTI_ADC_CSR);
#endif

	for (n = 0; n < insn->n; n++) {
		
		devpriv->AdcCmdStatus |= ADC_ST;
		writew(devpriv->AdcCmdStatus,
		       devpriv->io_addr + ICP_MULTI_ADC_CSR);
		devpriv->AdcCmdStatus &= ~ADC_ST;

#ifdef ICP_MULTI_EXTDEBUG
		printk("icp multi B n=%d ST=%4x\n", n,
		       readw(devpriv->io_addr + ICP_MULTI_ADC_CSR));
#endif

		udelay(1);

#ifdef ICP_MULTI_EXTDEBUG
		printk("icp multi C n=%d ST=%4x\n", n,
		       readw(devpriv->io_addr + ICP_MULTI_ADC_CSR));
#endif

		
		timeout = 100;
		while (timeout--) {
			if (!(readw(devpriv->io_addr +
				    ICP_MULTI_ADC_CSR) & ADC_BSY))
				goto conv_finish;

#ifdef ICP_MULTI_EXTDEBUG
			if (!(timeout % 10))
				printk("icp multi D n=%d tm=%d ST=%4x\n", n,
				       timeout,
				       readw(devpriv->io_addr +
					     ICP_MULTI_ADC_CSR));
#endif

			udelay(1);
		}

		
		comedi_error(dev, "A/D insn timeout");

		
		devpriv->IntEnable &= ~ADC_READY;
		writew(devpriv->IntEnable, devpriv->io_addr + ICP_MULTI_INT_EN);

		
		devpriv->IntStatus |= ADC_READY;
		writew(devpriv->IntStatus,
		       devpriv->io_addr + ICP_MULTI_INT_STAT);

		
		data[n] = 0;

#ifdef ICP_MULTI_EXTDEBUG
		printk
		    ("icp multi EDBG: END: icp_multi_insn_read_ai(...) n=%d\n",
		     n);
#endif
		return -ETIME;

conv_finish:
		data[n] =
		    (readw(devpriv->io_addr + ICP_MULTI_AI) >> 4) & 0x0fff;
	}

	
	devpriv->IntEnable &= ~ADC_READY;
	writew(devpriv->IntEnable, devpriv->io_addr + ICP_MULTI_INT_EN);

	
	devpriv->IntStatus |= ADC_READY;
	writew(devpriv->IntStatus, devpriv->io_addr + ICP_MULTI_INT_STAT);

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: END: icp_multi_insn_read_ai(...) n=%d\n", n);
#endif
	return n;
}


static int icp_multi_insn_write_ao(struct comedi_device *dev,
				   struct comedi_subdevice *s,
				   struct comedi_insn *insn, unsigned int *data)
{
	int n, chan, range, timeout;

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: BGN: icp_multi_insn_write_ao(...)\n");
#endif
	
	devpriv->IntEnable &= ~DAC_READY;
	writew(devpriv->IntEnable, devpriv->io_addr + ICP_MULTI_INT_EN);

	
	devpriv->IntStatus |= DAC_READY;
	writew(devpriv->IntStatus, devpriv->io_addr + ICP_MULTI_INT_STAT);

	
	chan = CR_CHAN(insn->chanspec);
	range = CR_RANGE(insn->chanspec);

	
	
	
	
	
	devpriv->DacCmdStatus &= 0xfccf;
	devpriv->DacCmdStatus |= this_board->rangecode[range];
	devpriv->DacCmdStatus |= (chan << 8);

	writew(devpriv->DacCmdStatus, devpriv->io_addr + ICP_MULTI_DAC_CSR);

	for (n = 0; n < insn->n; n++) {
		
		timeout = 100;
		while (timeout--) {
			if (!(readw(devpriv->io_addr +
				    ICP_MULTI_DAC_CSR) & DAC_BSY))
				goto dac_ready;

#ifdef ICP_MULTI_EXTDEBUG
			if (!(timeout % 10))
				printk("icp multi A n=%d tm=%d ST=%4x\n", n,
				       timeout,
				       readw(devpriv->io_addr +
					     ICP_MULTI_DAC_CSR));
#endif

			udelay(1);
		}

		
		comedi_error(dev, "D/A insn timeout");

		
		devpriv->IntEnable &= ~DAC_READY;
		writew(devpriv->IntEnable, devpriv->io_addr + ICP_MULTI_INT_EN);

		
		devpriv->IntStatus |= DAC_READY;
		writew(devpriv->IntStatus,
		       devpriv->io_addr + ICP_MULTI_INT_STAT);

		
		devpriv->ao_data[chan] = 0;

#ifdef ICP_MULTI_EXTDEBUG
		printk
		    ("icp multi EDBG: END: icp_multi_insn_write_ao(...) n=%d\n",
		     n);
#endif
		return -ETIME;

dac_ready:
		
		writew(data[n], devpriv->io_addr + ICP_MULTI_AO);

		
		devpriv->DacCmdStatus |= DAC_ST;
		writew(devpriv->DacCmdStatus,
		       devpriv->io_addr + ICP_MULTI_DAC_CSR);
		devpriv->DacCmdStatus &= ~DAC_ST;

		
		devpriv->ao_data[chan] = data[n];
	}

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: END: icp_multi_insn_write_ao(...) n=%d\n", n);
#endif
	return n;
}


static int icp_multi_insn_read_ao(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data)
{
	int n, chan;

	
	chan = CR_CHAN(insn->chanspec);

	
	for (n = 0; n < insn->n; n++)
		data[n] = devpriv->ao_data[chan];

	return n;
}


static int icp_multi_insn_bits_di(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data)
{
	data[1] = readw(devpriv->io_addr + ICP_MULTI_DI);

	return 2;
}


static int icp_multi_insn_bits_do(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data)
{
#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: BGN: icp_multi_insn_bits_do(...)\n");
#endif

	if (data[0]) {
		s->state &= ~data[0];
		s->state |= (data[0] & data[1]);

		printk("Digital outputs = %4x \n", s->state);

		writew(s->state, devpriv->io_addr + ICP_MULTI_DO);
	}

	data[1] = readw(devpriv->io_addr + ICP_MULTI_DI);

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: END: icp_multi_insn_bits_do(...)\n");
#endif
	return 2;
}


static int icp_multi_insn_read_ctr(struct comedi_device *dev,
				   struct comedi_subdevice *s,
				   struct comedi_insn *insn, unsigned int *data)
{
	return 0;
}


static int icp_multi_insn_write_ctr(struct comedi_device *dev,
				    struct comedi_subdevice *s,
				    struct comedi_insn *insn,
				    unsigned int *data)
{
	return 0;
}


static irqreturn_t interrupt_service_icp_multi(int irq, void *d)
{
	struct comedi_device *dev = d;
	int int_no;

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: BGN: interrupt_service_icp_multi(%d,...)\n",
	       irq);
#endif

	
	int_no = readw(devpriv->io_addr + ICP_MULTI_INT_STAT) & Status_IRQ;
	if (!int_no)
		
		return IRQ_NONE;

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: interrupt_service_icp_multi() ST: %4x\n",
	       readw(devpriv->io_addr + ICP_MULTI_INT_STAT));
#endif

	
	switch (int_no) {
	case ADC_READY:
		break;
	case DAC_READY:
		break;
	case DOUT_ERROR:
		break;
	case DIN_STATUS:
		break;
	case CIE0:
		break;
	case CIE1:
		break;
	case CIE2:
		break;
	case CIE3:
		break;
	default:
		break;

	}

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: END: interrupt_service_icp_multi(...)\n");
#endif
	return IRQ_HANDLED;
}

#if 0

static int check_channel_list(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      unsigned int *chanlist, unsigned int n_chan)
{
	unsigned int i;

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG:  check_channel_list(...,%d)\n", n_chan);
#endif
	
	if (n_chan < 1) {
		comedi_error(dev, "range/channel list is empty!");
		return 0;
	}
	
	for (i = 0; i < n_chan; i++) {
		
		if (CR_AREF(chanlist[i]) == AREF_DIFF) {
			if (CR_CHAN(chanlist[i]) > this_board->n_aichand) {
				comedi_error(dev,
					     "Incorrect differential ai channel number");
				return 0;
			}
		} else {
			if (CR_CHAN(chanlist[i]) > this_board->n_aichan) {
				comedi_error(dev,
					     "Incorrect ai channel number");
				return 0;
			}
		}
	}
	return 1;
}
#endif


static void setup_channel_list(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       unsigned int *chanlist, unsigned int n_chan)
{
	unsigned int i, range, chanprog;
	unsigned int diff;

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG:  setup_channel_list(...,%d)\n", n_chan);
#endif
	devpriv->act_chanlist_len = n_chan;
	devpriv->act_chanlist_pos = 0;

	for (i = 0; i < n_chan; i++) {
		
		chanprog = CR_CHAN(chanlist[i]);

		
		if (CR_AREF(chanlist[i]) == AREF_DIFF) {
			diff = 1;
			chanprog &= 0x0007;
		} else {
			diff = 0;
			chanprog &= 0x000f;
		}

		
		devpriv->AdcCmdStatus &= 0xf00f;

		
		if (diff) {
			
			devpriv->AdcCmdStatus |= (chanprog << 9);
			devpriv->AdcCmdStatus |= ADC_DI;
		} else
			
			devpriv->AdcCmdStatus |= (chanprog << 8);

		
		range = this_board->rangecode[CR_RANGE(chanlist[i])];
		
		devpriv->AdcCmdStatus |= range;

		
		writew(devpriv->AdcCmdStatus,
		       devpriv->io_addr + ICP_MULTI_ADC_CSR);

#ifdef ICP_MULTI_EXTDEBUG
		printk("GS: %2d. [%4x]=%4x %4x\n", i, chanprog, range,
		       devpriv->act_chanlist[i]);
#endif
	}

}


static int icp_multi_reset(struct comedi_device *dev)
{
	unsigned int i;

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp_multi EDBG: BGN: icp_multi_reset(...)\n");
#endif
	
	writew(0, devpriv->io_addr + ICP_MULTI_INT_EN);
	writew(0x00ff, devpriv->io_addr + ICP_MULTI_INT_STAT);

	if (this_board->n_aochan)
		
		for (i = 0; i < this_board->n_aochan; i++) {
			devpriv->DacCmdStatus &= 0xfcce;

			
			devpriv->DacCmdStatus |= (i << 8);

			
			writew(0, devpriv->io_addr + ICP_MULTI_AO);

			
			devpriv->DacCmdStatus |= DAC_ST;

			
			writew(devpriv->DacCmdStatus,
			       devpriv->io_addr + ICP_MULTI_DAC_CSR);

			
			udelay(1);
		}
	
	writew(0, devpriv->io_addr + ICP_MULTI_DO);

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: END: icp_multi_reset(...)\n");
#endif
	return 0;
}


static int icp_multi_attach(struct comedi_device *dev,
			    struct comedi_devconfig *it)
{
	struct comedi_subdevice *s;
	int ret, subdev, n_subdevices;
	unsigned int irq;
	struct pcilst_struct *card = NULL;
	resource_size_t io_addr[5], iobase;
	unsigned char pci_bus, pci_slot, pci_func;

	printk("icp_multi EDBG: BGN: icp_multi_attach(...)\n");

	
	ret = alloc_private(dev, sizeof(struct icp_multi_private));
	if (ret < 0)
		return ret;

	
	if (pci_list_builded++ == 0) {
		pci_card_list_init(PCI_VENDOR_ID_ICP,
#ifdef ICP_MULTI_EXTDEBUG
				   1
#else
				   0
#endif
		    );
	}

	printk("Anne's comedi%d: icp_multi: board=%s", dev->minor,
	       this_board->name);

	card = select_and_alloc_pci_card(PCI_VENDOR_ID_ICP,
					 this_board->device_id, it->options[0],
					 it->options[1]);

	if (card == NULL)
		return -EIO;

	devpriv->card = card;

	if ((pci_card_data(card, &pci_bus, &pci_slot, &pci_func, &io_addr[0],
			   &irq)) < 0) {
		printk(" - Can't get configuration data!\n");
		return -EIO;
	}

	iobase = io_addr[2];
	devpriv->phys_iobase = iobase;

	printk(", b:s:f=%d:%d:%d, io=0x%8llx \n", pci_bus, pci_slot, pci_func,
	       (unsigned long long)iobase);

	devpriv->io_addr = ioremap(iobase, ICP_MULTI_SIZE);

	if (devpriv->io_addr == NULL) {
		printk("ioremap failed.\n");
		return -ENOMEM;
	}
#ifdef ICP_MULTI_EXTDEBUG
	printk("0x%08llx mapped to %p, ", (unsigned long long)iobase,
	       devpriv->io_addr);
#endif

	dev->board_name = this_board->name;

	n_subdevices = 0;
	if (this_board->n_aichan)
		n_subdevices++;
	if (this_board->n_aochan)
		n_subdevices++;
	if (this_board->n_dichan)
		n_subdevices++;
	if (this_board->n_dochan)
		n_subdevices++;
	if (this_board->n_ctrs)
		n_subdevices++;

	ret = alloc_subdevices(dev, n_subdevices);
	if (ret < 0)
		return ret;

	icp_multi_reset(dev);

	if (this_board->have_irq) {
		if (irq) {
			if (request_irq(irq, interrupt_service_icp_multi,
					IRQF_SHARED, "Inova Icp Multi", dev)) {
				printk
				    (", unable to allocate IRQ %u, DISABLING IT",
				     irq);
				irq = 0;	
			} else
				printk(", irq=%u", irq);
		} else
			printk(", IRQ disabled");
	} else
		irq = 0;

	dev->irq = irq;

	printk(".\n");

	subdev = 0;

	if (this_board->n_aichan) {
		s = dev->subdevices + subdev;
		dev->read_subdev = s;
		s->type = COMEDI_SUBD_AI;
		s->subdev_flags = SDF_READABLE | SDF_COMMON | SDF_GROUND;
		if (this_board->n_aichand)
			s->subdev_flags |= SDF_DIFF;
		s->n_chan = this_board->n_aichan;
		s->maxdata = this_board->ai_maxdata;
		s->len_chanlist = this_board->n_aichan;
		s->range_table = this_board->rangelist_ai;
		s->insn_read = icp_multi_insn_read_ai;
		subdev++;
	}

	if (this_board->n_aochan) {
		s = dev->subdevices + subdev;
		s->type = COMEDI_SUBD_AO;
		s->subdev_flags = SDF_WRITABLE | SDF_GROUND | SDF_COMMON;
		s->n_chan = this_board->n_aochan;
		s->maxdata = this_board->ao_maxdata;
		s->len_chanlist = this_board->n_aochan;
		s->range_table = this_board->rangelist_ao;
		s->insn_write = icp_multi_insn_write_ao;
		s->insn_read = icp_multi_insn_read_ao;
		subdev++;
	}

	if (this_board->n_dichan) {
		s = dev->subdevices + subdev;
		s->type = COMEDI_SUBD_DI;
		s->subdev_flags = SDF_READABLE;
		s->n_chan = this_board->n_dichan;
		s->maxdata = 1;
		s->len_chanlist = this_board->n_dichan;
		s->range_table = &range_digital;
		s->io_bits = 0;
		s->insn_bits = icp_multi_insn_bits_di;
		subdev++;
	}

	if (this_board->n_dochan) {
		s = dev->subdevices + subdev;
		s->type = COMEDI_SUBD_DO;
		s->subdev_flags = SDF_WRITABLE | SDF_READABLE;
		s->n_chan = this_board->n_dochan;
		s->maxdata = 1;
		s->len_chanlist = this_board->n_dochan;
		s->range_table = &range_digital;
		s->io_bits = (1 << this_board->n_dochan) - 1;
		s->state = 0;
		s->insn_bits = icp_multi_insn_bits_do;
		subdev++;
	}

	if (this_board->n_ctrs) {
		s = dev->subdevices + subdev;
		s->type = COMEDI_SUBD_COUNTER;
		s->subdev_flags = SDF_WRITABLE | SDF_GROUND | SDF_COMMON;
		s->n_chan = this_board->n_ctrs;
		s->maxdata = 0xffff;
		s->len_chanlist = this_board->n_ctrs;
		s->state = 0;
		s->insn_read = icp_multi_insn_read_ctr;
		s->insn_write = icp_multi_insn_write_ctr;
		subdev++;
	}

	devpriv->valid = 1;

#ifdef ICP_MULTI_EXTDEBUG
	printk("icp multi EDBG: END: icp_multi_attach(...)\n");
#endif

	return 0;
}


static int icp_multi_detach(struct comedi_device *dev)
{

	if (dev->private)
		if (devpriv->valid)
			icp_multi_reset(dev);

	if (dev->irq)
		free_irq(dev->irq, dev);

	if (dev->private && devpriv->io_addr)
		iounmap(devpriv->io_addr);

	if (dev->private && devpriv->card)
		pci_card_free(devpriv->card);

	if (--pci_list_builded == 0)
		pci_card_list_cleanup(PCI_VENDOR_ID_ICP);

	return 0;
}
