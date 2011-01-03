

#include "rt_config.h"

#ifdef RALINK_ATE

#ifdef RT30xx
#define ATE_BBP_REG_NUM	168
UCHAR restore_BBP[ATE_BBP_REG_NUM]={0};
#endif 


UCHAR TemplateFrame[24] = {0x08,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0x00,0xAA,0xBB,0x12,0x34,0x56,0x00,0x11,0x22,0xAA,0xBB,0xCC,0x00,0x00};

extern RTMP_RF_REGS RF2850RegTable[];
extern UCHAR NUM_OF_2850_CHNL;

extern FREQUENCY_ITEM FreqItems3020[];
extern UCHAR NUM_OF_3020_CHNL;




static CHAR CCKRateTable[] = {0, 1, 2, 3, 8, 9, 10, 11, -1}; 
static CHAR OFDMRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, -1}; 
static CHAR HTMIXRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, -1}; 

static INT TxDmaBusy(
	IN PRTMP_ADAPTER pAd);

static INT RxDmaBusy(
	IN PRTMP_ADAPTER pAd);

static VOID RtmpDmaEnable(
	IN PRTMP_ADAPTER pAd,
	IN INT Enable);

static VOID BbpSoftReset(
	IN PRTMP_ADAPTER pAd);

static VOID RtmpRfIoWrite(
	IN PRTMP_ADAPTER pAd);

static INT ATESetUpFrame(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 TxIdx);

static INT ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index);

static INT ATECmdHandler(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

#ifndef RT30xx
static int CheckMCSValid(
	IN UCHAR Mode,
	IN UCHAR Mcs);
#endif 

#ifdef RT30xx
static int CheckMCSValid(
	IN UCHAR Mode,
	IN UCHAR Mcs,
	IN BOOLEAN bRT2070);
#endif 

#ifdef RTMP_MAC_PCI
static VOID ATEWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC	pOutTxWI,
	IN	BOOLEAN			FRAG,
	IN	BOOLEAN			CFACK,
	IN	BOOLEAN			InsTimestamp,
	IN	BOOLEAN			AMPDU,
	IN	BOOLEAN			Ack,
	IN	BOOLEAN			NSeq,		
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN	UCHAR			PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,
	IN	BOOLEAN			CfAck,
	IN	HTTRANSMIT_SETTING	*pTransmit);
#endif 


static VOID SetJapanFilter(
	IN	PRTMP_ADAPTER	pAd);


#ifdef RALINK_28xx_QA
static inline INT	DO_RACFG_CMD_ATE_START(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_STOP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_RF_WRITE_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_E2PROM_READ16(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_E2PROM_WRITE16(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_E2PROM_READ_ALL
(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_E2PROM_WRITE_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_IO_READ(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_IO_WRITE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_IO_READ_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_BBP_READ8(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_BBP_WRITE8(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_BBP_READ_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_GET_NOISE_LEVEL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_GET_COUNTER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_CLEAR_COUNTER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_TX_START(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_GET_TX_STATUS(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_TX_STOP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_RX_START(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_RX_STOP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_RX_STOP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_START_TX_CARRIER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_START_TX_CONT(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_START_TX_FRAME(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_BW(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_TX_POWER0(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_TX_POWER1(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_FREQ_OFFSET(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_GET_STATISTICS(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_RESET_COUNTER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SEL_TX_ANTENNA(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SEL_RX_ANTENNA(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_PREAMBLE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_CHANNEL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_ADDR1(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_ADDR2(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_ADDR3(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_RATE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_TX_FRAME_LEN(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_SET_TX_FRAME_COUNT(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_START_RX_FRAME(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_E2PROM_READ_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_E2PROM_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_IO_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_BBP_READ_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

static inline INT DO_RACFG_CMD_ATE_BBP_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN struct ate_racfghdr *pRaCfg
);

#endif 


#ifdef RTMP_MAC_PCI
static INT TxDmaBusy(
	IN PRTMP_ADAPTER pAd)
{
	INT result;
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	
	if (GloCfg.field.TxDMABusy)
		result = 1;
	else
		result = 0;

	return result;
}


static INT RxDmaBusy(
	IN PRTMP_ADAPTER pAd)
{
	INT result;
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	
	if (GloCfg.field.RxDMABusy)
		result = 1;
	else
		result = 0;

	return result;
}


static VOID RtmpDmaEnable(
	IN PRTMP_ADAPTER pAd,
	IN INT Enable)
{
	BOOLEAN value;
	ULONG WaitCnt;
	WPDMA_GLO_CFG_STRUC GloCfg;

	value = Enable > 0 ? 1 : 0;

	
	WaitCnt = 0;

	while (TxDmaBusy(pAd) || RxDmaBusy(pAd))
	{
		RTMPusecDelay(10);
		if (WaitCnt++ > 100)
			break;
	}

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	
	GloCfg.field.EnableTxDMA = value;
	GloCfg.field.EnableRxDMA = value;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);	
	RTMPusecDelay(5000);

	return;
}
#endif 




static VOID BbpSoftReset(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR BbpData = 0;

	
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &BbpData);
	BbpData |= 0x00000001; 
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BbpData);

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &BbpData);
	BbpData &= ~(0x00000001); 
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BbpData);

	return;
}


static VOID RtmpRfIoWrite(
	IN PRTMP_ADAPTER pAd)
{
	
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	RTMPusecDelay(200);

	
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 | 0x04));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	RTMPusecDelay(200);

	
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAd, (pAd->LatchRfRegs.R3 & (~0x04)));
	RTMP_RF_IO_WRITE32(pAd, pAd->LatchRfRegs.R4);

	return;
}


#ifdef RT30xx
static int CheckMCSValid(
	UCHAR Mode,
	UCHAR Mcs,
	BOOLEAN bRT2070)
#endif 
#ifndef RT30xx
static int CheckMCSValid(
	IN UCHAR Mode,
	IN UCHAR Mcs)
#endif 
{
	INT i;
	PCHAR pRateTab;

	switch (Mode)
	{
		case 0:
			pRateTab = CCKRateTable;
			break;
		case 1:
			pRateTab = OFDMRateTable;
			break;
		case 2:
		case 3:
#ifdef RT30xx
			if (bRT2070)
				pRateTab = OFDMRateTable;
			else
#endif 
			pRateTab = HTMIXRateTable;
			break;
		default:
			ATEDBGPRINT(RT_DEBUG_ERROR, ("unrecognizable Tx Mode %d\n", Mode));
			return -1;
			break;
	}

	i = 0;
	while (pRateTab[i] != -1)
	{
		if (pRateTab[i] == Mcs)
			return 0;
		i++;
	}

	return -1;
}


static INT ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	ULONG R;
	CHAR TxPower;
	UCHAR Bbp94 = 0;
	BOOLEAN bPowerReduce = FALSE;
#ifdef RTMP_RF_RW_SUPPORT
	UCHAR RFValue;
#endif 
#ifdef RALINK_28xx_QA
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		
		return 0;
	}
	else
#endif 
	{
		TxPower = index == 0 ? pAd->ate.TxPower0 : pAd->ate.TxPower1;

		if (pAd->ate.Channel <= 14)
		{
			if (TxPower > 31)
			{

				
				R = 31;
				if (TxPower <= 36)
					Bbp94 = BBPR94_DEFAULT + (UCHAR)(TxPower - 31);
			}
			else if (TxPower < 0)
			{

				
				R = 0;
				if (TxPower >= -6)
					Bbp94 = BBPR94_DEFAULT + TxPower;
			}
			else
			{
				
				R = (ULONG) TxPower;
				Bbp94 = BBPR94_DEFAULT;
			}

			ATEDBGPRINT(RT_DEBUG_TRACE, ("%s (TxPower=%d, R=%ld, BBP_R94=%d)\n", __FUNCTION__, TxPower, R, Bbp94));
		}
		else 
		{
			if (TxPower > 15)
			{

				
				R = 15;
			}
			else if (TxPower < 0)
			{

				
				
				ASSERT((TxPower >= -7));
				R = (ULONG)(TxPower + 7);
				bPowerReduce = TRUE;
			}
			else
			{
				
				R = (ULONG) TxPower;
			}

			ATEDBGPRINT(RT_DEBUG_TRACE, ("%s (TxPower=%d, R=%lu)\n", __FUNCTION__, TxPower, R));
		}

#ifdef RTMP_RF_RW_SUPPORT
		if (IS_RT30xx(pAd))
		{
			
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);
			RFValue = (RFValue & 0xE0) | TxPower;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);
			ATEDBGPRINT(RT_DEBUG_TRACE, ("3070 or 2070:%s (TxPower=%d, RFValue=%x)\n", __FUNCTION__, TxPower, RFValue));
		}
		else
#endif 
		{
		if (pAd->ate.Channel <= 14)
		{
			if (index == 0)
			{
				
				R = R << 9;
				R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);
				pAd->LatchRfRegs.R3 = R;
			}
			else
			{
				
				R = R << 6;
				R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);
				pAd->LatchRfRegs.R4 = R;
			}
		}
		else 
		{
			if (bPowerReduce == FALSE)
			{
				if (index == 0)
				{
					
					R = (R << 10) | (1 << 9);
					R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);
					pAd->LatchRfRegs.R3 = R;
				}
				else
				{
					
					R = (R << 7) | (1 << 6);
					R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);
					pAd->LatchRfRegs.R4 = R;
				}
			}
			else
			{
				if (index == 0)
				{
					
					R = (R << 10);
					R |= (pAd->LatchRfRegs.R3 & 0xffffc1ff);

					
					pAd->LatchRfRegs.R3 = (R & (~(1 << 9)));
				}
				else
				{
					
					R = (R << 7);
					R |= (pAd->LatchRfRegs.R4 & 0xfffff83f);

					
					pAd->LatchRfRegs.R4 = (R & (~(1 << 6)));
				}
			}
		}
		RtmpRfIoWrite(pAd);
	}


		return 0;
	}
}



#ifdef RTMP_MAC_PCI
static INT	ATECmdHandler(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UINT32			Value = 0;
	UCHAR			BbpData;
	UINT32			MacData = 0;
	PTXD_STRUC		pTxD;
	INT				index;
	UINT			i = 0, atemode = 0;
	PRXD_STRUC		pRxD;
	PRTMP_TX_RING	pTxRing = &pAd->TxRing[QID_AC_BE];
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
#ifdef	RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
    TXD_STRUC       TxD;
#endif

	ATEDBGPRINT(RT_DEBUG_TRACE, ("===> ATECmdHandler()\n"));

	ATEAsicSwitchChannel(pAd);

	
	AsicLockChannel(pAd, pAd->ate.Channel);

	RTMPusecDelay(5000);

	
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);

	
	BbpData = 0;

	
	MacData &= 0xFFFFFFEF;

	
	if (!strcmp(arg, "ATESTART"))
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("ATE: ATESTART\n"));

#if defined(LINUX) || defined(VXWORKS)
		
		if (!(ATE_ON(pAd)))
		{
			NICEraseFirmware(pAd);
		}
#endif 

		atemode = pAd->ate.Mode;
		pAd->ate.Mode = ATE_START;
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

		if (atemode == ATE_TXCARR)
		{
			
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= 0xFFFFFF00; 
		    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
		}
		else if (atemode == ATE_TXCARRSUPP)
		{
			
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= ~(1 << 7); 
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

			
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R24, &BbpData);
			BbpData &= 0xFFFFFFFE; 
		    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, BbpData);
		}

		
		else if ((atemode & ATE_TXFRAME) || (atemode == ATE_STOP))
		{
			PRTMP_TX_RING pTxRing = &pAd->TxRing[QID_AC_BE];

			if (atemode == ATE_TXCONT)
			{
				
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
				BbpData &= ~(1 << 7); 
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
			}

			
			RtmpDmaEnable(pAd, 0);
			for (i=0; i<TX_RING_SIZE; i++)
			{
				PNDIS_PACKET  pPacket;

#ifndef RT_BIG_ENDIAN
			    pTxD = (PTXD_STRUC)pAd->TxRing[QID_AC_BE].Cell[i].AllocVa;
#else
			pDestTxD = (PTXD_STRUC)pAd->TxRing[QID_AC_BE].Cell[i].AllocVa;
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				pTxD->DMADONE = 0;
				pPacket = pTxRing->Cell[i].pNdisPacket;

				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
				}

				
				pTxRing->Cell[i].pNdisPacket = NULL;

				pPacket = pTxRing->Cell[i].pNextNdisPacket;

				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
				}

				
				pTxRing->Cell[i].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif
			}

			
			RtmpDmaEnable(pAd, 1);
		}

		
		pAd->ate.LastSNR0 = 0;
		pAd->ate.LastSNR1 = 0;
		pAd->ate.LastRssi0 = 0;
		pAd->ate.LastRssi1 = 0;
		pAd->ate.LastRssi2 = 0;
		pAd->ate.AvgRssi0 = 0;
		pAd->ate.AvgRssi1 = 0;
		pAd->ate.AvgRssi2 = 0;
		pAd->ate.AvgRssi0X8 = 0;
		pAd->ate.AvgRssi1X8 = 0;
		pAd->ate.AvgRssi2X8 = 0;
		pAd->ate.NumOfAvgRssiSample = 0;

#ifdef RALINK_28xx_QA
		
		pAd->ate.bQATxStart = FALSE;
		pAd->ate.bQARxStart = FALSE;
		pAd->ate.seq = 0;

		
		pAd->ate.U2M = 0;
		pAd->ate.OtherData = 0;
		pAd->ate.Beacon = 0;
		pAd->ate.OtherCount = 0;
		pAd->ate.TxAc0 = 0;
		pAd->ate.TxAc1 = 0;
		pAd->ate.TxAc2 = 0;
		pAd->ate.TxAc3 = 0;
		
		pAd->ate.TxMgmt = 0;
		pAd->ate.RSSI0 = 0;
		pAd->ate.RSSI1 = 0;
		pAd->ate.RSSI2 = 0;
		pAd->ate.SNR0 = 0;
		pAd->ate.SNR1 = 0;

		
		pAd->ate.TxDoneCount = 0;
		
		pAd->ate.TxStatus = 0;
#endif 

		
		BbpSoftReset(pAd);


#ifdef CONFIG_STA_SUPPORT
		



#if defined(LINUX) || defined(VXWORKS)
		RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#endif 

		
		ATEDisableAsicProtect(pAd);
		RTMPStationStop(pAd);
#endif 

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value &= ~(1 << 2);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value &= ~(1 << 3);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
	}
	else if (!strcmp(arg, "ATESTOP"))
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("ATE: ATESTOP\n"));

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

		
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value &= (0xfffffff3);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

		
		RtmpDmaEnable(pAd, 0);

#ifdef LINUX
		pAd->ate.bFWLoading = TRUE;

		Status = NICLoadFirmware(pAd);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			ATEDBGPRINT(RT_DEBUG_ERROR, ("NICLoadFirmware failed, Status[=0x%08x]\n", Status));
			return FALSE;
		}
#endif 
		pAd->ate.Mode = ATE_STOP;

		
		BbpSoftReset(pAd);

		RTMP_ASIC_INTERRUPT_DISABLE(pAd);

		NICInitializeAdapter(pAd, TRUE);

		
		for (index = 0; index < RX_RING_SIZE; index++)
		{
			pRxD = (PRXD_STRUC) pAd->RxRing.Cell[index].AllocVa;
			pRxD->DDONE = 0;
		}

		
		NICReadEEPROMParameters(pAd, NULL);
		NICInitAsicFromEEPROM(pAd);

		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);

		
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);

		
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, 0xffffffff);
		
		RTMP_ASIC_INTERRUPT_ENABLE(pAd);

		

