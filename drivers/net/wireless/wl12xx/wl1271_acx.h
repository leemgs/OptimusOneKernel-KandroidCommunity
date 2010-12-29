

#ifndef __WL1271_ACX_H__
#define __WL1271_ACX_H__

#include "wl1271.h"
#include "wl1271_cmd.h"



#define WL1271_ACX_INTR_WATCHDOG           BIT(0)

#define WL1271_ACX_INTR_INIT_COMPLETE      BIT(1)

#define WL1271_ACX_INTR_EVENT_A            BIT(2)

#define WL1271_ACX_INTR_EVENT_B            BIT(3)

#define WL1271_ACX_INTR_CMD_COMPLETE       BIT(4)

#define WL1271_ACX_INTR_HW_AVAILABLE       BIT(5)

#define WL1271_ACX_INTR_DATA               BIT(6)

#define WL1271_ACX_INTR_TRACE_A            BIT(7)

#define WL1271_ACX_INTR_TRACE_B            BIT(8)

#define WL1271_ACX_INTR_ALL		   0xFFFFFFFF
#define WL1271_ACX_ALL_EVENTS_VECTOR       (WL1271_ACX_INTR_WATCHDOG      | \
					    WL1271_ACX_INTR_INIT_COMPLETE | \
					    WL1271_ACX_INTR_EVENT_A       | \
					    WL1271_ACX_INTR_EVENT_B       | \
					    WL1271_ACX_INTR_CMD_COMPLETE  | \
					    WL1271_ACX_INTR_HW_AVAILABLE  | \
					    WL1271_ACX_INTR_DATA)

#define WL1271_INTR_MASK                   (WL1271_ACX_INTR_EVENT_A |	\
					    WL1271_ACX_INTR_EVENT_B | \
					    WL1271_ACX_INTR_DATA)


struct acx_header {
	struct wl1271_cmd_header cmd;

	
	u16 id;

	
	u16 len;
};

struct acx_error_counter {
	struct acx_header header;

	
	
	
	u32 PLCP_error;

	
	
	
	u32 FCS_error;

	
	
	
	u32 valid_frame;

	
	
	u32 seq_num_miss;
} __attribute__ ((packed));

struct acx_revision {
	struct acx_header header;

	
	char fw_version[20];

	
	u32 hw_version;
} __attribute__ ((packed));

enum wl1271_psm_mode {
	
	WL1271_PSM_CAM = 0,

	
	WL1271_PSM_PS = 1,

	
	WL1271_PSM_ELP = 2,
};

struct acx_sleep_auth {
	struct acx_header header;

	
	
	
	
	u8  sleep_auth;
	u8  padding[3];
} __attribute__ ((packed));

enum {
	HOSTIF_PCI_MASTER_HOST_INDIRECT,
	HOSTIF_PCI_MASTER_HOST_DIRECT,
	HOSTIF_SLAVE,
	HOSTIF_PKT_RING,
	HOSTIF_DONTCARE = 0xFF
};

#define DEFAULT_UCAST_PRIORITY          0
#define DEFAULT_RX_Q_PRIORITY           0
#define DEFAULT_NUM_STATIONS            1
#define DEFAULT_RXQ_PRIORITY            0 
#define DEFAULT_RXQ_TYPE                0x07    
#define TRACE_BUFFER_MAX_SIZE           256

#define  DP_RX_PACKET_RING_CHUNK_SIZE 1600
#define  DP_TX_PACKET_RING_CHUNK_SIZE 1600
#define  DP_RX_PACKET_RING_CHUNK_NUM 2
#define  DP_TX_PACKET_RING_CHUNK_NUM 2
#define  DP_TX_COMPLETE_TIME_OUT 20
#define  FW_TX_CMPLT_BLOCK_SIZE 16

#define TX_MSDU_LIFETIME_MIN       0
#define TX_MSDU_LIFETIME_MAX       3000
#define TX_MSDU_LIFETIME_DEF       512
#define RX_MSDU_LIFETIME_MIN       0
#define RX_MSDU_LIFETIME_MAX       0xFFFFFFFF
#define RX_MSDU_LIFETIME_DEF       512000

