

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <plat/audio-simtec.h>

#include "s3c24xx-pcm.h"
#include "s3c24xx-i2s.h"
#include "s3c24xx_simtec.h"

#include "../codecs/tlv320aic3x.h"

static const struct snd_soc_dapm_widget dapm_widgets[] = {
	SND_SOC_DAPM_LINE("GSM Out", NULL),
	SND_SOC_DAPM_LINE("GSM In", NULL),
	SND_SOC_DAPM_LINE("Line In", NULL),
	SND_SOC_DAPM_LINE("Line Out", NULL),
	SND_SOC_DAPM_LINE("ZV", NULL),
	SND_SOC_DAPM_MIC("Mic Jack", NULL),
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
};

static const struct snd_soc_dapm_route base_map[] = {
	

	{ "Headphone Jack", NULL, "HPLOUT" },
	{ "Headphone Jack", NULL, "HPLCOM" },
	{ "Headphone Jack", NULL, "HPROUT" },
	{ "Headphone Jack", NULL, "HPRCOM" },

	

	{ "LINE1L", NULL, "ZV" },
	{ "LINE1R", NULL, "ZV" },

	

	{ "LINE2L", NULL, "Line In" },
	{ "LINE2R", NULL, "Line In" },

	

	{ "MIC3L", NULL, "Mic Jack" },

	

	{ "GSM Out", NULL, "MONO_LOUT" },
	{ "MIC3L", NULL, "GSM In" },

	
};


static int simtec_hermes_init(struct snd_soc_codec *codec)
{
	snd_soc_dapm_new_controls(codec, dapm_widgets,
				  ARRAY_SIZE(dapm_widgets));

	snd_soc_dapm_add_routes(codec, base_map, ARRAY_SIZE(base_map));

	snd_soc_dapm_enable_pin(codec, "Headphone Jack");
	snd_soc_dapm_enable_pin(codec, "Line In");
	snd_soc_dapm_enable_pin(codec, "Line Out");
	snd_soc_dapm_enable_pin(codec, "Mic Jack");

	simtec_audio_init(codec);
	snd_soc_dapm_sync(codec);

	return 0;
}

static struct aic3x_setup_data codec_setup = {
};

static struct snd_soc_dai_link simtec_dai_aic33 = {
	.name		= "tlv320aic33",
	.stream_name	= "TLV320AIC33",
	.cpu_dai	= &s3c24xx_i2s_dai,
	.codec_dai	= &aic3x_dai,
	.init		= simtec_hermes_init,
};


static struct snd_soc_card snd_soc_machine_simtec_aic33 = {
	.name		= "Simtec-Hermes",
	.platform	= &s3c24xx_soc_platform,
	.dai_link	= &simtec_dai_aic33,
	.num_links	= 1,
};


static struct snd_soc_device simtec_snd_devdata_aic33 = {
	.card		= &snd_soc_machine_simtec_aic33,
	.codec_dev	= &soc_codec_dev_aic3x,
	.codec_data	= &codec_setup,
};

static int __devinit simtec_audio_hermes_probe(struct platform_device *pd)
{
	dev_info(&pd->dev, "probing....\n");
	return simtec_audio_core_probe(pd, &simtec_snd_devdata_aic33);
}

static struct platform_driver simtec_audio_hermes_platdrv = {
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "s3c24xx-simtec-hermes-snd",
		.pm	= simtec_audio_pm,
	},
	.probe	= simtec_audio_hermes_probe,
	.remove	= __devexit_p(simtec_audio_remove),
};

MODULE_ALIAS("platform:s3c24xx-simtec-hermes-snd");

static int __init simtec_hermes_modinit(void)
{
	return platform_driver_register(&simtec_audio_hermes_platdrv);
}

static void __exit simtec_hermes_modexit(void)
{
	platform_driver_unregister(&simtec_audio_hermes_platdrv);
}

module_init(simtec_hermes_modinit);
module_exit(simtec_hermes_modexit);

MODULE_AUTHOR("Ben Dooks <ben@simtec.co.uk>");
MODULE_DESCRIPTION("ALSA SoC Simtec Audio support");
MODULE_LICENSE("GPL");
