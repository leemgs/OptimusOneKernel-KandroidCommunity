

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl4030.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>
#include <sound/tlv.h>

#include "twl4030.h"


static const u8 twl4030_reg[TWL4030_CACHEREGNUM] = {
	0x00, 
	0x91, 
	0xc3, 
	0x00, 
	0x00, 
	0x20, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x0c, 
	0x0c, 
	0x00, 
	0x00, 
	0x01, 
	0x00, 
	0x00, 
	0x00, 
	0x6c, 
	0x6c, 
	0x00, 
	0x00, 
	0x00, 
	0x0c, 
	0x00, 
	0x00, 
	0x00, 
	0x4b, 
	0x4b, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x24, 
	0x0a, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x16, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
	0x00, 
};


struct twl4030_priv {
	unsigned int bypass_state;
	unsigned int codec_powered;
	unsigned int codec_muted;

	struct snd_pcm_substream *master_substream;
	struct snd_pcm_substream *slave_substream;

	unsigned int configured;
	unsigned int rate;
	unsigned int sample_bits;
	unsigned int channels;

	unsigned int sysclk;

	
	unsigned int hsl_enabled;
	unsigned int hsr_enabled;
};


static inline unsigned int twl4030_read_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg)
{
	u8 *cache = codec->reg_cache;

	if (reg >= TWL4030_CACHEREGNUM)
		return -EIO;

	return cache[reg];
}


static inline void twl4030_write_reg_cache(struct snd_soc_codec *codec,
						u8 reg, u8 value)
{
	u8 *cache = codec->reg_cache;

	if (reg >= TWL4030_CACHEREGNUM)
		return;
	cache[reg] = value;
}


static int twl4030_write(struct snd_soc_codec *codec,
			unsigned int reg, unsigned int value)
{
	twl4030_write_reg_cache(codec, reg, value);
	if (likely(reg < TWL4030_REG_SW_SHADOW))
		return twl4030_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE, value,
					    reg);
	else
		return 0;
}

static void twl4030_codec_enable(struct snd_soc_codec *codec, int enable)
{
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 mode;

	if (enable == twl4030->codec_powered)
		return;

	mode = twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE);
	if (enable)
		mode |= TWL4030_CODECPDZ;
	else
		mode &= ~TWL4030_CODECPDZ;

	twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
	twl4030->codec_powered = enable;

	
	
	udelay(10);
}

static void twl4030_init_chip(struct snd_soc_codec *codec)
{
	u8 *cache = codec->reg_cache;
	int i;

	
	twl4030_codec_enable(codec, 0);

	
	for (i = TWL4030_REG_OPTION; i <= TWL4030_REG_MISC_SET_2; i++)
		twl4030_write(codec, i,	cache[i]);

}

static void twl4030_codec_mute(struct snd_soc_codec *codec, int mute)
{
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 reg_val;

	if (mute == twl4030->codec_muted)
		return;

	if (mute) {
		
		reg_val = twl4030_read_reg_cache(codec, TWL4030_REG_APLL_CTL);
		reg_val &= ~TWL4030_APLL_EN;
		twl4030_write(codec, TWL4030_REG_APLL_CTL, reg_val);
	} else {
		
		reg_val = twl4030_read_reg_cache(codec, TWL4030_REG_APLL_CTL);
		reg_val |= TWL4030_APLL_EN;
		twl4030_write(codec, TWL4030_REG_APLL_CTL, reg_val);
	}

	twl4030->codec_muted = mute;
}

static void twl4030_power_up(struct snd_soc_codec *codec)
{
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 anamicl, regmisc1, byte;
	int i = 0;

	if (twl4030->codec_powered)
		return;

	
	twl4030_codec_enable(codec, 1);

	
	anamicl = twl4030_read_reg_cache(codec, TWL4030_REG_ANAMICL);
	twl4030_write(codec, TWL4030_REG_ANAMICL,
		anamicl | TWL4030_CNCL_OFFSET_START);

	
	do {
		
		udelay(2000);
		twl4030_i2c_read_u8(TWL4030_MODULE_AUDIO_VOICE, &byte,
				    TWL4030_REG_ANAMICL);
	} while ((i++ < 100) &&
		 ((byte & TWL4030_CNCL_OFFSET_START) ==
		  TWL4030_CNCL_OFFSET_START));

	
	twl4030_write_reg_cache(codec, TWL4030_REG_ANAMICL, byte);

	
	regmisc1 = twl4030_read_reg_cache(codec, TWL4030_REG_MISC_SET_1);
	twl4030_write(codec, TWL4030_REG_MISC_SET_1,
		regmisc1 | TWL4030_SMOOTH_ANAVOL_EN);

	
	twl4030_codec_enable(codec, 0);
	twl4030_codec_enable(codec, 1);
}


static void twl4030_power_down(struct snd_soc_codec *codec)
{
	
	twl4030_codec_enable(codec, 0);
}


static const struct snd_kcontrol_new twl4030_dapm_earpiece_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_EAR_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_EAR_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_EAR_CTL, 2, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_EAR_CTL, 3, 1, 0),
};


static const struct snd_kcontrol_new twl4030_dapm_predrivel_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PREDL_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_PREDL_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_PREDL_CTL, 2, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_PREDL_CTL, 3, 1, 0),
};


static const struct snd_kcontrol_new twl4030_dapm_predriver_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PREDR_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_PREDR_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_PREDR_CTL, 2, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_PREDR_CTL, 3, 1, 0),
};


static const struct snd_kcontrol_new twl4030_dapm_hsol_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_HS_SEL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_HS_SEL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_HS_SEL, 2, 1, 0),
};


static const struct snd_kcontrol_new twl4030_dapm_hsor_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_HS_SEL, 3, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_HS_SEL, 4, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_HS_SEL, 5, 1, 0),
};


static const struct snd_kcontrol_new twl4030_dapm_carkitl_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PRECKL_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioL1", TWL4030_REG_PRECKL_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioL2", TWL4030_REG_PRECKL_CTL, 2, 1, 0),
};


static const struct snd_kcontrol_new twl4030_dapm_carkitr_controls[] = {
	SOC_DAPM_SINGLE("Voice", TWL4030_REG_PRECKR_CTL, 0, 1, 0),
	SOC_DAPM_SINGLE("AudioR1", TWL4030_REG_PRECKR_CTL, 1, 1, 0),
	SOC_DAPM_SINGLE("AudioR2", TWL4030_REG_PRECKR_CTL, 2, 1, 0),
};


static const char *twl4030_handsfreel_texts[] =
		{"Voice", "AudioL1", "AudioL2", "AudioR2"};

static const struct soc_enum twl4030_handsfreel_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_HFL_CTL, 0,
			ARRAY_SIZE(twl4030_handsfreel_texts),
			twl4030_handsfreel_texts);

static const struct snd_kcontrol_new twl4030_dapm_handsfreel_control =
SOC_DAPM_ENUM("Route", twl4030_handsfreel_enum);


static const struct snd_kcontrol_new twl4030_dapm_handsfreelmute_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_SW_SHADOW, 0, 1, 0);


static const char *twl4030_handsfreer_texts[] =
		{"Voice", "AudioR1", "AudioR2", "AudioL2"};

static const struct soc_enum twl4030_handsfreer_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_HFR_CTL, 0,
			ARRAY_SIZE(twl4030_handsfreer_texts),
			twl4030_handsfreer_texts);

static const struct snd_kcontrol_new twl4030_dapm_handsfreer_control =
SOC_DAPM_ENUM("Route", twl4030_handsfreer_enum);


static const struct snd_kcontrol_new twl4030_dapm_handsfreermute_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_SW_SHADOW, 1, 1, 0);



static const char *twl4030_vibra_texts[] =
		{"AudioL1", "AudioR1", "AudioL2", "AudioR2"};

static const struct soc_enum twl4030_vibra_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 2,
			ARRAY_SIZE(twl4030_vibra_texts),
			twl4030_vibra_texts);

static const struct snd_kcontrol_new twl4030_dapm_vibra_control =
SOC_DAPM_ENUM("Route", twl4030_vibra_enum);


static const char *twl4030_vibrapath_texts[] =
		{"Local vibrator", "Audio"};

static const struct soc_enum twl4030_vibrapath_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 4,
			ARRAY_SIZE(twl4030_vibrapath_texts),
			twl4030_vibrapath_texts);

static const struct snd_kcontrol_new twl4030_dapm_vibrapath_control =
SOC_DAPM_ENUM("Route", twl4030_vibrapath_enum);


