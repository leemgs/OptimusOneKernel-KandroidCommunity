

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

#include "et1310_phy.h"
#include "et1310_pm.h"
#include "et1310_jagcore.h"

#include "et131x_adapter.h"
#include "et131x_initpci.h"
#include "et131x_isr.h"

#include "et1310_tx.h"


static void et131x_update_tcb_list(struct et131x_adapter *etdev);
static void et131x_check_send_wait_list(struct et131x_adapter *etdev);
static inline void et131x_free_send_packet(struct et131x_adapter *etdev,
					   PMP_TCB pMpTcb);
static int et131x_send_packet(struct sk_buff *skb,
			      struct et131x_adapter *etdev);
static int nic_send_packet(struct et131x_adapter *etdev, PMP_TCB pMpTcb);


int et131x_tx_dma_memory_alloc(struct et131x_adapter *adapter)
{
	int desc_size = 0;
	TX_RING_t *tx_ring = &adapter->TxRing;

	
	adapter->TxRing.MpTcbMem = (MP_TCB *)kcalloc(NUM_TCB, sizeof(MP_TCB),
						      GFP_ATOMIC | GFP_DMA);
	if (!adapter->TxRing.MpTcbMem) {
		dev_err(&adapter->pdev->dev, "Cannot alloc memory for TCBs\n");
		return -ENOMEM;
	}

	
	desc_size = (sizeof(TX_DESC_ENTRY_t) * NUM_DESC_PER_RING_TX) + 4096 - 1;
	tx_ring->pTxDescRingVa =
	    (PTX_DESC_ENTRY_t) pci_alloc_consistent(adapter->pdev, desc_size,
						    &tx_ring->pTxDescRingPa);
	if (!adapter->TxRing.pTxDescRingVa) {
		dev_err(&adapter->pdev->dev, "Cannot alloc memory for Tx Ring\n");
		return -ENOMEM;
	}

	
	tx_ring->pTxDescRingAdjustedPa = tx_ring->pTxDescRingPa;

	
	et131x_align_allocated_memory(adapter,
				      &tx_ring->pTxDescRingAdjustedPa,
				      &tx_ring->TxDescOffset, 0x0FFF);

	tx_ring->pTxDescRingVa += tx_ring->TxDescOffset;

	
	tx_ring->pTxStatusVa = pci_alloc_consistent(adapter->pdev,
						    sizeof(TX_STATUS_BLOCK_t),
						    &tx_ring->pTxStatusPa);
	if (!adapter->TxRing.pTxStatusPa) {
		dev_err(&adapter->pdev->dev,
				  "Cannot alloc memory for Tx status block\n");
		return -ENOMEM;
	}

	
	tx_ring->pTxDummyBlkVa = pci_alloc_consistent(adapter->pdev,
						      NIC_MIN_PACKET_SIZE,
						      &tx_ring->pTxDummyBlkPa);
	if (!adapter->TxRing.pTxDummyBlkPa) {
		dev_err(&adapter->pdev->dev,
			"Cannot alloc memory for Tx dummy buffer\n");
		return -ENOMEM;
	}

	return 0;
}


void et131x_tx_dma_memory_free(struct et131x_adapter *adapter)
{
	int desc_size = 0;

	if (adapter->TxRing.pTxDescRingVa) {
		
		adapter->TxRing.pTxDescRingVa -= adapter->TxRing.TxDescOffset;

		desc_size =
		    (sizeof(TX_DESC_ENTRY_t) * NUM_DESC_PER_RING_TX) + 4096 - 1;

		pci_free_consistent(adapter->pdev,
				    desc_size,
				    adapter->TxRing.pTxDescRingVa,
				    adapter->TxRing.pTxDescRingPa);

		adapter->TxRing.pTxDescRingVa = NULL;
	}

	
	if (adapter->TxRing.pTxStatusVa) {
		pci_free_consistent(adapter->pdev,
				    sizeof(TX_STATUS_BLOCK_t),
				    adapter->TxRing.pTxStatusVa,
				    adapter->TxRing.pTxStatusPa);

		adapter->TxRing.pTxStatusVa = NULL;
	}

	
	if (adapter->TxRing.pTxDummyBlkVa) {
		pci_free_consistent(adapter->pdev,
				    NIC_MIN_PACKET_SIZE,
				    adapter->TxRing.pTxDummyBlkVa,
				    adapter->TxRing.pTxDummyBlkPa);

		adapter->TxRing.pTxDummyBlkVa = NULL;
	}

	
	kfree(adapter->TxRing.MpTcbMem);
}


