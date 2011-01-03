

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>
#include <sound/uda1380.h>

#include "uda1380.h"

static struct snd_soc_codec *uda1380_codec;


struct uda1380_priv {
	struct snd_soc_codec codec;
	u16 reg_cache[UDA1380_CACHEREGNUM];
	unsigned int dac_clk;
	struct work_struct work;
};


static const u16 uda1380_reg[UDA1380_CACHEREGNUM] = {
	0x0502, 0x0000, 0x0000, 0x3f3f,
	0x0202, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0xff00, 0x0000, 0x4800,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x8000, 0x0002, 0x0000,
};

static unsigned long uda1380_cache_dirty;


static inline unsigned int uda1380_read_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg)
{
	u16 *cache = codec->reg_cache;
	if (reg == UDA1380_RESET)
		return 0;
	if (reg >= UDA1380_CACHEREGNUM)
		return -1;
	return cache[reg];
}


static inline void uda1380_write_reg_cache(struct snd_soc_codec *codec,
	u16 reg, unsigned int value)
{
	u16 *cache = codec->reg_cache;

	if (reg >= UDA1380_CACHEREGNUM)
		return;
	if ((reg >= 0x10) && (cache[reg] != value))
		set_bit(reg - 0x10, &uda1380_cache_dirty);
	cache[reg] = value;
}


static int uda1380_write(struct snd_soc_codec *codec, unsigned int reg,
	unsigned int value)
{
	u8 data[3];

	
	data[0] = reg;
	data[1] = (value & 0xff00) >> 8;
	data[2] = value & 0x00ff;

	uda1380_write_reg_cache(codec, reg, value);

	
	if (!codec->active && (reg >= UDA1380_MVOL))
		return 0;
	pr_debug("uda1380: hw write %x val %x\n", reg, value);
	if (codec->hw_write(codec->control_data, data, 3) == 3) {
		unsigned int val;
		i2c_master_send(codec->control_data, data, 1);
		i2c_master_recv(codec->control_data, data, 2);
		val = (data[0]<<8) | data[1];
		if (val != value) {
			pr_debug("uda1380: READ BACK VAL %x\n",
					(data[0]<<8) | data[1]);
			return -EIO;
		}
		if (reg >= 0x10)
			clear_bit(reg - 0x10, &uda1380_cache_dirty);
		return 0;
	} else
		return -EIO;
}

#define uda1380_reset(c)	uda1380_write(c, UDA1380_RESET, 0)

static void uda1380_flush_work(struct work_struct *work)
{
	int bit, reg;

	for_each_bit(bit, &uda1380_cache_dirty, UDA1380_CACHEREGNUM - 0x10) {
		reg = 0x10 + bit;
		pr_debug("uda1380: flush reg %x val %x:\n", reg,
				uda1380_read_reg_cache(uda1380_codec, reg));
		uda1380_write(uda1380_codec, reg,
				uda1380_read_reg_cache(uda1380_codec, reg));
		clear_bit(bit, &uda1380_cache_dirty);
	}
}


static const char *uda1380_deemp[] = {
	"None",
	"32kHz",
	"44.1kHz",
	"48kHz",
	"96kHz",
};
static const char *uda1380_input_sel[] = {
	"Line",
	"Mic + Line R",
	"Line L",
	"Mic",
};
static const char *uda1380_output_sel[] = {
	"DAC",
	"Analog Mixer",
};
static const char *uda1380_spf_mode[] = {
	"Flat",
	"Minimum1",
	"Minimum2",
	"Maximum"
};
static const char *uda1380_capture_sel[] = {
	"ADC",
	"Digital Mixer"
};
static const char *uda1380_sel_ns[] = {
	"3rd-order",
	"5th-order"
};
static const char *uda1380_mix_control[] = {
	"off",
	"PCM only",
	"before sound processing",
	"after sound processing"
};
static const char *uda1380_sdet_setting[] = {
	"3200",
	"4800",
	"9600",
	"19200"
};
static const char *uda1380_os_setting[] = {
	"single-speed",
	"double-speed (no mixing)",
	"quad-speed (no mixing)"
};

