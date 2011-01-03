

#include "r8192E.h"
#include "r8192E_hw.h"
#include "r819xE_phyreg.h"
#include "r819xE_phy.h"
#include "r8190_rtl8256.h"


void PHY_SetRF8256Bandwidth(struct net_device* dev , HT_CHANNEL_WIDTH Bandwidth)	
{
	u8	eRFPath;
	struct r8192_priv *priv = ieee80211_priv(dev);

	
	for(eRFPath = 0; eRFPath <priv->NumTotalRFPath; eRFPath++)
	{
		if (!rtl8192_phy_CheckIsLegalRFPath(dev, eRFPath))
				continue;

		switch(Bandwidth)
		{
			case HT_CHANNEL_WIDTH_20:
				if(priv->card_8192_version == VERSION_8190_BD || priv->card_8192_version == VERSION_8190_BE)
				{
					rtl8192_phy_SetRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, 0x0b, bMask12Bits, 0x100); 
					rtl8192_phy_SetRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, 0x2c, bMask12Bits, 0x3d7);
					rtl8192_phy_SetRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, 0x0e, bMask12Bits, 0x021);

					
					
				}
				else
				{
					RT_TRACE(COMP_ERR, "PHY_SetRF8256Bandwidth(): unknown hardware version\n");
				}

				break;
			case HT_CHANNEL_WIDTH_20_40:
				if(priv->card_8192_version == VERSION_8190_BD ||priv->card_8192_version == VERSION_8190_BE)
				{
					rtl8192_phy_SetRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, 0x0b, bMask12Bits, 0x300); 
					rtl8192_phy_SetRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, 0x2c, bMask12Bits, 0x3ff);
					rtl8192_phy_SetRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, 0x0e, bMask12Bits, 0x0e1);

					
					#if 0
					if(priv->chan == 3 || priv->chan == 9) 
						rtl8192_phy_SetRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, 0x14, bMask12Bits, 0x59b);
					else
						rtl8192_phy_SetRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, 0x14, bMask12Bits, 0x5ab);
					#endif
				}
				else
				{
					RT_TRACE(COMP_ERR, "PHY_SetRF8256Bandwidth(): unknown hardware version\n");
				}


				break;
			default:
				RT_TRACE(COMP_ERR, "PHY_SetRF8256Bandwidth(): unknown Bandwidth: %#X\n",Bandwidth );
				break;

		}
	}
	return;
}

RT_STATUS PHY_RF8256_Config(struct net_device* dev)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	
	
	RT_STATUS rtStatus = RT_STATUS_SUCCESS;
	
	priv->NumTotalRFPath = RTL819X_TOTAL_RF_PATH;
	
	rtStatus = phy_RF8256_Config_ParaFile(dev);

	return rtStatus;
}

