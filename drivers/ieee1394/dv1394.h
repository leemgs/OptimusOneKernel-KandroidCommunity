

#ifndef _DV_1394_H
#define _DV_1394_H



#define DV1394_API_VERSION 0x20011127





#define DV1394_MAX_FRAMES 32


#define DV1394_NTSC_PACKETS_PER_FRAME 250
#define DV1394_PAL_PACKETS_PER_FRAME  300


#define DV1394_NTSC_FRAME_SIZE (480 * DV1394_NTSC_PACKETS_PER_FRAME)
#define DV1394_PAL_FRAME_SIZE  (480 * DV1394_PAL_PACKETS_PER_FRAME)



#include "ieee1394-ioctl.h"


enum pal_or_ntsc {
	DV1394_NTSC = 0,
	DV1394_PAL
};





struct dv1394_init {
	
	unsigned int api_version;

	
	unsigned int channel;

	
	unsigned int n_frames;

	
	enum pal_or_ntsc format;

	

	
	unsigned long cip_n;
	unsigned long cip_d;

	
	unsigned int syt_offset;
};









struct dv1394_status {
	
	struct dv1394_init init;

	
	int active_frame;

	
	unsigned int first_clear_frame;

	
	unsigned int n_clear_frames;

	
	unsigned int dropped_frames;

	
};


#endif 
