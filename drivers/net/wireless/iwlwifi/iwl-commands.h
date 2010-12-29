


#ifndef __iwl_commands_h__
#define __iwl_commands_h__

struct iwl_priv;


#define IWL_UCODE_MAJOR(ver)	(((ver) & 0xFF000000) >> 24)
#define IWL_UCODE_MINOR(ver)	(((ver) & 0x00FF0000) >> 16)
#define IWL_UCODE_API(ver)	(((ver) & 0x0000FF00) >> 8)
#define IWL_UCODE_SERIAL(ver)	((ver) & 0x000000FF)



#define IWL_CCK_RATES	4
#define IWL_OFDM_RATES	8
#define IWL_MAX_RATES	(IWL_CCK_RATES + IWL_OFDM_RATES)

enum {
	REPLY_ALIVE = 0x1,
	REPLY_ERROR = 0x2,

	
	REPLY_RXON = 0x10,
	REPLY_RXON_ASSOC = 0x11,
	REPLY_QOS_PARAM = 0x13,
	REPLY_RXON_TIMING = 0x14,

	
	REPLY_ADD_STA = 0x18,
	REPLY_REMOVE_STA = 0x19,	
	REPLY_REMOVE_ALL_STA = 0x1a,	

	
	REPLY_WEPKEY = 0x20,

	
	REPLY_3945_RX = 0x1b,           
	REPLY_TX = 0x1c,
	REPLY_RATE_SCALE = 0x47,	
	REPLY_LEDS_CMD = 0x48,
	REPLY_TX_LINK_QUALITY_CMD = 0x4e, 

	
	COEX_PRIORITY_TABLE_CMD = 0x5a,	
	COEX_MEDIUM_NOTIFICATION = 0x5b,
	COEX_EVENT_CMD = 0x5c,

	
	CALIBRATION_CFG_CMD = 0x65,
	CALIBRATION_RES_NOTIFICATION = 0x66,
	CALIBRATION_COMPLETE_NOTIFICATION = 0x67,

	
	RADAR_NOTIFICATION = 0x70,	
	REPLY_QUIET_CMD = 0x71,		
	REPLY_CHANNEL_SWITCH = 0x72,
	CHANNEL_SWITCH_NOTIFICATION = 0x73,
	REPLY_SPECTRUM_MEASUREMENT_CMD = 0x74,
	SPECTRUM_MEASURE_NOTIFICATION = 0x75,

	
	POWER_TABLE_CMD = 0x77,
	PM_SLEEP_NOTIFICATION = 0x7A,
	PM_DEBUG_STATISTIC_NOTIFIC = 0x7B,

	
	REPLY_SCAN_CMD = 0x80,
	REPLY_SCAN_ABORT_CMD = 0x81,
	SCAN_START_NOTIFICATION = 0x82,
	SCAN_RESULTS_NOTIFICATION = 0x83,
	SCAN_COMPLETE_NOTIFICATION = 0x84,

	
	BEACON_NOTIFICATION = 0x90,
	REPLY_TX_BEACON = 0x91,
	WHO_IS_AWAKE_NOTIFICATION = 0x94,	

	
	REPLY_TX_POWER_DBM_CMD = 0x95,
	QUIET_NOTIFICATION = 0x96,		
	REPLY_TX_PWR_TABLE_CMD = 0x97,
	REPLY_TX_POWER_DBM_CMD_V1 = 0x98,	
	TX_ANT_CONFIGURATION_CMD = 0x98,	
	MEASURE_ABORT_NOTIFICATION = 0x99,	

	
	REPLY_BT_CONFIG = 0x9b,

	
	REPLY_STATISTICS_CMD = 0x9c,
	STATISTICS_NOTIFICATION = 0x9d,

	
	REPLY_CARD_STATE_CMD = 0xa0,
	CARD_STATE_NOTIFICATION = 0xa1,

	
	MISSED_BEACONS_NOTIFICATION = 0xa2,

	REPLY_CT_KILL_CONFIG_CMD = 0xa4,
	SENSITIVITY_CMD = 0xa8,
	REPLY_PHY_CALIBRATION_CMD = 0xb0,
	REPLY_RX_PHY_CMD = 0xc0,
	REPLY_RX_MPDU_CMD = 0xc1,
	REPLY_RX = 0xc3,
	REPLY_COMPRESSED_BA = 0xc5,
	REPLY_MAX = 0xff
};




#define IWL_CMD_FAILED_MSK 0x40

#define SEQ_TO_QUEUE(s)	(((s) >> 8) & 0x1f)
#define QUEUE_TO_SEQ(q)	(((q) & 0x1f) << 8)
#define SEQ_TO_INDEX(s)	((s) & 0xff)
#define INDEX_TO_SEQ(i)	((i) & 0xff)
#define SEQ_HUGE_FRAME	cpu_to_le16(0x4000)
#define SEQ_RX_FRAME	cpu_to_le16(0x8000)


struct iwl_cmd_header {
	u8 cmd;		
	u8 flags;	
	
	__le16 sequence;

	
	u8 data[0];
} __attribute__ ((packed));



struct iwl3945_tx_power {
	u8 tx_gain;		
	u8 dsp_atten;		
} __attribute__ ((packed));


struct iwl3945_power_per_rate {
	u8 rate;		
	struct iwl3945_tx_power tpc;
	u8 reserved;
} __attribute__ ((packed));


#define RATE_MCS_CODE_MSK 0x7
#define RATE_MCS_SPATIAL_POS 3
#define RATE_MCS_SPATIAL_MSK 0x18
#define RATE_MCS_HT_DUP_POS 5
#define RATE_MCS_HT_DUP_MSK 0x20


#define RATE_MCS_FLAGS_POS 8
#define RATE_MCS_HT_POS 8
#define RATE_MCS_HT_MSK 0x100


#define RATE_MCS_CCK_POS 9
#define RATE_MCS_CCK_MSK 0x200


#define RATE_MCS_GF_POS 10
#define RATE_MCS_GF_MSK 0x400


#define RATE_MCS_HT40_POS 11
#define RATE_MCS_HT40_MSK 0x800


#define RATE_MCS_DUP_POS 12
#define RATE_MCS_DUP_MSK 0x1000


#define RATE_MCS_SGI_POS 13
#define RATE_MCS_SGI_MSK 0x2000


#define RATE_MCS_ANT_POS	14
#define RATE_MCS_ANT_A_MSK	0x04000
#define RATE_MCS_ANT_B_MSK	0x08000
#define RATE_MCS_ANT_C_MSK	0x10000
#define RATE_MCS_ANT_AB_MSK	(RATE_MCS_ANT_A_MSK | RATE_MCS_ANT_B_MSK)
#define RATE_MCS_ANT_ABC_MSK	(RATE_MCS_ANT_AB_MSK | RATE_MCS_ANT_C_MSK)
#define RATE_ANT_NUM 3

#define POWER_TABLE_NUM_ENTRIES			33
#define POWER_TABLE_NUM_HT_OFDM_ENTRIES		32
#define POWER_TABLE_CCK_ENTRY			32


union iwl4965_tx_power_dual_stream {
	struct {
		u8 radio_tx_gain[2];
		u8 dsp_predis_atten[2];
	} s;
	u32 dw;
};


struct tx_power_dual_stream {
	__le32 dw;
} __attribute__ ((packed));


struct iwl4965_tx_power_db {
	struct tx_power_dual_stream power_tbl[POWER_TABLE_NUM_ENTRIES];
} __attribute__ ((packed));


#define IWL50_TX_POWER_AUTO 0x7f
#define IWL50_TX_POWER_NO_CLOSED (0x1 << 6)

struct iwl5000_tx_power_dbm_cmd {
	s8 global_lmt; 
	u8 flags;
	s8 srv_chan_lmt; 
	u8 reserved;
} __attribute__ ((packed));



#define UCODE_VALID_OK	cpu_to_le32(0x1)
#define INITIALIZE_SUBTYPE    (9)


struct iwl_init_alive_resp {
	u8 ucode_minor;
	u8 ucode_major;
	__le16 reserved1;
	u8 sw_rev[8];
	u8 ver_type;
	u8 ver_subtype;		
	__le16 reserved2;
	__le32 log_event_table_ptr;
	__le32 error_event_table_ptr;
	__le32 timestamp;
	__le32 is_valid;

	
	__le32 voltage;		
	__le32 therm_r1[2];	
	__le32 therm_r2[2];	
	__le32 therm_r3[2];	
	__le32 therm_r4[2];	
	__le32 tx_atten[5][2];	
} __attribute__ ((packed));



struct iwl_alive_resp {
	u8 ucode_minor;
	u8 ucode_major;
	__le16 reserved1;
	u8 sw_rev[8];
	u8 ver_type;
	u8 ver_subtype;			
	__le16 reserved2;
	__le32 log_event_table_ptr;	
	__le32 error_event_table_ptr;	
	__le32 timestamp;
	__le32 is_valid;
} __attribute__ ((packed));


