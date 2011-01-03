

#ifndef _LINUX_IF_VLAN_H_
#define _LINUX_IF_VLAN_H_

#ifdef __KERNEL__
#include <linux/netdevice.h>
#include <linux/etherdevice.h>

#define VLAN_HLEN	4		
#define VLAN_ETH_ALEN	6		
#define VLAN_ETH_HLEN	18		
#define VLAN_ETH_ZLEN	64		


#define VLAN_ETH_DATA_LEN	1500	
#define VLAN_ETH_FRAME_LEN	1518	


struct vlan_hdr {
	__be16	h_vlan_TCI;
	__be16	h_vlan_encapsulated_proto;
};


struct vlan_ethhdr {
	unsigned char	h_dest[ETH_ALEN];
	unsigned char	h_source[ETH_ALEN];
	__be16		h_vlan_proto;
	__be16		h_vlan_TCI;
	__be16		h_vlan_encapsulated_proto;
};

#include <linux/skbuff.h>

static inline struct vlan_ethhdr *vlan_eth_hdr(const struct sk_buff *skb)
{
	return (struct vlan_ethhdr *)skb_mac_header(skb);
}

#define VLAN_VID_MASK	0xfff


extern void vlan_ioctl_set(int (*hook)(struct net *, void __user *));


#define VLAN_GROUP_ARRAY_LEN          4096
#define VLAN_GROUP_ARRAY_SPLIT_PARTS  8
#define VLAN_GROUP_ARRAY_PART_LEN     (VLAN_GROUP_ARRAY_LEN/VLAN_GROUP_ARRAY_SPLIT_PARTS)

struct vlan_group {
	struct net_device	*real_dev; 
	unsigned int		nr_vlans;
	struct hlist_node	hlist;	
	struct net_device **vlan_devices_arrays[VLAN_GROUP_ARRAY_SPLIT_PARTS];
	struct rcu_head		rcu;
};

static inline struct net_device *vlan_group_get_device(struct vlan_group *vg,
						       u16 vlan_id)
{
	struct net_device **array;
	array = vg->vlan_devices_arrays[vlan_id / VLAN_GROUP_ARRAY_PART_LEN];
	return array ? array[vlan_id % VLAN_GROUP_ARRAY_PART_LEN] : NULL;
}

static inline void vlan_group_set_device(struct vlan_group *vg,
					 u16 vlan_id,
					 struct net_device *dev)
{
	struct net_device **array;
	if (!vg)
		return;
	array = vg->vlan_devices_arrays[vlan_id / VLAN_GROUP_ARRAY_PART_LEN];
	array[vlan_id % VLAN_GROUP_ARRAY_PART_LEN] = dev;
}

#define vlan_tx_tag_present(__skb)	((__skb)->vlan_tci)
#define vlan_tx_tag_get(__skb)		((__skb)->vlan_tci)

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
extern struct net_device *vlan_dev_real_dev(const struct net_device *dev);
extern u16 vlan_dev_vlan_id(const struct net_device *dev);

extern int __vlan_hwaccel_rx(struct sk_buff *skb, struct vlan_group *grp,
			     u16 vlan_tci, int polling);
extern int vlan_hwaccel_do_receive(struct sk_buff *skb);
extern int vlan_gro_receive(struct napi_struct *napi, struct vlan_group *grp,
			    unsigned int vlan_tci, struct sk_buff *skb);
extern int vlan_gro_frags(struct napi_struct *napi, struct vlan_group *grp,
			  unsigned int vlan_tci);

#else
static inline struct net_device *vlan_dev_real_dev(const struct net_device *dev)
{
	BUG();
	return NULL;
}

static inline u16 vlan_dev_vlan_id(const struct net_device *dev)
{
	BUG();
	return 0;
}

static inline int __vlan_hwaccel_rx(struct sk_buff *skb, struct vlan_group *grp,
				    u16 vlan_tci, int polling)
{
	BUG();
	return NET_XMIT_SUCCESS;
}

static inline int vlan_hwaccel_do_receive(struct sk_buff *skb)
{
	return 0;
}

static inline int vlan_gro_receive(struct napi_struct *napi,
				   struct vlan_group *grp,
				   unsigned int vlan_tci, struct sk_buff *skb)
{
	return NET_RX_DROP;
}

static inline int vlan_gro_frags(struct napi_struct *napi,
				 struct vlan_group *grp, unsigned int vlan_tci)
{
	return NET_RX_DROP;
}
#endif


