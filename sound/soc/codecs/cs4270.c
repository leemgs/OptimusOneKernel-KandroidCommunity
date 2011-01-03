

#include <linux/module.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#include "cs4270.h"


#define CS4270_FORMATS (SNDRV_PCM_FMTBIT_S8      | \
			SNDRV_PCM_FMTBIT_S16_LE  | SNDRV_PCM_FMTBIT_S16_BE  | \
			SNDRV_PCM_FMTBIT_S18_3LE | SNDRV_PCM_FMTBIT_S18_3BE | \
			SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S20_3BE | \
			SNDRV_PCM_FMTBIT_S24_3LE | SNDRV_PCM_FMTBIT_S24_3BE | \
			SNDRV_PCM_FMTBIT_S24_LE  | SNDRV_PCM_FMTBIT_S24_BE)


#define CS4270_CHIPID	0x01	
#define CS4270_PWRCTL	0x02	
#define CS4270_MODE	0x03	
#define CS4270_FORMAT	0x04	
#define CS4270_TRANS	0x05	
#define CS4270_MUTE	0x06	
#define CS4270_VOLA	0x07	
#define CS4270_VOLB	0x08	

#define CS4270_FIRSTREG	0x01
#define CS4270_LASTREG	0x08
#define CS4270_NUMREGS	(CS4270_LASTREG - CS4270_FIRSTREG + 1)
#define CS4270_I2C_INCR	0x80


#define CS4270_CHIPID_ID	0xF0
#define CS4270_CHIPID_REV	0x0F
#define CS4270_PWRCTL_FREEZE	0x80
#define CS4270_PWRCTL_PDN_ADC	0x20
#define CS4270_PWRCTL_PDN_DAC	0x02
#define CS4270_PWRCTL_PDN	0x01
#define CS4270_PWRCTL_PDN_ALL	\
	(CS4270_PWRCTL_PDN_ADC | CS4270_PWRCTL_PDN_DAC | CS4270_PWRCTL_PDN)
#define CS4270_MODE_SPEED_MASK	0x30
#define CS4270_MODE_1X		0x00
#define CS4270_MODE_2X		0x10
#define CS4270_MODE_4X		0x20
#define CS4270_MODE_SLAVE	0x30
#define CS4270_MODE_DIV_MASK	0x0E
#define CS4270_MODE_DIV1	0x00
#define CS4270_MODE_DIV15	0x02
#define CS4270_MODE_DIV2	0x04
#define CS4270_MODE_DIV3	0x06
#define CS4270_MODE_DIV4	0x08
#define CS4270_MODE_POPGUARD	0x01
#define CS4270_FORMAT_FREEZE_A	0x80
#define CS4270_FORMAT_FREEZE_B	0x40
#define CS4270_FORMAT_LOOPBACK	0x20
#define CS4270_FORMAT_DAC_MASK	0x18
#define CS4270_FORMAT_DAC_LJ	0x00
#define CS4270_FORMAT_DAC_I2S	0x08
#define CS4270_FORMAT_DAC_RJ16	0x18
#define CS4270_FORMAT_DAC_RJ24	0x10
#define CS4270_FORMAT_ADC_MASK	0x01
#define CS4270_FORMAT_ADC_LJ	0x00
#define CS4270_FORMAT_ADC_I2S	0x01
#define CS4270_TRANS_ONE_VOL	0x80
#define CS4270_TRANS_SOFT	0x40
#define CS4270_TRANS_ZERO	0x20
#define CS4270_TRANS_INV_ADC_A	0x08
#define CS4270_TRANS_INV_ADC_B	0x10
#define CS4270_TRANS_INV_DAC_A	0x02
#define CS4270_TRANS_INV_DAC_B	0x04
#define CS4270_TRANS_DEEMPH	0x01
#define CS4270_MUTE_AUTO	0x20
#define CS4270_MUTE_ADC_A	0x08
#define CS4270_MUTE_ADC_B	0x10
#define CS4270_MUTE_POLARITY	0x04
#define CS4270_MUTE_DAC_A	0x01
#define CS4270_MUTE_DAC_B	0x02


struct cs4270_private {
	struct snd_soc_codec codec;
	u8 reg_cache[CS4270_NUMREGS];
	unsigned int mclk; 
	unsigned int mode; 
	unsigned int slave_mode;
	unsigned int manual_mute;
};


struct cs4270_mode_ratios {
	unsigned int ratio;
	u8 speed_mode;
	u8 mclk;
};

