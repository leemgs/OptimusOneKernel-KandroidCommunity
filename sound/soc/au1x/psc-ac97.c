

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/suspend.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <asm/mach-au1x00/au1000.h>
#include <asm/mach-au1x00/au1xxx_psc.h>

#include "psc.h"


#define AC97_RW_RETRIES	5

#define AC97_DIR	\
	(SND_SOC_DAIDIR_PLAYBACK | SND_SOC_DAIDIR_CAPTURE)

#define AC97_RATES	\
	SNDRV_PCM_RATE_8000_48000

#define AC97_FMTS	\
	(SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3BE)

#define AC97PCR_START(stype)	\
	((stype) == PCM_TX ? PSC_AC97PCR_TS : PSC_AC97PCR_RS)
#define AC97PCR_STOP(stype)	\
	((stype) == PCM_TX ? PSC_AC97PCR_TP : PSC_AC97PCR_RP)
#define AC97PCR_CLRFIFO(stype)	\
	((stype) == PCM_TX ? PSC_AC97PCR_TC : PSC_AC97PCR_RC)

#define AC97STAT_BUSY(stype)	\
	((stype) == PCM_TX ? PSC_AC97STAT_TB : PSC_AC97STAT_RB)


static struct au1xpsc_audio_data *au1xpsc_ac97_workdata;


static unsigned short au1xpsc_ac97_read(struct snd_ac97 *ac97,
					unsigned short reg)
{
	
	struct au1xpsc_audio_data *pscdata = au1xpsc_ac97_workdata;
	unsigned short data, retry, tmo;

	au_writel(PSC_AC97EVNT_CD, AC97_EVNT(pscdata));
	au_sync();

	retry = AC97_RW_RETRIES;
	do {
		mutex_lock(&pscdata->lock);

		au_writel(PSC_AC97CDC_RD | PSC_AC97CDC_INDX(reg),
			  AC97_CDC(pscdata));
		au_sync();

		tmo = 2000;
		while ((!(au_readl(AC97_EVNT(pscdata)) & PSC_AC97EVNT_CD))
			&& --tmo)
			udelay(2);

		data = au_readl(AC97_CDC(pscdata)) & 0xffff;

		au_writel(PSC_AC97EVNT_CD, AC97_EVNT(pscdata));
		au_sync();

		mutex_unlock(&pscdata->lock);
	} while (--retry && !tmo);

	return retry ? data : 0xffff;
}


static void au1xpsc_ac97_write(struct snd_ac97 *ac97, unsigned short reg,
				unsigned short val)
{
	
	struct au1xpsc_audio_data *pscdata = au1xpsc_ac97_workdata;
	unsigned int tmo, retry;

	au_writel(PSC_AC97EVNT_CD, AC97_EVNT(pscdata));
	au_sync();

	retry = AC97_RW_RETRIES;
	do {
		mutex_lock(&pscdata->lock);

		au_writel(PSC_AC97CDC_INDX(reg) | (val & 0xffff),
			  AC97_CDC(pscdata));
		au_sync();

		tmo = 2000;
		while ((!(au_readl(AC97_EVNT(pscdata)) & PSC_AC97EVNT_CD))
		       && --tmo)
			udelay(2);

		au_writel(PSC_AC97EVNT_CD, AC97_EVNT(pscdata));
		au_sync();

		mutex_unlock(&pscdata->lock);
	} while (--retry && !tmo);
}


static void au1xpsc_ac97_warm_reset(struct snd_ac97 *ac97)
{
	
	struct au1xpsc_audio_data *pscdata = au1xpsc_ac97_workdata;

	au_writel(PSC_AC97RST_SNC, AC97_RST(pscdata));
	au_sync();
	msleep(10);
	au_writel(0, AC97_RST(pscdata));
	au_sync();
}

static void au1xpsc_ac97_cold_reset(struct snd_ac97 *ac97)
{
	
	struct au1xpsc_audio_data *pscdata = au1xpsc_ac97_workdata;
	int i;

	
	au_writel(0, AC97_CFG(au1xpsc_ac97_workdata));
	au_sync();
	au_writel(PSC_CTRL_DISABLE, PSC_CTRL(pscdata));
	au_sync();

	
	au_writel(PSC_AC97RST_RST, AC97_RST(pscdata));
	au_sync();
	msleep(500);
	au_writel(0, AC97_RST(pscdata));
	au_sync();

	
	au_writel(PSC_CTRL_ENABLE, PSC_CTRL(pscdata));
	au_sync();

	
	i = 1000;
	while (!((au_readl(AC97_STAT(pscdata)) & PSC_AC97STAT_SR)) && (--i))
		msleep(1);

	if (i == 0) {
		printk(KERN_ERR "au1xpsc-ac97: PSC not ready!\n");
		return;
	}

	
	au_writel(pscdata->cfg | PSC_AC97CFG_DE_ENABLE, AC97_CFG(pscdata));
	au_sync();

	
	i = 1000;
	while (!((au_readl(AC97_STAT(pscdata)) & PSC_AC97STAT_DR)) && (--i))
		msleep(1);
	if (i == 0)
		printk(KERN_ERR "au1xpsc-ac97: AC97 ctrl not ready\n");
}


