



#ifndef RT2500PCI_H
#define RT2500PCI_H


#define RF2522				0x0000
#define RF2523				0x0001
#define RF2524				0x0002
#define RF2525				0x0003
#define RF2525E				0x0004
#define RF5222				0x0010


#define RT2560_VERSION_B		2
#define RT2560_VERSION_C		3
#define RT2560_VERSION_D		4


#define DEFAULT_RSSI_OFFSET		121


#define CSR_REG_BASE			0x0000
#define CSR_REG_SIZE			0x0174
#define EEPROM_BASE			0x0000
#define EEPROM_SIZE			0x0200
#define BBP_BASE			0x0000
#define BBP_SIZE			0x0040
#define RF_BASE				0x0004
#define RF_SIZE				0x0010


#define NUM_TX_QUEUES			2




#define CSR0				0x0000


#define CSR1				0x0004
#define CSR1_SOFT_RESET			FIELD32(0x00000001)
#define CSR1_BBP_RESET			FIELD32(0x00000002)
#define CSR1_HOST_READY			FIELD32(0x00000004)


#define CSR2				0x0008


#define CSR3				0x000c
#define CSR3_BYTE0			FIELD32(0x000000ff)
#define CSR3_BYTE1			FIELD32(0x0000ff00)
#define CSR3_BYTE2			FIELD32(0x00ff0000)
#define CSR3_BYTE3			FIELD32(0xff000000)


#define CSR4				0x0010
#define CSR4_BYTE4			FIELD32(0x000000ff)
#define CSR4_BYTE5			FIELD32(0x0000ff00)


#define CSR5				0x0014
#define CSR5_BYTE0			FIELD32(0x000000ff)
#define CSR5_BYTE1			FIELD32(0x0000ff00)
#define CSR5_BYTE2			FIELD32(0x00ff0000)
#define CSR5_BYTE3			FIELD32(0xff000000)


#define CSR6				0x0018
#define CSR6_BYTE4			FIELD32(0x000000ff)
#define CSR6_BYTE5			FIELD32(0x0000ff00)


#define CSR7				0x001c
#define CSR7_TBCN_EXPIRE		FIELD32(0x00000001)
#define CSR7_TWAKE_EXPIRE		FIELD32(0x00000002)
#define CSR7_TATIMW_EXPIRE		FIELD32(0x00000004)
#define CSR7_TXDONE_TXRING		FIELD32(0x00000008)
#define CSR7_TXDONE_ATIMRING		FIELD32(0x00000010)
#define CSR7_TXDONE_PRIORING		FIELD32(0x00000020)
#define CSR7_RXDONE			FIELD32(0x00000040)
#define CSR7_DECRYPTION_DONE		FIELD32(0x00000080)
#define CSR7_ENCRYPTION_DONE		FIELD32(0x00000100)
#define CSR7_UART1_TX_TRESHOLD		FIELD32(0x00000200)
#define CSR7_UART1_RX_TRESHOLD		FIELD32(0x00000400)
#define CSR7_UART1_IDLE_TRESHOLD	FIELD32(0x00000800)
#define CSR7_UART1_TX_BUFF_ERROR	FIELD32(0x00001000)
#define CSR7_UART1_RX_BUFF_ERROR	FIELD32(0x00002000)
#define CSR7_UART2_TX_TRESHOLD		FIELD32(0x00004000)
#define CSR7_UART2_RX_TRESHOLD		FIELD32(0x00008000)
#define CSR7_UART2_IDLE_TRESHOLD	FIELD32(0x00010000)
#define CSR7_UART2_TX_BUFF_ERROR	FIELD32(0x00020000)
#define CSR7_UART2_RX_BUFF_ERROR	FIELD32(0x00040000)
#define CSR7_TIMER_CSR3_EXPIRE		FIELD32(0x00080000)


