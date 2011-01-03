#include <linux/delay.h>
#include <linux/broadcast/broadcast_lg2102_includes.h>
#include <linux/broadcast/broadcast_lg2102.h>







#define LGD_INTERRUPT_LOCK()	
#define LGD_INTERRUPT_FREE() 

#define inp(port)		  (*((volatile byte *) (port)))
#define inpw(port)		  (*((volatile word *) (port)))
#define inpdw(port) 	  (*((volatile dword *)(port)))
#define outp(port, val)   (*((volatile byte *) (port)) = ((byte) (val)))
#define outpw(port, val)  (*((volatile word *) (port)) = ((word) (val)))
#define outpdw(port, val) (*((volatile dword *) (port)) = ((dword) (val)))

static LGD_UINT8   *ebi2_address;


ST_SUBCH_INFO		g_stDmbInfo;
ST_SUBCH_INFO		g_stDabInfo;
ST_SUBCH_INFO		g_stDataInfo;

LGD_UINT16 			m_nMpiCSsize 	= MPI_CS_SIZE;
LGD_UINT16			m_nSpiIntrSize  = LGD_INTERRUPT_SIZE;
ENSEMBLE_BAND 		m_ucRfBand 		= KOREA_BAND_ENABLE;










CTRL_MODE 			m_ucCommandMode 	= LGD_EBI_CTRL;
UPLOAD_MODE_INFO	m_ucUploadMode 		= STREAM_UPLOAD_SLAVE_PARALLEL; 
ST_TRANSMISSION		m_ucTransMode		= TRANSMISSION_MODE1;
CLOCK_SPEED			m_ucClockSpeed 		= LGD_OUTPUT_CLOCK_4096;
LGD_ACTIVE_MODE		m_ucMPI_CS_Active 	= LGD_ACTIVE_HIGH;
LGD_ACTIVE_MODE		m_ucMPI_CLK_Active 	= LGD_ACTIVE_LOW;
LGD_UINT16			m_unIntCtrl			= (LGD_INTERRUPT_POLARITY_HIGH | \
										   LGD_INTERRUPT_PULSE | \
										   LGD_INTERRUPT_AUTOCLEAR_ENABLE | \
										   (LGD_INTERRUPT_PULSE_COUNT & LGD_INTERRUPT_PULSE_COUNT_MASK));






PLL_MODE			m_ucPLL_Mode		= INPUT_CLOCK_24576KHZ;






LGD_DPD_MODE		m_ucDPD_Mode		= LGD_DPD_ON;





















void LGD_EBI_WRITE_ADDRESS_SETUP(LGD_UINT8 *address)
{	
 	if( address != NULL) {
	   ebi2_address = address;
 	}
}


void	LGD_MUST_DELAY( LGD_UINT16 msDelay)
{
	
	msleep(msDelay);
}

LGD_UINT8 LGD_DELAY(LGD_UINT8 ucI2CID, LGD_UINT16 uiDelay)
{	
	
	if(tdmb_lg2102_mdelay(uiDelay)==LGD_ERROR)  	
	{
		INTERFACE_USER_STOP(ucI2CID);
		return LGD_ERROR;
	}
	return LGD_SUCCESS;
}

LGD_UINT16 LGD_I2C_READ(LGD_UINT8 ucI2CID, LGD_UINT16 uiAddr)
{
#ifdef LGD_I2C_GPIO_CTRL_ENABLE	
	LGD_UINT8 acBuff[2];
	LGD_UINT16 wData;
	LGD_I2C_ACK AckStatus;
	
	AckStatus = LGD_GPIO_CTRL_READ(ucI2CID, uiAddr, acBuff, 2);
	if(AckStatus == I2C_ACK_SUCCESS){
		wData = ((LGD_UINT16)acBuff[0] << 8) | (LGD_UINT16)acBuff[1];
		return wData;
	}
	return LGD_ERROR;
#else
	
	LGD_UINT8 acBuff[2];
	LGD_UINT16 wData;
	boolean AckStatus;
	
	AckStatus= tdmb_lg2102_i2c_read_burst(uiAddr, acBuff,2);	
	if(AckStatus == TRUE){
		wData = ((LGD_UINT16)acBuff[0] << 8) | (LGD_UINT16)acBuff[1];
		return wData;
	}
	
	return LGD_ERROR;
#endif
}

