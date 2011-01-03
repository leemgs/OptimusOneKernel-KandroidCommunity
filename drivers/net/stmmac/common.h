

#include "descs.h"
#include <linux/io.h>


#define DMA_BUS_MODE		0x00001000	
#define DMA_XMT_POLL_DEMAND	0x00001004	
#define DMA_RCV_POLL_DEMAND	0x00001008	
#define DMA_RCV_BASE_ADDR	0x0000100c	
#define DMA_TX_BASE_ADDR	0x00001010	
#define DMA_STATUS		0x00001014	
#define DMA_CONTROL		0x00001018	
#define DMA_INTR_ENA		0x0000101c	
#define DMA_MISSED_FRAME_CTR	0x00001020	
#define DMA_CUR_TX_BUF_ADDR	0x00001050	
#define DMA_CUR_RX_BUF_ADDR	0x00001054	


#define DMA_CONTROL_ST		0x00002000	
#define DMA_CONTROL_SR		0x00000002	



#define DMA_INTR_ENA_NIE 0x00010000	
#define DMA_INTR_ENA_TIE 0x00000001	
#define DMA_INTR_ENA_TUE 0x00000004	
#define DMA_INTR_ENA_RIE 0x00000040	
#define DMA_INTR_ENA_ERE 0x00004000	

#define DMA_INTR_NORMAL	(DMA_INTR_ENA_NIE | DMA_INTR_ENA_RIE | \
			DMA_INTR_ENA_TIE)


#define DMA_INTR_ENA_AIE 0x00008000	
#define DMA_INTR_ENA_FBE 0x00002000	
#define DMA_INTR_ENA_ETE 0x00000400	
#define DMA_INTR_ENA_RWE 0x00000200	
#define DMA_INTR_ENA_RSE 0x00000100	
#define DMA_INTR_ENA_RUE 0x00000080	
#define DMA_INTR_ENA_UNE 0x00000020	
#define DMA_INTR_ENA_OVE 0x00000010	
#define DMA_INTR_ENA_TJE 0x00000008	
#define DMA_INTR_ENA_TSE 0x00000002	

#define DMA_INTR_ABNORMAL	(DMA_INTR_ENA_AIE | DMA_INTR_ENA_FBE | \
				DMA_INTR_ENA_UNE)


#define DMA_INTR_DEFAULT_MASK	(DMA_INTR_NORMAL | DMA_INTR_ABNORMAL)


#define DMA_STATUS_GPI		0x10000000	
#define DMA_STATUS_GMI		0x08000000	
#define DMA_STATUS_GLI		0x04000000	
#define DMA_STATUS_GMI		0x08000000
#define DMA_STATUS_GLI		0x04000000
#define DMA_STATUS_EB_MASK	0x00380000	
#define DMA_STATUS_EB_TX_ABORT	0x00080000	
#define DMA_STATUS_EB_RX_ABORT	0x00100000	
#define DMA_STATUS_TS_MASK	0x00700000	
#define DMA_STATUS_TS_SHIFT	20
#define DMA_STATUS_RS_MASK	0x000e0000	
#define DMA_STATUS_RS_SHIFT	17
#define DMA_STATUS_NIS	0x00010000	
#define DMA_STATUS_AIS	0x00008000	
#define DMA_STATUS_ERI	0x00004000	
#define DMA_STATUS_FBI	0x00002000	
#define DMA_STATUS_ETI	0x00000400	
#define DMA_STATUS_RWT	0x00000200	
#define DMA_STATUS_RPS	0x00000100	
#define DMA_STATUS_RU	0x00000080	
#define DMA_STATUS_RI	0x00000040	
#define DMA_STATUS_UNF	0x00000020	
#define DMA_STATUS_OVF	0x00000010	
#define DMA_STATUS_TJT	0x00000008	
#define DMA_STATUS_TU	0x00000004	
#define DMA_STATUS_TPS	0x00000002	
#define DMA_STATUS_TI	0x00000001	


#define HASH_TABLE_SIZE 64
#define PAUSE_TIME 0x200


#define FLOW_OFF	0
#define FLOW_RX		1
#define FLOW_TX		2
#define FLOW_AUTO	(FLOW_TX | FLOW_RX)


#define SF_DMA_MODE 1

#define HW_CSUM 1
#define NO_HW_CSUM 0


#define BUF_SIZE_16KiB 16384
#define BUF_SIZE_8KiB 8192
#define BUF_SIZE_4KiB 4096
#define BUF_SIZE_2KiB 2048


#define PMT_NOT_SUPPORTED 0
#define PMT_SUPPORTED 1


#define MAC_CTRL_REG		0x00000000	
#define MAC_ENABLE_TX		0x00000008	
#define MAC_RNABLE_RX		0x00000004	


#define MMC_CONTROL		0x00000100	
#define MMC_HIGH_INTR		0x00000104	
#define MMC_LOW_INTR		0x00000108	
#define MMC_HIGH_INTR_MASK	0x0000010c	
#define MMC_LOW_INTR_MASK	0x00000110	

#define MMC_CONTROL_MAX_FRM_MASK	0x0003ff8	
#define MMC_CONTROL_MAX_FRM_SHIFT	3
#define MMC_CONTROL_MAX_FRAME		0x7FF

struct stmmac_extra_stats {
	
