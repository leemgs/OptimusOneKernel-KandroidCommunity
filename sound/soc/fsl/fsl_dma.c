

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <asm/io.h>

#include "fsl_dma.h"


#define FSLDMA_PCM_FORMATS (SNDRV_PCM_FMTBIT_S8 	| \
			    SNDRV_PCM_FMTBIT_U8 	| \
			    SNDRV_PCM_FMTBIT_S16_LE     | \
			    SNDRV_PCM_FMTBIT_S16_BE     | \
			    SNDRV_PCM_FMTBIT_U16_LE     | \
			    SNDRV_PCM_FMTBIT_U16_BE     | \
			    SNDRV_PCM_FMTBIT_S24_LE     | \
			    SNDRV_PCM_FMTBIT_S24_BE     | \
			    SNDRV_PCM_FMTBIT_U24_LE     | \
			    SNDRV_PCM_FMTBIT_U24_BE     | \
			    SNDRV_PCM_FMTBIT_S32_LE     | \
			    SNDRV_PCM_FMTBIT_S32_BE     | \
			    SNDRV_PCM_FMTBIT_U32_LE     | \
			    SNDRV_PCM_FMTBIT_U32_BE)

#define FSLDMA_PCM_RATES (SNDRV_PCM_RATE_5512 | SNDRV_PCM_RATE_8000_192000 | \
			  SNDRV_PCM_RATE_CONTINUOUS)


static struct {
	dma_addr_t ssi_stx_phys;
	dma_addr_t ssi_srx_phys;
	struct ccsr_dma_channel __iomem *dma_channel[2];
	unsigned int irq[2];
	unsigned int assigned[2];
} dma_global_data;


#define NUM_DMA_LINKS   2


struct fsl_dma_private {
	struct fsl_dma_link_descriptor link[NUM_DMA_LINKS];
	unsigned int controller_id;
	unsigned int channel_id;
	struct ccsr_dma_channel __iomem *dma_channel;
	unsigned int irq;
	struct snd_pcm_substream *substream;
	dma_addr_t ssi_sxx_phys;
	dma_addr_t ld_buf_phys;
	unsigned int current_link;
	dma_addr_t dma_buf_phys;
	dma_addr_t dma_buf_next;
	dma_addr_t dma_buf_end;
	size_t period_size;
	unsigned int num_periods;
};


static const struct snd_pcm_hardware fsl_dma_hardware = {

	.info   		= SNDRV_PCM_INFO_INTERLEAVED |
				  SNDRV_PCM_INFO_MMAP |
				  SNDRV_PCM_INFO_MMAP_VALID |
				  SNDRV_PCM_INFO_JOINT_DUPLEX |
				  SNDRV_PCM_INFO_PAUSE,
	.formats		= FSLDMA_PCM_FORMATS,
	.rates  		= FSLDMA_PCM_RATES,
	.rate_min       	= 5512,
	.rate_max       	= 192000,
	.period_bytes_min       = 512,  	
	.period_bytes_max       = (u32) -1,
	.periods_min    	= NUM_DMA_LINKS,
	.periods_max    	= (unsigned int) -1,
	.buffer_bytes_max       = 128 * 1024,   
};


static void fsl_dma_abort_stream(struct snd_pcm_substream *substream)
{
	unsigned long flags;

	snd_pcm_stream_lock_irqsave(substream, flags);

	if (snd_pcm_running(substream))
		snd_pcm_stop(substream, SNDRV_PCM_STATE_XRUN);

	snd_pcm_stream_unlock_irqrestore(substream, flags);
}


