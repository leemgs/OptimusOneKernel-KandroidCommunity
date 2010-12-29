

#ifndef __WL1251_TX_H__
#define __WL1251_TX_H__

#include <linux/bitops.h>



#define TX_COMPLETE_REQUIRED_BIT	0x80
#define TX_STATUS_DATA_OUT_COUNT_MASK   0xf

#define WL1251_TX_ALIGN_TO 4
#define WL1251_TX_ALIGN(len) (((len) + WL1251_TX_ALIGN_TO - 1) & \
			     ~(WL1251_TX_ALIGN_TO - 1))
#define WL1251_TKIP_IV_SPACE 4

struct tx_control {
	
	unsigned rate_policy:3;

	
	unsigned ack_policy:1;

	
	unsigned packet_type:2;

	
	unsigned qos:1;

	
	unsigned tx_complete:1;

	
	unsigned xfer_pad:1;

	unsigned reserved:7;
} __attribute__ ((packed));


struct tx_double_buffer_desc {
	
	u16 length;

	
	u16 rate;

	
	u32 expiry_time;

	
	u8 xmit_queue;

	
	u8 id;

	struct tx_control control;

	
	u16 frag_threshold;

	
	u8 num_mem_blocks;

	u8 reserved;
} __attribute__ ((packed));

enum {
	TX_SUCCESS              = 0,
	TX_DMA_ERROR            = BIT(7),
	TX_DISABLED             = BIT(6),
	TX_RETRY_EXCEEDED       = BIT(5),
	TX_TIMEOUT              = BIT(4),
	TX_KEY_NOT_FOUND        = BIT(3),
	TX_ENCRYPT_FAIL         = BIT(2),
	TX_UNAVAILABLE_PRIORITY = BIT(1),
};

struct tx_result {
	
	u8 done_1;

	
	u8 id;

	
	u16 medium_usage;

	
	u32 medium_delay;

	
	u32 fw_hnadling_time;

	
	u8 lsb_seq_num;

	
	u8 ack_failures;

	
	u16 rate;

	u16 reserved;

	
	u8 status;

	
	u8 done_2;
} __attribute__ ((packed));

void wl1251_tx_work(struct work_struct *work);
void wl1251_tx_complete(struct wl1251 *wl);
void wl1251_tx_flush(struct wl1251 *wl);

#endif
