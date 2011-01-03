#ifndef _LINUX_VIRTIO_NET_H
#define _LINUX_VIRTIO_NET_H

#include <linux/types.h>
#include <linux/virtio_ids.h>
#include <linux/virtio_config.h>
#include <linux/if_ether.h>


#define VIRTIO_NET_F_CSUM	0	
#define VIRTIO_NET_F_GUEST_CSUM	1	
#define VIRTIO_NET_F_MAC	5	
#define VIRTIO_NET_F_GSO	6	
#define VIRTIO_NET_F_GUEST_TSO4	7	
#define VIRTIO_NET_F_GUEST_TSO6	8	
#define VIRTIO_NET_F_GUEST_ECN	9	
#define VIRTIO_NET_F_GUEST_UFO	10	
#define VIRTIO_NET_F_HOST_TSO4	11	
#define VIRTIO_NET_F_HOST_TSO6	12	
#define VIRTIO_NET_F_HOST_ECN	13	
#define VIRTIO_NET_F_HOST_UFO	14	
#define VIRTIO_NET_F_MRG_RXBUF	15	
#define VIRTIO_NET_F_STATUS	16	
#define VIRTIO_NET_F_CTRL_VQ	17	
#define VIRTIO_NET_F_CTRL_RX	18	
#define VIRTIO_NET_F_CTRL_VLAN	19	
#define VIRTIO_NET_F_CTRL_RX_EXTRA 20	

#define VIRTIO_NET_S_LINK_UP	1	

struct virtio_net_config {
	
	__u8 mac[6];
	
	__u16 status;
} __attribute__((packed));


struct virtio_net_hdr {
#define VIRTIO_NET_HDR_F_NEEDS_CSUM	1	
	__u8 flags;
#define VIRTIO_NET_HDR_GSO_NONE		0	
#define VIRTIO_NET_HDR_GSO_TCPV4	1	
#define VIRTIO_NET_HDR_GSO_UDP		3	
#define VIRTIO_NET_HDR_GSO_TCPV6	4	
#define VIRTIO_NET_HDR_GSO_ECN		0x80	
	__u8 gso_type;
	__u16 hdr_len;		
	__u16 gso_size;		
	__u16 csum_start;	
	__u16 csum_offset;	
};


struct virtio_net_hdr_mrg_rxbuf {
	struct virtio_net_hdr hdr;
	__u16 num_buffers;	
};


struct virtio_net_ctrl_hdr {
	__u8 class;
	__u8 cmd;
} __attribute__((packed));

typedef __u8 virtio_net_ctrl_ack;

#define VIRTIO_NET_OK     0
#define VIRTIO_NET_ERR    1


#define VIRTIO_NET_CTRL_RX    0
 #define VIRTIO_NET_CTRL_RX_PROMISC      0
 #define VIRTIO_NET_CTRL_RX_ALLMULTI     1
 #define VIRTIO_NET_CTRL_RX_ALLUNI       2
 #define VIRTIO_NET_CTRL_RX_NOMULTI      3
 #define VIRTIO_NET_CTRL_RX_NOUNI        4
 #define VIRTIO_NET_CTRL_RX_NOBCAST      5


struct virtio_net_ctrl_mac {
	__u32 entries;
	__u8 macs[][ETH_ALEN];
} __attribute__((packed));

#define VIRTIO_NET_CTRL_MAC    1
 #define VIRTIO_NET_CTRL_MAC_TABLE_SET        0


#define VIRTIO_NET_CTRL_VLAN       2
 #define VIRTIO_NET_CTRL_VLAN_ADD             0
 #define VIRTIO_NET_CTRL_VLAN_DEL             1

#endif 
