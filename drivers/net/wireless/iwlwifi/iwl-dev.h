


#ifndef __iwl_dev_h__
#define __iwl_dev_h__

#include <linux/pci.h> 
#include <linux/kernel.h>
#include <net/ieee80211_radiotap.h>

#include "iwl-eeprom.h"
#include "iwl-csr.h"
#include "iwl-prph.h"
#include "iwl-fh.h"
#include "iwl-debug.h"
#include "iwl-4965-hw.h"
#include "iwl-3945-hw.h"
#include "iwl-3945-led.h"
#include "iwl-led.h"
#include "iwl-power.h"
#include "iwl-agn-rs.h"


extern struct iwl_cfg iwl4965_agn_cfg;
extern struct iwl_cfg iwl5300_agn_cfg;
extern struct iwl_cfg iwl5100_agn_cfg;
extern struct iwl_cfg iwl5350_agn_cfg;
extern struct iwl_cfg iwl5100_bg_cfg;
extern struct iwl_cfg iwl5100_abg_cfg;
extern struct iwl_cfg iwl5150_agn_cfg;
extern struct iwl_cfg iwl6000h_2agn_cfg;
extern struct iwl_cfg iwl6000i_2agn_cfg;
extern struct iwl_cfg iwl6000_3agn_cfg;
extern struct iwl_cfg iwl6050_2agn_cfg;
extern struct iwl_cfg iwl6050_3agn_cfg;
extern struct iwl_cfg iwl1000_bgn_cfg;

struct iwl_tx_queue;


extern struct iwl_mod_params iwl50_mod_params;
extern struct iwl_ops iwl5000_ops;
extern struct iwl_ucode_ops iwl5000_ucode;
extern struct iwl_lib_ops iwl5000_lib;
extern struct iwl_hcmd_ops iwl5000_hcmd;
extern struct iwl_hcmd_utils_ops iwl5000_hcmd_utils;


extern u16 iwl5000_get_hcmd_size(u8 cmd_id, u16 len);
extern u16 iwl5000_build_addsta_hcmd(const struct iwl_addsta_cmd *cmd,
				     u8 *data);
extern void iwl5000_rts_tx_cmd_flag(struct ieee80211_tx_info *info,
				    __le32 *tx_flags);
extern int iwl5000_calc_rssi(struct iwl_priv *priv,
			     struct iwl_rx_phy_res *rx_resp);
extern int iwl5000_apm_init(struct iwl_priv *priv);
extern void iwl5000_apm_stop(struct iwl_priv *priv);
extern int iwl5000_apm_reset(struct iwl_priv *priv);
extern void iwl5000_nic_config(struct iwl_priv *priv);
extern u16 iwl5000_eeprom_calib_version(struct iwl_priv *priv);
extern const u8 *iwl5000_eeprom_query_addr(const struct iwl_priv *priv,
				    size_t offset);
extern void iwl5000_txq_update_byte_cnt_tbl(struct iwl_priv *priv,
					    struct iwl_tx_queue *txq,
					    u16 byte_cnt);
extern void iwl5000_txq_inval_byte_cnt_tbl(struct iwl_priv *priv,
				    struct iwl_tx_queue *txq);
extern int iwl5000_load_ucode(struct iwl_priv *priv);
extern void iwl5000_init_alive_start(struct iwl_priv *priv);
extern int iwl5000_alive_notify(struct iwl_priv *priv);
extern int iwl5000_hw_set_hw_params(struct iwl_priv *priv);
extern int iwl5000_txq_agg_enable(struct iwl_priv *priv, int txq_id,
			   int tx_fifo, int sta_id, int tid, u16 ssn_idx);
extern int iwl5000_txq_agg_disable(struct iwl_priv *priv, u16 txq_id,
			    u16 ssn_idx, u8 tx_fifo);
extern void iwl5000_txq_set_sched(struct iwl_priv *priv, u32 mask);
extern void iwl5000_setup_deferred_work(struct iwl_priv *priv);
extern void iwl5000_rx_handler_setup(struct iwl_priv *priv);
extern int iwl5000_hw_valid_rtc_data_addr(u32 addr);
extern int iwl5000_send_tx_power(struct iwl_priv *priv);
extern void iwl5000_temperature(struct iwl_priv *priv);


#define CT_KILL_THRESHOLD_LEGACY   110 
#define CT_KILL_THRESHOLD	   114 
#define CT_KILL_EXIT_THRESHOLD     95  


