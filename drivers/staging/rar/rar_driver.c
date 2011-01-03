#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/semaphore.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/ioctl.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/pagemap.h>
#include <linux/pci.h>
#include <linux/firmware.h>
#include <linux/sched.h>
#include "rar_driver.h"





#define LNC_MCR_OFFSET 0xD0


#define LNC_MDR_OFFSET 0xD4


#define LNC_MESSAGE_READ_OPCODE  0xD0
#define LNC_MESSAGE_WRITE_OPCODE 0xE0


#define LNC_MESSAGE_BYTE_WRITE_ENABLES 0xF


#define LNC_BUNIT_PORT 0x3


#define LNC_BRAR0L  0x10
#define LNC_BRAR0H  0x11
#define LNC_BRAR1L  0x12
#define LNC_BRAR1H  0x13


#define LNC_BRAR2L  0x14
#define LNC_BRAR2H  0x15



struct RAR_offsets {
	int low; 
	int high; 
};

struct pci_dev *rar_dev;
static uint32_t registered;


#define MRST_NUM_RAR 3

struct RAR_address_struct rar_addr[MRST_NUM_RAR];


static int __init rar_init_handler(void);
static void __exit rar_exit_handler(void);


static int __devinit rar_probe(struct pci_dev *pdev, const struct pci_device_id *ent);

static struct pci_device_id rar_pci_id_tbl[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_INTEL, 0x4110) },
	{ 0 }
};

MODULE_DEVICE_TABLE(pci, rar_pci_id_tbl);


static struct pci_driver rar_pci_driver = {
	.name = "rar_driver",
	.id_table = rar_pci_id_tbl,
	.probe = rar_probe
};


static int memrar_get_rar_addr(struct pci_dev* pdev,
	                      int offset,
	                      u32 *addr)
{
	

	int result = 0; 
	
	u32 const message =
	       (LNC_MESSAGE_READ_OPCODE << 24)
	       | (LNC_BUNIT_PORT << 16)
	       | (offset << 8)
	       | (LNC_MESSAGE_BYTE_WRITE_ENABLES << 4);

	printk(KERN_WARNING "rar- offset to LNC MSG is %x\n",offset);

	if (addr == 0)
		return -EINVAL;

	
	result = pci_write_config_dword(pdev,
	                          LNC_MCR_OFFSET,
	                          message);

	printk(KERN_WARNING "rar- result from send ctl register is %x\n"
	  ,result);

	if (!result)
		result = pci_read_config_dword(pdev,
		                              LNC_MDR_OFFSET,
				              addr);

	printk(KERN_WARNING "rar- result from read data register is %x\n",
	  result);

	printk(KERN_WARNING "rar- value read from data register is %x\n",
	  *addr);

	if (result)
		return -1;
	else
		return 0;
}

static int memrar_set_rar_addr(struct pci_dev* pdev,
	                      int offset,
	                      u32 addr)
{
	

	int result = 0; 

	
	u32 const message =
	       (LNC_MESSAGE_WRITE_OPCODE << 24)
	       | (LNC_BUNIT_PORT << 16)
	       | (offset << 8)
	       | (LNC_MESSAGE_BYTE_WRITE_ENABLES << 4);

	printk(KERN_WARNING "rar- offset to LNC MSG is %x\n",offset);

	if (addr == 0)
		return -EINVAL;

	
	result = pci_write_config_dword(pdev,
	                          LNC_MDR_OFFSET,
	                          addr);

	printk(KERN_WARNING "rar- result from send ctl register is %x\n"
	  ,result);

	if (!result)
		result = pci_write_config_dword(pdev,
		                              LNC_MCR_OFFSET,
				              message);

	printk(KERN_WARNING "rar- result from write data register is %x\n",
	  result);

	printk(KERN_WARNING "rar- value read to data register is %x\n",
	  addr);

	if (result)
		return -1;
	else
		return 0;
}


static int memrar_init_rar_params(struct pci_dev *pdev)
{
	struct RAR_offsets const offsets[] = {
	       { LNC_BRAR0L, LNC_BRAR0H },
	       { LNC_BRAR1L, LNC_BRAR1H },
	       { LNC_BRAR2L, LNC_BRAR2H }
	};

	size_t const num_offsets = sizeof(offsets) / sizeof(offsets[0]);
	struct RAR_offsets const *end = offsets + num_offsets;
	struct RAR_offsets const *i;
	unsigned int n = 0;
	int result = 0;

	

	

	

	if (pdev == NULL)
	       return -ENODEV;

	for (i = offsets; i != end; ++i, ++n) {
	       if (memrar_get_rar_addr (pdev,
		                       (*i).low,
		                       &(rar_addr[n].low)) != 0
		   || memrar_get_rar_addr (pdev,
		                          (*i).high,
		                          &(rar_addr[n].high)) != 0) {
		       result = -1;
		       break;
	       }
	}

	
	

	if (result == 0) {
	if(1) {
	       size_t z;
	       for (z = 0; z != MRST_NUM_RAR; ++z) {
			printk(KERN_WARNING "rar - BRAR[%Zd] physical address low\n"
			     "\tlow:  0x%08x\n"
			     "\thigh: 0x%08x\n",
			     z,
			     rar_addr[z].low,
			     rar_addr[z].high);
			}
	       }
	}

	return result;
}


static int __devinit rar_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	
	int error;

	

	DEBUG_PRINT_0(RAR_DEBUG_LEVEL_EXTENDED,
	  "Rar pci probe starting\n");
	error = 0;

	
	error = pci_enable_device(pdev);
	if (error) {
		DEBUG_PRINT_0(RAR_DEBUG_LEVEL_EXTENDED,
		  "error enabling pci device\n");
		goto end_function;
	}

	rar_dev = pdev;
	registered = 1;

	
	
	error=memrar_init_rar_params(rar_dev);

	if (error) {
		DEBUG_PRINT_0(RAR_DEBUG_LEVEL_EXTENDED,
		  "error getting RAR addresses device\n");
		registered = 0;
		goto end_function;
		}

end_function:

	return error;
}


static int __init rar_init_handler(void)
{
	return pci_register_driver(&rar_pci_driver);
}

static void __exit rar_exit_handler(void)
{
	pci_unregister_driver(&rar_pci_driver);
}

module_init(rar_init_handler);
module_exit(rar_exit_handler);

MODULE_LICENSE("GPL");



int get_rar_address(int rar_index,struct RAR_address_struct *addresses)
{
	if (registered && (rar_index < 3) && (rar_index >= 0)) {
		*addresses=rar_addr[rar_index];
		
		addresses->low = addresses->low & 0xfffffff0;
		addresses->high = addresses->high & 0xfffffff0;
		return 0;
		}

	else {
		return -ENODEV;
		}
}


EXPORT_SYMBOL(get_rar_address);


int lock_rar(int rar_index)
{
	u32 working_addr;
	int result;
if (registered && (rar_index < 3) && (rar_index >= 0)) {
	
	working_addr=rar_addr[rar_index].low & 0xfffffff0;

	
        result=memrar_set_rar_addr(rar_dev,rar_index,working_addr);
	return result;
	}

else {
	return -ENODEV;
	}
}
