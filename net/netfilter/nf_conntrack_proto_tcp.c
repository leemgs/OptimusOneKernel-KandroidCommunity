

#include <linux/types.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/ipv6.h>
#include <net/ip6_checksum.h>
#include <asm/unaligned.h>

#include <net/tcp.h>

#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <net/netfilter/nf_log.h>
#include <net/netfilter/ipv4/nf_conntrack_ipv4.h>
#include <net/netfilter/ipv6/nf_conntrack_ipv6.h>


static int nf_ct_tcp_be_liberal __read_mostly = 0;


static int nf_ct_tcp_loose __read_mostly = 1;


static int nf_ct_tcp_max_retrans __read_mostly = 3;

  

static const char *const tcp_conntrack_names[] = {
	"NONE",
	"SYN_SENT",
	"SYN_RECV",
	"ESTABLISHED",
	"FIN_WAIT",
	"CLOSE_WAIT",
	"LAST_ACK",
	"TIME_WAIT",
	"CLOSE",
	"SYN_SENT2",
};

#define SECS * HZ
#define MINS * 60 SECS
#define HOURS * 60 MINS
#define DAYS * 24 HOURS


static unsigned int nf_ct_tcp_timeout_max_retrans __read_mostly    =   5 MINS;
static unsigned int nf_ct_tcp_timeout_unacknowledged __read_mostly =   5 MINS;

static unsigned int tcp_timeouts[TCP_CONNTRACK_MAX] __read_mostly = {
	[TCP_CONNTRACK_SYN_SENT]	= 2 MINS,
	[TCP_CONNTRACK_SYN_RECV]	= 60 SECS,
	[TCP_CONNTRACK_ESTABLISHED]	= 5 DAYS,
	[TCP_CONNTRACK_FIN_WAIT]	= 2 MINS,
	[TCP_CONNTRACK_CLOSE_WAIT]	= 60 SECS,
	[TCP_CONNTRACK_LAST_ACK]	= 30 SECS,
	[TCP_CONNTRACK_TIME_WAIT]	= 2 MINS,
	[TCP_CONNTRACK_CLOSE]		= 10 SECS,
	[TCP_CONNTRACK_SYN_SENT2]	= 2 MINS,
};

#define sNO TCP_CONNTRACK_NONE
#define sSS TCP_CONNTRACK_SYN_SENT
#define sSR TCP_CONNTRACK_SYN_RECV
#define sES TCP_CONNTRACK_ESTABLISHED
#define sFW TCP_CONNTRACK_FIN_WAIT
#define sCW TCP_CONNTRACK_CLOSE_WAIT
#define sLA TCP_CONNTRACK_LAST_ACK
#define sTW TCP_CONNTRACK_TIME_WAIT
#define sCL TCP_CONNTRACK_CLOSE
#define sS2 TCP_CONNTRACK_SYN_SENT2
#define sIV TCP_CONNTRACK_MAX
#define sIG TCP_CONNTRACK_IGNORE


enum tcp_bit_set {
	TCP_SYN_SET,
	TCP_SYNACK_SET,
	TCP_FIN_SET,
	TCP_ACK_SET,
	TCP_RST_SET,
	TCP_NONE_SET,
};


static const u8 tcp_conntracks[2][6][TCP_CONNTRACK_MAX] = {
	{


	   { sSS, sSS, sIG, sIG, sIG, sIG, sIG, sSS, sSS, sS2 },


 { sIV, sIV, sIG, sIG, sIG, sIG, sIG, sIG, sIG, sSR },


    { sIV, sIV, sFW, sFW, sLA, sLA, sLA, sTW, sCL, sIV },


	   { sES, sIV, sES, sES, sCW, sCW, sTW, sTW, sCL, sIV },


    { sIV, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL },
   { sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV }
	},
	{


	   { sIV, sS2, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sS2 },


 { sIV, sSR, sSR, sIG, sIG, sIG, sIG, sIG, sIG, sSR },


    { sIV, sIV, sFW, sFW, sLA, sLA, sLA, sTW, sCL, sIV },


	   { sIV, sIG, sSR, sES, sCW, sCW, sTW, sTW, sCL, sIG },


    { sIV, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL },
   { sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV }
	}
};

static bool tcp_pkt_to_tuple(const struct sk_buff *skb, unsigned int dataoff,
			     struct nf_conntrack_tuple *tuple)
{
	const struct tcphdr *hp;
	struct tcphdr _hdr;

	
	hp = skb_header_pointer(skb, dataoff, 8, &_hdr);
	if (hp == NULL)
		return false;

	tuple->src.u.tcp.port = hp->source;
	tuple->dst.u.tcp.port = hp->dest;

	return true;
}

static bool tcp_invert_tuple(struct nf_conntrack_tuple *tuple,
			     const struct nf_conntrack_tuple *orig)
{
	tuple->src.u.tcp.port = orig->dst.u.tcp.port;
	tuple->dst.u.tcp.port = orig->src.u.tcp.port;
	return true;
}


static int tcp_print_tuple(struct seq_file *s,
			   const struct nf_conntrack_tuple *tuple)
{
	return seq_printf(s, "sport=%hu dport=%hu ",
			  ntohs(tuple->src.u.tcp.port),
			  ntohs(tuple->dst.u.tcp.port));
}