#ifdef CONFIG_STA_SUPPORT
		
		RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, STANORMAL);
#endif 

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value |= (1 << 2);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

		
		RtmpDmaEnable(pAd, 1);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value |= (1 << 3);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);


#ifdef CONFIG_STA_SUPPORT
		RTMPStationStart(pAd);
#endif 

#if defined(LINUX) || defined(VXWORKS)
		RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#endif 
	}
	else if (!strcmp(arg, "TXCARR"))
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("ATE: TXCARR\n"));
		pAd->ate.Mode = ATE_TXCARR;

		
		if (pAd->ate.bQATxStart == FALSE)
		{
			
			BbpSoftReset(pAd);

			
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= 0xFFFFFF00; 
			BbpData |= 0x000000C1; 
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

			
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value = Value | 0x00000010;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
		}
	}
	else if (!strcmp(arg, "TXCONT"))
	{
		if (pAd->ate.bQATxStart == TRUE)
		{
			
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
			MacData &= 0xFFFFFFEB;
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

			
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= 0xFFFFFF7F; 
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
		}

		
		ATEDBGPRINT(RT_DEBUG_TRACE, ("ATE: TXCONT\n"));

		
		pAd->ate.Mode = ATE_TXCONT;
		pAd->ate.TxCount = 50;

		


		
		BbpSoftReset(pAd);

		
		RtmpDmaEnable(pAd, 0);

		
		{
			RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * 0x10,  &pTxRing->TxDmaIdx);
			pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
			pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
			RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * 0x10, pTxRing->TxCpuIdx);
		}

		pAd->ate.TxDoneCount = 0;

		
		SetJapanFilter(pAd);

		for (i = 0; (i < TX_RING_SIZE-1) && (i < pAd->ate.TxCount); i++)
		{
			PNDIS_PACKET pPacket;
			UINT32 TxIdx = pTxRing->TxCpuIdx;

#ifndef RT_BIG_ENDIAN
			pTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
#else
			pDestTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
			
			pPacket = pTxRing->Cell[TxIdx].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			
			pTxRing->Cell[TxIdx].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[TxIdx].pNextNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			
			pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

			if (ATESetUpFrame(pAd, TxIdx) != 0)
				break;

			INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);
		}

		
		ATESetUpFrame(pAd, pTxRing->TxCpuIdx);

		
		RtmpDmaEnable(pAd, 1);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value |= (1 << 2);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value &= ~(1 << 3);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

#ifdef RALINK_28xx_QA
		if (pAd->ate.bQATxStart == TRUE)
		{
			pAd->ate.TxStatus = 1;
		}
#endif 

		
		RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pAd->TxRing[QID_AC_BE].TxCpuIdx);

		RTMPusecDelay(5000);


		
		
		RtmpDmaEnable(pAd, 0);

		
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
		BbpData |= 0x00000080; 
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

		pAd->ate.TxCount = 50;

		
		{
			RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * 0x10,  &pTxRing->TxDmaIdx);
			pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
			pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
			RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * 0x10, pTxRing->TxCpuIdx);
		}

		pAd->ate.TxDoneCount = 0;

		SetJapanFilter(pAd);

		for (i = 0; (i < TX_RING_SIZE-1) && (i < pAd->ate.TxCount); i++)
		{
			PNDIS_PACKET pPacket;
			UINT32 TxIdx = pTxRing->TxCpuIdx;

#ifndef RT_BIG_ENDIAN
			pTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
#else
			pDestTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
			
			pPacket = pTxRing->Cell[TxIdx].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			
			pTxRing->Cell[TxIdx].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[TxIdx].pNextNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			
			pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

			if (ATESetUpFrame(pAd, TxIdx) != 0)
				break;

			INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);
		}

		ATESetUpFrame(pAd, pTxRing->TxCpuIdx);

		
		RtmpDmaEnable(pAd, 1);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value |= (1 << 2);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value &= ~(1 << 3);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

#ifdef RALINK_28xx_QA
		if (pAd->ate.bQATxStart == TRUE)
		{
			pAd->ate.TxStatus = 1;
		}
#endif 

		
		RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pAd->TxRing[QID_AC_BE].TxCpuIdx);

		RTMPusecDelay(500);

		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacData);
		MacData |= 0x00000010;
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);
	}
	else if (!strcmp(arg, "TXFRAME"))
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("ATE: TXFRAME(Count=%d)\n", pAd->ate.TxCount));
		pAd->ate.Mode |= ATE_TXFRAME;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

		
		BbpSoftReset(pAd);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

		
		RtmpDmaEnable(pAd, 0);

		
		{
			RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * 0x10,  &pTxRing->TxDmaIdx);
			pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
			pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
			RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * 0x10, pTxRing->TxCpuIdx);
		}

		pAd->ate.TxDoneCount = 0;

		SetJapanFilter(pAd);

		for (i = 0; (i < TX_RING_SIZE-1) && (i < pAd->ate.TxCount); i++)
		{
			PNDIS_PACKET pPacket;
			UINT32 TxIdx = pTxRing->TxCpuIdx;

#ifndef RT_BIG_ENDIAN
			pTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
#else
			pDestTxD = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
			
			pPacket = pTxRing->Cell[TxIdx].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			
			pTxRing->Cell[TxIdx].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[TxIdx].pNextNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			
			pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

			if (ATESetUpFrame(pAd, TxIdx) != 0)
				break;

			INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);

		}

		ATESetUpFrame(pAd, pTxRing->TxCpuIdx);

		
		RtmpDmaEnable(pAd, 1);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value |= (1 << 2);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

#ifdef RALINK_28xx_QA
		
		if (pAd->ate.bQARxStart == FALSE)
		{
			
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
			Value &= ~(1 << 3);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
		}

		if (pAd->ate.bQATxStart == TRUE)
		{
			pAd->ate.TxStatus = 1;
		}
#else
		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value &= ~(1 << 3);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
#endif 

		RTMP_IO_READ32(pAd, TX_DTX_IDX0 + QID_AC_BE * RINGREG_DIFF, &pAd->TxRing[QID_AC_BE].TxDmaIdx);
		
		RTMP_IO_WRITE32(pAd, TX_CTX_IDX0 + QID_AC_BE * RINGREG_DIFF, pAd->TxRing[QID_AC_BE].TxCpuIdx);

		pAd->RalinkCounters.KickTxCount++;
	}
#ifdef RALINK_28xx_QA
	else if (!strcmp(arg, "TXSTOP"))
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("ATE: TXSTOP\n"));
		atemode = pAd->ate.Mode;
		pAd->ate.Mode &= ATE_TXSTOP;
		pAd->ate.bQATxStart = FALSE;


		if (atemode == ATE_TXCARR)
		{
			
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= 0xFFFFFF00; 
		    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
		}
		else if (atemode == ATE_TXCARRSUPP)
		{
			
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
			BbpData &= ~(1 << 7); 
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);

			
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R24, &BbpData);
			BbpData &= 0xFFFFFFFE; 
		    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, BbpData);
		}

		
		else if ((atemode & ATE_TXFRAME) || (atemode == ATE_STOP))
		{
			PRTMP_TX_RING pTxRing = &pAd->TxRing[QID_AC_BE];

			if (atemode == ATE_TXCONT)
			{
				
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &BbpData);
				BbpData &= ~(1 << 7); 
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
			}

			
			RtmpDmaEnable(pAd, 0);

			for (i=0; i<TX_RING_SIZE; i++)
			{
				PNDIS_PACKET  pPacket;

#ifndef RT_BIG_ENDIAN
			    pTxD = (PTXD_STRUC)pAd->TxRing[QID_AC_BE].Cell[i].AllocVa;
#else
			pDestTxD = (PTXD_STRUC)pAd->TxRing[QID_AC_BE].Cell[i].AllocVa;
			TxD = *pDestTxD;
			pTxD = &TxD;
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				pTxD->DMADONE = 0;
				pPacket = pTxRing->Cell[i].pNdisPacket;

				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
				}

				
				pTxRing->Cell[i].pNdisPacket = NULL;

				pPacket = pTxRing->Cell[i].pNextNdisPacket;

				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
				}

				
				pTxRing->Cell[i].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif
			}
			
			RtmpDmaEnable(pAd, 1);

		}

		
		pAd->ate.TxStatus = 0;

		
		BbpSoftReset(pAd);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value &= ~(1 << 2);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
	}
	else if (!strcmp(arg, "RXSTOP"))
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("ATE: RXSTOP\n"));
		atemode = pAd->ate.Mode;
		pAd->ate.Mode &= ATE_RXSTOP;
		pAd->ate.bQARxStart = FALSE;


		if (atemode == ATE_TXCARR)
		{
			;
		}
		else if (atemode == ATE_TXCARRSUPP)
		{
			;
		}

		
		else if ((atemode & ATE_TXFRAME) || (atemode == ATE_STOP))
		{
			if (atemode == ATE_TXCONT)
			{
				;
			}
		}

		
		BbpSoftReset(pAd);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value &= ~(1 << 3);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
	}
#endif 
	else if (!strcmp(arg, "RXFRAME"))
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("ATE: RXFRAME\n"));

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, BbpData);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacData);

		pAd->ate.Mode |= ATE_RXFRAME;

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value &= ~(1 << 2);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

		
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
		Value |= (1 << 3);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);
	}
	else
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("ATE: Invalid arg!\n"));
		return FALSE;
	}
	RTMPusecDelay(5000);

	ATEDBGPRINT(RT_DEBUG_TRACE, ("<=== ATECmdHandler()\n"));

	return TRUE;
}

#endif 




INT	Set_ATE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	if (ATECmdHandler(pAd, arg))
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_Proc Success\n"));


		return TRUE;
	}
	else
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_Proc Failed\n"));
		return FALSE;
	}
}



INT	Set_ATE_DA_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PSTRING				value;
	INT					i;

	
	if (strlen(arg) != 17)
		return FALSE;

    for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"))
	{
		
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;
		}

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pAd->ate.Addr3[i++], 1);
#endif 
	}

	
	if (i != 6)
	{
		return FALSE;
	}

#ifdef CONFIG_STA_SUPPORT
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_DA_Proc (DA = %2X:%2X:%2X:%2X:%2X:%2X)\n", pAd->ate.Addr3[0],
		pAd->ate.Addr3[1], pAd->ate.Addr3[2], pAd->ate.Addr3[3], pAd->ate.Addr3[4], pAd->ate.Addr3[5]));
#endif 

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_DA_Proc Success\n"));

	return TRUE;
}



INT	Set_ATE_SA_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PSTRING				value;
	INT					i;

	
	if (strlen(arg) != 17)
		return FALSE;

    for (i=0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"))
	{
		
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;
		}

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pAd->ate.Addr2[i++], 1);
#endif 
	}

	
	if (i != 6)
	{
		return FALSE;
	}

#ifdef CONFIG_STA_SUPPORT
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_SA_Proc (SA = %2X:%2X:%2X:%2X:%2X:%2X)\n", pAd->ate.Addr2[0],
		pAd->ate.Addr2[1], pAd->ate.Addr2[2], pAd->ate.Addr2[3], pAd->ate.Addr2[4], pAd->ate.Addr2[5]));
#endif 

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_SA_Proc Success\n"));

	return TRUE;
}



INT	Set_ATE_BSSID_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PSTRING				value;
	INT					i;

	
	if (strlen(arg) != 17)
		return FALSE;

    for (i=0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":"))
	{
		
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))))
		{
			return FALSE;
		}

#ifdef CONFIG_STA_SUPPORT
		AtoH(value, &pAd->ate.Addr1[i++], 1);
#endif 
	}

	
	if(i != 6)
	{
		return FALSE;
	}

#ifdef CONFIG_STA_SUPPORT
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_BSSID_Proc (BSSID = %2X:%2X:%2X:%2X:%2X:%2X)\n",	pAd->ate.Addr1[0],
		pAd->ate.Addr1[1], pAd->ate.Addr1[2], pAd->ate.Addr1[3], pAd->ate.Addr1[4], pAd->ate.Addr1[5]));
#endif 

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_BSSID_Proc Success\n"));

	return TRUE;
}



INT	Set_ATE_CHANNEL_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR channel;

	channel = simple_strtol(arg, 0, 10);

	
	if ((channel < 1) || (channel > 216))
	{
		ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_CHANNEL_Proc::Out of range, it should be in range of 1~14.\n"));
		return FALSE;
	}
	pAd->ate.Channel = channel;

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_CHANNEL_Proc (ATE Channel = %d)\n", pAd->ate.Channel));
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_CHANNEL_Proc Success\n"));


	return TRUE;
}



INT	Set_ATE_TX_POWER0_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	CHAR TxPower;

	TxPower = simple_strtol(arg, 0, 10);

	if (pAd->ate.Channel <= 14)
	{
		if ((TxPower > 31) || (TxPower < 0))
		{
			ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_POWER0_Proc::Out of range (Value=%d)\n", TxPower));
			return FALSE;
		}
	}
	else
	{
		if ((TxPower > 15) || (TxPower < -7))
		{
			ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_POWER0_Proc::Out of range (Value=%d)\n", TxPower));
			return FALSE;
		}
	}

	pAd->ate.TxPower0 = TxPower;
	ATETxPwrHandler(pAd, 0);
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_POWER0_Proc Success\n"));


	return TRUE;
}



INT	Set_ATE_TX_POWER1_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	CHAR TxPower;

	TxPower = simple_strtol(arg, 0, 10);

	if (pAd->ate.Channel <= 14)
	{
		if ((TxPower > 31) || (TxPower < 0))
		{
			ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_POWER1_Proc::Out of range (Value=%d)\n", TxPower));
			return FALSE;
		}
	}
	else
	{
		if ((TxPower > 15) || (TxPower < -7))
		{
			ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_POWER1_Proc::Out of range (Value=%d)\n", TxPower));
			return FALSE;
		}
	}

	pAd->ate.TxPower1 = TxPower;
	ATETxPwrHandler(pAd, 1);
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_POWER1_Proc Success\n"));


	return TRUE;
}



INT	Set_ATE_TX_Antenna_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	CHAR value;

	value = simple_strtol(arg, 0, 10);

	if ((value > 2) || (value < 0))
	{
		ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_Antenna_Proc::Out of range (Value=%d)\n", value));
		return FALSE;
	}

	pAd->ate.TxAntennaSel = value;

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_Antenna_Proc (Antenna = %d)\n", pAd->ate.TxAntennaSel));
	ATEDBGPRINT(RT_DEBUG_TRACE,("Ralink: Set_ATE_TX_Antenna_Proc Success\n"));

	
	ATEAsicSwitchChannel(pAd);


	return TRUE;
}



INT	Set_ATE_RX_Antenna_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	CHAR value;

	value = simple_strtol(arg, 0, 10);

	if ((value > 3) || (value < 0))
	{
		ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_RX_Antenna_Proc::Out of range (Value=%d)\n", value));
		return FALSE;
	}

	pAd->ate.RxAntennaSel = value;

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_RX_Antenna_Proc (Antenna = %d)\n", pAd->ate.RxAntennaSel));
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_RX_Antenna_Proc Success\n"));

	
	ATEAsicSwitchChannel(pAd);


	return TRUE;
}



INT	Set_ATE_TX_FREQOFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR RFFreqOffset = 0;
	ULONG R4 = 0;

	RFFreqOffset = simple_strtol(arg, 0, 10);
#ifndef RTMP_RF_RW_SUPPORT
	if (RFFreqOffset >= 64)
#endif 
	