static const struct soc_enum uda1380_deemp_enum[] = {
	SOC_ENUM_SINGLE(UDA1380_DEEMP, 8, 5, uda1380_deemp),
	SOC_ENUM_SINGLE(UDA1380_DEEMP, 0, 5, uda1380_deemp),
};
static const struct soc_enum uda1380_input_sel_enum =
	SOC_ENUM_SINGLE(UDA1380_ADC, 2, 4, uda1380_input_sel);		
static const struct soc_enum uda1380_output_sel_enum =
	SOC_ENUM_SINGLE(UDA1380_PM, 7, 2, uda1380_output_sel);		
static const struct soc_enum uda1380_spf_enum =
	SOC_ENUM_SINGLE(UDA1380_MODE, 14, 4, uda1380_spf_mode);		
static const struct soc_enum uda1380_capture_sel_enum =
	SOC_ENUM_SINGLE(UDA1380_IFACE, 6, 2, uda1380_capture_sel);	
static const struct soc_enum uda1380_sel_ns_enum =
	SOC_ENUM_SINGLE(UDA1380_MIXER, 14, 2, uda1380_sel_ns);		
static const struct soc_enum uda1380_mix_enum =
	SOC_ENUM_SINGLE(UDA1380_MIXER, 12, 4, uda1380_mix_control);	
static const struct soc_enum uda1380_sdet_enum =
	SOC_ENUM_SINGLE(UDA1380_MIXER, 4, 4, uda1380_sdet_setting);	
static const struct soc_enum uda1380_os_enum =
	SOC_ENUM_SINGLE(UDA1380_MIXER, 0, 3, uda1380_os_setting);	


static DECLARE_TLV_DB_SCALE(amix_tlv, -4950, 150, 1);


static const unsigned int mvol_tlv[] = {
	TLV_DB_RANGE_HEAD(3),
	0, 15, TLV_DB_SCALE_ITEM(-8200, 100, 1),
	16, 43, TLV_DB_SCALE_ITEM(-6600, 50, 0),
	44, 252, TLV_DB_SCALE_ITEM(-5200, 25, 0),
};


static const unsigned int vc_tlv[] = {
	TLV_DB_RANGE_HEAD(4),
	0, 7, TLV_DB_SCALE_ITEM(-7800, 150, 1),
	8, 15, TLV_DB_SCALE_ITEM(-6600, 75, 0),
	16, 43, TLV_DB_SCALE_ITEM(-6000, 50, 0),
	44, 228, TLV_DB_SCALE_ITEM(-4600, 25, 0),
};


static DECLARE_TLV_DB_SCALE(tr_tlv, 0, 200, 0);


static DECLARE_TLV_DB_SCALE(bb_tlv, 0, 200, 0);


static DECLARE_TLV_DB_SCALE(dec_tlv, -6400, 50, 1);


static DECLARE_TLV_DB_SCALE(pga_tlv, 0, 300, 0);


static DECLARE_TLV_DB_SCALE(vga_tlv, 0, 200, 0);

