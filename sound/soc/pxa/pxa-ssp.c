

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <asm/irq.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/pxa2xx-lib.h>

#include <mach/hardware.h>
#include <mach/dma.h>
#include <mach/regs-ssp.h>
#include <mach/audio.h>
#include <mach/ssp.h>

#include "pxa2xx-pcm.h"
#include "pxa-ssp.h"


struct ssp_priv {
	struct ssp_dev dev;
	unsigned int sysclk;
	int dai_fmt;
#ifdef CONFIG_PM
	struct ssp_state state;
#endif
};

static void dump_registers(struct ssp_device *ssp)
{
	dev_dbg(&ssp->pdev->dev, "SSCR0 0x%08x SSCR1 0x%08x SSTO 0x%08x\n",
		 ssp_read_reg(ssp, SSCR0), ssp_read_reg(ssp, SSCR1),
		 ssp_read_reg(ssp, SSTO));

	dev_dbg(&ssp->pdev->dev, "SSPSP 0x%08x SSSR 0x%08x SSACD 0x%08x\n",
		 ssp_read_reg(ssp, SSPSP), ssp_read_reg(ssp, SSSR),
		 ssp_read_reg(ssp, SSACD));
}

struct pxa2xx_pcm_dma_data {
	struct pxa2xx_pcm_dma_params params;
	char name[20];
};

static struct pxa2xx_pcm_dma_params *
ssp_get_dma_params(struct ssp_device *ssp, int width4, int out)
{
	struct pxa2xx_pcm_dma_data *dma;

	dma = kzalloc(sizeof(struct pxa2xx_pcm_dma_data), GFP_KERNEL);
	if (dma == NULL)
		return NULL;

	snprintf(dma->name, 20, "SSP%d PCM %s %s", ssp->port_id,
			width4 ? "32-bit" : "16-bit", out ? "out" : "in");

	dma->params.name = dma->name;
	dma->params.drcmr = &DRCMR(out ? ssp->drcmr_tx : ssp->drcmr_rx);
	dma->params.dcmd = (out ? (DCMD_INCSRCADDR | DCMD_FLOWTRG) :
				  (DCMD_INCTRGADDR | DCMD_FLOWSRC)) |
			(width4 ? DCMD_WIDTH4 : DCMD_WIDTH2) | DCMD_BURST16;
	dma->params.dev_addr = ssp->phys_base + SSDR;

	return &dma->params;
}

static int pxa_ssp_startup(struct snd_pcm_substream *substream,
			   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	struct ssp_priv *priv = cpu_dai->private_data;
	int ret = 0;

	if (!cpu_dai->active) {
		priv->dev.port = cpu_dai->id + 1;
		priv->dev.irq = NO_IRQ;
		clk_enable(priv->dev.ssp->clk);
		ssp_disable(&priv->dev);
	}

	if (cpu_dai->dma_data) {
		kfree(cpu_dai->dma_data);
		cpu_dai->dma_data = NULL;
	}
	return ret;
}

static void pxa_ssp_shutdown(struct snd_pcm_substream *substream,
			     struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	struct ssp_priv *priv = cpu_dai->private_data;

	if (!cpu_dai->active) {
		ssp_disable(&priv->dev);
		clk_disable(priv->dev.ssp->clk);
	}

	if (cpu_dai->dma_data) {
		kfree(cpu_dai->dma_data);
		cpu_dai->dma_data = NULL;
	}
}

#ifdef CONFIG_PM

static int pxa_ssp_suspend(struct snd_soc_dai *cpu_dai)
{
	struct ssp_priv *priv = cpu_dai->private_data;

	if (!cpu_dai->active)
		return 0;

	ssp_save_state(&priv->dev, &priv->state);
	clk_disable(priv->dev.ssp->clk);
	return 0;
}