void ConfigTxDmaRegs(struct et131x_adapter *etdev)
{
	struct _TXDMA_t __iomem *txdma = &etdev->regs->txdma;

	
	writel((uint32_t) (etdev->TxRing.pTxDescRingAdjustedPa >> 32),
	       &txdma->pr_base_hi);
	writel((uint32_t) etdev->TxRing.pTxDescRingAdjustedPa,
	       &txdma->pr_base_lo);

	
	writel(NUM_DESC_PER_RING_TX - 1, &txdma->pr_num_des.value);

	
	writel(0, &txdma->dma_wb_base_hi);
	writel(etdev->TxRing.pTxStatusPa, &txdma->dma_wb_base_lo);

	memset(etdev->TxRing.pTxStatusVa, 0, sizeof(TX_STATUS_BLOCK_t));

	writel(0, &txdma->service_request);
	etdev->TxRing.txDmaReadyToSend = 0;
}


void et131x_tx_dma_disable(struct et131x_adapter *etdev)
{
	
	writel(ET_TXDMA_CSR_HALT|ET_TXDMA_SNGL_EPKT,
					&etdev->regs->txdma.csr);
}


void et131x_tx_dma_enable(struct et131x_adapter *etdev)
{
	u32 csr = ET_TXDMA_SNGL_EPKT;
	if (etdev->RegistryPhyLoopbk)
		
		csr |= ET_TXDMA_CSR_HALT;
	else
		
		csr |= PARM_DMA_CACHE_DEF << ET_TXDMA_CACHE_SHIFT;
	writel(csr, &etdev->regs->txdma.csr);
}


void et131x_init_send(struct et131x_adapter *adapter)
{
	PMP_TCB pMpTcb;
	uint32_t TcbCount;
	TX_RING_t *tx_ring;

	
	tx_ring = &adapter->TxRing;
	pMpTcb = adapter->TxRing.MpTcbMem;

	tx_ring->TCBReadyQueueHead = pMpTcb;

	
	for (TcbCount = 0; TcbCount < NUM_TCB; TcbCount++) {
		memset(pMpTcb, 0, sizeof(MP_TCB));

		
		if (TcbCount < NUM_TCB - 1) {
			pMpTcb->Next = pMpTcb + 1;
		} else {
			tx_ring->TCBReadyQueueTail = pMpTcb;
			pMpTcb->Next = (PMP_TCB) NULL;
		}

		pMpTcb++;
	}

	
	tx_ring->CurrSendHead = (PMP_TCB) NULL;
	tx_ring->CurrSendTail = (PMP_TCB) NULL;

	INIT_LIST_HEAD(&adapter->TxRing.SendWaitQueue);
}


int et131x_send_packets(struct sk_buff *skb, struct net_device *netdev)
{
	int status = 0;
	struct et131x_adapter *etdev = NULL;

	etdev = netdev_priv(netdev);

	

	
	if (!list_empty(&etdev->TxRing.SendWaitQueue) ||
	    MP_TCB_RESOURCES_NOT_AVAILABLE(etdev)) {
		
		status = -ENOMEM;
	} else {
		
		
		if (MP_SHOULD_FAIL_SEND(etdev) || etdev->DriverNoPhyAccess
		    || !netif_carrier_ok(netdev)) {
			dev_kfree_skb_any(skb);
			skb = NULL;

			etdev->net_stats.tx_dropped++;
		} else {
			status = et131x_send_packet(skb, etdev);

			if (status == -ENOMEM) {

				
			} else if (status != 0) {
				
				dev_kfree_skb_any(skb);
				skb = NULL;
				etdev->net_stats.tx_dropped++;
			}
		}
	}
	return status;
}