#define IWL_NOISE_MEAS_NOT_AVAILABLE (-127)


#define DEFAULT_RTS_THRESHOLD     2347U
#define MIN_RTS_THRESHOLD         0U
#define MAX_RTS_THRESHOLD         2347U
#define MAX_MSDU_SIZE		  2304U
#define MAX_MPDU_SIZE		  2346U
#define DEFAULT_BEACON_INTERVAL   100U
#define	DEFAULT_SHORT_RETRY_LIMIT 7U
#define	DEFAULT_LONG_RETRY_LIMIT  4U

struct iwl_rx_mem_buffer {
	dma_addr_t real_dma_addr;
	dma_addr_t aligned_dma_addr;
	struct sk_buff *skb;
	struct list_head list;
};


struct iwl_device_cmd;

struct iwl_cmd_meta {
	
	struct iwl_host_cmd *source;
	
	void (*callback)(struct iwl_priv *priv,
			 struct iwl_device_cmd *cmd,
			 struct sk_buff *skb);

	
	u32 flags;

	DECLARE_PCI_UNMAP_ADDR(mapping)
	DECLARE_PCI_UNMAP_LEN(len)
};


struct iwl_queue {
	int n_bd;              
	int write_ptr;       
	int read_ptr;         
	dma_addr_t dma_addr;   
	int n_window;	       
	u32 id;
	int low_mark;	       
	int high_mark;         
} __attribute__ ((packed));


struct iwl_tx_info {
	struct sk_buff *skb[IWL_NUM_OF_TBS - 1];
};


#define TFD_TX_CMD_SLOTS 256
#define TFD_CMD_SLOTS 32

struct iwl_tx_queue {
	struct iwl_queue q;
	void *tfds;
	struct iwl_device_cmd **cmd;
	struct iwl_cmd_meta *meta;
	struct iwl_tx_info *txb;
	u8 need_update;
	u8 sched_retry;
	u8 active;
	u8 swq_id;
};

#define IWL_NUM_SCAN_RATES         (2)

struct iwl4965_channel_tgd_info {
	u8 type;
	s8 max_power;
};

struct iwl4965_channel_tgh_info {
	s64 last_radar_time;
};

#define IWL4965_MAX_RATE (33)

struct iwl3945_clip_group {
	
	const s8 clip_powers[IWL_MAX_RATES];
};


struct iwl3945_channel_power_info {
	struct iwl3945_tx_power tpc;	
	s8 power_table_index;	
	s8 base_power_index;	
	s8 requested_power;	
};


struct iwl3945_scan_power_info {
	struct iwl3945_tx_power tpc;	
	s8 power_table_index;	
	s8 requested_power;	
};


struct iwl_channel_info {
	struct iwl4965_channel_tgd_info tgd;
	struct iwl4965_channel_tgh_info tgh;
	struct iwl_eeprom_channel eeprom;	
	struct iwl_eeprom_channel ht40_eeprom;	

	u8 channel;	  
	u8 flags;	  
	s8 max_power_avg; 
	s8 curr_txpow;	  
	s8 min_power;	  
	s8 scan_power;	  

	u8 group_index;	  
	u8 band_index;	  
	enum ieee80211_band band;

	
	s8 ht40_max_power_avg;	
	s8 ht40_curr_txpow;	
	s8 ht40_min_power;	
	s8 ht40_scan_power;	
	u8 ht40_flags;		
	u8 ht40_extension_channel; 

	
	struct iwl3945_channel_power_info power_info[IWL4965_MAX_RATE];

	
	struct iwl3945_scan_power_info scan_pwr_info[IWL_NUM_SCAN_RATES];
};

#define IWL_TX_FIFO_AC0	0
#define IWL_TX_FIFO_AC1	1
#define IWL_TX_FIFO_AC2	2
#define IWL_TX_FIFO_AC3	3
#define IWL_TX_FIFO_HCCA_1	5
#define IWL_TX_FIFO_HCCA_2	6
#define IWL_TX_FIFO_NONE	7


#define IWL_MIN_NUM_QUEUES	10



enum iwl_pwr_src {
	IWL_PWR_SRC_VMAIN,
	IWL_PWR_SRC_VAUX,
};

#define IEEE80211_DATA_LEN              2304
#define IEEE80211_4ADDR_LEN             30
#define IEEE80211_HLEN                  (IEEE80211_4ADDR_LEN)
#define IEEE80211_FRAME_LEN             (IEEE80211_DATA_LEN + IEEE80211_HLEN)

