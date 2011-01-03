

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/in.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>
#include <linux/topology.h>
#include "net_driver.h"
#include "ethtool.h"
#include "tx.h"
#include "rx.h"
#include "efx.h"
#include "mdio_10g.h"
#include "falcon.h"

#define EFX_MAX_MTU (9 * 1024)


static struct workqueue_struct *refill_workqueue;


static struct workqueue_struct *reset_workqueue;




static unsigned int separate_tx_channels;
module_param(separate_tx_channels, uint, 0644);
MODULE_PARM_DESC(separate_tx_channels,
		 "Use separate channels for TX and RX");


static int napi_weight = 64;


unsigned int efx_monitor_interval = 1 * HZ;


static unsigned int allow_bad_hwaddr;


static unsigned int rx_irq_mod_usec = 60;


static unsigned int tx_irq_mod_usec = 150;


static unsigned int interrupt_mode;


static unsigned int rss_cpus;
module_param(rss_cpus, uint, 0444);
MODULE_PARM_DESC(rss_cpus, "Number of CPUs to use for Receive-Side Scaling");

static int phy_flash_cfg;
module_param(phy_flash_cfg, int, 0644);
MODULE_PARM_DESC(phy_flash_cfg, "Set PHYs into reflash mode initially");

static unsigned irq_adapt_low_thresh = 10000;
module_param(irq_adapt_low_thresh, uint, 0644);
MODULE_PARM_DESC(irq_adapt_low_thresh,
		 "Threshold score for reducing IRQ moderation");

static unsigned irq_adapt_high_thresh = 20000;
module_param(irq_adapt_high_thresh, uint, 0644);
MODULE_PARM_DESC(irq_adapt_high_thresh,
		 "Threshold score for increasing IRQ moderation");


static void efx_remove_channel(struct efx_channel *channel);
static void efx_remove_port(struct efx_nic *efx);
static void efx_fini_napi(struct efx_nic *efx);
static void efx_fini_channels(struct efx_nic *efx);

#define EFX_ASSERT_RESET_SERIALISED(efx)		\
	do {						\
		if (efx->state == STATE_RUNNING)	\
			ASSERT_RTNL();			\
	} while (0)




static int efx_process_channel(struct efx_channel *channel, int rx_quota)
{
	struct efx_nic *efx = channel->efx;
	int rx_packets;

	if (unlikely(efx->reset_pending != RESET_TYPE_NONE ||
		     !channel->enabled))
		return 0;

	rx_packets = falcon_process_eventq(channel, rx_quota);
	if (rx_packets == 0)
		return 0;

	
	if (channel->rx_pkt) {
		__efx_rx_packet(channel, channel->rx_pkt,
				channel->rx_pkt_csummed);
		channel->rx_pkt = NULL;
	}

	efx_rx_strategy(channel);

	efx_fast_push_rx_descriptors(&efx->rx_queue[channel->channel]);

	return rx_packets;
}


static inline void efx_channel_processed(struct efx_channel *channel)
{
	
	channel->work_pending = false;
	smp_wmb();

	falcon_eventq_read_ack(channel);
}


static int efx_poll(struct napi_struct *napi, int budget)
{
	struct efx_channel *channel =
		container_of(napi, struct efx_channel, napi_str);
	int rx_packets;

	EFX_TRACE(channel->efx, "channel %d NAPI poll executing on CPU %d\n",
		  channel->channel, raw_smp_processor_id());

	rx_packets = efx_process_channel(channel, budget);

	if (rx_packets < budget) {
		struct efx_nic *efx = channel->efx;

		if (channel->used_flags & EFX_USED_BY_RX &&
		    efx->irq_rx_adaptive &&
		    unlikely(++channel->irq_count == 1000)) {
			unsigned old_irq_moderation = channel->irq_moderation;

			if (unlikely(channel->irq_mod_score <
				     irq_adapt_low_thresh)) {
				channel->irq_moderation =
					max_t(int,
					      channel->irq_moderation -
					      FALCON_IRQ_MOD_RESOLUTION,
					      FALCON_IRQ_MOD_RESOLUTION);
			} else if (unlikely(channel->irq_mod_score >
					    irq_adapt_high_thresh)) {
				channel->irq_moderation =
					min(channel->irq_moderation +
					    FALCON_IRQ_MOD_RESOLUTION,
					    efx->irq_rx_moderation);
			}

			if (channel->irq_moderation != old_irq_moderation)
				falcon_set_int_moderation(channel);

			channel->irq_count = 0;
			channel->irq_mod_score = 0;
		}

		
		napi_complete(napi);
		efx_channel_processed(channel);
	}

	return rx_packets;
}


void efx_process_channel_now(struct efx_channel *channel)
{
	struct efx_nic *efx = channel->efx;

	BUG_ON(!channel->used_flags);
	BUG_ON(!channel->enabled);

	
	falcon_disable_interrupts(efx);
	if (efx->legacy_irq)
		synchronize_irq(efx->legacy_irq);
	if (channel->irq)
		synchronize_irq(channel->irq);

	
	napi_disable(&channel->napi_str);

	
	efx_process_channel(channel, efx->type->evq_size);

	
	efx_channel_processed(channel);

	napi_enable(&channel->napi_str);
	falcon_enable_interrupts(efx);
}


static int efx_probe_eventq(struct efx_channel *channel)
{
	EFX_LOG(channel->efx, "chan %d create event queue\n", channel->channel);

	return falcon_probe_eventq(channel);
}


static void efx_init_eventq(struct efx_channel *channel)
{
	EFX_LOG(channel->efx, "chan %d init event queue\n", channel->channel);

	channel->eventq_read_ptr = 0;

	falcon_init_eventq(channel);
}

static void efx_fini_eventq(struct efx_channel *channel)
{
	EFX_LOG(channel->efx, "chan %d fini event queue\n", channel->channel);

	falcon_fini_eventq(channel);
}

static void efx_remove_eventq(struct efx_channel *channel)
{
	EFX_LOG(channel->efx, "chan %d remove event queue\n", channel->channel);

	falcon_remove_eventq(channel);
}



static int efx_probe_channel(struct efx_channel *channel)
{
	struct efx_tx_queue *tx_queue;
	struct efx_rx_queue *rx_queue;
	int rc;

	EFX_LOG(channel->efx, "creating channel %d\n", channel->channel);

	rc = efx_probe_eventq(channel);
	if (rc)
		goto fail1;

	efx_for_each_channel_tx_queue(tx_queue, channel) {
		rc = efx_probe_tx_queue(tx_queue);
		if (rc)
			goto fail2;
	}

	efx_for_each_channel_rx_queue(rx_queue, channel) {
		rc = efx_probe_rx_queue(rx_queue);
		if (rc)
			goto fail3;
	}

	channel->n_rx_frm_trunc = 0;

	return 0;

 fail3:
	efx_for_each_channel_rx_queue(rx_queue, channel)
		efx_remove_rx_queue(rx_queue);
 fail2:
	efx_for_each_channel_tx_queue(tx_queue, channel)
		efx_remove_tx_queue(tx_queue);
 fail1:
	return rc;
}


