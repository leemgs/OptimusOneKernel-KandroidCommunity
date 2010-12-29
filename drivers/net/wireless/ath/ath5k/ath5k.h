

#ifndef _ATH5K_H
#define _ATH5K_H


#define CHAN_DEBUG	0

#include <linux/io.h>
#include <linux/types.h>
#include <net/mac80211.h>


#include "desc.h"


#include "eeprom.h"


#define PCI_DEVICE_ID_ATHEROS_AR5210 		0x0007 
#define PCI_DEVICE_ID_ATHEROS_AR5311 		0x0011 
#define PCI_DEVICE_ID_ATHEROS_AR5211 		0x0012 
#define PCI_DEVICE_ID_ATHEROS_AR5212 		0x0013 
#define PCI_DEVICE_ID_3COM_3CRDAG675 		0x0013 
#define PCI_DEVICE_ID_3COM_2_3CRPAG175 		0x0013 
#define PCI_DEVICE_ID_ATHEROS_AR5210_AP 	0x0207 
#define PCI_DEVICE_ID_ATHEROS_AR5212_IBM	0x1014 
#define PCI_DEVICE_ID_ATHEROS_AR5210_DEFAULT 	0x1107 
#define PCI_DEVICE_ID_ATHEROS_AR5212_DEFAULT 	0x1113 
#define PCI_DEVICE_ID_ATHEROS_AR5211_DEFAULT 	0x1112 
#define PCI_DEVICE_ID_ATHEROS_AR5212_FPGA 	0xf013 
#define PCI_DEVICE_ID_ATHEROS_AR5211_LEGACY 	0xff12 
#define PCI_DEVICE_ID_ATHEROS_AR5211_FPGA11B 	0xf11b 
#define PCI_DEVICE_ID_ATHEROS_AR5312_REV2 	0x0052 
#define PCI_DEVICE_ID_ATHEROS_AR5312_REV7 	0x0057 
#define PCI_DEVICE_ID_ATHEROS_AR5312_REV8 	0x0058 
#define PCI_DEVICE_ID_ATHEROS_AR5212_0014 	0x0014 
#define PCI_DEVICE_ID_ATHEROS_AR5212_0015 	0x0015 
#define PCI_DEVICE_ID_ATHEROS_AR5212_0016 	0x0016 
#define PCI_DEVICE_ID_ATHEROS_AR5212_0017 	0x0017 
#define PCI_DEVICE_ID_ATHEROS_AR5212_0018 	0x0018 
#define PCI_DEVICE_ID_ATHEROS_AR5212_0019 	0x0019 
#define PCI_DEVICE_ID_ATHEROS_AR2413 		0x001a 
#define PCI_DEVICE_ID_ATHEROS_AR5413 		0x001b 
#define PCI_DEVICE_ID_ATHEROS_AR5424 		0x001c 
#define PCI_DEVICE_ID_ATHEROS_AR5416 		0x0023 
#define PCI_DEVICE_ID_ATHEROS_AR5418 		0x0024 



#define ATH5K_PRINTF(fmt, ...)   printk("%s: " fmt, __func__, ##__VA_ARGS__)