static const struct snd_kcontrol_new twl4030_dapm_analoglmic_controls[] = {
	SOC_DAPM_SINGLE("Main Mic Capture Switch",
			TWL4030_REG_ANAMICL, 0, 1, 0),
	SOC_DAPM_SINGLE("Headset Mic Capture Switch",
			TWL4030_REG_ANAMICL, 1, 1, 0),
	SOC_DAPM_SINGLE("AUXL Capture Switch",
			TWL4030_REG_ANAMICL, 2, 1, 0),
	SOC_DAPM_SINGLE("Carkit Mic Capture Switch",
			TWL4030_REG_ANAMICL, 3, 1, 0),
};


static const struct snd_kcontrol_new twl4030_dapm_analogrmic_controls[] = {
	SOC_DAPM_SINGLE("Sub Mic Capture Switch", TWL4030_REG_ANAMICR, 0, 1, 0),
	SOC_DAPM_SINGLE("AUXR Capture Switch", TWL4030_REG_ANAMICR, 2, 1, 0),
};


static const char *twl4030_micpathtx1_texts[] =
		{"Analog", "Digimic0"};

static const struct soc_enum twl4030_micpathtx1_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_ADCMICSEL, 0,
			ARRAY_SIZE(twl4030_micpathtx1_texts),
			twl4030_micpathtx1_texts);

static const struct snd_kcontrol_new twl4030_dapm_micpathtx1_control =
SOC_DAPM_ENUM("Route", twl4030_micpathtx1_enum);


static const char *twl4030_micpathtx2_texts[] =
		{"Analog", "Digimic1"};

static const struct soc_enum twl4030_micpathtx2_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_ADCMICSEL, 2,
			ARRAY_SIZE(twl4030_micpathtx2_texts),
			twl4030_micpathtx2_texts);

static const struct snd_kcontrol_new twl4030_dapm_micpathtx2_control =
SOC_DAPM_ENUM("Route", twl4030_micpathtx2_enum);


static const struct snd_kcontrol_new twl4030_dapm_abypassr1_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXR1_APGA_CTL, 2, 1, 0);


static const struct snd_kcontrol_new twl4030_dapm_abypassl1_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXL1_APGA_CTL, 2, 1, 0);


static const struct snd_kcontrol_new twl4030_dapm_abypassr2_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXR2_APGA_CTL, 2, 1, 0);


static const struct snd_kcontrol_new twl4030_dapm_abypassl2_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_ARXL2_APGA_CTL, 2, 1, 0);


static const struct snd_kcontrol_new twl4030_dapm_abypassv_control =
	SOC_DAPM_SINGLE("Switch", TWL4030_REG_VDL_APGA_CTL, 2, 1, 0);


static const unsigned int twl4030_dapm_dbypass_tlv[] = {
	TLV_DB_RANGE_HEAD(2),
	0, 3, TLV_DB_SCALE_ITEM(-2400, 0, 1),
	4, 7, TLV_DB_SCALE_ITEM(-1800, 600, 0),
};


static const struct snd_kcontrol_new twl4030_dapm_dbypassl_control =
	SOC_DAPM_SINGLE_TLV("Volume",
			TWL4030_REG_ATX2ARXPGA, 3, 7, 0,
			twl4030_dapm_dbypass_tlv);


static const struct snd_kcontrol_new twl4030_dapm_dbypassr_control =
	SOC_DAPM_SINGLE_TLV("Volume",
			TWL4030_REG_ATX2ARXPGA, 0, 7, 0,
			twl4030_dapm_dbypass_tlv);


static DECLARE_TLV_DB_SCALE(twl4030_dapm_dbypassv_tlv, -5100, 100, 1);


static const struct snd_kcontrol_new twl4030_dapm_dbypassv_control =
	SOC_DAPM_SINGLE_TLV("Volume",
			TWL4030_REG_VSTPGA, 0, 0x29, 0,
			twl4030_dapm_dbypassv_tlv);

static int micpath_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct soc_enum *e = (struct soc_enum *)w->kcontrols->private_value;
	unsigned char adcmicsel, micbias_ctl;

	adcmicsel = twl4030_read_reg_cache(w->codec, TWL4030_REG_ADCMICSEL);
	micbias_ctl = twl4030_read_reg_cache(w->codec, TWL4030_REG_MICBIAS_CTL);
	
	if (e->shift_l) {
		
		if (adcmicsel & TWL4030_TX2IN_SEL)
			micbias_ctl |= TWL4030_MICBIAS2_CTL; 
		else
			micbias_ctl &= ~TWL4030_MICBIAS2_CTL;
	} else {
		
		if (adcmicsel & TWL4030_TX1IN_SEL)
			micbias_ctl |= TWL4030_MICBIAS1_CTL; 
		else
			micbias_ctl &= ~TWL4030_MICBIAS1_CTL;
	}

	twl4030_write(w->codec, TWL4030_REG_MICBIAS_CTL, micbias_ctl);

	return 0;
}


#define TWL4030_OUTPUT_PGA(pin_name, reg, mask)				\
static int pin_name##pga_event(struct snd_soc_dapm_widget *w,		\
		struct snd_kcontrol *kcontrol, int event)		\
{									\
	u8 reg_val;							\
									\
	switch (event) {						\
	case SND_SOC_DAPM_POST_PMU:					\
		twl4030_write(w->codec, reg,				\
			twl4030_read_reg_cache(w->codec, reg));		\
		break;							\
	case SND_SOC_DAPM_POST_PMD:					\
		reg_val = twl4030_read_reg_cache(w->codec, reg);	\
		twl4030_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,	\
					reg_val & (~mask),		\
					reg);				\
		break;							\
	}								\
	return 0;							\
}

TWL4030_OUTPUT_PGA(earpiece, TWL4030_REG_EAR_CTL, TWL4030_EAR_GAIN);
TWL4030_OUTPUT_PGA(predrivel, TWL4030_REG_PREDL_CTL, TWL4030_PREDL_GAIN);
TWL4030_OUTPUT_PGA(predriver, TWL4030_REG_PREDR_CTL, TWL4030_PREDR_GAIN);
TWL4030_OUTPUT_PGA(carkitl, TWL4030_REG_PRECKL_CTL, TWL4030_PRECKL_GAIN);
TWL4030_OUTPUT_PGA(carkitr, TWL4030_REG_PRECKR_CTL, TWL4030_PRECKR_GAIN);

static void handsfree_ramp(struct snd_soc_codec *codec, int reg, int ramp)
{
	unsigned char hs_ctl;

	hs_ctl = twl4030_read_reg_cache(codec, reg);

	if (ramp) {
		
		hs_ctl |= TWL4030_HF_CTL_REF_EN;
		twl4030_write(codec, reg, hs_ctl);
		udelay(10);
		hs_ctl |= TWL4030_HF_CTL_RAMP_EN;
		twl4030_write(codec, reg, hs_ctl);
		udelay(40);
		hs_ctl |= TWL4030_HF_CTL_LOOP_EN;
		hs_ctl |= TWL4030_HF_CTL_HB_EN;
		twl4030_write(codec, reg, hs_ctl);
	} else {
		
		hs_ctl &= ~TWL4030_HF_CTL_LOOP_EN;
		hs_ctl &= ~TWL4030_HF_CTL_HB_EN;
		twl4030_write(codec, reg, hs_ctl);
		hs_ctl &= ~TWL4030_HF_CTL_RAMP_EN;
		twl4030_write(codec, reg, hs_ctl);
		udelay(40);
		hs_ctl &= ~TWL4030_HF_CTL_REF_EN;
		twl4030_write(codec, reg, hs_ctl);
	}
}

static int handsfreelpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		handsfree_ramp(w->codec, TWL4030_REG_HFL_CTL, 1);
		break;
	case SND_SOC_DAPM_POST_PMD:
		handsfree_ramp(w->codec, TWL4030_REG_HFL_CTL, 0);
		break;
	}
	return 0;
}

static int handsfreerpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		handsfree_ramp(w->codec, TWL4030_REG_HFR_CTL, 1);
		break;
	case SND_SOC_DAPM_POST_PMD:
		handsfree_ramp(w->codec, TWL4030_REG_HFR_CTL, 0);
		break;
	}
	return 0;
}