static int tcp_print_conntrack(struct seq_file *s, struct nf_conn *ct)
{
	enum tcp_conntrack state;

	spin_lock_bh(&ct->lock);
	state = ct->proto.tcp.state;
	spin_unlock_bh(&ct->lock);

	return seq_printf(s, "%s ", tcp_conntrack_names[state]);
}

static unsigned int get_conntrack_index(const struct tcphdr *tcph)
{
	if (tcph->rst) return TCP_RST_SET;
	else if (tcph->syn) return (tcph->ack ? TCP_SYNACK_SET : TCP_SYN_SET);
	else if (tcph->fin) return TCP_FIN_SET;
	else if (tcph->ack) return TCP_ACK_SET;
	else return TCP_NONE_SET;
}



static inline __u32 segment_seq_plus_len(__u32 seq,
					 size_t len,
					 unsigned int dataoff,
					 const struct tcphdr *tcph)
{
	
	return (seq + len - dataoff - tcph->doff*4
		+ (tcph->syn ? 1 : 0) + (tcph->fin ? 1 : 0));
}


#define MAXACKWINCONST			66000
#define MAXACKWINDOW(sender)						\
	((sender)->td_maxwin > MAXACKWINCONST ? (sender)->td_maxwin	\
					      : MAXACKWINCONST)


static void tcp_options(const struct sk_buff *skb,
			unsigned int dataoff,
			const struct tcphdr *tcph,
			struct ip_ct_tcp_state *state)
{
	unsigned char buff[(15 * 4) - sizeof(struct tcphdr)];
	const unsigned char *ptr;
	int length = (tcph->doff*4) - sizeof(struct tcphdr);

	if (!length)
		return;

	ptr = skb_header_pointer(skb, dataoff + sizeof(struct tcphdr),
				 length, buff);
	BUG_ON(ptr == NULL);

	state->td_scale =
	state->flags = 0;

	while (length > 0) {
		int opcode=*ptr++;
		int opsize;

		switch (opcode) {
		case TCPOPT_EOL:
			return;
		case TCPOPT_NOP:	
			length--;
			continue;
		default:
			opsize=*ptr++;
			if (opsize < 2) 
				return;
			if (opsize > length)
				break;	

			if (opcode == TCPOPT_SACK_PERM
			    && opsize == TCPOLEN_SACK_PERM)
				state->flags |= IP_CT_TCP_FLAG_SACK_PERM;
			else if (opcode == TCPOPT_WINDOW
				 && opsize == TCPOLEN_WINDOW) {
				state->td_scale = *(u_int8_t *)ptr;

				if (state->td_scale > 14) {
					
					state->td_scale = 14;
				}
				state->flags |=
					IP_CT_TCP_FLAG_WINDOW_SCALE;
			}
			ptr += opsize - 2;
			length -= opsize;
		}
	}
}

static void tcp_sack(const struct sk_buff *skb, unsigned int dataoff,
                     const struct tcphdr *tcph, __u32 *sack)
{
	unsigned char buff[(15 * 4) - sizeof(struct tcphdr)];
	const unsigned char *ptr;
	int length = (tcph->doff*4) - sizeof(struct tcphdr);
	__u32 tmp;

	if (!length)
		return;

	ptr = skb_header_pointer(skb, dataoff + sizeof(struct tcphdr),
				 length, buff);
	BUG_ON(ptr == NULL);

	
	if (length == TCPOLEN_TSTAMP_ALIGNED*4
	    && *(__be32 *)ptr == htonl((TCPOPT_NOP << 24)
				       | (TCPOPT_NOP << 16)
				       | (TCPOPT_TIMESTAMP << 8)
				       | TCPOLEN_TIMESTAMP))
		return;

	while (length > 0) {
		int opcode = *ptr++;
		int opsize, i;

		switch (opcode) {
		case TCPOPT_EOL:
			return;
		case TCPOPT_NOP:	
			length--;
			continue;
		default:
			opsize = *ptr++;
			if (opsize < 2) 
				return;
			if (opsize > length)
				break;	

			if (opcode == TCPOPT_SACK
			    && opsize >= (TCPOLEN_SACK_BASE
					  + TCPOLEN_SACK_PERBLOCK)
			    && !((opsize - TCPOLEN_SACK_BASE)
				 % TCPOLEN_SACK_PERBLOCK)) {
				for (i = 0;
				     i < (opsize - TCPOLEN_SACK_BASE);
				     i += TCPOLEN_SACK_PERBLOCK) {
					tmp = get_unaligned_be32((__be32 *)(ptr+i)+1);

					if (after(tmp, *sack))
						*sack = tmp;
				}
				return;
			}
			ptr += opsize - 2;
			length -= opsize;
		}
	}
}

#ifdef CONFIG_NF_NAT_NEEDED
static inline s16 nat_offset(const struct nf_conn *ct,
			     enum ip_conntrack_dir dir,
			     u32 seq)
{
	typeof(nf_ct_nat_offset) get_offset = rcu_dereference(nf_ct_nat_offset);

	return get_offset != NULL ? get_offset(ct, dir, seq) : 0;
}
#define NAT_OFFSET(pf, ct, dir, seq) \
	(pf == NFPROTO_IPV4 ? nat_offset(ct, dir, seq) : 0)
#else
#define NAT_OFFSET(pf, ct, dir, seq)	0
#endif

