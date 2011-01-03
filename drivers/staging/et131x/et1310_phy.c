

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
#include "et131x_initpci.h"

#include "et1310_address_map.h"
#include "et1310_tx.h"
#include "et1310_rx.h"
#include "et1310_mac.h"


static int et131x_xcvr_init(struct et131x_adapter *adapter);


int PhyMiRead(struct et131x_adapter *adapter, uint8_t xcvrAddr,
	      uint8_t xcvrReg, uint16_t *value)
{
	struct _MAC_t __iomem *mac = &adapter->regs->mac;
	int status = 0;
	uint32_t delay;
	MII_MGMT_ADDR_t miiAddr;
	MII_MGMT_CMD_t miiCmd;
	MII_MGMT_INDICATOR_t miiIndicator;

	
	miiAddr.value = readl(&mac->mii_mgmt_addr.value);
	miiCmd.value = readl(&mac->mii_mgmt_cmd.value);

	
	writel(0, &mac->mii_mgmt_cmd.value);

	
	{
		MII_MGMT_ADDR_t mii_mgmt_addr = { 0 };

		mii_mgmt_addr.bits.phy_addr = xcvrAddr;
		mii_mgmt_addr.bits.reg_addr = xcvrReg;
		writel(mii_mgmt_addr.value, &mac->mii_mgmt_addr.value);
	}

	
	delay = 0;

	writel(0x1, &mac->mii_mgmt_cmd.value);

	do {
		udelay(50);
		delay++;
		miiIndicator.value = readl(&mac->mii_mgmt_indicator.value);
	} while ((miiIndicator.bits.not_valid || miiIndicator.bits.busy) &&
		 delay < 50);

	
	if (delay >= 50) {
		dev_warn(&adapter->pdev->dev,
			    "xcvrReg 0x%08x could not be read\n", xcvrReg);
		dev_warn(&adapter->pdev->dev, "status is  0x%08x\n",
			    miiIndicator.value);

		status = -EIO;
	}

	
	
	{
		MII_MGMT_STAT_t mii_mgmt_stat;

		mii_mgmt_stat.value = readl(&mac->mii_mgmt_stat.value);
		*value = (uint16_t) mii_mgmt_stat.bits.phy_stat;
	}

	
	writel(0, &mac->mii_mgmt_cmd.value);

	
	writel(miiAddr.value, &mac->mii_mgmt_addr.value);
	writel(miiCmd.value, &mac->mii_mgmt_cmd.value);

	return status;
}


int MiWrite(struct et131x_adapter *adapter, uint8_t xcvrReg, uint16_t value)
{
	struct _MAC_t __iomem *mac = &adapter->regs->mac;
	int status = 0;
	uint8_t xcvrAddr = adapter->Stats.xcvr_addr;
	uint32_t delay;
	MII_MGMT_ADDR_t miiAddr;
	MII_MGMT_CMD_t miiCmd;
	MII_MGMT_INDICATOR_t miiIndicator;

	
	miiAddr.value = readl(&mac->mii_mgmt_addr.value);
	miiCmd.value = readl(&mac->mii_mgmt_cmd.value);

	
	writel(0, &mac->mii_mgmt_cmd.value);

	
	{
		MII_MGMT_ADDR_t mii_mgmt_addr;

		mii_mgmt_addr.bits.phy_addr = xcvrAddr;
		mii_mgmt_addr.bits.reg_addr = xcvrReg;
		writel(mii_mgmt_addr.value, &mac->mii_mgmt_addr.value);
	}

	
	writel(value, &mac->mii_mgmt_ctrl.value);
	delay = 0;

	do {
		udelay(50);
		delay++;
		miiIndicator.value = readl(&mac->mii_mgmt_indicator.value);
	} while (miiIndicator.bits.busy && delay < 100);

	
	if (delay == 100) {
		uint16_t TempValue;

		dev_warn(&adapter->pdev->dev,
		    "xcvrReg 0x%08x could not be written", xcvrReg);
		dev_warn(&adapter->pdev->dev, "status is  0x%08x\n",
			    miiIndicator.value);
		dev_warn(&adapter->pdev->dev, "command is  0x%08x\n",
			    readl(&mac->mii_mgmt_cmd.value));

		MiRead(adapter, xcvrReg, &TempValue);

		status = -EIO;
	}

	
	writel(0, &mac->mii_mgmt_cmd.value);

	
	writel(miiAddr.value, &mac->mii_mgmt_addr.value);
	writel(miiCmd.value, &mac->mii_mgmt_cmd.value);

	return status;
}


