

#include "ath9k.h"

static const struct ath_rate_table ar5416_11na_ratetable = {
	42,
	{
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 6000, 
			5400, 0x0b, 0x00, 12,
			0, 0, 0, 0, 0, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 9000, 
			7800,  0x0f, 0x00, 18,
			0, 1, 1, 1, 1, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 12000, 
			10000, 0x0a, 0x00, 24,
			2, 2, 2, 2, 2, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 18000, 
			13900, 0x0e, 0x00, 36,
			2,  3, 3, 3, 3, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 24000, 
			17300, 0x09, 0x00, 48,
			4,  4, 4, 4, 4, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 36000, 
			23000, 0x0d, 0x00, 72,
			4,  5, 5, 5, 5, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 48000, 
			27400, 0x08, 0x00, 96,
			4,  6, 6, 6, 6, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 54000, 
			29300, 0x0c, 0x00, 108,
			4,  7, 7, 7, 7, 0 },
		{ VALID_2040, VALID_2040, WLAN_RC_PHY_HT_20_SS, 6500, 
			6400, 0x80, 0x00, 0,
			0, 8, 24, 8, 24, 3216 },
		{ VALID_20, VALID_20, WLAN_RC_PHY_HT_20_SS, 13000, 
			12700, 0x81, 0x00, 1,
			2, 9, 25, 9, 25, 6434 },
		{ VALID_20, VALID_20, WLAN_RC_PHY_HT_20_SS, 19500, 
			18800, 0x82, 0x00, 2,
			2, 10, 26, 10, 26, 9650 },
		{ VALID_20, VALID_20, WLAN_RC_PHY_HT_20_SS, 26000, 
			25000, 0x83, 0x00, 3,
			4,  11, 27, 11, 27, 12868 },
		{ VALID_20, VALID_20, WLAN_RC_PHY_HT_20_SS, 39000, 
			36700, 0x84, 0x00, 4,
			4,  12, 28, 12, 28, 19304 },
		{ INVALID, VALID_20, WLAN_RC_PHY_HT_20_SS, 52000, 
			48100, 0x85, 0x00, 5,
			4,  13, 29, 13, 29, 25740 },
		{ INVALID, VALID_20, WLAN_RC_PHY_HT_20_SS, 58500, 
			53500, 0x86, 0x00, 6,
			4,  14, 30, 14, 30,  28956 },
		{ INVALID, VALID_20, WLAN_RC_PHY_HT_20_SS, 65000, 
			59000, 0x87, 0x00, 7,
			4,  15, 31, 15, 32, 32180 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_20_DS, 13000, 
			12700, 0x88, 0x00,
			8, 3, 16, 33, 16, 33, 6430 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_20_DS, 26000, 
			24800, 0x89, 0x00, 9,
			2, 17, 34, 17, 34, 12860 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_20_DS, 39000, 
			36600, 0x8a, 0x00, 10,
			2, 18, 35, 18, 35, 19300 },
		{ VALID_20, INVALID, WLAN_RC_PHY_HT_20_DS, 52000, 
			48100, 0x8b, 0x00, 11,
			4,  19, 36, 19, 36, 25736 },
		{ VALID_20, INVALID, WLAN_RC_PHY_HT_20_DS, 78000, 
			69500, 0x8c, 0x00, 12,
			4,  20, 37, 20, 37, 38600 },
		{ VALID_20, INVALID, WLAN_RC_PHY_HT_20_DS, 104000, 
			89500, 0x8d, 0x00, 13,
			4,  21, 38, 21, 38, 51472 },
		{ VALID_20, INVALID, WLAN_RC_PHY_HT_20_DS, 117000, 
			98900, 0x8e, 0x00, 14,
			4,  22, 39, 22, 39, 57890 },
		{ VALID_20, INVALID, WLAN_RC_PHY_HT_20_DS, 130000, 
			108300, 0x8f, 0x00, 15,
			4,  23, 40, 23, 41, 64320 },
		{ VALID_40, VALID_40, WLAN_RC_PHY_HT_40_SS, 13500, 
			13200, 0x80, 0x00, 0,
			0, 8, 24, 24, 24, 6684 },
		{ VALID_40, VALID_40, WLAN_RC_PHY_HT_40_SS, 27500, 
			25900, 0x81, 0x00, 1,
			2, 9, 25, 25, 25, 13368 },
		{ VALID_40, VALID_40, WLAN_RC_PHY_HT_40_SS, 40500, 
			38600, 0x82, 0x00, 2,
			2, 10, 26, 26, 26, 20052 },
		{ VALID_40, VALID_40, WLAN_RC_PHY_HT_40_SS, 54000, 
			49800, 0x83, 0x00, 3,
			4,  11, 27, 27, 27, 26738 },
		{ VALID_40, VALID_40, WLAN_RC_PHY_HT_40_SS, 81500, 
			72200, 0x84, 0x00, 4,
			4,  12, 28, 28, 28, 40104 },
		{ INVALID, VALID_40, WLAN_RC_PHY_HT_40_SS, 108000, 
			92900, 0x85, 0x00, 5,
			4,  13, 29, 29, 29, 53476 },
		{ INVALID, VALID_40, WLAN_RC_PHY_HT_40_SS, 121500, 
			102700, 0x86, 0x00, 6,
			4,  14, 30, 30, 30, 60156 },
		{ INVALID, VALID_40, WLAN_RC_PHY_HT_40_SS, 135000, 
			112000, 0x87, 0x00, 7,
			4,  15, 31, 32, 32, 66840 },
		{ INVALID, VALID_40, WLAN_RC_PHY_HT_40_SS_HGI, 150000, 
			122000, 0x87, 0x00, 7,
			4,  15, 31, 32, 32, 74200 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_40_DS, 27000, 
			25800, 0x88, 0x00, 8,
			0, 16, 33, 33, 33, 13360 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_40_DS, 54000, 
			49800, 0x89, 0x00, 9,
			2, 17, 34, 34, 34, 26720 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_40_DS, 81000, 
			71900, 0x8a, 0x00, 10,
			2, 18, 35, 35, 35, 40080 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS, 108000, 
			92500, 0x8b, 0x00, 11,
			4,  19, 36, 36, 36, 53440 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS, 162000, 
			130300, 0x8c, 0x00, 12,
			4,  20, 37, 37, 37, 80160 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS, 216000, 
			162800, 0x8d, 0x00, 13,
			4,  21, 38, 38, 38, 106880 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS, 243000, 
			178200, 0x8e, 0x00, 14,
			4,  22, 39, 39, 39, 120240 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS, 270000, 
			192100, 0x8f, 0x00, 15,
			4,  23, 40, 41, 41, 133600 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS_HGI, 300000, 
			207000, 0x8f, 0x00, 15,
			4,  23, 40, 41, 41, 148400 },
	},
	50,  
	WLAN_RC_HT_FLAG,  
};



