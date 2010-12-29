
#ifndef __MACH_QDSP5_V2_LPA_H__
#define __MACH_QDSP5_V2_LPA_H__

#define LPA_OUTPUT_INTF_WB_CODEC 3
#define LPA_OUTPUT_INTF_SDAC     1
#define LPA_OUTPUT_INTF_MI2S     2

struct lpa_codec_config {
	uint32_t sample_rate;
	uint32_t sample_width;
	uint32_t output_interface;
	uint32_t num_channels;
};

struct lpa_drv;

struct lpa_drv *lpa_get(void);
void lpa_put(struct lpa_drv *lpa);
int lpa_cmd_codec_config(struct lpa_drv *lpa,
	struct lpa_codec_config *config_ptr);
int lpa_cmd_enable_codec(struct lpa_drv *lpa, bool enable);

#endif