struct acx_rx_msdu_lifetime {
	struct acx_header header;

	
	u32 lifetime;
} __attribute__ ((packed));


struct acx_rx_config {
	struct acx_header header;

	u32 config_options;
	u32 filter_options;
} __attribute__ ((packed));

struct acx_packet_detection {
	struct acx_header header;

	u32 threshold;
} __attribute__ ((packed));


enum acx_slot_type {
	SLOT_TIME_LONG = 0,
	SLOT_TIME_SHORT = 1,
	DEFAULT_SLOT_TIME = SLOT_TIME_SHORT,
	MAX_SLOT_TIMES = 0xFF
};

#define STATION_WONE_INDEX 0

struct acx_slot {
	struct acx_header header;

	u8 wone_index; 
	u8 slot_time;
	u8 reserved[6];
} __attribute__ ((packed));


#define ADDRESS_GROUP_MAX	(8)
#define ADDRESS_GROUP_MAX_LEN	(ETH_ALEN * ADDRESS_GROUP_MAX)

struct acx_dot11_grp_addr_tbl {
	struct acx_header header;

	u8 enabled;
	u8 num_groups;
	u8 pad[2];
	u8 mac_table[ADDRESS_GROUP_MAX_LEN];
} __attribute__ ((packed));


#define  RX_TIMEOUT_PS_POLL_MIN    0
#define  RX_TIMEOUT_PS_POLL_MAX    (200000)
#define  RX_TIMEOUT_PS_POLL_DEF    (15)
#define  RX_TIMEOUT_UPSD_MIN       0
#define  RX_TIMEOUT_UPSD_MAX       (200000)
#define  RX_TIMEOUT_UPSD_DEF       (15)

struct acx_rx_timeout {
	struct acx_header header;

	
	u16 ps_poll_timeout;

	
	u16 upsd_timeout;
} __attribute__ ((packed));

#define RTS_THRESHOLD_MIN              0
#define RTS_THRESHOLD_MAX              4096
#define RTS_THRESHOLD_DEF              2347

struct acx_rts_threshold {
	struct acx_header header;

	u16 threshold;
	u8 pad[2];
} __attribute__ ((packed));

struct acx_beacon_filter_option {
	struct acx_header header;

	u8 enable;

	
	u8 max_num_beacons;
	u8 pad[2];
} __attribute__ ((packed));


#define	BEACON_FILTER_TABLE_MAX_IE_NUM		       (32)
#define BEACON_FILTER_TABLE_MAX_VENDOR_SPECIFIC_IE_NUM (6)
#define BEACON_FILTER_TABLE_IE_ENTRY_SIZE	       (2)
#define BEACON_FILTER_TABLE_EXTRA_VENDOR_SPECIFIC_IE_SIZE (6)
#define BEACON_FILTER_TABLE_MAX_SIZE ((BEACON_FILTER_TABLE_MAX_IE_NUM * \
			    BEACON_FILTER_TABLE_IE_ENTRY_SIZE) + \
			   (BEACON_FILTER_TABLE_MAX_VENDOR_SPECIFIC_IE_NUM * \
			    BEACON_FILTER_TABLE_EXTRA_VENDOR_SPECIFIC_IE_SIZE))

struct acx_beacon_filter_ie_table {
	struct acx_header header;

	u8 num_ie;
	u8 table[BEACON_FILTER_TABLE_MAX_SIZE];
	u8 pad[3];
} __attribute__ ((packed));

enum {
	SG_ENABLE = 0,
	SG_DISABLE,
	SG_SENSE_NO_ACTIVITY,
	SG_SENSE_ACTIVE
};

struct acx_bt_wlan_coex {
	struct acx_header header;

	
	u8 enable;
	u8 pad[3];
} __attribute__ ((packed));

