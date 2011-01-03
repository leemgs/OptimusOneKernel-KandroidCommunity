


#include "../feat.h"
#include "../ccid.h"
#include "../dccp.h"
#include "ccid2.h"


#ifdef CONFIG_IP_DCCP_CCID2_DEBUG
static int ccid2_debug;
#define ccid2_pr_debug(format, a...)	DCCP_PR_DEBUG(ccid2_debug, format, ##a)

static void ccid2_hc_tx_check_sanity(const struct ccid2_hc_tx_sock *hctx)
{
	int len = 0;
	int pipe = 0;
	struct ccid2_seq *seqp = hctx->ccid2hctx_seqh;

	
	if (seqp != hctx->ccid2hctx_seqt) {
		seqp = seqp->ccid2s_prev;
		len++;
		if (!seqp->ccid2s_acked)
			pipe++;

		while (seqp != hctx->ccid2hctx_seqt) {
			struct ccid2_seq *prev = seqp->ccid2s_prev;

			len++;
			if (!prev->ccid2s_acked)
				pipe++;

			
			BUG_ON(dccp_delta_seqno(seqp->ccid2s_seq,
						prev->ccid2s_seq ) >= 0);
			BUG_ON(time_before(seqp->ccid2s_sent,
					   prev->ccid2s_sent));

			seqp = prev;
		}
	}

	BUG_ON(pipe != hctx->ccid2hctx_pipe);
	ccid2_pr_debug("len of chain=%d\n", len);

	do {
		seqp = seqp->ccid2s_prev;
		len++;
	} while (seqp != hctx->ccid2hctx_seqh);

	ccid2_pr_debug("total len=%d\n", len);
	BUG_ON(len != hctx->ccid2hctx_seqbufc * CCID2_SEQBUF_LEN);
}
#else
#define ccid2_pr_debug(format, a...)
#define ccid2_hc_tx_check_sanity(hctx)
#endif

static int ccid2_hc_tx_alloc_seq(struct ccid2_hc_tx_sock *hctx)
{
	struct ccid2_seq *seqp;
	int i;

	
	if (hctx->ccid2hctx_seqbufc >= (sizeof(hctx->ccid2hctx_seqbuf) /
					sizeof(struct ccid2_seq*)))
		return -ENOMEM;

	
	seqp = kmalloc(CCID2_SEQBUF_LEN * sizeof(struct ccid2_seq), gfp_any());
	if (seqp == NULL)
		return -ENOMEM;

	for (i = 0; i < (CCID2_SEQBUF_LEN - 1); i++) {
		seqp[i].ccid2s_next = &seqp[i + 1];
		seqp[i + 1].ccid2s_prev = &seqp[i];
	}
	seqp[CCID2_SEQBUF_LEN - 1].ccid2s_next = seqp;
	seqp->ccid2s_prev = &seqp[CCID2_SEQBUF_LEN - 1];

	
	if (hctx->ccid2hctx_seqbufc == 0)
		hctx->ccid2hctx_seqh = hctx->ccid2hctx_seqt = seqp;
	else {
		
		hctx->ccid2hctx_seqh->ccid2s_next = seqp;
		seqp->ccid2s_prev = hctx->ccid2hctx_seqh;

		hctx->ccid2hctx_seqt->ccid2s_prev = &seqp[CCID2_SEQBUF_LEN - 1];
		seqp[CCID2_SEQBUF_LEN - 1].ccid2s_next = hctx->ccid2hctx_seqt;
	}

	
	hctx->ccid2hctx_seqbuf[hctx->ccid2hctx_seqbufc] = seqp;
	hctx->ccid2hctx_seqbufc++;

	return 0;
}

static int ccid2_hc_tx_send_packet(struct sock *sk, struct sk_buff *skb)
{
	struct ccid2_hc_tx_sock *hctx = ccid2_hc_tx_sk(sk);

	if (hctx->ccid2hctx_pipe < hctx->ccid2hctx_cwnd)
		return 0;

	return 1; 
}

