
#ifndef IEEE80211_H
#define IEEE80211_H
#include <linux/if_ether.h> 
#include <linux/kernel.h>   
#include <linux/version.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/sched.h>

#include <linux/delay.h>
#include <linux/wireless.h>
#include <linux/ieee80211.h>

#include "rtl819x_HT.h"
#include "rtl819x_BA.h"
#include "rtl819x_TS.h"

#define KEY_TYPE_NA		0x0
#define KEY_TYPE_WEP40 		0x1
#define KEY_TYPE_TKIP		0x2
#define KEY_TYPE_CCMP		0x4
#define KEY_TYPE_WEP104		0x5

#define aSifsTime (((priv->ieee80211->current_network.mode == IEEE_A) || \
		    (priv->ieee80211->current_network.mode == IEEE_N_24G) || \
		    (priv->ieee80211->current_network.mode == IEEE_N_5G)) \
		   ? 16 : 10)

#define MGMT_QUEUE_NUM 5

#define IEEE_CMD_SET_WPA_PARAM			1
#define	IEEE_CMD_SET_WPA_IE			2
#define IEEE_CMD_SET_ENCRYPTION			3
#define IEEE_CMD_MLME				4

#define IEEE_PARAM_WPA_ENABLED			1
#define IEEE_PARAM_TKIP_COUNTERMEASURES		2
#define IEEE_PARAM_DROP_UNENCRYPTED		3
#define IEEE_PARAM_PRIVACY_INVOKED		4
#define IEEE_PARAM_AUTH_ALGS			5
#define IEEE_PARAM_IEEE_802_1X			6


#define IEEE_PARAM_WPAX_SELECT			7


#define IEEE_PROTO_WPA				1
#define IEEE_PROTO_RSN				2


#define IEEE_WPAX_USEGROUP			0
#define IEEE_WPAX_WEP40				1
#define IEEE_WPAX_TKIP				2
#define IEEE_WPAX_WRAP   			3
#define IEEE_WPAX_CCMP				4
#define IEEE_WPAX_WEP104			5

#define IEEE_KEY_MGMT_IEEE8021X			1
#define IEEE_KEY_MGMT_PSK			2

#define IEEE_MLME_STA_DEAUTH			1
#define IEEE_MLME_STA_DISASSOC			2


#define IEEE_CRYPT_ERR_UNKNOWN_ALG		2
#define IEEE_CRYPT_ERR_UNKNOWN_ADDR		3
#define IEEE_CRYPT_ERR_CRYPT_INIT_FAILED	4
#define IEEE_CRYPT_ERR_KEY_SET_FAILED		5
#define IEEE_CRYPT_ERR_TX_KEY_SET_FAILED	6
#define IEEE_CRYPT_ERR_CARD_CONF_FAILED		7


#define	IEEE_CRYPT_ALG_NAME_LEN			16

#define MAX_IE_LEN  0xff

typedef struct ieee_param {
	u32 cmd;
	u8 sta_addr[ETH_ALEN];
        union {
		struct {
			u8 name;
			u32 value;
		} wpa_param;
		struct {
			u32 len;
			u8 reserved[32];
			u8 data[0];
		} wpa_ie;
	        struct{
			int command;
    			int reason_code;
		} mlme;
		struct {
			u8 alg[IEEE_CRYPT_ALG_NAME_LEN];
			u8 set_tx;
			u32 err;
			u8 idx;
			u8 seq[8]; 
			u16 key_len;
			u8 key[0];
		} crypt;
	} u;
}ieee_param;

#define MSECS(t) msecs_to_jiffies(t)
#define msleep_interruptible_rsl  msleep_interruptible

#define IEEE80211_DATA_LEN		2304

#define IEEE80211_1ADDR_LEN 10
#define IEEE80211_2ADDR_LEN 16
#define IEEE80211_3ADDR_LEN 24
#define IEEE80211_4ADDR_LEN 30
#define IEEE80211_FCS_LEN    4
#define IEEE80211_HLEN			IEEE80211_4ADDR_LEN
#define IEEE80211_FRAME_LEN             (IEEE80211_DATA_LEN + IEEE80211_HLEN)
#define IEEE80211_MGMT_HDR_LEN 24
#define IEEE80211_DATA_HDR3_LEN 24
#define IEEE80211_DATA_HDR4_LEN 30

#define MIN_FRAG_THRESHOLD     256U
#define MAX_FRAG_THRESHOLD     2346U



#define IEEE80211_FCTL_FRAMETYPE	0x00fc
#define IEEE80211_FCTL_DSTODS		0x0300 
#define IEEE80211_FCTL_WEP		0x4000


#define IEEE80211_STYPE_MANAGE_ACT	0x00D0


#define IEEE80211_STYPE_BLOCKACK   0x0094


#define IEEE80211_QCTL_TID              0x000F


