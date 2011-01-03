
#include "../rt_config.h"
#ifdef RT2860
#include "firmware.h"
#include <linux/bitrev.h>
#endif
#ifdef RT2870

#include "../../rt3070/firmware.h"
#endif

UCHAR    BIT8[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
ULONG    BIT32[] = {0x00000001, 0x00000002, 0x00000004, 0x00000008,
					0x00000010, 0x00000020, 0x00000040, 0x00000080,
					0x00000100, 0x00000200, 0x00000400, 0x00000800,
					0x00001000, 0x00002000, 0x00004000, 0x00008000,
					0x00010000, 0x00020000, 0x00040000, 0x00080000,
					0x00100000, 0x00200000, 0x00400000, 0x00800000,
					0x01000000, 0x02000000, 0x04000000, 0x08000000,
					0x10000000, 0x20000000, 0x40000000, 0x80000000};

char*   CipherName[] = {"none","wep64","wep128","TKIP","AES","CKIP64","CKIP128"};




REG_PAIR   BBPRegTable[] = {
	{BBP_R65,		0x2C},		
	{BBP_R66,		0x38},	
	{BBP_R69,		0x12},
	{BBP_R70,		0xa},	
	{BBP_R73,		0x10},
	{BBP_R81,		0x37},
	{BBP_R82,		0x62},
	{BBP_R83,		0x6A},
	{BBP_R84,		0x99},	
	{BBP_R86,		0x00},	
	{BBP_R91,		0x04},	
	{BBP_R92,		0x00},	
	{BBP_R103,  	0x00}, 	
	{BBP_R105,		0x05},	
};
#define	NUM_BBP_REG_PARMS	(sizeof(BBPRegTable) / sizeof(REG_PAIR))




#ifdef RT2870
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
#define	NUM_RF_REG_PARMS	(sizeof(RT30xx_RFRegTable) / sizeof(REG_PAIR))
#endif 





RTMP_REG_PAIR	MACRegTable[] =	{
#if defined(HW_BEACON_OFFSET) && (HW_BEACON_OFFSET == 0x200)
	{BCN_OFFSET0,			0xf8f0e8e0}, 
	{BCN_OFFSET1,			0x6f77d0c8}, 
#elif defined(HW_BEACON_OFFSET) && (HW_BEACON_OFFSET == 0x100)
	{BCN_OFFSET0,			0xece8e4e0}, 
	{BCN_OFFSET1,			0xfcf8f4f0}, 
#else
    #error You must re-calculate new value for BCN_OFFSET0 & BCN_OFFSET1 in MACRegTable[]!!!
#endif 

	{LEGACY_BASIC_RATE,		0x0000013f}, 
	{HT_BASIC_RATE,		0x00008003}, 
	{MAC_SYS_CTRL,		0x00}, 
	{RX_FILTR_CFG,		0x17f97}, 
	{BKOFF_SLOT_CFG,	0x209}, 
	{TX_SW_CFG0,		0x0}, 		
	{TX_SW_CFG1,		0x80606}, 
	{TX_LINK_CFG,		0x1020},		
	{TX_TIMEOUT_CFG,	0x000a2090},	
	{MAX_LEN_CFG,		MAX_AGGREGATION_SIZE | 0x00001000},	
	{LED_CFG,		0x7f031e46}, 
	{PBF_MAX_PCNT,			0x1F3FBF9F}, 	
	{TX_RTY_CFG,			0x47d01f0f},	
	{AUTO_RSP_CFG,			0x00000013},	
	{CCK_PROT_CFG,			0x05740003 },	
	{OFDM_PROT_CFG,			0x05740003 },	

#ifdef RT2870
	{PBF_CFG, 				0xf40006}, 		
	{MM40_PROT_CFG,			0x3F44084},		
	{WPDMA_GLO_CFG,			0x00000030},
#endif 
	{GF20_PROT_CFG,			0x01744004},    
	{GF40_PROT_CFG,			0x03F44084},
	{MM20_PROT_CFG,			0x01744004},
#ifdef RT2860
	{MM40_PROT_CFG,			0x03F54084},
#endif
	{TXOP_CTRL_CFG,			0x0000583f,  },	
	{TX_RTS_CFG,			0x00092b20},
	{EXP_ACK_TIME,			0x002400ca},	
	{TXOP_HLDR_ET, 			0x00000002},

	
	{XIFS_TIME_CFG,			0x33a41010},
	{PWR_PIN_CFG,			0x00000003},	
};

RTMP_REG_PAIR	STAMACRegTable[] =	{
	{WMM_AIFSN_CFG,		0x00002273},
	{WMM_CWMIN_CFG,	0x00002344},
	{WMM_CWMAX_CFG,	0x000034aa},
};

#define	NUM_MAC_REG_PARMS		(sizeof(MACRegTable) / sizeof(RTMP_REG_PAIR))
#define	NUM_STA_MAC_REG_PARMS	(sizeof(STAMACRegTable) / sizeof(RTMP_REG_PAIR))

#ifdef RT2870



#define FIRMWARE_MINOR_VERSION	7

#endif 


#define FIRMWAREIMAGE_MAX_LENGTH	0x2000
#define FIRMWAREIMAGE_LENGTH		(sizeof (FirmwareImage) / sizeof(UCHAR))
#define FIRMWARE_MAJOR_VERSION	0

#define FIRMWAREIMAGEV1_LENGTH	0x1000
#define FIRMWAREIMAGEV2_LENGTH	0x1000

#ifdef RT2860
#define FIRMWARE_MINOR_VERSION	2
#endif



NDIS_STATUS	RTMPAllocAdapterBlock(
	IN  PVOID	handle,
	OUT	PRTMP_ADAPTER	*ppAdapter)
{
	PRTMP_ADAPTER	pAd;
	NDIS_STATUS		Status;
	INT 			index;
	UCHAR			*pBeaconBuf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPAllocAdapterBlock\n"));

	*ppAdapter = NULL;

	do
	{
		
		pBeaconBuf = kmalloc(MAX_BEACON_SIZE, MEM_ALLOC_FLAG);
		if (pBeaconBuf == NULL)
		{
			Status = NDIS_STATUS_FAILURE;
			DBGPRINT_ERR(("Failed to allocate memory - BeaconBuf!\n"));
			break;
		}

		Status = AdapterBlockAllocateMemory(handle, (PVOID *)&pAd);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT_ERR(("Failed to allocate memory - ADAPTER\n"));
			break;
		}
		pAd->BeaconBuf = pBeaconBuf;
		printk("\n\n=== pAd = %p, size = %d ===\n\n", pAd, (UINT32)sizeof(RTMP_ADAPTER));


		
		NdisAllocateSpinLock(&pAd->MgmtRingLock);
#ifdef RT2860
		NdisAllocateSpinLock(&pAd->RxRingLock);
#endif

		for (index =0 ; index < NUM_OF_TX_RING; index++)
		{
			NdisAllocateSpinLock(&pAd->TxSwQueueLock[index]);
			NdisAllocateSpinLock(&pAd->DeQueueLock[index]);
			pAd->DeQueueRunning[index] = FALSE;
		}

		NdisAllocateSpinLock(&pAd->irq_lock);

	} while (FALSE);

	if ((Status != NDIS_STATUS_SUCCESS) && (pBeaconBuf))
		kfree(pBeaconBuf);

	*ppAdapter = pAd;

	DBGPRINT_S(Status, ("<-- RTMPAllocAdapterBlock, Status=%x\n", Status));
	return Status;
}