LGD_UINT8 LGD_I2C_WRITE(LGD_UINT8 ucI2CID, LGD_UINT16 uiAddr, LGD_UINT16 uiData)
{
#ifdef LGD_I2C_GPIO_CTRL_ENABLE	
	LGD_UINT8 acBuff[2];
	LGD_UINT8 ucCnt = 0;
	LGD_I2C_ACK AckStatus;

	acBuff[ucCnt++] = (uiData >> 8) & 0xff;
	acBuff[ucCnt++] = uiData & 0xff;
	
	AckStatus = LGD_GPIO_CTRL_WRITE(ucI2CID, uiAddr, acBuff, ucCnt);
	if(AckStatus == I2C_ACK_SUCCESS)
		return LGD_SUCCESS;
	return LGD_ERROR;
#else
	
	LGD_UINT8 acBuff[2];
	LGD_UINT8 ucCnt = 0;
	boolean AckStatus;

	acBuff[ucCnt++] = (uiData >> 8) & 0xff;
	acBuff[ucCnt++] = uiData & 0xff;

	AckStatus = tdmb_lg2102_i2c_write_burst(uiAddr, acBuff, ucCnt);	
	if(AckStatus == TRUE)
		return LGD_SUCCESS;
	return LGD_ERROR;
#endif
}

LGD_UINT8 LGD_I2C_READ_BURST(LGD_UINT8 ucI2CID,  LGD_UINT16 uiAddr, LGD_UINT8* pData, LGD_UINT16 nSize)
{
#ifdef LGD_I2C_GPIO_CTRL_ENABLE	
	LGD_I2C_ACK AckStatus;
	AckStatus = LGD_GPIO_CTRL_READ(ucI2CID, uiAddr, pData, nSize);

	if(AckStatus == I2C_ACK_SUCCESS)
		return LGD_SUCCESS;
	return LGD_ERROR;
#else
	
	
	boolean AckStatus;
	
	AckStatus = tdmb_lg2102_i2c_read_burst(uiAddr, pData,nSize);	
	if(AckStatus == TRUE)
		return LGD_SUCCESS;
	return LGD_ERROR;
#endif
}

LGD_UINT8 LGD_EBI_WRITE(LGD_UINT8 ucI2CID, LGD_UINT16 uiAddr, LGD_UINT16 uiData)
{
	LGD_UINT16 uiCMD = LGD_REGISTER_CTRL(SPI_REGWRITE_CMD) | 1;
	LGD_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;

	LGD_INTERRUPT_LOCK();

	outp(ebi2_address, uiNewAddr >> 8 );
	outp(ebi2_address, uiNewAddr & 0xff );
	outp(ebi2_address, uiCMD >> 8 );
	outp(ebi2_address, uiCMD & 0xff );

	outp(ebi2_address, (uiData >> 8) & 0xff );
	outp(ebi2_address, uiData & 0xff );
	ndelay(10);

	
	LGD_INTERRUPT_FREE();
	return LGD_ERROR;
}

LGD_UINT16 LGD_EBI_READ(LGD_UINT8 ucI2CID, LGD_UINT16 uiAddr)
{

	LGD_UINT16 uiRcvData = 0;
	LGD_UINT16 uiCMD = LGD_REGISTER_CTRL(SPI_REGREAD_CMD) | 1;
	LGD_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;

	LGD_INTERRUPT_LOCK();
	outp(ebi2_address, uiNewAddr >> 8 );
	outp(ebi2_address, uiNewAddr & 0xff );
	outp(ebi2_address, uiCMD >> 8 );
	outp(ebi2_address, uiCMD & 0xff );
	
	uiRcvData  = (inp(ebi2_address) & 0xff) << 8;
	uiRcvData |= (inp(ebi2_address) & 0xff);
	



	LGD_INTERRUPT_FREE();
	return uiRcvData;
}