static int et131x_send_packet(struct sk_buff *skb,
			      struct et131x_adapter *etdev)
{
	int status = 0;
	PMP_TCB pMpTcb = NULL;
	uint16_t *shbufva;
	unsigned long flags;

	
	if (skb->len < ETH_HLEN) {
		return -EIO;
	}

	
	spin_lock_irqsave(&etdev->TCBReadyQLock, flags);

	pMpTcb = etdev->TxRing.TCBReadyQueueHead;

	if (pMpTcb == NULL) {
		spin_unlock_irqrestore(&etdev->TCBReadyQLock, flags);
		return -ENOMEM;
	}

	etdev->TxRing.TCBReadyQueueHead = pMpTcb->Next;

	if (etdev->TxRing.TCBReadyQueueHead == NULL)
		etdev->TxRing.TCBReadyQueueTail = NULL;

	spin_unlock_irqrestore(&etdev->TCBReadyQLock, flags);

	pMpTcb->PacketLength = skb->len;
	pMpTcb->Packet = skb;

	if ((skb->data != NULL) && ((skb->len - skb->data_len) >= 6)) {
		shbufva = (uint16_t *) skb->data;

		if ((shbufva[0] == 0xffff) &&
		    (shbufva[1] == 0xffff) && (shbufva[2] == 0xffff)) {
			pMpTcb->Flags |= fMP_DEST_BROAD;
		} else if ((shbufva[0] & 0x3) == 0x0001) {
			pMpTcb->Flags |=  fMP_DEST_MULTI;
		}
	}

	pMpTcb->Next = NULL;

	
	if (status == 0)
		status = nic_send_packet(etdev, pMpTcb);

	if (status != 0) {
		spin_lock_irqsave(&etdev->TCBReadyQLock, flags);

		if (etdev->TxRing.TCBReadyQueueTail) {
			etdev->TxRing.TCBReadyQueueTail->Next = pMpTcb;
		} else {
			
			etdev->TxRing.TCBReadyQueueHead = pMpTcb;
		}

		etdev->TxRing.TCBReadyQueueTail = pMpTcb;
		spin_unlock_irqrestore(&etdev->TCBReadyQLock, flags);
		return status;
	}
	WARN_ON(etdev->TxRing.nBusySend > NUM_TCB);
	return 0;
}


