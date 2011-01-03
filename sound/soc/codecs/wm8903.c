

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/initval.h>

#include "wm8903.h"


static u16 wm8903_reg_defaults[] = {
	0x8903,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0018,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0001,     
	0x0000,     
	0x0001,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0400,     
	0x0D07,     
	0x0000,     
	0x0000,     
	0x0050,     
	0x0242,     
	0x0008,     
	0x0022,     
	0x0000,     
	0x0000,     
	0x00C0,     
	0x00C0,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x00C0,     
	0x00C0,     
	0x0000,     
	0x0073,     
	0x09BF,     
	0x3241,     
	0x0020,     
	0x0000,     
	0x0085,     
	0x0085,     
	0x0044,     
	0x0044,     
	0x0000,     
	0x0000,     
	0x0008,     
	0x0004,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x002D,     
	0x002D,     
	0x0039,     
	0x0039,     
	0x0100,     
	0x0139,     
	0x0139,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0010,     
	0x0100,     
	0x00A4,     
	0x0807,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x000E,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0006,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0060,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0060,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x1F25,     
	0x2B19,     
	0x01C0,     
	0x01EF,     
	0x2B00,     
	0x0000,     
	0x01C0,     
	0x1C10,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x00A8,     
	0x00A8,     
	0x00A8,     
	0x0220,     
	0x01A0,     
	0x0000,     
	0xFFFF,     
	0x0000,     
	0x0000,     
	0x0003,     
	0x0000,     
	0x0000,     
	0x0005,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x03FF,     
	0x0007,     
	0x0040,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x4000,     
	0x6810,     
	0x0004,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0028,     
	0x0004,     
	0x0000,     
	0x0060,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
	0x0000,     
};

struct wm8903_priv {
	struct snd_soc_codec codec;
	u16 reg_cache[ARRAY_SIZE(wm8903_reg_defaults)];

	int sysclk;

	
	int class_w_users;
	int playback_active;
	int capture_active;

	struct snd_pcm_substream *master_substream;
	struct snd_pcm_substream *slave_substream;
};

static int wm8903_volatile_register(unsigned int reg)
{
	switch (reg) {
	case WM8903_SW_RESET_AND_ID:
	case WM8903_REVISION_NUMBER:
	case WM8903_INTERRUPT_STATUS_1:
	case WM8903_WRITE_SEQUENCER_4:
		return 1;

	default:
		return 0;
	}
}

static int wm8903_run_sequence(struct snd_soc_codec *codec, unsigned int start)
{
	u16 reg[5];
	struct i2c_client *i2c = codec->control_data;

	BUG_ON(start > 48);

	
	reg[0] = snd_soc_read(codec, WM8903_WRITE_SEQUENCER_0);
	reg[0] |= WM8903_WSEQ_ENA;
	snd_soc_write(codec, WM8903_WRITE_SEQUENCER_0, reg[0]);

	dev_dbg(&i2c->dev, "Starting sequence at %d\n", start);

	snd_soc_write(codec, WM8903_WRITE_SEQUENCER_3,
		     start | WM8903_WSEQ_START);

	
	do {
		msleep(10);

		reg[4] = snd_soc_read(codec, WM8903_WRITE_SEQUENCER_4);
	} while (reg[4] & WM8903_WSEQ_BUSY);

	dev_dbg(&i2c->dev, "Sequence complete\n");

	
	snd_soc_write(codec, WM8903_WRITE_SEQUENCER_0,
		     reg[0] & ~WM8903_WSEQ_ENA);

	return 0;
}

static void wm8903_sync_reg_cache(struct snd_soc_codec *codec, u16 *cache)
{
	int i;

	
	for (i = 0; i < ARRAY_SIZE(wm8903_reg_defaults); i++)
		cache[i] = codec->hw_read(codec, i);
}

static void wm8903_reset(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, WM8903_SW_RESET_AND_ID, 0);
	memcpy(codec->reg_cache, wm8903_reg_defaults,
	       sizeof(wm8903_reg_defaults));
}

#define WM8903_OUTPUT_SHORT 0x8
#define WM8903_OUTPUT_OUT   0x4
#define WM8903_OUTPUT_INT   0x2
#define WM8903_OUTPUT_IN    0x1

static int wm8903_cp_event(struct snd_soc_dapm_widget *w,
			   struct snd_kcontrol *kcontrol, int event)
{
	WARN_ON(event != SND_SOC_DAPM_POST_PMU);
	mdelay(4);

	return 0;
}


static int wm8903_output_event(struct snd_soc_dapm_widget *w,
			       struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	u16 val;
	u16 reg;
	u16 dcs_reg;
	u16 dcs_bit;
	int shift;

	switch (w->reg) {
	case WM8903_POWER_MANAGEMENT_2:
		reg = WM8903_ANALOGUE_HP_0;
		dcs_bit = 0 + w->shift;
		break;
	case WM8903_POWER_MANAGEMENT_3:
		reg = WM8903_ANALOGUE_LINEOUT_0;
		dcs_bit = 2 + w->shift;
		break;
	default:
		BUG();
		return -EINVAL;  
	}

	switch (w->shift) {
	case 0:
		shift = 0;
		break;
	case 1:
		shift = 4;
		break;
	default:
		BUG();
		return -EINVAL;  
	}

	if (event & SND_SOC_DAPM_PRE_PMU) {
		val = snd_soc_read(codec, reg);

		
		val &= ~(WM8903_OUTPUT_SHORT << shift);
		snd_soc_write(codec, reg, val);
	}

	if (event & SND_SOC_DAPM_POST_PMU) {
		val = snd_soc_read(codec, reg);

		val |= (WM8903_OUTPUT_IN << shift);
		snd_soc_write(codec, reg, val);

		val |= (WM8903_OUTPUT_INT << shift);
		snd_soc_write(codec, reg, val);

		
		val |= (WM8903_OUTPUT_OUT << shift);
		snd_soc_write(codec, reg, val);

		
		dcs_reg = snd_soc_read(codec, WM8903_DC_SERVO_0);
		dcs_reg |= dcs_bit;
		snd_soc_write(codec, WM8903_DC_SERVO_0, dcs_reg);

		
		val |= (WM8903_OUTPUT_SHORT << shift);
		snd_soc_write(codec, reg, val);
	}

	if (event & SND_SOC_DAPM_PRE_PMD) {
		val = snd_soc_read(codec, reg);

		
		val &= ~(WM8903_OUTPUT_SHORT << shift);
		snd_soc_write(codec, reg, val);

		
		dcs_reg = snd_soc_read(codec, WM8903_DC_SERVO_0);
		dcs_reg &= ~dcs_bit;
		snd_soc_write(codec, WM8903_DC_SERVO_0, dcs_reg);

		
		val &= ~((WM8903_OUTPUT_OUT | WM8903_OUTPUT_INT |
			  WM8903_OUTPUT_IN) << shift);
		snd_soc_write(codec, reg, val);
	}

	return 0;
}


static int wm8903_class_w_put(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_dapm_widget *widget = snd_kcontrol_chip(kcontrol);
	struct snd_soc_codec *codec = widget->codec;
	struct wm8903_priv *wm8903 = codec->private_data;
	struct i2c_client *i2c = codec->control_data;
	u16 reg;
	int ret;

	reg = snd_soc_read(codec, WM8903_CLASS_W_0);

	
	if (ucontrol->value.integer.value[0]) {
		if (wm8903->class_w_users == 0) {
			dev_dbg(&i2c->dev, "Disabling Class W\n");
			snd_soc_write(codec, WM8903_CLASS_W_0, reg &
				     ~(WM8903_CP_DYN_FREQ | WM8903_CP_DYN_V));
		}
		wm8903->class_w_users++;
	}

	
	ret = snd_soc_dapm_put_volsw(kcontrol, ucontrol);

	
	if (!ucontrol->value.integer.value[0]) {
		if (wm8903->class_w_users == 1) {
			dev_dbg(&i2c->dev, "Enabling Class W\n");
			snd_soc_write(codec, WM8903_CLASS_W_0, reg |
				     WM8903_CP_DYN_FREQ | WM8903_CP_DYN_V);
		}
		wm8903->class_w_users--;
	}

	dev_dbg(&i2c->dev, "Bypass use count now %d\n",
		wm8903->class_w_users);

	return ret;
}

