

#ifndef __REG_H__
#define __REG_H__

#include <linux/bitops.h>

#define REGISTERS_BASE 0x00300000
#define DRPW_BASE      0x00310000

#define REGISTERS_DOWN_SIZE 0x00008800
#define REGISTERS_WORK_SIZE 0x0000b000

#define HW_ACCESS_ELP_CTRL_REG_ADDR         0x1FFFC


#define ELPCTRL_WAKE_UP             0x1
#define ELPCTRL_WAKE_UP_WLAN_READY  0x5
#define ELPCTRL_SLEEP               0x0

#define ELPCTRL_WLAN_READY          0x2


#define SOR_CFG                        (REGISTERS_BASE + 0x0800)
#define ECPU_CTRL                      (REGISTERS_BASE + 0x0804)
#define HI_CFG                         (REGISTERS_BASE + 0x0808)
#define EE_START                       (REGISTERS_BASE + 0x080C)

#define CHIP_ID_B                      (REGISTERS_BASE + 0x5674)

#define CHIP_ID_1251_PG10	           (0x7010101)
#define CHIP_ID_1251_PG11	           (0x7020101)
#define CHIP_ID_1251_PG12	           (0x7030101)

#define ENABLE                         (REGISTERS_BASE + 0x5450)


#define ELP_CFG_MODE                   (REGISTERS_BASE + 0x5804)
#define ELP_CMD                        (REGISTERS_BASE + 0x5808)
#define PLL_CAL_TIME                   (REGISTERS_BASE + 0x5810)
#define CLK_REQ_TIME                   (REGISTERS_BASE + 0x5814)
#define CLK_BUF_TIME                   (REGISTERS_BASE + 0x5818)

#define CFG_PLL_SYNC_CNT               (REGISTERS_BASE + 0x5820)


#define SCR_PAD0                       (REGISTERS_BASE + 0x5608)
#define SCR_PAD1                       (REGISTERS_BASE + 0x560C)
#define SCR_PAD2                       (REGISTERS_BASE + 0x5610)
#define SCR_PAD3                       (REGISTERS_BASE + 0x5614)
#define SCR_PAD4                       (REGISTERS_BASE + 0x5618)
#define SCR_PAD4_SET                   (REGISTERS_BASE + 0x561C)
#define SCR_PAD4_CLR                   (REGISTERS_BASE + 0x5620)
#define SCR_PAD5                       (REGISTERS_BASE + 0x5624)
#define SCR_PAD5_SET                   (REGISTERS_BASE + 0x5628)
#define SCR_PAD5_CLR                   (REGISTERS_BASE + 0x562C)
#define SCR_PAD6                       (REGISTERS_BASE + 0x5630)
#define SCR_PAD7                       (REGISTERS_BASE + 0x5634)
#define SCR_PAD8                       (REGISTERS_BASE + 0x5638)
#define SCR_PAD9                       (REGISTERS_BASE + 0x563C)


#define SPARE_A1                       (REGISTERS_BASE + 0x0994)
#define SPARE_A2                       (REGISTERS_BASE + 0x0998)
#define SPARE_A3                       (REGISTERS_BASE + 0x099C)
#define SPARE_A4                       (REGISTERS_BASE + 0x09A0)
#define SPARE_A5                       (REGISTERS_BASE + 0x09A4)
#define SPARE_A6                       (REGISTERS_BASE + 0x09A8)
#define SPARE_A7                       (REGISTERS_BASE + 0x09AC)
#define SPARE_A8                       (REGISTERS_BASE + 0x09B0)
#define SPARE_B1                       (REGISTERS_BASE + 0x5420)
#define SPARE_B2                       (REGISTERS_BASE + 0x5424)
#define SPARE_B3                       (REGISTERS_BASE + 0x5428)
#define SPARE_B4                       (REGISTERS_BASE + 0x542C)
#define SPARE_B5                       (REGISTERS_BASE + 0x5430)
#define SPARE_B6                       (REGISTERS_BASE + 0x5434)
#define SPARE_B7                       (REGISTERS_BASE + 0x5438)
#define SPARE_B8                       (REGISTERS_BASE + 0x543C)

enum wl12xx_acx_int_reg {
	ACX_REG_INTERRUPT_TRIG,
	ACX_REG_INTERRUPT_TRIG_H,


	ACX_REG_INTERRUPT_MASK,


	ACX_REG_HINT_MASK_SET,


	ACX_REG_HINT_MASK_CLR,


	ACX_REG_INTERRUPT_NO_CLEAR,


	ACX_REG_INTERRUPT_CLEAR,


	ACX_REG_INTERRUPT_ACK,


	ACX_REG_SLV_SOFT_RESET,


	ACX_REG_EE_START,




	ACX_REG_ECPU_CONTROL,

	ACX_REG_TABLE_LEN
};

#define ACX_SLV_SOFT_RESET_BIT   BIT(0)
#define ACX_REG_EEPROM_START_BIT BIT(0)




#define REG_COMMAND_MAILBOX_PTR				(SCR_PAD0)


#define REG_EVENT_MAILBOX_PTR				(SCR_PAD1)




