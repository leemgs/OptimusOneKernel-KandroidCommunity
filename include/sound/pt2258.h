      

#ifndef __SOUND_PT2258_H
#define __SOUND_PT2258_H

struct snd_pt2258 {
	struct snd_card *card;
	struct snd_i2c_bus *i2c_bus;
	struct snd_i2c_device *i2c_dev;

	unsigned char volume[6];
	int mute;
};

extern int snd_pt2258_reset(struct snd_pt2258 *pt);
extern int snd_pt2258_build_controls(struct snd_pt2258 *pt);

#endif 