static const struct snd_kcontrol_new uda1380_snd_controls[] = {
	SOC_DOUBLE_TLV("Analog Mixer Volume", UDA1380_AMIX, 0, 8, 44, 1, amix_tlv),	
	SOC_DOUBLE_TLV("Master Playback Volume", UDA1380_MVOL, 0, 8, 252, 1, mvol_tlv),	
	SOC_SINGLE_TLV("ADC Playback Volume", UDA1380_MIXVOL, 8, 228, 1, vc_tlv),	
	SOC_SINGLE_TLV("PCM Playback Volume", UDA1380_MIXVOL, 0, 228, 1, vc_tlv),	
	SOC_ENUM("Sound Processing Filter", uda1380_spf_enum),				
	SOC_DOUBLE_TLV("Tone Control - Treble", UDA1380_MODE, 4, 12, 3, 0, tr_tlv), 	
	SOC_DOUBLE_TLV("Tone Control - Bass", UDA1380_MODE, 0, 8, 15, 0, bb_tlv),	
	SOC_SINGLE("Master Playback Switch", UDA1380_DEEMP, 14, 1, 1),		
	SOC_SINGLE("ADC Playback Switch", UDA1380_DEEMP, 11, 1, 1),		
	SOC_ENUM("ADC Playback De-emphasis", uda1380_deemp_enum[0]),		
	SOC_SINGLE("PCM Playback Switch", UDA1380_DEEMP, 3, 1, 1),		
	SOC_ENUM("PCM Playback De-emphasis", uda1380_deemp_enum[1]),		
	SOC_SINGLE("DAC Polarity inverting Switch", UDA1380_MIXER, 15, 1, 0),	
	SOC_ENUM("Noise Shaper", uda1380_sel_ns_enum),				
	SOC_ENUM("Digital Mixer Signal Control", uda1380_mix_enum),		
	SOC_SINGLE("Silence Detector Switch", UDA1380_MIXER, 6, 1, 0),		
	SOC_ENUM("Silence Detector Setting", uda1380_sdet_enum),		
	SOC_ENUM("Oversampling Input", uda1380_os_enum),			
	SOC_DOUBLE_S8_TLV("ADC Capture Volume", UDA1380_DEC, -128, 48, dec_tlv),	
	SOC_SINGLE("ADC Capture Switch", UDA1380_PGA, 15, 1, 1),		
	SOC_DOUBLE_TLV("Line Capture Volume", UDA1380_PGA, 0, 8, 8, 0, pga_tlv), 
	SOC_SINGLE("ADC Polarity inverting Switch", UDA1380_ADC, 12, 1, 0),	
	SOC_SINGLE_TLV("Mic Capture Volume", UDA1380_ADC, 8, 15, 0, vga_tlv),	
	SOC_SINGLE("DC Filter Bypass Switch", UDA1380_ADC, 1, 1, 0),		
	SOC_SINGLE("DC Filter Enable Switch", UDA1380_ADC, 0, 1, 0),		
	SOC_SINGLE("AGC Timing", UDA1380_AGC, 8, 7, 0),			
	SOC_SINGLE("AGC Target level", UDA1380_AGC, 2, 3, 1),			
	
	SOC_SINGLE("AGC Switch", UDA1380_AGC, 0, 1, 0),
};


static const struct snd_kcontrol_new uda1380_input_mux_control =
	SOC_DAPM_ENUM("Route", uda1380_input_sel_enum);


static const struct snd_kcontrol_new uda1380_output_mux_control =
	SOC_DAPM_ENUM("Route", uda1380_output_sel_enum);


static const struct snd_kcontrol_new uda1380_capture_mux_control =
	SOC_DAPM_ENUM("Route", uda1380_capture_sel_enum);


static const struct snd_soc_dapm_widget uda1380_dapm_widgets[] = {
	SND_SOC_DAPM_MUX("Input Mux", SND_SOC_NOPM, 0, 0,
		&uda1380_input_mux_control),
	SND_SOC_DAPM_MUX("Output Mux", SND_SOC_NOPM, 0, 0,
		&uda1380_output_mux_control),
	SND_SOC_DAPM_MUX("Capture Mux", SND_SOC_NOPM, 0, 0,
		&uda1380_capture_mux_control),
	SND_SOC_DAPM_PGA("Left PGA", UDA1380_PM, 3, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Right PGA", UDA1380_PM, 1, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Mic LNA", UDA1380_PM, 4, 0, NULL, 0),
	SND_SOC_DAPM_ADC("Left ADC", "Left Capture", UDA1380_PM, 2, 0),
	SND_SOC_DAPM_ADC("Right ADC", "Right Capture", UDA1380_PM, 0, 0),
	SND_SOC_DAPM_INPUT("VINM"),
	SND_SOC_DAPM_INPUT("VINL"),
	SND_SOC_DAPM_INPUT("VINR"),
	SND_SOC_DAPM_MIXER("Analog Mixer", UDA1380_PM, 6, 0, NULL, 0),
	SND_SOC_DAPM_OUTPUT("VOUTLHP"),
	SND_SOC_DAPM_OUTPUT("VOUTRHP"),
	SND_SOC_DAPM_OUTPUT("VOUTL"),
	SND_SOC_DAPM_OUTPUT("VOUTR"),
	SND_SOC_DAPM_DAC("DAC", "Playback", UDA1380_PM, 10, 0),
	SND_SOC_DAPM_PGA("HeadPhone Driver", UDA1380_PM, 13, 0, NULL, 0),
};

