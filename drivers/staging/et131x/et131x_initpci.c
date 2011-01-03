

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
#include <linux/io.h>
#include <linux/bitops.h>
#include <asm/system.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/ioport.h>
#include <linux/random.h>

#include "et1310_phy.h"
#include "et1310_pm.h"
#include "et1310_jagcore.h"

#include "et131x_adapter.h"
#include "et131x_netdev.h"
#include "et131x_config.h"
#include "et131x_isr.h"

#include "et1310_address_map.h"
#include "et1310_tx.h"
#include "et1310_rx.h"
#include "et1310_mac.h"
#include "et1310_eeprom.h"



#define PARM_SPEED_DUPLEX_MIN   0
#define PARM_SPEED_DUPLEX_MAX   5


static u32 et131x_nmi_disable;	
module_param(et131x_nmi_disable, uint, 0);
MODULE_PARM_DESC(et131x_nmi_disable, "Disable NMI (0-2) [0]");


static u32 et131x_speed_set;
module_param(et131x_speed_set, uint, 0);
MODULE_PARM_DESC(et131x_speed_set,
		"Set Link speed and dublex manually (0-5)  [0] \n  1 : 10Mb   Half-Duplex \n  2 : 10Mb   Full-Duplex \n  3 : 100Mb  Half-Duplex \n  4 : 100Mb  Full-Duplex \n  5 : 1000Mb Full-Duplex \n 0 : Auto Speed Auto Dublex");


int et131x_find_adapter(struct et131x_adapter *adapter, struct pci_dev *pdev)
{
	int result;
	uint8_t eepromStat;
	uint8_t maxPayload = 0;
	uint8_t read_size_reg;
	u8 rev;

	
	if (adapter->RegistryNMIDisable) {
		uint8_t RegisterVal;

		RegisterVal = inb(ET1310_NMI_DISABLE);
		RegisterVal &= 0xf3;

		if (adapter->RegistryNMIDisable == 2)
			RegisterVal |= 0xc;

		outb(ET1310_NMI_DISABLE, RegisterVal);
	}

	
	result = pci_read_config_byte(pdev, ET1310_PCI_EEPROM_STATUS,
				      &eepromStat);

	
	result = pci_read_config_byte(pdev, ET1310_PCI_EEPROM_STATUS,
				      &eepromStat);
	if (result != PCIBIOS_SUCCESSFUL) {
		dev_err(&pdev->dev, "Could not read PCI config space for "
			  "EEPROM Status\n");
		return -EIO;
	}

	
	if (eepromStat & 0x4C) {
		result = pci_read_config_byte(pdev, PCI_REVISION_ID, &rev);
		if (result != PCIBIOS_SUCCESSFUL) {
			dev_err(&pdev->dev,
				  "Could not read PCI config space for "
				  "Revision ID\n");
			return -EIO;
		} else if (rev == 0x01) {
			int32_t nLoop;
			uint8_t temp[4] = { 0xFE, 0x13, 0x10, 0xFF };

			
			for (nLoop = 0; nLoop < 3; nLoop++) {
				EepromWriteByte(adapter, nLoop, temp[nLoop]);
			}
		}

		dev_err(&pdev->dev, "Fatal EEPROM Status Error - 0x%04x\n", eepromStat);

		
		adapter->has_eeprom = 0;
		return -EIO;
	} else
		adapter->has_eeprom = 1;

	
	EepromReadByte(adapter, 0x70, &adapter->eepromData[0]);
	EepromReadByte(adapter, 0x71, &adapter->eepromData[1]);

	if (adapter->eepromData[0] != 0xcd)
		
		adapter->eepromData[1] = 0x00;

	
	result = pci_read_config_byte(pdev, ET1310_PCI_MAX_PYLD, &maxPayload);
	if (result != PCIBIOS_SUCCESSFUL) {
		dev_err(&pdev->dev,
		    "Could not read PCI config space for Max Payload Size\n");
		return -EIO;
	}

	
	maxPayload &= 0x07;	

	if (maxPayload < 2) {
		const uint16_t AckNak[2] = { 0x76, 0xD0 };
		const uint16_t Replay[2] = { 0x1E0, 0x2ED };

		result = pci_write_config_word(pdev, ET1310_PCI_ACK_NACK,
					       AckNak[maxPayload]);
		if (result != PCIBIOS_SUCCESSFUL) {
			dev_err(&pdev->dev,
			  "Could not write PCI config space for ACK/NAK\n");
			return -EIO;
		}

		result = pci_write_config_word(pdev, ET1310_PCI_REPLAY,
					       Replay[maxPayload]);
		if (result != PCIBIOS_SUCCESSFUL) {
			dev_err(&pdev->dev,
			  "Could not write PCI config space for Replay Timer\n");
			return -EIO;
		}
	}

	
	result = pci_write_config_byte(pdev, ET1310_PCI_L0L1LATENCY, 0x11);
	if (result != PCIBIOS_SUCCESSFUL) {
		dev_err(&pdev->dev,
		  "Could not write PCI config space for Latency Timers\n");
		return -EIO;
	}

	
	result = pci_read_config_byte(pdev, 0x51, &read_size_reg);
	if (result != PCIBIOS_SUCCESSFUL) {
		dev_err(&pdev->dev,
			"Could not read PCI config space for Max read size\n");
		return -EIO;
	}

	read_size_reg &= 0x8f;
	read_size_reg |= 0x40;

	result = pci_write_config_byte(pdev, 0x51, read_size_reg);
	if (result != PCIBIOS_SUCCESSFUL) {
		dev_err(&pdev->dev,
		      "Could not write PCI config space for Max read size\n");
		return -EIO;
	}

	
	if (adapter->has_eeprom) {
		int i;

		for (i = 0; i < ETH_ALEN; i++) {
			result = pci_read_config_byte(
					pdev, ET1310_PCI_MAC_ADDRESS + i,
					adapter->PermanentAddress + i);
			if (result != PCIBIOS_SUCCESSFUL) {
				dev_err(&pdev->dev, ";Could not read PCI config space for MAC address\n");
				return -EIO;
			}
		}
	}
	return 0;
}


