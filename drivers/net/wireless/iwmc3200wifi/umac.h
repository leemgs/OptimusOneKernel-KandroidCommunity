

#ifndef __IWM_UMAC_H__
#define __IWM_UMAC_H__

struct iwm_udma_in_hdr {
	__le32 cmd;
	__le32 size;
} __attribute__ ((packed));

struct iwm_udma_out_nonwifi_hdr {
	__le32 cmd;
	__le32 addr;
	__le32 op1_sz;
	__le32 op2;
} __attribute__ ((packed));

struct iwm_udma_out_wifi_hdr {
	__le32 cmd;
	__le32 meta_data;
} __attribute__ ((packed));


#define UMAC_WIFI_SEQ_NUM_BASE		1
#define UMAC_WIFI_SEQ_NUM_MAX		0x4000
#define UMAC_NONWIFI_SEQ_NUM_BASE	1
#define UMAC_NONWIFI_SEQ_NUM_MAX	0x10


#define WICO_MAC_ADDRESS_ADDR               0x604008F8


#define UMAC_HDI_ACT_TBL_IDX_TID_POS                  0
#define UMAC_HDI_ACT_TBL_IDX_TID_SEED                 0xF

#define UMAC_HDI_ACT_TBL_IDX_RA_POS                   4
#define UMAC_HDI_ACT_TBL_IDX_RA_SEED                  0xF

#define UMAC_HDI_ACT_TBL_IDX_RA_UMAC                  0xF
#define UMAC_HDI_ACT_TBL_IDX_TID_UMAC                 0x9
#define UMAC_HDI_ACT_TBL_IDX_TID_LMAC                 0xA

#define UMAC_HDI_ACT_TBL_IDX_HOST_CMD \
	((UMAC_HDI_ACT_TBL_IDX_RA_UMAC << UMAC_HDI_ACT_TBL_IDX_RA_POS) |\
	 (UMAC_HDI_ACT_TBL_IDX_TID_UMAC << UMAC_HDI_ACT_TBL_IDX_TID_POS))
#define UMAC_HDI_ACT_TBL_IDX_UMAC_CMD \
	((UMAC_HDI_ACT_TBL_IDX_RA_UMAC << UMAC_HDI_ACT_TBL_IDX_RA_POS) |\
	(UMAC_HDI_ACT_TBL_IDX_TID_LMAC << UMAC_HDI_ACT_TBL_IDX_TID_POS))


#define UMAC_ALIVE_PAGE_STS_GRP_NUM_POS		0
#define UMAC_ALIVE_PAGE_STS_GRP_NUM_SEED	0xF


#define UMAC_ALIVE_PAGE_STS_SGRP_NUM_POS	4
#define UMAC_ALIVE_PAGE_STS_SGRP_NUM_SEED	0xF


#define UMAC_ALIVE_PAGE_STS_GRP_MIN_SIZE_POS	8
#define UMAC_ALIVE_PAGE_STS_GRP_MIN_SIZE_SEED	0xFF


#define UMAC_ALIVE_PAGE_STS_GRP_MAX_SIZE_POS	16
#define UMAC_ALIVE_PAGE_STS_GRP_MAX_SIZE_SEED	0xFF


#define UMAC_ALIVE_PAGE_STS_SGRP_MAX_SIZE_POS	24
#define UMAC_ALIVE_PAGE_STS_SGRP_MAX_SIZE_SEED	0xFF


#define UMAC_REBOOT_BARKER		0xdeadbeef
#define UMAC_ACK_BARKER			0xfeedbabe
#define UMAC_PAD_TERMINAL		0xadadadad


#define UMAC_MU_FW_INST_DATA_12_ADDR    0xBF0000


#define UMAC_HDI_OUT_CMD_OPCODE_POS	0
#define UMAC_HDI_OUT_CMD_OPCODE_SEED	0xF


#define UMAC_HDI_OUT_CMD_EOT_POS	10
#define UMAC_HDI_OUT_CMD_EOT_SEED	0x1


#define UMAC_HDI_OUT_CMD_UTFD_ONLY_POS	11
#define UMAC_HDI_OUT_CMD_UTFD_ONLY_SEED	0x1


