

#ifndef _SSM2602_H
#define _SSM2602_H



#define SSM2602_LINVOL   0x00
#define SSM2602_RINVOL   0x01
#define SSM2602_LOUT1V   0x02
#define SSM2602_ROUT1V   0x03
#define SSM2602_APANA    0x04
#define SSM2602_APDIGI   0x05
#define SSM2602_PWR      0x06
#define SSM2602_IFACE    0x07
#define SSM2602_SRATE    0x08
#define SSM2602_ACTIVE   0x09
#define SSM2602_RESET	 0x0f




#define     LINVOL_LIN_VOL                0x01F   
#define     LINVOL_LIN_ENABLE_MUTE        0x080   
#define     LINVOL_LRIN_BOTH              0x100   


#define     RINVOL_RIN_VOL                0x01F   
#define     RINVOL_RIN_ENABLE_MUTE        0x080   
#define     RINVOL_RLIN_BOTH              0x100   


#define     LOUT1V_LHP_VOL                0x07F   
#define     LOUT1V_ENABLE_LZC             0x080   
#define     LOUT1V_LRHP_BOTH              0x100   


#define     ROUT1V_RHP_VOL                0x07F   
#define     ROUT1V_ENABLE_RZC             0x080   
#define     ROUT1V_RLHP_BOTH              0x100   


#define     APANA_ENABLE_MIC_BOOST       0x001   
#define     APANA_ENABLE_MIC_MUTE        0x002   
#define     APANA_ADC_IN_SELECT          0x004   
#define     APANA_ENABLE_BYPASS          0x008   
#define     APANA_SELECT_DAC             0x010   
#define     APANA_ENABLE_SIDETONE        0x020   
#define     APANA_SIDETONE_ATTN          0x0C0   
#define     APANA_ENABLE_MIC_BOOST2      0x100   


#define     APDIGI_ENABLE_ADC_HPF         0x001   
#define     APDIGI_DE_EMPHASIS            0x006   
#define     APDIGI_ENABLE_DAC_MUTE        0x008   
#define     APDIGI_STORE_OFFSET           0x010   


#define     PWR_LINE_IN_PDN            0x001   
#define     PWR_MIC_PDN                0x002   
#define     PWR_ADC_PDN                0x004   
#define     PWR_DAC_PDN                0x008   
#define     PWR_OUT_PDN                0x010   
#define     PWR_OSC_PDN                0x020   
#define     PWR_CLK_OUT_PDN            0x040   
#define     PWR_POWER_OFF              0x080   


#define     IFACE_IFACE_FORMAT           0x003   
#define     IFACE_AUDIO_DATA_LEN         0x00C   
#define     IFACE_DAC_LR_POLARITY        0x010   
#define     IFACE_DAC_LR_SWAP            0x020   
#define     IFACE_ENABLE_MASTER          0x040   
#define     IFACE_BCLK_INVERT            0x080   


#define     SRATE_ENABLE_USB_MODE        0x001   
#define     SRATE_BOS_RATE               0x002   
#define     SRATE_SAMPLE_RATE            0x03C   
#define     SRATE_CORECLK_DIV2           0x040   
#define     SRATE_CLKOUT_DIV2            0x080   


#define     ACTIVE_ACTIVATE_CODEC         0x001   



#define SSM2602_CACHEREGNUM 	10

#define SSM2602_SYSCLK	0
#define SSM2602_DAI		0

struct ssm2602_setup_data {
	int i2c_bus;
	unsigned short i2c_address;
};

extern struct snd_soc_dai ssm2602_dai;
extern struct snd_soc_codec_device soc_codec_dev_ssm2602;

#endif
