

#ifndef _VIDEO_1394_H
#define _VIDEO_1394_H

#include "ieee1394-ioctl.h"

#define VIDEO1394_DRIVER_NAME "video1394"

#define VIDEO1394_MAX_SIZE 0x4000000

enum {
	VIDEO1394_BUFFER_FREE = 0,
	VIDEO1394_BUFFER_QUEUED,
	VIDEO1394_BUFFER_READY
};

#define VIDEO1394_SYNC_FRAMES          0x00000001
#define VIDEO1394_INCLUDE_ISO_HEADERS  0x00000002
#define VIDEO1394_VARIABLE_PACKET_SIZE 0x00000004

struct video1394_mmap {
	int channel;			
	unsigned int sync_tag;
	unsigned int nb_buffers;
	unsigned int buf_size;
	unsigned int packet_size; 
	unsigned int fps;
	unsigned int syt_offset;
	unsigned int flags;
};


struct video1394_queue_variable {
	unsigned int channel;
	unsigned int buffer;
	unsigned int __user * packet_sizes; 
};

struct video1394_wait {
	unsigned int channel;
	unsigned int buffer;
	struct timeval filltime;	
};


#endif