static struct cs4270_mode_ratios cs4270_mode_ratios[] = {
	{64, CS4270_MODE_4X, CS4270_MODE_DIV1},
#ifndef CONFIG_SND_SOC_CS4270_VD33_ERRATA
	{96, CS4270_MODE_4X, CS4270_MODE_DIV15},
#endif
	{128, CS4270_MODE_2X, CS4270_MODE_DIV1},
	{192, CS4270_MODE_4X, CS4270_MODE_DIV3},
	{256, CS4270_MODE_1X, CS4270_MODE_DIV1},
	{384, CS4270_MODE_2X, CS4270_MODE_DIV3},
	{512, CS4270_MODE_1X, CS4270_MODE_DIV2},
	{768, CS4270_MODE_1X, CS4270_MODE_DIV3},
	{1024, CS4270_MODE_1X, CS4270_MODE_DIV4}
};


#define NUM_MCLK_RATIOS		ARRAY_SIZE(cs4270_mode_ratios)


static int cs4270_set_dai_sysclk(struct snd_soc_dai *codec_dai,
				 int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct cs4270_private *cs4270 = codec->private_data;
	unsigned int rates = 0;
	unsigned int rate_min = -1;
	unsigned int rate_max = 0;
	unsigned int i;

	cs4270->mclk = freq;

	for (i = 0; i < NUM_MCLK_RATIOS; i++) {
		unsigned int rate = freq / cs4270_mode_ratios[i].ratio;
		rates |= snd_pcm_rate_to_rate_bit(rate);
		if (rate < rate_min)
			rate_min = rate;
		if (rate > rate_max)
			rate_max = rate;
	}
	
	rates &= ~SNDRV_PCM_RATE_KNOT;

	if (!rates) {
		dev_err(codec->dev, "could not find a valid sample rate\n");
		return -EINVAL;
	}

	codec_dai->playback.rates = rates;
	codec_dai->playback.rate_min = rate_min;
	codec_dai->playback.rate_max = rate_max;

	codec_dai->capture.rates = rates;
	codec_dai->capture.rate_min = rate_min;
	codec_dai->capture.rate_max = rate_max;

	return 0;
}


static int cs4270_set_dai_fmt(struct snd_soc_dai *codec_dai,
			      unsigned int format)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct cs4270_private *cs4270 = codec->private_data;
	int ret = 0;

	
	switch (format & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	case SND_SOC_DAIFMT_LEFT_J:
		cs4270->mode = format & SND_SOC_DAIFMT_FORMAT_MASK;
		break;
	default:
		dev_err(codec->dev, "invalid dai format\n");
		ret = -EINVAL;
	}

	
	switch (format & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBS_CFS:
		cs4270->slave_mode = 1;
		break;
	case SND_SOC_DAIFMT_CBM_CFM:
		cs4270->slave_mode = 0;
		break;
	default:
		
		ret = -EINVAL;
	}

	return ret;
}


static int cs4270_fill_cache(struct snd_soc_codec *codec)
{
	u8 *cache = codec->reg_cache;
	struct i2c_client *i2c_client = codec->control_data;
	s32 length;

	length = i2c_smbus_read_i2c_block_data(i2c_client,
		CS4270_FIRSTREG | CS4270_I2C_INCR, CS4270_NUMREGS, cache);

	if (length != CS4270_NUMREGS) {
		dev_err(codec->dev, "i2c read failure, addr=0x%x\n",
		       i2c_client->addr);
		return -EIO;
	}

	return 0;
}


static unsigned int cs4270_read_reg_cache(struct snd_soc_codec *codec,
	unsigned int reg)
{
	u8 *cache = codec->reg_cache;

	if ((reg < CS4270_FIRSTREG) || (reg > CS4270_LASTREG))
		return -EIO;

	return cache[reg - CS4270_FIRSTREG];
}


static int cs4270_i2c_write(struct snd_soc_codec *codec, unsigned int reg,
			    unsigned int value)
{
	u8 *cache = codec->reg_cache;

	if ((reg < CS4270_FIRSTREG) || (reg > CS4270_LASTREG))
		return -EIO;

	
	if (cache[reg - CS4270_FIRSTREG] != value) {
		struct i2c_client *client = codec->control_data;
		if (i2c_smbus_write_byte_data(client, reg, value)) {
			dev_err(codec->dev, "i2c write failed\n");
			return -EIO;
		}

		
		cache[reg - CS4270_FIRSTREG] = value;
	}

	return 0;
}