static const struct ath_rate_table ar5416_11ng_ratetable = {
	46,
	{
		{ VALID_ALL, VALID_ALL, WLAN_RC_PHY_CCK, 1000, 
			900, 0x1b, 0x00, 2,
			0, 0, 0, 0, 0, 0 },
		{ VALID_ALL, VALID_ALL, WLAN_RC_PHY_CCK, 2000, 
			1900, 0x1a, 0x04, 4,
			1, 1, 1, 1, 1, 0 },
		{ VALID_ALL, VALID_ALL, WLAN_RC_PHY_CCK, 5500, 
			4900, 0x19, 0x04, 11,
			2, 2, 2, 2, 2, 0 },
		{ VALID_ALL, VALID_ALL, WLAN_RC_PHY_CCK, 11000, 
			8100, 0x18, 0x04, 22,
			3, 3, 3, 3, 3, 0 },
		{ INVALID, INVALID, WLAN_RC_PHY_OFDM, 6000, 
			5400, 0x0b, 0x00, 12,
			4, 4, 4, 4, 4, 0 },
		{ INVALID, INVALID, WLAN_RC_PHY_OFDM, 9000, 
			7800, 0x0f, 0x00, 18,
			4, 5, 5, 5, 5, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 12000, 
			10100, 0x0a, 0x00, 24,
			6, 6, 6, 6, 6, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 18000, 
			14100,  0x0e, 0x00, 36,
			6, 7, 7, 7, 7, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 24000, 
			17700, 0x09, 0x00, 48,
			8,  8, 8, 8, 8, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 36000, 
			23700, 0x0d, 0x00, 72,
			8,  9, 9, 9, 9, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 48000, 
			27400, 0x08, 0x00, 96,
			8,  10, 10, 10, 10, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 54000, 
			30900, 0x0c, 0x00, 108,
			8,  11, 11, 11, 11, 0 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_20_SS, 6500, 
			6400, 0x80, 0x00, 0,
			4, 12, 28, 12, 28, 3216 },
		{ VALID_20, VALID_20, WLAN_RC_PHY_HT_20_SS, 13000, 
			12700, 0x81, 0x00, 1,
			6, 13, 29, 13, 29, 6434 },
		{ VALID_20, VALID_20, WLAN_RC_PHY_HT_20_SS, 19500, 
			18800, 0x82, 0x00, 2,
			6, 14, 30, 14, 30, 9650 },
		{ VALID_20, VALID_20, WLAN_RC_PHY_HT_20_SS, 26000, 
			25000, 0x83, 0x00, 3,
			8,  15, 31, 15, 31, 12868 },
		{ VALID_20, VALID_20, WLAN_RC_PHY_HT_20_SS, 39000, 
			36700, 0x84, 0x00, 4,
			8,  16, 32, 16, 32, 19304 },
		{ INVALID, VALID_20, WLAN_RC_PHY_HT_20_SS, 52000, 
			48100, 0x85, 0x00, 5,
			8,  17, 33, 17, 33, 25740 },
		{ INVALID,  VALID_20, WLAN_RC_PHY_HT_20_SS, 58500, 
			53500, 0x86, 0x00, 6,
			8,  18, 34, 18, 34, 28956 },
		{ INVALID, VALID_20, WLAN_RC_PHY_HT_20_SS, 65000, 
			59000, 0x87, 0x00, 7,
			8,  19, 35, 19, 36, 32180 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_20_DS, 13000, 
			12700, 0x88, 0x00, 8,
			4, 20, 37, 20, 37, 6430 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_20_DS, 26000, 
			24800, 0x89, 0x00, 9,
			6, 21, 38, 21, 38, 12860 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_20_DS, 39000, 
			36600, 0x8a, 0x00, 10,
			6, 22, 39, 22, 39, 19300 },
		{ VALID_20, INVALID, WLAN_RC_PHY_HT_20_DS, 52000, 
			48100, 0x8b, 0x00, 11,
			8,  23, 40, 23, 40, 25736 },
		{ VALID_20, INVALID, WLAN_RC_PHY_HT_20_DS, 78000, 
			69500, 0x8c, 0x00, 12,
			8,  24, 41, 24, 41, 38600 },
		{ VALID_20, INVALID, WLAN_RC_PHY_HT_20_DS, 104000, 
			89500, 0x8d, 0x00, 13,
			8,  25, 42, 25, 42, 51472 },
		{ VALID_20, INVALID, WLAN_RC_PHY_HT_20_DS, 117000, 
			98900, 0x8e, 0x00, 14,
			8,  26, 43, 26, 44, 57890 },
		{ VALID_20, INVALID, WLAN_RC_PHY_HT_20_DS, 130000, 
			108300, 0x8f, 0x00, 15,
			8,  27, 44, 27, 45, 64320 },
		{ VALID_40, VALID_40, WLAN_RC_PHY_HT_40_SS, 13500, 
			13200, 0x80, 0x00, 0,
			8, 12, 28, 28, 28, 6684 },
		{ VALID_40, VALID_40, WLAN_RC_PHY_HT_40_SS, 27500, 
			25900, 0x81, 0x00, 1,
			8, 13, 29, 29, 29, 13368 },
		{ VALID_40, VALID_40, WLAN_RC_PHY_HT_40_SS, 40500, 
			38600, 0x82, 0x00, 2,
			8, 14, 30, 30, 30, 20052 },
		{ VALID_40, VALID_40, WLAN_RC_PHY_HT_40_SS, 54000, 
			49800, 0x83, 0x00, 3,
			8,  15, 31, 31, 31, 26738 },
		{ VALID_40, VALID_40, WLAN_RC_PHY_HT_40_SS, 81500, 
			72200, 0x84, 0x00, 4,
			8,  16, 32, 32, 32, 40104 },
		{ INVALID, VALID_40, WLAN_RC_PHY_HT_40_SS, 108000, 
			92900, 0x85, 0x00, 5,
			8,  17, 33, 33, 33, 53476 },
		{ INVALID,  VALID_40, WLAN_RC_PHY_HT_40_SS, 121500, 
			102700, 0x86, 0x00, 6,
			8,  18, 34, 34, 34, 60156 },
		{ INVALID, VALID_40, WLAN_RC_PHY_HT_40_SS, 135000, 
			112000, 0x87, 0x00, 7,
			8,  19, 35, 36, 36, 66840 },
		{ INVALID, VALID_40, WLAN_RC_PHY_HT_40_SS_HGI, 150000, 
			122000, 0x87, 0x00, 7,
			8,  19, 35, 36, 36, 74200 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_40_DS, 27000, 
			25800, 0x88, 0x00, 8,
			8, 20, 37, 37, 37, 13360 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_40_DS, 54000, 
			49800, 0x89, 0x00, 9,
			8, 21, 38, 38, 38, 26720 },
		{ INVALID, INVALID, WLAN_RC_PHY_HT_40_DS, 81000, 
			71900, 0x8a, 0x00, 10,
			8, 22, 39, 39, 39, 40080 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS, 108000, 
			92500, 0x8b, 0x00, 11,
			8,  23, 40, 40, 40, 53440 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS, 162000, 
			130300, 0x8c, 0x00, 12,
			8,  24, 41, 41, 41, 80160 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS, 216000, 
			162800, 0x8d, 0x00, 13,
			8,  25, 42, 42, 42, 106880 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS, 243000, 
			178200, 0x8e, 0x00, 14,
			8,  26, 43, 43, 43, 120240 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS, 270000, 
			192100, 0x8f, 0x00, 15,
			8,  27, 44, 45, 45, 133600 },
		{ VALID_40, INVALID, WLAN_RC_PHY_HT_40_DS_HGI, 300000, 
			207000, 0x8f, 0x00, 15,
			8,  27, 44, 45, 45, 148400 },
		},
	50,  
	WLAN_RC_HT_FLAG,  
};