static void ccid2_change_l_ack_ratio(struct sock *sk, u32 val)
{
	struct dccp_sock *dp = dccp_sk(sk);
	u32 max_ratio = DIV_ROUND_UP(ccid2_hc_tx_sk(sk)->ccid2hctx_cwnd, 2);

	
	if (val == 0 || val > max_ratio) {
		DCCP_WARN("Limiting Ack Ratio (%u) to %u\n", val, max_ratio);
		val = max_ratio;
	}
	if (val > DCCPF_ACK_RATIO_MAX)
		val = DCCPF_ACK_RATIO_MAX;

	if (val == dp->dccps_l_ack_ratio)
		return;

	ccid2_pr_debug("changing local ack ratio to %u\n", val);
	dp->dccps_l_ack_ratio = val;
}

static void ccid2_change_srtt(struct ccid2_hc_tx_sock *hctx, long val)
{
	ccid2_pr_debug("change SRTT to %ld\n", val);
	hctx->ccid2hctx_srtt = val;
}

static void ccid2_start_rto_timer(struct sock *sk);

static void ccid2_hc_tx_rto_expire(unsigned long data)
{
	struct sock *sk = (struct sock *)data;
	struct ccid2_hc_tx_sock *hctx = ccid2_hc_tx_sk(sk);
	long s;

	bh_lock_sock(sk);
	if (sock_owned_by_user(sk)) {
		sk_reset_timer(sk, &hctx->ccid2hctx_rtotimer,
			       jiffies + HZ / 5);
		goto out;
	}

	ccid2_pr_debug("RTO_EXPIRE\n");

	ccid2_hc_tx_check_sanity(hctx);

	
	hctx->ccid2hctx_rto <<= 1;

	s = hctx->ccid2hctx_rto / HZ;
	if (s > 60)
		hctx->ccid2hctx_rto = 60 * HZ;

	ccid2_start_rto_timer(sk);

	
	hctx->ccid2hctx_ssthresh = hctx->ccid2hctx_cwnd / 2;
	if (hctx->ccid2hctx_ssthresh < 2)
		hctx->ccid2hctx_ssthresh = 2;
	hctx->ccid2hctx_cwnd	 = 1;
	hctx->ccid2hctx_pipe	 = 0;

	
	hctx->ccid2hctx_seqt = hctx->ccid2hctx_seqh;
	hctx->ccid2hctx_packets_acked = 0;

	
	hctx->ccid2hctx_rpseq	 = 0;
	hctx->ccid2hctx_rpdupack = -1;
	ccid2_change_l_ack_ratio(sk, 1);
	ccid2_hc_tx_check_sanity(hctx);
out:
	bh_unlock_sock(sk);
	sock_put(sk);
}

static void ccid2_start_rto_timer(struct sock *sk)
{
	struct ccid2_hc_tx_sock *hctx = ccid2_hc_tx_sk(sk);

	ccid2_pr_debug("setting RTO timeout=%ld\n", hctx->ccid2hctx_rto);

	BUG_ON(timer_pending(&hctx->ccid2hctx_rtotimer));
	sk_reset_timer(sk, &hctx->ccid2hctx_rtotimer,
		       jiffies + hctx->ccid2hctx_rto);
}

