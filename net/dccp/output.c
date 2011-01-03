

#include <linux/dccp.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>

#include <net/inet_sock.h>
#include <net/sock.h>

#include "ackvec.h"
#include "ccid.h"
#include "dccp.h"

static inline void dccp_event_ack_sent(struct sock *sk)
{
	inet_csk_clear_xmit_timer(sk, ICSK_TIME_DACK);
}

static void dccp_skb_entail(struct sock *sk, struct sk_buff *skb)
{
	skb_set_owner_w(skb, sk);
	WARN_ON(sk->sk_send_head);
	sk->sk_send_head = skb;
}


static int dccp_transmit_skb(struct sock *sk, struct sk_buff *skb)
{
	if (likely(skb != NULL)) {
		const struct inet_sock *inet = inet_sk(sk);
		const struct inet_connection_sock *icsk = inet_csk(sk);
		struct dccp_sock *dp = dccp_sk(sk);
		struct dccp_skb_cb *dcb = DCCP_SKB_CB(skb);
		struct dccp_hdr *dh;
		
		const u32 dccp_header_size = sizeof(*dh) +
					     sizeof(struct dccp_hdr_ext) +
					  dccp_packet_hdr_len(dcb->dccpd_type);
		int err, set_ack = 1;
		u64 ackno = dp->dccps_gsr;
		
		dcb->dccpd_seq = ADD48(dp->dccps_gss, 1);

		switch (dcb->dccpd_type) {
		case DCCP_PKT_DATA:
			set_ack = 0;
			
		case DCCP_PKT_DATAACK:
		case DCCP_PKT_RESET:
			break;

		case DCCP_PKT_REQUEST:
			set_ack = 0;
			
			if (icsk->icsk_retransmits == 0)
				dcb->dccpd_seq = dp->dccps_iss;
			

		case DCCP_PKT_SYNC:
		case DCCP_PKT_SYNCACK:
			ackno = dcb->dccpd_ack_seq;
			
		default:
			
			WARN_ON(skb->sk);
			skb_set_owner_w(skb, sk);
			break;
		}

		if (dccp_insert_options(sk, skb)) {
			kfree_skb(skb);
			return -EPROTO;
		}


		
		dh = dccp_zeroed_hdr(skb, dccp_header_size);
		dh->dccph_type	= dcb->dccpd_type;
		dh->dccph_sport	= inet->sport;
		dh->dccph_dport	= inet->dport;
		dh->dccph_doff	= (dccp_header_size + dcb->dccpd_opt_len) / 4;
		dh->dccph_ccval	= dcb->dccpd_ccval;
		dh->dccph_cscov = dp->dccps_pcslen;
		
		dh->dccph_x	= 1;

		dccp_update_gss(sk, dcb->dccpd_seq);
		dccp_hdr_set_seq(dh, dp->dccps_gss);
		if (set_ack)
			dccp_hdr_set_ack(dccp_hdr_ack_bits(skb), ackno);

		switch (dcb->dccpd_type) {
		case DCCP_PKT_REQUEST:
			dccp_hdr_request(skb)->dccph_req_service =
							dp->dccps_service;
			
			dp->dccps_awl = dp->dccps_iss;
			break;
		case DCCP_PKT_RESET:
			dccp_hdr_reset(skb)->dccph_reset_code =
							dcb->dccpd_reset_code;
			break;
		}

		icsk->icsk_af_ops->send_check(sk, 0, skb);

		if (set_ack)
			dccp_event_ack_sent(sk);

		DCCP_INC_STATS(DCCP_MIB_OUTSEGS);

		err = icsk->icsk_af_ops->queue_xmit(skb, 0);
		return net_xmit_eval(err);
	}
	return -ENOBUFS;
}


static u32 dccp_determine_ccmps(const struct dccp_sock *dp)
{
	const struct ccid *tx_ccid = dp->dccps_hc_tx_ccid;

	if (tx_ccid == NULL || tx_ccid->ccid_ops == NULL)
		return 0;
	return tx_ccid->ccid_ops->ccid_ccmps;
}