VOID	RTMPReadTxPwrPerRate(
	IN	PRTMP_ADAPTER	pAd)
{
	ULONG		data, Adata, Gdata;
	USHORT		i, value, value2;
	INT			Apwrdelta, Gpwrdelta;
	UCHAR		t1,t2,t3,t4;
	BOOLEAN		bValid, bApwrdeltaMinus = TRUE, bGpwrdeltaMinus = TRUE;

	
	
	
	DBGPRINT(RT_DEBUG_TRACE, ("Txpower per Rate\n"));
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, value2);
	Apwrdelta = 0;
	Gpwrdelta = 0;

	if ((value2 & 0xff) != 0xff)
	{
		if ((value2 & 0x80))
			Gpwrdelta = (value2&0xf);

		if ((value2 & 0x40))
			bGpwrdeltaMinus = FALSE;
		else
			bGpwrdeltaMinus = TRUE;
	}
	if ((value2 & 0xff00) != 0xff00)
	{
		if ((value2 & 0x8000))
			Apwrdelta = ((value2&0xf00)>>8);

		if ((value2 & 0x4000))
			bApwrdeltaMinus = FALSE;
		else
			bApwrdeltaMinus = TRUE;
	}
	DBGPRINT(RT_DEBUG_TRACE, ("Gpwrdelta = %x, Apwrdelta = %x .\n", Gpwrdelta, Apwrdelta));

	
	
	
	for (i=0; i<5; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4, value);
		data = value;
		if (bApwrdeltaMinus == FALSE)
		{
			t1 = (value&0xf)+(Apwrdelta);
			if (t1 > 0xf)
				t1 = 0xf;
			t2 = ((value&0xf0)>>4)+(Apwrdelta);
			if (t2 > 0xf)
				t2 = 0xf;
			t3 = ((value&0xf00)>>8)+(Apwrdelta);
			if (t3 > 0xf)
				t3 = 0xf;
			t4 = ((value&0xf000)>>12)+(Apwrdelta);
			if (t4 > 0xf)
				t4 = 0xf;
		}
		else
		{
			if ((value&0xf) > Apwrdelta)
				t1 = (value&0xf)-(Apwrdelta);
			else
				t1 = 0;
			if (((value&0xf0)>>4) > Apwrdelta)
				t2 = ((value&0xf0)>>4)-(Apwrdelta);
			else
				t2 = 0;
			if (((value&0xf00)>>8) > Apwrdelta)
				t3 = ((value&0xf00)>>8)-(Apwrdelta);
			else
				t3 = 0;
			if (((value&0xf000)>>12) > Apwrdelta)
				t4 = ((value&0xf000)>>12)-(Apwrdelta);
			else
				t4 = 0;
		}
		Adata = t1 + (t2<<4) + (t3<<8) + (t4<<12);
		if (bGpwrdeltaMinus == FALSE)
		{
			t1 = (value&0xf)+(Gpwrdelta);
			if (t1 > 0xf)
				t1 = 0xf;
			t2 = ((value&0xf0)>>4)+(Gpwrdelta);
			if (t2 > 0xf)
				t2 = 0xf;
			t3 = ((value&0xf00)>>8)+(Gpwrdelta);
			if (t3 > 0xf)
				t3 = 0xf;
			t4 = ((value&0xf000)>>12)+(Gpwrdelta);
			if (t4 > 0xf)
				t4 = 0xf;
		}
		else
		{
			if ((value&0xf) > Gpwrdelta)
				t1 = (value&0xf)-(Gpwrdelta);
			else
				t1 = 0;
			if (((value&0xf0)>>4) > Gpwrdelta)
				t2 = ((value&0xf0)>>4)-(Gpwrdelta);
			else
				t2 = 0;
			if (((value&0xf00)>>8) > Gpwrdelta)
				t3 = ((value&0xf00)>>8)-(Gpwrdelta);
			else
				t3 = 0;
			if (((value&0xf000)>>12) > Gpwrdelta)
				t4 = ((value&0xf000)>>12)-(Gpwrdelta);
			else
				t4 = 0;
		}
		Gdata = t1 + (t2<<4) + (t3<<8) + (t4<<12);

		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4 + 2, value);
		if (bApwrdeltaMinus == FALSE)
		{
			t1 = (value&0xf)+(Apwrdelta);
			if (t1 > 0xf)
				t1 = 0xf;
			t2 = ((value&0xf0)>>4)+(Apwrdelta);
			if (t2 > 0xf)
				t2 = 0xf;
			t3 = ((value&0xf00)>>8)+(Apwrdelta);
			if (t3 > 0xf)
				t3 = 0xf;
			t4 = ((value&0xf000)>>12)+(Apwrdelta);
			if (t4 > 0xf)
				t4 = 0xf;
		}
		else
		{
			if ((value&0xf) > Apwrdelta)
				t1 = (value&0xf)-(Apwrdelta);
			else
				t1 = 0;
			if (((value&0xf0)>>4) > Apwrdelta)
				t2 = ((value&0xf0)>>4)-(Apwrdelta);
			else
				t2 = 0;
			if (((value&0xf00)>>8) > Apwrdelta)
				t3 = ((value&0xf00)>>8)-(Apwrdelta);
			else
				t3 = 0;
			if (((value&0xf000)>>12) > Apwrdelta)
				t4 = ((value&0xf000)>>12)-(Apwrdelta);
			else
				t4 = 0;
		}
		Adata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));
		if (bGpwrdeltaMinus == FALSE)
		{
			t1 = (value&0xf)+(Gpwrdelta);
			if (t1 > 0xf)
				t1 = 0xf;
			t2 = ((value&0xf0)>>4)+(Gpwrdelta);
			if (t2 > 0xf)
				t2 = 0xf;
			t3 = ((value&0xf00)>>8)+(Gpwrdelta);
			if (t3 > 0xf)
				t3 = 0xf;
			t4 = ((value&0xf000)>>12)+(Gpwrdelta);
			if (t4 > 0xf)
				t4 = 0xf;
		}
		else
		{
			if ((value&0xf) > Gpwrdelta)
				t1 = (value&0xf)-(Gpwrdelta);
			else
				t1 = 0;
			if (((value&0xf0)>>4) > Gpwrdelta)
				t2 = ((value&0xf0)>>4)-(Gpwrdelta);
			else
				t2 = 0;
			if (((value&0xf00)>>8) > Gpwrdelta)
				t3 = ((value&0xf00)>>8)-(Gpwrdelta);
			else
				t3 = 0;
			if (((value&0xf000)>>12) > Gpwrdelta)
				t4 = ((value&0xf000)>>12)-(Gpwrdelta);
			else
				t4 = 0;
		}
		Gdata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));
		data |= (value<<16);

		pAd->Tx20MPwrCfgABand[i] = pAd->Tx40MPwrCfgABand[i] = Adata;
		pAd->Tx20MPwrCfgGBand[i] = pAd->Tx40MPwrCfgGBand[i] = Gdata;

		if (data != 0xffffffff)
			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, data);
		DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 2.4G band-%lx,  Adata = %lx,  Gdata = %lx \n", data, Adata, Gdata));
	}

	
	
	
	bValid = TRUE;
	for (i=0; i<6; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + 2 + i*2, value);
		if (((value & 0x00FF) == 0x00FF) || ((value & 0xFF00) == 0xFF00))
		{
			bValid = FALSE;
			break;
		}
	}

	
	
	
	if (bValid)
	{
		for (i=0; i<4; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + i*4, value);
			if (bGpwrdeltaMinus == FALSE)
			{
				t1 = (value&0xf)+(Gpwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = ((value&0xf0)>>4)+(Gpwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = ((value&0xf00)>>8)+(Gpwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = ((value&0xf000)>>12)+(Gpwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if ((value&0xf) > Gpwrdelta)
					t1 = (value&0xf)-(Gpwrdelta);
				else
					t1 = 0;
				if (((value&0xf0)>>4) > Gpwrdelta)
					t2 = ((value&0xf0)>>4)-(Gpwrdelta);
				else
					t2 = 0;
				if (((value&0xf00)>>8) > Gpwrdelta)
					t3 = ((value&0xf00)>>8)-(Gpwrdelta);
				else
					t3 = 0;
				if (((value&0xf000)>>12) > Gpwrdelta)
					t4 = ((value&0xf000)>>12)-(Gpwrdelta);
				else
					t4 = 0;
			}
			Gdata = t1 + (t2<<4) + (t3<<8) + (t4<<12);

			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_2_4G + i*4 + 2, value);
			if (bGpwrdeltaMinus == FALSE)
			{
				t1 = (value&0xf)+(Gpwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = ((value&0xf0)>>4)+(Gpwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = ((value&0xf00)>>8)+(Gpwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = ((value&0xf000)>>12)+(Gpwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if ((value&0xf) > Gpwrdelta)
					t1 = (value&0xf)-(Gpwrdelta);
				else
					t1 = 0;
				if (((value&0xf0)>>4) > Gpwrdelta)
					t2 = ((value&0xf0)>>4)-(Gpwrdelta);
				else
					t2 = 0;
				if (((value&0xf00)>>8) > Gpwrdelta)
					t3 = ((value&0xf00)>>8)-(Gpwrdelta);
				else
					t3 = 0;
				if (((value&0xf000)>>12) > Gpwrdelta)
					t4 = ((value&0xf000)>>12)-(Gpwrdelta);
				else
					t4 = 0;
			}
			Gdata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));

			if (i == 0)
				pAd->Tx40MPwrCfgGBand[i+1] = (pAd->Tx40MPwrCfgGBand[i+1] & 0x0000FFFF) | (Gdata & 0xFFFF0000);
			else
				pAd->Tx40MPwrCfgGBand[i+1] = Gdata;

			DBGPRINT_RAW(RT_DEBUG_TRACE, ("40MHz BW, 2.4G band, Gdata = %lx \n", Gdata));
		}
	}

	
	
	
	bValid = TRUE;
	for (i=0; i<8; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + 2 + i*2, value);
		if (((value & 0x00FF) == 0x00FF) || ((value & 0xFF00) == 0xFF00))
		{
			bValid = FALSE;
			break;
		}
	}

	
	
	
	if (bValid)
	{
		for (i=0; i<5; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + i*4, value);
			if (bApwrdeltaMinus == FALSE)
			{
				t1 = (value&0xf)+(Apwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = ((value&0xf0)>>4)+(Apwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = ((value&0xf00)>>8)+(Apwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = ((value&0xf000)>>12)+(Apwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if ((value&0xf) > Apwrdelta)
					t1 = (value&0xf)-(Apwrdelta);
				else
					t1 = 0;
				if (((value&0xf0)>>4) > Apwrdelta)
					t2 = ((value&0xf0)>>4)-(Apwrdelta);
				else
					t2 = 0;
				if (((value&0xf00)>>8) > Apwrdelta)
					t3 = ((value&0xf00)>>8)-(Apwrdelta);
				else
					t3 = 0;
				if (((value&0xf000)>>12) > Apwrdelta)
					t4 = ((value&0xf000)>>12)-(Apwrdelta);
				else
					t4 = 0;
			}
			Adata = t1 + (t2<<4) + (t3<<8) + (t4<<12);

			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_5G + i*4 + 2, value);
			if (bApwrdeltaMinus == FALSE)
			{
				t1 = (value&0xf)+(Apwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = ((value&0xf0)>>4)+(Apwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = ((value&0xf00)>>8)+(Apwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = ((value&0xf000)>>12)+(Apwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if ((value&0xf) > Apwrdelta)
					t1 = (value&0xf)-(Apwrdelta);
				else
					t1 = 0;
				if (((value&0xf0)>>4) > Apwrdelta)
					t2 = ((value&0xf0)>>4)-(Apwrdelta);
				else
					t2 = 0;
				if (((value&0xf00)>>8) > Apwrdelta)
					t3 = ((value&0xf00)>>8)-(Apwrdelta);
				else
					t3 = 0;
				if (((value&0xf000)>>12) > Apwrdelta)
					t4 = ((value&0xf000)>>12)-(Apwrdelta);
				else
					t4 = 0;
			}
			Adata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));

			if (i == 0)
				pAd->Tx20MPwrCfgABand[i] = (pAd->Tx20MPwrCfgABand[i] & 0x0000FFFF) | (Adata & 0xFFFF0000);
			else
				pAd->Tx20MPwrCfgABand[i] = Adata;

			DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 5GHz band, Adata = %lx \n", Adata));
		}
	}

	
	
	
	bValid = TRUE;
	for (i=0; i<6; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + 2 + i*2, value);
		if (((value & 0x00FF) == 0x00FF) || ((value & 0xFF00) == 0xFF00))
		{
			bValid = FALSE;
			break;
		}
	}

	
	
	
	if (bValid)
	{
		for (i=0; i<4; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + i*4, value);
			if (bApwrdeltaMinus == FALSE)
			{
				t1 = (value&0xf)+(Apwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = ((value&0xf0)>>4)+(Apwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = ((value&0xf00)>>8)+(Apwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = ((value&0xf000)>>12)+(Apwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if ((value&0xf) > Apwrdelta)
					t1 = (value&0xf)-(Apwrdelta);
				else
					t1 = 0;
				if (((value&0xf0)>>4) > Apwrdelta)
					t2 = ((value&0xf0)>>4)-(Apwrdelta);
				else
					t2 = 0;
				if (((value&0xf00)>>8) > Apwrdelta)
					t3 = ((value&0xf00)>>8)-(Apwrdelta);
				else
					t3 = 0;
				if (((value&0xf000)>>12) > Apwrdelta)
					t4 = ((value&0xf000)>>12)-(Apwrdelta);
				else
					t4 = 0;
			}
			Adata = t1 + (t2<<4) + (t3<<8) + (t4<<12);

			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_40MHZ_5G + i*4 + 2, value);
			if (bApwrdeltaMinus == FALSE)
			{
				t1 = (value&0xf)+(Apwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = ((value&0xf0)>>4)+(Apwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = ((value&0xf00)>>8)+(Apwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = ((value&0xf000)>>12)+(Apwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if ((value&0xf) > Apwrdelta)
					t1 = (value&0xf)-(Apwrdelta);
				else
					t1 = 0;
				if (((value&0xf0)>>4) > Apwrdelta)
					t2 = ((value&0xf0)>>4)-(Apwrdelta);
				else
					t2 = 0;
				if (((value&0xf00)>>8) > Apwrdelta)
					t3 = ((value&0xf00)>>8)-(Apwrdelta);
				else
					t3 = 0;
				if (((value&0xf000)>>12) > Apwrdelta)
					t4 = ((value&0xf000)>>12)-(Apwrdelta);
				else
					t4 = 0;
			}
			Adata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));

			if (i == 0)
				pAd->Tx40MPwrCfgABand[i+1] = (pAd->Tx40MPwrCfgABand[i+1] & 0x0000FFFF) | (Adata & 0xFFFF0000);
			else
				pAd->Tx40MPwrCfgABand[i+1] = Adata;

			DBGPRINT_RAW(RT_DEBUG_TRACE, ("40MHz BW, 5GHz band, Adata = %lx \n", Adata));
		}
	}
}



VOID	RTMPReadChannelPwr(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR				i, choffset;
	EEPROM_TX_PWR_STRUC	    Power;
	EEPROM_TX_PWR_STRUC	    Power2;

	
	
	
	

	
	for (i = 0; i < 7; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2, Power2.word);
		pAd->TxPower[i * 2].Channel = i * 2 + 1;
		pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;

		if ((Power.field.Byte0 > 31) || (Power.field.Byte0 < 0))
			pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2].Power = Power.field.Byte0;

		if ((Power.field.Byte1 > 31) || (Power.field.Byte1 < 0))
			pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;

		if ((Power2.field.Byte0 > 31) || (Power2.field.Byte0 < 0))
			pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 > 31) || (Power2.field.Byte1 < 0))
			pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2 + 1].Power2 = Power2.field.Byte1;
	}

	
	
	choffset = 14;
	for (i = 0; i < 4; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 36 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 36 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 36 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
	}

	
	for (i = 0; i < 6; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + i * 2, Power2.word);

		if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;

		if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;
	}

	
	
	choffset = 14 + 12;
	for (i = 0; i < 5; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 100 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 100 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 100 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
	}
	pAd->TxPower[3 * 5 + choffset + 0].Channel		= 140;
	pAd->TxPower[3 * 5 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 5 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;

	
	for (i = 0; i < 8; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);

		if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;

		if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;
	}

	
	
	choffset = 14 + 12 + 16;
	for (i = 0; i < 2; i++)
	{
		pAd->TxPower[3 * i + choffset + 0].Channel	= 149 + i * 8 + 0;
		pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 1].Channel	= 149 + i * 8 + 2;
		pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * i + choffset + 2].Channel	= 149 + i * 8 + 4;
		pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
	}
	pAd->TxPower[3 * 2 + choffset + 0].Channel		= 165;
	pAd->TxPower[3 * 2 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
	pAd->TxPower[3 * 2 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;

	
	for (i = 0; i < 4; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);

		if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

		if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;

		if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
			pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
			pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;
	}

	
	choffset = 14 + 12 + 16 + 7;
}


NDIS_STATUS	NICReadRegParameters(
	IN	PRTMP_ADAPTER		pAd,
	IN	NDIS_HANDLE			WrapperConfigurationContext
	)
{
	NDIS_STATUS						Status = NDIS_STATUS_SUCCESS;
	DBGPRINT_S(Status, ("<-- NICReadRegParameters, Status=%x\n", Status));
	return Status;
}


#ifdef RT2870

VOID RTMPFilterCalibration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR	R55x = 0, value, FilterTarget = 0x1E, BBPValue=0;
	UINT	loop = 0, count = 0, loopcnt = 0, ReTry = 0;
	UCHAR	RF_R24_Value = 0;

	
#ifndef RT2870
	pAd->Mlme.CaliBW20RfR24 = 0x16;
	pAd->Mlme.CaliBW40RfR24 = 0x36;  
#else
	pAd->Mlme.CaliBW20RfR24 = 0x1F;
	pAd->Mlme.CaliBW40RfR24 = 0x2F; 
#endif
	do
	{
		if (loop == 1)	
		{
			
			RF_R24_Value = 0x27;
			RT30xxWriteRFRegister(pAd, RF_R24, RF_R24_Value);
			if (IS_RT3090(pAd))
				FilterTarget = 0x15;
			else
				FilterTarget = 0x19;

			
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue&= (~0x18);
			BBPValue|= (0x10);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
#ifdef RT2870
			
			RT30xxReadRFRegister(pAd, RF_R31, &value);
			value |= 0x20;
			RT30xxWriteRFRegister(pAd, RF_R31, value);
#endif
		}
		else			
		{
			
			RF_R24_Value = 0x07;
			RT30xxWriteRFRegister(pAd, RF_R24, RF_R24_Value);
			if (IS_RT3090(pAd))
				FilterTarget = 0x13;
			else
				FilterTarget = 0x16;
#ifdef RT2870
			
			RT30xxReadRFRegister(pAd, RF_R31, &value);
			value &= (~0x20);
			RT30xxWriteRFRegister(pAd, RF_R31, value);
#endif
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

VOID NICInitRT30xxRFRegisters(IN PRTMP_ADAPTER pAd)
{
	INT i;
	
	
	if (IS_RT3070(pAd) || IS_RT3071(pAd))
	{
		
		
		UINT32 RfReg = 0;
		UINT32 data;

		RT30xxReadRFRegister(pAd, RF_R30, (PUCHAR)&RfReg);
		RfReg |= 0x80;
		RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);
		RTMPusecDelay(1000);
		RfReg &= 0x7F;
		RT30xxWriteRFRegister(pAd, RF_R30, (UCHAR)RfReg);

		
		for (i = 0; i < NUM_RF_REG_PARMS; i++)
		{
			RT30xxWriteRFRegister(pAd, RT30xx_RFRegTable[i].Register, RT30xx_RFRegTable[i].Value);
		}

		if (IS_RT3070(pAd))
		{
			
			RTUSBReadMACRegister(pAd, LDO_CFG0, &data);
			data = ((data & 0xF0FFFFFF) | 0x0D000000);
			RTUSBWriteMACRegister(pAd, LDO_CFG0, data);
		}
		else if (IS_RT3071(pAd))
		{
			
			RT30xxReadRFRegister(pAd, RF_R06, (PUCHAR)&RfReg);
			RfReg |= 0x40;
			RT30xxWriteRFRegister(pAd, RF_R06, (UCHAR)RfReg);

			
			RT30xxWriteRFRegister(pAd, RF_R31, 0x14);

			
			if ((pAd->NicConfig2.field.DACTestBit == 1) && ((pAd->MACVersion & 0xffff) < 0x0211))
			{
				
				RTUSBReadMACRegister(pAd, LDO_CFG0, &data);
				data = ((data & 0xE0FFFFFF) | 0x0D000000);
				RTUSBWriteMACRegister(pAd, LDO_CFG0, data);
			}
			else
			{
				RTMP_IO_READ32(pAd, LDO_CFG0, &data);
				data = ((data & 0xE0FFFFFF) | 0x01000000);
				RTMP_IO_WRITE32(pAd, LDO_CFG0, data);
			}

			
			RTUSBReadMACRegister(pAd, GPIO_SWITCH, &data);
			data &= ~(0x20);
			RTUSBWriteMACRegister(pAd, GPIO_SWITCH, data);
		}

		
		RTMPFilterCalibration(pAd);

		
		if ((pAd->MACVersion & 0xffff) < 0x0211)
			RT30xxWriteRFRegister(pAd, RF_R27, 0x3);

		
		RTUSBReadMACRegister(pAd, OPT_14, &data);
		data |= 0x01;
		RTUSBWriteMACRegister(pAd, OPT_14, data);

		if (IS_RT3071(pAd))
		{
			
			RT30xxLoadRFNormalModeSetup(pAd);
		}
	}
}
#endif 



VOID	NICReadEEPROMParameters(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			mac_addr)
{
	UINT32			data = 0;
	USHORT			i, value, value2;
	UCHAR			TmpPhy;
	EEPROM_TX_PWR_STRUC	    Power;
	EEPROM_VERSION_STRUC    Version;
	EEPROM_ANTENNA_STRUC	Antenna;
	EEPROM_NIC_CONFIG2_STRUC    NicConfig2;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICReadEEPROMParameters\n"));

	
	RTMP_IO_READ32(pAd, E2PROM_CSR, &data);
	DBGPRINT(RT_DEBUG_TRACE, ("--> E2PROM_CSR = 0x%x\n", data));

	if((data & 0x30) == 0)
		pAd->EEPROMAddressNum = 6;		
	else if((data & 0x30) == 0x10)
		pAd->EEPROMAddressNum = 8;     
	else
		pAd->EEPROMAddressNum = 8;     
	DBGPRINT(RT_DEBUG_TRACE, ("--> EEPROMAddressNum = %d\n", pAd->EEPROMAddressNum ));

	
	
	if (mac_addr == NULL ||
		strlen(mac_addr) != 17 ||
		mac_addr[2] != ':'  || mac_addr[5] != ':'  || mac_addr[8] != ':' ||
		mac_addr[11] != ':' || mac_addr[14] != ':')
	{
		USHORT  Addr01,Addr23,Addr45 ;

		RT28xx_EEPROM_READ16(pAd, 0x04, Addr01);
		RT28xx_EEPROM_READ16(pAd, 0x06, Addr23);
		RT28xx_EEPROM_READ16(pAd, 0x08, Addr45);

		pAd->PermanentAddress[0] = (UCHAR)(Addr01 & 0xff);
		pAd->PermanentAddress[1] = (UCHAR)(Addr01 >> 8);
		pAd->PermanentAddress[2] = (UCHAR)(Addr23 & 0xff);
		pAd->PermanentAddress[3] = (UCHAR)(Addr23 >> 8);
		pAd->PermanentAddress[4] = (UCHAR)(Addr45 & 0xff);
		pAd->PermanentAddress[5] = (UCHAR)(Addr45 >> 8);

		DBGPRINT(RT_DEBUG_TRACE, ("Initialize MAC Address from E2PROM \n"));
	}
	else
	{
		INT		j;
		PUCHAR	macptr;

		macptr = mac_addr;

		for (j=0; j<MAC_ADDR_LEN; j++)
		{
			AtoH(macptr, &pAd->PermanentAddress[j], 1);
			macptr=macptr+3;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("Initialize MAC Address from module parameter \n"));
	}


	{
		
		if (pAd->PermanentAddress[0] == 0xff)
			pAd->PermanentAddress[0] = RandomByte(pAd)&0xf8;

		
		

		DBGPRINT_RAW(RT_DEBUG_TRACE,("E2PROM MAC: =%02x:%02x:%02x:%02x:%02x:%02x\n",
			pAd->PermanentAddress[0], pAd->PermanentAddress[1],
			pAd->PermanentAddress[2], pAd->PermanentAddress[3],
			pAd->PermanentAddress[4], pAd->PermanentAddress[5]));
		if (pAd->bLocalAdminMAC == FALSE)
		{
			MAC_DW0_STRUC csr2;
			MAC_DW1_STRUC csr3;
			COPY_MAC_ADDR(pAd->CurrentAddress, pAd->PermanentAddress);
			csr2.field.Byte0 = pAd->CurrentAddress[0];
			csr2.field.Byte1 = pAd->CurrentAddress[1];
			csr2.field.Byte2 = pAd->CurrentAddress[2];
			csr2.field.Byte3 = pAd->CurrentAddress[3];
			RTMP_IO_WRITE32(pAd, MAC_ADDR_DW0, csr2.word);
			csr3.word = 0;
			csr3.field.Byte4 = pAd->CurrentAddress[4];
			csr3.field.Byte5 = pAd->CurrentAddress[5];
			csr3.field.U2MeMask = 0xff;
			RTMP_IO_WRITE32(pAd, MAC_ADDR_DW1, csr3.word);
			DBGPRINT_RAW(RT_DEBUG_TRACE,("E2PROM MAC: =%02x:%02x:%02x:%02x:%02x:%02x\n",
				pAd->PermanentAddress[0], pAd->PermanentAddress[1],
				pAd->PermanentAddress[2], pAd->PermanentAddress[3],
				pAd->PermanentAddress[4], pAd->PermanentAddress[5]));
		}
	}

	
	
	RTMPReadChannelPwr(pAd);

	
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_VERSION_OFFSET, Version.word);
	pAd->EepromVersion = Version.field.Version + Version.field.FaeReleaseNumber * 256;
	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: Version = %d, FAE release #%d\n", Version.field.Version, Version.field.FaeReleaseNumber));

	if (Version.field.Version > VALID_EEPROM_VERSION)
	{
		DBGPRINT_ERR(("E2PROM: WRONG VERSION 0x%x, should be %d\n",Version.field.Version, VALID_EEPROM_VERSION));
		
	}

	
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET, value);
	pAd->EEPROMDefaultValue[0] = value;

	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC2_OFFSET, value);
	pAd->EEPROMDefaultValue[1] = value;

	RT28xx_EEPROM_READ16(pAd, 0x38, value);	
	pAd->EEPROMDefaultValue[2] = value;

	for(i = 0; i < 8; i++)
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_BBP_BASE_OFFSET + i*2, value);
		pAd->EEPROMDefaultValue[i+3] = value;
	}

	
	
	
	
	Antenna.word = pAd->EEPROMDefaultValue[0];
	if (Antenna.word == 0xFFFF)
	{
		if(IS_RT3090(pAd))
		{
			Antenna.word = 0;
			Antenna.field.RfIcType = RFIC_3020;
			Antenna.field.TxPath = 1;
			Antenna.field.RxPath = 1;
		}
		else
		{
		Antenna.word = 0;
		Antenna.field.RfIcType = RFIC_2820;
		Antenna.field.TxPath = 1;
		Antenna.field.RxPath = 2;
		DBGPRINT(RT_DEBUG_WARN, ("E2PROM error, hard code as 0x%04x\n", Antenna.word));
		}
	}

	
	if ((pAd->CommonCfg.TxStream == 0) || (pAd->CommonCfg.TxStream > Antenna.field.TxPath))
		pAd->CommonCfg.TxStream = Antenna.field.TxPath;

	if ((pAd->CommonCfg.RxStream == 0) || (pAd->CommonCfg.RxStream > Antenna.field.RxPath))
	{
		pAd->CommonCfg.RxStream = Antenna.field.RxPath;

		if ((pAd->MACVersion < RALINK_2883_VERSION) &&
			(pAd->CommonCfg.RxStream > 2))
		{
			
			pAd->CommonCfg.RxStream = 2;
		}
	}

	
	
	
	for(i=0; i<3; i++)
	{
	}

	NicConfig2.word = pAd->EEPROMDefaultValue[1];

	{
		if ((NicConfig2.word & 0x00ff) == 0xff)
		{
			NicConfig2.word &= 0xff00;
		}

		if ((NicConfig2.word >> 8) == 0xff)
		{
			NicConfig2.word &= 0x00ff;
		}
	}

	if (NicConfig2.field.DynamicTxAgcControl == 1)
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("NICReadEEPROMParameters: RxPath = %d, TxPath = %d\n", Antenna.field.RxPath, Antenna.field.TxPath));

	
	pAd->Antenna.word = Antenna.word;

	
	
	
	
	if ((Antenna.field.RfIcType != RFIC_2850) && (Antenna.field.RfIcType != RFIC_2750))
	{
		if ((pAd->CommonCfg.PhyMode == PHY_11ABG_MIXED) ||
			(pAd->CommonCfg.PhyMode == PHY_11A))
			pAd->CommonCfg.PhyMode = PHY_11BG_MIXED;
		else if ((pAd->CommonCfg.PhyMode == PHY_11ABGN_MIXED)	||
				 (pAd->CommonCfg.PhyMode == PHY_11AN_MIXED) 	||
				 (pAd->CommonCfg.PhyMode == PHY_11AGN_MIXED) 	||
				 (pAd->CommonCfg.PhyMode == PHY_11N_5G))
			pAd->CommonCfg.PhyMode = PHY_11BGN_MIXED;
	}

	
	
	{
		
		RT28xx_EEPROM_READ16(pAd, 0x6E, Power.word);
		pAd->TssiMinusBoundaryG[4] = Power.field.Byte0;
		pAd->TssiMinusBoundaryG[3] = Power.field.Byte1;
		RT28xx_EEPROM_READ16(pAd, 0x70, Power.word);
		pAd->TssiMinusBoundaryG[2] = Power.field.Byte0;
		pAd->TssiMinusBoundaryG[1] = Power.field.Byte1;
		RT28xx_EEPROM_READ16(pAd, 0x72, Power.word);
		pAd->TssiRefG   = Power.field.Byte0; 
		pAd->TssiPlusBoundaryG[1] = Power.field.Byte1;
		RT28xx_EEPROM_READ16(pAd, 0x74, Power.word);
		pAd->TssiPlusBoundaryG[2] = Power.field.Byte0;
		pAd->TssiPlusBoundaryG[3] = Power.field.Byte1;
		RT28xx_EEPROM_READ16(pAd, 0x76, Power.word);
		pAd->TssiPlusBoundaryG[4] = Power.field.Byte0;
		pAd->TxAgcStepG = Power.field.Byte1;
		pAd->TxAgcCompensateG = 0;
		pAd->TssiMinusBoundaryG[0] = pAd->TssiRefG;
		pAd->TssiPlusBoundaryG[0]  = pAd->TssiRefG;

		
		if (pAd->TssiRefG == 0xff)
			pAd->bAutoTxAgcG = FALSE;

		DBGPRINT(RT_DEBUG_TRACE,("E2PROM: G Tssi[-4 .. +4] = %d %d %d %d - %d -%d %d %d %d, step=%d, tuning=%d\n",
			pAd->TssiMinusBoundaryG[4], pAd->TssiMinusBoundaryG[3], pAd->TssiMinusBoundaryG[2], pAd->TssiMinusBoundaryG[1],
			pAd->TssiRefG,
			pAd->TssiPlusBoundaryG[1], pAd->TssiPlusBoundaryG[2], pAd->TssiPlusBoundaryG[3], pAd->TssiPlusBoundaryG[4],
			pAd->TxAgcStepG, pAd->bAutoTxAgcG));
	}
	
	{
		RT28xx_EEPROM_READ16(pAd, 0xD4, Power.word);
		pAd->TssiMinusBoundaryA[4] = Power.field.Byte0;
		pAd->TssiMinusBoundaryA[3] = Power.field.Byte1;
		RT28xx_EEPROM_READ16(pAd, 0xD6, Power.word);
		pAd->TssiMinusBoundaryA[2] = Power.field.Byte0;
		pAd->TssiMinusBoundaryA[1] = Power.field.Byte1;
		RT28xx_EEPROM_READ16(pAd, 0xD8, Power.word);
		pAd->TssiRefA   = Power.field.Byte0;
		pAd->TssiPlusBoundaryA[1] = Power.field.Byte1;
		RT28xx_EEPROM_READ16(pAd, 0xDA, Power.word);
		pAd->TssiPlusBoundaryA[2] = Power.field.Byte0;
		pAd->TssiPlusBoundaryA[3] = Power.field.Byte1;
		RT28xx_EEPROM_READ16(pAd, 0xDC, Power.word);
		pAd->TssiPlusBoundaryA[4] = Power.field.Byte0;
		pAd->TxAgcStepA = Power.field.Byte1;
		pAd->TxAgcCompensateA = 0;
		pAd->TssiMinusBoundaryA[0] = pAd->TssiRefA;
		pAd->TssiPlusBoundaryA[0]  = pAd->TssiRefA;

		
		if (pAd->TssiRefA == 0xff)
			pAd->bAutoTxAgcA = FALSE;

		DBGPRINT(RT_DEBUG_TRACE,("E2PROM: A Tssi[-4 .. +4] = %d %d %d %d - %d -%d %d %d %d, step=%d, tuning=%d\n",
			pAd->TssiMinusBoundaryA[4], pAd->TssiMinusBoundaryA[3], pAd->TssiMinusBoundaryA[2], pAd->TssiMinusBoundaryA[1],
			pAd->TssiRefA,
			pAd->TssiPlusBoundaryA[1], pAd->TssiPlusBoundaryA[2], pAd->TssiPlusBoundaryA[3], pAd->TssiPlusBoundaryA[4],
			pAd->TxAgcStepA, pAd->bAutoTxAgcA));
	}
	pAd->BbpRssiToDbmDelta = 0x0;

	
	RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, value);
	if ((value & 0x00FF) != 0x00FF)
		pAd->RfFreqOffset = (ULONG) (value & 0x00FF);
	else
		pAd->RfFreqOffset = 0;
	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: RF FreqOffset=0x%lx \n", pAd->RfFreqOffset));

	
	value = pAd->EEPROMDefaultValue[2] >> 8;		
	value2 = pAd->EEPROMDefaultValue[2] & 0x00FF;	

	if ((value <= REGION_MAXIMUM_BG_BAND) && (value2 <= REGION_MAXIMUM_A_BAND))
	{
		pAd->CommonCfg.CountryRegion = ((UCHAR) value) | 0x80;
		pAd->CommonCfg.CountryRegionForABand = ((UCHAR) value2) | 0x80;
		TmpPhy = pAd->CommonCfg.PhyMode;
		pAd->CommonCfg.PhyMode = 0xff;
		RTMPSetPhyMode(pAd, TmpPhy);
		SetCommonHT(pAd);
	}

	
	
	
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET, value);
	pAd->BGRssiOffset0 = value & 0x00ff;
	pAd->BGRssiOffset1 = (value >> 8);
	RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET+2, value);
	pAd->BGRssiOffset2 = value & 0x00ff;
	pAd->ALNAGain1 = (value >> 8);
	RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET, value);
	pAd->BLNAGain = value & 0x00ff;
	pAd->ALNAGain0 = (value >> 8);

	
	if ((pAd->BGRssiOffset0 < -10) || (pAd->BGRssiOffset0 > 10))
		pAd->BGRssiOffset0 = 0;

	
	if ((pAd->BGRssiOffset1 < -10) || (pAd->BGRssiOffset1 > 10))
		pAd->BGRssiOffset1 = 0;

	
	if ((pAd->BGRssiOffset2 < -10) || (pAd->BGRssiOffset2 > 10))
		pAd->BGRssiOffset2 = 0;

	RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_A_OFFSET, value);
	pAd->ARssiOffset0 = value & 0x00ff;
	pAd->ARssiOffset1 = (value >> 8);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_A_OFFSET+2), value);
	pAd->ARssiOffset2 = value & 0x00ff;
	pAd->ALNAGain2 = (value >> 8);

	if (((UCHAR)pAd->ALNAGain1 == 0xFF) || (pAd->ALNAGain1 == 0x00))
		pAd->ALNAGain1 = pAd->ALNAGain0;
	if (((UCHAR)pAd->ALNAGain2 == 0xFF) || (pAd->ALNAGain2 == 0x00))
		pAd->ALNAGain2 = pAd->ALNAGain0;

	
	if ((pAd->ARssiOffset0 < -10) || (pAd->ARssiOffset0 > 10))
		pAd->ARssiOffset0 = 0;

	
	if ((pAd->ARssiOffset1 < -10) || (pAd->ARssiOffset1 > 10))
		pAd->ARssiOffset1 = 0;

	
	if ((pAd->ARssiOffset2 < -10) || (pAd->ARssiOffset2 > 10))
		pAd->ARssiOffset2 = 0;

	
	
	
	RT28xx_EEPROM_READ16(pAd, 0x3a, value);
	pAd->LedCntl.word = (value&0xff00) >> 8;
	RT28xx_EEPROM_READ16(pAd, EEPROM_LED1_OFFSET, value);
	pAd->Led1 = value;
	RT28xx_EEPROM_READ16(pAd, EEPROM_LED2_OFFSET, value);
	pAd->Led2 = value;
	RT28xx_EEPROM_READ16(pAd, EEPROM_LED3_OFFSET, value);
	pAd->Led3 = value;

	RTMPReadTxPwrPerRate(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICReadEEPROMParameters\n"));
}