static void headset_ramp(struct snd_soc_codec *codec, int ramp)
{
	struct snd_soc_device *socdev = codec->socdev;
	struct twl4030_setup_data *setup = socdev->codec_data;

	unsigned char hs_gain, hs_pop;
	struct twl4030_priv *twl4030 = codec->private_data;
	
	unsigned int ramp_base[] = {524288, 1048576, 2097152, 4194304,
				    8388608, 16777216, 33554432, 67108864};

	hs_gain = twl4030_read_reg_cache(codec, TWL4030_REG_HS_GAIN_SET);
	hs_pop = twl4030_read_reg_cache(codec, TWL4030_REG_HS_POPN_SET);

	
	if (setup && setup->hs_extmute) {
		if (setup->set_hs_extmute) {
			setup->set_hs_extmute(1);
		} else {
			hs_pop |= TWL4030_EXTMUTE;
			twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		}
	}

	if (ramp) {
		
		hs_pop |= TWL4030_VMID_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		twl4030_write(codec, TWL4030_REG_HS_GAIN_SET, hs_gain);
		hs_pop |= TWL4030_RAMP_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		
		mdelay((ramp_base[(hs_pop & TWL4030_RAMP_DELAY) >> 2] /
			twl4030->sysclk) + 1);
	} else {
		
		hs_pop &= ~TWL4030_RAMP_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		
		mdelay((ramp_base[(hs_pop & TWL4030_RAMP_DELAY) >> 2] /
			twl4030->sysclk) + 1);
		
		twl4030_i2c_write_u8(TWL4030_MODULE_AUDIO_VOICE,
					hs_gain & (~0x0f),
					TWL4030_REG_HS_GAIN_SET);

		hs_pop &= ~TWL4030_VMID_EN;
		twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
	}

	
	if (setup && setup->hs_extmute) {
		if (setup->set_hs_extmute) {
			setup->set_hs_extmute(0);
		} else {
			hs_pop &= ~TWL4030_EXTMUTE;
			twl4030_write(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
		}
	}
}

static int headsetlpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct twl4030_priv *twl4030 = w->codec->private_data;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		
		if (!twl4030->hsr_enabled)
			headset_ramp(w->codec, 1);

		twl4030->hsl_enabled = 1;
		break;
	case SND_SOC_DAPM_POST_PMD:
		
		if (!twl4030->hsr_enabled)
			headset_ramp(w->codec, 0);

		twl4030->hsl_enabled = 0;
		break;
	}
	return 0;
}

static int headsetrpga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct twl4030_priv *twl4030 = w->codec->private_data;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		
		if (!twl4030->hsl_enabled)
			headset_ramp(w->codec, 1);

		twl4030->hsr_enabled = 1;
		break;
	case SND_SOC_DAPM_POST_PMD:
		
		if (!twl4030->hsl_enabled)
			headset_ramp(w->codec, 0);

		twl4030->hsr_enabled = 0;
		break;
	}
	return 0;
}

static int bypass_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct soc_mixer_control *m =
		(struct soc_mixer_control *)w->kcontrols->private_value;
	struct twl4030_priv *twl4030 = w->codec->private_data;
	unsigned char reg, misc;

	reg = twl4030_read_reg_cache(w->codec, m->reg);

	
	if (m->reg == TWL4030_REG_VSTPGA) {
		
		if (reg)
			twl4030->bypass_state |= (1 << 5);
		else
			twl4030->bypass_state &= ~(1 << 5);
	} else if (m->reg <= TWL4030_REG_ARXR2_APGA_CTL) {
		
		if (reg & (1 << m->shift))
			twl4030->bypass_state |=
				(1 << (m->reg - TWL4030_REG_ARXL1_APGA_CTL));
		else
			twl4030->bypass_state &=
				~(1 << (m->reg - TWL4030_REG_ARXL1_APGA_CTL));
	} else if (m->reg == TWL4030_REG_VDL_APGA_CTL) {
		
		if (reg & (1 << m->shift))
			twl4030->bypass_state |= (1 << 4);
		else
			twl4030->bypass_state &= ~(1 << 4);
	} else {
		
		if (reg & (0x7 << m->shift))
			twl4030->bypass_state |= (1 << (m->shift ? 7 : 6));
		else
			twl4030->bypass_state &= ~(1 << (m->shift ? 7 : 6));
	}

	
	misc = twl4030_read_reg_cache(w->codec, TWL4030_REG_MISC_SET_1);
	if (twl4030->bypass_state & 0x1F)
		misc |= TWL4030_FMLOOP_EN;
	else
		misc &= ~TWL4030_FMLOOP_EN;
	twl4030_write(w->codec, TWL4030_REG_MISC_SET_1, misc);

	if (w->codec->bias_level == SND_SOC_BIAS_STANDBY) {
		if (twl4030->bypass_state)
			twl4030_codec_mute(w->codec, 0);
		else
			twl4030_codec_mute(w->codec, 1);
	}
	return 0;
}


#define SOC_DOUBLE_TLV_TWL4030(xname, xreg, shift_left, shift_right, xmax,\
			       xinvert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = (xname),\
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_volsw, \
	.get = snd_soc_get_volsw_twl4030, \
	.put = snd_soc_put_volsw_twl4030, \
	.private_value = (unsigned long)&(struct soc_mixer_control) \
		{.reg = xreg, .shift = shift_left, .rshift = shift_right,\
		 .max = xmax, .invert = xinvert} }
#define SOC_DOUBLE_R_TLV_TWL4030(xname, reg_left, reg_right, xshift, xmax,\
				 xinvert, tlv_array) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = (xname),\
	.access = SNDRV_CTL_ELEM_ACCESS_TLV_READ |\
		 SNDRV_CTL_ELEM_ACCESS_READWRITE,\
	.tlv.p = (tlv_array), \
	.info = snd_soc_info_volsw_2r, \
	.get = snd_soc_get_volsw_r2_twl4030,\
	.put = snd_soc_put_volsw_r2_twl4030, \
	.private_value = (unsigned long)&(struct soc_mixer_control) \
		{.reg = reg_left, .rreg = reg_right, .shift = xshift, \
		 .rshift = xshift, .max = xmax, .invert = xinvert} }
#define SOC_SINGLE_TLV_TWL4030(xname, xreg, xshift, xmax, xinvert, tlv_array) \
	SOC_DOUBLE_TLV_TWL4030(xname, xreg, xshift, xshift, xmax, \
			       xinvert, tlv_array)

static int snd_soc_get_volsw_twl4030(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int shift = mc->shift;
	unsigned int rshift = mc->rshift;
	int max = mc->max;
	int mask = (1 << fls(max)) - 1;

	ucontrol->value.integer.value[0] =
		(snd_soc_read(codec, reg) >> shift) & mask;
	if (ucontrol->value.integer.value[0])
		ucontrol->value.integer.value[0] =
			max + 1 - ucontrol->value.integer.value[0];

	if (shift != rshift) {
		ucontrol->value.integer.value[1] =
			(snd_soc_read(codec, reg) >> rshift) & mask;
		if (ucontrol->value.integer.value[1])
			ucontrol->value.integer.value[1] =
				max + 1 - ucontrol->value.integer.value[1];
	}

	return 0;
}

static int snd_soc_put_volsw_twl4030(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int shift = mc->shift;
	unsigned int rshift = mc->rshift;
	int max = mc->max;
	int mask = (1 << fls(max)) - 1;
	unsigned short val, val2, val_mask;

	val = (ucontrol->value.integer.value[0] & mask);

	val_mask = mask << shift;
	if (val)
		val = max + 1 - val;
	val = val << shift;
	if (shift != rshift) {
		val2 = (ucontrol->value.integer.value[1] & mask);
		val_mask |= mask << rshift;
		if (val2)
			val2 = max + 1 - val2;
		val |= val2 << rshift;
	}
	return snd_soc_update_bits(codec, reg, val_mask, val);
}

static int snd_soc_get_volsw_r2_twl4030(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int reg2 = mc->rreg;
	unsigned int shift = mc->shift;
	int max = mc->max;
	int mask = (1<<fls(max))-1;

	ucontrol->value.integer.value[0] =
		(snd_soc_read(codec, reg) >> shift) & mask;
	ucontrol->value.integer.value[1] =
		(snd_soc_read(codec, reg2) >> shift) & mask;

	if (ucontrol->value.integer.value[0])
		ucontrol->value.integer.value[0] =
			max + 1 - ucontrol->value.integer.value[0];
	if (ucontrol->value.integer.value[1])
		ucontrol->value.integer.value[1] =
			max + 1 - ucontrol->value.integer.value[1];

	return 0;
}

static int snd_soc_put_volsw_r2_twl4030(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	unsigned int reg = mc->reg;
	unsigned int reg2 = mc->rreg;
	unsigned int shift = mc->shift;
	int max = mc->max;
	int mask = (1 << fls(max)) - 1;
	int err;
	unsigned short val, val2, val_mask;

	val_mask = mask << shift;
	val = (ucontrol->value.integer.value[0] & mask);
	val2 = (ucontrol->value.integer.value[1] & mask);

	if (val)
		val = max + 1 - val;
	if (val2)
		val2 = max + 1 - val2;

	val = val << shift;
	val2 = val2 << shift;

	err = snd_soc_update_bits(codec, reg, val_mask, val);
	if (err < 0)
		return err;

	err = snd_soc_update_bits(codec, reg2, val_mask, val2);
	return err;
}