#define ATH5K_PRINTK(_sc, _level, _fmt, ...) \
	printk(_level "ath5k %s: " _fmt, \
		((_sc) && (_sc)->hw) ? wiphy_name((_sc)->hw->wiphy) : "", \
		##__VA_ARGS__)

#define ATH5K_PRINTK_LIMIT(_sc, _level, _fmt, ...) do { \
	if (net_ratelimit()) \
		ATH5K_PRINTK(_sc, _level, _fmt, ##__VA_ARGS__); \
	} while (0)

#define ATH5K_INFO(_sc, _fmt, ...) \
	ATH5K_PRINTK(_sc, KERN_INFO, _fmt, ##__VA_ARGS__)

#define ATH5K_WARN(_sc, _fmt, ...) \
	ATH5K_PRINTK_LIMIT(_sc, KERN_WARNING, _fmt, ##__VA_ARGS__)

#define ATH5K_ERR(_sc, _fmt, ...) \
	ATH5K_PRINTK_LIMIT(_sc, KERN_ERR, _fmt, ##__VA_ARGS__)






#define AR5K_REG_SM(_val, _flags)					\
	(((_val) << _flags##_S) & (_flags))


#define AR5K_REG_MS(_val, _flags)					\
	(((_val) & (_flags)) >> _flags##_S)


#define AR5K_REG_WRITE_BITS(ah, _reg, _flags, _val)			\
	ath5k_hw_reg_write(ah, (ath5k_hw_reg_read(ah, _reg) & ~(_flags)) | \
	    (((_val) << _flags##_S) & (_flags)), _reg)

#define AR5K_REG_MASKED_BITS(ah, _reg, _flags, _mask)			\
	ath5k_hw_reg_write(ah, (ath5k_hw_reg_read(ah, _reg) &		\
			(_mask)) | (_flags), _reg)

#define AR5K_REG_ENABLE_BITS(ah, _reg, _flags)				\
	ath5k_hw_reg_write(ah, ath5k_hw_reg_read(ah, _reg) | (_flags), _reg)

#define AR5K_REG_DISABLE_BITS(ah, _reg, _flags)			\
	ath5k_hw_reg_write(ah, ath5k_hw_reg_read(ah, _reg) & ~(_flags), _reg)


#define AR5K_PHY_READ(ah, _reg)					\
	ath5k_hw_reg_read(ah, (ah)->ah_phy + ((_reg) << 2))

#define AR5K_PHY_WRITE(ah, _reg, _val)					\
	ath5k_hw_reg_write(ah, _val, (ah)->ah_phy + ((_reg) << 2))


#define AR5K_REG_READ_Q(ah, _reg, _queue)				\
	(ath5k_hw_reg_read(ah, _reg) & (1 << _queue))			\

#define AR5K_REG_WRITE_Q(ah, _reg, _queue)				\
	ath5k_hw_reg_write(ah, (1 << _queue), _reg)

#define AR5K_Q_ENABLE_BITS(_reg, _queue) do {				\
	_reg |= 1 << _queue;						\
} while (0)

#define AR5K_Q_DISABLE_BITS(_reg, _queue) do {				\
	_reg &= ~(1 << _queue);						\
} while (0)


#define AR5K_REG_WAIT(_i) do {						\
	if (_i % 64)							\
		udelay(1);						\
} while (0)


#define AR5K_INI_RFGAIN_5GHZ		0
#define AR5K_INI_RFGAIN_2GHZ		1


#define AR5K_INI_VAL_11A		0
#define AR5K_INI_VAL_11A_TURBO		1
#define AR5K_INI_VAL_11B		2
#define AR5K_INI_VAL_11G		3
#define AR5K_INI_VAL_11G_TURBO		4
#define AR5K_INI_VAL_XR			0
#define AR5K_INI_VAL_MAX		5


#define AR5K_LOW_ID(_a)(				\
(_a)[0] | (_a)[1] << 8 | (_a)[2] << 16 | (_a)[3] << 24	\
)

#define AR5K_HIGH_ID(_a)	((_a)[4] | (_a)[5] << 8)


#define AR5K_TUNE_DMA_BEACON_RESP		2
#define AR5K_TUNE_SW_BEACON_RESP		10
#define AR5K_TUNE_ADDITIONAL_SWBA_BACKOFF	0
#define AR5K_TUNE_RADAR_ALERT			false
#define AR5K_TUNE_MIN_TX_FIFO_THRES		1
#define AR5K_TUNE_MAX_TX_FIFO_THRES		((IEEE80211_MAX_LEN / 64) + 1)
#define AR5K_TUNE_REGISTER_TIMEOUT		20000

#define AR5K_TUNE_RSSI_THRES			129

#define AR5K_TUNE_BMISS_THRES			7
#define AR5K_TUNE_REGISTER_DWELL_TIME		20000
#define AR5K_TUNE_BEACON_INTERVAL		100
#define AR5K_TUNE_AIFS				2
#define AR5K_TUNE_AIFS_11B			2
#define AR5K_TUNE_AIFS_XR			0
#define AR5K_TUNE_CWMIN				15
#define AR5K_TUNE_CWMIN_11B			31
#define AR5K_TUNE_CWMIN_XR			3
#define AR5K_TUNE_CWMAX				1023
#define AR5K_TUNE_CWMAX_11B			1023
#define AR5K_TUNE_CWMAX_XR			7
#define AR5K_TUNE_NOISE_FLOOR			-72
#define AR5K_TUNE_MAX_TXPOWER			63
#define AR5K_TUNE_DEFAULT_TXPOWER		25
#define AR5K_TUNE_TPC_TXPOWER			false
#define AR5K_TUNE_HWTXTRIES			4

#define AR5K_INIT_CARR_SENSE_EN			1


#if defined(__BIG_ENDIAN)
#define AR5K_INIT_CFG	(		\
	AR5K_CFG_SWTD | AR5K_CFG_SWRD	\
)
#else
#define AR5K_INIT_CFG	0x00000000
#endif


#define	AR5K_INIT_CYCRSSI_THR1			2
#define AR5K_INIT_TX_LATENCY			502
#define AR5K_INIT_USEC				39
#define AR5K_INIT_USEC_TURBO			79
#define AR5K_INIT_USEC_32			31
#define AR5K_INIT_SLOT_TIME			396
#define AR5K_INIT_SLOT_TIME_TURBO		480
#define AR5K_INIT_ACK_CTS_TIMEOUT		1024
#define AR5K_INIT_ACK_CTS_TIMEOUT_TURBO		0x08000800
#define AR5K_INIT_PROG_IFS			920
#define AR5K_INIT_PROG_IFS_TURBO		960
#define AR5K_INIT_EIFS				3440
#define AR5K_INIT_EIFS_TURBO			6880
#define AR5K_INIT_SIFS				560
#define AR5K_INIT_SIFS_TURBO			480
#define AR5K_INIT_SH_RETRY			10
#define AR5K_INIT_LG_RETRY			AR5K_INIT_SH_RETRY
#define AR5K_INIT_SSH_RETRY			32
#define AR5K_INIT_SLG_RETRY			AR5K_INIT_SSH_RETRY
#define AR5K_INIT_TX_RETRY			10

#define AR5K_INIT_TRANSMIT_LATENCY		(			\
	(AR5K_INIT_TX_LATENCY << 14) | (AR5K_INIT_USEC_32 << 7) |	\
	(AR5K_INIT_USEC)						\
)
#define AR5K_INIT_TRANSMIT_LATENCY_TURBO	(			\
	(AR5K_INIT_TX_LATENCY << 14) | (AR5K_INIT_USEC_32 << 7) |	\
	(AR5K_INIT_USEC_TURBO)						\
)
#define AR5K_INIT_PROTO_TIME_CNTRL		(			\
	(AR5K_INIT_CARR_SENSE_EN << 26) | (AR5K_INIT_EIFS << 12) |	\
	(AR5K_INIT_PROG_IFS)						\
)
#define AR5K_INIT_PROTO_TIME_CNTRL_TURBO	(			\
	(AR5K_INIT_CARR_SENSE_EN << 26) | (AR5K_INIT_EIFS_TURBO << 12) | \
	(AR5K_INIT_PROG_IFS_TURBO)					\
)


#define	AR5K_TXQ_USEDEFAULT	((u32) -1)




enum ath5k_version {
	AR5K_AR5210	= 0,
	AR5K_AR5211	= 1,
	AR5K_AR5212	= 2,
};


enum ath5k_radio {
	AR5K_RF5110	= 0,
	AR5K_RF5111	= 1,
	AR5K_RF5112	= 2,
	AR5K_RF2413	= 3,
	AR5K_RF5413	= 4,
	AR5K_RF2316	= 5,
	AR5K_RF2317	= 6,
	AR5K_RF2425	= 7,
};



enum ath5k_srev_type {
	AR5K_VERSION_MAC,
	AR5K_VERSION_RAD,
};

struct ath5k_srev_name {
	const char		*sr_name;
	enum ath5k_srev_type	sr_type;
	u_int			sr_val;
};

#define AR5K_SREV_UNKNOWN	0xffff

#define AR5K_SREV_AR5210	0x00 
#define AR5K_SREV_AR5311	0x10 
#define AR5K_SREV_AR5311A	0x20 
#define AR5K_SREV_AR5311B	0x30 
#define AR5K_SREV_AR5211	0x40 
#define AR5K_SREV_AR5212	0x50 
#define AR5K_SREV_AR5212_V4	0x54 
#define AR5K_SREV_AR5213	0x55 
#define AR5K_SREV_AR5213A	0x59 
#define AR5K_SREV_AR2413	0x78 
#define AR5K_SREV_AR2414	0x70 
#define AR5K_SREV_AR5424	0x90 
#define AR5K_SREV_AR5413	0xa4 
#define AR5K_SREV_AR5414	0xa0 
#define AR5K_SREV_AR2415	0xb0 
#define AR5K_SREV_AR5416	0xc0 
#define AR5K_SREV_AR5418	0xca 
#define AR5K_SREV_AR2425	0xe0 
#define AR5K_SREV_AR2417	0xf0 

#define AR5K_SREV_RAD_5110	0x00
#define AR5K_SREV_RAD_5111	0x10
#define AR5K_SREV_RAD_5111A	0x15
#define AR5K_SREV_RAD_2111	0x20
#define AR5K_SREV_RAD_5112	0x30
#define AR5K_SREV_RAD_5112A	0x35
#define	AR5K_SREV_RAD_5112B	0x36
#define AR5K_SREV_RAD_2112	0x40
#define AR5K_SREV_RAD_2112A	0x45
#define	AR5K_SREV_RAD_2112B	0x46
#define AR5K_SREV_RAD_2413	0x50
#define AR5K_SREV_RAD_5413	0x60
#define AR5K_SREV_RAD_2316	0x70 
#define AR5K_SREV_RAD_2317	0x80
#define AR5K_SREV_RAD_5424	0xa0 
#define AR5K_SREV_RAD_2425	0xa2
#define AR5K_SREV_RAD_5133	0xc0

#define AR5K_SREV_PHY_5211	0x30
#define AR5K_SREV_PHY_5212	0x41
#define	AR5K_SREV_PHY_5212A	0x42
#define AR5K_SREV_PHY_5212B	0x43
#define AR5K_SREV_PHY_2413	0x45
#define AR5K_SREV_PHY_5413	0x61
#define AR5K_SREV_PHY_2425	0x70


#define IEEE80211_MAX_LEN       2500




#define MODULATION_XR 		0x00000200

#define MODULATION_TURBO	0x00000080

enum ath5k_driver_mode {
	AR5K_MODE_11A		=	0,
	AR5K_MODE_11A_TURBO	=	1,
	AR5K_MODE_11B		=	2,
	AR5K_MODE_11G		=	3,
	AR5K_MODE_11G_TURBO	=	4,
	AR5K_MODE_XR		=	0,
	AR5K_MODE_MAX		=	5
};

enum ath5k_ant_mode {
	AR5K_ANTMODE_DEFAULT	= 0,	
	AR5K_ANTMODE_FIXED_A	= 1,	
	AR5K_ANTMODE_FIXED_B	= 2,	
	AR5K_ANTMODE_SINGLE_AP	= 3,	
	AR5K_ANTMODE_SECTOR_AP	= 4,	
	AR5K_ANTMODE_SECTOR_STA	= 5,	
	AR5K_ANTMODE_DEBUG	= 6,	
	AR5K_ANTMODE_MAX,
};





struct ath5k_tx_status {
	u16	ts_seqnum;
	u16	ts_tstamp;
	u8	ts_status;
	u8	ts_rate[4];
	u8	ts_retry[4];
	u8	ts_final_idx;
	s8	ts_rssi;
	u8	ts_shortretry;
	u8	ts_longretry;
	u8	ts_virtcol;
	u8	ts_antenna;
};

#define AR5K_TXSTAT_ALTRATE	0x80
#define AR5K_TXERR_XRETRY	0x01
#define AR5K_TXERR_FILT		0x02
#define AR5K_TXERR_FIFO		0x04


enum ath5k_tx_queue {
	AR5K_TX_QUEUE_INACTIVE = 0,
	AR5K_TX_QUEUE_DATA,
	AR5K_TX_QUEUE_XR_DATA,
	AR5K_TX_QUEUE_BEACON,
	AR5K_TX_QUEUE_CAB,
	AR5K_TX_QUEUE_UAPSD,
};

#define	AR5K_NUM_TX_QUEUES		10
#define	AR5K_NUM_TX_QUEUES_NOQCU	2


enum ath5k_tx_queue_subtype {
	AR5K_WME_AC_BK = 0,	
	AR5K_WME_AC_BE, 	
	AR5K_WME_AC_VI, 	
	AR5K_WME_AC_VO, 	
};


enum ath5k_tx_queue_id {
	AR5K_TX_QUEUE_ID_NOQCU_DATA	= 0,
	AR5K_TX_QUEUE_ID_NOQCU_BEACON	= 1,
	AR5K_TX_QUEUE_ID_DATA_MIN	= 0, 
	AR5K_TX_QUEUE_ID_DATA_MAX	= 4, 
	AR5K_TX_QUEUE_ID_DATA_SVP	= 5, 
	AR5K_TX_QUEUE_ID_CAB		= 6, 
	AR5K_TX_QUEUE_ID_BEACON		= 7, 
	AR5K_TX_QUEUE_ID_UAPSD		= 8,
	AR5K_TX_QUEUE_ID_XR_DATA	= 9,
};


#define AR5K_TXQ_FLAG_TXOKINT_ENABLE		0x0001	
#define AR5K_TXQ_FLAG_TXERRINT_ENABLE		0x0002	
#define AR5K_TXQ_FLAG_TXEOLINT_ENABLE		0x0004	
#define AR5K_TXQ_FLAG_TXDESCINT_ENABLE		0x0008	
#define AR5K_TXQ_FLAG_TXURNINT_ENABLE		0x0010	
#define AR5K_TXQ_FLAG_CBRORNINT_ENABLE		0x0020	
#define AR5K_TXQ_FLAG_CBRURNINT_ENABLE		0x0040	
#define AR5K_TXQ_FLAG_QTRIGINT_ENABLE		0x0080	
#define AR5K_TXQ_FLAG_TXNOFRMINT_ENABLE		0x0100	
#define AR5K_TXQ_FLAG_BACKOFF_DISABLE		0x0200	
#define AR5K_TXQ_FLAG_RDYTIME_EXP_POLICY_ENABLE	0x0300	
#define AR5K_TXQ_FLAG_FRAG_BURST_BACKOFF_ENABLE	0x0800	
#define AR5K_TXQ_FLAG_POST_FR_BKOFF_DIS		0x1000	
#define AR5K_TXQ_FLAG_COMPRESSION_ENABLE	0x2000	


struct ath5k_txq_info {
	enum ath5k_tx_queue tqi_type;
	enum ath5k_tx_queue_subtype tqi_subtype;
	u16	tqi_flags;	
	u32	tqi_aifs;	
	s32	tqi_cw_min;	
	s32	tqi_cw_max;	
	u32	tqi_cbr_period; 
	u32	tqi_cbr_overflow_limit;
	u32	tqi_burst_time;
	u32	tqi_ready_time; 
};


enum ath5k_pkt_type {
	AR5K_PKT_TYPE_NORMAL		= 0,
	AR5K_PKT_TYPE_ATIM		= 1,
	AR5K_PKT_TYPE_PSPOLL		= 2,
	AR5K_PKT_TYPE_BEACON		= 3,
	AR5K_PKT_TYPE_PROBE_RESP	= 4,
	AR5K_PKT_TYPE_PIFS		= 5,
};


#define AR5K_TXPOWER_OFDM(_r, _v)	(			\
	((0 & 1) << ((_v) + 6)) |				\
	(((ah->ah_txpower.txp_rates_power_table[(_r)]) & 0x3f) << (_v))	\
)

