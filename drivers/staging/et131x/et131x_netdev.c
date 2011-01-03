

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

#include <linux/mii.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/ioport.h>

#include "et1310_phy.h"
#include "et1310_pm.h"
#include "et1310_jagcore.h"
#include "et1310_mac.h"
#include "et1310_tx.h"

#include "et131x_adapter.h"
#include "et131x_isr.h"
#include "et131x_initpci.h"

struct net_device_stats *et131x_stats(struct net_device *netdev);
int et131x_open(struct net_device *netdev);
int et131x_close(struct net_device *netdev);
int et131x_ioctl(struct net_device *netdev, struct ifreq *reqbuf, int cmd);
void et131x_multicast(struct net_device *netdev);
int et131x_tx(struct sk_buff *skb, struct net_device *netdev);
void et131x_tx_timeout(struct net_device *netdev);
int et131x_change_mtu(struct net_device *netdev, int new_mtu);
int et131x_set_mac_addr(struct net_device *netdev, void *new_mac);
void et131x_vlan_rx_register(struct net_device *netdev, struct vlan_group *grp);
void et131x_vlan_rx_add_vid(struct net_device *netdev, uint16_t vid);
void et131x_vlan_rx_kill_vid(struct net_device *netdev, uint16_t vid);

static const struct net_device_ops et131x_netdev_ops = {
	.ndo_open		= et131x_open,
	.ndo_stop		= et131x_close,
	.ndo_start_xmit		= et131x_tx,
	.ndo_set_multicast_list	= et131x_multicast,
	.ndo_tx_timeout		= et131x_tx_timeout,
	.ndo_change_mtu		= et131x_change_mtu,
	.ndo_set_mac_address	= et131x_set_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_get_stats		= et131x_stats,
	.ndo_do_ioctl		= et131x_ioctl,
};


struct net_device *et131x_device_alloc(void)
{
	struct net_device *netdev;

	
	netdev = alloc_etherdev(sizeof(struct et131x_adapter));

	if (netdev == NULL) {
		printk(KERN_ERR "et131x: Alloc of net_device struct failed\n");
		return NULL;
	}

	
	
	
	netdev->watchdog_timeo = ET131X_TX_TIMEOUT;
	netdev->netdev_ops = &et131x_netdev_ops;

	

	
	
	
	return netdev;
}


struct net_device_stats *et131x_stats(struct net_device *netdev)
{
	struct et131x_adapter *adapter = netdev_priv(netdev);
	struct net_device_stats *stats = &adapter->net_stats;
	CE_STATS_t *devstat = &adapter->Stats;

	stats->rx_packets = devstat->ipackets;
	stats->tx_packets = devstat->opackets;
	stats->rx_errors = devstat->length_err + devstat->alignment_err +
	    devstat->crc_err + devstat->code_violations + devstat->other_errors;
	stats->tx_errors = devstat->max_pkt_error;
	stats->multicast = devstat->multircv;
	stats->collisions = devstat->collisions;

	stats->rx_length_errors = devstat->length_err;
	stats->rx_over_errors = devstat->rx_ov_flow;
	stats->rx_crc_errors = devstat->crc_err;

	
	
	
	
	

	
	
	
	

	
	
	
	
	
	return stats;
}


int et131x_open(struct net_device *netdev)
{
	int result = 0;
	struct et131x_adapter *adapter = netdev_priv(netdev);

	
	add_timer(&adapter->ErrorTimer);

	
	result = request_irq(netdev->irq, et131x_isr, IRQF_SHARED,
					netdev->name, netdev);
	if (result) {
		dev_err(&adapter->pdev->dev, "c ould not register IRQ %d\n",
			netdev->irq);
		return result;
	}

	
	et131x_rx_dma_enable(adapter);
	et131x_tx_dma_enable(adapter);

	
	et131x_enable_interrupts(adapter);

	adapter->Flags |= fMP_ADAPTER_INTERRUPT_IN_USE;

	
	netif_start_queue(netdev);
	return result;
}


int et131x_close(struct net_device *netdev)
{
	struct et131x_adapter *adapter = netdev_priv(netdev);

	
	netif_stop_queue(netdev);

	
	et131x_rx_dma_disable(adapter);
	et131x_tx_dma_disable(adapter);

	
	et131x_disable_interrupts(adapter);

	
	adapter->Flags &= ~fMP_ADAPTER_INTERRUPT_IN_USE;
	free_irq(netdev->irq, netdev);

	
	del_timer_sync(&adapter->ErrorTimer);
	return 0;
}


int et131x_ioctl_mii(struct net_device *netdev, struct ifreq *reqbuf, int cmd)
{
	int status = 0;
	struct et131x_adapter *etdev = netdev_priv(netdev);
	struct mii_ioctl_data *data = if_mii(reqbuf);

	switch (cmd) {
	case SIOCGMIIPHY:
		data->phy_id = etdev->Stats.xcvr_addr;
		break;

	case SIOCGMIIREG:
		if (!capable(CAP_NET_ADMIN))
			status = -EPERM;
		else
			status = MiRead(etdev,
					data->reg_num, &data->val_out);
		break;

	case SIOCSMIIREG:
		if (!capable(CAP_NET_ADMIN))
			status = -EPERM;
		else
			status = MiWrite(etdev, data->reg_num,
					 data->val_in);
		break;

	default:
		status = -EOPNOTSUPP;
	}
	return status;
}


