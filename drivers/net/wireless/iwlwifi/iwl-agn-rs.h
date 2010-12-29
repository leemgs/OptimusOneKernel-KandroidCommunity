

#ifndef __iwl_agn_rs_h__
#define __iwl_agn_rs_h__

struct iwl_rate_info {
	u8 plcp;	
	u8 plcp_siso;	
	u8 plcp_mimo2;	
	u8 plcp_mimo3;  
	u8 ieee;	
	u8 prev_ieee;    
	u8 next_ieee;    
	u8 prev_rs;      
	u8 next_rs;      
	u8 prev_rs_tgg;  
	u8 next_rs_tgg;  
};

struct iwl3945_rate_info {
	u8 plcp;		
	u8 ieee;		
	u8 prev_ieee;		
	u8 next_ieee;		
	u8 prev_rs;		
	u8 next_rs;		
	u8 prev_rs_tgg;		
	u8 next_rs_tgg;		
	u8 table_rs_index;	
	u8 prev_table_rs;	
};


enum {
	IWL_RATE_1M_INDEX = 0,
	IWL_RATE_2M_INDEX,
	IWL_RATE_5M_INDEX,
	IWL_RATE_11M_INDEX,
	IWL_RATE_6M_INDEX,
	IWL_RATE_9M_INDEX,
	IWL_RATE_12M_INDEX,
	IWL_RATE_18M_INDEX,
	IWL_RATE_24M_INDEX,
	IWL_RATE_36M_INDEX,
	IWL_RATE_48M_INDEX,
	IWL_RATE_54M_INDEX,
	IWL_RATE_60M_INDEX,
	IWL_RATE_COUNT, 
	IWL_RATE_COUNT_LEGACY = IWL_RATE_COUNT - 1,	
	IWL_RATE_COUNT_3945 = IWL_RATE_COUNT - 1,
	IWL_RATE_INVM_INDEX = IWL_RATE_COUNT,
	IWL_RATE_INVALID = IWL_RATE_COUNT,
};

enum {
	IWL_RATE_6M_INDEX_TABLE = 0,
	IWL_RATE_9M_INDEX_TABLE,
	IWL_RATE_12M_INDEX_TABLE,
	IWL_RATE_18M_INDEX_TABLE,
	IWL_RATE_24M_INDEX_TABLE,
	IWL_RATE_36M_INDEX_TABLE,
	IWL_RATE_48M_INDEX_TABLE,
	IWL_RATE_54M_INDEX_TABLE,
	IWL_RATE_1M_INDEX_TABLE,
	IWL_RATE_2M_INDEX_TABLE,
	IWL_RATE_5M_INDEX_TABLE,
	IWL_RATE_11M_INDEX_TABLE,
	IWL_RATE_INVM_INDEX_TABLE = IWL_RATE_INVM_INDEX - 1,
};

enum {
	IWL_FIRST_OFDM_RATE = IWL_RATE_6M_INDEX,
	IWL39_LAST_OFDM_RATE = IWL_RATE_54M_INDEX,
	IWL_LAST_OFDM_RATE = IWL_RATE_60M_INDEX,
	IWL_FIRST_CCK_RATE = IWL_RATE_1M_INDEX,
	IWL_LAST_CCK_RATE = IWL_RATE_11M_INDEX,
};


#define	IWL_RATE_6M_MASK   (1 << IWL_RATE_6M_INDEX)
#define	IWL_RATE_9M_MASK   (1 << IWL_RATE_9M_INDEX)
#define	IWL_RATE_12M_MASK  (1 << IWL_RATE_12M_INDEX)
#define	IWL_RATE_18M_MASK  (1 << IWL_RATE_18M_INDEX)
#define	IWL_RATE_24M_MASK  (1 << IWL_RATE_24M_INDEX)
#define	IWL_RATE_36M_MASK  (1 << IWL_RATE_36M_INDEX)
#define	IWL_RATE_48M_MASK  (1 << IWL_RATE_48M_INDEX)
#define	IWL_RATE_54M_MASK  (1 << IWL_RATE_54M_INDEX)
#define IWL_RATE_60M_MASK  (1 << IWL_RATE_60M_INDEX)
#define	IWL_RATE_1M_MASK   (1 << IWL_RATE_1M_INDEX)
#define	IWL_RATE_2M_MASK   (1 << IWL_RATE_2M_INDEX)
#define	IWL_RATE_5M_MASK   (1 << IWL_RATE_5M_INDEX)
#define	IWL_RATE_11M_MASK  (1 << IWL_RATE_11M_INDEX)