static void ccid2_hc_tx_packet_sent(struct sock *sk, int more, unsigned int len)
{
	struct dccp_sock *dp = dccp_sk(sk);
	struct ccid2_hc_tx_sock *hctx = ccid2_hc_tx_sk(sk);
	struct ccid2_seq *next;

	hctx->ccid2hctx_pipe++;

	hctx->ccid2hctx_seqh->ccid2s_seq   = dp->dccps_gss;
	hctx->ccid2hctx_seqh->ccid2s_acked = 0;
	hctx->ccid2hctx_seqh->ccid2s_sent  = jiffies;

	next = hctx->ccid2hctx_seqh->ccid2s_next;
	
	if (next == hctx->ccid2hctx_seqt) {
		if (ccid2_hc_tx_alloc_seq(hctx)) {
			DCCP_CRIT("packet history - out of memory!");
			
			return;
		}
		next = hctx->ccid2hctx_seqh->ccid2s_next;
		BUG_ON(next == hctx->ccid2hctx_seqt);
	}
	hctx->ccid2hctx_seqh = next;

	ccid2_pr_debug("cwnd=%d pipe=%d\n", hctx->ccid2hctx_cwnd,
		       hctx->ccid2hctx_pipe);

	
#if 0
	
	hctx->ccid2hctx_arsent++;
	
	if (hctx->ccid2hctx_ackloss) {
		if (hctx->ccid2hctx_arsent >= hctx->ccid2hctx_cwnd) {
			hctx->ccid2hctx_arsent	= 0;
			hctx->ccid2hctx_ackloss	= 0;
		}
	} else {
		
		
		if (dp->dccps_l_ack_ratio > 1) {
			
			int denom = dp->dccps_l_ack_ratio * dp->dccps_l_ack_ratio -
				    dp->dccps_l_ack_ratio;

			denom = hctx->ccid2hctx_cwnd * hctx->ccid2hctx_cwnd / denom;

			if (hctx->ccid2hctx_arsent >= denom) {
				ccid2_change_l_ack_ratio(sk, dp->dccps_l_ack_ratio - 1);
				hctx->ccid2hctx_arsent = 0;
			}
		} else {
			
			hctx->ccid2hctx_arsent = 0; 
		}
	}
#endif

	
	if (!timer_pending(&hctx->ccid2hctx_rtotimer))
		ccid2_start_rto_timer(sk);

#ifdef CONFIG_IP_DCCP_CCID2_DEBUG
	do {
		struct ccid2_seq *seqp = hctx->ccid2hctx_seqt;

		while (seqp != hctx->ccid2hctx_seqh) {
			ccid2_pr_debug("out seq=%llu acked=%d time=%lu\n",
				       (unsigned long long)seqp->ccid2s_seq,
				       seqp->ccid2s_acked, seqp->ccid2s_sent);
			seqp = seqp->ccid2s_next;
		}
	} while (0);
	ccid2_pr_debug("=========\n");
	ccid2_hc_tx_check_sanity(hctx);
#endif
}


static int ccid2_ackvector(struct sock *sk, struct sk_buff *skb, int offset,
			   unsigned char **vec, unsigned char *veclen)
{
	const struct dccp_hdr *dh = dccp_hdr(skb);
	unsigned char *options = (unsigned char *)dh + dccp_hdr_len(skb);
	unsigned char *opt_ptr;
	const unsigned char *opt_end = (unsigned char *)dh +
					(dh->dccph_doff * 4);
	unsigned char opt, len;
	unsigned char *value;

	BUG_ON(offset < 0);
	options += offset;
	opt_ptr = options;
	if (opt_ptr >= opt_end)
		return -1;

	while (opt_ptr != opt_end) {
		opt   = *opt_ptr++;
		len   = 0;
		value = NULL;

		
		if (opt > DCCPO_MAX_RESERVED) {
			if (opt_ptr == opt_end)
				goto out_invalid_option;

			len = *opt_ptr++;
			if (len < 3)
				goto out_invalid_option;
			
			len     -= 2;
			value   = opt_ptr;
			opt_ptr += len;

			if (opt_ptr > opt_end)
				goto out_invalid_option;
		}

		switch (opt) {
		case DCCPO_ACK_VECTOR_0:
		case DCCPO_ACK_VECTOR_1:
			*vec	= value;
			*veclen = len;
			return offset + (opt_ptr - options);
		}
	}

	return -1;

out_invalid_option:
	DCCP_BUG("Invalid option - this should not happen (previous parsing)!");
	return -1;
}

static void ccid2_hc_tx_kill_rto_timer(struct sock *sk)
{
	struct ccid2_hc_tx_sock *hctx = ccid2_hc_tx_sk(sk);

	sk_stop_timer(sk, &hctx->ccid2hctx_rtotimer);
	ccid2_pr_debug("deleted RTO timer\n");
}