struct iwl_frame {
	union {
		struct ieee80211_hdr frame;
		struct iwl_tx_beacon_cmd beacon;
		u8 raw[IEEE80211_FRAME_LEN];
		u8 cmd[360];
	} u;
	struct list_head list;
};

#define SEQ_TO_SN(seq) (((seq) & IEEE80211_SCTL_SEQ) >> 4)
#define SN_TO_SEQ(ssn) (((ssn) << 4) & IEEE80211_SCTL_SEQ)
#define MAX_SN ((IEEE80211_SCTL_SEQ) >> 4)

enum {
	CMD_SYNC = 0,
	CMD_SIZE_NORMAL = 0,
	CMD_NO_SKB = 0,
	CMD_SIZE_HUGE = (1 << 0),
	CMD_ASYNC = (1 << 1),
	CMD_WANT_SKB = (1 << 2),
};

#define IWL_CMD_MAX_PAYLOAD 320


struct iwl_device_cmd {
	struct iwl_cmd_header hdr;	
	union {
		u32 flags;
		u8 val8;
		u16 val16;
		u32 val32;
		struct iwl_tx_cmd tx;
		u8 payload[IWL_CMD_MAX_PAYLOAD];
	} __attribute__ ((packed)) cmd;
} __attribute__ ((packed));

#define TFD_MAX_PAYLOAD_SIZE (sizeof(struct iwl_device_cmd))


struct iwl_host_cmd {
	const void *data;
	struct sk_buff *reply_skb;
	void (*callback)(struct iwl_priv *priv,
			 struct iwl_device_cmd *cmd,
			 struct sk_buff *skb);
	u32 flags;
	u16 len;
	u8 id;
};


#define RX_FREE_BUFFERS 64
#define RX_LOW_WATERMARK 8

#define SUP_RATE_11A_MAX_NUM_CHANNELS  8
#define SUP_RATE_11B_MAX_NUM_CHANNELS  4
#define SUP_RATE_11G_MAX_NUM_CHANNELS  12


struct iwl_rx_queue {
	__le32 *bd;
	dma_addr_t dma_addr;
	struct iwl_rx_mem_buffer pool[RX_QUEUE_SIZE + RX_FREE_BUFFERS];
	struct iwl_rx_mem_buffer *queue[RX_QUEUE_SIZE];
	u32 read;
	u32 write;
	u32 free_count;
	u32 write_actual;
	struct list_head rx_free;
	struct list_head rx_used;
	int need_update;
	struct iwl_rb_status *rb_stts;
	dma_addr_t rb_stts_dma;
	spinlock_t lock;
};

#define IWL_SUPPORTED_RATES_IE_LEN         8

#define MAX_TID_COUNT        9

#define IWL_INVALID_RATE     0xFF
#define IWL_INVALID_VALUE    -1


struct iwl_ht_agg {
	u16 txq_id;
	u16 frame_count;
	u16 wait_for_ba;
	u16 start_idx;
	u64 bitmap;
	u32 rate_n_flags;
#define IWL_AGG_OFF 0
#define IWL_AGG_ON 1
#define IWL_EMPTYING_HW_QUEUE_ADDBA 2
#define IWL_EMPTYING_HW_QUEUE_DELBA 3
	u8 state;
};


struct iwl_tid_data {
	u16 seq_number;
	u16 tfds_in_queue;
	struct iwl_ht_agg agg;
};

struct iwl_hw_key {
	enum ieee80211_key_alg alg;
	int keylen;
	u8 keyidx;
	u8 key[32];
};

union iwl_ht_rate_supp {
	u16 rates;
	struct {
		u8 siso_rate;
		u8 mimo_rate;
	};
};

#define CFG_HT_RX_AMPDU_FACTOR_DEF  (0x3)


#define CFG_HT_MPDU_DENSITY_4USEC   (0x5)
#define CFG_HT_MPDU_DENSITY_DEF CFG_HT_MPDU_DENSITY_4USEC

struct iwl_ht_info {
	
	u8 is_ht;
	u8 supported_chan_width;
	u8 sm_ps;
	struct ieee80211_mcs_info mcs;
	
	u8 extension_chan_offset;
	u8 ht_protection;
	u8 non_GF_STA_present;
};