VOID	NICInitAsicFromEEPROM(
	IN	PRTMP_ADAPTER	pAd)
{
	UINT32					data = 0;
	UCHAR	BBPR1 = 0;
	USHORT					i;
	EEPROM_ANTENNA_STRUC	Antenna;
	EEPROM_NIC_CONFIG2_STRUC    NicConfig2;
	UCHAR	BBPR3 = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitAsicFromEEPROM\n"));
	for(i = 3; i < NUM_EEPROM_BBP_PARMS; i++)
	{
		UCHAR BbpRegIdx, BbpValue;

		if ((pAd->EEPROMDefaultValue[i] != 0xFFFF) && (pAd->EEPROMDefaultValue[i] != 0))
		{
			BbpRegIdx = (UCHAR)(pAd->EEPROMDefaultValue[i] >> 8);
			BbpValue  = (UCHAR)(pAd->EEPROMDefaultValue[i] & 0xff);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BbpRegIdx, BbpValue);
		}
	}

#ifndef RT2870
	Antenna.word = pAd->Antenna.word;
#else
	Antenna.word = pAd->EEPROMDefaultValue[0];
	if (Antenna.word == 0xFFFF)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("E2PROM error, hard code as 0x%04x\n", Antenna.word));
		BUG_ON(Antenna.word == 0xFFFF);
	}