static void efx_set_channel_names(struct efx_nic *efx)
{
	struct efx_channel *channel;
	const char *type = "";
	int number;

	efx_for_each_channel(channel, efx) {
		number = channel->channel;
		if (efx->n_channels > efx->n_rx_queues) {
			if (channel->channel < efx->n_rx_queues) {
				type = "-rx";
			} else {
				type = "-tx";
				number -= efx->n_rx_queues;
			}
		}
		snprintf(channel->name, sizeof(channel->name),
			 "%s%s-%d", efx->name, type, number);
	}
}


static void efx_init_channels(struct efx_nic *efx)
{
	struct efx_tx_queue *tx_queue;
	struct efx_rx_queue *rx_queue;
	struct efx_channel *channel;

	
	efx->rx_buffer_len = (max(EFX_PAGE_IP_ALIGN, NET_IP_ALIGN) +
			      EFX_MAX_FRAME_LEN(efx->net_dev->mtu) +
			      efx->type->rx_buffer_padding);
	efx->rx_buffer_order = get_order(efx->rx_buffer_len);

	
	efx_for_each_channel(channel, efx) {
		EFX_LOG(channel->efx, "init chan %d\n", channel->channel);

		efx_init_eventq(channel);

		efx_for_each_channel_tx_queue(tx_queue, channel)
			efx_init_tx_queue(tx_queue);

		
		efx_rx_strategy(channel);

		efx_for_each_channel_rx_queue(rx_queue, channel)
			efx_init_rx_queue(rx_queue);

		WARN_ON(channel->rx_pkt != NULL);
		efx_rx_strategy(channel);
	}
}


static void efx_start_channel(struct efx_channel *channel)
{
	struct efx_rx_queue *rx_queue;

	EFX_LOG(channel->efx, "starting chan %d\n", channel->channel);

	
	channel->work_pending = false;
	channel->enabled = true;
	smp_wmb();

	napi_enable(&channel->napi_str);

	
	efx_for_each_channel_rx_queue(rx_queue, channel)
		efx_fast_push_rx_descriptors(rx_queue);
}


static void efx_stop_channel(struct efx_channel *channel)
{
	struct efx_rx_queue *rx_queue;

	if (!channel->enabled)
		return;

	EFX_LOG(channel->efx, "stop chan %d\n", channel->channel);

	channel->enabled = false;
	napi_disable(&channel->napi_str);

	
	efx_for_each_channel_rx_queue(rx_queue, channel) {
		spin_lock_bh(&rx_queue->add_lock);
		spin_unlock_bh(&rx_queue->add_lock);
	}
}

static void efx_fini_channels(struct efx_nic *efx)
{
	struct efx_channel *channel;
	struct efx_tx_queue *tx_queue;
	struct efx_rx_queue *rx_queue;
	int rc;

	EFX_ASSERT_RESET_SERIALISED(efx);
	BUG_ON(efx->port_enabled);

	rc = falcon_flush_queues(efx);
	if (rc)
		EFX_ERR(efx, "failed to flush queues\n");
	else
		EFX_LOG(efx, "successfully flushed all queues\n");

	efx_for_each_channel(channel, efx) {
		EFX_LOG(channel->efx, "shut down chan %d\n", channel->channel);

		efx_for_each_channel_rx_queue(rx_queue, channel)
			efx_fini_rx_queue(rx_queue);
		efx_for_each_channel_tx_queue(tx_queue, channel)
			efx_fini_tx_queue(tx_queue);
		efx_fini_eventq(channel);
	}
}

static void efx_remove_channel(struct efx_channel *channel)
{
	struct efx_tx_queue *tx_queue;
	struct efx_rx_queue *rx_queue;

	EFX_LOG(channel->efx, "destroy chan %d\n", channel->channel);

	efx_for_each_channel_rx_queue(rx_queue, channel)
		efx_remove_rx_queue(rx_queue);
	efx_for_each_channel_tx_queue(tx_queue, channel)
		efx_remove_tx_queue(tx_queue);
	efx_remove_eventq(channel);

	channel->used_flags = 0;
}

void efx_schedule_slow_fill(struct efx_rx_queue *rx_queue, int delay)
{
	queue_delayed_work(refill_workqueue, &rx_queue->work, delay);
}




static void efx_link_status_changed(struct efx_nic *efx)
{
	
	if (!netif_running(efx->net_dev))
		return;

	if (efx->port_inhibited) {
		netif_carrier_off(efx->net_dev);
		return;
	}

	if (efx->link_up != netif_carrier_ok(efx->net_dev)) {
		efx->n_link_state_changes++;

		if (efx->link_up)
			netif_carrier_on(efx->net_dev);
		else
			netif_carrier_off(efx->net_dev);
	}

	
	if (efx->link_up) {
		EFX_INFO(efx, "link up at %uMbps %s-duplex (MTU %d)%s\n",
			 efx->link_speed, efx->link_fd ? "full" : "half",
			 efx->net_dev->mtu,
			 (efx->promiscuous ? " [PROMISC]" : ""));
	} else {
		EFX_INFO(efx, "link down\n");
	}

}

static void efx_fini_port(struct efx_nic *efx);


void __efx_reconfigure_port(struct efx_nic *efx)
{
	WARN_ON(!mutex_is_locked(&efx->mac_lock));

	EFX_LOG(efx, "reconfiguring MAC from PHY settings on CPU %d\n",
		raw_smp_processor_id());

	
	if (efx_dev_registered(efx)) {
		netif_addr_lock_bh(efx->net_dev);
		netif_addr_unlock_bh(efx->net_dev);
	}

	falcon_deconfigure_mac_wrapper(efx);

	
	if (LOOPBACK_INTERNAL(efx))
		efx->phy_mode |= PHY_MODE_TX_DISABLED;
	else
		efx->phy_mode &= ~PHY_MODE_TX_DISABLED;
	efx->phy_op->reconfigure(efx);

	if (falcon_switch_mac(efx))
		goto fail;

	efx->mac_op->reconfigure(efx);

	
	efx_link_status_changed(efx);
	return;

fail:
	EFX_ERR(efx, "failed to reconfigure MAC\n");
	efx->port_enabled = false;
	efx_fini_port(efx);
}


void efx_reconfigure_port(struct efx_nic *efx)
{
	EFX_ASSERT_RESET_SERIALISED(efx);

	mutex_lock(&efx->mac_lock);
	__efx_reconfigure_port(efx);
	mutex_unlock(&efx->mac_lock);
}


static void efx_phy_work(struct work_struct *data)
{
	struct efx_nic *efx = container_of(data, struct efx_nic, phy_work);

	mutex_lock(&efx->mac_lock);
	if (efx->port_enabled)
		__efx_reconfigure_port(efx);
	mutex_unlock(&efx->mac_lock);
}

static void efx_mac_work(struct work_struct *data)
{
	struct efx_nic *efx = container_of(data, struct efx_nic, mac_work);

	mutex_lock(&efx->mac_lock);
	if (efx->port_enabled)
		efx->mac_op->irq(efx);
	mutex_unlock(&efx->mac_lock);
}

