


#ifndef OCTEON_ETHERNET_H
#define OCTEON_ETHERNET_H


struct octeon_ethernet {
	
	int port;
	
	int queue;
	
	int fau;
	
	int imode;
	
	struct sk_buff_head tx_free_list[16];
	
	struct net_device_stats stats
;	
	struct mii_if_info mii_info;
	
	uint64_t link_info;
	
	void (*poll) (struct net_device *dev);
};


int cvm_oct_free_work(void *work_queue_entry);


int cvm_oct_transmit_qos(struct net_device *dev, void *work_queue_entry,
			 int do_free, int qos);


static inline int cvm_oct_transmit(struct net_device *dev,
				   void *work_queue_entry, int do_free)
{
	return cvm_oct_transmit_qos(dev, work_queue_entry, do_free, 0);
}

extern int cvm_oct_rgmii_init(struct net_device *dev);
extern void cvm_oct_rgmii_uninit(struct net_device *dev);
extern int cvm_oct_rgmii_open(struct net_device *dev);
extern int cvm_oct_rgmii_stop(struct net_device *dev);

extern int cvm_oct_sgmii_init(struct net_device *dev);
extern void cvm_oct_sgmii_uninit(struct net_device *dev);
extern int cvm_oct_sgmii_open(struct net_device *dev);
extern int cvm_oct_sgmii_stop(struct net_device *dev);

extern int cvm_oct_spi_init(struct net_device *dev);
extern void cvm_oct_spi_uninit(struct net_device *dev);
extern int cvm_oct_xaui_init(struct net_device *dev);
extern void cvm_oct_xaui_uninit(struct net_device *dev);
extern int cvm_oct_xaui_open(struct net_device *dev);
extern int cvm_oct_xaui_stop(struct net_device *dev);

extern int cvm_oct_common_init(struct net_device *dev);
extern void cvm_oct_common_uninit(struct net_device *dev);

extern int always_use_pow;
extern int pow_send_group;
extern int pow_receive_group;
extern char pow_send_list[];
extern struct net_device *cvm_oct_device[];

#endif