RT_STATUS phy_RF8256_Config_ParaFile(struct net_device* dev)
{
	u32 	u4RegValue = 0;
	u8 	eRFPath;
	RT_STATUS				rtStatus = RT_STATUS_SUCCESS;
	BB_REGISTER_DEFINITION_T	*pPhyReg;
	struct r8192_priv *priv = ieee80211_priv(dev);
	u32	RegOffSetToBeCheck = 0x3;
	u32 	RegValueToBeCheck = 0x7f1;
	u32	RF3_Final_Value = 0;
	u8	ConstRetryTimes = 5, RetryTimes = 5;
	u8 ret = 0;
	
	
	
	for(eRFPath = (RF90_RADIO_PATH_E)RF90_PATH_A; eRFPath <priv->NumTotalRFPath; eRFPath++)
	{
		if (!rtl8192_phy_CheckIsLegalRFPath(dev, eRFPath))
				continue;

		pPhyReg = &priv->PHYRegDef[eRFPath];

		
	

		
		switch(eRFPath)
		{
		case RF90_PATH_A:
		case RF90_PATH_C:
			u4RegValue = rtl8192_QueryBBReg(dev, pPhyReg->rfintfs, bRFSI_RFENV);
			break;
		case RF90_PATH_B :
		case RF90_PATH_D:
			u4RegValue = rtl8192_QueryBBReg(dev, pPhyReg->rfintfs, bRFSI_RFENV<<16);
			break;
		}

		
		rtl8192_setBBreg(dev, pPhyReg->rfintfe, bRFSI_RFENV<<16, 0x1);

		
		rtl8192_setBBreg(dev, pPhyReg->rfintfo, bRFSI_RFENV, 0x1);

		
		rtl8192_setBBreg(dev, pPhyReg->rfHSSIPara2, b3WireAddressLength, 0x0); 	
		rtl8192_setBBreg(dev, pPhyReg->rfHSSIPara2, b3WireDataLength, 0x0);	

		rtl8192_phy_SetRFReg(dev, (RF90_RADIO_PATH_E) eRFPath, 0x0, bMask12Bits, 0xbf);

		
		
		rtStatus = rtl8192_phy_checkBBAndRF(dev, HW90_BLOCK_RF, (RF90_RADIO_PATH_E)eRFPath);
		if(rtStatus!= RT_STATUS_SUCCESS)
		{
			RT_TRACE(COMP_ERR, "PHY_RF8256_Config():Check Radio[%d] Fail!!\n", eRFPath);
			goto phy_RF8256_Config_ParaFile_Fail;
		}

		RetryTimes = ConstRetryTimes;
		RF3_Final_Value = 0;
		
		switch(eRFPath)
		{
		case RF90_PATH_A:
			while(RF3_Final_Value!=RegValueToBeCheck && RetryTimes!=0)
			{
				ret = rtl8192_phy_ConfigRFWithHeaderFile(dev,(RF90_RADIO_PATH_E)eRFPath);
				RF3_Final_Value = rtl8192_phy_QueryRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, RegOffSetToBeCheck, bMask12Bits);
				RT_TRACE(COMP_RF, "RF %d %d register final value: %x\n", eRFPath, RegOffSetToBeCheck, RF3_Final_Value);
				RetryTimes--;
			}
			break;
		case RF90_PATH_B:
			while(RF3_Final_Value!=RegValueToBeCheck && RetryTimes!=0)
			{
				ret = rtl8192_phy_ConfigRFWithHeaderFile(dev,(RF90_RADIO_PATH_E)eRFPath);
				RF3_Final_Value = rtl8192_phy_QueryRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, RegOffSetToBeCheck, bMask12Bits);
				RT_TRACE(COMP_RF, "RF %d %d register final value: %x\n", eRFPath, RegOffSetToBeCheck, RF3_Final_Value);
				RetryTimes--;
			}
			break;
		case RF90_PATH_C:
			while(RF3_Final_Value!=RegValueToBeCheck && RetryTimes!=0)
			{
				ret = rtl8192_phy_ConfigRFWithHeaderFile(dev,(RF90_RADIO_PATH_E)eRFPath);
				RF3_Final_Value = rtl8192_phy_QueryRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, RegOffSetToBeCheck, bMask12Bits);
				RT_TRACE(COMP_RF, "RF %d %d register final value: %x\n", eRFPath, RegOffSetToBeCheck, RF3_Final_Value);
				RetryTimes--;
			}
			break;
		case RF90_PATH_D:
			while(RF3_Final_Value!=RegValueToBeCheck && RetryTimes!=0)
			{
				ret = rtl8192_phy_ConfigRFWithHeaderFile(dev,(RF90_RADIO_PATH_E)eRFPath);
				RF3_Final_Value = rtl8192_phy_QueryRFReg(dev, (RF90_RADIO_PATH_E)eRFPath, RegOffSetToBeCheck, bMask12Bits);
				RT_TRACE(COMP_RF, "RF %d %d register final value: %x\n", eRFPath, RegOffSetToBeCheck, RF3_Final_Value);
				RetryTimes--;
			}
			break;
		}

		;
		switch(eRFPath)
		{
		case RF90_PATH_A:
		case RF90_PATH_C:
			rtl8192_setBBreg(dev, pPhyReg->rfintfs, bRFSI_RFENV, u4RegValue);
			break;
		case RF90_PATH_B :
		case RF90_PATH_D:
			rtl8192_setBBreg(dev, pPhyReg->rfintfs, bRFSI_RFENV<<16, u4RegValue);
			break;
		}

		if(ret){
			RT_TRACE(COMP_ERR, "phy_RF8256_Config_ParaFile():Radio[%d] Fail!!", eRFPath);
			goto phy_RF8256_Config_ParaFile_Fail;
		}

	}

	RT_TRACE(COMP_PHY, "PHY Initialization Success\n") ;
	return RT_STATUS_SUCCESS;

phy_RF8256_Config_ParaFile_Fail:
	RT_TRACE(COMP_ERR, "PHY Initialization failed\n") ;
	return RT_STATUS_FAILURE;
}


