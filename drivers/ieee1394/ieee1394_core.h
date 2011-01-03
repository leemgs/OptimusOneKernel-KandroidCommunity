#ifndef _IEEE1394_CORE_H
#define _IEEE1394_CORE_H

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <asm/atomic.h>

#include "hosts.h"
#include "ieee1394_types.h"

struct hpsb_packet {
	

	
	struct list_head driver_list;

	nodeid_t node_id;

	
	enum { hpsb_async, hpsb_raw } __attribute__((packed)) type;

	
	enum {
		hpsb_unused, hpsb_queued, hpsb_pending, hpsb_complete
	} __attribute__((packed)) state;

	
	signed char tlabel;
	signed char ack_code;
	unsigned char tcode;

	unsigned expect_response:1;
	unsigned no_waiter:1;

	
	unsigned speed_code:2;

	struct hpsb_host *host;
	unsigned int generation;

	atomic_t refcnt;
	struct list_head queue;

	
	void (*complete_routine)(void *);
	void *complete_data;

	
	unsigned long sendtime;

	
	size_t allocated_data_size;	

	
	size_t data_size;		
	size_t header_size;		

	
	quadlet_t *data;		
	quadlet_t header[5];
	quadlet_t embedded_data[0];	
};

void hpsb_set_packet_complete_task(struct hpsb_packet *packet,
				   void (*routine)(void *), void *data);
static inline struct hpsb_packet *driver_packet(struct list_head *l)
{
	return list_entry(l, struct hpsb_packet, driver_list);
}
void abort_timedouts(unsigned long __opaque);
struct hpsb_packet *hpsb_alloc_packet(size_t data_size);
void hpsb_free_packet(struct hpsb_packet *packet);


static inline unsigned int get_hpsb_generation(struct hpsb_host *host)
{
	return atomic_read(&host->generation);
}

int hpsb_send_phy_config(struct hpsb_host *host, int rootid, int gapcnt);
int hpsb_send_packet(struct hpsb_packet *packet);
int hpsb_send_packet_and_wait(struct hpsb_packet *packet);
int hpsb_reset_bus(struct hpsb_host *host, int type);
int hpsb_read_cycle_timer(struct hpsb_host *host, u32 *cycle_timer,
			  u64 *local_time);

int hpsb_bus_reset(struct hpsb_host *host);
void hpsb_selfid_received(struct hpsb_host *host, quadlet_t sid);
void hpsb_selfid_complete(struct hpsb_host *host, int phyid, int isroot);
void hpsb_packet_sent(struct hpsb_host *host, struct hpsb_packet *packet,
		      int ackcode);
void hpsb_packet_received(struct hpsb_host *host, quadlet_t *data, size_t size,
			  int write_acked);



#define IEEE1394_MAJOR			 171

#define IEEE1394_MINOR_BLOCK_RAW1394	   0
#define IEEE1394_MINOR_BLOCK_VIDEO1394	   1
#define IEEE1394_MINOR_BLOCK_DV1394	   2
#define IEEE1394_MINOR_BLOCK_EXPERIMENTAL 15

#define IEEE1394_CORE_DEV	  MKDEV(IEEE1394_MAJOR, 0)
#define IEEE1394_RAW1394_DEV	  MKDEV(IEEE1394_MAJOR, \
					IEEE1394_MINOR_BLOCK_RAW1394 * 16)
#define IEEE1394_VIDEO1394_DEV	  MKDEV(IEEE1394_MAJOR, \
					IEEE1394_MINOR_BLOCK_VIDEO1394 * 16)
#define IEEE1394_DV1394_DEV	  MKDEV(IEEE1394_MAJOR, \
					IEEE1394_MINOR_BLOCK_DV1394 * 16)
#define IEEE1394_EXPERIMENTAL_DEV MKDEV(IEEE1394_MAJOR, \
					IEEE1394_MINOR_BLOCK_EXPERIMENTAL * 16)


static inline unsigned char ieee1394_file_to_instance(struct file *file)
{
	int idx = cdev_index(file->f_path.dentry->d_inode);
	if (idx < 0)
		idx = 0;
	return idx;
}

extern int hpsb_disable_irm;


extern struct bus_type ieee1394_bus_type;
extern struct class hpsb_host_class;
extern struct class *hpsb_protocol_class;

#endif 
