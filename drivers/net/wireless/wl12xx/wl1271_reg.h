

#ifndef __REG_H__
#define __REG_H__

#include <linux/bitops.h>

#define REGISTERS_BASE 0x00300000
#define DRPW_BASE      0x00310000

#define REGISTERS_DOWN_SIZE 0x00008800
#define REGISTERS_WORK_SIZE 0x0000b000

#define HW_ACCESS_ELP_CTRL_REG_ADDR         0x1FFFC
#define STATUS_MEM_ADDRESS                  0x40400


#define ELPCTRL_WAKE_UP             0x1
#define ELPCTRL_WAKE_UP_WLAN_READY  0x5
#define ELPCTRL_SLEEP               0x0

#define ELPCTRL_WLAN_READY          0x2


#define ACX_REG_SLV_SOFT_RESET         (REGISTERS_BASE + 0x0000)

#define WL1271_SLV_REG_DATA            (REGISTERS_BASE + 0x0008)
#define WL1271_SLV_REG_ADATA           (REGISTERS_BASE + 0x000c)
#define WL1271_SLV_MEM_DATA            (REGISTERS_BASE + 0x0018)

#define FIQ_MASK                       (REGISTERS_BASE + 0x0400)
#define FIQ_MASK_L                     (REGISTERS_BASE + 0x0400)
#define FIQ_MASK_H                     (REGISTERS_BASE + 0x0404)
#define FIQ_MASK_SET                   (REGISTERS_BASE + 0x0408)
#define FIQ_MASK_SET_L                 (REGISTERS_BASE + 0x0408)
#define FIQ_MASK_SET_H                 (REGISTERS_BASE + 0x040C)
#define FIQ_MASK_CLR                   (REGISTERS_BASE + 0x0410)
#define FIQ_MASK_CLR_L                 (REGISTERS_BASE + 0x0410)
#define FIQ_MASK_CLR_H                 (REGISTERS_BASE + 0x0414)
#define IRQ_MASK                       (REGISTERS_BASE + 0x0418)
#define IRQ_MASK_L                     (REGISTERS_BASE + 0x0418)
#define IRQ_MASK_H                     (REGISTERS_BASE + 0x041C)
#define IRQ_MASK_SET                   (REGISTERS_BASE + 0x0420)
#define IRQ_MASK_SET_L                 (REGISTERS_BASE + 0x0420)
#define IRQ_MASK_SET_H                 (REGISTERS_BASE + 0x0424)
#define IRQ_MASK_CLR                   (REGISTERS_BASE + 0x0428)
#define IRQ_MASK_CLR_L                 (REGISTERS_BASE + 0x0428)
#define IRQ_MASK_CLR_H                 (REGISTERS_BASE + 0x042C)
#define ECPU_MASK                      (REGISTERS_BASE + 0x0448)
#define FIQ_STS_L                      (REGISTERS_BASE + 0x044C)
#define FIQ_STS_H                      (REGISTERS_BASE + 0x0450)
#define IRQ_STS_L                      (REGISTERS_BASE + 0x0454)
#define IRQ_STS_H                      (REGISTERS_BASE + 0x0458)
#define INT_STS_ND                     (REGISTERS_BASE + 0x0464)
#define INT_STS_RAW_L                  (REGISTERS_BASE + 0x0464)
#define INT_STS_RAW_H                  (REGISTERS_BASE + 0x0468)
#define INT_STS_CLR                    (REGISTERS_BASE + 0x04B4)
#define INT_STS_CLR_L                  (REGISTERS_BASE + 0x04B4)
#define INT_STS_CLR_H                  (REGISTERS_BASE + 0x04B8)
#define INT_ACK                        (REGISTERS_BASE + 0x046C)
#define INT_ACK_L                      (REGISTERS_BASE + 0x046C)
#define INT_ACK_H                      (REGISTERS_BASE + 0x0470)
#define INT_TRIG                       (REGISTERS_BASE + 0x0474)
#define INT_TRIG_L                     (REGISTERS_BASE + 0x0474)
#define INT_TRIG_H                     (REGISTERS_BASE + 0x0478)
#define HOST_STS_L                     (REGISTERS_BASE + 0x045C)
#define HOST_STS_H                     (REGISTERS_BASE + 0x0460)
#define HOST_MASK                      (REGISTERS_BASE + 0x0430)
#define HOST_MASK_L                    (REGISTERS_BASE + 0x0430)
#define HOST_MASK_H                    (REGISTERS_BASE + 0x0434)
#define HOST_MASK_SET                  (REGISTERS_BASE + 0x0438)
#define HOST_MASK_SET_L                (REGISTERS_BASE + 0x0438)
#define HOST_MASK_SET_H                (REGISTERS_BASE + 0x043C)
#define HOST_MASK_CLR                  (REGISTERS_BASE + 0x0440)
#define HOST_MASK_CLR_L                (REGISTERS_BASE + 0x0440)
#define HOST_MASK_CLR_H                (REGISTERS_BASE + 0x0444)

#define ACX_REG_INTERRUPT_TRIG         (REGISTERS_BASE + 0x0474)
#define ACX_REG_INTERRUPT_TRIG_H       (REGISTERS_BASE + 0x0478)