void PHY_SetRF8256CCKTxPower(struct net_device*	dev, u8	powerlevel)
{
	u32	TxAGC=0;
	struct r8192_priv *priv = ieee80211_priv(dev);
#ifdef RTL8190P
	u8				byte0, byte1;

	TxAGC |= ((powerlevel<<8)|powerlevel);
	TxAGC += priv->CCKTxPowerLevelOriginalOffset;

	if(priv->bDynamicTxLowPower == true  
		 ) 
	{
		if(priv->CustomerID == RT_CID_819x_Netcore)
			TxAGC = 0x2222;
		else
		TxAGC += ((priv->CckPwEnl<<8)|priv->CckPwEnl);
	}

	byte0 = (u8)(TxAGC & 0xff);
	byte1 = (u8)((TxAGC & 0xff00)>>8);
	if(byte0 > 0x24)
		byte0 = 0x24;
	if(byte1 > 0x24)
		byte1 = 0x24;
	if(priv->rf_type == RF_2T4R)	
	{	
			if(priv->RF_C_TxPwDiff > 0)
			{
				if( (byte0 + (u8)priv->RF_C_TxPwDiff) > 0x24)
					byte0 = 0x24 - priv->RF_C_TxPwDiff;
				if( (byte1 + (u8)priv->RF_C_TxPwDiff) > 0x24)
					byte1 = 0x24 - priv->RF_C_TxPwDiff;
			}
		}
	TxAGC = (byte1<<8) |byte0;
	write_nic_dword(dev, CCK_TXAGC, TxAGC);
#else
	#ifdef RTL8192E

	TxAGC = powerlevel;
	if(priv->bDynamicTxLowPower == true)
	{
		if(priv->CustomerID == RT_CID_819x_Netcore)
		TxAGC = 0x22;
	else
		TxAGC += priv->CckPwEnl;
	}
	if(TxAGC > 0x24)
		TxAGC = 0x24;
	rtl8192_setBBreg(dev, rTxAGC_CCK_Mcs32, bTxAGCRateCCK, TxAGC);
	#endif
#endif
}


void PHY_SetRF8256OFDMTxPower(struct net_device* dev, u8 powerlevel)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	
#ifdef RTL8190P
	u32				TxAGC1=0, TxAGC2=0, TxAGC2_tmp = 0;
	u8				i, byteVal1[4], byteVal2[4], byteVal3[4];

	if(priv->bDynamicTxHighPower == true)     
	{
		TxAGC1 |= ((powerlevel<<24)|(powerlevel<<16)|(powerlevel<<8)|powerlevel);
		
		TxAGC2_tmp = TxAGC1;

		TxAGC1 += priv->MCSTxPowerLevelOriginalOffset[0];
		TxAGC2 =0x03030303;

		
		TxAGC2_tmp += priv->MCSTxPowerLevelOriginalOffset[1];
	}
	else
	{
		TxAGC1 |= ((powerlevel<<24)|(powerlevel<<16)|(powerlevel<<8)|powerlevel);
		TxAGC2 = TxAGC1;

		TxAGC1 += priv->MCSTxPowerLevelOriginalOffset[0];
		TxAGC2 += priv->MCSTxPowerLevelOriginalOffset[1];

		TxAGC2_tmp = TxAGC2;

	}
	for(i=0; i<4; i++)
	{
		byteVal1[i] = (u8)(  (TxAGC1 & (0xff<<(i*8))) >>(i*8) );
		if(byteVal1[i] > 0x24)
			byteVal1[i] = 0x24;
		byteVal2[i] = (u8)(  (TxAGC2 & (0xff<<(i*8))) >>(i*8) );
		if(byteVal2[i] > 0x24)
			byteVal2[i] = 0x24;

		
		byteVal3[i] = (u8)(  (TxAGC2_tmp & (0xff<<(i*8))) >>(i*8) );
		if(byteVal3[i] > 0x24)
			byteVal3[i] = 0x24;
	}

	if(priv->rf_type == RF_2T4R)	
	{	
		if(priv->RF_C_TxPwDiff > 0)
		{
			for(i=0; i<4; i++)
			{
				if( (byteVal1[i] + (u8)priv->RF_C_TxPwDiff) > 0x24)
					byteVal1[i] = 0x24 - priv->RF_C_TxPwDiff;
				if( (byteVal2[i] + (u8)priv->RF_C_TxPwDiff) > 0x24)
					byteVal2[i] = 0x24 - priv->RF_C_TxPwDiff;
				if( (byteVal3[i] + (u8)priv->RF_C_TxPwDiff) > 0x24)
					byteVal3[i] = 0x24 - priv->RF_C_TxPwDiff;
			}
		}
	}

	TxAGC1 = (byteVal1[3]<<24) | (byteVal1[2]<<16) |(byteVal1[1]<<8) |byteVal1[0];
	TxAGC2 = (byteVal2[3]<<24) | (byteVal2[2]<<16) |(byteVal2[1]<<8) |byteVal2[0];

	
	TxAGC2_tmp = (byteVal3[3]<<24) | (byteVal3[2]<<16) |(byteVal3[1]<<8) |byteVal3[0];
	priv->Pwr_Track = TxAGC2_tmp;
	

	
	write_nic_dword(dev, MCS_TXAGC, TxAGC1);
	write_nic_dword(dev, MCS_TXAGC+4, TxAGC2);