#define CONFIG_IEEE80211_DEBUG
#ifdef CONFIG_IEEE80211_DEBUG
extern u32 ieee80211_debug_level;
#define IEEE80211_DEBUG(level, fmt, args...) \
	do { \
		if (ieee80211_debug_level & (level)) \
			printk(KERN_DEBUG "ieee80211: " fmt, ## args); \
	} while (0)
#define IEEE80211_DEBUG_DATA(level, data, datalen) \
	do { \
		if ((ieee80211_debug_level & (level)) == (level)) { \
			u8 *pdata = (u8 *)data; \
			int i; \
			printk(KERN_DEBUG "ieee80211: %s()\n", __func__); \
			for (i = 0; i < (int)(datalen); i++) { \
				printk("%2x ", pdata[i]); \
				if ((i + 1) % 16 == 0) \
					printk("\n"); \
			} \
			printk("\n"); \
		} \
	} while (0)
#else
#define IEEE80211_DEBUG(level, fmt, args...) do {} while (0)
#define IEEE80211_DEBUG_DATA(level, data, datalen) do {} while(0)
#endif	

#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARG(x) ((u8 *)(x))[0], ((u8 *)(x))[1], ((u8 *)(x))[2], \
		   ((u8 *)(x))[3], ((u8 *)(x))[4], ((u8 *)(x))[5]



#define IEEE80211_DL_INFO          (1<<0)
#define IEEE80211_DL_WX            (1<<1)
#define IEEE80211_DL_SCAN          (1<<2)
#define IEEE80211_DL_STATE         (1<<3)
#define IEEE80211_DL_MGMT          (1<<4)
#define IEEE80211_DL_FRAG          (1<<5)
#define IEEE80211_DL_EAP           (1<<6)
#define IEEE80211_DL_DROP          (1<<7)

#define IEEE80211_DL_TX            (1<<8)
#define IEEE80211_DL_RX            (1<<9)

#define IEEE80211_DL_HT		   (1 << 10)
#define IEEE80211_DL_BA		   (1 << 11)
#define IEEE80211_DL_TS		   (1 << 12)
#define IEEE80211_DL_QOS           (1 << 13)
#define IEEE80211_DL_REORDER	   (1 << 14)
#define IEEE80211_DL_IOT	   (1 << 15)
#define IEEE80211_DL_IPS	   (1 << 16)
#define IEEE80211_DL_TRACE	   (1 << 29)
#define IEEE80211_DL_DATA	   (1 << 30)
#define IEEE80211_DL_ERR	   (1 << 31)

#define IEEE80211_ERROR(f, a...) printk(KERN_ERR "ieee80211: " f, ## a)
#define IEEE80211_WARNING(f, a...) printk(KERN_WARNING "ieee80211: " f, ## a)
#define IEEE80211_DEBUG_INFO(f, a...)   IEEE80211_DEBUG(IEEE80211_DL_INFO, f, ## a)

#define IEEE80211_DEBUG_WX(f, a...)     IEEE80211_DEBUG(IEEE80211_DL_WX, f, ## a)
#define IEEE80211_DEBUG_SCAN(f, a...)   IEEE80211_DEBUG(IEEE80211_DL_SCAN, f, ## a)
#define IEEE80211_DEBUG_STATE(f, a...)  IEEE80211_DEBUG(IEEE80211_DL_STATE, f, ## a)
#define IEEE80211_DEBUG_MGMT(f, a...)  IEEE80211_DEBUG(IEEE80211_DL_MGMT, f, ## a)
#define IEEE80211_DEBUG_FRAG(f, a...)  IEEE80211_DEBUG(IEEE80211_DL_FRAG, f, ## a)
#define IEEE80211_DEBUG_EAP(f, a...)  IEEE80211_DEBUG(IEEE80211_DL_EAP, f, ## a)
#define IEEE80211_DEBUG_DROP(f, a...)  IEEE80211_DEBUG(IEEE80211_DL_DROP, f, ## a)
#define IEEE80211_DEBUG_TX(f, a...)  IEEE80211_DEBUG(IEEE80211_DL_TX, f, ## a)
#define IEEE80211_DEBUG_RX(f, a...)  IEEE80211_DEBUG(IEEE80211_DL_RX, f, ## a)
#define IEEE80211_DEBUG_QOS(f, a...)  IEEE80211_DEBUG(IEEE80211_DL_QOS, f, ## a)

#include <linux/netdevice.h>
#include <linux/if_arp.h> 

#ifndef WIRELESS_SPY
#define WIRELESS_SPY		
#endif
#include <net/iw_handler.h>	

#ifndef ETH_P_PAE
#define ETH_P_PAE 0x888E 
#endif 

#define ETH_P_PREAUTH 0x88C7 

#ifndef ETH_P_80211_RAW
#define ETH_P_80211_RAW (ETH_P_ECONET + 1)
#endif



#define P80211_OUI_LEN 3

struct ieee80211_snap_hdr {

        u8    dsap;   
        u8    ssap;   
        u8    ctrl;   
        u8    oui[P80211_OUI_LEN];    

} __attribute__ ((packed));

#define SNAP_SIZE sizeof(struct ieee80211_snap_hdr)

#define WLAN_FC_GET_VERS(fc) ((fc) & IEEE80211_FCTL_VERS)
#define WLAN_FC_GET_TYPE(fc) ((fc) & IEEE80211_FCTL_FTYPE)
#define WLAN_FC_GET_STYPE(fc) ((fc) & IEEE80211_FCTL_STYPE)

#define WLAN_FC_GET_FRAMETYPE(fc) ((fc) & IEEE80211_FCTL_FRAMETYPE)
#define WLAN_GET_SEQ_FRAG(seq) ((seq) & IEEE80211_SCTL_FRAG)
#define WLAN_GET_SEQ_SEQ(seq)  (((seq) & IEEE80211_SCTL_SEQ) >> 4)

#define RTL_WLAN_AUTH_LEAP 2

#define WLAN_CAPABILITY_BSS (1<<0)
#define WLAN_CAPABILITY_SHORT_SLOT (1<<10)

#define IEEE80211_STATMASK_SIGNAL (1<<0)
#define IEEE80211_STATMASK_RSSI (1<<1)
#define IEEE80211_STATMASK_NOISE (1<<2)
#define IEEE80211_STATMASK_RATE (1<<3)
#define IEEE80211_STATMASK_WEMASK 0x7

#define IEEE80211_CCK_MODULATION    (1<<0)
#define IEEE80211_OFDM_MODULATION   (1<<1)

#define IEEE80211_24GHZ_BAND     (1<<0)
#define IEEE80211_52GHZ_BAND     (1<<1)

#define IEEE80211_CCK_RATE_LEN  		4
#define IEEE80211_CCK_RATE_1MB		        0x02
#define IEEE80211_CCK_RATE_2MB		        0x04
#define IEEE80211_CCK_RATE_5MB		        0x0B
#define IEEE80211_CCK_RATE_11MB		        0x16
#define IEEE80211_OFDM_RATE_LEN 		8
#define IEEE80211_OFDM_RATE_6MB		        0x0C
#define IEEE80211_OFDM_RATE_9MB		        0x12
#define IEEE80211_OFDM_RATE_12MB		0x18
#define IEEE80211_OFDM_RATE_18MB		0x24
#define IEEE80211_OFDM_RATE_24MB		0x30
#define IEEE80211_OFDM_RATE_36MB		0x48
#define IEEE80211_OFDM_RATE_48MB		0x60
#define IEEE80211_OFDM_RATE_54MB		0x6C
#define IEEE80211_BASIC_RATE_MASK		0x80

#define IEEE80211_CCK_RATE_1MB_MASK		(1<<0)
#define IEEE80211_CCK_RATE_2MB_MASK		(1<<1)
#define IEEE80211_CCK_RATE_5MB_MASK		(1<<2)
#define IEEE80211_CCK_RATE_11MB_MASK		(1<<3)
#define IEEE80211_OFDM_RATE_6MB_MASK		(1<<4)
#define IEEE80211_OFDM_RATE_9MB_MASK		(1<<5)
#define IEEE80211_OFDM_RATE_12MB_MASK		(1<<6)
#define IEEE80211_OFDM_RATE_18MB_MASK		(1<<7)
#define IEEE80211_OFDM_RATE_24MB_MASK		(1<<8)
#define IEEE80211_OFDM_RATE_36MB_MASK		(1<<9)
#define IEEE80211_OFDM_RATE_48MB_MASK		(1<<10)
#define IEEE80211_OFDM_RATE_54MB_MASK		(1<<11)

#define IEEE80211_CCK_RATES_MASK	        0x0000000F
#define IEEE80211_CCK_BASIC_RATES_MASK	(IEEE80211_CCK_RATE_1MB_MASK | \
	IEEE80211_CCK_RATE_2MB_MASK)
#define IEEE80211_CCK_DEFAULT_RATES_MASK	(IEEE80211_CCK_BASIC_RATES_MASK | \
        IEEE80211_CCK_RATE_5MB_MASK | \
        IEEE80211_CCK_RATE_11MB_MASK)

#define IEEE80211_OFDM_RATES_MASK		0x00000FF0
#define IEEE80211_OFDM_BASIC_RATES_MASK	(IEEE80211_OFDM_RATE_6MB_MASK | \
	IEEE80211_OFDM_RATE_12MB_MASK | \
	IEEE80211_OFDM_RATE_24MB_MASK)
#define IEEE80211_OFDM_DEFAULT_RATES_MASK	(IEEE80211_OFDM_BASIC_RATES_MASK | \
	IEEE80211_OFDM_RATE_9MB_MASK  | \
	IEEE80211_OFDM_RATE_18MB_MASK | \
	IEEE80211_OFDM_RATE_36MB_MASK | \
	IEEE80211_OFDM_RATE_48MB_MASK | \
	IEEE80211_OFDM_RATE_54MB_MASK)
#define IEEE80211_DEFAULT_RATES_MASK (IEEE80211_OFDM_DEFAULT_RATES_MASK | \
                                IEEE80211_CCK_DEFAULT_RATES_MASK)

#define IEEE80211_NUM_OFDM_RATES	    8
#define IEEE80211_NUM_CCK_RATES	            4
#define IEEE80211_OFDM_SHIFT_MASK_A         4



#define IEEE80211_FC0_TYPE_MASK		0x0c
#define IEEE80211_FC0_TYPE_DATA		0x08
#define IEEE80211_FC0_SUBTYPE_MASK	0xB0
#define IEEE80211_FC0_SUBTYPE_QOS	0x80

