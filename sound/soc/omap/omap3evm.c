

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

static int omap3evm_hw_params(struct snd_pcm_substream *substream,
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
		printk(KERN_ERR "Can't set codec DAI configuration\n");
		return ret;
	}

	
	ret = snd_soc_dai_set_fmt(cpu_dai,
				  SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		printk(KERN_ERR "Can't set cpu DAI configuration\n");
		return ret;
	}

	
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, 26000000,
				     SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "Can't set codec system clock\n");
		return ret;
	}

	return 0;
}

static struct snd_soc_ops omap3evm_ops = {
	.hw_params = omap3evm_hw_params,
};


static struct snd_soc_dai_link omap3evm_dai = {
	.name 		= "TWL4030",
	.stream_name 	= "TWL4030",
	.cpu_dai 	= &omap_mcbsp_dai[0],
	.codec_dai 	= &twl4030_dai[TWL4030_DAI_HIFI],
	.ops 		= &omap3evm_ops,
};


static struct snd_soc_card snd_soc_omap3evm = {
	.name = "omap3evm",
	.platform = &omap_soc_platform,
	.dai_link = &omap3evm_dai,
	.num_links = 1,
};


static struct snd_soc_device omap3evm_snd_devdata = {
	.card = &snd_soc_omap3evm,
	.codec_dev = &soc_codec_dev_twl4030,
};

static struct platform_device *omap3evm_snd_device;

static int __init omap3evm_soc_init(void)
{
	int ret;

	if (!machine_is_omap3evm()) {
		pr_err("Not OMAP3 EVM!\n");
		return -ENODEV;
	}
	pr_info("OMAP3 EVM SoC init\n");

	omap3evm_snd_device = platform_device_alloc("soc-audio", -1);
	if (!omap3evm_snd_device) {
		printk(KERN_ERR "Platform device allocation failed\n");
		return -ENOMEM;
	}

	platform_set_drvdata(omap3evm_snd_device, &omap3evm_snd_devdata);
	omap3evm_snd_devdata.dev = &omap3evm_snd_device->dev;
	*(unsigned int *)omap3evm_dai.cpu_dai->private_data = 1;

	ret = platform_device_add(omap3evm_snd_device);
	if (ret)
		goto err1;

	return 0;

err1:
	printk(KERN_ERR "Unable to add platform device\n");
	platform_device_put(omap3evm_snd_device);

	return ret;
}

static void __exit omap3evm_soc_exit(void)
{
	platform_device_unregister(omap3evm_snd_device);
}

module_init(omap3evm_soc_init);
module_exit(omap3evm_soc_exit);

MODULE_AUTHOR("Anuj Aggarwal <anuj.aggarwal@ti.com>");
MODULE_DESCRIPTION("ALSA SoC OMAP3 EVM");
MODULE_LICENSE("GPL v2");
