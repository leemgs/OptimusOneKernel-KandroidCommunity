

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/tlv.h>
#include <sound/soc-dapm.h>
#include <linux/spi/spi.h>
#include "ad1836.h"


struct ad1836_priv {
	struct snd_soc_codec codec;
	u16 reg_cache[AD1836_NUM_REGS];
};

static struct snd_soc_codec *ad1836_codec;
struct snd_soc_codec_device soc_codec_dev_ad1836;
static int ad1836_register(struct ad1836_priv *ad1836);
static void ad1836_unregister(struct ad1836_priv *ad1836);


static const char *ad1836_deemp[] = {"None", "44.1kHz", "32kHz", "48kHz"};

static const struct soc_enum ad1836_deemp_enum =
	SOC_ENUM_SINGLE(AD1836_DAC_CTRL1, 8, 4, ad1836_deemp);

static const struct snd_kcontrol_new ad1836_snd_controls[] = {
	
	SOC_DOUBLE_R("DAC1 Volume", AD1836_DAC_L1_VOL,
			AD1836_DAC_R1_VOL, 0, 0x3FF, 0),
	SOC_DOUBLE_R("DAC2 Volume", AD1836_DAC_L2_VOL,
			AD1836_DAC_R2_VOL, 0, 0x3FF, 0),
	SOC_DOUBLE_R("DAC3 Volume", AD1836_DAC_L3_VOL,
			AD1836_DAC_R3_VOL, 0, 0x3FF, 0),

	
	SOC_DOUBLE("ADC1 Switch", AD1836_ADC_CTRL2, AD1836_ADCL1_MUTE,
		AD1836_ADCR1_MUTE, 1, 1),
	SOC_DOUBLE("ADC2 Switch", AD1836_ADC_CTRL2, AD1836_ADCL2_MUTE,
		AD1836_ADCR2_MUTE, 1, 1),

	
	SOC_DOUBLE("DAC1 Switch", AD1836_DAC_CTRL2, AD1836_DACL1_MUTE,
		AD1836_DACR1_MUTE, 1, 1),
	SOC_DOUBLE("DAC2 Switch", AD1836_DAC_CTRL2, AD1836_DACL2_MUTE,
		AD1836_DACR2_MUTE, 1, 1),
	SOC_DOUBLE("DAC3 Switch", AD1836_DAC_CTRL2, AD1836_DACL3_MUTE,
		AD1836_DACR3_MUTE, 1, 1),

	
	SOC_SINGLE("ADC High Pass Filter Switch", AD1836_ADC_CTRL1,
			AD1836_ADC_HIGHPASS_FILTER, 1, 0),

	
	SOC_ENUM("Playback Deemphasis", ad1836_deemp_enum),
};

static const struct snd_soc_dapm_widget ad1836_dapm_widgets[] = {
	SND_SOC_DAPM_DAC("DAC", "Playback", AD1836_DAC_CTRL1,
				AD1836_DAC_POWERDOWN, 1),
	SND_SOC_DAPM_ADC("ADC", "Capture", SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_SUPPLY("ADC_PWR", AD1836_ADC_CTRL1,
				AD1836_ADC_POWERDOWN, 1, NULL, 0),
	SND_SOC_DAPM_OUTPUT("DAC1OUT"),
	SND_SOC_DAPM_OUTPUT("DAC2OUT"),
	SND_SOC_DAPM_OUTPUT("DAC3OUT"),
	SND_SOC_DAPM_INPUT("ADC1IN"),
	SND_SOC_DAPM_INPUT("ADC2IN"),
};

static const struct snd_soc_dapm_route audio_paths[] = {
	{ "DAC", NULL, "ADC_PWR" },
	{ "ADC", NULL, "ADC_PWR" },
	{ "DAC1OUT", "DAC1 Switch", "DAC" },
	{ "DAC2OUT", "DAC2 Switch", "DAC" },
	{ "DAC3OUT", "DAC3 Switch", "DAC" },
	{ "ADC", "ADC1 Switch", "ADC1IN" },
	{ "ADC", "ADC2 Switch", "ADC2IN" },
};



static int ad1836_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	
	case SND_SOC_DAIFMT_DSP_A:
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_IB_IF:
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	
	case SND_SOC_DAIFMT_CBM_CFM:
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int ad1836_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params,
		struct snd_soc_dai *dai)
{
	int word_len = 0;

	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;

	
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		word_len = 3;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		word_len = 1;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
	case SNDRV_PCM_FORMAT_S32_LE:
		word_len = 0;
		break;
	}

	snd_soc_update_bits(codec, AD1836_DAC_CTRL1,
		AD1836_DAC_WORD_LEN_MASK, word_len);

	snd_soc_update_bits(codec, AD1836_ADC_CTRL2,
		AD1836_ADC_WORD_LEN_MASK, word_len);

	return 0;
}