enum {
	IWL_RATE_6M_PLCP  = 13,
	IWL_RATE_9M_PLCP  = 15,
	IWL_RATE_12M_PLCP = 5,
	IWL_RATE_18M_PLCP = 7,
	IWL_RATE_24M_PLCP = 9,
	IWL_RATE_36M_PLCP = 11,
	IWL_RATE_48M_PLCP = 1,
	IWL_RATE_54M_PLCP = 3,
	IWL_RATE_60M_PLCP = 3,
	IWL_RATE_1M_PLCP  = 10,
	IWL_RATE_2M_PLCP  = 20,
	IWL_RATE_5M_PLCP  = 55,
	IWL_RATE_11M_PLCP = 110,
	
	
};


enum {
	IWL_RATE_SISO_6M_PLCP = 0,
	IWL_RATE_SISO_12M_PLCP = 1,
	IWL_RATE_SISO_18M_PLCP = 2,
	IWL_RATE_SISO_24M_PLCP = 3,
	IWL_RATE_SISO_36M_PLCP = 4,
	IWL_RATE_SISO_48M_PLCP = 5,
	IWL_RATE_SISO_54M_PLCP = 6,
	IWL_RATE_SISO_60M_PLCP = 7,
	IWL_RATE_MIMO2_6M_PLCP  = 0x8,
	IWL_RATE_MIMO2_12M_PLCP = 0x9,
	IWL_RATE_MIMO2_18M_PLCP = 0xa,
	IWL_RATE_MIMO2_24M_PLCP = 0xb,
	IWL_RATE_MIMO2_36M_PLCP = 0xc,
	IWL_RATE_MIMO2_48M_PLCP = 0xd,
	IWL_RATE_MIMO2_54M_PLCP = 0xe,
	IWL_RATE_MIMO2_60M_PLCP = 0xf,
	IWL_RATE_MIMO3_6M_PLCP  = 0x10,
	IWL_RATE_MIMO3_12M_PLCP = 0x11,
	IWL_RATE_MIMO3_18M_PLCP = 0x12,
	IWL_RATE_MIMO3_24M_PLCP = 0x13,
	IWL_RATE_MIMO3_36M_PLCP = 0x14,
	IWL_RATE_MIMO3_48M_PLCP = 0x15,
	IWL_RATE_MIMO3_54M_PLCP = 0x16,
	IWL_RATE_MIMO3_60M_PLCP = 0x17,
	IWL_RATE_SISO_INVM_PLCP,
	IWL_RATE_MIMO2_INVM_PLCP = IWL_RATE_SISO_INVM_PLCP,
	IWL_RATE_MIMO3_INVM_PLCP = IWL_RATE_SISO_INVM_PLCP,
};


enum {
	IWL_RATE_6M_IEEE  = 12,
	IWL_RATE_9M_IEEE  = 18,
	IWL_RATE_12M_IEEE = 24,
	IWL_RATE_18M_IEEE = 36,
	IWL_RATE_24M_IEEE = 48,
	IWL_RATE_36M_IEEE = 72,
	IWL_RATE_48M_IEEE = 96,
	IWL_RATE_54M_IEEE = 108,
	IWL_RATE_60M_IEEE = 120,
	IWL_RATE_1M_IEEE  = 2,
	IWL_RATE_2M_IEEE  = 4,
	IWL_RATE_5M_IEEE  = 11,
	IWL_RATE_11M_IEEE = 22,
};