static const struct snd_soc_dapm_route audio_map[] = {

	
	{"HeadPhone Driver", NULL, "Output Mux"},
	{"VOUTR", NULL, "Output Mux"},
	{"VOUTL", NULL, "Output Mux"},

	{"Analog Mixer", NULL, "VINR"},
	{"Analog Mixer", NULL, "VINL"},
	{"Analog Mixer", NULL, "DAC"},

	{"Output Mux", "DAC", "DAC"},
	{"Output Mux", "Analog Mixer", "Analog Mixer"},

	

	
	{"VOUTLHP", NULL, "HeadPhone Driver"},
	{"VOUTRHP", NULL, "HeadPhone Driver"},

	
	{"Left ADC", NULL, "Input Mux"},
	{"Input Mux", "Mic", "Mic LNA"},
	{"Input Mux", "Mic + Line R", "Mic LNA"},
	{"Input Mux", "Line L", "Left PGA"},
	{"Input Mux", "Line", "Left PGA"},

	
	{"Right ADC", "Mic + Line R", "Right PGA"},
	{"Right ADC", "Line", "Right PGA"},

	
	{"Mic LNA", NULL, "VINM"},
	{"Left PGA", NULL, "VINL"},
	{"Right PGA", NULL, "VINR"},
};

static int uda1380_add_widgets(struct snd_soc_codec *codec)
{
	snd_soc_dapm_new_controls(codec, uda1380_dapm_widgets,
				  ARRAY_SIZE(uda1380_dapm_widgets));

	snd_soc_dapm_add_routes(codec, audio_map, ARRAY_SIZE(audio_map));

	snd_soc_dapm_new_widgets(codec);
	return 0;
}

static int uda1380_set_dai_fmt_both(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	int iface;

	
	iface = uda1380_read_reg_cache(codec, UDA1380_IFACE);
	iface &= ~(R01_SFORI_MASK | R01_SIM | R01_SFORO_MASK);

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iface |= R01_SFORI_I2S | R01_SFORO_I2S;
		break;
	case SND_SOC_DAIFMT_LSB:
		iface |= R01_SFORI_LSB16 | R01_SFORO_LSB16;
		break;
	case SND_SOC_DAIFMT_MSB:
		iface |= R01_SFORI_MSB | R01_SFORO_MSB;
	}

	
	if ((fmt & SND_SOC_DAIFMT_MASTER_MASK) != SND_SOC_DAIFMT_CBS_CFS)
		return -EINVAL;

	uda1380_write(codec, UDA1380_IFACE, iface);

	return 0;
}

static int uda1380_set_dai_fmt_playback(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	int iface;

	
	iface = uda1380_read_reg_cache(codec, UDA1380_IFACE);
	iface &= ~R01_SFORI_MASK;

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iface |= R01_SFORI_I2S;
		break;
	case SND_SOC_DAIFMT_LSB:
		iface |= R01_SFORI_LSB16;
		break;
	case SND_SOC_DAIFMT_MSB:
		iface |= R01_SFORI_MSB;
	}

	
	if ((fmt & SND_SOC_DAIFMT_MASTER_MASK) != SND_SOC_DAIFMT_CBS_CFS)
		return -EINVAL;

	uda1380_write(codec, UDA1380_IFACE, iface);

	return 0;
}

static int uda1380_set_dai_fmt_capture(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	int iface;

	
	iface = uda1380_read_reg_cache(codec, UDA1380_IFACE);
	iface &= ~(R01_SIM | R01_SFORO_MASK);

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		iface |= R01_SFORO_I2S;
		break;
	case SND_SOC_DAIFMT_LSB:
		iface |= R01_SFORO_LSB16;
		break;
	case SND_SOC_DAIFMT_MSB:
		iface |= R01_SFORO_MSB;
	}

	if ((fmt & SND_SOC_DAIFMT_MASTER_MASK) == SND_SOC_DAIFMT_CBM_CFM)
		iface |= R01_SIM;

	uda1380_write(codec, UDA1380_IFACE, iface);

	return 0;
}