int et131x_xcvr_find(struct et131x_adapter *adapter)
{
	int status = -ENODEV;
	uint8_t xcvr_addr;
	MI_IDR1_t idr1;
	MI_IDR2_t idr2;
	uint32_t xcvr_id;

	
	for (xcvr_addr = 0; xcvr_addr < 32; xcvr_addr++) {
		
		PhyMiRead(adapter, xcvr_addr,
			  (uint8_t) offsetof(MI_REGS_t, idr1),
			  &idr1.value);
		PhyMiRead(adapter, xcvr_addr,
			  (uint8_t) offsetof(MI_REGS_t, idr2),
			  &idr2.value);

		xcvr_id = (uint32_t) ((idr1.value << 16) | idr2.value);

		if ((idr1.value != 0) && (idr1.value != 0xffff)) {
			adapter->Stats.xcvr_id = xcvr_id;
			adapter->Stats.xcvr_addr = xcvr_addr;

			status = 0;
			break;
		}
	}
	return status;
}


int et131x_setphy_normal(struct et131x_adapter *adapter)
{
	int status;

	
	ET1310_PhyPowerDown(adapter, 0);
	status = et131x_xcvr_init(adapter);
	return status;
}


static int et131x_xcvr_init(struct et131x_adapter *adapter)
{
	int status = 0;
	MI_IMR_t imr;
	MI_ISR_t isr;
	MI_LCR2_t lcr2;

	
	adapter->Bmsr.value = 0;

	MiRead(adapter, (uint8_t) offsetof(MI_REGS_t, isr), &isr.value);

	MiRead(adapter, (uint8_t) offsetof(MI_REGS_t, imr), &imr.value);

	
	imr.bits.int_en = 0x1;
	imr.bits.link_status = 0x1;
	imr.bits.autoneg_status = 0x1;

	MiWrite(adapter, (uint8_t) offsetof(MI_REGS_t, imr), imr.value);

	
	if ((adapter->eepromData[1] & 0x4) == 0) {
		MiRead(adapter, (uint8_t) offsetof(MI_REGS_t, lcr2),
		       &lcr2.value);
		if ((adapter->eepromData[1] & 0x8) == 0)
			lcr2.bits.led_tx_rx = 0x3;
		else
			lcr2.bits.led_tx_rx = 0x4;
		lcr2.bits.led_link = 0xa;
		MiWrite(adapter, (uint8_t) offsetof(MI_REGS_t, lcr2),
			lcr2.value);
	}

	
	if (adapter->AiForceSpeed == 0 && adapter->AiForceDpx == 0) {
		if ((adapter->RegistryFlowControl == TxOnly) ||
		    (adapter->RegistryFlowControl == Both)) {
			ET1310_PhyAccessMiBit(adapter,
					      TRUEPHY_BIT_SET, 4, 11, NULL);
		} else {
			ET1310_PhyAccessMiBit(adapter,
					      TRUEPHY_BIT_CLEAR, 4, 11, NULL);
		}

		if (adapter->RegistryFlowControl == Both) {
			ET1310_PhyAccessMiBit(adapter,
					      TRUEPHY_BIT_SET, 4, 10, NULL);
		} else {
			ET1310_PhyAccessMiBit(adapter,
					      TRUEPHY_BIT_CLEAR, 4, 10, NULL);
		}

		
		ET1310_PhyAutoNeg(adapter, true);

		
		ET1310_PhyAccessMiBit(adapter, TRUEPHY_BIT_SET, 0, 9, NULL);
		return status;
	} else {
		ET1310_PhyAutoNeg(adapter, false);

		
		if (adapter->AiForceDpx != 1) {
			if ((adapter->RegistryFlowControl == TxOnly) ||
			    (adapter->RegistryFlowControl == Both)) {
				ET1310_PhyAccessMiBit(adapter,
						      TRUEPHY_BIT_SET, 4, 11,
						      NULL);
			} else {
				ET1310_PhyAccessMiBit(adapter,
						      TRUEPHY_BIT_CLEAR, 4, 11,
						      NULL);
			}

			if (adapter->RegistryFlowControl == Both) {
				ET1310_PhyAccessMiBit(adapter,
						      TRUEPHY_BIT_SET, 4, 10,
						      NULL);
			} else {
				ET1310_PhyAccessMiBit(adapter,
						      TRUEPHY_BIT_CLEAR, 4, 10,
						      NULL);
			}
		} else {
			ET1310_PhyAccessMiBit(adapter,
					      TRUEPHY_BIT_CLEAR, 4, 10, NULL);
			ET1310_PhyAccessMiBit(adapter,
					      TRUEPHY_BIT_CLEAR, 4, 11, NULL);
		}

		switch (adapter->AiForceSpeed) {
		case 10:
			if (adapter->AiForceDpx == 1)
				TPAL_SetPhy10HalfDuplex(adapter);
			else if (adapter->AiForceDpx == 2)
				TPAL_SetPhy10FullDuplex(adapter);
			else
				TPAL_SetPhy10Force(adapter);
			break;
		case 100:
			if (adapter->AiForceDpx == 1)
				TPAL_SetPhy100HalfDuplex(adapter);
			else if (adapter->AiForceDpx == 2)
				TPAL_SetPhy100FullDuplex(adapter);
			else
				TPAL_SetPhy100Force(adapter);
			break;
		case 1000:
			TPAL_SetPhy1000FullDuplex(adapter);
			break;
		}

		return status;
	}
}