static int pxa_ssp_resume(struct snd_soc_dai *cpu_dai)
{
	struct ssp_priv *priv = cpu_dai->private_data;

	if (!cpu_dai->active)
		return 0;

	clk_enable(priv->dev.ssp->clk);
	ssp_restore_state(&priv->dev, &priv->state);
	ssp_enable(&priv->dev);

	return 0;
}

#else
#define pxa_ssp_suspend	NULL
#define pxa_ssp_resume	NULL
#endif


static void ssp_set_scr(struct ssp_device *ssp, u32 div)
{
	u32 sscr0 = ssp_read_reg(ssp, SSCR0);

	if (cpu_is_pxa25x() && ssp->type == PXA25x_SSP) {
		sscr0 &= ~0x0000ff00;
		sscr0 |= ((div - 2)/2) << 8; 
	} else {
		sscr0 &= ~0x000fff00;
		sscr0 |= (div - 1) << 8;     
	}
	ssp_write_reg(ssp, SSCR0, sscr0);
}


static u32 ssp_get_scr(struct ssp_device *ssp)
{
	u32 sscr0 = ssp_read_reg(ssp, SSCR0);
	u32 div;

	if (cpu_is_pxa25x() && ssp->type == PXA25x_SSP)
		div = ((sscr0 >> 8) & 0xff) * 2 + 2;
	else
		div = ((sscr0 >> 8) & 0xfff) + 1;
	return div;
}


static int pxa_ssp_set_dai_sysclk(struct snd_soc_dai *cpu_dai,
	int clk_id, unsigned int freq, int dir)
{
	struct ssp_priv *priv = cpu_dai->private_data;
	struct ssp_device *ssp = priv->dev.ssp;
	int val;

	u32 sscr0 = ssp_read_reg(ssp, SSCR0) &
		~(SSCR0_ECS |  SSCR0_NCS | SSCR0_MOD | SSCR0_ACS);

	dev_dbg(&ssp->pdev->dev,
		"pxa_ssp_set_dai_sysclk id: %d, clk_id %d, freq %u\n",
		cpu_dai->id, clk_id, freq);

	switch (clk_id) {
	case PXA_SSP_CLK_NET_PLL:
		sscr0 |= SSCR0_MOD;
		break;
	case PXA_SSP_CLK_PLL:
		
		if (cpu_is_pxa25x())
			priv->sysclk = 1843200;
		else
			priv->sysclk = 13000000;
		break;
	case PXA_SSP_CLK_EXT:
		priv->sysclk = freq;
		sscr0 |= SSCR0_ECS;
		break;
	case PXA_SSP_CLK_NET:
		priv->sysclk = freq;
		sscr0 |= SSCR0_NCS | SSCR0_MOD;
		break;
	case PXA_SSP_CLK_AUDIO:
		priv->sysclk = 0;
		ssp_set_scr(ssp, 1);
		sscr0 |= SSCR0_ACS;
		break;
	default:
		return -ENODEV;
	}

	
	if (!cpu_is_pxa3xx())
		clk_disable(priv->dev.ssp->clk);
	val = ssp_read_reg(ssp, SSCR0) | sscr0;
	ssp_write_reg(ssp, SSCR0, val);
	if (!cpu_is_pxa3xx())
		clk_enable(priv->dev.ssp->clk);

	return 0;
}


