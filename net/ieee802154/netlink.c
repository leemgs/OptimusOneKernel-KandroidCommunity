

#include <linux/kernel.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <net/netlink.h>
#include <net/genetlink.h>
#include <net/sock.h>
#include <linux/nl802154.h>
#include <net/af_ieee802154.h>
#include <net/nl802154.h>
#include <net/ieee802154.h>
#include <net/ieee802154_netdev.h>

static unsigned int ieee802154_seq_num;
static DEFINE_SPINLOCK(ieee802154_seq_lock);

static struct genl_family ieee802154_coordinator_family = {
	.id		= GENL_ID_GENERATE,
	.hdrsize	= 0,
	.name		= IEEE802154_NL_NAME,
	.version	= 1,
	.maxattr	= IEEE802154_ATTR_MAX,
};

static struct genl_multicast_group ieee802154_coord_mcgrp = {
	.name		= IEEE802154_MCAST_COORD_NAME,
};

static struct genl_multicast_group ieee802154_beacon_mcgrp = {
	.name		= IEEE802154_MCAST_BEACON_NAME,
};


static struct sk_buff *ieee802154_nl_create(int flags, u8 req)
{
	void *hdr;
	struct sk_buff *msg = nlmsg_new(NLMSG_GOODSIZE, GFP_ATOMIC);
	unsigned long f;

	if (!msg)
		return NULL;

	spin_lock_irqsave(&ieee802154_seq_lock, f);
	hdr = genlmsg_put(msg, 0, ieee802154_seq_num++,
			&ieee802154_coordinator_family, flags, req);
	spin_unlock_irqrestore(&ieee802154_seq_lock, f);
	if (!hdr) {
		nlmsg_free(msg);
		return NULL;
	}

	return msg;
}

static int ieee802154_nl_finish(struct sk_buff *msg)
{
	
	void *hdr = genlmsg_data(NLMSG_DATA(msg->data));

	if (genlmsg_end(msg, hdr) < 0)
		goto out;

	return genlmsg_multicast(msg, 0, ieee802154_coord_mcgrp.id,
			GFP_ATOMIC);
out:
	nlmsg_free(msg);
	return -ENOBUFS;
}

int ieee802154_nl_assoc_indic(struct net_device *dev,
		struct ieee802154_addr *addr, u8 cap)
{
	struct sk_buff *msg;

	pr_debug("%s\n", __func__);

	if (addr->addr_type != IEEE802154_ADDR_LONG) {
		pr_err("%s: received non-long source address!\n", __func__);
		return -EINVAL;
	}

	msg = ieee802154_nl_create(0, IEEE802154_ASSOCIATE_INDIC);
	if (!msg)
		return -ENOBUFS;

	NLA_PUT_STRING(msg, IEEE802154_ATTR_DEV_NAME, dev->name);
	NLA_PUT_U32(msg, IEEE802154_ATTR_DEV_INDEX, dev->ifindex);
	NLA_PUT(msg, IEEE802154_ATTR_HW_ADDR, IEEE802154_ADDR_LEN,
			dev->dev_addr);

	NLA_PUT(msg, IEEE802154_ATTR_SRC_HW_ADDR, IEEE802154_ADDR_LEN,
			addr->hwaddr);

	NLA_PUT_U8(msg, IEEE802154_ATTR_CAPABILITY, cap);

	return ieee802154_nl_finish(msg);

nla_put_failure:
	nlmsg_free(msg);
	return -ENOBUFS;
}
EXPORT_SYMBOL(ieee802154_nl_assoc_indic);

int ieee802154_nl_assoc_confirm(struct net_device *dev, u16 short_addr,
		u8 status)
{
	struct sk_buff *msg;

	pr_debug("%s\n", __func__);

	msg = ieee802154_nl_create(0, IEEE802154_ASSOCIATE_CONF);
	if (!msg)
		return -ENOBUFS;

	NLA_PUT_STRING(msg, IEEE802154_ATTR_DEV_NAME, dev->name);
	NLA_PUT_U32(msg, IEEE802154_ATTR_DEV_INDEX, dev->ifindex);
	NLA_PUT(msg, IEEE802154_ATTR_HW_ADDR, IEEE802154_ADDR_LEN,
			dev->dev_addr);

	NLA_PUT_U16(msg, IEEE802154_ATTR_SHORT_ADDR, short_addr);
	NLA_PUT_U8(msg, IEEE802154_ATTR_STATUS, status);

	return ieee802154_nl_finish(msg);

nla_put_failure:
	nlmsg_free(msg);
	return -ENOBUFS;
}
EXPORT_SYMBOL(ieee802154_nl_assoc_confirm);

