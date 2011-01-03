

#include <linux/types.h>
#include <linux/random.h>
#include <net/sctp/sctp.h>
#include <net/sctp/sm.h>




static struct sctp_transport *sctp_transport_init(struct sctp_transport *peer,
						  const union sctp_addr *addr,
						  gfp_t gfp)
{
	
	peer->ipaddr = *addr;
	peer->af_specific = sctp_get_af_specific(addr->sa.sa_family);
	peer->asoc = NULL;

	peer->dst = NULL;
	memset(&peer->saddr, 0, sizeof(union sctp_addr));

	
	peer->rto = msecs_to_jiffies(sctp_rto_initial);
	peer->rtt = 0;
	peer->rttvar = 0;
	peer->srtt = 0;
	peer->rto_pending = 0;
	peer->hb_sent = 0;
	peer->fast_recovery = 0;

	peer->last_time_heard = jiffies;
	peer->last_time_used = jiffies;
	peer->last_time_ecne_reduced = jiffies;

	peer->init_sent_count = 0;

	peer->param_flags = SPP_HB_DISABLE |
			    SPP_PMTUD_ENABLE |
			    SPP_SACKDELAY_ENABLE;
	peer->hbinterval  = 0;

	
	peer->pathmaxrxt  = sctp_max_retrans_path;
	peer->error_count = 0;

	INIT_LIST_HEAD(&peer->transmitted);
	INIT_LIST_HEAD(&peer->send_ready);
	INIT_LIST_HEAD(&peer->transports);

	peer->T3_rtx_timer.expires = 0;
	peer->hb_timer.expires = 0;

	setup_timer(&peer->T3_rtx_timer, sctp_generate_t3_rtx_event,
			(unsigned long)peer);
	setup_timer(&peer->hb_timer, sctp_generate_heartbeat_event,
			(unsigned long)peer);

	
	get_random_bytes(&peer->hb_nonce, sizeof(peer->hb_nonce));

	atomic_set(&peer->refcnt, 1);
	peer->dead = 0;

	peer->malloced = 0;

	
	peer->cacc.changeover_active = 0;
	peer->cacc.cycling_changeover = 0;
	peer->cacc.next_tsn_at_change = 0;
	peer->cacc.cacc_saw_newack = 0;

	return peer;
}


struct sctp_transport *sctp_transport_new(const union sctp_addr *addr,
					  gfp_t gfp)
{
	struct sctp_transport *transport;

	transport = t_new(struct sctp_transport, gfp);
	if (!transport)
		goto fail;

	if (!sctp_transport_init(transport, addr, gfp))
		goto fail_init;

	transport->malloced = 1;
	SCTP_DBG_OBJCNT_INC(transport);

	return transport;

fail_init:
	kfree(transport);

fail:
	return NULL;
}


void sctp_transport_free(struct sctp_transport *transport)
{
	transport->dead = 1;

	
	if (del_timer(&transport->hb_timer))
		sctp_transport_put(transport);

	
	if (timer_pending(&transport->T3_rtx_timer) &&
	    del_timer(&transport->T3_rtx_timer))
		sctp_transport_put(transport);


	sctp_transport_put(transport);
}


static void sctp_transport_destroy(struct sctp_transport *transport)
{
	SCTP_ASSERT(transport->dead, "Transport is not dead", return);

	if (transport->asoc)
		sctp_association_put(transport->asoc);

	sctp_packet_free(&transport->packet);

	dst_release(transport->dst);
	kfree(transport);
	SCTP_DBG_OBJCNT_DEC(transport);
}


void sctp_transport_reset_timers(struct sctp_transport *transport, int force)
{
	

	if (force || !timer_pending(&transport->T3_rtx_timer))
		if (!mod_timer(&transport->T3_rtx_timer,
			       jiffies + transport->rto))
			sctp_transport_hold(transport);

	
	if (!mod_timer(&transport->hb_timer,
		       sctp_transport_timeout(transport)))
	    sctp_transport_hold(transport);
}


void sctp_transport_set_owner(struct sctp_transport *transport,
			      struct sctp_association *asoc)
{
	transport->asoc = asoc;
	sctp_association_hold(asoc);
}


void sctp_transport_pmtu(struct sctp_transport *transport)
{
	struct dst_entry *dst;

	dst = transport->af_specific->get_dst(NULL, &transport->ipaddr, NULL);

	if (dst) {
		transport->pathmtu = dst_mtu(dst);
		dst_release(dst);
	} else
		transport->pathmtu = SCTP_DEFAULT_MAXSEGMENT;
}


