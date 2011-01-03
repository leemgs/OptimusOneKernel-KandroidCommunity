

#ifndef __OMAP_PCM_H__
#define __OMAP_PCM_H__

struct omap_pcm_dma_data {
	char		*name;		
	int		dma_req;	
	unsigned long	port_addr;	
	int		sync_mode;	
	void (*set_threshold)(struct snd_pcm_substream *substream);
};

extern struct snd_soc_platform omap_soc_platform;

#endif