static inline int vlan_hwaccel_rx(struct sk_buff *skb,
				  struct vlan_group *grp,
				  u16 vlan_tci)
{
	return __vlan_hwaccel_rx(skb, grp, vlan_tci, 0);
}


static inline int vlan_hwaccel_receive_skb(struct sk_buff *skb,
					   struct vlan_group *grp,
					   u16 vlan_tci)
{
	return __vlan_hwaccel_rx(skb, grp, vlan_tci, 1);
}


static inline struct sk_buff *__vlan_put_tag(struct sk_buff *skb, u16 vlan_tci)
{
	struct vlan_ethhdr *veth;

	if (skb_cow_head(skb, VLAN_HLEN) < 0) {
		kfree_skb(skb);
		return NULL;
	}
	veth = (struct vlan_ethhdr *)skb_push(skb, VLAN_HLEN);

	
	memmove(skb->data, skb->data + VLAN_HLEN, 2 * VLAN_ETH_ALEN);
	skb->mac_header -= VLAN_HLEN;

	
	veth->h_vlan_proto = htons(ETH_P_8021Q);

	
	veth->h_vlan_TCI = htons(vlan_tci);

	skb->protocol = htons(ETH_P_8021Q);

	return skb;
}


static inline struct sk_buff *__vlan_hwaccel_put_tag(struct sk_buff *skb,
						     u16 vlan_tci)
{
	skb->vlan_tci = vlan_tci;
	return skb;
}

#define HAVE_VLAN_PUT_TAG


static inline struct sk_buff *vlan_put_tag(struct sk_buff *skb, u16 vlan_tci)
{
	if (skb->dev->features & NETIF_F_HW_VLAN_TX) {
		return __vlan_hwaccel_put_tag(skb, vlan_tci);
	} else {
		return __vlan_put_tag(skb, vlan_tci);
	}
}


static inline int __vlan_get_tag(const struct sk_buff *skb, u16 *vlan_tci)
{
	struct vlan_ethhdr *veth = (struct vlan_ethhdr *)skb->data;

	if (veth->h_vlan_proto != htons(ETH_P_8021Q)) {
		return -EINVAL;
	}

	*vlan_tci = ntohs(veth->h_vlan_TCI);
	return 0;
}


static inline int __vlan_hwaccel_get_tag(const struct sk_buff *skb,
					 u16 *vlan_tci)
{
	if (vlan_tx_tag_present(skb)) {
		*vlan_tci = skb->vlan_tci;
		return 0;
	} else {
		*vlan_tci = 0;
		return -EINVAL;
	}
}

#define HAVE_VLAN_GET_TAG


static inline int vlan_get_tag(const struct sk_buff *skb, u16 *vlan_tci)
{
	if (skb->dev->features & NETIF_F_HW_VLAN_TX) {
		return __vlan_hwaccel_get_tag(skb, vlan_tci);
	} else {
		return __vlan_get_tag(skb, vlan_tci);
	}
}

#endif 




enum vlan_ioctl_cmds {
	ADD_VLAN_CMD,
	DEL_VLAN_CMD,
	SET_VLAN_INGRESS_PRIORITY_CMD,
	SET_VLAN_EGRESS_PRIORITY_CMD,
	GET_VLAN_INGRESS_PRIORITY_CMD,
	GET_VLAN_EGRESS_PRIORITY_CMD,
	SET_VLAN_NAME_TYPE_CMD,
	SET_VLAN_FLAG_CMD,
	GET_VLAN_REALDEV_NAME_CMD, 
	GET_VLAN_VID_CMD 
};

enum vlan_flags {
	VLAN_FLAG_REORDER_HDR	= 0x1,
	VLAN_FLAG_GVRP		= 0x2,
};

enum vlan_name_types {
	VLAN_NAME_TYPE_PLUS_VID, 
	VLAN_NAME_TYPE_RAW_PLUS_VID, 
	VLAN_NAME_TYPE_PLUS_VID_NO_PAD, 
	VLAN_NAME_TYPE_RAW_PLUS_VID_NO_PAD, 
	VLAN_NAME_TYPE_HIGHEST
};

struct vlan_ioctl_args {
	int cmd; 
	char device1[24];

        union {
		char device2[24];
		int VID;
		unsigned int skb_priority;
		unsigned int name_type;
		unsigned int bind_type;
		unsigned int flag; 
        } u;

	short vlan_qos;   
};

#endif 