#define SOC_DAPM_SINGLE_W(xname, reg, shift, max, invert) \
{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
	.info = snd_soc_info_volsw, \
	.get = snd_soc_dapm_get_volsw, .put = wm8903_class_w_put, \
	.private_value =  SOC_SINGLE_VALUE(reg, shift, max, invert) }



static const DECLARE_TLV_DB_SCALE(digital_tlv, -7200, 75, 1);

static const DECLARE_TLV_DB_SCALE(digital_sidetone_tlv, -3600, 300, 0);
static const DECLARE_TLV_DB_SCALE(out_tlv, -5700, 100, 0);

static const DECLARE_TLV_DB_SCALE(drc_tlv_thresh, 0, 75, 0);
static const DECLARE_TLV_DB_SCALE(drc_tlv_amp, -2250, 75, 0);
static const DECLARE_TLV_DB_SCALE(drc_tlv_min, 0, 600, 0);
static const DECLARE_TLV_DB_SCALE(drc_tlv_max, 1200, 600, 0);
static const DECLARE_TLV_DB_SCALE(drc_tlv_startup, -300, 50, 0);

static const char *drc_slope_text[] = {
	"1", "1/2", "1/4", "1/8", "1/16", "0"
};

static const struct soc_enum drc_slope_r0 =
	SOC_ENUM_SINGLE(WM8903_DRC_2, 3, 6, drc_slope_text);

static const struct soc_enum drc_slope_r1 =
	SOC_ENUM_SINGLE(WM8903_DRC_2, 0, 6, drc_slope_text);

static const char *drc_attack_text[] = {
	"instantaneous",
	"363us", "762us", "1.45ms", "2.9ms", "5.8ms", "11.6ms", "23.2ms",
	"46.4ms", "92.8ms", "185.6ms"
};

static const struct soc_enum drc_attack =
	SOC_ENUM_SINGLE(WM8903_DRC_1, 12, 11, drc_attack_text);

static const char *drc_decay_text[] = {
	"186ms", "372ms", "743ms", "1.49s", "2.97s", "5.94s", "11.89s",
	"23.87s", "47.56s"
};

static const struct soc_enum drc_decay =
	SOC_ENUM_SINGLE(WM8903_DRC_1, 8, 9, drc_decay_text);

static const char *drc_ff_delay_text[] = {
	"5 samples", "9 samples"
};

static const struct soc_enum drc_ff_delay =
	SOC_ENUM_SINGLE(WM8903_DRC_0, 5, 2, drc_ff_delay_text);

static const char *drc_qr_decay_text[] = {
	"0.725ms", "1.45ms", "5.8ms"
};

static const struct soc_enum drc_qr_decay =
	SOC_ENUM_SINGLE(WM8903_DRC_1, 4, 3, drc_qr_decay_text);

static const char *drc_smoothing_text[] = {
	"Low", "Medium", "High"
};

static const struct soc_enum drc_smoothing =
	SOC_ENUM_SINGLE(WM8903_DRC_0, 11, 3, drc_smoothing_text);

static const char *soft_mute_text[] = {
	"Fast (fs/2)", "Slow (fs/32)"
};

static const struct soc_enum soft_mute =
	SOC_ENUM_SINGLE(WM8903_DAC_DIGITAL_1, 10, 2, soft_mute_text);

static const char *mute_mode_text[] = {
	"Hard", "Soft"
};

static const struct soc_enum mute_mode =
	SOC_ENUM_SINGLE(WM8903_DAC_DIGITAL_1, 9, 2, mute_mode_text);

static const char *dac_deemphasis_text[] = {
	"Disabled", "32kHz", "44.1kHz", "48kHz"
};

static const struct soc_enum dac_deemphasis =
	SOC_ENUM_SINGLE(WM8903_DAC_DIGITAL_1, 1, 4, dac_deemphasis_text);

static const char *companding_text[] = {
	"ulaw", "alaw"
};

static const struct soc_enum dac_companding =
	SOC_ENUM_SINGLE(WM8903_AUDIO_INTERFACE_0, 0, 2, companding_text);

static const struct soc_enum adc_companding =
	SOC_ENUM_SINGLE(WM8903_AUDIO_INTERFACE_0, 2, 2, companding_text);

static const char *input_mode_text[] = {
	"Single-Ended", "Differential Line", "Differential Mic"
};

static const struct soc_enum linput_mode_enum =
	SOC_ENUM_SINGLE(WM8903_ANALOGUE_LEFT_INPUT_1, 0, 3, input_mode_text);

static const struct soc_enum rinput_mode_enum =
	SOC_ENUM_SINGLE(WM8903_ANALOGUE_RIGHT_INPUT_1, 0, 3, input_mode_text);

static const char *linput_mux_text[] = {
	"IN1L", "IN2L", "IN3L"
};

static const struct soc_enum linput_enum =
	SOC_ENUM_SINGLE(WM8903_ANALOGUE_LEFT_INPUT_1, 2, 3, linput_mux_text);

static const struct soc_enum linput_inv_enum =
	SOC_ENUM_SINGLE(WM8903_ANALOGUE_LEFT_INPUT_1, 4, 3, linput_mux_text);

static const char *rinput_mux_text[] = {
	"IN1R", "IN2R", "IN3R"
};

static const struct soc_enum rinput_enum =
	SOC_ENUM_SINGLE(WM8903_ANALOGUE_RIGHT_INPUT_1, 2, 3, rinput_mux_text);

static const struct soc_enum rinput_inv_enum =
	SOC_ENUM_SINGLE(WM8903_ANALOGUE_RIGHT_INPUT_1, 4, 3, rinput_mux_text);


static const char *sidetone_text[] = {
	"None", "Left", "Right"
};

static const struct soc_enum lsidetone_enum =
	SOC_ENUM_SINGLE(WM8903_DAC_DIGITAL_0, 2, 3, sidetone_text);

static const struct soc_enum rsidetone_enum =
	SOC_ENUM_SINGLE(WM8903_DAC_DIGITAL_0, 0, 3, sidetone_text);