static inline void ccid2_new_ack(struct sock *sk,
				 struct ccid2_seq *seqp,
				 unsigned int *maxincr)
{
	struct ccid2_hc_tx_sock *hctx = ccid2_hc_tx_sk(sk);

	if (hctx->ccid2hctx_cwnd < hctx->ccid2hctx_ssthresh) {
		if (*maxincr > 0 && ++hctx->ccid2hctx_packets_acked == 2) {
			hctx->ccid2hctx_cwnd += 1;
			*maxincr	     -= 1;
			hctx->ccid2hctx_packets_acked = 0;
		}
	} else if (++hctx->ccid2hctx_packets_acked >= hctx->ccid2hctx_cwnd) {
			hctx->ccid2hctx_cwnd += 1;
			hctx->ccid2hctx_packets_acked = 0;
	}

	
	if (hctx->ccid2hctx_srtt == -1 ||
	    time_after(jiffies, hctx->ccid2hctx_lastrtt + hctx->ccid2hctx_srtt)) {
		unsigned long r = (long)jiffies - (long)seqp->ccid2s_sent;
		int s;

		
		if (hctx->ccid2hctx_srtt == -1) {
			ccid2_pr_debug("R: %lu Time=%lu seq=%llu\n",
				       r, jiffies,
				       (unsigned long long)seqp->ccid2s_seq);
			ccid2_change_srtt(hctx, r);
			hctx->ccid2hctx_rttvar = r >> 1;
		} else {
			
			long tmp = hctx->ccid2hctx_srtt - r;
			long srtt;

			if (tmp < 0)
				tmp *= -1;

			tmp >>= 2;
			hctx->ccid2hctx_rttvar *= 3;
			hctx->ccid2hctx_rttvar >>= 2;
			hctx->ccid2hctx_rttvar += tmp;

			
			srtt = hctx->ccid2hctx_srtt;
			srtt *= 7;
			srtt >>= 3;
			tmp = r >> 3;
			srtt += tmp;
			ccid2_change_srtt(hctx, srtt);
		}
		s = hctx->ccid2hctx_rttvar << 2;
		
		if (!s)
			s = 1;
		hctx->ccid2hctx_rto = hctx->ccid2hctx_srtt + s;

		
		s = hctx->ccid2hctx_rto / HZ;
		
#if 1
		if (s < 1)
			hctx->ccid2hctx_rto = HZ;
#endif
		
		if (s > 60)
			hctx->ccid2hctx_rto = HZ * 60;

		hctx->ccid2hctx_lastrtt = jiffies;

		ccid2_pr_debug("srtt: %ld rttvar: %ld rto: %ld (HZ=%d) R=%lu\n",
			       hctx->ccid2hctx_srtt, hctx->ccid2hctx_rttvar,
			       hctx->ccid2hctx_rto, HZ, r);
	}

	
	ccid2_hc_tx_kill_rto_timer(sk);
	ccid2_start_rto_timer(sk);
}

static void ccid2_hc_tx_dec_pipe(struct sock *sk)
{
	struct ccid2_hc_tx_sock *hctx = ccid2_hc_tx_sk(sk);

	if (hctx->ccid2hctx_pipe == 0)
		DCCP_BUG("pipe == 0");
	else
		hctx->ccid2hctx_pipe--;

	if (hctx->ccid2hctx_pipe == 0)
		ccid2_hc_tx_kill_rto_timer(sk);
}

static void ccid2_congestion_event(struct sock *sk, struct ccid2_seq *seqp)
{
	struct ccid2_hc_tx_sock *hctx = ccid2_hc_tx_sk(sk);

	if (time_before(seqp->ccid2s_sent, hctx->ccid2hctx_last_cong)) {
		ccid2_pr_debug("Multiple losses in an RTT---treating as one\n");
		return;
	}

	hctx->ccid2hctx_last_cong = jiffies;

	hctx->ccid2hctx_cwnd     = hctx->ccid2hctx_cwnd / 2 ? : 1U;
	hctx->ccid2hctx_ssthresh = max(hctx->ccid2hctx_cwnd, 2U);

	
	if (dccp_sk(sk)->dccps_l_ack_ratio > hctx->ccid2hctx_cwnd)
		ccid2_change_l_ack_ratio(sk, hctx->ccid2hctx_cwnd);
}

