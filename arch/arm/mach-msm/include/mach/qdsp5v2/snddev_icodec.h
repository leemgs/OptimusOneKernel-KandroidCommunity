
#ifndef __MACH_QDSP5_V2_SNDDEV_ICODEC_H
#define __MACH_QDSP5_V2_SNDDEV_ICODEC_H
#include <mach/qdsp5v2/adie_marimba.h>
#include <mach/qdsp5v2/audio_def.h>
#include <mach/pmic.h>

struct snddev_icodec_data {
	u32 capability; 
	const char *name;
	u32 copp_id; 
	u32 acdb_id; 
	
	struct adie_codec_dev_profile *profile;
	
	u8 channel_mode;
	enum hsed_controller *pmctl_id; 
	u32 pmctl_id_sz;
	u32 default_sample_rate;
	void (*pamp_on) (void);
	void (*pamp_off) (void);
	void (*voltage_on) (void);
	void (*voltage_off) (void);
	s32 max_voice_rx_vol[VOC_RX_VOL_ARRAY_NUM]; 
	s32 min_voice_rx_vol[VOC_RX_VOL_ARRAY_NUM];
	u32 dev_vol_type;
};
#endif
