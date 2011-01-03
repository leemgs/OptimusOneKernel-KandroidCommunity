

#include "../rt_config.h"


#ifdef RTMP_RF_RW_SUPPORT

NDIS_STATUS RT30xxWriteRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	UCHAR			value)
{
	RF_CSR_CFG_STRUC	rfcsr;
	UINT				i = 0;

	do
	{
		RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

		if (!rfcsr.field.RF_CSR_KICK)
			break;
		i++;
	}
	while ((i < RETRY_LIMIT) && (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)));

	if ((i == RETRY_LIMIT) || (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Retry count exhausted or device removed!!!\n"));
		return STATUS_UNSUCCESSFUL;
	}

	rfcsr.field.RF_CSR_WR = 1;
	rfcsr.field.RF_CSR_KICK = 1;
	rfcsr.field.TESTCSR_RFACC_REGNUM = regID;
	rfcsr.field.RF_CSR_DATA = value;

	RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);

	return NDIS_STATUS_SUCCESS;
}



NDIS_STATUS RT30xxReadRFRegister(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			regID,
	IN	PUCHAR			pValue)
{
	RF_CSR_CFG_STRUC	rfcsr;
	UINT				i=0, k=0;

	for (i=0; i<MAX_BUSY_COUNT; i++)
	{
		RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

		if (rfcsr.field.RF_CSR_KICK == BUSY)
		{
			continue;
		}
		rfcsr.word = 0;
		rfcsr.field.RF_CSR_WR = 0;
		rfcsr.field.RF_CSR_KICK = 1;
		rfcsr.field.TESTCSR_RFACC_REGNUM = regID;
		RTMP_IO_WRITE32(pAd, RF_CSR_CFG, rfcsr.word);
		for (k=0; k<MAX_BUSY_COUNT; k++)
		{
			RTMP_IO_READ32(pAd, RF_CSR_CFG, &rfcsr.word);

			if (rfcsr.field.RF_CSR_KICK == IDLE)
				break;
		}
		if ((rfcsr.field.RF_CSR_KICK == IDLE) &&
			(rfcsr.field.TESTCSR_RFACC_REGNUM == regID))
		{
			*pValue = (UCHAR)rfcsr.field.RF_CSR_DATA;
			break;
		}
	}
	if (rfcsr.field.RF_CSR_KICK == BUSY)
	{
		DBGPRINT_ERR(("RF read R%d=0x%x fail, i[%d], k[%d]\n", regID, rfcsr.word,i,k));
		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}


VOID NICInitRFRegisters(
	IN RTMP_ADAPTER *pAd)
{
	if (pAd->chipOps.AsicRfInit)
		pAd->chipOps.AsicRfInit(pAd);
}


VOID RtmpChipOpsRFHook(
	IN RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

	pChipOps->pRFRegTable = NULL;
	pChipOps->AsicRfInit = NULL;
	pChipOps->AsicRfTurnOn = NULL;
	pChipOps->AsicRfTurnOff = NULL;
	pChipOps->AsicReverseRfFromSleepMode = NULL;
	pChipOps->AsicHaltAction = NULL;
#ifdef RT33xx
if (IS_RT3390(pAd) && (pAd->infType == RTMP_DEV_INF_PCI))
		{
			pChipOps->pRFRegTable = RFRegTableOverRT3390;
			pChipOps->AsicHaltAction = RT33xxHaltAction;
			pChipOps->AsicRfTurnOff = RT33xxLoadRFSleepModeSetup;
			pChipOps->AsicRfInit = NICInitRT3390RFRegisters;
			pChipOps->AsicReverseRfFromSleepMode = RT33xxReverseRFSleepModeSetup;
		}
#else 
	

#ifdef RT30xx
	if (IS_RT30xx(pAd))
	{
		pChipOps->pRFRegTable = RT30xx_RFRegTable;
		pChipOps->AsicHaltAction = RT30xxHaltAction;
#ifdef RT3090
		if (IS_RT3090(pAd) && (pAd->infType == RTMP_DEV_INF_PCI))
		{
			pChipOps->AsicRfTurnOff = RT30xxLoadRFSleepModeSetup;
			pChipOps->AsicRfInit = NICInitRT3090RFRegisters;
			pChipOps->AsicReverseRfFromSleepMode = RT30xxReverseRFSleepModeSetup;
		}
#endif 
	}
#endif 
#endif 
}

#endif 