static int uda1380_trigger(struct snd_pcm_substream *substream, int cmd,
		struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct uda1380_priv *uda1380 = codec->private_data;
	int mixer = uda1380_read_reg_cache(codec, UDA1380_MIXER);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		uda1380_write_reg_cache(codec, UDA1380_MIXER,
					mixer & ~R14_SILENCE);
		schedule_work(&uda1380->work);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		uda1380_write_reg_cache(codec, UDA1380_MIXER,
					mixer | R14_SILENCE);
		schedule_work(&uda1380->work);
		break;
	}
	return 0;
}

static int uda1380_pcm_hw_params(struct snd_pcm_substream *substream,
				 struct snd_pcm_hw_params *params,
				 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	u16 clk = uda1380_read_reg_cache(codec, UDA1380_CLK);

	
	if (clk & R00_DAC_CLK) {
		int rate = params_rate(params);
		u16 pm = uda1380_read_reg_cache(codec, UDA1380_PM);
		clk &= ~0x3; 
		switch (rate) {
		case 6250 ... 12500:
			clk |= 0x0;
			break;
		case 12501 ... 25000:
			clk |= 0x1;
			break;
		case 25001 ... 50000:
			clk |= 0x2;
			break;
		case 50001 ... 100000:
			clk |= 0x3;
			break;
		}
		uda1380_write(codec, UDA1380_PM, R02_PON_PLL | pm);
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		clk |= R00_EN_DAC | R00_EN_INT;
	else
		clk |= R00_EN_ADC | R00_EN_DEC;

	uda1380_write(codec, UDA1380_CLK, clk);
	return 0;
}

static void uda1380_pcm_shutdown(struct snd_pcm_substream *substream,
				 struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	u16 clk = uda1380_read_reg_cache(codec, UDA1380_CLK);

	
	if (clk & R00_DAC_CLK) {
		u16 pm = uda1380_read_reg_cache(codec, UDA1380_PM);
		uda1380_write(codec, UDA1380_PM, ~R02_PON_PLL & pm);
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		clk &= ~(R00_EN_DAC | R00_EN_INT);
	else
		clk &= ~(R00_EN_ADC | R00_EN_DEC);

	uda1380_write(codec, UDA1380_CLK, clk);
}

static int uda1380_set_bias_level(struct snd_soc_codec *codec,
	enum snd_soc_bias_level level)
{
	int pm = uda1380_read_reg_cache(codec, UDA1380_PM);

	switch (level) {
	case SND_SOC_BIAS_ON:
	case SND_SOC_BIAS_PREPARE:
		uda1380_write(codec, UDA1380_PM, R02_PON_BIAS | pm);
		break;
	case SND_SOC_BIAS_STANDBY:
		uda1380_write(codec, UDA1380_PM, R02_PON_BIAS);
		break;
	case SND_SOC_BIAS_OFF:
		uda1380_write(codec, UDA1380_PM, 0x0);
		break;
	}
	codec->bias_level = level;
	return 0;
}

#define UDA1380_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
		       SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |\
		       SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000)

static struct snd_soc_dai_ops uda1380_dai_ops = {
	.hw_params	= uda1380_pcm_hw_params,
	.shutdown	= uda1380_pcm_shutdown,
	.trigger	= uda1380_trigger,
	.set_fmt	= uda1380_set_dai_fmt_both,
};

static struct snd_soc_dai_ops uda1380_dai_ops_playback = {
	.hw_params	= uda1380_pcm_hw_params,
	.shutdown	= uda1380_pcm_shutdown,
	.trigger	= uda1380_trigger,
	.set_fmt	= uda1380_set_dai_fmt_playback,
};

static struct snd_soc_dai_ops uda1380_dai_ops_capture = {
	.hw_params	= uda1380_pcm_hw_params,
	.shutdown	= uda1380_pcm_shutdown,
	.trigger	= uda1380_trigger,
	.set_fmt	= uda1380_set_dai_fmt_capture,
};

struct snd_soc_dai uda1380_dai[] = {
{
	.name = "UDA1380",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = UDA1380_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = UDA1380_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops = &uda1380_dai_ops,
},
{ 
	.name = "UDA1380",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = UDA1380_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
	.ops = &uda1380_dai_ops_playback,
},
{ 
	.name = "UDA1380",
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = UDA1380_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,
	},
	.ops = &uda1380_dai_ops_capture,
},
};
EXPORT_SYMBOL_GPL(uda1380_dai);