	unsigned long tx_underflow ____cacheline_aligned;
	unsigned long tx_carrier;
	unsigned long tx_losscarrier;
	unsigned long tx_heartbeat;
	unsigned long tx_deferred;
	unsigned long tx_vlan;
	unsigned long tx_jabber;
	unsigned long tx_frame_flushed;
	unsigned long tx_payload_error;
	unsigned long tx_ip_header_error;
	
	unsigned long rx_desc;
	unsigned long rx_partial;
	unsigned long rx_runt;
	unsigned long rx_toolong;
	unsigned long rx_collision;
	unsigned long rx_crc;
	unsigned long rx_lenght;
	unsigned long rx_mii;
	unsigned long rx_multicast;
	unsigned long rx_gmac_overflow;
	unsigned long rx_watchdog;
	unsigned long da_rx_filter_fail;
	unsigned long sa_rx_filter_fail;
	unsigned long rx_missed_cntr;
	unsigned long rx_overflow_cntr;
	unsigned long rx_vlan;
	
	unsigned long tx_undeflow_irq;
	unsigned long tx_process_stopped_irq;
	unsigned long tx_jabber_irq;
	unsigned long rx_overflow_irq;
	unsigned long rx_buf_unav_irq;
	unsigned long rx_process_stopped_irq;
	unsigned long rx_watchdog_irq;
	unsigned long tx_early_irq;
	unsigned long fatal_bus_error_irq;
	
	unsigned long threshold;
	unsigned long tx_pkt_n;
	unsigned long rx_pkt_n;
	unsigned long poll_n;
	unsigned long sched_timer_n;
	unsigned long normal_irq_n;
};


enum rx_frame_status {
	good_frame = 0,
	discard_frame = 1,
	csum_none = 2,
};

static inline void stmmac_set_mac_addr(unsigned long ioaddr, u8 addr[6],
			 unsigned int high, unsigned int low)
{
	unsigned long data;

	data = (addr[5] << 8) | addr[4];
	writel(data, ioaddr + high);
	data = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
	writel(data, ioaddr + low);

	return;
}

static inline void stmmac_get_mac_addr(unsigned long ioaddr,
				unsigned char *addr, unsigned int high,
				unsigned int low)
{
	unsigned int hi_addr, lo_addr;

	
	hi_addr = readl(ioaddr + high);
	lo_addr = readl(ioaddr + low);

	
	addr[0] = lo_addr & 0xff;
	addr[1] = (lo_addr >> 8) & 0xff;
	addr[2] = (lo_addr >> 16) & 0xff;
	addr[3] = (lo_addr >> 24) & 0xff;
	addr[4] = hi_addr & 0xff;
	addr[5] = (hi_addr >> 8) & 0xff;

	return;
}

struct stmmac_ops {
	
	void (*core_init) (unsigned long ioaddr) ____cacheline_aligned;
	
	int (*dma_init) (unsigned long ioaddr, int pbl, u32 dma_tx, u32 dma_rx);
	
	void (*dump_mac_regs) (unsigned long ioaddr);
	
	void (*dump_dma_regs) (unsigned long ioaddr);
	
	void (*dma_mode) (unsigned long ioaddr, int txmode, int rxmode);
	
	void (*dma_diagnostic_fr) (void *data, struct stmmac_extra_stats *x,
				   unsigned long ioaddr);
	
	void (*init_rx_desc) (struct dma_desc *p, unsigned int ring_size,
				int disable_rx_ic);
	
	void (*init_tx_desc) (struct dma_desc *p, unsigned int ring_size);

	
	void (*prepare_tx_desc) (struct dma_desc *p, int is_fs, int len,
				 int csum_flag);
	
	void (*set_tx_owner) (struct dma_desc *p);
	int (*get_tx_owner) (struct dma_desc *p);
	
	void (*close_tx_desc) (struct dma_desc *p);
	
	void (*release_tx_desc) (struct dma_desc *p);
	
	void (*clear_tx_ic) (struct dma_desc *p);
	
	int (*get_tx_ls) (struct dma_desc *p);
	
	int (*tx_status) (void *data, struct stmmac_extra_stats *x,
			  struct dma_desc *p, unsigned long ioaddr);
	
	int (*get_tx_len) (struct dma_desc *p);
	
	void (*host_irq_status) (unsigned long ioaddr);
	int (*get_rx_owner) (struct dma_desc *p);
	void (*set_rx_owner) (struct dma_desc *p);
	
	int (*get_rx_frame_len) (struct dma_desc *p);
	
	int (*rx_status) (void *data, struct stmmac_extra_stats *x,
			  struct dma_desc *p);
	
	void (*set_filter) (struct net_device *dev);
	
	void (*flow_ctrl) (unsigned long ioaddr, unsigned int duplex,
			   unsigned int fc, unsigned int pause_time);
	
	void (*pmt) (unsigned long ioaddr, unsigned long mode);
	
	void (*set_umac_addr) (unsigned long ioaddr, unsigned char *addr,
			     unsigned int reg_n);
	void (*get_umac_addr) (unsigned long ioaddr, unsigned char *addr,
			     unsigned int reg_n);
};

struct mac_link {
	int port;
	int duplex;
	int speed;
};

struct mii_regs {
	unsigned int addr;	
	unsigned int data;	
};

struct hw_cap {
	unsigned int version;	
	unsigned int pmt;	
	struct mac_link link;
	struct mii_regs mii;
};

struct mac_device_info {
	struct hw_cap hw;
	struct stmmac_ops *ops;
};

struct mac_device_info *gmac_setup(unsigned long addr);
struct mac_device_info *mac100_setup(unsigned long addr);