#define REG_ENABLE_TX_RX				(ENABLE)

#define REG_RX_CONFIG				(RX_CFG)
#define REG_RX_FILTER				(RX_FILTER_CFG)


#define RX_CFG_ENABLE_PHY_HEADER_PLCP	 0x0002


#define RX_CFG_PROMISCUOUS		 0x0008


#define RX_CFG_BSSID			 0x0020


#define RX_CFG_MAC			 0x0010

#define RX_CFG_ENABLE_ONLY_MY_DEST_MAC	 0x0010
#define RX_CFG_ENABLE_ANY_DEST_MAC	 0x0000
#define RX_CFG_ENABLE_ONLY_MY_BSSID	 0x0020
#define RX_CFG_ENABLE_ANY_BSSID		 0x0000


#define RX_CFG_DISABLE_BCAST		 0x0200

#define RX_CFG_ENABLE_ONLY_MY_SSID	 0x0400
#define RX_CFG_ENABLE_RX_CMPLT_FCS_ERROR 0x0800
#define RX_CFG_COPY_RX_STATUS		 0x2000
#define RX_CFG_TSF			 0x10000

#define RX_CONFIG_OPTION_ANY_DST_MY_BSS	 (RX_CFG_ENABLE_ANY_DEST_MAC | \
					  RX_CFG_ENABLE_ONLY_MY_BSSID)

#define RX_CONFIG_OPTION_MY_DST_ANY_BSS	 (RX_CFG_ENABLE_ONLY_MY_DEST_MAC\
					  | RX_CFG_ENABLE_ANY_BSSID)

#define RX_CONFIG_OPTION_ANY_DST_ANY_BSS (RX_CFG_ENABLE_ANY_DEST_MAC | \
					  RX_CFG_ENABLE_ANY_BSSID)

#define RX_CONFIG_OPTION_MY_DST_MY_BSS	 (RX_CFG_ENABLE_ONLY_MY_DEST_MAC\
					  | RX_CFG_ENABLE_ONLY_MY_BSSID)

#define RX_CONFIG_OPTION_FOR_SCAN  (RX_CFG_ENABLE_PHY_HEADER_PLCP \
				    | RX_CFG_ENABLE_RX_CMPLT_FCS_ERROR \
				    | RX_CFG_COPY_RX_STATUS | RX_CFG_TSF)

#define RX_CONFIG_OPTION_FOR_MEASUREMENT (RX_CFG_ENABLE_ANY_DEST_MAC)

#define RX_CONFIG_OPTION_FOR_JOIN	 (RX_CFG_ENABLE_ONLY_MY_BSSID | \
					  RX_CFG_ENABLE_ONLY_MY_DEST_MAC)

#define RX_CONFIG_OPTION_FOR_IBSS_JOIN   (RX_CFG_ENABLE_ONLY_MY_SSID | \
					  RX_CFG_ENABLE_ONLY_MY_DEST_MAC)

#define RX_FILTER_OPTION_DEF	      (CFG_RX_MGMT_EN | CFG_RX_DATA_EN\
				       | CFG_RX_CTL_EN | CFG_RX_BCN_EN\
				       | CFG_RX_AUTH_EN | CFG_RX_ASSOC_EN)

#define RX_FILTER_OPTION_FILTER_ALL	 0

#define RX_FILTER_OPTION_DEF_PRSP_BCN  (CFG_RX_PRSP_EN | CFG_RX_MGMT_EN\
					| CFG_RX_RCTS_ACK | CFG_RX_BCN_EN)

#define RX_FILTER_OPTION_JOIN	     (CFG_RX_MGMT_EN | CFG_RX_DATA_EN\
				      | CFG_RX_BCN_EN | CFG_RX_AUTH_EN\
				      | CFG_RX_ASSOC_EN | CFG_RX_RCTS_ACK\
				      | CFG_RX_PRSP_EN)



#define ACX_EE_CTL_REG                      EE_CTL
#define EE_WRITE                            0x00000001ul
#define EE_READ                             0x00000002ul


#define ACX_EE_ADDR_REG                     EE_ADDR


#define ACX_EE_DATA_REG                     EE_DATA


#define ACX_EE_CFG                          EE_CFG


#define ACX_GPIO_OUT_REG            GPIO_OUT
#define ACX_MAX_GPIO_LINES          15


#define ACX_CONT_WIND_CFG_REG    CONT_WIND_CFG
#define ACX_CONT_WIND_MIN_MASK   0x0000007f
#define ACX_CONT_WIND_MAX        0x03ff0000


#define HI_CFG_UART_ENABLE          0x00000004
#define HI_CFG_RST232_ENABLE        0x00000008
#define HI_CFG_CLOCK_REQ_SELECT     0x00000010
#define HI_CFG_HOST_INT_ENABLE      0x00000020
#define HI_CFG_VLYNQ_OUTPUT_ENABLE  0x00000040
#define HI_CFG_HOST_INT_ACTIVE_LOW  0x00000080
#define HI_CFG_UART_TX_OUT_GPIO_15  0x00000100
#define HI_CFG_UART_TX_OUT_GPIO_14  0x00000200
#define HI_CFG_UART_TX_OUT_GPIO_7   0x00000400


