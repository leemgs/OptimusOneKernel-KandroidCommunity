

#ifndef EFX_SELFTEST_H
#define EFX_SELFTEST_H

#include "net_driver.h"



struct efx_loopback_self_tests {
	int tx_sent[EFX_TX_QUEUE_COUNT];
	int tx_done[EFX_TX_QUEUE_COUNT];
	int rx_good;
	int rx_bad;
};

#define EFX_MAX_PHY_TESTS 20


struct efx_self_tests {
	
	int mdio;
	int nvram;
	int interrupt;
	int eventq_dma[EFX_MAX_CHANNELS];
	int eventq_int[EFX_MAX_CHANNELS];
	int eventq_poll[EFX_MAX_CHANNELS];
	
	int registers;
	int phy[EFX_MAX_PHY_TESTS];
	struct efx_loopback_self_tests loopback[LOOPBACK_TEST_MAX + 1];
};

extern void efx_loopback_rx_packet(struct efx_nic *efx,
				   const char *buf_ptr, int pkt_len);
extern int efx_selftest(struct efx_nic *efx,
			struct efx_self_tests *tests,
			unsigned flags);

#endif 