#endif
	pAd->Mlme.RealRxPath = (UCHAR) Antenna.field.RxPath;
	pAd->RfIcType = (UCHAR) Antenna.field.RfIcType;

#ifdef RT2870
	DBGPRINT(RT_DEBUG_WARN, ("pAd->RfIcType = %d, RealRxPath=%d, TxPath = %d\n", pAd->RfIcType, pAd->Mlme.RealRxPath,Antenna.field.TxPath));

	
	pAd->Antenna.word = Antenna.word;
#endif
	NicConfig2.word = pAd->EEPROMDefaultValue[1];

#ifdef RT2870
	{
		if ((NicConfig2.word & 0x00ff) == 0xff)
		{
			NicConfig2.word &= 0xff00;
		}

		if ((NicConfig2.word >> 8) == 0xff)
		{
			NicConfig2.word &= 0x00ff;
		}
	}
#endif
	
	pAd->NicConfig2.word = NicConfig2.word;

#ifdef RT2870
	
	if (pAd->RfIcType == RFIC_3020)
		AsicSetRxAnt(pAd, pAd->RxAnt.Pair1PrimaryRxAnt);
#endif
	
	
	
	if (pAd->LedCntl.word == 0xFF)
	{
		pAd->LedCntl.word = 0x01;
		pAd->Led1 = 0x5555;
		pAd->Led2 = 0x2221;
#ifdef RT2860
		pAd->Led3 = 0xA9F8;
#endif

#ifdef RT2870
		pAd->Led3 = 0x5627;
#endif 
	}

	AsicSendCommandToMcu(pAd, 0x52, 0xff, (UCHAR)pAd->Led1, (UCHAR)(pAd->Led1 >> 8));
	AsicSendCommandToMcu(pAd, 0x53, 0xff, (UCHAR)pAd->Led2, (UCHAR)(pAd->Led2 >> 8));
	AsicSendCommandToMcu(pAd, 0x54, 0xff, (UCHAR)pAd->Led3, (UCHAR)(pAd->Led3 >> 8));
    pAd->LedIndicatorStregth = 0xFF;
    RTMPSetSignalLED(pAd, -100);	

	{
		
		if (NicConfig2.field.HardwareRadioControl == 1)
		{
			pAd->StaCfg.bHardwareRadio = TRUE;

			
			RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &data);
			if ((data & 0x04) == 0)
			{
				pAd->StaCfg.bHwRadio = FALSE;
				pAd->StaCfg.bRadio = FALSE;
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
			}
		}
		else
			pAd->StaCfg.bHardwareRadio = FALSE;

		if (pAd->StaCfg.bRadio == FALSE)
		{
			RTMPSetLED(pAd, LED_RADIO_OFF);
		}
		else
		{
			RTMPSetLED(pAd, LED_RADIO_ON);
#ifdef RT2860
			AsicSendCommandToMcu(pAd, 0x30, 0xff, 0xff, 0x02);
			AsicSendCommandToMcu(pAd, 0x31, PowerWakeCID, 0x00, 0x00);
			
			AsicCheckCommanOk(pAd, PowerWakeCID);
#endif
		}
	}

	
	if (NicConfig2.field.CardbusAcceleration == 1)
	{
	}

	if (NicConfig2.field.DynamicTxAgcControl == 1)
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;

	
	pAd->CommonCfg.BandState = UNKNOWN_BAND;

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BBPR3);
	BBPR3 &= (~0x18);
	if(pAd->Antenna.field.RxPath == 3)
	{
		BBPR3 |= (0x10);
	}
	else if(pAd->Antenna.field.RxPath == 2)
	{
		BBPR3 |= (0x8);
	}
	else if(pAd->Antenna.field.RxPath == 1)
	{
		BBPR3 |= (0x0);
	}
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BBPR3);

	{
		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPR1);
		if(pAd->Antenna.field.TxPath == 1)
		{
		BBPR1 &= (~0x18);
		}
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPR1);

		DBGPRINT(RT_DEBUG_TRACE, ("Use Hw Radio Control Pin=%d; if used Pin=%d;\n", pAd->CommonCfg.bHardwareRadio, pAd->CommonCfg.bHardwareRadio));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("TxPath = %d, RxPath = %d, RFIC=%d, Polar+LED mode=%x\n", pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, pAd->RfIcType, pAd->LedCntl.word));
	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitAsicFromEEPROM\n"));
}