unsigned int dccp_sync_mss(struct sock *sk, u32 pmtu)
{
	struct inet_connection_sock *icsk = inet_csk(sk);
	struct dccp_sock *dp = dccp_sk(sk);
	u32 ccmps = dccp_determine_ccmps(dp);
	u32 cur_mps = ccmps ? min(pmtu, ccmps) : pmtu;

	
	cur_mps -= (icsk->icsk_af_ops->net_header_len + icsk->icsk_ext_hdr_len +
		    sizeof(struct dccp_hdr) + sizeof(struct dccp_hdr_ext));

	
	cur_mps -= roundup(1 + 6 + 10 + dp->dccps_send_ndp_count * 8 + 6 +
			   (dp->dccps_hc_rx_ackvec ? DCCPAV_MIN_OPTLEN : 0), 4);

	
	icsk->icsk_pmtu_cookie = pmtu;
	dp->dccps_mss_cache = cur_mps;

	return cur_mps;
}

EXPORT_SYMBOL_GPL(dccp_sync_mss);

void dccp_write_space(struct sock *sk)
{
	read_lock(&sk->sk_callback_lock);

	if (sk_has_sleeper(sk))
		wake_up_interruptible(sk->sk_sleep);
	
	if (sock_writeable(sk))
		sk_wake_async(sk, SOCK_WAKE_SPACE, POLL_OUT);

	read_unlock(&sk->sk_callback_lock);
}


static int dccp_wait_for_ccid(struct sock *sk, struct sk_buff *skb, int delay)
{
	struct dccp_sock *dp = dccp_sk(sk);
	DEFINE_WAIT(wait);
	unsigned long jiffdelay;
	int rc;

	do {
		dccp_pr_debug("delayed send by %d msec\n", delay);
		jiffdelay = msecs_to_jiffies(delay);

		prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);

		sk->sk_write_pending++;
		release_sock(sk);
		schedule_timeout(jiffdelay);
		lock_sock(sk);
		sk->sk_write_pending--;

		if (sk->sk_err)
			goto do_error;
		if (signal_pending(current))
			goto do_interrupted;

		rc = ccid_hc_tx_send_packet(dp->dccps_hc_tx_ccid, sk, skb);
	} while ((delay = rc) > 0);
out:
	finish_wait(sk->sk_sleep, &wait);
	return rc;

do_error:
	rc = -EPIPE;
	goto out;
do_interrupted:
	rc = -EINTR;
	goto out;
}

void dccp_write_xmit(struct sock *sk, int block)
{
	struct dccp_sock *dp = dccp_sk(sk);
	struct sk_buff *skb;

	while ((skb = skb_peek(&sk->sk_write_queue))) {
		int err = ccid_hc_tx_send_packet(dp->dccps_hc_tx_ccid, sk, skb);

		if (err > 0) {
			if (!block) {
				sk_reset_timer(sk, &dp->dccps_xmit_timer,
						msecs_to_jiffies(err)+jiffies);
				break;
			} else
				err = dccp_wait_for_ccid(sk, skb, err);
			if (err && err != -EINTR)
				DCCP_BUG("err=%d after dccp_wait_for_ccid", err);
		}

		skb_dequeue(&sk->sk_write_queue);
		if (err == 0) {
			struct dccp_skb_cb *dcb = DCCP_SKB_CB(skb);
			const int len = skb->len;

			if (sk->sk_state == DCCP_PARTOPEN) {
				const u32 cur_mps = dp->dccps_mss_cache - DCCP_FEATNEG_OVERHEAD;
				
				if (!list_empty(&dp->dccps_featneg) && len > cur_mps) {
					DCCP_WARN("Payload too large (%d) for featneg.\n", len);
					dccp_send_ack(sk);
					dccp_feat_list_purge(&dp->dccps_featneg);
				}

				inet_csk_schedule_ack(sk);
				inet_csk_reset_xmit_timer(sk, ICSK_TIME_DACK,
						  inet_csk(sk)->icsk_rto,
						  DCCP_RTO_MAX);
				dcb->dccpd_type = DCCP_PKT_DATAACK;
			} else if (dccp_ack_pending(sk))
				dcb->dccpd_type = DCCP_PKT_DATAACK;
			else
				dcb->dccpd_type = DCCP_PKT_DATA;

			err = dccp_transmit_skb(sk, skb);
			ccid_hc_tx_packet_sent(dp->dccps_hc_tx_ccid, sk, 0, len);
			if (err)
				DCCP_BUG("err=%d after ccid_hc_tx_packet_sent",
					 err);
		} else {
			dccp_pr_debug("packet discarded due to err=%d\n", err);
			kfree_skb(skb);
		}
	}
}


