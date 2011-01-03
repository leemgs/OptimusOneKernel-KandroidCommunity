
#ifndef __EEPROM_H__
#define __EEPROM_H__



#ifdef RTMP_PCI_SUPPORT

int rtmp_ee_prom_read16(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT			Offset,
	OUT USHORT			*pValue);

int rtmp_ee_prom_write16(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT			Offset,
	IN USHORT			value);
#endif 





#ifdef RT30xx
#ifdef RTMP_EFUSE_SUPPORT
int rtmp_ee_efuse_read16(
	IN RTMP_ADAPTER *pAd,
	IN USHORT Offset,
	OUT USHORT *pValue);

int rtmp_ee_efuse_write16(
	IN RTMP_ADAPTER *pAd,
	IN USHORT Offset,
	IN USHORT data);
#endif 
#endif 


INT RtmpChipOpsEepromHook(
	IN RTMP_ADAPTER *pAd,
	IN INT			infType);

#endif 