#define UDMA_HDI_OUT_CMD_NON_WIFI_HW_SEQ_NUM_POS   12
#define UDMA_HDI_OUT_CMD_NON_WIFI_HW_SEQ_NUM_SEED  0xF


#define UMAC_HDI_OUT_CMD_SIGNATURE_POS	16
#define UMAC_HDI_OUT_CMD_SIGNATURE_SEED	0xFFFF


#define UMAC_HDI_OUT_BYTE_COUNT_POS	0
#define UMAC_HDI_OUT_BYTE_COUNT_SEED	0xFFF


#define UMAC_HDI_OUT_CREDIT_GRP_POS	12
#define UMAC_HDI_OUT_CREDIT_GRP_SEED	0xF


#define UMAC_HDI_OUT_RATID_POS		16
#define UMAC_HDI_OUT_RATID_SEED		0xFF


#define UMAC_HDI_OUT_LMAC_OFFSET_POS	24
#define UMAC_HDI_OUT_LMAC_OFFSET_SEED	0xFF


#define UMAC_HDI_OUT_SIGNATURE		0xCBBC


#define UMAC_HDI_BUF_ALIGN_MSK		0xF


#define UMAC_HDI_IN_CMD_OPCODE_POS                0
#define UMAC_HDI_IN_CMD_OPCODE_SEED               0xF


#define UMAC_HDI_IN_CMD_NON_WIFI_RESP_POS         4
#define UMAC_HDI_IN_CMD_NON_WIFI_RESP_SEED        0x7


#define UMAC_HDI_IN_CMD_SOURCE_POS                4
#define UMAC_HDI_IN_CMD_SOURCE_SEED               0x3


#define UMAC_HDI_IN_CMD_EOT_POS                   6
#define UMAC_HDI_IN_CMD_EOT_SEED                  0x1


#define UMAC_HDI_IN_CMD_TIME_STAMP_PRESENT_POS    7
#define UMAC_HDI_IN_CMD_TIME_STAMP_PRESENT_SEED   0x1


#define UMAC_HDI_IN_CMD_NON_LAST_AMSDU_POS        8
#define UMAC_HDI_IN_CMD_NON_LAST_AMSDU_SEED       0x1


#define UMAC_HDI_IN_CMD_HW_SEQ_NUM_POS            9
#define UMAC_HDI_IN_CMD_HW_SEQ_NUM_SEED           0x7FFFFF


#define UDMA_HDI_IN_CMD_NON_WIFI_HW_SEQ_NUM_POS   12
#define UDMA_HDI_IN_CMD_NON_WIFI_HW_SEQ_NUM_SEED  0xF


#define UDMA_HDI_IN_CMD_NON_WIFI_HW_SIG_POS       16
#define UDMA_HDI_IN_CMD_NON_WIFI_HW_SIG_SEED      0xFFFF


#define UDMA_HDI_IN_CMD_NON_WIFI_HW_SIG           0xCBBC


#define UMAC_NOTIFY_OPCODE_ALIVE		0xA1
#define UMAC_NOTIFY_OPCODE_INIT_COMPLETE	0xA2
#define UMAC_NOTIFY_OPCODE_WIFI_CORE_STATUS	0xA3
#define UMAC_NOTIFY_OPCODE_ERROR		0xA4
#define UMAC_NOTIFY_OPCODE_DEBUG		0xA5
#define UMAC_NOTIFY_OPCODE_WIFI_IF_WRAPPER	0xB0
#define UMAC_NOTIFY_OPCODE_STATS		0xB1
#define UMAC_NOTIFY_OPCODE_PAGE_DEALLOC		0xB3
#define UMAC_NOTIFY_OPCODE_RX_TICKET		0xB4
#define UMAC_NOTIFY_OPCODE_MAX		        (UMAC_NOTIFY_OPCODE_RX_TICKET -\
						UMAC_NOTIFY_OPCODE_ALIVE + 1)
#define UMAC_NOTIFY_OPCODE_FIRST		(UMAC_NOTIFY_OPCODE_ALIVE)