#define AR5K_TXPOWER_CCK(_r, _v)	(			\
	(ah->ah_txpower.txp_rates_power_table[(_r)] & 0x3f) << (_v)	\
)


enum ath5k_dmasize {
	AR5K_DMASIZE_4B	= 0,
	AR5K_DMASIZE_8B,
	AR5K_DMASIZE_16B,
	AR5K_DMASIZE_32B,
	AR5K_DMASIZE_64B,
	AR5K_DMASIZE_128B,
	AR5K_DMASIZE_256B,
	AR5K_DMASIZE_512B
};





struct ath5k_rx_status {
	u16	rs_datalen;
	u16	rs_tstamp;
	u8	rs_status;
	u8	rs_phyerr;
	s8	rs_rssi;
	u8	rs_keyix;
	u8	rs_rate;
	u8	rs_antenna;
	u8	rs_more;
};

#define AR5K_RXERR_CRC		0x01
#define AR5K_RXERR_PHY		0x02
#define AR5K_RXERR_FIFO		0x04
#define AR5K_RXERR_DECRYPT	0x08
#define AR5K_RXERR_MIC		0x10
#define AR5K_RXKEYIX_INVALID	((u8) - 1)
#define AR5K_TXKEYIX_INVALID	((u32) - 1)




#define AR5K_BEACON_PERIOD	0x0000ffff
#define AR5K_BEACON_ENA		0x00800000 
#define AR5K_BEACON_RESET_TSF	0x01000000 

