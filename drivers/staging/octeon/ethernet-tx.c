
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/string.h>
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
#include "ethernet-tx.h"
#include "ethernet-util.h"

#include "cvmx-wqe.h"
#include "cvmx-fau.h"
#include "cvmx-pko.h"
#include "cvmx-helper.h"

#include "cvmx-gmxx-defs.h"


#ifndef GET_SKBUFF_QOS
#define GET_SKBUFF_QOS(skb) 0
#endif


int cvm_oct_xmit(struct sk_buff *skb, struct net_device *dev)
{
	cvmx_pko_command_word0_t pko_command;
	union cvmx_buf_ptr hw_buffer;
	uint64_t old_scratch;
	uint64_t old_scratch2;
	int dropped;
	int qos;
	int queue_it_up;
	struct octeon_ethernet *priv = netdev_priv(dev);
	int32_t skb_to_free;
	int32_t undo;
	int32_t buffers_to_free;
#if REUSE_SKBUFFS_WITHOUT_FREE
	unsigned char *fpa_head;
#endif

	
	prefetch(priv);

	
	dropped = 0;

	
	if ((CVMX_PKO_QUEUES_PER_PORT_INTERFACE0 > 1) ||
	    (CVMX_PKO_QUEUES_PER_PORT_INTERFACE1 > 1)) {
		qos = GET_SKBUFF_QOS(skb);
		if (qos <= 0)
			qos = 0;
		else if (qos >= cvmx_pko_get_num_queues(priv->port))
			qos = 0;
	} else
		qos = 0;

	if (USE_ASYNC_IOBDMA) {
		
		CVMX_SYNCIOBDMA;
		old_scratch = cvmx_scratch_read64(CVMX_SCR_SCRATCH);
		old_scratch2 = cvmx_scratch_read64(CVMX_SCR_SCRATCH + 8);

		
		cvmx_fau_async_fetch_and_add32(CVMX_SCR_SCRATCH + 8,
					       FAU_NUM_PACKET_BUFFERS_TO_FREE,
					       0);
		cvmx_fau_async_fetch_and_add32(CVMX_SCR_SCRATCH,
					       priv->fau + qos * 4,
					       MAX_SKB_TO_FREE);
	}

	
	if ((skb->len < 64) && OCTEON_IS_MODEL(OCTEON_CN3XXX)) {
		union cvmx_gmxx_prtx_cfg gmx_prt_cfg;
		int interface = INTERFACE(priv->port);
		int index = INDEX(priv->port);

		if (interface < 2) {
			
			gmx_prt_cfg.u64 =
			    cvmx_read_csr(CVMX_GMXX_PRTX_CFG(index, interface));
			if (gmx_prt_cfg.s.duplex == 0) {
				int add_bytes = 64 - skb->len;
				if ((skb_tail_pointer(skb) + add_bytes) <=
				    skb_end_pointer(skb))
					memset(__skb_put(skb, add_bytes), 0,
					       add_bytes);
			}
		}
	}

	
	hw_buffer.u64 = 0;
	hw_buffer.s.addr = cvmx_ptr_to_phys(skb->data);
	hw_buffer.s.pool = 0;
	hw_buffer.s.size =
	    (unsigned long)skb_end_pointer(skb) - (unsigned long)skb->head;

	
	pko_command.u64 = 0;
	pko_command.s.n2 = 1;	
	pko_command.s.segs = 1;
	pko_command.s.total_bytes = skb->len;
	pko_command.s.size0 = CVMX_FAU_OP_SIZE_32;
	pko_command.s.subone0 = 1;

	pko_command.s.dontfree = 1;
	pko_command.s.reg0 = priv->fau + qos * 4;
	
#if REUSE_SKBUFFS_WITHOUT_FREE
	fpa_head = skb->head + 128 - ((unsigned long)skb->head & 0x7f);
	if (unlikely(skb->data < fpa_head)) {
		
		goto dont_put_skbuff_in_hw;
	}
	if (unlikely
	    ((skb_end_pointer(skb) - fpa_head) < CVMX_FPA_PACKET_POOL_SIZE)) {
		
		goto dont_put_skbuff_in_hw;
	}
	if (unlikely(skb_shared(skb))) {
		
		goto dont_put_skbuff_in_hw;
	}
	if (unlikely(skb_cloned(skb))) {
		
		goto dont_put_skbuff_in_hw;
	}
	if (unlikely(skb_header_cloned(skb))) {
		
		goto dont_put_skbuff_in_hw;
	}
	if (unlikely(skb->destructor)) {
		
		goto dont_put_skbuff_in_hw;
	}
	if (unlikely(skb_shinfo(skb)->nr_frags)) {
		
		goto dont_put_skbuff_in_hw;
	}
	if (unlikely
	    (skb->truesize !=
	     sizeof(*skb) + skb_end_pointer(skb) - skb->head)) {
		
		goto dont_put_skbuff_in_hw;
	}

	
	pko_command.s.reg0 = 0;
	pko_command.s.dontfree = 0;

	hw_buffer.s.back = (skb->data - fpa_head) >> 7;
	*(struct sk_buff **)(fpa_head - sizeof(void *)) = skb;

	
	dst_release(skb_dst(skb));
	skb_dst_set(skb, NULL);
#ifdef CONFIG_XFRM
	secpath_put(skb->sp);
	skb->sp = NULL;
#endif
	nf_reset(skb);

#ifdef CONFIG_NET_SCHED
	skb->tc_index = 0;
#ifdef CONFIG_NET_CLS_ACT
	skb->tc_verd = 0;
#endif 
#endif 

dont_put_skbuff_in_hw:
#endif 

	
	if (USE_HW_TCPUDP_CHECKSUM && (skb->protocol == htons(ETH_P_IP)) &&
	    (ip_hdr(skb)->version == 4) && (ip_hdr(skb)->ihl == 5) &&
	    ((ip_hdr(skb)->frag_off == 0) || (ip_hdr(skb)->frag_off == 1 << 14))
	    && ((ip_hdr(skb)->protocol == IP_PROTOCOL_TCP)
		|| (ip_hdr(skb)->protocol == IP_PROTOCOL_UDP))) {
		
		pko_command.s.ipoffp1 = sizeof(struct ethhdr) + 1;
	}

	if (USE_ASYNC_IOBDMA) {
		
		CVMX_SYNCIOBDMA;
		skb_to_free = cvmx_scratch_read64(CVMX_SCR_SCRATCH);
		buffers_to_free = cvmx_scratch_read64(CVMX_SCR_SCRATCH + 8);
	} else {
		
		skb_to_free = cvmx_fau_fetch_and_add32(priv->fau + qos * 4,
						       MAX_SKB_TO_FREE);
		buffers_to_free =
		    cvmx_fau_fetch_and_add32(FAU_NUM_PACKET_BUFFERS_TO_FREE, 0);
	}

	
	undo = skb_to_free > 0 ?
		MAX_SKB_TO_FREE : skb_to_free + MAX_SKB_TO_FREE;
	if (undo > 0)
		cvmx_fau_atomic_add32(priv->fau+qos*4, -undo);
	skb_to_free = -skb_to_free > MAX_SKB_TO_FREE ?
		MAX_SKB_TO_FREE : -skb_to_free;

	
	if ((buffers_to_free < -100) && !pko_command.s.dontfree) {
		pko_command.s.dontfree = 1;
		pko_command.s.reg0 = priv->fau + qos * 4;
	}

	cvmx_pko_send_packet_prepare(priv->port, priv->queue + qos,
				     CVMX_PKO_LOCK_CMD_QUEUE);

	
	if (unlikely
	    (skb_queue_len(&priv->tx_free_list[qos]) >= MAX_OUT_QUEUE_DEPTH)) {
		
		dropped = 1;
	}
	
	else if (unlikely
		 (cvmx_pko_send_packet_finish
		  (priv->port, priv->queue + qos, pko_command, hw_buffer,
		   CVMX_PKO_LOCK_CMD_QUEUE))) {
		DEBUGPRINT("%s: Failed to send the packet\n", dev->name);
		dropped = 1;
	}

	if (USE_ASYNC_IOBDMA) {
		
		cvmx_scratch_write64(CVMX_SCR_SCRATCH, old_scratch);
		cvmx_scratch_write64(CVMX_SCR_SCRATCH + 8, old_scratch2);
	}

	queue_it_up = 0;
	if (unlikely(dropped)) {
		dev_kfree_skb_any(skb);
		priv->stats.tx_dropped++;
	} else {
		if (USE_SKBUFFS_IN_HW) {
			
			if (pko_command.s.dontfree)
				queue_it_up = 1;
			else
				cvmx_fau_atomic_add32
				    (FAU_NUM_PACKET_BUFFERS_TO_FREE, -1);
		} else {
			
			queue_it_up = 1;
		}
	}

	if (queue_it_up) {
		spin_lock(&priv->tx_free_list[qos].lock);
		__skb_queue_tail(&priv->tx_free_list[qos], skb);
		cvm_oct_free_tx_skbs(priv, skb_to_free, qos, 0);
		spin_unlock(&priv->tx_free_list[qos].lock);
	} else {
		cvm_oct_free_tx_skbs(priv, skb_to_free, qos, 1);
	}

	return 0;
}


