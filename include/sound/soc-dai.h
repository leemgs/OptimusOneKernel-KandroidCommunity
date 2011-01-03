

#ifndef __LINUX_SND_SOC_DAI_H
#define __LINUX_SND_SOC_DAI_H


#include <linux/list.h>

struct snd_pcm_substream;


#define SND_SOC_DAIFMT_I2S		0 
#define SND_SOC_DAIFMT_RIGHT_J		1 
#define SND_SOC_DAIFMT_LEFT_J		2 
#define SND_SOC_DAIFMT_DSP_A		3 
#define SND_SOC_DAIFMT_DSP_B		4 
#define SND_SOC_DAIFMT_AC97		5 


#define SND_SOC_DAIFMT_MSB		SND_SOC_DAIFMT_LEFT_J
#define SND_SOC_DAIFMT_LSB		SND_SOC_DAIFMT_RIGHT_J


#define SND_SOC_DAIFMT_CONT		(0 << 4) 
#define SND_SOC_DAIFMT_GATED		(1 << 4) 


#define SND_SOC_DAIFMT_NB_NF		(0 << 8) 
#define SND_SOC_DAIFMT_NB_IF		(1 << 8) 
#define SND_SOC_DAIFMT_IB_NF		(2 << 8) 
#define SND_SOC_DAIFMT_IB_IF		(3 << 8) 


#define SND_SOC_DAIFMT_CBM_CFM		(0 << 12) 
#define SND_SOC_DAIFMT_CBS_CFM		(1 << 12) 
#define SND_SOC_DAIFMT_CBM_CFS		(2 << 12) 
#define SND_SOC_DAIFMT_CBS_CFS		(3 << 12) 

#define SND_SOC_DAIFMT_FORMAT_MASK	0x000f
#define SND_SOC_DAIFMT_CLOCK_MASK	0x00f0
#define SND_SOC_DAIFMT_INV_MASK		0x0f00
#define SND_SOC_DAIFMT_MASTER_MASK	0xf000


#define SND_SOC_CLOCK_IN		0
#define SND_SOC_CLOCK_OUT		1

#define SND_SOC_STD_AC97_FMTS (SNDRV_PCM_FMTBIT_S8 |\
			       SNDRV_PCM_FMTBIT_S16_LE |\
			       SNDRV_PCM_FMTBIT_S16_BE |\
			       SNDRV_PCM_FMTBIT_S20_3LE |\
			       SNDRV_PCM_FMTBIT_S20_3BE |\
			       SNDRV_PCM_FMTBIT_S24_3LE |\
			       SNDRV_PCM_FMTBIT_S24_3BE |\
                               SNDRV_PCM_FMTBIT_S32_LE |\
                               SNDRV_PCM_FMTBIT_S32_BE)

struct snd_soc_dai_ops;
struct snd_soc_dai;
struct snd_ac97_bus_ops;


int snd_soc_register_dai(struct snd_soc_dai *dai);
void snd_soc_unregister_dai(struct snd_soc_dai *dai);
int snd_soc_register_dais(struct snd_soc_dai *dai, size_t count);
void snd_soc_unregister_dais(struct snd_soc_dai *dai, size_t count);


int snd_soc_dai_set_sysclk(struct snd_soc_dai *dai, int clk_id,
	unsigned int freq, int dir);

int snd_soc_dai_set_clkdiv(struct snd_soc_dai *dai,
	int div_id, int div);

int snd_soc_dai_set_pll(struct snd_soc_dai *dai,
	int pll_id, unsigned int freq_in, unsigned int freq_out);


int snd_soc_dai_set_fmt(struct snd_soc_dai *dai, unsigned int fmt);

int snd_soc_dai_set_tdm_slot(struct snd_soc_dai *dai,
	unsigned int tx_mask, unsigned int rx_mask, int slots, int slot_width);

int snd_soc_dai_set_tristate(struct snd_soc_dai *dai, int tristate);


int snd_soc_dai_digital_mute(struct snd_soc_dai *dai, int mute);


struct snd_soc_dai_ops {
	
	int (*set_sysclk)(struct snd_soc_dai *dai,
		int clk_id, unsigned int freq, int dir);
	int (*set_pll)(struct snd_soc_dai *dai,
		int pll_id, unsigned int freq_in, unsigned int freq_out);
	int (*set_clkdiv)(struct snd_soc_dai *dai, int div_id, int div);

	
	int (*set_fmt)(struct snd_soc_dai *dai, unsigned int fmt);
	int (*set_tdm_slot)(struct snd_soc_dai *dai,
		unsigned int tx_mask, unsigned int rx_mask,
		int slots, int slot_width);
	int (*set_tristate)(struct snd_soc_dai *dai, int tristate);

	
	int (*digital_mute)(struct snd_soc_dai *dai, int mute);

	
	int (*startup)(struct snd_pcm_substream *,
		struct snd_soc_dai *);
	void (*shutdown)(struct snd_pcm_substream *,
		struct snd_soc_dai *);
	int (*hw_params)(struct snd_pcm_substream *,
		struct snd_pcm_hw_params *, struct snd_soc_dai *);
	int (*hw_free)(struct snd_pcm_substream *,
		struct snd_soc_dai *);
	int (*prepare)(struct snd_pcm_substream *,
		struct snd_soc_dai *);
	int (*trigger)(struct snd_pcm_substream *, int,
		struct snd_soc_dai *);
};


struct snd_soc_dai {
	
	char *name;
	unsigned int id;
	int ac97_control;

	struct device *dev;
	void *ac97_pdata;	

	
	int (*probe)(struct platform_device *pdev,
		     struct snd_soc_dai *dai);
	void (*remove)(struct platform_device *pdev,
		       struct snd_soc_dai *dai);
	int (*suspend)(struct snd_soc_dai *dai);
	int (*resume)(struct snd_soc_dai *dai);

	
	struct snd_soc_dai_ops *ops;

	
	struct snd_soc_pcm_stream capture;
	struct snd_soc_pcm_stream playback;
	unsigned int symmetric_rates:1;

	
	struct snd_pcm_runtime *runtime;
	struct snd_soc_codec *codec;
	unsigned int active;
	unsigned char pop_wait:1;
	void *dma_data;

	
	void *private_data;

	
	struct snd_soc_platform *platform;

	struct list_head list;
};

#endif