static void fsl_dma_update_pointers(struct fsl_dma_private *dma_private)
{
	struct fsl_dma_link_descriptor *link =
		&dma_private->link[dma_private->current_link];

	
	if (dma_private->substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		link->source_addr =
			cpu_to_be32(dma_private->dma_buf_next);
	else
		link->dest_addr =
			cpu_to_be32(dma_private->dma_buf_next);

	
	dma_private->dma_buf_next += dma_private->period_size;

	if (dma_private->dma_buf_next >= dma_private->dma_buf_end)
		dma_private->dma_buf_next = dma_private->dma_buf_phys;

	if (++dma_private->current_link >= NUM_DMA_LINKS)
		dma_private->current_link = 0;
}


static irqreturn_t fsl_dma_isr(int irq, void *dev_id)
{
	struct fsl_dma_private *dma_private = dev_id;
	struct ccsr_dma_channel __iomem *dma_channel = dma_private->dma_channel;
	irqreturn_t ret = IRQ_NONE;
	u32 sr, sr2 = 0;

	
	sr = in_be32(&dma_channel->sr);

	if (sr & CCSR_DMA_SR_TE) {
		dev_err(dma_private->substream->pcm->card->dev,
			"DMA transmit error (controller=%u channel=%u irq=%u\n",
			dma_private->controller_id,
			dma_private->channel_id, irq);
		fsl_dma_abort_stream(dma_private->substream);
		sr2 |= CCSR_DMA_SR_TE;
		ret = IRQ_HANDLED;
	}

	if (sr & CCSR_DMA_SR_CH)
		ret = IRQ_HANDLED;

	if (sr & CCSR_DMA_SR_PE) {
		dev_err(dma_private->substream->pcm->card->dev,
			"DMA%u programming error (channel=%u irq=%u)\n",
			dma_private->controller_id,
			dma_private->channel_id, irq);
		fsl_dma_abort_stream(dma_private->substream);
		sr2 |= CCSR_DMA_SR_PE;
		ret = IRQ_HANDLED;
	}

	if (sr & CCSR_DMA_SR_EOLNI) {
		sr2 |= CCSR_DMA_SR_EOLNI;
		ret = IRQ_HANDLED;
	}

	if (sr & CCSR_DMA_SR_CB)
		ret = IRQ_HANDLED;

	if (sr & CCSR_DMA_SR_EOSI) {
		struct snd_pcm_substream *substream = dma_private->substream;

		
		snd_pcm_period_elapsed(substream);

		
		if (dma_private->num_periods != NUM_DMA_LINKS)
			fsl_dma_update_pointers(dma_private);

		sr2 |= CCSR_DMA_SR_EOSI;
		ret = IRQ_HANDLED;
	}

	if (sr & CCSR_DMA_SR_EOLSI) {
		sr2 |= CCSR_DMA_SR_EOLSI;
		ret = IRQ_HANDLED;
	}

	
	if (sr2)
		out_be32(&dma_channel->sr, sr2);

	return ret;
}


static int fsl_dma_new(struct snd_card *card, struct snd_soc_dai *dai,
	struct snd_pcm *pcm)
{
	static u64 fsl_dma_dmamask = DMA_BIT_MASK(32);
	int ret;

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &fsl_dma_dmamask;

	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = fsl_dma_dmamask;

	ret = snd_dma_alloc_pages(SNDRV_DMA_TYPE_DEV, card->dev,
		fsl_dma_hardware.buffer_bytes_max,
		&pcm->streams[0].substream->dma_buffer);
	if (ret) {
		dev_err(card->dev,
			"Can't allocate playback DMA buffer (size=%u)\n",
			fsl_dma_hardware.buffer_bytes_max);
		return -ENOMEM;
	}

	ret = snd_dma_alloc_pages(SNDRV_DMA_TYPE_DEV, card->dev,
		fsl_dma_hardware.buffer_bytes_max,
		&pcm->streams[1].substream->dma_buffer);
	if (ret) {
		snd_dma_free_pages(&pcm->streams[0].substream->dma_buffer);
		dev_err(card->dev,
			"Can't allocate capture DMA buffer (size=%u)\n",
			fsl_dma_hardware.buffer_bytes_max);
		return -ENOMEM;
	}

	return 0;
}


static int fsl_dma_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fsl_dma_private *dma_private;
	struct ccsr_dma_channel __iomem *dma_channel;
	dma_addr_t ld_buf_phys;
	u64 temp_link;  	
	u32 mr;
	unsigned int channel;
	int ret = 0;
	unsigned int i;

	
	ret = snd_pcm_hw_constraint_integer(runtime,
		SNDRV_PCM_HW_PARAM_PERIODS);
	if (ret < 0) {
		dev_err(substream->pcm->card->dev, "invalid buffer size\n");
		return ret;
	}

	channel = substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? 0 : 1;

	if (dma_global_data.assigned[channel]) {
		dev_err(substream->pcm->card->dev,
			"DMA channel already assigned\n");
		return -EBUSY;
	}

	dma_private = dma_alloc_coherent(substream->pcm->card->dev,
		sizeof(struct fsl_dma_private), &ld_buf_phys, GFP_KERNEL);
	if (!dma_private) {
		dev_err(substream->pcm->card->dev,
			"can't allocate DMA private data\n");
		return -ENOMEM;
	}
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		dma_private->ssi_sxx_phys = dma_global_data.ssi_stx_phys;
	else
		dma_private->ssi_sxx_phys = dma_global_data.ssi_srx_phys;

	dma_private->dma_channel = dma_global_data.dma_channel[channel];
	dma_private->irq = dma_global_data.irq[channel];
	dma_private->substream = substream;
	dma_private->ld_buf_phys = ld_buf_phys;
	dma_private->dma_buf_phys = substream->dma_buffer.addr;

	
	dma_private->controller_id = 0;
	dma_private->channel_id = channel;

	ret = request_irq(dma_private->irq, fsl_dma_isr, 0, "DMA", dma_private);
	if (ret) {
		dev_err(substream->pcm->card->dev,
			"can't register ISR for IRQ %u (ret=%i)\n",
			dma_private->irq, ret);
		dma_free_coherent(substream->pcm->card->dev,
			sizeof(struct fsl_dma_private),
			dma_private, dma_private->ld_buf_phys);
		return ret;
	}

	dma_global_data.assigned[channel] = 1;

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
	snd_soc_set_runtime_hwparams(substream, &fsl_dma_hardware);
	runtime->private_data = dma_private;

	

	dma_channel = dma_private->dma_channel;

	temp_link = dma_private->ld_buf_phys +
		sizeof(struct fsl_dma_link_descriptor);

	for (i = 0; i < NUM_DMA_LINKS; i++) {
		dma_private->link[i].next = cpu_to_be64(temp_link);

		temp_link += sizeof(struct fsl_dma_link_descriptor);
	}
	
	dma_private->link[i - 1].next = cpu_to_be64(dma_private->ld_buf_phys);

	
	out_be32(&dma_channel->clndar,
		CCSR_DMA_CLNDAR_ADDR(dma_private->ld_buf_phys));
	out_be32(&dma_channel->eclndar,
		CCSR_DMA_ECLNDAR_ADDR(dma_private->ld_buf_phys));

	
	out_be32(&dma_channel->bcr, 0);

	
	mr = in_be32(&dma_channel->mr) &
		~(CCSR_DMA_MR_CA | CCSR_DMA_MR_DAHE | CCSR_DMA_MR_SAHE);

	
	mr |= CCSR_DMA_MR_EOSIE | CCSR_DMA_MR_EIE | CCSR_DMA_MR_EMP_EN |
		CCSR_DMA_MR_EMS_EN;

	
	mr |= (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) ?
		CCSR_DMA_MR_DAHE : CCSR_DMA_MR_SAHE;

	out_be32(&dma_channel->mr, mr);

	return 0;
}