static int pxa_ssp_set_dai_clkdiv(struct snd_soc_dai *cpu_dai,
	int div_id, int div)
{
	struct ssp_priv *priv = cpu_dai->private_data;
	struct ssp_device *ssp = priv->dev.ssp;
	int val;

	switch (div_id) {
	case PXA_SSP_AUDIO_DIV_ACDS:
		val = (ssp_read_reg(ssp, SSACD) & ~0x7) | SSACD_ACDS(div);
		ssp_write_reg(ssp, SSACD, val);
		break;
	case PXA_SSP_AUDIO_DIV_SCDB:
		val = ssp_read_reg(ssp, SSACD);
		val &= ~SSACD_SCDB;
#if defined(CONFIG_PXA3xx)
		if (cpu_is_pxa3xx())
			val &= ~SSACD_SCDX8;
#endif
		switch (div) {
		case PXA_SSP_CLK_SCDB_1:
			val |= SSACD_SCDB;
			break;
		case PXA_SSP_CLK_SCDB_4:
			break;
#if defined(CONFIG_PXA3xx)
		case PXA_SSP_CLK_SCDB_8:
			if (cpu_is_pxa3xx())
				val |= SSACD_SCDX8;
			else
				return -EINVAL;
			break;
#endif
		default:
			return -EINVAL;
		}
		ssp_write_reg(ssp, SSACD, val);
		break;
	case PXA_SSP_DIV_SCR:
		ssp_set_scr(ssp, div);
		break;
	default:
		return -ENODEV;
	}

	return 0;
}


static int pxa_ssp_set_dai_pll(struct snd_soc_dai *cpu_dai,
	int pll_id, unsigned int freq_in, unsigned int freq_out)
{
	struct ssp_priv *priv = cpu_dai->private_data;
	struct ssp_device *ssp = priv->dev.ssp;
	u32 ssacd = ssp_read_reg(ssp, SSACD) & ~0x70;

#if defined(CONFIG_PXA3xx)
	if (cpu_is_pxa3xx())
		ssp_write_reg(ssp, SSACDD, 0);
#endif

	switch (freq_out) {
	case 5622000:
		break;
	case 11345000:
		ssacd |= (0x1 << 4);
		break;
	case 12235000:
		ssacd |= (0x2 << 4);
		break;
	case 14857000:
		ssacd |= (0x3 << 4);
		break;
	case 32842000:
		ssacd |= (0x4 << 4);
		break;
	case 48000000:
		ssacd |= (0x5 << 4);
		break;
	case 0:
		
		break;

	default:
#ifdef CONFIG_PXA3xx
		
		if (cpu_is_pxa3xx()) {
			u32 val;
			u64 tmp = 19968;
			tmp *= 1000000;
			do_div(tmp, freq_out);
			val = tmp;

			val = (val << 16) | 64;
			ssp_write_reg(ssp, SSACDD, val);

			ssacd |= (0x6 << 4);

			dev_dbg(&ssp->pdev->dev,
				"Using SSACDD %x to supply %uHz\n",
				val, freq_out);
			break;
		}
#endif

		return -EINVAL;
	}

	ssp_write_reg(ssp, SSACD, ssacd);

	return 0;
}


static int pxa_ssp_set_dai_tdm_slot(struct snd_soc_dai *cpu_dai,
	unsigned int tx_mask, unsigned int rx_mask, int slots, int slot_width)
{
	struct ssp_priv *priv = cpu_dai->private_data;
	struct ssp_device *ssp = priv->dev.ssp;
	u32 sscr0;

	sscr0 = ssp_read_reg(ssp, SSCR0);
	sscr0 &= ~(SSCR0_MOD | SSCR0_SlotsPerFrm(8) | SSCR0_EDSS | SSCR0_DSS);

	
	if (slot_width > 16)
		sscr0 |= SSCR0_EDSS | SSCR0_DataSize(slot_width - 16);
	else
		sscr0 |= SSCR0_DataSize(slot_width);

	if (slots > 1) {
		
		sscr0 |= SSCR0_MOD;

		
		sscr0 |= SSCR0_SlotsPerFrm(slots);

		
		ssp_write_reg(ssp, SSTSA, tx_mask);
		ssp_write_reg(ssp, SSRSA, rx_mask);
	}
	ssp_write_reg(ssp, SSCR0, sscr0);

	return 0;
}


