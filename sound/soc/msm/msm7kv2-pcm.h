

#ifndef _MSM_PCM_H
#define _MSM_PCM_H


#include <mach/qdsp5v2/qdsp5audppcmdi.h>
#include <mach/qdsp5v2/qdsp5audppmsg.h>
#include <mach/qdsp5v2/qdsp5audplaycmdi.h>
#include <mach/qdsp5v2/qdsp5audplaymsg.h>
#include <mach/qdsp5v2/audpp.h>
#include <mach/msm_adsp.h>
#include <mach/qdsp5v2/qdsp5audreccmdi.h>
#include <mach/qdsp5v2/qdsp5audrecmsg.h>
#include <mach/qdsp5v2/audpreproc.h>


#define FRAME_NUM               (8)
#define FRAME_SIZE              (2052 * 2)
#define MONO_DATA_SIZE          (2048)
#define STEREO_DATA_SIZE        (MONO_DATA_SIZE * 2)
#define CAPTURE_DMASZ           (FRAME_SIZE * FRAME_NUM)

#define BUFSZ			(960 * 5)
#define PLAYBACK_DMASZ 		(BUFSZ * 2)

#define MSM_PLAYBACK_DEFAULT_VOLUME 0 
#define MSM_PLAYBACK_DEFAULT_PAN 0

#define USE_FORMATS             SNDRV_PCM_FMTBIT_S16_LE
#define USE_CHANNELS_MIN        1
#define USE_CHANNELS_MAX        2

#define USE_RATE                \
			(SNDRV_PCM_RATE_8000_48000 | SNDRV_PCM_RATE_KNOT)
#define USE_RATE_MIN            8000
#define USE_RATE_MAX            48000
#define MAX_BUFFER_PLAYBACK_SIZE \
				PLAYBACK_DMASZ

#define CAPTURE_SIZE		4096
#define MAX_BUFFER_CAPTURE_SIZE (4096*4)
#define MAX_PERIOD_SIZE         BUFSZ
#define USE_PERIODS_MAX         1024
#define USE_PERIODS_MIN		1


#define MAX_DB			(16)
#define MIN_DB			(-50)
#define PCMPLAYBACK_DECODERID   5


#define	BUF_INVALID_LEN		0xFFFFFFFF
#define EVENT_MSG_ID 		((uint16_t)~0)

#define AUDDEC_DEC_PCM 		0

#define  AUDPP_DEC_STATUS_SLEEP	0
#define  AUDPP_DEC_STATUS_INIT  1
#define  AUDPP_DEC_STATUS_CFG   2
#define  AUDPP_DEC_STATUS_PLAY  3

extern int copy_count;
extern int intcnt;

struct buffer {
	void *data;
	unsigned size;
	unsigned used;
	unsigned addr;
};

struct buffer_rec {
	void *data;
	unsigned int size;
	unsigned int read;
	unsigned int addr;
};

struct audio_locks {
	struct mutex lock;
	struct mutex write_lock;
	struct mutex read_lock;
	spinlock_t read_dsp_lock;
	spinlock_t write_dsp_lock;
	spinlock_t mixer_lock;
	wait_queue_head_t read_wait;
	wait_queue_head_t write_wait;
	wait_queue_head_t wait;
	wait_queue_head_t eos_wait;
	wait_queue_head_t enable_wait;
};

extern struct audio_locks the_locks;

struct msm_audio_event_callbacks {
	
	void (*playback)(void *);
	void (*capture)(void *);
};


struct msm_audio {
	struct buffer out[2];
	struct buffer_rec in[8];

	uint8_t out_head;
	uint8_t out_tail;
	uint8_t out_needed; 
	atomic_t out_bytes;

	
	uint32_t out_sample_rate;
	uint32_t out_channel_mode;
	uint32_t out_weight;
	uint32_t out_buffer_size;

	struct snd_pcm_substream *substream;

	
	char *data;
	dma_addr_t phys;

	unsigned int pcm_size;
	unsigned int pcm_count;
	unsigned int pcm_irq_pos;       
	unsigned int pcm_buf_pos;       
	uint16_t source; 

	struct msm_adsp_module *audpre;
	struct msm_adsp_module *audrec;
	struct msm_adsp_module *audplay;
	enum msm_aud_decoder_state dec_state; 

	uint16_t session_id;
	uint32_t out_bits; 
	const char *module_name;
	unsigned queue_id;

	
	uint32_t samp_rate;
	uint32_t channel_mode;
	uint32_t buffer_size; 
	uint32_t type; 
	uint32_t dsp_cnt;
	uint32_t in_head; 
	uint32_t in_tail; 
	uint32_t in_count; 

	unsigned short samp_rate_index;
	uint32_t device_events; 
	int abort; 

	
	

	struct  msm_audio_event_callbacks *ops;

	int dir;
	int opened;
	int enabled;
	int running;
	int stopped; 
	int eos_ack;
	int mmap_flag;
	int period;
	struct audpp_cmd_cfg_object_params_volume vol_pan;
};




extern int alsa_dsp_send_buffer(struct msm_audio *prtd,
			unsigned idx, unsigned len);
extern int audio_dsp_out_enable(struct msm_audio *prtd, int yes);
extern struct snd_soc_platform msm_soc_platform;
extern struct snd_soc_dai msm_dais[2];
extern struct snd_soc_codec_device soc_codec_dev_msm;

extern int audrec_encoder_config(struct msm_audio *prtd);
extern int alsa_audrec_disable(struct msm_audio *prtd);
extern int alsa_audio_configure(struct msm_audio *prtd);
extern int alsa_audio_disable(struct msm_audio *prtd);
extern int alsa_buffer_read(struct msm_audio *prtd, void __user *buf,
		size_t count, loff_t *pos);
ssize_t alsa_send_buffer(struct msm_audio *prtd, const char __user *buf,
		size_t count, loff_t *pos);
extern struct msm_adsp_ops alsa_audrec_adsp_ops;
extern int alsa_in_record_config(struct msm_audio *prtd, int enable);
#endif 