union iwl_qos_capabity {
	struct {
		u8 edca_count:4;	
		u8 q_ack:1;		
		u8 queue_request:1;	
		u8 txop_request:1;	
		u8 reserved:1;		
	} q_AP;
	struct {
		u8 acvo_APSD:1;		
		u8 acvi_APSD:1;		
		u8 ac_bk_APSD:1;	
		u8 ac_be_APSD:1;	
		u8 q_ack:1;		
		u8 max_len:2;		
		u8 more_data_ack:1;	
	} q_STA;
	u8 val;
};


struct iwl_qos_info {
	int qos_active;
	union iwl_qos_capabity qos_cap;
	struct iwl_qosparam_cmd def_qos_parm;
};

#define STA_PS_STATUS_WAKE             0
#define STA_PS_STATUS_SLEEP            1


struct iwl3945_station_entry {
	struct iwl3945_addsta_cmd sta;
	struct iwl_tid_data tid[MAX_TID_COUNT];
	u8 used;
	u8 ps_status;
	struct iwl_hw_key keyinfo;
};

struct iwl_station_entry {
	struct iwl_addsta_cmd sta;
	struct iwl_tid_data tid[MAX_TID_COUNT];
	u8 used;
	u8 ps_status;
	struct iwl_hw_key keyinfo;
};


struct fw_desc {
	void *v_addr;		
	dma_addr_t p_addr;	
	u32 len;		
};


struct iwl_ucode_header {
	__le32 ver;	
	union {
		struct {
			__le32 inst_size;	
			__le32 data_size;	
			__le32 init_size;	
			__le32 init_data_size;	
			__le32 boot_size;	
			u8 data[0];		
		} v1;
		struct {
			__le32 build;		
			__le32 inst_size;	
			__le32 data_size;	
			__le32 init_size;	
			__le32 init_data_size;	
			__le32 boot_size;	
			u8 data[0];		
		} v2;
	} u;
};
#define UCODE_HEADER_SIZE(ver) ((ver) == 1 ? 24 : 28)

struct iwl4965_ibss_seq {
	u8 mac[ETH_ALEN];
	u16 seq_num;
	u16 frag_num;
	unsigned long packet_time;
	struct list_head list;
};

struct iwl_sensitivity_ranges {
	u16 min_nrg_cck;
	u16 max_nrg_cck;

	u16 nrg_th_cck;
	u16 nrg_th_ofdm;

	u16 auto_corr_min_ofdm;
	u16 auto_corr_min_ofdm_mrc;
	u16 auto_corr_min_ofdm_x1;
	u16 auto_corr_min_ofdm_mrc_x1;

	u16 auto_corr_max_ofdm;
	u16 auto_corr_max_ofdm_mrc;
	u16 auto_corr_max_ofdm_x1;
	u16 auto_corr_max_ofdm_mrc_x1;

	u16 auto_corr_max_cck;
	u16 auto_corr_max_cck_mrc;
	u16 auto_corr_min_cck;
	u16 auto_corr_min_cck_mrc;
};


#define KELVIN_TO_CELSIUS(x) ((x)-273)
#define CELSIUS_TO_KELVIN(x) ((x)+273)



struct iwl_hw_params {
	u8 max_txq_num;
	u8 dma_chnl_num;
	u16 scd_bc_tbls_size;
	u32 tfd_size;
	u8  tx_chains_num;
	u8  rx_chains_num;
	u8  valid_tx_ant;
	u8  valid_rx_ant;
	u16 max_rxq_size;
	u16 max_rxq_log;
	u32 rx_buf_size;
	u32 rx_wrt_ptr_reg;
	u32 max_pkt_size;
	u8  max_stations;
	u8  bcast_sta_id;
	u8  ht40_channel;
	u8  max_beacon_itrvl;	
	u32 max_inst_size;
	u32 max_data_size;
	u32 max_bsm_size;
	u32 ct_kill_threshold; 
	u32 ct_kill_exit_threshold; 
				    
	u32 calib_init_cfg;
	const struct iwl_sensitivity_ranges *sens;
};



extern void iwl_update_chain_flags(struct iwl_priv *priv);
extern int iwl_set_pwr_src(struct iwl_priv *priv, enum iwl_pwr_src src);
extern const u8 iwl_bcast_addr[ETH_ALEN];
extern int iwl_rxq_stop(struct iwl_priv *priv);
extern void iwl_txq_ctx_stop(struct iwl_priv *priv);
extern int iwl_queue_space(const struct iwl_queue *q);
static inline int iwl_queue_used(const struct iwl_queue *q, int i)
{
	return q->write_ptr >= q->read_ptr ?
		(i >= q->read_ptr && i < q->write_ptr) :
		!(i < q->read_ptr && i >= q->write_ptr);
}