#if 0

struct ath5k_beacon_state {
	u32	bs_next_beacon;
	u32	bs_next_dtim;
	u32	bs_interval;
	u8	bs_dtim_period;
	u8	bs_cfp_period;
	u16	bs_cfp_max_duration;
	u16	bs_cfp_du_remain;
	u16	bs_tim_offset;
	u16	bs_sleep_duration;
	u16	bs_bmiss_threshold;
	u32  	bs_cfp_next;
};
#endif



#define TSF_TO_TU(_tsf) (u32)((_tsf) >> 10)




enum ath5k_rfgain {
	AR5K_RFGAIN_INACTIVE = 0,
	AR5K_RFGAIN_ACTIVE,
	AR5K_RFGAIN_READ_REQUESTED,
	AR5K_RFGAIN_NEED_CHANGE,
};

struct ath5k_gain {
	u8			g_step_idx;
	u8			g_current;
	u8			g_target;
	u8			g_low;
	u8			g_high;
	u8			g_f_corr;
	u8			g_state;
};



#define AR5K_SLOT_TIME_9	396
#define AR5K_SLOT_TIME_20	880
#define AR5K_SLOT_TIME_MAX	0xffff


#define	CHANNEL_CW_INT	0x0008	
#define	CHANNEL_TURBO	0x0010	
#define	CHANNEL_CCK	0x0020	
#define	CHANNEL_OFDM	0x0040	
#define	CHANNEL_2GHZ	0x0080	
#define	CHANNEL_5GHZ	0x0100	
#define	CHANNEL_PASSIVE	0x0200	
#define	CHANNEL_DYN	0x0400	
#define	CHANNEL_XR	0x0800	

#define	CHANNEL_A	(CHANNEL_5GHZ|CHANNEL_OFDM)
#define	CHANNEL_B	(CHANNEL_2GHZ|CHANNEL_CCK)
#define	CHANNEL_G	(CHANNEL_2GHZ|CHANNEL_OFDM)
#define	CHANNEL_T	(CHANNEL_5GHZ|CHANNEL_OFDM|CHANNEL_TURBO)
#define	CHANNEL_TG	(CHANNEL_2GHZ|CHANNEL_OFDM|CHANNEL_TURBO)
#define	CHANNEL_108A	CHANNEL_T
#define	CHANNEL_108G	CHANNEL_TG
#define	CHANNEL_X	(CHANNEL_5GHZ|CHANNEL_OFDM|CHANNEL_XR)

