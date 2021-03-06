

#include <linux/types.h>
#include <linux/timer.h>
#include <linux/netfilter.h>
#include <linux/module.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/sctp.h>
#include <linux/string.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_ecache.h>



static const char *const sctp_conntrack_names[] = {
	"NONE",
	"CLOSED",
	"COOKIE_WAIT",
	"COOKIE_ECHOED",
	"ESTABLISHED",
	"SHUTDOWN_SENT",
	"SHUTDOWN_RECD",
	"SHUTDOWN_ACK_SENT",
};

#define SECS  * HZ
#define MINS  * 60 SECS
#define HOURS * 60 MINS
#define DAYS  * 24 HOURS

static unsigned int sctp_timeouts[SCTP_CONNTRACK_MAX] __read_mostly = {
	[SCTP_CONNTRACK_CLOSED]			= 10 SECS,
	[SCTP_CONNTRACK_COOKIE_WAIT]		= 3 SECS,
	[SCTP_CONNTRACK_COOKIE_ECHOED]		= 3 SECS,
	[SCTP_CONNTRACK_ESTABLISHED]		= 5 DAYS,
	[SCTP_CONNTRACK_SHUTDOWN_SENT]		= 300 SECS / 1000,
	[SCTP_CONNTRACK_SHUTDOWN_RECD]		= 300 SECS / 1000,
	[SCTP_CONNTRACK_SHUTDOWN_ACK_SENT]	= 3 SECS,
};

#define sNO SCTP_CONNTRACK_NONE
#define	sCL SCTP_CONNTRACK_CLOSED
#define	sCW SCTP_CONNTRACK_COOKIE_WAIT
#define	sCE SCTP_CONNTRACK_COOKIE_ECHOED
#define	sES SCTP_CONNTRACK_ESTABLISHED
#define	sSS SCTP_CONNTRACK_SHUTDOWN_SENT
#define	sSR SCTP_CONNTRACK_SHUTDOWN_RECD
#define	sSA SCTP_CONNTRACK_SHUTDOWN_ACK_SENT
#define	sIV SCTP_CONNTRACK_MAX






static const u8 sctp_conntracks[2][9][SCTP_CONNTRACK_MAX] = {
	{


 {sCW, sCW, sCW, sCE, sES, sSS, sSR, sSA},
 {sCL, sCL, sCW, sCE, sES, sSS, sSR, sSA},
 {sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL},
 {sCL, sCL, sCW, sCE, sSS, sSS, sSR, sSA},
 {sSA, sCL, sCW, sCE, sES, sSA, sSA, sSA},
 {sCL, sCL, sCW, sCE, sES, sSS, sSR, sSA},
 {sCL, sCL, sCE, sCE, sES, sSS, sSR, sSA},
 {sCL, sCL, sCW, sCE, sES, sSS, sSR, sSA},
 {sCL, sCL, sCW, sCE, sES, sSS, sSR, sCL}
	},
	{


 {sIV, sCL, sCW, sCE, sES, sSS, sSR, sSA},
 {sIV, sCL, sCW, sCE, sES, sSS, sSR, sSA},
 {sIV, sCL, sCL, sCL, sCL, sCL, sCL, sCL},
 {sIV, sCL, sCW, sCE, sSR, sSS, sSR, sSA},
 {sIV, sCL, sCW, sCE, sES, sSA, sSA, sSA},
 {sIV, sCL, sCW, sCL, sES, sSS, sSR, sSA},
 {sIV, sCL, sCW, sCE, sES, sSS, sSR, sSA},
 {sIV, sCL, sCW, sES, sES, sSS, sSR, sSA},
 {sIV, sCL, sCW, sCE, sES, sSS, sSR, sCL}
	}
};

static bool sctp_pkt_to_tuple(const struct sk_buff *skb, unsigned int dataoff,
			      struct nf_conntrack_tuple *tuple)
{
	const struct sctphdr *hp;
	struct sctphdr _hdr;

	
	hp = skb_header_pointer(skb, dataoff, 8, &_hdr);
	if (hp == NULL)
		return false;