static bool tcp_in_window(const struct nf_conn *ct,
			  struct ip_ct_tcp *state,
			  enum ip_conntrack_dir dir,
			  unsigned int index,
			  const struct sk_buff *skb,
			  unsigned int dataoff,
			  const struct tcphdr *tcph,
			  u_int8_t pf)
{
	struct net *net = nf_ct_net(ct);
	struct ip_ct_tcp_state *sender = &state->seen[dir];
	struct ip_ct_tcp_state *receiver = &state->seen[!dir];
	const struct nf_conntrack_tuple *tuple = &ct->tuplehash[dir].tuple;
	__u32 seq, ack, sack, end, win, swin;
	s16 receiver_offset;
	bool res;

	
	seq = ntohl(tcph->seq);
	ack = sack = ntohl(tcph->ack_seq);
	win = ntohs(tcph->window);
	end = segment_seq_plus_len(seq, skb->len, dataoff, tcph);

	if (receiver->flags & IP_CT_TCP_FLAG_SACK_PERM)
		tcp_sack(skb, dataoff, tcph, &sack);

	
	receiver_offset = NAT_OFFSET(pf, ct, !dir, ack - 1);
	ack -= receiver_offset;
	sack -= receiver_offset;

	pr_debug("tcp_in_window: START\n");
	pr_debug("tcp_in_window: ");
	nf_ct_dump_tuple(tuple);
	pr_debug("seq=%u ack=%u+(%d) sack=%u+(%d) win=%u end=%u\n",
		 seq, ack, receiver_offset, sack, receiver_offset, win, end);
	pr_debug("tcp_in_window: sender end=%u maxend=%u maxwin=%u scale=%i "
		 "receiver end=%u maxend=%u maxwin=%u scale=%i\n",
		 sender->td_end, sender->td_maxend, sender->td_maxwin,
		 sender->td_scale,
		 receiver->td_end, receiver->td_maxend, receiver->td_maxwin,
		 receiver->td_scale);

	if (sender->td_maxwin == 0) {
		
		if (tcph->syn) {
			
			sender->td_end =
			sender->td_maxend = end;
			sender->td_maxwin = (win == 0 ? 1 : win);

			tcp_options(skb, dataoff, tcph, sender);
			
			if (!(sender->flags & IP_CT_TCP_FLAG_WINDOW_SCALE
			      && receiver->flags & IP_CT_TCP_FLAG_WINDOW_SCALE))
				sender->td_scale =
				receiver->td_scale = 0;
			if (!tcph->ack)
				
				return true;
		} else {
			
			sender->td_end = end;
			sender->td_maxwin = (win == 0 ? 1 : win);
			sender->td_maxend = end + sender->td_maxwin;
		}
	} else if (((state->state == TCP_CONNTRACK_SYN_SENT
		     && dir == IP_CT_DIR_ORIGINAL)
		   || (state->state == TCP_CONNTRACK_SYN_RECV
		     && dir == IP_CT_DIR_REPLY))
		   && after(end, sender->td_end)) {
		
		sender->td_end =
		sender->td_maxend = end;
		sender->td_maxwin = (win == 0 ? 1 : win);

		tcp_options(skb, dataoff, tcph, sender);
	}

	if (!(tcph->ack)) {
		
		ack = sack = receiver->td_end;
	} else if (((tcp_flag_word(tcph) & (TCP_FLAG_ACK|TCP_FLAG_RST)) ==
		    (TCP_FLAG_ACK|TCP_FLAG_RST))
		   && (ack == 0)) {
		
		ack = sack = receiver->td_end;
	}

	if (seq == end
	    && (!tcph->rst
		|| (seq == 0 && state->state == TCP_CONNTRACK_SYN_SENT)))
		
		seq = end = sender->td_end;

	pr_debug("tcp_in_window: ");
	nf_ct_dump_tuple(tuple);
	pr_debug("seq=%u ack=%u+(%d) sack=%u+(%d) win=%u end=%u\n",
		 seq, ack, receiver_offset, sack, receiver_offset, win, end);
	pr_debug("tcp_in_window: sender end=%u maxend=%u maxwin=%u scale=%i "
		 "receiver end=%u maxend=%u maxwin=%u scale=%i\n",
		 sender->td_end, sender->td_maxend, sender->td_maxwin,
		 sender->td_scale,
		 receiver->td_end, receiver->td_maxend, receiver->td_maxwin,
		 receiver->td_scale);

	pr_debug("tcp_in_window: I=%i II=%i III=%i IV=%i\n",
		 before(seq, sender->td_maxend + 1),
		 after(end, sender->td_end - receiver->td_maxwin - 1),
		 before(sack, receiver->td_end + 1),
		 after(sack, receiver->td_end - MAXACKWINDOW(sender) - 1));

	if (before(seq, sender->td_maxend + 1) &&
	    after(end, sender->td_end - receiver->td_maxwin - 1) &&
	    before(sack, receiver->td_end + 1) &&
	    after(sack, receiver->td_end - MAXACKWINDOW(sender) - 1)) {
		
		if (!tcph->syn)
			win <<= sender->td_scale;

		
		swin = win + (sack - ack);
		if (sender->td_maxwin < swin)
			sender->td_maxwin = swin;
		if (after(end, sender->td_end)) {
			sender->td_end = end;
			sender->flags |= IP_CT_TCP_FLAG_DATA_UNACKNOWLEDGED;
		}
		if (tcph->ack) {
			if (!(sender->flags & IP_CT_TCP_FLAG_MAXACK_SET)) {
				sender->td_maxack = ack;
				sender->flags |= IP_CT_TCP_FLAG_MAXACK_SET;
			} else if (after(ack, sender->td_maxack))
				sender->td_maxack = ack;
		}

		
		if (after(end, sender->td_maxend))
			receiver->td_maxwin += end - sender->td_maxend;
		if (after(sack + win, receiver->td_maxend - 1)) {
			receiver->td_maxend = sack + win;
			if (win == 0)
				receiver->td_maxend++;
		}
		if (ack == receiver->td_end)
			receiver->flags &= ~IP_CT_TCP_FLAG_DATA_UNACKNOWLEDGED;

		
		if (index == TCP_ACK_SET) {
			if (state->last_dir == dir
			    && state->last_seq == seq
			    && state->last_ack == ack
			    && state->last_end == end
			    && state->last_win == win)
				state->retrans++;
			else {
				state->last_dir = dir;
				state->last_seq = seq;
				state->last_ack = ack;
				state->last_end = end;
				state->last_win = win;
				state->retrans = 0;
			}
		}
		res = true;
	} else {
		res = false;
		if (sender->flags & IP_CT_TCP_FLAG_BE_LIBERAL ||
		    nf_ct_tcp_be_liberal)
			res = true;
		if (!res && LOG_INVALID(net, IPPROTO_TCP))
			nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
			"nf_ct_tcp: %s ",
			before(seq, sender->td_maxend + 1) ?
			after(end, sender->td_end - receiver->td_maxwin - 1) ?
			before(sack, receiver->td_end + 1) ?
			after(sack, receiver->td_end - MAXACKWINDOW(sender) - 1) ? "BUG"
			: "ACK is under the lower bound (possible overly delayed ACK)"
			: "ACK is over the upper bound (ACKed data not seen yet)"
			: "SEQ is under the lower bound (already ACKed data retransmitted)"
			: "SEQ is over the upper bound (over the window of the receiver)");
	}

	pr_debug("tcp_in_window: res=%u sender end=%u maxend=%u maxwin=%u "
		 "receiver end=%u maxend=%u maxwin=%u\n",
		 res, sender->td_end, sender->td_maxend, sender->td_maxwin,
		 receiver->td_end, receiver->td_maxend, receiver->td_maxwin);

	return res;
}

