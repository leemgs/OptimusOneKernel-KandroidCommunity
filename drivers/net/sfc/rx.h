

#ifndef EFX_RX_H
#define EFX_RX_H

#include "net_driver.h"

int efx_probe_rx_queue(struct efx_rx_queue *rx_queue);
void efx_remove_rx_queue(struct efx_rx_queue *rx_queue);
void efx_init_rx_queue(struct efx_rx_queue *rx_queue);
void efx_fini_rx_queue(struct efx_rx_queue *rx_queue);

void efx_rx_strategy(struct efx_channel *channel);
void efx_fast_push_rx_descriptors(struct efx_rx_queue *rx_queue);
void efx_rx_work(struct work_struct *data);
void __efx_rx_packet(struct efx_channel *channel,
		     struct efx_rx_buffer *rx_buf, bool checksummed);

#endif 
