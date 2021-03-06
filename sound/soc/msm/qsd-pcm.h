

#ifndef _QSD_PCM_H
#define _QSD_PCM_H

#include <linux/msm_audio.h>

#include <mach/qdsp6/msm8k_ard.h>
#include <mach/qdsp6/msm8k_cad_write_pcm_format.h>
#include <mach/qdsp6/msm8k_cad_devices.h>
#include <mach/qdsp6/msm8k_cad.h>
#include <mach/qdsp6/msm8k_cad_ioctl.h>
#include <mach/qdsp6/msm8k_cad_volume.h>

extern void register_cb(void *);

#define USE_FORMATS             (SNDRV_PCM_FMTBIT_S16_LE)
#define USE_CHANNELS_MIN        1
#define USE_CHANNELS_MAX        2
#define USE_RATE                (SNDRV_PCM_RATE_8000_48000 \
				| SNDRV_PCM_RATE_CONTINUOUS)
#define USE_RATE_MIN            8000
#define USE_RATE_MAX            48000
#define MAX_BUFFER_SIZE         (4096*4)
#define MAX_PERIOD_SIZE         4096
#define MIN_PERIOD_SIZE         40
#define USE_PERIODS_MAX         1024
#define USE_PERIODS_MIN         1

#define PLAYBACK_STREAMS	4
#define CAPTURE_STREAMS		1

struct audio_locks {
	spinlock_t alsa_lock;
	struct mutex mixer_lock;
	wait_queue_head_t eos_wait;
};

struct qsd_ctl {
	uint16_t tx_volume; 
	uint16_t rx_volume; 
	int32_t strm_volume; 
	uint16_t update;
	int16_t pan;
	uint16_t device; 
	uint16_t tx_mute;		 
	uint16_t rx_mute;		 
};

extern struct audio_locks the_locks;
extern struct snd_pcm_ops qsd_pcm_ops;

struct qsd_audio {
	struct snd_pcm_substream *substream;

	
	char *data;
	dma_addr_t phys;

	unsigned int pcm_size;
	unsigned int pcm_count;
	unsigned int pcm_irq_pos;	
	unsigned int pcm_buf_pos;	

	int dir;
	int opened;
	int enabled;
	int running;
	int stopped;		
	int eos_ack;
	int intcnt;
	int timerintcnt;
	int buffer_cnt;
	int start;

	struct cad_open_struct_type cos;
	uint32_t cad_w_handle;
	struct cad_buf_struct_type cbs;
	atomic_t copy_count;
	struct timer_list timer;	
	unsigned long expiry_delta;
};

extern struct qsd_ctl qsd_glb_ctl;

extern struct snd_soc_dai msm_dais[2];
extern struct snd_soc_codec_device soc_codec_dev_msm;
extern struct snd_soc_platform qsd_soc_platform;

#endif 