#ifdef RTMP_RF_RW_SUPPORT

	if (RFFreqOffset >= 95)
#endif 
	{
		ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_FREQOFFSET_Proc::Out of range, it should be in range of 0~63.\n"));
		return FALSE;
	}

	pAd->ate.RFFreqOffset = RFFreqOffset;
#ifdef RTMP_RF_RW_SUPPORT
	if (IS_RT30xx(pAd) || IS_RT3572(pAd))
	{
		
		UCHAR RFValue;
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);

		RFValue = ((RFValue & 0x80) | pAd->ate.RFFreqOffset);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)RFValue);
	}
	else
#endif 
	{
		
		
		R4 = pAd->ate.RFFreqOffset << 15;
		R4 |= (pAd->LatchRfRegs.R4 & ((~0x001f8000)));
		pAd->LatchRfRegs.R4 = R4;

		RtmpRfIoWrite(pAd);
	}
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_FREQOFFSET_Proc (RFFreqOffset = %d)\n", pAd->ate.RFFreqOffset));
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_FREQOFFSET_Proc Success\n"));


	return TRUE;
}



INT	Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	INT i;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;

	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if ((BBPCurrentBW == 0)
#ifdef RT30xx
		|| IS_RT2070(pAd)
#endif 
		)
	{
		pAd->ate.TxWI.BW = BW_20;
	}
	else
	{
		pAd->ate.TxWI.BW = BW_40;
	}

	
	
	
	if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.TxWI.BW == BW_40))
	{
		ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\nBandwidth switch to 20\n"));
		pAd->ate.TxWI.BW = BW_20;
	}

	if (pAd->ate.TxWI.BW == BW_20)
	{
		if (pAd->ate.Channel <= 14)
		{
			for (i=0; i<5; i++)
			{
				if (pAd->Tx20MPwrCfgGBand[i] != 0xffffffff)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, pAd->Tx20MPwrCfgGBand[i]);
					RTMPusecDelay(5000);
				}
			}
		}
		else
		{
			for (i=0; i<5; i++)
			{
				if (pAd->Tx20MPwrCfgABand[i] != 0xffffffff)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, pAd->Tx20MPwrCfgABand[i]);
					RTMPusecDelay(5000);
				}
			}
		}

		
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
		value &= (~0x18);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);


		
		value = 0x3C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, value);

		
		
		value = 0x0B;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		
		value = 0x16;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		
		value = 0x08;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		
		value = 0x11;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

	    
		if (pAd->ate.Channel == 14)
		{
			INT TxMode = pAd->ate.TxWI.PHYMODE;

			if (TxMode == MODE_CCK)
			{
				
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
				value |= 0x20; 
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);
			}
		}

#ifdef RT30xx
		
		if (IS_RT30xx(pAd))
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR) pAd->Mlme.CaliBW20RfR24);
		else
#endif 
		
		{
			pAd->LatchRfRegs.R4 &= ~0x00200000;
			RtmpRfIoWrite(pAd);
		}

	}
	
	else if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel <= 14)
		{
			for (i=0; i<5; i++)
			{
				if (pAd->Tx40MPwrCfgGBand[i] != 0xffffffff)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, pAd->Tx40MPwrCfgGBand[i]);
					RTMPusecDelay(5000);
				}
			}
		}
		else
		{
			for (i=0; i<5; i++)
			{
				if (pAd->Tx40MPwrCfgABand[i] != 0xffffffff)
				{
					RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, pAd->Tx40MPwrCfgABand[i]);
					RTMPusecDelay(5000);
				}
			}
#ifdef DOT11_N_SUPPORT
			if ((pAd->ate.TxWI.PHYMODE >= MODE_HTMIX) && (pAd->ate.TxWI.MCS == 7))
			{
			value = 0x28;
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, value);
			}
#endif 
		}

		
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
		value &= (~0x18);
		value |= 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);


		
		value = 0x3C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, value);

		
		
		value = 0x0C;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		
		value = 0x1A;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		
		value = 0x0A;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		
		value = 0x16;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

		
#ifdef RT30xx
		
		if(IS_RT30xx(pAd))
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR) pAd->Mlme.CaliBW40RfR24);
		else
#endif 
		
		{
		pAd->LatchRfRegs.R4 |= 0x00200000;
		RtmpRfIoWrite(pAd);
		}
	}

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", pAd->ate.TxWI.BW));
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));


	return TRUE;
}



INT	Set_ATE_TX_LENGTH_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->ate.TxLength = simple_strtol(arg, 0, 10);

	if ((pAd->ate.TxLength < 24) || (pAd->ate.TxLength > (MAX_FRAME_SIZE - 34)))
	{
		pAd->ate.TxLength = (MAX_FRAME_SIZE - 34);
		ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_LENGTH_Proc::Out of range, it should be in range of 24~%d.\n", (MAX_FRAME_SIZE - 34)));
		return FALSE;
	}

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_LENGTH_Proc (TxLength = %d)\n", pAd->ate.TxLength));
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_LENGTH_Proc Success\n"));


	return TRUE;
}



INT	Set_ATE_TX_COUNT_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->ate.TxCount = simple_strtol(arg, 0, 10);

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_COUNT_Proc (TxCount = %d)\n", pAd->ate.TxCount));
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_COUNT_Proc Success\n"));


	return TRUE;
}



INT	Set_ATE_TX_MCS_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR MCS;
	INT result;

	MCS = simple_strtol(arg, 0, 10);
#ifndef RT30xx
	result = CheckMCSValid(pAd->ate.TxWI.PHYMODE, MCS);
#endif 

	
#ifdef RT30xx
	result = CheckMCSValid(pAd->ate.TxWI.PHYMODE, MCS, IS_RT2070(pAd));
#endif 


	if (result != -1)
	{
		pAd->ate.TxWI.MCS = (UCHAR)MCS;
	}
	else
	{
		ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_MCS_Proc::Out of range, refer to rate table.\n"));
		return FALSE;
	}

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_MCS_Proc (MCS = %d)\n", pAd->ate.TxWI.MCS));
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_MCS_Proc Success\n"));


	return TRUE;
}



INT	Set_ATE_TX_MODE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR BbpData = 0;

	pAd->ate.TxWI.PHYMODE = simple_strtol(arg, 0, 10);

	if (pAd->ate.TxWI.PHYMODE > 3)
	{
		pAd->ate.TxWI.PHYMODE = 0;
		ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_MODE_Proc::Out of range.\nIt should be in range of 0~3\n"));
		ATEDBGPRINT(RT_DEBUG_ERROR, ("0: CCK, 1: OFDM, 2: HT_MIX, 3: HT_GREEN_FIELD.\n"));
		return FALSE;
	}

	
	if (pAd->ate.TxWI.PHYMODE == MODE_CCK)
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpData);
		BbpData &= (~0x18);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpData);
		pAd->ate.TxWI.BW = BW_20;
		ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_MODE_Proc::CCK Only support 20MHZ. Switch to 20MHZ.\n"));
	}

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_MODE_Proc (TxMode = %d)\n", pAd->ate.TxWI.PHYMODE));
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_MODE_Proc Success\n"));


	return TRUE;
}



INT	Set_ATE_TX_GI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->ate.TxWI.ShortGI = simple_strtol(arg, 0, 10);

	if (pAd->ate.TxWI.ShortGI > 1)
	{
		pAd->ate.TxWI.ShortGI = 0;
		ATEDBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_GI_Proc::Out of range\n"));
		return FALSE;
	}

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_GI_Proc (GI = %d)\n", pAd->ate.TxWI.ShortGI));
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_GI_Proc Success\n"));


	return TRUE;
}


INT	Set_ATE_RX_FER_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->ate.bRxFER = simple_strtol(arg, 0, 10);

	if (pAd->ate.bRxFER == 1)
	{
		pAd->ate.RxCntPerSec = 0;
		pAd->ate.RxTotalCnt = 0;
	}

	ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_RX_FER_Proc (bRxFER = %d)\n", pAd->ate.bRxFER));
	ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_RX_FER_Proc Success\n"));


	return TRUE;
}


INT Set_ATE_Read_RF_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
#ifdef RTMP_RF_RW_SUPPORT

	if (IS_RT30xx(pAd) || IS_RT3572(pAd))
	{
		
		UCHAR RFValue;
		INT index=0;

		for (index = 0; index < 32; index++)
		{
			ATE_RF_IO_READ8_BY_REG_ID(pAd, index, (PUCHAR)&RFValue);
			ate_print("R%d=%d\n",index,RFValue);
		}
	}
	else

#endif 
	{
		ate_print(KERN_EMERG "R1 = %lx\n", pAd->LatchRfRegs.R1);
		ate_print(KERN_EMERG "R2 = %lx\n", pAd->LatchRfRegs.R2);
		ate_print(KERN_EMERG "R3 = %lx\n", pAd->LatchRfRegs.R3);
		ate_print(KERN_EMERG "R4 = %lx\n", pAd->LatchRfRegs.R4);
	}
	return TRUE;
}


INT Set_ATE_Write_RF1_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);

#ifdef RTMP_RF_RW_SUPPORT

	if (IS_RT30xx(pAd) || IS_RT3572(pAd))
	{
		ate_print("Warning!! RT3xxx Don't Support !\n");
		return FALSE;

	}
	else

#endif 
	{
		pAd->LatchRfRegs.R1 = value;
		RtmpRfIoWrite(pAd);
	}
	return TRUE;
}


INT Set_ATE_Write_RF2_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);

#ifdef RTMP_RF_RW_SUPPORT

	if (IS_RT30xx(pAd) || IS_RT3572(pAd))
	{
		ate_print("Warning!! RT3xxx Don't Support !\n");
		return FALSE;

	}
	else

#endif 
	{
		pAd->LatchRfRegs.R2 = value;
		RtmpRfIoWrite(pAd);
	}
	return TRUE;
}


INT Set_ATE_Write_RF3_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UINT32 value = simple_strtol(arg, 0, 16);

#ifdef RTMP_RF_RW_SUPPORT

	if (IS_RT30xx(pAd) || IS_RT3572(pAd))
	{
		ate_print("Warning!! RT3xxx Don't Support !\n");
		return FALSE;

	}
	else

#endif 
	{
		pAd->LatchRfRegs.R3 = value;
		RtmpRfIoWrite(pAd);
	}
	return TRUE;
}


INT Set_ATE_Write_RF4_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UINT32 value = (UINT32) simple_strtol(arg, 0, 16);

#ifdef RTMP_RF_RW_SUPPORT

	if (IS_RT30xx(pAd) || IS_RT3572(pAd))
	{
		ate_print("Warning!! RT3xxx Don't Support !\n");
		return FALSE;

	}
	else

#endif 
	{
		pAd->LatchRfRegs.R4 = value;
		RtmpRfIoWrite(pAd);
	}
	return TRUE;
}



#if defined(LINUX) || defined(VXWORKS)
INT Set_ATE_Load_E2P_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	BOOLEAN			ret = FALSE;
	PSTRING			src = EEPROM_BIN_FILE_NAME;
	RTMP_OS_FD		srcf;
	INT32			retval;
	USHORT			WriteEEPROM[(EEPROM_SIZE/2)];
	INT				FileLength = 0;
	UINT32			value = (UINT32) simple_strtol(arg, 0, 10);
	RTMP_OS_FS_INFO	osFSInfo;

	ATEDBGPRINT(RT_DEBUG_ERROR, ("===> %s (value=%d)\n\n", __FUNCTION__, value));

	if (value > 0)
	{
		
		NdisZeroMemory((PUCHAR)WriteEEPROM, EEPROM_SIZE);

		RtmpOSFSInfoChange(&osFSInfo, TRUE);

		do
		{
			
			srcf = RtmpOSFileOpen(src, O_RDONLY, 0);

			if (IS_FILE_OPEN_ERR(srcf))
			{
				ate_print("%s - Error opening file %s\n", __FUNCTION__, src);
				break;
			}

			
			FileLength = RtmpOSFileRead(srcf, (PSTRING)WriteEEPROM, EEPROM_SIZE);

			if (FileLength != EEPROM_SIZE)
			{
				ate_print("%s: error file length (=%d) in e2p.bin\n",
					   __FUNCTION__, FileLength);
				break;
			}
			else
			{
				
				rt_ee_write_all(pAd, WriteEEPROM);
				ret = TRUE;
			}
			break;
		} while(TRUE);

		
		if (IS_FILE_OPEN_ERR(srcf))
		{
				;
		}
		else
		{
			retval = RtmpOSFileClose(srcf);

			if (retval)
			{
				ATEDBGPRINT(RT_DEBUG_ERROR, ("--> Error %d closing %s\n", -retval, src));

			}
		}

		
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}

    ATEDBGPRINT(RT_DEBUG_ERROR, ("<=== %s (ret=%d)\n", __FUNCTION__, ret));

    return ret;

}
#endif 




INT Set_ATE_Read_E2P_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	USHORT buffer[EEPROM_SIZE/2];
	USHORT *p;
	int i;

	rt_ee_read_all(pAd, (USHORT *)buffer);
	p = buffer;
	for (i = 0; i < (EEPROM_SIZE/2); i++)
	{
		ate_print("%4.4x ", *p);
		if (((i+1) % 16) == 0)
			ate_print("\n");
		p++;
	}
	return TRUE;
}




INT	Set_ATE_Show_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ate_print("Mode=%d\n", pAd->ate.Mode);
	ate_print("TxPower0=%d\n", pAd->ate.TxPower0);
	ate_print("TxPower1=%d\n", pAd->ate.TxPower1);
	ate_print("TxAntennaSel=%d\n", pAd->ate.TxAntennaSel);
	ate_print("RxAntennaSel=%d\n", pAd->ate.RxAntennaSel);
	ate_print("BBPCurrentBW=%d\n", pAd->ate.TxWI.BW);
	ate_print("GI=%d\n", pAd->ate.TxWI.ShortGI);
	ate_print("MCS=%d\n", pAd->ate.TxWI.MCS);
	ate_print("TxMode=%d\n", pAd->ate.TxWI.PHYMODE);
	ate_print("Addr1=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pAd->ate.Addr1[0], pAd->ate.Addr1[1], pAd->ate.Addr1[2], pAd->ate.Addr1[3], pAd->ate.Addr1[4], pAd->ate.Addr1[5]);
	ate_print("Addr2=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pAd->ate.Addr2[0], pAd->ate.Addr2[1], pAd->ate.Addr2[2], pAd->ate.Addr2[3], pAd->ate.Addr2[4], pAd->ate.Addr2[5]);
	ate_print("Addr3=%02x:%02x:%02x:%02x:%02x:%02x\n",
		pAd->ate.Addr3[0], pAd->ate.Addr3[1], pAd->ate.Addr3[2], pAd->ate.Addr3[3], pAd->ate.Addr3[4], pAd->ate.Addr3[5]);
	ate_print("Channel=%d\n", pAd->ate.Channel);
	ate_print("TxLength=%d\n", pAd->ate.TxLength);
	ate_print("TxCount=%u\n", pAd->ate.TxCount);
	ate_print("RFFreqOffset=%d\n", pAd->ate.RFFreqOffset);
	ate_print(KERN_EMERG "Set_ATE_Show_Proc Success\n");
	return TRUE;
}


INT	Set_ATE_Help_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ate_print("ATE=ATESTART, ATESTOP, TXCONT, TXCARR, TXFRAME, RXFRAME\n");
	ate_print("ATEDA\n");
	ate_print("ATESA\n");
	ate_print("ATEBSSID\n");
	ate_print("ATECHANNEL, range:0~14(unless A band !)\n");
	ate_print("ATETXPOW0, set power level of antenna 1.\n");
	ate_print("ATETXPOW1, set power level of antenna 2.\n");
	ate_print("ATETXANT, set TX antenna. 0:all, 1:antenna one, 2:antenna two.\n");
	ate_print("ATERXANT, set RX antenna.0:all, 1:antenna one, 2:antenna two, 3:antenna three.\n");
	ate_print("ATETXFREQOFFSET, set frequency offset, range 0~63\n");
	ate_print("ATETXBW, set BandWidth, 0:20MHz, 1:40MHz.\n");
	ate_print("ATETXLEN, set Frame length, range 24~%d\n", (MAX_FRAME_SIZE - 34));
	ate_print("ATETXCNT, set how many frame going to transmit.\n");
	ate_print("ATETXMCS, set MCS, reference to rate table.\n");
	ate_print("ATETXMODE, set Mode 0:CCK, 1:OFDM, 2:HT-Mix, 3:GreenField, reference to rate table.\n");
	ate_print("ATETXGI, set GI interval, 0:Long, 1:Short\n");
	ate_print("ATERXFER, 0:disable Rx Frame error rate. 1:enable Rx Frame error rate.\n");
	ate_print("ATERRF, show all RF registers.\n");
	ate_print("ATEWRF1, set RF1 register.\n");
	ate_print("ATEWRF2, set RF2 register.\n");
	ate_print("ATEWRF3, set RF3 register.\n");
	ate_print("ATEWRF4, set RF4 register.\n");
	ate_print("ATELDE2P, load EEPROM from .bin file.\n");
	ate_print("ATERE2P, display all EEPROM content.\n");
	ate_print("ATESHOW, display all parameters of ATE.\n");
	ate_print("ATEHELP, online help.\n");

	return TRUE;
}





