

#define DRV_MODULE_VERSION	"Oct_09"

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#define STMMAC_VLAN_TAG_USED
#include <linux/if_vlan.h>
#endif

#include "common.h"
#ifdef CONFIG_STMMAC_TIMER
#include "stmmac_timer.h"
#endif

struct stmmac_priv {
	
	struct dma_desc *dma_tx ____cacheline_aligned;
	dma_addr_t dma_tx_phy;
	struct sk_buff **tx_skbuff;
	unsigned int cur_tx;
	unsigned int dirty_tx;
	unsigned int dma_tx_size;
	int tx_coe;
	int tx_coalesce;

	struct dma_desc *dma_rx ;
	unsigned int cur_rx;
	unsigned int dirty_rx;
	struct sk_buff **rx_skbuff;
	dma_addr_t *rx_skbuff_dma;
	struct sk_buff_head rx_recycle;

	struct net_device *dev;
	int is_gmac;
	dma_addr_t dma_rx_phy;
	unsigned int dma_rx_size;
	int rx_csum;
	unsigned int dma_buf_sz;
	struct device *device;
	struct mac_device_info *mac_type;

	struct stmmac_extra_stats xstats;
	struct napi_struct napi;

	phy_interface_t phy_interface;
	int pbl;
	int bus_id;
	int phy_addr;
	int phy_mask;
	int (*phy_reset) (void *priv);
	void (*fix_mac_speed) (void *priv, unsigned int speed);
	void *bsp_priv;

	int phy_irq;
	struct phy_device *phydev;
	int oldlink;
	int speed;
	int oldduplex;
	unsigned int flow_ctrl;
	unsigned int pause;
	struct mii_bus *mii;

	u32 msg_enable;
	spinlock_t lock;
	int wolopts;
	int wolenabled;
	int shutdown;
#ifdef CONFIG_STMMAC_TIMER
	struct stmmac_timer *tm;
#endif
#ifdef STMMAC_VLAN_TAG_USED
	struct vlan_group *vlgrp;
#endif
};

extern int stmmac_mdio_unregister(struct net_device *ndev);
extern int stmmac_mdio_register(struct net_device *ndev);
extern void stmmac_set_ethtool_ops(struct net_device *netdev);
