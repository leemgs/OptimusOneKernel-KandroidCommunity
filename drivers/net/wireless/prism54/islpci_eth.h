

#ifndef _ISLPCI_ETH_H
#define _ISLPCI_ETH_H

#include "isl_38xx.h"
#include "islpci_dev.h"

struct rfmon_header {
	__le16 unk0;		
	__le16 length;		
	__le32 clock;		
	u8 flags;
	u8 unk1;
	u8 rate;
	u8 unk2;
	__le16 freq;
	__le16 unk3;
	u8 rssi;
	u8 padding[3];
} __attribute__ ((packed));

struct rx_annex_header {
	u8 addr1[ETH_ALEN];
	u8 addr2[ETH_ALEN];
	struct rfmon_header rfmon;
} __attribute__ ((packed));


#define P80211CAPTURE_VERSION 0x80211001

struct avs_80211_1_header {
	__be32 version;
	__be32 length;
	__be64 mactime;
	__be64 hosttime;
	__be32 phytype;
	__be32 channel;
	__be32 datarate;
	__be32 antenna;
	__be32 priority;
	__be32 ssi_type;
	__be32 ssi_signal;
	__be32 ssi_noise;
	__be32 preamble;
	__be32 encoding;
};

void islpci_eth_cleanup_transmit(islpci_private *, isl38xx_control_block *);
netdev_tx_t islpci_eth_transmit(struct sk_buff *, struct net_device *);
int islpci_eth_receive(islpci_private *);
void islpci_eth_tx_timeout(struct net_device *);
void islpci_do_reset_and_wake(struct work_struct *);

#endif				
