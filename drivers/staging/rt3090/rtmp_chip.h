

#ifndef	__RTMP_CHIP_H__
#define	__RTMP_CHIP_H__

#include "rtmp_type.h"

#ifdef RT3090
#include "rt3090.h"
#endif 

#ifdef RT3370
#include "rt3370.h"
#endif 

#ifdef RT3390
#include "rt3390.h"
#endif 











#define IS_RT3090A(_pAd)				((((_pAd)->MACVersion & 0xffff0000) == 0x30900000))


#define IS_RT3090(_pAd)				((((_pAd)->MACVersion & 0xffff0000) == 0x30710000) || (IS_RT3090A(_pAd)))

#define IS_RT3070(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x30700000)
#define IS_RT3071(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x30710000)
#define IS_RT2070(_pAd)		(((_pAd)->RfIcType == RFIC_2020) || ((_pAd)->EFuseTag == 0x27))

#define IS_RT30xx(_pAd)		(((_pAd)->MACVersion & 0xfff00000) == 0x30700000||IS_RT3090A(_pAd))



#define IS_RT3572(_pAd)		(((_pAd)->MACVersion & 0xffff0000) == 0x35720000)
#define IS_VERSION_BEFORE_F(_pAd)			(((_pAd)->MACVersion&0xffff) <= 0x0211)

#define IS_VERSION_AFTER_F(_pAd)			((((_pAd)->MACVersion&0xffff) >= 0x0212) || (((_pAd)->b3090ESpecialChip == TRUE)))










#define IS_RT3390(_pAd)				(((_pAd)->MACVersion & 0xFFFF0000) == 0x33900000)




#define CHIP_PCI_CFG		0x0000
#define CHIP_PCI_EECTRL		0x0004
#define CHIP_PCI_MCUCTRL	0x0008

#define OPT_14			0x114

#define RETRY_LIMIT		10






#define	BUSY		                1
#define	IDLE		                0





#define EEDO                        0x08
#define EEDI                        0x04
#define EECS                        0x02
#define EESK                        0x01
#define EERL                        0x80

#define EEPROM_WRITE_OPCODE         0x05
#define EEPROM_READ_OPCODE          0x06
#define EEPROM_EWDS_OPCODE          0x10
#define EEPROM_EWEN_OPCODE          0x13

#define NUM_EEPROM_BBP_PARMS		19			
#define NUM_EEPROM_TX_G_PARMS		7
#define EEPROM_NIC1_OFFSET          0x34		
#define EEPROM_NIC2_OFFSET          0x36		
#define EEPROM_BBP_BASE_OFFSET		0xf0		
#define EEPROM_G_TX_PWR_OFFSET		0x52
#define EEPROM_G_TX2_PWR_OFFSET		0x60
#define EEPROM_LED1_OFFSET			0x3c
#define EEPROM_LED2_OFFSET			0x3e
#define EEPROM_LED3_OFFSET			0x40
#define EEPROM_LNA_OFFSET			0x44
#define EEPROM_RSSI_BG_OFFSET		0x46
#define EEPROM_TXMIXER_GAIN_2_4G	0x48
#define EEPROM_RSSI_A_OFFSET		0x4a
#define EEPROM_TXMIXER_GAIN_5G		0x4c
#define EEPROM_DEFINE_MAX_TXPWR		0x4e
#define EEPROM_TXPOWER_BYRATE_20MHZ_2_4G	0xde	
#define EEPROM_TXPOWER_BYRATE_40MHZ_2_4G	0xee	
#define EEPROM_TXPOWER_BYRATE_20MHZ_5G		0xfa	
#define EEPROM_TXPOWER_BYRATE_40MHZ_5G		0x10a	
#define EEPROM_A_TX_PWR_OFFSET      0x78
#define EEPROM_A_TX2_PWR_OFFSET      0xa6






#define EEPROM_VERSION_OFFSET       0x02
#define EEPROM_FREQ_OFFSET			0x3a
#define EEPROM_TXPOWER_BYRATE	0xde	
#define EEPROM_TXPOWER_DELTA		0x50	
#define VALID_EEPROM_VERSION        1



#define RT28xx_EEPROM_READ16(_pAd, _offset, _value)			\
	(_pAd)->chipOps.eeread((RTMP_ADAPTER *)(_pAd), (USHORT)(_offset), (PUSHORT)&(_value))

#define RT28xx_EEPROM_WRITE16(_pAd, _offset, _value)		\
	(_pAd)->chipOps.eewrite((RTMP_ADAPTER *)(_pAd), (USHORT)(_offset), (USHORT)(_value))










typedef union  _MCU_LEDCS_STRUC {
	struct	{
#ifdef RT_BIG_ENDIAN
		UCHAR		Polarity:1;
		UCHAR		LedMode:7;
#else
		UCHAR		LedMode:7;
		UCHAR		Polarity:1;
#endif 
	} field;
	UCHAR				word;
} MCU_LEDCS_STRUC, *PMCU_LEDCS_STRUC;





