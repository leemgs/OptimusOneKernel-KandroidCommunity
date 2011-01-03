

#ifndef EFX_EFX_H
#define EFX_EFX_H

#include "net_driver.h"


#define EFX_VENDID_SFC	        0x1924
#define FALCON_A_P_DEVID	0x0703
#define FALCON_A_S_DEVID        0x6703
#define FALCON_B_P_DEVID        0x0710


extern netdev_tx_t efx_xmit(struct efx_nic *efx,
				  struct efx_tx_queue *tx_queue,
				  struct sk_buff *skb);
extern void efx_stop_queue(struct efx_nic *efx);
extern void efx_wake_queue(struct efx_nic *efx);


extern void efx_xmit_done(struct efx_tx_queue *tx_queue, unsigned int index);
extern void efx_rx_packet(struct efx_rx_queue *rx_queue, unsigned int index,
			  unsigned int len, bool checksummed, bool discard);
extern void efx_schedule_slow_fill(struct efx_rx_queue *rx_queue, int delay);


extern void efx_process_channel_now(struct efx_channel *channel);
extern void efx_flush_queues(struct efx_nic *efx);


extern void efx_stats_disable(struct efx_nic *efx);
extern void efx_stats_enable(struct efx_nic *efx);
extern void efx_reconfigure_port(struct efx_nic *efx);
extern void __efx_reconfigure_port(struct efx_nic *efx);


extern void efx_reset_down(struct efx_nic *efx, enum reset_type method,
			   struct ethtool_cmd *ecmd);
extern int efx_reset_up(struct efx_nic *efx, enum reset_type method,
			struct ethtool_cmd *ecmd, bool ok);


extern void efx_schedule_reset(struct efx_nic *efx, enum reset_type type);
extern void efx_suspend(struct efx_nic *efx);
extern void efx_resume(struct efx_nic *efx);
extern void efx_init_irq_moderation(struct efx_nic *efx, int tx_usecs,
				    int rx_usecs, bool rx_adaptive);
extern int efx_request_power(struct efx_nic *efx, int mw, const char *name);
extern void efx_hex_dump(const u8 *, unsigned int, const char *);


extern int efx_port_dummy_op_int(struct efx_nic *efx);
extern void efx_port_dummy_op_void(struct efx_nic *efx);
extern void efx_port_dummy_op_blink(struct efx_nic *efx, bool blink);


#ifdef CONFIG_SFC_MTD
extern int efx_mtd_probe(struct efx_nic *efx);
extern void efx_mtd_rename(struct efx_nic *efx);
extern void efx_mtd_remove(struct efx_nic *efx);
#else
static inline int efx_mtd_probe(struct efx_nic *efx) { return 0; }
static inline void efx_mtd_rename(struct efx_nic *efx) {}
static inline void efx_mtd_remove(struct efx_nic *efx) {}
#endif

extern unsigned int efx_monitor_interval;

static inline void efx_schedule_channel(struct efx_channel *channel)
{
	EFX_TRACE(channel->efx, "channel %d scheduling NAPI poll on CPU%d\n",
		  channel->channel, raw_smp_processor_id());
	channel->work_pending = true;

	napi_schedule(&channel->napi_str);
}

#endif 