#define IEEE80211_QOS_HAS_SEQ(fc) \
	(((fc) & (IEEE80211_FC0_TYPE_MASK | IEEE80211_FC0_SUBTYPE_MASK)) == \
	 (IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_QOS))


#define IEEE_IBSS_MAC_HASH_SIZE 31
struct ieee_ibss_seq {
	u8 mac[ETH_ALEN];
	u16 seq_num[17];
	u16 frag_num[17];
	unsigned long packet_time[17];
	struct list_head list;
};


struct ieee80211_rx_stats {
	u32 mac_time[2];
	s8 rssi;
	u8 signal;
	u8 noise;
	u16 rate; 
	u8 received_channel;
	u8 control;
	u8 mask;
	u8 freq;
	u16 len;
	u64 tsf;
	u32 beacon_time;
	u8 nic_type;

	u16       Length;
	u8	  SignalQuality;	
	
	s32       RecvSignalPower;
	s8	  RxPower;		
	u8	  SignalStrength;	
	u16       bHwError:1;
	u16       bCRC:1;
	u16       bICV:1;
	u16       bShortPreamble:1;
	u16	  Antenna:1;		
	u16	  Decrypted:1;		
	u16	  Wakeup:1;		
	u16	  Reserved0:1;		
	u8        AGC;
	u32       TimeStampLow;
	u32       TimeStampHigh;
	bool      bShift;
	bool      bIsQosData;
	u8        UserPriority;

	
	u8        RxDrvInfoSize;
	u8        RxBufShift;
	bool      bIsAMPDU;
	bool      bFirstMPDU;
	bool      bContainHTC;
	bool      RxIs40MHzPacket;
	u32       RxPWDBAll;
	u8	  RxMIMOSignalStrength[4]; 
	s8        RxMIMOSignalQuality[2];
	bool      bPacketMatchBSSID;
	bool      bIsCCK;
	bool      bPacketToSelf;

	u8	  *virtual_address;
	
	u16	  packetlength;
	
	u16	  fraglength;
	u16	  fragoffset;		
	u16	  ntotalfrag;
	bool	  bisrxaggrsubframe;
	bool	  bPacketBeacon;	
	bool	  bToSelfBA;		
	char	  cck_adc_pwdb[4];	
	u16	  Seq_Num;
	u8	  nTotalAggPkt;		
};


#define IEEE80211_FRAG_CACHE_LEN 4

struct ieee80211_frag_entry {
	unsigned long first_frag_time;
	unsigned int seq;
	unsigned int last_frag;
	struct sk_buff *skb;
	u8 src_addr[ETH_ALEN];
	u8 dst_addr[ETH_ALEN];
};

struct ieee80211_stats {
	unsigned int tx_unicast_frames;
	unsigned int tx_multicast_frames;
	unsigned int tx_fragments;
	unsigned int tx_unicast_octets;
	unsigned int tx_multicast_octets;
	unsigned int tx_deferred_transmissions;
	unsigned int tx_single_retry_frames;
	unsigned int tx_multiple_retry_frames;
	unsigned int tx_retry_limit_exceeded;
	unsigned int tx_discards;
	unsigned int rx_unicast_frames;
	unsigned int rx_multicast_frames;
	unsigned int rx_fragments;
	unsigned int rx_unicast_octets;
	unsigned int rx_multicast_octets;
	unsigned int rx_fcs_errors;
	unsigned int rx_discards_no_buffer;
	unsigned int tx_discards_wrong_sa;
	unsigned int rx_discards_undecryptable;
	unsigned int rx_message_in_msg_fragments;
	unsigned int rx_message_in_bad_msg_fragments;
};

struct ieee80211_device;

#include "ieee80211_crypt.h"

#define SEC_KEY_1         (1<<0)
#define SEC_KEY_2         (1<<1)
#define SEC_KEY_3         (1<<2)
#define SEC_KEY_4         (1<<3)
#define SEC_ACTIVE_KEY    (1<<4)
#define SEC_AUTH_MODE     (1<<5)
#define SEC_UNICAST_GROUP (1<<6)
#define SEC_LEVEL         (1<<7)
#define SEC_ENABLED       (1<<8)

#define SEC_LEVEL_0      0 
#define SEC_LEVEL_1      1 
#define SEC_LEVEL_2      2 
#define SEC_LEVEL_2_CKIP 3 
#define SEC_LEVEL_3      4 

#define WEP_KEYS 		4
#define WEP_KEY_LEN		13
#define SCM_KEY_LEN             32

struct ieee80211_security {
	u16 active_key:2,
            enabled:1,
	    auth_mode:2,
            auth_algo:4,
            unicast_uses_group:1,
	    encrypt:1;
	u8 key_sizes[WEP_KEYS];
	u8 keys[WEP_KEYS][SCM_KEY_LEN];
	u8 level;
	u16 flags;
} __attribute__ ((packed));





enum {
        MFIE_TYPE_SSID = 0,
        MFIE_TYPE_RATES = 1,
        MFIE_TYPE_FH_SET = 2,
        MFIE_TYPE_DS_SET = 3,
        MFIE_TYPE_CF_SET = 4,
        MFIE_TYPE_TIM = 5,
        MFIE_TYPE_IBSS_SET = 6,
        MFIE_TYPE_COUNTRY = 7,
        MFIE_TYPE_HOP_PARAMS = 8,
        MFIE_TYPE_HOP_TABLE = 9,
        MFIE_TYPE_REQUEST = 10,
        MFIE_TYPE_CHALLENGE = 16,
        MFIE_TYPE_POWER_CONSTRAINT = 32,
        MFIE_TYPE_POWER_CAPABILITY = 33,
        MFIE_TYPE_TPC_REQUEST = 34,
        MFIE_TYPE_TPC_REPORT = 35,
        MFIE_TYPE_SUPP_CHANNELS = 36,
        MFIE_TYPE_CSA = 37,
        MFIE_TYPE_MEASURE_REQUEST = 38,
        MFIE_TYPE_MEASURE_REPORT = 39,
        MFIE_TYPE_QUIET = 40,
        MFIE_TYPE_IBSS_DFS = 41,
        MFIE_TYPE_ERP = 42,
        MFIE_TYPE_RSN = 48,
        MFIE_TYPE_RATES_EX = 50,
        MFIE_TYPE_HT_CAP= 45,
	 MFIE_TYPE_HT_INFO= 61,
	 MFIE_TYPE_AIRONET=133,
        MFIE_TYPE_GENERIC = 221,
        MFIE_TYPE_QOS_PARAMETER = 222,
};


struct rtl_ieee80211_hdr {
        __le16 frame_ctl;
        __le16 duration_id;
        u8 payload[0];
} __attribute__ ((packed));

struct ieee80211_hdr_1addr {
        __le16 frame_ctl;
        __le16 duration_id;
        u8 addr1[ETH_ALEN];
        u8 payload[0];
} __attribute__ ((packed));

struct ieee80211_hdr_2addr {
        __le16 frame_ctl;
        __le16 duration_id;
        u8 addr1[ETH_ALEN];
        u8 addr2[ETH_ALEN];
        u8 payload[0];
} __attribute__ ((packed));

struct ieee80211_hdr_3addr {
	__le16 frame_ctl;
	__le16 duration_id;
	u8 addr1[ETH_ALEN];
	u8 addr2[ETH_ALEN];
	u8 addr3[ETH_ALEN];
	__le16 seq_ctl;
        u8 payload[0];
} __attribute__ ((packed));