int dccp_retransmit_skb(struct sock *sk)
{
	WARN_ON(sk->sk_send_head == NULL);

	if (inet_csk(sk)->icsk_af_ops->rebuild_header(sk) != 0)
		return -EHOSTUNREACH; 

	
	inet_csk(sk)->icsk_retransmits++;

	return dccp_transmit_skb(sk, skb_clone(sk->sk_send_head, GFP_ATOMIC));
}

struct sk_buff *dccp_make_response(struct sock *sk, struct dst_entry *dst,
				   struct request_sock *req)
{
	struct dccp_hdr *dh;
	struct dccp_request_sock *dreq;
	const u32 dccp_header_size = sizeof(struct dccp_hdr) +
				     sizeof(struct dccp_hdr_ext) +
				     sizeof(struct dccp_hdr_response);
	struct sk_buff *skb = sock_wmalloc(sk, sk->sk_prot->max_header, 1,
					   GFP_ATOMIC);
	if (skb == NULL)
		return NULL;

	
	skb_reserve(skb, sk->sk_prot->max_header);

	skb_dst_set(skb, dst_clone(dst));

	dreq = dccp_rsk(req);
	if (inet_rsk(req)->acked)	
		dccp_inc_seqno(&dreq->dreq_iss);
	DCCP_SKB_CB(skb)->dccpd_type = DCCP_PKT_RESPONSE;
	DCCP_SKB_CB(skb)->dccpd_seq  = dreq->dreq_iss;

	
	if (dccp_feat_server_ccid_dependencies(dreq))
		goto response_failed;

	if (dccp_insert_options_rsk(dreq, skb))
		goto response_failed;

	
	dh = dccp_zeroed_hdr(skb, dccp_header_size);

	dh->dccph_sport	= inet_rsk(req)->loc_port;
	dh->dccph_dport	= inet_rsk(req)->rmt_port;
	dh->dccph_doff	= (dccp_header_size +
			   DCCP_SKB_CB(skb)->dccpd_opt_len) / 4;
	dh->dccph_type	= DCCP_PKT_RESPONSE;
	dh->dccph_x	= 1;
	dccp_hdr_set_seq(dh, dreq->dreq_iss);
	dccp_hdr_set_ack(dccp_hdr_ack_bits(skb), dreq->dreq_isr);
	dccp_hdr_response(skb)->dccph_resp_service = dreq->dreq_service;

	dccp_csum_outgoing(skb);

	
	inet_rsk(req)->acked = 1;
	DCCP_INC_STATS(DCCP_MIB_OUTSEGS);
	return skb;
response_failed:
	kfree_skb(skb);
	return NULL;
}

EXPORT_SYMBOL_GPL(dccp_make_response);