struct iwl_error_resp {
	__le32 error_type;
	u8 cmd_id;
	u8 reserved1;
	__le16 bad_cmd_seq_num;
	__le32 error_info;
	__le64 timestamp;
} __attribute__ ((packed));





enum {
	RXON_DEV_TYPE_AP = 1,
	RXON_DEV_TYPE_ESS = 3,
	RXON_DEV_TYPE_IBSS = 4,
	RXON_DEV_TYPE_SNIFFER = 6,
};


#define RXON_RX_CHAIN_DRIVER_FORCE_MSK		cpu_to_le16(0x1 << 0)
#define RXON_RX_CHAIN_DRIVER_FORCE_POS		(0)
#define RXON_RX_CHAIN_VALID_MSK			cpu_to_le16(0x7 << 1)
#define RXON_RX_CHAIN_VALID_POS			(1)
#define RXON_RX_CHAIN_FORCE_SEL_MSK		cpu_to_le16(0x7 << 4)
#define RXON_RX_CHAIN_FORCE_SEL_POS		(4)
#define RXON_RX_CHAIN_FORCE_MIMO_SEL_MSK	cpu_to_le16(0x7 << 7)
#define RXON_RX_CHAIN_FORCE_MIMO_SEL_POS	(7)
#define RXON_RX_CHAIN_CNT_MSK			cpu_to_le16(0x3 << 10)
#define RXON_RX_CHAIN_CNT_POS			(10)
#define RXON_RX_CHAIN_MIMO_CNT_MSK		cpu_to_le16(0x3 << 12)
#define RXON_RX_CHAIN_MIMO_CNT_POS		(12)
#define RXON_RX_CHAIN_MIMO_FORCE_MSK		cpu_to_le16(0x1 << 14)
#define RXON_RX_CHAIN_MIMO_FORCE_POS		(14)



#define RXON_FLG_BAND_24G_MSK           cpu_to_le32(1 << 0)
#define RXON_FLG_CCK_MSK                cpu_to_le32(1 << 1)

#define RXON_FLG_AUTO_DETECT_MSK        cpu_to_le32(1 << 2)

#define RXON_FLG_TGG_PROTECT_MSK        cpu_to_le32(1 << 3)

#define RXON_FLG_SHORT_SLOT_MSK          cpu_to_le32(1 << 4)
#define RXON_FLG_SHORT_PREAMBLE_MSK     cpu_to_le32(1 << 5)

#define RXON_FLG_DIS_DIV_MSK            cpu_to_le32(1 << 7)
#define RXON_FLG_ANT_SEL_MSK            cpu_to_le32(0x0f00)
#define RXON_FLG_ANT_A_MSK              cpu_to_le32(1 << 8)
#define RXON_FLG_ANT_B_MSK              cpu_to_le32(1 << 9)

#define RXON_FLG_RADAR_DETECT_MSK       cpu_to_le32(1 << 12)
#define RXON_FLG_TGJ_NARROW_BAND_MSK    cpu_to_le32(1 << 13)

#define RXON_FLG_TSF2HOST_MSK           cpu_to_le32(1 << 15)



#define RXON_FLG_CTRL_CHANNEL_LOC_POS		(22)
#define RXON_FLG_CTRL_CHANNEL_LOC_HI_MSK	cpu_to_le32(0x1 << 22)

#define RXON_FLG_HT_OPERATING_MODE_POS		(23)

#define RXON_FLG_HT_PROT_MSK			cpu_to_le32(0x1 << 23)
#define RXON_FLG_HT40_PROT_MSK			cpu_to_le32(0x2 << 23)

#define RXON_FLG_CHANNEL_MODE_POS		(25)
#define RXON_FLG_CHANNEL_MODE_MSK		cpu_to_le32(0x3 << 25)


enum {
	CHANNEL_MODE_LEGACY = 0,
	CHANNEL_MODE_PURE_40 = 1,
	CHANNEL_MODE_MIXED = 2,
	CHANNEL_MODE_RESERVED = 3,
};
#define RXON_FLG_CHANNEL_MODE_LEGACY	cpu_to_le32(CHANNEL_MODE_LEGACY << RXON_FLG_CHANNEL_MODE_POS)
#define RXON_FLG_CHANNEL_MODE_PURE_40	cpu_to_le32(CHANNEL_MODE_PURE_40 << RXON_FLG_CHANNEL_MODE_POS)
#define RXON_FLG_CHANNEL_MODE_MIXED	cpu_to_le32(CHANNEL_MODE_MIXED << RXON_FLG_CHANNEL_MODE_POS)


#define RXON_FLG_SELF_CTS_EN			cpu_to_le32(0x1<<30)



#define RXON_FILTER_PROMISC_MSK         cpu_to_le32(1 << 0)

#define RXON_FILTER_CTL2HOST_MSK        cpu_to_le32(1 << 1)

#define RXON_FILTER_ACCEPT_GRP_MSK      cpu_to_le32(1 << 2)

#define RXON_FILTER_DIS_DECRYPT_MSK     cpu_to_le32(1 << 3)

#define RXON_FILTER_DIS_GRP_DECRYPT_MSK cpu_to_le32(1 << 4)

#define RXON_FILTER_ASSOC_MSK           cpu_to_le32(1 << 5)

#define RXON_FILTER_BCON_AWARE_MSK      cpu_to_le32(1 << 6)



struct iwl3945_rxon_cmd {
	u8 node_addr[6];
	__le16 reserved1;
	u8 bssid_addr[6];
	__le16 reserved2;
	u8 wlap_bssid_addr[6];
	__le16 reserved3;
	u8 dev_type;
	u8 air_propagation;
	__le16 reserved4;
	u8 ofdm_basic_rates;
	u8 cck_basic_rates;
	__le16 assoc_id;
	__le32 flags;
	__le32 filter_flags;
	__le16 channel;
	__le16 reserved5;
} __attribute__ ((packed));

struct iwl4965_rxon_cmd {
	u8 node_addr[6];
	__le16 reserved1;
	u8 bssid_addr[6];
	__le16 reserved2;
	u8 wlap_bssid_addr[6];
	__le16 reserved3;
	u8 dev_type;
	u8 air_propagation;
	__le16 rx_chain;
	u8 ofdm_basic_rates;
	u8 cck_basic_rates;
	__le16 assoc_id;
	__le32 flags;
	__le32 filter_flags;
	__le16 channel;
	u8 ofdm_ht_single_stream_basic_rates;
	u8 ofdm_ht_dual_stream_basic_rates;
} __attribute__ ((packed));


struct iwl_rxon_cmd {
	u8 node_addr[6];
	__le16 reserved1;
	u8 bssid_addr[6];
	__le16 reserved2;
	u8 wlap_bssid_addr[6];
	__le16 reserved3;
	u8 dev_type;
	u8 air_propagation;
	__le16 rx_chain;
	u8 ofdm_basic_rates;
	u8 cck_basic_rates;
	__le16 assoc_id;
	__le32 flags;
	__le32 filter_flags;
	__le16 channel;
	u8 ofdm_ht_single_stream_basic_rates;
	u8 ofdm_ht_dual_stream_basic_rates;
	u8 ofdm_ht_triple_stream_basic_rates;
	u8 reserved5;
	__le16 acquisition_data;
	__le16 reserved6;
} __attribute__ ((packed));


struct iwl3945_rxon_assoc_cmd {
	__le32 flags;
	__le32 filter_flags;
	u8 ofdm_basic_rates;
	u8 cck_basic_rates;
	__le16 reserved;
} __attribute__ ((packed));

struct iwl4965_rxon_assoc_cmd {
	__le32 flags;
	__le32 filter_flags;
	u8 ofdm_basic_rates;
	u8 cck_basic_rates;
	u8 ofdm_ht_single_stream_basic_rates;
	u8 ofdm_ht_dual_stream_basic_rates;
	__le16 rx_chain_select_flags;
	__le16 reserved;
} __attribute__ ((packed));

struct iwl5000_rxon_assoc_cmd {
	__le32 flags;
	__le32 filter_flags;
	u8 ofdm_basic_rates;
	u8 cck_basic_rates;
	__le16 reserved1;
	u8 ofdm_ht_single_stream_basic_rates;
	u8 ofdm_ht_dual_stream_basic_rates;
	u8 ofdm_ht_triple_stream_basic_rates;
	u8 reserved2;
	__le16 rx_chain_select_flags;
	__le16 acquisition_data;
	__le32 reserved3;
} __attribute__ ((packed));

#define IWL_CONN_MAX_LISTEN_INTERVAL	10
#define IWL_MAX_UCODE_BEACON_INTERVAL	4 
#define IWL39_MAX_UCODE_BEACON_INTERVAL	1 


struct iwl_rxon_time_cmd {
	__le64 timestamp;
	__le16 beacon_interval;
	__le16 atim_window;
	__le32 beacon_init_val;
	__le16 listen_interval;
	__le16 reserved;
} __attribute__ ((packed));


struct iwl3945_channel_switch_cmd {
	u8 band;
	u8 expect_beacon;
	__le16 channel;
	__le32 rxon_flags;
	__le32 rxon_filter_flags;
	__le32 switch_time;
	struct iwl3945_power_per_rate power[IWL_MAX_RATES];
} __attribute__ ((packed));