#define CSR8				0x0020
#define CSR8_TBCN_EXPIRE		FIELD32(0x00000001)
#define CSR8_TWAKE_EXPIRE		FIELD32(0x00000002)
#define CSR8_TATIMW_EXPIRE		FIELD32(0x00000004)
#define CSR8_TXDONE_TXRING		FIELD32(0x00000008)
#define CSR8_TXDONE_ATIMRING		FIELD32(0x00000010)
#define CSR8_TXDONE_PRIORING		FIELD32(0x00000020)
#define CSR8_RXDONE			FIELD32(0x00000040)
#define CSR8_DECRYPTION_DONE		FIELD32(0x00000080)
#define CSR8_ENCRYPTION_DONE		FIELD32(0x00000100)
#define CSR8_UART1_TX_TRESHOLD		FIELD32(0x00000200)
#define CSR8_UART1_RX_TRESHOLD		FIELD32(0x00000400)
#define CSR8_UART1_IDLE_TRESHOLD	FIELD32(0x00000800)
#define CSR8_UART1_TX_BUFF_ERROR	FIELD32(0x00001000)
#define CSR8_UART1_RX_BUFF_ERROR	FIELD32(0x00002000)
#define CSR8_UART2_TX_TRESHOLD		FIELD32(0x00004000)
#define CSR8_UART2_RX_TRESHOLD		FIELD32(0x00008000)
#define CSR8_UART2_IDLE_TRESHOLD	FIELD32(0x00010000)
#define CSR8_UART2_TX_BUFF_ERROR	FIELD32(0x00020000)
#define CSR8_UART2_RX_BUFF_ERROR	FIELD32(0x00040000)
#define CSR8_TIMER_CSR3_EXPIRE		FIELD32(0x00080000)


#define CSR9				0x0024
#define CSR9_MAX_FRAME_UNIT		FIELD32(0x00000f80)


#define SECCSR0				0x0028
#define SECCSR0_KICK_DECRYPT		FIELD32(0x00000001)
#define SECCSR0_ONE_SHOT		FIELD32(0x00000002)
#define SECCSR0_DESC_ADDRESS		FIELD32(0xfffffffc)


#define CSR11				0x002c
#define CSR11_CWMIN			FIELD32(0x0000000f)
#define CSR11_CWMAX			FIELD32(0x000000f0)
#define CSR11_SLOT_TIME			FIELD32(0x00001f00)
#define CSR11_CW_SELECT			FIELD32(0x00002000)
#define CSR11_LONG_RETRY		FIELD32(0x00ff0000)
#define CSR11_SHORT_RETRY		FIELD32(0xff000000)


#define CSR12				0x0030
#define CSR12_BEACON_INTERVAL		FIELD32(0x0000ffff)
#define CSR12_CFP_MAX_DURATION		FIELD32(0xffff0000)


#define CSR13				0x0034
#define CSR13_ATIMW_DURATION		FIELD32(0x0000ffff)
#define CSR13_CFP_PERIOD		FIELD32(0x00ff0000)


#define CSR14				0x0038
#define CSR14_TSF_COUNT			FIELD32(0x00000001)
#define CSR14_TSF_SYNC			FIELD32(0x00000006)
#define CSR14_TBCN			FIELD32(0x00000008)
#define CSR14_TCFP			FIELD32(0x00000010)
#define CSR14_TATIMW			FIELD32(0x00000020)
#define CSR14_BEACON_GEN		FIELD32(0x00000040)
#define CSR14_CFP_COUNT_PRELOAD		FIELD32(0x0000ff00)
#define CSR14_TBCM_PRELOAD		FIELD32(0xffff0000)


#define CSR15				0x003c
#define CSR15_CFP			FIELD32(0x00000001)
#define CSR15_ATIMW			FIELD32(0x00000002)
#define CSR15_BEACON_SENT		FIELD32(0x00000004)


#define CSR16				0x0040
#define CSR16_LOW_TSFTIMER		FIELD32(0xffffffff)


#define CSR17				0x0044
#define CSR17_HIGH_TSFTIMER		FIELD32(0xffffffff)


#define CSR18				0x0048
#define CSR18_SIFS			FIELD32(0x000001ff)
#define CSR18_PIFS			FIELD32(0x001f0000)


#define CSR19				0x004c
#define CSR19_DIFS			FIELD32(0x0000ffff)
#define CSR19_EIFS			FIELD32(0xffff0000)


