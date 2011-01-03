


#include "../comedidev.h"

#include <linux/delay.h>
#include <linux/interrupt.h>

#include "comedi_pci.h"
#include "plx9052.h"
#include "8255.h"


#undef CBPCIMDAS_DEBUG




#define BADR0_SIZE 2		
#define BADR1_SIZE 4
#define BADR2_SIZE 6
#define BADR3_SIZE 16
#define BADR4_SIZE 4


#define ADC_TRIG 0
#define DAC0_OFFSET 2
#define DAC1_OFFSET 4


#define MUX_LIMITS 0
#define MAIN_CONN_DIO 1
#define ADC_STAT 2
#define ADC_CONV_STAT 3
#define ADC_INT 4
#define ADC_PACER 5
#define BURST_MODE 6
#define PROG_GAIN 7
#define CLK8254_1_DATA 8
#define CLK8254_2_DATA 9
#define CLK8254_3_DATA 10
#define CLK8254_CONTROL 11
#define USER_COUNTER 12
#define RESID_COUNT_H 13
#define RESID_COUNT_L 14


struct cb_pcimdas_board {
	const char *name;
	unsigned short device_id;
	int ai_se_chans;	
	int ai_diff_chans;	
	int ai_bits;		
	int ai_speed;		
	int ao_nchan;		
	int ao_bits;		
	int has_ao_fifo;	
	int ao_scan_speed;	
	int fifo_size;		
	int dio_bits;		
	int has_dio;		
	const struct comedi_lrange *ranges;
};

static const struct cb_pcimdas_board cb_pcimdas_boards[] = {
	{
	 .name = "PCIM-DAS1602/16",
	 .device_id = 0x56,
	 .ai_se_chans = 16,
	 .ai_diff_chans = 8,
	 .ai_bits = 16,
	 .ai_speed = 10000,	
	 .ao_nchan = 2,
	 .ao_bits = 12,
	 .has_ao_fifo = 0,	
	 .ao_scan_speed = 10000,
	 
	 .fifo_size = 1024,
	 .dio_bits = 24,
	 .has_dio = 1,

	 },
};


static DEFINE_PCI_DEVICE_TABLE(cb_pcimdas_pci_table) = {
	{
	PCI_VENDOR_ID_COMPUTERBOARDS, 0x0056, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{
	0}
};

MODULE_DEVICE_TABLE(pci, cb_pcimdas_pci_table);

#define N_BOARDS 1		


#define thisboard ((const struct cb_pcimdas_board *)dev->board_ptr)


struct cb_pcimdas_private {
	int data;

	
	struct pci_dev *pci_dev;

	
	unsigned long BADR0;
	unsigned long BADR1;
	unsigned long BADR2;
	unsigned long BADR3;
	unsigned long BADR4;

	
	unsigned int ao_readback[2];

	
	unsigned short int port_a;	
	unsigned short int port_b;	
	unsigned short int port_c;	
	unsigned short int dio_mode;	

};


#define devpriv ((struct cb_pcimdas_private *)dev->private)


static int cb_pcimdas_attach(struct comedi_device *dev,
			     struct comedi_devconfig *it);
static int cb_pcimdas_detach(struct comedi_device *dev);
static struct comedi_driver driver_cb_pcimdas = {
	.driver_name = "cb_pcimdas",
	.module = THIS_MODULE,
	.attach = cb_pcimdas_attach,
	.detach = cb_pcimdas_detach,
};

static int cb_pcimdas_ai_rinsn(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data);
static int cb_pcimdas_ao_winsn(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data);
static int cb_pcimdas_ao_rinsn(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data);


static int cb_pcimdas_attach(struct comedi_device *dev,
			     struct comedi_devconfig *it)
{
	struct comedi_subdevice *s;
	struct pci_dev *pcidev;
	int index;
	

	printk("comedi%d: cb_pcimdas: ", dev->minor);


	if (alloc_private(dev, sizeof(struct cb_pcimdas_private)) < 0)
		return -ENOMEM;


	printk("\n");

	for (pcidev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, NULL);
	     pcidev != NULL;
	     pcidev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pcidev)) {
		
		if (pcidev->vendor != PCI_VENDOR_ID_COMPUTERBOARDS)
			continue;
		
		for (index = 0; index < N_BOARDS; index++) {
			if (cb_pcimdas_boards[index].device_id !=
			    pcidev->device)
				continue;
			
			if (it->options[0] || it->options[1]) {
				
				if (pcidev->bus->number != it->options[0] ||
				    PCI_SLOT(pcidev->devfn) != it->options[1]) {
					continue;
				}
			}
			devpriv->pci_dev = pcidev;
			dev->board_ptr = cb_pcimdas_boards + index;
			goto found;
		}
	}

	printk("No supported ComputerBoards/MeasurementComputing card found on "
	       "requested position\n");
	return -EIO;

found:

	printk("Found %s on bus %i, slot %i\n", cb_pcimdas_boards[index].name,
	       pcidev->bus->number, PCI_SLOT(pcidev->devfn));

	
	switch (thisboard->device_id) {
	case 0x56:
		break;
	default:
		printk("THIS CARD IS UNSUPPORTED.\n"
		       "PLEASE REPORT USAGE TO <mocelet@sucs.org>\n");
	};

	if (comedi_pci_enable(pcidev, "cb_pcimdas")) {
		printk(" Failed to enable PCI device and request regions\n");
		return -EIO;
	}

	devpriv->BADR0 = pci_resource_start(devpriv->pci_dev, 0);
	devpriv->BADR1 = pci_resource_start(devpriv->pci_dev, 1);
	devpriv->BADR2 = pci_resource_start(devpriv->pci_dev, 2);
	devpriv->BADR3 = pci_resource_start(devpriv->pci_dev, 3);
	devpriv->BADR4 = pci_resource_start(devpriv->pci_dev, 4);