struct iwl_channel_switch_cmd {
	u8 band;
	u8 expect_beacon;
	__le16 channel;
	__le32 rxon_flags;
	__le32 rxon_filter_flags;
	__le32 switch_time;
	struct iwl4965_tx_power_db tx_power;
} __attribute__ ((packed));


struct iwl_csa_notification {
	__le16 band;
	__le16 channel;
	__le32 status;		
} __attribute__ ((packed));




struct iwl_ac_qos {
	__le16 cw_min;
	__le16 cw_max;
	u8 aifsn;
	u8 reserved1;
	__le16 edca_txop;
} __attribute__ ((packed));


#define QOS_PARAM_FLG_UPDATE_EDCA_MSK	cpu_to_le32(0x01)
#define QOS_PARAM_FLG_TGN_MSK		cpu_to_le32(0x02)
#define QOS_PARAM_FLG_TXOP_TYPE_MSK	cpu_to_le32(0x10)


#define AC_NUM                4


struct iwl_qosparam_cmd {
	__le32 qos_flags;
	struct iwl_ac_qos ac[AC_NUM];
} __attribute__ ((packed));





#define	IWL_AP_ID		0
#define IWL_MULTICAST_ID	1
#define	IWL_STA_ID		2
#define	IWL3945_BROADCAST_ID	24
#define IWL3945_STATION_COUNT	25
#define IWL4965_BROADCAST_ID	31
#define	IWL4965_STATION_COUNT	32
#define IWL5000_BROADCAST_ID	15
#define	IWL5000_STATION_COUNT	16

#define	IWL_STATION_COUNT	32 	
#define	IWL_INVALID_STATION 	255

#define STA_FLG_TX_RATE_MSK		cpu_to_le32(1 << 2);
#define STA_FLG_PWR_SAVE_MSK		cpu_to_le32(1 << 8);
#define STA_FLG_RTS_MIMO_PROT_MSK	cpu_to_le32(1 << 17)
#define STA_FLG_AGG_MPDU_8US_MSK	cpu_to_le32(1 << 18)
#define STA_FLG_MAX_AGG_SIZE_POS	(19)
#define STA_FLG_MAX_AGG_SIZE_MSK	cpu_to_le32(3 << 19)
#define STA_FLG_HT40_EN_MSK		cpu_to_le32(1 << 21)
#define STA_FLG_MIMO_DIS_MSK		cpu_to_le32(1 << 22)
#define STA_FLG_AGG_MPDU_DENSITY_POS	(23)
#define STA_FLG_AGG_MPDU_DENSITY_MSK	cpu_to_le32(7 << 23)


#define STA_CONTROL_MODIFY_MSK		0x01


#define STA_KEY_FLG_ENCRYPT_MSK	cpu_to_le16(0x0007)
#define STA_KEY_FLG_NO_ENC	cpu_to_le16(0x0000)
#define STA_KEY_FLG_WEP		cpu_to_le16(0x0001)
#define STA_KEY_FLG_CCMP	cpu_to_le16(0x0002)
#define STA_KEY_FLG_TKIP	cpu_to_le16(0x0003)

#define STA_KEY_FLG_KEYID_POS	8
#define STA_KEY_FLG_INVALID 	cpu_to_le16(0x0800)

#define STA_KEY_FLG_MAP_KEY_MSK	cpu_to_le16(0x0008)


#define STA_KEY_FLG_KEY_SIZE_MSK     cpu_to_le16(0x1000)
#define STA_KEY_MULTICAST_MSK        cpu_to_le16(0x4000)
#define STA_KEY_MAX_NUM		8


#define	STA_MODIFY_KEY_MASK		0x01
#define	STA_MODIFY_TID_DISABLE_TX	0x02
#define	STA_MODIFY_TX_RATE_MSK		0x04
#define STA_MODIFY_ADDBA_TID_MSK	0x08
#define STA_MODIFY_DELBA_TID_MSK	0x10


#define BUILD_RAxTID(sta_id, tid)	(((sta_id) << 4) + (tid))

struct iwl4965_keyinfo {
	__le16 key_flags;
	u8 tkip_rx_tsc_byte2;	
	u8 reserved1;
	__le16 tkip_rx_ttak[5];	
	u8 key_offset;
	u8 reserved2;
	u8 key[16];		
} __attribute__ ((packed));


struct iwl_keyinfo {
	__le16 key_flags;
	u8 tkip_rx_tsc_byte2;	
	u8 reserved1;
	__le16 tkip_rx_ttak[5];	
	u8 key_offset;
	u8 reserved2;
	u8 key[16];		
	__le64 tx_secur_seq_cnt;
	__le64 hw_tkip_mic_rx_key;
	__le64 hw_tkip_mic_tx_key;
} __attribute__ ((packed));


struct sta_id_modify {
	u8 addr[ETH_ALEN];
	__le16 reserved1;
	u8 sta_id;
	u8 modify_mask;
	__le16 reserved2;
} __attribute__ ((packed));



struct iwl3945_addsta_cmd {
	u8 mode;		
	u8 reserved[3];
	struct sta_id_modify sta;
	struct iwl4965_keyinfo key;
	__le32 station_flags;		
	__le32 station_flags_msk;	

	
	__le16 tid_disable_tx;

	__le16 rate_n_flags;

	
	u8 add_immediate_ba_tid;

	
	u8 remove_immediate_ba_tid;

	
	__le16 add_immediate_ba_ssn;
} __attribute__ ((packed));

struct iwl4965_addsta_cmd {
	u8 mode;		
	u8 reserved[3];
	struct sta_id_modify sta;
	struct iwl4965_keyinfo key;
	__le32 station_flags;		
	__le32 station_flags_msk;	

	
	__le16 tid_disable_tx;

	__le16	reserved1;

	
	u8 add_immediate_ba_tid;

	
	u8 remove_immediate_ba_tid;

	
	__le16 add_immediate_ba_ssn;

	__le32 reserved2;
} __attribute__ ((packed));


struct iwl_addsta_cmd {
	u8 mode;		
	u8 reserved[3];
	struct sta_id_modify sta;
	struct iwl_keyinfo key;
	__le32 station_flags;		
	__le32 station_flags_msk;	

	
	__le16 tid_disable_tx;

	__le16	rate_n_flags;		

	
	u8 add_immediate_ba_tid;

	
	u8 remove_immediate_ba_tid;

	
	__le16 add_immediate_ba_ssn;

	__le32 reserved2;
} __attribute__ ((packed));


#define ADD_STA_SUCCESS_MSK		0x1
#define ADD_STA_NO_ROOM_IN_TABLE	0x2
#define ADD_STA_NO_BLOCK_ACK_RESOURCE	0x4
#define ADD_STA_MODIFY_NON_EXIST_STA	0x8

struct iwl_add_sta_resp {
	u8 status;	
} __attribute__ ((packed));

#define REM_STA_SUCCESS_MSK              0x1

struct iwl_rem_sta_resp {
	u8 status;
} __attribute__ ((packed));


struct iwl_rem_sta_cmd {
	u8 num_sta;     
	u8 reserved[3];
	u8 addr[ETH_ALEN]; 
	u8 reserved2[2];
} __attribute__ ((packed));


struct iwl_wep_key {
	u8 key_index;
	u8 key_offset;
	u8 reserved1[2];
	u8 key_size;
	u8 reserved2[3];
	u8 key[16];
} __attribute__ ((packed));

struct iwl_wep_cmd {
	u8 num_keys;
	u8 global_key_type;
	u8 flags;
	u8 reserved;
	struct iwl_wep_key key[0];
} __attribute__ ((packed));

#define WEP_KEY_WEP_TYPE 1
#define WEP_KEYS_MAX 4
#define WEP_INVALID_OFFSET 0xff
#define WEP_KEY_LEN_64 5
#define WEP_KEY_LEN_128 13



#define RX_RES_STATUS_NO_CRC32_ERROR	cpu_to_le32(1 << 0)
#define RX_RES_STATUS_NO_RXE_OVERFLOW	cpu_to_le32(1 << 1)

#define RX_RES_PHY_FLAGS_BAND_24_MSK	cpu_to_le16(1 << 0)
#define RX_RES_PHY_FLAGS_MOD_CCK_MSK		cpu_to_le16(1 << 1)
#define RX_RES_PHY_FLAGS_SHORT_PREAMBLE_MSK	cpu_to_le16(1 << 2)
#define RX_RES_PHY_FLAGS_NARROW_BAND_MSK	cpu_to_le16(1 << 3)
#define RX_RES_PHY_FLAGS_ANTENNA_MSK		0xf0
#define RX_RES_PHY_FLAGS_ANTENNA_POS		4