	tuple->src.u.sctp.port = hp->source;
	tuple->dst.u.sctp.port = hp->dest;
	return true;
}

static bool sctp_invert_tuple(struct nf_conntrack_tuple *tuple,
			      const struct nf_conntrack_tuple *orig)
{
	tuple->src.u.sctp.port = orig->dst.u.sctp.port;
	tuple->dst.u.sctp.port = orig->src.u.sctp.port;
	return true;
}


static int sctp_print_tuple(struct seq_file *s,
			    const struct nf_conntrack_tuple *tuple)
{
	return seq_printf(s, "sport=%hu dport=%hu ",
			  ntohs(tuple->src.u.sctp.port),
			  ntohs(tuple->dst.u.sctp.port));
}


static int sctp_print_conntrack(struct seq_file *s, struct nf_conn *ct)
{
	enum sctp_conntrack state;

	spin_lock_bh(&ct->lock);
	state = ct->proto.sctp.state;
	spin_unlock_bh(&ct->lock);

	return seq_printf(s, "%s ", sctp_conntrack_names[state]);
}

#define for_each_sctp_chunk(skb, sch, _sch, offset, dataoff, count)	\
for ((offset) = (dataoff) + sizeof(sctp_sctphdr_t), (count) = 0;	\
	(offset) < (skb)->len &&					\
	((sch) = skb_header_pointer((skb), (offset), sizeof(_sch), &(_sch)));	\
	(offset) += (ntohs((sch)->length) + 3) & ~3, (count)++)


static int do_basic_checks(struct nf_conn *ct,
			   const struct sk_buff *skb,
			   unsigned int dataoff,
			   unsigned long *map)
{
	u_int32_t offset, count;
	sctp_chunkhdr_t _sch, *sch;
	int flag;

	flag = 0;

	for_each_sctp_chunk (skb, sch, _sch, offset, dataoff, count) {
		pr_debug("Chunk Num: %d  Type: %d\n", count, sch->type);

		if (sch->type == SCTP_CID_INIT ||
		    sch->type == SCTP_CID_INIT_ACK ||
		    sch->type == SCTP_CID_SHUTDOWN_COMPLETE)
			flag = 1;

		
		if (((sch->type == SCTP_CID_COOKIE_ACK ||
		      sch->type == SCTP_CID_COOKIE_ECHO ||
		      flag) &&
		     count != 0) || !sch->length) {
			pr_debug("Basic checks failed\n");
			return 1;
		}

		if (map)
			set_bit(sch->type, map);
	}

	pr_debug("Basic checks passed\n");
	return count == 0;
}

static int sctp_new_state(enum ip_conntrack_dir dir,
			  enum sctp_conntrack cur_state,
			  int chunk_type)
{
	int i;

	pr_debug("Chunk type: %d\n", chunk_type);

	switch (chunk_type) {
	case SCTP_CID_INIT:
		pr_debug("SCTP_CID_INIT\n");
		i = 0;
		break;
	case SCTP_CID_INIT_ACK:
		pr_debug("SCTP_CID_INIT_ACK\n");
		i = 1;
		break;
	case SCTP_CID_ABORT:
		pr_debug("SCTP_CID_ABORT\n");
		i = 2;
		break;
	case SCTP_CID_SHUTDOWN:
		pr_debug("SCTP_CID_SHUTDOWN\n");
		i = 3;
		break;
	case SCTP_CID_SHUTDOWN_ACK:
		pr_debug("SCTP_CID_SHUTDOWN_ACK\n");
		i = 4;
		break;
	case SCTP_CID_ERROR:
		pr_debug("SCTP_CID_ERROR\n");
		i = 5;
		break;
	case SCTP_CID_COOKIE_ECHO:
		pr_debug("SCTP_CID_COOKIE_ECHO\n");
		i = 6;
		break;
	case SCTP_CID_COOKIE_ACK:
		pr_debug("SCTP_CID_COOKIE_ACK\n");
		i = 7;
		break;
	case SCTP_CID_SHUTDOWN_COMPLETE:
		pr_debug("SCTP_CID_SHUTDOWN_COMPLETE\n");
		i = 8;
		break;
	default:
		
		pr_debug("Unknown chunk type, Will stay in %s\n",
			 sctp_conntrack_names[cur_state]);
		return cur_state;
	}

	pr_debug("dir: %d   cur_state: %s  chunk_type: %d  new_state: %s\n",
		 dir, sctp_conntrack_names[cur_state], chunk_type,
		 sctp_conntrack_names[sctp_conntracks[dir][i][cur_state]]);

	return sctp_conntracks[dir][i][cur_state];
}


