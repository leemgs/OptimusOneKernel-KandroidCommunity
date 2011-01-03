#ifndef __SOUND_SEQ_DEVICE_H
#define __SOUND_SEQ_DEVICE_H





#define ID_LEN	32


#define SNDRV_SEQ_DEVICE_FREE		0
#define SNDRV_SEQ_DEVICE_REGISTERED	1

struct snd_seq_device {
	
	struct snd_card *card;	
	int device;		
	char id[ID_LEN];	
	char name[80];		
	int argsize;		
	void *driver_data;	
	int status;		
	void *private_data;	
	void (*private_free)(struct snd_seq_device *device);
	struct list_head list;	
};



struct snd_seq_dev_ops {
	int (*init_device)(struct snd_seq_device *dev);
	int (*free_device)(struct snd_seq_device *dev);
};


void snd_seq_device_load_drivers(void);
int snd_seq_device_new(struct snd_card *card, int device, char *id, int argsize, struct snd_seq_device **result);
int snd_seq_device_register_driver(char *id, struct snd_seq_dev_ops *entry, int argsize);
int snd_seq_device_unregister_driver(char *id);

#define SNDRV_SEQ_DEVICE_ARGPTR(dev) (void *)((char *)(dev) + sizeof(struct snd_seq_device))



#define SNDRV_SEQ_DEV_ID_MIDISYNTH	"seq-midi"
#define SNDRV_SEQ_DEV_ID_OPL3		"opl3-synth"

#endif 