static const struct ath_rate_table ar5416_11a_ratetable = {
	8,
	{
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 6000, 
			5400, 0x0b, 0x00, (0x80|12),
			0, 0, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 9000, 
			7800, 0x0f, 0x00, 18,
			0, 1, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 12000, 
			10000, 0x0a, 0x00, (0x80|24),
			2, 2, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 18000, 
			13900, 0x0e, 0x00, 36,
			2, 3, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 24000, 
			17300, 0x09, 0x00, (0x80|48),
			4,  4, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 36000, 
			23000, 0x0d, 0x00, 72,
			4,  5, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 48000, 
			27400, 0x08, 0x00, 96,
			4,  6, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 54000, 
			29300, 0x0c, 0x00, 108,
			4,  7, 0 },
	},
	50,  
	0,   
};

static const struct ath_rate_table ar5416_11g_ratetable = {
	12,
	{
		{ VALID, VALID, WLAN_RC_PHY_CCK, 1000, 
			900, 0x1b, 0x00, 2,
			0, 0, 0 },
		{ VALID, VALID, WLAN_RC_PHY_CCK, 2000, 
			1900, 0x1a, 0x04, 4,
			1, 1, 0 },
		{ VALID, VALID, WLAN_RC_PHY_CCK, 5500, 
			4900, 0x19, 0x04, 11,
			2, 2, 0 },
		{ VALID, VALID, WLAN_RC_PHY_CCK, 11000, 
			8100, 0x18, 0x04, 22,
			3, 3, 0 },
		{ INVALID, INVALID, WLAN_RC_PHY_OFDM, 6000, 
			5400, 0x0b, 0x00, 12,
			4, 4, 0 },
		{ INVALID, INVALID, WLAN_RC_PHY_OFDM, 9000, 
			7800, 0x0f, 0x00, 18,
			4, 5, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 12000, 
			10000, 0x0a, 0x00, 24,
			6, 6, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 18000, 
			13900, 0x0e, 0x00, 36,
			6, 7, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 24000, 
			17300, 0x09, 0x00, 48,
			8,  8, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 36000, 
			23000, 0x0d, 0x00, 72,
			8,  9, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 48000, 
			27400, 0x08, 0x00, 96,
			8,  10, 0 },
		{ VALID, VALID, WLAN_RC_PHY_OFDM, 54000, 
			29300, 0x0c, 0x00, 108,
			8,  11, 0 },
	},
	50,  
	0,   
};

static inline int8_t median(int8_t a, int8_t b, int8_t c)
{
	if (a >= b) {
		if (b >= c)
			return b;
		else if (a > c)
			return c;
		else
			return a;
	} else {
		if (a >= c)
			return a;
		else if (b >= c)
			return c;
		else
			return b;
	}
}

static void ath_rc_sort_validrates(const struct ath_rate_table *rate_table,
				   struct ath_rate_priv *ath_rc_priv)
{
	u8 i, j, idx, idx_next;

	for (i = ath_rc_priv->max_valid_rate - 1; i > 0; i--) {
		for (j = 0; j <= i-1; j++) {
			idx = ath_rc_priv->valid_rate_index[j];
			idx_next = ath_rc_priv->valid_rate_index[j+1];

			if (rate_table->info[idx].ratekbps >
				rate_table->info[idx_next].ratekbps) {
				ath_rc_priv->valid_rate_index[j] = idx_next;
				ath_rc_priv->valid_rate_index[j+1] = idx;
			}
		}
	}
}

static void ath_rc_init_valid_txmask(struct ath_rate_priv *ath_rc_priv)
{
	u8 i;

	for (i = 0; i < ath_rc_priv->rate_table_size; i++)
		ath_rc_priv->valid_rate_index[i] = 0;
}

static inline void ath_rc_set_valid_txmask(struct ath_rate_priv *ath_rc_priv,
					   u8 index, int valid_tx_rate)
{
	ASSERT(index <= ath_rc_priv->rate_table_size);
	ath_rc_priv->valid_rate_index[index] = valid_tx_rate ? 1 : 0;
}

static inline
int ath_rc_get_nextvalid_txrate(const struct ath_rate_table *rate_table,
				struct ath_rate_priv *ath_rc_priv,
				u8 cur_valid_txrate,
				u8 *next_idx)
{
	u8 i;

	for (i = 0; i < ath_rc_priv->max_valid_rate - 1; i++) {
		if (ath_rc_priv->valid_rate_index[i] == cur_valid_txrate) {
			*next_idx = ath_rc_priv->valid_rate_index[i+1];
			return 1;
		}
	}

	
	*next_idx = 0;

	return 0;
}



static int ath_rc_valid_phyrate(u32 phy, u32 capflag, int ignore_cw)
{
	if (WLAN_RC_PHY_HT(phy) && !(capflag & WLAN_RC_HT_FLAG))
		return 0;
	if (WLAN_RC_PHY_DS(phy) && !(capflag & WLAN_RC_DS_FLAG))
		return 0;
	if (WLAN_RC_PHY_SGI(phy) && !(capflag & WLAN_RC_SGI_FLAG))
		return 0;
	if (!ignore_cw && WLAN_RC_PHY_HT(phy))
		if (WLAN_RC_PHY_40(phy) && !(capflag & WLAN_RC_40_FLAG))
			return 0;
	return 1;
}

static inline int
ath_rc_get_lower_rix(const struct ath_rate_table *rate_table,
		     struct ath_rate_priv *ath_rc_priv,
		     u8 cur_valid_txrate, u8 *next_idx)
{
	int8_t i;

	for (i = 1; i < ath_rc_priv->max_valid_rate ; i++) {
		if (ath_rc_priv->valid_rate_index[i] == cur_valid_txrate) {
			*next_idx = ath_rc_priv->valid_rate_index[i-1];
			return 1;
		}
	}

	return 0;
}