VOID ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd)
{
	UINT32 R2 = 0, R3 = DEFAULT_RF_TX_POWER, R4 = 0, Value = 0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, R66 = 0x30;
	RTMP_RF_REGS *RFRegTable;
	UCHAR Channel = 0;

	RFRegTable = NULL;

#ifdef RALINK_28xx_QA
	
	if ((pAd->ate.bQATxStart == TRUE) || (pAd->ate.bQARxStart == TRUE))
	{
		if (pAd->ate.Channel != pAd->LatchRfRegs.Channel)
		{
			pAd->ate.Channel = pAd->LatchRfRegs.Channel;
		}
		return;
	}
	else
#endif 
	Channel = pAd->ate.Channel;

	
	AsicAntennaSelect(pAd, Channel);

	
	TxPwer = pAd->ate.TxPower0;
	TxPwer2 = pAd->ate.TxPower1;
#ifdef RT30xx


	
	if (IS_RT30xx(pAd) && ((pAd->RfIcType == RFIC_3020) ||
			(pAd->RfIcType == RFIC_3021) || (pAd->RfIcType == RFIC_3022) ||
			(pAd->RfIcType == RFIC_2020)))
	{
		
		UCHAR RFValue = 0;

		for (index = 0; index < NUM_OF_3020_CHNL; index++)
		{
			if (Channel == FreqItems3020[index].Channel)
			{
				
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R02, FreqItems3020[index].N);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, FreqItems3020[index].K);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R06, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xFC) | FreqItems3020[index].R;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R06, (UCHAR)RFValue);

				
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R12, (PUCHAR)&RFValue);
				RFValue = (RFValue & 0xE0) | TxPwer;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);

				
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R23, (PUCHAR)&RFValue);
				
				RFValue = (RFValue & 0x80) | pAd->ate.RFFreqOffset;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R23, (UCHAR)RFValue);

				
				if (pAd->ate.TxWI.BW == BW_40)
				{
					RFValue = pAd->Mlme.CaliBW40RfR24;

				}
				else
				{
					RFValue = pAd->Mlme.CaliBW20RfR24;
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)RFValue);

				
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R07, (PUCHAR)&RFValue);
				RFValue = RFValue | 0x1;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R07, (UCHAR)RFValue);

				
				pAd->LatchRfRegs.Channel = Channel;

				break;
			}
		}

		ATEDBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
			Channel,
			pAd->RfIcType,
			TxPwer,
			TxPwer2,
			pAd->Antenna.field.TxPath,
			FreqItems3020[index].N,
			FreqItems3020[index].K,
			FreqItems3020[index].R));
	}
	else

#endif 
	{
		
		RFRegTable = RF2850RegTable;

		switch (pAd->RfIcType)
		{
			
			case RFIC_2820:
			case RFIC_2850:
			case RFIC_2720:
			case RFIC_2750:

				for (index = 0; index < NUM_OF_2850_CHNL; index++)
				{
					if (Channel == RFRegTable[index].Channel)
					{
						R2 = RFRegTable[index].R2;

						
						if (pAd->Antenna.field.TxPath == 1)
						{
							R2 |= 0x4000;
						}

						if (pAd->Antenna.field.RxPath == 2)
						{
							switch (pAd->ate.RxAntennaSel)
							{
								case 1:
									R2 |= 0x20040;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x00;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
									break;
								case 2:
									R2 |= 0x10040;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x01;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
									break;
								default:
									R2 |= 0x40;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									
									BbpValue |= 0x08;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
									break;
							}
						}
						else if (pAd->Antenna.field.RxPath == 1)
						{
							
							R2 |= 0x20040;
						}

						if (pAd->Antenna.field.TxPath == 2)
						{
							if (pAd->ate.TxAntennaSel == 1)
							{
								
								R2 |= 0x4000;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
								BbpValue &= 0xE7;		
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
							}
							else if (pAd->ate.TxAntennaSel == 2)
							{
								
								R2 |= 0x8000;
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
								BbpValue &= 0xE7;
								BbpValue |= 0x08;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
							}
							else
							{
								ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
								BbpValue &= 0xE7;
								BbpValue |= 0x10;
								ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);
							}
						}
						if (pAd->Antenna.field.RxPath == 3)
						{
							switch (pAd->ate.RxAntennaSel)
							{
								case 1:
									R2 |= 0x20040;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x00;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
									break;
								case 2:
									R2 |= 0x10040;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x01;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
									break;
								case 3:
									R2 |= 0x30000;
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x02;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
									break;
								default:
									ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
									BbpValue &= 0xE4;
									BbpValue |= 0x10;
									ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);
									break;
							}
						}

						if (Channel > 14)
						{
							
							R3 = (RFRegTable[index].R3 & 0xffffc1ff);
							R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->ate.RFFreqOffset << 15);

		                    
							
							if ((TxPwer >= -7) && (TxPwer < 0))
							{
								TxPwer = (7+TxPwer);
								TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
								R3 |= (TxPwer << 10);
								ATEDBGPRINT(RT_DEBUG_TRACE, ("ATEAsicSwitchChannel: TxPwer=%d \n", TxPwer));
							}
							else
							{
								TxPwer = (TxPwer > 0xF) ? (0xF) : (TxPwer);
								R3 |= (TxPwer << 10) | (1 << 9);
							}

							
							if ((TxPwer2 >= -7) && (TxPwer2 < 0))
							{
								TxPwer2 = (7+TxPwer2);
								TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
								R4 |= (TxPwer2 << 7);
								ATEDBGPRINT(RT_DEBUG_TRACE, ("ATEAsicSwitchChannel: TxPwer2=%d \n", TxPwer2));
							}
							else
							{
								TxPwer2 = (TxPwer2 > 0xF) ? (0xF) : (TxPwer2);
								R4 |= (TxPwer2 << 7) | (1 << 6);
							}
						}
						else
						{
							
							R3 = (RFRegTable[index].R3 & 0xffffc1ff) | (TxPwer << 9);
							
							R4 = (RFRegTable[index].R4 & (~0x001f87c0)) | (pAd->ate.RFFreqOffset << 15) | (TxPwer2 <<6);
						}

						
						if (pAd->ate.TxWI.BW == BW_40)
						{
							R4 |=0x200000;
						}

						
						pAd->LatchRfRegs.Channel = Channel;
						pAd->LatchRfRegs.R1 = RFRegTable[index].R1;
						pAd->LatchRfRegs.R2 = R2;
						pAd->LatchRfRegs.R3 = R3;
						pAd->LatchRfRegs.R4 = R4;

						RtmpRfIoWrite(pAd);

						break;
					}
				}
				break;

			default:
				break;
		}
	}

	
	if (Channel <= 14)
	{
	    UINT32 TxPinCfg = 0x00050F0A;

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		
	    if (pAd->NicConfig2.field.ExternalLNAForG)
	    {
	        ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x62);
	    }
	    else
	    {
	        ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0x84);
	    }

        
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R86, &BbpValue);

		ASSERT((BbpValue == 0x00));
		if ((BbpValue != 0x00))
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x00);
		}

		
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

        
		if (pAd->Antenna.field.TxPath == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}

		
		if (pAd->Antenna.field.TxPath == 2)
		{
			if (pAd->ate.TxAntennaSel == 1)
			{
				TxPinCfg &= 0xFFFFFFF7;
			}
			else if (pAd->ate.TxAntennaSel == 2)
			{
				TxPinCfg &= 0xFFFFFFFD;
			}
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}
	else
	{
	    UINT32	TxPinCfg = 0x00050F05;

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R82, 0xF2);

        
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R86, &BbpValue);

		ASSERT((BbpValue == 0x00));
		if ((BbpValue != 0x00))
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x00);
		}

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R91, &BbpValue);
		ASSERT((BbpValue == 0x04));

		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R92, &BbpValue);
		ASSERT((BbpValue == 0x00));

		
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x02);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		
		if (pAd->Antenna.field.TxPath == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		if (pAd->Antenna.field.RxPath == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}


    
	if (Channel <= 14)
	{
		
		R66 = 0x2E + GET_LNA_GAIN(pAd);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
	}
	else
	{
		
		if (pAd->ate.TxWI.BW == BW_20)
		{
			R66 = (UCHAR)(0x32 + (GET_LNA_GAIN(pAd)*5)/3);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
		}
		else
		{
			R66 = (UCHAR)(0x3A + (GET_LNA_GAIN(pAd)*5)/3);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, R66);
		}
	}

	
	RTMPusecDelay(1000);

#ifndef RTMP_RF_RW_SUPPORT
	if (Channel > 14)
	{
		
		ATEDBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, %dT) to , R1=0x%08lx, R2=0x%08lx, R3=0x%08lx, R4=0x%08lx\n",
								  Channel,
								  pAd->RfIcType,
								  pAd->Antenna.field.TxPath,
								  pAd->LatchRfRegs.R1,
								  pAd->LatchRfRegs.R2,
								  pAd->LatchRfRegs.R3,
								  pAd->LatchRfRegs.R4));
	}
	else
	{
		ATEDBGPRINT(RT_DEBUG_TRACE, ("SwitchChannel#%d(RF=%d, Pwr0=%u, Pwr1=%u, %dT) to , R1=0x%08lx, R2=0x%08lx, R3=0x%08lx, R4=0x%08lx\n",
								  Channel,
								  pAd->RfIcType,
								  (R3 & 0x00003e00) >> 9,
								  (R4 & 0x000007c0) >> 6,
								  pAd->Antenna.field.TxPath,
								  pAd->LatchRfRegs.R1,
								  pAd->LatchRfRegs.R2,
								  pAd->LatchRfRegs.R3,
								  pAd->LatchRfRegs.R4));
    }
#endif 
}






VOID ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd)
{
	INT			i, j;
	CHAR		DeltaPwr = 0;
	BOOLEAN		bAutoTxAgc = FALSE;
	UCHAR		TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep;
	UCHAR		BbpR49 = 0, idx;
	PCHAR		pTxAgcCompensate;
	ULONG		TxPwr[5];
	CHAR		Value;

	
	if (pAd->ate.TxWI.BW == BW_40)
	{
		if (pAd->ate.Channel > 14)
		{
			TxPwr[0] = pAd->Tx40MPwrCfgABand[0];
			TxPwr[1] = pAd->Tx40MPwrCfgABand[1];
			TxPwr[2] = pAd->Tx40MPwrCfgABand[2];
			TxPwr[3] = pAd->Tx40MPwrCfgABand[3];
			TxPwr[4] = pAd->Tx40MPwrCfgABand[4];
		}
		else
		{
			TxPwr[0] = pAd->Tx40MPwrCfgGBand[0];
			TxPwr[1] = pAd->Tx40MPwrCfgGBand[1];
			TxPwr[2] = pAd->Tx40MPwrCfgGBand[2];
			TxPwr[3] = pAd->Tx40MPwrCfgGBand[3];
			TxPwr[4] = pAd->Tx40MPwrCfgGBand[4];
		}
	}
	else
	{
		if (pAd->ate.Channel > 14)
		{
			TxPwr[0] = pAd->Tx20MPwrCfgABand[0];
			TxPwr[1] = pAd->Tx20MPwrCfgABand[1];
			TxPwr[2] = pAd->Tx20MPwrCfgABand[2];
			TxPwr[3] = pAd->Tx20MPwrCfgABand[3];
			TxPwr[4] = pAd->Tx20MPwrCfgABand[4];
		}
		else
		{
			TxPwr[0] = pAd->Tx20MPwrCfgGBand[0];
			TxPwr[1] = pAd->Tx20MPwrCfgGBand[1];
			TxPwr[2] = pAd->Tx20MPwrCfgGBand[2];
			TxPwr[3] = pAd->Tx20MPwrCfgGBand[3];
			TxPwr[4] = pAd->Tx20MPwrCfgGBand[4];
		}
	}

	
	
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
	{
		if (pAd->ate.Channel <= 14)
		{
			
			bAutoTxAgc         = pAd->bAutoTxAgcG;
			TssiRef            = pAd->TssiRefG;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryG[0];
			pTssiPlusBoundary  = &pAd->TssiPlusBoundaryG[0];
			TxAgcStep          = pAd->TxAgcStepG;
			pTxAgcCompensate   = &pAd->TxAgcCompensateG;
		}
		else
		{
			
			bAutoTxAgc         = pAd->bAutoTxAgcA;
			TssiRef            = pAd->TssiRefA;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryA[0];
			pTssiPlusBoundary  = &pAd->TssiPlusBoundaryA[0];
			TxAgcStep          = pAd->TxAgcStepA;
			pTxAgcCompensate   = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
		{
			
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);

			
			
			

			
			
			

			
			
			

			if (BbpR49 > pTssiMinusBoundary[1])
			{
				
				
				for (idx = 1; idx < 5; idx++)
				{
					
					if (BbpR49 <= pTssiMinusBoundary[idx])
						break;
				}

				

					*pTxAgcCompensate = -(TxAgcStep * (idx-1));



				DeltaPwr += (*pTxAgcCompensate);
				ATEDBGPRINT(RT_DEBUG_TRACE, ("-- Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
					BbpR49, TssiRef, TxAgcStep, idx-1));
			}
			else if (BbpR49 < pTssiPlusBoundary[1])
			{
				
				
				for (idx = 1; idx < 5; idx++)
				{
					
					if (BbpR49 >= pTssiPlusBoundary[idx])
						break;
				}

				
				*pTxAgcCompensate = TxAgcStep * (idx-1);
				DeltaPwr += (*pTxAgcCompensate);
				ATEDBGPRINT(RT_DEBUG_TRACE, ("++ Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					BbpR49, TssiRef, TxAgcStep, idx-1));
			}
			else
			{
				*pTxAgcCompensate = 0;
				ATEDBGPRINT(RT_DEBUG_TRACE, ("   Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					BbpR49, TssiRef, TxAgcStep, 0));
			}
		}
	}
	else
	{
		if (pAd->ate.Channel <= 14)
		{
			bAutoTxAgc         = pAd->bAutoTxAgcG;
			pTxAgcCompensate   = &pAd->TxAgcCompensateG;
		}
		else
		{
			bAutoTxAgc         = pAd->bAutoTxAgcA;
			pTxAgcCompensate   = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
			DeltaPwr += (*pTxAgcCompensate);
	}

	
	
	
	if (pAd->CommonCfg.TxPowerPercentage == 0xffffffff)       
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 90)  
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 60)  
	{
		DeltaPwr -= 1;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 30)  
	{
		DeltaPwr -= 3;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 15)  
	{
		DeltaPwr -= 6;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 9)   
	{
		DeltaPwr -= 9;
	}
	else                                           
	{
		DeltaPwr -= 12;
	}

	
	for (i=0; i<5; i++)
	{
		if (TxPwr[i] != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F); 

				if ((Value + DeltaPwr) < 0)
				{
					Value = 0; 
				}
				else if ((Value + DeltaPwr) > 0xF)
				{
					Value = 0xF; 
				}
				else
				{
					Value += DeltaPwr; 
				}

				
				TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);
			}

			
			
			
			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, TxPwr[i]);


		}
	}

}



