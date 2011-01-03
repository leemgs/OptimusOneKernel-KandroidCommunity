
#include "../dccp.h"
#include "ccid3.h"

#include <asm/unaligned.h>

#ifdef CONFIG_IP_DCCP_CCID3_DEBUG
static int ccid3_debug;
#define ccid3_pr_debug(format, a...)	DCCP_PR_DEBUG(ccid3_debug, format, ##a)
#else
#define ccid3_pr_debug(format, a...)
#endif


#ifdef CONFIG_IP_DCCP_CCID3_DEBUG
static const char *ccid3_tx_state_name(enum ccid3_hc_tx_states state)
{
	static const char *const ccid3_state_names[] = {
	[TFRC_SSTATE_NO_SENT]  = "NO_SENT",
	[TFRC_SSTATE_NO_FBACK] = "NO_FBACK",
	[TFRC_SSTATE_FBACK]    = "FBACK",
	[TFRC_SSTATE_TERM]     = "TERM",
	};

	return ccid3_state_names[state];
}
#endif

static void ccid3_hc_tx_set_state(struct sock *sk,
				  enum ccid3_hc_tx_states state)
{
	struct ccid3_hc_tx_sock *hctx = ccid3_hc_tx_sk(sk);
	enum ccid3_hc_tx_states oldstate = hctx->ccid3hctx_state;

	ccid3_pr_debug("%s(%p) %-8.8s -> %s\n",
		       dccp_role(sk), sk, ccid3_tx_state_name(oldstate),
		       ccid3_tx_state_name(state));
	WARN_ON(state == oldstate);
	hctx->ccid3hctx_state = state;
}


static inline u64 rfc3390_initial_rate(struct sock *sk)
{
	const struct ccid3_hc_tx_sock *hctx = ccid3_hc_tx_sk(sk);
	const __u32 w_init = clamp_t(__u32, 4380U,
			2 * hctx->ccid3hctx_s, 4 * hctx->ccid3hctx_s);

	return scaled_div(w_init << 6, hctx->ccid3hctx_rtt);
}


static void ccid3_update_send_interval(struct ccid3_hc_tx_sock *hctx)
{
	
	hctx->ccid3hctx_t_ipi = scaled_div32(((u64)hctx->ccid3hctx_s) << 6,
					     hctx->ccid3hctx_x);

	
	hctx->ccid3hctx_delta = min_t(u32, hctx->ccid3hctx_t_ipi / 2,
					   TFRC_OPSYS_HALF_TIME_GRAN);

	ccid3_pr_debug("t_ipi=%u, delta=%u, s=%u, X=%u\n",
		       hctx->ccid3hctx_t_ipi, hctx->ccid3hctx_delta,
		       hctx->ccid3hctx_s, (unsigned)(hctx->ccid3hctx_x >> 6));

}

static u32 ccid3_hc_tx_idle_rtt(struct ccid3_hc_tx_sock *hctx, ktime_t now)
{
	u32 delta = ktime_us_delta(now, hctx->ccid3hctx_t_last_win_count);

	return delta / hctx->ccid3hctx_rtt;
}


static void ccid3_hc_tx_update_x(struct sock *sk, ktime_t *stamp)
{
	struct ccid3_hc_tx_sock *hctx = ccid3_hc_tx_sk(sk);
	__u64 min_rate = 2 * hctx->ccid3hctx_x_recv;
	const  __u64 old_x = hctx->ccid3hctx_x;
	ktime_t now = stamp ? *stamp : ktime_get_real();

	
	if (ccid3_hc_tx_idle_rtt(hctx, now) >= 2) {
		min_rate = rfc3390_initial_rate(sk);
		min_rate = max(min_rate, 2 * hctx->ccid3hctx_x_recv);
	}

	if (hctx->ccid3hctx_p > 0) {

		hctx->ccid3hctx_x = min(((__u64)hctx->ccid3hctx_x_calc) << 6,
					min_rate);
		hctx->ccid3hctx_x = max(hctx->ccid3hctx_x,
					(((__u64)hctx->ccid3hctx_s) << 6) /
								TFRC_T_MBI);

	} else if (ktime_us_delta(now, hctx->ccid3hctx_t_ld)
				- (s64)hctx->ccid3hctx_rtt >= 0) {

		hctx->ccid3hctx_x = min(2 * hctx->ccid3hctx_x, min_rate);
		hctx->ccid3hctx_x = max(hctx->ccid3hctx_x,
			    scaled_div(((__u64)hctx->ccid3hctx_s) << 6,
				       hctx->ccid3hctx_rtt));
		hctx->ccid3hctx_t_ld = now;
	}

	if (hctx->ccid3hctx_x != old_x) {
		ccid3_pr_debug("X_prev=%u, X_now=%u, X_calc=%u, "
			       "X_recv=%u\n", (unsigned)(old_x >> 6),
			       (unsigned)(hctx->ccid3hctx_x >> 6),
			       hctx->ccid3hctx_x_calc,
			       (unsigned)(hctx->ccid3hctx_x_recv >> 6));

		ccid3_update_send_interval(hctx);
	}
}