static const struct snd_kcontrol_new wm8903_snd_controls[] = {


SOC_SINGLE("Left Input PGA Switch", WM8903_ANALOGUE_LEFT_INPUT_0,
	   7, 1, 1),
SOC_SINGLE("Left Input PGA Volume", WM8903_ANALOGUE_LEFT_INPUT_0,
	   0, 31, 0),
SOC_SINGLE("Left Input PGA Common Mode Switch", WM8903_ANALOGUE_LEFT_INPUT_1,
	   6, 1, 0),

SOC_SINGLE("Right Input PGA Switch", WM8903_ANALOGUE_RIGHT_INPUT_0,
	   7, 1, 1),
SOC_SINGLE("Right Input PGA Volume", WM8903_ANALOGUE_RIGHT_INPUT_0,
	   0, 31, 0),
SOC_SINGLE("Right Input PGA Common Mode Switch", WM8903_ANALOGUE_RIGHT_INPUT_1,
	   6, 1, 0),


SOC_SINGLE("DRC Switch", WM8903_DRC_0, 15, 1, 0),
SOC_ENUM("DRC Compressor Slope R0", drc_slope_r0),
SOC_ENUM("DRC Compressor Slope R1", drc_slope_r1),
SOC_SINGLE_TLV("DRC Compressor Threashold Volume", WM8903_DRC_3, 5, 124, 1,
	       drc_tlv_thresh),
SOC_SINGLE_TLV("DRC Volume", WM8903_DRC_3, 0, 30, 1, drc_tlv_amp),
SOC_SINGLE_TLV("DRC Minimum Gain Volume", WM8903_DRC_1, 2, 3, 1, drc_tlv_min),
SOC_SINGLE_TLV("DRC Maximum Gain Volume", WM8903_DRC_1, 0, 3, 0, drc_tlv_max),
SOC_ENUM("DRC Attack Rate", drc_attack),
SOC_ENUM("DRC Decay Rate", drc_decay),
SOC_ENUM("DRC FF Delay", drc_ff_delay),
SOC_SINGLE("DRC Anticlip Switch", WM8903_DRC_0, 1, 1, 0),
SOC_SINGLE("DRC QR Switch", WM8903_DRC_0, 2, 1, 0),
SOC_SINGLE_TLV("DRC QR Threashold Volume", WM8903_DRC_0, 6, 3, 0, drc_tlv_max),
SOC_ENUM("DRC QR Decay Rate", drc_qr_decay),
SOC_SINGLE("DRC Smoothing Switch", WM8903_DRC_0, 3, 1, 0),
SOC_SINGLE("DRC Smoothing Hysteresis Switch", WM8903_DRC_0, 0, 1, 0),
SOC_ENUM("DRC Smoothing Threashold", drc_smoothing),
SOC_SINGLE_TLV("DRC Startup Volume", WM8903_DRC_0, 6, 18, 0, drc_tlv_startup),

SOC_DOUBLE_R_TLV("Digital Capture Volume", WM8903_ADC_DIGITAL_VOLUME_LEFT,
		 WM8903_ADC_DIGITAL_VOLUME_RIGHT, 1, 96, 0, digital_tlv),
SOC_ENUM("ADC Companding Mode", adc_companding),
SOC_SINGLE("ADC Companding Switch", WM8903_AUDIO_INTERFACE_0, 3, 1, 0),

SOC_DOUBLE_TLV("Digital Sidetone Volume", WM8903_DAC_DIGITAL_0, 4, 8,
	       12, 0, digital_sidetone_tlv),


SOC_DOUBLE_R_TLV("Digital Playback Volume", WM8903_DAC_DIGITAL_VOLUME_LEFT,
		 WM8903_DAC_DIGITAL_VOLUME_RIGHT, 1, 120, 0, digital_tlv),
SOC_ENUM("DAC Soft Mute Rate", soft_mute),
SOC_ENUM("DAC Mute Mode", mute_mode),
SOC_SINGLE("DAC Mono Switch", WM8903_DAC_DIGITAL_1, 12, 1, 0),
SOC_ENUM("DAC De-emphasis", dac_deemphasis),
SOC_ENUM("DAC Companding Mode", dac_companding),
SOC_SINGLE("DAC Companding Switch", WM8903_AUDIO_INTERFACE_0, 1, 1, 0),


SOC_DOUBLE_R("Headphone Switch",
	     WM8903_ANALOGUE_OUT1_LEFT, WM8903_ANALOGUE_OUT1_RIGHT,
	     8, 1, 1),
SOC_DOUBLE_R("Headphone ZC Switch",
	     WM8903_ANALOGUE_OUT1_LEFT, WM8903_ANALOGUE_OUT1_RIGHT,
	     6, 1, 0),
SOC_DOUBLE_R_TLV("Headphone Volume",
		 WM8903_ANALOGUE_OUT1_LEFT, WM8903_ANALOGUE_OUT1_RIGHT,
		 0, 63, 0, out_tlv),


SOC_DOUBLE_R("Line Out Switch",
	     WM8903_ANALOGUE_OUT2_LEFT, WM8903_ANALOGUE_OUT2_RIGHT,
	     8, 1, 1),
SOC_DOUBLE_R("Line Out ZC Switch",
	     WM8903_ANALOGUE_OUT2_LEFT, WM8903_ANALOGUE_OUT2_RIGHT,
	     6, 1, 0),
SOC_DOUBLE_R_TLV("Line Out Volume",
		 WM8903_ANALOGUE_OUT2_LEFT, WM8903_ANALOGUE_OUT2_RIGHT,
		 0, 63, 0, out_tlv),


SOC_DOUBLE_R("Speaker Switch",
	     WM8903_ANALOGUE_OUT3_LEFT, WM8903_ANALOGUE_OUT3_RIGHT, 8, 1, 1),
SOC_DOUBLE_R("Speaker ZC Switch",
	     WM8903_ANALOGUE_OUT3_LEFT, WM8903_ANALOGUE_OUT3_RIGHT, 6, 1, 0),
SOC_DOUBLE_R_TLV("Speaker Volume",
		 WM8903_ANALOGUE_OUT3_LEFT, WM8903_ANALOGUE_OUT3_RIGHT,
		 0, 63, 0, out_tlv),
};

static const struct snd_kcontrol_new linput_mode_mux =
	SOC_DAPM_ENUM("Left Input Mode Mux", linput_mode_enum);

static const struct snd_kcontrol_new rinput_mode_mux =
	SOC_DAPM_ENUM("Right Input Mode Mux", rinput_mode_enum);

static const struct snd_kcontrol_new linput_mux =
	SOC_DAPM_ENUM("Left Input Mux", linput_enum);

static const struct snd_kcontrol_new linput_inv_mux =
	SOC_DAPM_ENUM("Left Inverting Input Mux", linput_inv_enum);

static const struct snd_kcontrol_new rinput_mux =
	SOC_DAPM_ENUM("Right Input Mux", rinput_enum);

static const struct snd_kcontrol_new rinput_inv_mux =
	SOC_DAPM_ENUM("Right Inverting Input Mux", rinput_inv_enum);

static const struct snd_kcontrol_new lsidetone_mux =
	SOC_DAPM_ENUM("DACL Sidetone Mux", lsidetone_enum);

static const struct snd_kcontrol_new rsidetone_mux =
	SOC_DAPM_ENUM("DACR Sidetone Mux", rsidetone_enum);

static const struct snd_kcontrol_new left_output_mixer[] = {
SOC_DAPM_SINGLE("DACL Switch", WM8903_ANALOGUE_LEFT_MIX_0, 3, 1, 0),
SOC_DAPM_SINGLE("DACR Switch", WM8903_ANALOGUE_LEFT_MIX_0, 2, 1, 0),
SOC_DAPM_SINGLE_W("Left Bypass Switch", WM8903_ANALOGUE_LEFT_MIX_0, 1, 1, 0),
SOC_DAPM_SINGLE_W("Right Bypass Switch", WM8903_ANALOGUE_LEFT_MIX_0, 0, 1, 0),
};

static const struct snd_kcontrol_new right_output_mixer[] = {
SOC_DAPM_SINGLE("DACL Switch", WM8903_ANALOGUE_RIGHT_MIX_0, 3, 1, 0),
SOC_DAPM_SINGLE("DACR Switch", WM8903_ANALOGUE_RIGHT_MIX_0, 2, 1, 0),
SOC_DAPM_SINGLE_W("Left Bypass Switch", WM8903_ANALOGUE_RIGHT_MIX_0, 1, 1, 0),
SOC_DAPM_SINGLE_W("Right Bypass Switch", WM8903_ANALOGUE_RIGHT_MIX_0, 0, 1, 0),
};

static const struct snd_kcontrol_new left_speaker_mixer[] = {
SOC_DAPM_SINGLE("DACL Switch", WM8903_ANALOGUE_SPK_MIX_LEFT_0, 3, 1, 0),
SOC_DAPM_SINGLE("DACR Switch", WM8903_ANALOGUE_SPK_MIX_LEFT_0, 2, 1, 0),
SOC_DAPM_SINGLE("Left Bypass Switch", WM8903_ANALOGUE_SPK_MIX_LEFT_0, 1, 1, 0),
SOC_DAPM_SINGLE("Right Bypass Switch", WM8903_ANALOGUE_SPK_MIX_LEFT_0,
		0, 1, 0),
};