struct ieee80211_hdr_4addr {
	__le16 frame_ctl;
	__le16 duration_id;
	u8 addr1[ETH_ALEN];
	u8 addr2[ETH_ALEN];
	u8 addr3[ETH_ALEN];
	__le16 seq_ctl;
	u8 addr4[ETH_ALEN];
        u8 payload[0];
} __attribute__ ((packed));

struct ieee80211_hdr_3addrqos {
	__le16 frame_ctl;
	__le16 duration_id;
	u8 addr1[ETH_ALEN];
	u8 addr2[ETH_ALEN];
	u8 addr3[ETH_ALEN];
	__le16 seq_ctl;
        u8 payload[0];
	__le16 qos_ctl;
} __attribute__ ((packed));

struct ieee80211_hdr_4addrqos {
	__le16 frame_ctl;
	__le16 duration_id;
	u8 addr1[ETH_ALEN];
	u8 addr2[ETH_ALEN];
	u8 addr3[ETH_ALEN];
	__le16 seq_ctl;
	u8 addr4[ETH_ALEN];
        u8 payload[0];
	__le16 qos_ctl;
} __attribute__ ((packed));

struct ieee80211_info_element {
	u8 id;
	u8 len;
	u8 data[0];
} __attribute__ ((packed));

struct ieee80211_authentication {
	struct ieee80211_hdr_3addr header;
	__le16 algorithm;
	__le16 transaction;
	__le16 status;
	
	struct ieee80211_info_element info_element[0];
} __attribute__ ((packed));

struct ieee80211_disassoc {
        struct ieee80211_hdr_3addr header;
        __le16 reason;
} __attribute__ ((packed));

struct ieee80211_probe_request {
	struct ieee80211_hdr_3addr header;
	
        struct ieee80211_info_element info_element[0];
} __attribute__ ((packed));

struct ieee80211_probe_response {
	struct ieee80211_hdr_3addr header;
	u32 time_stamp[2];
	__le16 beacon_interval;
	__le16 capability;
        
        struct ieee80211_info_element info_element[0];
} __attribute__ ((packed));

struct ieee80211_assoc_request_frame {
	struct ieee80211_hdr_3addr header;
	__le16 capability;
	__le16 listen_interval;
	
        struct ieee80211_info_element info_element[0];
} __attribute__ ((packed));

struct ieee80211_reassoc_request_frame {
	struct ieee80211_hdr_3addr header;
	__le16 capability;
	__le16 listen_interval;
	u8 current_ap[ETH_ALEN];
	
        struct ieee80211_info_element info_element[0];
} __attribute__ ((packed));

struct ieee80211_assoc_response_frame {
	struct ieee80211_hdr_3addr header;
	__le16 capability;
	__le16 status;
	__le16 aid;
	struct ieee80211_info_element info_element[0]; 
} __attribute__ ((packed));

struct ieee80211_txb {
	u8 nr_frags;
	u8 encrypted;
	u8 queue_index;
	u8 rts_included;
	u16 reserved;
	__le16 frag_size;
	__le16 payload_size;
	struct sk_buff *fragments[0];
};

#define MAX_SUBFRAME_COUNT 		  64
struct ieee80211_rxb {
	u8 nr_subframes;
	struct sk_buff *subframes[MAX_SUBFRAME_COUNT];
	u8 dst[ETH_ALEN];
	u8 src[ETH_ALEN];
}__attribute__((packed));


#define MAX_SWEEP_TAB_ENTRIES		  42
#define MAX_SWEEP_TAB_ENTRIES_PER_PACKET  7

#define MAX_RATES_LENGTH                  ((u8)12)
#define MAX_RATES_EX_LENGTH               ((u8)16)
#define MAX_NETWORK_COUNT                  128

#define MAX_CHANNEL_NUMBER                 161

#define IEEE80211_SOFTMAC_SCAN_TIME	   100 
#define IEEE80211_SOFTMAC_ASSOC_RETRY_TIME (HZ * 2)

#define CRC_LENGTH                 4U

#define MAX_WPA_IE_LEN 64

#define NETWORK_EMPTY_ESSID	(1 << 0)
#define NETWORK_HAS_OFDM	(1 << 1)
#define NETWORK_HAS_CCK		(1 << 2)


#define NETWORK_HAS_QOS_PARAMETERS	(1 << 3)
#define NETWORK_HAS_QOS_INFORMATION	(1 << 4)
#define NETWORK_HAS_QOS_MASK		(NETWORK_HAS_QOS_PARAMETERS | \
					 NETWORK_HAS_QOS_INFORMATION)

#define NETWORK_HAS_ERP_VALUE		(1 << 10)

#define QOS_QUEUE_NUM                   4
#define QOS_OUI_LEN                     3
#define QOS_OUI_TYPE                    2
#define QOS_ELEMENT_ID                  221
#define QOS_OUI_INFO_SUB_TYPE           0
#define QOS_OUI_PARAM_SUB_TYPE          1
#define QOS_VERSION_1                   1
#define QOS_AIFSN_MIN_VALUE             2

struct ieee80211_qos_information_element {
        u8 elementID;
        u8 length;
        u8 qui[QOS_OUI_LEN];
        u8 qui_type;
        u8 qui_subtype;
        u8 version;
        u8 ac_info;
} __attribute__ ((packed));

struct ieee80211_qos_ac_parameter {
        u8 aci_aifsn;
        u8 ecw_min_max;
        __le16 tx_op_limit;
} __attribute__ ((packed));

struct ieee80211_qos_parameter_info {
        struct ieee80211_qos_information_element info_element;
        u8 reserved;
        struct ieee80211_qos_ac_parameter ac_params_record[QOS_QUEUE_NUM];
} __attribute__ ((packed));

struct ieee80211_qos_parameters {
        __le16 cw_min[QOS_QUEUE_NUM];
        __le16 cw_max[QOS_QUEUE_NUM];
        u8 aifs[QOS_QUEUE_NUM];
        u8 flag[QOS_QUEUE_NUM];
        __le16 tx_op_limit[QOS_QUEUE_NUM];
} __attribute__ ((packed));

struct ieee80211_qos_data {
        struct ieee80211_qos_parameters parameters;
        int active;
        int supported;
        u8 param_count;
        u8 old_param_count;
};

struct ieee80211_tim_parameters {
        u8 tim_count;
        u8 tim_period;
} __attribute__ ((packed));

struct ieee80211_wmm_ac_param {
	u8 ac_aci_acm_aifsn;
	u8 ac_ecwmin_ecwmax;
	u16 ac_txop_limit;
};

struct ieee80211_wmm_ts_info {
	u8 ac_dir_tid;
	u8 ac_up_psb;
	u8 reserved;
} __attribute__ ((packed));

struct ieee80211_wmm_tspec_elem {
	struct ieee80211_wmm_ts_info ts_info;
	u16 norm_msdu_size;
	u16 max_msdu_size;
	u32 min_serv_inter;
	u32 max_serv_inter;
	u32 inact_inter;
	u32 suspen_inter;
	u32 serv_start_time;
	u32 min_data_rate;
	u32 mean_data_rate;
	u32 peak_data_rate;
	u32 max_burst_size;
	u32 delay_bound;
	u32 min_phy_rate;
	u16 surp_band_allow;
	u16 medium_time;
}__attribute__((packed));

enum eap_type {
	EAP_PACKET = 0,
	EAPOL_START,
	EAPOL_LOGOFF,
	EAPOL_KEY,
	EAPOL_ENCAP_ASF_ALERT
};

static const char *eap_types[] = {
	[EAP_PACKET]		= "EAP-Packet",
	[EAPOL_START]		= "EAPOL-Start",
	[EAPOL_LOGOFF]		= "EAPOL-Logoff",
	[EAPOL_KEY]		= "EAPOL-Key",
	[EAPOL_ENCAP_ASF_ALERT]	= "EAPOL-Encap-ASF-Alert"
};

