


#include <linux/interrupt.h>
#include "../comedidev.h"

#include <linux/delay.h>
#include <linux/list.h>
#include <linux/spinlock.h>

#include "comedi_pci.h"
#include "me4000.h"
#if 0

#include "me4000_fw.h"
#endif



static DEFINE_PCI_DEVICE_TABLE(me4000_pci_table) = {
	{
	PCI_VENDOR_ID_MEILHAUS, 0x4650, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4660, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4661, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4662, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4663, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4670, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4671, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4672, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4673, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4680, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4681, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4682, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	PCI_VENDOR_ID_MEILHAUS, 0x4683, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0}, {
	0}
};

MODULE_DEVICE_TABLE(pci, me4000_pci_table);

static const struct me4000_board me4000_boards[] = {
	{"ME-4650", 0x4650, {0, 0}, {16, 0, 0, 0}, {4}, {0}},

	{"ME-4660", 0x4660, {0, 0}, {32, 0, 16, 0}, {4}, {3}},
	{"ME-4660i", 0x4661, {0, 0}, {32, 0, 16, 0}, {4}, {3}},
	{"ME-4660s", 0x4662, {0, 0}, {32, 8, 16, 0}, {4}, {3}},
	{"ME-4660is", 0x4663, {0, 0}, {32, 8, 16, 0}, {4}, {3}},

	{"ME-4670", 0x4670, {4, 0}, {32, 0, 16, 1}, {4}, {3}},
	{"ME-4670i", 0x4671, {4, 0}, {32, 0, 16, 1}, {4}, {3}},
	{"ME-4670s", 0x4672, {4, 0}, {32, 8, 16, 1}, {4}, {3}},
	{"ME-4670is", 0x4673, {4, 0}, {32, 8, 16, 1}, {4}, {3}},

	{"ME-4680", 0x4680, {4, 4}, {32, 0, 16, 1}, {4}, {3}},
	{"ME-4680i", 0x4681, {4, 4}, {32, 0, 16, 1}, {4}, {3}},
	{"ME-4680s", 0x4682, {4, 4}, {32, 8, 16, 1}, {4}, {3}},
	{"ME-4680is", 0x4683, {4, 4}, {32, 8, 16, 1}, {4}, {3}},

	{0},
};

#define ME4000_BOARD_VERSIONS (ARRAY_SIZE(me4000_boards) - 1)


static int me4000_attach(struct comedi_device *dev,
			 struct comedi_devconfig *it);
static int me4000_detach(struct comedi_device *dev);
static struct comedi_driver driver_me4000 = {
driver_name:"me4000",
module:THIS_MODULE,
attach:me4000_attach,
detach:me4000_detach,
};


static int me4000_probe(struct comedi_device *dev, struct comedi_devconfig *it);
static int get_registers(struct comedi_device *dev, struct pci_dev *pci_dev_p);
static int init_board_info(struct comedi_device *dev,
			   struct pci_dev *pci_dev_p);
static int init_ao_context(struct comedi_device *dev);
static int init_ai_context(struct comedi_device *dev);
static int init_dio_context(struct comedi_device *dev);
static int init_cnt_context(struct comedi_device *dev);
static int xilinx_download(struct comedi_device *dev);
static int reset_board(struct comedi_device *dev);

static int me4000_dio_insn_bits(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data);

static int me4000_dio_insn_config(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data);

static int cnt_reset(struct comedi_device *dev, unsigned int channel);

static int cnt_config(struct comedi_device *dev,
		      unsigned int channel, unsigned int mode);

static int me4000_cnt_insn_config(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data);

static int me4000_cnt_insn_write(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 struct comedi_insn *insn, unsigned int *data);

static int me4000_cnt_insn_read(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data);

static int me4000_ai_insn_read(struct comedi_device *dev,
			       struct comedi_subdevice *subdevice,
			       struct comedi_insn *insn, unsigned int *data);

static int me4000_ai_cancel(struct comedi_device *dev,
			    struct comedi_subdevice *s);

static int ai_check_chanlist(struct comedi_device *dev,
			     struct comedi_subdevice *s,
			     struct comedi_cmd *cmd);

static int ai_round_cmd_args(struct comedi_device *dev,
			     struct comedi_subdevice *s,
			     struct comedi_cmd *cmd,
			     unsigned int *init_ticks,
			     unsigned int *scan_ticks,
			     unsigned int *chan_ticks);

static int ai_prepare(struct comedi_device *dev,
		      struct comedi_subdevice *s,
		      struct comedi_cmd *cmd,
		      unsigned int init_ticks,
		      unsigned int scan_ticks, unsigned int chan_ticks);

static int ai_write_chanlist(struct comedi_device *dev,
			     struct comedi_subdevice *s,
			     struct comedi_cmd *cmd);

static irqreturn_t me4000_ai_isr(int irq, void *dev_id);

static int me4000_ai_do_cmd_test(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 struct comedi_cmd *cmd);

static int me4000_ai_do_cmd(struct comedi_device *dev,
			    struct comedi_subdevice *s);

static int me4000_ao_insn_write(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data);

static int me4000_ao_insn_read(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data);



static inline void me4000_outb(struct comedi_device *dev, unsigned char value,
			       unsigned long port)
{
	PORT_PDEBUG("--> 0x%02X port 0x%04lX\n", value, port);
	outb(value, port);
}

static inline void me4000_outl(struct comedi_device *dev, unsigned long value,
			       unsigned long port)
{
	PORT_PDEBUG("--> 0x%08lX port 0x%04lX\n", value, port);
	outl(value, port);
}

static inline unsigned long me4000_inl(struct comedi_device *dev,
				       unsigned long port)
{
	unsigned long value;
	value = inl(port);
	PORT_PDEBUG("<-- 0x%08lX port 0x%04lX\n", value, port);
	return value;
}

static inline unsigned char me4000_inb(struct comedi_device *dev,
				       unsigned long port)
{
	unsigned char value;
	value = inb(port);
	PORT_PDEBUG("<-- 0x%08X port 0x%04lX\n", value, port);
	return value;
}

static const struct comedi_lrange me4000_ai_range = {
	4,
	{
	 UNI_RANGE(2.5),
	 UNI_RANGE(10),
	 BIP_RANGE(2.5),
	 BIP_RANGE(10),
	 }
};

static const struct comedi_lrange me4000_ao_range = {
	1,
	{
	 BIP_RANGE(10),
	 }
};