static struct dst_entry *sctp_transport_dst_check(struct sctp_transport *t)
{
	struct dst_entry *dst = t->dst;

	if (dst && dst->obsolete && dst->ops->check(dst, 0) == NULL) {
		dst_release(t->dst);
		t->dst = NULL;
		return NULL;
	}

	return dst;
}

void sctp_transport_update_pmtu(struct sctp_transport *t, u32 pmtu)
{
	struct dst_entry *dst;

	if (unlikely(pmtu < SCTP_DEFAULT_MINSEGMENT)) {
		printk(KERN_WARNING "%s: Reported pmtu %d too low, "
		       "using default minimum of %d\n",
		       __func__, pmtu,
		       SCTP_DEFAULT_MINSEGMENT);
		
		t->pathmtu = SCTP_DEFAULT_MINSEGMENT;
	} else {
		t->pathmtu = pmtu;
	}

	dst = sctp_transport_dst_check(t);
	if (dst)
		dst->ops->update_pmtu(dst, pmtu);
}


void sctp_transport_route(struct sctp_transport *transport,
			  union sctp_addr *saddr, struct sctp_sock *opt)
{
	struct sctp_association *asoc = transport->asoc;
	struct sctp_af *af = transport->af_specific;
	union sctp_addr *daddr = &transport->ipaddr;
	struct dst_entry *dst;

	dst = af->get_dst(asoc, daddr, saddr);

	if (saddr)
		memcpy(&transport->saddr, saddr, sizeof(union sctp_addr));
	else
		af->get_saddr(opt, asoc, dst, daddr, &transport->saddr);

	transport->dst = dst;
	if ((transport->param_flags & SPP_PMTUD_DISABLE) && transport->pathmtu) {
		return;
	}
	if (dst) {
		transport->pathmtu = dst_mtu(dst);

		
		if (asoc && (!asoc->peer.primary_path ||
				(transport == asoc->peer.active_path)))
			opt->pf->af->to_sk_saddr(&transport->saddr,
						 asoc->base.sk);
	} else
		transport->pathmtu = SCTP_DEFAULT_MAXSEGMENT;
}


void sctp_transport_hold(struct sctp_transport *transport)
{
	atomic_inc(&transport->refcnt);
}


void sctp_transport_put(struct sctp_transport *transport)
{
	if (atomic_dec_and_test(&transport->refcnt))
		sctp_transport_destroy(transport);
}


void sctp_transport_update_rto(struct sctp_transport *tp, __u32 rtt)
{
	
	SCTP_ASSERT(tp, "NULL transport", return);

	
	SCTP_ASSERT(tp->rto_pending, "rto_pending not set", return);

	if (tp->rttvar || tp->srtt) {
		

		
		tp->rttvar = tp->rttvar - (tp->rttvar >> sctp_rto_beta)
			+ ((abs(tp->srtt - rtt)) >> sctp_rto_beta);
		tp->srtt = tp->srtt - (tp->srtt >> sctp_rto_alpha)
			+ (rtt >> sctp_rto_alpha);
	} else {
		
		tp->srtt = rtt;
		tp->rttvar = rtt >> 1;
	}

	
	if (tp->rttvar == 0)
		tp->rttvar = SCTP_CLOCK_GRANULARITY;

	
	tp->rto = tp->srtt + (tp->rttvar << 2);

	
	if (tp->rto < tp->asoc->rto_min)
		tp->rto = tp->asoc->rto_min;

	
	if (tp->rto > tp->asoc->rto_max)
		tp->rto = tp->asoc->rto_max;

	tp->rtt = rtt;

	
	tp->rto_pending = 0;

	SCTP_DEBUG_PRINTK("%s: transport: %p, rtt: %d, srtt: %d "
			  "rttvar: %d, rto: %ld\n", __func__,
			  tp, rtt, tp->srtt, tp->rttvar, tp->rto);
}