#else
#ifdef RTL8192E
	u32 writeVal, powerBase0, powerBase1, writeVal_tmp;
	u8 index = 0;
	u16 RegOffset[6] = {0xe00, 0xe04, 0xe10, 0xe14, 0xe18, 0xe1c};
	u8 byte0, byte1, byte2, byte3;

	powerBase0 = powerlevel + priv->LegacyHTTxPowerDiff;	
	powerBase0 = (powerBase0<<24) | (powerBase0<<16) |(powerBase0<<8) |powerBase0;
	powerBase1 = powerlevel;							
	powerBase1 = (powerBase1<<24) | (powerBase1<<16) |(powerBase1<<8) |powerBase1;

	for(index=0; index<6; index++)
	{
		writeVal = priv->MCSTxPowerLevelOriginalOffset[index] + ((index<2)?powerBase0:powerBase1);
		byte0 = (u8)(writeVal & 0x7f);
		byte1 = (u8)((writeVal & 0x7f00)>>8);
		byte2 = (u8)((writeVal & 0x7f0000)>>16);
		byte3 = (u8)((writeVal & 0x7f000000)>>24);
		if(byte0 > 0x24)	
			byte0 = 0x24;
		if(byte1 > 0x24)
			byte1 = 0x24;
		if(byte2 > 0x24)
			byte2 = 0x24;
		if(byte3 > 0x24)
			byte3 = 0x24;

		if(index == 3)
		{
			writeVal_tmp = (byte3<<24) | (byte2<<16) |(byte1<<8) |byte0;
			priv->Pwr_Track = writeVal_tmp;
		}

		if(priv->bDynamicTxHighPower == true)     
		{
			writeVal = 0x03030303;
		}
		else
		{
			writeVal = (byte3<<24) | (byte2<<16) |(byte1<<8) |byte0;
		}
		rtl8192_setBBreg(dev, RegOffset[index], 0x7f7f7f7f, writeVal);
	}

#endif
#endif
	return;
}

