

#ifndef _BF5XX_AC97_PCM_H
#define _BF5XX_AC97_PCM_H

struct bf5xx_pcm_dma_params {
	char *name;			
};

struct bf5xx_gpio {
	u32 sys;
	u32 rx;
	u32 tx;
	u32 clk;
	u32 frm;
};


extern struct snd_soc_platform bf5xx_ac97_soc_platform;

#endif
