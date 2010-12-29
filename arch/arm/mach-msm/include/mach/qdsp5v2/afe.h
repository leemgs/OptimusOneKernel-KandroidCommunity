
#ifndef _MACH_QDSP5_V2_AFE_H
#define _MACH_QDSP5_V2_AFE_H

#include <asm/types.h>

#define AFE_HW_PATH_CODEC_RX    1
#define AFE_HW_PATH_CODEC_TX    2
#define AFE_HW_PATH_AUXPCM_RX   3
#define AFE_HW_PATH_AUXPCM_TX   4
#define AFE_HW_PATH_MI2S_RX     5
#define AFE_HW_PATH_MI2S_TX     6

#define AFE_VOLUME_UNITY 0x4000 

struct msm_afe_config {
	u16 sample_rate;
	u16 channel_mode;
	u16 volume;
	
};

int afe_enable(u8 path_id, struct msm_afe_config *config);

int afe_disable(u8 path_id);

int afe_config_aux_codec(int pcm_ctl_value, int aux_codec_intf_value,
			int data_format_pad);
int afe_config_fm_codec(int fm_enable, uint16_t source);

int afe_config_fm_volume(uint16_t volume);

#endif