#ifdef RTMP_MAC_PCI
static VOID ATEWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	PTXWI_STRUC	pOutTxWI,
	IN	BOOLEAN			FRAG,
	IN	BOOLEAN			CFACK,
	IN	BOOLEAN			InsTimestamp,
	IN	BOOLEAN			AMPDU,
	IN	BOOLEAN			Ack,
	IN	BOOLEAN			NSeq,		
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN	UCHAR			PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,
	IN	BOOLEAN			CfAck,
	IN	HTTRANSMIT_SETTING	*pTransmit)
{
	TXWI_STRUC		TxWI;
	PTXWI_STRUC	pTxWI;

	
	
	
	
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(&TxWI, TXWI_SIZE);
	pTxWI = &TxWI;

	pTxWI->FRAG= FRAG;

	pTxWI->CFACK = CFACK;
	pTxWI->TS= InsTimestamp;
	pTxWI->AMPDU = AMPDU;
	pTxWI->ACK = Ack;
	pTxWI->txop= Txopmode;

	pTxWI->NSEQ = NSeq;

	
	if ( BASize >7 )
		BASize =7;

	pTxWI->BAWinSize = BASize;
	pTxWI->WirelessCliID = WCID;
	pTxWI->MPDUtotalByteCount = Length;
	pTxWI->PacketId = PID;

	
	pTxWI->BW = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
	pTxWI->ShortGI = pTransmit->field.ShortGI;
	pTxWI->STBC = pTransmit->field.STBC;

	pTxWI->MCS = pTransmit->field.MCS;
	pTxWI->PHYMODE = pTransmit->field.MODE;
	pTxWI->CFACK = CfAck;
	pTxWI->MIMOps = 0;
	pTxWI->MpduDensity = 0;

	pTxWI->PacketId = pTxWI->MCS;
	NdisMoveMemory(pOutTxWI, &TxWI, sizeof(TXWI_STRUC));

    return;
}
#endif 





VOID ATEDisableAsicProtect(
	IN		PRTMP_ADAPTER	pAd)
{
	PROT_CFG_STRUC	ProtCfg, ProtCfg4;
	UINT32 Protect[6];
	USHORT			offset;
	UCHAR			i;
	UINT32 MacReg = 0;

	
	RTMP_IO_READ32(pAd, TX_RTS_CFG, &MacReg);
	MacReg &= 0xFF0000FF;
	MacReg |= (pAd->CommonCfg.RtsThreshold << 8);
	RTMP_IO_WRITE32(pAd, TX_RTS_CFG, MacReg);

	
	RTMPZeroMemory(Protect, sizeof(Protect));
	ProtCfg4.word = 0;
	ProtCfg.word = 0;
	ProtCfg.field.TxopAllowGF40 = 1;
	ProtCfg.field.TxopAllowGF20 = 1;
	ProtCfg.field.TxopAllowMM40 = 1;
	ProtCfg.field.TxopAllowMM20 = 1;
	ProtCfg.field.TxopAllowOfdm = 1;
	ProtCfg.field.TxopAllowCck = 1;
	ProtCfg.field.RTSThEn = 1;
	ProtCfg.field.ProtectNav = ASIC_SHORTNAV;

	
	ProtCfg.field.ProtectRate = pAd->CommonCfg.RtsRate;
	ProtCfg.field.ProtectCtrl = 0;
	Protect[0] = ProtCfg.word;
	Protect[1] = ProtCfg.word;

	
	
	
	
	

	
	
	
	
	
	
	Protect[2] = 0x01744004;

	
	
	
	
	
	
	Protect[3] = 0x03f44084;

	
	
	
	
	
	
	Protect[4] = 0x01744004;

	
	
	
	
	
	
	Protect[5] = 0x03f44084;

	pAd->CommonCfg.IOTestParm.bRTSLongProtOn = FALSE;

	offset = CCK_PROT_CFG;
	for (i = 0;i < 6;i++)
		RTMP_IO_WRITE32(pAd, offset + i*4, Protect[i]);

}






CHAR ATEConvertToRssi(
	IN PRTMP_ADAPTER pAd,
	IN	CHAR	Rssi,
	IN  UCHAR   RssiNumber)
{
	UCHAR	RssiOffset, LNAGain;

	
	if (Rssi == 0)
		return -99;

	LNAGain = GET_LNA_GAIN(pAd);
	if (pAd->LatchRfRegs.Channel > 14)
	{
		if (RssiNumber == 0)
			RssiOffset = pAd->ARssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->ARssiOffset1;
		else
			RssiOffset = pAd->ARssiOffset2;
	}
	else
	{
		if (RssiNumber == 0)
			RssiOffset = pAd->BGRssiOffset0;
		else if (RssiNumber == 1)
			RssiOffset = pAd->BGRssiOffset1;
		else
			RssiOffset = pAd->BGRssiOffset2;
	}

	return (-12 - RssiOffset - LNAGain - Rssi);
}



static VOID SetJapanFilter(
	IN		PRTMP_ADAPTER	pAd)
{
	UCHAR			BbpData = 0;

	
	
	
	
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BbpData);

    if ((pAd->ate.TxWI.PHYMODE == MODE_CCK) && (pAd->ate.Channel == 14) && (pAd->ate.TxWI.BW == BW_20))
    {
        BbpData |= 0x20;    
        ATEDBGPRINT(RT_DEBUG_TRACE, ("SetJapanFilter!!!\n"));
    }
    else
    {
		BbpData &= 0xdf;    
		ATEDBGPRINT(RT_DEBUG_TRACE, ("ClearJapanFilter!!!\n"));
    }

	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpData);
}


VOID ATESampleRssi(
	IN PRTMP_ADAPTER	pAd,
	IN PRXWI_STRUC		pRxWI)
{
	

	if (pRxWI->RSSI0 != 0)
	{
		pAd->ate.LastRssi0	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI0, RSSI_0);
		pAd->ate.AvgRssi0X8	= (pAd->ate.AvgRssi0X8 - pAd->ate.AvgRssi0) + pAd->ate.LastRssi0;
		pAd->ate.AvgRssi0	= pAd->ate.AvgRssi0X8 >> 3;
	}
	if (pRxWI->RSSI1 != 0)
	{
		pAd->ate.LastRssi1	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI1, RSSI_1);
		pAd->ate.AvgRssi1X8	= (pAd->ate.AvgRssi1X8 - pAd->ate.AvgRssi1) + pAd->ate.LastRssi1;
		pAd->ate.AvgRssi1	= pAd->ate.AvgRssi1X8 >> 3;
	}
	if (pRxWI->RSSI2 != 0)
	{
		pAd->ate.LastRssi2	= ATEConvertToRssi(pAd, (CHAR) pRxWI->RSSI2, RSSI_2);
		pAd->ate.AvgRssi2X8	= (pAd->ate.AvgRssi2X8 - pAd->ate.AvgRssi2) + pAd->ate.LastRssi2;
		pAd->ate.AvgRssi2	= pAd->ate.AvgRssi2X8 >> 3;
	}

	pAd->ate.LastSNR0 = (CHAR)(pRxWI->SNR0);
	pAd->ate.LastSNR1 = (CHAR)(pRxWI->SNR1);

	pAd->ate.NumOfAvgRssiSample ++;
}


#ifdef CONFIG_STA_SUPPORT
VOID RTMPStationStop(
    IN  PRTMP_ADAPTER   pAd)
{


    ATEDBGPRINT(RT_DEBUG_TRACE, ("==> RTMPStationStop\n"));

	


    ATEDBGPRINT(RT_DEBUG_TRACE, ("<== RTMPStationStop\n"));
}


VOID RTMPStationStart(
    IN  PRTMP_ADAPTER   pAd)
{
    ATEDBGPRINT(RT_DEBUG_TRACE, ("==> RTMPStationStart\n"));

#ifdef RTMP_MAC_PCI
	pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;

	

#endif 

	ATEDBGPRINT(RT_DEBUG_TRACE, ("<== RTMPStationStart\n"));
}
#endif 



#ifdef RTMP_MAC_PCI
static INT ATESetUpFrame(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 TxIdx)
{
	UINT j;
	PTXD_STRUC pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
    TXD_STRUC       TxD;
#endif
	PNDIS_PACKET pPacket;
	PUCHAR pDest;
	PVOID AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	HTTRANSMIT_SETTING	TxHTPhyMode;

	PRTMP_TX_RING pTxRing = &pAd->TxRing[QID_AC_BE];
	PTXWI_STRUC pTxWI = (PTXWI_STRUC) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	PUCHAR pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;

#ifdef RALINK_28xx_QA
	PHEADER_802_11	pHeader80211;
#endif 

	if (pAd->ate.bQATxStart == TRUE)
	{
		

		
		TxHTPhyMode.field.BW = pAd->ate.TxWI.BW;
		TxHTPhyMode.field.ShortGI = pAd->ate.TxWI.ShortGI;
		TxHTPhyMode.field.STBC = 0;
		TxHTPhyMode.field.MCS = pAd->ate.TxWI.MCS;
		TxHTPhyMode.field.MODE = pAd->ate.TxWI.PHYMODE;

		ATEWriteTxWI(pAd, pTxWI, pAd->ate.TxWI.FRAG, pAd->ate.TxWI.CFACK,
			pAd->ate.TxWI.TS,  pAd->ate.TxWI.AMPDU, pAd->ate.TxWI.ACK, pAd->ate.TxWI.NSEQ,
			pAd->ate.TxWI.BAWinSize, 0, pAd->ate.TxWI.MPDUtotalByteCount, pAd->ate.TxWI.PacketId, 0, 0,
			pAd->ate.TxWI.txop, pAd->ate.TxWI.CFACK, &TxHTPhyMode);
	}
	else
	{
		TxHTPhyMode.field.BW = pAd->ate.TxWI.BW;
		TxHTPhyMode.field.ShortGI = pAd->ate.TxWI.ShortGI;
		TxHTPhyMode.field.STBC = 0;
		TxHTPhyMode.field.MCS = pAd->ate.TxWI.MCS;
		TxHTPhyMode.field.MODE = pAd->ate.TxWI.PHYMODE;
		ATEWriteTxWI(pAd, pTxWI, FALSE, FALSE, FALSE,  FALSE, FALSE, FALSE,
			4, 0, pAd->ate.TxLength, 0, 0, 0, IFS_HTTXOP, FALSE, &TxHTPhyMode);
	}

	
#ifdef RALINK_28xx_QA
	if (pAd->ate.bQATxStart == TRUE)
	{
		NdisMoveMemory(pDMAHeaderBufVA+TXWI_SIZE, pAd->ate.Header, pAd->ate.HLen);
	}
	else
#endif 
	{
		NdisMoveMemory(pDMAHeaderBufVA+TXWI_SIZE, TemplateFrame, LENGTH_802_11);
		NdisMoveMemory(pDMAHeaderBufVA+TXWI_SIZE+4, pAd->ate.Addr1, ETH_LENGTH_OF_ADDRESS);
		NdisMoveMemory(pDMAHeaderBufVA+TXWI_SIZE+10, pAd->ate.Addr2, ETH_LENGTH_OF_ADDRESS);
		NdisMoveMemory(pDMAHeaderBufVA+TXWI_SIZE+16, pAd->ate.Addr3, ETH_LENGTH_OF_ADDRESS);
	}

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA)+TXWI_SIZE), DIR_READ, FALSE);
#endif 

	
#ifdef RALINK_28xx_QA
	if (pAd->ate.bQATxStart == TRUE)
	{
		pPacket = RTMP_AllocateRxPacketBuffer(pAd, pAd->ate.DLen + 0x100, FALSE, &AllocVa, &AllocPa);
	}
	else
#endif 
	{
		pPacket = RTMP_AllocateRxPacketBuffer(pAd, pAd->ate.TxLength, FALSE, &AllocVa, &AllocPa);
	}

	if (pPacket == NULL)
	{
		pAd->ate.TxCount = 0;
		ATEDBGPRINT(RT_DEBUG_TRACE, ("%s fail to alloc packet space.\n", __FUNCTION__));
		return -1;
	}
	pTxRing->Cell[TxIdx].pNextNdisPacket = pPacket;

	pDest = (PUCHAR) AllocVa;

#ifdef RALINK_28xx_QA
	if (pAd->ate.bQATxStart == TRUE)
	{
		RTPKT_TO_OSPKT(pPacket)->len = pAd->ate.DLen;
	}
	else
#endif 
	{
		RTPKT_TO_OSPKT(pPacket)->len = pAd->ate.TxLength - LENGTH_802_11;
	}

	
#ifdef RALINK_28xx_QA
	if (pAd->ate.bQATxStart == TRUE)
	{
		
		if ((pAd->ate.PLen != 0))
		{
			int j;

			for (j = 0; j < pAd->ate.DLen; j+=pAd->ate.PLen)
			{
				memcpy(RTPKT_TO_OSPKT(pPacket)->data + j, pAd->ate.Pattern, pAd->ate.PLen);
			}
		}
	}
	else
#endif 
	{
		for (j = 0; j < RTPKT_TO_OSPKT(pPacket)->len; j++)
		{
			pDest[j] = 0xA5;
		}
	}

	
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
#else
    pDestTxD  = (PTXD_STRUC)pTxRing->Cell[TxIdx].AllocVa;
    TxD = *pDestTxD;
    pTxD = &TxD;
#endif 

#ifdef RALINK_28xx_QA
	if (pAd->ate.bQATxStart == TRUE)
	{
		
		NdisZeroMemory(pTxD, TXD_SIZE);
		RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);
		
		pTxD->SDPtr0 = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
		pTxD->SDLen0 = TXWI_SIZE + pAd->ate.HLen;
		pTxD->LastSec0 = 0;
		pTxD->SDPtr1 = AllocPa;
		pTxD->SDLen1 = RTPKT_TO_OSPKT(pPacket)->len;
		pTxD->LastSec1 = 1;

		pDest = (PUCHAR)pTxWI;
		pDest += TXWI_SIZE;
		pHeader80211 = (PHEADER_802_11)pDest;

		
		if (pAd->ate.TxDoneCount == 0)
		{
			pAd->ate.seq = pHeader80211->Sequence;
		}
		else
			pHeader80211->Sequence = ++pAd->ate.seq;
	}
	else
