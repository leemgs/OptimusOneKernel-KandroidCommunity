
#ifndef __MACH_QDSP5_V2_QDSP5AFECMDI_H
#define __MACH_QDSP5_V2_QDSP5AFECMDI_H

#define QDSP5_DEVICE_mI2S_CODEC_RX 1     
#define QDSP5_DEVICE_mI2S_CODEC_TX 2     
#define QDSP5_DEVICE_AUX_CODEC_RX  3     
#define QDSP5_DEVICE_AUX_CODEC_TX  4     
#define QDSP5_DEVICE_mI2S_HDMI_RX  5     
#define QDSP5_DEVICE_mI2S_HDMI_TX  6     
#define QDSP5_DEVICE_ID_MAX        7

#define AFE_CMD_CODEC_CONFIG_CMD     0x1
#define AFE_CMD_CODEC_CONFIG_LEN sizeof(struct afe_cmd_codec_config)

struct afe_cmd_codec_config{
	uint16_t cmd_id;
	uint16_t device_id;
	uint16_t activity;
	uint16_t sample_rate;
	uint16_t channel_mode;
	uint16_t volume;
	uint16_t reserved;
} __attribute__ ((packed));

#define AFE_CMD_AUX_CODEC_CONFIG_CMD 	0x3
#define AFE_CMD_AUX_CODEC_CONFIG_LEN sizeof(struct afe_cmd_aux_codec_config)

struct afe_cmd_aux_codec_config{
	uint16_t cmd_id;
	uint16_t dma_path_ctl;
	uint16_t pcm_ctl;
	uint16_t eight_khz_int_mode;
	uint16_t aux_codec_intf_ctl;
	uint16_t data_format_padding_info;
} __attribute__ ((packed));

#define AFE_CMD_FM_RX_ROUTING_CMD	0x6
#define AFE_CMD_FM_RX_ROUTING_LEN sizeof(struct afe_cmd_fm_codec_config)

struct afe_cmd_fm_codec_config{
	uint16_t cmd_id;
	uint16_t enable;
	uint16_t device_id;
} __attribute__ ((packed));

#define AFE_CMD_FM_PLAYBACK_VOLUME_CMD	0x8
#define AFE_CMD_FM_PLAYBACK_VOLUME_LEN sizeof(struct afe_cmd_fm_volume_config)

struct afe_cmd_fm_volume_config{
	uint16_t cmd_id;
	uint16_t volume;
	uint16_t reserved;
} __attribute__ ((packed));

#endif
