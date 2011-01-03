

#ifndef __SOUND_SOC_FSL_MPC5200_DMA_H__
#define __SOUND_SOC_FSL_MPC5200_DMA_H__

#define PSC_STREAM_NAME_LEN 32


struct psc_dma_stream {
	struct snd_pcm_runtime *runtime;
	snd_pcm_uframes_t appl_ptr;

	int active;
	struct psc_dma *psc_dma;
	struct bcom_task *bcom_task;
	int irq;
	struct snd_pcm_substream *stream;
	dma_addr_t period_start;
	dma_addr_t period_end;
	dma_addr_t period_next_pt;
	dma_addr_t period_current_pt;
	int period_bytes;
	int period_size;
};


struct psc_dma {
	char name[32];
	struct mpc52xx_psc __iomem *psc_regs;
	struct mpc52xx_psc_fifo __iomem *fifo_regs;
	unsigned int irq;
	struct device *dev;
	spinlock_t lock;
	struct mutex mutex;
	u32 sicr;
	uint sysclk;
	int imr;
	int id;
	unsigned int slots;

	
	struct psc_dma_stream playback;
	struct psc_dma_stream capture;

	
	struct {
		unsigned long overrun_count;
		unsigned long underrun_count;
	} stats;
};

int mpc5200_audio_dma_create(struct of_device *op);
int mpc5200_audio_dma_destroy(struct of_device *op);

extern struct snd_soc_platform mpc5200_audio_dma_platform;

#endif 
