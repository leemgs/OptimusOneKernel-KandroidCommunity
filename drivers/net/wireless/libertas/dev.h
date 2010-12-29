
#ifndef _LBS_DEV_H_
#define _LBS_DEV_H_

#include <linux/netdevice.h>
#include <linux/wireless.h>
#include <linux/ethtool.h>
#include <linux/debugfs.h>

#include "defs.h"
#include "hostcmd.h"

extern const struct ethtool_ops lbs_ethtool_ops;

#define	MAX_BSSID_PER_CHANNEL		16

#define NR_TX_QUEUE			3


#define MAX_EXTENDED_SCAN_BSSID_LIST    MAX_BSSID_PER_CHANNEL * \
						MRVDRV_MAX_CHANNEL_SIZE + 1

#define	MAX_REGION_CHANNEL_NUM	2


struct chan_freq_power {
	
	u16 channel;
	
	u32 freq;
	
	u16 maxtxpower;
	
	u8 unsupported;
};


struct region_channel {
	
	u8 valid;
	
	u8 region;
	
	u8 band;
	
	u8 nrcfp;
	
	struct chan_freq_power *CFP;
};

struct lbs_802_11_security {
	u8 WPAenabled;
	u8 WPA2enabled;
	u8 wep_enabled;
	u8 auth_mode;
	u32 key_mgmt;
};


struct current_bss_params {
	
	u8 bssid[ETH_ALEN];
	
	u8 ssid[IW_ESSID_MAX_SIZE + 1];
	u8 ssid_len;

	
	u8 band;
	
	u8 channel;
	
	u8 rates[MAX_RATES + 1];
};


struct sleep_params {
	uint16_t sp_error;
	uint16_t sp_offset;
	uint16_t sp_stabletime;
	uint8_t  sp_calcontrol;
	uint8_t  sp_extsleepclk;
	uint16_t sp_reserved;
};


struct lbs_mesh_stats {
	u32	fwd_bcast_cnt;		
	u32	fwd_unicast_cnt;	
	u32	fwd_drop_ttl;		
	u32	fwd_drop_rbt;		
	u32	fwd_drop_noroute; 	
	u32	fwd_drop_nobuf;		
	u32	drop_blind;		
	u32	tx_failed_cnt;		
};


struct lbs_private {
	int mesh_open;
	int mesh_fw_ver;
	int infra_open;
	int mesh_autostart_enabled;

	char name[DEV_NAME_LEN];

	void *card;
	struct net_device *dev;

	struct net_device *mesh_dev; 
	struct net_device *rtap_net_dev;

	struct iw_statistics wstats;
	struct lbs_mesh_stats mstats;
	struct dentry *debugfs_dir;
	struct dentry *debugfs_debug;
	struct dentry *debugfs_files[6];

	struct dentry *events_dir;
	struct dentry *debugfs_events_files[6];

	struct dentry *regs_dir;
	struct dentry *debugfs_regs_files[6];

	u32 mac_offset;
	u32 bbp_offset;
	u32 rf_offset;

	
	u8 dnld_sent;

	
	struct task_struct *main_thread;
	wait_queue_head_t waitq;
	struct workqueue_struct *work_thread;

	struct work_struct mcast_work;

	
	struct delayed_work scan_work;
	struct delayed_work assoc_work;
	struct work_struct sync_channel;
	
	int scan_channel;
	u8 scan_ssid[IW_ESSID_MAX_SIZE + 1];
	u8 scan_ssid_len;

	
	int (*hw_host_to_card) (struct lbs_private *priv, u8 type, u8 *payload, u16 nb);
	void (*reset_card) (struct lbs_private *priv);

	
	uint32_t wol_criteria;
	uint8_t wol_gpio;
	uint8_t wol_gap;

	
	
	u32 fwrelease;
	u32 fwcapinfo;

	struct mutex lock;

	
	int tx_pending_len;		

	u8 tx_pending_buf[LBS_UPLD_SIZE];
	

	
	u16 seqnum;

	struct cmd_ctrl_node *cmd_array;
	
	struct cmd_ctrl_node *cur_cmd;
	int cur_cmd_retcode;
	
	
	struct list_head cmdfreeq;
	
	struct list_head cmdpendingq;

	wait_queue_head_t cmd_pending;

	
	u8 resp_idx;
	u8 resp_buf[2][LBS_UPLD_SIZE];
	u32 resp_len[2];

	
	struct kfifo *event_fifo;

	
	u8 nodename[16];

	
	spinlock_t driver_lock;

	
	struct timer_list command_timer;
	int nr_retries;
	int cmd_timed_out;

	
	struct current_bss_params curbssparams;