void et131x_Mii_check(struct et131x_adapter *etdev,
		      MI_BMSR_t bmsr, MI_BMSR_t bmsr_ints)
{
	uint8_t link_status;
	uint32_t autoneg_status;
	uint32_t speed;
	uint32_t duplex;
	uint32_t mdi_mdix;
	uint32_t masterslave;
	uint32_t polarity;
	unsigned long flags;

	if (bmsr_ints.bits.link_status) {
		if (bmsr.bits.link_status) {
			etdev->PoMgmt.TransPhyComaModeOnBoot = 20;

			
			spin_lock_irqsave(&etdev->Lock, flags);

			etdev->MediaState = NETIF_STATUS_MEDIA_CONNECT;
			etdev->Flags &= ~fMP_ADAPTER_LINK_DETECTION;

			spin_unlock_irqrestore(&etdev->Lock, flags);

			
			if (etdev->RegistryPhyLoopbk == false)
				netif_carrier_on(etdev->netdev);
		} else {
			dev_warn(&etdev->pdev->dev,
			    "Link down - cable problem ?\n");

			if (etdev->linkspeed == TRUEPHY_SPEED_10MBPS) {
				
				uint16_t Register18;

				MiRead(etdev, 0x12, &Register18);
				MiWrite(etdev, 0x12, Register18 | 0x4);
				MiWrite(etdev, 0x10, Register18 | 0x8402);
				MiWrite(etdev, 0x11, Register18 | 511);
				MiWrite(etdev, 0x12, Register18);
			}

			
			if (!(etdev->Flags & fMP_ADAPTER_LINK_DETECTION) ||
			  (etdev->MediaState == NETIF_STATUS_MEDIA_DISCONNECT)) {
				spin_lock_irqsave(&etdev->Lock, flags);
				etdev->MediaState =
				    NETIF_STATUS_MEDIA_DISCONNECT;
				spin_unlock_irqrestore(&etdev->Lock,
						       flags);

				
				if (etdev->RegistryPhyLoopbk == false)
					netif_carrier_off(etdev->netdev);
			}

			etdev->linkspeed = 0;
			etdev->duplex_mode = 0;

			
			et131x_free_busy_send_packets(etdev);

			
			et131x_init_send(etdev);

			
			et131x_reset_recv(etdev);

			
			et131x_soft_reset(etdev);

			
			et131x_adapter_setup(etdev);

			
			if (etdev->RegistryPhyComa == 1)
				EnablePhyComa(etdev);
		}
	}

	if (bmsr_ints.bits.auto_neg_complete ||
	    (etdev->AiForceDpx == 3 && bmsr_ints.bits.link_status)) {
		if (bmsr.bits.auto_neg_complete || etdev->AiForceDpx == 3) {
			ET1310_PhyLinkStatus(etdev,
					     &link_status, &autoneg_status,
					     &speed, &duplex, &mdi_mdix,
					     &masterslave, &polarity);

			etdev->linkspeed = speed;
			etdev->duplex_mode = duplex;

			etdev->PoMgmt.TransPhyComaModeOnBoot = 20;

			if (etdev->linkspeed == TRUEPHY_SPEED_10MBPS) {
				
				uint16_t Register18;

				MiRead(etdev, 0x12, &Register18);
				MiWrite(etdev, 0x12, Register18 | 0x4);
				MiWrite(etdev, 0x10, Register18 | 0x8402);
				MiWrite(etdev, 0x11, Register18 | 511);
				MiWrite(etdev, 0x12, Register18);
			}

			ConfigFlowControl(etdev);

			if (etdev->linkspeed == TRUEPHY_SPEED_1000MBPS &&
					etdev->RegistryJumboPacket > 2048)
				ET1310_PhyAndOrReg(etdev, 0x16, 0xcfff,
								   0x2000);

			SetRxDmaTimer(etdev);
			ConfigMACRegs2(etdev);
		}
	}
}


