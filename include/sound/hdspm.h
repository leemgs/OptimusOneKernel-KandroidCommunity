#ifndef __SOUND_HDSPM_H
#define __SOUND_HDSPM_H



#define HDSPM_MAX_CHANNELS      64





struct hdspm_peak_rms {

	unsigned int level_offset[1024];

	unsigned int input_peak[64];
	unsigned int playback_peak[64];
	unsigned int output_peak[64];
	unsigned int xxx_peak[64];	

	unsigned int reserved[256];	

	unsigned int input_rms_l[64];
	unsigned int playback_rms_l[64];
	unsigned int output_rms_l[64];
	unsigned int xxx_rms_l[64];	

	unsigned int input_rms_h[64];
	unsigned int playback_rms_h[64];
	unsigned int output_rms_h[64];
	unsigned int xxx_rms_h[64];	
};

struct hdspm_peak_rms_ioctl {
	struct hdspm_peak_rms *peak;
};


#define SNDRV_HDSPM_IOCTL_GET_PEAK_RMS \
	_IOR('H', 0x40, struct hdspm_peak_rms_ioctl)



struct hdspm_config_info {
	unsigned char pref_sync_ref;
	unsigned char wordclock_sync_check;
	unsigned char madi_sync_check;
	unsigned int system_sample_rate;
	unsigned int autosync_sample_rate;
	unsigned char system_clock_mode;
	unsigned char clock_source;
	unsigned char autosync_ref;
	unsigned char line_out;
	unsigned int passthru;
	unsigned int analog_out;
};

#define SNDRV_HDSPM_IOCTL_GET_CONFIG_INFO \
	_IOR('H', 0x41, struct hdspm_config_info)




struct hdspm_version {
	unsigned short firmware_rev;
};

#define SNDRV_HDSPM_IOCTL_GET_VERSION _IOR('H', 0x43, struct hdspm_version)










#define HDSPM_MIXER_CHANNELS HDSPM_MAX_CHANNELS

struct hdspm_channelfader {
	unsigned int in[HDSPM_MIXER_CHANNELS];
	unsigned int pb[HDSPM_MIXER_CHANNELS];
};

struct hdspm_mixer {
	struct hdspm_channelfader ch[HDSPM_MIXER_CHANNELS];
};

struct hdspm_mixer_ioctl {
	struct hdspm_mixer *mixer;
};


#define SNDRV_HDSPM_IOCTL_GET_MIXER _IOR('H', 0x44, struct hdspm_mixer_ioctl)


typedef struct hdspm_peak_rms hdspm_peak_rms_t;
typedef struct hdspm_config_info hdspm_config_info_t;
typedef struct hdspm_version hdspm_version_t;
typedef struct hdspm_channelfader snd_hdspm_channelfader_t;
typedef struct hdspm_mixer hdspm_mixer_t;

#endif				
