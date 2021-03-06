








#define	RF6052_MAX_TX_PWR	0x3F
#define	RF6052_MAX_REG		0x3F
#define	RF6052_MAX_PATH		4


















#if 1


extern void PHY_SetRF0222DBandwidth(struct net_device* dev , HT_CHANNEL_WIDTH Bandwidth);	
extern void PHY_SetRF8225Bandwidth(	struct net_device* dev ,	HT_CHANNEL_WIDTH Bandwidth);
extern bool PHY_RF8225_Config(struct net_device* dev );
extern void phy_RF8225_Config_HardCode(struct net_device*	dev);
extern bool phy_RF8225_Config_ParaFile(struct net_device* dev);
extern void PHY_SetRF8225CckTxPower(struct net_device* dev ,u8 powerlevel);
extern void PHY_SetRF8225OfdmTxPower(struct net_device* dev ,u8        powerlevel);
extern void PHY_SetRF0222DOfdmTxPower(struct net_device* dev ,u8 powerlevel);
extern void PHY_SetRF0222DCckTxPower(struct net_device* dev ,u8        powerlevel);




extern void PHY_SetRF8256Bandwidth(struct net_device* dev , HT_CHANNEL_WIDTH Bandwidth);
extern void PHY_RF8256_Config(struct net_device* dev);
extern void phy_RF8256_Config_ParaFile(struct net_device* dev);
extern void PHY_SetRF8256CCKTxPower(struct net_device*	dev, u8	powerlevel);
extern void PHY_SetRF8256OFDMTxPower(struct net_device* dev, u8 powerlevel);
#endif




extern	void		RF_ChangeTxPath(struct net_device  * dev, u16 DataRate);
extern	void		PHY_RF6052SetBandwidth(struct net_device  * dev,HT_CHANNEL_WIDTH	Bandwidth);
extern	void		PHY_RF6052SetCckTxPower(struct net_device  * dev, u8	powerlevel);
extern	void		PHY_RF6052SetOFDMTxPower(struct net_device  * dev, u8 powerlevel);
extern	RT_STATUS	PHY_RF6052_Config(struct net_device  * dev);
extern void PHY_RFShadowRefresh( struct net_device  		* dev);
extern void PHY_RFShadowWrite( struct net_device* dev, u32 eRFPath, u32 Offset, u32 Data);




