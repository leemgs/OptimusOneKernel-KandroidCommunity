

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>


#include "../codecs/wm8974.h"
#include "mx1_mx2-pcm.h"
#include "mxc-ssi.h"
#include <mach/gpio.h>
#include <mach/iomux.h>

#define IGNORED_ARG 0


static struct snd_soc_card mx27vis;


void audmux_connect_1_4(void)
{
	pr_debug("AUDMUX: normal operation mode\n");
	

	DAM_HPCR1 = 0x00000000;
	DAM_PPCR1 = 0x00000000;

	
	DAM_HPCR1 |= AUDMUX_HPCR_SYN;
	DAM_PPCR1 |= AUDMUX_PPCR_SYN;


	
	DAM_HPCR1 |= AUDMUX_HPCR_RXDSEL(3); 
	DAM_PPCR1 |= AUDMUX_PPCR_RXDSEL(0); 

	
	DAM_HPCR1 |= AUDMUX_HPCR_TFSDIR | AUDMUX_HPCR_TCLKDIR;
	DAM_HPCR1 |= AUDMUX_HPCR_TFCSEL(3); 

	return;
}

static int mx27vis_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	unsigned int pll_out = 0, bclk = 0, fmt = 0, mclk = 0;
	int ret = 0;

	
	switch (params_rate(params)) {
	case 8000:
		fmt = SND_SOC_DAIFMT_CBM_CFM;
		mclk = WM8974_MCLKDIV_12;
		pll_out = 24576000;
		break;
	case 16000:
		fmt = SND_SOC_DAIFMT_CBM_CFM;
		pll_out = 12288000;
		break;
	case 48000:
		fmt = SND_SOC_DAIFMT_CBM_CFM;
		bclk = WM8974_BCLKDIV_4;
		pll_out = 12288000;
		break;
	case 96000:
		fmt = SND_SOC_DAIFMT_CBM_CFM;
		bclk = WM8974_BCLKDIV_2;
		pll_out = 12288000;
		break;
	case 11025:
		fmt = SND_SOC_DAIFMT_CBM_CFM;
		bclk = WM8974_BCLKDIV_16;
		pll_out = 11289600;
		break;
	case 22050:
		fmt = SND_SOC_DAIFMT_CBM_CFM;
		bclk = WM8974_BCLKDIV_8;
		pll_out = 11289600;
		break;
	case 44100:
		fmt = SND_SOC_DAIFMT_CBM_CFM;
		bclk = WM8974_BCLKDIV_4;
		mclk = WM8974_MCLKDIV_2;
		pll_out = 11289600;
		break;
	case 88200:
		fmt = SND_SOC_DAIFMT_CBM_CFM;
		bclk = WM8974_BCLKDIV_2;
		pll_out = 11289600;
		break;
	}

	
	ret = codec_dai->ops->set_fmt(codec_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_IF |
		SND_SOC_DAIFMT_SYNC | fmt);
	if (ret < 0) {
		printk(KERN_ERR "Error from codec DAI configuration\n");
		return ret;
	}

	
	ret = cpu_dai->ops->set_fmt(cpu_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_SYNC | fmt);
	if (ret < 0) {
		printk(KERN_ERR "Error from cpu DAI configuration\n");
		return ret;
	}

	
	ret = cpu_dai->ops->set_tdm_slot(cpu_dai, 0, 2);

	
	ret = cpu_dai->ops->set_sysclk(cpu_dai, IMX_SSP_SYS_CLK, 0,
		SND_SOC_CLOCK_IN);
	if (ret < 0) {
		printk(KERN_ERR "Error when setting system SSI clk\n");
		return ret;
	}

	
	ret = codec_dai->ops->set_clkdiv(codec_dai, WM8974_BCLKDIV, bclk);
	if (ret < 0) {
		printk(KERN_ERR "Error when setting BCLK division\n");
		return ret;
	}


	
	ret = codec_dai->ops->set_pll(codec_dai, IGNORED_ARG,
					25000000, pll_out);
	if (ret < 0) {
		printk(KERN_ERR "Error when setting PLL input\n");
		return ret;
	}

	
	ret = codec_dai->ops->set_clkdiv(codec_dai, WM8974_MCLKDIV, mclk);
	if (ret < 0) {
		printk(KERN_ERR "Error when setting MCLK division\n");
		return ret;
	}

	return 0;
}