int et131x_ioctl(struct net_device *netdev, struct ifreq *reqbuf, int cmd)
{
	int status = 0;

	switch (cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		status = et131x_ioctl_mii(netdev, reqbuf, cmd);
		break;

	default:
		status = -EOPNOTSUPP;
	}
	return status;
}


int et131x_set_packet_filter(struct et131x_adapter *adapter)
{
	int status = 0;
	uint32_t filter = adapter->PacketFilter;
	RXMAC_CTRL_t ctrl;
	RXMAC_PF_CTRL_t pf_ctrl;

	ctrl.value = readl(&adapter->regs->rxmac.ctrl.value);
	pf_ctrl.value = readl(&adapter->regs->rxmac.pf_ctrl.value);

	
	ctrl.bits.pkt_filter_disable = 1;

	
	if ((filter & ET131X_PACKET_TYPE_PROMISCUOUS) || filter == 0) {
		pf_ctrl.bits.filter_broad_en = 0;
		pf_ctrl.bits.filter_multi_en = 0;
		pf_ctrl.bits.filter_uni_en = 0;
	} else {
		
		if (filter & ET131X_PACKET_TYPE_ALL_MULTICAST) {
			pf_ctrl.bits.filter_multi_en = 0;
		} else {
			SetupDeviceForMulticast(adapter);
			pf_ctrl.bits.filter_multi_en = 1;
			ctrl.bits.pkt_filter_disable = 0;
		}

		
		if (filter & ET131X_PACKET_TYPE_DIRECTED) {
			SetupDeviceForUnicast(adapter);
			pf_ctrl.bits.filter_uni_en = 1;
			ctrl.bits.pkt_filter_disable = 0;
		}

		
		if (filter & ET131X_PACKET_TYPE_BROADCAST) {
			pf_ctrl.bits.filter_broad_en = 1;
			ctrl.bits.pkt_filter_disable = 0;
		} else {
			pf_ctrl.bits.filter_broad_en = 0;
		}

		
		writel(pf_ctrl.value,
		       &adapter->regs->rxmac.pf_ctrl.value);
		writel(ctrl.value, &adapter->regs->rxmac.ctrl.value);
	}
	return status;
}


void et131x_multicast(struct net_device *netdev)
{
	struct et131x_adapter *adapter = netdev_priv(netdev);
	uint32_t PacketFilter = 0;
	uint32_t count;
	unsigned long flags;
	struct dev_mc_list *mclist = netdev->mc_list;

	spin_lock_irqsave(&adapter->Lock, flags);

	
	PacketFilter = adapter->PacketFilter;

	
	PacketFilter &= ~ET131X_PACKET_TYPE_MULTICAST;

	

	if (netdev->flags & IFF_PROMISC) {
		adapter->PacketFilter |= ET131X_PACKET_TYPE_PROMISCUOUS;
	} else {
		adapter->PacketFilter &= ~ET131X_PACKET_TYPE_PROMISCUOUS;
	}

	if (netdev->flags & IFF_ALLMULTI) {
		adapter->PacketFilter |= ET131X_PACKET_TYPE_ALL_MULTICAST;
	}

	if (netdev->mc_count > NIC_MAX_MCAST_LIST) {
		adapter->PacketFilter |= ET131X_PACKET_TYPE_ALL_MULTICAST;
	}

	if (netdev->mc_count < 1) {
		adapter->PacketFilter &= ~ET131X_PACKET_TYPE_ALL_MULTICAST;
		adapter->PacketFilter &= ~ET131X_PACKET_TYPE_MULTICAST;
	} else {
		adapter->PacketFilter |= ET131X_PACKET_TYPE_MULTICAST;
	}

	
	adapter->MCAddressCount = netdev->mc_count;

	if (netdev->mc_count) {
		count = netdev->mc_count - 1;
		memcpy(adapter->MCList[count], mclist->dmi_addr, ETH_ALEN);
	}

	
	if (PacketFilter != adapter->PacketFilter) {
		
		et131x_set_packet_filter(adapter);
	}
	spin_unlock_irqrestore(&adapter->Lock, flags);
}


int et131x_tx(struct sk_buff *skb, struct net_device *netdev)
{
	int status = 0;

	
	netdev->trans_start = jiffies;

	
	status = et131x_send_packets(skb, netdev);

	
	if (status != 0) {
		if (status == -ENOMEM) {
			
			netif_stop_queue(netdev);
			status = NETDEV_TX_BUSY;
		} else {
			status = NETDEV_TX_OK;
		}
	}
	return status;
}