static int nic_send_packet(struct et131x_adapter *etdev, PMP_TCB pMpTcb)
{
	uint32_t loopIndex;
	TX_DESC_ENTRY_t CurDesc[24];
	uint32_t FragmentNumber = 0;
	uint32_t thiscopy, remainder;
	struct sk_buff *pPacket = pMpTcb->Packet;
	uint32_t FragListCount = skb_shinfo(pPacket)->nr_frags + 1;
	struct skb_frag_struct *pFragList = &skb_shinfo(pPacket)->frags[0];
	unsigned long flags;

	
	if (FragListCount > 23) {
		return -EIO;
	}

	memset(CurDesc, 0, sizeof(TX_DESC_ENTRY_t) * (FragListCount + 1));

	for (loopIndex = 0; loopIndex < FragListCount; loopIndex++) {
		
		if (loopIndex == 0) {
			
			if ((pPacket->len - pPacket->data_len) <= 1514) {
				CurDesc[FragmentNumber].DataBufferPtrHigh = 0;
				CurDesc[FragmentNumber].word2.bits.
				    length_in_bytes =
				    pPacket->len - pPacket->data_len;

				
				CurDesc[FragmentNumber++].DataBufferPtrLow =
				    pci_map_single(etdev->pdev,
						   pPacket->data,
						   pPacket->len -
						   pPacket->data_len,
						   PCI_DMA_TODEVICE);
			} else {
				CurDesc[FragmentNumber].DataBufferPtrHigh = 0;
				CurDesc[FragmentNumber].word2.bits.
				    length_in_bytes =
				    ((pPacket->len - pPacket->data_len) / 2);

				
				CurDesc[FragmentNumber++].DataBufferPtrLow =
				    pci_map_single(etdev->pdev,
						   pPacket->data,
						   ((pPacket->len -
						     pPacket->data_len) / 2),
						   PCI_DMA_TODEVICE);
				CurDesc[FragmentNumber].DataBufferPtrHigh = 0;

				CurDesc[FragmentNumber].word2.bits.
				    length_in_bytes =
				    ((pPacket->len - pPacket->data_len) / 2);

				
				CurDesc[FragmentNumber++].DataBufferPtrLow =
				    pci_map_single(etdev->pdev,
						   pPacket->data +
						   ((pPacket->len -
						     pPacket->data_len) / 2),
						   ((pPacket->len -
						     pPacket->data_len) / 2),
						   PCI_DMA_TODEVICE);
			}
		} else {
			CurDesc[FragmentNumber].DataBufferPtrHigh = 0;
			CurDesc[FragmentNumber].word2.bits.length_in_bytes =
			    pFragList[loopIndex - 1].size;

			
			CurDesc[FragmentNumber++].DataBufferPtrLow =
			    pci_map_page(etdev->pdev,
					 pFragList[loopIndex - 1].page,
					 pFragList[loopIndex - 1].page_offset,
					 pFragList[loopIndex - 1].size,
					 PCI_DMA_TODEVICE);
		}
	}

	if (FragmentNumber == 0)
		return -EIO;

	if (etdev->linkspeed == TRUEPHY_SPEED_1000MBPS) {
		if (++etdev->TxRing.TxPacketsSinceLastinterrupt ==
		    PARM_TX_NUM_BUFS_DEF) {
			CurDesc[FragmentNumber - 1].word3.value = 0x5;
			etdev->TxRing.TxPacketsSinceLastinterrupt = 0;
		} else {
			CurDesc[FragmentNumber - 1].word3.value = 0x1;
		}
	} else {
		CurDesc[FragmentNumber - 1].word3.value = 0x5;
	}

	CurDesc[0].word3.bits.f = 1;

	pMpTcb->WrIndexStart = etdev->TxRing.txDmaReadyToSend;
	pMpTcb->PacketStaleCount = 0;

	spin_lock_irqsave(&etdev->SendHWLock, flags);

	thiscopy = NUM_DESC_PER_RING_TX -
				INDEX10(etdev->TxRing.txDmaReadyToSend);

	if (thiscopy >= FragmentNumber) {
		remainder = 0;
		thiscopy = FragmentNumber;
	} else {
		remainder = FragmentNumber - thiscopy;
	}

	memcpy(etdev->TxRing.pTxDescRingVa +
	       INDEX10(etdev->TxRing.txDmaReadyToSend), CurDesc,
	       sizeof(TX_DESC_ENTRY_t) * thiscopy);

	add_10bit(&etdev->TxRing.txDmaReadyToSend, thiscopy);

	if (INDEX10(etdev->TxRing.txDmaReadyToSend)== 0 ||
	    INDEX10(etdev->TxRing.txDmaReadyToSend) == NUM_DESC_PER_RING_TX) {
	     	etdev->TxRing.txDmaReadyToSend &= ~ET_DMA10_MASK;
	     	etdev->TxRing.txDmaReadyToSend ^= ET_DMA10_WRAP;
	}

	if (remainder) {
		memcpy(etdev->TxRing.pTxDescRingVa,
		       CurDesc + thiscopy,
		       sizeof(TX_DESC_ENTRY_t) * remainder);

		add_10bit(&etdev->TxRing.txDmaReadyToSend, remainder);
	}

	if (INDEX10(etdev->TxRing.txDmaReadyToSend) == 0) {
		if (etdev->TxRing.txDmaReadyToSend)
			pMpTcb->WrIndex = NUM_DESC_PER_RING_TX - 1;
		else
			pMpTcb->WrIndex= ET_DMA10_WRAP | (NUM_DESC_PER_RING_TX - 1);
	} else
		pMpTcb->WrIndex = etdev->TxRing.txDmaReadyToSend - 1;

	spin_lock(&etdev->TCBSendQLock);

	if (etdev->TxRing.CurrSendTail)
		etdev->TxRing.CurrSendTail->Next = pMpTcb;
	else
		etdev->TxRing.CurrSendHead = pMpTcb;

	etdev->TxRing.CurrSendTail = pMpTcb;

	WARN_ON(pMpTcb->Next != NULL);

	etdev->TxRing.nBusySend++;

	spin_unlock(&etdev->TCBSendQLock);

	
	writel(etdev->TxRing.txDmaReadyToSend,
	       &etdev->regs->txdma.service_request);

	
	if (etdev->linkspeed == TRUEPHY_SPEED_1000MBPS) {
		writel(PARM_TX_TIME_INT_DEF * NANO_IN_A_MICRO,
		       &etdev->regs->global.watchdog_timer);
	}
	spin_unlock_irqrestore(&etdev->SendHWLock, flags);

	return 0;
}



