

#define BFIN_MAC_CSUM_OFFLOAD

struct dma_descriptor {
	struct dma_descriptor *next_dma_desc;
	unsigned long start_addr;
	unsigned short config;
	unsigned short x_count;
};

struct status_area_rx {
#if defined(BFIN_MAC_CSUM_OFFLOAD)
	unsigned short ip_hdr_csum;	
	
	unsigned short ip_payload_csum;
#endif
	unsigned long status_word;	
};

struct status_area_tx {
	unsigned long status_word;	
};


struct net_dma_desc_rx {
	struct net_dma_desc_rx *next;
	struct sk_buff *skb;
	struct dma_descriptor desc_a;
	struct dma_descriptor desc_b;
	struct status_area_rx status;
};


struct net_dma_desc_tx {
	struct net_dma_desc_tx *next;
	struct sk_buff *skb;
	struct dma_descriptor desc_a;
	struct dma_descriptor desc_b;
	unsigned char packet[1560];
	struct status_area_tx status;
};

struct bfin_mac_local {
	
	struct net_device_stats stats;

	unsigned char Mac[6];	
	spinlock_t lock;

	
	int old_link;          
	int old_speed;
	int old_duplex;

	struct phy_device *phydev;
	struct mii_bus *mii_bus;
};

extern void bfin_get_ether_addr(char *addr);