static int efx_probe_port(struct efx_nic *efx)
{
	int rc;

	EFX_LOG(efx, "create port\n");

	
	rc = falcon_probe_port(efx);
	if (rc)
		goto err;

	if (phy_flash_cfg)
		efx->phy_mode = PHY_MODE_SPECIAL;

	
	if (is_valid_ether_addr(efx->mac_address)) {
		memcpy(efx->net_dev->dev_addr, efx->mac_address, ETH_ALEN);
	} else {
		EFX_ERR(efx, "invalid MAC address %pM\n",
			efx->mac_address);
		if (!allow_bad_hwaddr) {
			rc = -EINVAL;
			goto err;
		}
		random_ether_addr(efx->net_dev->dev_addr);
		EFX_INFO(efx, "using locally-generated MAC %pM\n",
			 efx->net_dev->dev_addr);
	}

	return 0;

 err:
	efx_remove_port(efx);
	return rc;
}

static int efx_init_port(struct efx_nic *efx)
{
	int rc;

	EFX_LOG(efx, "init port\n");

	rc = efx->phy_op->init(efx);
	if (rc)
		return rc;
	mutex_lock(&efx->mac_lock);
	efx->phy_op->reconfigure(efx);
	rc = falcon_switch_mac(efx);
	mutex_unlock(&efx->mac_lock);
	if (rc)
		goto fail;
	efx->mac_op->reconfigure(efx);

	efx->port_initialized = true;
	efx_stats_enable(efx);
	return 0;

fail:
	efx->phy_op->fini(efx);
	return rc;
}


static void efx_start_port(struct efx_nic *efx)
{
	EFX_LOG(efx, "start port\n");
	BUG_ON(efx->port_enabled);

	mutex_lock(&efx->mac_lock);
	efx->port_enabled = true;
	__efx_reconfigure_port(efx);
	efx->mac_op->irq(efx);
	mutex_unlock(&efx->mac_lock);
}


static void efx_stop_port(struct efx_nic *efx)
{
	EFX_LOG(efx, "stop port\n");

	mutex_lock(&efx->mac_lock);
	efx->port_enabled = false;
	mutex_unlock(&efx->mac_lock);

	
	if (efx_dev_registered(efx)) {
		netif_addr_lock_bh(efx->net_dev);
		netif_addr_unlock_bh(efx->net_dev);
	}
}

static void efx_fini_port(struct efx_nic *efx)
{
	EFX_LOG(efx, "shut down port\n");

	if (!efx->port_initialized)
		return;

	efx_stats_disable(efx);
	efx->phy_op->fini(efx);
	efx->port_initialized = false;

	efx->link_up = false;
	efx_link_status_changed(efx);
}

static void efx_remove_port(struct efx_nic *efx)
{
	EFX_LOG(efx, "destroying port\n");

	falcon_remove_port(efx);
}




static int efx_init_io(struct efx_nic *efx)
{
	struct pci_dev *pci_dev = efx->pci_dev;
	dma_addr_t dma_mask = efx->type->max_dma_mask;
	int rc;

	EFX_LOG(efx, "initialising I/O\n");

	rc = pci_enable_device(pci_dev);
	if (rc) {
		EFX_ERR(efx, "failed to enable PCI device\n");
		goto fail1;
	}

	pci_set_master(pci_dev);

	
	while (dma_mask > 0x7fffffffUL) {
		if (pci_dma_supported(pci_dev, dma_mask) &&
		    ((rc = pci_set_dma_mask(pci_dev, dma_mask)) == 0))
			break;
		dma_mask >>= 1;
	}
	if (rc) {
		EFX_ERR(efx, "could not find a suitable DMA mask\n");
		goto fail2;
	}
	EFX_LOG(efx, "using DMA mask %llx\n", (unsigned long long) dma_mask);
	rc = pci_set_consistent_dma_mask(pci_dev, dma_mask);
	if (rc) {
		
		EFX_ERR(efx, "failed to set consistent DMA mask\n");
		goto fail2;
	}

	efx->membase_phys = pci_resource_start(efx->pci_dev,
					       efx->type->mem_bar);
	rc = pci_request_region(pci_dev, efx->type->mem_bar, "sfc");
	if (rc) {
		EFX_ERR(efx, "request for memory BAR failed\n");
		rc = -EIO;
		goto fail3;
	}
	efx->membase = ioremap_nocache(efx->membase_phys,
				       efx->type->mem_map_size);
	if (!efx->membase) {
		EFX_ERR(efx, "could not map memory BAR %d at %llx+%x\n",
			efx->type->mem_bar,
			(unsigned long long)efx->membase_phys,
			efx->type->mem_map_size);
		rc = -ENOMEM;
		goto fail4;
	}
	EFX_LOG(efx, "memory BAR %u at %llx+%x (virtual %p)\n",
		efx->type->mem_bar, (unsigned long long)efx->membase_phys,
		efx->type->mem_map_size, efx->membase);

	return 0;

 fail4:
	pci_release_region(efx->pci_dev, efx->type->mem_bar);
 fail3:
	efx->membase_phys = 0;
 fail2:
	pci_disable_device(efx->pci_dev);
 fail1:
	return rc;
}

static void efx_fini_io(struct efx_nic *efx)
{
	EFX_LOG(efx, "shutting down I/O\n");

	if (efx->membase) {
		iounmap(efx->membase);
		efx->membase = NULL;
	}

	if (efx->membase_phys) {
		pci_release_region(efx->pci_dev, efx->type->mem_bar);
		efx->membase_phys = 0;
	}

	pci_disable_device(efx->pci_dev);
}


static int efx_wanted_rx_queues(void)
{
	cpumask_var_t core_mask;
	int count;
	int cpu;

	if (unlikely(!zalloc_cpumask_var(&core_mask, GFP_KERNEL))) {
		printk(KERN_WARNING
		       "sfc: RSS disabled due to allocation failure\n");
		return 1;
	}

	count = 0;
	for_each_online_cpu(cpu) {
		if (!cpumask_test_cpu(cpu, core_mask)) {
			++count;
			cpumask_or(core_mask, core_mask,
				   topology_core_cpumask(cpu));
		}
	}

	free_cpumask_var(core_mask);
	return count;
}