static int me4000_attach(struct comedi_device *dev, struct comedi_devconfig *it)
{
	struct comedi_subdevice *s;
	int result;

	CALL_PDEBUG("In me4000_attach()\n");

	result = me4000_probe(dev, it);
	if (result)
		return result;

	
	if (alloc_subdevices(dev, 4) < 0)
		return -ENOMEM;

    

	s = dev->subdevices + 0;

	if (thisboard->ai.count) {
		s->type = COMEDI_SUBD_AI;
		s->subdev_flags =
		    SDF_READABLE | SDF_COMMON | SDF_GROUND | SDF_DIFF;
		s->n_chan = thisboard->ai.count;
		s->maxdata = 0xFFFF;	
		s->len_chanlist = ME4000_AI_CHANNEL_LIST_COUNT;
		s->range_table = &me4000_ai_range;
		s->insn_read = me4000_ai_insn_read;

		if (info->irq > 0) {
			if (request_irq(info->irq, me4000_ai_isr,
					IRQF_SHARED, "ME-4000", dev)) {
				printk
				    ("comedi%d: me4000: me4000_attach(): Unable to allocate irq\n",
				     dev->minor);
			} else {
				dev->read_subdev = s;
				s->subdev_flags |= SDF_CMD_READ;
				s->cancel = me4000_ai_cancel;
				s->do_cmdtest = me4000_ai_do_cmd_test;
				s->do_cmd = me4000_ai_do_cmd;
			}
		} else {
			printk(KERN_WARNING
			       "comedi%d: me4000: me4000_attach(): No interrupt available\n",
			       dev->minor);
		}
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

    

	s = dev->subdevices + 1;

	if (thisboard->ao.count) {
		s->type = COMEDI_SUBD_AO;
		s->subdev_flags = SDF_WRITEABLE | SDF_COMMON | SDF_GROUND;
		s->n_chan = thisboard->ao.count;
		s->maxdata = 0xFFFF;	
		s->range_table = &me4000_ao_range;
		s->insn_write = me4000_ao_insn_write;
		s->insn_read = me4000_ao_insn_read;
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

    

	s = dev->subdevices + 2;

	if (thisboard->dio.count) {
		s->type = COMEDI_SUBD_DIO;
		s->subdev_flags = SDF_READABLE | SDF_WRITABLE;
		s->n_chan = thisboard->dio.count * 8;
		s->maxdata = 1;
		s->range_table = &range_digital;
		s->insn_bits = me4000_dio_insn_bits;
		s->insn_config = me4000_dio_insn_config;
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	
	if (!me4000_inl(dev, info->dio_context.dir_reg)) {
		s->io_bits |= 0xFF;
		me4000_outl(dev, ME4000_DIO_CTRL_BIT_MODE_0,
			    info->dio_context.dir_reg);
	}

    

	s = dev->subdevices + 3;

	if (thisboard->cnt.count) {
		s->type = COMEDI_SUBD_COUNTER;
		s->subdev_flags = SDF_READABLE | SDF_WRITABLE;
		s->n_chan = thisboard->cnt.count;
		s->maxdata = 0xFFFF;	
		s->insn_read = me4000_cnt_insn_read;
		s->insn_write = me4000_cnt_insn_write;
		s->insn_config = me4000_cnt_insn_config;
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	return 0;
}

static int me4000_probe(struct comedi_device *dev, struct comedi_devconfig *it)
{
	struct pci_dev *pci_device;
	int result, i;
	struct me4000_board *board;

	CALL_PDEBUG("In me4000_probe()\n");

	
	if (alloc_private(dev, sizeof(struct me4000_info)) < 0)
		return -ENOMEM;

	
	for (pci_device = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, NULL);
	     pci_device != NULL;
	     pci_device = pci_get_device(PCI_ANY_ID, PCI_ANY_ID, pci_device)) {
		if (pci_device->vendor == PCI_VENDOR_ID_MEILHAUS) {
			for (i = 0; i < ME4000_BOARD_VERSIONS; i++) {
				if (me4000_boards[i].device_id ==
				    pci_device->device) {
					
					if ((it->options[0] != 0)
					    || (it->options[1] != 0)) {
						
						if (pci_device->bus->number !=
						    it->options[0]
						    ||
						    PCI_SLOT(pci_device->devfn)
						    != it->options[1]) {
							continue;
						}
					}
					dev->board_ptr = me4000_boards + i;
					board =
					    (struct me4000_board *)
					    dev->board_ptr;
					info->pci_dev_p = pci_device;
					goto found;
				}
			}
		}
	}

	printk(KERN_ERR
	       "comedi%d: me4000: me4000_probe(): No supported board found (req. bus/slot : %d/%d)\n",
	       dev->minor, it->options[0], it->options[1]);
	return -ENODEV;

found:

	printk(KERN_INFO
	       "comedi%d: me4000: me4000_probe(): Found %s at PCI bus %d, slot %d\n",
	       dev->minor, me4000_boards[i].name, pci_device->bus->number,
	       PCI_SLOT(pci_device->devfn));

	
	dev->board_name = board->name;

	
	result = comedi_pci_enable(pci_device, dev->board_name);
	if (result) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_probe(): Cannot enable PCI device and request I/O regions\n",
		       dev->minor);
		return result;
	}

	
	result = get_registers(dev, pci_device);
	if (result) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_probe(): Cannot get registers\n",
		       dev->minor);
		return result;
	}
	
	result = init_board_info(dev, pci_device);
	if (result) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_probe(): Cannot init baord info\n",
		       dev->minor);
		return result;
	}

	
	result = init_ao_context(dev);
	if (result) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_probe(): Cannot init ao context\n",
		       dev->minor);
		return result;
	}

	
	result = init_ai_context(dev);
	if (result) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_probe(): Cannot init ai context\n",
		       dev->minor);
		return result;
	}

	
	result = init_dio_context(dev);
	if (result) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_probe(): Cannot init dio context\n",
		       dev->minor);
		return result;
	}

	
	result = init_cnt_context(dev);
	if (result) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_probe(): Cannot init cnt context\n",
		       dev->minor);
		return result;
	}

	
	result = xilinx_download(dev);
	if (result) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_probe(): Can't download firmware\n",
		       dev->minor);
		return result;
	}

	
	result = reset_board(dev);
	if (result) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_probe(): Can't reset board\n",
		       dev->minor);
		return result;
	}

	return 0;
}

static int get_registers(struct comedi_device *dev, struct pci_dev *pci_dev_p)
{

	CALL_PDEBUG("In get_registers()\n");

    

	info->plx_regbase = pci_resource_start(pci_dev_p, 1);
	if (info->plx_regbase == 0) {
		printk(KERN_ERR
		       "comedi%d: me4000: get_registers(): PCI base address 1 is not available\n",
		       dev->minor);
		return -ENODEV;
	}
	info->plx_regbase_size = pci_resource_len(pci_dev_p, 1);

    

	info->me4000_regbase = pci_resource_start(pci_dev_p, 2);
	if (info->me4000_regbase == 0) {
		printk(KERN_ERR
		       "comedi%d: me4000: get_registers(): PCI base address 2 is not available\n",
		       dev->minor);
		return -ENODEV;
	}
	info->me4000_regbase_size = pci_resource_len(pci_dev_p, 2);

    

	info->timer_regbase = pci_resource_start(pci_dev_p, 3);
	if (info->timer_regbase == 0) {
		printk(KERN_ERR
		       "comedi%d: me4000: get_registers(): PCI base address 3 is not available\n",
		       dev->minor);
		return -ENODEV;
	}
	info->timer_regbase_size = pci_resource_len(pci_dev_p, 3);

    

	info->program_regbase = pci_resource_start(pci_dev_p, 5);
	if (info->program_regbase == 0) {
		printk(KERN_ERR
		       "comedi%d: me4000: get_registers(): PCI base address 5 is not available\n",
		       dev->minor);
		return -ENODEV;
	}
	info->program_regbase_size = pci_resource_len(pci_dev_p, 5);

	return 0;
}

static int init_board_info(struct comedi_device *dev, struct pci_dev *pci_dev_p)
{
	int result;

	CALL_PDEBUG("In init_board_info()\n");

	
	
	

	
	result = pci_read_config_dword(pci_dev_p, 0x2C, &info->serial_no);
	if (result != PCIBIOS_SUCCESSFUL)
		return result;

	
	result = pci_read_config_byte(pci_dev_p, 0x08, &info->hw_revision);
	if (result != PCIBIOS_SUCCESSFUL)
		return result;

	
	info->vendor_id = pci_dev_p->vendor;

	
	info->device_id = pci_dev_p->device;

	
	info->irq = pci_dev_p->irq;

	return 0;
}

static int init_ao_context(struct comedi_device *dev)
{
	int i;

	CALL_PDEBUG("In init_ao_context()\n");

	for (i = 0; i < thisboard->ao.count; i++) {
		
		info->ao_context[i].irq = info->irq;

		switch (i) {
		case 0:
			info->ao_context[i].ctrl_reg =
			    info->me4000_regbase + ME4000_AO_00_CTRL_REG;
			info->ao_context[i].status_reg =
			    info->me4000_regbase + ME4000_AO_00_STATUS_REG;
			info->ao_context[i].fifo_reg =
			    info->me4000_regbase + ME4000_AO_00_FIFO_REG;
			info->ao_context[i].single_reg =
			    info->me4000_regbase + ME4000_AO_00_SINGLE_REG;
			info->ao_context[i].timer_reg =
			    info->me4000_regbase + ME4000_AO_00_TIMER_REG;
			info->ao_context[i].irq_status_reg =
			    info->me4000_regbase + ME4000_IRQ_STATUS_REG;
			info->ao_context[i].preload_reg =
			    info->me4000_regbase + ME4000_AO_LOADSETREG_XX;
			break;
		case 1:
			info->ao_context[i].ctrl_reg =
			    info->me4000_regbase + ME4000_AO_01_CTRL_REG;
			info->ao_context[i].status_reg =
			    info->me4000_regbase + ME4000_AO_01_STATUS_REG;
			info->ao_context[i].fifo_reg =
			    info->me4000_regbase + ME4000_AO_01_FIFO_REG;
			info->ao_context[i].single_reg =
			    info->me4000_regbase + ME4000_AO_01_SINGLE_REG;
			info->ao_context[i].timer_reg =
			    info->me4000_regbase + ME4000_AO_01_TIMER_REG;
			info->ao_context[i].irq_status_reg =
			    info->me4000_regbase + ME4000_IRQ_STATUS_REG;
			info->ao_context[i].preload_reg =
			    info->me4000_regbase + ME4000_AO_LOADSETREG_XX;
			break;
		case 2:
			info->ao_context[i].ctrl_reg =
			    info->me4000_regbase + ME4000_AO_02_CTRL_REG;
			info->ao_context[i].status_reg =
			    info->me4000_regbase + ME4000_AO_02_STATUS_REG;
			info->ao_context[i].fifo_reg =
			    info->me4000_regbase + ME4000_AO_02_FIFO_REG;
			info->ao_context[i].single_reg =
			    info->me4000_regbase + ME4000_AO_02_SINGLE_REG;
			info->ao_context[i].timer_reg =
			    info->me4000_regbase + ME4000_AO_02_TIMER_REG;
			info->ao_context[i].irq_status_reg =
			    info->me4000_regbase + ME4000_IRQ_STATUS_REG;
			info->ao_context[i].preload_reg =
			    info->me4000_regbase + ME4000_AO_LOADSETREG_XX;
			break;
		case 3:
			info->ao_context[i].ctrl_reg =
			    info->me4000_regbase + ME4000_AO_03_CTRL_REG;
			info->ao_context[i].status_reg =
			    info->me4000_regbase + ME4000_AO_03_STATUS_REG;
			info->ao_context[i].fifo_reg =
			    info->me4000_regbase + ME4000_AO_03_FIFO_REG;
			info->ao_context[i].single_reg =
			    info->me4000_regbase + ME4000_AO_03_SINGLE_REG;
			info->ao_context[i].timer_reg =
			    info->me4000_regbase + ME4000_AO_03_TIMER_REG;
			info->ao_context[i].irq_status_reg =
			    info->me4000_regbase + ME4000_IRQ_STATUS_REG;
			info->ao_context[i].preload_reg =
			    info->me4000_regbase + ME4000_AO_LOADSETREG_XX;
			break;
		default:
			break;
		}
	}

	return 0;
}