#define CSR20				0x0050
#define CSR20_DELAY_AFTER_TBCN		FIELD32(0x0000ffff)
#define CSR20_TBCN_BEFORE_WAKEUP	FIELD32(0x00ff0000)
#define CSR20_AUTOWAKE			FIELD32(0x01000000)


#define CSR21				0x0054
#define CSR21_RELOAD			FIELD32(0x00000001)
#define CSR21_EEPROM_DATA_CLOCK		FIELD32(0x00000002)
#define CSR21_EEPROM_CHIP_SELECT	FIELD32(0x00000004)
#define CSR21_EEPROM_DATA_IN		FIELD32(0x00000008)
#define CSR21_EEPROM_DATA_OUT		FIELD32(0x00000010)
#define CSR21_TYPE_93C46		FIELD32(0x00000020)


#define CSR22				0x0058
#define CSR22_CFP_DURATION_REMAIN	FIELD32(0x0000ffff)
#define CSR22_RELOAD_CFP_DURATION	FIELD32(0x00010000)




#define TXCSR0				0x0060
#define TXCSR0_KICK_TX			FIELD32(0x00000001)
#define TXCSR0_KICK_ATIM		FIELD32(0x00000002)
#define TXCSR0_KICK_PRIO		FIELD32(0x00000004)
#define TXCSR0_ABORT			FIELD32(0x00000008)


#define TXCSR1				0x0064
#define TXCSR1_ACK_TIMEOUT		FIELD32(0x000001ff)
#define TXCSR1_ACK_CONSUME_TIME		FIELD32(0x0003fe00)
#define TXCSR1_TSF_OFFSET		FIELD32(0x00fc0000)
#define TXCSR1_AUTORESPONDER		FIELD32(0x01000000)


#define TXCSR2				0x0068
#define TXCSR2_TXD_SIZE			FIELD32(0x000000ff)
#define TXCSR2_NUM_TXD			FIELD32(0x0000ff00)
#define TXCSR2_NUM_ATIM			FIELD32(0x00ff0000)
#define TXCSR2_NUM_PRIO			FIELD32(0xff000000)


#define TXCSR3				0x006c
#define TXCSR3_TX_RING_REGISTER		FIELD32(0xffffffff)


#define TXCSR4				0x0070
#define TXCSR4_ATIM_RING_REGISTER	FIELD32(0xffffffff)


#define TXCSR5				0x0074
#define TXCSR5_PRIO_RING_REGISTER	FIELD32(0xffffffff)


#define TXCSR6				0x0078
#define TXCSR6_BEACON_RING_REGISTER	FIELD32(0xffffffff)


#define TXCSR7				0x007c
#define TXCSR7_AR_POWERMANAGEMENT	FIELD32(0x00000001)


#define TXCSR8				0x0098
#define TXCSR8_BBP_ID0			FIELD32(0x0000007f)
#define TXCSR8_BBP_ID0_VALID		FIELD32(0x00000080)
#define TXCSR8_BBP_ID1			FIELD32(0x00007f00)
#define TXCSR8_BBP_ID1_VALID		FIELD32(0x00008000)
#define TXCSR8_BBP_ID2			FIELD32(0x007f0000)
#define TXCSR8_BBP_ID2_VALID		FIELD32(0x00800000)
#define TXCSR8_BBP_ID3			FIELD32(0x7f000000)
#define TXCSR8_BBP_ID3_VALID		FIELD32(0x80000000)


#define TXCSR9				0x0094
#define TXCSR9_OFDM_RATE		FIELD32(0x000000ff)
#define TXCSR9_OFDM_SERVICE		FIELD32(0x0000ff00)
#define TXCSR9_OFDM_LENGTH_LOW		FIELD32(0x00ff0000)
#define TXCSR9_OFDM_LENGTH_HIGH		FIELD32(0xff000000)




