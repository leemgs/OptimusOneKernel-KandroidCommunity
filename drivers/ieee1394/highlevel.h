#ifndef IEEE1394_HIGHLEVEL_H
#define IEEE1394_HIGHLEVEL_H

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/types.h>

struct module;

#include "ieee1394_types.h"

struct hpsb_host;


struct hpsb_address_serve {
	struct list_head host_list;	
	struct list_head hl_list;	
	const struct hpsb_address_ops *op;
	struct hpsb_host *host;
	u64 start;	
	u64 end;	
};



struct hpsb_highlevel {
	const char *name;

	

	
	void (*add_host)(struct hpsb_host *host);

	
	void (*remove_host)(struct hpsb_host *host);

	
	void (*host_reset)(struct hpsb_host *host);

	
	void (*fcp_request)(struct hpsb_host *host, int nodeid, int direction,
			    int cts, u8 *data, size_t length);

	
	struct list_head hl_list;
	struct list_head irq_list;
	struct list_head addr_list;

	struct list_head host_info_list;
	rwlock_t host_info_lock;
};

struct hpsb_address_ops {
	

	
	int (*read)(struct hpsb_host *host, int nodeid, quadlet_t *buffer,
		    u64 addr, size_t length, u16 flags);
	int (*write)(struct hpsb_host *host, int nodeid, int destid,
		     quadlet_t *data, u64 addr, size_t length, u16 flags);

	
	int (*lock)(struct hpsb_host *host, int nodeid, quadlet_t *store,
		    u64 addr, quadlet_t data, quadlet_t arg, int ext_tcode,
		    u16 flags);
	int (*lock64)(struct hpsb_host *host, int nodeid, octlet_t *store,
		      u64 addr, octlet_t data, octlet_t arg, int ext_tcode,
		      u16 flags);
};

void highlevel_add_host(struct hpsb_host *host);
void highlevel_remove_host(struct hpsb_host *host);
void highlevel_host_reset(struct hpsb_host *host);
int highlevel_read(struct hpsb_host *host, int nodeid, void *data, u64 addr,
		   unsigned int length, u16 flags);
int highlevel_write(struct hpsb_host *host, int nodeid, int destid, void *data,
		    u64 addr, unsigned int length, u16 flags);
int highlevel_lock(struct hpsb_host *host, int nodeid, quadlet_t *store,
		   u64 addr, quadlet_t data, quadlet_t arg, int ext_tcode,
		   u16 flags);
int highlevel_lock64(struct hpsb_host *host, int nodeid, octlet_t *store,
		     u64 addr, octlet_t data, octlet_t arg, int ext_tcode,
		     u16 flags);
void highlevel_fcp_request(struct hpsb_host *host, int nodeid, int direction,
			   void *data, size_t length);


static inline void hpsb_init_highlevel(struct hpsb_highlevel *hl)
{
	rwlock_init(&hl->host_info_lock);
	INIT_LIST_HEAD(&hl->host_info_list);
}
void hpsb_register_highlevel(struct hpsb_highlevel *hl);
void hpsb_unregister_highlevel(struct hpsb_highlevel *hl);

u64 hpsb_allocate_and_register_addrspace(struct hpsb_highlevel *hl,
					 struct hpsb_host *host,
					 const struct hpsb_address_ops *ops,
					 u64 size, u64 alignment,
					 u64 start, u64 end);
int hpsb_register_addrspace(struct hpsb_highlevel *hl, struct hpsb_host *host,
			    const struct hpsb_address_ops *ops,
			    u64 start, u64 end);
int hpsb_unregister_addrspace(struct hpsb_highlevel *hl, struct hpsb_host *host,
			      u64 start);

void *hpsb_get_hostinfo(struct hpsb_highlevel *hl, struct hpsb_host *host);
void *hpsb_create_hostinfo(struct hpsb_highlevel *hl, struct hpsb_host *host,
			   size_t data_size);
void hpsb_destroy_hostinfo(struct hpsb_highlevel *hl, struct hpsb_host *host);
void hpsb_set_hostinfo_key(struct hpsb_highlevel *hl, struct hpsb_host *host,
			   unsigned long key);
void *hpsb_get_hostinfo_bykey(struct hpsb_highlevel *hl, unsigned long key);
int hpsb_set_hostinfo(struct hpsb_highlevel *hl, struct hpsb_host *host,
		      void *data);

#endif 