static void efx_probe_interrupts(struct efx_nic *efx)
{
	int max_channels =
		min_t(int, efx->type->phys_addr_channels, EFX_MAX_CHANNELS);
	int rc, i;

	if (efx->interrupt_mode == EFX_INT_MODE_MSIX) {
		struct msix_entry xentries[EFX_MAX_CHANNELS];
		int wanted_ints;
		int rx_queues;

		
		rx_queues = rss_cpus ? rss_cpus : efx_wanted_rx_queues();
		wanted_ints = rx_queues + (separate_tx_channels ? 1 : 0);
		wanted_ints = min(wanted_ints, max_channels);

		for (i = 0; i < wanted_ints; i++)
			xentries[i].entry = i;
		rc = pci_enable_msix(efx->pci_dev, xentries, wanted_ints);
		if (rc > 0) {
			EFX_ERR(efx, "WARNING: Insufficient MSI-X vectors"
				" available (%d < %d).\n", rc, wanted_ints);
			EFX_ERR(efx, "WARNING: Performance may be reduced.\n");
			EFX_BUG_ON_PARANOID(rc >= wanted_ints);
			wanted_ints = rc;
			rc = pci_enable_msix(efx->pci_dev, xentries,
					     wanted_ints);
		}

		if (rc == 0) {
			efx->n_rx_queues = min(rx_queues, wanted_ints);
			efx->n_channels = wanted_ints;
			for (i = 0; i < wanted_ints; i++)
				efx->channel[i].irq = xentries[i].vector;
		} else {
			
			efx->interrupt_mode = EFX_INT_MODE_MSI;
			EFX_ERR(efx, "could not enable MSI-X\n");
		}
	}

	
	if (efx->interrupt_mode == EFX_INT_MODE_MSI) {
		efx->n_rx_queues = 1;
		efx->n_channels = 1;
		rc = pci_enable_msi(efx->pci_dev);
		if (rc == 0) {
			efx->channel[0].irq = efx->pci_dev->irq;
		} else {
			EFX_ERR(efx, "could not enable MSI\n");
			efx->interrupt_mode = EFX_INT_MODE_LEGACY;
		}
	}

	
	if (efx->interrupt_mode == EFX_INT_MODE_LEGACY) {
		efx->n_rx_queues = 1;
		efx->n_channels = 1 + (separate_tx_channels ? 1 : 0);
		efx->legacy_irq = efx->pci_dev->irq;
	}
}

static void efx_remove_interrupts(struct efx_nic *efx)
{
	struct efx_channel *channel;

	
	efx_for_each_channel(channel, efx)
		channel->irq = 0;
	pci_disable_msi(efx->pci_dev);
	pci_disable_msix(efx->pci_dev);

	
	efx->legacy_irq = 0;
}

static void efx_set_channels(struct efx_nic *efx)
{
	struct efx_tx_queue *tx_queue;
	struct efx_rx_queue *rx_queue;

	efx_for_each_tx_queue(tx_queue, efx) {
		if (separate_tx_channels)
			tx_queue->channel = &efx->channel[efx->n_channels-1];
		else
			tx_queue->channel = &efx->channel[0];
		tx_queue->channel->used_flags |= EFX_USED_BY_TX;
	}

	efx_for_each_rx_queue(rx_queue, efx) {
		rx_queue->channel = &efx->channel[rx_queue->queue];
		rx_queue->channel->used_flags |= EFX_USED_BY_RX;
	}
}

static int efx_probe_nic(struct efx_nic *efx)
{
	int rc;

	EFX_LOG(efx, "creating NIC\n");

	
	rc = falcon_probe_nic(efx);
	if (rc)
		return rc;

	
	efx_probe_interrupts(efx);

	efx_set_channels(efx);

	
	efx_init_irq_moderation(efx, tx_irq_mod_usec, rx_irq_mod_usec, true);

	return 0;
}

static void efx_remove_nic(struct efx_nic *efx)
{
	EFX_LOG(efx, "destroying NIC\n");

	efx_remove_interrupts(efx);
	falcon_remove_nic(efx);
}



static int efx_probe_all(struct efx_nic *efx)
{
	struct efx_channel *channel;
	int rc;

	
	rc = efx_probe_nic(efx);
	if (rc) {
		EFX_ERR(efx, "failed to create NIC\n");
		goto fail1;
	}

	
	rc = efx_probe_port(efx);
	if (rc) {
		EFX_ERR(efx, "failed to create port\n");
		goto fail2;
	}

	
	efx_for_each_channel(channel, efx) {
		rc = efx_probe_channel(channel);
		if (rc) {
			EFX_ERR(efx, "failed to create channel %d\n",
				channel->channel);
			goto fail3;
		}
	}
	efx_set_channel_names(efx);

	return 0;

 fail3:
	efx_for_each_channel(channel, efx)
		efx_remove_channel(channel);
	efx_remove_port(efx);
 fail2:
	efx_remove_nic(efx);
 fail1:
	return rc;
}


static void efx_start_all(struct efx_nic *efx)
{
	struct efx_channel *channel;

	EFX_ASSERT_RESET_SERIALISED(efx);

	
	if (efx->port_enabled)
		return;
	if ((efx->state != STATE_RUNNING) && (efx->state != STATE_INIT))
		return;
	if (efx_dev_registered(efx) && !netif_running(efx->net_dev))
		return;

	
	efx_start_port(efx);
	if (efx_dev_registered(efx))
		efx_wake_queue(efx);

	efx_for_each_channel(channel, efx)
		efx_start_channel(channel);

	falcon_enable_interrupts(efx);

	
	if (efx->state == STATE_RUNNING)
		queue_delayed_work(efx->workqueue, &efx->monitor_work,
				   efx_monitor_interval);
}


static void efx_flush_all(struct efx_nic *efx)
{
	struct efx_rx_queue *rx_queue;

	
	cancel_delayed_work_sync(&efx->monitor_work);

	
	efx_for_each_rx_queue(rx_queue, efx)
		cancel_delayed_work_sync(&rx_queue->work);

	
	cancel_work_sync(&efx->mac_work);
	cancel_work_sync(&efx->phy_work);

}


static void efx_stop_all(struct efx_nic *efx)
{
	struct efx_channel *channel;

	EFX_ASSERT_RESET_SERIALISED(efx);

	
	if (!efx->port_enabled)
		return;

	
	falcon_disable_interrupts(efx);
	if (efx->legacy_irq)
		synchronize_irq(efx->legacy_irq);
	efx_for_each_channel(channel, efx) {
		if (channel->irq)
			synchronize_irq(channel->irq);
	}

	
	efx_for_each_channel(channel, efx)
		efx_stop_channel(channel);

	
	efx_stop_port(efx);

	
	efx_flush_all(efx);

	
	falcon_deconfigure_mac_wrapper(efx);
	msleep(10); 
	falcon_drain_tx_fifo(efx);

	
	if (efx_dev_registered(efx)) {
		efx_stop_queue(efx);
		netif_tx_lock_bh(efx->net_dev);
		netif_tx_unlock_bh(efx->net_dev);
	}
}

static void efx_remove_all(struct efx_nic *efx)
{
	struct efx_channel *channel;

	efx_for_each_channel(channel, efx)
		efx_remove_channel(channel);
	efx_remove_port(efx);
	efx_remove_nic(efx);
}


void efx_flush_queues(struct efx_nic *efx)
{
	EFX_ASSERT_RESET_SERIALISED(efx);

	efx_stop_all(efx);

	efx_fini_channels(efx);
	efx_init_channels(efx);

	efx_start_all(efx);
}




void efx_init_irq_moderation(struct efx_nic *efx, int tx_usecs, int rx_usecs,
			     bool rx_adaptive)
{
	struct efx_tx_queue *tx_queue;
	struct efx_rx_queue *rx_queue;

	EFX_ASSERT_RESET_SERIALISED(efx);

	efx_for_each_tx_queue(tx_queue, efx)
		tx_queue->channel->irq_moderation = tx_usecs;

	efx->irq_rx_adaptive = rx_adaptive;
	efx->irq_rx_moderation = rx_usecs;
	efx_for_each_rx_queue(rx_queue, efx)
		rx_queue->channel->irq_moderation = rx_usecs;
}