void TPAL_SetPhy10HalfDuplex(struct et131x_adapter *etdev)
{
	
	ET1310_PhyPowerDown(etdev, 1);

	
	ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	ET1310_PhyAdvertise100BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	
	ET1310_PhyAdvertise10BaseT(etdev, TRUEPHY_ADV_DUPLEX_HALF);

	
	ET1310_PhyPowerDown(etdev, 0);
}


void TPAL_SetPhy10FullDuplex(struct et131x_adapter *etdev)
{
	
	ET1310_PhyPowerDown(etdev, 1);

	
	ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	ET1310_PhyAdvertise100BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	
	ET1310_PhyAdvertise10BaseT(etdev, TRUEPHY_ADV_DUPLEX_FULL);

	
	ET1310_PhyPowerDown(etdev, 0);
}


void TPAL_SetPhy10Force(struct et131x_adapter *etdev)
{
	
	ET1310_PhyPowerDown(etdev, 1);

	
	ET1310_PhyAutoNeg(etdev, false);

	
	ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);
	ET1310_PhyAdvertise10BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);
	ET1310_PhyAdvertise100BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	
	ET1310_PhySpeedSelect(etdev, TRUEPHY_SPEED_10MBPS);

	
	ET1310_PhyDuplexMode(etdev, TRUEPHY_DUPLEX_FULL);

	
	ET1310_PhyPowerDown(etdev, 0);
}


void TPAL_SetPhy100HalfDuplex(struct et131x_adapter *etdev)
{
	
	ET1310_PhyPowerDown(etdev, 1);

	
	ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	ET1310_PhyAdvertise10BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	
	ET1310_PhyAdvertise100BaseT(etdev, TRUEPHY_ADV_DUPLEX_HALF);

	
	ET1310_PhySpeedSelect(etdev, TRUEPHY_SPEED_100MBPS);

	
	ET1310_PhyPowerDown(etdev, 0);
}


void TPAL_SetPhy100FullDuplex(struct et131x_adapter *etdev)
{
	
	ET1310_PhyPowerDown(etdev, 1);

	
	ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	ET1310_PhyAdvertise10BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	
	ET1310_PhyAdvertise100BaseT(etdev, TRUEPHY_ADV_DUPLEX_FULL);

	
	ET1310_PhyPowerDown(etdev, 0);
}


