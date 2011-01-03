

#ifndef __DSA_PRIV_H
#define __DSA_PRIV_H

#include <linux/list.h>
#include <linux/phy.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <net/dsa.h>

struct dsa_switch {
	
	struct dsa_switch_tree	*dst;
	int			index;

	
	struct dsa_chip_data	*pd;

	
	struct dsa_switch_driver	*drv;

	
	struct mii_bus		*master_mii_bus;

	
	u32			dsa_port_mask;
	u32			phys_port_mask;
	struct mii_bus		*slave_mii_bus;
	struct net_device	*ports[DSA_MAX_PORTS];
};

struct dsa_switch_tree {
	
	struct dsa_platform_data	*pd;

	
	struct net_device	*master_netdev;
	__be16			tag_protocol;

	
	s8			cpu_switch;
	s8			cpu_port;

	
	int			link_poll_needed;
	struct work_struct	link_poll_work;
	struct timer_list	link_poll_timer;

	
	struct dsa_switch	*ds[DSA_MAX_SWITCHES];
};

static inline bool dsa_is_cpu_port(struct dsa_switch *ds, int p)
{
	return !!(ds->index == ds->dst->cpu_switch && p == ds->dst->cpu_port);
}

static inline u8 dsa_upstream_port(struct dsa_switch *ds)
{
	struct dsa_switch_tree *dst = ds->dst;

	
	if (dst->cpu_switch == ds->index)
		return dst->cpu_port;
	else
		return ds->pd->rtable[dst->cpu_switch];
}

struct dsa_slave_priv {
	
	struct net_device	*dev;

	
	struct dsa_switch	*parent;
	u8			port;

	
	struct phy_device	*phy;
};

struct dsa_switch_driver {
	struct list_head	list;

	__be16			tag_protocol;
	int			priv_size;

	
	char	*(*probe)(struct mii_bus *bus, int sw_addr);
	int	(*setup)(struct dsa_switch *ds);
	int	(*set_addr)(struct dsa_switch *ds, u8 *addr);

	
	int	(*phy_read)(struct dsa_switch *ds, int port, int regnum);
	int	(*phy_write)(struct dsa_switch *ds, int port,
			     int regnum, u16 val);

	
	void	(*poll_link)(struct dsa_switch *ds);

	
	void	(*get_strings)(struct dsa_switch *ds, int port, uint8_t *data);
	void	(*get_ethtool_stats)(struct dsa_switch *ds,
				     int port, uint64_t *data);
	int	(*get_sset_count)(struct dsa_switch *ds);
};


extern char dsa_driver_version[];
void register_switch_driver(struct dsa_switch_driver *type);
void unregister_switch_driver(struct dsa_switch_driver *type);


void dsa_slave_mii_bus_init(struct dsa_switch *ds);
struct net_device *dsa_slave_create(struct dsa_switch *ds,
				    struct device *parent,
				    int port, char *name);


netdev_tx_t dsa_xmit(struct sk_buff *skb, struct net_device *dev);


netdev_tx_t edsa_xmit(struct sk_buff *skb, struct net_device *dev);


netdev_tx_t trailer_xmit(struct sk_buff *skb, struct net_device *dev);


#endif