static inline void ccid3_hc_tx_update_s(struct ccid3_hc_tx_sock *hctx, int len)
{
	const u16 old_s = hctx->ccid3hctx_s;

	hctx->ccid3hctx_s = tfrc_ewma(hctx->ccid3hctx_s, len, 9);

	if (hctx->ccid3hctx_s != old_s)
		ccid3_update_send_interval(hctx);
}


static inline void ccid3_hc_tx_update_win_count(struct ccid3_hc_tx_sock *hctx,
						ktime_t now)
{
	u32 delta = ktime_us_delta(now, hctx->ccid3hctx_t_last_win_count),
	    quarter_rtts = (4 * delta) / hctx->ccid3hctx_rtt;

	if (quarter_rtts > 0) {
		hctx->ccid3hctx_t_last_win_count = now;
		hctx->ccid3hctx_last_win_count  += min(quarter_rtts, 5U);
		hctx->ccid3hctx_last_win_count	&= 0xF;		
	}
}

static void ccid3_hc_tx_no_feedback_timer(unsigned long data)
{
	struct sock *sk = (struct sock *)data;
	struct ccid3_hc_tx_sock *hctx = ccid3_hc_tx_sk(sk);
	unsigned long t_nfb = USEC_PER_SEC / 5;

	bh_lock_sock(sk);
	if (sock_owned_by_user(sk)) {
		
		
		goto restart_timer;
	}

	ccid3_pr_debug("%s(%p, state=%s) - entry \n", dccp_role(sk), sk,
		       ccid3_tx_state_name(hctx->ccid3hctx_state));

	if (hctx->ccid3hctx_state == TFRC_SSTATE_FBACK)
		ccid3_hc_tx_set_state(sk, TFRC_SSTATE_NO_FBACK);
	else if (hctx->ccid3hctx_state != TFRC_SSTATE_NO_FBACK)
		goto out;

	
	if (hctx->ccid3hctx_t_rto == 0 ||	
	    hctx->ccid3hctx_p == 0) {

		
		hctx->ccid3hctx_x = max(hctx->ccid3hctx_x / 2,
					(((__u64)hctx->ccid3hctx_s) << 6) /
								    TFRC_T_MBI);
		ccid3_update_send_interval(hctx);
	} else {
		
		BUG_ON(hctx->ccid3hctx_p && !hctx->ccid3hctx_x_calc);

		if (hctx->ccid3hctx_x_calc > (hctx->ccid3hctx_x_recv >> 5))
			hctx->ccid3hctx_x_recv =
				max(hctx->ccid3hctx_x_recv / 2,
				    (((__u64)hctx->ccid3hctx_s) << 6) /
							      (2 * TFRC_T_MBI));
		else {
			hctx->ccid3hctx_x_recv = hctx->ccid3hctx_x_calc;
			hctx->ccid3hctx_x_recv <<= 4;
		}
		ccid3_hc_tx_update_x(sk, NULL);
	}
	ccid3_pr_debug("Reduced X to %llu/64 bytes/sec\n",
			(unsigned long long)hctx->ccid3hctx_x);

	
	if (unlikely(hctx->ccid3hctx_t_rto == 0))	
		t_nfb = TFRC_INITIAL_TIMEOUT;
	else
		t_nfb = max(hctx->ccid3hctx_t_rto, 2 * hctx->ccid3hctx_t_ipi);

restart_timer:
	sk_reset_timer(sk, &hctx->ccid3hctx_no_feedback_timer,
			   jiffies + usecs_to_jiffies(t_nfb));
out:
	bh_unlock_sock(sk);
	sock_put(sk);
}