static int sctp_packet(struct nf_conn *ct,
		       const struct sk_buff *skb,
		       unsigned int dataoff,
		       enum ip_conntrack_info ctinfo,
		       u_int8_t pf,
		       unsigned int hooknum)
{
	enum sctp_conntrack new_state, old_state;
	enum ip_conntrack_dir dir = CTINFO2DIR(ctinfo);
	const struct sctphdr *sh;
	struct sctphdr _sctph;
	const struct sctp_chunkhdr *sch;
	struct sctp_chunkhdr _sch;
	u_int32_t offset, count;
	unsigned long map[256 / sizeof(unsigned long)] = { 0 };

	sh = skb_header_pointer(skb, dataoff, sizeof(_sctph), &_sctph);
	if (sh == NULL)
		goto out;

	if (do_basic_checks(ct, skb, dataoff, map) != 0)
		goto out;

	
	if (!test_bit(SCTP_CID_INIT, map) &&
	    !test_bit(SCTP_CID_SHUTDOWN_COMPLETE, map) &&
	    !test_bit(SCTP_CID_COOKIE_ECHO, map) &&
	    !test_bit(SCTP_CID_ABORT, map) &&
	    !test_bit(SCTP_CID_SHUTDOWN_ACK, map) &&
	    sh->vtag != ct->proto.sctp.vtag[dir]) {
		pr_debug("Verification tag check failed\n");
		goto out;
	}

	old_state = new_state = SCTP_CONNTRACK_NONE;
	spin_lock_bh(&ct->lock);
	for_each_sctp_chunk (skb, sch, _sch, offset, dataoff, count) {
		
		if (sch->type == SCTP_CID_INIT) {
			
			if (sh->vtag != 0)
				goto out_unlock;
		} else if (sch->type == SCTP_CID_ABORT) {
			
			if (sh->vtag != ct->proto.sctp.vtag[dir] &&
			    sh->vtag != ct->proto.sctp.vtag[!dir])
				goto out_unlock;
		} else if (sch->type == SCTP_CID_SHUTDOWN_COMPLETE) {
			
			if (sh->vtag != ct->proto.sctp.vtag[dir] &&
			    sh->vtag != ct->proto.sctp.vtag[!dir] &&
			    sch->flags & SCTP_CHUNK_FLAG_T)
				goto out_unlock;
		} else if (sch->type == SCTP_CID_COOKIE_ECHO) {
			
			if (sh->vtag != ct->proto.sctp.vtag[dir])
				goto out_unlock;
		}

		old_state = ct->proto.sctp.state;
		new_state = sctp_new_state(dir, old_state, sch->type);

		
		if (new_state == SCTP_CONNTRACK_MAX) {
			pr_debug("nf_conntrack_sctp: Invalid dir=%i ctype=%u "
				 "conntrack=%u\n",
				 dir, sch->type, old_state);
			goto out_unlock;
		}

		
		if (sch->type == SCTP_CID_INIT ||
		    sch->type == SCTP_CID_INIT_ACK) {
			sctp_inithdr_t _inithdr, *ih;

			ih = skb_header_pointer(skb, offset + sizeof(sctp_chunkhdr_t),
						sizeof(_inithdr), &_inithdr);
			if (ih == NULL)
				goto out_unlock;
			pr_debug("Setting vtag %x for dir %d\n",
				 ih->init_tag, !dir);
			ct->proto.sctp.vtag[!dir] = ih->init_tag;
		}

		ct->proto.sctp.state = new_state;
		if (old_state != new_state)
			nf_conntrack_event_cache(IPCT_PROTOINFO, ct);
	}
	spin_unlock_bh(&ct->lock);

	nf_ct_refresh_acct(ct, ctinfo, skb, sctp_timeouts[new_state]);

	if (old_state == SCTP_CONNTRACK_COOKIE_ECHOED &&
	    dir == IP_CT_DIR_REPLY &&
	    new_state == SCTP_CONNTRACK_ESTABLISHED) {
		pr_debug("Setting assured bit\n");
		set_bit(IPS_ASSURED_BIT, &ct->status);
		nf_conntrack_event_cache(IPCT_STATUS, ct);
	}

	return NF_ACCEPT;

out_unlock:
	spin_unlock_bh(&ct->lock);
out:
	return -NF_ACCEPT;
}


