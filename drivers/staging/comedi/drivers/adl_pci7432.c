


#include "../comedidev.h"
#include <linux/kernel.h>
#include "comedi_pci.h"

#define PCI7432_DI      0x00
#define PCI7432_DO	    0x00

#define PCI_DEVICE_ID_PCI7432 0x7432

static DEFINE_PCI_DEVICE_TABLE(adl_pci7432_pci_table) = {
	{
	PCI_VENDOR_ID_ADLINK, PCI_DEVICE_ID_PCI7432, PCI_ANY_ID,
		    PCI_ANY_ID, 0, 0, 0}, {
	0}
};

MODULE_DEVICE_TABLE(pci, adl_pci7432_pci_table);

struct adl_pci7432_private {
	int data;
	struct pci_dev *pci_dev;
};

#define devpriv ((struct adl_pci7432_private *)dev->private)

static int adl_pci7432_attach(struct comedi_device *dev,
			      struct comedi_devconfig *it);
static int adl_pci7432_detach(struct comedi_device *dev);
static struct comedi_driver driver_adl_pci7432 = {
	.driver_name = "adl_pci7432",
	.module = THIS_MODULE,
	.attach = adl_pci7432_attach,
	.detach = adl_pci7432_detach,
};



static int adl_pci7432_di_insn_bits(struct comedi_device *dev,
				    struct comedi_subdevice *s,
				    struct comedi_insn *insn,
				    unsigned int *data);

static int adl_pci7432_do_insn_bits(struct comedi_device *dev,
				    struct comedi_subdevice *s,
				    struct comedi_insn *insn,
				    unsigned int *data);



static int adl_pci7432_attach(struct comedi_device *dev,
			      struct comedi_devconfig *it)
{
	struct pci_dev *pcidev;
	struct comedi_subdevice *s;
	int bus, slot;

	printk("comedi: attempt to attach...\n");
	printk("comedi%d: adl_pci7432\n", dev->minor);

	dev->board_name = "pci7432";
	bus = it->options[0];
	slot = it->options[1];

	if (alloc_private(dev, sizeof(struct adl_pci7432_private)) < 0)
		return -ENOMEM;

	if (alloc_subdevices(dev, 2) < 0)
		return -ENOMEM;

	for (pcidev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, NULL);
	     pcidev != NULL;
	     pcidev = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pcidev)) {

		if (pcidev->vendor == PCI_VENDOR_ID_ADLINK &&
		    pcidev->device == PCI_DEVICE_ID_PCI7432) {
			if (bus || slot) {
				
				if (pcidev->bus->number != bus
				    || PCI_SLOT(pcidev->devfn) != slot) {
					continue;
				}
			}
			devpriv->pci_dev = pcidev;
			if (comedi_pci_enable(pcidev, "adl_pci7432") < 0) {
				printk
				    ("comedi%d: Failed to enable PCI device and request regions\n",
				     dev->minor);
				return -EIO;
			}
			dev->iobase = pci_resource_start(pcidev, 2);
			printk("comedi: base addr %4lx\n", dev->iobase);

			s = dev->subdevices + 0;
			s->type = COMEDI_SUBD_DI;
			s->subdev_flags =
			    SDF_READABLE | SDF_GROUND | SDF_COMMON;
			s->n_chan = 32;
			s->maxdata = 1;
			s->len_chanlist = 32;
			s->io_bits = 0x00000000;
			s->range_table = &range_digital;
			s->insn_bits = adl_pci7432_di_insn_bits;

			s = dev->subdevices + 1;
			s->type = COMEDI_SUBD_DO;
			s->subdev_flags =
			    SDF_WRITABLE | SDF_GROUND | SDF_COMMON;
			s->n_chan = 32;
			s->maxdata = 1;
			s->len_chanlist = 32;
			s->io_bits = 0xffffffff;
			s->range_table = &range_digital;
			s->insn_bits = adl_pci7432_do_insn_bits;

			printk("comedi: attached\n");

			return 1;
		}
	}

	printk("comedi%d: no supported board found! (req. bus/slot : %d/%d)\n",
	       dev->minor, bus, slot);
	return -EIO;
}

static int adl_pci7432_detach(struct comedi_device *dev)
{
	printk("comedi%d: pci7432: remove\n", dev->minor);

	if (devpriv && devpriv->pci_dev) {
		if (dev->iobase) {
			comedi_pci_disable(devpriv->pci_dev);
		}
		pci_dev_put(devpriv->pci_dev);
	}

	return 0;
}

static int adl_pci7432_do_insn_bits(struct comedi_device *dev,
				    struct comedi_subdevice *s,
				    struct comedi_insn *insn,
				    unsigned int *data)
{
	printk("comedi: pci7432_do_insn_bits called\n");
	printk("comedi: data0: %8x data1: %8x\n", data[0], data[1]);

	if (insn->n != 2)
		return -EINVAL;

	if (data[0]) {
		s->state &= ~data[0];
		s->state |= (data[0] & data[1]);

		printk("comedi: out: %8x on iobase %4lx\n", s->state,
		       dev->iobase + PCI7432_DO);
		outl(s->state & 0xffffffff, dev->iobase + PCI7432_DO);
	}
	return 2;
}

static int adl_pci7432_di_insn_bits(struct comedi_device *dev,
				    struct comedi_subdevice *s,
				    struct comedi_insn *insn,
				    unsigned int *data)
{
	printk("comedi: pci7432_di_insn_bits called\n");
	printk("comedi: data0: %8x data1: %8x\n", data[0], data[1]);

	if (insn->n != 2)
		return -EINVAL;

	data[1] = inl(dev->iobase + PCI7432_DI) & 0xffffffff;
	printk("comedi: data1 %8x\n", data[1]);

	return 2;
}

COMEDI_PCI_INITCLEANUP(driver_adl_pci7432, adl_pci7432_pci_table);