static const struct snd_kcontrol_new right_speaker_mixer[] = {
SOC_DAPM_SINGLE("DACL Switch", WM8903_ANALOGUE_SPK_MIX_RIGHT_0, 3, 1, 0),
SOC_DAPM_SINGLE("DACR Switch", WM8903_ANALOGUE_SPK_MIX_RIGHT_0, 2, 1, 0),
SOC_DAPM_SINGLE("Left Bypass Switch", WM8903_ANALOGUE_SPK_MIX_RIGHT_0,
		1, 1, 0),
SOC_DAPM_SINGLE("Right Bypass Switch", WM8903_ANALOGUE_SPK_MIX_RIGHT_0,
		0, 1, 0),
};

static const struct snd_soc_dapm_widget wm8903_dapm_widgets[] = {
SND_SOC_DAPM_INPUT("IN1L"),
SND_SOC_DAPM_INPUT("IN1R"),
SND_SOC_DAPM_INPUT("IN2L"),
SND_SOC_DAPM_INPUT("IN2R"),
SND_SOC_DAPM_INPUT("IN3L"),
SND_SOC_DAPM_INPUT("IN3R"),

SND_SOC_DAPM_OUTPUT("HPOUTL"),
SND_SOC_DAPM_OUTPUT("HPOUTR"),
SND_SOC_DAPM_OUTPUT("LINEOUTL"),
SND_SOC_DAPM_OUTPUT("LINEOUTR"),
SND_SOC_DAPM_OUTPUT("LOP"),
SND_SOC_DAPM_OUTPUT("LON"),
SND_SOC_DAPM_OUTPUT("ROP"),
SND_SOC_DAPM_OUTPUT("RON"),

SND_SOC_DAPM_MICBIAS("Mic Bias", WM8903_MIC_BIAS_CONTROL_0, 0, 0),

SND_SOC_DAPM_MUX("Left Input Mux", SND_SOC_NOPM, 0, 0, &linput_mux),
SND_SOC_DAPM_MUX("Left Input Inverting Mux", SND_SOC_NOPM, 0, 0,
		 &linput_inv_mux),
SND_SOC_DAPM_MUX("Left Input Mode Mux", SND_SOC_NOPM, 0, 0, &linput_mode_mux),

SND_SOC_DAPM_MUX("Right Input Mux", SND_SOC_NOPM, 0, 0, &rinput_mux),
SND_SOC_DAPM_MUX("Right Input Inverting Mux", SND_SOC_NOPM, 0, 0,
		 &rinput_inv_mux),
SND_SOC_DAPM_MUX("Right Input Mode Mux", SND_SOC_NOPM, 0, 0, &rinput_mode_mux),

SND_SOC_DAPM_PGA("Left Input PGA", WM8903_POWER_MANAGEMENT_0, 1, 0, NULL, 0),
SND_SOC_DAPM_PGA("Right Input PGA", WM8903_POWER_MANAGEMENT_0, 0, 0, NULL, 0),

SND_SOC_DAPM_ADC("ADCL", "Left HiFi Capture", WM8903_POWER_MANAGEMENT_6, 1, 0),
SND_SOC_DAPM_ADC("ADCR", "Right HiFi Capture", WM8903_POWER_MANAGEMENT_6, 0, 0),

SND_SOC_DAPM_MUX("DACL Sidetone", SND_SOC_NOPM, 0, 0, &lsidetone_mux),
SND_SOC_DAPM_MUX("DACR Sidetone", SND_SOC_NOPM, 0, 0, &rsidetone_mux),

SND_SOC_DAPM_DAC("DACL", "Left Playback", WM8903_POWER_MANAGEMENT_6, 3, 0),
SND_SOC_DAPM_DAC("DACR", "Right Playback", WM8903_POWER_MANAGEMENT_6, 2, 0),

SND_SOC_DAPM_MIXER("Left Output Mixer", WM8903_POWER_MANAGEMENT_1, 1, 0,
		   left_output_mixer, ARRAY_SIZE(left_output_mixer)),
SND_SOC_DAPM_MIXER("Right Output Mixer", WM8903_POWER_MANAGEMENT_1, 0, 0,
		   right_output_mixer, ARRAY_SIZE(right_output_mixer)),

SND_SOC_DAPM_MIXER("Left Speaker Mixer", WM8903_POWER_MANAGEMENT_4, 1, 0,
		   left_speaker_mixer, ARRAY_SIZE(left_speaker_mixer)),
SND_SOC_DAPM_MIXER("Right Speaker Mixer", WM8903_POWER_MANAGEMENT_4, 0, 0,
		   right_speaker_mixer, ARRAY_SIZE(right_speaker_mixer)),

SND_SOC_DAPM_PGA_E("Left Headphone Output PGA", WM8903_POWER_MANAGEMENT_2,
		   1, 0, NULL, 0, wm8903_output_event,
		   SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMU |
		   SND_SOC_DAPM_PRE_PMD),
SND_SOC_DAPM_PGA_E("Right Headphone Output PGA", WM8903_POWER_MANAGEMENT_2,
		   0, 0, NULL, 0, wm8903_output_event,
		   SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMU |
		   SND_SOC_DAPM_PRE_PMD),

SND_SOC_DAPM_PGA_E("Left Line Output PGA", WM8903_POWER_MANAGEMENT_3, 1, 0,
		   NULL, 0, wm8903_output_event,
		   SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMU |
		   SND_SOC_DAPM_PRE_PMD),
SND_SOC_DAPM_PGA_E("Right Line Output PGA", WM8903_POWER_MANAGEMENT_3, 0, 0,
		   NULL, 0, wm8903_output_event,
		   SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMU |
		   SND_SOC_DAPM_PRE_PMD),

SND_SOC_DAPM_PGA("Left Speaker PGA", WM8903_POWER_MANAGEMENT_5, 1, 0,
		 NULL, 0),
SND_SOC_DAPM_PGA("Right Speaker PGA", WM8903_POWER_MANAGEMENT_5, 0, 0,
		 NULL, 0),

SND_SOC_DAPM_SUPPLY("Charge Pump", WM8903_CHARGE_PUMP_0, 0, 0,
		    wm8903_cp_event, SND_SOC_DAPM_POST_PMU),
SND_SOC_DAPM_SUPPLY("CLK_DSP", WM8903_CLOCK_RATES_2, 1, 0, NULL, 0),
};