void TPAL_SetPhy100Force(struct et131x_adapter *etdev)
{
	
	ET1310_PhyPowerDown(etdev, 1);

	
	ET1310_PhyAutoNeg(etdev, false);

	
	ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);
	ET1310_PhyAdvertise10BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);
	ET1310_PhyAdvertise100BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	
	ET1310_PhySpeedSelect(etdev, TRUEPHY_SPEED_100MBPS);

	
	ET1310_PhyDuplexMode(etdev, TRUEPHY_DUPLEX_FULL);

	
	ET1310_PhyPowerDown(etdev, 0);
}


void TPAL_SetPhy1000FullDuplex(struct et131x_adapter *etdev)
{
	
	ET1310_PhyPowerDown(etdev, 1);

	
	ET1310_PhyAdvertise100BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	ET1310_PhyAdvertise10BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	
	ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_FULL);

	
	ET1310_PhyPowerDown(etdev, 0);
}


void TPAL_SetPhyAutoNeg(struct et131x_adapter *etdev)
{
	
	ET1310_PhyPowerDown(etdev, 1);

	
	ET1310_PhyAdvertise10BaseT(etdev, TRUEPHY_ADV_DUPLEX_BOTH);

	ET1310_PhyAdvertise100BaseT(etdev, TRUEPHY_ADV_DUPLEX_BOTH);

	if (etdev->pdev->device != ET131X_PCI_DEVICE_ID_FAST)
		ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_FULL);
	else
		ET1310_PhyAdvertise1000BaseT(etdev, TRUEPHY_ADV_DUPLEX_NONE);

	
	ET1310_PhyAutoNeg(etdev, true);

	
	ET1310_PhyPowerDown(etdev, 0);
}




static const uint16_t ConfigPhy[25][2] = {
	
	
	{0x880B, 0x0926},	
	{0x880C, 0x0926},	
	{0x880D, 0x0926},	

	{0x880E, 0xB4D3},	
	{0x880F, 0xB4D3},	
	{0x8810, 0xB4D3},	

	{0x8805, 0xB03E},	
	{0x8806, 0xB03E},	
	{0x8807, 0xFF00},	

	{0x8808, 0xE090},	
	{0x8809, 0xE110},	
	{0x880A, 0x0000},	

	{0x300D, 1},		

	{0x280C, 0x0180},	

	{0x1C21, 0x0002},	

	{0x3821, 6},		
	{0x381D, 1},		
	{0x381E, 1},		
	{0x381F, 1},		
	{0x3820, 1},		

	{0x8402, 0x01F0},	
	{0x800E, 20},		
	{0x800F, 24},		
	{0x8010, 46},		

	{0, 0}

};


void ET1310_PhyInit(struct et131x_adapter *etdev)
{
	uint16_t data, index;

	if (etdev == NULL)
		return;

	
	MiRead(etdev, PHY_ID_1, &data);
	MiRead(etdev, PHY_ID_2, &data);

	
	MiRead(etdev, PHY_MPHY_CONTROL_REG, &data); 
	MiWrite(etdev, PHY_MPHY_CONTROL_REG,	0x0006);

	
	MiWrite(etdev, PHY_INDEX_REG, 0x0402);
	MiRead(etdev, PHY_DATA_REG, &data);

	
	MiWrite(etdev, PHY_MPHY_CONTROL_REG, 0x0002);

	
	MiRead(etdev, PHY_ID_1, &data);
	MiRead(etdev, PHY_ID_2, &data);

	
	MiRead(etdev, PHY_MPHY_CONTROL_REG, &data); 
	MiWrite(etdev, PHY_MPHY_CONTROL_REG, 0x0006);

	
	MiWrite(etdev, PHY_INDEX_REG, 0x0402);
	MiRead(etdev, PHY_DATA_REG, &data);

	MiWrite(etdev, PHY_MPHY_CONTROL_REG, 0x0002);

	
	MiRead(etdev, PHY_CONTROL, &data);
	MiRead(etdev, PHY_MPHY_CONTROL_REG, &data); 
	MiWrite(etdev, PHY_CONTROL, 0x1840);

	MiWrite(etdev, PHY_MPHY_CONTROL_REG, 0x0007);

	
	index = 0;
	while (ConfigPhy[index][0] != 0x0000) {
		
		MiWrite(etdev, PHY_INDEX_REG, ConfigPhy[index][0]);
		MiWrite(etdev, PHY_DATA_REG, ConfigPhy[index][1]);

		
		MiWrite(etdev, PHY_INDEX_REG, ConfigPhy[index][0]);
		MiRead(etdev, PHY_DATA_REG, &data);

		
		index++;
	}
	

	MiRead(etdev, PHY_CONTROL, &data);		
	MiRead(etdev, PHY_MPHY_CONTROL_REG, &data);
	MiWrite(etdev, PHY_CONTROL, 0x1040);
	MiWrite(etdev, PHY_MPHY_CONTROL_REG, 0x0002);
}