static int mx27vis_hifi_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;

	
	return codec_dai->ops->set_pll(codec_dai, IGNORED_ARG, 0, 0);
}


static struct snd_soc_ops mx27vis_hifi_ops = {
	.hw_params = mx27vis_hifi_hw_params,
	.hw_free = mx27vis_hifi_hw_free,
};


static int mx27vis_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int mx27vis_resume(struct platform_device *pdev)
{
	return 0;
}

static int mx27vis_probe(struct platform_device *pdev)
{
	int ret = 0;

	ret = get_ssi_clk(0, &pdev->dev);

	if (ret < 0) {
		printk(KERN_ERR "%s: cant get ssi clock\n", __func__);
		return ret;
	}


	return 0;
}

static int mx27vis_remove(struct platform_device *pdev)
{
	put_ssi_clk(0);
	return 0;
}

static struct snd_soc_dai_link mx27vis_dai[] = {
{ 
	.name = "WM8974",
	.stream_name = "WM8974 HiFi",
	.cpu_dai = &imx_ssi_pcm_dai[0],
	.codec_dai = &wm8974_dai,
	.ops = &mx27vis_hifi_ops,
},
};

static struct snd_soc_card mx27vis = {
	.name = "mx27vis",
	.platform = &mx1_mx2_soc_platform,
	.probe = mx27vis_probe,
	.remove = mx27vis_remove,
	.suspend_pre = mx27vis_suspend,
	.resume_post = mx27vis_resume,
	.dai_link = mx27vis_dai,
	.num_links = ARRAY_SIZE(mx27vis_dai),
};

static struct snd_soc_device mx27vis_snd_devdata = {
	.card = &mx27vis,
	.codec_dev = &soc_codec_dev_wm8974,
};

static struct platform_device *mx27vis_snd_device;


void gpio_ssi_active(int ssi_num)
{
	int ret = 0;

	unsigned int ssi1_pins[] = {
		PC20_PF_SSI1_FS,
		PC21_PF_SSI1_RXD,
		PC22_PF_SSI1_TXD,
		PC23_PF_SSI1_CLK,
	};
	unsigned int ssi2_pins[] = {
		PC24_PF_SSI2_FS,
		PC25_PF_SSI2_RXD,
		PC26_PF_SSI2_TXD,
		PC27_PF_SSI2_CLK,
	};
	if (ssi_num == 0)
		ret = mxc_gpio_setup_multiple_pins(ssi1_pins,
				ARRAY_SIZE(ssi1_pins), "USB OTG");
	else
		ret = mxc_gpio_setup_multiple_pins(ssi2_pins,
				ARRAY_SIZE(ssi2_pins), "USB OTG");
	if (ret)
		printk(KERN_ERR "Error requesting ssi %x pins\n", ssi_num);
}


static int __init mx27vis_init(void)
{
	int ret;

	mx27vis_snd_device = platform_device_alloc("soc-audio", -1);
	if (!mx27vis_snd_device)
		return -ENOMEM;

	platform_set_drvdata(mx27vis_snd_device, &mx27vis_snd_devdata);
	mx27vis_snd_devdata.dev = &mx27vis_snd_device->dev;
	ret = platform_device_add(mx27vis_snd_device);

	if (ret) {
		printk(KERN_ERR "ASoC: Platform device allocation failed\n");
		platform_device_put(mx27vis_snd_device);
	}

	
	gpio_ssi_active(0);
	audmux_connect_1_4();

	return ret;
}

static void __exit mx27vis_exit(void)
{
	
}

module_init(mx27vis_init);
module_exit(mx27vis_exit);


MODULE_AUTHOR("Javier Martin, javier.martin@vista-silicon.com");
MODULE_DESCRIPTION("ALSA SoC WM8974 mx27vis");
MODULE_LICENSE("GPL");
