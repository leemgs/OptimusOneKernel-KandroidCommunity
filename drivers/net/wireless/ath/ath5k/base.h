


#ifndef _DEV_ATH_ATHVAR_H
#define _DEV_ATH_ATHVAR_H

#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/wireless.h>
#include <linux/if_ether.h>
#include <linux/leds.h>
#include <linux/rfkill.h>

#include "ath5k.h"
#include "debug.h"

#include "../regd.h"
#include "../ath.h"

#define	ATH_RXBUF	40		
#define	ATH_TXBUF	200		
#define ATH_BCBUF	1		

struct ath5k_buf {
	struct list_head	list;
	struct ath5k_desc	*desc;	
	dma_addr_t		daddr;	
	struct sk_buff		*skb;	
	dma_addr_t		skbaddr;
};


struct ath5k_txq {
	unsigned int		qnum;	
	u32			*link;	
	struct list_head	q;	
	spinlock_t		lock;	
	bool			setup;
};

#define ATH5K_LED_MAX_NAME_LEN 31


struct ath5k_led
{
	char name[ATH5K_LED_MAX_NAME_LEN + 1];	
	struct ath5k_softc *sc;			
	struct led_classdev led_dev;		
};


struct ath5k_rfkill {
	
	u16 gpio;
	
	bool polarity;
	
	struct tasklet_struct toggleq;
};

#if CHAN_DEBUG
#define ATH_CHAN_MAX	(26+26+26+200+200)
#else
#define ATH_CHAN_MAX	(14+14+14+252+20)
#endif


struct ath5k_softc {
	struct pci_dev		*pdev;		
	struct ath_common	common;
	void __iomem		*iobase;	
	struct mutex		lock;		
	struct ieee80211_tx_queue_stats tx_stats[AR5K_NUM_TX_QUEUES];
	struct ieee80211_low_level_stats ll_stats;
	struct ieee80211_hw	*hw;		
	struct ieee80211_supported_band sbands[IEEE80211_NUM_BANDS];
	struct ieee80211_channel channels[ATH_CHAN_MAX];
	struct ieee80211_rate	rates[IEEE80211_NUM_BANDS][AR5K_MAX_RATES];
	s8			rate_idx[IEEE80211_NUM_BANDS][AR5K_MAX_RATES];
	enum nl80211_iftype	opmode;
	struct ath5k_hw		*ah;		

	struct ieee80211_supported_band		*curband;

#ifdef CONFIG_ATH5K_DEBUG
	struct ath5k_dbg_info	debug;		
#endif 

	struct ath5k_buf	*bufptr;	
	struct ath5k_desc	*desc;		
	dma_addr_t		desc_daddr;	
	size_t			desc_len;	

	DECLARE_BITMAP(status, 5);
#define ATH_STAT_INVALID	0		
#define ATH_STAT_MRRETRY	1		
#define ATH_STAT_PROMISC	2
#define ATH_STAT_LEDSOFT	3		
#define ATH_STAT_STARTED	4		

	unsigned int		filter_flags;	
	unsigned int		curmode;	
	struct ieee80211_channel *curchan;	

	struct ieee80211_vif *vif;

	enum ath5k_int		imask;		

	DECLARE_BITMAP(keymap, AR5K_KEYCACHE_SIZE); 

	u8			bssidmask[ETH_ALEN];

	unsigned int		led_pin,	
				led_on;		

	struct tasklet_struct	restq;		

	unsigned int		rxbufsize;	
	struct list_head	rxbuf;		
	spinlock_t		rxbuflock;
	u32			*rxlink;	
	struct tasklet_struct	rxtq;		
	struct ath5k_led	rx_led;		

	struct list_head	txbuf;		
	spinlock_t		txbuflock;
	unsigned int		txbuf_len;	
	struct ath5k_txq	txqs[AR5K_NUM_TX_QUEUES];	
	struct ath5k_txq	*txq;		
	struct tasklet_struct	txtq;		
	struct ath5k_led	tx_led;		

	struct ath5k_rfkill	rf_kill;

	struct tasklet_struct	calib;		

	spinlock_t		block;		
	struct tasklet_struct	beacontq;	
	struct ath5k_buf	*bbuf;		
	unsigned int		bhalq,		
				bmisscount,	
				bintval,	
				bsent;
	unsigned int		nexttbtt;	
	struct ath5k_txq	*cabq;		

	int 			power_level;	
	bool			assoc;		
	bool			enable_beacon;	
};

#define ath5k_hw_hasbssidmask(_ah) \
	(ath5k_hw_get_capability(_ah, AR5K_CAP_BSSIDMASK, 0, NULL) == 0)
#define ath5k_hw_hasveol(_ah) \
	(ath5k_hw_get_capability(_ah, AR5K_CAP_VEOL, 0, NULL) == 0)

static inline struct ath_common *ath5k_hw_common(struct ath5k_hw *ah)
{
	return &ah->ah_sc->common;
}

static inline struct ath_regulatory *ath5k_hw_regulatory(struct ath5k_hw *ah)
{
	return &(ath5k_hw_common(ah)->regulatory);

}

#endif