static inline u8 get_cmd_index(struct iwl_queue *q, u32 index, int is_huge)
{
	
	if (is_huge)
		return q->n_window;	

	
	return index & (q->n_window - 1);
}


struct iwl_dma_ptr {
	dma_addr_t dma;
	void *addr;
	size_t size;
};

#define IWL_CHANNEL_WIDTH_20MHZ   0
#define IWL_CHANNEL_WIDTH_40MHZ   1

#define IWL_OPERATION_MODE_AUTO     0
#define IWL_OPERATION_MODE_HT_ONLY  1
#define IWL_OPERATION_MODE_MIXED    2
#define IWL_OPERATION_MODE_20MHZ    3

#define IWL_TX_CRC_SIZE 4
#define IWL_TX_DELIMITER_SIZE 4

#define TX_POWER_IWL_ILLEGAL_VOLTAGE -10000


#define INITIALIZATION_VALUE		0xFFFF
#define CAL_NUM_OF_BEACONS		20
#define MAXIMUM_ALLOWED_PATHLOSS	15

#define CHAIN_NOISE_MAX_DELTA_GAIN_CODE 3

#define MAX_FA_OFDM  50
#define MIN_FA_OFDM  5
#define MAX_FA_CCK   50
#define MIN_FA_CCK   5

#define AUTO_CORR_STEP_OFDM       1

#define AUTO_CORR_STEP_CCK     3
#define AUTO_CORR_MAX_TH_CCK   160

#define NRG_DIFF               2
#define NRG_STEP_CCK           2
#define NRG_MARGIN             8
#define MAX_NUMBER_CCK_NO_FA 100

#define AUTO_CORR_CCK_MIN_VAL_DEF    (125)

#define CHAIN_A             0
#define CHAIN_B             1
#define CHAIN_C             2
#define CHAIN_NOISE_DELTA_GAIN_INIT_VAL 4
#define ALL_BAND_FILTER			0xFF00
#define IN_BAND_FILTER			0xFF
#define MIN_AVERAGE_NOISE_MAX_VALUE	0xFFFFFFFF

#define NRG_NUM_PREV_STAT_L     20
#define NUM_RX_CHAINS           3

enum iwl4965_false_alarm_state {
	IWL_FA_TOO_MANY = 0,
	IWL_FA_TOO_FEW = 1,
	IWL_FA_GOOD_RANGE = 2,
};

enum iwl4965_chain_noise_state {
	IWL_CHAIN_NOISE_ALIVE = 0,  
	IWL_CHAIN_NOISE_ACCUMULATE,
	IWL_CHAIN_NOISE_CALIBRATED,
	IWL_CHAIN_NOISE_DONE,
};

enum iwl4965_calib_enabled_state {
	IWL_CALIB_DISABLED = 0,  
	IWL_CALIB_ENABLED = 1,
};



enum iwl_calib {
	IWL_CALIB_XTAL,
	IWL_CALIB_DC,
	IWL_CALIB_LO,
	IWL_CALIB_TX_IQ,
	IWL_CALIB_TX_IQ_PERD,
	IWL_CALIB_BASE_BAND,
	IWL_CALIB_MAX
};


struct iwl_calib_result {
	void *buf;
	size_t buf_len;
};

enum ucode_type {
	UCODE_NONE = 0,
	UCODE_INIT,
	UCODE_RT
};


struct iwl_sensitivity_data {
	u32 auto_corr_ofdm;
	u32 auto_corr_ofdm_mrc;
	u32 auto_corr_ofdm_x1;
	u32 auto_corr_ofdm_mrc_x1;
	u32 auto_corr_cck;
	u32 auto_corr_cck_mrc;

	u32 last_bad_plcp_cnt_ofdm;
	u32 last_fa_cnt_ofdm;
	u32 last_bad_plcp_cnt_cck;
	u32 last_fa_cnt_cck;

	u32 nrg_curr_state;
	u32 nrg_prev_state;
	u32 nrg_value[10];
	u8  nrg_silence_rssi[NRG_NUM_PREV_STAT_L];
	u32 nrg_silence_ref;
	u32 nrg_energy_idx;
	u32 nrg_silence_idx;
	u32 nrg_th_cck;
	s32 nrg_auto_corr_silence_diff;
	u32 num_in_cck_no_fa;
	u32 nrg_th_ofdm;
};


