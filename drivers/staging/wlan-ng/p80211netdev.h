

#ifndef _LINUX_P80211NETDEV_H
#define _LINUX_P80211NETDEV_H

#include <linux/interrupt.h>
#include <linux/wireless.h>
#include <linux/netdevice.h>

#undef netdevice_t
typedef struct net_device netdevice_t;

#define WLAN_RELEASE	"0.3.0-staging"

#define WLAN_DEVICE_CLOSED	0
#define WLAN_DEVICE_OPEN	1

#define WLAN_MACMODE_NONE	0
#define WLAN_MACMODE_IBSS_STA	1
#define WLAN_MACMODE_ESS_STA	2
#define WLAN_MACMODE_ESS_AP	3


#define WLAN_MSD_HWPRESENT_PENDING	1
#define WLAN_MSD_HWFAIL			2
#define WLAN_MSD_HWPRESENT		3
#define WLAN_MSD_FWLOAD_PENDING		4
#define WLAN_MSD_FWLOAD			5
#define WLAN_MSD_RUNNING_PENDING	6
#define WLAN_MSD_RUNNING		7

#ifndef ETH_P_ECONET
#define ETH_P_ECONET   0x0018	
#endif

#define ETH_P_80211_RAW        (ETH_P_ECONET + 1)

#ifndef ARPHRD_IEEE80211
#define ARPHRD_IEEE80211 801	
#endif

#ifndef ARPHRD_IEEE80211_PRISM	
#define ARPHRD_IEEE80211_PRISM 802
#endif


#define P80211_NSDCAP_HARDWAREWEP           0x01	
#define P80211_NSDCAP_SHORT_PREAMBLE        0x10	
#define P80211_NSDCAP_HWFRAGMENT            0x80	
#define P80211_NSDCAP_AUTOJOIN              0x100	
#define P80211_NSDCAP_NOSCAN                0x200	


typedef struct p80211_frmrx_t {
	u32 mgmt;
	u32 assocreq;
	u32 assocresp;
	u32 reassocreq;
	u32 reassocresp;
	u32 probereq;
	u32 proberesp;
	u32 beacon;
	u32 atim;
	u32 disassoc;
	u32 authen;
	u32 deauthen;
	u32 mgmt_unknown;
	u32 ctl;
	u32 pspoll;
	u32 rts;
	u32 cts;
	u32 ack;
	u32 cfend;
	u32 cfendcfack;
	u32 ctl_unknown;
	u32 data;
	u32 dataonly;
	u32 data_cfack;
	u32 data_cfpoll;
	u32 data__cfack_cfpoll;
	u32 null;
	u32 cfack;
	u32 cfpoll;
	u32 cfack_cfpoll;
	u32 data_unknown;
	u32 decrypt;
	u32 decrypt_err;
} p80211_frmrx_t;


struct iw_statistics *p80211wext_get_wireless_stats(netdevice_t * dev);

extern struct iw_handler_def p80211wext_handler_def;
int p80211wext_event_associated(struct wlandevice *wlandev, int assoc);


#define NUM_WEPKEYS 4
#define MAX_KEYLEN 32

#define HOSTWEP_DEFAULTKEY_MASK (BIT(1)|BIT(0))
#define HOSTWEP_DECRYPT  BIT(4)
#define HOSTWEP_ENCRYPT  BIT(5)
#define HOSTWEP_PRIVACYINVOKED BIT(6)
#define HOSTWEP_EXCLUDEUNENCRYPTED BIT(7)

extern int wlan_watchdog;
extern int wlan_wext_write;


typedef struct wlandevice {
	struct wlandevice *next;	
	void *priv;		

	
	char name[WLAN_DEVNAMELEN_MAX];	
	char *nsdname;

	u32 state;		
	u32 msdstate;		
	u32 hwremoved;		

	
	unsigned int irq;
	unsigned int iobase;
	unsigned int membase;
	u32 nsdcaps;		

	
	unsigned int ethconv;

	
	int (*open) (struct wlandevice * wlandev);
	int (*close) (struct wlandevice * wlandev);
	void (*reset) (struct wlandevice * wlandev);
	int (*txframe) (struct wlandevice * wlandev, struct sk_buff * skb,
			p80211_hdr_t * p80211_hdr,
			p80211_metawep_t * p80211_wep);
	int (*mlmerequest) (struct wlandevice * wlandev, p80211msg_t * msg);
	int (*set_multicast_list) (struct wlandevice * wlandev,
				   netdevice_t * dev);
	void (*tx_timeout) (struct wlandevice * wlandev);

	
	u8 bssid[WLAN_BSSID_LEN];
	p80211pstr32_t ssid;
	u32 macmode;
	int linkstatus;

	
	u8 wep_keys[NUM_WEPKEYS][MAX_KEYLEN];
	u8 wep_keylens[NUM_WEPKEYS];
	int hostwep;

	
	unsigned long request_pending;	

	
	
	
	netdevice_t *netdev;	
	struct net_device_stats linux_stats;

	
	struct tasklet_struct rx_bh;

	struct sk_buff_head nsd_rxq;

	
	struct p80211_frmrx_t rx;

	struct iw_statistics wstats;

	
	u8 spy_number;
	char spy_address[IW_MAX_SPY][ETH_ALEN];
	struct iw_quality spy_stat[IW_MAX_SPY];
} wlandevice_t;


int wep_change_key(wlandevice_t * wlandev, int keynum, u8 * key, int keylen);
int wep_decrypt(wlandevice_t * wlandev, u8 * buf, u32 len, int key_override,
		u8 * iv, u8 * icv);
int wep_encrypt(wlandevice_t * wlandev, u8 * buf, u8 * dst, u32 len, int keynum,
		u8 * iv, u8 * icv);

int wlan_setup(wlandevice_t * wlandev);
int wlan_unsetup(wlandevice_t * wlandev);
int register_wlandev(wlandevice_t * wlandev);
int unregister_wlandev(wlandevice_t * wlandev);
void p80211netdev_rx(wlandevice_t * wlandev, struct sk_buff *skb);
void p80211netdev_hwremoved(wlandevice_t * wlandev);
#endif