inline void et131x_free_send_packet(struct et131x_adapter *etdev,
							PMP_TCB pMpTcb)
{
	unsigned long flags;
	TX_DESC_ENTRY_t *desc = NULL;
	struct net_device_stats *stats = &etdev->net_stats;

	if (pMpTcb->Flags & fMP_DEST_BROAD)
		atomic_inc(&etdev->Stats.brdcstxmt);
	else if (pMpTcb->Flags & fMP_DEST_MULTI)
		atomic_inc(&etdev->Stats.multixmt);
	else
		atomic_inc(&etdev->Stats.unixmt);

	if (pMpTcb->Packet) {
		stats->tx_bytes += pMpTcb->Packet->len;

		
		do {
			desc =
			    (TX_DESC_ENTRY_t *) (etdev->TxRing.pTxDescRingVa +
			    	INDEX10(pMpTcb->WrIndexStart));

			pci_unmap_single(etdev->pdev,
					 desc->DataBufferPtrLow,
					 desc->word2.value, PCI_DMA_TODEVICE);

			add_10bit(&pMpTcb->WrIndexStart, 1);
			if (INDEX10(pMpTcb->WrIndexStart) >=
			    NUM_DESC_PER_RING_TX) {
			    	pMpTcb->WrIndexStart &= ~ET_DMA10_MASK;
			    	pMpTcb->WrIndexStart ^= ET_DMA10_WRAP;
			}
		} while (desc != (etdev->TxRing.pTxDescRingVa +
				INDEX10(pMpTcb->WrIndex)));

		dev_kfree_skb_any(pMpTcb->Packet);
	}

	memset(pMpTcb, 0, sizeof(MP_TCB));

	
	spin_lock_irqsave(&etdev->TCBReadyQLock, flags);

	etdev->Stats.opackets++;

	if (etdev->TxRing.TCBReadyQueueTail) {
		etdev->TxRing.TCBReadyQueueTail->Next = pMpTcb;
	} else {
		
		etdev->TxRing.TCBReadyQueueHead = pMpTcb;
	}

	etdev->TxRing.TCBReadyQueueTail = pMpTcb;

	spin_unlock_irqrestore(&etdev->TCBReadyQLock, flags);
	WARN_ON(etdev->TxRing.nBusySend < 0);
}


