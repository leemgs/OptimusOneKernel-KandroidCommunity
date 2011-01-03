

#ifndef IEEE1394_ISO_H
#define IEEE1394_ISO_H

#include <linux/spinlock_types.h>
#include <linux/wait.h>
#include <asm/atomic.h>
#include <asm/types.h>

#include "dma.h"

struct hpsb_host;






struct hpsb_iso_packet_info {
	
	__u32 offset;

	
	__u16 len;

	
	__u16 cycle;

	
	__u8 channel;

	
	__u8 tag;
	__u8 sy;

	
	__u16 total_len;
};

enum hpsb_iso_type { HPSB_ISO_RECV = 0, HPSB_ISO_XMIT = 1 };


enum raw1394_iso_dma_recv_mode {
	HPSB_ISO_DMA_DEFAULT = -1,
	HPSB_ISO_DMA_OLD_ABI = 0,
	HPSB_ISO_DMA_BUFFERFILL = 1,
	HPSB_ISO_DMA_PACKET_PER_BUFFER = 2
};

struct hpsb_iso {
	enum hpsb_iso_type type;

	
	struct hpsb_host *host;
	void *hostdata;

	
	void (*callback)(struct hpsb_iso*);

	
	wait_queue_head_t waitq;

	int speed; 
	int channel; 
	int dma_mode; 


	
	int irq_interval;

	
	struct dma_region data_buf;

	
	unsigned int buf_size;

	
	unsigned int buf_packets;

	
	spinlock_t lock;

	
	int first_packet;

	
	int pkt_dma;

	
	int n_ready_packets;

	
	atomic_t overflows;
	
	atomic_t skips;

	
	int bytes_discarded;

	
#define HPSB_ISO_DRIVER_INIT     (1<<0)
#define HPSB_ISO_DRIVER_STARTED  (1<<1)
	unsigned int flags;

	
	int prebuffer;

	
	int start_cycle;

	
	int xmit_cycle;

	
	struct hpsb_iso_packet_info *infos;
};



struct hpsb_iso* hpsb_iso_xmit_init(struct hpsb_host *host,
				    unsigned int data_buf_size,
				    unsigned int buf_packets,
				    int channel,
				    int speed,
				    int irq_interval,
				    void (*callback)(struct hpsb_iso*));
struct hpsb_iso* hpsb_iso_recv_init(struct hpsb_host *host,
				    unsigned int data_buf_size,
				    unsigned int buf_packets,
				    int channel,
				    int dma_mode,
				    int irq_interval,
				    void (*callback)(struct hpsb_iso*));
int hpsb_iso_recv_listen_channel(struct hpsb_iso *iso, unsigned char channel);
int hpsb_iso_recv_unlisten_channel(struct hpsb_iso *iso, unsigned char channel);
int hpsb_iso_recv_set_channel_mask(struct hpsb_iso *iso, u64 mask);
int hpsb_iso_xmit_start(struct hpsb_iso *iso, int start_on_cycle,
			int prebuffer);
int hpsb_iso_recv_start(struct hpsb_iso *iso, int start_on_cycle,
			int tag_mask, int sync);
void hpsb_iso_stop(struct hpsb_iso *iso);
void hpsb_iso_shutdown(struct hpsb_iso *iso);
int hpsb_iso_xmit_queue_packet(struct hpsb_iso *iso, u32 offset, u16 len,
			       u8 tag, u8 sy);
int hpsb_iso_xmit_sync(struct hpsb_iso *iso);
int hpsb_iso_recv_release_packets(struct hpsb_iso *recv,
				  unsigned int n_packets);
int hpsb_iso_recv_flush(struct hpsb_iso *iso);
int hpsb_iso_n_ready(struct hpsb_iso *iso);



void hpsb_iso_packet_sent(struct hpsb_iso *iso, int cycle, int error);
void hpsb_iso_packet_received(struct hpsb_iso *iso, u32 offset, u16 len,
			      u16 total_len, u16 cycle, u8 channel, u8 tag,
			      u8 sy);
void hpsb_iso_wake(struct hpsb_iso *iso);

#endif 
