
#ifndef CODEC_UTILS_H
#define CODEC_UTILS_H

#include <linux/earlysuspend.h>

#define ADRV_STATUS_AIO_INTF 0x00000001
#define ADRV_STATUS_OBUF_GIVEN 0x00000002
#define ADRV_STATUS_IBUF_GIVEN 0x00000004
#define ADRV_STATUS_FSYNC 0x00000008

#define PCM_BUFSZ_MIN 4800	
#define PCM_BUF_MAX_COUNT 5	
#define ROUTING_MODE_FTRT 1
#define ROUTING_MODE_RT 2

#define  AUDPP_DEC_STATUS_SLEEP	0
#define	 AUDPP_DEC_STATUS_INIT  1
#define  AUDPP_DEC_STATUS_CFG   2
#define  AUDPP_DEC_STATUS_PLAY  3
#define  AUDPP_DEC_STATUS_EOS   5


#define AVSYNC_EVENT_TIMEOUT 3000

struct buffer {
	void *data;
	unsigned size;
	unsigned used;		
	unsigned addr;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
struct audio_suspend_ctl {
	struct early_suspend node;
	struct audio *audio;
};
#endif

struct codec_operations {
	long (*ioctl)(struct file *, unsigned int, unsigned long);
	void (*adec_params)(struct audio *);
};

struct audio {
	spinlock_t dsp_lock;

	uint8_t out_needed; 
	struct list_head out_queue; 
	atomic_t out_bytes;

	struct mutex lock;
	struct mutex write_lock;
	wait_queue_head_t write_wait;

	struct msm_adsp_module *audplay;

	
	uint32_t out_sample_rate;
	uint32_t out_channel_mode;
	uint32_t out_bits; 

	
	char *data;
	int32_t phys; 

	uint32_t drv_status;
	int wflush; 
	int opened;
	int enabled;
	int running;
	int stopped; 
	int buf_refresh;
	int teos; 
	enum msm_aud_decoder_state dec_state;	
	int reserved; 
	char rsv_byte; 

	const char *module_name;
	unsigned queue_id;
	uint16_t dec_id;
	uint32_t read_ptr_offset;
	int16_t source;

#ifdef CONFIG_HAS_EARLYSUSPEND
	struct audio_suspend_ctl suspend_ctl;
#endif

#ifdef CONFIG_DEBUG_FS
	struct dentry *dentry;
#endif

	wait_queue_head_t wait;
	struct list_head free_event_queue;
	struct list_head event_queue;
	wait_queue_head_t event_wait;
	spinlock_t event_queue_lock;
	struct mutex get_event_lock;
	int event_abort;
	
	int avsync_flag;              
	wait_queue_head_t avsync_wait;
	
	uint16_t avsync[AUDPP_AVSYNC_CH_COUNT * AUDPP_AVSYNC_NUM_WORDS + 1];

	uint32_t device_events;
	uint32_t device_switch;       
	uint64_t bytecount_consumed;
	uint64_t bytecount_head;
	uint64_t bytecount_given;
	uint64_t bytecount_query;

	struct list_head pmem_region_queue; 

	int eq_enable;
	int eq_needs_commit;
	struct audpp_cmd_cfg_object_params_eqalizer eq;
	struct audpp_cmd_cfg_object_params_volume vol_pan;

	unsigned int minor_no;
	struct codec_operations codec_ops;
};

#endif 
