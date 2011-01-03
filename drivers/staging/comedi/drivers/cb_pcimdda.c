




#include "../comedidev.h"

#include "comedi_pci.h"

#include "8255.h"


#define PCI_ID_PCIM_DDA06_16 0x0053


struct board_struct {
	const char *name;
	unsigned short device_id;
	int ao_chans;
	int ao_bits;
	int dio_chans;
	int dio_method;
	int dio_offset;		
	int regs_badrindex;	
	int reg_sz;		
};

enum DIO_METHODS {
	DIO_NONE = 0,
	DIO_8255,
	DIO_INTERNAL		
};

static const struct board_struct boards[] = {
	{
	 .name = "cb_pcimdda06-16",
	 .device_id = PCI_ID_PCIM_DDA06_16,
	 .ao_chans = 6,
	 .ao_bits = 16,
	 .dio_chans = 24,
	 .dio_method = DIO_8255,
	 .dio_offset = 12,
	 .regs_badrindex = 3,
	 .reg_sz = 16,
	 }
};


#define thisboard    ((const struct board_struct *)dev->board_ptr)

#define REG_SZ (thisboard->reg_sz)
#define REGS_BADRINDEX (thisboard->regs_badrindex)



static DEFINE_PCI_DEVICE_TABLE(pci_table) = {
	{
	PCI_VENDOR_ID_COMPUTERBOARDS, PCI_ID_PCIM_DDA06_16, PCI_ANY_ID,
		    PCI_ANY_ID, 0, 0, 0}, {
	0}
};

MODULE_DEVICE_TABLE(pci, pci_table);


struct board_private_struct {
	unsigned long registers;	
	unsigned long dio_registers;
	char attached_to_8255;	
	char attached_successfully;	
	
	struct pci_dev *pci_dev;

#define MAX_AO_READBACK_CHANNELS 6
	
	unsigned int ao_readback[MAX_AO_READBACK_CHANNELS];

};


#define devpriv ((struct board_private_struct *)dev->private)


static int attach(struct comedi_device *dev, struct comedi_devconfig *it);
static int detach(struct comedi_device *dev);
static struct comedi_driver cb_pcimdda_driver = {
	.driver_name = "cb_pcimdda",
	.module = THIS_MODULE,
	.attach = attach,
	.detach = detach,
};

MODULE_AUTHOR("Calin A. Culianu <calin@rtlab.org>");
MODULE_DESCRIPTION("Comedi low-level driver for the Computerboards PCIM-DDA "
		   "series.  Currently only supports PCIM-DDA06-16 (which "
		   "also happens to be the only board in this series. :) ) ");
MODULE_LICENSE("GPL");
COMEDI_PCI_INITCLEANUP_NOMODULE(cb_pcimdda_driver, pci_table);

static int ao_winsn(struct comedi_device *dev, struct comedi_subdevice *s,
		    struct comedi_insn *insn, unsigned int *data);
static int ao_rinsn(struct comedi_device *dev, struct comedi_subdevice *s,
		    struct comedi_insn *insn, unsigned int *data);




static inline unsigned int figure_out_maxdata(int bits)
{
	return ((unsigned int)1 << bits) - 1;
}


static int probe(struct comedi_device *dev, const struct comedi_devconfig *it);




