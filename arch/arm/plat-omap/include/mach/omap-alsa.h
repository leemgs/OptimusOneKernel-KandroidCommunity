

#ifndef __OMAP_ALSA_H
#define __OMAP_ALSA_H

#include <mach/dma.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <mach/mcbsp.h>
#include <linux/platform_device.h>

#define DMA_BUF_SIZE	(1024 * 8)


struct audio_stream {
	char *id;		
	int stream_id;		
	int dma_dev;		
	int *lch;		
	char started;		
	int dma_q_head;		
	int dma_q_tail;		
	char dma_q_count;	
	int active:1;		
	int period;		
	int periods;		
	spinlock_t dma_lock;	
	struct snd_pcm_substream *stream;	
	unsigned linked:1;	
	int offset;		
	int (*hw_start)(void);  
	int (*hw_stop)(void);   
};


struct snd_card_omap_codec {
	struct snd_card *card;
	struct snd_pcm *pcm;
	long samplerate;
	struct audio_stream s[2];	
};


struct omap_alsa_codec_config {
	char 	*name;
	struct	omap_mcbsp_reg_cfg *mcbsp_regs_alsa;
	struct	snd_pcm_hw_constraint_list *hw_constraints_rates;
	struct	snd_pcm_hardware *snd_omap_alsa_playback;
	struct	snd_pcm_hardware *snd_omap_alsa_capture;
	void	(*codec_configure_dev)(void);
	void	(*codec_set_samplerate)(long);
	void	(*codec_clock_setup)(void);
	int	(*codec_clock_on)(void);
	int 	(*codec_clock_off)(void);
	int	(*get_default_samplerate)(void);
};


int snd_omap_mixer(struct snd_card_omap_codec *);
void snd_omap_init_mixer(void);

#ifdef CONFIG_PM
void snd_omap_suspend_mixer(void);
void snd_omap_resume_mixer(void);
#endif

int snd_omap_alsa_post_probe(struct platform_device *pdev, struct omap_alsa_codec_config *config);
int snd_omap_alsa_remove(struct platform_device *pdev);
#ifdef CONFIG_PM
int snd_omap_alsa_suspend(struct platform_device *pdev, pm_message_t state);
int snd_omap_alsa_resume(struct platform_device *pdev);
#else
#define snd_omap_alsa_suspend	NULL
#define snd_omap_alsa_resume	NULL
#endif

void callback_omap_alsa_sound_dma(void *);

#endif