static int cs4270_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params,
			    struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_device *socdev = rtd->socdev;
	struct snd_soc_codec *codec = socdev->card->codec;
	struct cs4270_private *cs4270 = codec->private_data;
	int ret;
	unsigned int i;
	unsigned int rate;
	unsigned int ratio;
	int reg;

	

	rate = params_rate(params);	
	ratio = cs4270->mclk / rate;	

	for (i = 0; i < NUM_MCLK_RATIOS; i++) {
		if (cs4270_mode_ratios[i].ratio == ratio)
			break;
	}

	if (i == NUM_MCLK_RATIOS) {
		
		dev_err(codec->dev, "could not find matching ratio\n");
		return -EINVAL;
	}

	

	reg = snd_soc_read(codec, CS4270_MODE);
	reg &= ~(CS4270_MODE_SPEED_MASK | CS4270_MODE_DIV_MASK);
	reg |= cs4270_mode_ratios[i].mclk;

	if (cs4270->slave_mode)
		reg |= CS4270_MODE_SLAVE;
	else
		reg |= cs4270_mode_ratios[i].speed_mode;

	ret = snd_soc_write(codec, CS4270_MODE, reg);
	if (ret < 0) {
		dev_err(codec->dev, "i2c write failed\n");
		return ret;
	}

	

	reg = snd_soc_read(codec, CS4270_FORMAT);
	reg &= ~(CS4270_FORMAT_DAC_MASK | CS4270_FORMAT_ADC_MASK);

	switch (cs4270->mode) {
	case SND_SOC_DAIFMT_I2S:
		reg |= CS4270_FORMAT_DAC_I2S | CS4270_FORMAT_ADC_I2S;
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		reg |= CS4270_FORMAT_DAC_LJ | CS4270_FORMAT_ADC_LJ;
		break;
	default:
		dev_err(codec->dev, "unknown dai format\n");
		return -EINVAL;
	}

	ret = snd_soc_write(codec, CS4270_FORMAT, reg);
	if (ret < 0) {
		dev_err(codec->dev, "i2c write failed\n");
		return ret;
	}

	return ret;
}


static int cs4270_dai_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
	struct cs4270_private *cs4270 = codec->private_data;
	int reg6;

	reg6 = snd_soc_read(codec, CS4270_MUTE);

	if (mute)
		reg6 |= CS4270_MUTE_DAC_A | CS4270_MUTE_DAC_B;
	else {
		reg6 &= ~(CS4270_MUTE_DAC_A | CS4270_MUTE_DAC_B);
		reg6 |= cs4270->manual_mute;
	}

	return snd_soc_write(codec, CS4270_MUTE, reg6);
}


static int cs4270_soc_put_mute(struct snd_kcontrol *kcontrol,
				struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct cs4270_private *cs4270 = codec->private_data;
	int left = !ucontrol->value.integer.value[0];
	int right = !ucontrol->value.integer.value[1];

	cs4270->manual_mute = (left ? CS4270_MUTE_DAC_A : 0) |
			      (right ? CS4270_MUTE_DAC_B : 0);

	return snd_soc_put_volsw(kcontrol, ucontrol);
}


static const struct snd_kcontrol_new cs4270_snd_controls[] = {
	SOC_DOUBLE_R("Master Playback Volume",
		CS4270_VOLA, CS4270_VOLB, 0, 0xFF, 1),
	SOC_SINGLE("Digital Sidetone Switch", CS4270_FORMAT, 5, 1, 0),
	SOC_SINGLE("Soft Ramp Switch", CS4270_TRANS, 6, 1, 0),
	SOC_SINGLE("Zero Cross Switch", CS4270_TRANS, 5, 1, 0),
	SOC_SINGLE("Popguard Switch", CS4270_MODE, 0, 1, 1),
	SOC_SINGLE("Auto-Mute Switch", CS4270_MUTE, 5, 1, 0),
	SOC_DOUBLE("Master Capture Switch", CS4270_MUTE, 3, 4, 1, 1),
	SOC_DOUBLE_EXT("Master Playback Switch", CS4270_MUTE, 0, 1, 1, 1,
		snd_soc_get_volsw, cs4270_soc_put_mute),
};


static struct snd_soc_codec *cs4270_codec;

