#ifndef _IEEE1394_HOSTS_H
#define _IEEE1394_HOSTS_H

#include <linux/device.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <asm/atomic.h>

struct pci_dev;
struct module;

#include "ieee1394_types.h"
#include "csr.h"
#include "highlevel.h"

struct hpsb_packet;
struct hpsb_iso;

struct hpsb_host {
	struct list_head host_list;

	void *hostdata;

	atomic_t generation;

	struct list_head pending_packets;
	struct timer_list timeout;
	unsigned long timeout_interval;

	int node_count;      
	int selfid_count;    
	int nodes_active;    

	nodeid_t node_id;    
	nodeid_t irm_id;     
	nodeid_t busmgr_id;  

	
	unsigned in_bus_reset:1;
	unsigned is_shutdown:1;
	unsigned resume_packet_sent:1;

	
	unsigned is_root:1;
	unsigned is_cycmst:1;
	unsigned is_irm:1;
	unsigned is_busmgr:1;

	int reset_retries;
	quadlet_t *topology_map;
	u8 *speed_map;

	int id;
	struct hpsb_host_driver *driver;
	struct pci_dev *pdev;
	struct device device;
	struct device host_dev;

	struct delayed_work delayed_reset;
	unsigned config_roms:31;
	unsigned update_config_rom:1;

	struct list_head addr_space;
	u64 low_addr_space;	
	u64 middle_addr_space;	

	u8 speed[ALL_NODES];	

	
	u8 next_tl[ALL_NODES];
	struct { DECLARE_BITMAP(map, 64); } tl_pool[ALL_NODES];

	struct csr_control csr;

	struct hpsb_address_serve dummy_zero_addr;
	struct hpsb_address_serve dummy_max_addr;
};

enum devctl_cmd {
	
	RESET_BUS,

	
	GET_CYCLE_COUNTER,

	
	SET_CYCLE_COUNTER,

	
	SET_BUS_ID,

	
	ACT_CYCLE_MASTER,

	
	CANCEL_REQUESTS,
};

enum isoctl_cmd {
	

	XMIT_INIT,
	XMIT_START,
	XMIT_STOP,
	XMIT_QUEUE,
	XMIT_SHUTDOWN,

	RECV_INIT,
	RECV_LISTEN_CHANNEL,   
	RECV_UNLISTEN_CHANNEL, 
	RECV_SET_CHANNEL_MASK, 
	RECV_START,
	RECV_STOP,
	RECV_RELEASE,
	RECV_SHUTDOWN,
	RECV_FLUSH
};

enum reset_types {
	
	LONG_RESET,

	
	SHORT_RESET,

	
	LONG_RESET_FORCE_ROOT, SHORT_RESET_FORCE_ROOT,

	
	LONG_RESET_NO_FORCE_ROOT, SHORT_RESET_NO_FORCE_ROOT
};

struct hpsb_host_driver {
	struct module *owner;
	const char *name;

	
	void (*set_hw_config_rom)(struct hpsb_host *host,
				  __be32 *config_rom);

	
	int (*transmit_packet)(struct hpsb_host *host,
			       struct hpsb_packet *packet);

	
	int (*devctl)(struct hpsb_host *host, enum devctl_cmd command, int arg);

	 
	int (*isoctl)(struct hpsb_iso *iso, enum isoctl_cmd command,
		      unsigned long arg);

	
	quadlet_t (*hw_csr_reg) (struct hpsb_host *host, int reg,
				 quadlet_t data, quadlet_t compare);
};

struct hpsb_host *hpsb_alloc_host(struct hpsb_host_driver *drv, size_t extra,
				  struct device *dev);
int hpsb_add_host(struct hpsb_host *host);
void hpsb_resume_host(struct hpsb_host *host);
void hpsb_remove_host(struct hpsb_host *host);
int hpsb_update_config_rom_image(struct hpsb_host *host);

#endif 
