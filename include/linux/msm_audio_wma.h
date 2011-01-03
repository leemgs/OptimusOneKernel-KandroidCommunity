

#ifndef __MSM_AUDIO_WMA_H
#define __MSM_AUDIO_WMA_H

#define AUDIO_GET_WMA_CONFIG  _IOR(AUDIO_IOCTL_MAGIC, \
	  (AUDIO_MAX_COMMON_IOCTL_NUM+0), unsigned)
#define AUDIO_SET_WMA_CONFIG  _IOW(AUDIO_IOCTL_MAGIC, \
	  (AUDIO_MAX_COMMON_IOCTL_NUM+1), unsigned)

struct msm_audio_wma_config {
	unsigned short 	armdatareqthr;
	unsigned short 	channelsdecoded;
	unsigned short 	wmabytespersec;
	unsigned short	wmasamplingfreq;
	unsigned short	wmaencoderopts;
};

#endif 