#ifdef RT_BIG_ENDIAN
typedef	union	_EEPROM_ANTENNA_STRUC	{
	struct	{
		USHORT      Rsv:4;
		USHORT      RfIcType:4;             
		USHORT		TxPath:4;	
		USHORT		RxPath:4;	
	}	field;
	USHORT			word;
}	EEPROM_ANTENNA_STRUC, *PEEPROM_ANTENNA_STRUC;
#else
typedef	union	_EEPROM_ANTENNA_STRUC	{
	struct	{
		USHORT		RxPath:4;	
		USHORT		TxPath:4;	
		USHORT      RfIcType:4;             
		USHORT      Rsv:4;
	}	field;
	USHORT			word;
}	EEPROM_ANTENNA_STRUC, *PEEPROM_ANTENNA_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef	union _EEPROM_NIC_CINFIG2_STRUC	{
	struct	{
		USHORT		DACTestBit:1;			
		USHORT		Rsv2:3;					
		USHORT		AntDiversity:1;			
		USHORT		Rsv1:1;					
		USHORT		BW40MAvailForA:1;			
		USHORT		BW40MAvailForG:1;			
		USHORT		EnableWPSPBC:1;                 
		USHORT		BW40MSidebandForA:1;
		USHORT		BW40MSidebandForG:1;
		USHORT		CardbusAcceleration:1;	
		USHORT		ExternalLNAForA:1;			
		USHORT		ExternalLNAForG:1;			
		USHORT		DynamicTxAgcControl:1;			
		USHORT		HardwareRadioControl:1;	
	}	field;
	USHORT			word;
}	EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#else
typedef	union _EEPROM_NIC_CINFIG2_STRUC	{
	struct {
		USHORT		HardwareRadioControl:1;	
		USHORT		DynamicTxAgcControl:1;			
		USHORT		ExternalLNAForG:1;				
		USHORT		ExternalLNAForA:1;			
		USHORT		CardbusAcceleration:1;	
		USHORT		BW40MSidebandForG:1;
		USHORT		BW40MSidebandForA:1;
		USHORT		EnableWPSPBC:1;                 
		USHORT		BW40MAvailForG:1;			
		USHORT		BW40MAvailForA:1;			
		USHORT		Rsv1:1;					
		USHORT		AntDiversity:1;			
		USHORT		Rsv2:3;					
		USHORT		DACTestBit:1;			
	}	field;
	USHORT			word;
}	EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#endif




#ifdef RT_BIG_ENDIAN
typedef	union	_EEPROM_TX_PWR_STRUC	{
	struct	{
		CHAR	Byte1;				
		CHAR	Byte0;				
	}	field;
	USHORT	word;
}	EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#else
typedef	union	_EEPROM_TX_PWR_STRUC	{
	struct	{
		CHAR	Byte0;				
		CHAR	Byte1;				
	}	field;
	USHORT	word;
}	EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef	union	_EEPROM_VERSION_STRUC	{
	struct	{
		UCHAR	Version;			
		UCHAR	FaeReleaseNumber;	
	}	field;
	USHORT	word;
}	EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#else
typedef	union	_EEPROM_VERSION_STRUC	{
	struct	{
		UCHAR	FaeReleaseNumber;	
		UCHAR	Version;			
	}	field;
	USHORT	word;
}	EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef	union	_EEPROM_LED_STRUC	{
	struct	{
		USHORT	Rsvd:3;				
		USHORT	LedMode:5;			
		USHORT	PolarityGPIO_4:1;	
		USHORT	PolarityGPIO_3:1;	
		USHORT	PolarityGPIO_2:1;	
		USHORT	PolarityGPIO_1:1;	
		USHORT	PolarityGPIO_0:1;	
		USHORT	PolarityACT:1;		
		USHORT	PolarityRDY_A:1;		
		USHORT	PolarityRDY_G:1;		
	}	field;
	USHORT	word;
}	EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#else
typedef	union	_EEPROM_LED_STRUC	{
	struct	{
		USHORT	PolarityRDY_G:1;		
		USHORT	PolarityRDY_A:1;		
		USHORT	PolarityACT:1;		
		USHORT	PolarityGPIO_0:1;	
		USHORT	PolarityGPIO_1:1;	
		USHORT	PolarityGPIO_2:1;	
		USHORT	PolarityGPIO_3:1;	
		USHORT	PolarityGPIO_4:1;	
		USHORT	LedMode:5;			
		USHORT	Rsvd:3;				
	}	field;
	USHORT	word;
}	EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef	union	_EEPROM_TXPOWER_DELTA_STRUC	{
	struct	{
		UCHAR	TxPowerEnable:1;
		UCHAR	Type:1;			
		UCHAR	DeltaValue:6;	
	}	field;
	UCHAR	value;
}	EEPROM_TXPOWER_DELTA_STRUC, *PEEPROM_TXPOWER_DELTA_STRUC;
#else
typedef	union	_EEPROM_TXPOWER_DELTA_STRUC	{
	struct	{
		UCHAR	DeltaValue:6;	
		UCHAR	Type:1;			
		UCHAR	TxPowerEnable:1;
	}	field;
	UCHAR	value;
}	EEPROM_TXPOWER_DELTA_STRUC, *PEEPROM_TXPOWER_DELTA_STRUC;
#endif

#endif	