#define RXCSR0				0x0080
#define RXCSR0_DISABLE_RX		FIELD32(0x00000001)
#define RXCSR0_DROP_CRC			FIELD32(0x00000002)
#define RXCSR0_DROP_PHYSICAL		FIELD32(0x00000004)
#define RXCSR0_DROP_CONTROL		FIELD32(0x00000008)
#define RXCSR0_DROP_NOT_TO_ME		FIELD32(0x00000010)
#define RXCSR0_DROP_TODS		FIELD32(0x00000020)
#define RXCSR0_DROP_VERSION_ERROR	FIELD32(0x00000040)
#define RXCSR0_PASS_CRC			FIELD32(0x00000080)
#define RXCSR0_PASS_PLCP		FIELD32(0x00000100)
#define RXCSR0_DROP_MCAST		FIELD32(0x00000200)
#define RXCSR0_DROP_BCAST		FIELD32(0x00000400)
#define RXCSR0_ENABLE_QOS		FIELD32(0x00000800)


#define RXCSR1				0x0084
#define RXCSR1_RXD_SIZE			FIELD32(0x000000ff)
#define RXCSR1_NUM_RXD			FIELD32(0x0000ff00)


#define RXCSR2				0x0088
#define RXCSR2_RX_RING_REGISTER		FIELD32(0xffffffff)


#define RXCSR3				0x0090
#define RXCSR3_BBP_ID0			FIELD32(0x0000007f)
#define RXCSR3_BBP_ID0_VALID		FIELD32(0x00000080)
#define RXCSR3_BBP_ID1			FIELD32(0x00007f00)
#define RXCSR3_BBP_ID1_VALID		FIELD32(0x00008000)
#define RXCSR3_BBP_ID2			FIELD32(0x007f0000)
#define RXCSR3_BBP_ID2_VALID		FIELD32(0x00800000)
#define RXCSR3_BBP_ID3			FIELD32(0x7f000000)
#define RXCSR3_BBP_ID3_VALID		FIELD32(0x80000000)


#define ARCSR1				0x009c
#define ARCSR1_AR_BBP_DATA2		FIELD32(0x000000ff)
#define ARCSR1_AR_BBP_ID2		FIELD32(0x0000ff00)
#define ARCSR1_AR_BBP_DATA3		FIELD32(0x00ff0000)
#define ARCSR1_AR_BBP_ID3		FIELD32(0xff000000)




#define PCICSR				0x008c
#define PCICSR_BIG_ENDIAN		FIELD32(0x00000001)
#define PCICSR_RX_TRESHOLD		FIELD32(0x00000006)
#define PCICSR_TX_TRESHOLD		FIELD32(0x00000018)
#define PCICSR_BURST_LENTH		FIELD32(0x00000060)
#define PCICSR_ENABLE_CLK		FIELD32(0x00000080)
#define PCICSR_READ_MULTIPLE		FIELD32(0x00000100)
#define PCICSR_WRITE_INVALID		FIELD32(0x00000200)


#define CNT0				0x00a0
#define CNT0_FCS_ERROR			FIELD32(0x0000ffff)


#define TIMECSR2			0x00a8
#define CNT1				0x00ac
#define CNT2				0x00b0
#define TIMECSR3			0x00b4


#define CNT3				0x00b8
#define CNT3_FALSE_CCA			FIELD32(0x0000ffff)


#define CNT4				0x00bc
#define CNT5				0x00c0




#define PWRCSR0				0x00c4


#define PSCSR0				0x00c8
#define PSCSR1				0x00cc
#define PSCSR2				0x00d0
#define PSCSR3				0x00d4


#define PWRCSR1				0x00d8
#define PWRCSR1_SET_STATE		FIELD32(0x00000001)
#define PWRCSR1_BBP_DESIRE_STATE	FIELD32(0x00000006)
#define PWRCSR1_RF_DESIRE_STATE		FIELD32(0x00000018)
#define PWRCSR1_BBP_CURR_STATE		FIELD32(0x00000060)
#define PWRCSR1_RF_CURR_STATE		FIELD32(0x00000180)
#define PWRCSR1_PUT_TO_SLEEP		FIELD32(0x00000200)


#define TIMECSR				0x00dc
#define TIMECSR_US_COUNT		FIELD32(0x000000ff)
#define TIMECSR_US_64_COUNT		FIELD32(0x0000ff00)
#define TIMECSR_BEACON_EXPECT		FIELD32(0x00070000)