static int pxa_ssp_set_dai_tristate(struct snd_soc_dai *cpu_dai,
	int tristate)
{
	struct ssp_priv *priv = cpu_dai->private_data;
	struct ssp_device *ssp = priv->dev.ssp;
	u32 sscr1;

	sscr1 = ssp_read_reg(ssp, SSCR1);
	if (tristate)
		sscr1 &= ~SSCR1_TTE;
	else
		sscr1 |= SSCR1_TTE;
	ssp_write_reg(ssp, SSCR1, sscr1);

	return 0;
}


static int pxa_ssp_set_dai_fmt(struct snd_soc_dai *cpu_dai,
		unsigned int fmt)
{
	struct ssp_priv *priv = cpu_dai->private_data;
	struct ssp_device *ssp = priv->dev.ssp;
	u32 sscr0;
	u32 sscr1;
	u32 sspsp;

	
	if (priv->dai_fmt == fmt)
		return 0;

	
	if (ssp_read_reg(ssp, SSCR0) & SSCR0_SSE) {
		dev_err(&ssp->pdev->dev,
			"can't change hardware dai format: stream is in use");
		return -EINVAL;
	}

	
	sscr0 = ssp_read_reg(ssp, SSCR0) &
		(SSCR0_ECS |  SSCR0_NCS | SSCR0_MOD | SSCR0_ACS);
	sscr1 = SSCR1_RxTresh(8) | SSCR1_TxTresh(7);
	sspsp = 0;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		sscr1 |= SSCR1_SCLKDIR | SSCR1_SFRMDIR;
		break;
	case SND_SOC_DAIFMT_CBM_CFS:
		sscr1 |= SSCR1_SCLKDIR;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		sspsp |= SSPSP_SFRMP;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		break;
	case SND_SOC_DAIFMT_IB_IF:
		sspsp |= SSPSP_SCMODE(2);
		break;
	case SND_SOC_DAIFMT_IB_NF:
		sspsp |= SSPSP_SCMODE(2) | SSPSP_SFRMP;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		sscr0 |= SSCR0_PSP;
		sscr1 |= SSCR1_RWOT | SSCR1_TRAIL;
		
		break;

	case SND_SOC_DAIFMT_DSP_A:
		sspsp |= SSPSP_FSRT;
	case SND_SOC_DAIFMT_DSP_B:
		sscr0 |= SSCR0_MOD | SSCR0_PSP;
		sscr1 |= SSCR1_TRAIL | SSCR1_RWOT;
		break;

	default:
		return -EINVAL;
	}

	ssp_write_reg(ssp, SSCR0, sscr0);
	ssp_write_reg(ssp, SSCR1, sscr1);
	ssp_write_reg(ssp, SSPSP, sspsp);

	dump_registers(ssp);

	
	priv->dai_fmt = fmt;

	return 0;
}


static int pxa_ssp_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	struct ssp_priv *priv = cpu_dai->private_data;
	struct ssp_device *ssp = priv->dev.ssp;
	int chn = params_channels(params);
	u32 sscr0;
	u32 sspsp;
	int width = snd_pcm_format_physical_width(params_format(params));
	int ttsa = ssp_read_reg(ssp, SSTSA) & 0xf;

	
	if (cpu_dai->dma_data)
		kfree(cpu_dai->dma_data);

	
	cpu_dai->dma_data = ssp_get_dma_params(ssp,
			((chn == 2) && (ttsa != 1)) || (width == 32),
			substream->stream == SNDRV_PCM_STREAM_PLAYBACK);

	
	if (ssp_read_reg(ssp, SSCR0) & SSCR0_SSE)
		return 0;

	
	sscr0 = ssp_read_reg(ssp, SSCR0) & ~(SSCR0_DSS | SSCR0_EDSS);
	ssp_write_reg(ssp, SSCR0, sscr0);

	
	sscr0 = ssp_read_reg(ssp, SSCR0);
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
#ifdef CONFIG_PXA3xx
		if (cpu_is_pxa3xx())
			sscr0 |= SSCR0_FPCKE;