NDIS_STATUS	NICInitializeAdapter(
	IN	PRTMP_ADAPTER	pAd,
	IN   BOOLEAN    bHardReset)
{
	NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
	WPDMA_GLO_CFG_STRUC	GloCfg;
#ifdef RT2860
	UINT32			Value;
	DELAY_INT_CFG_STRUC	IntCfg;
#endif
	ULONG	i =0, j=0;
	AC_TXOP_CSR0_STRUC	csr0;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitializeAdapter\n"));

	
retry:
	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0))
			break;

		RTMPusecDelay(1000);
		i++;
	}while ( i<100);
	DBGPRINT(RT_DEBUG_TRACE, ("<== DMA offset 0x208 = 0x%x\n", GloCfg.word));
	GloCfg.word &= 0xff0;
	GloCfg.field.EnTXWriteBackDDONE =1;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);

	
	pAd->BeaconOffset[0] = HW_BEACON_BASE0;
	pAd->BeaconOffset[1] = HW_BEACON_BASE1;
	pAd->BeaconOffset[2] = HW_BEACON_BASE2;
	pAd->BeaconOffset[3] = HW_BEACON_BASE3;
	pAd->BeaconOffset[4] = HW_BEACON_BASE4;
	pAd->BeaconOffset[5] = HW_BEACON_BASE5;
	pAd->BeaconOffset[6] = HW_BEACON_BASE6;
	pAd->BeaconOffset[7] = HW_BEACON_BASE7;

	
	
	

	
	
#ifdef RT2860
	RTMP_IO_WRITE32(pAd, WPDMA_RST_IDX, 0x1003f);	
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe1f);
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe00);
#endif

	
	if (NICInitializeAsic(pAd , bHardReset) != NDIS_STATUS_SUCCESS)
	{
		if (j++ == 0)
		{
			NICLoadFirmware(pAd);
			goto retry;
		}
		return NDIS_STATUS_FAILURE;
	}


#ifdef RT2860
	
	Value = RTMP_GetPhysicalAddressLow(pAd->TxRing[QID_AC_BK].Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, TX_BASE_PTR1, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR1 : 0x%x\n", Value));

	
	Value = RTMP_GetPhysicalAddressLow(pAd->TxRing[QID_AC_BE].Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, TX_BASE_PTR0, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR0 : 0x%x\n", Value));

	
	Value = RTMP_GetPhysicalAddressLow(pAd->TxRing[QID_AC_VI].Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, TX_BASE_PTR2, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR2 : 0x%x\n", Value));

	
	Value = RTMP_GetPhysicalAddressLow(pAd->TxRing[QID_AC_VO].Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, TX_BASE_PTR3, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR3 : 0x%x\n", Value));

	
	  Value = RTMP_GetPhysicalAddressLow(pAd->TxRing[QID_HCCA].Cell[0].AllocPa);
	  RTMP_IO_WRITE32(pAd, TX_BASE_PTR4, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR4 : 0x%x\n", Value));

	
	Value = RTMP_GetPhysicalAddressLow(pAd->MgmtRing.Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, TX_BASE_PTR5, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> TX_BASE_PTR5 : 0x%x\n", Value));

	
	Value = RTMP_GetPhysicalAddressLow(pAd->RxRing.Cell[0].AllocPa);
	RTMP_IO_WRITE32(pAd, RX_BASE_PTR, Value);
	DBGPRINT(RT_DEBUG_TRACE, ("--> RX_BASE_PTR : 0x%x\n", Value));

	
	pAd->RxRing.RxSwReadIdx = 0;
	pAd->RxRing.RxCpuIdx = RX_RING_SIZE-1;
	RTMP_IO_WRITE32(pAd, RX_CRX_IDX, pAd->RxRing.RxCpuIdx);

	
	{
		for (i=0; i<NUM_OF_TX_RING; i++)
		{
			pAd->TxRing[i].TxSwFreeIdx = 0;
			pAd->TxRing[i].TxCpuIdx = 0;
			RTMP_IO_WRITE32(pAd, (TX_CTX_IDX0 + i * 0x10) ,  pAd->TxRing[i].TxCpuIdx);
		}
	}

	
	pAd->MgmtRing.TxSwFreeIdx = 0;
	pAd->MgmtRing.TxCpuIdx = 0;
	RTMP_IO_WRITE32(pAd, TX_MGMTCTX_IDX,  pAd->MgmtRing.TxCpuIdx);

	
	
	

	
	Value = TX_RING_SIZE;
	RTMP_IO_WRITE32(pAd, TX_MAX_CNT0, Value);
	RTMP_IO_WRITE32(pAd, TX_MAX_CNT1, Value);
	RTMP_IO_WRITE32(pAd, TX_MAX_CNT2, Value);
	RTMP_IO_WRITE32(pAd, TX_MAX_CNT3, Value);
	RTMP_IO_WRITE32(pAd, TX_MAX_CNT4, Value);
	Value = MGMT_RING_SIZE;
	RTMP_IO_WRITE32(pAd, TX_MGMTMAX_CNT, Value);

	
	Value = RX_RING_SIZE;
	RTMP_IO_WRITE32(pAd, RX_MAX_CNT, Value);
#endif 


	
	csr0.word = 0;
	RTMP_IO_WRITE32(pAd, WMM_TXOP0_CFG, csr0.word);
	if (pAd->CommonCfg.PhyMode == PHY_11B)
	{
		csr0.field.Ac0Txop = 192;	
		csr0.field.Ac1Txop = 96;	
	}
	else
	{
		csr0.field.Ac0Txop = 96;	
		csr0.field.Ac1Txop = 48;	
	}
	RTMP_IO_WRITE32(pAd, WMM_TXOP1_CFG, csr0.word);


#ifdef RT2860
	
	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0))
			break;

		RTMPusecDelay(1000);
		i++;
	}while ( i < 100);

	GloCfg.word &= 0xff0;
	GloCfg.field.EnTXWriteBackDDONE =1;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);

	IntCfg.word = 0;
	RTMP_IO_WRITE32(pAd, DELAY_INT_CFG, IntCfg.word);
#endif


	
	
	

	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitializeAdapter\n"));
	return Status;
}


NDIS_STATUS	NICInitializeAsic(
	IN	PRTMP_ADAPTER	pAd,
	IN  BOOLEAN		bHardReset)
{
	ULONG			Index = 0;
	UCHAR			R0 = 0xff;
	UINT32			MacCsr12 = 0, Counter = 0;
#ifdef RT2870
	UINT32			MacCsr0 = 0;
	NTSTATUS		Status;
	UCHAR			Value = 0xff;
	UINT32			eFuseCtrl;
#endif
	USHORT			KeyIdx;
	INT				i,apidx;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitializeAsic\n"));

#ifdef RT2860
	if (bHardReset == TRUE)
	{
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x3);
	}
	else
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x1);
#endif
#ifdef RT2870
	
	
	
	Index = 0;

	
	
	do
	{
		RTMP_IO_READ32(pAd, MAC_CSR0, &MacCsr0);

		if ((MacCsr0 != 0x00) && (MacCsr0 != 0xFFFFFFFF))
			break;

		RTMPusecDelay(10);
	} while (Index++ < 100);

	pAd->MACVersion = MacCsr0;
	DBGPRINT(RT_DEBUG_TRACE, ("MAC_CSR0  [ Ver:Rev=0x%08x]\n", pAd->MACVersion));
	
	RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &MacCsr12);
	MacCsr12 &= (~0x2000);
	RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, MacCsr12);

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x3);
	RTMP_IO_WRITE32(pAd, USB_DMA_CFG, 0x0);
	Status = RTUSBVenderReset(pAd);
#endif

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);

	
#ifdef RT2860
	for (Index = 0; Index < NUM_MAC_REG_PARMS; Index++)
	{
		RTMP_IO_WRITE32(pAd, MACRegTable[Index].Register, MACRegTable[Index].Value);
	}
#endif
#ifdef RT2870
	for(Index=0; Index<NUM_MAC_REG_PARMS; Index++)
	{
#ifdef RT3070
		if ((MACRegTable[Index].Register == TX_SW_CFG0) && (IS_RT3070(pAd) || IS_RT3071(pAd)))
		{
			MACRegTable[Index].Value = 0x00000400;
		}
#endif 
		RTMP_IO_WRITE32(pAd, (USHORT)MACRegTable[Index].Register, MACRegTable[Index].Value);
	}
#endif 

	{
		for (Index = 0; Index < NUM_STA_MAC_REG_PARMS; Index++)
		{
#ifdef RT2860
			RTMP_IO_WRITE32(pAd, STAMACRegTable[Index].Register, STAMACRegTable[Index].Value);
#endif
#ifdef RT2870
			RTMP_IO_WRITE32(pAd, (USHORT)STAMACRegTable[Index].Register, STAMACRegTable[Index].Value);
#endif
		}
	}

	
	if (IS_RT3090(pAd))
	{
		RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0);

		
		if ((pAd->MACVersion & 0xffff) < 0x0211)
		{
			if (pAd->NicConfig2.field.DACTestBit == 1)
			{
				RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x1F);	
			}
			else
			{
				RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0F);	
			}
		}
		else
		{
			RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x0);
		}
	}
#ifdef RT2870
	else if (IS_RT3070(pAd))
	{
		RTMP_IO_WRITE32(pAd, TX_SW_CFG1, 0);
		RTMP_IO_WRITE32(pAd, TX_SW_CFG2, 0x1F);	
	}
#endif 

	
	
	
	Index = 0;
	do
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacCsr12);

		if ((MacCsr12 & 0x03) == 0)	
			break;

		DBGPRINT(RT_DEBUG_TRACE, ("Check MAC_STATUS_CFG  = Busy = %x\n", MacCsr12));
		RTMPusecDelay(1000);
	} while (Index++ < 100);

    
	
	RTMP_IO_WRITE32(pAd, H2M_BBP_AGENT, 0);	
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CSR, 0);
	RTMPusecDelay(1000);

	
	Index = 0;
	do
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R0, &R0);
		DBGPRINT(RT_DEBUG_TRACE, ("BBP version = %x\n", R0));
	} while ((++Index < 20) && ((R0 == 0xff) || (R0 == 0x00)));
	

	if ((R0 == 0xff) || (R0 == 0x00))
		return NDIS_STATUS_FAILURE;

	
	for (Index = 0; Index < NUM_BBP_REG_PARMS; Index++)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBPRegTable[Index].Register, BBPRegTable[Index].Value);
	}

#ifndef RT2870
	
	if ((pAd->MACVersion&0xffff) != 0x0101)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R84, 0x19);
#else
	
	
	if (((pAd->MACVersion&0xffff) != 0x0101) && (!IS_RT30xx(pAd)))
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R84, 0x19);


	if (IS_RT30xx(pAd))
	{	
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R79, 0x13);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R80, 0x05);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R81, 0x33);
	}

	if (IS_RT3090(pAd))
	{
		UCHAR		bbpreg=0;

		
		if ((pAd->MACVersion & 0xffff) >= 0x0211)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R103, 0xc0);
		}

		
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R138, &bbpreg);
		if (pAd->Antenna.field.TxPath == 1)
		{
			
			bbpreg = (bbpreg | 0x20);
		}

		if (pAd->Antenna.field.RxPath == 1)
		{
			
			bbpreg &= (~0x2);
		}
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R138, bbpreg);

		
		if ((pAd->MACVersion & 0xffff) >= 0x0211)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R31, &bbpreg);
			bbpreg &= (~0x3);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R31, bbpreg);
		}
	}