void ET1310_PhyReset(struct et131x_adapter *etdev)
{
	MiWrite(etdev, PHY_CONTROL, 0x8000);
}

void ET1310_PhyPowerDown(struct et131x_adapter *etdev, bool down)
{
	uint16_t data;

	MiRead(etdev, PHY_CONTROL, &data);

	if (down == false) {
		
		data &= ~0x0800;
		MiWrite(etdev, PHY_CONTROL, data);
	} else {
		
		data |= 0x0800;
		MiWrite(etdev, PHY_CONTROL, data);
	}
}

void ET1310_PhyAutoNeg(struct et131x_adapter *etdev, bool enable)
{
	uint16_t data;

	MiRead(etdev, PHY_CONTROL, &data);

	if (enable == true) {
		
		data |= 0x1000;
		MiWrite(etdev, PHY_CONTROL, data);
	} else {
		
		data &= ~0x1000;
		MiWrite(etdev, PHY_CONTROL, data);
	}
}

void ET1310_PhyDuplexMode(struct et131x_adapter *etdev, uint16_t duplex)
{
	uint16_t data;

	MiRead(etdev, PHY_CONTROL, &data);

	if (duplex == TRUEPHY_DUPLEX_FULL) {
		
		data |= 0x100;
		MiWrite(etdev, PHY_CONTROL, data);
	} else {
		
		data &= ~0x100;
		MiWrite(etdev, PHY_CONTROL, data);
	}
}

void ET1310_PhySpeedSelect(struct et131x_adapter *etdev, uint16_t speed)
{
	uint16_t data;

	
	MiRead(etdev, PHY_CONTROL, &data);

	
	data &= ~0x2040;

	
	switch (speed) {
	case TRUEPHY_SPEED_10MBPS:
		
		break;

	case TRUEPHY_SPEED_100MBPS:
		
		data |= 0x2000;
		break;

	case TRUEPHY_SPEED_1000MBPS:
	default:
		data |= 0x0040;
		break;
	}

	
	MiWrite(etdev, PHY_CONTROL, data);
}

void ET1310_PhyAdvertise1000BaseT(struct et131x_adapter *etdev,
				  uint16_t duplex)
{
	uint16_t data;

	
	MiRead(etdev, PHY_1000_CONTROL, &data);

	
	data &= ~0x0300;

	switch (duplex) {
	case TRUEPHY_ADV_DUPLEX_NONE:
		
		break;

	case TRUEPHY_ADV_DUPLEX_FULL:
		
		data |= 0x0200;
		break;

	case TRUEPHY_ADV_DUPLEX_HALF:
		
		data |= 0x0100;
		break;

	case TRUEPHY_ADV_DUPLEX_BOTH:
	default:
		data |= 0x0300;
		break;
	}

	
	MiWrite(etdev, PHY_1000_CONTROL, data);
}

void ET1310_PhyAdvertise100BaseT(struct et131x_adapter *etdev,
				 uint16_t duplex)
{
	uint16_t data;

	
	MiRead(etdev, PHY_AUTO_ADVERTISEMENT, &data);

	
	data &= ~0x0180;

	switch (duplex) {
	case TRUEPHY_ADV_DUPLEX_NONE:
		
		break;

	case TRUEPHY_ADV_DUPLEX_FULL:
		
		data |= 0x0100;
		break;

	case TRUEPHY_ADV_DUPLEX_HALF:
		
		data |= 0x0080;
		break;

	case TRUEPHY_ADV_DUPLEX_BOTH:
	default:
		
		data |= 0x0180;
		break;
	}

	
	MiWrite(etdev, PHY_AUTO_ADVERTISEMENT, data);
}

