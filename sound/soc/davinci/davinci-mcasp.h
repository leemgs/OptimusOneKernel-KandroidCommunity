

#ifndef DAVINCI_MCASP_H
#define DAVINCI_MCASP_H

#include <linux/io.h>
#include <mach/asp.h>
#include "davinci-pcm.h"

extern struct snd_soc_dai davinci_mcasp_dai[];

#define DAVINCI_MCASP_RATES	SNDRV_PCM_RATE_8000_96000
#define DAVINCI_MCASP_I2S_DAI	0
#define DAVINCI_MCASP_DIT_DAI	1

enum {
	DAVINCI_AUDIO_WORD_8 = 0,
	DAVINCI_AUDIO_WORD_12,
	DAVINCI_AUDIO_WORD_16,
	DAVINCI_AUDIO_WORD_20,
	DAVINCI_AUDIO_WORD_24,
	DAVINCI_AUDIO_WORD_32,
	DAVINCI_AUDIO_WORD_28,  
};

struct davinci_audio_dev {
	
	struct davinci_pcm_dma_params dma_params[2];
	void __iomem *base;
	int sample_rate;
	struct clk *clk;
	unsigned int codec_fmt;

	
	int	tdm_slots;
	u8	op_mode;
	u8	num_serializer;
	u8	*serial_dir;
	u8	version;

	
	u8	txnumevt;
	u8	rxnumevt;
};

#endif	
