

#ifndef __WL1271_TX_H__
#define __WL1271_TX_H__

#define TX_HW_BLOCK_SPARE                2
#define TX_HW_BLOCK_SHIFT_DIV            8

#define TX_HW_MGMT_PKT_LIFETIME_TU       2000

#define TX_HW_DEFAULT_AID                1

#define TX_HW_ATTR_SAVE_RETRIES          BIT(0)
#define TX_HW_ATTR_HEADER_PAD            BIT(1)
#define TX_HW_ATTR_SESSION_COUNTER       (BIT(2) | BIT(3) | BIT(4))
#define TX_HW_ATTR_RATE_POLICY           (BIT(5) | BIT(6) | BIT(7) | \
					  BIT(8) | BIT(9))
#define TX_HW_ATTR_LAST_WORD_PAD         (BIT(10) | BIT(11))
#define TX_HW_ATTR_TX_CMPLT_REQ          BIT(12)

#define TX_HW_ATTR_OFST_SAVE_RETRIES     0
#define TX_HW_ATTR_OFST_HEADER_PAD       1
#define TX_HW_ATTR_OFST_SESSION_COUNTER  2
#define TX_HW_ATTR_OFST_RATE_POLICY      5
#define TX_HW_ATTR_OFST_LAST_WORD_PAD    10
#define TX_HW_ATTR_OFST_TX_CMPLT_REQ     12

#define TX_HW_RESULT_QUEUE_LEN           16
#define TX_HW_RESULT_QUEUE_LEN_MASK      0xf

#define WL1271_TX_ALIGN_TO 4
#define WL1271_TX_ALIGN(len) (((len) + WL1271_TX_ALIGN_TO - 1) & \
			     ~(WL1271_TX_ALIGN_TO - 1))
#define WL1271_TKIP_IV_SPACE 4

struct wl1271_tx_hw_descr {
	
	u16 length;
	
	u8 extra_mem_blocks;
	
	u8 total_mem_blocks;
	
	u32 start_time;
	
	u16 life_time;
	
	u16 tx_attr;
	
	u8 id;
	
	u8 tid;
	
	u8 aid;
	u8 reserved;
} __attribute__ ((packed));

enum wl1271_tx_hw_res_status {
	TX_SUCCESS          = 0,
	TX_HW_ERROR         = 1,
	TX_DISABLED         = 2,
	TX_RETRY_EXCEEDED   = 3,
	TX_TIMEOUT          = 4,
	TX_KEY_NOT_FOUND    = 5,
	TX_PEER_NOT_FOUND   = 6,
	TX_SESSION_MISMATCH = 7
};

struct wl1271_tx_hw_res_descr {
	
	u8 id;
	
	u8 status;
	
	u16 medium_usage;
	
	u32 fw_handling_time;
	
	u32 medium_delay;
	
	u8 lsb_security_sequence_number;
	
	u8 ack_failures;
	
	u8 rate_class_index;
	
	u8 spare;
} __attribute__ ((packed));

struct wl1271_tx_hw_res_if {
	u32 tx_result_fw_counter;
	u32 tx_result_host_counter;
	struct wl1271_tx_hw_res_descr tx_results_queue[TX_HW_RESULT_QUEUE_LEN];
} __attribute__ ((packed));

void wl1271_tx_work(struct work_struct *work);
void wl1271_tx_complete(struct wl1271 *wl, u32 count);
void wl1271_tx_flush(struct wl1271 *wl);

#endif