struct iwl_chain_noise_data {
	u32 active_chains;
	u32 chain_noise_a;
	u32 chain_noise_b;
	u32 chain_noise_c;
	u32 chain_signal_a;
	u32 chain_signal_b;
	u32 chain_signal_c;
	u16 beacon_count;
	u8 disconn_array[NUM_RX_CHAINS];
	u8 delta_gain_code[NUM_RX_CHAINS];
	u8 radio_write;
	u8 state;
};

#define	EEPROM_SEM_TIMEOUT 10		
#define EEPROM_SEM_RETRY_LIMIT 1000	

#define IWL_TRAFFIC_ENTRIES	(256)
#define IWL_TRAFFIC_ENTRY_SIZE  (64)

enum {
	MEASUREMENT_READY = (1 << 0),
	MEASUREMENT_ACTIVE = (1 << 1),
};

enum iwl_nvm_type {
	NVM_DEVICE_TYPE_EEPROM = 0,
	NVM_DEVICE_TYPE_OTP,
};


enum iwl_access_mode {
	IWL_OTP_ACCESS_ABSOLUTE,
	IWL_OTP_ACCESS_RELATIVE,
};


enum iwl_pa_type {
	IWL_PA_SYSTEM = 0,
	IWL_PA_HYBRID = 1,
	IWL_PA_INTERNAL = 2,
};


struct isr_statistics {
	u32 hw;
	u32 sw;
	u32 sw_err;
	u32 sch;
	u32 alive;
	u32 rfkill;
	u32 ctkill;
	u32 wakeup;
	u32 rx;
	u32 rx_handlers[REPLY_MAX];
	u32 tx;
	u32 unhandled;
};

#ifdef CONFIG_IWLWIFI_DEBUGFS

enum iwl_mgmt_stats {
	MANAGEMENT_ASSOC_REQ = 0,
	MANAGEMENT_ASSOC_RESP,
	MANAGEMENT_REASSOC_REQ,
	MANAGEMENT_REASSOC_RESP,
	MANAGEMENT_PROBE_REQ,
	MANAGEMENT_PROBE_RESP,
	MANAGEMENT_BEACON,
	MANAGEMENT_ATIM,
	MANAGEMENT_DISASSOC,
	MANAGEMENT_AUTH,
	MANAGEMENT_DEAUTH,
	MANAGEMENT_ACTION,
	MANAGEMENT_MAX,
};

enum iwl_ctrl_stats {
	CONTROL_BACK_REQ =  0,
	CONTROL_BACK,
	CONTROL_PSPOLL,
	CONTROL_RTS,
	CONTROL_CTS,
	CONTROL_ACK,
	CONTROL_CFEND,
	CONTROL_CFENDACK,
	CONTROL_MAX,
};

struct traffic_stats {
	u32 mgmt[MANAGEMENT_MAX];
	u32 ctrl[CONTROL_MAX];
	u32 data_cnt;
	u64 data_bytes;
};
#else
struct traffic_stats {
	u64 data_bytes;
};
#endif

#define IWL_MAX_NUM_QUEUES	20 

struct iwl_priv {

	
	struct ieee80211_hw *hw;
	struct ieee80211_channel *ieee_channels;
	struct ieee80211_rate *ieee_rates;
	struct iwl_cfg *cfg;

	
	struct list_head free_frames;
	int frames_count;

	enum ieee80211_band band;
	int alloc_rxb_skb;

	void (*rx_handlers[REPLY_MAX])(struct iwl_priv *priv,
				       struct iwl_rx_mem_buffer *rxb);

	struct ieee80211_supported_band bands[IEEE80211_NUM_BANDS];

#if defined(CONFIG_IWLWIFI_SPECTRUM_MEASUREMENT) || defined(CONFIG_IWL3945_SPECTRUM_MEASUREMENT)
	
	struct iwl_spectrum_notification measure_report;
	u8 measurement_status;
#endif
	
