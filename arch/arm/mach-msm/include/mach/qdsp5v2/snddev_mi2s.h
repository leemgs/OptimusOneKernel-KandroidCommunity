
#ifndef __MACH_QDSP5_V2_SNDDEV_MI2S_H
#define __MACH_QDSP5_V2_SNDDEV_MI2S_H

struct snddev_mi2s_data {
	u32 capability; 
	const char *name;
	u32 copp_id; 
	u32 acdb_id; 
	u8 channel_mode;
	u8 sd_lines;
	void (*route) (void);
	void (*deroute) (void);
	u32 default_sample_rate;
};

int mi2s_config_clk_gpio(void);

int mi2s_config_data_gpio(u32 direction, u8 sd_line_mask);

int mi2s_unconfig_clk_gpio(void);

int mi2s_unconfig_data_gpio(u32 direction, u8 sd_line_mask);

#endif