static u8 ath_rc_init_validrates(struct ath_rate_priv *ath_rc_priv,
				 const struct ath_rate_table *rate_table,
				 u32 capflag)
{
	u8 i, hi = 0;
	u32 valid;

	for (i = 0; i < rate_table->rate_cnt; i++) {
		valid = (!(ath_rc_priv->ht_cap & WLAN_RC_DS_FLAG) ?
			 rate_table->info[i].valid_single_stream :
			 rate_table->info[i].valid);
		if (valid == 1) {
			u32 phy = rate_table->info[i].phy;
			u8 valid_rate_count = 0;

			if (!ath_rc_valid_phyrate(phy, capflag, 0))
				continue;

			valid_rate_count = ath_rc_priv->valid_phy_ratecnt[phy];

			ath_rc_priv->valid_phy_rateidx[phy][valid_rate_count] = i;
			ath_rc_priv->valid_phy_ratecnt[phy] += 1;
			ath_rc_set_valid_txmask(ath_rc_priv, i, 1);
			hi = A_MAX(hi, i);
		}
	}

	return hi;
}

static u8 ath_rc_setvalid_rates(struct ath_rate_priv *ath_rc_priv,
				const struct ath_rate_table *rate_table,
				struct ath_rateset *rateset,
				u32 capflag)
{
	u8 i, j, hi = 0;

	
	for (i = 0; i < rateset->rs_nrates; i++) {
		for (j = 0; j < rate_table->rate_cnt; j++) {
			u32 phy = rate_table->info[j].phy;
			u32 valid = (!(ath_rc_priv->ht_cap & WLAN_RC_DS_FLAG) ?
				     rate_table->info[j].valid_single_stream :
				     rate_table->info[j].valid);
			u8 rate = rateset->rs_rates[i];
			u8 dot11rate = rate_table->info[j].dot11rate;

			

			if (((rate & 0x7F) == (dot11rate & 0x7F)) &&
			    ((valid & WLAN_RC_CAP_MODE(capflag)) ==
			     WLAN_RC_CAP_MODE(capflag)) &&
			    !WLAN_RC_PHY_HT(phy)) {
				u8 valid_rate_count = 0;

				if (!ath_rc_valid_phyrate(phy, capflag, 0))
					continue;

				valid_rate_count =
					ath_rc_priv->valid_phy_ratecnt[phy];

				ath_rc_priv->valid_phy_rateidx[phy]
					[valid_rate_count] = j;
				ath_rc_priv->valid_phy_ratecnt[phy] += 1;
				ath_rc_set_valid_txmask(ath_rc_priv, j, 1);
				hi = A_MAX(hi, j);
			}
		}
	}

	return hi;
}

static u8 ath_rc_setvalid_htrates(struct ath_rate_priv *ath_rc_priv,
				  const struct ath_rate_table *rate_table,
				  u8 *mcs_set, u32 capflag)
{
	struct ath_rateset *rateset = (struct ath_rateset *)mcs_set;

	u8 i, j, hi = 0;

	
	for (i = 0; i < rateset->rs_nrates; i++) {
		for (j = 0; j < rate_table->rate_cnt; j++) {
			u32 phy = rate_table->info[j].phy;
			u32 valid = (!(ath_rc_priv->ht_cap & WLAN_RC_DS_FLAG) ?
				     rate_table->info[j].valid_single_stream :
				     rate_table->info[j].valid);
			u8 rate = rateset->rs_rates[i];
			u8 dot11rate = rate_table->info[j].dot11rate;

			if (((rate & 0x7F) != (dot11rate & 0x7F)) ||
			    !WLAN_RC_PHY_HT(phy) ||
			    !WLAN_RC_PHY_HT_VALID(valid, capflag))
				continue;

			if (!ath_rc_valid_phyrate(phy, capflag, 0))
				continue;

			ath_rc_priv->valid_phy_rateidx[phy]
				[ath_rc_priv->valid_phy_ratecnt[phy]] = j;
			ath_rc_priv->valid_phy_ratecnt[phy] += 1;
			ath_rc_set_valid_txmask(ath_rc_priv, j, 1);
			hi = A_MAX(hi, j);
		}
	}

	return hi;
}


static u8 ath_rc_get_highest_rix(struct ath_softc *sc,
			         struct ath_rate_priv *ath_rc_priv,
				 const struct ath_rate_table *rate_table,
				 int *is_probing)
{
	u32 best_thruput, this_thruput, now_msec;
	u8 rate, next_rate, best_rate, maxindex, minindex;
	int8_t index = 0;

	now_msec = jiffies_to_msecs(jiffies);
	*is_probing = 0;
	best_thruput = 0;
	maxindex = ath_rc_priv->max_valid_rate-1;
	minindex = 0;
	best_rate = minindex;

	
	for (index = maxindex; index >= minindex ; index--) {
		u8 per_thres;

		rate = ath_rc_priv->valid_rate_index[index];
		if (rate > ath_rc_priv->rate_max_phy)
			continue;

		
		per_thres = ath_rc_priv->per[rate];
		if (per_thres < 12)
			per_thres = 12;

		this_thruput = rate_table->info[rate].user_ratekbps *
			(100 - per_thres);

		if (best_thruput <= this_thruput) {
			best_thruput = this_thruput;
			best_rate    = rate;
		}
	}

	rate = best_rate;

	

	if (rate >= ath_rc_priv->rate_max_phy) {
		rate = ath_rc_priv->rate_max_phy;

		
		if (ath_rc_get_nextvalid_txrate(rate_table,
					ath_rc_priv, rate, &next_rate) &&
		    (now_msec - ath_rc_priv->probe_time >
		     rate_table->probe_interval) &&
		    (ath_rc_priv->hw_maxretry_pktcnt >= 1)) {
			rate = next_rate;
			ath_rc_priv->probe_rate = rate;
			ath_rc_priv->probe_time = now_msec;
			ath_rc_priv->hw_maxretry_pktcnt = 0;
			*is_probing = 1;
		}
	}

	if (rate > (ath_rc_priv->rate_table_size - 1))
		rate = ath_rc_priv->rate_table_size - 1;

	if (rate_table->info[rate].valid &&
	    (ath_rc_priv->ht_cap & WLAN_RC_DS_FLAG))
		return rate;

	if (rate_table->info[rate].valid_single_stream &&
	    !(ath_rc_priv->ht_cap & WLAN_RC_DS_FLAG))
		return rate;

	
	WARN_ON(1);

	rate = ath_rc_priv->valid_rate_index[0];

	return rate;
}