#define RX_RES_STATUS_SEC_TYPE_MSK	(0x7 << 8)
#define RX_RES_STATUS_SEC_TYPE_NONE	(0x0 << 8)
#define RX_RES_STATUS_SEC_TYPE_WEP	(0x1 << 8)
#define RX_RES_STATUS_SEC_TYPE_CCMP	(0x2 << 8)
#define RX_RES_STATUS_SEC_TYPE_TKIP	(0x3 << 8)
#define	RX_RES_STATUS_SEC_TYPE_ERR	(0x7 << 8)

#define RX_RES_STATUS_STATION_FOUND	(1<<6)
#define RX_RES_STATUS_NO_STATION_INFO_MISMATCH	(1<<7)

#define RX_RES_STATUS_DECRYPT_TYPE_MSK	(0x3 << 11)
#define RX_RES_STATUS_NOT_DECRYPT	(0x0 << 11)
#define RX_RES_STATUS_DECRYPT_OK	(0x3 << 11)
#define RX_RES_STATUS_BAD_ICV_MIC	(0x1 << 11)
#define RX_RES_STATUS_BAD_KEY_TTAK	(0x2 << 11)

#define RX_MPDU_RES_STATUS_ICV_OK	(0x20)
#define RX_MPDU_RES_STATUS_MIC_OK	(0x40)
#define RX_MPDU_RES_STATUS_TTAK_OK	(1 << 7)
#define RX_MPDU_RES_STATUS_DEC_DONE_MSK	(0x800)


struct iwl3945_rx_frame_stats {
	u8 phy_count;
	u8 id;
	u8 rssi;
	u8 agc;
	__le16 sig_avg;
	__le16 noise_diff;
	u8 payload[0];
} __attribute__ ((packed));

struct iwl3945_rx_frame_hdr {
	__le16 channel;
	__le16 phy_flags;
	u8 reserved1;
	u8 rate;
	__le16 len;
	u8 payload[0];
} __attribute__ ((packed));

struct iwl3945_rx_frame_end {
	__le32 status;
	__le64 timestamp;
	__le32 beacon_timestamp;
} __attribute__ ((packed));


struct iwl3945_rx_frame {
	struct iwl3945_rx_frame_stats stats;
	struct iwl3945_rx_frame_hdr hdr;
	struct iwl3945_rx_frame_end end;
} __attribute__ ((packed));

#define IWL39_RX_FRAME_SIZE	(4 + sizeof(struct iwl3945_rx_frame))



#define IWL49_RX_RES_PHY_CNT 14
#define IWL49_RX_PHY_FLAGS_ANTENNAE_OFFSET	(4)
#define IWL49_RX_PHY_FLAGS_ANTENNAE_MASK	(0x70)
#define IWL49_AGC_DB_MASK			(0x3f80)	
#define IWL49_AGC_DB_POS			(7)
struct iwl4965_rx_non_cfg_phy {
	__le16 ant_selection;	
	__le16 agc_info;	
	u8 rssi_info[6];	
	u8 pad[0];
} __attribute__ ((packed));


#define IWL50_RX_RES_PHY_CNT 8
#define IWL50_RX_RES_AGC_IDX     1
#define IWL50_RX_RES_RSSI_AB_IDX 2
#define IWL50_RX_RES_RSSI_C_IDX  3
#define IWL50_OFDM_AGC_MSK 0xfe00
#define IWL50_OFDM_AGC_BIT_POS 9
#define IWL50_OFDM_RSSI_A_MSK 0x00ff
#define IWL50_OFDM_RSSI_A_BIT_POS 0
#define IWL50_OFDM_RSSI_B_MSK 0xff0000
#define IWL50_OFDM_RSSI_B_BIT_POS 16
#define IWL50_OFDM_RSSI_C_MSK 0x00ff
#define IWL50_OFDM_RSSI_C_BIT_POS 0

struct iwl5000_non_cfg_phy {
	__le32 non_cfg_phy[IWL50_RX_RES_PHY_CNT];  
} __attribute__ ((packed));



struct iwl_rx_phy_res {
	u8 non_cfg_phy_cnt;     
	u8 cfg_phy_cnt;		
	u8 stat_id;		
	u8 reserved1;
	__le64 timestamp;	
	__le32 beacon_time_stamp; 
	__le16 phy_flags;	
	__le16 channel;		
	u8 non_cfg_phy_buf[32]; 
	__le32 rate_n_flags;	
	__le16 byte_count;	
	__le16 reserved3;
} __attribute__ ((packed));

struct iwl4965_rx_mpdu_res_start {
	__le16 byte_count;
	__le16 reserved;
} __attribute__ ((packed));







#define TX_CMD_FLG_RTS_CTS_MSK cpu_to_le32(1 << 0)


#define TX_CMD_FLG_RTS_MSK cpu_to_le32(1 << 1)


#define TX_CMD_FLG_CTS_MSK cpu_to_le32(1 << 2)


#define TX_CMD_FLG_ACK_MSK cpu_to_le32(1 << 3)


#define TX_CMD_FLG_STA_RATE_MSK cpu_to_le32(1 << 4)


#define TX_CMD_FLG_IMM_BA_RSP_MASK  cpu_to_le32(1 << 6)


#define TX_CMD_FLG_FULL_TXOP_PROT_MSK cpu_to_le32(1 << 7)


#define TX_CMD_FLG_ANT_SEL_MSK cpu_to_le32(0xf00)
#define TX_CMD_FLG_ANT_A_MSK cpu_to_le32(1 << 8)
#define TX_CMD_FLG_ANT_B_MSK cpu_to_le32(1 << 9)


#define TX_CMD_FLG_BT_DIS_MSK cpu_to_le32(1 << 12)


#define TX_CMD_FLG_SEQ_CTL_MSK cpu_to_le32(1 << 13)


#define TX_CMD_FLG_MORE_FRAG_MSK cpu_to_le32(1 << 14)


#define TX_CMD_FLG_TSF_MSK cpu_to_le32(1 << 16)


#define TX_CMD_FLG_MH_PAD_MSK cpu_to_le32(1 << 20)


#define TX_CMD_FLG_AGG_CCMP_MSK cpu_to_le32(1 << 22)


#define TX_CMD_FLG_DUR_MSK cpu_to_le32(1 << 25)



#define TX_CMD_SEC_WEP  	0x01
#define TX_CMD_SEC_CCM  	0x02
#define TX_CMD_SEC_TKIP		0x03
#define TX_CMD_SEC_MSK		0x03
#define TX_CMD_SEC_SHIFT	6
#define TX_CMD_SEC_KEY128	0x08


#define WEP_IV_LEN 4
#define WEP_ICV_LEN 4
#define CCMP_MIC_LEN 8
#define TKIP_ICV_LEN 4



struct iwl3945_tx_cmd {
	
	__le16 len;

	
	__le16 next_frame_len;

	__le32 tx_flags;	

	u8 rate;

	
	u8 sta_id;
	u8 tid_tspec;
	u8 sec_ctl;
	u8 key[16];
	union {
		u8 byte[8];
		__le16 word[4];
		__le32 dw[2];
	} tkip_mic;
	__le32 next_frame_info;
	union {
		__le32 life_time;
		__le32 attempt;
	} stop_time;
	u8 supp_rates[2];
	u8 rts_retry_limit;	
	u8 data_retry_limit;	
	union {
		__le16 pm_frame_timeout;
		__le16 attempt_duration;
	} timeout;

	
	__le16 driver_txop;

	
	u8 payload[0];
	struct ieee80211_hdr hdr[0];
} __attribute__ ((packed));


struct iwl3945_tx_resp {
	u8 failure_rts;
	u8 failure_frame;
	u8 bt_kill_count;
	u8 rate;
	__le32 wireless_media_time;
	__le32 status;		
} __attribute__ ((packed));



struct iwl_dram_scratch {
	u8 try_cnt;		
	u8 bt_kill_cnt;		
	__le16 reserved;
} __attribute__ ((packed));

struct iwl_tx_cmd {
	
	__le16 len;

	
	__le16 next_frame_len;

	__le32 tx_flags;	

	
	struct iwl_dram_scratch scratch;

	
	__le32 rate_n_flags;	

	
	u8 sta_id;

	
	u8 sec_ctl;		

	
	u8 initial_rate_index;
	u8 reserved;
	u8 key[16];
	__le16 next_frame_flags;
	__le16 reserved2;
	union {
		__le32 life_time;
		__le32 attempt;
	} stop_time;

	
	__le32 dram_lsb_ptr;
	u8 dram_msb_ptr;

	u8 rts_retry_limit;	
	u8 data_retry_limit;	
	u8 tid_tspec;
	union {
		__le16 pm_frame_timeout;
		__le16 attempt_duration;
	} timeout;

	
	__le16 driver_txop;

	
	u8 payload[0];
	struct ieee80211_hdr hdr[0];
} __attribute__ ((packed));