static struct snd_soc_dai_ops cs4270_dai_ops = {
	.hw_params	= cs4270_hw_params,
	.set_sysclk	= cs4270_set_dai_sysclk,
	.set_fmt	= cs4270_set_dai_fmt,
	.digital_mute	= cs4270_dai_mute,
};

struct snd_soc_dai cs4270_dai = {
	.name = "cs4270",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = 0,
		.formats = CS4270_FORMATS,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = 0,
		.formats = CS4270_FORMATS,
	},
	.ops = &cs4270_dai_ops,
};
EXPORT_SYMBOL_GPL(cs4270_dai);


static int cs4270_probe(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);
	struct snd_soc_codec *codec = cs4270_codec;
	int ret;

	
	socdev->card->codec = codec;

	
	ret = snd_soc_new_pcms(socdev, SNDRV_DEFAULT_IDX1, SNDRV_DEFAULT_STR1);
	if (ret < 0) {
		dev_err(codec->dev, "failed to create pcms\n");
		return ret;
	}

	
	ret = snd_soc_add_controls(codec, cs4270_snd_controls,
				ARRAY_SIZE(cs4270_snd_controls));
	if (ret < 0) {
		dev_err(codec->dev, "failed to add controls\n");
		goto error_free_pcms;
	}

	
	ret = snd_soc_init_card(socdev);
	if (ret < 0) {
		dev_err(codec->dev, "failed to register card\n");
		goto error_free_pcms;
	}

	return 0;

error_free_pcms:
	snd_soc_free_pcms(socdev);

	return ret;
}


static int cs4270_remove(struct platform_device *pdev)
{
	struct snd_soc_device *socdev = platform_get_drvdata(pdev);

	snd_soc_free_pcms(socdev);

	return 0;
};


static int cs4270_i2c_probe(struct i2c_client *i2c_client,
	const struct i2c_device_id *id)
{
	struct snd_soc_codec *codec;
	struct cs4270_private *cs4270;
	unsigned int reg;
	int ret;

	
	if (cs4270_codec) {
		dev_err(&i2c_client->dev, "ignoring CS4270 at addr %X\n",
		       i2c_client->addr);
		dev_err(&i2c_client->dev, "only one per board allowed\n");
		
		return -ENODEV;
	}

	

	ret = i2c_smbus_read_byte_data(i2c_client, CS4270_CHIPID);
	if (ret < 0) {
		dev_err(&i2c_client->dev, "failed to read i2c at addr %X\n",
		       i2c_client->addr);
		return ret;
	}
	
	if ((ret & 0xF0) != 0xC0) {
		dev_err(&i2c_client->dev, "device at addr %X is not a CS4270\n",
		       i2c_client->addr);
		return -ENODEV;
	}

	dev_info(&i2c_client->dev, "found device at i2c address %X\n",
		i2c_client->addr);
	dev_info(&i2c_client->dev, "hardware revision %X\n", ret & 0xF);

	
	cs4270 = kzalloc(sizeof(struct cs4270_private), GFP_KERNEL);
	if (!cs4270) {
		dev_err(&i2c_client->dev, "could not allocate codec\n");
		return -ENOMEM;
	}
	codec = &cs4270->codec;

	mutex_init(&codec->mutex);
	INIT_LIST_HEAD(&codec->dapm_widgets);
	INIT_LIST_HEAD(&codec->dapm_paths);

	codec->dev = &i2c_client->dev;
	codec->name = "CS4270";
	codec->owner = THIS_MODULE;
	codec->dai = &cs4270_dai;
	codec->num_dai = 1;
	codec->private_data = cs4270;
	codec->control_data = i2c_client;
	codec->read = cs4270_read_reg_cache;
	codec->write = cs4270_i2c_write;
	codec->reg_cache = cs4270->reg_cache;
	codec->reg_cache_size = CS4270_NUMREGS;

	

	ret = cs4270_fill_cache(codec);
	if (ret < 0) {
		dev_err(&i2c_client->dev, "failed to fill register cache\n");
		goto error_free_codec;
	}

	

	reg = cs4270_read_reg_cache(codec, CS4270_MUTE);
	reg &= ~CS4270_MUTE_AUTO;
	ret = cs4270_i2c_write(codec, CS4270_MUTE, reg);
	if (ret < 0) {
		dev_err(&i2c_client->dev, "i2c write failed\n");
		return ret;
	}

	

	reg = cs4270_read_reg_cache(codec, CS4270_TRANS);
	reg &= ~(CS4270_TRANS_SOFT | CS4270_TRANS_ZERO);
	ret = cs4270_i2c_write(codec, CS4270_TRANS, reg);
	if (ret < 0) {
		dev_err(&i2c_client->dev, "i2c write failed\n");
		return ret;
	}

	
	cs4270_dai.dev = &i2c_client->dev;

	
	cs4270_codec = codec;
	ret = snd_soc_register_dai(&cs4270_dai);
	if (ret < 0) {
		dev_err(&i2c_client->dev, "failed to register DAIe\n");
		goto error_free_codec;
	}

	i2c_set_clientdata(i2c_client, cs4270);

	return 0;

error_free_codec:
	kfree(cs4270);
	cs4270_codec = NULL;
	cs4270_dai.dev = NULL;

	return ret;
}