struct snd_ac97_bus_ops soc_ac97_ops = {
	.read		= au1xpsc_ac97_read,
	.write		= au1xpsc_ac97_write,
	.reset		= au1xpsc_ac97_cold_reset,
	.warm_reset	= au1xpsc_ac97_warm_reset,
};
EXPORT_SYMBOL_GPL(soc_ac97_ops);

static int au1xpsc_ac97_hw_params(struct snd_pcm_substream *substream,
				  struct snd_pcm_hw_params *params,
				  struct snd_soc_dai *dai)
{
	
	struct au1xpsc_audio_data *pscdata = au1xpsc_ac97_workdata;
	unsigned long r, ro, stat;
	int chans, stype = SUBSTREAM_TYPE(substream);

	chans = params_channels(params);

	r = ro = au_readl(AC97_CFG(pscdata));
	stat = au_readl(AC97_STAT(pscdata));

	
	if (stat & (PSC_AC97STAT_TB | PSC_AC97STAT_RB)) {
		
		if ((PSC_AC97CFG_GET_LEN(r) != params->msbits) ||
		    (pscdata->rate != params_rate(params)))
			return -EINVAL;
	} else {

		
		r &= ~PSC_AC97CFG_LEN_MASK;
		r |= PSC_AC97CFG_SET_LEN(params->msbits);

		
		if (stype == PCM_TX) {
			r &= ~PSC_AC97CFG_TXSLOT_MASK;
			r |= PSC_AC97CFG_TXSLOT_ENA(3);
			r |= PSC_AC97CFG_TXSLOT_ENA(4);
		} else {
			r &= ~PSC_AC97CFG_RXSLOT_MASK;
			r |= PSC_AC97CFG_RXSLOT_ENA(3);
			r |= PSC_AC97CFG_RXSLOT_ENA(4);
		}

		
		if (!(r ^ ro))
			goto out;

		
		mutex_lock(&pscdata->lock);

		
		au_writel(r & ~PSC_AC97CFG_DE_ENABLE, AC97_CFG(pscdata));
		au_sync();

		
		while (au_readl(AC97_STAT(pscdata)) & PSC_AC97STAT_DR)
			asm volatile ("nop");

		
		au_writel(r, AC97_CFG(pscdata));
		au_sync();

		
		au_writel(r | PSC_AC97CFG_DE_ENABLE, AC97_CFG(pscdata));
		au_sync();

		
		while (!(au_readl(AC97_STAT(pscdata)) & PSC_AC97STAT_DR))
			asm volatile ("nop");

		mutex_unlock(&pscdata->lock);

		pscdata->cfg = r;
		pscdata->rate = params_rate(params);
	}

out:
	return 0;
}

