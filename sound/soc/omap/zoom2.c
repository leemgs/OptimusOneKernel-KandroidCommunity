

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <mach/gpio.h>
#include <mach/mcbsp.h>

#include "omap-mcbsp.h"
#include "omap-pcm.h"
#include "../codecs/twl4030.h"

#define ZOOM2_HEADSET_MUX_GPIO		(OMAP_MAX_GPIO_LINES + 15)
#define ZOOM2_HEADSET_EXTMUTE_GPIO	153

static int zoom2_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	
	ret = snd_soc_dai_set_fmt(codec_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
					SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	return 0;
}

static struct snd_soc_ops zoom2_ops = {
	.hw_params = zoom2_hw_params,
};

static int zoom2_hw_voice_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	
	ret = snd_soc_dai_set_fmt(codec_dai,
				SND_SOC_DAIFMT_DSP_A |
				SND_SOC_DAIFMT_IB_NF |
				SND_SOC_DAIFMT_CBM_CFM);
	if (ret) {
		printk(KERN_ERR "can't set codec DAI configuration\n");
		return ret;
	}

	
	ret = snd_soc_dai_set_fmt(cpu_dai,
				SND_SOC_DAIFMT_DSP_A |
				SND_SOC_DAIFMT_IB_NF |
				SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "can't set cpu DAI configuration\n");
		return ret;
	}

	
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
					SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "can't set codec system clock\n");
		return ret;
	}

	return 0;
}

static struct snd_soc_ops zoom2_voice_ops = {
	.hw_params = zoom2_hw_voice_params,
};


static const struct snd_soc_dapm_widget zoom2_twl4030_dapm_widgets[] = {
	SND_SOC_DAPM_MIC("Ext Mic", NULL),
	SND_SOC_DAPM_SPK("Ext Spk", NULL),
	SND_SOC_DAPM_MIC("Headset Mic", NULL),
	SND_SOC_DAPM_HP("Headset Stereophone", NULL),
	SND_SOC_DAPM_LINE("Aux In", NULL),
};

static const struct snd_soc_dapm_route audio_map[] = {
	
	{"MAINMIC", NULL, "Mic Bias 1"},
	{"SUBMIC", NULL, "Mic Bias 2"},
	{"Mic Bias 1", NULL, "Ext Mic"},
	{"Mic Bias 2", NULL, "Ext Mic"},

	
	{"Ext Spk", NULL, "HFL"},
	{"Ext Spk", NULL, "HFR"},

	
	{"Headset Stereophone", NULL, "HSOL"},
	{"Headset Stereophone", NULL, "HSOR"},

	
	{"HSMIC", NULL, "Headset Mic Bias"},
	{"Headset Mic Bias", NULL, "Headset Mic"},

	
	{"Aux In", NULL, "AUXL"},
	{"Aux In", NULL, "AUXR"},
};

static int zoom2_twl4030_init(struct snd_soc_codec *codec)
{
	int ret;

	
	ret = snd_soc_dapm_new_controls(codec, zoom2_twl4030_dapm_widgets,
				ARRAY_SIZE(zoom2_twl4030_dapm_widgets));
	if (ret)
		return ret;

	
	snd_soc_dapm_add_routes(codec, audio_map, ARRAY_SIZE(audio_map));

	
	snd_soc_dapm_enable_pin(codec, "Ext Mic");
	snd_soc_dapm_enable_pin(codec, "Ext Spk");
	snd_soc_dapm_enable_pin(codec, "Headset Mic");
	snd_soc_dapm_enable_pin(codec, "Headset Stereophone");
	snd_soc_dapm_enable_pin(codec, "Aux In");

	
	snd_soc_dapm_nc_pin(codec, "CARKITMIC");
	snd_soc_dapm_nc_pin(codec, "DIGIMIC0");
	snd_soc_dapm_nc_pin(codec, "DIGIMIC1");

	snd_soc_dapm_nc_pin(codec, "OUTL");
	snd_soc_dapm_nc_pin(codec, "OUTR");
	snd_soc_dapm_nc_pin(codec, "EARPIECE");
	snd_soc_dapm_nc_pin(codec, "PREDRIVEL");
	snd_soc_dapm_nc_pin(codec, "PREDRIVER");
	snd_soc_dapm_nc_pin(codec, "CARKITL");
	snd_soc_dapm_nc_pin(codec, "CARKITR");

	ret = snd_soc_dapm_sync(codec);

	return ret;
}

