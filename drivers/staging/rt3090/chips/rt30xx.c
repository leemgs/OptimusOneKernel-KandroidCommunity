


#ifdef RT30xx


#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif 

#include "../rt_config.h"





REG_PAIR   RT30xx_RFRegTable[] = {
        {RF_R04,          0x40},
        {RF_R05,          0x03},
        {RF_R06,          0x02},
        {RF_R07,          0x70},
        {RF_R09,          0x0F},
        {RF_R10,          0x41},
        {RF_R11,          0x21},
        {RF_R12,          0x7B},
        {RF_R14,          0x90},
        {RF_R15,          0x58},
        {RF_R16,          0xB3},
        {RF_R17,          0x92},
        {RF_R18,          0x2C},
        {RF_R19,          0x02},
        {RF_R20,          0xBA},
        {RF_R21,          0xDB},
        {RF_R24,          0x16},
        {RF_R25,          0x01},
        {RF_R29,          0x1F},
};

UCHAR NUM_RF_REG_PARMS = (sizeof(RT30xx_RFRegTable) / sizeof(REG_PAIR));









VOID RT30xxSetRxAnt(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ant)
{
	UINT32	Value;
	UINT32	x;

	if ((pAd->EepromAccess) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))	||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF)) ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		return;
	}

	
	if (Ant == 0)
	{
		
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
		x |= (EESK);
		RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);
#else
		AsicSendCommandToMcu(pAd, 0x73, 0xFF, 0x1, 0x0);
#endif 

		RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
		Value &= ~(0x0808);
		RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to main antenna\n"));
	}
	else
	{
		
#ifdef RTMP_MAC_PCI
		RTMP_IO_READ32(pAd, E2PROM_CSR, &x);
		x &= ~(EESK);
		RTMP_IO_WRITE32(pAd, E2PROM_CSR, x);
#else
		AsicSendCommandToMcu(pAd, 0x73, 0xFF, 0x0, 0x0);
#endif 
		RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &Value);
		Value &= ~(0x0808);
		Value |= 0x08;
		RTMP_IO_WRITE32(pAd, GPIO_CTRL_CFG, Value);
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("AsicSetRxAnt, switch to aux antenna\n"));
	}
}



VOID RTMPFilterCalibration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR	R55x = 0, value, FilterTarget = 0x1E, BBPValue=0;
	UINT	loop = 0, count = 0, loopcnt = 0, ReTry = 0;
	UCHAR	RF_R24_Value = 0;

	
	pAd->Mlme.CaliBW20RfR24 = 0x1F;
	pAd->Mlme.CaliBW40RfR24 = 0x2F; 

	do
	{
		if (loop == 1)	
		{
			
			RF_R24_Value = 0x27;
			RT30xxWriteRFRegister(pAd, RF_R24, RF_R24_Value);
			if (IS_RT3090(pAd) || IS_RT3572(pAd)|| IS_RT3390(pAd))
				FilterTarget = 0x15;
			else
				FilterTarget = 0x19;

			
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue&= (~0x18);
			BBPValue|= (0x10);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);

			
			RT30xxReadRFRegister(pAd, RF_R31, &value);
			value |= 0x20;
			RT30xxWriteRFRegister(pAd, RF_R31, value);
		}
		else			
		{
			
			RF_R24_Value = 0x07;
			RT30xxWriteRFRegister(pAd, RF_R24, RF_R24_Value);
			if (IS_RT3090(pAd) || IS_RT3572(pAd)|| IS_RT3390(pAd))
				FilterTarget = 0x13;
			else
				FilterTarget = 0x16;

			
			RT30xxReadRFRegister(pAd, RF_R31, &value);
			value &= (~0x20);
			RT30xxWriteRFRegister(pAd, RF_R31, value);
		}

		
		RT30xxReadRFRegister(pAd, RF_R22, &value);
		value |= 0x01;
		RT30xxWriteRFRegister(pAd, RF_R22, value);

		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0);

		do
		{
			
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, 0x90);

			RTMPusecDelay(1000);
			
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R55, &value);
			R55x = value & 0xFF;

		} while ((ReTry++ < 100) && (R55x == 0));

		
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x06);

		while(TRUE)
		{
			
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R25, 0x90);

			
			RTMPusecDelay(1000);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R55, &value);
			value &= 0xFF;
			if ((R55x - value) < FilterTarget)
			{
				RF_R24_Value ++;
			}
			else if ((R55x - value) == FilterTarget)
			{
				RF_R24_Value ++;
				count ++;
			}
			else
			{
				break;
			}

			
			if (loopcnt++ > 100)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("RTMPFilterCalibration - can't find a valid value, loopcnt=%d stop calibrating", loopcnt));
				break;
			}

			
			RT30xxWriteRFRegister(pAd, RF_R24, RF_R24_Value);
		}

		if (count > 0)
		{
			RF_R24_Value = RF_R24_Value - ((count) ? (1) : (0));
		}

		
		if (loopcnt < 100)
		{
			if (loop++ == 0)
			{
				
				pAd->Mlme.CaliBW20RfR24 = (UCHAR)RF_R24_Value;
			}
			else
			{
				
				pAd->Mlme.CaliBW40RfR24 = (UCHAR)RF_R24_Value;
				break;
			}
		}
		else
			break;

		RT30xxWriteRFRegister(pAd, RF_R24, RF_R24_Value);

		
		count = 0;
	} while(TRUE);

	
	
	
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0);

	RT30xxReadRFRegister(pAd, RF_R22, &value);
	value &= ~(0x01);
	RT30xxWriteRFRegister(pAd, RF_R22, value);

	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
	BBPValue&= (~0x18);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPFilterCalibration - CaliBW20RfR24=0x%x, CaliBW40RfR24=0x%x\n", pAd->Mlme.CaliBW20RfR24, pAd->Mlme.CaliBW40RfR24));
}