#define MACCSR0				0x00e0


#define MACCSR1				0x00e4
#define MACCSR1_KICK_RX			FIELD32(0x00000001)
#define MACCSR1_ONESHOT_RXMODE		FIELD32(0x00000002)
#define MACCSR1_BBPRX_RESET_MODE	FIELD32(0x00000004)
#define MACCSR1_AUTO_TXBBP		FIELD32(0x00000008)
#define MACCSR1_AUTO_RXBBP		FIELD32(0x00000010)
#define MACCSR1_LOOPBACK		FIELD32(0x00000060)
#define MACCSR1_INTERSIL_IF		FIELD32(0x00000080)


#define RALINKCSR			0x00e8
#define RALINKCSR_AR_BBP_DATA0		FIELD32(0x000000ff)
#define RALINKCSR_AR_BBP_ID0		FIELD32(0x00007f00)
#define RALINKCSR_AR_BBP_VALID0		FIELD32(0x00008000)
#define RALINKCSR_AR_BBP_DATA1		FIELD32(0x00ff0000)
#define RALINKCSR_AR_BBP_ID1		FIELD32(0x7f000000)
#define RALINKCSR_AR_BBP_VALID1		FIELD32(0x80000000)


#define BCNCSR				0x00ec
#define BCNCSR_CHANGE			FIELD32(0x00000001)
#define BCNCSR_DELTATIME		FIELD32(0x0000001e)
#define BCNCSR_NUM_BEACON		FIELD32(0x00001fe0)
#define BCNCSR_MODE			FIELD32(0x00006000)
#define BCNCSR_PLUS			FIELD32(0x00008000)




#define BBPCSR				0x00f0
#define BBPCSR_VALUE			FIELD32(0x000000ff)
#define BBPCSR_REGNUM			FIELD32(0x00007f00)
#define BBPCSR_BUSY			FIELD32(0x00008000)
#define BBPCSR_WRITE_CONTROL		FIELD32(0x00010000)


#define RFCSR				0x00f4
#define RFCSR_VALUE			FIELD32(0x00ffffff)
#define RFCSR_NUMBER_OF_BITS		FIELD32(0x1f000000)
#define RFCSR_IF_SELECT			FIELD32(0x20000000)
#define RFCSR_PLL_LD			FIELD32(0x40000000)
#define RFCSR_BUSY			FIELD32(0x80000000)


#define LEDCSR				0x00f8
#define LEDCSR_ON_PERIOD		FIELD32(0x000000ff)
#define LEDCSR_OFF_PERIOD		FIELD32(0x0000ff00)
#define LEDCSR_LINK			FIELD32(0x00010000)
#define LEDCSR_ACTIVITY			FIELD32(0x00020000)
#define LEDCSR_LINK_POLARITY		FIELD32(0x00040000)
#define LEDCSR_ACTIVITY_POLARITY	FIELD32(0x00080000)
#define LEDCSR_LED_DEFAULT		FIELD32(0x00100000)


#define SECCSR3				0x00fc


#define RXPTR				0x0100
#define TXPTR				0x0104
#define PRIPTR				0x0108
#define ATIMPTR				0x010c


#define TXACKCSR0			0x0110


#define ACKCNT0				0x0114
#define ACKCNT1				0x0118




#define GPIOCSR				0x0120
#define GPIOCSR_BIT0			FIELD32(0x00000001)
#define GPIOCSR_BIT1			FIELD32(0x00000002)
#define GPIOCSR_BIT2			FIELD32(0x00000004)
#define GPIOCSR_BIT3			FIELD32(0x00000008)
#define GPIOCSR_BIT4			FIELD32(0x00000010)
#define GPIOCSR_BIT5			FIELD32(0x00000020)
#define GPIOCSR_BIT6			FIELD32(0x00000040)
#define GPIOCSR_BIT7			FIELD32(0x00000080)
#define GPIOCSR_DIR0			FIELD32(0x00000100)
#define GPIOCSR_DIR1			FIELD32(0x00000200)
#define GPIOCSR_DIR2			FIELD32(0x00000400)
#define GPIOCSR_DIR3			FIELD32(0x00000800)
#define GPIOCSR_DIR4			FIELD32(0x00001000)
#define GPIOCSR_DIR5			FIELD32(0x00002000)
#define GPIOCSR_DIR6			FIELD32(0x00004000)
#define GPIOCSR_DIR7			FIELD32(0x00008000)