#define	CHANNEL_ALL 	(CHANNEL_OFDM|CHANNEL_CCK|CHANNEL_2GHZ|CHANNEL_5GHZ| \
		CHANNEL_TURBO)

#define	CHANNEL_ALL_NOTURBO 	(CHANNEL_ALL & ~CHANNEL_TURBO)
#define CHANNEL_MODES		CHANNEL_ALL


#define IS_CHAN_XR(_c)	((_c->hw_value & CHANNEL_XR) != 0)
#define IS_CHAN_B(_c)	((_c->hw_value & CHANNEL_B) != 0)


struct ath5k_athchan_2ghz {
	u32	a2_flags;
	u16	a2_athchan;
};





#define AR5K_MAX_RATES 32


#define ATH5K_RATE_CODE_1M	0x1B
#define ATH5K_RATE_CODE_2M	0x1A
#define ATH5K_RATE_CODE_5_5M	0x19
#define ATH5K_RATE_CODE_11M	0x18

#define ATH5K_RATE_CODE_6M	0x0B
#define ATH5K_RATE_CODE_9M	0x0F
#define ATH5K_RATE_CODE_12M	0x0A
#define ATH5K_RATE_CODE_18M	0x0E
#define ATH5K_RATE_CODE_24M	0x09
#define ATH5K_RATE_CODE_36M	0x0D
#define ATH5K_RATE_CODE_48M	0x08
#define ATH5K_RATE_CODE_54M	0x0C

#define ATH5K_RATE_CODE_XR_500K	0x07
#define ATH5K_RATE_CODE_XR_1M	0x02
#define ATH5K_RATE_CODE_XR_2M	0x06
#define ATH5K_RATE_CODE_XR_3M	0x01


#define AR5K_SET_SHORT_PREAMBLE 0x04



#define AR5K_KEYCACHE_SIZE	8




#define	AR5K_RSSI_EP_MULTIPLIER	(1<<7)

#define AR5K_ASSERT_ENTRY(_e, _s) do {		\
	if (_e >= _s)				\
		return (false);			\
} while (0)




enum ath5k_int {
	AR5K_INT_RXOK	= 0x00000001,
	AR5K_INT_RXDESC	= 0x00000002,
	AR5K_INT_RXERR	= 0x00000004,
	AR5K_INT_RXNOFRM = 0x00000008,
	AR5K_INT_RXEOL	= 0x00000010,
	AR5K_INT_RXORN	= 0x00000020,
	AR5K_INT_TXOK	= 0x00000040,
	AR5K_INT_TXDESC	= 0x00000080,
	AR5K_INT_TXERR	= 0x00000100,
	AR5K_INT_TXNOFRM = 0x00000200,
	AR5K_INT_TXEOL	= 0x00000400,
	AR5K_INT_TXURN	= 0x00000800,
	AR5K_INT_MIB	= 0x00001000,
	AR5K_INT_SWI	= 0x00002000,
	AR5K_INT_RXPHY	= 0x00004000,
	AR5K_INT_RXKCM	= 0x00008000,
	AR5K_INT_SWBA	= 0x00010000,
	AR5K_INT_BRSSI	= 0x00020000,
	AR5K_INT_BMISS	= 0x00040000,
	AR5K_INT_FATAL	= 0x00080000, 
	AR5K_INT_BNR	= 0x00100000, 
	AR5K_INT_TIM	= 0x00200000, 
	AR5K_INT_DTIM	= 0x00400000, 
	AR5K_INT_DTIM_SYNC =	0x00800000, 
	AR5K_INT_GPIO	=	0x01000000,
	AR5K_INT_BCN_TIMEOUT =	0x02000000, 
	AR5K_INT_CAB_TIMEOUT =	0x04000000, 
	AR5K_INT_RX_DOPPLER =	0x08000000, 
	AR5K_INT_QCBRORN =	0x10000000, 
	AR5K_INT_QCBRURN =	0x20000000, 
	AR5K_INT_QTRIG	=	0x40000000, 
	AR5K_INT_GLOBAL =	0x80000000,

	AR5K_INT_COMMON  = AR5K_INT_RXOK
		| AR5K_INT_RXDESC
		| AR5K_INT_RXERR
		| AR5K_INT_RXNOFRM
		| AR5K_INT_RXEOL
		| AR5K_INT_RXORN
		| AR5K_INT_TXOK
		| AR5K_INT_TXDESC
		| AR5K_INT_TXERR
		| AR5K_INT_TXNOFRM
		| AR5K_INT_TXEOL
		| AR5K_INT_TXURN
		| AR5K_INT_MIB
		| AR5K_INT_SWI
		| AR5K_INT_RXPHY
		| AR5K_INT_RXKCM
		| AR5K_INT_SWBA
		| AR5K_INT_BRSSI
		| AR5K_INT_BMISS
		| AR5K_INT_GPIO
		| AR5K_INT_GLOBAL,

	AR5K_INT_NOCARD	= 0xffffffff
};


enum ath5k_software_interrupt {
	AR5K_SWI_FULL_CALIBRATION = 0x01,
	AR5K_SWI_SHORT_CALIBRATION = 0x02,
};


enum ath5k_power_mode {
	AR5K_PM_UNDEFINED = 0,
	AR5K_PM_AUTO,
	AR5K_PM_AWAKE,
	AR5K_PM_FULL_SLEEP,
	AR5K_PM_NETWORK_SLEEP,
};


#define AR5K_LED_INIT	0 
#define AR5K_LED_SCAN	1 
#define AR5K_LED_AUTH	2 
#define AR5K_LED_ASSOC	3 
#define AR5K_LED_RUN	4 


#define AR5K_SOFTLED_PIN	0
#define AR5K_SOFTLED_ON		0
#define AR5K_SOFTLED_OFF	1