static void efx_monitor(struct work_struct *data)
{
	struct efx_nic *efx = container_of(data, struct efx_nic,
					   monitor_work.work);
	int rc;

	EFX_TRACE(efx, "hardware monitor executing on CPU %d\n",
		  raw_smp_processor_id());

	
	if (!mutex_trylock(&efx->mac_lock))
		goto out_requeue;
	if (!efx->port_enabled)
		goto out_unlock;
	rc = efx->board_info.monitor(efx);
	if (rc) {
		EFX_ERR(efx, "Board sensor %s; shutting down PHY\n",
			(rc == -ERANGE) ? "reported fault" : "failed");
		efx->phy_mode |= PHY_MODE_LOW_POWER;
		falcon_sim_phy_event(efx);
	}
	efx->phy_op->poll(efx);
	efx->mac_op->poll(efx);

out_unlock:
	mutex_unlock(&efx->mac_lock);
out_requeue:
	queue_delayed_work(efx->workqueue, &efx->monitor_work,
			   efx_monitor_interval);
}




static int efx_ioctl(struct net_device *net_dev, struct ifreq *ifr, int cmd)
{
	struct efx_nic *efx = netdev_priv(net_dev);
	struct mii_ioctl_data *data = if_mii(ifr);

	EFX_ASSERT_RESET_SERIALISED(efx);

	
	if ((cmd == SIOCGMIIREG || cmd == SIOCSMIIREG) &&
	    (data->phy_id & 0xfc00) == 0x0400)
		data->phy_id ^= MDIO_PHY_ID_C45 | 0x0400;

	return mdio_mii_ioctl(&efx->mdio, data, cmd);
}



static int efx_init_napi(struct efx_nic *efx)
{
	struct efx_channel *channel;

	efx_for_each_channel(channel, efx) {
		channel->napi_dev = efx->net_dev;
		netif_napi_add(channel->napi_dev, &channel->napi_str,
			       efx_poll, napi_weight);
	}
	return 0;
}

static void efx_fini_napi(struct efx_nic *efx)
{
	struct efx_channel *channel;

	efx_for_each_channel(channel, efx) {
		if (channel->napi_dev)
			netif_napi_del(&channel->napi_str);
		channel->napi_dev = NULL;
	}
}



#ifdef CONFIG_NET_POLL_CONTROLLER


static void efx_netpoll(struct net_device *net_dev)
{
	struct efx_nic *efx = netdev_priv(net_dev);
	struct efx_channel *channel;

	efx_for_each_channel(channel, efx)
		efx_schedule_channel(channel);
}

#endif




static int efx_net_open(struct net_device *net_dev)
{
	struct efx_nic *efx = netdev_priv(net_dev);
	EFX_ASSERT_RESET_SERIALISED(efx);

	EFX_LOG(efx, "opening device %s on CPU %d\n", net_dev->name,
		raw_smp_processor_id());

	if (efx->state == STATE_DISABLED)
		return -EIO;
	if (efx->phy_mode & PHY_MODE_SPECIAL)
		return -EBUSY;

	efx_start_all(efx);
	return 0;
}


static int efx_net_stop(struct net_device *net_dev)
{
	struct efx_nic *efx = netdev_priv(net_dev);

	EFX_LOG(efx, "closing %s on CPU %d\n", net_dev->name,
		raw_smp_processor_id());

	if (efx->state != STATE_DISABLED) {
		
		efx_stop_all(efx);
		efx_fini_channels(efx);
		efx_init_channels(efx);
	}

	return 0;
}

void efx_stats_disable(struct efx_nic *efx)
{
	spin_lock(&efx->stats_lock);
	++efx->stats_disable_count;
	spin_unlock(&efx->stats_lock);
}

void efx_stats_enable(struct efx_nic *efx)
{
	spin_lock(&efx->stats_lock);
	--efx->stats_disable_count;
	spin_unlock(&efx->stats_lock);
}


static struct net_device_stats *efx_net_stats(struct net_device *net_dev)
{
	struct efx_nic *efx = netdev_priv(net_dev);
	struct efx_mac_stats *mac_stats = &efx->mac_stats;
	struct net_device_stats *stats = &net_dev->stats;

	
	if (!spin_trylock(&efx->stats_lock))
		return stats;
	if (!efx->stats_disable_count) {
		efx->mac_op->update_stats(efx);
		falcon_update_nic_stats(efx);
	}
	spin_unlock(&efx->stats_lock);

	stats->rx_packets = mac_stats->rx_packets;
	stats->tx_packets = mac_stats->tx_packets;
	stats->rx_bytes = mac_stats->rx_bytes;
	stats->tx_bytes = mac_stats->tx_bytes;
	stats->multicast = mac_stats->rx_multicast;
	stats->collisions = mac_stats->tx_collision;
	stats->rx_length_errors = (mac_stats->rx_gtjumbo +
				   mac_stats->rx_length_error);
	stats->rx_over_errors = efx->n_rx_nodesc_drop_cnt;
	stats->rx_crc_errors = mac_stats->rx_bad;
	stats->rx_frame_errors = mac_stats->rx_align_error;
	stats->rx_fifo_errors = mac_stats->rx_overflow;
	stats->rx_missed_errors = mac_stats->rx_missed;
	stats->tx_window_errors = mac_stats->tx_late_collision;

	stats->rx_errors = (stats->rx_length_errors +
			    stats->rx_over_errors +
			    stats->rx_crc_errors +
			    stats->rx_frame_errors +
			    stats->rx_fifo_errors +
			    stats->rx_missed_errors +
			    mac_stats->rx_symbol_error);
	stats->tx_errors = (stats->tx_window_errors +
			    mac_stats->tx_bad);

	return stats;
}


static void efx_watchdog(struct net_device *net_dev)
{
	struct efx_nic *efx = netdev_priv(net_dev);

	EFX_ERR(efx, "TX stuck with stop_count=%d port_enabled=%d:"
		" resetting channels\n",
		atomic_read(&efx->netif_stop_count), efx->port_enabled);

	efx_schedule_reset(efx, RESET_TYPE_TX_WATCHDOG);
}



static int efx_change_mtu(struct net_device *net_dev, int new_mtu)
{
	struct efx_nic *efx = netdev_priv(net_dev);
	int rc = 0;

	EFX_ASSERT_RESET_SERIALISED(efx);

	if (new_mtu > EFX_MAX_MTU)
		return -EINVAL;

	efx_stop_all(efx);

	EFX_LOG(efx, "changing MTU to %d\n", new_mtu);

	efx_fini_channels(efx);
	net_dev->mtu = new_mtu;
	efx_init_channels(efx);

	efx_start_all(efx);
	return rc;
}