static void ath_rc_rate_set_series(const struct ath_rate_table *rate_table,
				   struct ieee80211_tx_rate *rate,
				   struct ieee80211_tx_rate_control *txrc,
				   u8 tries, u8 rix, int rtsctsenable)
{
	rate->count = tries;
	rate->idx = rix;

	if (txrc->short_preamble)
		rate->flags |= IEEE80211_TX_RC_USE_SHORT_PREAMBLE;
	if (txrc->rts || rtsctsenable)
		rate->flags |= IEEE80211_TX_RC_USE_RTS_CTS;
	if (WLAN_RC_PHY_40(rate_table->info[rix].phy))
		rate->flags |= IEEE80211_TX_RC_40_MHZ_WIDTH;
	if (WLAN_RC_PHY_SGI(rate_table->info[rix].phy))
		rate->flags |= IEEE80211_TX_RC_SHORT_GI;
	if (WLAN_RC_PHY_HT(rate_table->info[rix].phy))
		rate->flags |= IEEE80211_TX_RC_MCS;
}

static void ath_rc_rate_set_rtscts(struct ath_softc *sc,
				   const struct ath_rate_table *rate_table,
				   struct ieee80211_tx_info *tx_info)
{
	struct ieee80211_tx_rate *rates = tx_info->control.rates;
	int i = 0, rix = 0, cix, enable_g_protection = 0;

	
	for (i = 3; i >= 0; i--) {
		if (rates[i].count && (rates[i].idx >= 0)) {
			rix = rates[i].idx;
			break;
		}
	}
	cix = rate_table->info[rix].ctrl_rate;

	
	if (sc->hw->conf.channel->band == IEEE80211_BAND_2GHZ &&
	    !conf_is_ht(&sc->hw->conf))
		enable_g_protection = 1;

	
	if ((sc->sc_flags & SC_OP_PROTECT_ENABLE) &&
	    (rate_table->info[rix].phy == WLAN_RC_PHY_OFDM ||
	     WLAN_RC_PHY_HT(rate_table->info[rix].phy))) {
		rates[0].flags |= IEEE80211_TX_RC_USE_CTS_PROTECT;
		cix = rate_table->info[enable_g_protection].ctrl_rate;
	}

	tx_info->control.rts_cts_rate_idx = cix;
}

static void ath_get_rate(void *priv, struct ieee80211_sta *sta, void *priv_sta,
			 struct ieee80211_tx_rate_control *txrc)
{
	struct ath_softc *sc = priv;
	struct ath_rate_priv *ath_rc_priv = priv_sta;
	const struct ath_rate_table *rate_table;
	struct sk_buff *skb = txrc->skb;
	struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
	struct ieee80211_tx_rate *rates = tx_info->control.rates;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	__le16 fc = hdr->frame_control;
	u8 try_per_rate, i = 0, rix, nrix;
	int is_probe = 0;

	if (rate_control_send_low(sta, priv_sta, txrc))
		return;

	
	try_per_rate = sc->hw->max_rate_tries;

	rate_table = sc->cur_rate_table;
	rix = ath_rc_get_highest_rix(sc, ath_rc_priv, rate_table, &is_probe);
	nrix = rix;

	if (is_probe) {
		
		ath_rc_rate_set_series(rate_table, &rates[i++], txrc,
				       1, nrix, 0);

		
		ath_rc_get_lower_rix(rate_table, ath_rc_priv, rix, &nrix);
		ath_rc_rate_set_series(rate_table, &rates[i++], txrc,
				       try_per_rate, nrix, 0);

		tx_info->flags |= IEEE80211_TX_CTL_RATE_CTRL_PROBE;
	} else {
		
		ath_rc_rate_set_series(rate_table, &rates[i++], txrc,
				       try_per_rate, nrix, 0);
	}

	
	for ( ; i < 4; i++) {
		
		if (i + 1 == 4)
			try_per_rate = 4;

		ath_rc_get_lower_rix(rate_table, ath_rc_priv, rix, &nrix);
		
		ath_rc_rate_set_series(rate_table, &rates[i], txrc,
				       try_per_rate, nrix, 1);
	}

	
	if ((sc->hw->conf.channel->band == IEEE80211_BAND_2GHZ) &&
	    (conf_is_ht(&sc->hw->conf))) {
		u8 dot11rate = rate_table->info[rix].dot11rate;
		u8 phy = rate_table->info[rix].phy;
		if (i == 4 &&
		    ((dot11rate == 2 && phy == WLAN_RC_PHY_HT_40_SS) ||
		     (dot11rate == 3 && phy == WLAN_RC_PHY_HT_20_SS))) {
			rates[3].idx = rates[2].idx;
			rates[3].flags = rates[2].flags;
		}
	}

	
	if (ieee80211_has_morefrags(fc) ||
	    (le16_to_cpu(hdr->seq_ctrl) & IEEE80211_SCTL_FRAG)) {
		rates[1].count = rates[2].count = rates[3].count = 0;
		rates[1].idx = rates[2].idx = rates[3].idx = 0;
		rates[0].count = ATH_TXMAXTRY;
	}

	
	ath_rc_rate_set_rtscts(sc, rate_table, tx_info);
}

static bool ath_rc_update_per(struct ath_softc *sc,
			      const struct ath_rate_table *rate_table,
			      struct ath_rate_priv *ath_rc_priv,
			      struct ath_tx_info_priv *tx_info_priv,
			      int tx_rate, int xretries, int retries,
			      u32 now_msec)
{
	bool state_change = false;
	int count;
	u8 last_per;
	static u32 nretry_to_per_lookup[10] = {
		100 * 0 / 1,
		100 * 1 / 4,
		100 * 1 / 2,
		100 * 3 / 4,
		100 * 4 / 5,
		100 * 5 / 6,
		100 * 6 / 7,
		100 * 7 / 8,
		100 * 8 / 9,
		100 * 9 / 10
	};

	last_per = ath_rc_priv->per[tx_rate];

	if (xretries) {
		if (xretries == 1) {
			ath_rc_priv->per[tx_rate] += 30;
			if (ath_rc_priv->per[tx_rate] > 100)
				ath_rc_priv->per[tx_rate] = 100;
		} else {
			
			count = ARRAY_SIZE(nretry_to_per_lookup);
			if (retries >= count)
				retries = count - 1;

			
			ath_rc_priv->per[tx_rate] =
				(u8)(last_per - (last_per >> 3) + (100 >> 3));
		}

		

		if (ath_rc_priv->probe_rate == tx_rate)
			ath_rc_priv->probe_rate = 0;

	} else { 
		count = ARRAY_SIZE(nretry_to_per_lookup);
		if (retries >= count)
			retries = count - 1;

		if (tx_info_priv->n_bad_frames) {
			
			if (tx_info_priv->n_frames > 0) {
				int n_frames, n_bad_frames;
				u8 cur_per, new_per;

				n_bad_frames = retries * tx_info_priv->n_frames +
					tx_info_priv->n_bad_frames;
				n_frames = tx_info_priv->n_frames * (retries + 1);
				cur_per = (100 * n_bad_frames / n_frames) >> 3;
				new_per = (u8)(last_per - (last_per >> 3) + cur_per);
				ath_rc_priv->per[tx_rate] = new_per;
			}
		} else {
			ath_rc_priv->per[tx_rate] =
				(u8)(last_per - (last_per >> 3) +
				     (nretry_to_per_lookup[retries] >> 3));
		}


		
		if (ath_rc_priv->probe_rate && ath_rc_priv->probe_rate == tx_rate) {
			if (retries > 0 || 2 * tx_info_priv->n_bad_frames >
				tx_info_priv->n_frames) {
				
				ath_rc_priv->probe_rate = 0;
			} else {
				u8 probe_rate = 0;

				ath_rc_priv->rate_max_phy =
					ath_rc_priv->probe_rate;
				probe_rate = ath_rc_priv->probe_rate;

				if (ath_rc_priv->per[probe_rate] > 30)
					ath_rc_priv->per[probe_rate] = 20;

				ath_rc_priv->probe_rate = 0;

				
				ath_rc_priv->probe_time =
					now_msec - rate_table->probe_interval / 2;
			}
		}

		if (retries > 0) {
			
			ath_rc_priv->hw_maxretry_pktcnt = 0;
		} else {
			
			if (tx_rate == ath_rc_priv->rate_max_phy &&
			    ath_rc_priv->hw_maxretry_pktcnt < 255) {
				ath_rc_priv->hw_maxretry_pktcnt++;
			}

		}
	}

	return state_change;
}