#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20
#define	TH_ECE	0x40
#define	TH_CWR	0x80


static const u8 tcp_valid_flags[(TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG) + 1] =
{
	[TH_SYN]			= 1,
	[TH_SYN|TH_URG]			= 1,
	[TH_SYN|TH_ACK]			= 1,
	[TH_RST]			= 1,
	[TH_RST|TH_ACK]			= 1,
	[TH_FIN|TH_ACK]			= 1,
	[TH_FIN|TH_ACK|TH_URG]		= 1,
	[TH_ACK]			= 1,
	[TH_ACK|TH_URG]			= 1,
};


static int tcp_error(struct net *net,
		     struct sk_buff *skb,
		     unsigned int dataoff,
		     enum ip_conntrack_info *ctinfo,
		     u_int8_t pf,
		     unsigned int hooknum)
{
	const struct tcphdr *th;
	struct tcphdr _tcph;
	unsigned int tcplen = skb->len - dataoff;
	u_int8_t tcpflags;

	
	th = skb_header_pointer(skb, dataoff, sizeof(_tcph), &_tcph);
	if (th == NULL) {
		if (LOG_INVALID(net, IPPROTO_TCP))
			nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
				"nf_ct_tcp: short packet ");
		return -NF_ACCEPT;
	}

	
	if (th->doff*4 < sizeof(struct tcphdr) || tcplen < th->doff*4) {
		if (LOG_INVALID(net, IPPROTO_TCP))
			nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
				"nf_ct_tcp: truncated/malformed packet ");
		return -NF_ACCEPT;
	}

	
	
	if (net->ct.sysctl_checksum && hooknum == NF_INET_PRE_ROUTING &&
	    nf_checksum(skb, hooknum, dataoff, IPPROTO_TCP, pf)) {
		if (LOG_INVALID(net, IPPROTO_TCP))
			nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
				  "nf_ct_tcp: bad TCP checksum ");
		return -NF_ACCEPT;
	}

	
	tcpflags = (((u_int8_t *)th)[13] & ~(TH_ECE|TH_CWR|TH_PUSH));
	if (!tcp_valid_flags[tcpflags]) {
		if (LOG_INVALID(net, IPPROTO_TCP))
			nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
				  "nf_ct_tcp: invalid TCP flag combination ");
		return -NF_ACCEPT;
	}

	return NF_ACCEPT;
}


