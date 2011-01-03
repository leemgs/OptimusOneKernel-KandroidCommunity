



#ifndef PCM_H
#define PCM_H


#include <sound/pcm.h>

#include "driver.h"
#include "usbdefs.h"



#define LINE6_ISO_BUFFERS	8


#define LINE6_ISO_PACKETS	2


#define LINE6_ISO_INTERVAL	1


#define LINE6_ISO_PACKET_SIZE_MAX	252



#define s2m(s)	(((struct snd_line6_pcm *) \
		   snd_pcm_substream_chip(s))->line6->ifcdev)


enum {
	BIT_RUNNING_PLAYBACK,
	BIT_RUNNING_CAPTURE,
	BIT_PAUSE_PLAYBACK,
	BIT_PREPARED
};

struct line6_pcm_properties {
	struct snd_pcm_hardware snd_line6_playback_hw, snd_line6_capture_hw;
	struct snd_pcm_hw_constraint_ratdens snd_line6_rates;
	int bytes_per_frame;
};

struct snd_line6_pcm {
	
	struct usb_line6 *line6;

	
	struct line6_pcm_properties *properties;

	
	struct snd_pcm *pcm;

	
	struct urb *urb_audio_out[LINE6_ISO_BUFFERS];

	
	struct urb *urb_audio_in[LINE6_ISO_BUFFERS];

	
	unsigned char *wrap_out;

	
	unsigned char *buffer_in;

	
	snd_pcm_uframes_t pos_out;

	
	unsigned bytes_out;

	
	unsigned count_out;

	
	unsigned period_out;

	
	snd_pcm_uframes_t pos_out_done;

	
	unsigned bytes_in;

	
	unsigned count_in;

	
	unsigned period_in;

	
	snd_pcm_uframes_t pos_in_done;

	
	unsigned long active_urb_out;

	
	int max_packet_size;

	
	int ep_audio_read;

	
	int ep_audio_write;

	
	unsigned long active_urb_in;

	
	unsigned long unlink_urb_out;

	
	unsigned long unlink_urb_in;

	
	spinlock_t lock_audio_out;

	
	spinlock_t lock_audio_in;

	
	spinlock_t lock_trigger;

	
	int volume[2];

	
	unsigned long flags;
};


extern int line6_init_pcm(struct usb_line6 *line6,
			  struct line6_pcm_properties *properties);
extern int snd_line6_trigger(struct snd_pcm_substream *substream, int cmd);
extern int snd_line6_prepare(struct snd_pcm_substream *substream);


#endif
