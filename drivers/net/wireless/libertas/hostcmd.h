
#ifndef _LBS_HOSTCMD_H
#define _LBS_HOSTCMD_H

#include <linux/wireless.h>
#include "11d.h"
#include "types.h"




struct txpd {
	
	union {
		
		__le32 tx_status;
		struct {
			
			u8 bss_type;
			
			u8 bss_num;
			
			__le16 reserved;
		} bss;
	} u;
	
	__le32 tx_control;
	__le32 tx_packet_location;
	
	__le16 tx_packet_length;
	
	u8 tx_dest_addr_high[2];
	
	u8 tx_dest_addr_low[4];
	
	u8 priority;
	
	u8 powermgmt;
	
	u8 pktdelay_2ms;
	
	u8 reserved1;
} __attribute__ ((packed));


struct rxpd {
	
	union {
		
		__le16 status;
		struct {
			
			u8 bss_type;
			
			u8 bss_num;
		} __attribute__ ((packed)) bss;
	} __attribute__ ((packed)) u;

	
	u8 snr;

	
	u8 rx_control;

	
	__le16 pkt_len;

	
	u8 nf;

	
	u8 rx_rate;

	
	__le32 pkt_ptr;

	
	__le32 next_rxpd_ptr;

	
	u8 priority;
	u8 reserved[3];
} __attribute__ ((packed));

struct cmd_header {
	__le16 command;
	__le16 size;
	__le16 seqnum;
	__le16 result;
} __attribute__ ((packed));

struct cmd_ctrl_node {
	struct list_head list;
	int result;
	
	int (*callback)(struct lbs_private *, unsigned long, struct cmd_header *);
	unsigned long callback_arg;
	
	struct cmd_header *cmdbuf;
	
	u16 cmdwaitqwoken;
	wait_queue_head_t cmdwait_q;
};


struct enc_key {
	u16 len;
	u16 flags;  
	u16 type; 
	u8 key[32];
};


struct lbs_offset_value {
	u32 offset;
	u32 value;
} __attribute__ ((packed));



struct cmd_ds_gen {
	__le16 command;
	__le16 size;
	__le16 seqnum;
	__le16 result;
	void *cmdresp[0];
} __attribute__ ((packed));

#define S_DS_GEN sizeof(struct cmd_ds_gen)



struct cmd_ds_get_hw_spec {
	struct cmd_header hdr;

	
	__le16 hwifversion;
	
	__le16 version;
	
	__le16 nr_txpd;
	
	__le16 nr_mcast_adr;
	
	u8 permanentaddr[6];

	
	__le16 regioncode;

	
	__le16 nr_antenna;

	
	__le32 fwrelease;

	
	__le32 wcb_base;
	
	__le32 rxpd_rdptr;

	
	__le32 rxpd_wrptr;

	
	__le32 fwcapinfo;
} __attribute__ ((packed));

struct cmd_ds_802_11_subscribe_event {
	struct cmd_header hdr;

	__le16 action;
	__le16 events;

	
	uint8_t tlv[128];
} __attribute__ ((packed));


struct cmd_ds_802_11_scan {
	struct cmd_header hdr;

	uint8_t bsstype;
	uint8_t bssid[ETH_ALEN];
	uint8_t tlvbuffer[0];
#if 0
	mrvlietypes_ssidparamset_t ssidParamSet;
	mrvlietypes_chanlistparamset_t ChanListParamSet;
	mrvlietypes_ratesparamset_t OpRateSet;
#endif
} __attribute__ ((packed));

struct cmd_ds_802_11_scan_rsp {
	struct cmd_header hdr;

	__le16 bssdescriptsize;
	uint8_t nr_sets;
	uint8_t bssdesc_and_tlvbuffer[0];
} __attribute__ ((packed));

struct cmd_ds_802_11_get_log {
	struct cmd_header hdr;

	__le32 mcasttxframe;
	__le32 failed;
	__le32 retry;
	__le32 multiretry;
	__le32 framedup;
	__le32 rtssuccess;
	__le32 rtsfailure;
	__le32 ackfailure;
	__le32 rxfrag;
	__le32 mcastrxframe;
	__le32 fcserror;
	__le32 txframe;
	__le32 wepundecryptable;
} __attribute__ ((packed));

struct cmd_ds_mac_control {
	struct cmd_header hdr;
	__le16 action;
	u16 reserved;
} __attribute__ ((packed));

struct cmd_ds_mac_multicast_adr {
	struct cmd_header hdr;
	__le16 action;
	__le16 nr_of_adrs;
	u8 maclist[ETH_ALEN * MRVDRV_MAX_MULTICAST_LIST_SIZE];
} __attribute__ ((packed));