void et131x_error_timer_handler(unsigned long data)
{
	struct et131x_adapter *etdev = (struct et131x_adapter *) data;
	u32 pm_csr;

	pm_csr = readl(&etdev->regs->global.pm_csr);

	if ((pm_csr & ET_PM_PHY_SW_COMA) == 0)
		UpdateMacStatHostCounters(etdev);
	else
		dev_err(&etdev->pdev->dev,
		    "No interrupts, in PHY coma, pm_csr = 0x%x\n", pm_csr);

	if (!etdev->Bmsr.bits.link_status &&
	    etdev->RegistryPhyComa &&
	    etdev->PoMgmt.TransPhyComaModeOnBoot < 11) {
		etdev->PoMgmt.TransPhyComaModeOnBoot++;
	}

	if (etdev->PoMgmt.TransPhyComaModeOnBoot == 10) {
		if (!etdev->Bmsr.bits.link_status
		    && etdev->RegistryPhyComa) {
			if ((pm_csr & ET_PM_PHY_SW_COMA) == 0) {
				
				et131x_enable_interrupts(etdev);
				EnablePhyComa(etdev);
			}
		}
	}

	
	mod_timer(&etdev->ErrorTimer, jiffies +
					  TX_ERROR_PERIOD * HZ / 1000);
}


void et131x_link_detection_handler(unsigned long data)
{
	struct et131x_adapter *etdev = (struct et131x_adapter *) data;
	unsigned long flags;

	if (etdev->MediaState == 0) {
		spin_lock_irqsave(&etdev->Lock, flags);

		etdev->MediaState = NETIF_STATUS_MEDIA_DISCONNECT;
		etdev->Flags &= ~fMP_ADAPTER_LINK_DETECTION;

		spin_unlock_irqrestore(&etdev->Lock, flags);

		netif_carrier_off(etdev->netdev);
	}
}