int ieee802154_nl_disassoc_indic(struct net_device *dev,
		struct ieee802154_addr *addr, u8 reason)
{
	struct sk_buff *msg;

	pr_debug("%s\n", __func__);

	msg = ieee802154_nl_create(0, IEEE802154_DISASSOCIATE_INDIC);
	if (!msg)
		return -ENOBUFS;

	NLA_PUT_STRING(msg, IEEE802154_ATTR_DEV_NAME, dev->name);
	NLA_PUT_U32(msg, IEEE802154_ATTR_DEV_INDEX, dev->ifindex);
	NLA_PUT(msg, IEEE802154_ATTR_HW_ADDR, IEEE802154_ADDR_LEN,
			dev->dev_addr);

	if (addr->addr_type == IEEE802154_ADDR_LONG)
		NLA_PUT(msg, IEEE802154_ATTR_SRC_HW_ADDR, IEEE802154_ADDR_LEN,
				addr->hwaddr);
	else
		NLA_PUT_U16(msg, IEEE802154_ATTR_SRC_SHORT_ADDR,
				addr->short_addr);

	NLA_PUT_U8(msg, IEEE802154_ATTR_REASON, reason);

	return ieee802154_nl_finish(msg);

nla_put_failure:
	nlmsg_free(msg);
	return -ENOBUFS;
}
EXPORT_SYMBOL(ieee802154_nl_disassoc_indic);

int ieee802154_nl_disassoc_confirm(struct net_device *dev, u8 status)
{
	struct sk_buff *msg;

	pr_debug("%s\n", __func__);

	msg = ieee802154_nl_create(0, IEEE802154_DISASSOCIATE_CONF);
	if (!msg)
		return -ENOBUFS;

	NLA_PUT_STRING(msg, IEEE802154_ATTR_DEV_NAME, dev->name);
	NLA_PUT_U32(msg, IEEE802154_ATTR_DEV_INDEX, dev->ifindex);
	NLA_PUT(msg, IEEE802154_ATTR_HW_ADDR, IEEE802154_ADDR_LEN,
			dev->dev_addr);

	NLA_PUT_U8(msg, IEEE802154_ATTR_STATUS, status);

	return ieee802154_nl_finish(msg);

nla_put_failure:
	nlmsg_free(msg);
	return -ENOBUFS;
}
EXPORT_SYMBOL(ieee802154_nl_disassoc_confirm);

int ieee802154_nl_beacon_indic(struct net_device *dev,
		u16 panid, u16 coord_addr)
{
	struct sk_buff *msg;

	pr_debug("%s\n", __func__);

	msg = ieee802154_nl_create(0, IEEE802154_BEACON_NOTIFY_INDIC);
	if (!msg)
		return -ENOBUFS;

	NLA_PUT_STRING(msg, IEEE802154_ATTR_DEV_NAME, dev->name);
	NLA_PUT_U32(msg, IEEE802154_ATTR_DEV_INDEX, dev->ifindex);
	NLA_PUT(msg, IEEE802154_ATTR_HW_ADDR, IEEE802154_ADDR_LEN,
			dev->dev_addr);
	NLA_PUT_U16(msg, IEEE802154_ATTR_COORD_SHORT_ADDR, coord_addr);
	NLA_PUT_U16(msg, IEEE802154_ATTR_COORD_PAN_ID, panid);

	return ieee802154_nl_finish(msg);

nla_put_failure:
	nlmsg_free(msg);
	return -ENOBUFS;
}
EXPORT_SYMBOL(ieee802154_nl_beacon_indic);