struct cmd_ds_gspi_bus_config {
	struct cmd_header hdr;
	__le16 action;
	__le16 bus_delay_mode;
	__le16 host_time_delay_to_read_port;
	__le16 host_time_delay_to_read_register;
} __attribute__ ((packed));

struct cmd_ds_802_11_authenticate {
	struct cmd_header hdr;

	u8 bssid[ETH_ALEN];
	u8 authtype;
	u8 reserved[10];
} __attribute__ ((packed));

struct cmd_ds_802_11_deauthenticate {
	struct cmd_header hdr;

	u8 macaddr[ETH_ALEN];
	__le16 reasoncode;
} __attribute__ ((packed));

struct cmd_ds_802_11_associate {
	struct cmd_header hdr;

	u8 bssid[6];
	__le16 capability;
	__le16 listeninterval;
	__le16 bcnperiod;
	u8 dtimperiod;
	u8 iebuf[512];    
} __attribute__ ((packed));

struct cmd_ds_802_11_associate_response {
	struct cmd_header hdr;

	__le16 capability;
	__le16 statuscode;
	__le16 aid;
	u8 iebuf[512];
} __attribute__ ((packed));

struct cmd_ds_802_11_set_wep {
	struct cmd_header hdr;

	
	__le16 action;

	
	__le16 keyindex;

	
	uint8_t keytype[4];
	uint8_t keymaterial[4][16];
} __attribute__ ((packed));

struct cmd_ds_802_3_get_stat {
	__le32 xmitok;
	__le32 rcvok;
	__le32 xmiterror;
	__le32 rcverror;
	__le32 rcvnobuffer;
	__le32 rcvcrcerror;
} __attribute__ ((packed));

struct cmd_ds_802_11_get_stat {
	__le32 txfragmentcnt;
	__le32 mcasttxframecnt;
	__le32 failedcnt;
	__le32 retrycnt;
	__le32 Multipleretrycnt;
	__le32 rtssuccesscnt;
	__le32 rtsfailurecnt;
	__le32 ackfailurecnt;
	__le32 frameduplicatecnt;
	__le32 rxfragmentcnt;
	__le32 mcastrxframecnt;
	__le32 fcserrorcnt;
	__le32 bcasttxframecnt;
	__le32 bcastrxframecnt;
	__le32 txbeacon;
	__le32 rxbeacon;
	__le32 wepundecryptable;
} __attribute__ ((packed));

struct cmd_ds_802_11_snmp_mib {
	struct cmd_header hdr;

	__le16 action;
	__le16 oid;
	__le16 bufsize;
	u8 value[128];
} __attribute__ ((packed));

struct cmd_ds_mac_reg_map {
	__le16 buffersize;
	u8 regmap[128];
	__le16 reserved;
} __attribute__ ((packed));

struct cmd_ds_bbp_reg_map {
	__le16 buffersize;
	u8 regmap[128];
	__le16 reserved;
} __attribute__ ((packed));

struct cmd_ds_rf_reg_map {
	__le16 buffersize;
	u8 regmap[64];
	__le16 reserved;
} __attribute__ ((packed));

struct cmd_ds_mac_reg_access {
	__le16 action;
	__le16 offset;
	__le32 value;
} __attribute__ ((packed));

struct cmd_ds_bbp_reg_access {
	__le16 action;
	__le16 offset;
	u8 value;
	u8 reserved[3];
} __attribute__ ((packed));

struct cmd_ds_rf_reg_access {
	__le16 action;
	__le16 offset;
	u8 value;
	u8 reserved[3];
} __attribute__ ((packed));

struct cmd_ds_802_11_radio_control {
	struct cmd_header hdr;

	__le16 action;
	__le16 control;
} __attribute__ ((packed));

struct cmd_ds_802_11_beacon_control {
	__le16 action;
	__le16 beacon_enable;
	__le16 beacon_period;
} __attribute__ ((packed));

struct cmd_ds_802_11_sleep_params {
	struct cmd_header hdr;

	
	__le16 action;

	
	__le16 error;

	
	__le16 offset;

	
	__le16 stabletime;

	
	uint8_t calcontrol;

	
	uint8_t externalsleepclk;

	
	__le16 reserved;
} __attribute__ ((packed));

struct cmd_ds_802_11_inactivity_timeout {
	struct cmd_header hdr;

	
	__le16 action;

	
	__le16 timeout;
} __attribute__ ((packed));

struct cmd_ds_802_11_rf_channel {
	struct cmd_header hdr;

	__le16 action;
	__le16 channel;
	__le16 rftype;      
	__le16 reserved;    
	u8 channellist[32]; 
} __attribute__ ((packed));

struct cmd_ds_802_11_rssi {
	