static const char *twl4030_op_modes_texts[] = {
	"Option 2 (voice/audio)", "Option 1 (audio)"
};

static const struct soc_enum twl4030_op_modes_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_CODEC_MODE, 0,
			ARRAY_SIZE(twl4030_op_modes_texts),
			twl4030_op_modes_texts);

static int snd_soc_put_twl4030_opmode_enum_double(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct twl4030_priv *twl4030 = codec->private_data;
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	unsigned short val;
	unsigned short mask, bitmask;

	if (twl4030->configured) {
		printk(KERN_ERR "twl4030 operation mode cannot be "
			"changed on-the-fly\n");
		return -EBUSY;
	}

	for (bitmask = 1; bitmask < e->max; bitmask <<= 1)
		;
	if (ucontrol->value.enumerated.item[0] > e->max - 1)
		return -EINVAL;

	val = ucontrol->value.enumerated.item[0] << e->shift_l;
	mask = (bitmask - 1) << e->shift_l;
	if (e->shift_l != e->shift_r) {
		if (ucontrol->value.enumerated.item[1] > e->max - 1)
			return -EINVAL;
		val |= ucontrol->value.enumerated.item[1] << e->shift_r;
		mask |= (bitmask - 1) << e->shift_r;
	}

	return snd_soc_update_bits(codec, e->reg, mask, val);
}


static DECLARE_TLV_DB_SCALE(digital_fine_tlv, -6300, 100, 1);


static DECLARE_TLV_DB_SCALE(digital_coarse_tlv, 0, 600, 0);


static DECLARE_TLV_DB_SCALE(digital_voice_downlink_tlv, -3700, 100, 1);


static DECLARE_TLV_DB_SCALE(analog_tlv, -2400, 200, 0);


static DECLARE_TLV_DB_SCALE(output_tvl, -1200, 600, 1);


static DECLARE_TLV_DB_SCALE(output_ear_tvl, -600, 600, 1);


static DECLARE_TLV_DB_SCALE(digital_capture_tlv, 0, 100, 0);


static DECLARE_TLV_DB_SCALE(input_gain_tlv, 0, 600, 0);


static const char *twl4030_avadc_clk_priority_texts[] = {
	"Voice high priority", "HiFi high priority"
};

static const struct soc_enum twl4030_avadc_clk_priority_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_AVADC_CTL, 2,
			ARRAY_SIZE(twl4030_avadc_clk_priority_texts),
			twl4030_avadc_clk_priority_texts);

static const char *twl4030_rampdelay_texts[] = {
	"27/20/14 ms", "55/40/27 ms", "109/81/55 ms", "218/161/109 ms",
	"437/323/218 ms", "874/645/437 ms", "1748/1291/874 ms",
	"3495/2581/1748 ms"
};

static const struct soc_enum twl4030_rampdelay_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_HS_POPN_SET, 2,
			ARRAY_SIZE(twl4030_rampdelay_texts),
			twl4030_rampdelay_texts);


static const char *twl4030_vibradirmode_texts[] = {
	"Vibra H-bridge direction", "Audio data MSB",
};

static const struct soc_enum twl4030_vibradirmode_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 5,
			ARRAY_SIZE(twl4030_vibradirmode_texts),
			twl4030_vibradirmode_texts);


static const char *twl4030_vibradir_texts[] = {
	"Positive polarity", "Negative polarity",
};

static const struct soc_enum twl4030_vibradir_enum =
	SOC_ENUM_SINGLE(TWL4030_REG_VIBRA_CTL, 1,
			ARRAY_SIZE(twl4030_vibradir_texts),
			twl4030_vibradir_texts);

static const struct snd_kcontrol_new twl4030_snd_controls[] = {
	
	SOC_ENUM_EXT("Codec Operation Mode", twl4030_op_modes_enum,
		snd_soc_get_enum_double,
		snd_soc_put_twl4030_opmode_enum_double),

	
	SOC_DOUBLE_R_TLV("DAC1 Digital Fine Playback Volume",
		TWL4030_REG_ARXL1PGA, TWL4030_REG_ARXR1PGA,
		0, 0x3f, 0, digital_fine_tlv),
	SOC_DOUBLE_R_TLV("DAC2 Digital Fine Playback Volume",
		TWL4030_REG_ARXL2PGA, TWL4030_REG_ARXR2PGA,
		0, 0x3f, 0, digital_fine_tlv),

	SOC_DOUBLE_R_TLV("DAC1 Digital Coarse Playback Volume",
		TWL4030_REG_ARXL1PGA, TWL4030_REG_ARXR1PGA,
		6, 0x2, 0, digital_coarse_tlv),
	SOC_DOUBLE_R_TLV("DAC2 Digital Coarse Playback Volume",
		TWL4030_REG_ARXL2PGA, TWL4030_REG_ARXR2PGA,
		6, 0x2, 0, digital_coarse_tlv),

	SOC_DOUBLE_R_TLV("DAC1 Analog Playback Volume",
		TWL4030_REG_ARXL1_APGA_CTL, TWL4030_REG_ARXR1_APGA_CTL,
		3, 0x12, 1, analog_tlv),
	SOC_DOUBLE_R_TLV("DAC2 Analog Playback Volume",
		TWL4030_REG_ARXL2_APGA_CTL, TWL4030_REG_ARXR2_APGA_CTL,
		3, 0x12, 1, analog_tlv),
	SOC_DOUBLE_R("DAC1 Analog Playback Switch",
		TWL4030_REG_ARXL1_APGA_CTL, TWL4030_REG_ARXR1_APGA_CTL,
		1, 1, 0),
	SOC_DOUBLE_R("DAC2 Analog Playback Switch",
		TWL4030_REG_ARXL2_APGA_CTL, TWL4030_REG_ARXR2_APGA_CTL,
		1, 1, 0),

	
	SOC_SINGLE_TLV("DAC Voice Digital Downlink Volume",
		TWL4030_REG_VRXPGA, 0, 0x31, 0, digital_voice_downlink_tlv),

	SOC_SINGLE_TLV("DAC Voice Analog Downlink Volume",
		TWL4030_REG_VDL_APGA_CTL, 3, 0x12, 1, analog_tlv),

	SOC_SINGLE("DAC Voice Analog Downlink Switch",
		TWL4030_REG_VDL_APGA_CTL, 1, 1, 0),

	
	SOC_DOUBLE_R_TLV_TWL4030("PreDriv Playback Volume",
		TWL4030_REG_PREDL_CTL, TWL4030_REG_PREDR_CTL,
		4, 3, 0, output_tvl),

	SOC_DOUBLE_TLV_TWL4030("Headset Playback Volume",
		TWL4030_REG_HS_GAIN_SET, 0, 2, 3, 0, output_tvl),

	SOC_DOUBLE_R_TLV_TWL4030("Carkit Playback Volume",
		TWL4030_REG_PRECKL_CTL, TWL4030_REG_PRECKR_CTL,
		4, 3, 0, output_tvl),

	SOC_SINGLE_TLV_TWL4030("Earpiece Playback Volume",
		TWL4030_REG_EAR_CTL, 4, 3, 0, output_ear_tvl),

	
	SOC_DOUBLE_R_TLV("TX1 Digital Capture Volume",
		TWL4030_REG_ATXL1PGA, TWL4030_REG_ATXR1PGA,
		0, 0x1f, 0, digital_capture_tlv),
	SOC_DOUBLE_R_TLV("TX2 Digital Capture Volume",
		TWL4030_REG_AVTXL2PGA, TWL4030_REG_AVTXR2PGA,
		0, 0x1f, 0, digital_capture_tlv),

	SOC_DOUBLE_TLV("Analog Capture Volume", TWL4030_REG_ANAMIC_GAIN,
		0, 3, 5, 0, input_gain_tlv),

	SOC_ENUM("AVADC Clock Priority", twl4030_avadc_clk_priority_enum),

	SOC_ENUM("HS ramp delay", twl4030_rampdelay_enum),

	SOC_ENUM("Vibra H-bridge mode", twl4030_vibradirmode_enum),
	SOC_ENUM("Vibra H-bridge direction", twl4030_vibradir_enum),
};

