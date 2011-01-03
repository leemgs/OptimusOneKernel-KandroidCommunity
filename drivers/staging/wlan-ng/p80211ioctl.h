

#ifndef _P80211IOCTL_H
#define _P80211IOCTL_H



#define P80211_IFTEST		(SIOCDEVPRIVATE + 0)
#define P80211_IFREQ		(SIOCDEVPRIVATE + 1)




#define P80211_IOCTL_MAGIC	(0x4a2d464dUL)






typedef struct p80211ioctl_req {
	char name[WLAN_DEVNAMELEN_MAX];
	caddr_t data;
	u32 magic;
	u16 len;
	u32 result;
} __attribute__ ((packed)) p80211ioctl_req_t;

#endif 