#endif 
	{
		NdisZeroMemory(pTxD, TXD_SIZE);
		RTMPWriteTxDescriptor(pAd, pTxD, FALSE, FIFO_EDCA);
		
		pTxD->SDPtr0 = RTMP_GetPhysicalAddressLow (pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
		pTxD->SDLen0 = TXWI_SIZE + LENGTH_802_11;
		pTxD->LastSec0 = 0;
		pTxD->SDPtr1 = AllocPa;
		pTxD->SDLen1 = RTPKT_TO_OSPKT(pPacket)->len;
		pTxD->LastSec1 = 1;
	}

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange((PUCHAR)pTxWI, TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA)+TXWI_SIZE), DIR_WRITE, FALSE);
    RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
    WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif 

	return 0;
}

#endif 




VOID rt_ee_read_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	USHORT i;
	USHORT value;


	for (i = 0 ; i < EEPROM_SIZE/2 ; )
	{
		
		RT28xx_EEPROM_READ16(pAd, i*2, value);
		Data[i] = value;
		i++;
	}
}


VOID rt_ee_write_all(PRTMP_ADAPTER pAd, USHORT *Data)
{
	USHORT i;
	USHORT value;


	for (i = 0 ; i < EEPROM_SIZE/2 ; )
	{
		
		value = Data[i];
		RT28xx_EEPROM_WRITE16(pAd, i*2, value);
		i++;
	}
}


#ifdef RALINK_28xx_QA
VOID ATE_QA_Statistics(
	IN PRTMP_ADAPTER			pAd,
	IN PRXWI_STRUC				pRxWI,
	IN PRT28XX_RXD_STRUC		pRxD,
	IN PHEADER_802_11			pHeader)
{
	
	if (pHeader != NULL)
	{
		if (pHeader->FC.Type == BTYPE_DATA)
		{
			if (pRxD->U2M)
				pAd->ate.U2M++;
			else
				pAd->ate.OtherData++;
		}
		else if (pHeader->FC.Type == BTYPE_MGMT)
		{
			if (pHeader->FC.SubType == SUBTYPE_BEACON)
				pAd->ate.Beacon++;
			else
				pAd->ate.OtherCount++;
		}
		else if (pHeader->FC.Type == BTYPE_CNTL)
		{
			pAd->ate.OtherCount++;
		}
	}
	pAd->ate.RSSI0 = pRxWI->RSSI0;
	pAd->ate.RSSI1 = pRxWI->RSSI1;
	pAd->ate.RSSI2 = pRxWI->RSSI2;
	pAd->ate.SNR0 = pRxWI->SNR0;
	pAd->ate.SNR1 = pRxWI->SNR1;
}



#define RACFG_CMD_RF_WRITE_ALL		0x0000
#define RACFG_CMD_E2PROM_READ16		0x0001
#define RACFG_CMD_E2PROM_WRITE16	0x0002
#define RACFG_CMD_E2PROM_READ_ALL	0x0003
#define RACFG_CMD_E2PROM_WRITE_ALL	0x0004
#define RACFG_CMD_IO_READ			0x0005
#define RACFG_CMD_IO_WRITE			0x0006
#define RACFG_CMD_IO_READ_BULK		0x0007
#define RACFG_CMD_BBP_READ8			0x0008
#define RACFG_CMD_BBP_WRITE8		0x0009
#define RACFG_CMD_BBP_READ_ALL		0x000a
#define RACFG_CMD_GET_COUNTER		0x000b
#define RACFG_CMD_CLEAR_COUNTER		0x000c

#define RACFG_CMD_RSV1				0x000d
#define RACFG_CMD_RSV2				0x000e
#define RACFG_CMD_RSV3				0x000f

#define RACFG_CMD_TX_START			0x0010
#define RACFG_CMD_GET_TX_STATUS		0x0011
#define RACFG_CMD_TX_STOP			0x0012
#define RACFG_CMD_RX_START			0x0013
#define RACFG_CMD_RX_STOP			0x0014
#define RACFG_CMD_GET_NOISE_LEVEL	0x0015

#define RACFG_CMD_ATE_START			0x0080
#define RACFG_CMD_ATE_STOP			0x0081

#define RACFG_CMD_ATE_START_TX_CARRIER		0x0100
#define RACFG_CMD_ATE_START_TX_CONT			0x0101
#define RACFG_CMD_ATE_START_TX_FRAME		0x0102
#define RACFG_CMD_ATE_SET_BW	            0x0103
#define RACFG_CMD_ATE_SET_TX_POWER0	        0x0104
#define RACFG_CMD_ATE_SET_TX_POWER1			0x0105
#define RACFG_CMD_ATE_SET_FREQ_OFFSET		0x0106
#define RACFG_CMD_ATE_GET_STATISTICS		0x0107
#define RACFG_CMD_ATE_RESET_COUNTER			0x0108
#define RACFG_CMD_ATE_SEL_TX_ANTENNA		0x0109
#define RACFG_CMD_ATE_SEL_RX_ANTENNA		0x010a
#define RACFG_CMD_ATE_SET_PREAMBLE			0x010b
#define RACFG_CMD_ATE_SET_CHANNEL			0x010c
#define RACFG_CMD_ATE_SET_ADDR1				0x010d
#define RACFG_CMD_ATE_SET_ADDR2				0x010e
#define RACFG_CMD_ATE_SET_ADDR3				0x010f
#define RACFG_CMD_ATE_SET_RATE				0x0110
#define RACFG_CMD_ATE_SET_TX_FRAME_LEN		0x0111
#define RACFG_CMD_ATE_SET_TX_FRAME_COUNT	0x0112
#define RACFG_CMD_ATE_START_RX_FRAME		0x0113
#define RACFG_CMD_ATE_E2PROM_READ_BULK	0x0114
#define RACFG_CMD_ATE_E2PROM_WRITE_BULK	0x0115
#define RACFG_CMD_ATE_IO_WRITE_BULK		0x0116
#define RACFG_CMD_ATE_BBP_READ_BULK		0x0117
#define RACFG_CMD_ATE_BBP_WRITE_BULK	0x0118
#define RACFG_CMD_ATE_RF_READ_BULK		0x0119
#define RACFG_CMD_ATE_RF_WRITE_BULK		0x011a


static VOID memcpy_exl(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len);
static VOID memcpy_exs(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len);
static VOID RTMP_IO_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, UINT32 len);