static bool sctp_new(struct nf_conn *ct, const struct sk_buff *skb,
		     unsigned int dataoff)
{
	enum sctp_conntrack new_state;
	const struct sctphdr *sh;
	struct sctphdr _sctph;
	const struct sctp_chunkhdr *sch;
	struct sctp_chunkhdr _sch;
	u_int32_t offset, count;
	unsigned long map[256 / sizeof(unsigned long)] = { 0 };

	sh = skb_header_pointer(skb, dataoff, sizeof(_sctph), &_sctph);
	if (sh == NULL)
		return false;

	if (do_basic_checks(ct, skb, dataoff, map) != 0)
		return false;

	
	if (test_bit(SCTP_CID_ABORT, map) ||
	    test_bit(SCTP_CID_SHUTDOWN_COMPLETE, map) ||
	    test_bit(SCTP_CID_COOKIE_ACK, map))
		return false;

	new_state = SCTP_CONNTRACK_MAX;
	for_each_sctp_chunk (skb, sch, _sch, offset, dataoff, count) {
		
		new_state = sctp_new_state(IP_CT_DIR_ORIGINAL,
					   SCTP_CONNTRACK_NONE, sch->type);

		
		if (new_state == SCTP_CONNTRACK_NONE ||
		    new_state == SCTP_CONNTRACK_MAX) {
			pr_debug("nf_conntrack_sctp: invalid new deleting.\n");
			return false;
		}

		
		if (sch->type == SCTP_CID_INIT) {
			if (sh->vtag == 0) {
				sctp_inithdr_t _inithdr, *ih;

				ih = skb_header_pointer(skb, offset + sizeof(sctp_chunkhdr_t),
							sizeof(_inithdr), &_inithdr);
				if (ih == NULL)
					return false;

				pr_debug("Setting vtag %x for new conn\n",
					 ih->init_tag);

				ct->proto.sctp.vtag[IP_CT_DIR_REPLY] =
								ih->init_tag;
			} else {
				
				return false;
			}
		}
		
		else {
			pr_debug("Setting vtag %x for new conn OOTB\n",
				 sh->vtag);
			ct->proto.sctp.vtag[IP_CT_DIR_REPLY] = sh->vtag;
		}

		ct->proto.sctp.state = new_state;
	}

	return true;
}

#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_conntrack.h>

static int sctp_to_nlattr(struct sk_buff *skb, struct nlattr *nla,
			  struct nf_conn *ct)
{
	struct nlattr *nest_parms;

	spin_lock_bh(&ct->lock);
	nest_parms = nla_nest_start(skb, CTA_PROTOINFO_SCTP | NLA_F_NESTED);
	if (!nest_parms)
		goto nla_put_failure;

	NLA_PUT_U8(skb, CTA_PROTOINFO_SCTP_STATE, ct->proto.sctp.state);

	NLA_PUT_BE32(skb,
		     CTA_PROTOINFO_SCTP_VTAG_ORIGINAL,
		     ct->proto.sctp.vtag[IP_CT_DIR_ORIGINAL]);

	NLA_PUT_BE32(skb,
		     CTA_PROTOINFO_SCTP_VTAG_REPLY,
		     ct->proto.sctp.vtag[IP_CT_DIR_REPLY]);

