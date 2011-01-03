

#include "et131x_version.h"
#include "et131x_defs.h"

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
#include <linux/pci.h>
#include <asm/system.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/ioport.h>

#include "et1310_phy.h"
#include "et1310_pm.h"
#include "et1310_jagcore.h"
#include "et1310_mac.h"

#include "et131x_adapter.h"



void et131x_enable_interrupts(struct et131x_adapter *adapter)
{
	u32 mask;

	
	if (adapter->FlowControl == TxOnly || adapter->FlowControl == Both)
		mask = INT_MASK_ENABLE;
	else
		mask = INT_MASK_ENABLE_NO_FLOW;

	if (adapter->DriverNoPhyAccess)
		mask |= ET_INTR_PHY;

	adapter->CachedMaskValue = mask;
	writel(mask, &adapter->regs->global.int_mask);
}



void et131x_disable_interrupts(struct et131x_adapter *adapter)
{
	
	adapter->CachedMaskValue = INT_MASK_DISABLE;
	writel(INT_MASK_DISABLE, &adapter->regs->global.int_mask);
}




irqreturn_t et131x_isr(int irq, void *dev_id)
{
	bool handled = true;
	struct net_device *netdev = (struct net_device *)dev_id;
	struct et131x_adapter *adapter = NULL;
	u32 status;

	if (!netif_device_present(netdev)) {
		handled = false;
		goto out;
	}

	adapter = netdev_priv(netdev);

	

	
	et131x_disable_interrupts(adapter);

	
	status = readl(&adapter->regs->global.int_status);

	if (adapter->FlowControl == TxOnly ||
	    adapter->FlowControl == Both) {
		status &= ~INT_MASK_ENABLE;
	} else {
		status &= ~INT_MASK_ENABLE_NO_FLOW;
	}

	
	if (!status) {
		handled = false;
		et131x_enable_interrupts(adapter);
		goto out;
	}

	

	if (status & ET_INTR_WATCHDOG) {
		PMP_TCB pMpTcb = adapter->TxRing.CurrSendHead;

		if (pMpTcb)
			if (++pMpTcb->PacketStaleCount > 1)
				status |= ET_INTR_TXDMA_ISR;

		if (adapter->RxRing.UnfinishedReceives)
			status |= ET_INTR_RXDMA_XFR_DONE;
		else if (pMpTcb == NULL)
			writel(0, &adapter->regs->global.watchdog_timer);

		status &= ~ET_INTR_WATCHDOG;
	}

	if (status == 0) {
		
		et131x_enable_interrupts(adapter);
		goto out;
	}

	
	adapter->Stats.InterruptStatus = status;

	
	schedule_work(&adapter->task);
out:
	return IRQ_RETVAL(handled);
}


void et131x_isr_handler(struct work_struct *work)
{
	struct et131x_adapter *etdev =
		container_of(work, struct et131x_adapter, task);
	u32 status = etdev->Stats.InterruptStatus;
	ADDRESS_MAP_t __iomem *iomem = etdev->regs;

	
	
	if (status & ET_INTR_TXDMA_ISR) {
		et131x_handle_send_interrupt(etdev);
	}

	
	if (status & ET_INTR_RXDMA_XFR_DONE) {
		et131x_handle_recv_interrupt(etdev);
	}

	status &= 0xffffffd7;

	if (status) {
		
		if (status & ET_INTR_TXDMA_ERR) {
			u32 txdma_err;

			
			txdma_err = readl(&iomem->txdma.TxDmaError);

			dev_warn(&etdev->pdev->dev,
				    "TXDMA_ERR interrupt, error = %d\n",
				    txdma_err);
		}

		
		if (status & (ET_INTR_RXDMA_FB_R0_LOW | ET_INTR_RXDMA_FB_R1_LOW)) {
			

			
			if (etdev->FlowControl == TxOnly ||
			    etdev->FlowControl == Both) {
				u32 pm_csr;

				
				pm_csr = readl(&iomem->global.pm_csr);
				if ((pm_csr & ET_PM_PHY_SW_COMA) == 0) {
					TXMAC_BP_CTRL_t bp_ctrl = { 0 };

					bp_ctrl.bits.bp_req = 1;
					bp_ctrl.bits.bp_xonxoff = 1;
					writel(bp_ctrl.value,
					       &iomem->txmac.bp_ctrl.value);
				}
			}
		}

		
		if (status & ET_INTR_RXDMA_STAT_LOW) {

			
		}

		
		if (status & ET_INTR_RXDMA_ERR) {
			
			

			etdev->TxMacTest.value =
				readl(&iomem->txmac.tx_test.value);
			dev_warn(&etdev->pdev->dev,
				    "RxDMA_ERR interrupt, error %x\n",
				    etdev->TxMacTest.value);
		}

		
		if (status & ET_INTR_WOL) {
			
			dev_err(&etdev->pdev->dev, "WAKE_ON_LAN interrupt\n");
		}

		
		if (status & ET_INTR_PHY) {
			u32 pm_csr;
			MI_BMSR_t BmsrInts, BmsrData;
			MI_ISR_t myIsr;

			
			pm_csr = readl(&iomem->global.pm_csr);
			if (pm_csr & ET_PM_PHY_SW_COMA) {
				
				DisablePhyComa(etdev);
			}

			
			MiRead(etdev, (uint8_t) offsetof(MI_REGS_t, isr),
			       &myIsr.value);

			if (!etdev->ReplicaPhyLoopbk) {
				MiRead(etdev,
				       (uint8_t) offsetof(MI_REGS_t, bmsr),
				       &BmsrData.value);

				BmsrInts.value =
				    etdev->Bmsr.value ^ BmsrData.value;
				etdev->Bmsr.value = BmsrData.value;

				
				et131x_Mii_check(etdev, BmsrData, BmsrInts);
			}
		}

		
		if (status & ET_INTR_TXMAC) {
			etdev->TxRing.TxMacErr.value =
				readl(&iomem->txmac.err.value);

			
			dev_warn(&etdev->pdev->dev,
				    "TXMAC interrupt, error 0x%08x\n",
				    etdev->TxRing.TxMacErr.value);

			
		}

		
		if (status & ET_INTR_RXMAC) {
			
			

			dev_warn(&etdev->pdev->dev,
			  "RXMAC interrupt, error 0x%08x.  Requesting reset\n",
				    readl(&iomem->rxmac.err_reg.value));

			dev_warn(&etdev->pdev->dev,
				    "Enable 0x%08x, Diag 0x%08x\n",
				    readl(&iomem->rxmac.ctrl.value),
				    readl(&iomem->rxmac.rxq_diag.value));

			
		}

		
		if (status & ET_INTR_MAC_STAT) {
			
			HandleMacStatInterrupt(etdev);
		}

		
		if (status & ET_INTR_SLV_TIMEOUT) {
			
		}
	}
	et131x_enable_interrupts(etdev);
}