#endif
		sscr0 |= SSCR0_DataSize(16);
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		sscr0 |= (SSCR0_EDSS | SSCR0_DataSize(8));
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		sscr0 |= (SSCR0_EDSS | SSCR0_DataSize(16));
		break;
	}
	ssp_write_reg(ssp, SSCR0, sscr0);

	switch (priv->dai_fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
	       sspsp = ssp_read_reg(ssp, SSPSP);

		if ((ssp_get_scr(ssp) == 4) && (width == 16)) {
			

#ifdef CONFIG_PXA3xx
			if (!cpu_is_pxa3xx())
				return -EINVAL;

			sspsp |= SSPSP_SFRMWDTH(width * 2);
			sspsp |= SSPSP_SFRMDLY(width * 4);
			sspsp |= SSPSP_EDMYSTOP(3);
			sspsp |= SSPSP_DMYSTOP(3);
			sspsp |= SSPSP_DMYSTRT(1);
#else
			return -EINVAL;
#endif
		} else {
			
			sspsp |= SSPSP_SFRMWDTH(width + 1);
			sspsp |= SSPSP_SFRMDLY((width + 1) * 2);
			sspsp |= SSPSP_DMYSTRT(1);
		}

		ssp_write_reg(ssp, SSPSP, sspsp);
		break;
	default:
		break;
	}

	
	if ((sscr0 & SSCR0_MOD) && !ttsa) {
		dev_err(&ssp->pdev->dev, "No TDM timeslot configured\n");
		return -EINVAL;
	}

	dump_registers(ssp);

	return 0;
}

static int pxa_ssp_trigger(struct snd_pcm_substream *substream, int cmd,
			   struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	int ret = 0;
	struct ssp_priv *priv = cpu_dai->private_data;
	struct ssp_device *ssp = priv->dev.ssp;
	int val;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_RESUME:
		ssp_enable(&priv->dev);
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		val = ssp_read_reg(ssp, SSCR1);
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			val |= SSCR1_TSRE;
		else
			val |= SSCR1_RSRE;
		ssp_write_reg(ssp, SSCR1, val);
		val = ssp_read_reg(ssp, SSSR);
		ssp_write_reg(ssp, SSSR, val);
		break;
	case SNDRV_PCM_TRIGGER_START:
		val = ssp_read_reg(ssp, SSCR1);
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			val |= SSCR1_TSRE;
		else
			val |= SSCR1_RSRE;
		ssp_write_reg(ssp, SSCR1, val);
		ssp_enable(&priv->dev);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
		val = ssp_read_reg(ssp, SSCR1);
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			val &= ~SSCR1_TSRE;
		else
			val &= ~SSCR1_RSRE;
		ssp_write_reg(ssp, SSCR1, val);
		break;
	case SNDRV_PCM_TRIGGER_SUSPEND:
		ssp_disable(&priv->dev);
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		val = ssp_read_reg(ssp, SSCR1);
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			val &= ~SSCR1_TSRE;
		else
			val &= ~SSCR1_RSRE;
		ssp_write_reg(ssp, SSCR1, val);
		break;

	default:
		ret = -EINVAL;
	}

	dump_registers(ssp);

	return ret;
}

static int pxa_ssp_probe(struct platform_device *pdev,
			    struct snd_soc_dai *dai)
{
	struct ssp_priv *priv;
	int ret;