enum ath5k_capability_type {
	AR5K_CAP_REG_DMN		= 0,	
	AR5K_CAP_TKIP_MIC		= 2,	
	AR5K_CAP_TKIP_SPLIT		= 3,	
	AR5K_CAP_PHYCOUNTERS		= 4,	
	AR5K_CAP_DIVERSITY		= 5,	
	AR5K_CAP_NUM_TXQUEUES		= 6,	
	AR5K_CAP_VEOL			= 7,	
	AR5K_CAP_COMPRESSION		= 8,	
	AR5K_CAP_BURST			= 9,	
	AR5K_CAP_FASTFRAME		= 10,	
	AR5K_CAP_TXPOW			= 11,	
	AR5K_CAP_TPC			= 12,	
	AR5K_CAP_BSSIDMASK		= 13,	
	AR5K_CAP_MCAST_KEYSRCH		= 14,	
	AR5K_CAP_TSF_ADJUST		= 15,	
	AR5K_CAP_XR			= 16,	
	AR5K_CAP_WME_TKIPMIC 		= 17,	
	AR5K_CAP_CHAN_HALFRATE 		= 18,	
	AR5K_CAP_CHAN_QUARTERRATE 	= 19,	
	AR5K_CAP_RFSILENT		= 20,	
};



struct ath5k_capabilities {
	
	DECLARE_BITMAP(cap_mode, AR5K_MODE_MAX);

	
	struct {
		u16	range_2ghz_min;
		u16	range_2ghz_max;
		u16	range_5ghz_min;
		u16	range_5ghz_max;
	} cap_range;

	
	struct ath5k_eeprom_info	cap_eeprom;

	
	struct {
		u8	q_tx_num;
	} cap_queues;
};






#define AR5K_MAX_GPIO		10
#define AR5K_MAX_RF_BANKS	8


struct ath5k_hw {
	u32			ah_magic;

	struct ath5k_softc	*ah_sc;
	void __iomem		*ah_iobase;

	enum ath5k_int		ah_imr;

	enum nl80211_iftype	ah_op_mode;
	struct ieee80211_channel *ah_current_channel;
	bool			ah_turbo;
	bool			ah_calibration;
	bool			ah_single_chip;
	bool			ah_aes_support;
	bool			ah_combined_mic;

	enum ath5k_version	ah_version;
	enum ath5k_radio	ah_radio;
	u32			ah_phy;
	u32			ah_mac_srev;
	u16			ah_mac_version;
	u16			ah_mac_revision;
	u16			ah_phy_revision;
	u16			ah_radio_5ghz_revision;
	u16			ah_radio_2ghz_revision;

#define ah_modes		ah_capabilities.cap_mode
#define ah_ee_version		ah_capabilities.cap_eeprom.ee_version

	u32			ah_atim_window;
	u32			ah_aifs;
	u32			ah_cw_min;
	u32			ah_cw_max;
	u32			ah_limit_tx_retries;

	
	u32			ah_ant_ctl[AR5K_EEPROM_N_MODES][AR5K_ANT_MAX];
	u8			ah_ant_mode;
	u8			ah_tx_ant;
	u8			ah_def_ant;
	bool			ah_software_retry;

	u8			ah_sta_id[ETH_ALEN];

	
	u8			ah_bssid[ETH_ALEN];
	u8			ah_bssid_mask[ETH_ALEN];

	int			ah_gpio_npins;

	struct ath5k_capabilities ah_capabilities;

	struct ath5k_txq_info	ah_txq[AR5K_NUM_TX_QUEUES];
	u32			ah_txq_status;
	u32			ah_txq_imr_txok;
	u32			ah_txq_imr_txerr;
	u32			ah_txq_imr_txurn;
	u32			ah_txq_imr_txdesc;
	u32			ah_txq_imr_txeol;
	u32			ah_txq_imr_cbrorn;
	u32			ah_txq_imr_cbrurn;
	u32			ah_txq_imr_qtrig;
	u32			ah_txq_imr_nofrm;
	u32			ah_txq_isr;
	u32			*ah_rf_banks;
	size_t			ah_rf_banks_size;
	size_t			ah_rf_regs_count;
	struct ath5k_gain	ah_gain;
	u8			ah_offset[AR5K_MAX_RF_BANKS];


	struct {
		
		u8		tmpL[AR5K_EEPROM_N_PD_GAINS]
					[AR5K_EEPROM_POWER_TABLE_SIZE];
		u8		tmpR[AR5K_EEPROM_N_PD_GAINS]
					[AR5K_EEPROM_POWER_TABLE_SIZE];
		u8		txp_pd_table[AR5K_EEPROM_POWER_TABLE_SIZE * 2];
		u16		txp_rates_power_table[AR5K_MAX_RATES];
		u8		txp_min_idx;
		bool		txp_tpc;
		
		s16		txp_min_pwr;
		s16		txp_max_pwr;
		
		s16		txp_offset;
		s16		txp_ofdm;
		s16		txp_cck_ofdm_gainf_delta;
		
		s16		txp_cck_ofdm_pwr_delta;
	} ah_txpower;

	struct {
		bool		r_enabled;
		int		r_last_alert;
		struct ieee80211_channel r_last_channel;
	} ah_radar;

	
	s32			ah_noise_floor;

	
	unsigned long		ah_cal_tstamp;

	
	u8			ah_cal_intval;

	
	u8			ah_swi_mask;

	
	int (*ah_setup_rx_desc)(struct ath5k_hw *ah, struct ath5k_desc *desc,
				u32 size, unsigned int flags);
	int (*ah_setup_tx_desc)(struct ath5k_hw *, struct ath5k_desc *,
		unsigned int, unsigned int, enum ath5k_pkt_type, unsigned int,
		unsigned int, unsigned int, unsigned int, unsigned int,
		unsigned int, unsigned int, unsigned int);
	int (*ah_setup_mrr_tx_desc)(struct ath5k_hw *, struct ath5k_desc *,
		unsigned int, unsigned int, unsigned int, unsigned int,
		unsigned int, unsigned int);
	int (*ah_proc_tx_desc)(struct ath5k_hw *, struct ath5k_desc *,
		struct ath5k_tx_status *);
	int (*ah_proc_rx_desc)(struct ath5k_hw *, struct ath5k_desc *,
		struct ath5k_rx_status *);
};