#define MAX_DOZE_WAITING_TIMES_9x 64
static bool
SetRFPowerState8190(
	struct net_device* dev,
	RT_RF_POWER_STATE	eRFPowerState
	)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	PRT_POWER_SAVE_CONTROL	pPSC = (PRT_POWER_SAVE_CONTROL)(&(priv->ieee80211->PowerSaveControl));
	bool bResult = true;
	
	u8	i = 0, QueueID = 0;
	ptx_ring	head=NULL,tail=NULL;

	if(priv->SetRFPowerStateInProgress == true)
		return false;
	RT_TRACE(COMP_POWER, "===========> SetRFPowerState8190()!\n");
	priv->SetRFPowerStateInProgress = true;

	switch(priv->rf_chip)
	{
		case RF_8256:
		switch( eRFPowerState )
		{
			case eRfOn:
				RT_TRACE(COMP_POWER, "SetRFPowerState8190() eRfOn !\n");
						
					
					
				#ifdef RTL8190P
				if(priv->rf_type == RF_2T4R)
				{
					
					rtl8192_setBBreg(dev, rFPGA0_XA_RFInterfaceOE, BIT4, 0x1); 
					
					rtl8192_setBBreg(dev, rFPGA0_XC_RFInterfaceOE, BIT4, 0x1); 
					
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter4, 0xf00, 0xf);
					
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter1, 0x1e0, 0xf); 
					
					rtl8192_setBBreg(dev, rOFDM0_TRxPathEnable, 0xf, 0xf);
					
					rtl8192_setBBreg(dev, rOFDM1_TRxPathEnable, 0xf, 0xf);
					
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter1, 0x1e00, 0xf); 
				}
				else if(priv->rf_type == RF_1T2R)	
				{
					
					rtl8192_setBBreg(dev, rFPGA0_XC_RFInterfaceOE, BIT4, 0x1); 
					
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter4, 0xc00, 0x3);
					
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter1, 0x180, 0x3); 
					
					rtl8192_setBBreg(dev, rOFDM0_TRxPathEnable, 0xc, 0x3);
					
					rtl8192_setBBreg(dev, rOFDM1_TRxPathEnable, 0xc, 0x3);
					
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter1, 0x1800, 0x3); 
				}
				#else
				write_nic_byte(dev, ANAPAR, 0x37);
				write_nic_byte(dev, MacBlkCtrl, 0x17); 
				mdelay(1);
				

				priv->bHwRfOffAction = 0;
				

				
				write_nic_byte(dev, BB_RESET, (read_nic_byte(dev, BB_RESET)|BIT0));

				
				
				rtl8192_setBBreg(dev, rFPGA0_AnalogParameter2, 0x20000000, 0x1); 
				
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter1, 0x60, 0x3);		
				
				rtl8192_setBBreg(dev, rFPGA0_AnalogParameter1, 0x98, 0x13); 
				
				rtl8192_setBBreg(dev, rFPGA0_AnalogParameter4, 0xf03, 0xf03);
				
				
				
				

				
				
					rtl8192_setBBreg(dev, rFPGA0_XA_RFInterfaceOE, BIT4, 0x1);		
				rtl8192_setBBreg(dev, rFPGA0_XB_RFInterfaceOE, BIT4, 0x1);		
				#endif
						break;

				
				
				
				
			case eRfSleep:
			case eRfOff:
				RT_TRACE(COMP_POWER, "SetRFPowerState8190() eRfOff/Sleep !\n");
				if (pPSC->bLeisurePs)
				{
					for(QueueID = 0, i = 0; QueueID < MAX_TX_QUEUE; )
					{
						switch(QueueID) {
							case MGNT_QUEUE:
								tail=priv->txmapringtail;
								head=priv->txmapringhead;
								break;

							case BK_QUEUE:
								tail=priv->txbkpringtail;
								head=priv->txbkpringhead;
								break;

							case BE_QUEUE:
								tail=priv->txbepringtail;
								head=priv->txbepringhead;
								break;

							case VI_QUEUE:
								tail=priv->txvipringtail;
								head=priv->txvipringhead;
								break;

							case VO_QUEUE:
								tail=priv->txvopringtail;
								head=priv->txvopringhead;
								break;

							default:
								tail=head=NULL;
								break;
						}
						if(tail == head)
						{
							
							QueueID++;
							continue;
						}
						else
						{
							RT_TRACE(COMP_POWER, "eRf Off/Sleep: %d times BusyQueue[%d] !=0 before doze!\n", (i+1), QueueID);
							udelay(10);
							i++;
						}

						if(i >= MAX_DOZE_WAITING_TIMES_9x)
						{
							RT_TRACE(COMP_POWER, "\n\n\n TimeOut!! SetRFPowerState8190(): eRfOff: %d times BusyQueue[%d] != 0 !!!\n\n\n", MAX_DOZE_WAITING_TIMES_9x, QueueID);
							break;
						}
					}
				}
				#ifdef RTL8190P
				if(priv->rf_type == RF_2T4R)
				{
					
					rtl8192_setBBreg(dev, rFPGA0_XA_RFInterfaceOE, BIT4, 0x0);		
				}
				
				rtl8192_setBBreg(dev, rFPGA0_XC_RFInterfaceOE, BIT4, 0x0); 
				
				rtl8192_setBBreg(dev, rFPGA0_AnalogParameter4, 0xf00, 0x0);
				
				rtl8192_setBBreg(dev, rFPGA0_AnalogParameter1, 0x1e0, 0x0); 
				
				rtl8192_setBBreg(dev, rOFDM0_TRxPathEnable, 0xf, 0x0);
				
				rtl8192_setBBreg(dev, rOFDM1_TRxPathEnable, 0xf, 0x0);
				
				rtl8192_setBBreg(dev, rFPGA0_AnalogParameter1, 0x1e00, 0x0); 
