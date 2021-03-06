
#ifndef __LINUX_MFD_MSM_MARIMBA_CODEC_H
#define __LINUX_MFD_MSM_MARIMBA_CODEC_H

#include <mach/qdsp5v2/adie_marimba.h>

struct adie_codec_register {
	u8 reg;
	u8 mask;
	u8 val;
};

struct adie_codec_register_image {
	struct adie_codec_register *regs;
	u32 img_sz;
};

struct adie_codec_path {
	struct adie_codec_dev_profile *profile;
	struct adie_codec_register_image img;
	u32 hwsetting_idx;
	u32 stage_idx;
	u32 curr_stage;
};

int adie_codec_open(struct adie_codec_dev_profile *profile,
	struct adie_codec_path **path_pptr);
int adie_codec_setpath(struct adie_codec_path *path_ptr,
	u32 freq_plan, u32 osr);
int adie_codec_proceed_stage(struct adie_codec_path *path_ptr, u32 state);
int adie_codec_close(struct adie_codec_path *path_ptr);
u32 adie_codec_freq_supported(struct adie_codec_dev_profile *profile,
							u32 requested_freq);
int adie_codec_enable_sidetone(struct adie_codec_path *rx_path_ptr, u32 enable);

int adie_codec_set_device_digital_volume(struct adie_codec_path *path_ptr,
		u32 num_channels, u32 vol_percentage );

int adie_codec_set_device_analog_volume(struct adie_codec_path *path_ptr,
		u32 num_channels, u32 volume );
#endif