	uint16_t mesh_tlv;
	u8 mesh_ssid[IW_ESSID_MAX_SIZE + 1];
	u8 mesh_ssid_len;

	
	u8 mode;

	
	struct list_head network_list;
	struct list_head network_free_list;
	struct bss_descriptor *networks;

	u16 beacon_period;
	u8 beacon_enable;
	u8 adhoccreate;

	
	u16 capability;

	
	u8 current_addr[ETH_ALEN];
	u8 multicastlist[MRVDRV_MAX_MULTICAST_LIST_SIZE][ETH_ALEN];
	u32 nr_of_multicastmacaddr;

	


	uint16_t enablehwauto;
	uint16_t ratebitmap;

	u8 txretrycount;

	
	struct sk_buff *currenttxskb;

	
	u16 mac_control;
	u32 connect_status;
	u32 mesh_connect_status;
	u16 regioncode;
	s16 txpower_cur;
	s16 txpower_min;
	s16 txpower_max;

	
	u8 surpriseremoved;

	u16 psmode;		
	u32 psstate;
	u8 needtowakeup;

	struct assoc_request * pending_assoc_req;
	struct assoc_request * in_progress_assoc_req;

	
	struct lbs_802_11_security secinfo;

	
	struct enc_key wep_keys[4];
	u16 wep_tx_keyidx;

	
	struct enc_key wpa_mcast_key;
	struct enc_key wpa_unicast_key;


#define MAX_WPA_IE_LEN 64

	
	u8 wpa_ie[MAX_WPA_IE_LEN];
	u8 wpa_ie_len;

	
	u16 SNR[MAX_TYPE_B][MAX_TYPE_AVG];
	u16 NF[MAX_TYPE_B][MAX_TYPE_AVG];
	u8 RSSI[MAX_TYPE_B][MAX_TYPE_AVG];
	u8 rawSNR[DEFAULT_DATA_AVG_FACTOR];
	u8 rawNF[DEFAULT_DATA_AVG_FACTOR];
	u16 nextSNRNF;
	u16 numSNRNF;

	u8 radio_on;

	
	u8 cur_rate;

	

#define	MAX_REGION_CHANNEL_NUM	2
	
	struct region_channel region_channel[MAX_REGION_CHANNEL_NUM];

	struct region_channel universal_channel[MAX_REGION_CHANNEL_NUM];

	
	struct lbs_802_11d_domain_reg domainreg;
	struct parsed_region_chan_11d parsed_region_chan;

	
	u32 enable11d;

	
	struct lbs_offset_value offsetvalue;

	u32 monitormode;
	u8 fw_ready;
};

extern struct cmd_confirm_sleep confirm_sleep;


struct bss_descriptor {
	u8 bssid[ETH_ALEN];

	u8 ssid[IW_ESSID_MAX_SIZE + 1];
	u8 ssid_len;

	u16 capability;
	u32 rssi;
	u32 channel;
	u16 beaconperiod;
	__le16 atimwindow;

	
	u8 mode;

	
	u8 rates[MAX_RATES + 1];

	unsigned long last_scanned;

	union ieee_phy_param_set phy;
	union ieee_ss_param_set ss;

	struct ieee_ie_country_info_full_set countryinfo;

	u8 wpa_ie[MAX_WPA_IE_LEN];
	size_t wpa_ie_len;
	u8 rsn_ie[MAX_WPA_IE_LEN];
	size_t rsn_ie_len;

	u8 mesh;

	struct list_head list;
};


struct assoc_request {
#define ASSOC_FLAG_SSID			1
#define ASSOC_FLAG_CHANNEL		2
#define ASSOC_FLAG_BAND			3
#define ASSOC_FLAG_MODE			4
#define ASSOC_FLAG_BSSID		5
#define ASSOC_FLAG_WEP_KEYS		6
#define ASSOC_FLAG_WEP_TX_KEYIDX	7
#define ASSOC_FLAG_WPA_MCAST_KEY	8
#define ASSOC_FLAG_WPA_UCAST_KEY	9
#define ASSOC_FLAG_SECINFO		10
#define ASSOC_FLAG_WPA_IE		11
	unsigned long flags;

	u8 ssid[IW_ESSID_MAX_SIZE + 1];
	u8 ssid_len;
	u8 channel;
	u8 band;
	u8 mode;
	u8 bssid[ETH_ALEN] __attribute__ ((aligned (2)));

	
	struct enc_key wep_keys[4];
	u16 wep_tx_keyidx;

	
	struct enc_key wpa_mcast_key;
	struct enc_key wpa_unicast_key;

	struct lbs_802_11_security secinfo;

	
	u8 wpa_ie[MAX_WPA_IE_LEN];
	u8 wpa_ie_len;

	
	struct bss_descriptor bss;
};

#endif