static int tcp_packet(struct nf_conn *ct,
		      const struct sk_buff *skb,
		      unsigned int dataoff,
		      enum ip_conntrack_info ctinfo,
		      u_int8_t pf,
		      unsigned int hooknum)
{
	struct net *net = nf_ct_net(ct);
	struct nf_conntrack_tuple *tuple;
	enum tcp_conntrack new_state, old_state;
	enum ip_conntrack_dir dir;
	const struct tcphdr *th;
	struct tcphdr _tcph;
	unsigned long timeout;
	unsigned int index;

	th = skb_header_pointer(skb, dataoff, sizeof(_tcph), &_tcph);
	BUG_ON(th == NULL);

	spin_lock_bh(&ct->lock);
	old_state = ct->proto.tcp.state;
	dir = CTINFO2DIR(ctinfo);
	index = get_conntrack_index(th);
	new_state = tcp_conntracks[dir][index][old_state];
	tuple = &ct->tuplehash[dir].tuple;

	switch (new_state) {
	case TCP_CONNTRACK_SYN_SENT:
		if (old_state < TCP_CONNTRACK_TIME_WAIT)
			break;
		
		if (((ct->proto.tcp.seen[dir].flags
		      | ct->proto.tcp.seen[!dir].flags)
		     & IP_CT_TCP_FLAG_CLOSE_INIT)
		    || (ct->proto.tcp.last_dir == dir
		        && ct->proto.tcp.last_index == TCP_RST_SET)) {
			
			spin_unlock_bh(&ct->lock);

			
			if (nf_ct_kill(ct))
				return -NF_REPEAT;
			return NF_DROP;
		}
		
	case TCP_CONNTRACK_IGNORE:
		
		if (index == TCP_SYNACK_SET
		    && ct->proto.tcp.last_index == TCP_SYN_SET
		    && ct->proto.tcp.last_dir != dir
		    && ntohl(th->ack_seq) == ct->proto.tcp.last_end) {
			
			spin_unlock_bh(&ct->lock);
			if (LOG_INVALID(net, IPPROTO_TCP))
				nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
					  "nf_ct_tcp: killing out of sync session ");
			nf_ct_kill(ct);
			return NF_DROP;
		}
		ct->proto.tcp.last_index = index;
		ct->proto.tcp.last_dir = dir;
		ct->proto.tcp.last_seq = ntohl(th->seq);
		ct->proto.tcp.last_end =
		    segment_seq_plus_len(ntohl(th->seq), skb->len, dataoff, th);

		spin_unlock_bh(&ct->lock);
		if (LOG_INVALID(net, IPPROTO_TCP))
			nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
				  "nf_ct_tcp: invalid packet ignored ");
		return NF_ACCEPT;
	case TCP_CONNTRACK_MAX:
		
		pr_debug("nf_ct_tcp: Invalid dir=%i index=%u ostate=%u\n",
			 dir, get_conntrack_index(th), old_state);
		spin_unlock_bh(&ct->lock);
		if (LOG_INVALID(net, IPPROTO_TCP))
			nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
				  "nf_ct_tcp: invalid state ");
		return -NF_ACCEPT;
	case TCP_CONNTRACK_CLOSE:
		if (index == TCP_RST_SET
		    && (ct->proto.tcp.seen[!dir].flags & IP_CT_TCP_FLAG_MAXACK_SET)
		    && before(ntohl(th->seq), ct->proto.tcp.seen[!dir].td_maxack)) {
			
			spin_unlock_bh(&ct->lock);
			if (LOG_INVALID(net, IPPROTO_TCP))
				nf_log_packet(pf, 0, skb, NULL, NULL, NULL,
					  "nf_ct_tcp: invalid RST ");
			return -NF_ACCEPT;
		}
		if (index == TCP_RST_SET
		    && ((test_bit(IPS_SEEN_REPLY_BIT, &ct->status)
			 && ct->proto.tcp.last_index == TCP_SYN_SET)
			|| (!test_bit(IPS_ASSURED_BIT, &ct->status)
			    && ct->proto.tcp.last_index == TCP_ACK_SET))
		    && ntohl(th->ack_seq) == ct->proto.tcp.last_end) {
			
			goto in_window;
		}
		
	default:
		
		break;
	}

	if (!tcp_in_window(ct, &ct->proto.tcp, dir, index,
			   skb, dataoff, th, pf)) {
		spin_unlock_bh(&ct->lock);
		return -NF_ACCEPT;
	}
     in_window:
	
	ct->proto.tcp.last_index = index;
	ct->proto.tcp.last_dir = dir;

	pr_debug("tcp_conntracks: ");
	nf_ct_dump_tuple(tuple);
	pr_debug("syn=%i ack=%i fin=%i rst=%i old=%i new=%i\n",
		 (th->syn ? 1 : 0), (th->ack ? 1 : 0),
		 (th->fin ? 1 : 0), (th->rst ? 1 : 0),
		 old_state, new_state);

	ct->proto.tcp.state = new_state;
	if (old_state != new_state
	    && new_state == TCP_CONNTRACK_FIN_WAIT)
		ct->proto.tcp.seen[dir].flags |= IP_CT_TCP_FLAG_CLOSE_INIT;

	if (ct->proto.tcp.retrans >= nf_ct_tcp_max_retrans &&
	    tcp_timeouts[new_state] > nf_ct_tcp_timeout_max_retrans)
		timeout = nf_ct_tcp_timeout_max_retrans;
	else if ((ct->proto.tcp.seen[0].flags | ct->proto.tcp.seen[1].flags) &
		 IP_CT_TCP_FLAG_DATA_UNACKNOWLEDGED &&
		 tcp_timeouts[new_state] > nf_ct_tcp_timeout_unacknowledged)
		timeout = nf_ct_tcp_timeout_unacknowledged;
	else
		timeout = tcp_timeouts[new_state];
	spin_unlock_bh(&ct->lock);

	if (new_state != old_state)
		nf_conntrack_event_cache(IPCT_PROTOINFO, ct);

	if (!test_bit(IPS_SEEN_REPLY_BIT, &ct->status)) {
		
		if (th->rst) {
			nf_ct_kill_acct(ct, ctinfo, skb);
			return NF_ACCEPT;
		}
	} else if (!test_bit(IPS_ASSURED_BIT, &ct->status)
		   && (old_state == TCP_CONNTRACK_SYN_RECV
		       || old_state == TCP_CONNTRACK_ESTABLISHED)
		   && new_state == TCP_CONNTRACK_ESTABLISHED) {
		
		set_bit(IPS_ASSURED_BIT, &ct->status);
		nf_conntrack_event_cache(IPCT_STATUS, ct);
	}
	nf_ct_refresh_acct(ct, ctinfo, skb, timeout);

	return NF_ACCEPT;
}