static int ccid3_hc_tx_send_packet(struct sock *sk, struct sk_buff *skb)
{
	struct dccp_sock *dp = dccp_sk(sk);
	struct ccid3_hc_tx_sock *hctx = ccid3_hc_tx_sk(sk);
	ktime_t now = ktime_get_real();
	s64 delay;

	
	if (unlikely(skb->len == 0))
		return -EBADMSG;

	switch (hctx->ccid3hctx_state) {
	case TFRC_SSTATE_NO_SENT:
		sk_reset_timer(sk, &hctx->ccid3hctx_no_feedback_timer,
			       (jiffies +
				usecs_to_jiffies(TFRC_INITIAL_TIMEOUT)));
		hctx->ccid3hctx_last_win_count	 = 0;
		hctx->ccid3hctx_t_last_win_count = now;

		
		hctx->ccid3hctx_t_nom = now;

		hctx->ccid3hctx_s = skb->len;

		
		if (dp->dccps_syn_rtt) {
			ccid3_pr_debug("SYN RTT = %uus\n", dp->dccps_syn_rtt);
			hctx->ccid3hctx_rtt  = dp->dccps_syn_rtt;
			hctx->ccid3hctx_x    = rfc3390_initial_rate(sk);
			hctx->ccid3hctx_t_ld = now;
		} else {
			
			hctx->ccid3hctx_rtt = DCCP_FALLBACK_RTT;
			hctx->ccid3hctx_x   = hctx->ccid3hctx_s;
			hctx->ccid3hctx_x <<= 6;
		}
		ccid3_update_send_interval(hctx);

		ccid3_hc_tx_set_state(sk, TFRC_SSTATE_NO_FBACK);
		break;
	case TFRC_SSTATE_NO_FBACK:
	case TFRC_SSTATE_FBACK:
		delay = ktime_us_delta(hctx->ccid3hctx_t_nom, now);
		ccid3_pr_debug("delay=%ld\n", (long)delay);
		
		if (delay - (s64)hctx->ccid3hctx_delta >= 1000)
			return (u32)delay / 1000L;

		ccid3_hc_tx_update_win_count(hctx, now);
		break;
	case TFRC_SSTATE_TERM:
		DCCP_BUG("%s(%p) - Illegal state TERM", dccp_role(sk), sk);
		return -EINVAL;
	}

	
	dp->dccps_hc_tx_insert_options = 1;
	DCCP_SKB_CB(skb)->dccpd_ccval = hctx->ccid3hctx_last_win_count;

	
	hctx->ccid3hctx_t_nom = ktime_add_us(hctx->ccid3hctx_t_nom,
					     hctx->ccid3hctx_t_ipi);
	return 0;
}

static void ccid3_hc_tx_packet_sent(struct sock *sk, int more,
				    unsigned int len)
{
	struct ccid3_hc_tx_sock *hctx = ccid3_hc_tx_sk(sk);

	ccid3_hc_tx_update_s(hctx, len);

	if (tfrc_tx_hist_add(&hctx->ccid3hctx_hist, dccp_sk(sk)->dccps_gss))
		DCCP_CRIT("packet history - out of memory!");
}

