

#define KMSG_COMPONENT "IPVS"
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt

#include <linux/in.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

#include <net/ip_vs.h>




#define PORT_ISAKMP	500


static struct ip_vs_conn *
ah_esp_conn_in_get(int af, const struct sk_buff *skb, struct ip_vs_protocol *pp,
		   const struct ip_vs_iphdr *iph, unsigned int proto_off,
		   int inverse)
{
	struct ip_vs_conn *cp;

	if (likely(!inverse)) {
		cp = ip_vs_conn_in_get(af, IPPROTO_UDP,
				       &iph->saddr,
				       htons(PORT_ISAKMP),
				       &iph->daddr,
				       htons(PORT_ISAKMP));
	} else {
		cp = ip_vs_conn_in_get(af, IPPROTO_UDP,
				       &iph->daddr,
				       htons(PORT_ISAKMP),
				       &iph->saddr,
				       htons(PORT_ISAKMP));
	}

	if (!cp) {
		
		IP_VS_DBG_BUF(12, "Unknown ISAKMP entry for outin packet "
			      "%s%s %s->%s\n",
			      inverse ? "ICMP+" : "",
			      pp->name,
			      IP_VS_DBG_ADDR(af, &iph->saddr),
			      IP_VS_DBG_ADDR(af, &iph->daddr));
	}

	return cp;
}


static struct ip_vs_conn *
ah_esp_conn_out_get(int af, const struct sk_buff *skb,
		    struct ip_vs_protocol *pp,
		    const struct ip_vs_iphdr *iph,
		    unsigned int proto_off,
		    int inverse)
{
	struct ip_vs_conn *cp;

	if (likely(!inverse)) {
		cp = ip_vs_conn_out_get(af, IPPROTO_UDP,
					&iph->saddr,
					htons(PORT_ISAKMP),
					&iph->daddr,
					htons(PORT_ISAKMP));
	} else {
		cp = ip_vs_conn_out_get(af, IPPROTO_UDP,
					&iph->daddr,
					htons(PORT_ISAKMP),
					&iph->saddr,
					htons(PORT_ISAKMP));
	}

	if (!cp) {
		IP_VS_DBG_BUF(12, "Unknown ISAKMP entry for inout packet "
			      "%s%s %s->%s\n",
			      inverse ? "ICMP+" : "",
			      pp->name,
			      IP_VS_DBG_ADDR(af, &iph->saddr),
			      IP_VS_DBG_ADDR(af, &iph->daddr));
	}

	return cp;
}


static int
ah_esp_conn_schedule(int af, struct sk_buff *skb, struct ip_vs_protocol *pp,
		     int *verdict, struct ip_vs_conn **cpp)
{
	
	*verdict = NF_ACCEPT;
	return 0;
}


static void
ah_esp_debug_packet_v4(struct ip_vs_protocol *pp, const struct sk_buff *skb,
		       int offset, const char *msg)
{
	char buf[256];
	struct iphdr _iph, *ih;

	ih = skb_header_pointer(skb, offset, sizeof(_iph), &_iph);
	if (ih == NULL)
		sprintf(buf, "%s TRUNCATED", pp->name);
	else
		sprintf(buf, "%s %pI4->%pI4",
			pp->name, &ih->saddr, &ih->daddr);

	pr_debug("%s: %s\n", msg, buf);
}

#ifdef CONFIG_IP_VS_IPV6
static void
ah_esp_debug_packet_v6(struct ip_vs_protocol *pp, const struct sk_buff *skb,
		       int offset, const char *msg)
{
	char buf[256];
	struct ipv6hdr _iph, *ih;

	ih = skb_header_pointer(skb, offset, sizeof(_iph), &_iph);
	if (ih == NULL)
		sprintf(buf, "%s TRUNCATED", pp->name);
	else
		sprintf(buf, "%s %pI6->%pI6",
			pp->name, &ih->saddr, &ih->daddr);

	pr_debug("%s: %s\n", msg, buf);
}
#endif

static void
ah_esp_debug_packet(struct ip_vs_protocol *pp, const struct sk_buff *skb,
		    int offset, const char *msg)
{
#ifdef CONFIG_IP_VS_IPV6
	if (skb->protocol == htons(ETH_P_IPV6))
		ah_esp_debug_packet_v6(pp, skb, offset, msg);
	else
#endif
		ah_esp_debug_packet_v4(pp, skb, offset, msg);
}


static void ah_esp_init(struct ip_vs_protocol *pp)
{
	
}


static void ah_esp_exit(struct ip_vs_protocol *pp)
{
	
}


#ifdef CONFIG_IP_VS_PROTO_AH
struct ip_vs_protocol ip_vs_protocol_ah = {
	.name =			"AH",
	.protocol =		IPPROTO_AH,
	.num_states =		1,
	.dont_defrag =		1,
	.init =			ah_esp_init,
	.exit =			ah_esp_exit,
	.conn_schedule =	ah_esp_conn_schedule,
	.conn_in_get =		ah_esp_conn_in_get,
	.conn_out_get =		ah_esp_conn_out_get,
	.snat_handler =		NULL,
	.dnat_handler =		NULL,
	.csum_check =		NULL,
	.state_transition =	NULL,
	.register_app =		NULL,
	.unregister_app =	NULL,
	.app_conn_bind =	NULL,
	.debug_packet =		ah_esp_debug_packet,
	.timeout_change =	NULL,		
	.set_state_timeout =	NULL,
};
#endif

#ifdef CONFIG_IP_VS_PROTO_ESP
struct ip_vs_protocol ip_vs_protocol_esp = {
	.name =			"ESP",
	.protocol =		IPPROTO_ESP,
	.num_states =		1,
	.dont_defrag =		1,
	.init =			ah_esp_init,
	.exit =			ah_esp_exit,
	.conn_schedule =	ah_esp_conn_schedule,
	.conn_in_get =		ah_esp_conn_in_get,
	.conn_out_get =		ah_esp_conn_out_get,
	.snat_handler =		NULL,
	.dnat_handler =		NULL,
	.csum_check =		NULL,
	.state_transition =	NULL,
	.register_app =		NULL,
	.unregister_app =	NULL,
	.app_conn_bind =	NULL,
	.debug_packet =		ah_esp_debug_packet,
	.timeout_change =	NULL,		
};
#endif