#define UMAC_HDI_OUT_OPCODE_PING		0x0
#define UMAC_HDI_OUT_OPCODE_READ		0x1
#define UMAC_HDI_OUT_OPCODE_WRITE		0x2
#define UMAC_HDI_OUT_OPCODE_JUMP		0x3
#define UMAC_HDI_OUT_OPCODE_REBOOT		0x4
#define UMAC_HDI_OUT_OPCODE_WRITE_PERSISTENT	0x5
#define UMAC_HDI_OUT_OPCODE_READ_PERSISTENT	0x6
#define UMAC_HDI_OUT_OPCODE_READ_MODIFY_WRITE	0x7

#define UMAC_HDI_OUT_OPCODE_WRITE_AUX_REG	0xB
#define UMAC_HDI_OUT_OPCODE_WIFI		0xF


#define UMAC_HDI_IN_OPCODE_PING			0x0
#define UMAC_HDI_IN_OPCODE_READ			0x1
#define UMAC_HDI_IN_OPCODE_WRITE		0x2
#define UMAC_HDI_IN_OPCODE_WRITE_PERSISTENT	0x5
#define UMAC_HDI_IN_OPCODE_READ_PERSISTENT	0x6
#define UMAC_HDI_IN_OPCODE_READ_MODIFY_WRITE	0x7
#define UMAC_HDI_IN_OPCODE_EP_MGMT		0x8
#define UMAC_HDI_IN_OPCODE_CREDIT_CHANGE	0x9
#define UMAC_HDI_IN_OPCODE_CTRL_DATABASE	0xA
#define UMAC_HDI_IN_OPCODE_WRITE_AUX_REG	0xB
#define UMAC_HDI_IN_OPCODE_NONWIFI_MAX \
		(UMAC_HDI_IN_OPCODE_WRITE_AUX_REG + 1)
#define UMAC_HDI_IN_OPCODE_WIFI			0xF


#define UMAC_HDI_IN_SOURCE_FHRX			0x0
#define UMAC_HDI_IN_SOURCE_UDMA			0x1
#define UMAC_HDI_IN_SOURCE_FW			0x2
#define UMAC_HDI_IN_SOURCE_RESERVED		0x3


#define UMAC_CMD_OPCODE_ECHO                    0x01
#define UMAC_CMD_OPCODE_HALT                    0x02
#define UMAC_CMD_OPCODE_RESET                   0x03
#define UMAC_CMD_OPCODE_BULK_EP_INACT_TIMEOUT   0x09
#define UMAC_CMD_OPCODE_URB_CANCEL_ACK          0x0A
#define UMAC_CMD_OPCODE_DCACHE_FLUSH            0x0B
#define UMAC_CMD_OPCODE_EEPROM_PROXY            0x0C
#define UMAC_CMD_OPCODE_TX_ECHO                 0x0D
#define UMAC_CMD_OPCODE_DBG_MON                 0x0E
#define UMAC_CMD_OPCODE_INTERNAL_TX             0x0F
#define UMAC_CMD_OPCODE_SET_PARAM_FIX           0x10
#define UMAC_CMD_OPCODE_SET_PARAM_VAR           0x11
#define UMAC_CMD_OPCODE_GET_PARAM               0x12
#define UMAC_CMD_OPCODE_DBG_EVENT_WRAPPER       0x13
#define UMAC_CMD_OPCODE_TARGET                  0x14
#define UMAC_CMD_OPCODE_STATISTIC_REQUEST       0x15
#define UMAC_CMD_OPCODE_GET_CHAN_INFO_LIST	0x16
#define UMAC_CMD_OPCODE_SET_PARAM_LIST		0x17
#define UMAC_CMD_OPCODE_GET_PARAM_LIST		0x18
#define UMAC_CMD_OPCODE_BASE_WRAPPER            0xFA
#define UMAC_CMD_OPCODE_LMAC_WRAPPER            0xFB
#define UMAC_CMD_OPCODE_HW_TEST_WRAPPER         0xFC
#define UMAC_CMD_OPCODE_WIFI_IF_WRAPPER         0xFD
#define UMAC_CMD_OPCODE_WIFI_WRAPPER            0xFE
#define UMAC_CMD_OPCODE_WIFI_PASS_THROUGH       0xFF


