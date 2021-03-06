


#ifndef _MARIMBA_H
#define _MARIMBA_H_

#include <linux/types.h>
#include <linux/i2c.h>
#include <mach/msm_ts.h>
#include <mach/vreg.h>

#define MARIMBA_NUM_CHILD			4

#define MARIMBA_SLAVE_ID_MARIMBA	0x00
#define MARIMBA_SLAVE_ID_FM			0x01
#define MARIMBA_SLAVE_ID_CDC		0x02
#define MARIMBA_SLAVE_ID_QMEMBIST	0x03

#define MARIMBA_ID_TSADC			0x04

#define MARIMBA_SSBI_ADAP		0x7

struct marimba{
	struct i2c_client *client;

	struct i2c_msg xfer_msg[2];

	struct mutex xfer_lock;

	int mod_id;
};

struct marimba_top_level_platform_data{
	int slave_id;     
};

struct marimba_fm_platform_data{
	int irq;
	int (*fm_setup)(struct marimba_fm_platform_data *pdata);
	void (*fm_shutdown)(struct marimba_fm_platform_data *pdata);
	struct vreg *vreg_s2;
	struct vreg *vreg_xo_out;
};

struct marimba_codec_platform_data{
	int (*marimba_codec_power)(int vreg_on);
};

struct marimba_tsadc_setup_params {
	bool pen_irq_en;
	bool tsadc_en;
};

enum sample_period {
	TSADC_CLK_3 = 0,
	TSADC_CLK_24,
	TSADC_CLK_36,
	TSADC_CLK_48,
	TSADC_CLK_1,
	TSADC_CLK_2,
	TSADC_CLK_6,
	TSADC_CLK_12,
	TSADC_CLOCK_MAX
};

struct marimba_tsadc_config_params2 {
	unsigned long input_clk_khz;
	enum sample_period sample_prd;
};

struct marimba_tsadc_config_params3 {
	unsigned long prechg_time_nsecs;
	unsigned long stable_time_nsecs;
	unsigned long tsadc_test_mode;
};

struct marimba_tsadc_platform_data {
	int (*marimba_tsadc_power)(int vreg_on);
	int (*init)(void);
	int (*exit)(void);
	int (*level_vote)(int vote_on);
	bool tsadc_prechg_en;
	struct marimba_tsadc_setup_params setup;
	struct marimba_tsadc_config_params2 params2;
	struct marimba_tsadc_config_params3 params3;

	struct msm_ts_platform_data *tssc_data;
};


struct marimba_platform_data {
	struct marimba_top_level_platform_data	*marimba_tp_level;
	struct marimba_fm_platform_data		*fm;
	struct marimba_codec_platform_data	*codec;
	struct marimba_tsadc_platform_data	*tsadc;
	u8 slave_id[MARIMBA_NUM_CHILD + 1];
	u32 (*marimba_setup) (void);
	void (*marimba_shutdown) (void);
};


int marimba_read(struct marimba *, u8 reg, u8 *value, unsigned num_bytes);
int marimba_write(struct marimba *, u8 reg, u8 *value, unsigned num_bytes);


int marimba_read_bit_mask(struct marimba *, u8 reg, u8 *value,
					unsigned num_bytes, u8 mask);
int marimba_write_bit_mask(struct marimba *, u8 reg, u8 *value,
					unsigned num_bytes, u8 mask);


int marimba_ssbi_read(struct marimba *, u16 reg, u8 *value, int len);
int marimba_ssbi_write(struct marimba *, u16 reg , u8 *value, int len);

#endif