static int init_ai_context(struct comedi_device *dev)
{

	CALL_PDEBUG("In init_ai_context()\n");

	info->ai_context.irq = info->irq;

	info->ai_context.ctrl_reg = info->me4000_regbase + ME4000_AI_CTRL_REG;
	info->ai_context.status_reg =
	    info->me4000_regbase + ME4000_AI_STATUS_REG;
	info->ai_context.channel_list_reg =
	    info->me4000_regbase + ME4000_AI_CHANNEL_LIST_REG;
	info->ai_context.data_reg = info->me4000_regbase + ME4000_AI_DATA_REG;
	info->ai_context.chan_timer_reg =
	    info->me4000_regbase + ME4000_AI_CHAN_TIMER_REG;
	info->ai_context.chan_pre_timer_reg =
	    info->me4000_regbase + ME4000_AI_CHAN_PRE_TIMER_REG;
	info->ai_context.scan_timer_low_reg =
	    info->me4000_regbase + ME4000_AI_SCAN_TIMER_LOW_REG;
	info->ai_context.scan_timer_high_reg =
	    info->me4000_regbase + ME4000_AI_SCAN_TIMER_HIGH_REG;
	info->ai_context.scan_pre_timer_low_reg =
	    info->me4000_regbase + ME4000_AI_SCAN_PRE_TIMER_LOW_REG;
	info->ai_context.scan_pre_timer_high_reg =
	    info->me4000_regbase + ME4000_AI_SCAN_PRE_TIMER_HIGH_REG;
	info->ai_context.start_reg = info->me4000_regbase + ME4000_AI_START_REG;
	info->ai_context.irq_status_reg =
	    info->me4000_regbase + ME4000_IRQ_STATUS_REG;
	info->ai_context.sample_counter_reg =
	    info->me4000_regbase + ME4000_AI_SAMPLE_COUNTER_REG;

	return 0;
}

static int init_dio_context(struct comedi_device *dev)
{

	CALL_PDEBUG("In init_dio_context()\n");

	info->dio_context.dir_reg = info->me4000_regbase + ME4000_DIO_DIR_REG;
	info->dio_context.ctrl_reg = info->me4000_regbase + ME4000_DIO_CTRL_REG;
	info->dio_context.port_0_reg =
	    info->me4000_regbase + ME4000_DIO_PORT_0_REG;
	info->dio_context.port_1_reg =
	    info->me4000_regbase + ME4000_DIO_PORT_1_REG;
	info->dio_context.port_2_reg =
	    info->me4000_regbase + ME4000_DIO_PORT_2_REG;
	info->dio_context.port_3_reg =
	    info->me4000_regbase + ME4000_DIO_PORT_3_REG;

	return 0;
}

static int init_cnt_context(struct comedi_device *dev)
{

	CALL_PDEBUG("In init_cnt_context()\n");

	info->cnt_context.ctrl_reg = info->timer_regbase + ME4000_CNT_CTRL_REG;
	info->cnt_context.counter_0_reg =
	    info->timer_regbase + ME4000_CNT_COUNTER_0_REG;
	info->cnt_context.counter_1_reg =
	    info->timer_regbase + ME4000_CNT_COUNTER_1_REG;
	info->cnt_context.counter_2_reg =
	    info->timer_regbase + ME4000_CNT_COUNTER_2_REG;

	return 0;
}

#define FIRMWARE_NOT_AVAILABLE 1
#if FIRMWARE_NOT_AVAILABLE
extern unsigned char *xilinx_firm;
#endif

static int xilinx_download(struct comedi_device *dev)
{
	u32 value = 0;
	wait_queue_head_t queue;
	int idx = 0;
	int size = 0;

	CALL_PDEBUG("In xilinx_download()\n");

	init_waitqueue_head(&queue);

	
	outl(0x10, info->plx_regbase + PLX_INTCSR);

	
	value = inl(info->plx_regbase + PLX_ICR);
	value |= 0x100;
	outl(value, info->plx_regbase + PLX_ICR);

	
	inb(info->program_regbase + 0xC8);

	
	udelay(20);
	if (!(inl(info->plx_regbase + PLX_INTCSR) & 0x20)) {
		printk(KERN_ERR
		       "comedi%d: me4000: xilinx_download(): Can't init Xilinx\n",
		       dev->minor);
		return -EIO;
	}

	
	value = inl(info->plx_regbase + PLX_ICR);
	value &= ~0x100;
	outl(value, info->plx_regbase + PLX_ICR);
	if (FIRMWARE_NOT_AVAILABLE) {
		comedi_error(dev,
			     "xilinx firmware unavailable due to licensing, aborting");
		return -EIO;
	} else {
		
		size = (xilinx_firm[0] << 24) + (xilinx_firm[1] << 16) +
		    (xilinx_firm[2] << 8) + xilinx_firm[3];
		udelay(10);

		for (idx = 0; idx < size; idx++) {
			outb(xilinx_firm[16 + idx], info->program_regbase);
			udelay(10);

			
			if (inl(info->plx_regbase + PLX_ICR) & 0x20) {
				printk(KERN_ERR
				       "comedi%d: me4000: xilinx_download(): Xilinx is still busy (idx = %d)\n",
				       dev->minor, idx);
				return -EIO;
			}
		}
	}

	
	if (inl(info->plx_regbase + PLX_ICR) & 0x4) {
	} else {
		printk(KERN_ERR
		       "comedi%d: me4000: xilinx_download(): DONE flag is not set\n",
		       dev->minor);
		printk(KERN_ERR
		       "comedi%d: me4000: xilinx_download(): Download not succesful\n",
		       dev->minor);
		return -EIO;
	}

	
	value = inl(info->plx_regbase + PLX_ICR);
	value |= 0x100;
	outl(value, info->plx_regbase + PLX_ICR);

	return 0;
}

static int reset_board(struct comedi_device *dev)
{
	unsigned long icr;

	CALL_PDEBUG("In reset_board()\n");

	
	icr = me4000_inl(dev, info->plx_regbase + PLX_ICR);
	icr |= 0x40000000;
	me4000_outl(dev, icr, info->plx_regbase + PLX_ICR);
	icr &= ~0x40000000;
	me4000_outl(dev, icr, info->plx_regbase + PLX_ICR);

	
	me4000_outl(dev, 0x8000,
		    info->me4000_regbase + ME4000_AO_00_SINGLE_REG);
	me4000_outl(dev, 0x8000,
		    info->me4000_regbase + ME4000_AO_01_SINGLE_REG);
	me4000_outl(dev, 0x8000,
		    info->me4000_regbase + ME4000_AO_02_SINGLE_REG);
	me4000_outl(dev, 0x8000,
		    info->me4000_regbase + ME4000_AO_03_SINGLE_REG);

	
	me4000_outl(dev,
		    ME4000_AI_CTRL_BIT_IMMEDIATE_STOP | ME4000_AI_CTRL_BIT_STOP,
		    info->me4000_regbase + ME4000_AI_CTRL_REG);

	
	me4000_outl(dev,
		    ME4000_AO_CTRL_BIT_IMMEDIATE_STOP | ME4000_AO_CTRL_BIT_STOP,
		    info->me4000_regbase + ME4000_AO_00_CTRL_REG);
	me4000_outl(dev,
		    ME4000_AO_CTRL_BIT_IMMEDIATE_STOP | ME4000_AO_CTRL_BIT_STOP,
		    info->me4000_regbase + ME4000_AO_01_CTRL_REG);
	me4000_outl(dev,
		    ME4000_AO_CTRL_BIT_IMMEDIATE_STOP | ME4000_AO_CTRL_BIT_STOP,
		    info->me4000_regbase + ME4000_AO_02_CTRL_REG);
	me4000_outl(dev,
		    ME4000_AO_CTRL_BIT_IMMEDIATE_STOP | ME4000_AO_CTRL_BIT_STOP,
		    info->me4000_regbase + ME4000_AO_03_CTRL_REG);

	
	me4000_outl(dev, 0x43, info->plx_regbase + PLX_INTCSR);

	
	me4000_outl(dev, ME4000_AO_DEMUX_ADJUST_VALUE,
		    info->me4000_regbase + ME4000_AO_DEMUX_ADJUST_REG);

	
	if (!(me4000_inl(dev, info->me4000_regbase + ME4000_DIO_DIR_REG) & 0x1)) {
		me4000_outl(dev, 0x1,
			    info->me4000_regbase + ME4000_DIO_CTRL_REG);
	}

	return 0;
}