#define UMAC_WIFI_IF_CMD_SET_PROFILE                     0x11
#define UMAC_WIFI_IF_CMD_INVALIDATE_PROFILE              0x12
#define UMAC_WIFI_IF_CMD_SET_EXCLUDE_LIST                0x13
#define UMAC_WIFI_IF_CMD_SCAN_REQUEST                    0x14
#define UMAC_WIFI_IF_CMD_SCAN_CONFIG                     0x15
#define UMAC_WIFI_IF_CMD_ADD_WEP40_KEY                   0x16
#define UMAC_WIFI_IF_CMD_ADD_WEP104_KEY                  0x17
#define UMAC_WIFI_IF_CMD_ADD_TKIP_KEY                    0x18
#define UMAC_WIFI_IF_CMD_ADD_CCMP_KEY                    0x19
#define UMAC_WIFI_IF_CMD_REMOVE_KEY                      0x1A
#define UMAC_WIFI_IF_CMD_GLOBAL_TX_KEY_ID                0x1B
#define UMAC_WIFI_IF_CMD_SET_HOST_EXTENDED_IE            0x1C
#define UMAC_WIFI_IF_CMD_GET_SUPPORTED_CHANNELS          0x1E
#define UMAC_WIFI_IF_CMD_TX_PWR_TRIGGER                  0x20


#define UMAC_WIFI_IF_FLG_PORT_DEF                        0x00
#define UMAC_WIFI_IF_FLG_PORT_PAN                        0x01
#define UMAC_WIFI_IF_FLG_PORT_PAN_INVALID                WIFI_IF_FLG_PORT_DEF


#define UMAC_WIFI_IF_FLG_ACT_GET                         0x10
#define UMAC_WIFI_IF_FLG_ACT_SET                         0x20


#define UMAC_FW_CMD_BYTE_COUNT_POS            0
#define UMAC_FW_CMD_BYTE_COUNT_SEED           0xFFF


#define UMAC_FW_CMD_STATUS_POS                12
#define UMAC_FW_CMD_STATUS_SEED               0xF


#define UMAC_FW_CMD_TX_DRV_FULL_CMD_POS       16
#define UMAC_FW_CMD_TX_DRV_FULL_CMD_SEED      0x1


#define UMAC_FW_CMD_TX_FW_CMD_POS             17
#define UMAC_FW_CMD_TX_FW_CMD_SEED            0x1


#define UMAC_FW_CMD_TX_PLAINTEXT_POS          18
#define UMAC_FW_CMD_TX_PLAINTEXT_SEED         0x1


#define UMAC_FW_CMD_TX_STA_COLOR_POS          20
#define UMAC_FW_CMD_TX_STA_COLOR_SEED         0x7


#define UMAC_FW_CMD_TX_LIFETIME_TU_POS        24
#define UMAC_FW_CMD_TX_LIFETIME_TU_SEED       0xFF


#define UMAC_DEV_CMD_FLAGS_RESP_REQ_POS		5
#define UMAC_DEV_CMD_FLAGS_RESP_REQ_SEED	0x1


#define UMAC_DEV_CMD_FLAGS_ABORT_POS		6
#define UMAC_DEV_CMD_FLAGS_ABORT_SEED		0x1


#define DEV_CMD_FLAGS_FLD_INTERNAL_POS		7
#define DEV_CMD_FLAGS_FLD_INTERNAL_SEED		0x1



#define IWM_RX_TICKET_DROP           0x0
#define IWM_RX_TICKET_RELEASE        0x1
#define IWM_RX_TICKET_SNIFFER        0x2
#define IWM_RX_TICKET_ENQUEUE        0x3


#define IWM_RX_TICKET_PAD_SIZE_MSK        0x2
#define IWM_RX_TICKET_SPECIAL_SNAP_MSK    0x4
#define IWM_RX_TICKET_AMSDU_MSK           0x8
#define IWM_RX_TICKET_DROP_REASON_POS       4
#define IWM_RX_TICKET_DROP_REASON_MSK (0x1F << RX_TICKET_FLAGS_DROP_REASON_POS)