int ieee802154_nl_scan_confirm(struct net_device *dev,
		u8 status, u8 scan_type, u32 unscanned, u8 page,
		u8 *edl)
{
	struct sk_buff *msg;

	pr_debug("%s\n", __func__);

	msg = ieee802154_nl_create(0, IEEE802154_SCAN_CONF);
	if (!msg)
		return -ENOBUFS;

	NLA_PUT_STRING(msg, IEEE802154_ATTR_DEV_NAME, dev->name);
	NLA_PUT_U32(msg, IEEE802154_ATTR_DEV_INDEX, dev->ifindex);
	NLA_PUT(msg, IEEE802154_ATTR_HW_ADDR, IEEE802154_ADDR_LEN,
			dev->dev_addr);

	NLA_PUT_U8(msg, IEEE802154_ATTR_STATUS, status);
	NLA_PUT_U8(msg, IEEE802154_ATTR_SCAN_TYPE, scan_type);
	NLA_PUT_U32(msg, IEEE802154_ATTR_CHANNELS, unscanned);
	NLA_PUT_U8(msg, IEEE802154_ATTR_PAGE, page);

	if (edl)
		NLA_PUT(msg, IEEE802154_ATTR_ED_LIST, 27, edl);

	return ieee802154_nl_finish(msg);

nla_put_failure:
	nlmsg_free(msg);
	return -ENOBUFS;
}
EXPORT_SYMBOL(ieee802154_nl_scan_confirm);

int ieee802154_nl_start_confirm(struct net_device *dev, u8 status)
{
	struct sk_buff *msg;

	pr_debug("%s\n", __func__);

	msg = ieee802154_nl_create(0, IEEE802154_START_CONF);
	if (!msg)
		return -ENOBUFS;

	NLA_PUT_STRING(msg, IEEE802154_ATTR_DEV_NAME, dev->name);
	NLA_PUT_U32(msg, IEEE802154_ATTR_DEV_INDEX, dev->ifindex);
	NLA_PUT(msg, IEEE802154_ATTR_HW_ADDR, IEEE802154_ADDR_LEN,
			dev->dev_addr);

	NLA_PUT_U8(msg, IEEE802154_ATTR_STATUS, status);

	return ieee802154_nl_finish(msg);

nla_put_failure:
	nlmsg_free(msg);
	return -ENOBUFS;
}
EXPORT_SYMBOL(ieee802154_nl_start_confirm);

static int ieee802154_nl_fill_iface(struct sk_buff *msg, u32 pid,
	u32 seq, int flags, struct net_device *dev)
{
	void *hdr;

	pr_debug("%s\n", __func__);

	hdr = genlmsg_put(msg, 0, seq, &ieee802154_coordinator_family, flags,
		IEEE802154_LIST_IFACE);
	if (!hdr)
		goto out;

	NLA_PUT_STRING(msg, IEEE802154_ATTR_DEV_NAME, dev->name);
	NLA_PUT_U32(msg, IEEE802154_ATTR_DEV_INDEX, dev->ifindex);

	NLA_PUT(msg, IEEE802154_ATTR_HW_ADDR, IEEE802154_ADDR_LEN,
		dev->dev_addr);
	NLA_PUT_U16(msg, IEEE802154_ATTR_SHORT_ADDR,
		ieee802154_mlme_ops(dev)->get_short_addr(dev));
	NLA_PUT_U16(msg, IEEE802154_ATTR_PAN_ID,
		ieee802154_mlme_ops(dev)->get_pan_id(dev));
	return genlmsg_end(msg, hdr);

nla_put_failure:
	genlmsg_cancel(msg, hdr);
out:
	return -EMSGSIZE;
}


static struct net_device *ieee802154_nl_get_dev(struct genl_info *info)
{
	struct net_device *dev;

	if (info->attrs[IEEE802154_ATTR_DEV_NAME]) {
		char name[IFNAMSIZ + 1];
		nla_strlcpy(name, info->attrs[IEEE802154_ATTR_DEV_NAME],
				sizeof(name));
		dev = dev_get_by_name(&init_net, name);
	} else if (info->attrs[IEEE802154_ATTR_DEV_INDEX])
		dev = dev_get_by_index(&init_net,
			nla_get_u32(info->attrs[IEEE802154_ATTR_DEV_INDEX]));
	else
		return NULL;