static int me4000_detach(struct comedi_device *dev)
{
	CALL_PDEBUG("In me4000_detach()\n");

	if (info) {
		if (info->pci_dev_p) {
			reset_board(dev);
			if (info->plx_regbase)
				comedi_pci_disable(info->pci_dev_p);
			pci_dev_put(info->pci_dev_p);
		}
	}

	return 0;
}



static int me4000_ai_insn_read(struct comedi_device *dev,
			       struct comedi_subdevice *subdevice,
			       struct comedi_insn *insn, unsigned int *data)
{

	int chan = CR_CHAN(insn->chanspec);
	int rang = CR_RANGE(insn->chanspec);
	int aref = CR_AREF(insn->chanspec);

	unsigned long entry = 0;
	unsigned long tmp;
	long lval;

	CALL_PDEBUG("In me4000_ai_insn_read()\n");

	if (insn->n == 0) {
		return 0;
	} else if (insn->n > 1) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_insn_read(): Invalid instruction length %d\n",
		       dev->minor, insn->n);
		return -EINVAL;
	}

	switch (rang) {
	case 0:
		entry |= ME4000_AI_LIST_RANGE_UNIPOLAR_2_5;
		break;
	case 1:
		entry |= ME4000_AI_LIST_RANGE_UNIPOLAR_10;
		break;
	case 2:
		entry |= ME4000_AI_LIST_RANGE_BIPOLAR_2_5;
		break;
	case 3:
		entry |= ME4000_AI_LIST_RANGE_BIPOLAR_10;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_insn_read(): Invalid range specified\n",
		       dev->minor);
		return -EINVAL;
	}

	switch (aref) {
	case AREF_GROUND:
	case AREF_COMMON:
		if (chan >= thisboard->ai.count) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_insn_read(): Analog input is not available\n",
			       dev->minor);
			return -EINVAL;
		}
		entry |= ME4000_AI_LIST_INPUT_SINGLE_ENDED | chan;
		break;

	case AREF_DIFF:
		if (rang == 0 || rang == 1) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_insn_read(): Range must be bipolar when aref = diff\n",
			       dev->minor);
			return -EINVAL;
		}

		if (chan >= thisboard->ai.diff_count) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_insn_read(): Analog input is not available\n",
			       dev->minor);
			return -EINVAL;
		}
		entry |= ME4000_AI_LIST_INPUT_DIFFERENTIAL | chan;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_insn_read(): Invalid aref specified\n",
		       dev->minor);
		return -EINVAL;
	}

	entry |= ME4000_AI_LIST_LAST_ENTRY;

	
	tmp = me4000_inl(dev, info->ai_context.ctrl_reg);
	tmp &= ~(ME4000_AI_CTRL_BIT_CHANNEL_FIFO |
		 ME4000_AI_CTRL_BIT_DATA_FIFO |
		 ME4000_AI_CTRL_BIT_STOP | ME4000_AI_CTRL_BIT_IMMEDIATE_STOP);
	me4000_outl(dev, tmp, info->ai_context.ctrl_reg);

	
	tmp &= ~(ME4000_AI_CTRL_BIT_MODE_0 | ME4000_AI_CTRL_BIT_MODE_1 |
		 ME4000_AI_CTRL_BIT_MODE_2);
	me4000_outl(dev, tmp, info->ai_context.ctrl_reg);

	
	tmp |= ME4000_AI_CTRL_BIT_CHANNEL_FIFO | ME4000_AI_CTRL_BIT_DATA_FIFO;
	me4000_outl(dev, tmp, info->ai_context.ctrl_reg);

	
	me4000_outl(dev, entry, info->ai_context.channel_list_reg);

	
	me4000_outl(dev, ME4000_AI_MIN_TICKS, info->ai_context.chan_timer_reg);
	me4000_outl(dev, ME4000_AI_MIN_TICKS,
		    info->ai_context.chan_pre_timer_reg);

	
	me4000_inl(dev, info->ai_context.start_reg);

	
	udelay(10);
	if (!
	    (me4000_inl(dev, info->ai_context.status_reg) &
	     ME4000_AI_STATUS_BIT_EF_DATA)) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_insn_read(): Value not available after wait\n",
		       dev->minor);
		return -EIO;
	}

	
	lval = me4000_inl(dev, info->ai_context.data_reg) & 0xFFFF;
	data[0] = lval ^ 0x8000;

	return 1;
}

static int me4000_ai_cancel(struct comedi_device *dev,
			    struct comedi_subdevice *s)
{
	unsigned long tmp;

	CALL_PDEBUG("In me4000_ai_cancel()\n");

	
	tmp = me4000_inl(dev, info->ai_context.ctrl_reg);
	tmp &= ~(ME4000_AI_CTRL_BIT_STOP | ME4000_AI_CTRL_BIT_IMMEDIATE_STOP);
	me4000_outl(dev, tmp, info->ai_context.ctrl_reg);

	
	me4000_outl(dev, 0x0, info->ai_context.ctrl_reg);

	return 0;
}

static int ai_check_chanlist(struct comedi_device *dev,
			     struct comedi_subdevice *s, struct comedi_cmd *cmd)
{
	int aref;
	int i;

	CALL_PDEBUG("In ai_check_chanlist()\n");

	
	if (!cmd->chanlist_len) {
		printk(KERN_ERR
		       "comedi%d: me4000: ai_check_chanlist(): No channel list available\n",
		       dev->minor);
		return -EINVAL;
	}

	
	if (cmd->chanlist_len > ME4000_AI_CHANNEL_LIST_COUNT) {
		printk(KERN_ERR
		       "comedi%d: me4000: ai_check_chanlist(): Channel list is to large\n",
		       dev->minor);
		return -EINVAL;
	}

	
	if (!cmd->chanlist) {
		printk(KERN_ERR
		       "comedi%d: me4000: ai_check_chanlist(): NULL pointer to channel list\n",
		       dev->minor);
		return -EFAULT;
	}

	
	aref = CR_AREF(cmd->chanlist[0]);
	for (i = 0; i < cmd->chanlist_len; i++) {
		if (CR_AREF(cmd->chanlist[i]) != aref) {
			printk(KERN_ERR
			       "comedi%d: me4000: ai_check_chanlist(): Mode is not equal for all entries\n",
			       dev->minor);
			return -EINVAL;
		}
	}

	
	if (aref == SDF_DIFF) {
		for (i = 0; i < cmd->chanlist_len; i++) {
			if (CR_CHAN(cmd->chanlist[i]) >=
			    thisboard->ai.diff_count) {
				printk(KERN_ERR
				       "comedi%d: me4000: ai_check_chanlist(): Channel number to high\n",
				       dev->minor);
				return -EINVAL;
			}
		}
	} else {
		for (i = 0; i < cmd->chanlist_len; i++) {
			if (CR_CHAN(cmd->chanlist[i]) >= thisboard->ai.count) {
				printk(KERN_ERR
				       "comedi%d: me4000: ai_check_chanlist(): Channel number to high\n",
				       dev->minor);
				return -EINVAL;
			}
		}
	}

	
	if (aref == SDF_DIFF) {
		for (i = 0; i < cmd->chanlist_len; i++) {
			if (CR_RANGE(cmd->chanlist[i]) != 1 &&
			    CR_RANGE(cmd->chanlist[i]) != 2) {
				printk(KERN_ERR
				       "comedi%d: me4000: ai_check_chanlist(): Bipolar is not selected in differential mode\n",
				       dev->minor);
				return -EINVAL;
			}
		}
	}

	return 0;
}

static int ai_round_cmd_args(struct comedi_device *dev,
			     struct comedi_subdevice *s,
			     struct comedi_cmd *cmd,
			     unsigned int *init_ticks,
			     unsigned int *scan_ticks, unsigned int *chan_ticks)
{

	int rest;

	CALL_PDEBUG("In ai_round_cmd_args()\n");

	*init_ticks = 0;
	*scan_ticks = 0;
	*chan_ticks = 0;

	PDEBUG("ai_round_cmd_arg(): start_arg = %d\n", cmd->start_arg);
	PDEBUG("ai_round_cmd_arg(): scan_begin_arg = %d\n",
	       cmd->scan_begin_arg);
	PDEBUG("ai_round_cmd_arg(): convert_arg = %d\n", cmd->convert_arg);

	if (cmd->start_arg) {
		*init_ticks = (cmd->start_arg * 33) / 1000;
		rest = (cmd->start_arg * 33) % 1000;

		if (cmd->flags & TRIG_ROUND_NEAREST) {
			if (rest > 33)
				(*init_ticks)++;
		} else if (cmd->flags & TRIG_ROUND_UP) {
			if (rest)
				(*init_ticks)++;
		}
	}

	if (cmd->scan_begin_arg) {
		*scan_ticks = (cmd->scan_begin_arg * 33) / 1000;
		rest = (cmd->scan_begin_arg * 33) % 1000;

		if (cmd->flags & TRIG_ROUND_NEAREST) {
			if (rest > 33)
				(*scan_ticks)++;
		} else if (cmd->flags & TRIG_ROUND_UP) {
			if (rest)
				(*scan_ticks)++;
		}
	}

	if (cmd->convert_arg) {
		*chan_ticks = (cmd->convert_arg * 33) / 1000;
		rest = (cmd->convert_arg * 33) % 1000;

		if (cmd->flags & TRIG_ROUND_NEAREST) {
			if (rest > 33)
				(*chan_ticks)++;
		} else if (cmd->flags & TRIG_ROUND_UP) {
			if (rest)
				(*chan_ticks)++;
		}
	}

	PDEBUG("ai_round_cmd_args(): init_ticks = %d\n", *init_ticks);
	PDEBUG("ai_round_cmd_args(): scan_ticks = %d\n", *scan_ticks);
	PDEBUG("ai_round_cmd_args(): chan_ticks = %d\n", *chan_ticks);

	return 0;
}

