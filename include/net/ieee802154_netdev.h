

#ifndef IEEE802154_NETDEVICE_H
#define IEEE802154_NETDEVICE_H


struct ieee802154_mac_cb {
	u8 lqi;
	struct ieee802154_addr sa;
	struct ieee802154_addr da;
	u8 flags;
	u8 seq;
};

static inline struct ieee802154_mac_cb *mac_cb(struct sk_buff *skb)
{
	return (struct ieee802154_mac_cb *)skb->cb;
}

#define MAC_CB_FLAG_TYPEMASK		((1 << 3) - 1)

#define MAC_CB_FLAG_ACKREQ		(1 << 3)
#define MAC_CB_FLAG_SECEN		(1 << 4)
#define MAC_CB_FLAG_INTRAPAN		(1 << 5)

static inline int mac_cb_is_ackreq(struct sk_buff *skb)
{
	return mac_cb(skb)->flags & MAC_CB_FLAG_ACKREQ;
}

static inline int mac_cb_is_secen(struct sk_buff *skb)
{
	return mac_cb(skb)->flags & MAC_CB_FLAG_SECEN;
}

static inline int mac_cb_is_intrapan(struct sk_buff *skb)
{
	return mac_cb(skb)->flags & MAC_CB_FLAG_INTRAPAN;
}

static inline int mac_cb_type(struct sk_buff *skb)
{
	return mac_cb(skb)->flags & MAC_CB_FLAG_TYPEMASK;
}

#define IEEE802154_MAC_SCAN_ED		0
#define IEEE802154_MAC_SCAN_ACTIVE	1
#define IEEE802154_MAC_SCAN_PASSIVE	2
#define IEEE802154_MAC_SCAN_ORPHAN	3


struct ieee802154_mlme_ops {
	int (*assoc_req)(struct net_device *dev,
			struct ieee802154_addr *addr,
			u8 channel, u8 page, u8 cap);
	int (*assoc_resp)(struct net_device *dev,
			struct ieee802154_addr *addr,
			u16 short_addr, u8 status);
	int (*disassoc_req)(struct net_device *dev,
			struct ieee802154_addr *addr,
			u8 reason);
	int (*start_req)(struct net_device *dev,
			struct ieee802154_addr *addr,
			u8 channel, u8 page, u8 bcn_ord, u8 sf_ord,
			u8 pan_coord, u8 blx, u8 coord_realign);
	int (*scan_req)(struct net_device *dev,
			u8 type, u32 channels, u8 page, u8 duration);

	
	u16 (*get_pan_id)(struct net_device *dev);
	u16 (*get_short_addr)(struct net_device *dev);
	u8 (*get_dsn)(struct net_device *dev);
	u8 (*get_bsn)(struct net_device *dev);
};

static inline struct ieee802154_mlme_ops *ieee802154_mlme_ops(
		struct net_device *dev)
{
	return dev->ml_priv;
}

#endif