int cvm_oct_xmit_pow(struct sk_buff *skb, struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	void *packet_buffer;
	void *copy_location;

	
	cvmx_wqe_t *work = cvmx_fpa_alloc(CVMX_FPA_WQE_POOL);
	if (unlikely(work == NULL)) {
		DEBUGPRINT("%s: Failed to allocate a work queue entry\n",
			   dev->name);
		priv->stats.tx_dropped++;
		dev_kfree_skb(skb);
		return 0;
	}

	
	packet_buffer = cvmx_fpa_alloc(CVMX_FPA_PACKET_POOL);
	if (unlikely(packet_buffer == NULL)) {
		DEBUGPRINT("%s: Failed to allocate a packet buffer\n",
			   dev->name);
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, DONT_WRITEBACK(1));
		priv->stats.tx_dropped++;
		dev_kfree_skb(skb);
		return 0;
	}

	
	copy_location = packet_buffer + sizeof(uint64_t);
	copy_location += ((CVMX_HELPER_FIRST_MBUFF_SKIP + 7) & 0xfff8) + 6;

	
	memcpy(copy_location, skb->data, skb->len);

	
	work->hw_chksum = skb->csum;
	work->len = skb->len;
	work->ipprt = priv->port;
	work->qos = priv->port & 0x7;
	work->grp = pow_send_group;
	work->tag_type = CVMX_HELPER_INPUT_TAG_TYPE;
	work->tag = pow_send_group;	
	
	work->word2.u64 = 0;
	work->word2.s.bufs = 1;
	work->packet_ptr.u64 = 0;
	work->packet_ptr.s.addr = cvmx_ptr_to_phys(copy_location);
	work->packet_ptr.s.pool = CVMX_FPA_PACKET_POOL;
	work->packet_ptr.s.size = CVMX_FPA_PACKET_POOL_SIZE;
	work->packet_ptr.s.back = (copy_location - packet_buffer) >> 7;

	if (skb->protocol == htons(ETH_P_IP)) {
		work->word2.s.ip_offset = 14;
#if 0
		work->word2.s.vlan_valid = 0;	
		work->word2.s.vlan_cfi = 0;	
		work->word2.s.vlan_id = 0;	
		work->word2.s.dec_ipcomp = 0;	
#endif
		work->word2.s.tcp_or_udp =
		    (ip_hdr(skb)->protocol == IP_PROTOCOL_TCP)
		    || (ip_hdr(skb)->protocol == IP_PROTOCOL_UDP);
#if 0
		
		work->word2.s.dec_ipsec = 0;
		
		work->word2.s.is_v6 = 0;
		
		work->word2.s.software = 0;
		
		work->word2.s.L4_error = 0;
#endif
		work->word2.s.is_frag = !((ip_hdr(skb)->frag_off == 0)
					  || (ip_hdr(skb)->frag_off ==
					      1 << 14));
#if 0
		
		work->word2.s.IP_exc = 0;
#endif
		work->word2.s.is_bcast = (skb->pkt_type == PACKET_BROADCAST);
		work->word2.s.is_mcast = (skb->pkt_type == PACKET_MULTICAST);
#if 0
		
		work->word2.s.not_IP = 0;
		
		work->word2.s.rcv_error = 0;
		
		work->word2.s.err_code = 0;
#endif

		
		memcpy(work->packet_data, skb->data + 10,
		       sizeof(work->packet_data));
	} else {
#if 0
		work->word2.snoip.vlan_valid = 0;	
		work->word2.snoip.vlan_cfi = 0;	
		work->word2.snoip.vlan_id = 0;	
		work->word2.snoip.software = 0;	
#endif
		work->word2.snoip.is_rarp = skb->protocol == htons(ETH_P_RARP);
		work->word2.snoip.is_arp = skb->protocol == htons(ETH_P_ARP);
		work->word2.snoip.is_bcast =
		    (skb->pkt_type == PACKET_BROADCAST);
		work->word2.snoip.is_mcast =
		    (skb->pkt_type == PACKET_MULTICAST);
		work->word2.snoip.not_IP = 1;	
#if 0
		
		work->word2.snoip.rcv_error = 0;
		
		work->word2.snoip.err_code = 0;
#endif
		memcpy(work->packet_data, skb->data, sizeof(work->packet_data));
	}

	
	cvmx_pow_work_submit(work, work->tag, work->tag_type, work->qos,
			     work->grp);
	priv->stats.tx_packets++;
	priv->stats.tx_bytes += skb->len;
	dev_kfree_skb(skb);
	return 0;
}