static const struct snd_soc_dapm_route intercon[] = {

	{ "Left Input Mux", "IN1L", "IN1L" },
	{ "Left Input Mux", "IN2L", "IN2L" },
	{ "Left Input Mux", "IN3L", "IN3L" },

	{ "Left Input Inverting Mux", "IN1L", "IN1L" },
	{ "Left Input Inverting Mux", "IN2L", "IN2L" },
	{ "Left Input Inverting Mux", "IN3L", "IN3L" },

	{ "Right Input Mux", "IN1R", "IN1R" },
	{ "Right Input Mux", "IN2R", "IN2R" },
	{ "Right Input Mux", "IN3R", "IN3R" },

	{ "Right Input Inverting Mux", "IN1R", "IN1R" },
	{ "Right Input Inverting Mux", "IN2R", "IN2R" },
	{ "Right Input Inverting Mux", "IN3R", "IN3R" },

	{ "Left Input Mode Mux", "Single-Ended", "Left Input Inverting Mux" },
	{ "Left Input Mode Mux", "Differential Line",
	  "Left Input Mux" },
	{ "Left Input Mode Mux", "Differential Line",
	  "Left Input Inverting Mux" },
	{ "Left Input Mode Mux", "Differential Mic",
	  "Left Input Mux" },
	{ "Left Input Mode Mux", "Differential Mic",
	  "Left Input Inverting Mux" },

	{ "Right Input Mode Mux", "Single-Ended",
	  "Right Input Inverting Mux" },
	{ "Right Input Mode Mux", "Differential Line",
	  "Right Input Mux" },
	{ "Right Input Mode Mux", "Differential Line",
	  "Right Input Inverting Mux" },
	{ "Right Input Mode Mux", "Differential Mic",
	  "Right Input Mux" },
	{ "Right Input Mode Mux", "Differential Mic",
	  "Right Input Inverting Mux" },

	{ "Left Input PGA", NULL, "Left Input Mode Mux" },
	{ "Right Input PGA", NULL, "Right Input Mode Mux" },

	{ "ADCL", NULL, "Left Input PGA" },
	{ "ADCL", NULL, "CLK_DSP" },
	{ "ADCR", NULL, "Right Input PGA" },
	{ "ADCR", NULL, "CLK_DSP" },

	{ "DACL Sidetone", "Left", "ADCL" },
	{ "DACL Sidetone", "Right", "ADCR" },
	{ "DACR Sidetone", "Left", "ADCL" },
	{ "DACR Sidetone", "Right", "ADCR" },

	{ "DACL", NULL, "DACL Sidetone" },
	{ "DACL", NULL, "CLK_DSP" },
	{ "DACR", NULL, "DACR Sidetone" },
	{ "DACR", NULL, "CLK_DSP" },

	{ "Left Output Mixer", "Left Bypass Switch", "Left Input PGA" },
	{ "Left Output Mixer", "Right Bypass Switch", "Right Input PGA" },
	{ "Left Output Mixer", "DACL Switch", "DACL" },
	{ "Left Output Mixer", "DACR Switch", "DACR" },

	{ "Right Output Mixer", "Left Bypass Switch", "Left Input PGA" },
	{ "Right Output Mixer", "Right Bypass Switch", "Right Input PGA" },
	{ "Right Output Mixer", "DACL Switch", "DACL" },
	{ "Right Output Mixer", "DACR Switch", "DACR" },

	{ "Left Speaker Mixer", "Left Bypass Switch", "Left Input PGA" },
	{ "Left Speaker Mixer", "Right Bypass Switch", "Right Input PGA" },
	{ "Left Speaker Mixer", "DACL Switch", "DACL" },
	{ "Left Speaker Mixer", "DACR Switch", "DACR" },

	{ "Right Speaker Mixer", "Left Bypass Switch", "Left Input PGA" },
	{ "Right Speaker Mixer", "Right Bypass Switch", "Right Input PGA" },
	{ "Right Speaker Mixer", "DACL Switch", "DACL" },
	{ "Right Speaker Mixer", "DACR Switch", "DACR" },

	{ "Left Line Output PGA", NULL, "Left Output Mixer" },
	{ "Right Line Output PGA", NULL, "Right Output Mixer" },

	{ "Left Headphone Output PGA", NULL, "Left Output Mixer" },
	{ "Right Headphone Output PGA", NULL, "Right Output Mixer" },

	{ "Left Speaker PGA", NULL, "Left Speaker Mixer" },
	{ "Right Speaker PGA", NULL, "Right Speaker Mixer" },

	{ "HPOUTL", NULL, "Left Headphone Output PGA" },
	{ "HPOUTR", NULL, "Right Headphone Output PGA" },

	{ "LINEOUTL", NULL, "Left Line Output PGA" },
	{ "LINEOUTR", NULL, "Right Line Output PGA" },

	{ "LOP", NULL, "Left Speaker PGA" },
	{ "LON", NULL, "Left Speaker PGA" },

	{ "ROP", NULL, "Right Speaker PGA" },
	{ "RON", NULL, "Right Speaker PGA" },

	{ "Left Headphone Output PGA", NULL, "Charge Pump" },
	{ "Right Headphone Output PGA", NULL, "Charge Pump" },
	{ "Left Line Output PGA", NULL, "Charge Pump" },
	{ "Right Line Output PGA", NULL, "Charge Pump" },
};

static int wm8903_add_widgets(struct snd_soc_codec *codec)
{
	snd_soc_dapm_new_controls(codec, wm8903_dapm_widgets,
				  ARRAY_SIZE(wm8903_dapm_widgets));

	snd_soc_dapm_add_routes(codec, intercon, ARRAY_SIZE(intercon));

	snd_soc_dapm_new_widgets(codec);

	return 0;
}

static int wm8903_set_bias_level(struct snd_soc_codec *codec,
				 enum snd_soc_bias_level level)
{
	struct i2c_client *i2c = codec->control_data;
	u16 reg, reg2;

	switch (level) {
	case SND_SOC_BIAS_ON:
	case SND_SOC_BIAS_PREPARE:
		reg = snd_soc_read(codec, WM8903_VMID_CONTROL_0);
		reg &= ~(WM8903_VMID_RES_MASK);
		reg |= WM8903_VMID_RES_50K;
		snd_soc_write(codec, WM8903_VMID_CONTROL_0, reg);
		break;

	case SND_SOC_BIAS_STANDBY:
		if (codec->bias_level == SND_SOC_BIAS_OFF) {
			snd_soc_write(codec, WM8903_CLOCK_RATES_2,
				     WM8903_CLK_SYS_ENA);

			
			snd_soc_write(codec, WM8903_WRITE_SEQUENCER_0, 0x11);
			snd_soc_write(codec, WM8903_WRITE_SEQUENCER_1, 0x1257);
			snd_soc_write(codec, WM8903_WRITE_SEQUENCER_2, 0x2);

			wm8903_run_sequence(codec, 0);
			wm8903_sync_reg_cache(codec, codec->reg_cache);

			
			reg = snd_soc_read(codec,
					  WM8903_CONTROL_INTERFACE_TEST_1);
			snd_soc_write(codec, WM8903_CONTROL_INTERFACE_TEST_1,
				     reg | WM8903_TEST_KEY);
			reg2 = snd_soc_read(codec, WM8903_CHARGE_PUMP_TEST_1);
			snd_soc_write(codec, WM8903_CHARGE_PUMP_TEST_1,
				     reg2 | WM8903_CP_SW_KELVIN_MODE_MASK);
			snd_soc_write(codec, WM8903_CONTROL_INTERFACE_TEST_1,
				     reg);

			
			dev_dbg(&i2c->dev, "Enabling Class W\n");
			snd_soc_write(codec, WM8903_CLASS_W_0, reg |
				     WM8903_CP_DYN_FREQ | WM8903_CP_DYN_V);
		}

		reg = snd_soc_read(codec, WM8903_VMID_CONTROL_0);
		reg &= ~(WM8903_VMID_RES_MASK);
		reg |= WM8903_VMID_RES_250K;
		snd_soc_write(codec, WM8903_VMID_CONTROL_0, reg);
		break;

	case SND_SOC_BIAS_OFF:
		wm8903_run_sequence(codec, 32);
		reg = snd_soc_read(codec, WM8903_CLOCK_RATES_2);
		reg &= ~WM8903_CLK_SYS_ENA;
		snd_soc_write(codec, WM8903_CLOCK_RATES_2, reg);
		break;
	}

	codec->bias_level = level;

	return 0;
}

static int wm8903_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				 int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct wm8903_priv *wm8903 = codec->private_data;

	wm8903->sysclk = freq;

	return 0;
}