#define FIFOCSR0			0x0128
#define FIFOCSR1			0x012c


#define BCNCSR1				0x0130
#define BCNCSR1_PRELOAD			FIELD32(0x0000ffff)
#define BCNCSR1_BEACON_CWMIN		FIELD32(0x000f0000)


#define MACCSR2				0x0134
#define MACCSR2_DELAY			FIELD32(0x000000ff)


#define TESTCSR				0x0138


#define ARCSR2				0x013c
#define ARCSR2_SIGNAL			FIELD32(0x000000ff)
#define ARCSR2_SERVICE			FIELD32(0x0000ff00)
#define ARCSR2_LENGTH			FIELD32(0xffff0000)


#define ARCSR3				0x0140
#define ARCSR3_SIGNAL			FIELD32(0x000000ff)
#define ARCSR3_SERVICE			FIELD32(0x0000ff00)
#define ARCSR3_LENGTH			FIELD32(0xffff0000)


#define ARCSR4				0x0144
#define ARCSR4_SIGNAL			FIELD32(0x000000ff)
#define ARCSR4_SERVICE			FIELD32(0x0000ff00)
#define ARCSR4_LENGTH			FIELD32(0xffff0000)


#define ARCSR5				0x0148
#define ARCSR5_SIGNAL			FIELD32(0x000000ff)
#define ARCSR5_SERVICE			FIELD32(0x0000ff00)
#define ARCSR5_LENGTH			FIELD32(0xffff0000)


#define ARTCSR0				0x014c
#define ARTCSR0_ACK_CTS_11MBS		FIELD32(0x000000ff)
#define ARTCSR0_ACK_CTS_5_5MBS		FIELD32(0x0000ff00)
#define ARTCSR0_ACK_CTS_2MBS		FIELD32(0x00ff0000)
#define ARTCSR0_ACK_CTS_1MBS		FIELD32(0xff000000)



#define ARTCSR1				0x0150
#define ARTCSR1_ACK_CTS_6MBS		FIELD32(0x000000ff)
#define ARTCSR1_ACK_CTS_9MBS		FIELD32(0x0000ff00)
#define ARTCSR1_ACK_CTS_12MBS		FIELD32(0x00ff0000)
#define ARTCSR1_ACK_CTS_18MBS		FIELD32(0xff000000)


#define ARTCSR2				0x0154
#define ARTCSR2_ACK_CTS_24MBS		FIELD32(0x000000ff)
#define ARTCSR2_ACK_CTS_36MBS		FIELD32(0x0000ff00)
#define ARTCSR2_ACK_CTS_48MBS		FIELD32(0x00ff0000)
#define ARTCSR2_ACK_CTS_54MBS		FIELD32(0xff000000)


#define SECCSR1				0x0158
#define SECCSR1_KICK_ENCRYPT		FIELD32(0x00000001)
#define SECCSR1_ONE_SHOT		FIELD32(0x00000002)
#define SECCSR1_DESC_ADDRESS		FIELD32(0xfffffffc)


#define BBPCSR1				0x015c
#define BBPCSR1_CCK			FIELD32(0x00000003)
#define BBPCSR1_CCK_FLIP		FIELD32(0x00000004)
#define BBPCSR1_OFDM			FIELD32(0x00030000)
#define BBPCSR1_OFDM_FLIP		FIELD32(0x00040000)


#define DBANDCSR0			0x0160
#define DBANDCSR1			0x0164


#define BBPPCSR				0x0168


#define DBGSEL0				0x016c
#define DBGSEL1				0x0170


#define BISTCSR				0x0174


#define MCAST0				0x0178
#define MCAST1				0x017c