static void ath_rc_update_ht(struct ath_softc *sc,
			     struct ath_rate_priv *ath_rc_priv,
			     struct ath_tx_info_priv *tx_info_priv,
			     int tx_rate, int xretries, int retries)
{
	u32 now_msec = jiffies_to_msecs(jiffies);
	int rate;
	u8 last_per;
	bool state_change = false;
	const struct ath_rate_table *rate_table = sc->cur_rate_table;
	int size = ath_rc_priv->rate_table_size;

	if ((tx_rate < 0) || (tx_rate > rate_table->rate_cnt))
		return;

	last_per = ath_rc_priv->per[tx_rate];

	
	state_change = ath_rc_update_per(sc, rate_table, ath_rc_priv,
					 tx_info_priv, tx_rate, xretries,
					 retries, now_msec);

	
	if (ath_rc_priv->per[tx_rate] >= 55 && tx_rate > 0 &&
	    rate_table->info[tx_rate].ratekbps <=
	    rate_table->info[ath_rc_priv->rate_max_phy].ratekbps) {
		ath_rc_get_lower_rix(rate_table, ath_rc_priv,
				     (u8)tx_rate, &ath_rc_priv->rate_max_phy);

		
		ath_rc_priv->probe_time = now_msec;
	}

	
	
	if (ath_rc_priv->per[tx_rate] < last_per) {
		for (rate = tx_rate - 1; rate >= 0; rate--) {

			if (ath_rc_priv->per[rate] >
			    ath_rc_priv->per[rate+1]) {
				ath_rc_priv->per[rate] =
					ath_rc_priv->per[rate+1];
			}
		}
	}

	
	for (rate = tx_rate; rate < size - 1; rate++) {
		if (ath_rc_priv->per[rate+1] <
		    ath_rc_priv->per[rate])
			ath_rc_priv->per[rate+1] =
				ath_rc_priv->per[rate];
	}

	
	if (now_msec - ath_rc_priv->per_down_time >=
	    rate_table->probe_interval) {
		for (rate = 0; rate < size; rate++) {
			ath_rc_priv->per[rate] =
				7 * ath_rc_priv->per[rate] / 8;
		}

		ath_rc_priv->per_down_time = now_msec;
	}

	ath_debug_stat_retries(sc, tx_rate, xretries, retries,
			       ath_rc_priv->per[tx_rate]);

}

static int ath_rc_get_rateindex(const struct ath_rate_table *rate_table,
				struct ieee80211_tx_rate *rate)
{
	int rix;

	if ((rate->flags & IEEE80211_TX_RC_40_MHZ_WIDTH) &&
	    (rate->flags & IEEE80211_TX_RC_SHORT_GI))
		rix = rate_table->info[rate->idx].ht_index;
	else if (rate->flags & IEEE80211_TX_RC_SHORT_GI)
		rix = rate_table->info[rate->idx].sgi_index;
	else if (rate->flags & IEEE80211_TX_RC_40_MHZ_WIDTH)
		rix = rate_table->info[rate->idx].cw40index;
	else
		rix = rate_table->info[rate->idx].base_index;

	return rix;
}

static void ath_rc_tx_status(struct ath_softc *sc,
			     struct ath_rate_priv *ath_rc_priv,
			     struct ieee80211_tx_info *tx_info,
			     int final_ts_idx, int xretries, int long_retry)
{
	struct ath_tx_info_priv *tx_info_priv = ATH_TX_INFO_PRIV(tx_info);
	const struct ath_rate_table *rate_table;
	struct ieee80211_tx_rate *rates = tx_info->status.rates;
	u8 flags;
	u32 i = 0, rix;

	rate_table = sc->cur_rate_table;

	
	if (final_ts_idx != 0) {
		
		for (i = 0; i < final_ts_idx ; i++) {
			if (rates[i].count != 0 && (rates[i].idx >= 0)) {
				flags = rates[i].flags;

				

				if ((flags & IEEE80211_TX_RC_40_MHZ_WIDTH) &&
				    !(ath_rc_priv->ht_cap & WLAN_RC_40_FLAG))
					return;

				rix = ath_rc_get_rateindex(rate_table, &rates[i]);
				ath_rc_update_ht(sc, ath_rc_priv,
						tx_info_priv, rix,
						xretries ? 1 : 2,
						rates[i].count);
			}
		}
	} else {
		
		if (rates[0].count == 1 && xretries == 1)
			xretries = 2;
	}

	flags = rates[i].flags;

	
	if ((flags & IEEE80211_TX_RC_40_MHZ_WIDTH) &&
	    !(ath_rc_priv->ht_cap & WLAN_RC_40_FLAG))
		return;

	rix = ath_rc_get_rateindex(rate_table, &rates[i]);
	ath_rc_update_ht(sc, ath_rc_priv, tx_info_priv, rix,
			 xretries, long_retry);
}