static void ccid3_hc_tx_packet_recv(struct sock *sk, struct sk_buff *skb)
{
	struct ccid3_hc_tx_sock *hctx = ccid3_hc_tx_sk(sk);
	struct ccid3_options_received *opt_recv;
	ktime_t now;
	unsigned long t_nfb;
	u32 pinv, r_sample;

	
	if (!(DCCP_SKB_CB(skb)->dccpd_type == DCCP_PKT_ACK ||
	      DCCP_SKB_CB(skb)->dccpd_type == DCCP_PKT_DATAACK))
		return;
	
	if (hctx->ccid3hctx_state != TFRC_SSTATE_FBACK &&
	    hctx->ccid3hctx_state != TFRC_SSTATE_NO_FBACK)
		return;

	opt_recv = &hctx->ccid3hctx_options_received;
	now = ktime_get_real();

	
	r_sample = tfrc_tx_hist_rtt(hctx->ccid3hctx_hist,
				    DCCP_SKB_CB(skb)->dccpd_ack_seq, now);
	if (r_sample == 0) {
		DCCP_WARN("%s(%p): %s with bogus ACK-%llu\n", dccp_role(sk), sk,
			  dccp_packet_name(DCCP_SKB_CB(skb)->dccpd_type),
			  (unsigned long long)DCCP_SKB_CB(skb)->dccpd_ack_seq);
		return;
	}

	
	hctx->ccid3hctx_x_recv = opt_recv->ccid3or_receive_rate;
	hctx->ccid3hctx_x_recv <<= 6;

	
	pinv = opt_recv->ccid3or_loss_event_rate;
	if (pinv == ~0U || pinv == 0)	       
		hctx->ccid3hctx_p = 0;
	else				       
		hctx->ccid3hctx_p = scaled_div(1, pinv);
	
	r_sample = dccp_sample_rtt(sk, r_sample);
	hctx->ccid3hctx_rtt = tfrc_ewma(hctx->ccid3hctx_rtt, r_sample, 9);
	
	if (hctx->ccid3hctx_state == TFRC_SSTATE_NO_FBACK) {
		ccid3_hc_tx_set_state(sk, TFRC_SSTATE_FBACK);

		if (hctx->ccid3hctx_t_rto == 0) {
			
			hctx->ccid3hctx_x    = rfc3390_initial_rate(sk);
			hctx->ccid3hctx_t_ld = now;

			ccid3_update_send_interval(hctx);

			goto done_computing_x;
		} else if (hctx->ccid3hctx_p == 0) {
			
			goto done_computing_x;
		}
	}

	
	if (hctx->ccid3hctx_p > 0)
		hctx->ccid3hctx_x_calc =
				tfrc_calc_x(hctx->ccid3hctx_s,
					    hctx->ccid3hctx_rtt,
					    hctx->ccid3hctx_p);
	ccid3_hc_tx_update_x(sk, &now);

done_computing_x:
	ccid3_pr_debug("%s(%p), RTT=%uus (sample=%uus), s=%u, "
			       "p=%u, X_calc=%u, X_recv=%u, X=%u\n",
			       dccp_role(sk),
			       sk, hctx->ccid3hctx_rtt, r_sample,
			       hctx->ccid3hctx_s, hctx->ccid3hctx_p,
			       hctx->ccid3hctx_x_calc,
			       (unsigned)(hctx->ccid3hctx_x_recv >> 6),
			       (unsigned)(hctx->ccid3hctx_x >> 6));

	
	sk_stop_timer(sk, &hctx->ccid3hctx_no_feedback_timer);

	
	sk->sk_write_space(sk);

	
	hctx->ccid3hctx_t_rto = max_t(u32, 4 * hctx->ccid3hctx_rtt,
					   (CONFIG_IP_DCCP_CCID3_RTO *
					    (USEC_PER_SEC / 1000)));
	
	t_nfb = max(hctx->ccid3hctx_t_rto, 2 * hctx->ccid3hctx_t_ipi);

	ccid3_pr_debug("%s(%p), Scheduled no feedback timer to "
		       "expire in %lu jiffies (%luus)\n",
		       dccp_role(sk),
		       sk, usecs_to_jiffies(t_nfb), t_nfb);

	sk_reset_timer(sk, &hctx->ccid3hctx_no_feedback_timer,
			   jiffies + usecs_to_jiffies(t_nfb));
}

