

#include "et131x_version.h"
#include "et131x_defs.h"

#include <linux/pci.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>

#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/in.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <asm/system.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/ioport.h>

#include "et1310_phy.h"
#include "et1310_pm.h"
#include "et1310_jagcore.h"
#include "et1310_eeprom.h"

#include "et131x_adapter.h"
#include "et131x_initpci.h"
#include "et131x_isr.h"

#include "et1310_tx.h"





#define LBCIF_DWORD0_GROUP_OFFSET       0xAC
#define LBCIF_DWORD1_GROUP_OFFSET       0xB0


#define LBCIF_ADDRESS_REGISTER_OFFSET   0xAC
#define LBCIF_DATA_REGISTER_OFFSET      0xB0
#define LBCIF_CONTROL_REGISTER_OFFSET   0xB1
#define LBCIF_STATUS_REGISTER_OFFSET    0xB2


#define LBCIF_CONTROL_SEQUENTIAL_READ   0x01
#define LBCIF_CONTROL_PAGE_WRITE        0x02
#define LBCIF_CONTROL_UNUSED1           0x04
#define LBCIF_CONTROL_EEPROM_RELOAD     0x08
#define LBCIF_CONTROL_UNUSED2           0x10
#define LBCIF_CONTROL_TWO_BYTE_ADDR     0x20
#define LBCIF_CONTROL_I2C_WRITE         0x40
#define LBCIF_CONTROL_LBCIF_ENABLE      0x80


#define LBCIF_STATUS_PHY_QUEUE_AVAIL    0x01
#define LBCIF_STATUS_I2C_IDLE           0x02
#define LBCIF_STATUS_ACK_ERROR          0x04
#define LBCIF_STATUS_GENERAL_ERROR      0x08
#define LBCIF_STATUS_UNUSED             0x30
#define LBCIF_STATUS_CHECKSUM_ERROR     0x40
#define LBCIF_STATUS_EEPROM_PRESENT     0x80


#define MAX_NUM_REGISTER_POLLS          1000
#define MAX_NUM_WRITE_RETRIES           2


#define EXTRACT_DATA_REGISTER(x)    (u8)(x & 0xFF)
#define EXTRACT_STATUS_REGISTER(x)  (u8)((x >> 16) & 0xFF)
#define EXTRACT_CONTROL_REG(x)      (u8)((x >> 8) & 0xFF)