static inline const char *eap_get_type(int type)
{
	return ((u32)type >= ARRAY_SIZE(eap_types)) ? "Unknown" : eap_types[type];
}

struct eapol {
	u8 snap[6];
	u16 ethertype;
	u8 version;
	u8 type;
	u16 length;
} __attribute__ ((packed));

struct ieee80211_softmac_stats {
	unsigned int rx_ass_ok;
	unsigned int rx_ass_err;
	unsigned int rx_probe_rq;
	unsigned int tx_probe_rs;
	unsigned int tx_beacons;
	unsigned int rx_auth_rq;
	unsigned int rx_auth_rs_ok;
	unsigned int rx_auth_rs_err;
	unsigned int tx_auth_rq;
	unsigned int no_auth_rs;
	unsigned int no_ass_rs;
	unsigned int tx_ass_rq;
	unsigned int rx_ass_rq;
	unsigned int tx_probe_rq;
	unsigned int reassoc;
	unsigned int swtxstop;
	unsigned int swtxawake;
	unsigned char CurrentShowTxate;
	unsigned char last_packet_rate;
	unsigned int txretrycount;
};

#define BEACON_PROBE_SSID_ID_POSITION 12

struct ieee80211_info_element_hdr {
	u8 id;
	u8 len;
} __attribute__ ((packed));



#define IEEE80211_DEFAULT_TX_ESSID "Penguin"
#define IEEE80211_DEFAULT_BASIC_RATE 2 

enum {WMM_all_frame, WMM_two_frame, WMM_four_frame, WMM_six_frame};
#define MAX_SP_Len  (WMM_all_frame << 4)
#define IEEE80211_QOS_TID 0x0f
#define QOS_CTL_NOTCONTAIN_ACK (0x01 << 5)

#define IEEE80211_DTIM_MBCAST 4
#define IEEE80211_DTIM_UCAST 2
#define IEEE80211_DTIM_VALID 1
#define IEEE80211_DTIM_INVALID 0

#define IEEE80211_PS_DISABLED 0
#define IEEE80211_PS_UNICAST IEEE80211_DTIM_UCAST
#define IEEE80211_PS_MBCAST IEEE80211_DTIM_MBCAST



#ifdef WMM_Hang_8187
#undef WMM_Hang_8187
#endif

#define WME_AC_BK   0x00
#define WME_AC_BE   0x01
#define WME_AC_VI   0x02
#define WME_AC_VO   0x03
#define WME_ACI_MASK 0x03
#define WME_AIFSN_MASK 0x03
#define WME_AC_PRAM_LEN 16



#define UP2AC(up) (		   \
	((up) < 1) ? WME_AC_BE : \
	((up) < 3) ? WME_AC_BK : \
	((up) < 4) ? WME_AC_BE : \
	((up) < 6) ? WME_AC_VI : \
	WME_AC_VO)


#define AC2UP(_ac)	(       \
	((_ac) == WME_AC_VO) ? 6 : \
	((_ac) == WME_AC_VI) ? 5 : \
	((_ac) == WME_AC_BK) ? 1 : \
	0)

#define	ETHER_ADDR_LEN		6	


#define ETHERNET_HEADER_SIZE	14

struct	ether_header {
	u8 ether_dhost[ETHER_ADDR_LEN];
	u8 ether_shost[ETHER_ADDR_LEN];
	u16 ether_type;
} __attribute__((packed));

#ifndef ETHERTYPE_PAE
#define	ETHERTYPE_PAE	0x888e		
#endif
#ifndef ETHERTYPE_IP
#define	ETHERTYPE_IP	0x0800		
#endif

struct ieee80211_network {
	
	u8 bssid[ETH_ALEN];
	u8 channel;
	
	u8 ssid[IW_ESSID_MAX_SIZE + 1];
	u8 ssid_len;

        struct ieee80211_qos_data qos_data;

	
	bool	bWithAironetIE;
	bool	bCkipSupported;
	bool	bCcxRmEnable;
	u16 	CcxRmState[2];

	
	bool	bMBssidValid;
	u8	MBssidMask;
	u8	MBssid[6];

	
	bool	bWithCcxVerNum;
	u8	BssCcxVerNumber;

	
	struct ieee80211_rx_stats stats;
	u16 capability;
	u8  rates[MAX_RATES_LENGTH];
	u8  rates_len;
	u8  rates_ex[MAX_RATES_EX_LENGTH];
	u8  rates_ex_len;
	unsigned long last_scanned;
	u8  mode;
	u32 flags;
	u32 last_associate;
	u32 time_stamp[2];
	u16 beacon_interval;
	u16 listen_interval;
	u16 atim_window;
	u8  erp_value;
	u8  wpa_ie[MAX_WPA_IE_LEN];
	size_t wpa_ie_len;
	u8  rsn_ie[MAX_WPA_IE_LEN];
	size_t rsn_ie_len;

        struct ieee80211_tim_parameters tim;
	u8  dtim_period;
	u8  dtim_data;
	u32 last_dtim_sta_time[2];

        
        u8 wmm_info;
        struct ieee80211_wmm_ac_param wmm_param[4];
        u8 QoS_Enable;
	u8 Turbo_Enable;
	u16 CountryIeLen;
	u8 CountryIeBuf[MAX_IE_LEN];

	
	BSS_HT	bssht;
	
	bool broadcom_cap_exist;
	bool realtek_cap_exit;
	bool marvell_cap_exist;
	bool ralink_cap_exist;
	bool atheros_cap_exist;
	bool cisco_cap_exist;
	bool unknown_cap_exist;
	bool berp_info_valid;
	bool buseprotection;

	struct list_head list;	
};

enum ieee80211_state {

	
	IEEE80211_NOLINK = 0,

	

	
	IEEE80211_ASSOCIATING,
	IEEE80211_ASSOCIATING_RETRY,

	
	IEEE80211_ASSOCIATING_AUTHENTICATING,

	
	IEEE80211_ASSOCIATING_AUTHENTICATED,

	
	IEEE80211_LINKED,

	
	IEEE80211_LINKED_SCANNING,

};

#define DEFAULT_MAX_SCAN_AGE (15 * HZ)
#define DEFAULT_FTS 2346

#define CFG_IEEE80211_RESERVE_FCS (1<<0)
#define CFG_IEEE80211_COMPUTE_FCS (1<<1)

#define IEEE80211_24GHZ_MIN_CHANNEL 1
#define IEEE80211_24GHZ_MAX_CHANNEL 14
#define IEEE80211_24GHZ_CHANNELS (IEEE80211_24GHZ_MAX_CHANNEL - \
                                  IEEE80211_24GHZ_MIN_CHANNEL + 1)

#define IEEE80211_52GHZ_MIN_CHANNEL 34
#define IEEE80211_52GHZ_MAX_CHANNEL 165
#define IEEE80211_52GHZ_CHANNELS (IEEE80211_52GHZ_MAX_CHANNEL - \
                                  IEEE80211_52GHZ_MIN_CHANNEL + 1)

typedef struct tx_pending_t{
	int frag;
	struct ieee80211_txb *txb;
}tx_pending_t;

enum {
	COUNTRY_CODE_FCC = 0,
	COUNTRY_CODE_IC = 1,
	COUNTRY_CODE_ETSI = 2,
	COUNTRY_CODE_SPAIN = 3,
	COUNTRY_CODE_FRANCE = 4,
	COUNTRY_CODE_MKK = 5,
	COUNTRY_CODE_MKK1 = 6,
	COUNTRY_CODE_ISRAEL = 7,
	COUNTRY_CODE_TELEC,
	COUNTRY_CODE_MIC,
	COUNTRY_CODE_GLOBAL_DOMAIN
};