extern struct ath5k_hw *ath5k_hw_attach(struct ath5k_softc *sc);
extern void ath5k_hw_detach(struct ath5k_hw *ah);


extern int ath5k_init_leds(struct ath5k_softc *sc);
extern void ath5k_led_enable(struct ath5k_softc *sc);
extern void ath5k_led_off(struct ath5k_softc *sc);
extern void ath5k_unregister_leds(struct ath5k_softc *sc);


extern int ath5k_hw_nic_wakeup(struct ath5k_hw *ah, int flags, bool initial);
extern int ath5k_hw_on_hold(struct ath5k_hw *ah);
extern int ath5k_hw_reset(struct ath5k_hw *ah, enum nl80211_iftype op_mode, struct ieee80211_channel *channel, bool change_channel);

extern int ath5k_hw_set_power(struct ath5k_hw *ah, enum ath5k_power_mode mode, bool set_chip, u16 sleep_duration);


extern void ath5k_hw_start_rx_dma(struct ath5k_hw *ah);
extern int ath5k_hw_stop_rx_dma(struct ath5k_hw *ah);
extern u32 ath5k_hw_get_rxdp(struct ath5k_hw *ah);
extern void ath5k_hw_set_rxdp(struct ath5k_hw *ah, u32 phys_addr);
extern int ath5k_hw_start_tx_dma(struct ath5k_hw *ah, unsigned int queue);
extern int ath5k_hw_stop_tx_dma(struct ath5k_hw *ah, unsigned int queue);
extern u32 ath5k_hw_get_txdp(struct ath5k_hw *ah, unsigned int queue);
extern int ath5k_hw_set_txdp(struct ath5k_hw *ah, unsigned int queue,
				u32 phys_addr);
extern int ath5k_hw_update_tx_triglevel(struct ath5k_hw *ah, bool increase);

extern bool ath5k_hw_is_intr_pending(struct ath5k_hw *ah);
extern int ath5k_hw_get_isr(struct ath5k_hw *ah, enum ath5k_int *interrupt_mask);
extern enum ath5k_int ath5k_hw_set_imr(struct ath5k_hw *ah, enum
ath5k_int new_mask);
extern void ath5k_hw_update_mib_counters(struct ath5k_hw *ah, struct ieee80211_low_level_stats *stats);


extern int ath5k_eeprom_init(struct ath5k_hw *ah);
extern void ath5k_eeprom_detach(struct ath5k_hw *ah);
extern int ath5k_eeprom_read_mac(struct ath5k_hw *ah, u8 *mac);
extern bool ath5k_eeprom_is_hb63(struct ath5k_hw *ah);


extern int ath5k_hw_set_opmode(struct ath5k_hw *ah);

extern void ath5k_hw_get_lladdr(struct ath5k_hw *ah, u8 *mac);
extern int ath5k_hw_set_lladdr(struct ath5k_hw *ah, const u8 *mac);
extern void ath5k_hw_set_associd(struct ath5k_hw *ah, const u8 *bssid, u16 assoc_id);
extern int ath5k_hw_set_bssid_mask(struct ath5k_hw *ah, const u8 *mask);

extern void ath5k_hw_start_rx_pcu(struct ath5k_hw *ah);
extern void ath5k_hw_stop_rx_pcu(struct ath5k_hw *ah);

extern void ath5k_hw_set_mcast_filter(struct ath5k_hw *ah, u32 filter0, u32 filter1);
extern int ath5k_hw_set_mcast_filter_idx(struct ath5k_hw *ah, u32 index);
extern int ath5k_hw_clear_mcast_filter_idx(struct ath5k_hw *ah, u32 index);
extern u32 ath5k_hw_get_rx_filter(struct ath5k_hw *ah);
extern void ath5k_hw_set_rx_filter(struct ath5k_hw *ah, u32 filter);

extern u32 ath5k_hw_get_tsf32(struct ath5k_hw *ah);
extern u64 ath5k_hw_get_tsf64(struct ath5k_hw *ah);
extern void ath5k_hw_set_tsf64(struct ath5k_hw *ah, u64 tsf64);
extern void ath5k_hw_reset_tsf(struct ath5k_hw *ah);
extern void ath5k_hw_init_beacon(struct ath5k_hw *ah, u32 next_beacon, u32 interval);
#if 0
extern int ath5k_hw_set_beacon_timers(struct ath5k_hw *ah, const struct ath5k_beacon_state *state);
extern void ath5k_hw_reset_beacon(struct ath5k_hw *ah);
extern int ath5k_hw_beaconq_finish(struct ath5k_hw *ah, unsigned long phys_addr);
#endif

void ath5k_hw_set_ack_bitrate_high(struct ath5k_hw *ah, bool high);

extern int ath5k_hw_set_ack_timeout(struct ath5k_hw *ah, unsigned int timeout);
extern unsigned int ath5k_hw_get_ack_timeout(struct ath5k_hw *ah);
extern int ath5k_hw_set_cts_timeout(struct ath5k_hw *ah, unsigned int timeout);
extern unsigned int ath5k_hw_get_cts_timeout(struct ath5k_hw *ah);

extern int ath5k_hw_reset_key(struct ath5k_hw *ah, u16 entry);
extern int ath5k_hw_is_key_valid(struct ath5k_hw *ah, u16 entry);
extern int ath5k_hw_set_key(struct ath5k_hw *ah, u16 entry, const struct ieee80211_key_conf *key, const u8 *mac);
extern int ath5k_hw_set_key_lladdr(struct ath5k_hw *ah, u16 entry, const u8 *mac);


