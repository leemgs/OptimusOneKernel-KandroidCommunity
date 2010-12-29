
#ifndef __MACH_QDSP6V2_SNDDEV_ECODEC_H
#define __MACH_QDSP6V2_SNDDEV_ECODEC_H
#include <mach/qdsp5v2/audio_def.h>

struct snddev_ecodec_data {
	u32 capability; 
	const char *name;
	u32 copp_id; 
	u32 acdb_id; 
	u8 channel_mode;
	u32 conf_pcm_ctl_val;
	u32 conf_aux_codec_intf;
	u32 conf_data_format_padding_val;
	s32 max_voice_rx_vol[VOC_RX_VOL_ARRAY_NUM]; 
	s32 min_voice_rx_vol[VOC_RX_VOL_ARRAY_NUM];
};
#endif