struct sk_buff *dccp_ctl_make_reset(struct sock *sk, struct sk_buff *rcv_skb)
{
	struct dccp_hdr *rxdh = dccp_hdr(rcv_skb), *dh;
	struct dccp_skb_cb *dcb = DCCP_SKB_CB(rcv_skb);
	const u32 dccp_hdr_reset_len = sizeof(struct dccp_hdr) +
				       sizeof(struct dccp_hdr_ext) +
				       sizeof(struct dccp_hdr_reset);
	struct dccp_hdr_reset *dhr;
	struct sk_buff *skb;

	skb = alloc_skb(sk->sk_prot->max_header, GFP_ATOMIC);
	if (skb == NULL)
		return NULL;

	skb_reserve(skb, sk->sk_prot->max_header);

	
	dh = dccp_zeroed_hdr(skb, dccp_hdr_reset_len);
	dh->dccph_type	= DCCP_PKT_RESET;
	dh->dccph_sport	= rxdh->dccph_dport;
	dh->dccph_dport	= rxdh->dccph_sport;
	dh->dccph_doff	= dccp_hdr_reset_len / 4;
	dh->dccph_x	= 1;

	dhr = dccp_hdr_reset(skb);
	dhr->dccph_reset_code = dcb->dccpd_reset_code;

	switch (dcb->dccpd_reset_code) {
	case DCCP_RESET_CODE_PACKET_ERROR:
		dhr->dccph_reset_data[0] = rxdh->dccph_type;
		break;
	case DCCP_RESET_CODE_OPTION_ERROR:	
	case DCCP_RESET_CODE_MANDATORY_ERROR:
		memcpy(dhr->dccph_reset_data, dcb->dccpd_reset_data, 3);
		break;
	}
	
	if (dcb->dccpd_ack_seq != DCCP_PKT_WITHOUT_ACK_SEQ)
		dccp_hdr_set_seq(dh, ADD48(dcb->dccpd_ack_seq, 1));
	dccp_hdr_set_ack(dccp_hdr_ack_bits(skb), dcb->dccpd_seq);

	dccp_csum_outgoing(skb);
	return skb;
}

EXPORT_SYMBOL_GPL(dccp_ctl_make_reset);


int dccp_send_reset(struct sock *sk, enum dccp_reset_codes code)
{
	struct sk_buff *skb;
	
	int err = inet_csk(sk)->icsk_af_ops->rebuild_header(sk);

	if (err != 0)
		return err;

	skb = sock_wmalloc(sk, sk->sk_prot->max_header, 1, GFP_ATOMIC);
	if (skb == NULL)
		return -ENOBUFS;

	
	skb_reserve(skb, sk->sk_prot->max_header);
	DCCP_SKB_CB(skb)->dccpd_type	   = DCCP_PKT_RESET;
	DCCP_SKB_CB(skb)->dccpd_reset_code = code;

	return dccp_transmit_skb(sk, skb);
}


static inline void dccp_connect_init(struct sock *sk)
{
	struct dccp_sock *dp = dccp_sk(sk);
	struct dst_entry *dst = __sk_dst_get(sk);
	struct inet_connection_sock *icsk = inet_csk(sk);

	sk->sk_err = 0;
	sock_reset_flag(sk, SOCK_DONE);

	dccp_sync_mss(sk, dst_mtu(dst));

	
	dp->dccps_gar = dp->dccps_iss;

	icsk->icsk_retransmits = 0;
}

int dccp_connect(struct sock *sk)
{
	struct sk_buff *skb;
	struct inet_connection_sock *icsk = inet_csk(sk);

	
	if (dccp_feat_finalise_settings(dccp_sk(sk)))
		return -EPROTO;

	dccp_connect_init(sk);

	skb = alloc_skb(sk->sk_prot->max_header, sk->sk_allocation);
	if (unlikely(skb == NULL))
		return -ENOBUFS;

	
	skb_reserve(skb, sk->sk_prot->max_header);

	DCCP_SKB_CB(skb)->dccpd_type = DCCP_PKT_REQUEST;

	dccp_skb_entail(sk, skb);
	dccp_transmit_skb(sk, skb_clone(skb, GFP_KERNEL));
	DCCP_INC_STATS(DCCP_MIB_ACTIVEOPENS);

	
	inet_csk_reset_xmit_timer(sk, ICSK_TIME_RETRANS,
				  icsk->icsk_rto, DCCP_RTO_MAX);
	return 0;
}