enum {
	TX_STATUS_SUCCESS = 0x01,
	TX_STATUS_DIRECT_DONE = 0x02,
	TX_STATUS_FAIL_SHORT_LIMIT = 0x82,
	TX_STATUS_FAIL_LONG_LIMIT = 0x83,
	TX_STATUS_FAIL_FIFO_UNDERRUN = 0x84,
	TX_STATUS_FAIL_MGMNT_ABORT = 0x85,
	TX_STATUS_FAIL_NEXT_FRAG = 0x86,
	TX_STATUS_FAIL_LIFE_EXPIRE = 0x87,
	TX_STATUS_FAIL_DEST_PS = 0x88,
	TX_STATUS_FAIL_ABORTED = 0x89,
	TX_STATUS_FAIL_BT_RETRY = 0x8a,
	TX_STATUS_FAIL_STA_INVALID = 0x8b,
	TX_STATUS_FAIL_FRAG_DROPPED = 0x8c,
	TX_STATUS_FAIL_TID_DISABLE = 0x8d,
	TX_STATUS_FAIL_FRAME_FLUSHED = 0x8e,
	TX_STATUS_FAIL_INSUFFICIENT_CF_POLL = 0x8f,
	TX_STATUS_FAIL_TX_LOCKED = 0x90,
	TX_STATUS_FAIL_NO_BEACON_ON_RADAR = 0x91,
};

#define	TX_PACKET_MODE_REGULAR		0x0000
#define	TX_PACKET_MODE_BURST_SEQ	0x0100
#define	TX_PACKET_MODE_BURST_FIRST	0x0200

enum {
	TX_POWER_PA_NOT_ACTIVE = 0x0,
};

enum {
	TX_STATUS_MSK = 0x000000ff,		
	TX_STATUS_DELAY_MSK = 0x00000040,
	TX_STATUS_ABORT_MSK = 0x00000080,
	TX_PACKET_MODE_MSK = 0x0000ff00,	
	TX_FIFO_NUMBER_MSK = 0x00070000,	
	TX_RESERVED = 0x00780000,		
	TX_POWER_PA_DETECT_MSK = 0x7f800000,	
	TX_ABORT_REQUIRED_MSK = 0x80000000,	
};

static inline bool iwl_is_tx_success(u32 status)
{
	status &= TX_STATUS_MSK;
	return (status == TX_STATUS_SUCCESS) ||
	       (status == TX_STATUS_DIRECT_DONE);
}





enum {
	AGG_TX_STATE_TRANSMITTED = 0x00,
	AGG_TX_STATE_UNDERRUN_MSK = 0x01,
	AGG_TX_STATE_BT_PRIO_MSK = 0x02,
	AGG_TX_STATE_FEW_BYTES_MSK = 0x04,
	AGG_TX_STATE_ABORT_MSK = 0x08,
	AGG_TX_STATE_LAST_SENT_TTL_MSK = 0x10,
	AGG_TX_STATE_LAST_SENT_TRY_CNT_MSK = 0x20,
	AGG_TX_STATE_LAST_SENT_BT_KILL_MSK = 0x40,
	AGG_TX_STATE_SCD_QUERY_MSK = 0x80,
	AGG_TX_STATE_TEST_BAD_CRC32_MSK = 0x100,
	AGG_TX_STATE_RESPONSE_MSK = 0x1ff,
	AGG_TX_STATE_DUMP_TX_MSK = 0x200,
	AGG_TX_STATE_DELAY_TX_MSK = 0x400
};

#define AGG_TX_STATE_LAST_SENT_MSK  (AGG_TX_STATE_LAST_SENT_TTL_MSK | \
				     AGG_TX_STATE_LAST_SENT_TRY_CNT_MSK | \
				     AGG_TX_STATE_LAST_SENT_BT_KILL_MSK)


#define AGG_TX_STATE_TRY_CNT_POS 12
#define AGG_TX_STATE_TRY_CNT_MSK 0xf000


#define AGG_TX_STATE_SEQ_NUM_POS 16
#define AGG_TX_STATE_SEQ_NUM_MSK 0xffff0000


struct agg_tx_status {
	__le16 status;
	__le16 sequence;
} __attribute__ ((packed));

struct iwl4965_tx_resp {
	u8 frame_count;		
	u8 bt_kill_count;	
	u8 failure_rts;		
	u8 failure_frame;	

	
	__le32 rate_n_flags;	

	
	__le16 wireless_media_time;	

	__le16 reserved;
	__le32 pa_power1;	
	__le32 pa_power2;

	
	union {
		__le32 status;
		struct agg_tx_status agg_status[0]; 
	} u;
} __attribute__ ((packed));



#define IWL50_TX_RES_INIT_RATE_INDEX_POS	0
#define IWL50_TX_RES_INIT_RATE_INDEX_MSK	0x0f
#define IWL50_TX_RES_RATE_TABLE_COLOR_POS	4
#define IWL50_TX_RES_RATE_TABLE_COLOR_MSK	0x70
#define IWL50_TX_RES_INV_RATE_INDEX_MSK	0x80


#define IWL50_TX_RES_TID_POS	0
#define IWL50_TX_RES_TID_MSK	0x0f
#define IWL50_TX_RES_RA_POS	4
#define IWL50_TX_RES_RA_MSK	0xf0

struct iwl5000_tx_resp {
	u8 frame_count;		
	u8 bt_kill_count;	
	u8 failure_rts;		
	u8 failure_frame;	

	
	__le32 rate_n_flags;	

	
	__le16 wireless_media_time;	

	u8 pa_status;		
	u8 pa_integ_res_a[3];
	u8 pa_integ_res_b[3];
	u8 pa_integ_res_C[3];

	__le32 tfd_info;
	__le16 seq_ctl;
	__le16 byte_cnt;
	u8 tlc_info;
	u8 ra_tid;		
	__le16 frame_ctrl;
	
	struct agg_tx_status status;	
} __attribute__ ((packed));

struct iwl_compressed_ba_resp {
	__le32 sta_addr_lo32;
	__le16 sta_addr_hi16;
	__le16 reserved;

	
	u8 sta_id;
	u8 tid;
	__le16 seq_ctl;
	__le64 bitmap;
	__le16 scd_flow;
	__le16 scd_ssn;
} __attribute__ ((packed));



struct iwl3945_txpowertable_cmd {
	u8 band;		
	u8 reserved;
	__le16 channel;
	struct iwl3945_power_per_rate power[IWL_MAX_RATES];
} __attribute__ ((packed));

struct iwl4965_txpowertable_cmd {
	u8 band;		
	u8 reserved;
	__le16 channel;
	struct iwl4965_tx_power_db tx_power;
} __attribute__ ((packed));



struct iwl3945_rate_scaling_info {
	__le16 rate_n_flags;
	u8 try_cnt;
	u8 next_rate_index;
} __attribute__ ((packed));

struct iwl3945_rate_scaling_cmd {
	u8 table_id;
	u8 reserved[3];
	struct iwl3945_rate_scaling_info table[IWL_MAX_RATES];
} __attribute__ ((packed));



#define  LINK_QUAL_FLAGS_SET_STA_TLC_RTS_MSK	(1 << 0)


#define  LINK_QUAL_AC_NUM AC_NUM


#define  LINK_QUAL_MAX_RETRY_NUM 16


#define  LINK_QUAL_ANT_A_MSK (1 << 0)
#define  LINK_QUAL_ANT_B_MSK (1 << 1)
#define  LINK_QUAL_ANT_MSK   (LINK_QUAL_ANT_A_MSK|LINK_QUAL_ANT_B_MSK)



struct iwl_link_qual_general_params {
	u8 flags;

	
	u8 mimo_delimiter;

	
	u8 single_stream_ant_msk;	

	
	u8 dual_stream_ant_msk;		

	
	u8 start_rate_index[LINK_QUAL_AC_NUM];
} __attribute__ ((packed));

#define LINK_QUAL_AGG_TIME_LIMIT_DEF	(4000) 
#define LINK_QUAL_AGG_TIME_LIMIT_MAX	(65535)
#define LINK_QUAL_AGG_TIME_LIMIT_MIN	(0)

#define LINK_QUAL_AGG_DISABLE_START_DEF	(3)
#define LINK_QUAL_AGG_DISABLE_START_MAX	(255)
#define LINK_QUAL_AGG_DISABLE_START_MIN	(0)

#define LINK_QUAL_AGG_FRAME_LIMIT_DEF	(31)
#define LINK_QUAL_AGG_FRAME_LIMIT_MAX	(63)
#define LINK_QUAL_AGG_FRAME_LIMIT_MIN	(0)


struct iwl_link_qual_agg_params {

	
	__le16 agg_time_limit;

	
	u8 agg_dis_start_th;

	
	u8 agg_frame_cnt_limit;

	__le32 reserved;
} __attribute__ ((packed));


struct iwl_link_quality_cmd {

	
	u8 sta_id;
	u8 reserved1;
	__le16 control;		
	struct iwl_link_qual_general_params general_params;
	struct iwl_link_qual_agg_params agg_params;

	
	struct {
		__le32 rate_n_flags;	
	} rs_table[LINK_QUAL_MAX_RETRY_NUM];
	__le32 reserved2;
} __attribute__ ((packed));


struct iwl_bt_cmd {
	u8 flags;
	u8 lead_time;
	u8 max_kill;
	u8 reserved;
	__le32 kill_ack_mask;
	__le32 kill_cts_mask;
} __attribute__ ((packed));