static int wm8903_set_dai_fmt(struct snd_soc_dai *codec_dai,
			      unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 aif1 = snd_soc_read(codec, WM8903_AUDIO_INTERFACE_1);

	aif1 &= ~(WM8903_LRCLK_DIR | WM8903_BCLK_DIR | WM8903_AIF_FMT_MASK |
		  WM8903_AIF_LRCLK_INV | WM8903_AIF_BCLK_INV);

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	case SND_SOC_DAIFMT_CBS_CFM:
		aif1 |= WM8903_LRCLK_DIR;
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		aif1 |= WM8903_LRCLK_DIR | WM8903_BCLK_DIR;
		break;
	case SND_SOC_DAIFMT_CBM_CFS:
		aif1 |= WM8903_BCLK_DIR;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_DSP_A:
		aif1 |= 0x3;
		break;
	case SND_SOC_DAIFMT_DSP_B:
		aif1 |= 0x3 | WM8903_AIF_LRCLK_INV;
		break;
	case SND_SOC_DAIFMT_I2S:
		aif1 |= 0x2;
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		aif1 |= 0x1;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		break;
	default:
		return -EINVAL;
	}

	
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_DSP_A:
	case SND_SOC_DAIFMT_DSP_B:
		
		switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
		case SND_SOC_DAIFMT_NB_NF:
			break;
		case SND_SOC_DAIFMT_IB_NF:
			aif1 |= WM8903_AIF_BCLK_INV;
			break;
		default:
			return -EINVAL;
		}
		break;
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_RIGHT_J:
	case SND_SOC_DAIFMT_LEFT_J:
		switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
		case SND_SOC_DAIFMT_NB_NF:
			break;
		case SND_SOC_DAIFMT_IB_IF:
			aif1 |= WM8903_AIF_BCLK_INV | WM8903_AIF_LRCLK_INV;
			break;
		case SND_SOC_DAIFMT_IB_NF:
			aif1 |= WM8903_AIF_BCLK_INV;
			break;
		case SND_SOC_DAIFMT_NB_IF:
			aif1 |= WM8903_AIF_LRCLK_INV;
			break;
		default:
			return -EINVAL;
		}
		break;
	default:
		return -EINVAL;
	}

	snd_soc_write(codec, WM8903_AUDIO_INTERFACE_1, aif1);

	return 0;
}

static int wm8903_digital_mute(struct snd_soc_dai *codec_dai, int mute)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	u16 reg;

	reg = snd_soc_read(codec, WM8903_DAC_DIGITAL_1);

	if (mute)
		reg |= WM8903_DAC_MUTE;
	else
		reg &= ~WM8903_DAC_MUTE;

	snd_soc_write(codec, WM8903_DAC_DIGITAL_1, reg);

	return 0;
}


static struct {
	int div;
	int rate;
	int mode;
	int mclk_div;
} clk_sys_ratios[] = {
	{   64, 0x0, 0x0, 1 },
	{   68, 0x0, 0x1, 1 },
	{  125, 0x0, 0x2, 1 },
	{  128, 0x1, 0x0, 1 },
	{  136, 0x1, 0x1, 1 },
	{  192, 0x2, 0x0, 1 },
	{  204, 0x2, 0x1, 1 },

	{   64, 0x0, 0x0, 2 },
	{   68, 0x0, 0x1, 2 },
	{  125, 0x0, 0x2, 2 },
	{  128, 0x1, 0x0, 2 },
	{  136, 0x1, 0x1, 2 },
	{  192, 0x2, 0x0, 2 },
	{  204, 0x2, 0x1, 2 },

	{  250, 0x2, 0x2, 1 },
	{  256, 0x3, 0x0, 1 },
	{  272, 0x3, 0x1, 1 },
	{  384, 0x4, 0x0, 1 },
	{  408, 0x4, 0x1, 1 },
	{  375, 0x4, 0x2, 1 },
	{  512, 0x5, 0x0, 1 },
	{  544, 0x5, 0x1, 1 },
	{  500, 0x5, 0x2, 1 },
	{  768, 0x6, 0x0, 1 },
	{  816, 0x6, 0x1, 1 },
	{  750, 0x6, 0x2, 1 },
	{ 1024, 0x7, 0x0, 1 },
	{ 1088, 0x7, 0x1, 1 },
	{ 1000, 0x7, 0x2, 1 },
	{ 1408, 0x8, 0x0, 1 },
	{ 1496, 0x8, 0x1, 1 },
	{ 1536, 0x9, 0x0, 1 },
	{ 1632, 0x9, 0x1, 1 },
	{ 1500, 0x9, 0x2, 1 },

	{  250, 0x2, 0x2, 2 },
	{  256, 0x3, 0x0, 2 },
	{  272, 0x3, 0x1, 2 },
	{  384, 0x4, 0x0, 2 },
	{  408, 0x4, 0x1, 2 },
	{  375, 0x4, 0x2, 2 },
	{  512, 0x5, 0x0, 2 },
	{  544, 0x5, 0x1, 2 },
	{  500, 0x5, 0x2, 2 },
	{  768, 0x6, 0x0, 2 },
	{  816, 0x6, 0x1, 2 },
	{  750, 0x6, 0x2, 2 },
	{ 1024, 0x7, 0x0, 2 },
	{ 1088, 0x7, 0x1, 2 },
	{ 1000, 0x7, 0x2, 2 },
	{ 1408, 0x8, 0x0, 2 },
	{ 1496, 0x8, 0x1, 2 },
	{ 1536, 0x9, 0x0, 2 },
	{ 1632, 0x9, 0x1, 2 },
	{ 1500, 0x9, 0x2, 2 },
};


static struct {
	int ratio;
	int div;
} bclk_divs[] = {
	{  10,  0 },
	{  20,  2 },
	{  30,  3 },
	{  40,  4 },
	{  50,  5 },
	{  60,  7 },
	{  80,  8 },
	{ 100,  9 },
	{ 120, 11 },
	{ 160, 12 },
	{ 200, 13 },
	{ 220, 14 },
	{ 240, 15 },
	{ 300, 17 },
	{ 320, 18 },
	{ 440, 19 },
	{ 480, 20 },
};


static struct {
	int rate;
	int value;
} sample_rates[] = {
	{  8000,  0 },
	{ 11025,  1 },
	{ 12000,  2 },
	{ 16000,  3 },
	{ 22050,  4 },
	{ 24000,  5 },
	{ 32000,  6 },
	{ 44100,  7 },
	{ 48000,  8 },
	{ 88200,  9 },
	{ 96000, 10 },
	{ 0,      0 },
};

static int wm8903_startup(struct snd_pcm_substream *substream,
			  struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct wm8903_priv *wm8903 = codec->private_data;
	struct i2c_client *i2c = codec->control_data;
	struct snd_pcm_runtime *master_runtime;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		wm8903->playback_active++;
	else
		wm8903->capture_active++;

	
	if (wm8903->master_substream) {
		master_runtime = wm8903->master_substream->runtime;

		dev_dbg(&i2c->dev, "Constraining to %d bits\n",
			master_runtime->sample_bits);

		snd_pcm_hw_constraint_minmax(substream->runtime,
					     SNDRV_PCM_HW_PARAM_SAMPLE_BITS,
					     master_runtime->sample_bits,
					     master_runtime->sample_bits);

		wm8903->slave_substream = substream;
	} else
		wm8903->master_substream = substream;

	return 0;
}

static void wm8903_shutdown(struct snd_pcm_substream *substream,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct wm8903_priv *wm8903 = codec->private_data;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		wm8903->playback_active--;
	else
		wm8903->capture_active--;

	if (wm8903->master_substream == substream)
		wm8903->master_substream = wm8903->slave_substream;

	wm8903->slave_substream = NULL;
}

