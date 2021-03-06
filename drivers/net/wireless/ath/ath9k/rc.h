

#ifndef RC_H
#define RC_H

struct ath_softc;

#define ATH_RATE_MAX     30
#define RATE_TABLE_SIZE  64
#define MAX_TX_RATE_PHY  48



#define INVALID    0x0
#define VALID      0x1
#define VALID_20   0x2
#define VALID_40   0x4
#define VALID_2040 (VALID_20|VALID_40)
#define VALID_ALL  (VALID_2040|VALID)

enum {
	WLAN_RC_PHY_OFDM,
	WLAN_RC_PHY_CCK,
	WLAN_RC_PHY_HT_20_SS,
	WLAN_RC_PHY_HT_20_DS,
	WLAN_RC_PHY_HT_40_SS,
	WLAN_RC_PHY_HT_40_DS,
	WLAN_RC_PHY_HT_20_SS_HGI,
	WLAN_RC_PHY_HT_20_DS_HGI,
	WLAN_RC_PHY_HT_40_SS_HGI,
	WLAN_RC_PHY_HT_40_DS_HGI,
	WLAN_RC_PHY_MAX
};

#define WLAN_RC_PHY_DS(_phy)   ((_phy == WLAN_RC_PHY_HT_20_DS)		\
				|| (_phy == WLAN_RC_PHY_HT_40_DS)	\
				|| (_phy == WLAN_RC_PHY_HT_20_DS_HGI)	\
				|| (_phy == WLAN_RC_PHY_HT_40_DS_HGI))
#define WLAN_RC_PHY_40(_phy)   ((_phy == WLAN_RC_PHY_HT_40_SS)		\
				|| (_phy == WLAN_RC_PHY_HT_40_DS)	\
				|| (_phy == WLAN_RC_PHY_HT_40_SS_HGI)	\
				|| (_phy == WLAN_RC_PHY_HT_40_DS_HGI))
#define WLAN_RC_PHY_SGI(_phy)  ((_phy == WLAN_RC_PHY_HT_20_SS_HGI)      \
				|| (_phy == WLAN_RC_PHY_HT_20_DS_HGI)   \
				|| (_phy == WLAN_RC_PHY_HT_40_SS_HGI)   \
				|| (_phy == WLAN_RC_PHY_HT_40_DS_HGI))

#define WLAN_RC_PHY_HT(_phy)    (_phy >= WLAN_RC_PHY_HT_20_SS)

#define WLAN_RC_CAP_MODE(capflag) (((capflag & WLAN_RC_HT_FLAG) ?	\
		(capflag & WLAN_RC_40_FLAG) ? VALID_40 : VALID_20 : VALID))


#define WLAN_RC_PHY_HT_VALID(flag, capflag)			\
	(((flag & VALID_20) && !(capflag & WLAN_RC_40_FLAG)) || \
	 ((flag & VALID_40) && (capflag & WLAN_RC_40_FLAG)))

#define WLAN_RC_DS_FLAG         (0x01)
#define WLAN_RC_40_FLAG         (0x02)
#define WLAN_RC_SGI_FLAG        (0x04)
#define WLAN_RC_HT_FLAG         (0x08)


struct ath_rate_table {
	int rate_cnt;
	struct {
		int valid;
		int valid_single_stream;
		u8 phy;
		u32 ratekbps;
		u32 user_ratekbps;
		u8 ratecode;
		u8 short_preamble;
		u8 dot11rate;
		u8 ctrl_rate;
		u8 base_index;
		u8 cw40index;
		u8 sgi_index;
		u8 ht_index;
		u32 max_4ms_framelen;
	} info[RATE_TABLE_SIZE];
	u32 probe_interval;
	u8 initial_ratemax;
};

struct ath_rateset {
	u8 rs_nrates;
	u8 rs_rates[ATH_RATE_MAX];
};


struct ath_rate_priv {
	u8 rate_table_size;
	u8 probe_rate;
	u8 hw_maxretry_pktcnt;
	u8 max_valid_rate;
	u8 valid_rate_index[RATE_TABLE_SIZE];
	u8 ht_cap;
	u8 valid_phy_ratecnt[WLAN_RC_PHY_MAX];
	u8 valid_phy_rateidx[WLAN_RC_PHY_MAX][RATE_TABLE_SIZE];
	u8 rate_max_phy;
	u8 per[RATE_TABLE_SIZE];
	u32 probe_time;
	u32 per_down_time;
	u32 probe_interval;
	u32 prev_data_rix;
	u32 tx_triglevel_max;
	struct ath_rateset neg_rates;
	struct ath_rateset neg_ht_rates;
	struct ath_rate_softc *asc;
};

enum ath9k_internal_frame_type {
	ATH9K_NOT_INTERNAL,
	ATH9K_INT_PAUSE,
	ATH9K_INT_UNPAUSE
};

struct ath_tx_info_priv {
	struct ath_wiphy *aphy;
	struct ath_tx_status tx;
	int n_frames;
	int n_bad_frames;
	bool update_rc;
	enum ath9k_internal_frame_type frame_type;
};

#define ATH_TX_INFO_PRIV(tx_info) \
	((struct ath_tx_info_priv *)((tx_info)->rate_driver_data[0]))

void ath_rate_attach(struct ath_softc *sc);
u8 ath_rate_findrateix(struct ath_softc *sc, u8 dot11_rate);
int ath_rate_control_register(void);
void ath_rate_control_unregister(void);

#endif 