#define PTA_ANTENNA_TYPE_DEF		  (0)
#define PTA_BT_HP_MAXTIME_DEF		  (2000)
#define PTA_WLAN_HP_MAX_TIME_DEF	  (5000)
#define PTA_SENSE_DISABLE_TIMER_DEF	  (1350)
#define PTA_PROTECTIVE_RX_TIME_DEF	  (1500)
#define PTA_PROTECTIVE_TX_TIME_DEF	  (1500)
#define PTA_TIMEOUT_NEXT_BT_LP_PACKET_DEF (3000)
#define PTA_SIGNALING_TYPE_DEF		  (1)
#define PTA_AFH_LEVERAGE_ON_DEF		  (0)
#define PTA_NUMBER_QUIET_CYCLE_DEF	  (0)
#define PTA_MAX_NUM_CTS_DEF		  (3)
#define PTA_NUMBER_OF_WLAN_PACKETS_DEF	  (2)
#define PTA_NUMBER_OF_BT_PACKETS_DEF	  (2)
#define PTA_PROTECTIVE_RX_TIME_FAST_DEF	  (1500)
#define PTA_PROTECTIVE_TX_TIME_FAST_DEF	  (3000)
#define PTA_CYCLE_TIME_FAST_DEF		  (8700)
#define PTA_RX_FOR_AVALANCHE_DEF	  (5)
#define PTA_ELP_HP_DEF			  (0)
#define PTA_ANTI_STARVE_PERIOD_DEF	  (500)
#define PTA_ANTI_STARVE_NUM_CYCLE_DEF	  (4)
#define PTA_ALLOW_PA_SD_DEF		  (1)
#define PTA_TIME_BEFORE_BEACON_DEF	  (6300)
#define PTA_HPDM_MAX_TIME_DEF		  (1600)
#define PTA_TIME_OUT_NEXT_WLAN_DEF	  (2550)
#define PTA_AUTO_MODE_NO_CTS_DEF	  (0)
#define PTA_BT_HP_RESPECTED_DEF		  (3)
#define PTA_WLAN_RX_MIN_RATE_DEF	  (24)
#define PTA_ACK_MODE_DEF		  (1)

struct acx_bt_wlan_coex_param {
	struct acx_header header;

	
	u32 min_rate;

	
	u16 bt_hp_max_time;

	
	u16 wlan_hp_max_time;

	
	u16 sense_disable_timer;

	
	u16 rx_time_bt_hp;
	u16 tx_time_bt_hp;

	
	u16 rx_time_bt_hp_fast;
	u16 tx_time_bt_hp_fast;

	
	u16 wlan_cycle_fast;

	
	u16 bt_anti_starvation_period;

	
	u16 next_bt_lp_packet;

	
	u16 wake_up_beacon;

	
	u16 hp_dm_max_guard_time;

	
	u16 next_wlan_packet;

	
	u8 antenna_type;

	
	u8 signal_type;

	
	u8 afh_leverage_on;

	
	u8 quiet_cycle_num;

	
	u8 max_cts;

	
	u8 wlan_packets_num;

	
	u8 bt_packets_num;

	
	u8 missed_rx_avalanche;

	
	u8 wlan_elp_hp;

	
	u8 bt_anti_starvation_cycles;

	u8 ack_mode_dual_ant;

	
	u8 pa_sd_enable;

	
	u8 pta_auto_mode_enable;

	
	u8 bt_hp_respected_num;
} __attribute__ ((packed));

#define CCA_THRSH_ENABLE_ENERGY_D       0x140A
#define CCA_THRSH_DISABLE_ENERGY_D      0xFFEF

struct acx_energy_detection {
	struct acx_header header;

	
	u16 rx_cca_threshold;
	u8 tx_energy_detection;
	u8 pad;
} __attribute__ ((packed));

#define BCN_RX_TIMEOUT_DEF_VALUE        10000
#define BROADCAST_RX_TIMEOUT_DEF_VALUE  20000
#define RX_BROADCAST_IN_PS_DEF_VALUE    1
#define CONSECUTIVE_PS_POLL_FAILURE_DEF 4