static int uda1380_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	uda1380_set_bias_level(codec, SND_SOC_BIAS_OFF);
	return 0;
}

static int uda1380_resume(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;
	int i;
	u8 data[2];
	u16 *cache = codec->reg_cache;

	
	for (i = 0; i < ARRAY_SIZE(uda1380_reg); i++) {
		data[0] = (i << 1) | ((cache[i] >> 8) & 0x0001);
		data[1] = cache[i] & 0x00ff;
		codec->hw_write(codec->control_data, data, 2);
	}
	uda1380_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	uda1380_set_bias_level(codec, codec->suspend_bias_level);
	return 0;
}

static int uda1380_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec;
	struct uda1380_platform_data *pdata;
	int ret = 0;

	if (uda1380_codec == NULL) {
		dev_err(&pdev->dev, "Codec device not registered\n");
		return -ENODEV;
	}

	socdev->card->codec = uda1380_codec;
	codec = uda1380_codec;
	pdata = codec->dev->platform_data;

	
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		dev_err(codec->dev, "failed to create pcms: %d\n", ret);
		goto pcm_err;
	}

	
	uda1380_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
	
	switch (pdata->dac_clk) {
	case UDA1380_DAC_CLK_SYSCLK:
		uda1380_write(codec, UDA1380_CLK, 0);
		break;
	case UDA1380_DAC_CLK_WSPLL:
		uda1380_write(codec, UDA1380_CLK, R00_DAC_CLK);
		break;
	}

	snd_soc_add_controls(codec, uda1380_snd_controls,
				ARRAY_SIZE(uda1380_snd_controls));
	uda1380_add_widgets(codec);
	ret = snd_soc_init_card(socdev);
	if (ret < 0) {
		dev_err(codec->dev, "failed to register card: %d\n", ret);
		goto card_err;
	}

	return ret;

card_err:
	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);
pcm_err:
	return ret;
}


static int uda1380_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = socdev->card->codec;

	if (codec->control_data)
		uda1380_set_bias_level(codec, SND_SOC_BIAS_OFF);

	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_uda1380 = {
	.probe = 	uda1380_probe,
	.remove = 	uda1380_remove,
	.suspend = 	uda1380_suspend,
	.resume =	uda1380_resume,
};
EXPORT_SYMBOL_GPL(soc_codec_dev_uda1380);

static int uda1380_register(struct uda1380_priv *uda1380)
{
	int ret, i;
	struct snd_soc_codec *codec = &uda1380->codec;
	struct uda1380_platform_data *pdata = codec->dev->platform_data;

	if (uda1380_codec) {
		dev_err(codec->dev, "Another UDA1380 is registered\n");
		return -EINVAL;
	}

	if (!pdata || !pdata->gpio_power || !pdata->gpio_reset)
		return -EINVAL;

	ret = gpio_request(pdata->gpio_power, "uda1380 power");
	if (ret)
		goto err_out;
	ret = gpio_request(pdata->gpio_reset, "uda1380 reset");
	if (ret)
		goto err_gpio;

	gpio_direction_output(pdata->gpio_power, 1);

	
	gpio_direction_output(pdata->gpio_reset, 1);
	udelay(5);
	gpio_set_value(pdata->gpio_reset, 0);

	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	codec->private_data = uda1380;
	codec->name = "UDA1380";
	codec->owner = THIS_MODULE;
	codec->read = uda1380_read_reg_cache;
	codec->write = uda1380_write;
	codec->bias_level = SND_SOC_BIAS_OFF;
	codec->set_bias_level = uda1380_set_bias_level;
	codec->dai = uda1380_dai;
	codec->num_dai = ARRAY_SIZE(uda1380_dai);
	codec->reg_cache_size = ARRAY_SIZE(uda1380_reg);
	codec->reg_cache = &uda1380->reg_cache;
	codec->reg_cache_step = 1;

	memcpy(codec->reg_cache, uda1380_reg, sizeof(uda1380_reg));

	ret = uda1380_reset(codec);
	if (ret < 0) {
		dev_err(codec->dev, "Failed to issue reset\n");
		goto err_reset;
	}

	INIT_WORK(&uda1380->work, uda1380_flush_work);

	for (i = 0; i < ARRAY_SIZE(uda1380_dai); i++)
		uda1380_dai[i].dev = codec->dev;

	uda1380_codec = codec;

	ret = snd_soc_register_codec(codec);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register codec: %d\n", ret);
		goto err_reset;
	}

	ret = snd_soc_register_dais(uda1380_dai, ARRAY_SIZE(uda1380_dai));
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register DAIs: %d\n", ret);
		goto err_dai;
	}

	return 0;