#define IWL_CCK_BASIC_RATES_MASK    \
       (IWL_RATE_1M_MASK          | \
	IWL_RATE_2M_MASK)

#define IWL_CCK_RATES_MASK          \
       (IWL_BASIC_RATES_MASK      | \
	IWL_RATE_5M_MASK          | \
	IWL_RATE_11M_MASK)

#define IWL_OFDM_BASIC_RATES_MASK   \
	(IWL_RATE_6M_MASK         | \
	IWL_RATE_12M_MASK         | \
	IWL_RATE_24M_MASK)

#define IWL_OFDM_RATES_MASK         \
       (IWL_OFDM_BASIC_RATES_MASK | \
	IWL_RATE_9M_MASK          | \
	IWL_RATE_18M_MASK         | \
	IWL_RATE_36M_MASK         | \
	IWL_RATE_48M_MASK         | \
	IWL_RATE_54M_MASK)

#define IWL_BASIC_RATES_MASK         \
	(IWL_OFDM_BASIC_RATES_MASK | \
	 IWL_CCK_BASIC_RATES_MASK)

#define IWL_RATES_MASK ((1 << IWL_RATE_COUNT) - 1)

#define IWL_INVALID_VALUE    -1

#define IWL_MIN_RSSI_VAL                 -100
#define IWL_MAX_RSSI_VAL                    0


#define IWL_LEGACY_FAILURE_LIMIT	160
#define IWL_LEGACY_SUCCESS_LIMIT	480
#define IWL_LEGACY_TABLE_COUNT		160

#define IWL_NONE_LEGACY_FAILURE_LIMIT	400
#define IWL_NONE_LEGACY_SUCCESS_LIMIT	4500
#define IWL_NONE_LEGACY_TABLE_COUNT	1500


#define IWL_RS_GOOD_RATIO		12800	
#define IWL_RATE_SCALE_SWITCH		10880	
#define IWL_RATE_HIGH_TH		10880	
#define IWL_RATE_INCREASE_TH		6400	
#define IWL_RATE_DECREASE_TH		1920	


#define IWL_LEGACY_SWITCH_ANTENNA1      0
#define IWL_LEGACY_SWITCH_ANTENNA2      1
#define IWL_LEGACY_SWITCH_SISO          2
#define IWL_LEGACY_SWITCH_MIMO2_AB      3
#define IWL_LEGACY_SWITCH_MIMO2_AC      4
#define IWL_LEGACY_SWITCH_MIMO2_BC      5
#define IWL_LEGACY_SWITCH_MIMO3_ABC     6


#define IWL_SISO_SWITCH_ANTENNA1        0
#define IWL_SISO_SWITCH_ANTENNA2        1
#define IWL_SISO_SWITCH_MIMO2_AB        2
#define IWL_SISO_SWITCH_MIMO2_AC        3
#define IWL_SISO_SWITCH_MIMO2_BC        4
#define IWL_SISO_SWITCH_GI              5
#define IWL_SISO_SWITCH_MIMO3_ABC       6



#define IWL_MIMO2_SWITCH_ANTENNA1       0
#define IWL_MIMO2_SWITCH_ANTENNA2       1
#define IWL_MIMO2_SWITCH_SISO_A         2
#define IWL_MIMO2_SWITCH_SISO_B         3
#define IWL_MIMO2_SWITCH_SISO_C         4
#define IWL_MIMO2_SWITCH_GI             5
#define IWL_MIMO2_SWITCH_MIMO3_ABC      6



#define IWL_MIMO3_SWITCH_ANTENNA1       0
#define IWL_MIMO3_SWITCH_ANTENNA2       1
#define IWL_MIMO3_SWITCH_SISO_A         2
#define IWL_MIMO3_SWITCH_SISO_B         3
#define IWL_MIMO3_SWITCH_SISO_C         4
#define IWL_MIMO3_SWITCH_MIMO2_AB       5
#define IWL_MIMO3_SWITCH_MIMO2_AC       6
#define IWL_MIMO3_SWITCH_MIMO2_BC       7
#define IWL_MIMO3_SWITCH_GI             8