static int wm8903_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct wm8903_priv *wm8903 = codec->private_data;
	struct i2c_client *i2c = codec->control_data;
	int fs = params_rate(params);
	int bclk;
	int bclk_div;
	int i;
	int dsp_config;
	int clk_config;
	int best_val;
	int cur_val;
	int clk_sys;

	u16 aif1 = snd_soc_read(codec, WM8903_AUDIO_INTERFACE_1);
	u16 aif2 = snd_soc_read(codec, WM8903_AUDIO_INTERFACE_2);
	u16 aif3 = snd_soc_read(codec, WM8903_AUDIO_INTERFACE_3);
	u16 clock0 = snd_soc_read(codec, WM8903_CLOCK_RATES_0);
	u16 clock1 = snd_soc_read(codec, WM8903_CLOCK_RATES_1);
	u16 dac_digital1 = snd_soc_read(codec, WM8903_DAC_DIGITAL_1);

	if (substream == wm8903->slave_substream) {
		dev_dbg(&i2c->dev, "Ignoring hw_params for slave substream\n");
		return 0;
	}

	
	if (fs <= 24000)
		dac_digital1 |= WM8903_DAC_SB_FILT;
	else
		dac_digital1 &= ~WM8903_DAC_SB_FILT;

	
	dsp_config = 0;
	best_val = abs(sample_rates[dsp_config].rate - fs);
	for (i = 1; i < ARRAY_SIZE(sample_rates); i++) {
		cur_val = abs(sample_rates[i].rate - fs);
		if (cur_val <= best_val) {
			dsp_config = i;
			best_val = cur_val;
		}
	}

	
	if (wm8903->capture_active)
		switch (sample_rates[dsp_config].rate) {
		case 88200:
		case 96000:
			dev_err(&i2c->dev, "%dHz unsupported by ADC\n",
				fs);
			return -EINVAL;

		default:
			break;
		}

	dev_dbg(&i2c->dev, "DSP fs = %dHz\n", sample_rates[dsp_config].rate);
	clock1 &= ~WM8903_SAMPLE_RATE_MASK;
	clock1 |= sample_rates[dsp_config].value;

	aif1 &= ~WM8903_AIF_WL_MASK;
	bclk = 2 * fs;
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		bclk *= 16;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		bclk *= 20;
		aif1 |= 0x4;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		bclk *= 24;
		aif1 |= 0x8;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		bclk *= 32;
		aif1 |= 0xc;
		break;
	default:
		return -EINVAL;
	}

	dev_dbg(&i2c->dev, "MCLK = %dHz, target sample rate = %dHz\n",
		wm8903->sysclk, fs);

	
	clk_config = 0;
	best_val = abs((wm8903->sysclk /
			(clk_sys_ratios[0].mclk_div *
			 clk_sys_ratios[0].div)) - fs);
	for (i = 1; i < ARRAY_SIZE(clk_sys_ratios); i++) {
		cur_val = abs((wm8903->sysclk /
			       (clk_sys_ratios[i].mclk_div *
				clk_sys_ratios[i].div)) - fs);

		if (cur_val <= best_val) {
			clk_config = i;
			best_val = cur_val;
		}
	}

	if (clk_sys_ratios[clk_config].mclk_div == 2) {
		clock0 |= WM8903_MCLKDIV2;
		clk_sys = wm8903->sysclk / 2;
	} else {
		clock0 &= ~WM8903_MCLKDIV2;
		clk_sys = wm8903->sysclk;
	}

	clock1 &= ~(WM8903_CLK_SYS_RATE_MASK |
		    WM8903_CLK_SYS_MODE_MASK);
	clock1 |= clk_sys_ratios[clk_config].rate << WM8903_CLK_SYS_RATE_SHIFT;
	clock1 |= clk_sys_ratios[clk_config].mode << WM8903_CLK_SYS_MODE_SHIFT;

	dev_dbg(&i2c->dev, "CLK_SYS_RATE=%x, CLK_SYS_MODE=%x div=%d\n",
		clk_sys_ratios[clk_config].rate,
		clk_sys_ratios[clk_config].mode,
		clk_sys_ratios[clk_config].div);

	dev_dbg(&i2c->dev, "Actual CLK_SYS = %dHz\n", clk_sys);

	
	bclk_div = 0;
	best_val = ((clk_sys * 10) / bclk_divs[0].ratio) - bclk;
	i = 1;
	while (i < ARRAY_SIZE(bclk_divs)) {
		cur_val = ((clk_sys * 10) / bclk_divs[i].ratio) - bclk;
		if (cur_val < 0) 
			break;
		bclk_div = i;
		best_val = cur_val;
		i++;
	}

	aif2 &= ~WM8903_BCLK_DIV_MASK;
	aif3 &= ~WM8903_LRCLK_RATE_MASK;

	dev_dbg(&i2c->dev, "BCLK ratio %d for %dHz - actual BCLK = %dHz\n",
		bclk_divs[bclk_div].ratio / 10, bclk,
		(clk_sys * 10) / bclk_divs[bclk_div].ratio);

	aif2 |= bclk_divs[bclk_div].div;
	aif3 |= bclk / fs;

	snd_soc_write(codec, WM8903_CLOCK_RATES_0, clock0);
	snd_soc_write(codec, WM8903_CLOCK_RATES_1, clock1);
	snd_soc_write(codec, WM8903_AUDIO_INTERFACE_1, aif1);
	snd_soc_write(codec, WM8903_AUDIO_INTERFACE_2, aif2);
	snd_soc_write(codec, WM8903_AUDIO_INTERFACE_3, aif3);
	snd_soc_write(codec, WM8903_DAC_DIGITAL_1, dac_digital1);

	return 0;
}

#define WM8903_PLAYBACK_RATES (SNDRV_PCM_RATE_8000 |\
			       SNDRV_PCM_RATE_11025 |	\
			       SNDRV_PCM_RATE_16000 |	\
			       SNDRV_PCM_RATE_22050 |	\
			       SNDRV_PCM_RATE_32000 |	\
			       SNDRV_PCM_RATE_44100 |	\
			       SNDRV_PCM_RATE_48000 |	\
			       SNDRV_PCM_RATE_88200 |	\
			       SNDRV_PCM_RATE_96000)

#define WM8903_CAPTURE_RATES (SNDRV_PCM_RATE_8000 |\
			      SNDRV_PCM_RATE_11025 |	\
			      SNDRV_PCM_RATE_16000 |	\
			      SNDRV_PCM_RATE_22050 |	\
			      SNDRV_PCM_RATE_32000 |	\
			      SNDRV_PCM_RATE_44100 |	\
			      SNDRV_PCM_RATE_48000)

#define WM8903_FORMATS (SNDRV_PCM_FMTBIT_S16_LE |\
			SNDRV_PCM_FMTBIT_S20_3LE |\
			SNDRV_PCM_FMTBIT_S24_LE)

static struct snd_soc_dai_ops wm8903_dai_ops = {
	.startup	= wm8903_startup,
	.shutdown	= wm8903_shutdown,
	.hw_params	= wm8903_hw_params,
	.digital_mute	= wm8903_digital_mute,
	.set_fmt	= wm8903_set_dai_fmt,
	.set_sysclk	= wm8903_set_dai_sysclk,
};

struct snd_soc_dai wm8903_dai = {
	.name = "WM8903",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = WM8903_PLAYBACK_RATES,
		.formats = WM8903_FORMATS,
	},
	.capture = {
		 .stream_name = "Capture",
		 .channels_min = 2,
		 .channels_max = 2,
		 .rates = WM8903_CAPTURE_RATES,
		 .formats = WM8903_FORMATS,
	 },
	.ops = &wm8903_dai_ops,
	.symmetric_rates = 1,
};
EXPORT_SYMBOL_GPL(wm8903_dai);

static int wm8903_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	wm8903_set_bias_level(codec, SND_SOC_BIAS_OFF);

	return 0;
}

static int wm8903_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;
	struct i2c_client *i2c = codec->control_data;
	int i;
	u16 *reg_cache = codec->reg_cache;
	u16 *tmp_cache = kmemdup(reg_cache, sizeof(wm8903_reg_defaults),
				 GFP_KERNEL);

	
	wm8903_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	wm8903_set_bias_level(codec, codec->suspend_bias_level);

	
	if (tmp_cache) {
		for (i = 2; i < ARRAY_SIZE(wm8903_reg_defaults); i++)
			if (tmp_cache[i] != reg_cache[i])
				snd_soc_write(codec, i, tmp_cache[i]);
		kfree(tmp_cache);
	} else {
		dev_err(&i2c->dev, "Failed to allocate temporary cache\n");
	}

	return 0;
}