#include "ieee80211_r8192s.h"

struct ieee80211_device {
	struct net_device *dev;
        struct ieee80211_security sec;

	
	u8 hwsec_active;
	bool is_silent_reset;
	bool is_roaming;
	bool ieee_up;
	bool bSupportRemoteWakeUp;
	RT_PS_MODE dot11PowerSaveMode;
	bool actscanning;
	bool be_scan_inprogress;
	bool beinretry;
	RT_RF_POWER_STATE eRFPowerState;
	u32 RfOffReason;
	bool is_set_key;

	
	PRT_HIGH_THROUGHPUT pHTInfo;
	spinlock_t bw_spinlock;

	spinlock_t reorder_spinlock;
	
	u8	Regdot11HTOperationalRateSet[16];	
	u8	dot11HTOperationalRateSet[16];		
	u8	RegHTSuppRateSet[16];
	u8	HTCurrentOperaRate;
	u8	HTHighestOperaRate;
	
	u8	bTxDisableRateFallBack;
	u8 	bTxUseDriverAssingedRate;
	atomic_t	atm_chnlop;
	atomic_t	atm_swbw;

	
	struct list_head	Tx_TS_Admit_List;
	struct list_head	Tx_TS_Pending_List;
	struct list_head	Tx_TS_Unused_List;
	TX_TS_RECORD		TxTsRecord[TOTAL_TS_NUM];
	
	struct list_head	Rx_TS_Admit_List;
	struct list_head	Rx_TS_Pending_List;
	struct list_head	Rx_TS_Unused_List;
	RX_TS_RECORD		RxTsRecord[TOTAL_TS_NUM];

	RX_REORDER_ENTRY	RxReorderEntry[128];
	struct list_head	RxReorder_Unused_List;

	
	
	u8 ForcedPriority;

	
	struct net_device_stats stats;
	struct ieee80211_stats ieee_stats;
	struct ieee80211_softmac_stats softmac_stats;

	
	struct list_head network_free_list;
	struct list_head network_list;
	struct ieee80211_network *networks;
	int scans;
	int scan_age;

	int iw_mode; 
	struct iw_spy_data spy_data;

	spinlock_t lock;
	spinlock_t wpax_suitlist_lock;

	int tx_headroom; 
	u32 config;

	
	int open_wep; 
	int auth_mode;
	int reset_on_keychange; 

	
	int host_encrypt;
	int host_encrypt_msdu;
	int host_decrypt;
        
        int host_mc_decrypt;

        
        
        int host_strip_iv_icv;

        int host_open_frag;
        int host_build_iv;
	int ieee802_1x; 

	
	bool bHalfWirelessN24GMode;
	int wpa_enabled;
	int drop_unencrypted;
	int tkip_countermeasures;
	int privacy_invoked;
	size_t wpa_ie_len;
	u8 *wpa_ie;
	u8 ap_mac_addr[6];
	u16 pairwise_key_type;
	u16 group_key_type;
	struct list_head crypt_deinit_list;
	struct ieee80211_crypt_data *crypt[WEP_KEYS];
	int tx_keyidx; 
	struct timer_list crypt_deinit_timer;
        int crypt_quiesced;

	int bcrx_sta_key; 

	
	
	struct ieee80211_frag_entry frag_cache[17][IEEE80211_FRAG_CACHE_LEN];
	unsigned int frag_next_idx[17];
	u16 fts; 
#define DEFAULT_RTS_THRESHOLD 2346U
#define MIN_RTS_THRESHOLD 1
#define MAX_RTS_THRESHOLD 2346U
        u16 rts; 

        
        u8 bssid[ETH_ALEN];

	
	struct ieee80211_network current_network;

	enum ieee80211_state state;

	int short_slot;
	int reg_mode;
	int mode;       
	int modulation; 
	int freq_band;  
	int abg_true;   

	
	short sync_scan_hurryup;
	u16 scan_watch_dog;
        int perfect_rssi;
        int worst_rssi;

        u16 prev_seq_ctl;       

	
	void *pDot11dInfo;
	bool bGlobalDomain;
	int rate;       
	int basic_rate;
	
	short active_scan;

	
	u16 softmac_features;

	
	u16 seq_ctrl[5];

	
	u16 associate_seq;

	
	u16 assoc_id;

	
	u8 ack_tx_to_ieee;
	short ps;
	short sta_sleep;
	int ps_timeout;
	int ps_period;
	struct tasklet_struct ps_task;
	u32 ps_th;
	u32 ps_tl;

	short raw_tx;
	
	short queue_stop;
	short scanning;
	short proto_started;

	struct semaphore wx_sem;
	struct semaphore scan_sem;

	spinlock_t mgmt_tx_lock;
	spinlock_t beacon_lock;

	short beacon_txing;

	short wap_set;
	short ssid_set;

	u8  wpax_type_set;    
	u32 wpax_type_notify; 

	
	char init_wmmparam_flag;
	
	u8  qos_support;

	
	struct list_head ibss_mac_hash[IEEE_IBSS_MAC_HASH_SIZE];

	
	u16 last_rxseq_num[17]; 
	u16 last_rxfrag_num[17];
	unsigned long last_packet_time[17];

	
	unsigned long last_rx_ps_time;

	
	struct sk_buff *mgmt_queue_ring[MGMT_QUEUE_NUM];
	int mgmt_queue_head;
	int mgmt_queue_tail;


	u8 AsocRetryCount;
	unsigned int hw_header;
	struct sk_buff_head skb_waitQ[MAX_QUEUE_SIZE];
	struct sk_buff_head skb_aggQ[MAX_QUEUE_SIZE];
	struct sk_buff_head skb_drv_aggQ[MAX_QUEUE_SIZE];
	u32 sta_edca_param[4];
	bool aggregation;
	
	bool enable_rx_imm_BA;
	bool bibsscoordinator;

	
	bool bdynamic_txpower_enable;

	bool bCTSToSelfEnable;
	u8 CTSToSelfTH;

	u32	fsync_time_interval;
	u32	fsync_rate_bitmap;
	u8	fsync_rssi_threshold;
	bool	bfsync_enable;

	u8	fsync_multiple_timeinterval;	
	u32	fsync_firstdiff_ratethreshold;	
	u32	fsync_seconddiff_ratethreshold;	
	Fsync_State	fsync_state;
	bool		bis_any_nonbepkts;
	
	struct bandwidth_autoswitch bandwidth_auto_switch;
	
	bool FwRWRF;

	
	struct rt_link_detect LinkDetectInfo;

	struct rt_power_save_control PowerSaveControl;


	
	struct  tx_pending_t tx_pending;

	
	struct timer_list associate_timer;

	
	struct timer_list beacon_timer;
        struct work_struct associate_complete_wq;
        struct work_struct associate_procedure_wq;
        struct delayed_work softmac_scan_wq;
        struct delayed_work associate_retry_wq;
	struct delayed_work start_ibss_wq;
	struct delayed_work hw_wakeup_wq;
	struct delayed_work hw_sleep_wq;
	struct delayed_work link_change_wq;
        struct work_struct wx_sync_scan_wq;
        struct workqueue_struct *wq;

	
	void (*set_security)(struct net_device *dev,
			     struct ieee80211_security *sec);

	
	int (*hard_start_xmit)(struct ieee80211_txb *txb,
			       struct net_device *dev);

	int (*reset_port)(struct net_device *dev);
	int (*is_queue_full)(struct net_device *dev, int pri);