void et131x_tx_timeout(struct net_device *netdev)
{
	struct et131x_adapter *etdev = netdev_priv(netdev);
	PMP_TCB pMpTcb;
	unsigned long flags;

	
	if (etdev->Flags & fMP_ADAPTER_LINK_DETECTION)
		return;

	
	if (etdev->Flags & fMP_ADAPTER_NON_RECOVER_ERROR)
		return;

	
	if (etdev->Flags & fMP_ADAPTER_HARDWARE_ERROR) {
		dev_err(&etdev->pdev->dev, "hardware error - reset\n");
		return;
	}

	
	spin_lock_irqsave(&etdev->TCBSendQLock, flags);

	pMpTcb = etdev->TxRing.CurrSendHead;

	if (pMpTcb != NULL) {
		pMpTcb->Count++;

		if (pMpTcb->Count > NIC_SEND_HANG_THRESHOLD) {
			TX_DESC_ENTRY_t StuckDescriptors[10];

			if (INDEX10(pMpTcb->WrIndex) > 7) {
				memcpy(StuckDescriptors,
				       etdev->TxRing.pTxDescRingVa +
				       INDEX10(pMpTcb->WrIndex) - 6,
				       sizeof(TX_DESC_ENTRY_t) * 10);
			}

			spin_unlock_irqrestore(&etdev->TCBSendQLock,
					       flags);

			dev_warn(&etdev->pdev->dev,
				"Send stuck - reset.  pMpTcb->WrIndex %x, Flags 0x%08x\n",
				pMpTcb->WrIndex,
				pMpTcb->Flags);

			et131x_close(netdev);
			et131x_open(netdev);

			return;
		}
	}

	spin_unlock_irqrestore(&etdev->TCBSendQLock, flags);
}


int et131x_change_mtu(struct net_device *netdev, int new_mtu)
{
	int result = 0;
	struct et131x_adapter *adapter = netdev_priv(netdev);

	
	if (new_mtu < 64 || new_mtu > 9216)
		return -EINVAL;

	
	netif_stop_queue(netdev);

	
	et131x_rx_dma_disable(adapter);
	et131x_tx_dma_disable(adapter);

	
	et131x_disable_interrupts(adapter);
	et131x_handle_send_interrupt(adapter);
	et131x_handle_recv_interrupt(adapter);

	
	netdev->mtu = new_mtu;

	
	et131x_adapter_memory_free(adapter);

	
	adapter->RegistryJumboPacket = new_mtu + 14;
	et131x_soft_reset(adapter);

	
	result = et131x_adapter_memory_alloc(adapter);
	if (result != 0) {
		dev_warn(&adapter->pdev->dev,
			"Change MTU failed; couldn't re-alloc DMA memory\n");
		return result;
	}

	et131x_init_send(adapter);

	et131x_setup_hardware_properties(adapter);
	memcpy(netdev->dev_addr, adapter->CurrentAddress, ETH_ALEN);

	
	et131x_adapter_setup(adapter);

	
	if (adapter->Flags & fMP_ADAPTER_INTERRUPT_IN_USE)
		et131x_enable_interrupts(adapter);

	
	et131x_rx_dma_enable(adapter);
	et131x_tx_dma_enable(adapter);

	
	netif_wake_queue(netdev);
	return result;
}


int et131x_set_mac_addr(struct net_device *netdev, void *new_mac)
{
	int result = 0;
	struct et131x_adapter *adapter = netdev_priv(netdev);
	struct sockaddr *address = new_mac;

	

	if (adapter == NULL)
		return -ENODEV;

	
	if (!is_valid_ether_addr(address->sa_data))
		return -EINVAL;

	
	netif_stop_queue(netdev);

	
	et131x_rx_dma_disable(adapter);
	et131x_tx_dma_disable(adapter);

	
	et131x_disable_interrupts(adapter);
	et131x_handle_send_interrupt(adapter);
	et131x_handle_recv_interrupt(adapter);

	
	
	

	memcpy(netdev->dev_addr, address->sa_data, netdev->addr_len);

	printk(KERN_INFO
		"%s: Setting MAC address to %02x:%02x:%02x:%02x:%02x:%02x\n",
			netdev->name,
			netdev->dev_addr[0], netdev->dev_addr[1],
			netdev->dev_addr[2], netdev->dev_addr[3],
			netdev->dev_addr[4], netdev->dev_addr[5]);

	
	et131x_adapter_memory_free(adapter);

	
	
	

	et131x_soft_reset(adapter);

	
	result = et131x_adapter_memory_alloc(adapter);
	if (result != 0) {
		dev_err(&adapter->pdev->dev,
			"Change MAC failed; couldn't re-alloc DMA memory\n");
		return result;
	}

	et131x_init_send(adapter);

	et131x_setup_hardware_properties(adapter);
	
	

	
	et131x_adapter_setup(adapter);

	
	if (adapter->Flags & fMP_ADAPTER_INTERRUPT_IN_USE)
		et131x_enable_interrupts(adapter);

	
	et131x_rx_dma_enable(adapter);
	et131x_tx_dma_enable(adapter);

	
	netif_wake_queue(netdev);
	return result;
}