struct acx_beacon_broadcast {
	struct acx_header header;

	u16 beacon_rx_timeout;
	u16 broadcast_timeout;

	
	u8 rx_broadcast_in_ps;

	
	u8 ps_poll_threshold;
	u8 pad[2];
} __attribute__ ((packed));

struct acx_event_mask {
	struct acx_header header;

	u32 event_mask;
	u32 high_event_mask; 
} __attribute__ ((packed));

#define CFG_RX_FCS		BIT(2)
#define CFG_RX_ALL_GOOD		BIT(3)
#define CFG_UNI_FILTER_EN	BIT(4)
#define CFG_BSSID_FILTER_EN	BIT(5)
#define CFG_MC_FILTER_EN	BIT(6)
#define CFG_MC_ADDR0_EN		BIT(7)
#define CFG_MC_ADDR1_EN		BIT(8)
#define CFG_BC_REJECT_EN	BIT(9)
#define CFG_SSID_FILTER_EN	BIT(10)
#define CFG_RX_INT_FCS_ERROR	BIT(11)
#define CFG_RX_INT_ENCRYPTED	BIT(12)
#define CFG_RX_WR_RX_STATUS	BIT(13)
#define CFG_RX_FILTER_NULTI	BIT(14)
#define CFG_RX_RESERVE		BIT(15)
#define CFG_RX_TIMESTAMP_TSF	BIT(16)

#define CFG_RX_RSV_EN		BIT(0)
#define CFG_RX_RCTS_ACK		BIT(1)
#define CFG_RX_PRSP_EN		BIT(2)
#define CFG_RX_PREQ_EN		BIT(3)
#define CFG_RX_MGMT_EN		BIT(4)
#define CFG_RX_FCS_ERROR	BIT(5)
#define CFG_RX_DATA_EN		BIT(6)
#define CFG_RX_CTL_EN		BIT(7)
#define CFG_RX_CF_EN		BIT(8)
#define CFG_RX_BCN_EN		BIT(9)
#define CFG_RX_AUTH_EN		BIT(10)
#define CFG_RX_ASSOC_EN		BIT(11)

#define SCAN_PASSIVE		BIT(0)
#define SCAN_5GHZ_BAND		BIT(1)
#define SCAN_TRIGGERED		BIT(2)
#define SCAN_PRIORITY_HIGH	BIT(3)

struct acx_feature_config {
	struct acx_header header;

	u32 options;
	u32 data_flow_options;
} __attribute__ ((packed));

struct acx_current_tx_power {
	struct acx_header header;

	u8  current_tx_power;
	u8  padding[3];
} __attribute__ ((packed));

enum acx_wake_up_event {
	WAKE_UP_EVENT_BEACON_BITMAP	= 0x01, 
	WAKE_UP_EVENT_DTIM_BITMAP	= 0x02,	
	WAKE_UP_EVENT_N_DTIM_BITMAP	= 0x04, 
	WAKE_UP_EVENT_N_BEACONS_BITMAP	= 0x08, 
	WAKE_UP_EVENT_BITS_MASK		= 0x0F
};

struct acx_wake_up_condition {
	struct acx_header header;

	u8 wake_up_event; 
	u8 listen_interval;
	u8 pad[2];
} __attribute__ ((packed));

struct acx_aid {
	struct acx_header header;

	
	u16 aid;
	u8 pad[2];
} __attribute__ ((packed));

enum acx_preamble_type {
	ACX_PREAMBLE_LONG = 0,
	ACX_PREAMBLE_SHORT = 1
};

struct acx_preamble {
	struct acx_header header;

	
	u8 preamble;
	u8 padding[3];
} __attribute__ ((packed));

enum acx_ctsprotect_type {
	CTSPROTECT_DISABLE = 0,
	CTSPROTECT_ENABLE = 1
};

struct acx_ctsprotect {
	struct acx_header header;
	u8 ctsprotect;
	u8 padding[3];
} __attribute__ ((packed));

struct acx_tx_statistics {
	u32 internal_desc_overflow;
}  __attribute__ ((packed));

