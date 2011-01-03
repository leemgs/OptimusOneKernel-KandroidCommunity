

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/i2c.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <plat/audio-simtec.h>

#include "s3c24xx-pcm.h"
#include "s3c24xx-i2s.h"
#include "s3c24xx_simtec.h"

static struct s3c24xx_audio_simtec_pdata *pdata;
static struct clk *xtal_clk;

static int spk_gain;
static int spk_unmute;


static int speaker_gain_get(struct snd_kcontrol *kcontrol,
			    struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = spk_gain;
	return 0;
}


static void speaker_gain_set(int value)
{
	gpio_set_value_cansleep(pdata->amp_gain[0], value & 1);
	gpio_set_value_cansleep(pdata->amp_gain[1], value >> 1);
}


static int speaker_gain_put(struct snd_kcontrol *kcontrol,
			    struct snd_ctl_elem_value *ucontrol)
{
	int value = ucontrol->value.integer.value[0];

	spk_gain = value;

	if (!spk_unmute)
		speaker_gain_set(value);

	return 0;
}

static const struct snd_kcontrol_new amp_gain_controls[] = {
	SOC_SINGLE_EXT("Speaker Gain", 0, 0, 3, 0,
		       speaker_gain_get, speaker_gain_put),
};


static void spk_unmute_state(int to)
{
	pr_debug("%s: to=%d\n", __func__, to);

	spk_unmute = to;
	gpio_set_value(pdata->amp_gpio, to);

	
	if (to && pdata->amp_gain[0] > 0)
		speaker_gain_set(spk_gain);
}


static int speaker_unmute_get(struct snd_kcontrol *kcontrol,
			    struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = spk_unmute;
	return 0;
}


static int speaker_unmute_put(struct snd_kcontrol *kcontrol,
			    struct snd_ctl_elem_value *ucontrol)
{
	spk_unmute_state(ucontrol->value.integer.value[0]);
	return 0;
}


static const struct snd_kcontrol_new amp_unmute_controls[] = {
	SOC_SINGLE_EXT("Speaker Switch", 0, 0, 1, 0,
		       speaker_unmute_get, speaker_unmute_put),
};

void simtec_audio_init(struct snd_soc_codec *codec)
{
	if (pdata->amp_gpio > 0) {
		pr_debug("%s: adding amp routes\n", __func__);

		snd_soc_add_controls(codec, amp_unmute_controls,
				     ARRAY_SIZE(amp_unmute_controls));
	}

	if (pdata->amp_gain[0] > 0) {
		pr_debug("%s: adding amp controls\n", __func__);
		snd_soc_add_controls(codec, amp_gain_controls,
				     ARRAY_SIZE(amp_gain_controls));
	}
}
EXPORT_SYMBOL_GPL(simtec_audio_init);

#define CODEC_CLOCK 12000000


static int simtec_hw_params(struct snd_pcm_substream *substream,
			    struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret;

	
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret) {
		pr_err("%s: failed set cpu dai format\n", __func__);
		return ret;
	}

	
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
				  SND_SOC_DAIFMT_NB_NF |
				  SND_SOC_DAIFMT_CBM_CFM);
	if (ret) {
		pr_err("%s: failed set codec dai format\n", __func__);
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(codec_dai, 0,
				     CODEC_CLOCK, SND_SOC_CLOCK_IN);
	if (ret) {
		pr_err( "%s: failed setting codec sysclk\n", __func__);
		return ret;
	}

	if (pdata->use_mpllin) {
		ret = snd_soc_dai_set_sysclk(cpu_dai, S3C24XX_CLKSRC_MPLL,
					     0, SND_SOC_CLOCK_OUT);

		if (ret) {
			pr_err("%s: failed to set MPLLin as clksrc\n",
			       __func__);
			return ret;
		}
	}

	if (pdata->output_cdclk) {
		int cdclk_scale;

		cdclk_scale = clk_get_rate(xtal_clk) / CODEC_CLOCK;
		cdclk_scale--;

		ret = snd_soc_dai_set_clkdiv(cpu_dai, S3C24XX_DIV_PRESCALER,
					     cdclk_scale);
	}

	return 0;
}

static int simtec_call_startup(struct s3c24xx_audio_simtec_pdata *pd)
{
	
	if (pd->startup)
		pd->startup();

	return 0;
}