#define HINT_MASK                      (REGISTERS_BASE + 0x0494)
#define HINT_MASK_SET                  (REGISTERS_BASE + 0x0498)
#define HINT_MASK_CLR                  (REGISTERS_BASE + 0x049C)
#define HINT_STS_ND_MASKED             (REGISTERS_BASE + 0x04A0)

#define HINT_STS_ND		       (REGISTERS_BASE + 0x04B0)
#define HINT_STS_CLR                   (REGISTERS_BASE + 0x04A4)
#define HINT_ACK                       (REGISTERS_BASE + 0x04A8)
#define HINT_TRIG                      (REGISTERS_BASE + 0x04AC)


#define ACX_REG_INTERRUPT_MASK         (REGISTERS_BASE + 0x04DC)


#define ACX_REG_HINT_MASK_SET          (REGISTERS_BASE + 0x04E0)


#define ACX_REG_HINT_MASK_CLR          (REGISTERS_BASE + 0x04E4)


#define ACX_REG_INTERRUPT_NO_CLEAR     (REGISTERS_BASE + 0x04E8)


#define ACX_REG_INTERRUPT_CLEAR        (REGISTERS_BASE + 0x04F8)


#define ACX_REG_INTERRUPT_ACK          (REGISTERS_BASE + 0x04F0)

#define RX_DRIVER_DUMMY_WRITE_ADDRESS  (REGISTERS_BASE + 0x0534)
#define RX_DRIVER_COUNTER_ADDRESS      (REGISTERS_BASE + 0x0538)


#define SOR_CFG                        (REGISTERS_BASE + 0x0800)




#define ACX_REG_ECPU_CONTROL           (REGISTERS_BASE + 0x0804)

#define HI_CFG                         (REGISTERS_BASE + 0x0808)


#define ACX_REG_EE_START               (REGISTERS_BASE + 0x080C)

#define OCP_POR_CTR                    (REGISTERS_BASE + 0x09B4)
#define OCP_DATA_WRITE                 (REGISTERS_BASE + 0x09B8)
#define OCP_DATA_READ                  (REGISTERS_BASE + 0x09BC)
#define OCP_CMD                        (REGISTERS_BASE + 0x09C0)

#define WL1271_HOST_WR_ACCESS          (REGISTERS_BASE + 0x09F8)

#define CHIP_ID_B                      (REGISTERS_BASE + 0x5674)

#define CHIP_ID_1271_PG10              (0x4030101)
#define CHIP_ID_1271_PG20              (0x4030111)

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

#define PLL_PARAMETERS                 (REGISTERS_BASE + 0x6040)
#define WU_COUNTER_PAUSE               (REGISTERS_BASE + 0x6008)
#define WELP_ARM_COMMAND               (REGISTERS_BASE + 0x6100)
#define DRPW_SCRATCH_START             (DRPW_BASE + 0x002C)


#define ACX_SLV_SOFT_RESET_BIT   BIT(1)
#define ACX_REG_EEPROM_START_BIT BIT(1)




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



#define ACX_PHY_ADDR_REG                SBB_ADDR
#define ACX_PHY_DATA_REG                SBB_DATA
#define ACX_PHY_CTRL_REG                SBB_CTL
#define ACX_PHY_REG_WR_MASK             0x00000001ul
#define ACX_PHY_REG_RD_MASK             0x00000002ul



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


#define HW_SLAVE_REG_ADDR_REG		0x00000004
#define HW_SLAVE_REG_DATA_REG		0x00000008
#define HW_SLAVE_REG_CTRL_REG		0x0000000c

#define SLAVE_AUTO_INC				0x00010000
#define SLAVE_NO_AUTO_INC			0x00000000
#define SLAVE_HOST_LITTLE_ENDIAN	0x00000000

#define HW_SLAVE_MEM_ADDR_REG		SLV_MEM_ADDR
#define HW_SLAVE_MEM_DATA_REG		SLV_MEM_DATA
#define HW_SLAVE_MEM_CTRL_REG		SLV_MEM_CTL
#define HW_SLAVE_MEM_ENDIAN_REG		SLV_END_CTL

#define HW_FUNC_EVENT_INT_EN		0x8000
#define HW_FUNC_EVENT_MASK_REG		0x00000034

#define ACX_MAC_TIMESTAMP_REG	(MAC_TIMESTAMP)


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




#define TNETW1251_CHIP_ID_PG1_0         0x07010101
#define TNETW1251_CHIP_ID_PG1_1         0x07020101
#define TNETW1251_CHIP_ID_PG1_2	        0x07030101






#define INTR_TRIG_CMD       BIT(0)


#define INTR_TRIG_EVENT_ACK BIT(1)


#define INTR_TRIG_TX_PROC0 BIT(2)


#define INTR_TRIG_RX_PROC0 BIT(3)

#define INTR_TRIG_DEBUG_ACK BIT(4)

#define INTR_TRIG_STATE_CHANGED BIT(5)





#define INTR_TRIG_RX_PROC1 BIT(17)


#define INTR_TRIG_TX_PROC1 BIT(18)

#endif