struct acx_rx_statistics {
	u32 out_of_mem;
	u32 hdr_overflow;
	u32 hw_stuck;
	u32 dropped;
	u32 fcs_err;
	u32 xfr_hint_trig;
	u32 path_reset;
	u32 reset_counter;
} __attribute__ ((packed));

struct acx_dma_statistics {
	u32 rx_requested;
	u32 rx_errors;
	u32 tx_requested;
	u32 tx_errors;
}  __attribute__ ((packed));

struct acx_isr_statistics {
	
	u32 cmd_cmplt;

	
	u32 fiqs;

	
	u32 rx_headers;

	
	u32 rx_completes;

	
	u32 rx_mem_overflow;

	
	u32 rx_rdys;

	
	u32 irqs;

	
	u32 tx_procs;

	
	u32 decrypt_done;

	
	u32 dma0_done;

	
	u32 dma1_done;

	
	u32 tx_exch_complete;

	
	u32 commands;

	
	u32 rx_procs;

	
	u32 hw_pm_mode_changes;

	
	u32 host_acknowledges;

	
	u32 pci_pm;

	
	u32 wakeups;

	
	u32 low_rssi;
} __attribute__ ((packed));

struct acx_wep_statistics {
	
	u32 addr_key_count;

	
	u32 default_key_count;

	u32 reserved;

	
	u32 key_not_found;

	
	u32 decrypt_fail;

	
	u32 packets;

	
	u32 interrupt;
} __attribute__ ((packed));

#define ACX_MISSED_BEACONS_SPREAD 10

struct acx_pwr_statistics {
	
	u32 ps_enter;

	
	u32 elp_enter;

	
	u32 missing_bcns;

	
	u32 wake_on_host;

	
	u32 wake_on_timer_exp;

	
	u32 tx_with_ps;

	
	u32 tx_without_ps;

	
	u32 rcvd_beacons;

	
	u32 power_save_off;

	
	u16 enable_ps;

	
	u16 disable_ps;

	
	u32 fix_tsf_ps;

	
	u32 cont_miss_bcns_spread[ACX_MISSED_BEACONS_SPREAD];

	
	u32 rcvd_awake_beacons;
} __attribute__ ((packed));

struct acx_mic_statistics {
	u32 rx_pkts;
	u32 calc_failure;
} __attribute__ ((packed));

struct acx_aes_statistics {
	u32 encrypt_fail;
	u32 decrypt_fail;
	u32 encrypt_packets;
	u32 decrypt_packets;
	u32 encrypt_interrupt;
	u32 decrypt_interrupt;
} __attribute__ ((packed));

struct acx_event_statistics {
	u32 heart_beat;
	u32 calibration;
	u32 rx_mismatch;
	u32 rx_mem_empty;
	u32 rx_pool;
	u32 oom_late;
	u32 phy_transmit_error;
	u32 tx_stuck;
} __attribute__ ((packed));

struct acx_ps_statistics {
	u32 pspoll_timeouts;
	u32 upsd_timeouts;
	u32 upsd_max_sptime;
	u32 upsd_max_apturn;
	u32 pspoll_max_apturn;
	u32 pspoll_utilization;
	u32 upsd_utilization;
} __attribute__ ((packed));

struct acx_rxpipe_statistics {
	u32 rx_prep_beacon_drop;
	u32 descr_host_int_trig_rx_data;
	u32 beacon_buffer_thres_host_int_trig_rx_data;
	u32 missed_beacon_host_int_trig_rx_data;
	u32 tx_xfr_host_int_trig_rx_data;
} __attribute__ ((packed));

struct acx_statistics {
	struct acx_header header;

	struct acx_tx_statistics tx;
	struct acx_rx_statistics rx;
	struct acx_dma_statistics dma;
	struct acx_isr_statistics isr;
	struct acx_wep_statistics wep;
	struct acx_pwr_statistics pwr;
	struct acx_aes_statistics aes;
	struct acx_mic_statistics mic;
	struct acx_event_statistics event;
	struct acx_ps_statistics ps;
	struct acx_rxpipe_statistics rxpipe;
} __attribute__ ((packed));