#ifdef USE_ACTIVE_HIGH
#define HI_CFG_DEF_VAL              \
	(HI_CFG_UART_ENABLE |        \
	HI_CFG_RST232_ENABLE |      \
	HI_CFG_CLOCK_REQ_SELECT |   \
	HI_CFG_HOST_INT_ENABLE)
#else
#define HI_CFG_DEF_VAL              \
	(HI_CFG_UART_ENABLE |        \
	HI_CFG_RST232_ENABLE |      \
	HI_CFG_CLOCK_REQ_SELECT |   \
	HI_CFG_HOST_INT_ENABLE)

#endif

#define REF_FREQ_19_2                       0
#define REF_FREQ_26_0                       1
#define REF_FREQ_38_4                       2
#define REF_FREQ_40_0                       3
#define REF_FREQ_33_6                       4
#define REF_FREQ_NUM                        5

#define LUT_PARAM_INTEGER_DIVIDER           0
#define LUT_PARAM_FRACTIONAL_DIVIDER        1
#define LUT_PARAM_ATTN_BB                   2
#define LUT_PARAM_ALPHA_BB                  3
#define LUT_PARAM_STOP_TIME_BB              4
#define LUT_PARAM_BB_PLL_LOOP_FILTER        5
#define LUT_PARAM_NUM                       6

#define ACX_EEPROMLESS_IND_REG              (SCR_PAD4)
#define USE_EEPROM                          0
#define SOFT_RESET_MAX_TIME                 1000000
#define SOFT_RESET_STALL_TIME               1000
#define NVS_DATA_BUNDARY_ALIGNMENT          4



#define CHUNK_SIZE          512


#define FW_HDR_SIZE 8

#define ECPU_CONTROL_HALT					0x00000101





enum {
	RADIO_BAND_2_4GHZ = 0,  
	RADIO_BAND_5GHZ = 1,    
	RADIO_BAND_JAPAN_4_9_GHZ = 2,
	DEFAULT_BAND = RADIO_BAND_2_4GHZ,
	INVALID_BAND = 0xFE,
	MAX_RADIO_BANDS = 0xFF
};

enum {
	NO_RATE      = 0,
	RATE_1MBPS   = 0x0A,
	RATE_2MBPS   = 0x14,
	RATE_5_5MBPS = 0x37,
	RATE_6MBPS   = 0x0B,
	RATE_9MBPS   = 0x0F,
	RATE_11MBPS  = 0x6E,
	RATE_12MBPS  = 0x0A,
	RATE_18MBPS  = 0x0E,
	RATE_22MBPS  = 0xDC,
	RATE_24MBPS  = 0x09,
	RATE_36MBPS  = 0x0D,
	RATE_48MBPS  = 0x08,
	RATE_54MBPS  = 0x0C
};

enum {
	RATE_INDEX_1MBPS   =  0,
	RATE_INDEX_2MBPS   =  1,
	RATE_INDEX_5_5MBPS =  2,
	RATE_INDEX_6MBPS   =  3,
	RATE_INDEX_9MBPS   =  4,
	RATE_INDEX_11MBPS  =  5,
	RATE_INDEX_12MBPS  =  6,
	RATE_INDEX_18MBPS  =  7,
	RATE_INDEX_22MBPS  =  8,
	RATE_INDEX_24MBPS  =  9,
	RATE_INDEX_36MBPS  =  10,
	RATE_INDEX_48MBPS  =  11,
	RATE_INDEX_54MBPS  =  12,
	RATE_INDEX_MAX     =  RATE_INDEX_54MBPS,
	MAX_RATE_INDEX,
	INVALID_RATE_INDEX = MAX_RATE_INDEX,
	RATE_INDEX_ENUM_MAX_SIZE = 0x7FFFFFFF
};

enum {
	RATE_MASK_1MBPS = 0x1,
	RATE_MASK_2MBPS = 0x2,
	RATE_MASK_5_5MBPS = 0x4,
	RATE_MASK_11MBPS = 0x20,
};

#define SHORT_PREAMBLE_BIT   BIT(0) 
#define OFDM_RATE_BIT        BIT(6)
#define PBCC_RATE_BIT        BIT(7)

enum {
	CCK_LONG = 0,
	CCK_SHORT = SHORT_PREAMBLE_BIT,
	PBCC_LONG = PBCC_RATE_BIT,
	PBCC_SHORT = PBCC_RATE_BIT | SHORT_PREAMBLE_BIT,
	OFDM = OFDM_RATE_BIT
};









#define INTR_TRIG_CMD       BIT(0)


#define INTR_TRIG_EVENT_ACK BIT(1)


#define INTR_TRIG_TX_PROC0 BIT(2)


#define INTR_TRIG_RX_PROC0 BIT(3)

#define INTR_TRIG_DEBUG_ACK BIT(4)

#define INTR_TRIG_STATE_CHANGED BIT(5)





#define INTR_TRIG_RX_PROC1 BIT(17)


#define INTR_TRIG_TX_PROC1 BIT(18)

#endif