static const
struct ath_rate_table *ath_choose_rate_table(struct ath_softc *sc,
					     enum ieee80211_band band,
					     bool is_ht,
					     bool is_cw_40)
{
	int mode = 0;

	switch(band) {
	case IEEE80211_BAND_2GHZ:
		mode = ATH9K_MODE_11G;
		if (is_ht)
			mode = ATH9K_MODE_11NG_HT20;
		if (is_cw_40)
			mode = ATH9K_MODE_11NG_HT40PLUS;
		break;
	case IEEE80211_BAND_5GHZ:
		mode = ATH9K_MODE_11A;
		if (is_ht)
			mode = ATH9K_MODE_11NA_HT20;
		if (is_cw_40)
			mode = ATH9K_MODE_11NA_HT40PLUS;
		break;
	default:
		DPRINTF(sc, ATH_DBG_CONFIG, "Invalid band\n");
		return NULL;
	}

	BUG_ON(mode >= ATH9K_MODE_MAX);

	DPRINTF(sc, ATH_DBG_CONFIG, "Choosing rate table for mode: %d\n", mode);
	return sc->hw_rate_table[mode];
}

static void ath_rc_init(struct ath_softc *sc,
			struct ath_rate_priv *ath_rc_priv,
			struct ieee80211_supported_band *sband,
			struct ieee80211_sta *sta,
			const struct ath_rate_table *rate_table)
{
	struct ath_rateset *rateset = &ath_rc_priv->neg_rates;
	u8 *ht_mcs = (u8 *)&ath_rc_priv->neg_ht_rates;
	u8 i, j, k, hi = 0, hthi = 0;

	if (!rate_table) {
		DPRINTF(sc, ATH_DBG_FATAL, "Rate table not initialized\n");
		return;
	}

	
	ath_rc_priv->rate_table_size = RATE_TABLE_SIZE;

	
	for (i = 0 ; i < ath_rc_priv->rate_table_size; i++) {
		ath_rc_priv->per[i] = 0;
	}

	
	ath_rc_init_valid_txmask(ath_rc_priv);

	for (i = 0; i < WLAN_RC_PHY_MAX; i++) {
		for (j = 0; j < MAX_TX_RATE_PHY; j++)
			ath_rc_priv->valid_phy_rateidx[i][j] = 0;
		ath_rc_priv->valid_phy_ratecnt[i] = 0;
	}

	if (!rateset->rs_nrates) {
		
		hi = ath_rc_init_validrates(ath_rc_priv, rate_table,
					    ath_rc_priv->ht_cap);
	} else {
		
		hi = ath_rc_setvalid_rates(ath_rc_priv, rate_table,
					   rateset, ath_rc_priv->ht_cap);
		if (ath_rc_priv->ht_cap & WLAN_RC_HT_FLAG) {
			hthi = ath_rc_setvalid_htrates(ath_rc_priv,
						       rate_table,
						       ht_mcs,
						       ath_rc_priv->ht_cap);
		}
		hi = A_MAX(hi, hthi);
	}

	ath_rc_priv->rate_table_size = hi + 1;
	ath_rc_priv->rate_max_phy = 0;
	ASSERT(ath_rc_priv->rate_table_size <= RATE_TABLE_SIZE);

	for (i = 0, k = 0; i < WLAN_RC_PHY_MAX; i++) {
		for (j = 0; j < ath_rc_priv->valid_phy_ratecnt[i]; j++) {
			ath_rc_priv->valid_rate_index[k++] =
				ath_rc_priv->valid_phy_rateidx[i][j];
		}

		if (!ath_rc_valid_phyrate(i, rate_table->initial_ratemax, 1)
		    || !ath_rc_priv->valid_phy_ratecnt[i])
			continue;

		ath_rc_priv->rate_max_phy = ath_rc_priv->valid_phy_rateidx[i][j-1];
	}
	ASSERT(ath_rc_priv->rate_table_size <= RATE_TABLE_SIZE);
	ASSERT(k <= RATE_TABLE_SIZE);

	ath_rc_priv->max_valid_rate = k;
	ath_rc_sort_validrates(rate_table, ath_rc_priv);
	ath_rc_priv->rate_max_phy = ath_rc_priv->valid_rate_index[k-4];
	sc->cur_rate_table = rate_table;

	DPRINTF(sc, ATH_DBG_CONFIG, "RC Initialized with capabilities: 0x%x\n",
		ath_rc_priv->ht_cap);
}

static u8 ath_rc_build_ht_caps(struct ath_softc *sc, struct ieee80211_sta *sta,
			       bool is_cw40, bool is_sgi40)
{
	u8 caps = 0;

	if (sta->ht_cap.ht_supported) {
		caps = WLAN_RC_HT_FLAG;
		if (sc->sc_ah->caps.tx_chainmask != 1 &&
		    ath9k_hw_getcapability(sc->sc_ah, ATH9K_CAP_DS, 0, NULL)) {
			if (sta->ht_cap.mcs.rx_mask[1])
				caps |= WLAN_RC_DS_FLAG;
		}
		if (is_cw40)
			caps |= WLAN_RC_40_FLAG;
		if (is_sgi40)
			caps |= WLAN_RC_SGI_FLAG;
	}

	return caps;
}





static void ath_tx_status(void *priv, struct ieee80211_supported_band *sband,
			  struct ieee80211_sta *sta, void *priv_sta,
			  struct sk_buff *skb)
{
	struct ath_softc *sc = priv;
	struct ath_rate_priv *ath_rc_priv = priv_sta;
	struct ath_tx_info_priv *tx_info_priv = NULL;
	struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
	struct ieee80211_hdr *hdr;
	int final_ts_idx, tx_status = 0, is_underrun = 0;
	__le16 fc;

	hdr = (struct ieee80211_hdr *)skb->data;
	fc = hdr->frame_control;
	tx_info_priv = ATH_TX_INFO_PRIV(tx_info);
	final_ts_idx = tx_info_priv->tx.ts_rateindex;

	if (!priv_sta || !ieee80211_is_data(fc) ||
	    !tx_info_priv->update_rc)
		goto exit;

	if (tx_info_priv->tx.ts_status & ATH9K_TXERR_FILT)
		goto exit;

	
	if (tx_info_priv->tx.ts_flags &
	    (ATH9K_TX_DATA_UNDERRUN | ATH9K_TX_DELIM_UNDERRUN) &&
	    ((sc->sc_ah->tx_trig_level) >= ath_rc_priv->tx_triglevel_max)) {
		tx_status = 1;
		is_underrun = 1;
	}

	if ((tx_info_priv->tx.ts_status & ATH9K_TXERR_XRETRY) ||
	    (tx_info_priv->tx.ts_status & ATH9K_TXERR_FIFO))
		tx_status = 1;

	ath_rc_tx_status(sc, ath_rc_priv, tx_info, final_ts_idx, tx_status,
			 (is_underrun) ? sc->hw->max_rate_tries :
			 tx_info_priv->tx.ts_longretry);

	
	if (conf_is_ht(&sc->hw->conf) &&
	    !(skb->protocol == cpu_to_be16(ETH_P_PAE))) {
		if (ieee80211_is_data_qos(fc)) {
			u8 *qc, tid;
			struct ath_node *an;

			qc = ieee80211_get_qos_ctl(hdr);
			tid = qc[0] & 0xf;
			an = (struct ath_node *)sta->drv_priv;

			if(ath_tx_aggr_check(sc, an, tid))
				ieee80211_start_tx_ba_session(sc->hw, hdr->addr1, tid);
		}
	}

