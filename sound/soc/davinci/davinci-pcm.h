

#ifndef _DAVINCI_PCM_H
#define _DAVINCI_PCM_H

#include <mach/edma.h>
#include <mach/asp.h>


struct davinci_pcm_dma_params {
	int channel;			
	unsigned short acnt;
	dma_addr_t dma_addr;		
	enum dma_event_q eventq_no;	
	unsigned char data_type;	
	unsigned char convert_mono_stereo;
};


extern struct snd_soc_platform davinci_soc_platform;

#endif