#define UARTCSR0			0x0180
#define UARTCSR1			0x0184
#define UARTCSR3			0x0188
#define UARTCSR4			0x018c
#define UART2CSR0			0x0190
#define UART2CSR1			0x0194
#define UART2CSR3			0x0198
#define UART2CSR4			0x019c




#define BBP_R2_TX_ANTENNA		FIELD8(0x03)
#define BBP_R2_TX_IQ_FLIP		FIELD8(0x04)


#define BBP_R14_RX_ANTENNA		FIELD8(0x03)
#define BBP_R14_RX_IQ_FLIP		FIELD8(0x04)


#define BBP_R70_JAPAN_FILTER		FIELD8(0x08)




#define RF1_TUNER			FIELD32(0x00020000)


#define RF3_TUNER			FIELD32(0x00000100)
#define RF3_TXPOWER			FIELD32(0x00003e00)




#define EEPROM_MAC_ADDR_0		0x0002
#define EEPROM_MAC_ADDR_BYTE0		FIELD16(0x00ff)
#define EEPROM_MAC_ADDR_BYTE1		FIELD16(0xff00)
#define EEPROM_MAC_ADDR1		0x0003
#define EEPROM_MAC_ADDR_BYTE2		FIELD16(0x00ff)
#define EEPROM_MAC_ADDR_BYTE3		FIELD16(0xff00)
#define EEPROM_MAC_ADDR_2		0x0004
#define EEPROM_MAC_ADDR_BYTE4		FIELD16(0x00ff)
#define EEPROM_MAC_ADDR_BYTE5		FIELD16(0xff00)


#define EEPROM_ANTENNA			0x10
#define EEPROM_ANTENNA_NUM		FIELD16(0x0003)
#define EEPROM_ANTENNA_TX_DEFAULT	FIELD16(0x000c)
#define EEPROM_ANTENNA_RX_DEFAULT	FIELD16(0x0030)
#define EEPROM_ANTENNA_LED_MODE		FIELD16(0x01c0)
#define EEPROM_ANTENNA_DYN_TXAGC	FIELD16(0x0200)
#define EEPROM_ANTENNA_HARDWARE_RADIO	FIELD16(0x0400)
#define EEPROM_ANTENNA_RF_TYPE		FIELD16(0xf800)


#define EEPROM_NIC			0x11
#define EEPROM_NIC_CARDBUS_ACCEL	FIELD16(0x0001)
#define EEPROM_NIC_DYN_BBP_TUNE		FIELD16(0x0002)
#define EEPROM_NIC_CCK_TX_POWER		FIELD16(0x000c)


#define EEPROM_GEOGRAPHY		0x12
#define EEPROM_GEOGRAPHY_GEO		FIELD16(0x0f00)


#define EEPROM_BBP_START		0x13
#define EEPROM_BBP_SIZE			16
#define EEPROM_BBP_VALUE		FIELD16(0x00ff)
#define EEPROM_BBP_REG_ID		FIELD16(0xff00)


#define EEPROM_TXPOWER_START		0x23
#define EEPROM_TXPOWER_SIZE		7
#define EEPROM_TXPOWER_1		FIELD16(0x00ff)
#define EEPROM_TXPOWER_2		FIELD16(0xff00)


#define EEPROM_CALIBRATE_OFFSET		0x3e
#define EEPROM_CALIBRATE_OFFSET_RSSI	FIELD16(0x00ff)


#define TXD_DESC_SIZE			( 11 * sizeof(__le32) )
#define RXD_DESC_SIZE			( 11 * sizeof(__le32) )




#define TXD_W0_OWNER_NIC		FIELD32(0x00000001)
#define TXD_W0_VALID			FIELD32(0x00000002)
#define TXD_W0_RESULT			FIELD32(0x0000001c)
#define TXD_W0_RETRY_COUNT		FIELD32(0x000000e0)
#define TXD_W0_MORE_FRAG		FIELD32(0x00000100)
#define TXD_W0_ACK			FIELD32(0x00000200)
#define TXD_W0_TIMESTAMP		FIELD32(0x00000400)
#define TXD_W0_OFDM			FIELD32(0x00000800)
#define TXD_W0_CIPHER_OWNER		FIELD32(0x00001000)
#define TXD_W0_IFS			FIELD32(0x00006000)
#define TXD_W0_RETRY_MODE		FIELD32(0x00008000)
#define TXD_W0_DATABYTE_COUNT		FIELD32(0x0fff0000)
#define TXD_W0_CIPHER_ALG		FIELD32(0xe0000000)