	__le16 N;

	__le16 reserved_0;
	__le16 reserved_1;
	__le16 reserved_2;
} __attribute__ ((packed));

struct cmd_ds_802_11_rssi_rsp {
	__le16 SNR;
	__le16 noisefloor;
	__le16 avgSNR;
	__le16 avgnoisefloor;
} __attribute__ ((packed));

struct cmd_ds_802_11_mac_address {
	struct cmd_header hdr;

	__le16 action;
	u8 macadd[ETH_ALEN];
} __attribute__ ((packed));

struct cmd_ds_802_11_rf_tx_power {
	struct cmd_header hdr;

	__le16 action;
	__le16 curlevel;
	s8 maxlevel;
	s8 minlevel;
} __attribute__ ((packed));

struct cmd_ds_802_11_rf_antenna {
	__le16 action;

	
	__le16 antennamode;

} __attribute__ ((packed));

struct cmd_ds_802_11_monitor_mode {
	__le16 action;
	__le16 mode;
} __attribute__ ((packed));

struct cmd_ds_set_boot2_ver {
	struct cmd_header hdr;

	__le16 action;
	__le16 version;
} __attribute__ ((packed));

struct cmd_ds_802_11_fw_wake_method {
	struct cmd_header hdr;

	__le16 action;
	__le16 method;
} __attribute__ ((packed));

struct cmd_ds_802_11_sleep_period {
	struct cmd_header hdr;

	__le16 action;
	__le16 period;
} __attribute__ ((packed));

struct cmd_ds_802_11_ps_mode {
	__le16 action;
	__le16 nullpktinterval;
	__le16 multipledtim;
	__le16 reserved;
	__le16 locallisteninterval;
} __attribute__ ((packed));

struct cmd_confirm_sleep {
	struct cmd_header hdr;

	__le16 action;
	__le16 nullpktinterval;
	__le16 multipledtim;
	__le16 reserved;
	__le16 locallisteninterval;
} __attribute__ ((packed));

struct cmd_ds_802_11_data_rate {
	struct cmd_header hdr;

	__le16 action;
	__le16 reserved;
	u8 rates[MAX_RATES];
} __attribute__ ((packed));

struct cmd_ds_802_11_rate_adapt_rateset {
	struct cmd_header hdr;
	__le16 action;
	__le16 enablehwauto;
	__le16 bitmap;
} __attribute__ ((packed));

struct cmd_ds_802_11_ad_hoc_start {
	struct cmd_header hdr;

	u8 ssid[IW_ESSID_MAX_SIZE];
	u8 bsstype;
	__le16 beaconperiod;
	u8 dtimperiod;   
	struct ieee_ie_ibss_param_set ibss;
	u8 reserved1[4];
	struct ieee_ie_ds_param_set ds;
	u8 reserved2[4];
	__le16 probedelay;  
	__le16 capability;
	u8 rates[MAX_RATES];
	u8 tlv_memory_size_pad[100];
} __attribute__ ((packed));

struct cmd_ds_802_11_ad_hoc_result {
	struct cmd_header hdr;

	u8 pad[3];
	u8 bssid[ETH_ALEN];
} __attribute__ ((packed));

struct adhoc_bssdesc {
	u8 bssid[ETH_ALEN];
	u8 ssid[IW_ESSID_MAX_SIZE];
	u8 type;
	__le16 beaconperiod;
	u8 dtimperiod;
	__le64 timestamp;
	__le64 localtime;
	struct ieee_ie_ds_param_set ds;
	u8 reserved1[4];
	struct ieee_ie_ibss_param_set ibss;
	u8 reserved2[4];
	__le16 capability;
	u8 rates[MAX_RATES];

	
} __attribute__ ((packed));

struct cmd_ds_802_11_ad_hoc_join {
	struct cmd_header hdr;

	struct adhoc_bssdesc bss;
	__le16 failtimeout;   
	__le16 probedelay;    
} __attribute__ ((packed));

struct cmd_ds_802_11_ad_hoc_stop {
	struct cmd_header hdr;
} __attribute__ ((packed));

struct cmd_ds_802_11_enable_rsn {
	struct cmd_header hdr;

	__le16 action;
	__le16 enable;
} __attribute__ ((packed));

struct MrvlIEtype_keyParamSet {
	
	__le16 type;

	
	__le16 length;

	
	__le16 keytypeid;

	
	__le16 keyinfo;

	
	__le16 keylen;

	
	u8 key[32];
} __attribute__ ((packed));

#define MAX_WOL_RULES 		16

struct host_wol_rule {
	uint8_t rule_no;
	uint8_t rule_ops;
	__le16 sig_offset;
	__le16 sig_length;
	__le16 reserve;
	__be32 sig_mask;
	__be32 signature;
} __attribute__ ((packed));