static const struct snd_soc_dapm_widget twl4030_dapm_widgets[] = {
	
	SND_SOC_DAPM_INPUT("MAINMIC"),
	SND_SOC_DAPM_INPUT("HSMIC"),
	SND_SOC_DAPM_INPUT("AUXL"),
	SND_SOC_DAPM_INPUT("CARKITMIC"),
	
	SND_SOC_DAPM_INPUT("SUBMIC"),
	SND_SOC_DAPM_INPUT("AUXR"),
	
	SND_SOC_DAPM_INPUT("DIGIMIC0"),
	SND_SOC_DAPM_INPUT("DIGIMIC1"),

	
	SND_SOC_DAPM_OUTPUT("OUTL"),
	SND_SOC_DAPM_OUTPUT("OUTR"),
	SND_SOC_DAPM_OUTPUT("EARPIECE"),
	SND_SOC_DAPM_OUTPUT("PREDRIVEL"),
	SND_SOC_DAPM_OUTPUT("PREDRIVER"),
	SND_SOC_DAPM_OUTPUT("HSOL"),
	SND_SOC_DAPM_OUTPUT("HSOR"),
	SND_SOC_DAPM_OUTPUT("CARKITL"),
	SND_SOC_DAPM_OUTPUT("CARKITR"),
	SND_SOC_DAPM_OUTPUT("HFL"),
	SND_SOC_DAPM_OUTPUT("HFR"),
	SND_SOC_DAPM_OUTPUT("VIBRA"),

	
	SND_SOC_DAPM_DAC("DAC Right1", "Right Front HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Left1", "Left Front HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Right2", "Right Rear HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Left2", "Left Rear HiFi Playback",
			SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_DAC("DAC Voice", "Voice Playback",
			SND_SOC_NOPM, 0, 0),

	
	SND_SOC_DAPM_SWITCH_E("Right1 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassr1_control, bypass_event,
			SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Left1 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassl1_control,
			bypass_event, SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Right2 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassr2_control,
			bypass_event, SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Left2 Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassl2_control,
			bypass_event, SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Voice Analog Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_abypassv_control,
			bypass_event, SND_SOC_DAPM_POST_REG),

	
	SND_SOC_DAPM_SWITCH_E("Left Digital Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_dbypassl_control, bypass_event,
			SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Right Digital Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_dbypassr_control, bypass_event,
			SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_SWITCH_E("Voice Digital Loopback", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_dbypassv_control, bypass_event,
			SND_SOC_DAPM_POST_REG),

	
	SND_SOC_DAPM_MIXER("Digital R1 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital L1 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 1, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital R2 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 2, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital L2 Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 3, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Digital Voice Playback Mixer",
			TWL4030_REG_AVDAC_CTL, 4, 0, NULL, 0),

	
	SND_SOC_DAPM_MIXER("Analog R1 Playback Mixer",
			TWL4030_REG_ARXR1_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog L1 Playback Mixer",
			TWL4030_REG_ARXL1_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog R2 Playback Mixer",
			TWL4030_REG_ARXR2_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog L2 Playback Mixer",
			TWL4030_REG_ARXL2_APGA_CTL, 0, 0, NULL, 0),
	SND_SOC_DAPM_MIXER("Analog Voice Playback Mixer",
			TWL4030_REG_VDL_APGA_CTL, 0, 0, NULL, 0),

	
	
	SND_SOC_DAPM_MIXER("Earpiece Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_earpiece_controls[0],
			ARRAY_SIZE(twl4030_dapm_earpiece_controls)),
	SND_SOC_DAPM_PGA_E("Earpiece PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, earpiecepga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	
	SND_SOC_DAPM_MIXER("PredriveL Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_predrivel_controls[0],
			ARRAY_SIZE(twl4030_dapm_predrivel_controls)),
	SND_SOC_DAPM_PGA_E("PredriveL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, predrivelpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER("PredriveR Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_predriver_controls[0],
			ARRAY_SIZE(twl4030_dapm_predriver_controls)),
	SND_SOC_DAPM_PGA_E("PredriveR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, predriverpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	
	SND_SOC_DAPM_MIXER("HeadsetL Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_hsol_controls[0],
			ARRAY_SIZE(twl4030_dapm_hsol_controls)),
	SND_SOC_DAPM_PGA_E("HeadsetL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, headsetlpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER("HeadsetR Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_hsor_controls[0],
			ARRAY_SIZE(twl4030_dapm_hsor_controls)),
	SND_SOC_DAPM_PGA_E("HeadsetR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, headsetrpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	
	SND_SOC_DAPM_MIXER("CarkitL Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_carkitl_controls[0],
			ARRAY_SIZE(twl4030_dapm_carkitl_controls)),
	SND_SOC_DAPM_PGA_E("CarkitL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, carkitlpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MIXER("CarkitR Mixer", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_carkitr_controls[0],
			ARRAY_SIZE(twl4030_dapm_carkitr_controls)),
	SND_SOC_DAPM_PGA_E("CarkitR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, carkitrpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),

	
	
	SND_SOC_DAPM_MUX("HandsfreeL Mux", SND_SOC_NOPM, 0, 0,
		&twl4030_dapm_handsfreel_control),
	SND_SOC_DAPM_SWITCH("HandsfreeL", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_handsfreelmute_control),
	SND_SOC_DAPM_PGA_E("HandsfreeL PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, handsfreelpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	SND_SOC_DAPM_MUX("HandsfreeR Mux", SND_SOC_NOPM, 5, 0,
		&twl4030_dapm_handsfreer_control),
	SND_SOC_DAPM_SWITCH("HandsfreeR", SND_SOC_NOPM, 0, 0,
			&twl4030_dapm_handsfreermute_control),
	SND_SOC_DAPM_PGA_E("HandsfreeR PGA", SND_SOC_NOPM,
			0, 0, NULL, 0, handsfreerpga_event,
			SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD),
	
	SND_SOC_DAPM_MUX("Vibra Mux", TWL4030_REG_VIBRA_CTL, 0, 0,
		&twl4030_dapm_vibra_control),
	SND_SOC_DAPM_MUX("Vibra Route", SND_SOC_NOPM, 0, 0,
		&twl4030_dapm_vibrapath_control),

	
	SND_SOC_DAPM_ADC("ADC Virtual Left1", "Left Front Capture",
		SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_ADC("ADC Virtual Right1", "Right Front Capture",
		SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_ADC("ADC Virtual Left2", "Left Rear Capture",
		SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_ADC("ADC Virtual Right2", "Right Rear Capture",
		SND_SOC_NOPM, 0, 0),

	
	SND_SOC_DAPM_MUX_E("TX1 Capture Route", SND_SOC_NOPM, 0, 0,
		&twl4030_dapm_micpathtx1_control, micpath_event,
		SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD|
		SND_SOC_DAPM_POST_REG),
	SND_SOC_DAPM_MUX_E("TX2 Capture Route", SND_SOC_NOPM, 0, 0,
		&twl4030_dapm_micpathtx2_control, micpath_event,
		SND_SOC_DAPM_POST_PMU|SND_SOC_DAPM_POST_PMD|
		SND_SOC_DAPM_POST_REG),

	
	SND_SOC_DAPM_MIXER("Analog Left",
		TWL4030_REG_ANAMICL, 4, 0,
		&twl4030_dapm_analoglmic_controls[0],
		ARRAY_SIZE(twl4030_dapm_analoglmic_controls)),
	SND_SOC_DAPM_MIXER("Analog Right",
		TWL4030_REG_ANAMICR, 4, 0,
		&twl4030_dapm_analogrmic_controls[0],
		ARRAY_SIZE(twl4030_dapm_analogrmic_controls)),

	SND_SOC_DAPM_PGA("ADC Physical Left",
		TWL4030_REG_AVADC_CTL, 3, 0, NULL, 0),
	SND_SOC_DAPM_PGA("ADC Physical Right",
		TWL4030_REG_AVADC_CTL, 1, 0, NULL, 0),

	SND_SOC_DAPM_PGA("Digimic0 Enable",
		TWL4030_REG_ADCMICSEL, 1, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Digimic1 Enable",
		TWL4030_REG_ADCMICSEL, 3, 0, NULL, 0),

	SND_SOC_DAPM_MICBIAS("Mic Bias 1", TWL4030_REG_MICBIAS_CTL, 0, 0),
	SND_SOC_DAPM_MICBIAS("Mic Bias 2", TWL4030_REG_MICBIAS_CTL, 1, 0),
	SND_SOC_DAPM_MICBIAS("Headset Mic Bias", TWL4030_REG_MICBIAS_CTL, 2, 0),

};

static const struct snd_soc_dapm_route intercon[] = {
	{"Digital L1 Playback Mixer", NULL, "DAC Left1"},
	{"Digital R1 Playback Mixer", NULL, "DAC Right1"},
	{"Digital L2 Playback Mixer", NULL, "DAC Left2"},
	{"Digital R2 Playback Mixer", NULL, "DAC Right2"},
	{"Digital Voice Playback Mixer", NULL, "DAC Voice"},

	{"Analog L1 Playback Mixer", NULL, "Digital L1 Playback Mixer"},
	{"Analog R1 Playback Mixer", NULL, "Digital R1 Playback Mixer"},
	{"Analog L2 Playback Mixer", NULL, "Digital L2 Playback Mixer"},
	{"Analog R2 Playback Mixer", NULL, "Digital R2 Playback Mixer"},
	{"Analog Voice Playback Mixer", NULL, "Digital Voice Playback Mixer"},

	
	
	{"Earpiece Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"Earpiece Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"Earpiece Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"Earpiece Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"Earpiece PGA", NULL, "Earpiece Mixer"},
	
	{"PredriveL Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"PredriveL Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"PredriveL Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"PredriveL Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"PredriveL PGA", NULL, "PredriveL Mixer"},
	
	{"PredriveR Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"PredriveR Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"PredriveR Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"PredriveR Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"PredriveR PGA", NULL, "PredriveR Mixer"},
	
	{"HeadsetL Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"HeadsetL Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"HeadsetL Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"HeadsetL PGA", NULL, "HeadsetL Mixer"},
	
	{"HeadsetR Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"HeadsetR Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"HeadsetR Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"HeadsetR PGA", NULL, "HeadsetR Mixer"},
	
	{"CarkitL Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"CarkitL Mixer", "AudioL1", "Analog L1 Playback Mixer"},
	{"CarkitL Mixer", "AudioL2", "Analog L2 Playback Mixer"},
	{"CarkitL PGA", NULL, "CarkitL Mixer"},
	
	{"CarkitR Mixer", "Voice", "Analog Voice Playback Mixer"},
	{"CarkitR Mixer", "AudioR1", "Analog R1 Playback Mixer"},
	{"CarkitR Mixer", "AudioR2", "Analog R2 Playback Mixer"},
	{"CarkitR PGA", NULL, "CarkitR Mixer"},
	
	{"HandsfreeL Mux", "Voice", "Analog Voice Playback Mixer"},
	{"HandsfreeL Mux", "AudioL1", "Analog L1 Playback Mixer"},
	{"HandsfreeL Mux", "AudioL2", "Analog L2 Playback Mixer"},
	{"HandsfreeL Mux", "AudioR2", "Analog R2 Playback Mixer"},
	{"HandsfreeL", "Switch", "HandsfreeL Mux"},
	{"HandsfreeL PGA", NULL, "HandsfreeL"},
	
	{"HandsfreeR Mux", "Voice", "Analog Voice Playback Mixer"},
	{"HandsfreeR Mux", "AudioR1", "Analog R1 Playback Mixer"},
	{"HandsfreeR Mux", "AudioR2", "Analog R2 Playback Mixer"},
	{"HandsfreeR Mux", "AudioL2", "Analog L2 Playback Mixer"},
	{"HandsfreeR", "Switch", "HandsfreeR Mux"},
	{"HandsfreeR PGA", NULL, "HandsfreeR"},
	
	{"Vibra Mux", "AudioL1", "DAC Left1"},
	{"Vibra Mux", "AudioR1", "DAC Right1"},
	{"Vibra Mux", "AudioL2", "DAC Left2"},
	{"Vibra Mux", "AudioR2", "DAC Right2"},

	
	{"OUTL", NULL, "Analog L2 Playback Mixer"},
	{"OUTR", NULL, "Analog R2 Playback Mixer"},
	{"EARPIECE", NULL, "Earpiece PGA"},
	{"PREDRIVEL", NULL, "PredriveL PGA"},
	{"PREDRIVER", NULL, "PredriveR PGA"},
	{"HSOL", NULL, "HeadsetL PGA"},
	{"HSOR", NULL, "HeadsetR PGA"},
	{"CARKITL", NULL, "CarkitL PGA"},
	{"CARKITR", NULL, "CarkitR PGA"},
	{"HFL", NULL, "HandsfreeL PGA"},
	{"HFR", NULL, "HandsfreeR PGA"},
	{"Vibra Route", "Audio", "Vibra Mux"},
	{"VIBRA", NULL, "Vibra Route"},

	
	{"Analog Left", "Main Mic Capture Switch", "MAINMIC"},
	{"Analog Left", "Headset Mic Capture Switch", "HSMIC"},
	{"Analog Left", "AUXL Capture Switch", "AUXL"},
	{"Analog Left", "Carkit Mic Capture Switch", "CARKITMIC"},

	{"Analog Right", "Sub Mic Capture Switch", "SUBMIC"},
	{"Analog Right", "AUXR Capture Switch", "AUXR"},

	{"ADC Physical Left", NULL, "Analog Left"},
	{"ADC Physical Right", NULL, "Analog Right"},

	{"Digimic0 Enable", NULL, "DIGIMIC0"},
	{"Digimic1 Enable", NULL, "DIGIMIC1"},

	
	{"TX1 Capture Route", "Analog", "ADC Physical Left"},
	{"TX1 Capture Route", "Digimic0", "Digimic0 Enable"},
	
	{"TX1 Capture Route", "Analog", "ADC Physical Right"},
	{"TX1 Capture Route", "Digimic0", "Digimic0 Enable"},
	
	{"TX2 Capture Route", "Analog", "ADC Physical Left"},
	{"TX2 Capture Route", "Digimic1", "Digimic1 Enable"},
	
	{"TX2 Capture Route", "Analog", "ADC Physical Right"},
	{"TX2 Capture Route", "Digimic1", "Digimic1 Enable"},

	{"ADC Virtual Left1", NULL, "TX1 Capture Route"},
	{"ADC Virtual Right1", NULL, "TX1 Capture Route"},
	{"ADC Virtual Left2", NULL, "TX2 Capture Route"},
	{"ADC Virtual Right2", NULL, "TX2 Capture Route"},

	
	{"Right1 Analog Loopback", "Switch", "Analog Right"},
	{"Left1 Analog Loopback", "Switch", "Analog Left"},
	{"Right2 Analog Loopback", "Switch", "Analog Right"},
	{"Left2 Analog Loopback", "Switch", "Analog Left"},
	{"Voice Analog Loopback", "Switch", "Analog Left"},

	{"Analog R1 Playback Mixer", NULL, "Right1 Analog Loopback"},
	{"Analog L1 Playback Mixer", NULL, "Left1 Analog Loopback"},
	{"Analog R2 Playback Mixer", NULL, "Right2 Analog Loopback"},
	{"Analog L2 Playback Mixer", NULL, "Left2 Analog Loopback"},
	{"Analog Voice Playback Mixer", NULL, "Voice Analog Loopback"},

	
	{"Right Digital Loopback", "Volume", "TX1 Capture Route"},
	{"Left Digital Loopback", "Volume", "TX1 Capture Route"},
	{"Voice Digital Loopback", "Volume", "TX2 Capture Route"},

	{"Digital R2 Playback Mixer", NULL, "Right Digital Loopback"},
	{"Digital L2 Playback Mixer", NULL, "Left Digital Loopback"},
	{"Digital Voice Playback Mixer", NULL, "Voice Digital Loopback"},

};

static int twl4030_add_widgets(struct snd_soc_codec *codec)
{
	snd_soc_dapm_new_controls(codec, twl4030_dapm_widgets,
				 ARRAY_SIZE(twl4030_dapm_widgets));

	snd_soc_dapm_add_routes(codec, intercon, ARRAY_SIZE(intercon));

	snd_soc_dapm_new_widgets(codec);
	return 0;
}

static int twl4030_set_bias_level(struct snd_soc_codec *codec,
				  enum snd_soc_bias_level level)
{
	struct twl4030_priv *twl4030 = codec->private_data;

	switch (level) {
	case SND_SOC_BIAS_ON:
		twl4030_codec_mute(codec, 0);
		break;
	case SND_SOC_BIAS_PREPARE:
		twl4030_power_up(codec);
		if (twl4030->bypass_state)
			twl4030_codec_mute(codec, 0);
		else
			twl4030_codec_mute(codec, 1);
		break;
	case SND_SOC_BIAS_STANDBY:
		twl4030_power_up(codec);
		if (twl4030->bypass_state)
			twl4030_codec_mute(codec, 0);
		else
			twl4030_codec_mute(codec, 1);
		break;
	case SND_SOC_BIAS_OFF:
		twl4030_power_down(codec);
		break;
	}
	codec->bias_level = level;

	return 0;
}

static void twl4030_constraints(struct twl4030_priv *twl4030,
				struct snd_pcm_substream *mst_substream)
{
	struct snd_pcm_substream *slv_substream;

	
	if (mst_substream == twl4030->master_substream)
		slv_substream = twl4030->slave_substream;
	else if (mst_substream == twl4030->slave_substream)
		slv_substream = twl4030->master_substream;
	else 
		return;

	
	snd_pcm_hw_constraint_minmax(slv_substream->runtime,
				SNDRV_PCM_HW_PARAM_RATE,
				twl4030->rate,
				twl4030->rate);

	snd_pcm_hw_constraint_minmax(slv_substream->runtime,
				SNDRV_PCM_HW_PARAM_SAMPLE_BITS,
				twl4030->sample_bits,
				twl4030->sample_bits);

	snd_pcm_hw_constraint_minmax(slv_substream->runtime,
				SNDRV_PCM_HW_PARAM_CHANNELS,
				twl4030->channels,
				twl4030->channels);
}


static void twl4030_tdm_enable(struct snd_soc_codec *codec, int direction,
				int enable)
{
	u8 reg, mask;

	reg = twl4030_read_reg_cache(codec, TWL4030_REG_OPTION);

	if (direction == SNDRV_PCM_STREAM_PLAYBACK)
		mask = TWL4030_ARXL1_VRX_EN | TWL4030_ARXR1_EN;
	else
		mask = TWL4030_ATXL2_VTXL_EN | TWL4030_ATXR2_VTXR_EN;

	if (enable)
		reg |= mask;
	else
		reg &= ~mask;

	twl4030_write(codec, TWL4030_REG_OPTION, reg);
}

static int twl4030_startup(struct snd_pcm_substream *substream,
			   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;

	if (twl4030->master_substream) {
		twl4030->slave_substream = substream;
		
		if (twl4030->configured)
			twl4030_constraints(twl4030, twl4030->master_substream);
	} else {
		if (!(twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE) &
			TWL4030_OPTION_1)) {
			
			snd_pcm_hw_constraint_minmax(substream->runtime,
						SNDRV_PCM_HW_PARAM_CHANNELS,
						2, 2);
		}
		twl4030->master_substream = substream;
	}

	return 0;
}

static void twl4030_shutdown(struct snd_pcm_substream *substream,
			     struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;

	if (twl4030->master_substream == substream)
		twl4030->master_substream = twl4030->slave_substream;

	twl4030->slave_substream = NULL;

	
	if (!twl4030->master_substream)
		twl4030->configured = 0;
	 else if (!twl4030->master_substream->runtime->channels)
		twl4030->configured = 0;

	 
	if (substream->runtime->channels == 4)
		twl4030_tdm_enable(codec, substream->stream, 0);
}

static int twl4030_hw_params(struct snd_pcm_substream *substream,
			   struct snd_pcm_hw_params *params,
			   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 mode, old_mode, format, old_format;

	 
	if (params_channels(params) == 4) {
		format = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
		mode = twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE);

		
		if ((mode & TWL4030_OPTION_1) &&
		    ((format & TWL4030_AIF_FORMAT) == TWL4030_AIF_FORMAT_TDM))
			twl4030_tdm_enable(codec, substream->stream, 1);
		else
			return -EINVAL;
	}

	if (twl4030->configured)
		
		return 0;

	
	old_mode = twl4030_read_reg_cache(codec,
			TWL4030_REG_CODEC_MODE) & ~TWL4030_CODECPDZ;
	mode = old_mode & ~TWL4030_APLL_RATE;

	switch (params_rate(params)) {
	case 8000:
		mode |= TWL4030_APLL_RATE_8000;
		break;
	case 11025:
		mode |= TWL4030_APLL_RATE_11025;
		break;
	case 12000:
		mode |= TWL4030_APLL_RATE_12000;
		break;
	case 16000:
		mode |= TWL4030_APLL_RATE_16000;
		break;
	case 22050:
		mode |= TWL4030_APLL_RATE_22050;
		break;
	case 24000:
		mode |= TWL4030_APLL_RATE_24000;
		break;
	case 32000:
		mode |= TWL4030_APLL_RATE_32000;
		break;
	case 44100:
		mode |= TWL4030_APLL_RATE_44100;
		break;
	case 48000:
		mode |= TWL4030_APLL_RATE_48000;
		break;
	case 96000:
		mode |= TWL4030_APLL_RATE_96000;
		break;
	default:
		printk(KERN_ERR "TWL4030 hw params: unknown rate %d\n",
			params_rate(params));
		return -EINVAL;
	}

	if (mode != old_mode) {
		
		twl4030_codec_enable(codec, 0);
		twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
		twl4030_codec_enable(codec, 1);
	}

	
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
	format = old_format;
	format &= ~TWL4030_DATA_WIDTH;
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		format |= TWL4030_DATA_WIDTH_16S_16W;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		format |= TWL4030_DATA_WIDTH_32S_24W;
		break;
	default:
		printk(KERN_ERR "TWL4030 hw params: unknown format %d\n",
			params_format(params));
		return -EINVAL;
	}

	if (format != old_format) {

		
		twl4030_codec_enable(codec, 0);

		
		twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);

		
		twl4030_codec_enable(codec, 1);
	}

	
	twl4030->configured = 1;
	twl4030->rate = params_rate(params);
	twl4030->sample_bits = hw_param_interval(params,
					SNDRV_PCM_HW_PARAM_SAMPLE_BITS)->min;
	twl4030->channels = params_channels(params);

	
	if (twl4030->slave_substream)
		twl4030_constraints(twl4030, substream);

	return 0;
}

static int twl4030_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct twl4030_priv *twl4030 = codec->private_data;
	u8 infreq;

	switch (freq) {
	case 19200000:
		infreq = TWL4030_APLL_INFREQ_19200KHZ;
		twl4030->sysclk = 19200;
		break;
	case 26000000:
		infreq = TWL4030_APLL_INFREQ_26000KHZ;
		twl4030->sysclk = 26000;
		break;
	case 38400000:
		infreq = TWL4030_APLL_INFREQ_38400KHZ;
		twl4030->sysclk = 38400;
		break;
	default:
		printk(KERN_ERR "TWL4030 set sysclk: unknown rate %d\n",
			freq);
		return -EINVAL;
	}

	infreq |= TWL4030_APLL_EN;
	twl4030_write(codec, TWL4030_REG_APLL_CTL, infreq);

	return 0;
}

static int twl4030_set_dai_fmt(struct snd_soc_dai *codec_dai,
			     unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u8 old_format, format;

	
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);
	format = old_format;

	
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		format &= ~(TWL4030_AIF_SLAVE_EN);
		format &= ~(TWL4030_CLK256FS_EN);
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		format |= TWL4030_AIF_SLAVE_EN;
		format |= TWL4030_CLK256FS_EN;
		break;
	default:
		return -EINVAL;
	}

	
	format &= ~TWL4030_AIF_FORMAT;
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		format |= TWL4030_AIF_FORMAT_CODEC;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		format |= TWL4030_AIF_FORMAT_TDM;
		break;
	default:
		return -EINVAL;
	}

	if (format != old_format) {

		
		twl4030_codec_enable(codec, 0);

		
		twl4030_write(codec, TWL4030_REG_AUDIO_IF, format);

		
		twl4030_codec_enable(codec, 1);
	}

	return 0;
}

static int twl4030_set_tristate(struct snd_soc_dai *dai, int tristate)
{
	struct snd_soc_codec *codec = dai->codec;
	u8 reg = twl4030_read_reg_cache(codec, TWL4030_REG_AUDIO_IF);

	if (tristate)
		reg |= TWL4030_AIF_TRI_EN;
	else
		reg &= ~TWL4030_AIF_TRI_EN;

	return twl4030_write(codec, TWL4030_REG_AUDIO_IF, reg);
}


static void twl4030_voice_enable(struct snd_soc_codec *codec, int direction,
				int enable)
{
	u8 reg, mask;

	reg = twl4030_read_reg_cache(codec, TWL4030_REG_OPTION);

	if (direction == SNDRV_PCM_STREAM_PLAYBACK)
		mask = TWL4030_ARXL1_VRX_EN;
	else
		mask = TWL4030_ATXL2_VTXL_EN | TWL4030_ATXR2_VTXR_EN;

	if (enable)
		reg |= mask;
	else
		reg &= ~mask;

	twl4030_write(codec, TWL4030_REG_OPTION, reg);
}

static int twl4030_voice_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	u8 infreq;
	u8 mode;

	
	infreq = twl4030_read_reg_cache(codec, TWL4030_REG_APLL_CTL)
		& TWL4030_APLL_INFREQ;

	if (infreq != TWL4030_APLL_INFREQ_26000KHZ) {
		printk(KERN_ERR "TWL4030 voice startup: "
			"MCLK is not 26MHz, call set_sysclk() on init\n");
		return -EINVAL;
	}

	
	mode = twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE)
		& TWL4030_OPT_MODE;

	if (mode != TWL4030_OPTION_2) {
		printk(KERN_ERR "TWL4030 voice startup: "
			"the codec mode is not option2\n");
		return -EINVAL;
	}

	return 0;
}

static void twl4030_voice_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;

	
	twl4030_voice_enable(codec, substream->stream, 0);
}

static int twl4030_voice_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	u8 old_mode, mode;

	
	twl4030_voice_enable(codec, substream->stream, 1);

	
	old_mode = twl4030_read_reg_cache(codec, TWL4030_REG_CODEC_MODE)
		& ~(TWL4030_CODECPDZ);
	mode = old_mode;

	switch (params_rate(params)) {
	case 8000:
		mode &= ~(TWL4030_SEL_16K);
		break;
	case 16000:
		mode |= TWL4030_SEL_16K;
		break;
	default:
		printk(KERN_ERR "TWL4030 voice hw params: unknown rate %d\n",
			params_rate(params));
		return -EINVAL;
	}

	if (mode != old_mode) {
		
		twl4030_codec_enable(codec, 0);
		twl4030_write(codec, TWL4030_REG_CODEC_MODE, mode);
		twl4030_codec_enable(codec, 1);
	}

	return 0;
}