static void ai_write_timer(struct comedi_device *dev,
			   unsigned int init_ticks,
			   unsigned int scan_ticks, unsigned int chan_ticks)
{

	CALL_PDEBUG("In ai_write_timer()\n");

	me4000_outl(dev, init_ticks - 1,
		    info->ai_context.scan_pre_timer_low_reg);
	me4000_outl(dev, 0x0, info->ai_context.scan_pre_timer_high_reg);

	if (scan_ticks) {
		me4000_outl(dev, scan_ticks - 1,
			    info->ai_context.scan_timer_low_reg);
		me4000_outl(dev, 0x0, info->ai_context.scan_timer_high_reg);
	}

	me4000_outl(dev, chan_ticks - 1, info->ai_context.chan_pre_timer_reg);
	me4000_outl(dev, chan_ticks - 1, info->ai_context.chan_timer_reg);
}

static int ai_prepare(struct comedi_device *dev,
		      struct comedi_subdevice *s,
		      struct comedi_cmd *cmd,
		      unsigned int init_ticks,
		      unsigned int scan_ticks, unsigned int chan_ticks)
{

	unsigned long tmp = 0;

	CALL_PDEBUG("In ai_prepare()\n");

	
	ai_write_timer(dev, init_ticks, scan_ticks, chan_ticks);

	
	me4000_outl(dev, tmp, info->ai_context.ctrl_reg);

	
	if ((cmd->start_src == TRIG_EXT &&
	     cmd->scan_begin_src == TRIG_TIMER &&
	     cmd->convert_src == TRIG_TIMER) ||
	    (cmd->start_src == TRIG_EXT &&
	     cmd->scan_begin_src == TRIG_FOLLOW &&
	     cmd->convert_src == TRIG_TIMER)) {
		tmp = ME4000_AI_CTRL_BIT_MODE_1 |
		    ME4000_AI_CTRL_BIT_CHANNEL_FIFO |
		    ME4000_AI_CTRL_BIT_DATA_FIFO;
	} else if (cmd->start_src == TRIG_EXT &&
		   cmd->scan_begin_src == TRIG_EXT &&
		   cmd->convert_src == TRIG_TIMER) {
		tmp = ME4000_AI_CTRL_BIT_MODE_2 |
		    ME4000_AI_CTRL_BIT_CHANNEL_FIFO |
		    ME4000_AI_CTRL_BIT_DATA_FIFO;
	} else if (cmd->start_src == TRIG_EXT &&
		   cmd->scan_begin_src == TRIG_EXT &&
		   cmd->convert_src == TRIG_EXT) {
		tmp = ME4000_AI_CTRL_BIT_MODE_0 |
		    ME4000_AI_CTRL_BIT_MODE_1 |
		    ME4000_AI_CTRL_BIT_CHANNEL_FIFO |
		    ME4000_AI_CTRL_BIT_DATA_FIFO;
	} else {
		tmp = ME4000_AI_CTRL_BIT_MODE_0 |
		    ME4000_AI_CTRL_BIT_CHANNEL_FIFO |
		    ME4000_AI_CTRL_BIT_DATA_FIFO;
	}

	
	if (cmd->stop_src == TRIG_COUNT) {
		me4000_outl(dev, cmd->chanlist_len * cmd->stop_arg,
			    info->ai_context.sample_counter_reg);
		tmp |= ME4000_AI_CTRL_BIT_HF_IRQ | ME4000_AI_CTRL_BIT_SC_IRQ;
	} else if (cmd->stop_src == TRIG_NONE &&
		   cmd->scan_end_src == TRIG_COUNT) {
		me4000_outl(dev, cmd->scan_end_arg,
			    info->ai_context.sample_counter_reg);
		tmp |= ME4000_AI_CTRL_BIT_HF_IRQ | ME4000_AI_CTRL_BIT_SC_IRQ;
	} else {
		tmp |= ME4000_AI_CTRL_BIT_HF_IRQ;
	}

	
	me4000_outl(dev, tmp, info->ai_context.ctrl_reg);

	
	ai_write_chanlist(dev, s, cmd);

	return 0;
}

static int ai_write_chanlist(struct comedi_device *dev,
			     struct comedi_subdevice *s, struct comedi_cmd *cmd)
{
	unsigned int entry;
	unsigned int chan;
	unsigned int rang;
	unsigned int aref;
	int i;

	CALL_PDEBUG("In ai_write_chanlist()\n");

	for (i = 0; i < cmd->chanlist_len; i++) {
		chan = CR_CHAN(cmd->chanlist[i]);
		rang = CR_RANGE(cmd->chanlist[i]);
		aref = CR_AREF(cmd->chanlist[i]);

		entry = chan;

		if (rang == 0) {
			entry |= ME4000_AI_LIST_RANGE_UNIPOLAR_2_5;
		} else if (rang == 1) {
			entry |= ME4000_AI_LIST_RANGE_UNIPOLAR_10;
		} else if (rang == 2) {
			entry |= ME4000_AI_LIST_RANGE_BIPOLAR_2_5;
		} else {
			entry |= ME4000_AI_LIST_RANGE_BIPOLAR_10;
		}

		if (aref == SDF_DIFF) {
			entry |= ME4000_AI_LIST_INPUT_DIFFERENTIAL;
		} else {
			entry |= ME4000_AI_LIST_INPUT_SINGLE_ENDED;
		}

		me4000_outl(dev, entry, info->ai_context.channel_list_reg);
	}

	return 0;
}

static int me4000_ai_do_cmd(struct comedi_device *dev,
			    struct comedi_subdevice *s)
{
	int err;
	unsigned int init_ticks = 0;
	unsigned int scan_ticks = 0;
	unsigned int chan_ticks = 0;
	struct comedi_cmd *cmd = &s->async->cmd;

	CALL_PDEBUG("In me4000_ai_do_cmd()\n");

	
	err = me4000_ai_cancel(dev, s);
	if (err)
		return err;

	
	err = ai_round_cmd_args(dev,
				s, cmd, &init_ticks, &scan_ticks, &chan_ticks);
	if (err)
		return err;

	
	err = ai_prepare(dev, s, cmd, init_ticks, scan_ticks, chan_ticks);
	if (err)
		return err;

	
	me4000_inl(dev, info->ai_context.start_reg);

	return 0;
}


