

#ifndef __IPV6_HDR_H_
#define __IPV6_HDR_H_

#define IPV6_ADDR_LEN 16
#define IPV6_HDR_LEN  40


#define IPV6_LINK_LOCAL_ADDR_PREFIX		0xFE8
#define IPV6_SITE_LOCAL_ADDR_PREFIX		0xFEC
#define IPV6_LOCAL_ADDR_PREFIX			0xFE8
#define IPV6_MULTICAST_ADDR_PREFIX		0xFF
#define IPV6_LOOPBACK_ADDR				0x1
#define IPV6_UNSPECIFIED_ADDR			0x0


#define IPV6_NEXT_HEADER_HOP_BY_HOP		0x00	
#define IPV6_NEXT_HEADER_DESTINATION	0x3c	
#define IPV6_NEXT_HEADER_ROUTING		0x2b	
#define IPV6_NEXT_HEADER_FRAGMENT		0x2c	
#define IPV6_NEXT_HEADER_AUTHENTICATION	0x33	
#define IPV6_NEXT_HEADER_ENCAPSULATION	0x32	
#define IPV6_NEXT_HEADER_NONE			0x3b	

#define IPV6_NEXT_HEADER_TCP			0x06
#define IPV6_NEXT_HEADER_UDP			0x11
#define IPV6_NEXT_HEADER_ICMPV6			0x3a


#define ICMPV6_MSG_TYPE_ROUTER_SOLICITATION			0x85 
#define ROUTER_SOLICITATION_FIXED_LEN				8

#define ICMPV6_MSG_TYPE_ROUTER_ADVERTISEMENT		0x86 
#define ROUTER_ADVERTISEMENT_FIXED_LEN				16

#define ICMPV6_MSG_TYPE_NEIGHBOR_SOLICITATION		0x87 
#define NEIGHBOR_SOLICITATION_FIXED_LEN				24

#define ICMPV6_MSG_TYPE_NEIGHBOR_ADVERTISEMENT		0x88 
#define NEIGHBOR_ADVERTISEMENT_FIXED_LEN			24

#define ICMPV6_MSG_TYPE_REDIRECT					0x89 
#define REDIRECT_FIXED_LEN							40


typedef struct rt_ipv6_addr_
{
	union
	{
		UCHAR	ipv6Addr8[16];
		USHORT	ipv6Addr16[8];
		UINT32	ipv6Addr32[4];
	}addr;
#define ipv6_addr			addr.ipv6Addr8
#define ipv6_addr16			addr.ipv6Addr16
#define ipv6_addr32			addr.ipv6Addr32
}RT_IPV6_ADDR, *PRT_IPV6_ADDR;


#define PRINT_IPV6_ADDR(ipv6Addr)	\
	OS_NTOHS((ipv6Addr).ipv6_addr16[0]), \
	OS_NTOHS((ipv6Addr).ipv6_addr16[1]), \
	OS_NTOHS((ipv6Addr).ipv6_addr16[2]), \
	OS_NTOHS((ipv6Addr).ipv6_addr16[3]), \
	OS_NTOHS((ipv6Addr).ipv6_addr16[4]), \
	OS_NTOHS((ipv6Addr).ipv6_addr16[5]), \
	OS_NTOHS((ipv6Addr).ipv6_addr16[6]), \
	OS_NTOHS((ipv6Addr).ipv6_addr16[7])



typedef struct PACKED _rt_ipv6_hdr_
{
	UINT32			ver:4,
					trafficClass:8,
				flowLabel:20;
	USHORT			payload_len;
	UCHAR			nextHdr;
	UCHAR			hopLimit;
	RT_IPV6_ADDR	srcAddr;
	RT_IPV6_ADDR	dstAddr;
}RT_IPV6_HDR, *PRT_IPV6_HDR;


typedef struct PACKED _rt_ipv6_ext_hdr_
{
	UCHAR	nextProto; 
	UCHAR	extHdrLen; 
	UCHAR	octets[1]; 
}RT_IPV6_EXT_HDR, *PRT_IPV6_EXT_HDR;



typedef struct PACKED _rt_ipv6_icmpv6_hdr_
{
	UCHAR	type;
	UCHAR	code;
	USHORT	chksum;
	UCHAR	octets[1]; 
}RT_ICMPV6_HDR, *PRT_ICMPV6_HDR;


typedef struct PACKED _rt_icmp6_option_hdr_
{
	UCHAR type;
	UCHAR len;
	UCHAR octet[1];
}RT_ICMPV6_OPTION_HDR, *PRT_ICMPV6_OPTION_HDR;

typedef enum{

	TYPE_SRC_LL_ADDR	= 1,
	TYPE_TGT_LL_ADDR	= 2,
	TYPE_PREFIX_INFO			= 3,
	TYPE_REDIRECTED_HDR			= 4,
	TYPE_MTU					= 5,
}ICMPV6_OPTIONS_TYPE_DEF;


static inline BOOLEAN IPv6ExtHdrHandle(
	RT_IPV6_EXT_HDR		*pExtHdr,
	UCHAR				*pProto,
	UINT32				*pOffset)
{
	UCHAR nextProto = 0xff;
	UINT32 extLen = 0;
	BOOLEAN status = TRUE;

	
	switch (*pProto)
	{
		case IPV6_NEXT_HEADER_HOP_BY_HOP:
			
			nextProto = pExtHdr->nextProto;
			extLen = (pExtHdr->extHdrLen + 1) * 8;
			break;

		case IPV6_NEXT_HEADER_DESTINATION:
			
			nextProto = pExtHdr->nextProto;
			extLen = (pExtHdr->extHdrLen + 1) * 8;
			break;

		case IPV6_NEXT_HEADER_ROUTING:
			
			nextProto = pExtHdr->nextProto;
			extLen = (pExtHdr->extHdrLen + 1) * 8;
			break;

		case IPV6_NEXT_HEADER_FRAGMENT:
			
			nextProto = pExtHdr->nextProto;
			extLen = 8; 
			break;

		case IPV6_NEXT_HEADER_AUTHENTICATION:
		
			
			nextProto = 0xFF;
			status = FALSE;
			break;

		default:
			nextProto = 0xFF;
			status = FALSE;
			break;
	}

	*pProto = nextProto;
	*pOffset += extLen;
	

	return status;

}

#endif 