#define ACX_MAX_RATE_CLASSES       8
#define ACX_RATE_MASK_UNSPECIFIED  0
#define ACX_RATE_MASK_ALL          0x1eff
#define ACX_RATE_RETRY_LIMIT       10

struct acx_rate_class {
	u32 enabled_rates;
	u8 short_retry_limit;
	u8 long_retry_limit;
	u8 aflags;
	u8 reserved;
};

struct acx_rate_policy {
	struct acx_header header;

	u32 rate_class_cnt;
	struct acx_rate_class rate_class[ACX_MAX_RATE_CLASSES];
} __attribute__ ((packed));

#define WL1271_ACX_AC_COUNT 4

struct acx_ac_cfg {
	struct acx_header header;
	u8 ac;
	u8 cw_min;
	u16 cw_max;
	u8 aifsn;
	u8 reserved;
	u16 tx_op_limit;
} __attribute__ ((packed));

enum wl1271_acx_ac {
	WL1271_ACX_AC_BE = 0,
	WL1271_ACX_AC_BK = 1,
	WL1271_ACX_AC_VI = 2,
	WL1271_ACX_AC_VO = 3,
	WL1271_ACX_AC_CTS2SELF = 4,
	WL1271_ACX_AC_ANY_TID = 0x1F,
	WL1271_ACX_AC_INVALID = 0xFF,
};

enum wl1271_acx_ps_scheme {
	WL1271_ACX_PS_SCHEME_LEGACY = 0,
	WL1271_ACX_PS_SCHEME_UPSD_TRIGGER = 1,
	WL1271_ACX_PS_SCHEME_LEGACY_PSPOLL = 2,
	WL1271_ACX_PS_SCHEME_SAPSD = 3,
};

enum wl1271_acx_ack_policy {
	WL1271_ACX_ACK_POLICY_LEGACY = 0,
	WL1271_ACX_ACK_POLICY_NO_ACK = 1,
	WL1271_ACX_ACK_POLICY_BLOCK = 2,
};

#define WL1271_ACX_TID_COUNT 7

struct acx_tid_config {
	struct acx_header header;
	u8 queue_id;
	u8 channel_type;
	u8 tsid;
	u8 ps_scheme;
	u8 ack_policy;
	u8 padding[3];
	u32 apsd_conf[2];
} __attribute__ ((packed));

struct acx_frag_threshold {
	struct acx_header header;
	u16 frag_threshold;
	u8 padding[2];
} __attribute__ ((packed));

#define WL1271_ACX_TX_COMPL_TIMEOUT   5
#define WL1271_ACX_TX_COMPL_THRESHOLD 5

struct acx_tx_config_options {
	struct acx_header header;
	u16 tx_compl_timeout;     
	u16 tx_compl_threshold;   
} __attribute__ ((packed));

#define ACX_RX_MEM_BLOCKS     64
#define ACX_TX_MIN_MEM_BLOCKS 64
#define ACX_TX_DESCRIPTORS    32
#define ACX_NUM_SSID_PROFILES 1

struct wl1271_acx_config_memory {
	struct acx_header header;

	u8 rx_mem_block_num;
	u8 tx_min_mem_block_num;
	u8 num_stations;
	u8 num_ssid_profiles;
	u32 total_tx_descriptors;
} __attribute__ ((packed));

struct wl1271_acx_mem_map {
	struct acx_header header;

	void *code_start;
	void *code_end;

	void *wep_defkey_start;
	void *wep_defkey_end;

	void *sta_table_start;
	void *sta_table_end;

	void *packet_template_start;
	void *packet_template_end;

	
	u32 tx_result;
	u32 tx_result_queue_start;

	void *queue_memory_start;
	void *queue_memory_end;

	u32 packet_memory_pool_start;
	u32 packet_memory_pool_end;

	void *debug_buffer1_start;
	void *debug_buffer1_end;