static int me4000_ai_do_cmd_test(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 struct comedi_cmd *cmd)
{

	unsigned int init_ticks;
	unsigned int chan_ticks;
	unsigned int scan_ticks;
	int err = 0;

	CALL_PDEBUG("In me4000_ai_do_cmd_test()\n");

	PDEBUG("me4000_ai_do_cmd_test(): subdev         = %d\n", cmd->subdev);
	PDEBUG("me4000_ai_do_cmd_test(): flags          = %08X\n", cmd->flags);
	PDEBUG("me4000_ai_do_cmd_test(): start_src      = %08X\n",
	       cmd->start_src);
	PDEBUG("me4000_ai_do_cmd_test(): start_arg      = %d\n",
	       cmd->start_arg);
	PDEBUG("me4000_ai_do_cmd_test(): scan_begin_src = %08X\n",
	       cmd->scan_begin_src);
	PDEBUG("me4000_ai_do_cmd_test(): scan_begin_arg = %d\n",
	       cmd->scan_begin_arg);
	PDEBUG("me4000_ai_do_cmd_test(): convert_src    = %08X\n",
	       cmd->convert_src);
	PDEBUG("me4000_ai_do_cmd_test(): convert_arg    = %d\n",
	       cmd->convert_arg);
	PDEBUG("me4000_ai_do_cmd_test(): scan_end_src   = %08X\n",
	       cmd->scan_end_src);
	PDEBUG("me4000_ai_do_cmd_test(): scan_end_arg   = %d\n",
	       cmd->scan_end_arg);
	PDEBUG("me4000_ai_do_cmd_test(): stop_src       = %08X\n",
	       cmd->stop_src);
	PDEBUG("me4000_ai_do_cmd_test(): stop_arg       = %d\n", cmd->stop_arg);
	PDEBUG("me4000_ai_do_cmd_test(): chanlist       = %d\n",
	       (unsigned int)cmd->chanlist);
	PDEBUG("me4000_ai_do_cmd_test(): chanlist_len   = %d\n",
	       cmd->chanlist_len);

	
	cmd->flags &= TRIG_ROUND_NEAREST | TRIG_ROUND_UP | TRIG_ROUND_DOWN;

	
	ai_round_cmd_args(dev, s, cmd, &init_ticks, &scan_ticks, &chan_ticks);

	
	switch (cmd->start_src) {
	case TRIG_NOW:
	case TRIG_EXT:
		break;
	case TRIG_ANY:
		cmd->start_src &= TRIG_NOW | TRIG_EXT;
		err++;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid start source\n",
		       dev->minor);
		cmd->start_src = TRIG_NOW;
		err++;
	}
	switch (cmd->scan_begin_src) {
	case TRIG_FOLLOW:
	case TRIG_TIMER:
	case TRIG_EXT:
		break;
	case TRIG_ANY:
		cmd->scan_begin_src &= TRIG_FOLLOW | TRIG_TIMER | TRIG_EXT;
		err++;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid scan begin source\n",
		       dev->minor);
		cmd->scan_begin_src = TRIG_FOLLOW;
		err++;
	}
	switch (cmd->convert_src) {
	case TRIG_TIMER:
	case TRIG_EXT:
		break;
	case TRIG_ANY:
		cmd->convert_src &= TRIG_TIMER | TRIG_EXT;
		err++;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid convert source\n",
		       dev->minor);
		cmd->convert_src = TRIG_TIMER;
		err++;
	}
	switch (cmd->scan_end_src) {
	case TRIG_NONE:
	case TRIG_COUNT:
		break;
	case TRIG_ANY:
		cmd->scan_end_src &= TRIG_NONE | TRIG_COUNT;
		err++;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid scan end source\n",
		       dev->minor);
		cmd->scan_end_src = TRIG_NONE;
		err++;
	}
	switch (cmd->stop_src) {
	case TRIG_NONE:
	case TRIG_COUNT:
		break;
	case TRIG_ANY:
		cmd->stop_src &= TRIG_NONE | TRIG_COUNT;
		err++;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid stop source\n",
		       dev->minor);
		cmd->stop_src = TRIG_NONE;
		err++;
	}
	if (err)
		return 1;

	
	if (cmd->start_src == TRIG_NOW &&
	    cmd->scan_begin_src == TRIG_TIMER &&
	    cmd->convert_src == TRIG_TIMER) {
	} else if (cmd->start_src == TRIG_NOW &&
		   cmd->scan_begin_src == TRIG_FOLLOW &&
		   cmd->convert_src == TRIG_TIMER) {
	} else if (cmd->start_src == TRIG_EXT &&
		   cmd->scan_begin_src == TRIG_TIMER &&
		   cmd->convert_src == TRIG_TIMER) {
	} else if (cmd->start_src == TRIG_EXT &&
		   cmd->scan_begin_src == TRIG_FOLLOW &&
		   cmd->convert_src == TRIG_TIMER) {
	} else if (cmd->start_src == TRIG_EXT &&
		   cmd->scan_begin_src == TRIG_EXT &&
		   cmd->convert_src == TRIG_TIMER) {
	} else if (cmd->start_src == TRIG_EXT &&
		   cmd->scan_begin_src == TRIG_EXT &&
		   cmd->convert_src == TRIG_EXT) {
	} else {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid start trigger combination\n",
		       dev->minor);
		cmd->start_src = TRIG_NOW;
		cmd->scan_begin_src = TRIG_FOLLOW;
		cmd->convert_src = TRIG_TIMER;
		err++;
	}

	if (cmd->stop_src == TRIG_NONE && cmd->scan_end_src == TRIG_NONE) {
	} else if (cmd->stop_src == TRIG_COUNT &&
		   cmd->scan_end_src == TRIG_NONE) {
	} else if (cmd->stop_src == TRIG_NONE &&
		   cmd->scan_end_src == TRIG_COUNT) {
	} else if (cmd->stop_src == TRIG_COUNT &&
		   cmd->scan_end_src == TRIG_COUNT) {
	} else {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid stop trigger combination\n",
		       dev->minor);
		cmd->stop_src = TRIG_NONE;
		cmd->scan_end_src = TRIG_NONE;
		err++;
	}
	if (err)
		return 2;

	
	if (cmd->chanlist_len < 1) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): No channel list\n",
		       dev->minor);
		cmd->chanlist_len = 1;
		err++;
	}
	if (init_ticks < 66) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): Start arg to low\n",
		       dev->minor);
		cmd->start_arg = 2000;
		err++;
	}
	if (scan_ticks && scan_ticks < 67) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): Scan begin arg to low\n",
		       dev->minor);
		cmd->scan_begin_arg = 2031;
		err++;
	}
	if (chan_ticks < 66) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_do_cmd_test(): Convert arg to low\n",
		       dev->minor);
		cmd->convert_arg = 2000;
		err++;
	}

	if (err)
		return 3;

	
	if (cmd->start_src == TRIG_NOW &&
	    cmd->scan_begin_src == TRIG_TIMER &&
	    cmd->convert_src == TRIG_TIMER) {

		
		if (init_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid start arg\n",
			       dev->minor);
			cmd->start_arg = 2000;	
			err++;
		}
		if (chan_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid convert arg\n",
			       dev->minor);
			cmd->convert_arg = 2000;	
			err++;
		}
		if (scan_ticks <= cmd->chanlist_len * chan_ticks) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid scan end arg\n",
			       dev->minor);
			cmd->scan_end_arg = 2000 * cmd->chanlist_len + 31;	
			err++;
		}
	} else if (cmd->start_src == TRIG_NOW &&
		   cmd->scan_begin_src == TRIG_FOLLOW &&
		   cmd->convert_src == TRIG_TIMER) {

		
		if (init_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid start arg\n",
			       dev->minor);
			cmd->start_arg = 2000;	
			err++;
		}
		if (chan_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid convert arg\n",
			       dev->minor);
			cmd->convert_arg = 2000;	
			err++;
		}
	} else if (cmd->start_src == TRIG_EXT &&
		   cmd->scan_begin_src == TRIG_TIMER &&
		   cmd->convert_src == TRIG_TIMER) {

		
		if (init_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid start arg\n",
			       dev->minor);
			cmd->start_arg = 2000;	
			err++;
		}
		if (chan_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid convert arg\n",
			       dev->minor);
			cmd->convert_arg = 2000;	
			err++;
		}
		if (scan_ticks <= cmd->chanlist_len * chan_ticks) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid scan end arg\n",
			       dev->minor);
			cmd->scan_end_arg = 2000 * cmd->chanlist_len + 31;	
			err++;
		}
	} else if (cmd->start_src == TRIG_EXT &&
		   cmd->scan_begin_src == TRIG_FOLLOW &&
		   cmd->convert_src == TRIG_TIMER) {

		
		if (init_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid start arg\n",
			       dev->minor);
			cmd->start_arg = 2000;	
			err++;
		}
		if (chan_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid convert arg\n",
			       dev->minor);
			cmd->convert_arg = 2000;	
			err++;
		}
	} else if (cmd->start_src == TRIG_EXT &&
		   cmd->scan_begin_src == TRIG_EXT &&
		   cmd->convert_src == TRIG_TIMER) {

		
		if (init_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid start arg\n",
			       dev->minor);
			cmd->start_arg = 2000;	
			err++;
		}
		if (chan_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid convert arg\n",
			       dev->minor);
			cmd->convert_arg = 2000;	
			err++;
		}
	} else if (cmd->start_src == TRIG_EXT &&
		   cmd->scan_begin_src == TRIG_EXT &&
		   cmd->convert_src == TRIG_EXT) {

		
		if (init_ticks < ME4000_AI_MIN_TICKS) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid start arg\n",
			       dev->minor);
			cmd->start_arg = 2000;	
			err++;
		}
	}
	if (cmd->stop_src == TRIG_COUNT) {
		if (cmd->stop_arg == 0) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid stop arg\n",
			       dev->minor);
			cmd->stop_arg = 1;
			err++;
		}
	}
	if (cmd->scan_end_src == TRIG_COUNT) {
		if (cmd->scan_end_arg == 0) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_do_cmd_test(): Invalid scan end arg\n",
			       dev->minor);
			cmd->scan_end_arg = 1;
			err++;
		}
	}

	if (err)
		return 4;

	
	if (ai_check_chanlist(dev, s, cmd))
		return 5;

	return 0;
}