void sctp_transport_raise_cwnd(struct sctp_transport *transport,
			       __u32 sack_ctsn, __u32 bytes_acked)
{
	__u32 cwnd, ssthresh, flight_size, pba, pmtu;

	cwnd = transport->cwnd;
	flight_size = transport->flight_size;

	
	if (transport->fast_recovery &&
	    TSN_lte(transport->fast_recovery_exit, sack_ctsn))
		transport->fast_recovery = 0;

	
	if (TSN_lte(sack_ctsn, transport->asoc->ctsn_ack_point) ||
	    (flight_size < cwnd))
		return;

	ssthresh = transport->ssthresh;
	pba = transport->partial_bytes_acked;
	pmtu = transport->asoc->pathmtu;

	if (cwnd <= ssthresh) {
		
		if (transport->fast_recovery)
			return;

		if (bytes_acked > pmtu)
			cwnd += pmtu;
		else
			cwnd += bytes_acked;
		SCTP_DEBUG_PRINTK("%s: SLOW START: transport: %p, "
				  "bytes_acked: %d, cwnd: %d, ssthresh: %d, "
				  "flight_size: %d, pba: %d\n",
				  __func__,
				  transport, bytes_acked, cwnd,
				  ssthresh, flight_size, pba);
	} else {
		
		pba += bytes_acked;
		if (pba >= cwnd) {
			cwnd += pmtu;
			pba = ((cwnd < pba) ? (pba - cwnd) : 0);
		}
		SCTP_DEBUG_PRINTK("%s: CONGESTION AVOIDANCE: "
				  "transport: %p, bytes_acked: %d, cwnd: %d, "
				  "ssthresh: %d, flight_size: %d, pba: %d\n",
				  __func__,
				  transport, bytes_acked, cwnd,
				  ssthresh, flight_size, pba);
	}

	transport->cwnd = cwnd;
	transport->partial_bytes_acked = pba;
}


void sctp_transport_lower_cwnd(struct sctp_transport *transport,
			       sctp_lower_cwnd_t reason)
{
	switch (reason) {
	case SCTP_LOWER_CWND_T3_RTX:
		
		transport->ssthresh = max(transport->cwnd/2,
					  4*transport->asoc->pathmtu);
		transport->cwnd = transport->asoc->pathmtu;

		
		transport->fast_recovery = 0;
		break;

	case SCTP_LOWER_CWND_FAST_RTX:
		
		if (transport->fast_recovery)
			return;

		
		transport->fast_recovery = 1;
		transport->fast_recovery_exit = transport->asoc->next_tsn - 1;

		transport->ssthresh = max(transport->cwnd/2,
					  4*transport->asoc->pathmtu);
		transport->cwnd = transport->ssthresh;
		break;

	case SCTP_LOWER_CWND_ECNE:
		
		if (time_after(jiffies, transport->last_time_ecne_reduced +
					transport->rtt)) {
			transport->ssthresh = max(transport->cwnd/2,
						  4*transport->asoc->pathmtu);
			transport->cwnd = transport->ssthresh;
			transport->last_time_ecne_reduced = jiffies;
		}
		break;

	case SCTP_LOWER_CWND_INACTIVE:
		
		if (time_after(jiffies, transport->last_time_used +
					transport->rto))
			transport->cwnd = max(transport->cwnd/2,
						 4*transport->asoc->pathmtu);
		break;
	}

	transport->partial_bytes_acked = 0;
	SCTP_DEBUG_PRINTK("%s: transport: %p reason: %d cwnd: "
			  "%d ssthresh: %d\n", __func__,
			  transport, reason,
			  transport->cwnd, transport->ssthresh);
}


unsigned long sctp_transport_timeout(struct sctp_transport *t)
{
	unsigned long timeout;
	timeout = t->rto + sctp_jitter(t->rto);
	if (t->state != SCTP_UNCONFIRMED)
		timeout += t->hbinterval;
	timeout += jiffies;
	return timeout;
}


void sctp_transport_reset(struct sctp_transport *t)
{
	struct sctp_association *asoc = t->asoc;

	
	t->cwnd = min(4*asoc->pathmtu, max_t(__u32, 2*asoc->pathmtu, 4380));
	t->ssthresh = asoc->peer.i.a_rwnd;
	t->rto = asoc->rto_initial;
	t->rtt = 0;
	t->srtt = 0;
	t->rttvar = 0;

	
	t->partial_bytes_acked = 0;
	t->flight_size = 0;
	t->error_count = 0;
	t->rto_pending = 0;
	t->hb_sent = 0;
	t->fast_recovery = 0;

	
	t->cacc.changeover_active = 0;
	t->cacc.cycling_changeover = 0;
	t->cacc.next_tsn_at_change = 0;
	t->cacc.cacc_saw_newack = 0;
}