err_dai:
	snd_soc_unregister_codec(codec);
err_reset:
	gpio_set_value(pdata->gpio_power, 0);
	gpio_free(pdata->gpio_reset);
err_gpio:
	gpio_free(pdata->gpio_power);
err_out:
	return ret;
}

static void uda1380_unregister(struct uda1380_priv *uda1380)
{
	struct snd_soc_codec *codec = &uda1380->codec;
	struct uda1380_platform_data *pdata = codec->dev->platform_data;

	snd_soc_unregister_dais(uda1380_dai, ARRAY_SIZE(uda1380_dai));
	snd_soc_unregister_codec(&uda1380->codec);

	gpio_set_value(pdata->gpio_power, 0);
	gpio_free(pdata->gpio_reset);
	gpio_free(pdata->gpio_power);

	kfree(uda1380);
	uda1380_codec = NULL;
}

#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
static __devinit int uda1380_i2c_probe(struct i2c_client *i2c,
				      const struct i2c_device_id *id)
{
	struct uda1380_priv *uda1380;
	struct snd_soc_codec *codec;
	int ret;

	uda1380 = kzalloc(sizeof(struct uda1380_priv), GFP_KERNEL);
	if (uda1380 == NULL)
		return -ENOMEM;

	codec = &uda1380->codec;
	codec->hw_write = (hw_write_t)i2c_master_send;

	i2c_set_clientdata(i2c, uda1380);
	codec->control_data = i2c;

	codec->dev = &i2c->dev;

	ret = uda1380_register(uda1380);
	if (ret != 0)
		kfree(uda1380);

	return ret;
}

static int __devexit uda1380_i2c_remove(struct i2c_client *i2c)
{
	struct uda1380_priv *uda1380 = i2c_get_clientdata(i2c);
	uda1380_unregister(uda1380);
	return 0;
}

static const struct i2c_device_id uda1380_i2c_id[] = {
	{ "uda1380", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, uda1380_i2c_id);

static struct i2c_driver uda1380_i2c_driver = {
	.driver = {
		.name =  "UDA1380 I2C Codec",
		.owner = THIS_MODULE,
	},
	.probe =    uda1380_i2c_probe,
	.remove =   __devexit_p(uda1380_i2c_remove),
	.id_table = uda1380_i2c_id,
};
#endif

static int __init uda1380_modinit(void)
{
	int ret;
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	ret = i2c_add_driver(&uda1380_i2c_driver);
	if (ret != 0)
		pr_err("Failed to register UDA1380 I2C driver: %d\n", ret);
#endif
	return 0;
}
module_init(uda1380_modinit);

static void __exit uda1380_exit(void)
{
#if defined(CONFIG_I2C) || defined(CONFIG_I2C_MODULE)
	i2c_del_driver(&uda1380_i2c_driver);
#endif
}
module_exit(uda1380_exit);

MODULE_AUTHOR("Giorgio Padrin");
MODULE_DESCRIPTION("Audio support for codec Philips UDA1380");
MODULE_LICENSE("GPL");
