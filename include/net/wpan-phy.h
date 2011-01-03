

#ifndef WPAN_PHY_H
#define WPAN_PHY_H

#include <linux/netdevice.h>
#include <linux/mutex.h>

struct wpan_phy {
	struct mutex pib_lock;

	
	u8 current_channel;
	u8 current_page;
	u32 channels_supported;
	u8 transmit_power;
	u8 cca_mode;

	struct device dev;
	int idx;

	char priv[0] __attribute__((__aligned__(NETDEV_ALIGN)));
};

struct wpan_phy *wpan_phy_alloc(size_t priv_size);
int wpan_phy_register(struct device *parent, struct wpan_phy *phy);
void wpan_phy_unregister(struct wpan_phy *phy);
void wpan_phy_free(struct wpan_phy *phy);

static inline void *wpan_phy_priv(struct wpan_phy *phy)
{
	BUG_ON(!phy);
	return &phy->priv;
}

struct wpan_phy *wpan_phy_find(const char *str);
static inline const char *wpan_phy_name(struct wpan_phy *phy)
{
	return dev_name(&phy->dev);
}
#endif