	if (!dev)
		return NULL;

	if (dev->type != ARPHRD_IEEE802154) {
		dev_put(dev);
		return NULL;
	}

	return dev;
}

static int ieee802154_associate_req(struct sk_buff *skb,
		struct genl_info *info)
{
	struct net_device *dev;
	struct ieee802154_addr addr;
	u8 page;
	int ret = -EINVAL;

	if (!info->attrs[IEEE802154_ATTR_CHANNEL] ||
	    !info->attrs[IEEE802154_ATTR_COORD_PAN_ID] ||
	    (!info->attrs[IEEE802154_ATTR_COORD_HW_ADDR] &&
		!info->attrs[IEEE802154_ATTR_COORD_SHORT_ADDR]) ||
	    !info->attrs[IEEE802154_ATTR_CAPABILITY])
		return -EINVAL;

	dev = ieee802154_nl_get_dev(info);
	if (!dev)
		return -ENODEV;

	if (info->attrs[IEEE802154_ATTR_COORD_HW_ADDR]) {
		addr.addr_type = IEEE802154_ADDR_LONG;
		nla_memcpy(addr.hwaddr,
				info->attrs[IEEE802154_ATTR_COORD_HW_ADDR],
				IEEE802154_ADDR_LEN);
	} else {
		addr.addr_type = IEEE802154_ADDR_SHORT;
		addr.short_addr = nla_get_u16(
				info->attrs[IEEE802154_ATTR_COORD_SHORT_ADDR]);
	}
	addr.pan_id = nla_get_u16(info->attrs[IEEE802154_ATTR_COORD_PAN_ID]);

	if (info->attrs[IEEE802154_ATTR_PAGE])
		page = nla_get_u8(info->attrs[IEEE802154_ATTR_PAGE]);
	else
		page = 0;

	ret = ieee802154_mlme_ops(dev)->assoc_req(dev, &addr,
			nla_get_u8(info->attrs[IEEE802154_ATTR_CHANNEL]),
			page,
			nla_get_u8(info->attrs[IEEE802154_ATTR_CAPABILITY]));

	dev_put(dev);
	return ret;
}

static int ieee802154_associate_resp(struct sk_buff *skb,
		struct genl_info *info)
{
	struct net_device *dev;
	struct ieee802154_addr addr;
	int ret = -EINVAL;

	if (!info->attrs[IEEE802154_ATTR_STATUS] ||
	    !info->attrs[IEEE802154_ATTR_DEST_HW_ADDR] ||
	    !info->attrs[IEEE802154_ATTR_DEST_SHORT_ADDR])
		return -EINVAL;

	dev = ieee802154_nl_get_dev(info);
	if (!dev)
		return -ENODEV;

	addr.addr_type = IEEE802154_ADDR_LONG;
	nla_memcpy(addr.hwaddr, info->attrs[IEEE802154_ATTR_DEST_HW_ADDR],
			IEEE802154_ADDR_LEN);
	addr.pan_id = ieee802154_mlme_ops(dev)->get_pan_id(dev);


	ret = ieee802154_mlme_ops(dev)->assoc_resp(dev, &addr,
		nla_get_u16(info->attrs[IEEE802154_ATTR_DEST_SHORT_ADDR]),
		nla_get_u8(info->attrs[IEEE802154_ATTR_STATUS]));

	dev_put(dev);
	return ret;
}

static int ieee802154_disassociate_req(struct sk_buff *skb,
		struct genl_info *info)
{
	struct net_device *dev;
	struct ieee802154_addr addr;
	int ret = -EINVAL;

	if ((!info->attrs[IEEE802154_ATTR_DEST_HW_ADDR] &&
		!info->attrs[IEEE802154_ATTR_DEST_SHORT_ADDR]) ||
	    !info->attrs[IEEE802154_ATTR_REASON])
		return -EINVAL;

	dev = ieee802154_nl_get_dev(info);
	if (!dev)
		return -ENODEV;