#define IWL_MAX_11N_MIMO3_SEARCH IWL_MIMO3_SWITCH_GI
#define IWL_MAX_SEARCH IWL_MIMO2_SWITCH_MIMO3_ABC



#define IWL_ACTION_LIMIT		3	

#define LQ_SIZE		2	


#define IWL_AGG_TPT_THREHOLD	0
#define IWL_AGG_LOAD_THRESHOLD	10
#define IWL_AGG_ALL_TID		0xff
#define TID_QUEUE_CELL_SPACING	50	
#define TID_QUEUE_MAX_SIZE	20
#define TID_ROUND_VALUE		5	
#define TID_MAX_LOAD_COUNT	8

#define TID_MAX_TIME_DIFF ((TID_QUEUE_MAX_SIZE - 1) * TID_QUEUE_CELL_SPACING)
#define TIME_WRAP_AROUND(x, y) (((y) > (x)) ? (y) - (x) : (0-(x)) + (y))

extern const struct iwl_rate_info iwl_rates[IWL_RATE_COUNT];
extern const struct iwl3945_rate_info iwl3945_rates[IWL_RATE_COUNT_3945];

enum iwl_table_type {
	LQ_NONE,
	LQ_G,		
	LQ_A,
	LQ_SISO,	
	LQ_MIMO2,
	LQ_MIMO3,
	LQ_MAX,
};

#define is_legacy(tbl) (((tbl) == LQ_G) || ((tbl) == LQ_A))
#define is_siso(tbl) ((tbl) == LQ_SISO)
#define is_mimo2(tbl) ((tbl) == LQ_MIMO2)
#define is_mimo3(tbl) ((tbl) == LQ_MIMO3)
#define is_mimo(tbl) (is_mimo2(tbl) || is_mimo3(tbl))
#define is_Ht(tbl) (is_siso(tbl) || is_mimo(tbl))
#define is_a_band(tbl) ((tbl) == LQ_A)
#define is_g_and(tbl) ((tbl) == LQ_G)

#define	ANT_NONE	0x0
#define	ANT_A		BIT(0)
#define	ANT_B		BIT(1)
#define	ANT_AB		(ANT_A | ANT_B)
#define ANT_C		BIT(2)
#define	ANT_AC		(ANT_A | ANT_C)
#define ANT_BC		(ANT_B | ANT_C)
#define ANT_ABC		(ANT_AB | ANT_C)

#define IWL_MAX_MCS_DISPLAY_SIZE	12

struct iwl_rate_mcs_info {
	char	mbps[IWL_MAX_MCS_DISPLAY_SIZE];
	char	mcs[IWL_MAX_MCS_DISPLAY_SIZE];
};

static inline u8 num_of_ant(u8 mask)
{
	return  !!((mask) & ANT_A) +
		!!((mask) & ANT_B) +
		!!((mask) & ANT_C);
}

static inline u8 first_antenna(u8 mask)
{
	if (mask & ANT_A)
		return ANT_A;
	if (mask & ANT_B)
		return ANT_B;
	return ANT_C;
}


static inline u8 iwl_get_prev_ieee_rate(u8 rate_index)
{
	u8 rate = iwl_rates[rate_index].prev_ieee;

	if (rate == IWL_RATE_INVALID)
		rate = rate_index;
	return rate;
}

static inline u8 iwl3945_get_prev_ieee_rate(u8 rate_index)
{
	u8 rate = iwl3945_rates[rate_index].prev_ieee;

	if (rate == IWL_RATE_INVALID)
		rate = rate_index;
	return rate;
}


extern void iwl3945_rate_scale_init(struct ieee80211_hw *hw, s32 sta_id);


extern int iwlagn_rate_control_register(void);
extern int iwl3945_rate_control_register(void);


extern void iwlagn_rate_control_unregister(void);
extern void iwl3945_rate_control_unregister(void);

#endif 