static int twl4030_voice_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u8 infreq;

	switch (freq) {
	case 26000000:
		infreq = TWL4030_APLL_INFREQ_26000KHZ;
		break;
	default:
		printk(KERN_ERR "TWL4030 voice set sysclk: unknown rate %d\n",
			freq);
		return -EINVAL;
	}

	infreq |= TWL4030_APLL_EN;
	twl4030_write(codec, TWL4030_REG_APLL_CTL, infreq);

	return 0;
}

static int twl4030_voice_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u8 old_format, format;

	
	old_format = twl4030_read_reg_cache(codec, TWL4030_REG_VOICE_IF);
	format = old_format;

	
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		format &= ~(TWL4030_VIF_SLAVE_EN);
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		format |= TWL4030_VIF_SLAVE_EN;
		break;
	default:
		return -EINVAL;
	}

	
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_IB_NF:
		format &= ~(TWL4030_VIF_FORMAT);
		break;
	case SND_SOC_DAIFMT_NB_IF:
		format |= TWL4030_VIF_FORMAT;
		break;
	default:
		return -EINVAL;
	}

	if (format != old_format) {
		
		twl4030_codec_enable(codec, 0);
		twl4030_write(codec, TWL4030_REG_VOICE_IF, format);
		twl4030_codec_enable(codec, 1);
	}

	return 0;
}