VOID RtmpDoAte(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq)
{
	USHORT Command_Id;
	INT	Status = NDIS_STATUS_SUCCESS;
	struct ate_racfghdr *pRaCfg;


	if ((pRaCfg = kmalloc(sizeof(struct ate_racfghdr), GFP_KERNEL)) == NULL)
	{
		Status = -EINVAL;
		return;
	}

	NdisZeroMemory(pRaCfg, sizeof(struct ate_racfghdr));

    if (copy_from_user((PUCHAR)pRaCfg, wrq->u.data.pointer, wrq->u.data.length))
	{
		Status = -EFAULT;
		kfree(pRaCfg);
		return;
	}

	Command_Id = ntohs(pRaCfg->command_id);

	ATEDBGPRINT(RT_DEBUG_TRACE,("\n%s: Command_Id = 0x%04x !\n", __FUNCTION__, Command_Id));

	switch (Command_Id)
	{
		
		case RACFG_CMD_ATE_START:
			Status=DO_RACFG_CMD_ATE_START(pAdapter,wrq,pRaCfg);
			break;

		
		case RACFG_CMD_ATE_STOP:
			Status=DO_RACFG_CMD_ATE_STOP(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_RF_WRITE_ALL:
			Status=DO_RACFG_CMD_RF_WRITE_ALL(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_E2PROM_READ16:
			Status=DO_RACFG_CMD_E2PROM_READ16(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_E2PROM_WRITE16:
			Status=DO_RACFG_CMD_E2PROM_WRITE16(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_E2PROM_READ_ALL:
			Status=DO_RACFG_CMD_E2PROM_READ_ALL(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_E2PROM_WRITE_ALL:
			Status=DO_RACFG_CMD_E2PROM_WRITE_ALL(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_IO_READ:
			Status=DO_RACFG_CMD_IO_READ(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_IO_WRITE:
			Status=DO_RACFG_CMD_IO_WRITE(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_IO_READ_BULK:
			Status=DO_RACFG_CMD_IO_READ_BULK(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_BBP_READ8:
			Status=DO_RACFG_CMD_BBP_READ8(pAdapter,wrq,pRaCfg);
			break;
		case RACFG_CMD_BBP_WRITE8:
			Status=DO_RACFG_CMD_BBP_WRITE8(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_BBP_READ_ALL:
			Status=DO_RACFG_CMD_BBP_READ_ALL(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_E2PROM_READ_BULK:
			Status=DO_RACFG_CMD_ATE_E2PROM_READ_BULK(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_E2PROM_WRITE_BULK:
			Status=DO_RACFG_CMD_ATE_E2PROM_WRITE_BULK(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_IO_WRITE_BULK:
			Status=DO_RACFG_CMD_ATE_IO_WRITE_BULK(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_BBP_READ_BULK:
			Status=DO_RACFG_CMD_ATE_BBP_READ_BULK(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_BBP_WRITE_BULK:
			Status=DO_RACFG_CMD_ATE_BBP_WRITE_BULK(pAdapter,wrq,pRaCfg);
			break;


		case RACFG_CMD_GET_NOISE_LEVEL:
			Status=DO_RACFG_CMD_GET_NOISE_LEVEL(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_GET_COUNTER:
			Status=DO_RACFG_CMD_GET_COUNTER(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_CLEAR_COUNTER:
			Status=DO_RACFG_CMD_CLEAR_COUNTER(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_TX_START:
			Status=DO_RACFG_CMD_TX_START(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_GET_TX_STATUS:
			Status=DO_RACFG_CMD_GET_TX_STATUS(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_TX_STOP:
			Status=DO_RACFG_CMD_TX_STOP(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_RX_START:
			Status=DO_RACFG_CMD_RX_START(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_RX_STOP:
			Status=DO_RACFG_CMD_RX_STOP(pAdapter,wrq,pRaCfg);
			break;

		
		
		case RACFG_CMD_ATE_START_TX_CARRIER:
			Status=DO_RACFG_CMD_ATE_START_TX_CARRIER(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_START_TX_CONT:
			Status=DO_RACFG_CMD_ATE_START_TX_CONT(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_START_TX_FRAME:
			Status=DO_RACFG_CMD_ATE_START_TX_FRAME(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_BW:
			Status=DO_RACFG_CMD_ATE_SET_BW(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_TX_POWER0:
			Status=DO_RACFG_CMD_ATE_SET_TX_POWER0(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_TX_POWER1:
			Status=DO_RACFG_CMD_ATE_SET_TX_POWER1(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_FREQ_OFFSET:
			Status=DO_RACFG_CMD_ATE_SET_TX_POWER1(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_GET_STATISTICS:
			Status=DO_RACFG_CMD_ATE_GET_STATISTICS(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_RESET_COUNTER:
			Status=DO_RACFG_CMD_ATE_RESET_COUNTER(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SEL_TX_ANTENNA:
			Status=DO_RACFG_CMD_ATE_SEL_TX_ANTENNA(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SEL_RX_ANTENNA:
			Status=DO_RACFG_CMD_ATE_SEL_TX_ANTENNA(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_PREAMBLE:
			Status=DO_RACFG_CMD_ATE_SET_PREAMBLE(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_CHANNEL:
			Status=DO_RACFG_CMD_ATE_SET_CHANNEL(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_ADDR1:
			Status=DO_RACFG_CMD_ATE_SET_ADDR1(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_ADDR2:
			Status=DO_RACFG_CMD_ATE_SET_ADDR2(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_ADDR3:
			Status=DO_RACFG_CMD_ATE_SET_ADDR3(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_RATE:
			Status=DO_RACFG_CMD_ATE_SET_RATE(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_TX_FRAME_LEN:
			Status=DO_RACFG_CMD_ATE_SET_TX_FRAME_LEN(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_SET_TX_FRAME_COUNT:
			Status=DO_RACFG_CMD_ATE_SET_TX_FRAME_COUNT(pAdapter,wrq,pRaCfg);
			break;

		case RACFG_CMD_ATE_START_RX_FRAME:
			Status=DO_RACFG_CMD_ATE_START_RX_FRAME(pAdapter,wrq,pRaCfg);
			break;
		default:
			break;
	}

    ASSERT(pRaCfg != NULL);

    if (pRaCfg != NULL)
	    kfree(pRaCfg);

	return;
}


VOID BubbleSort(INT32 n, INT32 a[])
{
	INT32 k, j, temp;

	for (k = n-1;  k>0;  k--)
	{
		for (j = 0; j<k; j++)
		{
			if (a[j] > a[j+1])
			{
				temp = a[j];
				a[j]=a[j+1];
				a[j+1]=temp;
			}
		}
	}
}


VOID CalNoiseLevel(PRTMP_ADAPTER pAd, UCHAR channel, INT32 RSSI[3][10])
{
	INT32		RSSI0, RSSI1, RSSI2;
	CHAR		Rssi0Offset, Rssi1Offset, Rssi2Offset;
	UCHAR		BbpR50Rssi0 = 0, BbpR51Rssi1 = 0, BbpR52Rssi2 = 0;
	UCHAR		Org_BBP66value = 0, Org_BBP69value = 0, Org_BBP70value = 0, data = 0;
	USHORT		LNA_Gain = 0;
	INT32       j = 0;
	UCHAR		Org_Channel = pAd->ate.Channel;
	USHORT	    GainValue = 0, OffsetValue = 0;

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R66, &Org_BBP66value);
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R69, &Org_BBP69value);
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R70, &Org_BBP70value);

	
	
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET, GainValue);

	
	if (channel <= 14)
	{
		LNA_Gain = GainValue & 0x00FF;

		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET, OffsetValue);
		Rssi0Offset = OffsetValue & 0x00FF;
		Rssi1Offset = (OffsetValue & 0xFF00) >> 8;
		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_BG_OFFSET + 2), OffsetValue);
		Rssi2Offset = OffsetValue & 0x00FF;
	}
	else
	{
		LNA_Gain = (GainValue & 0xFF00) >> 8;

		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_A_OFFSET, OffsetValue);
		Rssi0Offset = OffsetValue & 0x00FF;
		Rssi1Offset = (OffsetValue & 0xFF00) >> 8;
		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_A_OFFSET + 2), OffsetValue);
		Rssi2Offset = OffsetValue & 0x00FF;
	}
	
	{
		pAd->ate.Channel = channel;
		ATEAsicSwitchChannel(pAd);
		mdelay(5);

		data = 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, data);
		data = 0x40;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, data);
		data = 0x40;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, data);
		mdelay(5);

		
		pAd->ate.bQARxStart = TRUE;
		Set_ATE_Proc(pAd, "RXFRAME");

		mdelay(5);

		for (j = 0; j < 10; j++)
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R50, &BbpR50Rssi0);
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R51, &BbpR51Rssi1);
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R52, &BbpR52Rssi2);

			mdelay(10);

			
			if (BbpR50Rssi0 == 0)
			{
				RSSI0 = -100;
			}
			else
			{
				RSSI0 = (INT32)(-12 - BbpR50Rssi0 - LNA_Gain - Rssi0Offset);
			}
			RSSI[0][j] = RSSI0;

			if ( pAd->Antenna.field.RxPath >= 2 ) 
			{
				
				if (BbpR51Rssi1 == 0)
				{
					RSSI1 = -100;
				}
				else
				{
					RSSI1 = (INT32)(-12 - BbpR51Rssi1 - LNA_Gain - Rssi1Offset);
				}
				RSSI[1][j] = RSSI1;
			}

			if ( pAd->Antenna.field.RxPath >= 3 ) 
			{
				
				if (BbpR52Rssi2 == 0)
					RSSI2 = -100;
				else
					RSSI2 = (INT32)(-12 - BbpR52Rssi2 - LNA_Gain - Rssi2Offset);

				RSSI[2][j] = RSSI2;
			}
		}

		
		Set_ATE_Proc(pAd, "RXSTOP");

		mdelay(5);

		BubbleSort(10, RSSI[0]);	

		if ( pAd->Antenna.field.RxPath >= 2 ) 
		{
			BubbleSort(10, RSSI[1]);
		}

		if ( pAd->Antenna.field.RxPath >= 3 ) 
		{
			BubbleSort(10, RSSI[2]);
		}
	}

	pAd->ate.Channel = Org_Channel;
	ATEAsicSwitchChannel(pAd);

	
    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R66, Org_BBP66value);
    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, Org_BBP69value);
    ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, Org_BBP70value);

	return;
}


BOOLEAN SyncTxRxConfig(PRTMP_ADAPTER pAd, USHORT offset, UCHAR value)
{
	UCHAR tmp = 0, bbp_data = 0;

	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, offset, &bbp_data);
	}
	else
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, offset, &bbp_data);
	}

	
	ASSERT(bbp_data == value);

	switch (offset)
	{
		case BBP_R1:
			
			tmp = (bbp_data & ((1 << 4) | (1 << 3))) >> 3;
		    switch (tmp)
		    {
				
		        case 2:
					
					pAd->ate.TxAntennaSel = 0;
		            break;
				
		        case 0:
					
					pAd->ate.TxAntennaSel = 1;
		            break;
				
		        case 1:
					
					pAd->ate.TxAntennaSel = 2;
		            break;
		        default:
		            DBGPRINT(RT_DEBUG_TRACE, ("%s -- Sth. wrong!  : return FALSE; \n", __FUNCTION__));
		            return FALSE;
		    }
			break;

		case BBP_R3:
			
			tmp = (bbp_data & ((1 << 1) | (1 << 0)));
		    switch(tmp)
		    {
				
		        case 3:
					
					pAd->ate.RxAntennaSel = 0;
		            break;
				
		        case 0:
					
					pAd->ate.RxAntennaSel = 1;
					tmp = ((bbp_data & ((1 << 4) | (1 << 3))) >> 3);
					if (tmp == 2)
					{
						
						pAd->ate.RxAntennaSel = 0;
					}
		            break;
				
		        case 1:
					
					pAd->ate.RxAntennaSel = 2;
		            break;
				
		        case 2:
					
					pAd->ate.RxAntennaSel = 3;
		            break;
		        default:
		            DBGPRINT(RT_DEBUG_ERROR, ("%s -- Impossible!  : return FALSE; \n", __FUNCTION__));
		            return FALSE;
		    }
			break;

        default:
            DBGPRINT(RT_DEBUG_ERROR, ("%s -- Sth. wrong!  : return FALSE; \n", __FUNCTION__));
            return FALSE;

	}
	return TRUE;
}


static VOID memcpy_exl(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len)
{
	ULONG i, Value = 0;
	ULONG *pDst, *pSrc;
	UCHAR *p8;

	p8 = src;
	pDst = (ULONG *) dst;
	pSrc = (ULONG *) src;

	for (i = 0 ; i < (len/4); i++)
	{
		
		memmove(&Value, pSrc, 4);
		Value = htonl(Value);
		memmove(pDst, &Value, 4);
		pDst++;
		pSrc++;
	}
	if ((len % 4) != 0)
	{
		
		memmove(&Value, pSrc, (len % 4));
		Value = htonl(Value);
		memmove(pDst, &Value, (len % 4));
	}
}


static VOID memcpy_exs(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len)
{
	ULONG i;
	UCHAR *pDst, *pSrc;

	pDst = dst;
	pSrc = src;

	for (i = 0; i < (len/2); i++)
	{
		memmove(pDst, pSrc, 2);
		*((USHORT *)pDst) = htons(*((USHORT *)pDst));
		pDst+=2;
		pSrc+=2;
	}

	if ((len % 2) != 0)
	{
		memmove(pDst, pSrc, 1);
	}
}


static VOID RTMP_IO_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, UINT32 len)
{
	UINT32 i, Value;
	UINT32 *pDst, *pSrc;

	pDst = (UINT32 *) dst;
	pSrc = (UINT32 *) src;

	for (i = 0 ; i < (len/4); i++)
	{
		RTMP_IO_READ32(pAd, (ULONG)pSrc, &Value);
		Value = htonl(Value);
		memmove(pDst, &Value, 4);
		pDst++;
		pSrc++;
	}
	return;
}


INT Set_TxStop_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("Set_TxStop_Proc\n"));

	if (Set_ATE_Proc(pAd, "TXSTOP"))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


INT Set_RxStop_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("Set_RxStop_Proc\n"));

	if (Set_ATE_Proc(pAd, "RXSTOP"))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


#ifdef DBG
INT Set_EERead_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	USHORT buffer[EEPROM_SIZE/2];
	USHORT *p;
	INT i;

	rt_ee_read_all(pAd, (USHORT *)buffer);
	p = buffer;

	for (i = 0; i < (EEPROM_SIZE/2); i++)
	{
		ate_print(KERN_EMERG "%4.4x ", *p);
		if (((i+1) % 16) == 0)
			ate_print(KERN_EMERG "\n");
		p++;
	}

	return TRUE;
}


INT Set_EEWrite_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	USHORT offset = 0, value;
	PSTRING p2 = arg;

	while ((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}

	if (*p2 == ':')
	{
		A2Hex(offset, arg);
		A2Hex(value, p2 + 1);
	}
	else
	{
		A2Hex(value, arg);
	}

	if (offset >= EEPROM_SIZE)
	{
		ate_print(KERN_EMERG "Offset can not exceed EEPROM_SIZE( == 0x%04x)\n", EEPROM_SIZE);
		return FALSE;
	}

	RT28xx_EEPROM_WRITE16(pAd, offset, value);

	return TRUE;
}


INT Set_BBPRead_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR value = 0, offset;

	A2Hex(offset, arg);

	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, offset,  &value);
	}
	else
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, offset,  &value);
	}

	ate_print(KERN_EMERG "%x\n", value);

	return TRUE;
}


INT Set_BBPWrite_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	USHORT offset = 0;
	PSTRING p2 = arg;
	UCHAR value;

	while ((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}

	if (*p2 == ':')
	{
		A2Hex(offset, arg);
		A2Hex(value, p2 + 1);
	}
	else
	{
		A2Hex(value, arg);
	}

	if (ATE_ON(pAd))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, offset,  value);
	}
	else
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, offset,  value);
	}

	return TRUE;
}


INT Set_RFWrite_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	PSTRING p2, p3, p4;
	UINT32 R1, R2, R3, R4;

	p2 = arg;

	while ((*p2 != ':') && (*p2 != '\0'))
	{
		p2++;
	}

	if (*p2 != ':')
		return FALSE;

	p3 = p2 + 1;

	while((*p3 != ':') && (*p3 != '\0'))
	{
		p3++;
	}

	if (*p3 != ':')
		return FALSE;

	p4 = p3 + 1;

	while ((*p4 != ':') && (*p4 != '\0'))
	{
		p4++;
	}

	if (*p4 != ':')
		return FALSE;


	A2Hex(R1, arg);
	A2Hex(R2, p2 + 1);
	A2Hex(R3, p3 + 1);
	A2Hex(R4, p4 + 1);

	RTMP_RF_IO_WRITE32(pAd, R1);
	RTMP_RF_IO_WRITE32(pAd, R2);
	RTMP_RF_IO_WRITE32(pAd, R3);
	RTMP_RF_IO_WRITE32(pAd, R4);

	return TRUE;
}
#endif 
#endif 




#ifdef RALINK_28xx_QA
#define	LEN_OF_ARG 16

#define RESPONSE_TO_GUI(__pRaCfg, __pwrq, __Length, __Status)									\
	(__pRaCfg)->length = htons((__Length));														\
	(__pRaCfg)->status = htons((__Status));														\
	(__pwrq)->u.data.length = sizeof((__pRaCfg)->magic_no) + sizeof((__pRaCfg)->command_type)	\
							+ sizeof((__pRaCfg)->command_id) + sizeof((__pRaCfg)->length)		\
							+ sizeof((__pRaCfg)->sequence) + ntohs((__pRaCfg)->length);			\
	ATEDBGPRINT(RT_DEBUG_TRACE, ("wrq->u.data.length = %d\n", (__pwrq)->u.data.length));		\
	if (copy_to_user((__pwrq)->u.data.pointer, (UCHAR *)(__pRaCfg), (__pwrq)->u.data.length))	\
	{																							\
		ATEDBGPRINT(RT_DEBUG_ERROR, ("copy_to_user() fail in %s\n", __FUNCTION__));				\
		return (-EFAULT);																		\
	}																							\
	else																						\
	{																							\
		ATEDBGPRINT(RT_DEBUG_TRACE, ("%s is done !\n", __FUNCTION__));							\
	}

static inline INT	DO_RACFG_CMD_ATE_START(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START\n"));

	
	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
	Set_ATE_Proc(pAdapter, "ATESTART");

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_STOP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	INT32 ret;

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_STOP\n"));

	
	pRaCfg->length = ntohs(pRaCfg->length);

	if (pRaCfg->length == sizeof(pAdapter->ate.AtePid))
	{
		
		memcpy((UCHAR *)&pAdapter->ate.AtePid,
						(&pRaCfg->data[0]) - 2,
						sizeof(pAdapter->ate.AtePid));

		
		RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

		
		ret = KILL_THREAD_PID(pAdapter->ate.AtePid, SIGTERM, 1);

		if (ret)
		{
			ATEDBGPRINT(RT_DEBUG_ERROR, ("%s: unable to kill ate thread\n", pAdapter->net_dev->name));
		}
	}


	
	if (ATE_ON(pAdapter))
	{
		
		Set_ATE_Proc(pAdapter, "ATESTOP");
		ATEDBGPRINT(RT_DEBUG_TRACE, ("RACFG_CMD_AP_START is done !\n"));
	}

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_RF_WRITE_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 R1, R2, R3, R4;
	USHORT channel;

	memcpy(&R1, pRaCfg->data-2, 4);
	memcpy(&R2, pRaCfg->data+2, 4);
	memcpy(&R3, pRaCfg->data+6, 4);
	memcpy(&R4, pRaCfg->data+10, 4);
	memcpy(&channel, pRaCfg->data+14, 2);

	pAdapter->LatchRfRegs.R1 = ntohl(R1);
	pAdapter->LatchRfRegs.R2 = ntohl(R2);
	pAdapter->LatchRfRegs.R3 = ntohl(R3);
	pAdapter->LatchRfRegs.R4 = ntohl(R4);
	pAdapter->LatchRfRegs.Channel = ntohs(channel);

	RTMP_RF_IO_WRITE32(pAdapter, pAdapter->LatchRfRegs.R1);
	RTMP_RF_IO_WRITE32(pAdapter, pAdapter->LatchRfRegs.R2);
	RTMP_RF_IO_WRITE32(pAdapter, pAdapter->LatchRfRegs.R3);
	RTMP_RF_IO_WRITE32(pAdapter, pAdapter->LatchRfRegs.R4);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return  NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_E2PROM_READ16(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT16	offset=0, value=0;
	USHORT  tmp=0;

	offset = ntohs(pRaCfg->status);

	
	RT28xx_EEPROM_READ16(pAdapter, offset, tmp);
	value = tmp;
	value = htons(value);

	ATEDBGPRINT(RT_DEBUG_TRACE,("EEPROM Read offset = 0x%04x, value = 0x%04x\n", offset, value));
	memcpy(pRaCfg->data, &value, 2);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+2, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_E2PROM_WRITE16(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset, value;

	offset = ntohs(pRaCfg->status);
	memcpy(&value, pRaCfg->data, 2);
	value = ntohs(value);
	RT28xx_EEPROM_WRITE16(pAdapter, offset, value);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_E2PROM_READ_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT buffer[EEPROM_SIZE/2];

	rt_ee_read_all(pAdapter,(USHORT *)buffer);
	memcpy_exs(pAdapter, pRaCfg->data, (UCHAR *)buffer, EEPROM_SIZE);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+EEPROM_SIZE, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_E2PROM_WRITE_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT buffer[EEPROM_SIZE/2];

	NdisZeroMemory((UCHAR *)buffer, EEPROM_SIZE);
	memcpy_exs(pAdapter, (UCHAR *)buffer, (UCHAR *)&pRaCfg->status, EEPROM_SIZE);
	rt_ee_write_all(pAdapter,(USHORT *)buffer);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_IO_READ(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset;
	UINT32	value;

	memcpy(&offset, &pRaCfg->status, 4);
	offset = ntohl(offset);

	
	offset &= 0x0000FFFF;
	RTMP_IO_READ32(pAdapter, offset, &value);
	value = htonl(value);
	memcpy(pRaCfg->data, &value, 4);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+4, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_IO_WRITE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset, value;

	memcpy(&offset, pRaCfg->data-2, 4);
	memcpy(&value, pRaCfg->data+2, 4);

	offset = ntohl(offset);

	
	offset &= 0x0000FFFF;
	value = ntohl(value);
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_IO_WRITE: offset = %x, value = %x\n", offset, value));
	RTMP_IO_WRITE32(pAdapter, offset, value);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_IO_READ_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32	offset;
	USHORT	len;

	memcpy(&offset, &pRaCfg->status, 4);
	offset = ntohl(offset);

	
	offset &= 0x0000FFFF;
	memcpy(&len, pRaCfg->data+2, 2);
	len = ntohs(len);

	if (len > 371)
	{
		ATEDBGPRINT(RT_DEBUG_TRACE,("length requested is too large, make it smaller\n"));
		pRaCfg->length = htons(2);
		pRaCfg->status = htons(1);
		return -EFAULT;
	}

	RTMP_IO_READ_BULK(pAdapter, pRaCfg->data, (UCHAR *)offset, len*4);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+(len*4), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_BBP_READ8(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset;
	UCHAR	value;

	value = 0;
	offset = ntohs(pRaCfg->status);

	if (ATE_ON(pAdapter))
	{
		ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, offset, &value);
	}
	else
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, offset, &value);
	}

	pRaCfg->data[0] = value;

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+1, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_BBP_WRITE8(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT	offset;
	UCHAR	value;

	offset = ntohs(pRaCfg->status);
	memcpy(&value, pRaCfg->data, 1);

	if (ATE_ON(pAdapter))
	{
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAdapter, offset, value);
	}
	else
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAdapter, offset, value);
	}

	if ((offset == BBP_R1) || (offset == BBP_R3))
	{
		SyncTxRxConfig(pAdapter, offset, value);
	}

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_BBP_READ_ALL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT bbp_reg_index;

	for (bbp_reg_index = 0; bbp_reg_index < MAX_BBP_ID+1; bbp_reg_index++)
	{
		pRaCfg->data[bbp_reg_index] = 0;

		if (ATE_ON(pAdapter))
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, bbp_reg_index, &pRaCfg->data[bbp_reg_index]);
		}
		else
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, bbp_reg_index, &pRaCfg->data[bbp_reg_index]);
		}
	}

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+MAX_BBP_ID+1, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_GET_NOISE_LEVEL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UCHAR	channel;
	INT32   buffer[3][10];

	channel = (ntohs(pRaCfg->status) & 0x00FF);
	CalNoiseLevel(pAdapter, channel, buffer);
	memcpy_exl(pAdapter, (UCHAR *)pRaCfg->data, (UCHAR *)&(buffer[0][0]), (sizeof(INT32)*3*10));

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+(sizeof(INT32)*3*10), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_GET_COUNTER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	memcpy_exl(pAdapter, &pRaCfg->data[0], (UCHAR *)&pAdapter->ate.U2M, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[4], (UCHAR *)&pAdapter->ate.OtherData, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[8], (UCHAR *)&pAdapter->ate.Beacon, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[12], (UCHAR *)&pAdapter->ate.OtherCount, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[16], (UCHAR *)&pAdapter->ate.TxAc0, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[20], (UCHAR *)&pAdapter->ate.TxAc1, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[24], (UCHAR *)&pAdapter->ate.TxAc2, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[28], (UCHAR *)&pAdapter->ate.TxAc3, 4);
	
	memcpy_exl(pAdapter, &pRaCfg->data[36], (UCHAR *)&pAdapter->ate.TxMgmt, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[40], (UCHAR *)&pAdapter->ate.RSSI0, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[44], (UCHAR *)&pAdapter->ate.RSSI1, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[48], (UCHAR *)&pAdapter->ate.RSSI2, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[52], (UCHAR *)&pAdapter->ate.SNR0, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[56], (UCHAR *)&pAdapter->ate.SNR1, 4);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+60, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_CLEAR_COUNTER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	pAdapter->ate.U2M = 0;
	pAdapter->ate.OtherData = 0;
	pAdapter->ate.Beacon = 0;
	pAdapter->ate.OtherCount = 0;
	pAdapter->ate.TxAc0 = 0;
	pAdapter->ate.TxAc1 = 0;
	pAdapter->ate.TxAc2 = 0;
	pAdapter->ate.TxAc3 = 0;
	
	pAdapter->ate.TxMgmt = 0;
	pAdapter->ate.TxDoneCount = 0;

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_TX_START(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT *p;
	USHORT	err = 1;
	UCHAR   Bbp22Value = 0, Bbp24Value = 0;

	if ((pAdapter->ate.TxStatus != 0) && (pAdapter->ate.Mode & ATE_TXFRAME))
	{
		ATEDBGPRINT(RT_DEBUG_TRACE,("Ate Tx is already running, to run next Tx, you must stop it first\n"));
		err = 2;
		goto TX_START_ERROR;
	}
	else if ((pAdapter->ate.TxStatus != 0) && !(pAdapter->ate.Mode & ATE_TXFRAME))
	{
		int i = 0;

		while ((i++ < 10) && (pAdapter->ate.TxStatus != 0))
		{
			RTMPusecDelay(5000);
		}

		
		pAdapter->ate.TxStatus = 0;
		pAdapter->ate.TxDoneCount = 0;
		pAdapter->ate.bQATxStart = FALSE;
	}

	
	if (ntohs(pRaCfg->length) != 0)
	{
		

		NdisMoveMemory(&pAdapter->ate.TxWI, pRaCfg->data + 2, 16);
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange((PUCHAR)&pAdapter->ate.TxWI, TYPE_TXWI);
#endif 

		NdisMoveMemory(&pAdapter->ate.TxCount, pRaCfg->data + 18, 4);
		pAdapter->ate.TxCount = ntohl(pAdapter->ate.TxCount);

		p = (USHORT *)(&pRaCfg->data[22]);

		
		pAdapter->ate.QID = 0;

		p = (USHORT *)(&pRaCfg->data[24]);
		pAdapter->ate.HLen = ntohs(*p);

		if (pAdapter->ate.HLen > 32)
		{
			ATEDBGPRINT(RT_DEBUG_ERROR,("pAdapter->ate.HLen > 32\n"));
			err = 3;
			goto TX_START_ERROR;
		}

		NdisMoveMemory(&pAdapter->ate.Header, pRaCfg->data + 26, pAdapter->ate.HLen);

		pAdapter->ate.PLen = ntohs(pRaCfg->length) - (pAdapter->ate.HLen + 28);

		if (pAdapter->ate.PLen > 32)
		{
			ATEDBGPRINT(RT_DEBUG_ERROR,("pAdapter->ate.PLen > 32\n"));
			err = 4;
			goto TX_START_ERROR;
		}

		NdisMoveMemory(&pAdapter->ate.Pattern, pRaCfg->data + 26 + pAdapter->ate.HLen, pAdapter->ate.PLen);
		pAdapter->ate.DLen = pAdapter->ate.TxWI.MPDUtotalByteCount - pAdapter->ate.HLen;
	}

	ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, BBP_R22, &Bbp22Value);

	switch (Bbp22Value)
	{
		case BBP22_TXFRAME:
			{
				if (pAdapter->ate.TxCount == 0)
				{
#ifdef RTMP_MAC_PCI
					pAdapter->ate.TxCount = 0xFFFFFFFF;
#endif 
				}
				ATEDBGPRINT(RT_DEBUG_TRACE,("START TXFRAME\n"));
				pAdapter->ate.bQATxStart = TRUE;
				Set_ATE_Proc(pAdapter, "TXFRAME");
			}
			break;

		case BBP22_TXCONT_OR_CARRSUPP:
			{
				ATEDBGPRINT(RT_DEBUG_TRACE,("BBP22_TXCONT_OR_CARRSUPP\n"));
				ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, BBP_R24, &Bbp24Value);

				switch (Bbp24Value)
				{
					case BBP24_TXCONT:
						{
							ATEDBGPRINT(RT_DEBUG_TRACE,("START TXCONT\n"));
							pAdapter->ate.bQATxStart = TRUE;
							Set_ATE_Proc(pAdapter, "TXCONT");
						}
						break;

					case BBP24_CARRSUPP:
						{
							ATEDBGPRINT(RT_DEBUG_TRACE,("START TXCARRSUPP\n"));
							pAdapter->ate.bQATxStart = TRUE;
							pAdapter->ate.Mode |= ATE_TXCARRSUPP;
						}
						break;

					default:
						{
							ATEDBGPRINT(RT_DEBUG_ERROR,("Unknown TX subtype !"));
						}
						break;
				}
			}
			break;

		case BBP22_TXCARR:
			{
				ATEDBGPRINT(RT_DEBUG_TRACE,("START TXCARR\n"));
				pAdapter->ate.bQATxStart = TRUE;
				Set_ATE_Proc(pAdapter, "TXCARR");
			}
			break;

		default:
			{
				ATEDBGPRINT(RT_DEBUG_ERROR,("Unknown Start TX subtype !"));
			}
			break;
	}

	if (pAdapter->ate.bQATxStart == TRUE)
	{
		RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);
		return NDIS_STATUS_SUCCESS;
	}