#define IWM_RX_DROP_NO_DROP                          0x0
#define IWM_RX_DROP_BAD_CRC                          0x1

#define IWM_RX_DROP_LMAC_ADDR_FILTER                 0x2

#define IWM_RX_DROP_MCAST_ADDR_FILTER                0x3

#define IWM_RX_DROP_CTL_FRAME                        0x4

#define IWM_RX_DROP_OUR_TX                           0x5

#define IWM_RX_DROP_CLASS_FILTER                     0x6

#define IWM_RX_DROP_DUPLICATE_FILTER                 0x7

#define IWM_RX_DROP_SEC_ERR                          0x8

#define IWM_RX_DROP_SEC_NO_ENCRYPTION                0x9

#define IWM_RX_DROP_SEC_REPLAY_ERR                   0xa

#define IWM_RX_DROP_SEC_KEY_COLOR_MISMATCH           0xb
#define IWM_RX_DROP_SEC_TKIP_COUNTER_MEASURE         0xc

#define IWM_RX_DROP_FRAG_NO_RESOURCE                 0xd

#define IWM_RX_DROP_FRAG_ERR                         0xe
#define IWM_RX_DROP_FRAG_LOST                        0xf
#define IWM_RX_DROP_FRAG_COMPLETE                    0x10

#define IWM_RX_DROP_MANAGEMENT                       0x11

#define IWM_RX_DROP_NO_STATION                       0x12

#define IWM_RX_DROP_NULL_DATA                        0x13
#define IWM_RX_DROP_BA_REORDER_OLD_SEQCTL            0x14
#define IWM_RX_DROP_BA_REORDER_DUPLICATE             0x15

struct iwm_rx_ticket {
	__le16 action;
	__le16 id;
	__le16 flags;
	u8 payload_offset; 
	u8 tail_len; 
} __attribute__ ((packed));

struct iwm_rx_mpdu_hdr {
	__le16 len;
	__le16 reserved;
} __attribute__ ((packed));



struct iwm_dev_cmd_hdr {
	u8 cmd;
	u8 flags;
	__le16 seq_num;
} __attribute__ ((packed));

struct iwm_umac_fw_cmd_hdr {
	__le32 meta_data;
	struct iwm_dev_cmd_hdr cmd;
} __attribute__ ((packed));

struct iwm_umac_wifi_out_hdr {
	struct iwm_udma_out_wifi_hdr hw_hdr;
	struct iwm_umac_fw_cmd_hdr sw_hdr;
} __attribute__ ((packed));

struct iwm_umac_nonwifi_out_hdr {
	struct iwm_udma_out_nonwifi_hdr hw_hdr;
} __attribute__ ((packed));

struct iwm_umac_wifi_in_hdr {
	struct iwm_udma_in_hdr hw_hdr;
	struct iwm_umac_fw_cmd_hdr sw_hdr;
} __attribute__ ((packed));

struct iwm_umac_nonwifi_in_hdr {
	struct iwm_udma_in_hdr hw_hdr;
	__le32 time_stamp;
} __attribute__ ((packed));

#define IWM_UMAC_PAGE_SIZE	0x200


struct iwm_fw_version {
	u8 minor;
	u8 major;
	__le16 id;
};

struct iwm_fw_build {
	u8 type;
	u8 subtype;
	u8 platform;
	u8 opt;
};

struct iwm_fw_alive_hdr {
	struct iwm_fw_version ver;
	struct iwm_fw_build build;
	__le32 os_build;
	__le32 log_hdr_addr;
	__le32 log_buf_addr;
	__le32 sys_timer_addr;
};

#define WAIT_NOTIF_TIMEOUT     (2 * HZ)
#define SCAN_COMPLETE_TIMEOUT  (3 * HZ)

#define UMAC_NTFY_ALIVE_STATUS_ERR		0xDEAD
#define UMAC_NTFY_ALIVE_STATUS_OK		0xCAFE

