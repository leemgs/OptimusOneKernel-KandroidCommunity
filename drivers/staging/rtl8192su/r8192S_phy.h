
 
#ifndef _R8192S_PHY_H
#define _R8192S_PHY_H



#define LOOP_LIMIT				5
#define MAX_STALL_TIME			50		
#define AntennaDiversityValue		0x80	
#define MAX_TXPWR_IDX_NMODE_92S	63





#define MAX_PRECMD_CNT 			16
#define MAX_RFDEPENDCMD_CNT 	16
#define MAX_POSTCMD_CNT 		16



typedef enum _SwChnlCmdID{
	CmdID_End,
	CmdID_SetTxPowerLevel,
	CmdID_BBRegWrite10,
	CmdID_WritePortUlong,
	CmdID_WritePortUshort,
	CmdID_WritePortUchar,
	CmdID_RF_WriteReg,
}SwChnlCmdID;



typedef struct _SwChnlCmd{
	SwChnlCmdID	CmdID;
	u32			Para1;
	u32			Para2;
	u32			msDelay;
}__attribute__ ((packed)) SwChnlCmd;

extern u32 rtl819XMACPHY_Array_PG[];
extern u32 rtl819XPHY_REG_1T2RArray[];
extern u32 rtl819XAGCTAB_Array[];
extern u32 rtl819XRadioA_Array[];
extern u32 rtl819XRadioB_Array[];
extern u32 rtl819XRadioC_Array[];
extern u32 rtl819XRadioD_Array[];

typedef enum _HW90_BLOCK{
	HW90_BLOCK_MAC = 0,
	HW90_BLOCK_PHY0 = 1,
	HW90_BLOCK_PHY1 = 2,
	HW90_BLOCK_RF = 3,
	HW90_BLOCK_MAXIMUM = 4, 
}HW90_BLOCK_E, *PHW90_BLOCK_E;

typedef enum _RF90_RADIO_PATH{
	RF90_PATH_A = 0,			
	RF90_PATH_B = 1,			
	RF90_PATH_C = 2,			
	RF90_PATH_D = 3,			
	RF90_PATH_MAX	= 4,			
}RF90_RADIO_PATH_E, *PRF90_RADIO_PATH_E;

#define bMaskByte0                0xff
#define bMaskByte1                0xff00
#define bMaskByte2                0xff0000
#define bMaskByte3                0xff000000
#define bMaskHWord                0xffff0000
#define bMaskLWord                0x0000ffff
#define bMaskDWord                0xffffffff

typedef enum _VERSION_8190{
	
	VERSION_8190_BD=0x3,
	VERSION_8190_BE
}VERSION_8190,*PVERSION_8190;





extern	u32	rtl8192_QueryBBReg(struct net_device* dev,u32 RegAddr, u32 BitMask);
extern	void	rtl8192_setBBreg(struct net_device* dev,u32 RegAddr, u32 BitMask,u32 Data);
extern	u32	rtl8192_phy_QueryRFReg(struct net_device* dev,RF90_RADIO_PATH_E eRFPath, u32 RegAddr, u32 BitMask);
extern	void	rtl8192_phy_SetRFReg(struct net_device* dev,RF90_RADIO_PATH_E eRFPath, u32 RegAddr,u32 BitMask,u32 Data);

bool rtl8192_phy_checkBBAndRF(struct net_device* dev, HW90_BLOCK_E CheckBlock, RF90_RADIO_PATH_E eRFPath);



extern	bool 	PHY_MACConfig8192S(struct net_device* dev);
extern	bool 	PHY_BBConfig8192S(struct net_device* dev);
extern	bool 	PHY_RFConfig8192S(struct net_device* dev);

extern	u8	rtl8192_phy_ConfigRFWithHeaderFile(struct net_device* dev,RF90_RADIO_PATH_E eRFPath);
extern	void	rtl8192_SetBWMode(struct net_device* dev,HT_CHANNEL_WIDTH ChnlWidth,HT_EXTCHNL_OFFSET Offset	);
extern	u8	rtl8192_phy_SwChnl(struct net_device* dev,u8 channel);
extern	u8	rtl8192_phy_CheckIsLegalRFPath(struct net_device* dev,u32 eRFPath	);
extern	void    rtl8192_BBConfig(struct net_device* dev);
extern	void 	PHY_IQCalibrateBcut(struct net_device* dev);
extern	void 	PHY_IQCalibrate(struct net_device* dev);
extern	void 	PHY_GetHWRegOriginalValue(struct net_device* dev);

extern void 	InitialGainOperateWorkItemCallBack(struct work_struct *work);

void PHY_SetTxPowerLevel8192S(struct net_device* dev, u8  channel);
void PHY_InitialGain8192S(struct net_device* dev,u8 Operation   );


bool HalSetFwCmd8192S(struct net_device* dev, FW_CMD_IO_TYPE                FwCmdIO);
extern void PHY_SetBeaconHwReg( struct net_device* dev, u16 BeaconInterval);
void ChkFwCmdIoDone(struct net_device* dev);

#endif	