LGD_UINT8 LGD_EBI_READ_BURST(LGD_UINT8 ucI2CID,  LGD_UINT16 uiAddr, LGD_UINT8* pData, LGD_UINT16 nSize)
{
	LGD_UINT16 uiLoop, nIndex = 0, anLength[2], uiCMD, unDataCnt;
	LGD_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;
	
	if(nSize > LGD_MPI_MAX_BUFF) return LGD_ERROR;
	memset((LGD_INT8*)anLength, 0, sizeof(anLength));

	if(nSize > LGD_TDMB_LENGTH_MASK) {
		anLength[nIndex++] = LGD_TDMB_LENGTH_MASK;
		anLength[nIndex++] = nSize - LGD_TDMB_LENGTH_MASK;
	}
	else anLength[nIndex++] = nSize;

	LGD_INTERRUPT_LOCK();
	for(uiLoop = 0; uiLoop < nIndex; uiLoop++){

		uiCMD = LGD_REGISTER_CTRL(SPI_MEMREAD_CMD) | (anLength[uiLoop] & LGD_TDMB_LENGTH_MASK);
		
	outp(ebi2_address, uiNewAddr >> 8 );
	outp(ebi2_address, uiNewAddr & 0xff );
	outp(ebi2_address, uiCMD >> 8 );
	outp(ebi2_address, uiCMD & 0xff );
	
		
		for(unDataCnt = 0 ; unDataCnt < anLength[uiLoop]; unDataCnt++){
			
			*pData++ = inp(ebi2_address) & 0xff;
		}
	}
	LGD_INTERRUPT_FREE();

	return LGD_SUCCESS;
}

LGD_UINT16 LGD_SPI_REG_READ(LGD_UINT8 ucI2CID, LGD_UINT16 uiAddr)
{
	LGD_UINT16 uiRcvData = 0;
	LGD_UINT16 uiCMD = LGD_REGISTER_CTRL(SPI_REGREAD_CMD) | 1;
	LGD_UINT8 auiBuff[6];
	LGD_UINT8 cCnt = 0;
	LGD_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;

	auiBuff[cCnt++] = uiNewAddr >> 8;
	auiBuff[cCnt++] = uiNewAddr & 0xff;
	auiBuff[cCnt++] = uiCMD >> 8;
	auiBuff[cCnt++] = uiCMD & 0xff;
	LGD_INTERRUPT_LOCK();
	

	
	LGD_INTERRUPT_FREE();
	return uiRcvData;
}

LGD_UINT8 LGD_SPI_REG_WRITE(LGD_UINT8 ucI2CID, LGD_UINT16 uiAddr, LGD_UINT16 uiData)
{
	LGD_UINT16 uiCMD = LGD_REGISTER_CTRL(SPI_REGWRITE_CMD) | 1;
	LGD_UINT8 auiBuff[6];
	LGD_UINT8 cCnt = 0;
	LGD_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;

	auiBuff[cCnt++] = uiNewAddr >> 8;
	auiBuff[cCnt++] = uiNewAddr & 0xff;
	auiBuff[cCnt++] = uiCMD >> 8;
	auiBuff[cCnt++] = uiCMD & 0xff;
	auiBuff[cCnt++] = uiData >> 8;
	auiBuff[cCnt++] = uiData & 0xff;
	LGD_INTERRUPT_LOCK();
	
	

	LGD_INTERRUPT_FREE();
	return LGD_SUCCESS;
}