extern int ath5k_hw_get_tx_queueprops(struct ath5k_hw *ah, int queue, struct ath5k_txq_info *queue_info);
extern int ath5k_hw_set_tx_queueprops(struct ath5k_hw *ah, int queue,
				const struct ath5k_txq_info *queue_info);
extern int ath5k_hw_setup_tx_queue(struct ath5k_hw *ah,
				enum ath5k_tx_queue queue_type,
				struct ath5k_txq_info *queue_info);
extern u32 ath5k_hw_num_tx_pending(struct ath5k_hw *ah, unsigned int queue);
extern void ath5k_hw_release_tx_queue(struct ath5k_hw *ah, unsigned int queue);
extern int ath5k_hw_reset_tx_queue(struct ath5k_hw *ah, unsigned int queue);
extern unsigned int ath5k_hw_get_slot_time(struct ath5k_hw *ah);
extern int ath5k_hw_set_slot_time(struct ath5k_hw *ah, unsigned int slot_time);


extern int ath5k_hw_init_desc_functions(struct ath5k_hw *ah);


extern void ath5k_hw_set_ledstate(struct ath5k_hw *ah, unsigned int state);
extern int ath5k_hw_set_gpio_input(struct ath5k_hw *ah, u32 gpio);
extern int ath5k_hw_set_gpio_output(struct ath5k_hw *ah, u32 gpio);
extern u32 ath5k_hw_get_gpio(struct ath5k_hw *ah, u32 gpio);
extern int ath5k_hw_set_gpio(struct ath5k_hw *ah, u32 gpio, u32 val);
extern void ath5k_hw_set_gpio_intr(struct ath5k_hw *ah, unsigned int gpio, u32 interrupt_level);


extern void ath5k_rfkill_hw_start(struct ath5k_hw *ah);
extern void ath5k_rfkill_hw_stop(struct ath5k_hw *ah);


int ath5k_hw_set_capabilities(struct ath5k_hw *ah);
extern int ath5k_hw_get_capability(struct ath5k_hw *ah, enum ath5k_capability_type cap_type, u32 capability, u32 *result);
extern int ath5k_hw_enable_pspoll(struct ath5k_hw *ah, u8 *bssid, u16 assoc_id);
extern int ath5k_hw_disable_pspoll(struct ath5k_hw *ah);


extern int ath5k_hw_write_initvals(struct ath5k_hw *ah, u8 mode, bool change_channel);


extern int ath5k_hw_rfregs_init(struct ath5k_hw *ah,
				struct ieee80211_channel *channel,
				unsigned int mode);
extern int ath5k_hw_rfgain_init(struct ath5k_hw *ah, unsigned int freq);
extern enum ath5k_rfgain ath5k_hw_gainf_calibrate(struct ath5k_hw *ah);
extern int ath5k_hw_rfgain_opt_init(struct ath5k_hw *ah);

extern bool ath5k_channel_ok(struct ath5k_hw *ah, u16 freq, unsigned int flags);
extern int ath5k_hw_channel(struct ath5k_hw *ah, struct ieee80211_channel *channel);

extern int ath5k_hw_phy_calibrate(struct ath5k_hw *ah, struct ieee80211_channel *channel);
extern int ath5k_hw_noise_floor_calibration(struct ath5k_hw *ah, short freq);
extern void ath5k_hw_calibration_poll(struct ath5k_hw *ah);

bool ath5k_hw_chan_has_spur_noise(struct ath5k_hw *ah,
				struct ieee80211_channel *channel);
void ath5k_hw_set_spur_mitigation_filter(struct ath5k_hw *ah,
				struct ieee80211_channel *channel);

extern u16 ath5k_hw_radio_revision(struct ath5k_hw *ah, unsigned int chan);
extern int ath5k_hw_phy_disable(struct ath5k_hw *ah);

extern void ath5k_hw_set_antenna_mode(struct ath5k_hw *ah, u8 ant_mode);
extern void ath5k_hw_set_def_antenna(struct ath5k_hw *ah, u8 ant);
extern unsigned int ath5k_hw_get_def_antenna(struct ath5k_hw *ah);

extern int ath5k_hw_txpower(struct ath5k_hw *ah, struct ieee80211_channel *channel, u8 ee_mode, u8 txpower);
extern int ath5k_hw_set_txpower_limit(struct ath5k_hw *ah, u8 txpower);




static inline unsigned int ath5k_hw_htoclock(unsigned int usec, bool turbo)
{
	return turbo ? (usec * 80) : (usec * 40);
}


static inline unsigned int ath5k_hw_clocktoh(unsigned int clock, bool turbo)
{
	return turbo ? (clock / 80) : (clock / 40);
}


static inline u32 ath5k_hw_reg_read(struct ath5k_hw *ah, u16 reg)
{
	return ioread32(ah->ah_iobase + reg);
}


static inline void ath5k_hw_reg_write(struct ath5k_hw *ah, u32 val, u16 reg)
{
	iowrite32(val, ah->ah_iobase + reg);
}

#if defined(_ATH5K_RESET) || defined(_ATH5K_PHY)

static int ath5k_hw_register_timeout(struct ath5k_hw *ah, u32 reg, u32 flag,
		u32 val, bool is_set)
{
	int i;
	u32 data;

	for (i = AR5K_TUNE_REGISTER_TIMEOUT; i > 0; i--) {
		data = ath5k_hw_reg_read(ah, reg);
		if (is_set && (data & flag))
			break;
		else if ((data & flag) == val)
			break;
		udelay(15);
	}

	return (i <= 0) ? -EAGAIN : 0;
}
#endif

static inline u32 ath5k_hw_bitswap(u32 val, unsigned int bits)
{
	u32 retval = 0, bit, i;

	for (i = 0; i < bits; i++) {
		bit = (val >> i) & 1;
		retval = (retval << 1) | bit;
	}

	return retval;
}

static inline int ath5k_pad_size(int hdrlen)
{
	return (hdrlen < 24) ? 0 : hdrlen & 3;
}

#endif
