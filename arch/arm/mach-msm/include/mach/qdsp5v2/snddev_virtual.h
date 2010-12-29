
#ifndef __MACH_QDSP5_V2_SNDDEV_VIRTUAL_H
#define __MACH_QDSP5_V2_SNDDEV_VIRTUAL_H
#include <mach/qdsp5v2/audio_def.h>

struct snddev_virtual_data {
	u32 capability; 
	const char *name;
	u32 copp_id; 
	u32 acdb_id; 
};
#endif