#ifdef CBPCIMDAS_DEBUG
	printk("devpriv->BADR0 = 0x%lx\n", devpriv->BADR0);
	printk("devpriv->BADR1 = 0x%lx\n", devpriv->BADR1);
	printk("devpriv->BADR2 = 0x%lx\n", devpriv->BADR2);
	printk("devpriv->BADR3 = 0x%lx\n", devpriv->BADR3);
	printk("devpriv->BADR4 = 0x%lx\n", devpriv->BADR4);
#endif










	
	dev->board_name = thisboard->name;


	if (alloc_subdevices(dev, 3) < 0)
		return -ENOMEM;

	s = dev->subdevices + 0;
	
	
	s->type = COMEDI_SUBD_AI;
	s->subdev_flags = SDF_READABLE | SDF_GROUND;
	s->n_chan = thisboard->ai_se_chans;
	s->maxdata = (1 << thisboard->ai_bits) - 1;
	s->range_table = &range_unknown;
	s->len_chanlist = 1;	
	
	s->insn_read = cb_pcimdas_ai_rinsn;

	s = dev->subdevices + 1;
	
	s->type = COMEDI_SUBD_AO;
	s->subdev_flags = SDF_WRITABLE;
	s->n_chan = thisboard->ao_nchan;
	s->maxdata = 1 << thisboard->ao_bits;
	s->range_table = &range_unknown;	
	s->insn_write = &cb_pcimdas_ao_winsn;
	s->insn_read = &cb_pcimdas_ao_rinsn;

	s = dev->subdevices + 2;
	
	if (thisboard->has_dio) {
		subdev_8255_init(dev, s, NULL, devpriv->BADR4);
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	printk("attached\n");

	return 1;
}


static int cb_pcimdas_detach(struct comedi_device *dev)
{
#ifdef CBPCIMDAS_DEBUG
	if (devpriv) {
		printk("devpriv->BADR0 = 0x%lx\n", devpriv->BADR0);
		printk("devpriv->BADR1 = 0x%lx\n", devpriv->BADR1);
		printk("devpriv->BADR2 = 0x%lx\n", devpriv->BADR2);
		printk("devpriv->BADR3 = 0x%lx\n", devpriv->BADR3);
		printk("devpriv->BADR4 = 0x%lx\n", devpriv->BADR4);
	}
#endif
	printk("comedi%d: cb_pcimdas: remove\n", dev->minor);
	if (dev->irq)
		free_irq(dev->irq, dev);
	if (devpriv) {
		if (devpriv->pci_dev) {
			if (devpriv->BADR0) {
				comedi_pci_disable(devpriv->pci_dev);
			}
			pci_dev_put(devpriv->pci_dev);
		}
	}

	return 0;
}


static int cb_pcimdas_ai_rinsn(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data)
{
	int n, i;
	unsigned int d;
	unsigned int busy;
	int chan = CR_CHAN(insn->chanspec);
	unsigned short chanlims;
	int maxchans;

	

	
	if ((inb(devpriv->BADR3 + 2) & 0x20) == 0)	
		maxchans = thisboard->ai_diff_chans;
	else
		maxchans = thisboard->ai_se_chans;

	if (chan > (maxchans - 1))
		return -ETIMEDOUT;	

	
	d = inb(devpriv->BADR3 + 5);
	if ((d & 0x03) > 0) {	
		d = d & 0xfd;
		outb(d, devpriv->BADR3 + 5);
	}
	outb(0x01, devpriv->BADR3 + 6);	
	outb(0x00, devpriv->BADR3 + 7);	

	
	chanlims = chan | (chan << 4);
	outb(chanlims, devpriv->BADR3 + 0);

	
	for (n = 0; n < insn->n; n++) {
		
		outw(0, devpriv->BADR2 + 0);

#define TIMEOUT 1000		
		

		
		for (i = 0; i < TIMEOUT; i++) {
			busy = inb(devpriv->BADR3 + 2) & 0x80;
			if (!busy)
				break;
		}
		if (i == TIMEOUT) {
			printk("timeout\n");
			return -ETIMEDOUT;
		}
		
		d = inw(devpriv->BADR2 + 0);

		
		

		data[n] = d;
	}

	
	return n;
}

static int cb_pcimdas_ao_winsn(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data)
{
	int i;
	int chan = CR_CHAN(insn->chanspec);

	
	for (i = 0; i < insn->n; i++) {
		switch (chan) {
		case 0:
			outw(data[i] & 0x0FFF, devpriv->BADR2 + DAC0_OFFSET);
			break;
		case 1:
			outw(data[i] & 0x0FFF, devpriv->BADR2 + DAC1_OFFSET);
			break;
		default:
			return -1;
		}
		devpriv->ao_readback[chan] = data[i];
	}

	
	return i;
}


static int cb_pcimdas_ao_rinsn(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data)
{
	int i;
	int chan = CR_CHAN(insn->chanspec);

	for (i = 0; i < insn->n; i++)
		data[i] = devpriv->ao_readback[chan];

	return i;
}


COMEDI_PCI_INITCLEANUP(driver_cb_pcimdas, cb_pcimdas_pci_table);