	int (*handle_management)(struct net_device *dev,
				 struct ieee80211_network *network, u16 type);
	int (*is_qos_active)(struct net_device *dev, struct sk_buff *skb);

	
	int (*softmac_hard_start_xmit)(struct sk_buff *skb,
			       struct net_device *dev);

	
	void (*softmac_data_hard_start_xmit)(struct sk_buff *skb,
			       struct net_device *dev,int rate);

	
	void (*data_hard_stop)(struct net_device *dev);

	
	void (*data_hard_resume)(struct net_device *dev);

	
	void (*set_chan)(struct net_device *dev,short ch);

	
	void (*scan_syncro)(struct net_device *dev);
	void (*start_scan)(struct net_device *dev);
	void (*stop_scan)(struct net_device *dev);

	
	void (*link_change)(struct net_device *dev);

	
	void (*start_send_beacons) (struct net_device *dev);
	void (*stop_send_beacons) (struct net_device *dev);

	
	void (*sta_wake_up) (struct net_device *dev);
	void (*enter_sleep_state) (struct net_device *dev, u32 th, u32 tl);
	short (*ps_is_queue_empty) (struct net_device *dev);

	int (*handle_beacon)(struct net_device *dev,
			     struct ieee80211_probe_response *beacon,
			     struct ieee80211_network *network);
	int (*handle_assoc_response)(struct net_device *dev,
				struct ieee80211_assoc_response_frame *resp,
				struct ieee80211_network *network);

	
	short (*check_nic_enough_desc)(struct net_device *dev, int queue_index);
	
	void (*SetBWModeHandler)(struct net_device *dev,
				 HT_CHANNEL_WIDTH Bandwidth,
				 HT_EXTCHNL_OFFSET Offset);
	bool (*GetNmodeSupportBySecCfg)(struct net_device* dev);
	void (*SetWirelessMode)(struct net_device* dev, u8 wireless_mode);
	bool (*GetHalfNmodeSupportByAPsHandler)(struct net_device* dev);
	bool (*is_ap_in_wep_tkip)(struct net_device* dev);
	void (*InitialGainHandler)(struct net_device *dev, u8 Operation);
	bool (*SetFwCmdHandler)(struct net_device *dev, FW_CMD_IO_TYPE FwCmdIO);
	void (*LedControlHandler)(struct net_device *dev,
				  LED_CTL_MODE LedAction);
	
	u8 priv[0];
};

#define IEEE_A            (1<<0)
#define IEEE_B            (1<<1)
#define IEEE_G            (1<<2)
#define IEEE_N_24G 		  (1<<4)
#define	IEEE_N_5G		  (1<<5)
#define IEEE_MODE_MASK    (IEEE_A|IEEE_B|IEEE_G)




#define IEEE_SOFTMAC_SCAN (1<<2)


#define IEEE_SOFTMAC_ASSOCIATE (1<<3)


#define IEEE_SOFTMAC_PROBERQ (1<<4)


#define IEEE_SOFTMAC_PROBERS (1<<5)


#define IEEE_SOFTMAC_TX_QUEUE (1<<7)


#define IEEE_SOFTMAC_SINGLE_QUEUE (1<<8)


#define IEEE_SOFTMAC_BEACONS (1<<6)

static inline void *ieee80211_priv(struct net_device *dev)
{
	return ((struct ieee80211_device *)netdev_priv(dev))->priv;
}

extern inline int ieee80211_is_empty_essid(const char *essid, int essid_len)
{
	
	if (essid_len == 1 && essid[0] == ' ')
		return 1;

	
	while (essid_len) {
		essid_len--;
		if (essid[essid_len] != '\0')
			return 0;
	}

	return 1;
}

extern inline int ieee80211_is_valid_mode(struct ieee80211_device *ieee, int mode)
{
	
	if ((mode & IEEE_A) &&
	    (ieee->modulation & IEEE80211_OFDM_MODULATION) &&
	    (ieee->freq_band & IEEE80211_52GHZ_BAND))
		return 1;

	if ((mode & IEEE_G) &&
	    (ieee->modulation & IEEE80211_OFDM_MODULATION) &&
	    (ieee->freq_band & IEEE80211_24GHZ_BAND))
		return 1;

	if ((mode & IEEE_B) &&
	    (ieee->modulation & IEEE80211_CCK_MODULATION) &&
	    (ieee->freq_band & IEEE80211_24GHZ_BAND))
		return 1;

	return 0;
}

extern inline int ieee80211_get_hdrlen(u16 fc)
{
	int hdrlen = IEEE80211_3ADDR_LEN;

	switch (WLAN_FC_GET_TYPE(fc)) {
	case IEEE80211_FTYPE_DATA:
		if ((fc & IEEE80211_FCTL_FROMDS) && (fc & IEEE80211_FCTL_TODS))
			hdrlen = IEEE80211_4ADDR_LEN; 
		if(IEEE80211_QOS_HAS_SEQ(fc))
			hdrlen += 2; 
		break;
	case IEEE80211_FTYPE_CTL:
		switch (WLAN_FC_GET_STYPE(fc)) {
		case IEEE80211_STYPE_CTS:
		case IEEE80211_STYPE_ACK:
			hdrlen = IEEE80211_1ADDR_LEN;
			break;
		default:
			hdrlen = IEEE80211_2ADDR_LEN;
			break;
		}
		break;
	}

	return hdrlen;
}

static inline u8 *ieee80211_get_payload(struct rtl_ieee80211_hdr *hdr)
{
        switch (ieee80211_get_hdrlen(le16_to_cpu(hdr->frame_ctl))) {
        case IEEE80211_1ADDR_LEN:
                return ((struct ieee80211_hdr_1addr *)hdr)->payload;
        case IEEE80211_2ADDR_LEN:
                return ((struct ieee80211_hdr_2addr *)hdr)->payload;
        case IEEE80211_3ADDR_LEN:
                return ((struct ieee80211_hdr_3addr *)hdr)->payload;
        case IEEE80211_4ADDR_LEN:
                return ((struct ieee80211_hdr_4addr *)hdr)->payload;
        }
        return NULL;
}

static inline int ieee80211_is_ofdm_rate(u8 rate)
{
        switch (rate & ~IEEE80211_BASIC_RATE_MASK) {
        case IEEE80211_OFDM_RATE_6MB:
        case IEEE80211_OFDM_RATE_9MB:
        case IEEE80211_OFDM_RATE_12MB:
        case IEEE80211_OFDM_RATE_18MB:
        case IEEE80211_OFDM_RATE_24MB:
        case IEEE80211_OFDM_RATE_36MB:
        case IEEE80211_OFDM_RATE_48MB:
        case IEEE80211_OFDM_RATE_54MB:
                return 1;
        }
        return 0;
}

static inline int ieee80211_is_cck_rate(u8 rate)
{
        switch (rate & ~IEEE80211_BASIC_RATE_MASK) {
        case IEEE80211_CCK_RATE_1MB:
        case IEEE80211_CCK_RATE_2MB:
        case IEEE80211_CCK_RATE_5MB:
        case IEEE80211_CCK_RATE_11MB:
                return 1;
        }
        return 0;
}



extern void free_ieee80211(struct net_device *dev);
extern struct net_device *alloc_ieee80211(int sizeof_priv);

extern int ieee80211_set_encryption(struct ieee80211_device *ieee);



extern int ieee80211_encrypt_fragment(
	struct ieee80211_device *ieee,
	struct sk_buff *frag,
	int hdr_len);

extern int rtl8192_ieee80211_xmit(struct sk_buff *skb,
			  struct net_device *dev);
extern void ieee80211_txb_free(struct ieee80211_txb *);



extern int ieee80211_rx(struct ieee80211_device *ieee, struct sk_buff *skb,
			struct ieee80211_rx_stats *rx_stats);