	if (info->attrs[IEEE802154_ATTR_DEST_HW_ADDR]) {
		addr.addr_type = IEEE802154_ADDR_LONG;
		nla_memcpy(addr.hwaddr,
				info->attrs[IEEE802154_ATTR_DEST_HW_ADDR],
				IEEE802154_ADDR_LEN);
	} else {
		addr.addr_type = IEEE802154_ADDR_SHORT;
		addr.short_addr = nla_get_u16(
				info->attrs[IEEE802154_ATTR_DEST_SHORT_ADDR]);
	}
	addr.pan_id = ieee802154_mlme_ops(dev)->get_pan_id(dev);

	ret = ieee802154_mlme_ops(dev)->disassoc_req(dev, &addr,
			nla_get_u8(info->attrs[IEEE802154_ATTR_REASON]));

	dev_put(dev);
	return ret;
}


static int ieee802154_start_req(struct sk_buff *skb, struct genl_info *info)
{
	struct net_device *dev;
	struct ieee802154_addr addr;

	u8 channel, bcn_ord, sf_ord;
	u8 page;
	int pan_coord, blx, coord_realign;
	int ret;

	if (!info->attrs[IEEE802154_ATTR_COORD_PAN_ID] ||
	    !info->attrs[IEEE802154_ATTR_COORD_SHORT_ADDR] ||
	    !info->attrs[IEEE802154_ATTR_CHANNEL] ||
	    !info->attrs[IEEE802154_ATTR_BCN_ORD] ||
	    !info->attrs[IEEE802154_ATTR_SF_ORD] ||
	    !info->attrs[IEEE802154_ATTR_PAN_COORD] ||
	    !info->attrs[IEEE802154_ATTR_BAT_EXT] ||
	    !info->attrs[IEEE802154_ATTR_COORD_REALIGN]
	 )
		return -EINVAL;

	dev = ieee802154_nl_get_dev(info);
	if (!dev)
		return -ENODEV;

	addr.addr_type = IEEE802154_ADDR_SHORT;
	addr.short_addr = nla_get_u16(
			info->attrs[IEEE802154_ATTR_COORD_SHORT_ADDR]);
	addr.pan_id = nla_get_u16(info->attrs[IEEE802154_ATTR_COORD_PAN_ID]);

	channel = nla_get_u8(info->attrs[IEEE802154_ATTR_CHANNEL]);
	bcn_ord = nla_get_u8(info->attrs[IEEE802154_ATTR_BCN_ORD]);
	sf_ord = nla_get_u8(info->attrs[IEEE802154_ATTR_SF_ORD]);
	pan_coord = nla_get_u8(info->attrs[IEEE802154_ATTR_PAN_COORD]);
	blx = nla_get_u8(info->attrs[IEEE802154_ATTR_BAT_EXT]);
	coord_realign = nla_get_u8(info->attrs[IEEE802154_ATTR_COORD_REALIGN]);

	if (info->attrs[IEEE802154_ATTR_PAGE])
		page = nla_get_u8(info->attrs[IEEE802154_ATTR_PAGE]);
	else
		page = 0;


	if (addr.short_addr == IEEE802154_ADDR_BROADCAST) {
		ieee802154_nl_start_confirm(dev, IEEE802154_NO_SHORT_ADDRESS);
		dev_put(dev);
		return -EINVAL;
	}

	ret = ieee802154_mlme_ops(dev)->start_req(dev, &addr, channel, page,
		bcn_ord, sf_ord, pan_coord, blx, coord_realign);

	dev_put(dev);
	return ret;
}

static int ieee802154_scan_req(struct sk_buff *skb, struct genl_info *info)
{
	struct net_device *dev;
	int ret;
	u8 type;
	u32 channels;
	u8 duration;
	u8 page;

	if (!info->attrs[IEEE802154_ATTR_SCAN_TYPE] ||
	    !info->attrs[IEEE802154_ATTR_CHANNELS] ||
	    !info->attrs[IEEE802154_ATTR_DURATION])
		return -EINVAL;

	dev = ieee802154_nl_get_dev(info);
	if (!dev)
		return -ENODEV;

	type = nla_get_u8(info->attrs[IEEE802154_ATTR_SCAN_TYPE]);
	channels = nla_get_u32(info->attrs[IEEE802154_ATTR_CHANNELS]);
	duration = nla_get_u8(info->attrs[IEEE802154_ATTR_DURATION]);

	if (info->attrs[IEEE802154_ATTR_PAGE])
		page = nla_get_u8(info->attrs[IEEE802154_ATTR_PAGE]);
	else
		page = 0;


	ret = ieee802154_mlme_ops(dev)->scan_req(dev, type, channels, page,
			duration);

	dev_put(dev);
	return ret;
}