#define AD1836_SPI_REG_SHFT 12
#define AD1836_SPI_READ     (1 << 11)
#define AD1836_SPI_VAL_MSK  0x3FF



static int ad1836_write_reg(struct snd_soc_codec *codec, unsigned int reg,
		unsigned int value)
{
	u16 *reg_cache = codec->reg_cache;
	int ret = 0;

	if (value != reg_cache[reg]) {
		unsigned short buf;
		struct spi_transfer t = {
			.tx_buf = &buf,
			.len = 2,
		};
		struct spi_message m;

		buf = (reg << AD1836_SPI_REG_SHFT) |
			(value & AD1836_SPI_VAL_MSK);
		spi_message_init(&m);
		spi_message_add_tail(&t, &m);
		ret = spi_sync(codec->control_data, &m);
		if (ret == 0)
			reg_cache[reg] = value;
	}

	return ret;
}


static unsigned int ad1836_read_reg_cache(struct snd_soc_codec *codec,
					  unsigned int reg)
{
	u16 *reg_cache = codec->reg_cache;

	if (reg >= codec->reg_cache_size)
		return -EINVAL;

	return reg_cache[reg];
}

static int __devinit ad1836_spi_probe(struct spi_device *spi)
{
	struct snd_soc_codec *codec;
	struct ad1836_priv *ad1836;

	ad1836 = kzalloc(sizeof(struct ad1836_priv), GFP_KERNEL);
	if (ad1836 == NULL)
		return -ENOMEM;

	codec = &ad1836->codec;
	codec->control_data = spi;
	codec->dev = &spi->dev;

	dev_set_drvdata(&spi->dev, ad1836);

	return ad1836_register(ad1836);
}

static int __devexit ad1836_spi_remove(struct spi_device *spi)
{
	struct ad1836_priv *ad1836 = dev_get_drvdata(&spi->dev);

	ad1836_unregister(ad1836);
	return 0;
}

static struct spi_driver ad1836_spi_driver = {
	.driver = {
		.name	= "ad1836",
		.owner	= THIS_MODULE,
	},
	.probe		= ad1836_spi_probe,
	.remove		= __devexit_p(ad1836_spi_remove),
};

static struct snd_soc_dai_ops ad1836_dai_ops = {
	.hw_params = ad1836_hw_params,
	.set_fmt = ad1836_set_dai_fmt,
};


struct snd_soc_dai ad1836_dai = {
	.name = "AD1836",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 6,
		.rates = SNDRV_PCM_RATE_48000,
		.formats = SNDRV_PCM_FMTBIT_S32_LE | SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 2,
		.channels_max = 4,
		.rates = SNDRV_PCM_RATE_48000,
		.formats = SNDRV_PCM_FMTBIT_S32_LE | SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE,
	},
	.ops = &ad1836_dai_ops,
};
EXPORT_SYMBOL_GPL(ad1836_dai);