static int twl4030_voice_set_tristate(struct snd_soc_dai *dai, int tristate)
{
	struct snd_soc_codec *codec = dai->codec;
	u8 reg = twl4030_read_reg_cache(codec, TWL4030_REG_VOICE_IF);

	if (tristate)
		reg |= TWL4030_VIF_TRI_EN;
	else
		reg &= ~TWL4030_VIF_TRI_EN;

	return twl4030_write(codec, TWL4030_REG_VOICE_IF, reg);
}

#define TWL4030_RATES	 (SNDRV_PCM_RATE_8000_48000)
#define TWL4030_FORMATS	 (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FORMAT_S24_LE)

static struct snd_soc_dai_ops twl4030_dai_ops = {
	.startup	= twl4030_startup,
	.shutdown	= twl4030_shutdown,
	.hw_params	= twl4030_hw_params,
	.set_sysclk	= twl4030_set_dai_sysclk,
	.set_fmt	= twl4030_set_dai_fmt,
	.set_tristate	= twl4030_set_tristate,
};

static struct snd_soc_dai_ops twl4030_dai_voice_ops = {
	.startup	= twl4030_voice_startup,
	.shutdown	= twl4030_voice_shutdown,
	.hw_params	= twl4030_voice_hw_params,
	.set_sysclk	= twl4030_voice_set_dai_sysclk,
	.set_fmt	= twl4030_voice_set_dai_fmt,
	.set_tristate	= twl4030_voice_set_tristate,
};