LGD_UINT8 LGD_SPI_READ_BURST(LGD_UINT8 ucI2CID, LGD_UINT16 uiAddr, LGD_UINT8* pBuff, LGD_UINT16 wSize)
{
	LGD_UINT16 uiLoop, nIndex = 0, anLength[2], uiCMD, unDataCnt;
	LGD_UINT8 auiBuff[6];
	LGD_UINT16 uiNewAddr = (ucI2CID == TDMB_I2C_ID82) ? (uiAddr | 0x8000) : uiAddr;

	if(wSize > LGD_MPI_MAX_BUFF) return LGD_ERROR;
	memset((LGD_INT8*)anLength, 0, sizeof(anLength));

	if(wSize > LGD_TDMB_LENGTH_MASK) {
		anLength[nIndex++] = LGD_TDMB_LENGTH_MASK;
		anLength[nIndex++] = wSize - LGD_TDMB_LENGTH_MASK;
	}
	else anLength[nIndex++] = wSize;

	LGD_INTERRUPT_LOCK();
	for(uiLoop = 0; uiLoop < nIndex; uiLoop++){

		auiBuff[0] = uiNewAddr >> 8;
		auiBuff[1] = uiNewAddr & 0xff;
		uiCMD = LGD_REGISTER_CTRL(SPI_MEMREAD_CMD) | (anLength[uiLoop] & LGD_TDMB_LENGTH_MASK);
		auiBuff[2] = uiCMD >> 8;
		auiBuff[3] = uiCMD & 0xff;
		
		
		for(unDataCnt = 0 ; unDataCnt < anLength[uiLoop]; unDataCnt++){
			
		}
	}
	LGD_INTERRUPT_FREE();

	return LGD_SUCCESS;
}

LGD_UINT8 INTERFACE_DBINIT(void)
{
	memset(&g_stDmbInfo,	0, sizeof(ST_SUBCH_INFO));
	memset(&g_stDabInfo,	0, sizeof(ST_SUBCH_INFO));
	memset(&g_stDataInfo,	0, sizeof(ST_SUBCH_INFO));
	
	return LGD_SUCCESS;
}

void INTERFACE_UPLOAD_MODE(LGD_UINT8 ucI2CID, UPLOAD_MODE_INFO ucUploadMode)
{
	m_ucUploadMode = ucUploadMode;
	LGD_UPLOAD_MODE(ucI2CID);
}

LGD_UINT8 INTERFACE_PLL_MODE(LGD_UINT8 ucI2CID, PLL_MODE ucPllMode)
{
	m_ucPLL_Mode = ucPllMode;
	return LGD_PLL_SET(ucI2CID);
}


LGD_UINT8 INTERFACE_INIT(LGD_UINT8 ucI2CID)
{
	return LGD_INIT(ucI2CID);
}

LGD_UINT8 INTERFACE_RESET_CH(LGD_UINT8 ucI2CID)
{
	LGD_RESET_MPI(ucI2CID);

	return LGD_SUCCESS;	
}



LGD_ERROR_INFO INTERFACE_ERROR_STATUS(LGD_UINT8 ucI2CID)
{
	ST_BBPINFO* pInfo;
	pInfo = LGD_GET_STRINFO(ucI2CID);
	return pInfo->nBbpStatus;
}


  










LGD_UINT8 INTERFACE_START(LGD_UINT8 ucI2CID, ST_SUBCH_INFO* pChInfo)
{
	return LGD_CHANNEL_START(ucI2CID, pChInfo);
}

LGD_UINT8 INTERFACE_RE_SYNCDETECTOR(LGD_UINT8 ucI2CID, ST_SUBCH_INFO* pMultiInfo)
{
	return (LGD_UINT8)LGD_RE_SYNCDETECTOR(ucI2CID, pMultiInfo);
}

LGD_UINT8 INTERFACE_RE_SYNC(LGD_UINT8 ucI2CID)
{
	return LGD_RE_SYNC(ucI2CID);
}







