

#ifndef _MX1_MX2_PCM_H
#define _MX1_MX2_PCM_H


struct mx1_mx2_pcm_dma_params {
	char *name;			
	unsigned int transfer_type;	
	dma_addr_t per_address;		
	int event_id;			
	int watermark_level;		
	int per_config;			
	int mem_config;			
 };


extern struct snd_soc_platform mx1_mx2_soc_platform;

#endif