static int ccid3_hc_tx_parse_options(struct sock *sk, unsigned char option,
				     unsigned char len, u16 idx,
				     unsigned char *value)
{
	int rc = 0;
	const struct dccp_sock *dp = dccp_sk(sk);
	struct ccid3_hc_tx_sock *hctx = ccid3_hc_tx_sk(sk);
	struct ccid3_options_received *opt_recv;
	__be32 opt_val;

	opt_recv = &hctx->ccid3hctx_options_received;

	if (opt_recv->ccid3or_seqno != dp->dccps_gsr) {
		opt_recv->ccid3or_seqno		     = dp->dccps_gsr;
		opt_recv->ccid3or_loss_event_rate    = ~0;
		opt_recv->ccid3or_loss_intervals_idx = 0;
		opt_recv->ccid3or_loss_intervals_len = 0;
		opt_recv->ccid3or_receive_rate	     = 0;
	}

	switch (option) {
	case TFRC_OPT_LOSS_EVENT_RATE:
		if (unlikely(len != 4)) {
			DCCP_WARN("%s(%p), invalid len %d "
				  "for TFRC_OPT_LOSS_EVENT_RATE\n",
				  dccp_role(sk), sk, len);
			rc = -EINVAL;
		} else {
			opt_val = get_unaligned((__be32 *)value);
			opt_recv->ccid3or_loss_event_rate = ntohl(opt_val);
			ccid3_pr_debug("%s(%p), LOSS_EVENT_RATE=%u\n",
				       dccp_role(sk), sk,
				       opt_recv->ccid3or_loss_event_rate);
		}
		break;
	case TFRC_OPT_LOSS_INTERVALS:
		opt_recv->ccid3or_loss_intervals_idx = idx;
		opt_recv->ccid3or_loss_intervals_len = len;
		ccid3_pr_debug("%s(%p), LOSS_INTERVALS=(%u, %u)\n",
			       dccp_role(sk), sk,
			       opt_recv->ccid3or_loss_intervals_idx,
			       opt_recv->ccid3or_loss_intervals_len);
		break;
	case TFRC_OPT_RECEIVE_RATE:
		if (unlikely(len != 4)) {
			DCCP_WARN("%s(%p), invalid len %d "
				  "for TFRC_OPT_RECEIVE_RATE\n",
				  dccp_role(sk), sk, len);
			rc = -EINVAL;
		} else {
			opt_val = get_unaligned((__be32 *)value);
			opt_recv->ccid3or_receive_rate = ntohl(opt_val);
			ccid3_pr_debug("%s(%p), RECEIVE_RATE=%u\n",
				       dccp_role(sk), sk,
				       opt_recv->ccid3or_receive_rate);
		}
		break;
	}

	return rc;
}

static int ccid3_hc_tx_init(struct ccid *ccid, struct sock *sk)
{
	struct ccid3_hc_tx_sock *hctx = ccid_priv(ccid);

	hctx->ccid3hctx_state = TFRC_SSTATE_NO_SENT;
	hctx->ccid3hctx_hist = NULL;
	setup_timer(&hctx->ccid3hctx_no_feedback_timer,
			ccid3_hc_tx_no_feedback_timer, (unsigned long)sk);

	return 0;
}

static void ccid3_hc_tx_exit(struct sock *sk)
{
	struct ccid3_hc_tx_sock *hctx = ccid3_hc_tx_sk(sk);

	ccid3_hc_tx_set_state(sk, TFRC_SSTATE_TERM);
	sk_stop_timer(sk, &hctx->ccid3hctx_no_feedback_timer);

	tfrc_tx_hist_purge(&hctx->ccid3hctx_hist);
}

static void ccid3_hc_tx_get_info(struct sock *sk, struct tcp_info *info)
{
	struct ccid3_hc_tx_sock *hctx;

	
	if (sk->sk_state == DCCP_LISTEN)
		return;

	hctx = ccid3_hc_tx_sk(sk);
	info->tcpi_rto = hctx->ccid3hctx_t_rto;
	info->tcpi_rtt = hctx->ccid3hctx_rtt;
}

static int ccid3_hc_tx_getsockopt(struct sock *sk, const int optname, int len,
				  u32 __user *optval, int __user *optlen)
{
	const struct ccid3_hc_tx_sock *hctx;
	const void *val;

	
	if (sk->sk_state == DCCP_LISTEN)
		return -EINVAL;

	hctx = ccid3_hc_tx_sk(sk);
	switch (optname) {
	case DCCP_SOCKOPT_CCID_TX_INFO:
		if (len < sizeof(hctx->ccid3hctx_tfrc))
			return -EINVAL;
		len = sizeof(hctx->ccid3hctx_tfrc);
		val = &hctx->ccid3hctx_tfrc;
		break;
	default:
		return -ENOPROTOOPT;
	}

	if (put_user(len, optlen) || copy_to_user(optval, val, len))
		return -EFAULT;

	return 0;
}