#else 
					
				
				rtl8192_setBBreg(dev, rFPGA0_XA_RFInterfaceOE, BIT4, 0x0);		
					rtl8192_setBBreg(dev, rFPGA0_XB_RFInterfaceOE, BIT4, 0x0);		
					
				
					
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter4, 0xf03, 0x0); 
				
					
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter1, 0x98, 0x0); 
					
					
					
					
				
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter1, 0x60, 0x0);		
					
					rtl8192_setBBreg(dev, rFPGA0_AnalogParameter2, 0x20000000, 0x0); 


					
					
					
					
					write_nic_byte(dev, BB_RESET, (read_nic_byte(dev, BB_RESET)|BIT0)); 
					
					write_nic_byte(dev, MacBlkCtrl, 0x0); 
					
					write_nic_byte(dev, ANAPAR, 0x07); 
				priv->bHwRfOffAction = 0;
				
				#endif
					break;

			default:
					bResult = false;
					RT_TRACE(COMP_ERR, "SetRFPowerState8190(): unknow state to set: 0x%X!!!\n", eRFPowerState);
					break;
		}

		break;

		default:
			RT_TRACE(COMP_ERR, "SetRFPowerState8190(): Unknown RF type\n");
			break;
	}

	if(bResult)
	{
		
		priv->ieee80211->eRFPowerState = eRFPowerState;

		switch(priv->rf_chip )
		{
			case RF_8256:
			switch(priv->ieee80211->eRFPowerState)
			{
				case eRfOff:
				
				
				
					if(priv->ieee80211->RfOffReason==RF_CHANGE_BY_IPS )
					{
						#ifdef TO_DO
						Adapter->HalFunc.LedControlHandler(Adapter,LED_CTL_NO_LINK);
						#endif
					}
					else
					{
					
						#ifdef TO_DO
						Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_POWER_OFF);
						#endif
					}
					break;

				case eRfOn:
				
				
					if( priv->ieee80211->state == IEEE80211_LINKED)
					{
						#ifdef TO_DO
						Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_LINK);
						#endif
					}
					else
					{
					
						#ifdef TO_DO
						Adapter->HalFunc.LedControlHandler(Adapter, LED_CTL_NO_LINK);
						#endif
					}
					break;

				default:
			
					break;
			}

			break;

			default:
				RT_TRACE(COMP_ERR, "SetRFPowerState8190(): Unknown RF type\n");
				break;
		}
	}

	priv->SetRFPowerStateInProgress = false;
	RT_TRACE(COMP_POWER, "<=========== SetRFPowerState8190() bResult = %d!\n", bResult);
	return bResult;
}













static bool
SetRFPowerState(
	struct net_device* dev,
	RT_RF_POWER_STATE	eRFPowerState
	)
{
	struct r8192_priv *priv = ieee80211_priv(dev);

	bool bResult = false;

	RT_TRACE(COMP_RF,"---------> SetRFPowerState(): eRFPowerState(%d)\n", eRFPowerState);
#ifdef RTL8192E
	if(eRFPowerState == priv->ieee80211->eRFPowerState && priv->bHwRfOffAction == 0)
#else
	if(eRFPowerState == priv->ieee80211->eRFPowerState)
#endif
	{
		RT_TRACE(COMP_POWER, "<--------- SetRFPowerState(): discard the request for eRFPowerState(%d) is the same.\n", eRFPowerState);
		return bResult;
	}

	bResult = SetRFPowerState8190(dev, eRFPowerState);

	RT_TRACE(COMP_POWER, "<--------- SetRFPowerState(): bResult(%d)\n", bResult);

	return bResult;
}

static void
MgntDisconnectIBSS(
	struct net_device* dev
)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	
	u8			i;
	bool	bFilterOutNonAssociatedBSSID = false;

	

	priv->ieee80211->state = IEEE80211_NOLINK;


	for(i=0;i<6;i++)  priv->ieee80211->current_network.bssid[i]= 0x55;
	priv->OpMode = RT_OP_MODE_NO_LINK;
	write_nic_word(dev, BSSIDR, ((u16*)priv->ieee80211->current_network.bssid)[0]);
	write_nic_dword(dev, BSSIDR+2, ((u32*)(priv->ieee80211->current_network.bssid+2))[0]);
	{
			RT_OP_MODE	OpMode = priv->OpMode;
			
			u8	btMsr = read_nic_byte(dev, MSR);

			btMsr &= 0xfc;

			switch(OpMode)
			{
			case RT_OP_MODE_INFRASTRUCTURE:
				btMsr |= MSR_LINK_MANAGED;
				
				break;

			case RT_OP_MODE_IBSS:
				btMsr |= MSR_LINK_ADHOC;
				
				break;

			case RT_OP_MODE_AP:
				btMsr |= MSR_LINK_MASTER;
				
				break;

			default:
				btMsr |= MSR_LINK_NONE;
				break;
			}

			write_nic_byte(dev, MSR, btMsr);

			
			
	}
	ieee80211_stop_send_beacons(priv->ieee80211);

	
	bFilterOutNonAssociatedBSSID = false;
	{
			u32 RegRCR, Type;
			Type = bFilterOutNonAssociatedBSSID;
			RegRCR = read_nic_dword(dev,RCR);
			priv->ReceiveConfig = RegRCR;
			if (Type == true)
				RegRCR |= (RCR_CBSSID);
			else if (Type == false)
				RegRCR &= (~RCR_CBSSID);

			{
				write_nic_dword(dev, RCR,RegRCR);
				priv->ReceiveConfig = RegRCR;
			}

		}
	
	notify_wx_assoc_event(priv->ieee80211);

}