static int au1xpsc_ac97_trigger(struct snd_pcm_substream *substream,
				int cmd, struct snd_soc_dai *dai)
{
	
	struct au1xpsc_audio_data *pscdata = au1xpsc_ac97_workdata;
	int ret, stype = SUBSTREAM_TYPE(substream);

	ret = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
		au_writel(AC97PCR_CLRFIFO(stype), AC97_PCR(pscdata));
		au_sync();
		au_writel(AC97PCR_START(stype), AC97_PCR(pscdata));
		au_sync();
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
		au_writel(AC97PCR_STOP(stype), AC97_PCR(pscdata));
		au_sync();

		while (au_readl(AC97_STAT(pscdata)) & AC97STAT_BUSY(stype))
			asm volatile ("nop");

		au_writel(AC97PCR_CLRFIFO(stype), AC97_PCR(pscdata));
		au_sync();

		break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

static int au1xpsc_ac97_probe(struct platform_device *pdev,
			      struct snd_soc_dai *dai)
{
	int ret;
	struct resource *r;
	unsigned long sel;

	if (au1xpsc_ac97_workdata)
		return -EBUSY;

	au1xpsc_ac97_workdata =
		kzalloc(sizeof(struct au1xpsc_audio_data), GFP_KERNEL);
	if (!au1xpsc_ac97_workdata)
		return -ENOMEM;

	mutex_init(&au1xpsc_ac97_workdata->lock);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!r) {
		ret = -ENODEV;
		goto out0;
	}

	ret = -EBUSY;
	au1xpsc_ac97_workdata->ioarea =
		request_mem_region(r->start, r->end - r->start + 1,
					"au1xpsc_ac97");
	if (!au1xpsc_ac97_workdata->ioarea)
		goto out0;

	au1xpsc_ac97_workdata->mmio = ioremap(r->start, 0xffff);
	if (!au1xpsc_ac97_workdata->mmio)
		goto out1;

	
	au1xpsc_ac97_workdata->cfg = PSC_AC97CFG_RT_FIFO8 |
				     PSC_AC97CFG_TT_FIFO8 |
				     PSC_AC97CFG_DE_ENABLE;

	
	sel = au_readl(PSC_SEL(au1xpsc_ac97_workdata)) & PSC_SEL_CLK_MASK;
	au_writel(PSC_CTRL_DISABLE, PSC_CTRL(au1xpsc_ac97_workdata));
	au_sync();
	au_writel(0, PSC_SEL(au1xpsc_ac97_workdata));
	au_sync();
	au_writel(PSC_SEL_PS_AC97MODE | sel, PSC_SEL(au1xpsc_ac97_workdata));
	au_sync();
	

	return 0;

out1:
	release_resource(au1xpsc_ac97_workdata->ioarea);
	kfree(au1xpsc_ac97_workdata->ioarea);
out0:
	kfree(au1xpsc_ac97_workdata);
	au1xpsc_ac97_workdata = NULL;
	return ret;
}

static void au1xpsc_ac97_remove(struct platform_device *pdev,
				struct snd_soc_dai *dai)
{
	
	au_writel(0, AC97_CFG(au1xpsc_ac97_workdata));
	au_sync();
	au_writel(PSC_CTRL_DISABLE, PSC_CTRL(au1xpsc_ac97_workdata));
	au_sync();

	iounmap(au1xpsc_ac97_workdata->mmio);
	release_resource(au1xpsc_ac97_workdata->ioarea);
	kfree(au1xpsc_ac97_workdata->ioarea);
	kfree(au1xpsc_ac97_workdata);
	au1xpsc_ac97_workdata = NULL;
}

static int au1xpsc_ac97_suspend(struct snd_soc_dai *dai)
{
	
	au1xpsc_ac97_workdata->pm[0] =
			au_readl(PSC_SEL(au1xpsc_ac97_workdata));

	au_writel(0, AC97_CFG(au1xpsc_ac97_workdata));
	au_sync();
	au_writel(PSC_CTRL_DISABLE, PSC_CTRL(au1xpsc_ac97_workdata));
	au_sync();

	return 0;
}

static int au1xpsc_ac97_resume(struct snd_soc_dai *dai)
{
	
	au_writel(au1xpsc_ac97_workdata->pm[0] | PSC_SEL_PS_AC97MODE,
			PSC_SEL(au1xpsc_ac97_workdata));
	au_sync();

	
	return 0;
}

static struct snd_soc_dai_ops au1xpsc_ac97_dai_ops = {
	.trigger	= au1xpsc_ac97_trigger,
	.hw_params	= au1xpsc_ac97_hw_params,
};

struct snd_soc_dai au1xpsc_ac97_dai = {
	.name			= "au1xpsc_ac97",
	.ac97_control		= 1,
	.probe			= au1xpsc_ac97_probe,
	.remove			= au1xpsc_ac97_remove,
	.suspend		= au1xpsc_ac97_suspend,
	.resume			= au1xpsc_ac97_resume,
	.playback = {
		.rates		= AC97_RATES,
		.formats	= AC97_FMTS,
		.channels_min	= 2,
		.channels_max	= 2,
	},
	.capture = {
		.rates		= AC97_RATES,
		.formats	= AC97_FMTS,
		.channels_min	= 2,
		.channels_max	= 2,
	},
	.ops = &au1xpsc_ac97_dai_ops,
};
EXPORT_SYMBOL_GPL(au1xpsc_ac97_dai);

static int __init au1xpsc_ac97_init(void)
{
	au1xpsc_ac97_workdata = NULL;
	return snd_soc_register_dai(&au1xpsc_ac97_dai);
}

static void __exit au1xpsc_ac97_exit(void)
{
	snd_soc_unregister_dai(&au1xpsc_ac97_dai);
}

module_init(au1xpsc_ac97_init);
module_exit(au1xpsc_ac97_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Au12x0/Au1550 PSC AC97 ALSA ASoC audio driver");
MODULE_AUTHOR("Manuel Lauss <manuel.lauss@gmail.com>");