#define UMAC_NTFY_INIT_COMPLETE_STATUS_ERR	0xDEAD
#define UMAC_NTFY_INIT_COMPLETE_STATUS_OK	0xCAFE

#define UMAC_NTFY_WIFI_CORE_STATUS_LINK_EN      0x40
#define UMAC_NTFY_WIFI_CORE_STATUS_MLME_EN      0x80

#define IWM_MACS_OUT_GROUPS	6
#define IWM_MACS_OUT_SGROUPS	1


#define WIFI_IF_NTFY_ASSOC_START			0x80
#define WIFI_IF_NTFY_ASSOC_COMPLETE			0x81
#define WIFI_IF_NTFY_PROFILE_INVALIDATE_COMPLETE	0x82
#define WIFI_IF_NTFY_CONNECTION_TERMINATED		0x83
#define WIFI_IF_NTFY_SCAN_COMPLETE			0x84
#define WIFI_IF_NTFY_STA_TABLE_CHANGE			0x85
#define WIFI_IF_NTFY_EXTENDED_IE_REQUIRED		0x86
#define WIFI_IF_NTFY_RADIO_PREEMPTION			0x87
#define WIFI_IF_NTFY_BSS_TRK_TABLE_CHANGED		0x88
#define WIFI_IF_NTFY_BSS_TRK_ENTRIES_REMOVED		0x89
#define WIFI_IF_NTFY_LINK_QUALITY_STATISTICS		0x8A
#define WIFI_IF_NTFY_MGMT_FRAME				0x8B


#define WIFI_DBG_IF_NTFY_SCAN_SUPER_JOB_START		0xE0
#define WIFI_DBG_IF_NTFY_SCAN_SUPER_JOB_COMPLETE	0xE1
#define WIFI_DBG_IF_NTFY_SCAN_CHANNEL_START		0xE2
#define WIFI_DBG_IF_NTFY_SCAN_CHANNEL_RESULT		0xE3
#define WIFI_DBG_IF_NTFY_SCAN_MINI_JOB_START		0xE4
#define WIFI_DBG_IF_NTFY_SCAN_MINI_JOB_COMPLETE		0xE5
#define WIFI_DBG_IF_NTFY_CNCT_ATC_START			0xE6
#define WIFI_DBG_IF_NTFY_COEX_NOTIFICATION		0xE7
#define WIFI_DBG_IF_NTFY_COEX_HANDLE_ENVELOP		0xE8
#define WIFI_DBG_IF_NTFY_COEX_HANDLE_RELEASE_ENVELOP	0xE9

#define WIFI_IF_NTFY_MAX 0xff


struct iwm_umac_notif_wifi_if {
	struct iwm_umac_wifi_in_hdr hdr;
	u8 status;
	u8 flags;
	__le16 buf_size;
} __attribute__ ((packed));

#define UMAC_ROAM_REASON_FIRST_SELECTION	0x1
#define UMAC_ROAM_REASON_AP_DEAUTH		0x2
#define UMAC_ROAM_REASON_AP_CONNECT_LOST	0x3
#define UMAC_ROAM_REASON_RSSI			0x4
#define UMAC_ROAM_REASON_AP_ASSISTED_ROAM	0x5
#define UMAC_ROAM_REASON_IBSS_COALESCING	0x6

struct iwm_umac_notif_assoc_start {
	struct iwm_umac_notif_wifi_if mlme_hdr;
	__le32 roam_reason;
	u8 bssid[ETH_ALEN];
	u8 reserved[2];
} __attribute__ ((packed));

#define UMAC_ASSOC_COMPLETE_SUCCESS		0x0
#define UMAC_ASSOC_COMPLETE_FAILURE		0x1

struct iwm_umac_notif_assoc_complete {
	struct iwm_umac_notif_wifi_if mlme_hdr;
	__le32 status;
	u8 bssid[ETH_ALEN];
	u8 band;
	u8 channel;
} __attribute__ ((packed));

#define UMAC_PROFILE_INVALID_ASSOC_TIMEOUT	0x0
#define UMAC_PROFILE_INVALID_ROAM_TIMEOUT	0x1
#define UMAC_PROFILE_INVALID_REQUEST		0x2
#define UMAC_PROFILE_INVALID_RF_PREEMPTED	0x3