	void *debug_buffer2_start;
	void *debug_buffer2_end;

	
	u32 num_tx_mem_blocks;

	
	u32 num_rx_mem_blocks;

	
	u8 *tx_cbuf;
	u8 *rx_cbuf;
	void *rx_ctrl;
	void *tx_ctrl;
} __attribute__ ((packed));

enum wl1271_acx_rx_queue_type {
	RX_QUEUE_TYPE_RX_LOW_PRIORITY,    
	RX_QUEUE_TYPE_RX_HIGH_PRIORITY,   
	RX_QUEUE_TYPE_NUM,
	RX_QUEUE_TYPE_MAX = USHORT_MAX
};

#define WL1271_RX_INTR_THRESHOLD_DEF  0       
#define WL1271_RX_INTR_THRESHOLD_MIN  0
#define WL1271_RX_INTR_THRESHOLD_MAX  15

#define WL1271_RX_INTR_TIMEOUT_DEF    5
#define WL1271_RX_INTR_TIMEOUT_MIN    1
#define WL1271_RX_INTR_TIMEOUT_MAX    100

struct wl1271_acx_rx_config_opt {
	struct acx_header header;

	u16 mblk_threshold;
	u16 threshold;
	u16 timeout;
	u8 queue_type;
	u8 reserved;
} __attribute__ ((packed));

enum {
	ACX_WAKE_UP_CONDITIONS      = 0x0002,
	ACX_MEM_CFG                 = 0x0003,
	ACX_SLOT                    = 0x0004,
	ACX_AC_CFG                  = 0x0007,
	ACX_MEM_MAP                 = 0x0008,
	ACX_AID                     = 0x000A,
	
	ACX_FW_REV                  = 0x000D,
	ACX_MEDIUM_USAGE            = 0x000F,
	ACX_RX_CFG                  = 0x0010,
	ACX_TX_QUEUE_CFG            = 0x0011, 
	ACX_STATISTICS              = 0x0013, 
	ACX_PWR_CONSUMPTION_STATISTICS = 0x0014,
	ACX_FEATURE_CFG             = 0x0015,
	ACX_TID_CFG                 = 0x001A,
	ACX_PS_RX_STREAMING         = 0x001B,
	ACX_BEACON_FILTER_OPT       = 0x001F,
	ACX_NOISE_HIST              = 0x0021,
	ACX_HDK_VERSION             = 0x0022, 
	ACX_PD_THRESHOLD            = 0x0023,
	ACX_TX_CONFIG_OPT           = 0x0024,
	ACX_CCA_THRESHOLD           = 0x0025,
	ACX_EVENT_MBOX_MASK         = 0x0026,
	ACX_CONN_MONIT_PARAMS       = 0x002D,
	ACX_CONS_TX_FAILURE         = 0x002F,
	ACX_BCN_DTIM_OPTIONS        = 0x0031,
	ACX_SG_ENABLE               = 0x0032,
	ACX_SG_CFG                  = 0x0033,
	ACX_BEACON_FILTER_TABLE     = 0x0038,
	ACX_ARP_IP_FILTER           = 0x0039,
	ACX_ROAMING_STATISTICS_TBL  = 0x003B,
	ACX_RATE_POLICY             = 0x003D,
	ACX_CTS_PROTECTION          = 0x003E,
	ACX_SLEEP_AUTH              = 0x003F,
	ACX_PREAMBLE_TYPE	    = 0x0040,
	ACX_ERROR_CNT               = 0x0041,
	ACX_IBSS_FILTER		    = 0x0044,
	ACX_SERVICE_PERIOD_TIMEOUT  = 0x0045,
	ACX_TSF_INFO                = 0x0046,
	ACX_CONFIG_PS_WMM           = 0x0049,
	ACX_ENABLE_RX_DATA_FILTER   = 0x004A,
	ACX_SET_RX_DATA_FILTER      = 0x004B,
	ACX_GET_DATA_FILTER_STATISTICS = 0x004C,
	ACX_RX_CONFIG_OPT           = 0x004E,
	ACX_FRAG_CFG                = 0x004F,
	ACX_BET_ENABLE              = 0x0050,
	ACX_RSSI_SNR_TRIGGER        = 0x0051,
	ACX_RSSI_SNR_WEIGHTS        = 0x0051,
	ACX_KEEP_ALIVE_MODE         = 0x0052,
	ACX_SET_KEEP_ALIVE_CONFIG   = 0x0054,
	ACX_BA_SESSION_RESPONDER_POLICY = 0x0055,
	ACX_BA_SESSION_INITIATOR_POLICY = 0x0056,
	ACX_PEER_HT_CAP             = 0x0057,
	ACX_HT_BSS_OPERATION        = 0x0058,
	ACX_COEX_ACTIVITY           = 0x0059,
	DOT11_RX_MSDU_LIFE_TIME     = 0x1004,
	DOT11_CUR_TX_PWR            = 0x100D,
	DOT11_RX_DOT11_MODE         = 0x1012,
	DOT11_RTS_THRESHOLD         = 0x1013,
	DOT11_GROUP_ADDRESS_TBL     = 0x1014,