static bool tcp_new(struct nf_conn *ct, const struct sk_buff *skb,
		    unsigned int dataoff)
{
	enum tcp_conntrack new_state;
	const struct tcphdr *th;
	struct tcphdr _tcph;
	const struct ip_ct_tcp_state *sender = &ct->proto.tcp.seen[0];
	const struct ip_ct_tcp_state *receiver = &ct->proto.tcp.seen[1];

	th = skb_header_pointer(skb, dataoff, sizeof(_tcph), &_tcph);
	BUG_ON(th == NULL);

	
	new_state
		= tcp_conntracks[0][get_conntrack_index(th)]
		[TCP_CONNTRACK_NONE];

	
	if (new_state >= TCP_CONNTRACK_MAX) {
		pr_debug("nf_ct_tcp: invalid new deleting.\n");
		return false;
	}

	if (new_state == TCP_CONNTRACK_SYN_SENT) {
		
		ct->proto.tcp.seen[0].td_end =
			segment_seq_plus_len(ntohl(th->seq), skb->len,
					     dataoff, th);
		ct->proto.tcp.seen[0].td_maxwin = ntohs(th->window);
		if (ct->proto.tcp.seen[0].td_maxwin == 0)
			ct->proto.tcp.seen[0].td_maxwin = 1;
		ct->proto.tcp.seen[0].td_maxend =
			ct->proto.tcp.seen[0].td_end;

		tcp_options(skb, dataoff, th, &ct->proto.tcp.seen[0]);
		ct->proto.tcp.seen[1].flags = 0;
	} else if (nf_ct_tcp_loose == 0) {
		
		return false;
	} else {
		
		ct->proto.tcp.seen[0].td_end =
			segment_seq_plus_len(ntohl(th->seq), skb->len,
					     dataoff, th);
		ct->proto.tcp.seen[0].td_maxwin = ntohs(th->window);
		if (ct->proto.tcp.seen[0].td_maxwin == 0)
			ct->proto.tcp.seen[0].td_maxwin = 1;
		ct->proto.tcp.seen[0].td_maxend =
			ct->proto.tcp.seen[0].td_end +
			ct->proto.tcp.seen[0].td_maxwin;
		ct->proto.tcp.seen[0].td_scale = 0;

		
		ct->proto.tcp.seen[0].flags =
		ct->proto.tcp.seen[1].flags = IP_CT_TCP_FLAG_SACK_PERM |
					      IP_CT_TCP_FLAG_BE_LIBERAL;
	}

	ct->proto.tcp.seen[1].td_end = 0;
	ct->proto.tcp.seen[1].td_maxend = 0;
	ct->proto.tcp.seen[1].td_maxwin = 0;
	ct->proto.tcp.seen[1].td_scale = 0;

	
	ct->proto.tcp.state = TCP_CONNTRACK_NONE;
	ct->proto.tcp.last_index = TCP_NONE_SET;

	pr_debug("tcp_new: sender end=%u maxend=%u maxwin=%u scale=%i "
		 "receiver end=%u maxend=%u maxwin=%u scale=%i\n",
		 sender->td_end, sender->td_maxend, sender->td_maxwin,
		 sender->td_scale,
		 receiver->td_end, receiver->td_maxend, receiver->td_maxwin,
		 receiver->td_scale);
	return true;
}

#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

static int tcp_to_nlattr(struct sk_buff *skb, struct nlattr *nla,
			 struct nf_conn *ct)
{
	struct nlattr *nest_parms;
	struct nf_ct_tcp_flags tmp = {};

	spin_lock_bh(&ct->lock);
	nest_parms = nla_nest_start(skb, CTA_PROTOINFO_TCP | NLA_F_NESTED);
	if (!nest_parms)
		goto nla_put_failure;

	NLA_PUT_U8(skb, CTA_PROTOINFO_TCP_STATE, ct->proto.tcp.state);

	NLA_PUT_U8(skb, CTA_PROTOINFO_TCP_WSCALE_ORIGINAL,
		   ct->proto.tcp.seen[0].td_scale);

	NLA_PUT_U8(skb, CTA_PROTOINFO_TCP_WSCALE_REPLY,
		   ct->proto.tcp.seen[1].td_scale);

	tmp.flags = ct->proto.tcp.seen[0].flags;
	NLA_PUT(skb, CTA_PROTOINFO_TCP_FLAGS_ORIGINAL,
		sizeof(struct nf_ct_tcp_flags), &tmp);

	tmp.flags = ct->proto.tcp.seen[1].flags;
	NLA_PUT(skb, CTA_PROTOINFO_TCP_FLAGS_REPLY,
		sizeof(struct nf_ct_tcp_flags), &tmp);
	spin_unlock_bh(&ct->lock);