#define MEASUREMENT_FILTER_FLAG (RXON_FILTER_PROMISC_MSK         | \
				 RXON_FILTER_CTL2HOST_MSK        | \
				 RXON_FILTER_ACCEPT_GRP_MSK      | \
				 RXON_FILTER_DIS_DECRYPT_MSK     | \
				 RXON_FILTER_DIS_GRP_DECRYPT_MSK | \
				 RXON_FILTER_ASSOC_MSK           | \
				 RXON_FILTER_BCON_AWARE_MSK)

struct iwl_measure_channel {
	__le32 duration;	
	u8 channel;		
	u8 type;		
	__le16 reserved;
} __attribute__ ((packed));


struct iwl_spectrum_cmd {
	__le16 len;		
	u8 token;		
	u8 id;			
	u8 origin;		
	u8 periodic;		
	__le16 path_loss_timeout;
	__le32 start_time;	
	__le32 reserved2;
	__le32 flags;		
	__le32 filter_flags;	
	__le16 channel_count;	
	__le16 reserved3;
	struct iwl_measure_channel channels[10];
} __attribute__ ((packed));


struct iwl_spectrum_resp {
	u8 token;
	u8 id;			
	__le16 status;		
} __attribute__ ((packed));

enum iwl_measurement_state {
	IWL_MEASUREMENT_START = 0,
	IWL_MEASUREMENT_STOP = 1,
};

enum iwl_measurement_status {
	IWL_MEASUREMENT_OK = 0,
	IWL_MEASUREMENT_CONCURRENT = 1,
	IWL_MEASUREMENT_CSA_CONFLICT = 2,
	IWL_MEASUREMENT_TGH_CONFLICT = 3,
	
	IWL_MEASUREMENT_STOPPED = 6,
	IWL_MEASUREMENT_TIMEOUT = 7,
	IWL_MEASUREMENT_PERIODIC_FAILED = 8,
};

#define NUM_ELEMENTS_IN_HISTOGRAM 8

struct iwl_measurement_histogram {
	__le32 ofdm[NUM_ELEMENTS_IN_HISTOGRAM];	
	__le32 cck[NUM_ELEMENTS_IN_HISTOGRAM];	
} __attribute__ ((packed));


struct iwl_measurement_cca_counters {
	__le32 ofdm;
	__le32 cck;
} __attribute__ ((packed));

enum iwl_measure_type {
	IWL_MEASURE_BASIC = (1 << 0),
	IWL_MEASURE_CHANNEL_LOAD = (1 << 1),
	IWL_MEASURE_HISTOGRAM_RPI = (1 << 2),
	IWL_MEASURE_HISTOGRAM_NOISE = (1 << 3),
	IWL_MEASURE_FRAME = (1 << 4),
	
	IWL_MEASURE_IDLE = (1 << 7),
};


struct iwl_spectrum_notification {
	u8 id;			
	u8 token;
	u8 channel_index;	
	u8 state;		
	__le32 start_time;	
	u8 band;		
	u8 channel;
	u8 type;		
	u8 reserved1;
	
	__le32 cca_ofdm;	
	__le32 cca_cck;		
	__le32 cca_time;	
	u8 basic_type;		
	u8 reserved2[3];
	struct iwl_measurement_histogram histogram;
	__le32 stop_time;	
	__le32 status;		
} __attribute__ ((packed));




#define IWL_POWER_VEC_SIZE 5

#define IWL_POWER_DRIVER_ALLOW_SLEEP_MSK	cpu_to_le16(BIT(0))
#define IWL_POWER_SLEEP_OVER_DTIM_MSK		cpu_to_le16(BIT(2))
#define IWL_POWER_PCI_PM_MSK			cpu_to_le16(BIT(3))
#define IWL_POWER_FAST_PD			cpu_to_le16(BIT(4))

struct iwl3945_powertable_cmd {
	__le16 flags;
	u8 reserved[2];
	__le32 rx_data_timeout;
	__le32 tx_data_timeout;
	__le32 sleep_interval[IWL_POWER_VEC_SIZE];
} __attribute__ ((packed));

struct iwl_powertable_cmd {
	__le16 flags;
	u8 keep_alive_seconds;		
	u8 debug_flags;			
	__le32 rx_data_timeout;
	__le32 tx_data_timeout;
	__le32 sleep_interval[IWL_POWER_VEC_SIZE];
	__le32 keep_alive_beacons;
} __attribute__ ((packed));


struct iwl_sleep_notification {
	u8 pm_sleep_mode;
	u8 pm_wakeup_src;
	__le16 reserved;
	__le32 sleep_time;
	__le32 tsf_low;
	__le32 bcon_timer;
} __attribute__ ((packed));


enum {
	IWL_PM_NO_SLEEP = 0,
	IWL_PM_SLP_MAC = 1,
	IWL_PM_SLP_FULL_MAC_UNASSOCIATE = 2,
	IWL_PM_SLP_FULL_MAC_CARD_STATE = 3,
	IWL_PM_SLP_PHY = 4,
	IWL_PM_SLP_REPENT = 5,
	IWL_PM_WAKEUP_BY_TIMER = 6,
	IWL_PM_WAKEUP_BY_DRIVER = 7,
	IWL_PM_WAKEUP_BY_RFKILL = 8,
	
	IWL_PM_NUM_OF_MODES = 12,
};


#define CARD_STATE_CMD_DISABLE 0x00	
#define CARD_STATE_CMD_ENABLE  0x01	
#define CARD_STATE_CMD_HALT    0x02	
struct iwl_card_state_cmd {
	__le32 status;		
} __attribute__ ((packed));


struct iwl_card_state_notif {
	__le32 flags;
} __attribute__ ((packed));

#define HW_CARD_DISABLED   0x01
#define SW_CARD_DISABLED   0x02
#define RF_CARD_DISABLED   0x04
#define RXON_CARD_DISABLED 0x10

struct iwl_ct_kill_config {
	__le32   reserved;
	__le32   critical_temperature_M;
	__le32   critical_temperature_R;
}  __attribute__ ((packed));


struct iwl_ct_kill_throttling_config {
	__le32   critical_temperature_exit;
	__le32   reserved;
	__le32   critical_temperature_enter;
}  __attribute__ ((packed));



#define SCAN_CHANNEL_TYPE_PASSIVE cpu_to_le32(0)
#define SCAN_CHANNEL_TYPE_ACTIVE  cpu_to_le32(1)




struct iwl3945_scan_channel {
	
	u8 type;
	u8 channel;	
	struct iwl3945_tx_power tpc;
	__le16 active_dwell;	
	__le16 passive_dwell;	
} __attribute__ ((packed));


#define IWL39_SCAN_PROBE_MASK(n) ((BIT(n) | (BIT(n) - BIT(1))))

struct iwl_scan_channel {
	
	__le32 type;
	__le16 channel;	
	u8 tx_gain;		
	u8 dsp_atten;		
	__le16 active_dwell;	
	__le16 passive_dwell;	
} __attribute__ ((packed));


#define IWL_SCAN_PROBE_MASK(n) 	cpu_to_le32((BIT(n) | (BIT(n) - BIT(1))))


struct iwl_ssid_ie {
	u8 id;
	u8 len;
	u8 ssid[32];
} __attribute__ ((packed));

#define PROBE_OPTION_MAX_3945		4
#define PROBE_OPTION_MAX		20
#define TX_CMD_LIFE_TIME_INFINITE	cpu_to_le32(0xFFFFFFFF)
#define IWL_GOOD_CRC_TH			cpu_to_le16(1)
#define IWL_MAX_SCAN_SIZE 1024
#define IWL_MAX_PROBE_REQUEST		200



struct iwl3945_scan_cmd {
	__le16 len;
	u8 reserved0;
	u8 channel_count;	
	__le16 quiet_time;	
	__le16 quiet_plcp_th;	
	__le16 good_CRC_th;	
	__le16 reserved1;
	__le32 max_out_time;	
	__le32 suspend_time;	
	__le32 flags;		
	__le32 filter_flags;	

	
	struct iwl3945_tx_cmd tx_cmd;

	
	struct iwl_ssid_ie direct_scan[PROBE_OPTION_MAX_3945];

	
	u8 data[0];
} __attribute__ ((packed));

struct iwl_scan_cmd {
	__le16 len;
	u8 reserved0;
	u8 channel_count;	
	__le16 quiet_time;	
	__le16 quiet_plcp_th;	
	__le16 good_CRC_th;	
	__le16 rx_chain;	
	__le32 max_out_time;	
	__le32 suspend_time;	
	__le32 flags;		
	__le32 filter_flags;	

	
	struct iwl_tx_cmd tx_cmd;

	
	struct iwl_ssid_ie direct_scan[PROBE_OPTION_MAX];

	
	u8 data[0];
} __attribute__ ((packed));


#define CAN_ABORT_STATUS	cpu_to_le32(0x1)

#define ABORT_STATUS            0x2