static int fsl_dma_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *hw_params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fsl_dma_private *dma_private = runtime->private_data;

	
	unsigned int sample_size =
		snd_pcm_format_physical_width(params_format(hw_params));

	
	unsigned int frame_size = 2 * (sample_size / 8);

	
	dma_addr_t ssi_sxx_phys = dma_private->ssi_sxx_phys;

	
	size_t buffer_size = params_buffer_bytes(hw_params);

	
	size_t period_size = params_period_bytes(hw_params);

	
	dma_addr_t temp_addr = substream->dma_buffer.addr;

	
	struct ccsr_dma_channel __iomem *dma_channel = dma_private->dma_channel;

	u32 mr; 

	unsigned int i;

	
	dma_private->period_size = period_size;
	dma_private->num_periods = params_periods(hw_params);
	dma_private->dma_buf_end = dma_private->dma_buf_phys + buffer_size;
	dma_private->dma_buf_next = dma_private->dma_buf_phys +
		(NUM_DMA_LINKS * period_size);

	if (dma_private->dma_buf_next >= dma_private->dma_buf_end)
		
		dma_private->dma_buf_next = dma_private->dma_buf_phys;

	mr = in_be32(&dma_channel->mr) & ~(CCSR_DMA_MR_BWC_MASK |
		  CCSR_DMA_MR_SAHTS_MASK | CCSR_DMA_MR_DAHTS_MASK);

	
	switch (sample_size) {
	case 8:
		mr |= CCSR_DMA_MR_DAHTS_1 | CCSR_DMA_MR_SAHTS_1;
		ssi_sxx_phys += 3;
		break;
	case 16:
		mr |= CCSR_DMA_MR_DAHTS_2 | CCSR_DMA_MR_SAHTS_2;
		ssi_sxx_phys += 2;
		break;
	case 32:
		mr |= CCSR_DMA_MR_DAHTS_4 | CCSR_DMA_MR_SAHTS_4;
		break;
	default:
		
		dev_err(substream->pcm->card->dev,
			"unsupported sample size %u\n", sample_size);
		return -EINVAL;
	}

	
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		mr |= CCSR_DMA_MR_BWC(2 * frame_size);
	else
		mr |= CCSR_DMA_MR_BWC(frame_size);

	out_be32(&dma_channel->mr, mr);

	for (i = 0; i < NUM_DMA_LINKS; i++) {
		struct fsl_dma_link_descriptor *link = &dma_private->link[i];

		link->count = cpu_to_be32(period_size);

		
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			link->source_addr = cpu_to_be32(temp_addr);
			link->source_attr = cpu_to_be32(CCSR_DMA_ATR_SNOOP);

			link->dest_addr = cpu_to_be32(ssi_sxx_phys);
			link->dest_attr = cpu_to_be32(CCSR_DMA_ATR_NOSNOOP);
		} else {
			link->source_addr = cpu_to_be32(ssi_sxx_phys);
			link->source_attr = cpu_to_be32(CCSR_DMA_ATR_NOSNOOP);

			link->dest_addr = cpu_to_be32(temp_addr);
			link->dest_attr = cpu_to_be32(CCSR_DMA_ATR_SNOOP);
		}

		temp_addr += period_size;
	}

	return 0;
}