LGD_UINT8 INTERFACE_SCAN(LGD_UINT8 ucI2CID, LGD_UINT32 ulFreq)
{
	
	ST_FIC_DB* pstFicDb;
	
	pstFicDb = LGD_GETFIC_DB(ucI2CID);	
	
	INTERFACE_DBINIT();

	if(!LGD_ENSEMBLE_SCAN(ucI2CID, ulFreq))
	{
		LGD_ERROR_INFO err = INTERFACE_ERROR_STATUS(ucI2CID);
		printk("INTERFACE_SCAN error_result  = (0x%x)\n", err); 
		return LGD_ERROR;
	}
 	pstFicDb->ulRFFreq = ulFreq;
	
#if 0 
	for(nIndex = 0; nIndex < pstFicDb->ucSubChCnt; nIndex++){
		switch(pstFicDb->aucTmID[nIndex])
		{
		case TMID_1 : pChInfo = &g_stDmbInfo.astSubChInfo[g_stDmbInfo.nSetCnt++];	break;
		case TMID_0 : pChInfo = &g_stDabInfo.astSubChInfo[g_stDabInfo.nSetCnt++];	break;
		default   : pChInfo = &g_stDataInfo.astSubChInfo[g_stDataInfo.nSetCnt++];	break;
		}
		LGD_UPDATE(pChInfo, pstFicDb, nIndex);
	}
#endif	
	return LGD_SUCCESS;
}




LGD_UINT16 INTERFACE_GETDMB_CNT(void)
{
	return (LGD_UINT16)g_stDmbInfo.nSetCnt;
}




LGD_UINT16 INTERFACE_GETDAB_CNT(void)
{
	return (LGD_UINT16)g_stDabInfo.nSetCnt;
}




LGD_UINT16 INTERFACE_GETDATA_CNT(void)
{
	return (LGD_UINT16)g_stDataInfo.nSetCnt;
}




LGD_UINT8* INTERFACE_GETENSEMBLE_LABEL(LGD_UINT8 ucI2CID)
{
	ST_FIC_DB* pstFicDb;
	pstFicDb = LGD_GETFIC_DB(ucI2CID);
	return pstFicDb->aucEnsembleLabel;
}




LGD_CHANNEL_INFO* INTERFACE_GETDB_DMB(LGD_INT16 uiPos)
{
	if(uiPos >= MAX_SUBCH_SIZE) return LGD_NULL;
	if(uiPos >= g_stDmbInfo.nSetCnt) return LGD_NULL;
	return &g_stDmbInfo.astSubChInfo[uiPos];
}




LGD_CHANNEL_INFO* INTERFACE_GETDB_DAB(LGD_INT16 uiPos)
{
	if(uiPos >= MAX_SUBCH_SIZE) return LGD_NULL;
	if(uiPos >= g_stDabInfo.nSetCnt) return LGD_NULL;
	return &g_stDabInfo.astSubChInfo[uiPos];
}




LGD_CHANNEL_INFO* INTERFACE_GETDB_DATA(LGD_INT16 uiPos)
{
	if(uiPos >= MAX_SUBCH_SIZE) return LGD_NULL;
	if(uiPos >= g_stDataInfo.nSetCnt) return LGD_NULL;
	return &g_stDataInfo.astSubChInfo[uiPos];
}


LGD_UINT8 INTERFACE_RECONFIG(LGD_UINT8 ucI2CID)
{
	return LGD_FIC_RECONFIGURATION_HW_CHECK(ucI2CID);
}

LGD_UINT8 INTERFACE_STATUS_CHECK(LGD_UINT8 ucI2CID)
{
	return LGD_STATUS_CHECK(ucI2CID);
}

LGD_UINT16 INTERFACE_GET_CER(LGD_UINT8 ucI2CID)
{
	return LGD_GET_CER(ucI2CID);
}

LGD_UINT8 INTERFACE_GET_SNR(LGD_UINT8 ucI2CID)
{
	return LGD_GET_SNR(ucI2CID);
}

LGD_DOUBLE32 INTERFACE_GET_POSTBER(LGD_UINT8 ucI2CID)
{
	return LGD_GET_POSTBER(ucI2CID);
}


LGD_UINT16 INTERFACE_GET_TPERRCNT(LGD_UINT8 ucI2CID)
{
	return LGD_GET_TPERRCNT(ucI2CID);
}