static int zoom2_twl4030_voice_init(struct snd_soc_codec *codec)
{
	unsigned short reg;

	
	reg = codec->read(codec, TWL4030_REG_VOICE_IF);
	reg |= TWL4030_VIF_DIN_EN | TWL4030_VIF_DOUT_EN | TWL4030_VIF_EN;
	codec->write(codec, TWL4030_REG_VOICE_IF, reg);

	return 0;
}


static struct snd_soc_dai_link zoom2_dai[] = {
	{
		.name = "TWL4030 I2S",
		.stream_name = "TWL4030 Audio",
		.cpu_dai = &omap_mcbsp_dai[0],
		.codec_dai = &twl4030_dai[TWL4030_DAI_HIFI],
		.init = zoom2_twl4030_init,
		.ops = &zoom2_ops,
	},
	{
		.name = "TWL4030 PCM",
		.stream_name = "TWL4030 Voice",
		.cpu_dai = &omap_mcbsp_dai[1],
		.codec_dai = &twl4030_dai[TWL4030_DAI_VOICE],
		.init = zoom2_twl4030_voice_init,
		.ops = &zoom2_voice_ops,
	},
};


static struct snd_soc_card snd_soc_zoom2 = {
	.name = "Zoom2",
	.platform = &omap_soc_platform,
	.dai_link = zoom2_dai,
	.num_links = ARRAY_SIZE(zoom2_dai),
};


void zoom2_set_hs_extmute(int mute)
{
	gpio_set_value(ZOOM2_HEADSET_EXTMUTE_GPIO, mute);
}


static struct twl4030_setup_data twl4030_setup = {
	.ramp_delay_value = 3,	
	.sysclk = 26000,
	.hs_extmute = 1,
	.set_hs_extmute = zoom2_set_hs_extmute,
};


static struct snd_soc_device zoom2_snd_devdata = {
	.card = &snd_soc_zoom2,
	.codec_dev = &soc_codec_dev_twl4030,
	.codec_data = &twl4030_setup,
};

static struct platform_device *zoom2_snd_device;

static int __init zoom2_soc_init(void)
{
	int ret;

	if (!machine_is_omap_zoom2()) {
		pr_debug("Not Zoom2!\n");
		return -ENODEV;
	}
	printk(KERN_INFO "Zoom2 SoC init\n");

	zoom2_snd_device = platform_device_alloc("soc-audio", -1);
	if (!zoom2_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(zoom2_snd_device, &zoom2_snd_devdata);
	zoom2_snd_devdata.dev = &zoom2_snd_device->dev;
	*(unsigned int *)zoom2_dai[0].cpu_dai->private_data = 1; 
	*(unsigned int *)zoom2_dai[1].cpu_dai->private_data = 2; 

	ret = platform_device_add(zoom2_snd_device);
	if (ret)
		goto err1;

	BUG_ON(gpio_request(ZOOM2_HEADSET_MUX_GPIO, "hs_mux") < 0);
	gpio_direction_output(ZOOM2_HEADSET_MUX_GPIO, 0);

	BUG_ON(gpio_request(ZOOM2_HEADSET_EXTMUTE_GPIO, "ext_mute") < 0);
	gpio_direction_output(ZOOM2_HEADSET_EXTMUTE_GPIO, 0);

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(zoom2_snd_device);

	return ret;
}
module_init(zoom2_soc_init);

static void __exit zoom2_soc_exit(void)
{
	gpio_free(ZOOM2_HEADSET_MUX_GPIO);
	gpio_free(ZOOM2_HEADSET_EXTMUTE_GPIO);

	platform_device_unregister(zoom2_snd_device);
}
module_exit(zoom2_soc_exit);

MODULE_AUTHOR("Misael Lopez Cruz <x0052729@ti.com>");
MODULE_DESCRIPTION("ALSA SoC Zoom2");
MODULE_LICENSE("GPL");