struct wol_config {
	uint8_t action;
	uint8_t pattern;
	uint8_t no_rules_in_cmd;
	uint8_t result;
	struct host_wol_rule rule[MAX_WOL_RULES];
} __attribute__ ((packed));

struct cmd_ds_host_sleep {
	struct cmd_header hdr;
	__le32 criteria;
	uint8_t gpio;
	uint16_t gap;
	struct wol_config wol_conf;
} __attribute__ ((packed));



struct cmd_ds_802_11_key_material {
	struct cmd_header hdr;

	__le16 action;
	struct MrvlIEtype_keyParamSet keyParamSet[2];
} __attribute__ ((packed));

struct cmd_ds_802_11_eeprom_access {
	struct cmd_header hdr;
	__le16 action;
	__le16 offset;
	__le16 len;
	
#define LBS_EEPROM_READ_LEN 20
	u8 value[LBS_EEPROM_READ_LEN];
} __attribute__ ((packed));

struct cmd_ds_802_11_tpc_cfg {
	struct cmd_header hdr;

	__le16 action;
	uint8_t enable;
	int8_t P0;
	int8_t P1;
	int8_t P2;
	uint8_t usesnr;
} __attribute__ ((packed));


struct cmd_ds_802_11_pa_cfg {
	struct cmd_header hdr;

	__le16 action;
	uint8_t enable;
	int8_t P0;
	int8_t P1;
	int8_t P2;
} __attribute__ ((packed));


struct cmd_ds_802_11_led_ctrl {
	__le16 action;
	__le16 numled;
	u8 data[256];
} __attribute__ ((packed));

struct cmd_ds_802_11_afc {
	__le16 afc_auto;
	union {
		struct {
			__le16 threshold;
			__le16 period;
		};
		struct {
			__le16 timing_offset; 
			__le16 carrier_offset; 
		};
	};
} __attribute__ ((packed));

struct cmd_tx_rate_query {
	__le16 txrate;
} __attribute__ ((packed));

struct cmd_ds_get_tsf {
	__le64 tsfvalue;
} __attribute__ ((packed));

struct cmd_ds_bt_access {
	__le16 action;
	__le32 id;
	u8 addr1[ETH_ALEN];
	u8 addr2[ETH_ALEN];
} __attribute__ ((packed));

struct cmd_ds_fwt_access {
	__le16 action;
	__le32 id;
	u8 valid;
	u8 da[ETH_ALEN];
	u8 dir;
	u8 ra[ETH_ALEN];
	__le32 ssn;
	__le32 dsn;
	__le32 metric;
	u8 rate;
	u8 hopcount;
	u8 ttl;
	__le32 expiration;
	u8 sleepmode;
	__le32 snr;
	__le32 references;
	u8 prec[ETH_ALEN];
} __attribute__ ((packed));


struct cmd_ds_mesh_config {
	struct cmd_header hdr;

        __le16 action;
        __le16 channel;
        __le16 type;
        __le16 length;
        u8 data[128];   
} __attribute__ ((packed));


struct cmd_ds_mesh_access {
	struct cmd_header hdr;

	__le16 action;
	__le32 data[32];	
} __attribute__ ((packed));


#define MESH_STATS_NUM 8

struct cmd_ds_command {
	
	__le16 command;
	__le16 size;
	__le16 seqnum;
	__le16 result;

	
	union {
		struct cmd_ds_802_11_ps_mode psmode;
		struct cmd_ds_802_11_get_stat gstat;
		struct cmd_ds_802_3_get_stat gstat_8023;
		struct cmd_ds_802_11_rf_antenna rant;
		struct cmd_ds_802_11_monitor_mode monitor;
		struct cmd_ds_802_11_rssi rssi;
		struct cmd_ds_802_11_rssi_rsp rssirsp;
		struct cmd_ds_mac_reg_access macreg;
		struct cmd_ds_bbp_reg_access bbpreg;
		struct cmd_ds_rf_reg_access rfreg;

		struct cmd_ds_802_11d_domain_info domaininfo;
		struct cmd_ds_802_11d_domain_info domaininforesp;

		struct cmd_ds_802_11_tpc_cfg tpccfg;
		struct cmd_ds_802_11_afc afc;
		struct cmd_ds_802_11_led_ctrl ledgpio;

		struct cmd_tx_rate_query txrate;
		struct cmd_ds_bt_access bt;
		struct cmd_ds_fwt_access fwt;
		struct cmd_ds_get_tsf gettsf;
		struct cmd_ds_802_11_beacon_control bcn_ctrl;
	} params;
} __attribute__ ((packed));

#endif