#endif
	if (pAd->MACVersion == 0x28600100)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x12);
    }

	if (pAd->MACVersion >= RALINK_2880E_VERSION && pAd->MACVersion < RALINK_3070_VERSION) 
	{
		
		UINT32 csr;
		RTMP_IO_READ32(pAd, MAX_LEN_CFG, &csr);
		csr &= 0xFFF;
		csr |= 0x2000;
		RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, csr);
	}

#ifdef RT2870
{
	UCHAR	MAC_Value[]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0,0};

	
	Value = 0xff;
	for(Index =0 ;Index < 254;Index++)
	{
		RTUSBMultiWrite(pAd, (USHORT)(MAC_WCID_BASE + Index * 8), MAC_Value, 8);
	}
}
#endif 

	
	{
		if (pAd->StaCfg.bRadio == FALSE)
		{

			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
			DBGPRINT(RT_DEBUG_TRACE, ("Set Radio Off\n"));
		}
	}

	
	RTMP_IO_READ32(pAd, RX_STA_CNT0, &Counter);
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &Counter);
	RTMP_IO_READ32(pAd, RX_STA_CNT2, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT0, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT1, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT2, &Counter);

	
	
	
	if (bHardReset)
	{
		for (KeyIdx = 0; KeyIdx < 4; KeyIdx++)
		{
			RTMP_IO_WRITE32(pAd, SHARED_KEY_MODE_BASE + 4*KeyIdx, 0);
		}

		
		for (KeyIdx = 0; KeyIdx < 256; KeyIdx++)
		{
			RTMP_IO_WRITE32(pAd, MAC_WCID_ATTRIBUTE_BASE + (KeyIdx * HW_WCID_ATTRI_SIZE), 1);
		}
	}


	
	if (bHardReset == TRUE)
	{
		
		for (apidx = 0; apidx < HW_BEACON_MAX_COUNT; apidx++)
		{
			for (i = 0; i < HW_BEACON_OFFSET>>2; i+=4)
				RTMP_IO_WRITE32(pAd, pAd->BeaconOffset[apidx] + i, 0x00);
		}
	}
#ifdef RT2870
	AsicDisableSync(pAd);
	
	RTMP_IO_READ32(pAd, RX_STA_CNT0, &Counter);
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &Counter);
	RTMP_IO_READ32(pAd, RX_STA_CNT2, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT0, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT1, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT2, &Counter);
	
	RTMP_IO_READ32(pAd, USB_CYC_CFG, &Counter);
	Counter&=0xffffff00;
	Counter|=0x000001e;
	RTMP_IO_WRITE32(pAd, USB_CYC_CFG, Counter);

	pAd->bUseEfuse=FALSE;
	RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrl);
	pAd->bUseEfuse = ( (eFuseCtrl & 0x80000000) == 0x80000000) ? 1 : 0;
	if(pAd->bUseEfuse)
	{
			DBGPRINT(RT_DEBUG_TRACE, ("NVM is Efuse\n"));
	}
	else
	{
			DBGPRINT(RT_DEBUG_TRACE, ("NVM is EEPROM\n"));
	}
#endif

	{
		
		if ((pAd->MACVersion&0xffff) != 0x0101)
			RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, 0x583f);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitializeAsic\n"));
	return NDIS_STATUS_SUCCESS;
}


#ifdef RT2860
VOID NICRestoreBBPValue(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR		index;
	UCHAR		Value = 0;
	ULONG		Data;

	DBGPRINT(RT_DEBUG_TRACE, ("--->  NICRestoreBBPValue !!!!!!!!!!!!!!!!!!!!!!!  \n"));
	
	for (index = 0; index < NUM_BBP_REG_PARMS; index++)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBPRegTable[index].Register, BBPRegTable[index].Value);
	}
	
	if (pAd->MACVersion == 0x28600100)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x12);
	}

	
	if (INFRA_ON(pAd))
	{
		
		if ((pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel) && (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
		{
			
			pAd->CommonCfg.BBPCurrentBW = BW_40;
			AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
			Value &= (~0x18);
			Value |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

			
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
			Value &= (~0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
			
			pAd->StaCfg.BBPR3 = Value;

			RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
			Data &= 0xfffffffe;
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);

			if (pAd->MACVersion == 0x28600100)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
				DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
			}

			DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Lower LINK UP !!! Control Channel at Below. Central = %d \n", pAd->CommonCfg.CentralChannel ));
		}
		else if ((pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel) && (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
		{
			
			pAd->CommonCfg.BBPCurrentBW = BW_40;
			AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
			Value &= (~0x18);
			Value |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

			RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
			Data |= 0x1;
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
			Value |= (0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
			
			pAd->StaCfg.BBPR3 = Value;

			if (pAd->MACVersion == 0x28600100)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
				DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
			}

			DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Upper LINK UP !!! Control Channel at UpperCentral = %d \n", pAd->CommonCfg.CentralChannel ));
		}
		else
		{
			pAd->CommonCfg.BBPCurrentBW = BW_20;
			AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.Channel);

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
			Value &= (~0x18);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

			RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
			Data &= 0xfffffffe;
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);

			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
			Value &= (~0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
			
			pAd->StaCfg.BBPR3 = Value;

			if (pAd->MACVersion == 0x28600100)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x08);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x11);
				DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
			}

			DBGPRINT(RT_DEBUG_TRACE, ("!!!20MHz LINK UP !!! \n" ));
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<---  NICRestoreBBPValue !!!!!!!!!!!!!!!!!!!!!!!  \n"));
}
#endif 


VOID	NICIssueReset(
	IN	PRTMP_ADAPTER	pAd)
{
	UINT32	Value = 0;
	DBGPRINT(RT_DEBUG_TRACE, ("--> NICIssueReset\n"));

	
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Value);
	Value &= (0xfffffff3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Value);

	
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x03); 
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);

	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICIssueReset\n"));
}


BOOLEAN	NICCheckForHang(
	IN	PRTMP_ADAPTER	pAd)
{
	return (FALSE);
}

VOID NICUpdateFifoStaCounters(
	IN PRTMP_ADAPTER pAd)
{
	TX_STA_FIFO_STRUC	StaFifo;
	MAC_TABLE_ENTRY		*pEntry;
	UCHAR				i = 0;
	UCHAR			pid = 0, wcid = 0;
	CHAR				reTry;
	UCHAR				succMCS;

		do
		{
			RTMP_IO_READ32(pAd, TX_STA_FIFO, &StaFifo.word);

			if (StaFifo.field.bValid == 0)
				break;

			wcid = (UCHAR)StaFifo.field.wcid;


		
			if ((StaFifo.field.TxAckRequired == 0) || (wcid >= MAX_LEN_OF_MAC_TABLE))
			{
				i++;
				continue;
			}

			
			pid = (UCHAR)StaFifo.field.PidType;

			pEntry = &pAd->MacTab.Content[wcid];

			pEntry->DebugFIFOCount++;

			if (StaFifo.field.TxBF) 
				pEntry->TxBFCount++;

#ifdef UAPSD_AP_SUPPORT
			UAPSD_SP_AUE_Handle(pAd, pEntry, StaFifo.field.TxSuccess);
#endif 

			if (!StaFifo.field.TxSuccess)
			{
				pEntry->FIFOCount++;
				pEntry->OneSecTxFailCount++;

				if (pEntry->FIFOCount >= 1)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("#"));
					pEntry->NoBADataCountDown = 64;

					if(pEntry->PsMode == PWR_ACTIVE)
					{
						int tid;
						for (tid=0; tid<NUM_OF_TID; tid++)
						{
							BAOriSessionTearDown(pAd, pEntry->Aid,  tid, FALSE, FALSE);
						}

						
						pEntry->ContinueTxFailCnt++;
					}
					else
					{
						
						
						pEntry->FIFOCount = 0;
						pEntry->ContinueTxFailCnt = 0;
					}
				}
			}
			else
			{
				if ((pEntry->PsMode != PWR_SAVE) && (pEntry->NoBADataCountDown > 0))
				{
					pEntry->NoBADataCountDown--;
					if (pEntry->NoBADataCountDown==0)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("@\n"));
					}
				}

				pEntry->FIFOCount = 0;
				pEntry->OneSecTxNoRetryOkCount++;
				
				pEntry->NoDataIdleCount = 0;
				pEntry->ContinueTxFailCnt = 0;
			}

			succMCS = StaFifo.field.SuccessRate & 0x7F;

			reTry = pid - succMCS;

			if (StaFifo.field.TxSuccess)
			{
				pEntry->TXMCSExpected[pid]++;
				if (pid == succMCS)
				{
					pEntry->TXMCSSuccessful[pid]++;
				}
				else
				{
					pEntry->TXMCSAutoFallBack[pid][succMCS]++;
				}
			}
			else
			{
				pEntry->TXMCSFailed[pid]++;
			}

			if (reTry > 0)
			{
				if ((pid >= 12) && succMCS <=7)
				{
					reTry -= 4;
				}
				pEntry->OneSecTxRetryOkCount += reTry;
			}

			i++;
			
		} while ( i < (2*TX_RING_SIZE) );

}


