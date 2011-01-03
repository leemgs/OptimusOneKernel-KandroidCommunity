

#ifdef RT3390

#include "../rt_config.h"


#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif 


VOID NICInitRT3390RFRegisters(IN PRTMP_ADAPTER pAd)
{
		INT i;
	
	
	if (IS_RT3090(pAd)||IS_RT3390(pAd)||IS_RT3572(pAd))
	{
		
		
		UINT32 RfReg = 0, data;

		RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RfReg);
		RfReg |= 0x80;
		RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);
		RTMPusecDelay(1000);
		RfReg &= 0x7F;
		RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);

		
		RT30xxWriteRFRegister(pAd, RF_R24, 0x0F);
		RT30xxWriteRFRegister(pAd, RF_R31, 0x0F);

		if (IS_RT3390(pAd))
		{
			
			RTMP_IO_READ32(pAd, GPIO_SWITCH, &data);
			data &= ~(0x20);
			RTMP_IO_WRITE32(pAd, GPIO_SWITCH, data);

			
			for (i = 0; i < NUM_RF_REG_PARMS_OVER_RT3390; i++)
			{
				RT30xxWriteRFRegister(pAd, RFRegTableOverRT3390[i].Register, RFRegTableOverRT3390[i].Value);
			}
		}

		
		RTMP_IO_READ32(pAd, GPIO_SWITCH, &data);
		data &= ~(0x20);
		RTMP_IO_WRITE32(pAd, GPIO_SWITCH, data);

		
		for (i = 0; i < NUM_RF_REG_PARMS_OVER_RT3390; i++)
		{
			RT30xxWriteRFRegister(pAd, RFRegTableOverRT3390[i].Register, RFRegTableOverRT3390[i].Value);
		}

		
		RT30xxReadRFRegister(pAd, RF_R06, (PUCHAR)&RfReg);
		RfReg |= 0x40;
		RT30xxWriteRFRegister(pAd, RF_R06, (UCHAR)RfReg);

		
		RTMPFilterCalibration(pAd);

		
		if ((pAd->MACVersion & 0xffff) < 0x0211)
			RT30xxWriteRFRegister(pAd, RF_R27, 0x3);

		
		RTMP_IO_READ32(pAd, OPT_14, &data);
		data |= 0x01;
		RTMP_IO_WRITE32(pAd, OPT_14, data);

		
		if (pAd->RfIcType == RFIC_3020)
			AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);

		
		RT33xxLoadRFNormalModeSetup(pAd);
	}

}

#endif 
