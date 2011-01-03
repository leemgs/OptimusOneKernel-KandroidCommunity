
#ifndef _LINUX_UDP_H
#define _LINUX_UDP_H

#include <linux/types.h>

struct udphdr {
	__be16	source;
	__be16	dest;
	__be16	len;
	__sum16	check;
};


#define UDP_CORK	1	
#define UDP_ENCAP	100	


#define UDP_ENCAP_ESPINUDP_NON_IKE	1 
#define UDP_ENCAP_ESPINUDP	2 
#define UDP_ENCAP_L2TPINUDP	3 

#ifdef __KERNEL__
#include <net/inet_sock.h>
#include <linux/skbuff.h>
#include <net/netns/hash.h>

static inline struct udphdr *udp_hdr(const struct sk_buff *skb)
{
	return (struct udphdr *)skb_transport_header(skb);
}

#define UDP_HTABLE_SIZE		128

static inline int udp_hashfn(struct net *net, const unsigned num)
{
	return (num + net_hash_mix(net)) & (UDP_HTABLE_SIZE - 1);
}

struct udp_sock {
	
	struct inet_sock inet;
	int		 pending;	
	unsigned int	 corkflag;	
  	__u16		 encap_type;	
	
	__u16		 len;		
	
	__u16		 pcslen;
	__u16		 pcrlen;

#define UDPLITE_BIT      0x1  		
#define UDPLITE_SEND_CC  0x2  		
#define UDPLITE_RECV_CC  0x4		
	__u8		 pcflag;        
	__u8		 unused[3];
	
	int (*encap_rcv)(struct sock *sk, struct sk_buff *skb);
};

static inline struct udp_sock *udp_sk(const struct sock *sk)
{
	return (struct udp_sock *)sk;
}

#define IS_UDPLITE(__sk) (udp_sk(__sk)->pcflag)

#endif

#endif	