	nla_nest_end(skb, nest_parms);

	return 0;

nla_put_failure:
	spin_unlock_bh(&ct->lock);
	return -1;
}

static const struct nla_policy tcp_nla_policy[CTA_PROTOINFO_TCP_MAX+1] = {
	[CTA_PROTOINFO_TCP_STATE]	    = { .type = NLA_U8 },
	[CTA_PROTOINFO_TCP_WSCALE_ORIGINAL] = { .type = NLA_U8 },
	[CTA_PROTOINFO_TCP_WSCALE_REPLY]    = { .type = NLA_U8 },
	[CTA_PROTOINFO_TCP_FLAGS_ORIGINAL]  = { .len = sizeof(struct nf_ct_tcp_flags) },
	[CTA_PROTOINFO_TCP_FLAGS_REPLY]	    = { .len =  sizeof(struct nf_ct_tcp_flags) },
};

static int nlattr_to_tcp(struct nlattr *cda[], struct nf_conn *ct)
{
	struct nlattr *pattr = cda[CTA_PROTOINFO_TCP];
	struct nlattr *tb[CTA_PROTOINFO_TCP_MAX+1];
	int err;

	
	if (!pattr)
		return 0;

	err = nla_parse_nested(tb, CTA_PROTOINFO_TCP_MAX, pattr, tcp_nla_policy);
	if (err < 0)
		return err;

	if (tb[CTA_PROTOINFO_TCP_STATE] &&
	    nla_get_u8(tb[CTA_PROTOINFO_TCP_STATE]) >= TCP_CONNTRACK_MAX)
		return -EINVAL;

	spin_lock_bh(&ct->lock);
	if (tb[CTA_PROTOINFO_TCP_STATE])
		ct->proto.tcp.state = nla_get_u8(tb[CTA_PROTOINFO_TCP_STATE]);

	if (tb[CTA_PROTOINFO_TCP_FLAGS_ORIGINAL]) {
		struct nf_ct_tcp_flags *attr =
			nla_data(tb[CTA_PROTOINFO_TCP_FLAGS_ORIGINAL]);
		ct->proto.tcp.seen[0].flags &= ~attr->mask;
		ct->proto.tcp.seen[0].flags |= attr->flags & attr->mask;
	}

	if (tb[CTA_PROTOINFO_TCP_FLAGS_REPLY]) {
		struct nf_ct_tcp_flags *attr =
			nla_data(tb[CTA_PROTOINFO_TCP_FLAGS_REPLY]);
		ct->proto.tcp.seen[1].flags &= ~attr->mask;
		ct->proto.tcp.seen[1].flags |= attr->flags & attr->mask;
	}

	if (tb[CTA_PROTOINFO_TCP_WSCALE_ORIGINAL] &&
	    tb[CTA_PROTOINFO_TCP_WSCALE_REPLY] &&
	    ct->proto.tcp.seen[0].flags & IP_CT_TCP_FLAG_WINDOW_SCALE &&
	    ct->proto.tcp.seen[1].flags & IP_CT_TCP_FLAG_WINDOW_SCALE) {
		ct->proto.tcp.seen[0].td_scale =
			nla_get_u8(tb[CTA_PROTOINFO_TCP_WSCALE_ORIGINAL]);
		ct->proto.tcp.seen[1].td_scale =
			nla_get_u8(tb[CTA_PROTOINFO_TCP_WSCALE_REPLY]);
	}
	spin_unlock_bh(&ct->lock);

	return 0;
}

static int tcp_nlattr_size(void)
{
	return nla_total_size(0)	   
		+ nla_policy_len(tcp_nla_policy, CTA_PROTOINFO_TCP_MAX + 1);
}

static int tcp_nlattr_tuple_size(void)
{
	return nla_policy_len(nf_ct_port_nla_policy, CTA_PROTO_MAX + 1);
}
#endif