static snd_pcm_uframes_t fsl_dma_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fsl_dma_private *dma_private = runtime->private_data;
	struct ccsr_dma_channel __iomem *dma_channel = dma_private->dma_channel;
	dma_addr_t position;
	snd_pcm_uframes_t frames;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		position = in_be32(&dma_channel->sar);
	else
		position = in_be32(&dma_channel->dar);

	
	if (!position)
		return 0;

	if ((position < dma_private->dma_buf_phys) ||
	    (position > dma_private->dma_buf_end)) {
		dev_err(substream->pcm->card->dev,
			"dma pointer is out of range, halting stream\n");
		return SNDRV_PCM_POS_XRUN;
	}

	frames = bytes_to_frames(runtime, position - dma_private->dma_buf_phys);

	
	if (frames == runtime->buffer_size)
		frames = 0;

	return frames;
}


static int fsl_dma_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fsl_dma_private *dma_private = runtime->private_data;

	if (dma_private) {
		struct ccsr_dma_channel __iomem *dma_channel;

		dma_channel = dma_private->dma_channel;

		
		out_be32(&dma_channel->mr, CCSR_DMA_MR_CA);
		out_be32(&dma_channel->mr, 0);

		
		out_be32(&dma_channel->sr, -1);
		out_be32(&dma_channel->clndar, 0);
		out_be32(&dma_channel->eclndar, 0);
		out_be32(&dma_channel->satr, 0);
		out_be32(&dma_channel->sar, 0);
		out_be32(&dma_channel->datr, 0);
		out_be32(&dma_channel->dar, 0);
		out_be32(&dma_channel->bcr, 0);
		out_be32(&dma_channel->nlndar, 0);
		out_be32(&dma_channel->enlndar, 0);
	}

	return 0;
}