static int ieee802154_list_iface(struct sk_buff *skb,
	struct genl_info *info)
{
	
	struct sk_buff *msg;
	struct net_device *dev = NULL;
	int rc = -ENOBUFS;

	pr_debug("%s\n", __func__);

	dev = ieee802154_nl_get_dev(info);
	if (!dev)
		return -ENODEV;

	msg = nlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!msg)
		goto out_dev;

	rc = ieee802154_nl_fill_iface(msg, info->snd_pid, info->snd_seq,
			0, dev);
	if (rc < 0)
		goto out_free;

	dev_put(dev);

	return genlmsg_unicast(&init_net, msg, info->snd_pid);
out_free:
	nlmsg_free(msg);
out_dev:
	dev_put(dev);
	return rc;

}

static int ieee802154_dump_iface(struct sk_buff *skb,
	struct netlink_callback *cb)
{
	struct net *net = sock_net(skb->sk);
	struct net_device *dev;
	int idx;
	int s_idx = cb->args[0];

	pr_debug("%s\n", __func__);

	idx = 0;
	for_each_netdev(net, dev) {
		if (idx < s_idx || (dev->type != ARPHRD_IEEE802154))
			goto cont;

		if (ieee802154_nl_fill_iface(skb, NETLINK_CB(cb->skb).pid,
			cb->nlh->nlmsg_seq, NLM_F_MULTI, dev) < 0)
			break;
cont:
		idx++;
	}
	cb->args[0] = idx;

	return skb->len;
}

#define IEEE802154_OP(_cmd, _func)			\
	{						\
		.cmd	= _cmd,				\
		.policy	= ieee802154_policy,		\
		.doit	= _func,			\
		.dumpit	= NULL,				\
		.flags	= GENL_ADMIN_PERM,		\
	}

#define IEEE802154_DUMP(_cmd, _func, _dump)		\
	{						\
		.cmd	= _cmd,				\
		.policy	= ieee802154_policy,		\
		.doit	= _func,			\
		.dumpit	= _dump,			\
	}

static struct genl_ops ieee802154_coordinator_ops[] = {
	IEEE802154_OP(IEEE802154_ASSOCIATE_REQ, ieee802154_associate_req),
	IEEE802154_OP(IEEE802154_ASSOCIATE_RESP, ieee802154_associate_resp),
	IEEE802154_OP(IEEE802154_DISASSOCIATE_REQ, ieee802154_disassociate_req),
	IEEE802154_OP(IEEE802154_SCAN_REQ, ieee802154_scan_req),
	IEEE802154_OP(IEEE802154_START_REQ, ieee802154_start_req),
	IEEE802154_DUMP(IEEE802154_LIST_IFACE, ieee802154_list_iface,
							ieee802154_dump_iface),
};

static int __init ieee802154_nl_init(void)
{
	int rc;
	int i;

	rc = genl_register_family(&ieee802154_coordinator_family);
	if (rc)
		goto fail;

	rc = genl_register_mc_group(&ieee802154_coordinator_family,
			&ieee802154_coord_mcgrp);
	if (rc)
		goto fail;

	rc = genl_register_mc_group(&ieee802154_coordinator_family,
			&ieee802154_beacon_mcgrp);
	if (rc)
		goto fail;


	for (i = 0; i < ARRAY_SIZE(ieee802154_coordinator_ops); i++) {
		rc = genl_register_ops(&ieee802154_coordinator_family,
				&ieee802154_coordinator_ops[i]);
		if (rc)
			goto fail;
	}

	return 0;

fail:
	genl_unregister_family(&ieee802154_coordinator_family);
	return rc;
}
module_init(ieee802154_nl_init);

static void __exit ieee802154_nl_exit(void)
{
	genl_unregister_family(&ieee802154_coordinator_family);
}
module_exit(ieee802154_nl_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("ieee 802.15.4 configuration interface");