static int ad1836_register(struct ad1836_priv *ad1836)
{
	int ret;
	struct snd_soc_codec *codec = &ad1836->codec;

	if (ad1836_codec) {
		dev_err(codec->dev, "Another ad1836 is registered\n");
		return -EINVAL;
	}

	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);
	codec->private_data = ad1836;
	codec->reg_cache = ad1836->reg_cache;
	codec->reg_cache_size = AD1836_NUM_REGS;
	codec->name = "AD1836";
	codec->owner = THIS_MODULE;
	codec->dai = &ad1836_dai;
	codec->num_dai = 1;
	codec->write = ad1836_write_reg;
	codec->read = ad1836_read_reg_cache;
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	ad1836_dai.dev = codec->dev;
	ad1836_codec = codec;

	
	
	codec->write(codec, AD1836_DAC_CTRL1, 0x300);
	
	codec->write(codec, AD1836_DAC_CTRL2, 0x0);
	
	codec->write(codec, AD1836_ADC_CTRL1, 0x100);
	
	codec->write(codec, AD1836_ADC_CTRL2, 0x180);
	
	codec->write(codec, AD1836_ADC_CTRL3, 0x3A);
	
	codec->write(codec, AD1836_DAC_L1_VOL, 0x3FF);
	codec->write(codec, AD1836_DAC_R1_VOL, 0x3FF);
	codec->write(codec, AD1836_DAC_L2_VOL, 0x3FF);
	codec->write(codec, AD1836_DAC_R2_VOL, 0x3FF);
	codec->write(codec, AD1836_DAC_L3_VOL, 0x3FF);
	codec->write(codec, AD1836_DAC_R3_VOL, 0x3FF);

	ret = snd_soc_register_codec(codec);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register codec: %d\n", ret);
		kfree(ad1836);
		return ret;
	}

	ret = snd_soc_register_dai(&ad1836_dai);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to register DAI: %d\n", ret);
		snd_soc_unregister_codec(codec);
		kfree(ad1836);
		return ret;
	}

	return 0;
}

static void ad1836_unregister(struct ad1836_priv *ad1836)
{
	snd_soc_unregister_dai(&ad1836_dai);
	snd_soc_unregister_codec(&ad1836->codec);
	kfree(ad1836);
	ad1836_codec = NULL;
}

static int ad1836_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec;
	int ret = 0;

	if (ad1836_codec == NULL) {
		dev_err(&pdev->dev, "Codec device not registered\n");
		return -ENODEV;
	}

	socdev->card->codec = ad1836_codec;
	codec = ad1836_codec;

	
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		dev_err(codec->dev, "failed to create pcms: %d\n", ret);
		goto pcm_err;
	}

	snd_soc_add_controls(codec, ad1836_snd_controls,
			     ARRAY_SIZE(ad1836_snd_controls));
	snd_soc_dapm_new_controls(codec, ad1836_dapm_widgets,
				  ARRAY_SIZE(ad1836_dapm_widgets));
	snd_soc_dapm_add_routes(codec, audio_paths, ARRAY_SIZE(audio_paths));
	snd_soc_dapm_new_widgets(codec);

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


static int ad1836_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);

	snd_soc_free_pcms(socdev);
	snd_soc_dapm_free(socdev);

	return 0;
}

struct snd_soc_codec_device soc_codec_dev_ad1836 = {
	.probe = 	ad1836_probe,
	.remove = 	ad1836_remove,
};
EXPORT_SYMBOL_GPL(soc_codec_dev_ad1836);

static int __init ad1836_init(void)
{
	int ret;

	ret = spi_register_driver(&ad1836_spi_driver);
	if (ret != 0) {
		printk(KERN_ERR "Failed to register ad1836 SPI driver: %d\n",
				ret);
	}

	return ret;
}
module_init(ad1836_init);

static void __exit ad1836_exit(void)
{
	spi_unregister_driver(&ad1836_spi_driver);
}
module_exit(ad1836_exit);

MODULE_DESCRIPTION("ASoC ad1836 driver");
MODULE_AUTHOR("Barry Song <21cnbao@gmail.com>");
MODULE_LICENSE("GPL");