static int fsl_dma_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct fsl_dma_private *dma_private = runtime->private_data;
	int dir = substream->stream == SNDRV_PCM_STREAM_PLAYBACK ? 0 : 1;

	if (dma_private) {
		if (dma_private->irq)
			free_irq(dma_private->irq, dma_private);

		if (dma_private->ld_buf_phys) {
			dma_unmap_single(substream->pcm->card->dev,
				dma_private->ld_buf_phys,
				sizeof(dma_private->link), DMA_TO_DEVICE);
		}

		
		dma_free_coherent(substream->pcm->card->dev,
			sizeof(struct fsl_dma_private),
			dma_private, dma_private->ld_buf_phys);
		substream->runtime->private_data = NULL;
	}

	dma_global_data.assigned[dir] = 0;

	return 0;
}


static void fsl_dma_free_dma_buffers(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(pcm->streams); i++) {
		substream = pcm->streams[i].substream;
		if (substream) {
			snd_dma_free_pages(&substream->dma_buffer);
			substream->dma_buffer.area = NULL;
			substream->dma_buffer.addr = 0;
		}
	}
}

static struct snd_pcm_ops fsl_dma_ops = {
	.open   	= fsl_dma_open,
	.close  	= fsl_dma_close,
	.ioctl  	= snd_pcm_lib_ioctl,
	.hw_params      = fsl_dma_hw_params,
	.hw_free	= fsl_dma_hw_free,
	.pointer	= fsl_dma_pointer,
};

struct snd_soc_platform fsl_soc_platform = {
	.name   	= "fsl-dma",
	.pcm_ops	= &fsl_dma_ops,
	.pcm_new	= fsl_dma_new,
	.pcm_free       = fsl_dma_free_dma_buffers,
};
EXPORT_SYMBOL_GPL(fsl_soc_platform);


int fsl_dma_configure(struct fsl_dma_info *dma_info)
{
	static int initialized;

	
	if (initialized)
		return 0;

	dma_global_data.ssi_stx_phys = dma_info->ssi_stx_phys;
	dma_global_data.ssi_srx_phys = dma_info->ssi_srx_phys;
	dma_global_data.dma_channel[0] = dma_info->dma_channel[0];
	dma_global_data.dma_channel[1] = dma_info->dma_channel[1];
	dma_global_data.irq[0] = dma_info->dma_irq[0];
	dma_global_data.irq[1] = dma_info->dma_irq[1];
	dma_global_data.assigned[0] = 0;
	dma_global_data.assigned[1] = 0;

	initialized = 1;
	return 1;
}
EXPORT_SYMBOL_GPL(fsl_dma_configure);

static int __init fsl_soc_platform_init(void)
{
	return snd_soc_register_platform(&fsl_soc_platform);
}
module_init(fsl_soc_platform_init);

static void __exit fsl_soc_platform_exit(void)
{
	snd_soc_unregister_platform(&fsl_soc_platform);
}
module_exit(fsl_soc_platform_exit);

MODULE_AUTHOR("Timur Tabi <timur@freescale.com>");
MODULE_DESCRIPTION("Freescale Elo DMA ASoC PCM module");
MODULE_LICENSE("GPL");