EXPORT_SYMBOL_GPL(dccp_connect);

void dccp_send_ack(struct sock *sk)
{
	
	if (sk->sk_state != DCCP_CLOSED) {
		struct sk_buff *skb = alloc_skb(sk->sk_prot->max_header,
						GFP_ATOMIC);

		if (skb == NULL) {
			inet_csk_schedule_ack(sk);
			inet_csk(sk)->icsk_ack.ato = TCP_ATO_MIN;
			inet_csk_reset_xmit_timer(sk, ICSK_TIME_DACK,
						  TCP_DELACK_MAX,
						  DCCP_RTO_MAX);
			return;
		}

		
		skb_reserve(skb, sk->sk_prot->max_header);
		DCCP_SKB_CB(skb)->dccpd_type = DCCP_PKT_ACK;
		dccp_transmit_skb(sk, skb);
	}
}

EXPORT_SYMBOL_GPL(dccp_send_ack);

#if 0

void dccp_send_delayed_ack(struct sock *sk)
{
	struct inet_connection_sock *icsk = inet_csk(sk);
	
	unsigned long timeout = jiffies + 2 * HZ;

	
	if (icsk->icsk_ack.pending & ICSK_ACK_TIMER) {
		
		if (icsk->icsk_ack.blocked) {
			dccp_send_ack(sk);
			return;
		}

		if (!time_before(timeout, icsk->icsk_ack.timeout))
			timeout = icsk->icsk_ack.timeout;
	}
	icsk->icsk_ack.pending |= ICSK_ACK_SCHED | ICSK_ACK_TIMER;
	icsk->icsk_ack.timeout = timeout;
	sk_reset_timer(sk, &icsk->icsk_delack_timer, timeout);
}
#endif

void dccp_send_sync(struct sock *sk, const u64 ackno,
		    const enum dccp_pkt_type pkt_type)
{
	
	struct sk_buff *skb = alloc_skb(sk->sk_prot->max_header, GFP_ATOMIC);

	if (skb == NULL) {
		
		DCCP_CRIT("could not send %s", dccp_packet_name(pkt_type));
		return;
	}

	
	skb_reserve(skb, sk->sk_prot->max_header);
	DCCP_SKB_CB(skb)->dccpd_type = pkt_type;
	DCCP_SKB_CB(skb)->dccpd_ack_seq = ackno;

	dccp_transmit_skb(sk, skb);
}

EXPORT_SYMBOL_GPL(dccp_send_sync);


void dccp_send_close(struct sock *sk, const int active)
{
	struct dccp_sock *dp = dccp_sk(sk);
	struct sk_buff *skb;
	const gfp_t prio = active ? GFP_KERNEL : GFP_ATOMIC;

	skb = alloc_skb(sk->sk_prot->max_header, prio);
	if (skb == NULL)
		return;

	
	skb_reserve(skb, sk->sk_prot->max_header);
	if (dp->dccps_role == DCCP_ROLE_SERVER && !dp->dccps_server_timewait)
		DCCP_SKB_CB(skb)->dccpd_type = DCCP_PKT_CLOSEREQ;
	else
		DCCP_SKB_CB(skb)->dccpd_type = DCCP_PKT_CLOSE;

	if (active) {
		dccp_write_xmit(sk, 1);
		dccp_skb_entail(sk, skb);
		dccp_transmit_skb(sk, skb_clone(skb, prio));
		
		inet_csk_reset_xmit_timer(sk, ICSK_TIME_RETRANS,
					  DCCP_TIMEOUT_INIT, DCCP_RTO_MAX);
	} else
		dccp_transmit_skb(sk, skb);
}
