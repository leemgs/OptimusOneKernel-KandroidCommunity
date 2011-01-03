

#include "../rt_config.h"



static inline VOID RaiseClock(
    IN	PRTMP_ADAPTER	pAd,
    IN  UINT32 *x)
{
	*x = *x | EESK;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, *x);
	RTMPusecDelay(1);				
}


static inline VOID LowerClock(
    IN	PRTMP_ADAPTER	pAd,
    IN  UINT32 *x)
{
	*x = *x & ~EESK;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, *x);
	RTMPusecDelay(1);
}


static inline USHORT ShiftInBits(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32		x,i;
	USHORT      data=0;

	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

	x &= ~( EEDO | EEDI);

	for(i=0; i<16; i++)
	{
		data = data << 1;
		RaiseClock(pAd, &x);

		RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
		LowerClock(pAd, &x); 

		x &= ~(EEDI);
		if(x & EEDO)
		    data |= 1;
	}

	return data;
}



static inline VOID ShiftOutBits(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT			data,
	IN USHORT			count)
{
	UINT32       x,mask;

	mask = 0x01 << (count - 1);
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

	x &= ~(EEDO | EEDI);

	do
	{
	    x &= ~EEDI;
	    if(data & mask)		x |= EEDI;

	    RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	    RaiseClock(pAd, &x);
	    LowerClock(pAd, &x);

	    mask = mask >> 1;
	} while(mask);

	x &= ~EEDI;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);
}



static inline VOID EEpromCleanup(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32 x;

	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

	x &= ~(EECS | EEDI);
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);
}


static inline VOID EWEN(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32	x;

	
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);

	
	ShiftOutBits(pAd, EEPROM_EWEN_OPCODE, 5);
	ShiftOutBits(pAd, 0, 6);

	EEpromCleanup(pAd);
}


static inline VOID EWDS(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32	x;

	
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	
	RaiseClock(pAd, &x);
	LowerClock(pAd, &x);

	
	ShiftOutBits(pAd, EEPROM_EWDS_OPCODE, 5);
	ShiftOutBits(pAd, 0, 6);

	EEpromCleanup(pAd);
}



int rtmp_ee_prom_read16(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT			Offset,
	OUT USHORT			*pValue)
{
	UINT32		x;
	USHORT		data;

#ifdef RT30xx
#ifdef ANT_DIVERSITY_SUPPORT
	if (pAd->NicConfig2.field.AntDiversity)
	{
		pAd->EepromAccess = TRUE;
	}
#endif 
#endif 

	Offset /= 2;
	
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	
	if (!(IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)))
	{
		
		RaiseClock(pAd, &x);
		LowerClock(pAd, &x);
	}

	
	ShiftOutBits(pAd, EEPROM_READ_OPCODE, 3);
	ShiftOutBits(pAd, Offset, pAd->EEPROMAddressNum);

	
	data = ShiftInBits(pAd);

	EEpromCleanup(pAd);

#ifdef RT30xx
#ifdef ANT_DIVERSITY_SUPPORT
	
	
	
	if ((pAd->NicConfig2.field.AntDiversity))
	{
		pAd->EepromAccess = FALSE;
		AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);
	}
#endif 
#endif 

	*pValue = data;

	return NDIS_STATUS_SUCCESS;
}


int rtmp_ee_prom_write16(
    IN  PRTMP_ADAPTER	pAd,
    IN  USHORT Offset,
    IN  USHORT Data)
{
	UINT32 x;

#ifdef RT30xx
#ifdef ANT_DIVERSITY_SUPPORT
	if (pAd->NicConfig2.field.AntDiversity)
	{
		pAd->EepromAccess = TRUE;
	}
#endif 
#endif 

	Offset /= 2;

	EWEN(pAd);

	
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
	x &= ~(EEDI | EEDO | EESK);
	x |= EECS;
	RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);

	
	if (!(IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd)))
	{
		
		RaiseClock(pAd, &x);
		LowerClock(pAd, &x);
	}

	
	ShiftOutBits(pAd, EEPROM_WRITE_OPCODE, 3);
	ShiftOutBits(pAd, Offset, pAd->EEPROMAddressNum);
	ShiftOutBits(pAd, Data, 16);		

	
	RTMP_IO_READ32(pAd, E2PROM_CSR, &x);

	EEpromCleanup(pAd);

	RTMPusecDelay(10000);	

	EWDS(pAd);

	EEpromCleanup(pAd);

#ifdef RT30xx
#ifdef ANT_DIVERSITY_SUPPORT
	
	
	
	if ((pAd->NicConfig2.field.AntDiversity) )
	{
		pAd->EepromAccess = FALSE;
		AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);
	}
#endif 
#endif 

	return NDIS_STATUS_SUCCESS;

}