TX_START_ERROR:
	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), err);

	return err;
}


static inline INT DO_RACFG_CMD_GET_TX_STATUS(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 count=0;

	count = htonl(pAdapter->ate.TxDoneCount);
	NdisMoveMemory(pRaCfg->data, &count, 4);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+4, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_TX_STOP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_TX_STOP\n"));

	Set_ATE_Proc(pAdapter, "TXSTOP");

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_RX_START(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_START\n"));

	pAdapter->ate.bQARxStart = TRUE;
	Set_ATE_Proc(pAdapter, "RXFRAME");

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_RX_STOP(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_STOP\n"));

	Set_ATE_Proc(pAdapter, "RXSTOP");

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_START_TX_CARRIER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_CARRIER\n"));

	Set_ATE_Proc(pAdapter, "TXCARR");

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_START_TX_CONT(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_CONT\n"));

	Set_ATE_Proc(pAdapter, "TXCONT");

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_START_TX_FRAME(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_START_TX_FRAME\n"));

	Set_ATE_Proc(pAdapter, "TXFRAME");

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_BW(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_BW\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);
	sprintf((char *)str, "%d", value);

	Set_ATE_TX_BW_Proc(pAdapter, str);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_TX_POWER0(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_POWER0\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_POWER0_Proc(pAdapter, str);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_TX_POWER1(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_POWER1\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_POWER1_Proc(pAdapter, str);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_FREQ_OFFSET(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_FREQ_OFFSET\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_FREQOFFSET_Proc(pAdapter, str);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_GET_STATISTICS(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_GET_STATISTICS\n"));

	memcpy_exl(pAdapter, &pRaCfg->data[0], (UCHAR *)&pAdapter->ate.TxDoneCount, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[4], (UCHAR *)&pAdapter->WlanCounters.RetryCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[8], (UCHAR *)&pAdapter->WlanCounters.FailedCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[12], (UCHAR *)&pAdapter->WlanCounters.RTSSuccessCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[16], (UCHAR *)&pAdapter->WlanCounters.RTSFailureCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[20], (UCHAR *)&pAdapter->WlanCounters.ReceivedFragmentCount.QuadPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[24], (UCHAR *)&pAdapter->WlanCounters.FCSErrorCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[28], (UCHAR *)&pAdapter->Counters8023.RxNoBuffer, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[32], (UCHAR *)&pAdapter->WlanCounters.FrameDuplicateCount.u.LowPart, 4);
	memcpy_exl(pAdapter, &pRaCfg->data[36], (UCHAR *)&pAdapter->RalinkCounters.OneSecFalseCCACnt, 4);

	if (pAdapter->ate.RxAntennaSel == 0)
	{
		INT32 RSSI0 = 0;
		INT32 RSSI1 = 0;
		INT32 RSSI2 = 0;

		RSSI0 = (INT32)(pAdapter->ate.LastRssi0 - pAdapter->BbpRssiToDbmDelta);
		RSSI1 = (INT32)(pAdapter->ate.LastRssi1 - pAdapter->BbpRssiToDbmDelta);
		RSSI2 = (INT32)(pAdapter->ate.LastRssi2 - pAdapter->BbpRssiToDbmDelta);
		memcpy_exl(pAdapter, &pRaCfg->data[40], (UCHAR *)&RSSI0, 4);
		memcpy_exl(pAdapter, &pRaCfg->data[44], (UCHAR *)&RSSI1, 4);
		memcpy_exl(pAdapter, &pRaCfg->data[48], (UCHAR *)&RSSI2, 4);
		RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+52, NDIS_STATUS_SUCCESS);
	}
	else
	{
		INT32 RSSI0 = 0;

		RSSI0 = (INT32)(pAdapter->ate.LastRssi0 - pAdapter->BbpRssiToDbmDelta);
		memcpy_exl(pAdapter, &pRaCfg->data[40], (UCHAR *)&RSSI0, 4);
		RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+44, NDIS_STATUS_SUCCESS);
	}

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_RESET_COUNTER(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 1;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_RESET_COUNTER\n"));

	sprintf((char *)str, "%d", value);
	Set_ResetStatCounter_Proc(pAdapter, str);

	pAdapter->ate.TxDoneCount = 0;

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SEL_TX_ANTENNA(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SEL_TX_ANTENNA\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_Antenna_Proc(pAdapter, str);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SEL_RX_ANTENNA(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SEL_RX_ANTENNA\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_RX_Antenna_Proc(pAdapter, str);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_PREAMBLE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_PREAMBLE\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_MODE_Proc(pAdapter, str);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_CHANNEL(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_CHANNEL\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_CHANNEL_Proc(pAdapter, str);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_ADDR1(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR1\n"));

	
	memcpy(pAdapter->ate.Addr1, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_ADDR2(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR2\n"));

	
	memcpy(pAdapter->ate.Addr2, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_ADDR3(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_ADDR3\n"));

	
	memcpy(pAdapter->ate.Addr3, (PUCHAR)(pRaCfg->data - 2), MAC_ADDR_LEN);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_RATE(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_RATE\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_MCS_Proc(pAdapter, str);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_TX_FRAME_LEN(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	SHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_FRAME_LEN\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);
	sprintf((char *)str, "%d", value);
	Set_ATE_TX_LENGTH_Proc(pAdapter, str);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_SET_TX_FRAME_COUNT(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT    value = 0;
	STRING    str[LEN_OF_ARG];

	NdisZeroMemory(str, LEN_OF_ARG);

	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_ATE_SET_TX_FRAME_COUNT\n"));

	memcpy((PUCHAR)&value, (PUCHAR)&(pRaCfg->status), 2);
	value = ntohs(value);

#ifdef RTMP_MAC_PCI
	
	if (value == 0)
	{
		
		pAdapter->ate.TxCount = 0xFFFFFFFF;
		ATEDBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_COUNT_Proc (TxCount = %d)\n", pAdapter->ate.TxCount));
		ATEDBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_COUNT_Proc Success\n"));


	}
	else
#endif 
	{
		sprintf((char *)str, "%d", value);
		Set_ATE_TX_COUNT_Proc(pAdapter, str);
	}

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_START_RX_FRAME(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	ATEDBGPRINT(RT_DEBUG_TRACE,("RACFG_CMD_RX_START\n"));

	Set_ATE_Proc(pAdapter, "RXFRAME");

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_E2PROM_READ_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT buffer[EEPROM_SIZE/2];

	offset = ntohs(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = ntohs(len);

	rt_ee_read_all(pAdapter, (USHORT *)buffer);

	if (offset + len <= EEPROM_SIZE)
		memcpy_exs(pAdapter, pRaCfg->data, (UCHAR *)buffer+offset, len);
	else
		ATEDBGPRINT(RT_DEBUG_ERROR, ("exceed EEPROM size\n"));

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_E2PROM_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT buffer[EEPROM_SIZE/2];

	offset = ntohs(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = ntohs(len);

	rt_ee_read_all(pAdapter,(USHORT *)buffer);
	memcpy_exs(pAdapter, (UCHAR *)buffer + offset, (UCHAR *)pRaCfg->data + 2, len);
	rt_ee_write_all(pAdapter,(USHORT *)buffer);

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_IO_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	UINT32 offset, i, value;
	USHORT len;

	memcpy(&offset, &pRaCfg->status, 4);
	offset = ntohl(offset);
	memcpy(&len, pRaCfg->data+2, 2);
	len = ntohs(len);

	for (i = 0; i < len; i += 4)
	{
		memcpy_exl(pAdapter, (UCHAR *)&value, pRaCfg->data+4+i, 4);
		ATEDBGPRINT(RT_DEBUG_TRACE,("Write %x %x\n", offset + i, value));
		RTMP_IO_WRITE32(pAdapter, ((offset+i) & (0xffff)), value);
	}

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_BBP_READ_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT j;

	offset = ntohs(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = ntohs(len);

	for (j = offset; j < (offset+len); j++)
	{
		pRaCfg->data[j - offset] = 0;

		if (pAdapter->ate.Mode == ATE_STOP)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAdapter, j, &pRaCfg->data[j - offset]);
		}
		else
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAdapter, j, &pRaCfg->data[j - offset]);
		}
	}

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status)+len, NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


static inline INT DO_RACFG_CMD_ATE_BBP_WRITE_BULK(
	IN	PRTMP_ADAPTER	pAdapter,
	IN	struct iwreq	*wrq,
	IN  struct ate_racfghdr *pRaCfg)
{
	USHORT offset;
	USHORT len;
	USHORT j;
	UCHAR *value;

	offset = ntohs(pRaCfg->status);
	memcpy(&len, pRaCfg->data, 2);
	len = ntohs(len);

	for (j = offset; j < (offset+len); j++)
	{
		value = pRaCfg->data + 2 + (j - offset);
		if (pAdapter->ate.Mode == ATE_STOP)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAdapter, j,  *value);
		}
		else
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAdapter, j,  *value);
		}
	}

	RESPONSE_TO_GUI(pRaCfg, wrq, sizeof(pRaCfg->status), NDIS_STATUS_SUCCESS);

	return NDIS_STATUS_SUCCESS;
}


#endif 
#endif	