	u32 ucode_beacon_time;

	
	struct iwl_channel_info *channel_info;	
	u8 channel_count;	

	
	const struct iwl3945_clip_group clip39_groups[5];

	
	s32 temperature;	
	s32 last_temperature;

	
	struct iwl_calib_result calib_results[IWL_CALIB_MAX];

	
	unsigned long last_scan_jiffies;
	unsigned long next_scan_jiffies;
	unsigned long scan_start;
	unsigned long scan_pass_start;
	unsigned long scan_start_tsf;
	void *scan;
	int scan_bands;
	struct cfg80211_scan_request *scan_request;
	u8 scan_tx_ant[IEEE80211_NUM_BANDS];
	u8 mgmt_tx_ant;

	
	spinlock_t lock;	
	spinlock_t hcmd_lock;	
	spinlock_t reg_lock;	
	struct mutex mutex;

	
	struct pci_dev *pci_dev;

	
	void __iomem *hw_base;
	u32  hw_rev;
	u32  hw_wa_rev;
	u8   rev_id;

	
	u32 ucode_ver;			
	struct fw_desc ucode_code;	
	struct fw_desc ucode_data;	
	struct fw_desc ucode_data_backup;	
	struct fw_desc ucode_init;	
	struct fw_desc ucode_init_data;	
	struct fw_desc ucode_boot;	
	enum ucode_type ucode_type;
	u8 ucode_write_complete;	


	struct iwl_rxon_time_cmd rxon_timing;

	
	const struct iwl_rxon_cmd active_rxon;
	struct iwl_rxon_cmd staging_rxon;

	struct iwl_rxon_cmd recovery_rxon;

	
	struct iwl_init_alive_resp card_alive_init;
	struct iwl_alive_resp card_alive;

#ifdef CONFIG_IWLWIFI_LEDS
	unsigned long last_blink_time;
	u8 last_blink_rate;
	u8 allow_blinking;
	u64 led_tpt;
	struct iwl_led led[IWL_LED_TRG_MAX];
	unsigned int rxtxpackets;
#endif
	u16 active_rate;
	u16 active_rate_basic;

	u8 assoc_station_added;
	u8 start_calib;
	struct iwl_sensitivity_data sensitivity_data;
	struct iwl_chain_noise_data chain_noise_data;
	__le16 sensitivity_tbl[HD_TABLE_SIZE];

	struct iwl_ht_info current_ht_config;
	u8 last_phy_res[100];

	
	s8 data_retry_limit;
	u8 retry_rate;

	wait_queue_head_t wait_command_queue;

	int activity_timer_active;

	
	struct iwl_rx_queue rxq;
	struct iwl_tx_queue txq[IWL_MAX_NUM_QUEUES];
	unsigned long txq_ctx_active_msk;
	struct iwl_dma_ptr  kw;	
	struct iwl_dma_ptr  scd_bc_tbls;

	u32 scd_base_addr;	

	unsigned long status;

	int last_rx_rssi;	
	int last_rx_noise;	

	
	struct traffic_stats tx_stats;
	struct traffic_stats rx_stats;

	
	struct isr_statistics isr_stats;

	struct iwl_power_mgr power_data;
	struct iwl_tt_mgmt thermal_throttle;

	struct iwl_notif_statistics statistics;
	unsigned long last_statistics_time;

	
	u16 rates_mask;

	u8 bssid[ETH_ALEN];
	u16 rts_threshold;
	u8 mac_addr[ETH_ALEN];

	
	spinlock_t sta_lock;
	int num_stations;
	struct iwl_station_entry stations[IWL_STATION_COUNT];
	struct iwl_wep_key wep_keys[WEP_KEYS_MAX];
	u8 default_wep_key;
	u8 key_mapping_key;
	unsigned long ucode_key_table;

	
#define IWL_MAX_HW_QUEUES	32
	unsigned long queue_stopped[BITS_TO_LONGS(IWL_MAX_HW_QUEUES)];
	
	atomic_t queue_stop_count[4];

	
	u8 is_open;

	u8 mac80211_registered;

	
	u32 last_beacon_time;
	u64 last_tsf;

	
	u8 *eeprom;
	int    nvm_device_type;
	struct iwl_eeprom_calib_info *calib_info;

	enum nl80211_iftype iw_mode;

	struct sk_buff *ibss_beacon;

	
	u64 timestamp;
	u16 beacon_int;
	struct ieee80211_vif *vif;

	
	void *shared_virt;
	dma_addr_t shared_phys;
	
	struct iwl_hw_params hw_params;

	
	__le32 *ict_tbl;
	dma_addr_t ict_tbl_dma;
	dma_addr_t aligned_ict_tbl_dma;
	int ict_index;
	void *ict_tbl_vir;
	u32 inta;
	bool use_ict;