enum ccid3_fback_type {
	CCID3_FBACK_NONE = 0,
	CCID3_FBACK_INITIAL,
	CCID3_FBACK_PERIODIC,
	CCID3_FBACK_PARAM_CHANGE
};

#ifdef CONFIG_IP_DCCP_CCID3_DEBUG
static const char *ccid3_rx_state_name(enum ccid3_hc_rx_states state)
{
	static const char *const ccid3_rx_state_names[] = {
	[TFRC_RSTATE_NO_DATA] = "NO_DATA",
	[TFRC_RSTATE_DATA]    = "DATA",
	[TFRC_RSTATE_TERM]    = "TERM",
	};

	return ccid3_rx_state_names[state];
}
#endif

static void ccid3_hc_rx_set_state(struct sock *sk,
				  enum ccid3_hc_rx_states state)
{
	struct ccid3_hc_rx_sock *hcrx = ccid3_hc_rx_sk(sk);
	enum ccid3_hc_rx_states oldstate = hcrx->ccid3hcrx_state;

	ccid3_pr_debug("%s(%p) %-8.8s -> %s\n",
		       dccp_role(sk), sk, ccid3_rx_state_name(oldstate),
		       ccid3_rx_state_name(state));
	WARN_ON(state == oldstate);
	hcrx->ccid3hcrx_state = state;
}

static void ccid3_hc_rx_send_feedback(struct sock *sk,
				      const struct sk_buff *skb,
				      enum ccid3_fback_type fbtype)
{
	struct ccid3_hc_rx_sock *hcrx = ccid3_hc_rx_sk(sk);
	struct dccp_sock *dp = dccp_sk(sk);
	ktime_t now;
	s64 delta = 0;

	if (unlikely(hcrx->ccid3hcrx_state == TFRC_RSTATE_TERM))
		return;

	now = ktime_get_real();

	switch (fbtype) {
	case CCID3_FBACK_INITIAL:
		hcrx->ccid3hcrx_x_recv = 0;
		hcrx->ccid3hcrx_pinv   = ~0U;   
		break;
	case CCID3_FBACK_PARAM_CHANGE:
		
		if (hcrx->ccid3hcrx_x_recv > 0)
			break;
		
	case CCID3_FBACK_PERIODIC:
		delta = ktime_us_delta(now, hcrx->ccid3hcrx_tstamp_last_feedback);
		if (delta <= 0)
			DCCP_BUG("delta (%ld) <= 0", (long)delta);
		else
			hcrx->ccid3hcrx_x_recv =
				scaled_div32(hcrx->ccid3hcrx_bytes_recv, delta);
		break;
	default:
		return;
	}

	ccid3_pr_debug("Interval %ldusec, X_recv=%u, 1/p=%u\n", (long)delta,
		       hcrx->ccid3hcrx_x_recv, hcrx->ccid3hcrx_pinv);

	hcrx->ccid3hcrx_tstamp_last_feedback = now;
	hcrx->ccid3hcrx_last_counter	     = dccp_hdr(skb)->dccph_ccval;
	hcrx->ccid3hcrx_bytes_recv	     = 0;

	dp->dccps_hc_rx_insert_options = 1;
	dccp_send_ack(sk);
}

static int ccid3_hc_rx_insert_options(struct sock *sk, struct sk_buff *skb)
{
	const struct ccid3_hc_rx_sock *hcrx;
	__be32 x_recv, pinv;

	if (!(sk->sk_state == DCCP_OPEN || sk->sk_state == DCCP_PARTOPEN))
		return 0;

	hcrx = ccid3_hc_rx_sk(sk);

	if (dccp_packet_without_ack(skb))
		return 0;

	x_recv = htonl(hcrx->ccid3hcrx_x_recv);
	pinv   = htonl(hcrx->ccid3hcrx_pinv);

	if (dccp_insert_option(sk, skb, TFRC_OPT_LOSS_EVENT_RATE,
			       &pinv, sizeof(pinv)) ||
	    dccp_insert_option(sk, skb, TFRC_OPT_RECEIVE_RATE,
			       &x_recv, sizeof(x_recv)))
		return -1;

	return 0;
}