	MAX_DOT11_IE = DOT11_GROUP_ADDRESS_TBL,

	MAX_IE = 0xFFFF
};


int wl1271_acx_wake_up_conditions(struct wl1271 *wl, u8 wake_up_event,
				  u8 listen_interval);
int wl1271_acx_sleep_auth(struct wl1271 *wl, u8 sleep_auth);
int wl1271_acx_fw_version(struct wl1271 *wl, char *buf, size_t len);
int wl1271_acx_tx_power(struct wl1271 *wl, int power);
int wl1271_acx_feature_cfg(struct wl1271 *wl);
int wl1271_acx_mem_map(struct wl1271 *wl,
		       struct acx_header *mem_map, size_t len);
int wl1271_acx_rx_msdu_life_time(struct wl1271 *wl, u32 life_time);
int wl1271_acx_rx_config(struct wl1271 *wl, u32 config, u32 filter);
int wl1271_acx_pd_threshold(struct wl1271 *wl);
int wl1271_acx_slot(struct wl1271 *wl, enum acx_slot_type slot_time);
int wl1271_acx_group_address_tbl(struct wl1271 *wl);
int wl1271_acx_service_period_timeout(struct wl1271 *wl);
int wl1271_acx_rts_threshold(struct wl1271 *wl, u16 rts_threshold);
int wl1271_acx_beacon_filter_opt(struct wl1271 *wl);
int wl1271_acx_beacon_filter_table(struct wl1271 *wl);
int wl1271_acx_sg_enable(struct wl1271 *wl);
int wl1271_acx_sg_cfg(struct wl1271 *wl);
int wl1271_acx_cca_threshold(struct wl1271 *wl);
int wl1271_acx_bcn_dtim_options(struct wl1271 *wl);
int wl1271_acx_aid(struct wl1271 *wl, u16 aid);
int wl1271_acx_event_mbox_mask(struct wl1271 *wl, u32 event_mask);
int wl1271_acx_set_preamble(struct wl1271 *wl, enum acx_preamble_type preamble);
int wl1271_acx_cts_protect(struct wl1271 *wl,
			    enum acx_ctsprotect_type ctsprotect);
int wl1271_acx_statistics(struct wl1271 *wl, struct acx_statistics *stats);
int wl1271_acx_rate_policies(struct wl1271 *wl);
int wl1271_acx_ac_cfg(struct wl1271 *wl);
int wl1271_acx_tid_cfg(struct wl1271 *wl);
int wl1271_acx_frag_threshold(struct wl1271 *wl);
int wl1271_acx_tx_config_options(struct wl1271 *wl);
int wl1271_acx_mem_cfg(struct wl1271 *wl);
int wl1271_acx_init_mem_config(struct wl1271 *wl);
int wl1271_acx_init_rx_interrupt(struct wl1271 *wl);

#endif 
