

#ifndef HTC_35MM_REMOTE_H
#define HTC_35MM_REMOTE_H


int htc_35mm_jack_plug_event(int insert, int *hpin_stable);
int htc_35mm_key_event(int key, int *hpin_stable);


struct h35mm_platform_data {
	int (*plug_event_enable)(void);
	int (*headset_has_mic)(void);
	int (*key_event_enable)(void);
	int (*key_event_disable)(void);
};
#endif