void ConfigGlobalRegs(struct et131x_adapter *etdev)
{
	struct _GLOBAL_t __iomem *regs = &etdev->regs->global;

	if (etdev->RegistryPhyLoopbk == false) {
		if (etdev->RegistryJumboPacket < 2048) {
			
			writel(0, &regs->rxq_start_addr);
			writel(PARM_RX_MEM_END_DEF, &regs->rxq_end_addr);
			writel(PARM_RX_MEM_END_DEF + 1, &regs->txq_start_addr);
			writel(INTERNAL_MEM_SIZE - 1, &regs->txq_end_addr);
		} else if (etdev->RegistryJumboPacket < 8192) {
			
			writel(0, &regs->rxq_start_addr);
			writel(INTERNAL_MEM_RX_OFFSET, &regs->rxq_end_addr);
			writel(INTERNAL_MEM_RX_OFFSET + 1, &regs->txq_start_addr);
			writel(INTERNAL_MEM_SIZE - 1, &regs->txq_end_addr);
		} else {
			
			writel(0x0000, &regs->rxq_start_addr);
			writel(0x01b3, &regs->rxq_end_addr);
			writel(0x01b4, &regs->txq_start_addr);
			writel(INTERNAL_MEM_SIZE - 1,&regs->txq_end_addr);
		}

		
		writel(0, &regs->loopback);
	} else {
		
		writel(0, &regs->rxq_start_addr);
		writel(INTERNAL_MEM_SIZE - 1, &regs->rxq_end_addr);
		writel(0, &regs->txq_start_addr);
		writel(INTERNAL_MEM_SIZE - 1, &regs->txq_end_addr);

		
		writel(ET_LOOP_MAC, &regs->loopback);
	}

	
	writel(0, &regs->msi_config);

	
	writel(0, &regs->watchdog_timer);
}



int et131x_adapter_setup(struct et131x_adapter *etdev)
{
	int status = 0;

	
	ConfigGlobalRegs(etdev);

	ConfigMACRegs1(etdev);

	
	
	writel(ET_MMC_ENABLE, &etdev->regs->mmc.mmc_ctrl);

	ConfigRxMacRegs(etdev);
	ConfigTxMacRegs(etdev);

	ConfigRxDmaRegs(etdev);
	ConfigTxDmaRegs(etdev);

	ConfigMacStatRegs(etdev);

	
	status = et131x_xcvr_find(etdev);

	if (status != 0)
		dev_warn(&etdev->pdev->dev, "Could not find the xcvr\n");

	
	ET1310_PhyInit(etdev);

	
	ET1310_PhyReset(etdev);

	
	ET1310_PhyPowerDown(etdev, 1);

	
	if (etdev->pdev->device != ET131X_PCI_DEVICE_ID_FAST)
		ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_FULL);
	else
		ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	
	ET1310_PhyPowerDown(etdev, 0);

	et131x_setphy_normal(etdev);
;	return status;
}


void et131x_setup_hardware_properties(struct et131x_adapter *adapter)
{
	
	if (adapter->PermanentAddress[0] == 0x00 &&
	    adapter->PermanentAddress[1] == 0x00 &&
	    adapter->PermanentAddress[2] == 0x00 &&
	    adapter->PermanentAddress[3] == 0x00 &&
	    adapter->PermanentAddress[4] == 0x00 &&
	    adapter->PermanentAddress[5] == 0x00) {
		
		get_random_bytes(&adapter->CurrentAddress[5], 1);
		
		memcpy(adapter->PermanentAddress,
			adapter->CurrentAddress, ETH_ALEN);
	} else {
		
		memcpy(adapter->CurrentAddress,
		       adapter->PermanentAddress, ETH_ALEN);
	}
}


void et131x_soft_reset(struct et131x_adapter *adapter)
{
	
	writel(0xc00f0000, &adapter->regs->mac.cfg1.value);

	
	writel(0x7F, &adapter->regs->global.sw_reset);
	writel(0x000f0000, &adapter->regs->mac.cfg1.value);
	writel(0x00000000, &adapter->regs->mac.cfg1.value);
}


void et131x_align_allocated_memory(struct et131x_adapter *adapter,
				   uint64_t *phys_addr,
				   uint64_t *offset, uint64_t mask)
{
	uint64_t new_addr;

	*offset = 0;

	new_addr = *phys_addr & ~mask;

	if (new_addr != *phys_addr) {
		
		new_addr += mask + 1;
		
		*offset = new_addr - *phys_addr;
		
		*phys_addr = new_addr;
	}
}


int et131x_adapter_memory_alloc(struct et131x_adapter *adapter)
{
	int status = 0;

	do {
		
		status = et131x_tx_dma_memory_alloc(adapter);
		if (status != 0) {
			dev_err(&adapter->pdev->dev,
				  "et131x_tx_dma_memory_alloc FAILED\n");
			break;
		}

		
		status = et131x_rx_dma_memory_alloc(adapter);
		if (status != 0) {
			dev_err(&adapter->pdev->dev,
				  "et131x_rx_dma_memory_alloc FAILED\n");
			et131x_tx_dma_memory_free(adapter);
			break;
		}

		
		status = et131x_init_recv(adapter);
		if (status != 0) {
			dev_err(&adapter->pdev->dev,
				"et131x_init_recv FAILED\n");
			et131x_tx_dma_memory_free(adapter);
			et131x_rx_dma_memory_free(adapter);
			break;
		}
	} while (0);
	return status;
}