int cvm_oct_transmit_qos(struct net_device *dev, void *work_queue_entry,
			 int do_free, int qos)
{
	unsigned long flags;
	union cvmx_buf_ptr hw_buffer;
	cvmx_pko_command_word0_t pko_command;
	int dropped;
	struct octeon_ethernet *priv = netdev_priv(dev);
	cvmx_wqe_t *work = work_queue_entry;

	if (!(dev->flags & IFF_UP)) {
		DEBUGPRINT("%s: Device not up\n", dev->name);
		if (do_free)
			cvm_oct_free_work(work);
		return -1;
	}

	
	if ((CVMX_PKO_QUEUES_PER_PORT_INTERFACE0 > 1) ||
	    (CVMX_PKO_QUEUES_PER_PORT_INTERFACE1 > 1)) {
		if (qos <= 0)
			qos = 0;
		else if (qos >= cvmx_pko_get_num_queues(priv->port))
			qos = 0;
	} else
		qos = 0;

	
	dropped = 0;

	local_irq_save(flags);
	cvmx_pko_send_packet_prepare(priv->port, priv->queue + qos,
				     CVMX_PKO_LOCK_CMD_QUEUE);

	
	hw_buffer.u64 = 0;
	hw_buffer.s.addr = work->packet_ptr.s.addr;
	hw_buffer.s.pool = CVMX_FPA_PACKET_POOL;
	hw_buffer.s.size = CVMX_FPA_PACKET_POOL_SIZE;
	hw_buffer.s.back = work->packet_ptr.s.back;

	
	pko_command.u64 = 0;
	pko_command.s.n2 = 1;	
	pko_command.s.dontfree = !do_free;
	pko_command.s.segs = work->word2.s.bufs;
	pko_command.s.total_bytes = work->len;

	
	if (unlikely(work->word2.s.not_IP || work->word2.s.IP_exc))
		pko_command.s.ipoffp1 = 0;
	else
		pko_command.s.ipoffp1 = sizeof(struct ethhdr) + 1;

	
	if (unlikely
	    (cvmx_pko_send_packet_finish
	     (priv->port, priv->queue + qos, pko_command, hw_buffer,
	      CVMX_PKO_LOCK_CMD_QUEUE))) {
		DEBUGPRINT("%s: Failed to send the packet\n", dev->name);
		dropped = -1;
	}
	local_irq_restore(flags);

	if (unlikely(dropped)) {
		if (do_free)
			cvm_oct_free_work(work);
		priv->stats.tx_dropped++;
	} else if (do_free)
		cvmx_fpa_free(work, CVMX_FPA_WQE_POOL, DONT_WRITEBACK(1));

	return dropped;
}
EXPORT_SYMBOL(cvm_oct_transmit_qos);


void cvm_oct_tx_shutdown(struct net_device *dev)
{
	struct octeon_ethernet *priv = netdev_priv(dev);
	unsigned long flags;
	int qos;

	for (qos = 0; qos < 16; qos++) {
		spin_lock_irqsave(&priv->tx_free_list[qos].lock, flags);
		while (skb_queue_len(&priv->tx_free_list[qos]))
			dev_kfree_skb_any(__skb_dequeue
					  (&priv->tx_free_list[qos]));
		spin_unlock_irqrestore(&priv->tx_free_list[qos].lock, flags);
	}
}