static int efx_set_mac_address(struct net_device *net_dev, void *data)
{
	struct efx_nic *efx = netdev_priv(net_dev);
	struct sockaddr *addr = data;
	char *new_addr = addr->sa_data;

	EFX_ASSERT_RESET_SERIALISED(efx);

	if (!is_valid_ether_addr(new_addr)) {
		EFX_ERR(efx, "invalid ethernet MAC address requested: %pM\n",
			new_addr);
		return -EINVAL;
	}

	memcpy(net_dev->dev_addr, new_addr, net_dev->addr_len);

	
	efx_reconfigure_port(efx);

	return 0;
}


static void efx_set_multicast_list(struct net_device *net_dev)
{
	struct efx_nic *efx = netdev_priv(net_dev);
	struct dev_mc_list *mc_list = net_dev->mc_list;
	union efx_multicast_hash *mc_hash = &efx->multicast_hash;
	bool promiscuous = !!(net_dev->flags & IFF_PROMISC);
	bool changed = (efx->promiscuous != promiscuous);
	u32 crc;
	int bit;
	int i;

	efx->promiscuous = promiscuous;

	
	if (promiscuous || (net_dev->flags & IFF_ALLMULTI)) {
		memset(mc_hash, 0xff, sizeof(*mc_hash));
	} else {
		memset(mc_hash, 0x00, sizeof(*mc_hash));
		for (i = 0; i < net_dev->mc_count; i++) {
			crc = ether_crc_le(ETH_ALEN, mc_list->dmi_addr);
			bit = crc & (EFX_MCAST_HASH_ENTRIES - 1);
			set_bit_le(bit, mc_hash->byte);
			mc_list = mc_list->next;
		}
	}

	if (!efx->port_enabled)
		
		return;

	if (changed)
		queue_work(efx->workqueue, &efx->phy_work);

	
	falcon_set_multicast_hash(efx);
}

static const struct net_device_ops efx_netdev_ops = {
	.ndo_open		= efx_net_open,
	.ndo_stop		= efx_net_stop,
	.ndo_get_stats		= efx_net_stats,
	.ndo_tx_timeout		= efx_watchdog,
	.ndo_start_xmit		= efx_hard_start_xmit,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= efx_ioctl,
	.ndo_change_mtu		= efx_change_mtu,
	.ndo_set_mac_address	= efx_set_mac_address,
	.ndo_set_multicast_list = efx_set_multicast_list,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller = efx_netpoll,
#endif
};

static void efx_update_name(struct efx_nic *efx)
{
	strcpy(efx->name, efx->net_dev->name);
	efx_mtd_rename(efx);
	efx_set_channel_names(efx);
}

static int efx_netdev_event(struct notifier_block *this,
			    unsigned long event, void *ptr)
{
	struct net_device *net_dev = ptr;

	if (net_dev->netdev_ops == &efx_netdev_ops &&
	    event == NETDEV_CHANGENAME)
		efx_update_name(netdev_priv(net_dev));

	return NOTIFY_DONE;
}

static struct notifier_block efx_netdev_notifier = {
	.notifier_call = efx_netdev_event,
};

static ssize_t
show_phy_type(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct efx_nic *efx = pci_get_drvdata(to_pci_dev(dev));
	return sprintf(buf, "%d\n", efx->phy_type);
}
static DEVICE_ATTR(phy_type, 0644, show_phy_type, NULL);

static int efx_register_netdev(struct efx_nic *efx)
{
	struct net_device *net_dev = efx->net_dev;
	int rc;

	net_dev->watchdog_timeo = 5 * HZ;
	net_dev->irq = efx->pci_dev->irq;
	net_dev->netdev_ops = &efx_netdev_ops;
	SET_NETDEV_DEV(net_dev, &efx->pci_dev->dev);
	SET_ETHTOOL_OPS(net_dev, &efx_ethtool_ops);

	
	efx->mac_op->update_stats(efx);
	memset(&efx->mac_stats, 0, sizeof(efx->mac_stats));

	rtnl_lock();

	rc = dev_alloc_name(net_dev, net_dev->name);
	if (rc < 0)
		goto fail_locked;
	efx_update_name(efx);

	rc = register_netdevice(net_dev);
	if (rc)
		goto fail_locked;

	
	netif_carrier_off(efx->net_dev);

	rtnl_unlock();

	rc = device_create_file(&efx->pci_dev->dev, &dev_attr_phy_type);
	if (rc) {
		EFX_ERR(efx, "failed to init net dev attributes\n");
		goto fail_registered;
	}

	return 0;

fail_locked:
	rtnl_unlock();
	EFX_ERR(efx, "could not register net dev\n");
	return rc;

fail_registered:
	unregister_netdev(net_dev);
	return rc;
}

static void efx_unregister_netdev(struct efx_nic *efx)
{
	struct efx_tx_queue *tx_queue;

	if (!efx->net_dev)
		return;

	BUG_ON(netdev_priv(efx->net_dev) != efx);

	
	efx_for_each_tx_queue(tx_queue, efx)
		efx_release_tx_buffers(tx_queue);

	if (efx_dev_registered(efx)) {
		strlcpy(efx->name, pci_name(efx->pci_dev), sizeof(efx->name));
		device_remove_file(&efx->pci_dev->dev, &dev_attr_phy_type);
		unregister_netdev(efx->net_dev);
	}
}




void efx_reset_down(struct efx_nic *efx, enum reset_type method,
		    struct ethtool_cmd *ecmd)
{
	EFX_ASSERT_RESET_SERIALISED(efx);

	efx_stats_disable(efx);
	efx_stop_all(efx);
	mutex_lock(&efx->mac_lock);
	mutex_lock(&efx->spi_lock);

	efx->phy_op->get_settings(efx, ecmd);

	efx_fini_channels(efx);
	if (efx->port_initialized && method != RESET_TYPE_INVISIBLE)
		efx->phy_op->fini(efx);
}


int efx_reset_up(struct efx_nic *efx, enum reset_type method,
		 struct ethtool_cmd *ecmd, bool ok)
{
	int rc;

	EFX_ASSERT_RESET_SERIALISED(efx);

	rc = falcon_init_nic(efx);
	if (rc) {
		EFX_ERR(efx, "failed to initialise NIC\n");
		ok = false;
	}

	if (efx->port_initialized && method != RESET_TYPE_INVISIBLE) {
		if (ok) {
			rc = efx->phy_op->init(efx);
			if (rc)
				ok = false;
		}
		if (!ok)
			efx->port_initialized = false;
	}

	if (ok) {
		efx_init_channels(efx);

		if (efx->phy_op->set_settings(efx, ecmd))
			EFX_ERR(efx, "could not restore PHY settings\n");
	}

	mutex_unlock(&efx->spi_lock);
	mutex_unlock(&efx->mac_lock);

	if (ok) {
		efx_start_all(efx);
		efx_stats_enable(efx);
	}
	return rc;
}