void et131x_adapter_memory_free(struct et131x_adapter *adapter)
{
	
	et131x_tx_dma_memory_free(adapter);
	et131x_rx_dma_memory_free(adapter);
}


void et131x_config_parse(struct et131x_adapter *etdev)
{
	static const u8 default_mac[] = { 0x00, 0x05, 0x3d, 0x00, 0x02, 0x00 };
	static const u8 duplex[] = { 0, 1, 2, 1, 2, 2 };
	static const u16 speed[] = { 0, 10, 10, 100, 100, 1000 };

	if (et131x_speed_set)
		dev_info(&etdev->pdev->dev,
			"Speed set manually to : %d \n", et131x_speed_set);

	etdev->SpeedDuplex = et131x_speed_set;
	etdev->RegistryJumboPacket = 1514;	

	etdev->RegistryNMIDisable = et131x_nmi_disable;

	
	memcpy(etdev->CurrentAddress, default_mac, ETH_ALEN);

	
	if (etdev->pdev->device == ET131X_PCI_DEVICE_ID_FAST &&
	    etdev->SpeedDuplex == 5)
		etdev->SpeedDuplex = 4;

	etdev->AiForceSpeed = speed[etdev->SpeedDuplex];
	etdev->AiForceDpx = duplex[etdev->SpeedDuplex];	
}




void __devexit et131x_pci_remove(struct pci_dev *pdev)
{
	struct net_device *netdev;
	struct et131x_adapter *adapter;

	
	netdev = (struct net_device *) pci_get_drvdata(pdev);
	adapter = netdev_priv(netdev);

	
	unregister_netdev(netdev);
	et131x_adapter_memory_free(adapter);
	iounmap(adapter->regs);
	pci_dev_put(adapter->pdev);
	free_netdev(netdev);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
}