#ifdef CONFIG_SYSCTL
static unsigned int tcp_sysctl_table_users;
static struct ctl_table_header *tcp_sysctl_header;
static struct ctl_table tcp_sysctl_table[] = {
	{
		.procname	= "nf_conntrack_tcp_timeout_syn_sent",
		.data		= &tcp_timeouts[TCP_CONNTRACK_SYN_SENT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_tcp_timeout_syn_recv",
		.data		= &tcp_timeouts[TCP_CONNTRACK_SYN_RECV],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_tcp_timeout_established",
		.data		= &tcp_timeouts[TCP_CONNTRACK_ESTABLISHED],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_tcp_timeout_fin_wait",
		.data		= &tcp_timeouts[TCP_CONNTRACK_FIN_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_tcp_timeout_close_wait",
		.data		= &tcp_timeouts[TCP_CONNTRACK_CLOSE_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_tcp_timeout_last_ack",
		.data		= &tcp_timeouts[TCP_CONNTRACK_LAST_ACK],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_tcp_timeout_time_wait",
		.data		= &tcp_timeouts[TCP_CONNTRACK_TIME_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_tcp_timeout_close",
		.data		= &tcp_timeouts[TCP_CONNTRACK_CLOSE],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_tcp_timeout_max_retrans",
		.data		= &nf_ct_tcp_timeout_max_retrans,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_tcp_timeout_unacknowledged",
		.data		= &nf_ct_tcp_timeout_unacknowledged,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.ctl_name	= NET_NF_CONNTRACK_TCP_LOOSE,
		.procname	= "nf_conntrack_tcp_loose",
		.data		= &nf_ct_tcp_loose,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= NET_NF_CONNTRACK_TCP_BE_LIBERAL,
		.procname       = "nf_conntrack_tcp_be_liberal",
		.data           = &nf_ct_tcp_be_liberal,
		.maxlen         = sizeof(unsigned int),
		.mode           = 0644,
		.proc_handler   = proc_dointvec,
	},
	{
		.ctl_name	= NET_NF_CONNTRACK_TCP_MAX_RETRANS,
		.procname	= "nf_conntrack_tcp_max_retrans",
		.data		= &nf_ct_tcp_max_retrans,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= 0
	}
};

#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
static struct ctl_table tcp_compat_sysctl_table[] = {
	{
		.procname	= "ip_conntrack_tcp_timeout_syn_sent",
		.data		= &tcp_timeouts[TCP_CONNTRACK_SYN_SENT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_syn_sent2",
		.data		= &tcp_timeouts[TCP_CONNTRACK_SYN_SENT2],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_syn_recv",
		.data		= &tcp_timeouts[TCP_CONNTRACK_SYN_RECV],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_established",
		.data		= &tcp_timeouts[TCP_CONNTRACK_ESTABLISHED],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_fin_wait",
		.data		= &tcp_timeouts[TCP_CONNTRACK_FIN_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_close_wait",
		.data		= &tcp_timeouts[TCP_CONNTRACK_CLOSE_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_last_ack",
		.data		= &tcp_timeouts[TCP_CONNTRACK_LAST_ACK],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_time_wait",
		.data		= &tcp_timeouts[TCP_CONNTRACK_TIME_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_close",
		.data		= &tcp_timeouts[TCP_CONNTRACK_CLOSE],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_tcp_timeout_max_retrans",
		.data		= &nf_ct_tcp_timeout_max_retrans,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.ctl_name	= NET_IPV4_NF_CONNTRACK_TCP_LOOSE,
		.procname	= "ip_conntrack_tcp_loose",
		.data		= &nf_ct_tcp_loose,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_NF_CONNTRACK_TCP_BE_LIBERAL,
		.procname	= "ip_conntrack_tcp_be_liberal",
		.data		= &nf_ct_tcp_be_liberal,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= NET_IPV4_NF_CONNTRACK_TCP_MAX_RETRANS,
		.procname	= "ip_conntrack_tcp_max_retrans",
		.data		= &nf_ct_tcp_max_retrans,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.ctl_name	= 0
	}
};
#endif 
#endif 

struct nf_conntrack_l4proto nf_conntrack_l4proto_tcp4 __read_mostly =
{
	.l3proto		= PF_INET,
	.l4proto 		= IPPROTO_TCP,
	.name 			= "tcp",
	.pkt_to_tuple 		= tcp_pkt_to_tuple,
	.invert_tuple 		= tcp_invert_tuple,
	.print_tuple 		= tcp_print_tuple,
	.print_conntrack 	= tcp_print_conntrack,
	.packet 		= tcp_packet,
	.new 			= tcp_new,
	.error			= tcp_error,
#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
	.to_nlattr		= tcp_to_nlattr,
	.nlattr_size		= tcp_nlattr_size,
	.from_nlattr		= nlattr_to_tcp,
	.tuple_to_nlattr	= nf_ct_port_tuple_to_nlattr,
	.nlattr_to_tuple	= nf_ct_port_nlattr_to_tuple,
	.nlattr_tuple_size	= tcp_nlattr_tuple_size,
	.nla_policy		= nf_ct_port_nla_policy,
#endif
#ifdef CONFIG_SYSCTL
	.ctl_table_users	= &tcp_sysctl_table_users,
	.ctl_table_header	= &tcp_sysctl_header,
	.ctl_table		= tcp_sysctl_table,
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
	.ctl_compat_table	= tcp_compat_sysctl_table,
#endif
#endif
};
EXPORT_SYMBOL_GPL(nf_conntrack_l4proto_tcp4);

struct nf_conntrack_l4proto nf_conntrack_l4proto_tcp6 __read_mostly =
{
	.l3proto		= PF_INET6,
	.l4proto 		= IPPROTO_TCP,
	.name 			= "tcp",
	.pkt_to_tuple 		= tcp_pkt_to_tuple,
	.invert_tuple 		= tcp_invert_tuple,
	.print_tuple 		= tcp_print_tuple,
	.print_conntrack 	= tcp_print_conntrack,
	.packet 		= tcp_packet,
	.new 			= tcp_new,
	.error			= tcp_error,
#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
	.to_nlattr		= tcp_to_nlattr,
	.nlattr_size		= tcp_nlattr_size,
	.from_nlattr		= nlattr_to_tcp,
	.tuple_to_nlattr	= nf_ct_port_tuple_to_nlattr,
	.nlattr_to_tuple	= nf_ct_port_nlattr_to_tuple,
	.nlattr_tuple_size	= tcp_nlattr_tuple_size,
	.nla_policy		= nf_ct_port_nla_policy,
#endif
#ifdef CONFIG_SYSCTL
	.ctl_table_users	= &tcp_sysctl_table_users,
	.ctl_table_header	= &tcp_sysctl_header,
	.ctl_table		= tcp_sysctl_table,
#endif
};
EXPORT_SYMBOL_GPL(nf_conntrack_l4proto_tcp6);
