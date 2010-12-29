
#ifndef __MACH_QDSP5_V2_AUDIO_INTERCT_H
#define __MACH_QDSP5_V2_AUDIO_INTERCT_H

#define AUDIO_INTERCT_ADSP 0
#define AUDIO_INTERCT_LPA 1
#define AUDIO_ADSP_A 1
#define AUDIO_ADSP_V 0

void audio_interct_lpa(u32 source);
void audio_interct_aux_regsel(u32 source);
void audio_interct_rpcm_source(u32 source);
void audio_interct_tpcm_source(u32 source);
void audio_interct_pcmmi2s(u32 source);
void audio_interct_codec(u32 source);
void audio_interct_multichannel(u32 source);

#endif
