

#ifndef _VNIC_ENIC_H_
#define _VNIC_ENIC_H_


struct vnic_enet_config {
	u32 flags;
	u32 wq_desc_count;
	u32 rq_desc_count;
	u16 mtu;
	u16 intr_timer;
	u8 intr_timer_type;
	u8 intr_mode;
	char devname[16];
};

#define VENETF_TSO		0x1	
#define VENETF_LRO		0x2	
#define VENETF_RXCSUM		0x4	
#define VENETF_TXCSUM		0x8	
#define VENETF_RSS		0x10	
#define VENETF_RSSHASH_IPV4	0x20	
#define VENETF_RSSHASH_TCPIPV4	0x40	
#define VENETF_RSSHASH_IPV6	0x80	
#define VENETF_RSSHASH_TCPIPV6	0x100	
#define VENETF_RSSHASH_IPV6_EX	0x200	
#define VENETF_RSSHASH_TCPIPV6_EX 0x400	

#endif 
