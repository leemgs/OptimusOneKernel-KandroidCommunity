

#ifndef _AF_IEEE802154_H
#define _AF_IEEE802154_H

#include <linux/socket.h> 

enum {
	IEEE802154_ADDR_NONE = 0x0,
	
	IEEE802154_ADDR_SHORT = 0x2, 
	IEEE802154_ADDR_LONG = 0x3, 
};


#define IEEE802154_ADDR_LEN	8

struct ieee802154_addr {
	int addr_type;
	u16 pan_id;
	union {
		u8 hwaddr[IEEE802154_ADDR_LEN];
		u16 short_addr;
	};
};

#define IEEE802154_PANID_BROADCAST	0xffff
#define IEEE802154_ADDR_BROADCAST	0xffff
#define IEEE802154_ADDR_UNDEF		0xfffe

struct sockaddr_ieee802154 {
	sa_family_t family; 
	struct ieee802154_addr addr;
};


#define SOL_IEEE802154	0

#define WPAN_WANTACK	0

#endif