VOID NICUpdateRawCounters(
	IN PRTMP_ADAPTER pAd)
{
	UINT32	OldValue;
	RX_STA_CNT0_STRUC	 RxStaCnt0;
	RX_STA_CNT1_STRUC   RxStaCnt1;
	RX_STA_CNT2_STRUC   RxStaCnt2;
	TX_STA_CNT0_STRUC 	 TxStaCnt0;
	TX_STA_CNT1_STRUC	 StaTx1;
	TX_STA_CNT2_STRUC	 StaTx2;
	TX_AGG_CNT_STRUC	TxAggCnt;
	TX_AGG_CNT0_STRUC	TxAggCnt0;
	TX_AGG_CNT1_STRUC	TxAggCnt1;
	TX_AGG_CNT2_STRUC	TxAggCnt2;
	TX_AGG_CNT3_STRUC	TxAggCnt3;
	TX_AGG_CNT4_STRUC	TxAggCnt4;
	TX_AGG_CNT5_STRUC	TxAggCnt5;
	TX_AGG_CNT6_STRUC	TxAggCnt6;
	TX_AGG_CNT7_STRUC	TxAggCnt7;

	RTMP_IO_READ32(pAd, RX_STA_CNT0, &RxStaCnt0.word);
	RTMP_IO_READ32(pAd, RX_STA_CNT2, &RxStaCnt2.word);

	{
		RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt1.word);
	    
	    pAd->PrivateInfo.PhyRxErrCnt += RxStaCnt1.field.PlcpErr;
		
		pAd->RalinkCounters.OneSecFalseCCACnt += RxStaCnt1.field.FalseCca;
	}

	
	OldValue= pAd->WlanCounters.FCSErrorCount.u.LowPart;
	pAd->WlanCounters.FCSErrorCount.u.LowPart += (RxStaCnt0.field.CrcErr); 
	if (pAd->WlanCounters.FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters.FCSErrorCount.u.HighPart++;

	
	pAd->RalinkCounters.OneSecRxFcsErrCnt += RxStaCnt0.field.CrcErr;
	OldValue = pAd->RalinkCounters.RealFcsErrCount.u.LowPart;
	pAd->RalinkCounters.RealFcsErrCount.u.LowPart += RxStaCnt0.field.CrcErr;
	if (pAd->RalinkCounters.RealFcsErrCount.u.LowPart < OldValue)
		pAd->RalinkCounters.RealFcsErrCount.u.HighPart++;

	
	pAd->RalinkCounters.DuplicateRcv += RxStaCnt2.field.RxDupliCount;
	pAd->WlanCounters.FrameDuplicateCount.u.LowPart += RxStaCnt2.field.RxDupliCount;
	
	pAd->Counters8023.RxNoBuffer += (RxStaCnt2.field.RxFifoOverflowCount);

#ifdef RT2870
	if (pAd->RalinkCounters.RxCount != pAd->watchDogRxCnt)
	{
		pAd->watchDogRxCnt = pAd->RalinkCounters.RxCount;
		pAd->watchDogRxOverFlowCnt = 0;
	}
	else
	{
		if (RxStaCnt2.field.RxFifoOverflowCount)
			pAd->watchDogRxOverFlowCnt++;
		else
			pAd->watchDogRxOverFlowCnt = 0;
	}
#endif 


	if (!pAd->bUpdateBcnCntDone)
	{
	
	RTMP_IO_READ32(pAd, TX_STA_CNT0, &TxStaCnt0.word);
	RTMP_IO_READ32(pAd, TX_STA_CNT1, &StaTx1.word);
	RTMP_IO_READ32(pAd, TX_STA_CNT2, &StaTx2.word);
	pAd->RalinkCounters.OneSecBeaconSentCnt += TxStaCnt0.field.TxBeaconCount;
	pAd->RalinkCounters.OneSecTxRetryOkCount += StaTx1.field.TxRetransmit;
	pAd->RalinkCounters.OneSecTxNoRetryOkCount += StaTx1.field.TxSuccess;
	pAd->RalinkCounters.OneSecTxFailCount += TxStaCnt0.field.TxFailCount;
	pAd->WlanCounters.TransmittedFragmentCount.u.LowPart += StaTx1.field.TxSuccess;
	pAd->WlanCounters.RetryCount.u.LowPart += StaTx1.field.TxRetransmit;
	pAd->WlanCounters.FailedCount.u.LowPart += TxStaCnt0.field.TxFailCount;
	}

	{
		RTMP_IO_READ32(pAd, TX_AGG_CNT, &TxAggCnt.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT0, &TxAggCnt0.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT1, &TxAggCnt1.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT2, &TxAggCnt2.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT3, &TxAggCnt3.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT4, &TxAggCnt4.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT5, &TxAggCnt5.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT6, &TxAggCnt6.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT7, &TxAggCnt7.word);
		pAd->RalinkCounters.TxAggCount += TxAggCnt.field.AggTxCount;
		pAd->RalinkCounters.TxNonAggCount += TxAggCnt.field.NonAggTxCount;
		pAd->RalinkCounters.TxAgg1MPDUCount += TxAggCnt0.field.AggSize1Count;
		pAd->RalinkCounters.TxAgg2MPDUCount += TxAggCnt0.field.AggSize2Count;

		pAd->RalinkCounters.TxAgg3MPDUCount += TxAggCnt1.field.AggSize3Count;
		pAd->RalinkCounters.TxAgg4MPDUCount += TxAggCnt1.field.AggSize4Count;
		pAd->RalinkCounters.TxAgg5MPDUCount += TxAggCnt2.field.AggSize5Count;
		pAd->RalinkCounters.TxAgg6MPDUCount += TxAggCnt2.field.AggSize6Count;

		pAd->RalinkCounters.TxAgg7MPDUCount += TxAggCnt3.field.AggSize7Count;
		pAd->RalinkCounters.TxAgg8MPDUCount += TxAggCnt3.field.AggSize8Count;
		pAd->RalinkCounters.TxAgg9MPDUCount += TxAggCnt4.field.AggSize9Count;
		pAd->RalinkCounters.TxAgg10MPDUCount += TxAggCnt4.field.AggSize10Count;

		pAd->RalinkCounters.TxAgg11MPDUCount += TxAggCnt5.field.AggSize11Count;
		pAd->RalinkCounters.TxAgg12MPDUCount += TxAggCnt5.field.AggSize12Count;
		pAd->RalinkCounters.TxAgg13MPDUCount += TxAggCnt6.field.AggSize13Count;
		pAd->RalinkCounters.TxAgg14MPDUCount += TxAggCnt6.field.AggSize14Count;

		pAd->RalinkCounters.TxAgg15MPDUCount += TxAggCnt7.field.AggSize15Count;
		pAd->RalinkCounters.TxAgg16MPDUCount += TxAggCnt7.field.AggSize16Count;

		
		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += TxAggCnt0.field.AggSize1Count;
		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt0.field.AggSize2Count / 2);

		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt1.field.AggSize3Count / 3);
		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt1.field.AggSize4Count / 4);

		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt2.field.AggSize5Count / 5);
		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt2.field.AggSize6Count / 6);

		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt3.field.AggSize7Count / 7);
		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt3.field.AggSize8Count / 8);

		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt4.field.AggSize9Count / 9);
		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt4.field.AggSize10Count / 10);

		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt5.field.AggSize11Count / 11);
		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt5.field.AggSize12Count / 12);

		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt6.field.AggSize13Count / 13);
		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt6.field.AggSize14Count / 14);

		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt7.field.AggSize15Count / 15);
		pAd->RalinkCounters.TransmittedAMPDUCount.u.LowPart += (TxAggCnt7.field.AggSize16Count / 16);
	}



}



VOID	NICResetFromError(
	IN	PRTMP_ADAPTER	pAd)
{
	
	
	

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x1);
	
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);

	NICInitializeAdapter(pAd, FALSE);
	NICInitAsicFromEEPROM(pAd);

	
	AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
	AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
}


VOID NICEraseFirmware(
	IN PRTMP_ADAPTER pAd)
{
	ULONG i;

	for(i=0; i<MAX_FIRMWARE_IMAGE_SIZE; i+=4)
		RTMP_IO_WRITE32(pAd, FIRMWARE_IMAGE_BASE + i, 0);

}


NDIS_STATUS NICLoadFirmware(
	IN PRTMP_ADAPTER pAd)
{
	NDIS_STATUS		Status = NDIS_STATUS_SUCCESS;
	PUCHAR			pFirmwareImage;
	ULONG			FileLength, Index;
	
	UINT32			MacReg = 0;
#ifdef RT2870
	UINT32			Version = (pAd->MACVersion >> 16);
#endif 

	pFirmwareImage = FirmwareImage;
	FileLength = sizeof(FirmwareImage);
#ifdef RT2870
	
	
	if (FIRMWAREIMAGE_LENGTH == FIRMWAREIMAGE_MAX_LENGTH)
	
	
	{
		if ((Version != 0x2860) && (Version != 0x2872) && (Version != 0x3070))
		{	
			
			pFirmwareImage = (PUCHAR)&FirmwareImage[FIRMWAREIMAGEV1_LENGTH];
			FileLength = FIRMWAREIMAGEV2_LENGTH;
		}
		else
		{
			
			pFirmwareImage = FirmwareImage;
			FileLength = FIRMWAREIMAGEV1_LENGTH;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("KH: bin file should be 8KB.\n"));
		Status = NDIS_STATUS_FAILURE;
	}

#endif 

	RT28XX_WRITE_FIRMWARE(pAd, pFirmwareImage, FileLength);

	
	Index = 0;
	do
	{
		RTMP_IO_READ32(pAd, PBF_SYS_CTRL, &MacReg);

		if (MacReg & 0x80)
			break;

		RTMPusecDelay(1000);
	} while (Index++ < 1000);

    if (Index > 1000)
	{
		Status = NDIS_STATUS_FAILURE;
		DBGPRINT(RT_DEBUG_ERROR, ("NICLoadFirmware: MCU is not ready\n\n\n"));
	} 

    DBGPRINT(RT_DEBUG_TRACE,
			 ("<=== %s (status=%d)\n", __func__, Status));
    return Status;
} 



NDIS_STATUS NICLoadRateSwitchingParams(
	IN PRTMP_ADAPTER pAd)
{
	return NDIS_STATUS_SUCCESS;
}


ULONG	RTMPNotAllZero(
	IN	PVOID	pSrc1,
	IN	ULONG	Length)
{
	PUCHAR	pMem1;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] != 0x0)
		{
			break;
		}
	}

	if (Index == Length)
	{
		return (0);
	}
	else
	{
		return (1);
	}
}


ULONG	RTMPCompareMemory(
	IN	PVOID	pSrc1,
	IN	PVOID	pSrc2,
	IN	ULONG	Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] > pMem2[Index])
			return (1);
		else if (pMem1[Index] < pMem2[Index])
			return (2);
	}

	
	return (0);
}


VOID	RTMPZeroMemory(
	IN	PVOID	pSrc,
	IN	ULONG	Length)
{
	PUCHAR	pMem;
	ULONG	Index = 0;

	pMem = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem[Index] = 0x00;
	}
}

VOID	RTMPFillMemory(
	IN	PVOID	pSrc,
	IN	ULONG	Length,
	IN	UCHAR	Fill)
{
	PUCHAR	pMem;
	ULONG	Index = 0;

	pMem = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem[Index] = Fill;
	}
}


VOID	RTMPMoveMemory(
	OUT	PVOID	pDest,
	IN	PVOID	pSrc,
	IN	ULONG	Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	UINT	Index;

	ASSERT((Length==0) || (pDest && pSrc));

	pMem1 = (PUCHAR) pDest;
	pMem2 = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem1[Index] = pMem2[Index];
	}
}