static u32 ccid3_first_li(struct sock *sk)
{
	struct ccid3_hc_rx_sock *hcrx = ccid3_hc_rx_sk(sk);
	u32 x_recv, p, delta;
	u64 fval;

	if (hcrx->ccid3hcrx_rtt == 0) {
		DCCP_WARN("No RTT estimate available, using fallback RTT\n");
		hcrx->ccid3hcrx_rtt = DCCP_FALLBACK_RTT;
	}

	delta = ktime_to_us(net_timedelta(hcrx->ccid3hcrx_tstamp_last_feedback));
	x_recv = scaled_div32(hcrx->ccid3hcrx_bytes_recv, delta);
	if (x_recv == 0) {		
		DCCP_WARN("X_recv==0\n");
		if ((x_recv = hcrx->ccid3hcrx_x_recv) == 0) {
			DCCP_BUG("stored value of X_recv is zero");
			return ~0U;
		}
	}

	fval = scaled_div(hcrx->ccid3hcrx_s, hcrx->ccid3hcrx_rtt);
	fval = scaled_div32(fval, x_recv);
	p = tfrc_calc_x_reverse_lookup(fval);

	ccid3_pr_debug("%s(%p), receive rate=%u bytes/s, implied "
		       "loss rate=%u\n", dccp_role(sk), sk, x_recv, p);

	return p == 0 ? ~0U : scaled_div(1, p);
}

static void ccid3_hc_rx_packet_recv(struct sock *sk, struct sk_buff *skb)
{
	struct ccid3_hc_rx_sock *hcrx = ccid3_hc_rx_sk(sk);
	enum ccid3_fback_type do_feedback = CCID3_FBACK_NONE;
	const u64 ndp = dccp_sk(sk)->dccps_options_received.dccpor_ndp;
	const bool is_data_packet = dccp_data_packet(skb);

	if (unlikely(hcrx->ccid3hcrx_state == TFRC_RSTATE_NO_DATA)) {
		if (is_data_packet) {
			const u32 payload = skb->len - dccp_hdr(skb)->dccph_doff * 4;
			do_feedback = CCID3_FBACK_INITIAL;
			ccid3_hc_rx_set_state(sk, TFRC_RSTATE_DATA);
			hcrx->ccid3hcrx_s = payload;
			
		}
		goto update_records;
	}

	if (tfrc_rx_hist_duplicate(&hcrx->ccid3hcrx_hist, skb))
		return; 

	if (is_data_packet) {
		const u32 payload = skb->len - dccp_hdr(skb)->dccph_doff * 4;
		
		hcrx->ccid3hcrx_s = tfrc_ewma(hcrx->ccid3hcrx_s, payload, 9);
		hcrx->ccid3hcrx_bytes_recv += payload;
	}

	
	if (tfrc_rx_handle_loss(&hcrx->ccid3hcrx_hist, &hcrx->ccid3hcrx_li_hist,
				skb, ndp, ccid3_first_li, sk)) {
		do_feedback = CCID3_FBACK_PARAM_CHANGE;
		goto done_receiving;
	}

	if (tfrc_rx_hist_loss_pending(&hcrx->ccid3hcrx_hist))
		return; 

	
	if (unlikely(!is_data_packet))
		goto update_records;

	if (!tfrc_lh_is_initialised(&hcrx->ccid3hcrx_li_hist)) {
		const u32 sample = tfrc_rx_hist_sample_rtt(&hcrx->ccid3hcrx_hist, skb);
		
		if (sample != 0)
			hcrx->ccid3hcrx_rtt = tfrc_ewma(hcrx->ccid3hcrx_rtt, sample, 9);

	} else if (tfrc_lh_update_i_mean(&hcrx->ccid3hcrx_li_hist, skb)) {
		
		do_feedback = CCID3_FBACK_PARAM_CHANGE;
	}

	
	if (SUB16(dccp_hdr(skb)->dccph_ccval, hcrx->ccid3hcrx_last_counter) > 3)
		do_feedback = CCID3_FBACK_PERIODIC;

update_records:
	tfrc_rx_hist_add_packet(&hcrx->ccid3hcrx_hist, skb, ndp);

done_receiving:
	if (do_feedback)
		ccid3_hc_rx_send_feedback(sk, skb, do_feedback);
}