static void ccid2_hc_tx_packet_recv(struct sock *sk, struct sk_buff *skb)
{
	struct dccp_sock *dp = dccp_sk(sk);
	struct ccid2_hc_tx_sock *hctx = ccid2_hc_tx_sk(sk);
	u64 ackno, seqno;
	struct ccid2_seq *seqp;
	unsigned char *vector;
	unsigned char veclen;
	int offset = 0;
	int done = 0;
	unsigned int maxincr = 0;

	ccid2_hc_tx_check_sanity(hctx);
	
	seqno = DCCP_SKB_CB(skb)->dccpd_seq;

	
	
	if (hctx->ccid2hctx_rpdupack == -1) {
		hctx->ccid2hctx_rpdupack = 0;
		hctx->ccid2hctx_rpseq = seqno;
	} else {
		
		if (dccp_delta_seqno(hctx->ccid2hctx_rpseq, seqno) == 1)
			hctx->ccid2hctx_rpseq = seqno;
		
		else if (after48(seqno, hctx->ccid2hctx_rpseq)) {
			hctx->ccid2hctx_rpdupack++;

			
			if (hctx->ccid2hctx_rpdupack >= NUMDUPACK) {
				hctx->ccid2hctx_rpdupack = -1; 
				hctx->ccid2hctx_rpseq = 0;

				ccid2_change_l_ack_ratio(sk, 2 * dp->dccps_l_ack_ratio);
			}
		}
	}

	
	
	if (hctx->ccid2hctx_seqh == hctx->ccid2hctx_seqt)
		return;

	switch (DCCP_SKB_CB(skb)->dccpd_type) {
	case DCCP_PKT_ACK:
	case DCCP_PKT_DATAACK:
		break;
	default:
		return;
	}

	ackno = DCCP_SKB_CB(skb)->dccpd_ack_seq;
	if (after48(ackno, hctx->ccid2hctx_high_ack))
		hctx->ccid2hctx_high_ack = ackno;

	seqp = hctx->ccid2hctx_seqt;
	while (before48(seqp->ccid2s_seq, ackno)) {
		seqp = seqp->ccid2s_next;
		if (seqp == hctx->ccid2hctx_seqh) {
			seqp = hctx->ccid2hctx_seqh->ccid2s_prev;
			break;
		}
	}

	
	if (hctx->ccid2hctx_cwnd < hctx->ccid2hctx_ssthresh)
		maxincr = DIV_ROUND_UP(dp->dccps_l_ack_ratio, 2);

	
	while ((offset = ccid2_ackvector(sk, skb, offset,
					 &vector, &veclen)) != -1) {
		
		while (veclen--) {
			const u8 rl = *vector & DCCP_ACKVEC_LEN_MASK;
			u64 ackno_end_rl = SUB48(ackno, rl);

			ccid2_pr_debug("ackvec start:%llu end:%llu\n",
				       (unsigned long long)ackno,
				       (unsigned long long)ackno_end_rl);
			
			while (after48(seqp->ccid2s_seq, ackno)) {
				if (seqp == hctx->ccid2hctx_seqt) {
					done = 1;
					break;
				}
				seqp = seqp->ccid2s_prev;
			}
			if (done)
				break;

			
			while (between48(seqp->ccid2s_seq,ackno_end_rl,ackno)) {
				const u8 state = *vector &
						 DCCP_ACKVEC_STATE_MASK;

				
				if (state != DCCP_ACKVEC_STATE_NOT_RECEIVED &&
				    !seqp->ccid2s_acked) {
					if (state ==
					    DCCP_ACKVEC_STATE_ECN_MARKED) {
						ccid2_congestion_event(sk,
								       seqp);
					} else
						ccid2_new_ack(sk, seqp,
							      &maxincr);

					seqp->ccid2s_acked = 1;
					ccid2_pr_debug("Got ack for %llu\n",
						       (unsigned long long)seqp->ccid2s_seq);
					ccid2_hc_tx_dec_pipe(sk);
				}
				if (seqp == hctx->ccid2hctx_seqt) {
					done = 1;
					break;
				}
				seqp = seqp->ccid2s_prev;
			}
			if (done)
				break;

			ackno = SUB48(ackno_end_rl, 1);
			vector++;
		}
		if (done)
			break;
	}

	
	seqp = hctx->ccid2hctx_seqt;
	while (before48(seqp->ccid2s_seq, hctx->ccid2hctx_high_ack)) {
		seqp = seqp->ccid2s_next;
		if (seqp == hctx->ccid2hctx_seqh) {
			seqp = hctx->ccid2hctx_seqh->ccid2s_prev;
			break;
		}
	}
	done = 0;
	while (1) {
		if (seqp->ccid2s_acked) {
			done++;
			if (done == NUMDUPACK)
				break;
		}
		if (seqp == hctx->ccid2hctx_seqt)
			break;
		seqp = seqp->ccid2s_prev;
	}

	
	if (done == NUMDUPACK) {
		struct ccid2_seq *last_acked = seqp;

		
		while (1) {
			if (!seqp->ccid2s_acked) {
				ccid2_pr_debug("Packet lost: %llu\n",
					       (unsigned long long)seqp->ccid2s_seq);
				
				ccid2_congestion_event(sk, seqp);
				ccid2_hc_tx_dec_pipe(sk);
			}
			if (seqp == hctx->ccid2hctx_seqt)
				break;
			seqp = seqp->ccid2s_prev;
		}

		hctx->ccid2hctx_seqt = last_acked;
	}

	
	while (hctx->ccid2hctx_seqt != hctx->ccid2hctx_seqh) {
		if (!hctx->ccid2hctx_seqt->ccid2s_acked)
			break;

		hctx->ccid2hctx_seqt = hctx->ccid2hctx_seqt->ccid2s_next;
	}

	ccid2_hc_tx_check_sanity(hctx);
}