	spin_unlock_bh(&ct->lock);

	nla_nest_end(skb, nest_parms);

	return 0;

nla_put_failure:
	spin_unlock_bh(&ct->lock);
	return -1;
}

static const struct nla_policy sctp_nla_policy[CTA_PROTOINFO_SCTP_MAX+1] = {
	[CTA_PROTOINFO_SCTP_STATE]	    = { .type = NLA_U8 },
	[CTA_PROTOINFO_SCTP_VTAG_ORIGINAL]  = { .type = NLA_U32 },
	[CTA_PROTOINFO_SCTP_VTAG_REPLY]     = { .type = NLA_U32 },
};

static int nlattr_to_sctp(struct nlattr *cda[], struct nf_conn *ct)
{
	struct nlattr *attr = cda[CTA_PROTOINFO_SCTP];
	struct nlattr *tb[CTA_PROTOINFO_SCTP_MAX+1];
	int err;

	
	if (!attr)
		return 0;

	err = nla_parse_nested(tb,
			       CTA_PROTOINFO_SCTP_MAX,
			       attr,
			       sctp_nla_policy);
	if (err < 0)
		return err;

	if (!tb[CTA_PROTOINFO_SCTP_STATE] ||
	    !tb[CTA_PROTOINFO_SCTP_VTAG_ORIGINAL] ||
	    !tb[CTA_PROTOINFO_SCTP_VTAG_REPLY])
		return -EINVAL;

	spin_lock_bh(&ct->lock);
	ct->proto.sctp.state = nla_get_u8(tb[CTA_PROTOINFO_SCTP_STATE]);
	ct->proto.sctp.vtag[IP_CT_DIR_ORIGINAL] =
		nla_get_be32(tb[CTA_PROTOINFO_SCTP_VTAG_ORIGINAL]);
	ct->proto.sctp.vtag[IP_CT_DIR_REPLY] =
		nla_get_be32(tb[CTA_PROTOINFO_SCTP_VTAG_REPLY]);
	spin_unlock_bh(&ct->lock);

	return 0;
}

static int sctp_nlattr_size(void)
{
	return nla_total_size(0)	
		+ nla_policy_len(sctp_nla_policy, CTA_PROTOINFO_SCTP_MAX + 1);
}
#endif

#ifdef CONFIG_SYSCTL
static unsigned int sctp_sysctl_table_users;
static struct ctl_table_header *sctp_sysctl_header;
static struct ctl_table sctp_sysctl_table[] = {
	{
		.procname	= "nf_conntrack_sctp_timeout_closed",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_CLOSED],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_sctp_timeout_cookie_wait",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_COOKIE_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_sctp_timeout_cookie_echoed",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_COOKIE_ECHOED],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_sctp_timeout_established",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_ESTABLISHED],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_sctp_timeout_shutdown_sent",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_SHUTDOWN_SENT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_sctp_timeout_shutdown_recd",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_SHUTDOWN_RECD],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "nf_conntrack_sctp_timeout_shutdown_ack_sent",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_SHUTDOWN_ACK_SENT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.ctl_name = 0
	}
};

#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
static struct ctl_table sctp_compat_sysctl_table[] = {
	{
		.procname	= "ip_conntrack_sctp_timeout_closed",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_CLOSED],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_sctp_timeout_cookie_wait",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_COOKIE_WAIT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_sctp_timeout_cookie_echoed",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_COOKIE_ECHOED],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_sctp_timeout_established",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_ESTABLISHED],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_sctp_timeout_shutdown_sent",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_SHUTDOWN_SENT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_sctp_timeout_shutdown_recd",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_SHUTDOWN_RECD],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.procname	= "ip_conntrack_sctp_timeout_shutdown_ack_sent",
		.data		= &sctp_timeouts[SCTP_CONNTRACK_SHUTDOWN_ACK_SENT],
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{
		.ctl_name = 0
	}
};
#endif 
#endif