struct iwm_umac_notif_profile_invalidate {
	struct iwm_umac_notif_wifi_if mlme_hdr;
	__le32 reason;
} __attribute__ ((packed));

#define UMAC_SCAN_RESULT_SUCCESS  0x0
#define UMAC_SCAN_RESULT_ABORTED  0x1
#define UMAC_SCAN_RESULT_REJECTED 0x2
#define UMAC_SCAN_RESULT_FAILED   0x3

struct iwm_umac_notif_scan_complete {
	struct iwm_umac_notif_wifi_if mlme_hdr;
	__le32 type;
	__le32 result;
	u8 seq_num;
} __attribute__ ((packed));

#define UMAC_OPCODE_ADD_MODIFY	0x0
#define UMAC_OPCODE_REMOVE	0x1
#define UMAC_OPCODE_CLEAR_ALL	0x2

#define UMAC_STA_FLAG_QOS	0x1

struct iwm_umac_notif_sta_info {
	struct iwm_umac_notif_wifi_if mlme_hdr;
	__le32 opcode;
	u8 mac_addr[ETH_ALEN];
	u8 sta_id; 
	u8 flags;
} __attribute__ ((packed));

#define UMAC_BAND_2GHZ 0
#define UMAC_BAND_5GHZ 1

#define UMAC_CHANNEL_WIDTH_20MHZ 0
#define UMAC_CHANNEL_WIDTH_40MHZ 1

struct iwm_umac_notif_bss_info {
	struct iwm_umac_notif_wifi_if mlme_hdr;
	__le32 type;
	__le32 timestamp;
	__le16 table_idx;
	__le16 frame_len;
	u8 band;
	u8 channel;
	s8 rssi;
	u8 reserved;
	u8 frame_buf[1];
} __attribute__ ((packed));

#define IWM_BSS_REMOVE_INDEX_MSK           0x0fff
#define IWM_BSS_REMOVE_FLAGS_MSK           0xfc00

#define IWM_BSS_REMOVE_FLG_AGE             0x1000
#define IWM_BSS_REMOVE_FLG_TIMEOUT         0x2000
#define IWM_BSS_REMOVE_FLG_TABLE_FULL      0x4000

struct iwm_umac_notif_bss_removed {
	struct iwm_umac_notif_wifi_if mlme_hdr;
	__le32 count;
	__le16 entries[0];
} __attribute__ ((packed));

struct iwm_umac_notif_mgt_frame {
	struct iwm_umac_notif_wifi_if mlme_hdr;
	__le16 len;
	u8 frame[1];
} __attribute__ ((packed));

struct iwm_umac_notif_alive {
	struct iwm_umac_wifi_in_hdr hdr;
	__le16 status;
	__le16 reserved1;
	struct iwm_fw_alive_hdr alive_data;
	__le16 reserved2;
	__le16 page_grp_count;
	__le32 page_grp_state[IWM_MACS_OUT_GROUPS];
} __attribute__ ((packed));

struct iwm_umac_notif_init_complete {
	struct iwm_umac_wifi_in_hdr hdr;
	__le16 status;
	__le16 reserved;
} __attribute__ ((packed));


enum {
	UMAC_SYS_ERR_CAT_NONE = 0,
	UMAC_SYS_ERR_CAT_BOOT,
	UMAC_SYS_ERR_CAT_UMAC,
	UMAC_SYS_ERR_CAT_UAXM,
	UMAC_SYS_ERR_CAT_LMAC,
	UMAC_SYS_ERR_CAT_MAX
};

struct iwm_fw_error_hdr {
	__le32 category;
	__le32 status;
	__le32 pc;
	__le32 blink1;
	__le32 blink2;
	__le32 ilink1;
	__le32 ilink2;
	__le32 data1;
	__le32 data2;
	__le32 line_num;
	__le32 umac_status;
	__le32 lmac_status;
	__le32 sdio_status;
	__le32 dbm_sample_ctrl;
	__le32 dbm_buf_base;
	__le32 dbm_buf_end;
	__le32 dbm_buf_write_ptr;
	__le32 dbm_buf_cycle_cnt;
} __attribute__ ((packed));