struct iwl_scanreq_notification {
	__le32 status;		
} __attribute__ ((packed));


struct iwl_scanstart_notification {
	__le32 tsf_low;
	__le32 tsf_high;
	__le32 beacon_timer;
	u8 channel;
	u8 band;
	u8 reserved[2];
	__le32 status;
} __attribute__ ((packed));

#define  SCAN_OWNER_STATUS 0x1;
#define  MEASURE_OWNER_STATUS 0x2;

#define NUMBER_OF_STATISTICS 1	

struct iwl_scanresults_notification {
	u8 channel;
	u8 band;
	u8 reserved[2];
	__le32 tsf_low;
	__le32 tsf_high;
	__le32 statistics[NUMBER_OF_STATISTICS];
} __attribute__ ((packed));


struct iwl_scancomplete_notification {
	u8 scanned_channels;
	u8 status;
	u8 reserved;
	u8 last_channel;
	__le32 tsf_low;
	__le32 tsf_high;
} __attribute__ ((packed));






struct iwl3945_beacon_notif {
	struct iwl3945_tx_resp beacon_notify_hdr;
	__le32 low_tsf;
	__le32 high_tsf;
	__le32 ibss_mgr_status;
} __attribute__ ((packed));

struct iwl4965_beacon_notif {
	struct iwl4965_tx_resp beacon_notify_hdr;
	__le32 low_tsf;
	__le32 high_tsf;
	__le32 ibss_mgr_status;
} __attribute__ ((packed));



struct iwl3945_tx_beacon_cmd {
	struct iwl3945_tx_cmd tx;
	__le16 tim_idx;
	u8 tim_size;
	u8 reserved1;
	struct ieee80211_hdr frame[0];	
} __attribute__ ((packed));

struct iwl_tx_beacon_cmd {
	struct iwl_tx_cmd tx;
	__le16 tim_idx;
	u8 tim_size;
	u8 reserved1;
	struct ieee80211_hdr frame[0];	
} __attribute__ ((packed));



#define IWL_TEMP_CONVERT 260

#define SUP_RATE_11A_MAX_NUM_CHANNELS  8
#define SUP_RATE_11B_MAX_NUM_CHANNELS  4
#define SUP_RATE_11G_MAX_NUM_CHANNELS  12


struct rate_histogram {
	union {
		__le32 a[SUP_RATE_11A_MAX_NUM_CHANNELS];
		__le32 b[SUP_RATE_11B_MAX_NUM_CHANNELS];
		__le32 g[SUP_RATE_11G_MAX_NUM_CHANNELS];
	} success;
	union {
		__le32 a[SUP_RATE_11A_MAX_NUM_CHANNELS];
		__le32 b[SUP_RATE_11B_MAX_NUM_CHANNELS];
		__le32 g[SUP_RATE_11G_MAX_NUM_CHANNELS];
	} failed;
} __attribute__ ((packed));



struct iwl39_statistics_rx_phy {
	__le32 ina_cnt;
	__le32 fina_cnt;
	__le32 plcp_err;
	__le32 crc32_err;
	__le32 overrun_err;
	__le32 early_overrun_err;
	__le32 crc32_good;
	__le32 false_alarm_cnt;
	__le32 fina_sync_err_cnt;
	__le32 sfd_timeout;
	__le32 fina_timeout;
	__le32 unresponded_rts;
	__le32 rxe_frame_limit_overrun;
	__le32 sent_ack_cnt;
	__le32 sent_cts_cnt;
} __attribute__ ((packed));

struct iwl39_statistics_rx_non_phy {
	__le32 bogus_cts;	
	__le32 bogus_ack;	
	__le32 non_bssid_frames;	
	__le32 filtered_frames;	
	__le32 non_channel_beacons;	
} __attribute__ ((packed));

struct iwl39_statistics_rx {
	struct iwl39_statistics_rx_phy ofdm;
	struct iwl39_statistics_rx_phy cck;
	struct iwl39_statistics_rx_non_phy general;
} __attribute__ ((packed));

struct iwl39_statistics_tx {
	__le32 preamble_cnt;
	__le32 rx_detected_cnt;
	__le32 bt_prio_defer_cnt;
	__le32 bt_prio_kill_cnt;
	__le32 few_bytes_cnt;
	__le32 cts_timeout;
	__le32 ack_timeout;
	__le32 expected_ack_cnt;
	__le32 actual_ack_cnt;
} __attribute__ ((packed));

struct statistics_dbg {
	__le32 burst_check;
	__le32 burst_count;
	__le32 reserved[4];
} __attribute__ ((packed));

struct iwl39_statistics_div {
	__le32 tx_on_a;
	__le32 tx_on_b;
	__le32 exec_time;
	__le32 probe_time;
} __attribute__ ((packed));

struct iwl39_statistics_general {
	__le32 temperature;
	struct statistics_dbg dbg;
	__le32 sleep_time;
	__le32 slots_out;
	__le32 slots_idle;
	__le32 ttl_timestamp;
	struct iwl39_statistics_div div;
} __attribute__ ((packed));

struct statistics_rx_phy {
	__le32 ina_cnt;
	__le32 fina_cnt;
	__le32 plcp_err;
	__le32 crc32_err;
	__le32 overrun_err;
	__le32 early_overrun_err;
	__le32 crc32_good;
	__le32 false_alarm_cnt;
	__le32 fina_sync_err_cnt;
	__le32 sfd_timeout;
	__le32 fina_timeout;
	__le32 unresponded_rts;
	__le32 rxe_frame_limit_overrun;
	__le32 sent_ack_cnt;
	__le32 sent_cts_cnt;
	__le32 sent_ba_rsp_cnt;
	__le32 dsp_self_kill;
	__le32 mh_format_err;
	__le32 re_acq_main_rssi_sum;
	__le32 reserved3;
} __attribute__ ((packed));

struct statistics_rx_ht_phy {
	__le32 plcp_err;
	__le32 overrun_err;
	__le32 early_overrun_err;
	__le32 crc32_good;
	__le32 crc32_err;
	__le32 mh_format_err;
	__le32 agg_crc32_good;
	__le32 agg_mpdu_cnt;
	__le32 agg_cnt;
	__le32 reserved2;
} __attribute__ ((packed));

#define INTERFERENCE_DATA_AVAILABLE      cpu_to_le32(1)

struct statistics_rx_non_phy {
	__le32 bogus_cts;	
	__le32 bogus_ack;	
	__le32 non_bssid_frames;	
	__le32 filtered_frames;	
	__le32 non_channel_beacons;	
	__le32 channel_beacons;	
	__le32 num_missed_bcon;	
	__le32 adc_rx_saturation_time;	
	__le32 ina_detection_search_time;
	__le32 beacon_silence_rssi_a;	
	__le32 beacon_silence_rssi_b;	
	__le32 beacon_silence_rssi_c;	
	__le32 interference_data_flag;	
	__le32 channel_load;		
	__le32 dsp_false_alarms;	
	__le32 beacon_rssi_a;
	__le32 beacon_rssi_b;
	__le32 beacon_rssi_c;
	__le32 beacon_energy_a;
	__le32 beacon_energy_b;
	__le32 beacon_energy_c;
} __attribute__ ((packed));

struct statistics_rx {
	struct statistics_rx_phy ofdm;
	struct statistics_rx_phy cck;
	struct statistics_rx_non_phy general;
	struct statistics_rx_ht_phy ofdm_ht;
} __attribute__ ((packed));


struct statistics_tx_power {
	u8 ant_a;
	u8 ant_b;
	u8 ant_c;
	u8 reserved;
} __attribute__ ((packed));

struct statistics_tx_non_phy_agg {
	__le32 ba_timeout;
	__le32 ba_reschedule_frames;
	__le32 scd_query_agg_frame_cnt;
	__le32 scd_query_no_agg;
	__le32 scd_query_agg;
	__le32 scd_query_mismatch;
	__le32 frame_not_ready;
	__le32 underrun;
	__le32 bt_prio_kill;
	__le32 rx_ba_rsp_cnt;
} __attribute__ ((packed));

struct statistics_tx {
	__le32 preamble_cnt;
	__le32 rx_detected_cnt;
	__le32 bt_prio_defer_cnt;
	__le32 bt_prio_kill_cnt;
	__le32 few_bytes_cnt;
	__le32 cts_timeout;
	__le32 ack_timeout;
	__le32 expected_ack_cnt;
	__le32 actual_ack_cnt;
	__le32 dump_msdu_cnt;
	__le32 burst_abort_next_frame_mismatch_cnt;
	__le32 burst_abort_missing_next_frame_cnt;
	__le32 cts_timeout_collision;
	__le32 ack_or_ba_timeout_collision;
	struct statistics_tx_non_phy_agg agg;
	struct statistics_tx_power tx_power;
	__le32 reserved1;
} __attribute__ ((packed));


struct statistics_div {
	__le32 tx_on_a;
	__le32 tx_on_b;
	__le32 exec_time;
	__le32 probe_time;
	__le32 reserved1;
	__le32 reserved2;
} __attribute__ ((packed));