static int attach(struct comedi_device *dev, struct comedi_devconfig *it)
{
	struct comedi_subdevice *s;
	int err;


	if (alloc_private(dev, sizeof(struct board_private_struct)) < 0)
		return -ENOMEM;


	err = probe(dev, it);
	if (err)
		return err;


	printk("comedi%d: %s: ", dev->minor, thisboard->name);


	dev->board_name = thisboard->name;


	if (alloc_subdevices(dev, 2) < 0)
		return -ENOMEM;

	s = dev->subdevices + 0;

	
	s->type = COMEDI_SUBD_AO;
	s->subdev_flags = SDF_WRITABLE | SDF_READABLE;
	s->n_chan = thisboard->ao_chans;
	s->maxdata = figure_out_maxdata(thisboard->ao_bits);
	
	if (it->options[2]) {
		s->range_table = &range_bipolar10;
	} else {
		s->range_table = &range_bipolar5;
	}
	s->insn_write = &ao_winsn;
	s->insn_read = &ao_rinsn;

	s = dev->subdevices + 1;
	
	if (thisboard->dio_chans) {
		switch (thisboard->dio_method) {
		case DIO_8255:
			
			subdev_8255_init(dev, s, NULL, devpriv->dio_registers);
			devpriv->attached_to_8255 = 1;
			break;
		case DIO_INTERNAL:
		default:
			printk("DIO_INTERNAL not implemented yet!\n");
			return -ENXIO;
			break;
		}
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	devpriv->attached_successfully = 1;

	printk("attached\n");

	return 1;
}


static int detach(struct comedi_device *dev)
{
	if (devpriv) {

		if (dev->subdevices && devpriv->attached_to_8255) {
			
			subdev_8255_cleanup(dev, dev->subdevices + 2);
			devpriv->attached_to_8255 = 0;
		}

		if (devpriv->pci_dev) {
			if (devpriv->registers) {
				comedi_pci_disable(devpriv->pci_dev);
			}
			pci_dev_put(devpriv->pci_dev);
		}

		if (devpriv->attached_successfully && thisboard)
			printk("comedi%d: %s: detached\n", dev->minor,
			       thisboard->name);

	}

	return 0;
}

static int ao_winsn(struct comedi_device *dev, struct comedi_subdevice *s,
		    struct comedi_insn *insn, unsigned int *data)
{
	int i;
	int chan = CR_CHAN(insn->chanspec);
	unsigned long offset = devpriv->registers + chan * 2;

	
	for (i = 0; i < insn->n; i++) {
		
		outb((char)(data[i] & 0x00ff), offset);
		
		outb((char)(data[i] >> 8 & 0x00ff), offset + 1);

		
		devpriv->ao_readback[chan] = data[i];
	}

	
	return i;
}


static int ao_rinsn(struct comedi_device *dev, struct comedi_subdevice *s,
		    struct comedi_insn *insn, unsigned int *data)
{
	int i;
	int chan = CR_CHAN(insn->chanspec);

	for (i = 0; i < insn->n; i++) {
		inw(devpriv->registers + chan * 2);
		
		data[i] = devpriv->ao_readback[chan];
	}

	return i;
}




static int probe(struct comedi_device *dev, const struct comedi_devconfig *it)
{
	struct pci_dev *pcidev;
	int index;
	unsigned long registers;

	for (pcidev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, NULL);
	     pcidev != NULL;
	     pcidev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pcidev)) {
		
		if (pcidev->vendor != PCI_VENDOR_ID_COMPUTERBOARDS)
			continue;
		
		for (index = 0; index < ARRAY_SIZE(boards); index++) {
			if (boards[index].device_id != pcidev->device)
				continue;
			
			if (it->options[0] || it->options[1]) {
				
				if (pcidev->bus->number != it->options[0] ||
				    PCI_SLOT(pcidev->devfn) != it->options[1]) {
					continue;
				}
			}
			

			devpriv->pci_dev = pcidev;
			dev->board_ptr = boards + index;
			if (comedi_pci_enable(pcidev, thisboard->name)) {
				printk
				    ("cb_pcimdda: Failed to enable PCI device and request regions\n");
				return -EIO;
			}
			registers =
			    pci_resource_start(devpriv->pci_dev,
					       REGS_BADRINDEX);
			devpriv->registers = registers;
			devpriv->dio_registers
			    = devpriv->registers + thisboard->dio_offset;
			return 0;
		}
	}

	printk("cb_pcimdda: No supported ComputerBoards/MeasurementComputing "
	       "card found at the requested position\n");
	return -ENODEV;
}
