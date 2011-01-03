


#define MAC_IOSIZE 0x10000
#define NUM_RX_DMA 4       
#define NUM_TX_DMA 4       

#define NUM_RX_BUFFS 4
#define NUM_TX_BUFFS 4
#define MAX_BUF_SIZE 2048

#define ETH_TX_TIMEOUT HZ/4
#define MAC_MIN_PKT_SIZE 64

#define MULTICAST_FILTER_LIMIT 64


typedef struct db_dest {
	struct db_dest *pnext;
	volatile u32 *vaddr;
	dma_addr_t dma_addr;
} db_dest_t;


typedef struct tx_dma {
	u32 status;
	u32 buff_stat;
	u32 len;
	u32 pad;
} tx_dma_t;

typedef struct rx_dma {
	u32 status;
	u32 buff_stat;
	u32 pad[2];
} rx_dma_t;



typedef struct mac_reg {
	u32 control;
	u32 mac_addr_high;
	u32 mac_addr_low;
	u32 multi_hash_high;
	u32 multi_hash_low;
	u32 mii_control;
	u32 mii_data;
	u32 flow_control;
	u32 vlan1_tag;
	u32 vlan2_tag;
} mac_reg_t;


struct au1000_private {
	db_dest_t *pDBfree;
	db_dest_t db[NUM_RX_BUFFS+NUM_TX_BUFFS];
	volatile rx_dma_t *rx_dma_ring[NUM_RX_DMA];
	volatile tx_dma_t *tx_dma_ring[NUM_TX_DMA];
	db_dest_t *rx_db_inuse[NUM_RX_DMA];
	db_dest_t *tx_db_inuse[NUM_TX_DMA];
	u32 rx_head;
	u32 tx_head;
	u32 tx_tail;
	u32 tx_full;

	int mac_id;

	int mac_enabled;       

	int old_link;          
	int old_speed;
	int old_duplex;

	struct phy_device *phy_dev;
	struct mii_bus *mii_bus;

	
	volatile mac_reg_t *mac;  
	volatile u32 *enable;     

	u32 vaddr;                
	dma_addr_t dma_addr;      

	spinlock_t lock;       
};