static int efx_reset(struct efx_nic *efx)
{
	struct ethtool_cmd ecmd;
	enum reset_type method = efx->reset_pending;
	int rc = 0;

	
	rtnl_lock();

	
	if (efx->state != STATE_RUNNING) {
		EFX_INFO(efx, "scheduled reset quenched. NIC not RUNNING\n");
		goto out_unlock;
	}

	EFX_INFO(efx, "resetting (%d)\n", method);

	efx_reset_down(efx, method, &ecmd);

	rc = falcon_reset_hw(efx, method);
	if (rc) {
		EFX_ERR(efx, "failed to reset hardware\n");
		goto out_disable;
	}

	
	efx->reset_pending = RESET_TYPE_NONE;

	
	pci_set_master(efx->pci_dev);

	
	if (method == RESET_TYPE_DISABLE) {
		efx_reset_up(efx, method, &ecmd, false);
		rc = -EIO;
	} else {
		rc = efx_reset_up(efx, method, &ecmd, true);
	}

out_disable:
	if (rc) {
		EFX_ERR(efx, "has been disabled\n");
		efx->state = STATE_DISABLED;
		dev_close(efx->net_dev);
	} else {
		EFX_LOG(efx, "reset complete\n");
	}

out_unlock:
	rtnl_unlock();
	return rc;
}


static void efx_reset_work(struct work_struct *data)
{
	struct efx_nic *nic = container_of(data, struct efx_nic, reset_work);

	efx_reset(nic);
}

void efx_schedule_reset(struct efx_nic *efx, enum reset_type type)
{
	enum reset_type method;

	if (efx->reset_pending != RESET_TYPE_NONE) {
		EFX_INFO(efx, "quenching already scheduled reset\n");
		return;
	}

	switch (type) {
	case RESET_TYPE_INVISIBLE:
	case RESET_TYPE_ALL:
	case RESET_TYPE_WORLD:
	case RESET_TYPE_DISABLE:
		method = type;
		break;
	case RESET_TYPE_RX_RECOVERY:
	case RESET_TYPE_RX_DESC_FETCH:
	case RESET_TYPE_TX_DESC_FETCH:
	case RESET_TYPE_TX_SKIP:
		method = RESET_TYPE_INVISIBLE;
		break;
	default:
		method = RESET_TYPE_ALL;
		break;
	}

	if (method != type)
		EFX_LOG(efx, "scheduling reset (%d:%d)\n", type, method);
	else
		EFX_LOG(efx, "scheduling reset (%d)\n", method);

	efx->reset_pending = method;

	queue_work(reset_workqueue, &efx->reset_work);
}




static struct pci_device_id efx_pci_table[] __devinitdata = {
	{PCI_DEVICE(EFX_VENDID_SFC, FALCON_A_P_DEVID),
	 .driver_data = (unsigned long) &falcon_a_nic_type},
	{PCI_DEVICE(EFX_VENDID_SFC, FALCON_B_P_DEVID),
	 .driver_data = (unsigned long) &falcon_b_nic_type},
	{0}			
};


int efx_port_dummy_op_int(struct efx_nic *efx)
{
	return 0;
}
void efx_port_dummy_op_void(struct efx_nic *efx) {}
void efx_port_dummy_op_blink(struct efx_nic *efx, bool blink) {}

static struct efx_mac_operations efx_dummy_mac_operations = {
	.reconfigure	= efx_port_dummy_op_void,
	.poll		= efx_port_dummy_op_void,
	.irq		= efx_port_dummy_op_void,
};

static struct efx_phy_operations efx_dummy_phy_operations = {
	.init		 = efx_port_dummy_op_int,
	.reconfigure	 = efx_port_dummy_op_void,
	.poll		 = efx_port_dummy_op_void,
	.fini		 = efx_port_dummy_op_void,
	.clear_interrupt = efx_port_dummy_op_void,
};

static struct efx_board efx_dummy_board_info = {
	.init		= efx_port_dummy_op_int,
	.init_leds	= efx_port_dummy_op_void,
	.set_id_led	= efx_port_dummy_op_blink,
	.monitor	= efx_port_dummy_op_int,
	.blink		= efx_port_dummy_op_blink,
	.fini		= efx_port_dummy_op_void,
};




static int efx_init_struct(struct efx_nic *efx, struct efx_nic_type *type,
			   struct pci_dev *pci_dev, struct net_device *net_dev)
{
	struct efx_channel *channel;
	struct efx_tx_queue *tx_queue;
	struct efx_rx_queue *rx_queue;
	int i;

	
	memset(efx, 0, sizeof(*efx));
	spin_lock_init(&efx->biu_lock);
	spin_lock_init(&efx->phy_lock);
	mutex_init(&efx->spi_lock);
	INIT_WORK(&efx->reset_work, efx_reset_work);
	INIT_DELAYED_WORK(&efx->monitor_work, efx_monitor);
	efx->pci_dev = pci_dev;
	efx->state = STATE_INIT;
	efx->reset_pending = RESET_TYPE_NONE;
	strlcpy(efx->name, pci_name(pci_dev), sizeof(efx->name));
	efx->board_info = efx_dummy_board_info;

	efx->net_dev = net_dev;
	efx->rx_checksum_enabled = true;
	spin_lock_init(&efx->netif_stop_lock);
	spin_lock_init(&efx->stats_lock);
	efx->stats_disable_count = 1;
	mutex_init(&efx->mac_lock);
	efx->mac_op = &efx_dummy_mac_operations;
	efx->phy_op = &efx_dummy_phy_operations;
	efx->mdio.dev = net_dev;
	INIT_WORK(&efx->phy_work, efx_phy_work);
	INIT_WORK(&efx->mac_work, efx_mac_work);
	atomic_set(&efx->netif_stop_count, 1);

	for (i = 0; i < EFX_MAX_CHANNELS; i++) {
		channel = &efx->channel[i];
		channel->efx = efx;
		channel->channel = i;
		channel->work_pending = false;
	}
	for (i = 0; i < EFX_TX_QUEUE_COUNT; i++) {
		tx_queue = &efx->tx_queue[i];
		tx_queue->efx = efx;
		tx_queue->queue = i;
		tx_queue->buffer = NULL;
		tx_queue->channel = &efx->channel[0]; 
		tx_queue->tso_headers_free = NULL;
	}
	for (i = 0; i < EFX_MAX_RX_QUEUES; i++) {
		rx_queue = &efx->rx_queue[i];
		rx_queue->efx = efx;
		rx_queue->queue = i;
		rx_queue->channel = &efx->channel[0]; 
		rx_queue->buffer = NULL;
		spin_lock_init(&rx_queue->add_lock);
		INIT_DELAYED_WORK(&rx_queue->work, efx_rx_work);
	}

	efx->type = type;

	
	EFX_BUG_ON_PARANOID(efx->type->txd_ring_mask &
			    (efx->type->txd_ring_mask + 1));
	EFX_BUG_ON_PARANOID(efx->type->rxd_ring_mask &
			    (efx->type->rxd_ring_mask + 1));
	EFX_BUG_ON_PARANOID(efx->type->evq_size &
			    (efx->type->evq_size - 1));
	
	EFX_BUG_ON_PARANOID(efx->type->evq_size <
			    (efx->type->txd_ring_mask + 1 +
			     efx->type->rxd_ring_mask + 1));
	EFX_BUG_ON_PARANOID(efx->type->phys_addr_channels > EFX_MAX_CHANNELS);

	
	efx->interrupt_mode = max(efx->type->max_interrupt_mode,
				  interrupt_mode);

	
	snprintf(efx->workqueue_name, sizeof(efx->workqueue_name), "sfc%s",
		 pci_name(pci_dev));
	efx->workqueue = create_singlethread_workqueue(efx->workqueue_name);
	if (!efx->workqueue)
		return -ENOMEM;