LGD_DOUBLE32 INTERFACE_GET_PREBER(LGD_UINT8 ucI2CID)
{
	return LGD_GET_PREBER(ucI2CID);
}




void INTERFACE_USER_STOP(LGD_UINT8 ucI2CID)
{
	ST_BBPINFO* pInfo;
	pInfo = LGD_GET_STRINFO(ucI2CID);
	pInfo->ucStop = 1;
}


void INTERFACE_INT_ENABLE(LGD_UINT8 ucI2CID, LGD_UINT16 unSet)
{
	LGD_INTERRUPT_SET(ucI2CID, unSet);
}


LGD_UINT8 INTERFACE_INT_CHECK(LGD_UINT8 ucI2CID)
{
	LGD_UINT16 nValue = 0;

	nValue = LGD_CMD_READ(ucI2CID, APB_INT_BASE+ 0x01);
	if(!(nValue & LGD_MPI_INTERRUPT_ENABLE))
		return FALSE;

	return TRUE;
}

void INTERFACE_INT_CLEAR(LGD_UINT8 ucI2CID, LGD_UINT16 unClr)
{
	LGD_INTERRUPT_CLEAR(ucI2CID, unClr);
}


LGD_UINT8 INTERFACE_ISR(LGD_UINT8 ucI2CID, LGD_UINT8* pBuff)
{
	LGD_UINT16 unLoop;
	LGD_UINT32 ulAddrSelect;

	if(m_ucCommandMode != LGD_EBI_CTRL){
		if(m_ucUploadMode == STREAM_UPLOAD_SPI){
			LGD_SPI_READ_BURST(ucI2CID, APB_STREAM_BASE, pBuff, LGD_INTERRUPT_SIZE);
		}
		
		else if(m_ucUploadMode == STREAM_UPLOAD_SLAVE_PARALLEL)
		{
			ulAddrSelect = (ucI2CID == TDMB_I2C_ID80) ? STREAM_PARALLEL_ADDRESS : STREAM_PARALLEL_ADDRESS_CS;
			for(unLoop = 0; unLoop < LGD_INTERRUPT_SIZE; unLoop++){
				pBuff[unLoop] = *(volatile LGD_UINT8*)ulAddrSelect & 0xff;
			}
		}
	}
	else
	{
		LGD_EBI_READ_BURST(ucI2CID, APB_STREAM_BASE, pBuff, LGD_INTERRUPT_SIZE);
	}

	if((m_unIntCtrl & LGD_INTERRUPT_LEVEL) && (!(m_unIntCtrl & LGD_INTERRUPT_AUTOCLEAR_ENABLE)))
		INTERFACE_INT_CLEAR(ucI2CID, LGD_MPI_INTERRUPT_ENABLE);
	
	return LGD_SUCCESS;
}


LGD_UINT8 INTERFACE_CHANGE_BAND(LGD_UINT8 ucI2CID, LGD_UINT16 usBand)
{
	switch(usBand){
		case 0x01 : 
			m_ucRfBand = KOREA_BAND_ENABLE; 
			m_ucTransMode = TRANSMISSION_MODE1; 
			break;
		case 0x02 : 
			m_ucRfBand = BANDIII_ENABLE;
			m_ucTransMode = TRANSMISSION_MODE1; 
			break;
		case 0x04 : 
			m_ucRfBand = LBAND_ENABLE;
			m_ucTransMode = TRANSMISSION_MODE2; 
			break;
		case 0x08 : 
			
			
			
		case 0x10 : 
			m_ucRfBand = CHINA_ENABLE;
			m_ucTransMode = TRANSMISSION_MODE1; break;
		default : return LGD_ERROR;
	}
	return LGD_SUCCESS;
}

LGD_UINT8 INTERFACE_FIC_UPDATE_CHECK(LGD_UINT8 ucI2CID)
{
	ST_BBPINFO* pInfo;
	pInfo = LGD_GET_STRINFO(ucI2CID);

	if(pInfo->IsChangeEnsemble == TRUE)
		return TRUE;

	return FALSE;
}