static int ccid2_hc_tx_init(struct ccid *ccid, struct sock *sk)
{
	struct ccid2_hc_tx_sock *hctx = ccid_priv(ccid);
	struct dccp_sock *dp = dccp_sk(sk);
	u32 max_ratio;

	
	hctx->ccid2hctx_ssthresh  = ~0U;

	
	hctx->ccid2hctx_cwnd = clamp(4380U / dp->dccps_mss_cache, 2U, 4U);

	
	max_ratio = DIV_ROUND_UP(hctx->ccid2hctx_cwnd, 2);
	if (dp->dccps_l_ack_ratio == 0 || dp->dccps_l_ack_ratio > max_ratio)
		dp->dccps_l_ack_ratio = max_ratio;

	
	if (ccid2_hc_tx_alloc_seq(hctx))
		return -ENOMEM;

	hctx->ccid2hctx_rto	 = 3 * HZ;
	ccid2_change_srtt(hctx, -1);
	hctx->ccid2hctx_rttvar	 = -1;
	hctx->ccid2hctx_rpdupack = -1;
	hctx->ccid2hctx_last_cong = jiffies;
	setup_timer(&hctx->ccid2hctx_rtotimer, ccid2_hc_tx_rto_expire,
			(unsigned long)sk);

	ccid2_hc_tx_check_sanity(hctx);
	return 0;
}

static void ccid2_hc_tx_exit(struct sock *sk)
{
	struct ccid2_hc_tx_sock *hctx = ccid2_hc_tx_sk(sk);
	int i;

	ccid2_hc_tx_kill_rto_timer(sk);

	for (i = 0; i < hctx->ccid2hctx_seqbufc; i++)
		kfree(hctx->ccid2hctx_seqbuf[i]);
	hctx->ccid2hctx_seqbufc = 0;
}

static void ccid2_hc_rx_packet_recv(struct sock *sk, struct sk_buff *skb)
{
	const struct dccp_sock *dp = dccp_sk(sk);
	struct ccid2_hc_rx_sock *hcrx = ccid2_hc_rx_sk(sk);

	switch (DCCP_SKB_CB(skb)->dccpd_type) {
	case DCCP_PKT_DATA:
	case DCCP_PKT_DATAACK:
		hcrx->ccid2hcrx_data++;
		if (hcrx->ccid2hcrx_data >= dp->dccps_r_ack_ratio) {
			dccp_send_ack(sk);
			hcrx->ccid2hcrx_data = 0;
		}
		break;
	}
}

struct ccid_operations ccid2_ops = {
	.ccid_id		= DCCPC_CCID2,
	.ccid_name		= "TCP-like",
	.ccid_hc_tx_obj_size	= sizeof(struct ccid2_hc_tx_sock),
	.ccid_hc_tx_init	= ccid2_hc_tx_init,
	.ccid_hc_tx_exit	= ccid2_hc_tx_exit,
	.ccid_hc_tx_send_packet	= ccid2_hc_tx_send_packet,
	.ccid_hc_tx_packet_sent	= ccid2_hc_tx_packet_sent,
	.ccid_hc_tx_packet_recv	= ccid2_hc_tx_packet_recv,
	.ccid_hc_rx_obj_size	= sizeof(struct ccid2_hc_rx_sock),
	.ccid_hc_rx_packet_recv	= ccid2_hc_rx_packet_recv,
};

#ifdef CONFIG_IP_DCCP_CCID2_DEBUG
module_param(ccid2_debug, bool, 0644);
MODULE_PARM_DESC(ccid2_debug, "Enable CCID-2 debug messages");
#endif