VOID RT30xxLoadRFNormalModeSetup(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR RFValue;

	
	RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
	RFValue = (RFValue & (~0x0C)) | 0x31;
	RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R15, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R15, RFValue);

	

	
	RT30xxReadRFRegister(pAd, RF_R20, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R20, RFValue);

	
	RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
	RFValue &= (~0x08);
	RT30xxWriteRFRegister(pAd, RF_R21, RFValue);

	
	
	RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
	
	
	if (IS_RT3090(pAd))	
	{
		if ((pAd->MACVersion & 0xffff) < 0x0211)
			RFValue = (RFValue & (~0x77)) | 0x3;
		else
			RFValue = (RFValue & (~0x77));
		RT30xxWriteRFRegister(pAd, RF_R27, RFValue);
	}
	
}


VOID RT30xxLoadRFSleepModeSetup(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR RFValue;
	UINT32 MACValue;


	{
		
		RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
		RFValue &= (~0x01);
		RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

		
		RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
		RFValue &= (~0x30);
		RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

		
		RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
		RFValue &= (~0x0E);
		RT30xxWriteRFRegister(pAd, RF_R09, RFValue);

		
		RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
		RFValue &= (~0x80);
		RT30xxWriteRFRegister(pAd, RF_R21, RFValue);
	}

	if (IS_RT3090(pAd) ||	
		IS_RT3572(pAd) ||
		(IS_RT3070(pAd) && ((pAd->MACVersion & 0xffff) < 0x0201)))
	{
		{
			RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
			RFValue |= 0x77;
			RT30xxWriteRFRegister(pAd, RF_R27, RFValue);
		}

		RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
		MACValue |= 0x1D000000;
		RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
	}
}


VOID RT30xxReverseRFSleepModeSetup(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR RFValue;
	UINT32 MACValue;

	{
		
		RT30xxReadRFRegister(pAd, RF_R01, &RFValue);
		RFValue |= 0x01;
		RT30xxWriteRFRegister(pAd, RF_R01, RFValue);

		
		RT30xxReadRFRegister(pAd, RF_R07, &RFValue);
		RFValue |= 0x30;
		RT30xxWriteRFRegister(pAd, RF_R07, RFValue);

		
		RT30xxReadRFRegister(pAd, RF_R09, &RFValue);
		RFValue |= 0x0E;
		RT30xxWriteRFRegister(pAd, RF_R09, RFValue);

		
		RT30xxReadRFRegister(pAd, RF_R21, &RFValue);
		RFValue |= 0x80;
		RT30xxWriteRFRegister(pAd, RF_R21, RFValue);
	}

	if (IS_RT3090(pAd) ||	
		IS_RT3572(pAd) ||
		IS_RT3390(pAd) ||
		(IS_RT3070(pAd) && ((pAd->MACVersion & 0xffff) < 0x0201)))
	{
		{
			RT30xxReadRFRegister(pAd, RF_R27, &RFValue);
			if ((pAd->MACVersion & 0xffff) < 0x0211)
				RFValue = (RFValue & (~0x77)) | 0x3;
			else
				RFValue = (RFValue & (~0x77));
			RT30xxWriteRFRegister(pAd, RF_R27, RFValue);
		}

		
		if ((pAd->NicConfig2.field.DACTestBit == 1) && ((pAd->MACVersion & 0xffff) < 0x0211))
		{
			
			RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
			MACValue = ((MACValue & 0xE0FFFFFF) | 0x0D000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
		}
		else
		{
			RTMP_IO_READ32(pAd, LDO_CFG0, &MACValue);
			MACValue = ((MACValue & 0xE0FFFFFF) | 0x01000000);
			RTMP_IO_WRITE32(pAd, LDO_CFG0, MACValue);
		}
	}

	if(IS_RT3572(pAd))
		RT30xxWriteRFRegister(pAd, RF_R08, 0x80);
}


VOID RT30xxHaltAction(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32		TxPinCfg = 0x00050F0F;

	
	
	
	if (IS_RT3070(pAd) || IS_RT3071(pAd) || IS_RT3572(pAd))
	{
		if ((IS_RT3071(pAd) || IS_RT3572(pAd))
#ifdef RTMP_EFUSE_SUPPORT
			&& (pAd->bUseEfuse)
#endif 
			)
		{
			TxPinCfg &= 0xFFFBF0F0; 
		}
		else
		{
			TxPinCfg &= 0xFFFFF0F0;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}
}

#endif 