	u32 inta_mask;
	
	u16 assoc_id;
	u16 assoc_capability;

	struct iwl_qos_info qos_data;

	struct workqueue_struct *workqueue;

	struct work_struct up;
	struct work_struct restart;
	struct work_struct calibrated_work;
	struct work_struct scan_completed;
	struct work_struct rx_replenish;
	struct work_struct abort_scan;
	struct work_struct update_link_led;
	struct work_struct auth_work;
	struct work_struct report_work;
	struct work_struct request_scan;
	struct work_struct beacon_update;
	struct work_struct tt_work;
	struct work_struct ct_enter;
	struct work_struct ct_exit;

	struct tasklet_struct irq_tasklet;

	struct delayed_work init_alive_start;
	struct delayed_work alive_start;
	struct delayed_work scan_check;

	
	struct delayed_work thermal_periodic;
	struct delayed_work rfkill_poll;

	
	s8 tx_power_user_lmt;
	s8 tx_power_device_lmt;


#ifdef CONFIG_IWLWIFI_DEBUG
	
	u32 debug_level; 
	u32 framecnt_to_us;
	atomic_t restrict_refcnt;
	bool disable_ht40;
#ifdef CONFIG_IWLWIFI_DEBUGFS
	
	u16 tx_traffic_idx;
	u16 rx_traffic_idx;
	u8 *tx_traffic;
	u8 *rx_traffic;
	struct iwl_debugfs *dbgfs;
#endif 
#endif 

	struct work_struct txpower_work;
	u32 disable_sens_cal;
	u32 disable_chain_noise_cal;
	u32 disable_tx_power_cal;
	struct work_struct run_time_calib_work;
	struct timer_list statistics_periodic;
	bool hw_ready;
	
#define IWL_DEFAULT_TX_POWER 0x0F

	struct iwl3945_notif_statistics statistics_39;

	u32 sta_supp_rates;
}; 

static inline void iwl_txq_ctx_activate(struct iwl_priv *priv, int txq_id)
{
	set_bit(txq_id, &priv->txq_ctx_active_msk);
}

static inline void iwl_txq_ctx_deactivate(struct iwl_priv *priv, int txq_id)
{
	clear_bit(txq_id, &priv->txq_ctx_active_msk);
}

#ifdef CONFIG_IWLWIFI_DEBUG
const char *iwl_get_tx_fail_reason(u32 status);

static inline u32 iwl_get_debug_level(struct iwl_priv *priv)
{
	if (priv->debug_level)
		return priv->debug_level;
	else
		return iwl_debug_level;
}
#else
static inline const char *iwl_get_tx_fail_reason(u32 status) { return ""; }

static inline u32 iwl_get_debug_level(struct iwl_priv *priv)
{
	return iwl_debug_level;
}
#endif


static inline struct ieee80211_hdr *iwl_tx_queue_get_hdr(struct iwl_priv *priv,
							 int txq_id, int idx)
{
	if (priv->txq[txq_id].txb[idx].skb[0])
		return (struct ieee80211_hdr *)priv->txq[txq_id].
				txb[idx].skb[0]->data;
	return NULL;
}


static inline int iwl_is_associated(struct iwl_priv *priv)
{
	return (priv->active_rxon.filter_flags & RXON_FILTER_ASSOC_MSK) ? 1 : 0;
}

static inline int is_channel_valid(const struct iwl_channel_info *ch_info)
{
	if (ch_info == NULL)
		return 0;
	return (ch_info->flags & EEPROM_CHANNEL_VALID) ? 1 : 0;
}

static inline int is_channel_radar(const struct iwl_channel_info *ch_info)
{
	return (ch_info->flags & EEPROM_CHANNEL_RADAR) ? 1 : 0;
}

static inline u8 is_channel_a_band(const struct iwl_channel_info *ch_info)
{
	return ch_info->band == IEEE80211_BAND_5GHZ;
}

static inline u8 is_channel_bg_band(const struct iwl_channel_info *ch_info)
{
	return ch_info->band == IEEE80211_BAND_2GHZ;
}

static inline int is_channel_passive(const struct iwl_channel_info *ch)
{
	return (!(ch->flags & EEPROM_CHANNEL_ACTIVE)) ? 1 : 0;
}

static inline int is_channel_ibss(const struct iwl_channel_info *ch)
{
	return ((ch->flags & EEPROM_CHANNEL_IBSS)) ? 1 : 0;
}

#endif				
