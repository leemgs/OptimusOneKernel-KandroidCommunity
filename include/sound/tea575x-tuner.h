#ifndef __SOUND_TEA575X_TUNER_H
#define __SOUND_TEA575X_TUNER_H



#include <linux/videodev2.h>
#include <media/v4l2-dev.h>
#include <media/v4l2-ioctl.h>

struct snd_tea575x;

struct snd_tea575x_ops {
	void (*write)(struct snd_tea575x *tea, unsigned int val);
	unsigned int (*read)(struct snd_tea575x *tea);
	void (*mute)(struct snd_tea575x *tea, unsigned int mute);
};

struct snd_tea575x {
	struct snd_card *card;
	struct video_device *vd;	
	int dev_nr;			
	int tea5759;			
	int mute;			
	unsigned int freq_fixup;	
	unsigned int val;		
	unsigned long freq;		
	unsigned long in_use;		
	struct snd_tea575x_ops *ops;
	void *private_data;
};

void snd_tea575x_init(struct snd_tea575x *tea);
void snd_tea575x_exit(struct snd_tea575x *tea);

#endif 