static irqreturn_t me4000_ai_isr(int irq, void *dev_id)
{
	unsigned int tmp;
	struct comedi_device *dev = dev_id;
	struct comedi_subdevice *s = dev->subdevices;
	struct me4000_ai_context *ai_context = &info->ai_context;
	int i;
	int c = 0;
	long lval;

	ISR_PDEBUG("me4000_ai_isr() is executed\n");

	if (!dev->attached) {
		ISR_PDEBUG("me4000_ai_isr() premature interrupt\n");
		return IRQ_NONE;
	}

	
	s->async->events = 0;

	
	if (irq != ai_context->irq) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ai_isr(): Incorrect interrupt num: %d\n",
		       dev->minor, irq);
		return IRQ_HANDLED;
	}

	if (me4000_inl(dev,
		       ai_context->irq_status_reg) &
	    ME4000_IRQ_STATUS_BIT_AI_HF) {
		ISR_PDEBUG
		    ("me4000_ai_isr(): Fifo half full interrupt occured\n");

		
		tmp = me4000_inl(dev, ai_context->ctrl_reg);

		if (!(tmp & ME4000_AI_STATUS_BIT_FF_DATA) &&
		    !(tmp & ME4000_AI_STATUS_BIT_HF_DATA) &&
		    (tmp & ME4000_AI_STATUS_BIT_EF_DATA)) {
			ISR_PDEBUG("me4000_ai_isr(): Fifo full\n");
			c = ME4000_AI_FIFO_COUNT;

			
			tmp |= ME4000_AI_CTRL_BIT_IMMEDIATE_STOP;
			tmp &= ~(ME4000_AI_CTRL_BIT_HF_IRQ |
				 ME4000_AI_CTRL_BIT_SC_IRQ);
			me4000_outl(dev, tmp, ai_context->ctrl_reg);

			s->async->events |= COMEDI_CB_ERROR | COMEDI_CB_EOA;

			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_isr(): FIFO overflow\n",
			       dev->minor);
		} else if ((tmp & ME4000_AI_STATUS_BIT_FF_DATA)
			   && !(tmp & ME4000_AI_STATUS_BIT_HF_DATA)
			   && (tmp & ME4000_AI_STATUS_BIT_EF_DATA)) {
			ISR_PDEBUG("me4000_ai_isr(): Fifo half full\n");

			s->async->events |= COMEDI_CB_BLOCK;

			c = ME4000_AI_FIFO_COUNT / 2;
		} else {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_isr(): Can't determine state of fifo\n",
			       dev->minor);
			c = 0;

			
			tmp |= ME4000_AI_CTRL_BIT_IMMEDIATE_STOP;
			tmp &= ~(ME4000_AI_CTRL_BIT_HF_IRQ |
				 ME4000_AI_CTRL_BIT_SC_IRQ);
			me4000_outl(dev, tmp, ai_context->ctrl_reg);

			s->async->events |= COMEDI_CB_ERROR | COMEDI_CB_EOA;

			printk(KERN_ERR
			       "comedi%d: me4000: me4000_ai_isr(): Undefined FIFO state\n",
			       dev->minor);
		}

		ISR_PDEBUG("me4000_ai_isr(): Try to read %d values\n", c);

		for (i = 0; i < c; i++) {
			
			lval = inl(ai_context->data_reg) & 0xFFFF;
			lval ^= 0x8000;

			if (!comedi_buf_put(s->async, lval)) {
				
				tmp |= ME4000_AI_CTRL_BIT_IMMEDIATE_STOP;
				tmp &= ~(ME4000_AI_CTRL_BIT_HF_IRQ |
					 ME4000_AI_CTRL_BIT_SC_IRQ);
				me4000_outl(dev, tmp, ai_context->ctrl_reg);

				s->async->events |= COMEDI_CB_OVERFLOW;

				printk(KERN_ERR
				       "comedi%d: me4000: me4000_ai_isr(): Buffer overflow\n",
				       dev->minor);

				break;
			}
		}

		
		ISR_PDEBUG("me4000_ai_isr(): Reset fifo half full interrupt\n");
		tmp |= ME4000_AI_CTRL_BIT_HF_IRQ_RESET;
		me4000_outl(dev, tmp, ai_context->ctrl_reg);
		tmp &= ~ME4000_AI_CTRL_BIT_HF_IRQ_RESET;
		me4000_outl(dev, tmp, ai_context->ctrl_reg);
	}

	if (me4000_inl(dev,
		       ai_context->irq_status_reg) & ME4000_IRQ_STATUS_BIT_SC) {
		ISR_PDEBUG
		    ("me4000_ai_isr(): Sample counter interrupt occured\n");

		s->async->events |= COMEDI_CB_BLOCK | COMEDI_CB_EOA;

		
		tmp = me4000_inl(dev, ai_context->ctrl_reg);
		tmp |= ME4000_AI_CTRL_BIT_IMMEDIATE_STOP;
		tmp &= ~(ME4000_AI_CTRL_BIT_HF_IRQ | ME4000_AI_CTRL_BIT_SC_IRQ);
		me4000_outl(dev, tmp, ai_context->ctrl_reg);

		
		while (inl(ai_context->ctrl_reg) & ME4000_AI_STATUS_BIT_EF_DATA) {
			
			lval = inl(ai_context->data_reg) & 0xFFFF;
			lval ^= 0x8000;

			if (!comedi_buf_put(s->async, lval)) {
				printk(KERN_ERR
				       "comedi%d: me4000: me4000_ai_isr(): Buffer overflow\n",
				       dev->minor);
				s->async->events |= COMEDI_CB_OVERFLOW;
				break;
			}
		}

		
		ISR_PDEBUG
		    ("me4000_ai_isr(): Reset interrupt from sample counter\n");
		tmp |= ME4000_AI_CTRL_BIT_SC_IRQ_RESET;
		me4000_outl(dev, tmp, ai_context->ctrl_reg);
		tmp &= ~ME4000_AI_CTRL_BIT_SC_IRQ_RESET;
		me4000_outl(dev, tmp, ai_context->ctrl_reg);
	}

	ISR_PDEBUG("me4000_ai_isr(): Events = 0x%X\n", s->async->events);

	if (s->async->events)
		comedi_event(dev, s);

	return IRQ_HANDLED;
}



static int me4000_ao_insn_write(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data)
{

	int chan = CR_CHAN(insn->chanspec);
	int rang = CR_RANGE(insn->chanspec);
	int aref = CR_AREF(insn->chanspec);
	unsigned long tmp;

	CALL_PDEBUG("In me4000_ao_insn_write()\n");

	if (insn->n == 0) {
		return 0;
	} else if (insn->n > 1) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ao_insn_write(): Invalid instruction length %d\n",
		       dev->minor, insn->n);
		return -EINVAL;
	}

	if (chan >= thisboard->ao.count) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ao_insn_write(): Invalid channel %d\n",
		       dev->minor, insn->n);
		return -EINVAL;
	}

	if (rang != 0) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ao_insn_write(): Invalid range %d\n",
		       dev->minor, insn->n);
		return -EINVAL;
	}

	if (aref != AREF_GROUND && aref != AREF_COMMON) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_ao_insn_write(): Invalid aref %d\n",
		       dev->minor, insn->n);
		return -EINVAL;
	}

	
	tmp = me4000_inl(dev, info->ao_context[chan].ctrl_reg);
	tmp |= ME4000_AO_CTRL_BIT_IMMEDIATE_STOP;
	me4000_outl(dev, tmp, info->ao_context[chan].ctrl_reg);

	
	me4000_outl(dev, 0x0, info->ao_context[chan].ctrl_reg);

	
	me4000_outl(dev, data[0], info->ao_context[chan].single_reg);

	
	info->ao_context[chan].mirror = data[0];

	return 1;
}

static int me4000_ao_insn_read(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data)
{
	int chan = CR_CHAN(insn->chanspec);

	if (insn->n == 0) {
		return 0;
	} else if (insn->n > 1) {
		printk
		    ("comedi%d: me4000: me4000_ao_insn_read(): Invalid instruction length\n",
		     dev->minor);
		return -EINVAL;
	}

	data[0] = info->ao_context[chan].mirror;

	return 1;
}



static int me4000_dio_insn_bits(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data)
{

	CALL_PDEBUG("In me4000_dio_insn_bits()\n");

	
	if (insn->n == 0)
		return 0;

	if (insn->n != 2) {
		printk
		    ("comedi%d: me4000: me4000_dio_insn_bits(): Invalid instruction length\n",
		     dev->minor);
		return -EINVAL;
	}

	
	if (data[0]) {
		
		if ((s->io_bits & data[0]) != data[0])
			return -EIO;

		s->state &= ~data[0];
		s->state |= data[0] & data[1];

		
		me4000_outl(dev, (s->state >> 0) & 0xFF,
			    info->dio_context.port_0_reg);
		me4000_outl(dev, (s->state >> 8) & 0xFF,
			    info->dio_context.port_1_reg);
		me4000_outl(dev, (s->state >> 16) & 0xFF,
			    info->dio_context.port_2_reg);
		me4000_outl(dev, (s->state >> 24) & 0xFF,
			    info->dio_context.port_3_reg);
	}

	
	data[1] =
	    ((me4000_inl(dev, info->dio_context.port_0_reg) & 0xFF) << 0) |
	    ((me4000_inl(dev, info->dio_context.port_1_reg) & 0xFF) << 8) |
	    ((me4000_inl(dev, info->dio_context.port_2_reg) & 0xFF) << 16) |
	    ((me4000_inl(dev, info->dio_context.port_3_reg) & 0xFF) << 24);

	return 2;
}