	return 0;
}

static void efx_fini_struct(struct efx_nic *efx)
{
	if (efx->workqueue) {
		destroy_workqueue(efx->workqueue);
		efx->workqueue = NULL;
	}
}




static void efx_pci_remove_main(struct efx_nic *efx)
{
	EFX_ASSERT_RESET_SERIALISED(efx);

	
	if (!efx->membase)
		return;

	efx_fini_channels(efx);
	efx_fini_port(efx);

	
	efx->board_info.fini(efx);
	falcon_fini_interrupt(efx);

	efx_fini_napi(efx);
	efx_remove_all(efx);
}


static void efx_pci_remove(struct pci_dev *pci_dev)
{
	struct efx_nic *efx;

	efx = pci_get_drvdata(pci_dev);
	if (!efx)
		return;

	
	rtnl_lock();
	efx->state = STATE_FINI;
	dev_close(efx->net_dev);

	
	rtnl_unlock();

	if (efx->membase == NULL)
		goto out;

	efx_unregister_netdev(efx);

	efx_mtd_remove(efx);

	
	cancel_work_sync(&efx->reset_work);

	efx_pci_remove_main(efx);

out:
	efx_fini_io(efx);
	EFX_LOG(efx, "shutdown successful\n");

	pci_set_drvdata(pci_dev, NULL);
	efx_fini_struct(efx);
	free_netdev(efx->net_dev);
};


static int efx_pci_probe_main(struct efx_nic *efx)
{
	int rc;

	
	rc = efx_probe_all(efx);
	if (rc)
		goto fail1;

	rc = efx_init_napi(efx);
	if (rc)
		goto fail2;

	
	rc = efx->board_info.init(efx);
	if (rc) {
		EFX_ERR(efx, "failed to initialise board\n");
		goto fail3;
	}

	rc = falcon_init_nic(efx);
	if (rc) {
		EFX_ERR(efx, "failed to initialise NIC\n");
		goto fail4;
	}

	rc = efx_init_port(efx);
	if (rc) {
		EFX_ERR(efx, "failed to initialise port\n");
		goto fail5;
	}

	efx_init_channels(efx);

	rc = falcon_init_interrupt(efx);
	if (rc)
		goto fail6;

	return 0;

 fail6:
	efx_fini_channels(efx);
	efx_fini_port(efx);
 fail5:
 fail4:
	efx->board_info.fini(efx);
 fail3:
	efx_fini_napi(efx);
 fail2:
	efx_remove_all(efx);
 fail1:
	return rc;
}


static int __devinit efx_pci_probe(struct pci_dev *pci_dev,
				   const struct pci_device_id *entry)
{
	struct efx_nic_type *type = (struct efx_nic_type *) entry->driver_data;
	struct net_device *net_dev;
	struct efx_nic *efx;
	int i, rc;

	
	net_dev = alloc_etherdev(sizeof(*efx));
	if (!net_dev)
		return -ENOMEM;
	net_dev->features |= (NETIF_F_IP_CSUM | NETIF_F_SG |
			      NETIF_F_HIGHDMA | NETIF_F_TSO |
			      NETIF_F_GRO);
	
	net_dev->vlan_features |= (NETIF_F_ALL_CSUM | NETIF_F_SG |
				   NETIF_F_HIGHDMA | NETIF_F_TSO);
	efx = netdev_priv(net_dev);
	pci_set_drvdata(pci_dev, efx);
	rc = efx_init_struct(efx, type, pci_dev, net_dev);
	if (rc)
		goto fail1;

	EFX_INFO(efx, "Solarflare Communications NIC detected\n");

	
	rc = efx_init_io(efx);
	if (rc)
		goto fail2;

	
	for (i = 0; i < 5; i++) {
		rc = efx_pci_probe_main(efx);

		
		cancel_work_sync(&efx->reset_work);

		if (rc == 0) {
			if (efx->reset_pending != RESET_TYPE_NONE) {
				
				efx_pci_remove_main(efx);
				rc = -EIO;
			} else {
				break;
			}
		}

		
		if ((efx->reset_pending != RESET_TYPE_INVISIBLE) &&
		    (efx->reset_pending != RESET_TYPE_ALL))
			goto fail3;

		efx->reset_pending = RESET_TYPE_NONE;
	}

	if (rc) {
		EFX_ERR(efx, "Could not reset NIC\n");
		goto fail4;
	}

	
	efx->state = STATE_RUNNING;

	efx_mtd_probe(efx); 

	rc = efx_register_netdev(efx);
	if (rc)
		goto fail5;

	EFX_LOG(efx, "initialisation successful\n");
	return 0;

 fail5:
	efx_pci_remove_main(efx);
 fail4:
 fail3:
	efx_fini_io(efx);
 fail2:
	efx_fini_struct(efx);
 fail1:
	EFX_LOG(efx, "initialisation failed. rc=%d\n", rc);
	free_netdev(net_dev);
	return rc;
}

static struct pci_driver efx_pci_driver = {
	.name		= EFX_DRIVER_NAME,
	.id_table	= efx_pci_table,
	.probe		= efx_pci_probe,
	.remove		= efx_pci_remove,
};



module_param(interrupt_mode, uint, 0444);
MODULE_PARM_DESC(interrupt_mode,
		 "Interrupt mode (0=>MSIX 1=>MSI 2=>legacy)");

static int __init efx_init_module(void)
{
	int rc;

	printk(KERN_INFO "Solarflare NET driver v" EFX_DRIVER_VERSION "\n");

	rc = register_netdevice_notifier(&efx_netdev_notifier);
	if (rc)
		goto err_notifier;

	refill_workqueue = create_workqueue("sfc_refill");
	if (!refill_workqueue) {
		rc = -ENOMEM;
		goto err_refill;
	}
	reset_workqueue = create_singlethread_workqueue("sfc_reset");
	if (!reset_workqueue) {
		rc = -ENOMEM;
		goto err_reset;
	}

	rc = pci_register_driver(&efx_pci_driver);
	if (rc < 0)
		goto err_pci;

	return 0;

 err_pci:
	destroy_workqueue(reset_workqueue);
 err_reset:
	destroy_workqueue(refill_workqueue);
 err_refill:
	unregister_netdevice_notifier(&efx_netdev_notifier);
 err_notifier:
	return rc;
}

static void __exit efx_exit_module(void)
{
	printk(KERN_INFO "Solarflare NET driver unloading\n");

	pci_unregister_driver(&efx_pci_driver);
	destroy_workqueue(reset_workqueue);
	destroy_workqueue(refill_workqueue);
	unregister_netdevice_notifier(&efx_netdev_notifier);

}

module_init(efx_init_module);
module_exit(efx_exit_module);

MODULE_AUTHOR("Michael Brown <mbrown@fensystems.co.uk> and "
	      "Solarflare Communications");
MODULE_DESCRIPTION("Solarflare Communications network driver");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(pci, efx_pci_table);