int __devinit et131x_pci_setup(struct pci_dev *pdev,
			       const struct pci_device_id *ent)
{
	int result = 0;
	int pm_cap;
	bool pci_using_dac;
	struct net_device *netdev = NULL;
	struct et131x_adapter *adapter = NULL;

	
	result = pci_enable_device(pdev);
	if (result != 0) {
		dev_err(&adapter->pdev->dev,
			"pci_enable_device() failed\n");
		goto out;
	}

	
	if (!(pci_resource_flags(pdev, 0) & IORESOURCE_MEM)) {
		dev_err(&adapter->pdev->dev,
			  "Can't find PCI device's base address\n");
		result = -ENODEV;
		goto out;
	}

	result = pci_request_regions(pdev, DRIVER_NAME);
	if (result != 0) {
		dev_err(&adapter->pdev->dev,
			"Can't get PCI resources\n");
		goto err_disable;
	}

	
	pci_set_master(pdev);

	
	pm_cap = pci_find_capability(pdev, PCI_CAP_ID_PM);
	if (pm_cap == 0) {
		dev_err(&adapter->pdev->dev,
			  "Cannot find Power Management capabilities\n");
		result = -EIO;
		goto err_release_res;
	}

	
	if (!pci_set_dma_mask(pdev, 0xffffffffffffffffULL)) {
		pci_using_dac = true;

		result =
		    pci_set_consistent_dma_mask(pdev, 0xffffffffffffffffULL);
		if (result != 0) {
			dev_err(&pdev->dev,
				  "Unable to obtain 64 bit DMA for consistent allocations\n");
			goto err_release_res;
		}
	} else if (!pci_set_dma_mask(pdev, 0xffffffffULL)) {
		pci_using_dac = false;
	} else {
		dev_err(&adapter->pdev->dev,
			"No usable DMA addressing method\n");
		result = -EIO;
		goto err_release_res;
	}

	
	netdev = et131x_device_alloc();
	if (netdev == NULL) {
		dev_err(&adapter->pdev->dev,
			"Couldn't alloc netdev struct\n");
		result = -ENOMEM;
		goto err_release_res;
	}

	
	SET_NETDEV_DEV(netdev, &pdev->dev);
	

	
	
	
	

	
	adapter = netdev_priv(netdev);
	adapter->pdev = pci_dev_get(pdev);
	adapter->netdev = netdev;

	
	netdev->irq = pdev->irq;
	netdev->base_addr = pdev->resource[0].start;

	
	spin_lock_init(&adapter->Lock);
	spin_lock_init(&adapter->TCBSendQLock);
	spin_lock_init(&adapter->TCBReadyQLock);
	spin_lock_init(&adapter->SendHWLock);
	spin_lock_init(&adapter->SendWaitLock);
	spin_lock_init(&adapter->RcvLock);
	spin_lock_init(&adapter->RcvPendLock);
	spin_lock_init(&adapter->FbrLock);
	spin_lock_init(&adapter->PHYLock);

	
	et131x_config_parse(adapter);

	
	
	et131x_find_adapter(adapter, pdev);

	

	adapter->regs = ioremap_nocache(pci_resource_start(pdev, 0),
					      pci_resource_len(pdev, 0));
	if (adapter->regs == NULL) {
		dev_err(&pdev->dev, "Cannot map device registers\n");
		result = -ENOMEM;
		goto err_free_dev;
	}

	

	
	writel(ET_PMCSR_INIT,  &adapter->regs->global.pm_csr);

	
	et131x_soft_reset(adapter);

	
	et131x_disable_interrupts(adapter);

	
	result = et131x_adapter_memory_alloc(adapter);
	if (result != 0) {
		dev_err(&pdev->dev, "Could not alloc adapater memory (DMA)\n");
		goto err_iounmap;
	}

	
	et131x_init_send(adapter);

	
	INIT_WORK(&adapter->task, et131x_isr_handler);

	
	et131x_setup_hardware_properties(adapter);

	memcpy(netdev->dev_addr, adapter->CurrentAddress, ETH_ALEN);

	
	et131x_adapter_setup(adapter);

	
	init_timer(&adapter->ErrorTimer);

	adapter->ErrorTimer.expires = jiffies + TX_ERROR_PERIOD * HZ / 1000;
	adapter->ErrorTimer.function = et131x_error_timer_handler;
	adapter->ErrorTimer.data = (unsigned long)adapter;

	
	et131x_link_detection_handler((unsigned long)adapter);

	
	adapter->PoMgmt.TransPhyComaModeOnBoot = 0;

	

	
	result = register_netdev(netdev);
	if (result != 0) {
		dev_err(&pdev->dev, "register_netdev() failed\n");
		goto err_mem_free;
	}

	
	pci_set_drvdata(pdev, netdev);

	pci_save_state(adapter->pdev);

out:
	return result;

err_mem_free:
	et131x_adapter_memory_free(adapter);
err_iounmap:
	iounmap(adapter->regs);
err_free_dev:
	pci_dev_put(pdev);
	free_netdev(netdev);
err_release_res:
	pci_release_regions(pdev);
err_disable:
	pci_disable_device(pdev);
	goto out;
}

static struct pci_device_id et131x_pci_table[] __devinitdata = {
	{ET131X_PCI_VENDOR_ID, ET131X_PCI_DEVICE_ID_GIG, PCI_ANY_ID,
	 PCI_ANY_ID, 0, 0, 0UL},
	{ET131X_PCI_VENDOR_ID, ET131X_PCI_DEVICE_ID_FAST, PCI_ANY_ID,
	 PCI_ANY_ID, 0, 0, 0UL},
	{0,}
};

MODULE_DEVICE_TABLE(pci, et131x_pci_table);

static struct pci_driver et131x_driver = {
      .name	= DRIVER_NAME,
      .id_table	= et131x_pci_table,
      .probe	= et131x_pci_setup,
      .remove	= __devexit_p(et131x_pci_remove),
      .suspend	= NULL,		
      .resume	= NULL,		
};



static int et131x_init_module(void)
{
	if (et131x_speed_set < PARM_SPEED_DUPLEX_MIN ||
	    et131x_speed_set > PARM_SPEED_DUPLEX_MAX) {
		printk(KERN_WARNING "et131x: invalid speed setting ignored.\n");
	    	et131x_speed_set = 0;
	}
	return pci_register_driver(&et131x_driver);
}


static void et131x_cleanup_module(void)
{
	pci_unregister_driver(&et131x_driver);
}

module_init(et131x_init_module);
module_exit(et131x_cleanup_module);



MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_INFO);
MODULE_LICENSE(DRIVER_LICENSE);
