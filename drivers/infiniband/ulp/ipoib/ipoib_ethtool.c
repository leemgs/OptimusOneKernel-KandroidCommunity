

#include <linux/kernel.h>
#include <linux/ethtool.h>
#include <linux/netdevice.h>

#include "ipoib.h"

static void ipoib_get_drvinfo(struct net_device *netdev,
			      struct ethtool_drvinfo *drvinfo)
{
	strncpy(drvinfo->driver, "ipoib", sizeof(drvinfo->driver) - 1);
}

static u32 ipoib_get_rx_csum(struct net_device *dev)
{
	struct ipoib_dev_priv *priv = netdev_priv(dev);
	return test_bit(IPOIB_FLAG_CSUM, &priv->flags) &&
		!test_bit(IPOIB_FLAG_ADMIN_CM, &priv->flags);
}

static int ipoib_get_coalesce(struct net_device *dev,
			      struct ethtool_coalesce *coal)
{
	struct ipoib_dev_priv *priv = netdev_priv(dev);

	coal->rx_coalesce_usecs = priv->ethtool.coalesce_usecs;
	coal->tx_coalesce_usecs = priv->ethtool.coalesce_usecs;
	coal->rx_max_coalesced_frames = priv->ethtool.max_coalesced_frames;
	coal->tx_max_coalesced_frames = priv->ethtool.max_coalesced_frames;

	return 0;
}

static int ipoib_set_coalesce(struct net_device *dev,
			      struct ethtool_coalesce *coal)
{
	struct ipoib_dev_priv *priv = netdev_priv(dev);
	int ret;

	
	if (coal->rx_coalesce_usecs       > 0xffff ||
	    coal->rx_max_coalesced_frames > 0xffff)
		return -EINVAL;

	ret = ib_modify_cq(priv->recv_cq, coal->rx_max_coalesced_frames,
			   coal->rx_coalesce_usecs);
	if (ret && ret != -ENOSYS) {
		ipoib_warn(priv, "failed modifying CQ (%d)\n", ret);
		return ret;
	}

	coal->tx_coalesce_usecs       = coal->rx_coalesce_usecs;
	coal->tx_max_coalesced_frames = coal->rx_max_coalesced_frames;
	priv->ethtool.coalesce_usecs       = coal->rx_coalesce_usecs;
	priv->ethtool.max_coalesced_frames = coal->rx_max_coalesced_frames;

	return 0;
}

static const char ipoib_stats_keys[][ETH_GSTRING_LEN] = {
	"LRO aggregated", "LRO flushed",
	"LRO avg aggr", "LRO no desc"
};

static void ipoib_get_strings(struct net_device *netdev, u32 stringset, u8 *data)
{
	switch (stringset) {
	case ETH_SS_STATS:
		memcpy(data, *ipoib_stats_keys,	sizeof(ipoib_stats_keys));
		break;
	}
}

static int ipoib_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(ipoib_stats_keys);
	default:
		return -EOPNOTSUPP;
	}
}

static void ipoib_get_ethtool_stats(struct net_device *dev,
				struct ethtool_stats *stats, uint64_t *data)
{
	struct ipoib_dev_priv *priv = netdev_priv(dev);
	int index = 0;

	
	data[index++] = priv->lro.lro_mgr.stats.aggregated;
	data[index++] = priv->lro.lro_mgr.stats.flushed;
	if (priv->lro.lro_mgr.stats.flushed)
		data[index++] = priv->lro.lro_mgr.stats.aggregated /
				priv->lro.lro_mgr.stats.flushed;
	else
		data[index++] = 0;
	data[index++] = priv->lro.lro_mgr.stats.no_desc;
}

static const struct ethtool_ops ipoib_ethtool_ops = {
	.get_drvinfo		= ipoib_get_drvinfo,
	.get_rx_csum		= ipoib_get_rx_csum,
	.get_coalesce		= ipoib_get_coalesce,
	.set_coalesce		= ipoib_set_coalesce,
	.get_flags		= ethtool_op_get_flags,
	.set_flags		= ethtool_op_set_flags,
	.get_strings		= ipoib_get_strings,
	.get_sset_count		= ipoib_get_sset_count,
	.get_ethtool_stats	= ipoib_get_ethtool_stats,
};

void ipoib_set_ethtool_ops(struct net_device *dev)
{
	SET_ETHTOOL_OPS(dev, &ipoib_ethtool_ops);
}