static struct snd_soc_ops simtec_snd_ops = {
	.hw_params	= simtec_hw_params,
};


static int attach_gpio_amp(struct device *dev,
			   struct s3c24xx_audio_simtec_pdata *pd)
{
	int ret;

	
	if (pdata->amp_gain[0] > 0) {
		ret = gpio_request(pd->amp_gain[0], "gpio-amp-gain0");
		if (ret) {
			dev_err(dev, "cannot get amp gpio gain0\n");
			return ret;
		}

		ret = gpio_request(pd->amp_gain[1], "gpio-amp-gain1");
		if (ret) {
			dev_err(dev, "cannot get amp gpio gain1\n");
			gpio_free(pdata->amp_gain[0]);
			return ret;
		}

		gpio_direction_output(pd->amp_gain[0], 0);
		gpio_direction_output(pd->amp_gain[1], 0);
	}

	
	if (pdata->amp_gpio > 0) {
		ret = gpio_request(pd->amp_gpio, "gpio-amp");
		if (ret) {
			dev_err(dev, "cannot get amp gpio %d (%d)\n",
				pd->amp_gpio, ret);
			goto err_amp;
		}

		
		spk_unmute_state(0);
	}

	return 0;

err_amp:
	if (pd->amp_gain[0] > 0) {
		gpio_free(pd->amp_gain[0]);
		gpio_free(pd->amp_gain[1]);
	}

	return ret;
}

static void detach_gpio_amp(struct s3c24xx_audio_simtec_pdata *pd)
{
	if (pd->amp_gain[0] > 0) {
		gpio_free(pd->amp_gain[0]);
		gpio_free(pd->amp_gain[1]);
	}

	if (pd->amp_gpio > 0)
		gpio_free(pd->amp_gpio);
}

#ifdef CONFIG_PM
int simtec_audio_resume(struct device *dev)
{
	simtec_call_startup(pdata);
	return 0;
}

struct dev_pm_ops simtec_audio_pmops = {
	.resume	= simtec_audio_resume,
};
EXPORT_SYMBOL_GPL(simtec_audio_pmops);
#endif

int __devinit simtec_audio_core_probe(struct platform_device *pdev,
				      struct snd_soc_device *socdev)
{
	struct platform_device *snd_dev;
	int ret;

	socdev->card->dai_link->ops = &simtec_snd_ops;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data supplied\n");
		return -EINVAL;
	}

	simtec_call_startup(pdata);

	xtal_clk = clk_get(&pdev->dev, "xtal");
	if (IS_ERR(xtal_clk)) {
		dev_err(&pdev->dev, "could not get clkout0\n");
		return -EINVAL;
	}

	dev_info(&pdev->dev, "xtal rate is %ld\n", clk_get_rate(xtal_clk));

	ret = attach_gpio_amp(&pdev->dev, pdata);
	if (ret)
		goto err_clk;

	snd_dev = platform_device_alloc("soc-audio", -1);
	if (!snd_dev) {
		dev_err(&pdev->dev, "failed to alloc soc-audio devicec\n");
		ret = -ENOMEM;
		goto err_gpio;
	}

	platform_set_drvdata(snd_dev, socdev);
	socdev->dev = &snd_dev->dev;

	ret = platform_device_add(snd_dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to add soc-audio dev\n");
		goto err_pdev;
	}

	platform_set_drvdata(pdev, snd_dev);
	return 0;

err_pdev:
	platform_device_put(snd_dev);

err_gpio:
	detach_gpio_amp(pdata);

err_clk:
	clk_put(xtal_clk);
	return ret;
}
EXPORT_SYMBOL_GPL(simtec_audio_core_probe);

int __devexit simtec_audio_remove(struct platform_device *pdev)
{
	struct platform_device *snd_dev = platform_get_drvdata(pdev);

	platform_device_unregister(snd_dev);

	detach_gpio_amp(pdata);
	clk_put(xtal_clk);
	return 0;
}
EXPORT_SYMBOL_GPL(simtec_audio_remove);

MODULE_AUTHOR("Ben Dooks <ben@simtec.co.uk>");
MODULE_DESCRIPTION("ALSA SoC Simtec Audio common support");
MODULE_LICENSE("GPL");
