


#ifndef __iwl_3945_hw__
#define __iwl_3945_hw__

#include "iwl-eeprom.h"


#define IWL_CMD_QUEUE_NUM	4


#define SHORT_SLOT_TIME 9
#define LONG_SLOT_TIME 20


#define IWL39_RSSI_OFFSET	95


#define EEPROM_SKU_CAP_OP_MODE_MRC                      (1 << 7)


struct iwl3945_eeprom_txpower_sample {
	u8 gain_index;		
	s8 power;		
	u16 v_det;		
} __attribute__ ((packed));


struct iwl3945_eeprom_txpower_group {
	struct iwl3945_eeprom_txpower_sample samples[5];  
	s32 a, b, c, d, e;	
	s32 Fa, Fb, Fc, Fd, Fe;	
	s8 saturation_power;	
	u8 group_channel;	
	s16 temperature;	
} __attribute__ ((packed));


struct iwl3945_eeprom_temperature_corr {
	u32 Ta;
	u32 Tb;
	u32 Tc;
	u32 Td;
	u32 Te;
} __attribute__ ((packed));


struct iwl3945_eeprom {
	u8 reserved0[16];
	u16 device_id;	
	u8 reserved1[2];
	u16 pmc;		
	u8 reserved2[20];
	u8 mac_address[6];	
	u8 reserved3[58];
	u16 board_revision;	
	u8 reserved4[11];
	u8 board_pba_number[9];	
	u8 reserved5[8];
	u16 version;		
	u8 sku_cap;		
	u8 leds_mode;		
	u16 oem_mode;
	u16 wowlan_mode;	
	u16 leds_time_interval;	
	u8 leds_off_time;	
	u8 leds_on_time;	
	u8 almgor_m_version;	
	u8 antenna_switch_type;	
	u8 reserved6[42];
	u8 sku_id[4];		


	u16 band_1_count;	
	struct iwl_eeprom_channel band_1_channels[14];  


	u16 band_2_count;	
	struct iwl_eeprom_channel band_2_channels[13];  


	u16 band_3_count;	
	struct iwl_eeprom_channel band_3_channels[12];  


	u16 band_4_count;	
	struct iwl_eeprom_channel band_4_channels[11];  


	u16 band_5_count;	
	struct iwl_eeprom_channel band_5_channels[6];  

	u8 reserved9[194];


#define IWL_NUM_TX_CALIB_GROUPS 5
	struct iwl3945_eeprom_txpower_group groups[IWL_NUM_TX_CALIB_GROUPS];

	struct iwl3945_eeprom_temperature_corr corrections;  
	u8 reserved16[172];	
} __attribute__ ((packed));

#define IWL3945_EEPROM_IMG_SIZE 1024



#define PCI_CFG_REV_ID_BIT_BASIC_SKU                (0x40)	
#define PCI_CFG_REV_ID_BIT_RTP                      (0x80)	


#define IWL39_NUM_QUEUES        5
#define IWL_NUM_SCAN_RATES         (2)

#define IWL_DEFAULT_TX_RETRY  15



#define RFD_SIZE                              4
#define NUM_TFD_CHUNKS                        4

#define RX_QUEUE_SIZE                         256
#define RX_QUEUE_MASK                         255
#define RX_QUEUE_SIZE_LOG                     8

#define U32_PAD(n)		((4-(n))&0x3)

#define TFD_CTL_COUNT_SET(n)       (n << 24)
#define TFD_CTL_COUNT_GET(ctl)     ((ctl >> 24) & 7)
#define TFD_CTL_PAD_SET(n)         (n << 28)
#define TFD_CTL_PAD_GET(ctl)       (ctl >> 28)


#define RX_FREE_BUFFERS 64
#define RX_LOW_WATERMARK 8


#define IWL39_RTC_INST_LOWER_BOUND		(0x000000)
#define IWL39_RTC_INST_UPPER_BOUND		(0x014000)

#define IWL39_RTC_DATA_LOWER_BOUND		(0x800000)
#define IWL39_RTC_DATA_UPPER_BOUND		(0x808000)

#define IWL39_RTC_INST_SIZE (IWL39_RTC_INST_UPPER_BOUND - \
				IWL39_RTC_INST_LOWER_BOUND)
#define IWL39_RTC_DATA_SIZE (IWL39_RTC_DATA_UPPER_BOUND - \
				IWL39_RTC_DATA_LOWER_BOUND)

#define IWL39_MAX_INST_SIZE IWL39_RTC_INST_SIZE
#define IWL39_MAX_DATA_SIZE IWL39_RTC_DATA_SIZE


#define IWL39_MAX_BSM_SIZE IWL39_RTC_INST_SIZE

static inline int iwl3945_hw_valid_rtc_data_addr(u32 addr)
{
	return (addr >= IWL39_RTC_DATA_LOWER_BOUND) &&
	       (addr < IWL39_RTC_DATA_UPPER_BOUND);
}


struct iwl3945_shared {
	__le32 tx_base_ptr[8];
} __attribute__ ((packed));

static inline u8 iwl3945_hw_get_rate(__le16 rate_n_flags)
{
	return le16_to_cpu(rate_n_flags) & 0xFF;
}

static inline u16 iwl3945_hw_get_rate_n_flags(__le16 rate_n_flags)
{
	return le16_to_cpu(rate_n_flags);
}

static inline __le16 iwl3945_hw_set_rate_n_flags(u8 rate, u16 flags)
{
	return cpu_to_le16((u16)rate|flags);
}
#endif