int EepromWriteByte(struct et131x_adapter *etdev, u32 addr, u8 data)
{
	struct pci_dev *pdev = etdev->pdev;
	int index;
	int retries;
	int err = 0;
	int i2c_wack = 0;
	int writeok = 0;
	u8 control;
	u8 status = 0;
	u32 dword1 = 0;
	u32 val = 0;

	

	
	for (index = 0; index < MAX_NUM_REGISTER_POLLS; index++) {
		
		if (pci_read_config_dword(pdev, LBCIF_DWORD1_GROUP_OFFSET,
					  &dword1)) {
			err = 1;
			break;
		}

		status = EXTRACT_STATUS_REGISTER(dword1);

		if (status & LBCIF_STATUS_PHY_QUEUE_AVAIL &&
			status & LBCIF_STATUS_I2C_IDLE)
			
			break;
	}

	if (err || (index >= MAX_NUM_REGISTER_POLLS))
		return FAILURE;

	
	control = 0;
	control |= LBCIF_CONTROL_LBCIF_ENABLE | LBCIF_CONTROL_I2C_WRITE;

	if (pci_write_config_byte(pdev, LBCIF_CONTROL_REGISTER_OFFSET,
				  control)) {
		return FAILURE;
	}

	i2c_wack = 1;

	

	for (retries = 0; retries < MAX_NUM_WRITE_RETRIES; retries++) {
		
		if (pci_write_config_dword(pdev, LBCIF_ADDRESS_REGISTER_OFFSET,
					   addr)) {
			break;
		}

		
		if (pci_write_config_byte(pdev, LBCIF_DATA_REGISTER_OFFSET,
					  data)) {
			break;
		}

		
		for (index = 0; index < MAX_NUM_REGISTER_POLLS; index++) {
			
			if (pci_read_config_dword(pdev,
						  LBCIF_DWORD1_GROUP_OFFSET,
						  &dword1)) {
				err = 1;
				break;
			}

			status = EXTRACT_STATUS_REGISTER(dword1);

			if (status & LBCIF_STATUS_PHY_QUEUE_AVAIL &&
				status & LBCIF_STATUS_I2C_IDLE) {
				
				break;
			}
		}

		if (err || (index >= MAX_NUM_REGISTER_POLLS))
			break;

		
		if (status & LBCIF_STATUS_GENERAL_ERROR
		    && etdev->pdev->revision == 0) {
			break;
		}

		
		if (status & LBCIF_STATUS_ACK_ERROR) {
			
			udelay(10);
			continue;
		}

		writeok = 1;
		break;
	}

	
	udelay(10);
	index = 0;
	while (i2c_wack) {
		control &= ~LBCIF_CONTROL_I2C_WRITE;

		if (pci_write_config_byte(pdev, LBCIF_CONTROL_REGISTER_OFFSET,
					  control)) {
			writeok = 0;
		}

		
		do {
			pci_write_config_dword(pdev,
					       LBCIF_ADDRESS_REGISTER_OFFSET,
					       addr);
			do {
				pci_read_config_dword(pdev,
					LBCIF_DATA_REGISTER_OFFSET, &val);
			} while ((val & 0x00010000) == 0);
		} while (val & 0x00040000);

		control = EXTRACT_CONTROL_REG(val);

		if (control != 0xC0 || index == 10000)
			break;

		index++;
	}

	return writeok ? SUCCESS : FAILURE;
}


int EepromReadByte(struct et131x_adapter *etdev, u32 addr, u8 *pdata)
{
	struct pci_dev *pdev = etdev->pdev;
	int index;
	int err = 0;
	u8 control;
	u8 status = 0;
	u32 dword1 = 0;

	

	
	for (index = 0; index < MAX_NUM_REGISTER_POLLS; index++) {
		
		if (pci_read_config_dword(pdev, LBCIF_DWORD1_GROUP_OFFSET,
					  &dword1)) {
			err = 1;
			break;
		}

		status = EXTRACT_STATUS_REGISTER(dword1);

		if (status & LBCIF_STATUS_PHY_QUEUE_AVAIL &&
		    status & LBCIF_STATUS_I2C_IDLE) {
			
			break;
		}
	}

	if (err || (index >= MAX_NUM_REGISTER_POLLS))
		return FAILURE;

	
	control = 0;
	control |= LBCIF_CONTROL_LBCIF_ENABLE;

	if (pci_write_config_byte(pdev, LBCIF_CONTROL_REGISTER_OFFSET,
				  control)) {
		return FAILURE;
	}

	

	if (pci_write_config_dword(pdev, LBCIF_ADDRESS_REGISTER_OFFSET,
				   addr)) {
		return FAILURE;
	}

	
	for (index = 0; index < MAX_NUM_REGISTER_POLLS; index++) {
		
		if (pci_read_config_dword(pdev, LBCIF_DWORD1_GROUP_OFFSET,
					  &dword1)) {
			err = 1;
			break;
		}

		status = EXTRACT_STATUS_REGISTER(dword1);

		if (status & LBCIF_STATUS_PHY_QUEUE_AVAIL
		    && status & LBCIF_STATUS_I2C_IDLE) {
			
			break;
		}
	}

	if (err || (index >= MAX_NUM_REGISTER_POLLS))
		return FAILURE;

	
	*pdata = EXTRACT_DATA_REGISTER(dword1);

	return (status & LBCIF_STATUS_ACK_ERROR) ? FAILURE : SUCCESS;
}