void et131x_free_busy_send_packets(struct et131x_adapter *etdev)
{
	PMP_TCB pMpTcb;
	struct list_head *entry;
	unsigned long flags;
	uint32_t FreeCounter = 0;

	while (!list_empty(&etdev->TxRing.SendWaitQueue)) {
		spin_lock_irqsave(&etdev->SendWaitLock, flags);

		etdev->TxRing.nWaitSend--;
		spin_unlock_irqrestore(&etdev->SendWaitLock, flags);

		entry = etdev->TxRing.SendWaitQueue.next;
	}

	etdev->TxRing.nWaitSend = 0;

	
	spin_lock_irqsave(&etdev->TCBSendQLock, flags);

	pMpTcb = etdev->TxRing.CurrSendHead;

	while ((pMpTcb != NULL) && (FreeCounter < NUM_TCB)) {
		PMP_TCB pNext = pMpTcb->Next;

		etdev->TxRing.CurrSendHead = pNext;

		if (pNext == NULL)
			etdev->TxRing.CurrSendTail = NULL;

		etdev->TxRing.nBusySend--;

		spin_unlock_irqrestore(&etdev->TCBSendQLock, flags);

		FreeCounter++;
		et131x_free_send_packet(etdev, pMpTcb);

		spin_lock_irqsave(&etdev->TCBSendQLock, flags);

		pMpTcb = etdev->TxRing.CurrSendHead;
	}

	WARN_ON(FreeCounter == NUM_TCB);

	spin_unlock_irqrestore(&etdev->TCBSendQLock, flags);

	etdev->TxRing.nBusySend = 0;
}


void et131x_handle_send_interrupt(struct et131x_adapter *etdev)
{
	
	et131x_update_tcb_list(etdev);

	
	et131x_check_send_wait_list(etdev);
}


static void et131x_update_tcb_list(struct et131x_adapter *etdev)
{
	unsigned long flags;
	u32 ServiceComplete;
	PMP_TCB pMpTcb;
	u32 index;

	ServiceComplete = readl(&etdev->regs->txdma.NewServiceComplete);
	index = INDEX10(ServiceComplete);

	
	spin_lock_irqsave(&etdev->TCBSendQLock, flags);

	pMpTcb = etdev->TxRing.CurrSendHead;

	while (pMpTcb &&
	       ((ServiceComplete ^ pMpTcb->WrIndex) & ET_DMA10_WRAP) &&
	       index < INDEX10(pMpTcb->WrIndex)) {
		etdev->TxRing.nBusySend--;
		etdev->TxRing.CurrSendHead = pMpTcb->Next;
		if (pMpTcb->Next == NULL)
			etdev->TxRing.CurrSendTail = NULL;

		spin_unlock_irqrestore(&etdev->TCBSendQLock, flags);
		et131x_free_send_packet(etdev, pMpTcb);
		spin_lock_irqsave(&etdev->TCBSendQLock, flags);

		
		pMpTcb = etdev->TxRing.CurrSendHead;
	}
	while (pMpTcb &&
	       !((ServiceComplete ^ pMpTcb->WrIndex) & ET_DMA10_WRAP)
	       && index > (pMpTcb->WrIndex & ET_DMA10_MASK)) {
		etdev->TxRing.nBusySend--;
		etdev->TxRing.CurrSendHead = pMpTcb->Next;
		if (pMpTcb->Next == NULL)
			etdev->TxRing.CurrSendTail = NULL;

		spin_unlock_irqrestore(&etdev->TCBSendQLock, flags);
		et131x_free_send_packet(etdev, pMpTcb);
		spin_lock_irqsave(&etdev->TCBSendQLock, flags);

		
		pMpTcb = etdev->TxRing.CurrSendHead;
	}

	
	if (etdev->TxRing.nBusySend <= (NUM_TCB / 3))
		netif_wake_queue(etdev->netdev);

	spin_unlock_irqrestore(&etdev->TCBSendQLock, flags);
}


static void et131x_check_send_wait_list(struct et131x_adapter *etdev)
{
	unsigned long flags;

	spin_lock_irqsave(&etdev->SendWaitLock, flags);

	while (!list_empty(&etdev->TxRing.SendWaitQueue) &&
				MP_TCB_RESOURCES_AVAILABLE(etdev)) {
		struct list_head *entry;

		entry = etdev->TxRing.SendWaitQueue.next;

		etdev->TxRing.nWaitSend--;
	}

	spin_unlock_irqrestore(&etdev->SendWaitLock, flags);
}