struct statistics_general {
	__le32 temperature;
	__le32 temperature_m;
	struct statistics_dbg dbg;
	__le32 sleep_time;
	__le32 slots_out;
	__le32 slots_idle;
	__le32 ttl_timestamp;
	struct statistics_div div;
	__le32 rx_enable_counter;
	__le32 reserved1;
	__le32 reserved2;
	__le32 reserved3;
} __attribute__ ((packed));


#define IWL_STATS_CONF_CLEAR_STATS cpu_to_le32(0x1)	
#define IWL_STATS_CONF_DISABLE_NOTIF cpu_to_le32(0x2)
struct iwl_statistics_cmd {
	__le32 configuration_flags;	
} __attribute__ ((packed));


#define STATISTICS_REPLY_FLG_BAND_24G_MSK         cpu_to_le32(0x2)
#define STATISTICS_REPLY_FLG_HT40_MODE_MSK        cpu_to_le32(0x8)

struct iwl3945_notif_statistics {
	__le32 flag;
	struct iwl39_statistics_rx rx;
	struct iwl39_statistics_tx tx;
	struct iwl39_statistics_general general;
} __attribute__ ((packed));

struct iwl_notif_statistics {
	__le32 flag;
	struct statistics_rx rx;
	struct statistics_tx tx;
	struct statistics_general general;
} __attribute__ ((packed));




#define CONSECUTIVE_MISSED_BCONS_TH 20

struct iwl_missed_beacon_notif {
	__le32 consequtive_missed_beacons;
	__le32 total_missed_becons;
	__le32 num_expected_beacons;
	__le32 num_recvd_beacons;
} __attribute__ ((packed));







#define HD_TABLE_SIZE  (11)	
#define HD_MIN_ENERGY_CCK_DET_INDEX                 (0)	
#define HD_MIN_ENERGY_OFDM_DET_INDEX                (1)
#define HD_AUTO_CORR32_X1_TH_ADD_MIN_INDEX          (2)
#define HD_AUTO_CORR32_X1_TH_ADD_MIN_MRC_INDEX      (3)
#define HD_AUTO_CORR40_X4_TH_ADD_MIN_MRC_INDEX      (4)
#define HD_AUTO_CORR32_X4_TH_ADD_MIN_INDEX          (5)
#define HD_AUTO_CORR32_X4_TH_ADD_MIN_MRC_INDEX      (6)
#define HD_BARKER_CORR_TH_ADD_MIN_INDEX             (7)
#define HD_BARKER_CORR_TH_ADD_MIN_MRC_INDEX         (8)
#define HD_AUTO_CORR40_X4_TH_ADD_MIN_INDEX          (9)
#define HD_OFDM_ENERGY_TH_IN_INDEX                  (10)


#define SENSITIVITY_CMD_CONTROL_DEFAULT_TABLE	cpu_to_le16(0)
#define SENSITIVITY_CMD_CONTROL_WORK_TABLE	cpu_to_le16(1)


struct iwl_sensitivity_cmd {
	__le16 control;			
	__le16 table[HD_TABLE_SIZE];	
} __attribute__ ((packed));






enum {
	IWL_PHY_CALIBRATE_DIFF_GAIN_CMD		= 7,
	IWL_PHY_CALIBRATE_DC_CMD		= 8,
	IWL_PHY_CALIBRATE_LO_CMD		= 9,
	IWL_PHY_CALIBRATE_RX_BB_CMD		= 10,
	IWL_PHY_CALIBRATE_TX_IQ_CMD		= 11,
	IWL_PHY_CALIBRATE_RX_IQ_CMD		= 12,
	IWL_PHY_CALIBRATION_NOISE_CMD		= 13,
	IWL_PHY_CALIBRATE_AGC_TABLE_CMD		= 14,
	IWL_PHY_CALIBRATE_CRYSTAL_FRQ_CMD	= 15,
	IWL_PHY_CALIBRATE_BASE_BAND_CMD		= 16,
	IWL_PHY_CALIBRATE_TX_IQ_PERD_CMD	= 17,
	IWL_PHY_CALIBRATE_CHAIN_NOISE_RESET_CMD	= 18,
	IWL_PHY_CALIBRATE_CHAIN_NOISE_GAIN_CMD	= 19,
};


#define IWL_CALIB_INIT_CFG_ALL	cpu_to_le32(0xffffffff)

struct iwl_calib_cfg_elmnt_s {
	__le32 is_enable;
	__le32 start;
	__le32 send_res;
	__le32 apply_res;
	__le32 reserved;
} __attribute__ ((packed));

struct iwl_calib_cfg_status_s {
	struct iwl_calib_cfg_elmnt_s once;
	struct iwl_calib_cfg_elmnt_s perd;
	__le32 flags;
} __attribute__ ((packed));

struct iwl_calib_cfg_cmd {
	struct iwl_calib_cfg_status_s ucd_calib_cfg;
	struct iwl_calib_cfg_status_s drv_calib_cfg;
	__le32 reserved1;
} __attribute__ ((packed));

struct iwl_calib_hdr {
	u8 op_code;
	u8 first_group;
	u8 groups_num;
	u8 data_valid;
} __attribute__ ((packed));

struct iwl_calib_cmd {
	struct iwl_calib_hdr hdr;
	u8 data[0];
} __attribute__ ((packed));


struct iwl_calib_diff_gain_cmd {
	struct iwl_calib_hdr hdr;
	s8 diff_gain_a;		
	s8 diff_gain_b;
	s8 diff_gain_c;
	u8 reserved1;
} __attribute__ ((packed));

struct iwl_calib_xtal_freq_cmd {
	struct iwl_calib_hdr hdr;
	u8 cap_pin1;
	u8 cap_pin2;
	u8 pad[2];
} __attribute__ ((packed));


struct iwl_calib_chain_noise_reset_cmd {
	struct iwl_calib_hdr hdr;
	u8 data[0];
};


struct iwl_calib_chain_noise_gain_cmd {
	struct iwl_calib_hdr hdr;
	u8 delta_gain_1;
	u8 delta_gain_2;
	u8 pad[2];
} __attribute__ ((packed));




struct iwl_led_cmd {
	__le32 interval;	
	u8 id;			
	u8 off;			
	u8 on;			
	u8 reserved;
} __attribute__ ((packed));


enum {
	COEX_UNASSOC_IDLE		= 0,
	COEX_UNASSOC_MANUAL_SCAN	= 1,
	COEX_UNASSOC_AUTO_SCAN		= 2,
	COEX_CALIBRATION		= 3,
	COEX_PERIODIC_CALIBRATION	= 4,
	COEX_CONNECTION_ESTAB		= 5,
	COEX_ASSOCIATED_IDLE		= 6,
	COEX_ASSOC_MANUAL_SCAN		= 7,
	COEX_ASSOC_AUTO_SCAN		= 8,
	COEX_ASSOC_ACTIVE_LEVEL		= 9,
	COEX_RF_ON			= 10,
	COEX_RF_OFF			= 11,
	COEX_STAND_ALONE_DEBUG		= 12,
	COEX_IPAN_ASSOC_LEVEL		= 13,
	COEX_RSRVD1			= 14,
	COEX_RSRVD2			= 15,
	COEX_NUM_OF_EVENTS		= 16
};

struct iwl_wimax_coex_event_entry {
	u8 request_prio;
	u8 win_medium_prio;
	u8 reserved;
	u8 flags;
} __attribute__ ((packed));




#define COEX_FLAGS_STA_TABLE_VALID_MSK      (0x1)

#define COEX_FLAGS_UNASSOC_WA_UNMASK_MSK    (0x4)

#define COEX_FLAGS_ASSOC_WA_UNMASK_MSK      (0x8)

#define COEX_FLAGS_COEX_ENABLE_MSK          (0x80)

struct iwl_wimax_coex_cmd {
	u8 flags;
	u8 reserved[3];
	struct iwl_wimax_coex_event_entry sta_prio[COEX_NUM_OF_EVENTS];
} __attribute__ ((packed));



struct iwl_rx_packet {
	__le32 len_n_flags;
	struct iwl_cmd_header hdr;
	union {
		struct iwl3945_rx_frame rx_frame;
		struct iwl3945_tx_resp tx_resp;
		struct iwl3945_beacon_notif beacon_status;

		struct iwl_alive_resp alive_frame;
		struct iwl_spectrum_notification spectrum_notif;
		struct iwl_csa_notification csa_notif;
		struct iwl_error_resp err_resp;
		struct iwl_card_state_notif card_state_notif;
		struct iwl_add_sta_resp add_sta;
		struct iwl_rem_sta_resp rem_sta;
		struct iwl_sleep_notification sleep_notif;
		struct iwl_spectrum_resp spectrum;
		struct iwl_notif_statistics stats;
		struct iwl_compressed_ba_resp compressed_ba;
		struct iwl_missed_beacon_notif missed_beacon;
		__le32 status;
		u8 raw[0];
	} u;
} __attribute__ ((packed));

int iwl_agn_check_rxon_cmd(struct iwl_priv *priv);

#endif				