static int ccid3_hc_rx_init(struct ccid *ccid, struct sock *sk)
{
	struct ccid3_hc_rx_sock *hcrx = ccid_priv(ccid);

	hcrx->ccid3hcrx_state = TFRC_RSTATE_NO_DATA;
	tfrc_lh_init(&hcrx->ccid3hcrx_li_hist);
	return tfrc_rx_hist_alloc(&hcrx->ccid3hcrx_hist);
}

static void ccid3_hc_rx_exit(struct sock *sk)
{
	struct ccid3_hc_rx_sock *hcrx = ccid3_hc_rx_sk(sk);

	ccid3_hc_rx_set_state(sk, TFRC_RSTATE_TERM);

	tfrc_rx_hist_purge(&hcrx->ccid3hcrx_hist);
	tfrc_lh_cleanup(&hcrx->ccid3hcrx_li_hist);
}

static void ccid3_hc_rx_get_info(struct sock *sk, struct tcp_info *info)
{
	const struct ccid3_hc_rx_sock *hcrx;

	
	if (sk->sk_state == DCCP_LISTEN)
		return;

	hcrx = ccid3_hc_rx_sk(sk);
	info->tcpi_ca_state = hcrx->ccid3hcrx_state;
	info->tcpi_options  |= TCPI_OPT_TIMESTAMPS;
	info->tcpi_rcv_rtt  = hcrx->ccid3hcrx_rtt;
}

static int ccid3_hc_rx_getsockopt(struct sock *sk, const int optname, int len,
				  u32 __user *optval, int __user *optlen)
{
	const struct ccid3_hc_rx_sock *hcrx;
	struct tfrc_rx_info rx_info;
	const void *val;

	
	if (sk->sk_state == DCCP_LISTEN)
		return -EINVAL;

	hcrx = ccid3_hc_rx_sk(sk);
	switch (optname) {
	case DCCP_SOCKOPT_CCID_RX_INFO:
		if (len < sizeof(rx_info))
			return -EINVAL;
		rx_info.tfrcrx_x_recv = hcrx->ccid3hcrx_x_recv;
		rx_info.tfrcrx_rtt    = hcrx->ccid3hcrx_rtt;
		rx_info.tfrcrx_p      = hcrx->ccid3hcrx_pinv == 0 ? ~0U :
					   scaled_div(1, hcrx->ccid3hcrx_pinv);
		len = sizeof(rx_info);
		val = &rx_info;
		break;
	default:
		return -ENOPROTOOPT;
	}

	if (put_user(len, optlen) || copy_to_user(optval, val, len))
		return -EFAULT;

	return 0;
}

struct ccid_operations ccid3_ops = {
	.ccid_id		   = DCCPC_CCID3,
	.ccid_name		   = "TCP-Friendly Rate Control",
	.ccid_hc_tx_obj_size	   = sizeof(struct ccid3_hc_tx_sock),
	.ccid_hc_tx_init	   = ccid3_hc_tx_init,
	.ccid_hc_tx_exit	   = ccid3_hc_tx_exit,
	.ccid_hc_tx_send_packet	   = ccid3_hc_tx_send_packet,
	.ccid_hc_tx_packet_sent	   = ccid3_hc_tx_packet_sent,
	.ccid_hc_tx_packet_recv	   = ccid3_hc_tx_packet_recv,
	.ccid_hc_tx_parse_options  = ccid3_hc_tx_parse_options,
	.ccid_hc_rx_obj_size	   = sizeof(struct ccid3_hc_rx_sock),
	.ccid_hc_rx_init	   = ccid3_hc_rx_init,
	.ccid_hc_rx_exit	   = ccid3_hc_rx_exit,
	.ccid_hc_rx_insert_options = ccid3_hc_rx_insert_options,
	.ccid_hc_rx_packet_recv	   = ccid3_hc_rx_packet_recv,
	.ccid_hc_rx_get_info	   = ccid3_hc_rx_get_info,
	.ccid_hc_tx_get_info	   = ccid3_hc_tx_get_info,
	.ccid_hc_rx_getsockopt	   = ccid3_hc_rx_getsockopt,
	.ccid_hc_tx_getsockopt	   = ccid3_hc_tx_getsockopt,
};

#ifdef CONFIG_IP_DCCP_CCID3_DEBUG
module_param(ccid3_debug, bool, 0644);
MODULE_PARM_DESC(ccid3_debug, "Enable CCID-3 debug messages");
#endif