static struct nf_conntrack_l4proto nf_conntrack_l4proto_sctp4 __read_mostly = {
	.l3proto		= PF_INET,
	.l4proto 		= IPPROTO_SCTP,
	.name 			= "sctp",
	.pkt_to_tuple 		= sctp_pkt_to_tuple,
	.invert_tuple 		= sctp_invert_tuple,
	.print_tuple 		= sctp_print_tuple,
	.print_conntrack	= sctp_print_conntrack,
	.packet 		= sctp_packet,
	.new 			= sctp_new,
	.me 			= THIS_MODULE,
#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
	.to_nlattr		= sctp_to_nlattr,
	.nlattr_size		= sctp_nlattr_size,
	.from_nlattr		= nlattr_to_sctp,
	.tuple_to_nlattr	= nf_ct_port_tuple_to_nlattr,
	.nlattr_tuple_size	= nf_ct_port_nlattr_tuple_size,
	.nlattr_to_tuple	= nf_ct_port_nlattr_to_tuple,
	.nla_policy		= nf_ct_port_nla_policy,
#endif
#ifdef CONFIG_SYSCTL
	.ctl_table_users	= &sctp_sysctl_table_users,
	.ctl_table_header	= &sctp_sysctl_header,
	.ctl_table		= sctp_sysctl_table,
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
	.ctl_compat_table	= sctp_compat_sysctl_table,
#endif
#endif
};

static struct nf_conntrack_l4proto nf_conntrack_l4proto_sctp6 __read_mostly = {
	.l3proto		= PF_INET6,
	.l4proto 		= IPPROTO_SCTP,
	.name 			= "sctp",
	.pkt_to_tuple 		= sctp_pkt_to_tuple,
	.invert_tuple 		= sctp_invert_tuple,
	.print_tuple 		= sctp_print_tuple,
	.print_conntrack	= sctp_print_conntrack,
	.packet 		= sctp_packet,
	.new 			= sctp_new,
	.me 			= THIS_MODULE,
#if defined(CONFIG_NF_CT_NETLINK) || defined(CONFIG_NF_CT_NETLINK_MODULE)
	.to_nlattr		= sctp_to_nlattr,
	.nlattr_size		= sctp_nlattr_size,
	.from_nlattr		= nlattr_to_sctp,
	.tuple_to_nlattr	= nf_ct_port_tuple_to_nlattr,
	.nlattr_tuple_size	= nf_ct_port_nlattr_tuple_size,
	.nlattr_to_tuple	= nf_ct_port_nlattr_to_tuple,
	.nla_policy		= nf_ct_port_nla_policy,
#endif
#ifdef CONFIG_SYSCTL
	.ctl_table_users	= &sctp_sysctl_table_users,
	.ctl_table_header	= &sctp_sysctl_header,
	.ctl_table		= sctp_sysctl_table,
#endif
};

static int __init nf_conntrack_proto_sctp_init(void)
{
	int ret;

	ret = nf_conntrack_l4proto_register(&nf_conntrack_l4proto_sctp4);
	if (ret) {
		printk("nf_conntrack_l4proto_sctp4: protocol register failed\n");
		goto out;
	}
	ret = nf_conntrack_l4proto_register(&nf_conntrack_l4proto_sctp6);
	if (ret) {
		printk("nf_conntrack_l4proto_sctp6: protocol register failed\n");
		goto cleanup_sctp4;
	}

	return ret;

 cleanup_sctp4:
	nf_conntrack_l4proto_unregister(&nf_conntrack_l4proto_sctp4);
 out:
	return ret;
}

static void __exit nf_conntrack_proto_sctp_fini(void)
{
	nf_conntrack_l4proto_unregister(&nf_conntrack_l4proto_sctp6);
	nf_conntrack_l4proto_unregister(&nf_conntrack_l4proto_sctp4);
}

module_init(nf_conntrack_proto_sctp_init);
module_exit(nf_conntrack_proto_sctp_fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kiran Kumar Immidi");
MODULE_DESCRIPTION("Netfilter connection tracking protocol helper for SCTP");
MODULE_ALIAS("ip_conntrack_proto_sctp");