static struct snd_soc_codec *wm8903_codec;

static __devinit int wm8903_i2c_probe(struct i2c_client *i2c,
				      const struct i2c_device_id *id)
{
	struct wm8903_priv *wm8903;
	struct snd_soc_codec *codec;
	int ret;
	u16 val;

	wm8903 = kzalloc(sizeof(struct wm8903_priv), GFP_KERNEL);
	if (wm8903 == NULL)
		return -ENOMEM;

	codec = &wm8903->codec;

	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	codec->dev = &i2c->dev;
	codec->name = "WM8903";
	codec->owner = THIS_MODULE;
	codec->bias_level = SND_SOC_BIAS_OFF;
	codec->set_bias_level = wm8903_set_bias_level;
	codec->dai = &wm8903_dai;
	codec->num_dai = 1;
	codec->reg_cache_size = ARRAY_SIZE(wm8903->reg_cache);
	codec->reg_cache = &wm8903->reg_cache[0];
	codec->private_data = wm8903;
	codec->volatile_register = wm8903_volatile_register;

	i2c_set_clientdata(i2c, codec);
	codec->control_data = i2c;

	ret = snd_soc_codec_set_cache_io(codec, 8, 16, SND_SOC_I2C);
	if (ret != 0) {
		dev_err(&i2c->dev, "Failed to set cache I/O: %d\n", ret);
		goto err;
	}

	val = snd_soc_read(codec, WM8903_SW_RESET_AND_ID);
	if (val != wm8903_reg_defaults[WM8903_SW_RESET_AND_ID]) {
		dev_err(&i2c->dev,
			"Device with ID register %x is not a WM8903\n", val);
		return -ENODEV;
	}

	val = snd_soc_read(codec, WM8903_REVISION_NUMBER);
	dev_info(&i2c->dev, "WM8903 revision %d\n",
		 val & WM8903_CHIP_REV_MASK);

	wm8903_reset(codec);

	
	wm8903_set_bias_level(codec, SND_SOC_BIAS_STANDBY);

	
	val = snd_soc_read(codec, WM8903_ADC_DIGITAL_VOLUME_LEFT);
	val |= WM8903_ADCVU;
	snd_soc_write(codec, WM8903_ADC_DIGITAL_VOLUME_LEFT, val);
	snd_soc_write(codec, WM8903_ADC_DIGITAL_VOLUME_RIGHT, val);

	val = snd_soc_read(codec, WM8903_DAC_DIGITAL_VOLUME_LEFT);
	val |= WM8903_DACVU;
	snd_soc_write(codec, WM8903_DAC_DIGITAL_VOLUME_LEFT, val);
	snd_soc_write(codec, WM8903_DAC_DIGITAL_VOLUME_RIGHT, val);

	val = snd_soc_read(codec, WM8903_ANALOGUE_OUT1_LEFT);
	val |= WM8903_HPOUTVU;
	snd_soc_write(codec, WM8903_ANALOGUE_OUT1_LEFT, val);
	snd_soc_write(codec, WM8903_ANALOGUE_OUT1_RIGHT, val);

	val = snd_soc_read(codec, WM8903_ANALOGUE_OUT2_LEFT);
	val |= WM8903_LINEOUTVU;
	snd_soc_write(codec, WM8903_ANALOGUE_OUT2_LEFT, val);
	snd_soc_write(codec, WM8903_ANALOGUE_OUT2_RIGHT, val);

	val = snd_soc_read(codec, WM8903_ANALOGUE_OUT3_LEFT);
	val |= WM8903_SPKVU;
	snd_soc_write(codec, WM8903_ANALOGUE_OUT3_LEFT, val);
	snd_soc_write(codec, WM8903_ANALOGUE_OUT3_RIGHT, val);

	
	val = snd_soc_read(codec, WM8903_DAC_DIGITAL_1);
	val |= WM8903_DAC_MUTEMODE;
	snd_soc_write(codec, WM8903_DAC_DIGITAL_1, val);

	wm8903_dai.dev = &i2c->dev;
	wm8903_codec = codec;

	ret = snd_soc_register_codec(codec);
	if (ret != 0) {
		dev_err(&i2c->dev, "Failed to register codec: %d\n", ret);
		goto err;
	}

	ret = snd_soc_register_dai(&wm8903_dai);
	if (ret != 0) {
		dev_err(&i2c->dev, "Failed to register DAI: %d\n", ret);
		goto err_codec;
	}

	return ret;

err_codec:
	snd_soc_unregister_codec(codec);
err:
	wm8903_codec = NULL;
	kfree(wm8903);
	return ret;
}

static __devexit int wm8903_i2c_remove(struct i2c_client *client)
{
	struct snd_soc_codec *codec = i2c_get_clientdata(client);

	snd_soc_unregister_dai(&wm8903_dai);
	snd_soc_unregister_codec(codec);

	wm8903_set_bias_level(codec, SND_SOC_BIAS_OFF);

	kfree(codec->private_data);

	wm8903_codec = NULL;
	wm8903_dai.dev = NULL;

	return 0;
}

#ifdef CONFIG_PM
static int wm8903_i2c_suspend(struct i2c_client *client, pm_message_t msg)
{
	return snd_soc_suspend_device(&client->dev);
}

static int wm8903_i2c_resume(struct i2c_client *client)
{
	return snd_soc_resume_device(&client->dev);
}
#else
#define wm8903_i2c_suspend NULL
#define wm8903_i2c_resume NULL
#endif


static const struct i2c_device_id wm8903_i2c_id[] = {
       { "wm8903", 0 },
       { }
};
MODULE_DEVICE_TABLE(i2c, wm8903_i2c_id);

static struct i2c_driver wm8903_i2c_driver = {
	.driver = {
		.name = "WM8903",
		.owner = THIS_MODULE,
	},
	.probe    = wm8903_i2c_probe,
	.remove   = __devexit_p(wm8903_i2c_remove),
	.suspend  = wm8903_i2c_suspend,
	.resume   = wm8903_i2c_resume,
	.id_table = wm8903_i2c_id,
};

static int wm8903_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	int ret = 0;

	if (!wm8903_codec) {
		dev_err(&pdev->dev, "I2C device not yet probed\n");
		goto err;
	}

	socdev->card->codec = wm8903_codec;

	
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to create pcms\n");
		goto err;
	}

	snd_soc_add_controls(socdev->card->codec, wm8903_snd_controls,
				ARRAY_SIZE(wm8903_snd_controls));
	wm8903_add_widgets(socdev->card->codec);

	ret = snd_soc_init_card(socdev);
	if (ret < 0) {
		dev_err(&pdev->dev, "wm8903: failed to register card\n");
		goto card_err;
	}

	return ret;

card_err:
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
err:
	return ret;
}


static int wm8903_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	if (codec->control_data)
		wm8903_set_bias_level(codec, SND_SOC_BIAS_OFF);

	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_wm8903 = {
	.probe = 	wm8903_probe,
	.remove = 	wm8903_remove,
	.suspend = 	wm8903_suspend,
	.resume =	wm8903_resume,
};
EXPORT_SYMBOL_GPL(soc_codec_dev_wm8903);

static int __init wm8903_modinit(void)
{
	return i2c_add_driver(&wm8903_i2c_driver);
}
module_init(wm8903_modinit);

static void __exit wm8903_exit(void)
{
	i2c_del_driver(&wm8903_i2c_driver);
}
module_exit(wm8903_exit);

MODULE_DESCRIPTION("ASoC WM8903 driver");
MODULE_AUTHOR("Mark Brown <broonie@opensource.wolfsonmicro.cm>");
MODULE_LICENSE("GPL");