static void
MlmeDisassociateRequest(
	struct net_device* dev,
	u8* 		asSta,
	u8			asRsn
	)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	u8 i;

	RemovePeerTS(priv->ieee80211, asSta);

	SendDisassociation( priv->ieee80211, asSta, asRsn );

	if(memcpy(priv->ieee80211->current_network.bssid,asSta,6) == NULL)
	{
		
		
		priv->ieee80211->state = IEEE80211_NOLINK;
		
		for(i=0;i<6;i++)  priv->ieee80211->current_network.bssid[i] = 0x22;


		priv->OpMode = RT_OP_MODE_NO_LINK;
		{
			RT_OP_MODE	OpMode = priv->OpMode;
			
			u8 btMsr = read_nic_byte(dev, MSR);

			btMsr &= 0xfc;

			switch(OpMode)
			{
			case RT_OP_MODE_INFRASTRUCTURE:
				btMsr |= MSR_LINK_MANAGED;
				
				break;

			case RT_OP_MODE_IBSS:
				btMsr |= MSR_LINK_ADHOC;
				
				break;

			case RT_OP_MODE_AP:
				btMsr |= MSR_LINK_MASTER;
				
				break;

			default:
				btMsr |= MSR_LINK_NONE;
				break;
			}

			write_nic_byte(dev, MSR, btMsr);

			
			
		}
		ieee80211_disassociate(priv->ieee80211);

		write_nic_word(dev, BSSIDR, ((u16*)priv->ieee80211->current_network.bssid)[0]);
		write_nic_dword(dev, BSSIDR+2, ((u32*)(priv->ieee80211->current_network.bssid+2))[0]);

	}

}


static void
MgntDisconnectAP(
	struct net_device* dev,
	u8 asRsn
)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	bool bFilterOutNonAssociatedBSSID = false;








	
#ifdef TO_DO
	if(   pMgntInfo->SecurityInfo.AuthMode > RT_802_11AuthModeAutoSwitch ||
		(pMgntInfo->bAPSuportCCKM && pMgntInfo->bCCX8021xenable) )	
	{
		SecClearAllKeys(Adapter);
		RT_TRACE(COMP_SEC, DBG_LOUD,("======>CCKM clear key..."))
	}
#endif
	
	bFilterOutNonAssociatedBSSID = false;
	{
			u32 RegRCR, Type;

			Type = bFilterOutNonAssociatedBSSID;
			
			RegRCR = read_nic_dword(dev,RCR);
			priv->ReceiveConfig = RegRCR;

			if (Type == true)
				RegRCR |= (RCR_CBSSID);
			else if (Type == false)
				RegRCR &= (~RCR_CBSSID);

			write_nic_dword(dev, RCR,RegRCR);
			priv->ReceiveConfig = RegRCR;


	}
	
	
	MlmeDisassociateRequest( dev, priv->ieee80211->current_network.bssid, asRsn );

	priv->ieee80211->state = IEEE80211_NOLINK;
	
}


static bool
MgntDisconnect(
	struct net_device* dev,
	u8 asRsn
)
{
	struct r8192_priv *priv = ieee80211_priv(dev);

	
	
	
#ifdef TO_DO
	if(pMgntInfo->mPss != eAwake)
	{
		
		
		
		
		
		PlatformSetTimer( Adapter, &(pMgntInfo->AwakeTimer), 0 );
	}
#endif
	
#ifdef TO_DO
	if(pMgntInfo->mActingAsAp)
	{
		RT_TRACE(COMP_MLME, DBG_LOUD, ("MgntDisconnect() ===> AP_DisassociateAllStation\n"));
		AP_DisassociateAllStation(Adapter, unspec_reason);
		return TRUE;
	}
#endif
	
	

	
	if( priv->ieee80211->state == IEEE80211_LINKED )
	{
		if( priv->ieee80211->iw_mode == IW_MODE_ADHOC )
		{
			
			MgntDisconnectIBSS(dev);
		}
		if( priv->ieee80211->iw_mode == IW_MODE_INFRA )
		{
			
			
			
			
			
			
			MgntDisconnectAP(dev, asRsn);
		}

		
		
	}

	return true;
}









