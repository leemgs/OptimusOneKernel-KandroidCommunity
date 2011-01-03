

#ifndef __ETH1394_H
#define __ETH1394_H

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <asm/byteorder.h>

#include "ieee1394.h"
#include "ieee1394_types.h"


#define ETHER1394_REGION_ADDR_LEN	4096


#define ETHER1394_GASP_SPECIFIER_ID	0x00005E
#define ETHER1394_GASP_SPECIFIER_ID_HI	((0x00005E >> 8) & 0xffff)
#define ETHER1394_GASP_SPECIFIER_ID_LO	(0x00005E & 0xff)
#define ETHER1394_GASP_VERSION		1

#define ETHER1394_GASP_OVERHEAD	(2 * sizeof(quadlet_t))	

#define ETHER1394_GASP_BUFFERS	16

#define NODE_SET		(ALL_NODES + 1)		

enum eth1394_bc_states { ETHER1394_BC_ERROR,
			 ETHER1394_BC_RUNNING,
			 ETHER1394_BC_STOPPED };



struct eth1394_priv {
	struct hpsb_host *host;		
	u16 bc_maxpayload;		
	u8 bc_sspd;			
	u64 local_fifo;			
	spinlock_t lock;		
	int broadcast_channel;		
	enum eth1394_bc_states bc_state; 
	struct hpsb_iso *iso;		
	int bc_dgl;			
	struct list_head ip_node_list;	
	struct unit_directory *ud_list[ALL_NODES]; 

	struct work_struct wake;	
	struct net_device *wake_dev;	
	nodeid_t wake_node;		
};



#define ETH1394_ALEN (8)
#define ETH1394_HLEN (10)

struct eth1394hdr {
	unsigned char	h_dest[ETH1394_ALEN];	
	__be16		h_proto;		
}  __attribute__((packed));

static inline struct eth1394hdr *eth1394_hdr(const struct sk_buff *skb)
{
	return (struct eth1394hdr *)skb_mac_header(skb);
}

typedef enum {ETH1394_GASP, ETH1394_WRREQ} eth1394_tx_type;




#if defined __BIG_ENDIAN_BITFIELD
struct eth1394_uf_hdr {
	u16 lf:2;
	u16 res:14;
	__be16 ether_type;		
} __attribute__((packed));
#elif defined __LITTLE_ENDIAN_BITFIELD
struct eth1394_uf_hdr {
	u16 res:14;
	u16 lf:2;
	__be16 ether_type;
} __attribute__((packed));
#else
#error Unknown bit field type
#endif


#if defined __BIG_ENDIAN_BITFIELD
struct eth1394_ff_hdr {
	u16 lf:2;
	u16 res1:2;
	u16 dg_size:12;		
	__be16 ether_type;		
	u16 dgl;		
	u16 res2;
} __attribute__((packed));
#elif defined __LITTLE_ENDIAN_BITFIELD
struct eth1394_ff_hdr {
	u16 dg_size:12;
	u16 res1:2;
	u16 lf:2;
	__be16 ether_type;
	u16 dgl;
	u16 res2;
} __attribute__((packed));
#else
#error Unknown bit field type
#endif


#if defined __BIG_ENDIAN_BITFIELD
struct eth1394_sf_hdr {
	u16 lf:2;
	u16 res1:2;
	u16 dg_size:12;		
	u16 res2:4;
	u16 fg_off:12;		
	u16 dgl;		
	u16 res3;
} __attribute__((packed));
#elif defined __LITTLE_ENDIAN_BITFIELD
struct eth1394_sf_hdr {
	u16 dg_size:12;
	u16 res1:2;
	u16 lf:2;
	u16 fg_off:12;
	u16 res2:4;
	u16 dgl;
	u16 res3;
} __attribute__((packed));
#else
#error Unknown bit field type
#endif

#if defined __BIG_ENDIAN_BITFIELD
struct eth1394_common_hdr {
	u16 lf:2;
	u16 pad1:14;
} __attribute__((packed));
#elif defined __LITTLE_ENDIAN_BITFIELD
struct eth1394_common_hdr {
	u16 pad1:14;
	u16 lf:2;
} __attribute__((packed));
#else
#error Unknown bit field type
#endif

struct eth1394_hdr_words {
	u16 word1;
	u16 word2;
	u16 word3;
	u16 word4;
};

union eth1394_hdr {
	struct eth1394_common_hdr common;
	struct eth1394_uf_hdr uf;
	struct eth1394_ff_hdr ff;
	struct eth1394_sf_hdr sf;
	struct eth1394_hdr_words words;
};




#define ETH1394_HDR_LF_UF	0	
#define ETH1394_HDR_LF_FF	1	
#define ETH1394_HDR_LF_LF	2	
#define ETH1394_HDR_LF_IF	3	

#define IP1394_HW_ADDR_LEN	16	


struct eth1394_arp {
	u16 hw_type;		
	u16 proto_type;		
	u8 hw_addr_len;		
	u8 ip_addr_len;		
	u16 opcode;		
	

	__be64 s_uniq_id;	
	u8 max_rec;		
	u8 sspd;		
	__be16 fifo_hi;		
	__be32 fifo_lo;		
	u32 sip;		
	u32 tip;		
};


#define ETHER1394_TIMEOUT	100000


struct packet_task {
	struct sk_buff *skb;
	int outstanding_pkts;
	eth1394_tx_type tx_type;
	int max_payload;
	struct hpsb_packet *packet;
	struct eth1394_priv *priv;
	union eth1394_hdr hdr;
	u64 addr;
	u16 dest_node;
};

#endif 