static int me4000_dio_insn_config(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data)
{
	unsigned long tmp;
	int chan = CR_CHAN(insn->chanspec);

	CALL_PDEBUG("In me4000_dio_insn_config()\n");

	if (data[0] == INSN_CONFIG_DIO_QUERY) {
		data[1] =
		    (s->io_bits & (1 << chan)) ? COMEDI_OUTPUT : COMEDI_INPUT;
		return insn->n;
	}

	

	tmp = me4000_inl(dev, info->dio_context.ctrl_reg);

	if (data[0] == COMEDI_OUTPUT) {
		if (chan < 8) {
			s->io_bits |= 0xFF;
			tmp &= ~(ME4000_DIO_CTRL_BIT_MODE_0 |
				 ME4000_DIO_CTRL_BIT_MODE_1);
			tmp |= ME4000_DIO_CTRL_BIT_MODE_0;
		} else if (chan < 16) {
			
			if (!me4000_inl(dev, info->dio_context.dir_reg))
				return -ENODEV;

			s->io_bits |= 0xFF00;
			tmp &= ~(ME4000_DIO_CTRL_BIT_MODE_2 |
				 ME4000_DIO_CTRL_BIT_MODE_3);
			tmp |= ME4000_DIO_CTRL_BIT_MODE_2;
		} else if (chan < 24) {
			s->io_bits |= 0xFF0000;
			tmp &= ~(ME4000_DIO_CTRL_BIT_MODE_4 |
				 ME4000_DIO_CTRL_BIT_MODE_5);
			tmp |= ME4000_DIO_CTRL_BIT_MODE_4;
		} else if (chan < 32) {
			s->io_bits |= 0xFF000000;
			tmp &= ~(ME4000_DIO_CTRL_BIT_MODE_6 |
				 ME4000_DIO_CTRL_BIT_MODE_7);
			tmp |= ME4000_DIO_CTRL_BIT_MODE_6;
		} else {
			return -EINVAL;
		}
	} else {
		if (chan < 8) {
			
			if (!me4000_inl(dev, info->dio_context.dir_reg))
				return -ENODEV;

			s->io_bits &= ~0xFF;
			tmp &= ~(ME4000_DIO_CTRL_BIT_MODE_0 |
				 ME4000_DIO_CTRL_BIT_MODE_1);
		} else if (chan < 16) {
			s->io_bits &= ~0xFF00;
			tmp &= ~(ME4000_DIO_CTRL_BIT_MODE_2 |
				 ME4000_DIO_CTRL_BIT_MODE_3);
		} else if (chan < 24) {
			s->io_bits &= ~0xFF0000;
			tmp &= ~(ME4000_DIO_CTRL_BIT_MODE_4 |
				 ME4000_DIO_CTRL_BIT_MODE_5);
		} else if (chan < 32) {
			s->io_bits &= ~0xFF000000;
			tmp &= ~(ME4000_DIO_CTRL_BIT_MODE_6 |
				 ME4000_DIO_CTRL_BIT_MODE_7);
		} else {
			return -EINVAL;
		}
	}

	me4000_outl(dev, tmp, info->dio_context.ctrl_reg);

	return 1;
}



static int cnt_reset(struct comedi_device *dev, unsigned int channel)
{

	CALL_PDEBUG("In cnt_reset()\n");

	switch (channel) {
	case 0:
		me4000_outb(dev, 0x30, info->cnt_context.ctrl_reg);
		me4000_outb(dev, 0x00, info->cnt_context.counter_0_reg);
		me4000_outb(dev, 0x00, info->cnt_context.counter_0_reg);
		break;
	case 1:
		me4000_outb(dev, 0x70, info->cnt_context.ctrl_reg);
		me4000_outb(dev, 0x00, info->cnt_context.counter_1_reg);
		me4000_outb(dev, 0x00, info->cnt_context.counter_1_reg);
		break;
	case 2:
		me4000_outb(dev, 0xB0, info->cnt_context.ctrl_reg);
		me4000_outb(dev, 0x00, info->cnt_context.counter_2_reg);
		me4000_outb(dev, 0x00, info->cnt_context.counter_2_reg);
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: cnt_reset(): Invalid channel\n",
		       dev->minor);
		return -EINVAL;
	}

	return 0;
}

static int cnt_config(struct comedi_device *dev, unsigned int channel,
		      unsigned int mode)
{
	int tmp = 0;

	CALL_PDEBUG("In cnt_config()\n");

	switch (channel) {
	case 0:
		tmp |= ME4000_CNT_COUNTER_0;
		break;
	case 1:
		tmp |= ME4000_CNT_COUNTER_1;
		break;
	case 2:
		tmp |= ME4000_CNT_COUNTER_2;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: cnt_config(): Invalid channel\n",
		       dev->minor);
		return -EINVAL;
	}

	switch (mode) {
	case 0:
		tmp |= ME4000_CNT_MODE_0;
		break;
	case 1:
		tmp |= ME4000_CNT_MODE_1;
		break;
	case 2:
		tmp |= ME4000_CNT_MODE_2;
		break;
	case 3:
		tmp |= ME4000_CNT_MODE_3;
		break;
	case 4:
		tmp |= ME4000_CNT_MODE_4;
		break;
	case 5:
		tmp |= ME4000_CNT_MODE_5;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: cnt_config(): Invalid counter mode\n",
		       dev->minor);
		return -EINVAL;
	}

	
	tmp |= 0x30;
	me4000_outb(dev, tmp, info->cnt_context.ctrl_reg);

	return 0;
}

static int me4000_cnt_insn_config(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data)
{

	int err;

	CALL_PDEBUG("In me4000_cnt_insn_config()\n");

	switch (data[0]) {
	case GPCT_RESET:
		if (insn->n != 1) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_cnt_insn_config(): Invalid instruction length%d\n",
			       dev->minor, insn->n);
			return -EINVAL;
		}

		err = cnt_reset(dev, insn->chanspec);
		if (err)
			return err;
		break;
	case GPCT_SET_OPERATION:
		if (insn->n != 2) {
			printk(KERN_ERR
			       "comedi%d: me4000: me4000_cnt_insn_config(): Invalid instruction length%d\n",
			       dev->minor, insn->n);
			return -EINVAL;
		}

		err = cnt_config(dev, insn->chanspec, data[1]);
		if (err)
			return err;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_cnt_insn_config(): Invalid instruction\n",
		       dev->minor);
		return -EINVAL;
	}

	return 2;
}

static int me4000_cnt_insn_read(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data)
{

	unsigned short tmp;

	CALL_PDEBUG("In me4000_cnt_insn_read()\n");

	if (insn->n == 0)
		return 0;

	if (insn->n > 1) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_cnt_insn_read(): Invalid instruction length %d\n",
		       dev->minor, insn->n);
		return -EINVAL;
	}

	switch (insn->chanspec) {
	case 0:
		tmp = me4000_inb(dev, info->cnt_context.counter_0_reg);
		data[0] = tmp;
		tmp = me4000_inb(dev, info->cnt_context.counter_0_reg);
		data[0] |= tmp << 8;
		break;
	case 1:
		tmp = me4000_inb(dev, info->cnt_context.counter_1_reg);
		data[0] = tmp;
		tmp = me4000_inb(dev, info->cnt_context.counter_1_reg);
		data[0] |= tmp << 8;
		break;
	case 2:
		tmp = me4000_inb(dev, info->cnt_context.counter_2_reg);
		data[0] = tmp;
		tmp = me4000_inb(dev, info->cnt_context.counter_2_reg);
		data[0] |= tmp << 8;
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_cnt_insn_read(): Invalid channel %d\n",
		       dev->minor, insn->chanspec);
		return -EINVAL;
	}

	return 1;
}

static int me4000_cnt_insn_write(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 struct comedi_insn *insn, unsigned int *data)
{

	unsigned short tmp;

	CALL_PDEBUG("In me4000_cnt_insn_write()\n");

	if (insn->n == 0) {
		return 0;
	} else if (insn->n > 1) {
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_cnt_insn_write(): Invalid instruction length %d\n",
		       dev->minor, insn->n);
		return -EINVAL;
	}

	switch (insn->chanspec) {
	case 0:
		tmp = data[0] & 0xFF;
		me4000_outb(dev, tmp, info->cnt_context.counter_0_reg);
		tmp = (data[0] >> 8) & 0xFF;
		me4000_outb(dev, tmp, info->cnt_context.counter_0_reg);
		break;
	case 1:
		tmp = data[0] & 0xFF;
		me4000_outb(dev, tmp, info->cnt_context.counter_1_reg);
		tmp = (data[0] >> 8) & 0xFF;
		me4000_outb(dev, tmp, info->cnt_context.counter_1_reg);
		break;
	case 2:
		tmp = data[0] & 0xFF;
		me4000_outb(dev, tmp, info->cnt_context.counter_2_reg);
		tmp = (data[0] >> 8) & 0xFF;
		me4000_outb(dev, tmp, info->cnt_context.counter_2_reg);
		break;
	default:
		printk(KERN_ERR
		       "comedi%d: me4000: me4000_cnt_insn_write(): Invalid channel %d\n",
		       dev->minor, insn->chanspec);
		return -EINVAL;
	}

	return 1;
}

COMEDI_PCI_INITCLEANUP(driver_me4000, me4000_pci_table);
