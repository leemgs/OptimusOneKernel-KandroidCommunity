#ifndef __NET_IPIP_H
#define __NET_IPIP_H 1

#include <linux/if_tunnel.h>
#include <net/ip.h>


#define IPTUNNEL_ERR_TIMEO	(30*HZ)

struct ip_tunnel
{
	struct ip_tunnel	*next;
	struct net_device	*dev;

	int			err_count;	
	unsigned long		err_time;	

	
	__u32			i_seqno;	
	__u32			o_seqno;	
	int			hlen;		
	int			mlink;

	struct ip_tunnel_parm	parms;

	struct ip_tunnel_prl_entry	*prl;		
	unsigned int			prl_count;	
};

struct ip_tunnel_prl_entry
{
	struct ip_tunnel_prl_entry	*next;
	__be32				addr;
	u16				flags;
};

#define IPTUNNEL_XMIT() do {						\
	int err;							\
	int pkt_len = skb->len - skb_transport_offset(skb);		\
									\
	skb->ip_summed = CHECKSUM_NONE;					\
	ip_select_ident(iph, &rt->u.dst, NULL);				\
									\
	err = ip_local_out(skb);					\
	if (net_xmit_eval(err) == 0) {					\
		stats->tx_bytes += pkt_len;				\
		stats->tx_packets++;					\
	} else {							\
		stats->tx_errors++;					\
		stats->tx_aborted_errors++;				\
	}								\
} while (0)

#endif