struct snd_soc_dai twl4030_dai[] = {
{
	.name = "twl4030",
	.playback = {
		.stream_name = "HiFi Playback",
		.channels_min = 2,
		.channels_max = 4,
		.rates = TWL4030_RATES | SNDRV_PCM_RATE_96000,
		.formats = TWL4030_FORMATS,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 2,
		.channels_max = 4,
		.rates = TWL4030_RATES,
		.formats = TWL4030_FORMATS,},
	.ops = &twl4030_dai_ops,
},
{
	.name = "twl4030 Voice",
	.playback = {
		.stream_name = "Voice Playback",
		.channels_min = 1,
		.channels_max = 1,
		.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops = &twl4030_dai_voice_ops,
},
};
EXPORT_SYMBOL_GPL(twl4030_dai);

static int twl4030_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	twl4030_set_bias_level(codec, SND_SOC_BIAS_OFF);

	return 0;
}

static int twl4030_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	twl4030_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	twl4030_set_bias_level(codec, codec->suspend_bias_level);
	return 0;
}



static int twl4030_init(struct snd_soc_device *socdev)
{
	struct snd_soc_codec *codec = socdev->card->codec;
	struct twl4030_setup_data *setup = socdev->codec_data;
	struct twl4030_priv *twl4030 = codec->private_data;
	int ret = 0;

	printk(KERN_INFO "TWL4030 Audio Codec init \n");

	codec->name = "twl4030";
	codec->owner = THIS_MODULE;
	codec->read = twl4030_read_reg_cache;
	codec->write = twl4030_write;
	codec->set_bias_level = twl4030_set_bias_level;
	codec->dai = twl4030_dai;
	codec->num_dai = ARRAY_SIZE(twl4030_dai),
	codec->reg_cache_size = sizeof(twl4030_reg);
	codec->reg_cache = kmemdup(twl4030_reg, sizeof(twl4030_reg),
					GFP_KERNEL);
	if (codec->reg_cache == NULL)
		return -ENOMEM;

	
	if (setup) {
		unsigned char hs_pop;

		if (setup->sysclk)
			twl4030->sysclk = setup->sysclk;
		else
			twl4030->sysclk = 26000;

		hs_pop = twl4030_read_reg_cache(codec, TWL4030_REG_HS_POPN_SET);
		hs_pop &= ~TWL4030_RAMP_DELAY;
		hs_pop |= (setup->ramp_delay_value << 2);
		twl4030_write_reg_cache(codec, TWL4030_REG_HS_POPN_SET, hs_pop);
	} else {
		twl4030->sysclk = 26000;
	}

	
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		printk(KERN_ERR "twl4030: failed to create pcms\n");
		goto pcm_err;
	}

	twl4030_init_chip(codec);

	
	twl4030_set_bias_level(codec, SND_SOC_BIAS_STANDBY);

	snd_soc_add_controls(codec, twl4030_snd_controls,
				ARRAY_SIZE(twl4030_snd_controls));
	twl4030_add_widgets(codec);

	ret = snd_soc_init_card(socdev);
	if (ret < 0) {
		printk(KERN_ERR "twl4030: failed to register card\n");
		goto card_err;
	}

	return ret;

card_err:
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
pcm_err:
	kfree(codec->reg_cache);
	return ret;
}

static struct snd_soc_device *twl4030_socdev;

static int twl4030_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec;
	struct twl4030_priv *twl4030;

	codec = kzalloc(sizeof(struct snd_soc_codec), GFP_KERNEL);
	if (codec == NULL)
		return -ENOMEM;

	twl4030 = kzalloc(sizeof(struct twl4030_priv), GFP_KERNEL);
	if (twl4030 == NULL) {
		kfree(codec);
		return -ENOMEM;
	}

	codec->private_data = twl4030;
	socdev->card->codec = codec;
	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	twl4030_socdev = socdev;
	twl4030_init(socdev);

	return 0;
}

static int twl4030_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	printk(KERN_INFO "TWL4030 Audio Codec remove\n");
	twl4030_set_bias_level(codec, SND_SOC_BIAS_OFF);
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
	kfree(codec->private_data);
	kfree(codec);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_twl4030 = {
	.probe = twl4030_probe,
	.remove = twl4030_remove,
	.suspend = twl4030_suspend,
	.resume = twl4030_resume,
};
EXPORT_SYMBOL_GPL(soc_codec_dev_twl4030);

static int __init twl4030_modinit(void)
{
	return snd_soc_register_dais(&twl4030_dai[0], ARRAY_SIZE(twl4030_dai));
}
module_init(twl4030_modinit);

static void __exit twl4030_exit(void)
{
	snd_soc_unregister_dais(&twl4030_dai[0], ARRAY_SIZE(twl4030_dai));
}
module_exit(twl4030_exit);

MODULE_DESCRIPTION("ASoC TWL4030 codec driver");
MODULE_AUTHOR("Steve Sakoman");
MODULE_LICENSE("GPL");