	priv = kzalloc(sizeof(struct ssp_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev.ssp = ssp_request(dai->id + 1, "SoC audio");
	if (priv->dev.ssp == NULL) {
		ret = -ENODEV;
		goto err_priv;
	}

	priv->dai_fmt = (unsigned int) -1;
	dai->private_data = priv;

	return 0;

err_priv:
	kfree(priv);
	return ret;
}

static void pxa_ssp_remove(struct platform_device *pdev,
			      struct snd_soc_dai *dai)
{
	struct ssp_priv *priv = dai->private_data;
	ssp_free(priv->dev.ssp);
}

#define PXA_SSP_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
			  SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 |	\
			  SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000 |	\
			  SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)

#define PXA_SSP_FORMATS (SNDRV_PCM_FMTBIT_S16_LE |\
			    SNDRV_PCM_FMTBIT_S24_LE |	\
			    SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_dai_ops pxa_ssp_dai_ops = {
	.startup	= pxa_ssp_startup,
	.shutdown	= pxa_ssp_shutdown,
	.trigger	= pxa_ssp_trigger,
	.hw_params	= pxa_ssp_hw_params,
	.set_sysclk	= pxa_ssp_set_dai_sysclk,
	.set_clkdiv	= pxa_ssp_set_dai_clkdiv,
	.set_pll	= pxa_ssp_set_dai_pll,
	.set_fmt	= pxa_ssp_set_dai_fmt,
	.set_tdm_slot	= pxa_ssp_set_dai_tdm_slot,
	.set_tristate	= pxa_ssp_set_dai_tristate,
};

struct snd_soc_dai pxa_ssp_dai[] = {
	{
		.name = "pxa2xx-ssp1",
		.id = 0,
		.probe = pxa_ssp_probe,
		.remove = pxa_ssp_remove,
		.suspend = pxa_ssp_suspend,
		.resume = pxa_ssp_resume,
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rates = PXA_SSP_RATES,
			.formats = PXA_SSP_FORMATS,
		},
		.capture = {
			 .channels_min = 1,
			 .channels_max = 2,
			.rates = PXA_SSP_RATES,
			.formats = PXA_SSP_FORMATS,
		 },
		.ops = &pxa_ssp_dai_ops,
	},
	{	.name = "pxa2xx-ssp2",
		.id = 1,
		.probe = pxa_ssp_probe,
		.remove = pxa_ssp_remove,
		.suspend = pxa_ssp_suspend,
		.resume = pxa_ssp_resume,
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rates = PXA_SSP_RATES,
			.formats = PXA_SSP_FORMATS,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rates = PXA_SSP_RATES,
			.formats = PXA_SSP_FORMATS,
		 },
		.ops = &pxa_ssp_dai_ops,
	},
	{
		.name = "pxa2xx-ssp3",
		.id = 2,
		.probe = pxa_ssp_probe,
		.remove = pxa_ssp_remove,
		.suspend = pxa_ssp_suspend,
		.resume = pxa_ssp_resume,
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rates = PXA_SSP_RATES,
			.formats = PXA_SSP_FORMATS,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rates = PXA_SSP_RATES,
			.formats = PXA_SSP_FORMATS,
		 },
		.ops = &pxa_ssp_dai_ops,
	},
	{
		.name = "pxa2xx-ssp4",
		.id = 3,
		.probe = pxa_ssp_probe,
		.remove = pxa_ssp_remove,
		.suspend = pxa_ssp_suspend,
		.resume = pxa_ssp_resume,
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rates = PXA_SSP_RATES,
			.formats = PXA_SSP_FORMATS,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rates = PXA_SSP_RATES,
			.formats = PXA_SSP_FORMATS,
		 },
		.ops = &pxa_ssp_dai_ops,
	},
};
EXPORT_SYMBOL_GPL(pxa_ssp_dai);

static int __init pxa_ssp_init(void)
{
	return snd_soc_register_dais(pxa_ssp_dai, ARRAY_SIZE(pxa_ssp_dai));
}
module_init(pxa_ssp_init);

static void __exit pxa_ssp_exit(void)
{
	snd_soc_unregister_dais(pxa_ssp_dai, ARRAY_SIZE(pxa_ssp_dai));
}
module_exit(pxa_ssp_exit);


MODULE_AUTHOR("Mark Brown <broonie@opensource.wolfsonmicro.com>");
MODULE_DESCRIPTION("PXA SSP/PCM SoC Interface");
MODULE_LICENSE("GPL");