static int cs4270_i2c_remove(struct i2c_client *i2c_client)
{
	struct cs4270_private *cs4270 = i2c_get_clientdata(i2c_client);

	kfree(cs4270);
	cs4270_codec = NULL;
	cs4270_dai.dev = NULL;

	return 0;
}


static struct i2c_device_id cs4270_id[] = {
	{"cs4270", 0},
	{}
};
MODULE_DEVICE_TABLE(i2c, cs4270_id);

#ifdef CONFIG_PM



static int cs4270_i2c_suspend(struct i2c_client *client, pm_message_t mesg)
{
	struct cs4270_private *cs4270 = i2c_get_clientdata(client);
	struct snd_soc_codec *codec = &cs4270->codec;

	return snd_soc_suspend_device(codec->dev);
}

static int cs4270_i2c_resume(struct i2c_client *client)
{
	struct cs4270_private *cs4270 = i2c_get_clientdata(client);
	struct snd_soc_codec *codec = &cs4270->codec;

	return snd_soc_resume_device(codec->dev);
}

static int cs4270_soc_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	struct snd_soc_codec *codec = cs4270_codec;
	int reg = snd_soc_read(codec, CS4270_PWRCTL) | CS4270_PWRCTL_PDN_ALL;

	return snd_soc_write(codec, CS4270_PWRCTL, reg);
}

static int cs4270_soc_resume(struct platform_device *pdev)
{
	struct snd_soc_codec *codec = cs4270_codec;
	struct i2c_client *i2c_client = codec->control_data;
	int reg;

	
	ndelay(500);

	
	for (reg = CS4270_FIRSTREG; reg <= CS4270_LASTREG; reg++) {
		u8 val = snd_soc_read(codec, reg);

		if (i2c_smbus_write_byte_data(i2c_client, reg, val)) {
			dev_err(codec->dev, "i2c write failed\n");
			return -EIO;
		}
	}

	
	reg = snd_soc_read(codec, CS4270_PWRCTL);
	reg &= ~CS4270_PWRCTL_PDN_ALL;

	return snd_soc_write(codec, CS4270_PWRCTL, reg);
}
#else
#define cs4270_i2c_suspend	NULL
#define cs4270_i2c_resume	NULL
#define cs4270_soc_suspend	NULL
#define cs4270_soc_resume	NULL
#endif 


static struct i2c_driver cs4270_i2c_driver = {
	.driver = {
		.name = "cs4270",
		.owner = THIS_MODULE,
	},
	.id_table = cs4270_id,
	.probe = cs4270_i2c_probe,
	.remove = cs4270_i2c_remove,
	.suspend = cs4270_i2c_suspend,
	.resume = cs4270_i2c_resume,
};


struct snd_soc_codec_device soc_codec_device_cs4270 = {
	.probe = 	cs4270_probe,
	.remove = 	cs4270_remove,
	.suspend =	cs4270_soc_suspend,
	.resume =	cs4270_soc_resume,
};
EXPORT_SYMBOL_GPL(soc_codec_device_cs4270);

static int __init cs4270_init(void)
{
	pr_info("Cirrus Logic CS4270 ALSA SoC Codec Driver\n");

	return i2c_add_driver(&cs4270_i2c_driver);
}
module_init(cs4270_init);

static void __exit cs4270_exit(void)
{
	i2c_del_driver(&cs4270_i2c_driver);
}
module_exit(cs4270_exit);

MODULE_AUTHOR("Timur Tabi <timur@freescale.com>");
MODULE_DESCRIPTION("Cirrus Logic CS4270 ALSA SoC Codec Driver");
MODULE_LICENSE("GPL");