	ath_debug_stat_rc(sc, skb);
exit:
	kfree(tx_info_priv);
}

static void ath_rate_init(void *priv, struct ieee80211_supported_band *sband,
                          struct ieee80211_sta *sta, void *priv_sta)
{
	struct ath_softc *sc = priv;
	struct ath_rate_priv *ath_rc_priv = priv_sta;
	const struct ath_rate_table *rate_table = NULL;
	bool is_cw40, is_sgi40;
	int i, j = 0;

	for (i = 0; i < sband->n_bitrates; i++) {
		if (sta->supp_rates[sband->band] & BIT(i)) {
			ath_rc_priv->neg_rates.rs_rates[j]
				= (sband->bitrates[i].bitrate * 2) / 10;
			j++;
		}
	}
	ath_rc_priv->neg_rates.rs_nrates = j;

	if (sta->ht_cap.ht_supported) {
		for (i = 0, j = 0; i < 77; i++) {
			if (sta->ht_cap.mcs.rx_mask[i/8] & (1<<(i%8)))
				ath_rc_priv->neg_ht_rates.rs_rates[j++] = i;
			if (j == ATH_RATE_MAX)
				break;
		}
		ath_rc_priv->neg_ht_rates.rs_nrates = j;
	}

	is_cw40 = sta->ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40;
	is_sgi40 = sta->ht_cap.cap & IEEE80211_HT_CAP_SGI_40;

	

	if ((sc->sc_ah->opmode == NL80211_IFTYPE_STATION) ||
	    (sc->sc_ah->opmode == NL80211_IFTYPE_MESH_POINT) ||
	    (sc->sc_ah->opmode == NL80211_IFTYPE_ADHOC)) {
		rate_table = ath_choose_rate_table(sc, sband->band,
						   sta->ht_cap.ht_supported,
						   is_cw40);
	} else if (sc->sc_ah->opmode == NL80211_IFTYPE_AP) {
		
		rate_table = sc->cur_rate_table;
	}

	ath_rc_priv->ht_cap = ath_rc_build_ht_caps(sc, sta, is_cw40, is_sgi40);
	ath_rc_init(sc, priv_sta, sband, sta, rate_table);
}

static void ath_rate_update(void *priv, struct ieee80211_supported_band *sband,
			    struct ieee80211_sta *sta, void *priv_sta,
			    u32 changed)
{
	struct ath_softc *sc = priv;
	struct ath_rate_priv *ath_rc_priv = priv_sta;
	const struct ath_rate_table *rate_table = NULL;
	bool oper_cw40 = false, oper_sgi40;
	bool local_cw40 = (ath_rc_priv->ht_cap & WLAN_RC_40_FLAG) ?
		true : false;
	bool local_sgi40 = (ath_rc_priv->ht_cap & WLAN_RC_SGI_FLAG) ?
		true : false;

	

	if (changed & IEEE80211_RC_HT_CHANGED) {
		if (sc->sc_ah->opmode != NL80211_IFTYPE_STATION)
			return;

		if (sc->hw->conf.channel_type == NL80211_CHAN_HT40MINUS ||
		    sc->hw->conf.channel_type == NL80211_CHAN_HT40PLUS)
			oper_cw40 = true;

		oper_sgi40 = (sta->ht_cap.cap & IEEE80211_HT_CAP_SGI_40) ?
			true : false;

		if ((local_cw40 != oper_cw40) || (local_sgi40 != oper_sgi40)) {
			rate_table = ath_choose_rate_table(sc, sband->band,
						   sta->ht_cap.ht_supported,
						   oper_cw40);
			ath_rc_priv->ht_cap = ath_rc_build_ht_caps(sc, sta,
						   oper_cw40, oper_sgi40);
			ath_rc_init(sc, priv_sta, sband, sta, rate_table);

			DPRINTF(sc, ATH_DBG_CONFIG,
				"Operating HT Bandwidth changed to: %d\n",
				sc->hw->conf.channel_type);
		}
	}
}

static void *ath_rate_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
{
	struct ath_wiphy *aphy = hw->priv;
	return aphy->sc;
}

static void ath_rate_free(void *priv)
{
	return;
}

static void *ath_rate_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
{
	struct ath_softc *sc = priv;
	struct ath_rate_priv *rate_priv;

	rate_priv = kzalloc(sizeof(struct ath_rate_priv), gfp);
	if (!rate_priv) {
		DPRINTF(sc, ATH_DBG_FATAL,
			"Unable to allocate private rc structure\n");
		return NULL;
	}

	rate_priv->tx_triglevel_max = sc->sc_ah->caps.tx_triglevel_max;

	return rate_priv;
}

static void ath_rate_free_sta(void *priv, struct ieee80211_sta *sta,
			      void *priv_sta)
{
	struct ath_rate_priv *rate_priv = priv_sta;
	kfree(rate_priv);
}

static struct rate_control_ops ath_rate_ops = {
	.module = NULL,
	.name = "ath9k_rate_control",
	.tx_status = ath_tx_status,
	.get_rate = ath_get_rate,
	.rate_init = ath_rate_init,
	.rate_update = ath_rate_update,
	.alloc = ath_rate_alloc,
	.free = ath_rate_free,
	.alloc_sta = ath_rate_alloc_sta,
	.free_sta = ath_rate_free_sta,
};

void ath_rate_attach(struct ath_softc *sc)
{
	sc->hw_rate_table[ATH9K_MODE_11A] =
		&ar5416_11a_ratetable;
	sc->hw_rate_table[ATH9K_MODE_11G] =
		&ar5416_11g_ratetable;
	sc->hw_rate_table[ATH9K_MODE_11NA_HT20] =
		&ar5416_11na_ratetable;
	sc->hw_rate_table[ATH9K_MODE_11NG_HT20] =
		&ar5416_11ng_ratetable;
	sc->hw_rate_table[ATH9K_MODE_11NA_HT40PLUS] =
		&ar5416_11na_ratetable;
	sc->hw_rate_table[ATH9K_MODE_11NA_HT40MINUS] =
		&ar5416_11na_ratetable;
	sc->hw_rate_table[ATH9K_MODE_11NG_HT40PLUS] =
		&ar5416_11ng_ratetable;
	sc->hw_rate_table[ATH9K_MODE_11NG_HT40MINUS] =
		&ar5416_11ng_ratetable;
}

int ath_rate_control_register(void)
{
	return ieee80211_rate_control_register(&ath_rate_ops);
}

void ath_rate_control_unregister(void)
{
	ieee80211_rate_control_unregister(&ath_rate_ops);
}
