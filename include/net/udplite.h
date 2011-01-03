
#ifndef _UDPLITE_H
#define _UDPLITE_H

#include <net/ip6_checksum.h>


#define UDPLITE_SEND_CSCOV   10 
#define UDPLITE_RECV_CSCOV   11 

extern struct proto 		udplite_prot;
extern struct udp_table		udplite_table;


static __inline__ int udplite_getfrag(void *from, char *to, int  offset,
				      int len, int odd, struct sk_buff *skb)
{
	return memcpy_fromiovecend(to, (struct iovec *) from, offset, len);
}


static inline int udplite_sk_init(struct sock *sk)
{
	udp_sk(sk)->pcflag = UDPLITE_BIT;
	return 0;
}


static inline int udplite_checksum_init(struct sk_buff *skb, struct udphdr *uh)
{
	u16 cscov;

        
	if (uh->check == 0) {
		LIMIT_NETDEBUG(KERN_DEBUG "UDPLITE: zeroed checksum field\n");
		return 1;
	}

	cscov = ntohs(uh->len);

	if (cscov == 0)		 
		;
	else if (cscov < 8  || cscov > skb->len) {
		
		LIMIT_NETDEBUG(KERN_DEBUG "UDPLITE: bad csum coverage %d/%d\n",
			       cscov, skb->len);
		return 1;

	} else if (cscov < skb->len) {
        	UDP_SKB_CB(skb)->partial_cov = 1;
		UDP_SKB_CB(skb)->cscov = cscov;
		if (skb->ip_summed == CHECKSUM_COMPLETE)
			skb->ip_summed = CHECKSUM_NONE;
        }

	return 0;
}

static inline int udplite_sender_cscov(struct udp_sock *up, struct udphdr *uh)
{
	int cscov = up->len;

	
	if (up->pcflag & UDPLITE_SEND_CC)    {
		if (up->pcslen < up->len) {
		
			if (0 < up->pcslen) {
			       cscov = up->pcslen;
			}
			uh->len = htons(up->pcslen);
		}
	
	}
	return cscov;
}

static inline __wsum udplite_csum_outgoing(struct sock *sk, struct sk_buff *skb)
{
	int cscov = udplite_sender_cscov(udp_sk(sk), udp_hdr(skb));
	__wsum csum = 0;

	skb->ip_summed = CHECKSUM_NONE;     

	skb_queue_walk(&sk->sk_write_queue, skb) {
		const int off = skb_transport_offset(skb);
		const int len = skb->len - off;

		csum = skb_checksum(skb, off, (cscov > len)? len : cscov, csum);

		if ((cscov -= len) <= 0)
			break;
	}
	return csum;
}

extern void	udplite4_register(void);
extern int 	udplite_get_port(struct sock *sk, unsigned short snum,
			int (*scmp)(const struct sock *, const struct sock *));
#endif	
