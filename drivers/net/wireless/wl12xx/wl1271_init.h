

#ifndef __WL1271_INIT_H__
#define __WL1271_INIT_H__

#include "wl1271.h"

int wl1271_hw_init_power_auth(struct wl1271 *wl);
int wl1271_hw_init(struct wl1271 *wl);


#define TEST_CMD_INI_FILE_RADIO_PARAM   0x19
#define TEST_CMD_INI_FILE_GENERAL_PARAM 0x1E

struct wl1271_general_parms {
	u8 id;
	u8 padding[3];

	u8 ref_clk;
	u8 settling_time;
	u8 clk_valid_on_wakeup;
	u8 dc2dcmode;
	u8 single_dual_band;

	u8 tx_bip_fem_autodetect;
	u8 tx_bip_fem_manufacturer;
	u8 settings;
} __attribute__ ((packed));

enum ref_clk_enum {
	REF_CLK_19_2_E,
	REF_CLK_26_E,
	REF_CLK_38_4_E,
	REF_CLK_52_E
};

#define RSSI_AND_PROCESS_COMPENSATION_SIZE 15
#define NUMBER_OF_SUB_BANDS_5  7
#define NUMBER_OF_RATE_GROUPS  6
#define NUMBER_OF_CHANNELS_2_4 14
#define NUMBER_OF_CHANNELS_5   35

struct wl1271_radio_parms {
	u8 id;
	u8 padding[3];

	
	
	u8 rx_trace_loss;
	u8 tx_trace_loss;
	s8 rx_rssi_and_proc_compens[RSSI_AND_PROCESS_COMPENSATION_SIZE];

	
	u8 rx_trace_loss_5[NUMBER_OF_SUB_BANDS_5];
	u8 tx_trace_loss_5[NUMBER_OF_SUB_BANDS_5];
	s8 rx_rssi_and_proc_compens_5[RSSI_AND_PROCESS_COMPENSATION_SIZE];

	
	
	s16 tx_ref_pd_voltage;
	s8  tx_ref_power;
	s8  tx_offset_db;

	s8  tx_rate_limits_normal[NUMBER_OF_RATE_GROUPS];
	s8  tx_rate_limits_degraded[NUMBER_OF_RATE_GROUPS];

	s8  tx_channel_limits_11b[NUMBER_OF_CHANNELS_2_4];
	s8  tx_channel_limits_ofdm[NUMBER_OF_CHANNELS_2_4];
	s8  tx_pdv_rate_offsets[NUMBER_OF_RATE_GROUPS];

	u8  tx_ibias[NUMBER_OF_RATE_GROUPS];
	u8  rx_fem_insertion_loss;

	u8 padding2;

	
	s16 tx_ref_pd_voltage_5[NUMBER_OF_SUB_BANDS_5];
	s8  tx_ref_power_5[NUMBER_OF_SUB_BANDS_5];
	s8  tx_offset_db_5[NUMBER_OF_SUB_BANDS_5];

	s8  tx_rate_limits_normal_5[NUMBER_OF_RATE_GROUPS];
	s8  tx_rate_limits_degraded_5[NUMBER_OF_RATE_GROUPS];

	s8  tx_channel_limits_ofdm_5[NUMBER_OF_CHANNELS_5];
	s8  tx_pdv_rate_offsets_5[NUMBER_OF_RATE_GROUPS];

	
	s8  tx_ibias_5[NUMBER_OF_RATE_GROUPS];
	s8  rx_fem_insertion_loss_5[NUMBER_OF_SUB_BANDS_5];

	u8 padding3[2];
} __attribute__ ((packed));

#endif