void ET1310_PhyAdvertise10BaseT(struct et131x_adapter *etdev,
				uint16_t duplex)
{
	uint16_t data;

	
	MiRead(etdev, PHY_AUTO_ADVERTISEMENT, &data);

	
	data &= ~0x0060;

	switch (duplex) {
	case TRUEPHY_ADV_DUPLEX_NONE:
		
		break;

	case TRUEPHY_ADV_DUPLEX_FULL:
		
		data |= 0x0040;
		break;

	case TRUEPHY_ADV_DUPLEX_HALF:
		
		data |= 0x0020;
		break;

	case TRUEPHY_ADV_DUPLEX_BOTH:
	default:
		
		data |= 0x0060;
		break;
	}

	
	MiWrite(etdev, PHY_AUTO_ADVERTISEMENT, data);
}

void ET1310_PhyLinkStatus(struct et131x_adapter *etdev,
			  uint8_t *link_status,
			  uint32_t *autoneg,
			  uint32_t *linkspeed,
			  uint32_t *duplex_mode,
			  uint32_t *mdi_mdix,
			  uint32_t *masterslave, uint32_t *polarity)
{
	uint16_t mistatus = 0;
	uint16_t is1000BaseT = 0;
	uint16_t vmi_phystatus = 0;
	uint16_t control = 0;

	MiRead(etdev, PHY_STATUS, &mistatus);
	MiRead(etdev, PHY_1000_STATUS, &is1000BaseT);
	MiRead(etdev, PHY_PHY_STATUS, &vmi_phystatus);
	MiRead(etdev, PHY_CONTROL, &control);

	if (link_status) {
		*link_status =
		    (unsigned char)((vmi_phystatus & 0x0040) ? 1 : 0);
	}

	if (autoneg) {
		*autoneg =
		    (control & 0x1000) ? ((vmi_phystatus & 0x0020) ?
					    TRUEPHY_ANEG_COMPLETE :
					    TRUEPHY_ANEG_NOT_COMPLETE) :
		    TRUEPHY_ANEG_DISABLED;
	}

	if (linkspeed)
		*linkspeed = (vmi_phystatus & 0x0300) >> 8;

	if (duplex_mode)
		*duplex_mode = (vmi_phystatus & 0x0080) >> 7;

	if (mdi_mdix)
		
		*mdi_mdix = 0;

	if (masterslave) {
		*masterslave =
		    (is1000BaseT & 0x4000) ? TRUEPHY_CFG_MASTER :
		    TRUEPHY_CFG_SLAVE;
	}

	if (polarity) {
		*polarity =
		    (vmi_phystatus & 0x0400) ? TRUEPHY_POLARITY_INVERTED :
		    TRUEPHY_POLARITY_NORMAL;
	}
}

void ET1310_PhyAndOrReg(struct et131x_adapter *etdev,
			uint16_t regnum, uint16_t andMask, uint16_t orMask)
{
	uint16_t reg;

	
	MiRead(etdev, regnum, &reg);

	
	reg &= andMask;

	
	reg |= orMask;

	
	MiWrite(etdev, regnum, reg);
}

void ET1310_PhyAccessMiBit(struct et131x_adapter *etdev, uint16_t action,
			   uint16_t regnum, uint16_t bitnum, uint8_t *value)
{
	uint16_t reg;
	uint16_t mask = 0;

	
	mask = 0x0001 << bitnum;

	
	MiRead(etdev, regnum, &reg);

	switch (action) {
	case TRUEPHY_BIT_READ:
		if (value != NULL)
			*value = (reg & mask) >> bitnum;
		break;

	case TRUEPHY_BIT_SET:
		reg |= mask;
		MiWrite(etdev, regnum, reg);
		break;

	case TRUEPHY_BIT_CLEAR:
		reg &= ~mask;
		MiWrite(etdev, regnum, reg);
		break;

	default:
		break;
	}
}