#define TXD_W1_BUFFER_ADDRESS		FIELD32(0xffffffff)


#define TXD_W2_IV_OFFSET		FIELD32(0x0000003f)
#define TXD_W2_AIFS			FIELD32(0x000000c0)
#define TXD_W2_CWMIN			FIELD32(0x00000f00)
#define TXD_W2_CWMAX			FIELD32(0x0000f000)


#define TXD_W3_PLCP_SIGNAL		FIELD32(0x000000ff)
#define TXD_W3_PLCP_SERVICE		FIELD32(0x0000ff00)
#define TXD_W3_PLCP_LENGTH_LOW		FIELD32(0x00ff0000)
#define TXD_W3_PLCP_LENGTH_HIGH		FIELD32(0xff000000)


#define TXD_W4_IV			FIELD32(0xffffffff)


#define TXD_W5_EIV			FIELD32(0xffffffff)


#define TXD_W6_KEY			FIELD32(0xffffffff)
#define TXD_W7_KEY			FIELD32(0xffffffff)
#define TXD_W8_KEY			FIELD32(0xffffffff)
#define TXD_W9_KEY			FIELD32(0xffffffff)


#define TXD_W10_RTS			FIELD32(0x00000001)
#define TXD_W10_TX_RATE			FIELD32(0x000000fe)




#define RXD_W0_OWNER_NIC		FIELD32(0x00000001)
#define RXD_W0_UNICAST_TO_ME		FIELD32(0x00000002)
#define RXD_W0_MULTICAST		FIELD32(0x00000004)
#define RXD_W0_BROADCAST		FIELD32(0x00000008)
#define RXD_W0_MY_BSS			FIELD32(0x00000010)
#define RXD_W0_CRC_ERROR		FIELD32(0x00000020)
#define RXD_W0_OFDM			FIELD32(0x00000040)
#define RXD_W0_PHYSICAL_ERROR		FIELD32(0x00000080)
#define RXD_W0_CIPHER_OWNER		FIELD32(0x00000100)
#define RXD_W0_ICV_ERROR		FIELD32(0x00000200)
#define RXD_W0_IV_OFFSET		FIELD32(0x0000fc00)
#define RXD_W0_DATABYTE_COUNT		FIELD32(0x0fff0000)
#define RXD_W0_CIPHER_ALG		FIELD32(0xe0000000)


#define RXD_W1_BUFFER_ADDRESS		FIELD32(0xffffffff)


#define RXD_W2_SIGNAL			FIELD32(0x000000ff)
#define RXD_W2_RSSI			FIELD32(0x0000ff00)
#define RXD_W2_TA			FIELD32(0xffff0000)


#define RXD_W3_TA			FIELD32(0xffffffff)


#define RXD_W4_IV			FIELD32(0xffffffff)


#define RXD_W5_EIV			FIELD32(0xffffffff)


#define RXD_W6_KEY			FIELD32(0xffffffff)
#define RXD_W7_KEY			FIELD32(0xffffffff)
#define RXD_W8_KEY			FIELD32(0xffffffff)
#define RXD_W9_KEY			FIELD32(0xffffffff)


#define RXD_W10_DROP			FIELD32(0x00000001)


#define MIN_TXPOWER	0
#define MAX_TXPOWER	31
#define DEFAULT_TXPOWER	24

#define TXPOWER_FROM_DEV(__txpower) \
	(((u8)(__txpower)) > MAX_TXPOWER) ? DEFAULT_TXPOWER : (__txpower)

#define TXPOWER_TO_DEV(__txpower) \
	clamp_t(char, __txpower, MIN_TXPOWER, MAX_TXPOWER)

#endif 