bool
MgntActSet_RF_State(
	struct net_device* dev,
	RT_RF_POWER_STATE	StateToSet,
	RT_RF_CHANGE_SOURCE ChangeSource
	)
{
	struct r8192_priv *priv = ieee80211_priv(dev);
	bool 			bActionAllowed = false;
	bool 			bConnectBySSID = false;
	RT_RF_POWER_STATE	rtState;
	u16					RFWaitCounter = 0;
	unsigned long flag;
	RT_TRACE(COMP_POWER, "===>MgntActSet_RF_State(): StateToSet(%d)\n",StateToSet);

	
	
	
	

	while(true)
	{
		spin_lock_irqsave(&priv->rf_ps_lock,flag);
		if(priv->RFChangeInProgress)
		{
			spin_unlock_irqrestore(&priv->rf_ps_lock,flag);
			RT_TRACE(COMP_POWER, "MgntActSet_RF_State(): RF Change in progress! Wait to set..StateToSet(%d).\n", StateToSet);

			
			while(priv->RFChangeInProgress)
			{
				RFWaitCounter ++;
				RT_TRACE(COMP_POWER, "MgntActSet_RF_State(): Wait 1 ms (%d times)...\n", RFWaitCounter);
				udelay(1000); 

				
				if(RFWaitCounter > 100)
				{
					RT_TRACE(COMP_ERR, "MgntActSet_RF_State(): Wait too logn to set RF\n");
					
					return false;
				}
			}
		}
		else
		{
			priv->RFChangeInProgress = true;
			spin_unlock_irqrestore(&priv->rf_ps_lock,flag);
			break;
		}
	}

	rtState = priv->ieee80211->eRFPowerState;

	switch(StateToSet)
	{
	case eRfOn:
		
		
		
		

		priv->ieee80211->RfOffReason &= (~ChangeSource);

		if(! priv->ieee80211->RfOffReason)
		{
			priv->ieee80211->RfOffReason = 0;
			bActionAllowed = true;


			if(rtState == eRfOff && ChangeSource >=RF_CHANGE_BY_HW )
			{
				bConnectBySSID = true;
			}
		}
		else
			RT_TRACE(COMP_POWER, "MgntActSet_RF_State - eRfon reject pMgntInfo->RfOffReason= 0x%x, ChangeSource=0x%X\n", priv->ieee80211->RfOffReason, ChangeSource);

		break;

	case eRfOff:

			if (priv->ieee80211->RfOffReason > RF_CHANGE_BY_IPS)
			{
				
				
				
				
				
				
				MgntDisconnect(dev, disas_lv_ss);

				
				

			}


		priv->ieee80211->RfOffReason |= ChangeSource;
		bActionAllowed = true;
		break;

	case eRfSleep:
		priv->ieee80211->RfOffReason |= ChangeSource;
		bActionAllowed = true;
		break;

	default:
		break;
	}

	if(bActionAllowed)
	{
		RT_TRACE(COMP_POWER, "MgntActSet_RF_State(): Action is allowed.... StateToSet(%d), RfOffReason(%#X)\n", StateToSet, priv->ieee80211->RfOffReason);
				
		SetRFPowerState(dev, StateToSet);
		
		if(StateToSet == eRfOn)
		{
			
			if(bConnectBySSID)
			{
				
			}
		}
		
		else if(StateToSet == eRfOff)
		{
			
		}
	}
	else
	{
		RT_TRACE(COMP_POWER, "MgntActSet_RF_State(): Action is rejected.... StateToSet(%d), ChangeSource(%#X), RfOffReason(%#X)\n", StateToSet, ChangeSource, priv->ieee80211->RfOffReason);
	}

	
	spin_lock_irqsave(&priv->rf_ps_lock,flag);
	priv->RFChangeInProgress = false;
	spin_unlock_irqrestore(&priv->rf_ps_lock,flag);

	RT_TRACE(COMP_POWER, "<===MgntActSet_RF_State()\n");
	return bActionAllowed;
}


