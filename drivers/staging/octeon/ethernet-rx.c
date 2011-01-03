
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cache.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/string.h>
#include <linux/prefetch.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <net/dst.h>
#ifdef CONFIG_XFRM
#include <linux/xfrm.h>
#include <net/xfrm.h>
#endif 

#include <asm/atomic.h>

#include <asm/octeon/octeon.h>

#include "ethernet-defines.h"
#include "octeon-ethernet.h"
#include "ethernet-mem.h"
#include "ethernet-util.h"

#include "cvmx-helper.h"
#include "cvmx-wqe.h"
#include "cvmx-fau.h"
#include "cvmx-pow.h"
#include "cvmx-pip.h"
#include "cvmx-scratch.h"

#include "cvmx-gmxx-defs.h"

struct cvm_tasklet_wrapper {
	struct tasklet_struct t;
};



static struct cvm_tasklet_wrapper cvm_oct_tasklet[NR_CPUS];


irqreturn_t cvm_oct_do_interrupt(int cpl, void *dev_id)
{
	
	if (INTERRUPT_LIMIT)
		cvmx_write_csr(CVMX_POW_WQ_INT, 1 << pow_receive_group);
	else
		cvmx_write_csr(CVMX_POW_WQ_INT, 0x10001 << pow_receive_group);
	preempt_disable();
	tasklet_schedule(&cvm_oct_tasklet[smp_processor_id()].t);
	preempt_enable();
	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER

void cvm_oct_poll_controller(struct net_device *dev)
{
	preempt_disable();
	tasklet_schedule(&cvm_oct_tasklet[smp_processor_id()].t);
	preempt_enable();
}
#endif


static inline int cvm_oct_check_rcv_error(cvmx_wqe_t *work)
{
	if ((work->word2.snoip.err_code == 10) && (work->len <= 64)) {
		
	} else
	    if (USE_10MBPS_PREAMBLE_WORKAROUND
		&& ((work->word2.snoip.err_code == 5)
		    || (work->word2.snoip.err_code == 7))) {

		
		int interface = cvmx_helper_get_interface_num(work->ipprt);
		int index = cvmx_helper_get_interface_index_num(work->ipprt);
		union cvmx_gmxx_rxx_frm_ctl gmxx_rxx_frm_ctl;
		gmxx_rxx_frm_ctl.u64 =
		    cvmx_read_csr(CVMX_GMXX_RXX_FRM_CTL(index, interface));
		if (gmxx_rxx_frm_ctl.s.pre_chk == 0) {

			uint8_t *ptr =
			    cvmx_phys_to_ptr(work->packet_ptr.s.addr);
			int i = 0;

			while (i < work->len - 1) {
				if (*ptr != 0x55)
					break;
				ptr++;
				i++;
			}

			if (*ptr == 0xd5) {
				
				work->packet_ptr.s.addr += i + 1;
				work->len -= i + 5;
			} else if ((*ptr & 0xf) == 0xd) {
				
				work->packet_ptr.s.addr += i;
				work->len -= i + 4;
				for (i = 0; i < work->len; i++) {
					*ptr =
					    ((*ptr & 0xf0) >> 4) |
					    ((*(ptr + 1) & 0xf) << 4);
					ptr++;
				}
			} else {
				DEBUGPRINT("Port %d unknown preamble, packet "
					   "dropped\n",
				     work->ipprt);
				
				cvm_oct_free_work(work);
				return 1;
			}
		}
	} else {
		DEBUGPRINT("Port %d receive error code %d, packet dropped\n",
			   work->ipprt, work->word2.snoip.err_code);
		cvm_oct_free_work(work);
		return 1;
	}

	return 0;
}


void cvm_oct_tasklet_rx(unsigned long unused)
{
	const int coreid = cvmx_get_core_num();
	uint64_t old_group_mask;
	uint64_t old_scratch;
	int rx_count = 0;
	int number_to_free;
	int num_freed;
	int packet_not_copied;

	
	prefetch(cvm_oct_device);

	if (USE_ASYNC_IOBDMA) {
		
		CVMX_SYNCIOBDMA;
		old_scratch = cvmx_scratch_read64(CVMX_SCR_SCRATCH);
	}

	
	old_group_mask = cvmx_read_csr(CVMX_POW_PP_GRP_MSKX(coreid));
	cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(coreid),
		       (old_group_mask & ~0xFFFFull) | 1 << pow_receive_group);

	if (USE_ASYNC_IOBDMA)
		cvmx_pow_work_request_async(CVMX_SCR_SCRATCH, CVMX_POW_NO_WAIT);

	while (1) {
		struct sk_buff *skb = NULL;
		int skb_in_hw;
		cvmx_wqe_t *work;

		if (USE_ASYNC_IOBDMA) {
			work = cvmx_pow_work_response_async(CVMX_SCR_SCRATCH);
		} else {
			if ((INTERRUPT_LIMIT == 0)
			    || likely(rx_count < MAX_RX_PACKETS))
				work =
				    cvmx_pow_work_request_sync
				    (CVMX_POW_NO_WAIT);
			else
				work = NULL;
		}
		prefetch(work);
		if (work == NULL)
			break;

		
		if (USE_ASYNC_IOBDMA) {

			if ((INTERRUPT_LIMIT == 0)
			    || likely(rx_count < MAX_RX_PACKETS))
				cvmx_pow_work_request_async_nocheck
				    (CVMX_SCR_SCRATCH, CVMX_POW_NO_WAIT);
			else {
				cvmx_scratch_write64(CVMX_SCR_SCRATCH,
						     0x8000000000000000ull);
				cvmx_pow_tag_sw_null_nocheck();
			}
		}

		skb_in_hw = USE_SKBUFFS_IN_HW && work->word2.s.bufs == 1;
		if (likely(skb_in_hw)) {
			skb =
			    *(struct sk_buff
			      **)(cvm_oct_get_buffer_ptr(work->packet_ptr) -
				  sizeof(void *));
			prefetch(&skb->head);
			prefetch(&skb->len);
		}
		prefetch(cvm_oct_device[work->ipprt]);

		rx_count++;
		
		if (unlikely(work->word2.snoip.rcv_error)) {
			if (cvm_oct_check_rcv_error(work))
				continue;
		}

		
		if (likely(skb_in_hw)) {
			
			skb->data =
			    skb->head + work->packet_ptr.s.addr -
			    cvmx_ptr_to_phys(skb->head);
			prefetch(skb->data);
			skb->len = work->len;
			skb_set_tail_pointer(skb, skb->len);
			packet_not_copied = 1;
		} else {

			
			skb = dev_alloc_skb(work->len);
			if (!skb) {
				DEBUGPRINT("Port %d failed to allocate "
					   "skbuff, packet dropped\n",
				     work->ipprt);
				cvm_oct_free_work(work);
				continue;
			}

			
			if (unlikely(work->word2.s.bufs == 0)) {
				uint8_t *ptr = work->packet_data;

				if (likely(!work->word2.s.not_IP)) {
					
					if (work->word2.s.is_v6)
						ptr += 2;
					else
						ptr += 6;
				}
				memcpy(skb_put(skb, work->len), ptr, work->len);
				
			} else {
				int segments = work->word2.s.bufs;
				union cvmx_buf_ptr segment_ptr =
					work->packet_ptr;
				int len = work->len;

				while (segments--) {
					union cvmx_buf_ptr next_ptr =
					    *(union cvmx_buf_ptr *)
					    cvmx_phys_to_ptr(segment_ptr.s.
							     addr - 8);
			
					int segment_size =
					    CVMX_FPA_PACKET_POOL_SIZE -
					    (segment_ptr.s.addr -
					     (((segment_ptr.s.addr >> 7) -
					       segment_ptr.s.back) << 7));
					
					if (segment_size > len)
						segment_size = len;
					
					memcpy(skb_put(skb, segment_size),
					       cvmx_phys_to_ptr(segment_ptr.s.
								addr),
					       segment_size);
					
					len -= segment_size;
					segment_ptr = next_ptr;
				}
			}
			packet_not_copied = 0;
		}

		if (likely((work->ipprt < TOTAL_NUMBER_OF_PORTS) &&
			   cvm_oct_device[work->ipprt])) {
			struct net_device *dev = cvm_oct_device[work->ipprt];
			struct octeon_ethernet *priv = netdev_priv(dev);

			
			if (likely(dev->flags & IFF_UP)) {
				skb->protocol = eth_type_trans(skb, dev);
				skb->dev = dev;

				if (unlikely
				    (work->word2.s.not_IP
				     || work->word2.s.IP_exc
				     || work->word2.s.L4_error))
					skb->ip_summed = CHECKSUM_NONE;
				else
					skb->ip_summed = CHECKSUM_UNNECESSARY;

				
				if (work->ipprt >= CVMX_PIP_NUM_INPUT_PORTS) {
#ifdef CONFIG_64BIT
					atomic64_add(1, (atomic64_t *)&priv->stats.rx_packets);
					atomic64_add(skb->len, (atomic64_t *)&priv->stats.rx_bytes);
#else
					atomic_add(1, (atomic_t *)&priv->stats.rx_packets);
					atomic_add(skb->len, (atomic_t *)&priv->stats.rx_bytes);
#endif
				}
				netif_receive_skb(skb);
			} else {
				
				
#ifdef CONFIG_64BIT
				atomic64_add(1, (atomic64_t *)&priv->stats.rx_dropped);
#else
				atomic_add(1, (atomic_t *)&priv->stats.rx_dropped);
#endif
				dev_kfree_skb_irq(skb);
			}
		} else {
			
			DEBUGPRINT("Port %d not controlled by Linux, packet "
				   "dropped\n",
			     work->ipprt);
			dev_kfree_skb_irq(skb);
		}
		
		if (USE_SKBUFFS_IN_HW && likely(packet_not_copied)) {
			
			cvmx_fau_atomic_add32(FAU_NUM_PACKET_BUFFERS_TO_FREE,
					      1);

			cvmx_fpa_free(work, CVMX_FPA_WQE_POOL,
				      DONT_WRITEBACK(1));
		} else {
			cvm_oct_free_work(work);
		}
	}

	
	cvmx_write_csr(CVMX_POW_PP_GRP_MSKX(coreid), old_group_mask);
	if (USE_ASYNC_IOBDMA) {
		
		cvmx_scratch_write64(CVMX_SCR_SCRATCH, old_scratch);
	}

	if (USE_SKBUFFS_IN_HW) {
		
		number_to_free =
		    cvmx_fau_fetch_and_add32(FAU_NUM_PACKET_BUFFERS_TO_FREE, 0);

		if (number_to_free > 0) {
			cvmx_fau_atomic_add32(FAU_NUM_PACKET_BUFFERS_TO_FREE,
					      -number_to_free);
			num_freed =
			    cvm_oct_mem_fill_fpa(CVMX_FPA_PACKET_POOL,
						 CVMX_FPA_PACKET_POOL_SIZE,
						 number_to_free);
			if (num_freed != number_to_free) {
				cvmx_fau_atomic_add32
				    (FAU_NUM_PACKET_BUFFERS_TO_FREE,
				     number_to_free - num_freed);
			}
		}
	}
}

void cvm_oct_rx_initialize(void)
{
	int i;
	
	for (i = 0; i < NR_CPUS; i++)
		tasklet_init(&cvm_oct_tasklet[i].t, cvm_oct_tasklet_rx, 0);
}

void cvm_oct_rx_shutdown(void)
{
	int i;
	
	for (i = 0; i < NR_CPUS; i++)
		tasklet_kill(&cvm_oct_tasklet[i].t);
}