extern void ieee80211_rx_mgt(struct ieee80211_device *ieee,
			     struct ieee80211_hdr_4addr *header,
			     struct ieee80211_rx_stats *stats);


extern int ieee80211_wx_get_scan(struct ieee80211_device *ieee,
				 struct iw_request_info *info,
				 union iwreq_data *wrqu, char *key);
extern int ieee80211_wx_set_encode(struct ieee80211_device *ieee,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *key);
extern int ieee80211_wx_get_encode(struct ieee80211_device *ieee,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *key);
extern int ieee80211_wx_set_encode_ext(struct ieee80211_device *ieee,
                            struct iw_request_info *info,
                            union iwreq_data* wrqu, char *extra);
extern int ieee80211_wx_set_auth(struct ieee80211_device *ieee,
                               struct iw_request_info *info,
                               struct iw_param *data, char *extra);
extern int ieee80211_wx_set_mlme(struct ieee80211_device *ieee,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra);
extern int ieee80211_wx_set_gen_ie(struct ieee80211_device *ieee, u8 *ie, size_t len);


extern short ieee80211_is_54g(struct ieee80211_network net);
extern short ieee80211_is_shortslot(struct ieee80211_network net);
extern int ieee80211_rx_frame_softmac(struct ieee80211_device *ieee, struct sk_buff *skb,
			struct ieee80211_rx_stats *rx_stats, u16 type,
			u16 stype);
extern void ieee80211_softmac_new_net(struct ieee80211_device *ieee, struct ieee80211_network *net);

void SendDisassociation(struct ieee80211_device *ieee, u8* asSta, u8 asRsn);
extern void ieee80211_softmac_xmit(struct ieee80211_txb *txb, struct ieee80211_device *ieee);

extern void ieee80211_stop_send_beacons(struct ieee80211_device *ieee);
extern void notify_wx_assoc_event(struct ieee80211_device *ieee);
extern void ieee80211_softmac_check_all_nets(struct ieee80211_device *ieee);
extern void ieee80211_start_bss(struct ieee80211_device *ieee);
extern void ieee80211_start_master_bss(struct ieee80211_device *ieee);
extern void ieee80211_start_ibss(struct ieee80211_device *ieee);
extern void ieee80211_softmac_init(struct ieee80211_device *ieee);
extern void ieee80211_softmac_free(struct ieee80211_device *ieee);
extern void ieee80211_associate_abort(struct ieee80211_device *ieee);
extern void ieee80211_disassociate(struct ieee80211_device *ieee);
extern void ieee80211_stop_scan(struct ieee80211_device *ieee);
extern void ieee80211_start_scan_syncro(struct ieee80211_device *ieee);
extern void ieee80211_check_all_nets(struct ieee80211_device *ieee);
extern void ieee80211_start_protocol(struct ieee80211_device *ieee);
extern void ieee80211_stop_protocol(struct ieee80211_device *ieee);
extern void ieee80211_softmac_start_protocol(struct ieee80211_device *ieee);
extern void ieee80211_softmac_stop_protocol(struct ieee80211_device *ieee);
extern void ieee80211_reset_queue(struct ieee80211_device *ieee);
extern void ieee80211_wake_queue(struct ieee80211_device *ieee);
extern void ieee80211_stop_queue(struct ieee80211_device *ieee);
extern struct sk_buff *ieee80211_get_beacon(struct ieee80211_device *ieee);
extern void ieee80211_start_send_beacons(struct ieee80211_device *ieee);
extern void ieee80211_stop_send_beacons(struct ieee80211_device *ieee);
extern int ieee80211_wpa_supplicant_ioctl(struct ieee80211_device *ieee, struct iw_point *p);
extern void notify_wx_assoc_event(struct ieee80211_device *ieee);
extern void ieee80211_ps_tx_ack(struct ieee80211_device *ieee, short success);

extern void softmac_mgmt_xmit(struct sk_buff *skb, struct ieee80211_device *ieee);


extern void ieee80211_tkip_null(void);
extern void ieee80211_wep_null(void);
extern void ieee80211_ccmp_null(void);



extern int ieee80211_wx_get_wap(struct ieee80211_device *ieee,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *ext);

extern int ieee80211_wx_set_wap(struct ieee80211_device *ieee,
			 struct iw_request_info *info,
			 union iwreq_data *awrq,
			 char *extra);

extern int ieee80211_wx_get_essid(struct ieee80211_device *ieee, struct iw_request_info *a,union iwreq_data *wrqu,char *b);

extern int ieee80211_wx_set_rate(struct ieee80211_device *ieee,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra);

extern int ieee80211_wx_get_rate(struct ieee80211_device *ieee,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra);

extern int ieee80211_wx_set_mode(struct ieee80211_device *ieee, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b);

extern int ieee80211_wx_set_scan(struct ieee80211_device *ieee, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b);

extern int ieee80211_wx_set_essid(struct ieee80211_device *ieee,
			      struct iw_request_info *a,
			      union iwreq_data *wrqu, char *extra);

extern int ieee80211_wx_get_mode(struct ieee80211_device *ieee, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b);

extern int ieee80211_wx_set_freq(struct ieee80211_device *ieee, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b);

extern int ieee80211_wx_get_freq(struct ieee80211_device *ieee, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b);

extern void ieee80211_wx_sync_scan_wq(struct work_struct *work);

extern int ieee80211_wx_set_rawtx(struct ieee80211_device *ieee,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra);

extern int ieee80211_wx_get_name(struct ieee80211_device *ieee,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra);

extern int ieee80211_wx_set_power(struct ieee80211_device *ieee,
				 struct iw_request_info *info,
				 union iwreq_data *wrqu, char *extra);

extern int ieee80211_wx_get_power(struct ieee80211_device *ieee,
				 struct iw_request_info *info,
				 union iwreq_data *wrqu, char *extra);

extern int ieee80211_wx_set_rts(struct ieee80211_device *ieee,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra);

extern int ieee80211_wx_get_rts(struct ieee80211_device *ieee,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra);

extern void ieee80211_softmac_scan_syncro(struct ieee80211_device *ieee);

extern const long ieee80211_wlan_frequencies[];

extern inline void ieee80211_increment_scans(struct ieee80211_device *ieee)
{
	ieee->scans++;
}

extern inline int ieee80211_get_scans(struct ieee80211_device *ieee)
{
	return ieee->scans;
}

static inline const char *escape_essid(const char *essid, u8 essid_len) {
	static char escaped[IW_ESSID_MAX_SIZE * 2 + 1];
	const char *s = essid;
	char *d = escaped;

	if (ieee80211_is_empty_essid(essid, essid_len)) {
		memcpy(escaped, "<hidden>", sizeof("<hidden>"));
		return escaped;
	}

	essid_len = min(essid_len, (u8)IW_ESSID_MAX_SIZE);
	while (essid_len--) {
		if (*s == '\0') {
			*d++ = '\\';
			*d++ = '0';
			s++;
		} else {
			*d++ = *s++;
		}
	}
	*d = '\0';
	return escaped;
}

/* For the function is more related to hardware setting, it's better to use the
 * ieee handler to refer to it.
 */
extern short check_nic_enough_desc(struct net_device *dev, int queue_index);
extern int ieee80211_data_xmit(struct sk_buff *skb, struct net_device *dev);
extern int ieee80211_parse_info_param(struct ieee80211_device *ieee,
		struct ieee80211_info_element *info_element,
		u16 length,
		struct ieee80211_network *network,
		struct ieee80211_rx_stats *stats);

extern void ieee80211_indicate_packets(struct ieee80211_device *ieee,
				       struct ieee80211_rxb **prxbIndicateArray,
				       u8 index);
#define RT_ASOC_RETRY_LIMIT	5
#endif 