VOID	UserCfgInit(
	IN	PRTMP_ADAPTER pAd)
{
    UINT key_index, bss_index;

	DBGPRINT(RT_DEBUG_TRACE, ("--> UserCfgInit\n"));

	
	
	
#ifdef RT2870
	pAd->BulkOutReq = 0;

	pAd->BulkOutComplete = 0;
	pAd->BulkOutCompleteOther = 0;
	pAd->BulkOutCompleteCancel = 0;
	pAd->BulkInReq = 0;
	pAd->BulkInComplete = 0;
	pAd->BulkInCompleteFail = 0;

	
	
	pAd->bUsbTxBulkAggre = 0;

	
	pAd->LedIndicatorStregth = 0xFF;

	pAd->CommonCfg.MaxPktOneTxBulk = 2;
	pAd->CommonCfg.TxBulkFactor = 1;
	pAd->CommonCfg.RxBulkFactor =1;

	pAd->CommonCfg.TxPower = 100; 

	NdisZeroMemory(&pAd->CommonCfg.IOTestParm, sizeof(pAd->CommonCfg.IOTestParm));
#endif 

	for(key_index=0; key_index<SHARE_KEY_NUM; key_index++)
	{
		for(bss_index = 0; bss_index < MAX_MBSSID_NUM; bss_index++)
		{
			pAd->SharedKey[bss_index][key_index].KeyLen = 0;
			pAd->SharedKey[bss_index][key_index].CipherAlg = CIPHER_NONE;
		}
	}

#ifdef RT2870
	pAd->EepromAccess = FALSE;
#endif
	pAd->Antenna.word = 0;
	pAd->CommonCfg.BBPCurrentBW = BW_20;

	pAd->LedCntl.word = 0;
#ifdef RT2860
	pAd->LedIndicatorStregth = 0;
	pAd->RLnkCtrlOffset = 0;
	pAd->HostLnkCtrlOffset = 0;
	pAd->CheckDmaBusyCount = 0;
#endif

	pAd->bAutoTxAgcA = FALSE;			
	pAd->bAutoTxAgcG = FALSE;			
	pAd->RfIcType = RFIC_2820;

	
	pAd->CommonCfg.CentralChannel = 1;
	pAd->bForcePrintTX = FALSE;
	pAd->bForcePrintRX = FALSE;
	pAd->bStaFifoTest = FALSE;
	pAd->bProtectionTest = FALSE;
	pAd->bHCCATest = FALSE;
	pAd->bGenOneHCCA = FALSE;
	pAd->CommonCfg.Dsifs = 10;      
	pAd->CommonCfg.TxPower = 100; 
	pAd->CommonCfg.TxPowerPercentage = 0xffffffff; 
	pAd->CommonCfg.TxPowerDefault = 0xffffffff; 
	pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto; 
	pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
	pAd->CommonCfg.RtsThreshold = 2347;
	pAd->CommonCfg.FragmentThreshold = 2346;
	pAd->CommonCfg.UseBGProtection = 0;    
	pAd->CommonCfg.bEnableTxBurst = TRUE; 
	pAd->CommonCfg.PhyMode = 0xff;     
	pAd->CommonCfg.BandState = UNKNOWN_BAND;
	pAd->CommonCfg.RadarDetect.CSPeriod = 10;
	pAd->CommonCfg.RadarDetect.CSCount = 0;
	pAd->CommonCfg.RadarDetect.RDMode = RD_NORMAL_MODE;
	pAd->CommonCfg.RadarDetect.ChMovingTime = 65;
	pAd->CommonCfg.RadarDetect.LongPulseRadarTh = 3;
	pAd->CommonCfg.bAPSDCapable = FALSE;
	pAd->CommonCfg.bNeedSendTriggerFrame = FALSE;
	pAd->CommonCfg.TriggerTimerCount = 0;
	pAd->CommonCfg.bAPSDForcePowerSave = FALSE;
	pAd->CommonCfg.bCountryFlag = FALSE;
	pAd->CommonCfg.TxStream = 0;
	pAd->CommonCfg.RxStream = 0;

	NdisZeroMemory(&pAd->BeaconTxWI, sizeof(pAd->BeaconTxWI));

	NdisZeroMemory(&pAd->CommonCfg.HtCapability, sizeof(pAd->CommonCfg.HtCapability));
	pAd->HTCEnable = FALSE;
	pAd->bBroadComHT = FALSE;
	pAd->CommonCfg.bRdg = FALSE;

	NdisZeroMemory(&pAd->CommonCfg.AddHTInfo, sizeof(pAd->CommonCfg.AddHTInfo));
	pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_ENABLE;
	pAd->CommonCfg.BACapability.field.MpduDensity = 0;
	pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
	pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64; 
	pAd->CommonCfg.BACapability.field.TxBAWinLimit = 64; 
	DBGPRINT(RT_DEBUG_TRACE, ("--> UserCfgInit. BACapability = 0x%x\n", pAd->CommonCfg.BACapability.word));

	pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
	BATableInit(pAd, &pAd->BATable);

	pAd->CommonCfg.bExtChannelSwitchAnnouncement = 1;
	pAd->CommonCfg.bHTProtect = 1;
	pAd->CommonCfg.bMIMOPSEnable = TRUE;
	pAd->CommonCfg.bBADecline = FALSE;
	pAd->CommonCfg.bDisableReordering = FALSE;

	pAd->CommonCfg.TxBASize = 7;

	pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;

	
	
	
	
	pAd->CommonCfg.TxRate = RATE_6;

	pAd->CommonCfg.MlmeTransmit.field.MCS = MCS_RATE_6;
	pAd->CommonCfg.MlmeTransmit.field.BW = BW_20;
	pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;

	pAd->CommonCfg.BeaconPeriod = 100;     

	
	
	
	{
		RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_DIRECT);
		RX_FILTER_CLEAR_FLAG(pAd, fRX_FILTER_ACCEPT_MULTICAST);
		RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_BROADCAST);
		RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_ALL_MULTICAST);

		pAd->StaCfg.Psm = PWR_ACTIVE;

		pAd->StaCfg.OrigWepStatus = Ndis802_11EncryptionDisabled;
		pAd->StaCfg.PairCipher = Ndis802_11EncryptionDisabled;
		pAd->StaCfg.GroupCipher = Ndis802_11EncryptionDisabled;
		pAd->StaCfg.bMixCipher = FALSE;
		pAd->StaCfg.DefaultKeyId = 0;

		
		pAd->StaCfg.PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pAd->StaCfg.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		pAd->StaCfg.LastMicErrorTime = 0;
		pAd->StaCfg.MicErrCnt        = 0;
		pAd->StaCfg.bBlockAssoc      = FALSE;
		pAd->StaCfg.WpaState         = SS_NOTUSE;

		pAd->CommonCfg.NdisRadioStateOff = FALSE;		

		pAd->StaCfg.RssiTrigger = 0;
		NdisZeroMemory(&pAd->StaCfg.RssiSample, sizeof(RSSI_SAMPLE));
		pAd->StaCfg.RssiTriggerMode = RSSI_TRIGGERED_UPON_BELOW_THRESHOLD;
		pAd->StaCfg.AtimWin = 0;
		pAd->StaCfg.DefaultListenCount = 3;
		pAd->StaCfg.BssType = BSS_INFRA;  
		pAd->StaCfg.bScanReqIsFromWebUI = FALSE;
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);

		pAd->StaCfg.bAutoTxRateSwitch = TRUE;
		pAd->StaCfg.DesiredTransmitSetting.field.MCS = MCS_AUTO;
	}

	
	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_INFRA_ON);

	
	pAd->CommonCfg.PhyMode = PHY_11BG_MIXED;		
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);  

	{
		
		pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeCAM;
		pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
		pAd->StaCfg.bWindowsACCAMEnable = FALSE;

		RTMPInitTimer(pAd, &pAd->StaCfg.StaQuickResponeForRateUpTimer, GET_TIMER_FUNCTION(StaQuickResponeForRateUpExec), pAd, FALSE);
		pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = FALSE;

		
		pAd->StaCfg.ScanCnt = 0;

		
		pAd->StaCfg.CCXEnable = FALSE;
		pAd->StaCfg.CCXReqType = MSRN_TYPE_UNUSED;
		pAd->StaCfg.CCXQosECWMin	= 4;
		pAd->StaCfg.CCXQosECWMax	= 10;

		pAd->StaCfg.bHwRadio  = TRUE; 
		pAd->StaCfg.bSwRadio  = TRUE; 
		pAd->StaCfg.bRadio    = TRUE; 
		pAd->StaCfg.bHardwareRadio = FALSE;		
		pAd->StaCfg.bShowHiddenSSID = FALSE;		

		
		pAd->StaCfg.bAutoReconnect = TRUE;

		
		
		pAd->StaCfg.LastScanTime = 0;
		NdisZeroMemory(pAd->nickname, IW_ESSID_MAX_SIZE+1);
		sprintf(pAd->nickname, "%s", STA_NIC_DEVICE_NAME);
		RTMPInitTimer(pAd, &pAd->StaCfg.WpaDisassocAndBlockAssocTimer, GET_TIMER_FUNCTION(WpaDisassocApAndBlockAssoc), pAd, FALSE);
		pAd->StaCfg.IEEE8021X = FALSE;
		pAd->StaCfg.IEEE8021x_required_keys = FALSE;
		pAd->StaCfg.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
		pAd->StaCfg.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;
	}

	
	pAd->ExtraInfo = EXTRA_INFO_CLEAR;

	
	pAd->bConfigChanged = FALSE;

	
	
	


	
	
	
	
	pAd->BbpTuning.bEnable                = TRUE;
	pAd->BbpTuning.FalseCcaLowerThreshold = 100;
	pAd->BbpTuning.FalseCcaUpperThreshold = 512;
	pAd->BbpTuning.R66Delta               = 4;
	pAd->Mlme.bEnableAutoAntennaCheck = TRUE;

	
	
	
	
	pAd->BbpTuning.R66CurrentValue = 0x38;

	pAd->Bbp94 = BBPR94_DEFAULT;
	pAd->BbpForCCK = FALSE;

	
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));
	InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
	NdisAllocateSpinLock(&pAd->MacTabLock);

	pAd->CommonCfg.bWiFiTest = FALSE;
#ifdef RT2860
	pAd->bPCIclkOff = FALSE;

	RTMP_SET_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
#endif
	DBGPRINT(RT_DEBUG_TRACE, ("<-- UserCfgInit\n"));
}


UCHAR BtoH(char ch)
{
	if (ch >= '0' && ch <= '9') return (ch - '0');        
	if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  
	if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  
	return(255);
}


















void AtoH(char * src, UCHAR * dest, int destlen)
{
	char * srcptr;
	PUCHAR destTemp;

	srcptr = src;
	destTemp = (PUCHAR) dest;

	while(destlen--)
	{
		*destTemp = BtoH(*srcptr++) << 4;    
		*destTemp += BtoH(*srcptr++);      
		destTemp++;
	}
}

VOID	RTMPPatchMacBbpBug(
	IN	PRTMP_ADAPTER	pAd)
{
	ULONG	Index;

	
	for (Index = 0; Index < NUM_BBP_REG_PARMS; Index++)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBPRegTable[Index].Register, (UCHAR)BBPRegTable[Index].Value);
	}

	
	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	AsicLockChannel(pAd, pAd->CommonCfg.Channel);

	
	NICInitAsicFromEEPROM(pAd);
}


VOID	RTMPInitTimer(
	IN	PRTMP_ADAPTER			pAd,
	IN	PRALINK_TIMER_STRUCT	pTimer,
	IN	PVOID					pTimerFunc,
	IN	PVOID					pData,
	IN	BOOLEAN					Repeat)
{
	
	
	
	
	
	pTimer->Valid      = TRUE;

	pTimer->PeriodicType = Repeat;
	pTimer->State      = FALSE;
	pTimer->cookie = (ULONG) pData;

#ifdef RT2870
	pTimer->pAd = pAd;
#endif 

	RTMP_OS_Init_Timer(pAd,	&pTimer->TimerObj,	pTimerFunc, (PVOID) pTimer);
}


VOID	RTMPSetTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	IN	ULONG					Value)
{
	if (pTimer->Valid)
	{
		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;
		if (pTimer->PeriodicType == TRUE)
		{
			pTimer->Repeat = TRUE;
			RTMP_SetPeriodicTimer(&pTimer->TimerObj, Value);
		}
		else
		{
			pTimer->Repeat = FALSE;
			RTMP_OS_Add_Timer(&pTimer->TimerObj, Value);
		}
	}
	else
	{
		DBGPRINT_ERR(("RTMPSetTimer failed, Timer hasn't been initialize!\n"));
	}
}



VOID	RTMPModTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	IN	ULONG					Value)
{
	BOOLEAN	Cancel;

	if (pTimer->Valid)
	{
		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;
		if (pTimer->PeriodicType == TRUE)
		{
			RTMPCancelTimer(pTimer, &Cancel);
			RTMPSetTimer(pTimer, Value);
		}
		else
		{
			RTMP_OS_Mod_Timer(&pTimer->TimerObj, Value);
		}
	}
	else
	{
		DBGPRINT_ERR(("RTMPModTimer failed, Timer hasn't been initialize!\n"));
	}
}


VOID	RTMPCancelTimer(
	IN	PRALINK_TIMER_STRUCT	pTimer,
	OUT	BOOLEAN					*pCancelled)
{
	if (pTimer->Valid)
	{
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;
			RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RT2870
		
		

		RT2870_TimerQ_Remove(pTimer->pAd, pTimer);
#endif 
	}
	else
	{
		
		
		
		
		DBGPRINT_ERR(("RTMPCancelTimer failed, Timer hasn't been initialize!\n"));
	}
}


VOID RTMPSetLED(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			Status)
{
	
	UCHAR			HighByte = 0;
	UCHAR			LowByte;

	LowByte = pAd->LedCntl.field.LedMode&0x7f;
	switch (Status)
	{
		case LED_LINK_DOWN:
			HighByte = 0x20;
			AsicSendCommandToMcu(pAd, 0x50, 0xff, LowByte, HighByte);
			pAd->LedIndicatorStregth = 0;
			break;
		case LED_LINK_UP:
			if (pAd->CommonCfg.Channel > 14)
				HighByte = 0xa0;
			else
				HighByte = 0x60;
			AsicSendCommandToMcu(pAd, 0x50, 0xff, LowByte, HighByte);
			break;
		case LED_RADIO_ON:
			HighByte = 0x20;
			AsicSendCommandToMcu(pAd, 0x50, 0xff, LowByte, HighByte);
			break;
		case LED_HALT:
			LowByte = 0; 
		case LED_RADIO_OFF:
			HighByte = 0;
			AsicSendCommandToMcu(pAd, 0x50, 0xff, LowByte, HighByte);
			break;
        case LED_WPS:
			HighByte = 0x10;
			AsicSendCommandToMcu(pAd, 0x50, 0xff, LowByte, HighByte);
			break;
		case LED_ON_SITE_SURVEY:
			HighByte = 0x08;
			AsicSendCommandToMcu(pAd, 0x50, 0xff, LowByte, HighByte);
			break;
		case LED_POWER_UP:
			HighByte = 0x04;
			AsicSendCommandToMcu(pAd, 0x50, 0xff, LowByte, HighByte);
			break;
		default:
			DBGPRINT(RT_DEBUG_WARN, ("RTMPSetLED::Unknown Status %d\n", Status));
			break;
	}

    
	
	
	
	if ((Status != LED_ON_SITE_SURVEY) && (Status != LED_POWER_UP))
		pAd->LedStatus = Status;

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetLED::Mode=%d,HighByte=0x%02x,LowByte=0x%02x\n", pAd->LedCntl.field.LedMode, HighByte, LowByte));
}


VOID RTMPSetSignalLED(
	IN PRTMP_ADAPTER 	pAd,
	IN NDIS_802_11_RSSI Dbm)
{
	UCHAR		nLed = 0;

	
	
	
	if (pAd->LedCntl.field.LedMode != LED_MODE_SIGNAL_STREGTH)
	{
		return;
	}

	if (Dbm <= -90)
		nLed = 0;
	else if (Dbm <= -81)
		nLed = 1;
	else if (Dbm <= -71)
		nLed = 3;
	else if (Dbm <= -67)
		nLed = 7;
	else if (Dbm <= -57)
		nLed = 15;
	else
		nLed = 31;

	
	
	
	if (pAd->LedIndicatorStregth != nLed)
	{
		AsicSendCommandToMcu(pAd, 0x51, 0xff, nLed, pAd->LedCntl.field.Polarity);
		pAd->LedIndicatorStregth = nLed;
	}
}


VOID RTMPEnableRxTx(
	IN PRTMP_ADAPTER	pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPEnableRxTx\n"));

	
	RT28XXDMAEnable(pAd);

	
	if (pAd->OpMode == OPMODE_AP)
	{
		UINT32 rx_filter_flag = APNORMAL;


		RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, rx_filter_flag);     
	}
	else
	{
		RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, STANORMAL);     
	}

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xc);
	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPEnableRxTx\n"));
}