struct iwm_umac_notif_error {
	struct iwm_umac_wifi_in_hdr hdr;
	struct iwm_fw_error_hdr err;
} __attribute__ ((packed));

#define UMAC_DEALLOC_NTFY_CHANGES_CNT_POS	0
#define UMAC_DEALLOC_NTFY_CHANGES_CNT_SEED	0xff
#define UMAC_DEALLOC_NTFY_CHANGES_MSK_POS	8
#define UMAC_DEALLOC_NTFY_CHANGES_MSK_SEED	0xffffff
#define UMAC_DEALLOC_NTFY_PAGE_CNT_POS		0
#define UMAC_DEALLOC_NTFY_PAGE_CNT_SEED		0xffffff
#define UMAC_DEALLOC_NTFY_GROUP_NUM_POS		24
#define UMAC_DEALLOC_NTFY_GROUP_NUM_SEED	0xf

struct iwm_umac_notif_page_dealloc {
	struct iwm_umac_wifi_in_hdr hdr;
	__le32 changes;
	__le32 grp_info[IWM_MACS_OUT_GROUPS];
} __attribute__ ((packed));

struct iwm_umac_notif_wifi_status {
	struct iwm_umac_wifi_in_hdr hdr;
	__le16 status;
	__le16 reserved;
} __attribute__ ((packed));

struct iwm_umac_notif_rx_ticket {
	struct iwm_umac_wifi_in_hdr hdr;
	u8 num_tickets;
	u8 reserved[3];
	struct iwm_rx_ticket tickets[1];
} __attribute__ ((packed));


#define UMAC_NTF_RATE_SAMPLE_NR	4

#define IWM_UMAC_MGMT_TID	8
#define IWM_UMAC_TID_NR		8

struct iwm_umac_notif_stats {
	struct iwm_umac_wifi_in_hdr hdr;
	__le32 flags;
	__le32 timestamp;
	__le16 tid_load[IWM_UMAC_TID_NR + 2]; 
	__le16 tx_rate[UMAC_NTF_RATE_SAMPLE_NR];
	__le16 rx_rate[UMAC_NTF_RATE_SAMPLE_NR];
	s32 rssi_dbm;
	s32 noise_dbm;
	__le32 supp_rates;
	__le32 missed_beacons;
	__le32 rx_beacons;
	__le32 rx_dir_pkts;
	__le32 rx_nondir_pkts;
	__le32 rx_multicast;
	__le32 rx_errors;
	__le32 rx_drop_other_bssid;
	__le32 rx_drop_decode;
	__le32 rx_drop_reassembly;
	__le32 rx_drop_bad_len;
	__le32 rx_drop_overflow;
	__le32 rx_drop_crc;
	__le32 rx_drop_missed;
	__le32 tx_dir_pkts;
	__le32 tx_nondir_pkts;
	__le32 tx_failure;
	__le32 tx_errors;
	__le32 tx_drop_max_retry;
	__le32 tx_err_abort;
	__le32 tx_err_carrier;
	__le32 rx_bytes;
	__le32 tx_bytes;
	__le32 tx_power;
	__le32 tx_max_power;
	__le32 roam_threshold;
	__le32 ap_assoc_nr;
	__le32 scan_full;
	__le32 scan_abort;
	__le32 ap_nr;
	__le32 roam_nr;
	__le32 roam_missed_beacons;
	__le32 roam_rssi;
	__le32 roam_unassoc;
	__le32 roam_deauth;
	__le32 roam_ap_loadblance;
} __attribute__ ((packed));


struct iwm_umac_wifi_if {
	u8 oid;
	u8 flags;
	__le16 buf_size;
} __attribute__ ((packed));

#define IWM_SEQ_NUM_HOST_MSK	0x0000
#define IWM_SEQ_NUM_UMAC_MSK	0x4000
#define IWM_SEQ_NUM_LMAC_MSK	0x8000
#define IWM_SEQ_NUM_MSK		0xC000

#endif
