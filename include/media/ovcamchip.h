

#ifndef __LINUX_OVCAMCHIP_H
#define __LINUX_OVCAMCHIP_H

#include <linux/videodev.h>
#include <media/v4l2-common.h>






enum {
	OVCAMCHIP_CID_CONT,		
	OVCAMCHIP_CID_BRIGHT,		
	OVCAMCHIP_CID_SAT,		
	OVCAMCHIP_CID_HUE,		
	OVCAMCHIP_CID_EXP,		
	OVCAMCHIP_CID_FREQ,		
	OVCAMCHIP_CID_BANDFILT,		
	OVCAMCHIP_CID_AUTOBRIGHT,	
	OVCAMCHIP_CID_AUTOEXP,		
	OVCAMCHIP_CID_BACKLIGHT,	
	OVCAMCHIP_CID_MIRROR,		
};


#define NUM_CC_TYPES	9
enum {
	CC_UNKNOWN,
	CC_OV76BE,
	CC_OV7610,
	CC_OV7620,
	CC_OV7620AE,
	CC_OV6620,
	CC_OV6630,
	CC_OV6630AE,
	CC_OV6630AF,
};





#define OV7xx0_SID   (0x42 >> 1)
#define OV6xx0_SID   (0xC0 >> 1)





struct ovcamchip_control {
	__u32 id;
	__s32 value;
};

struct ovcamchip_window {
	int x;
	int y;
	int width;
	int height;
	int format;
	int quarter;		

	
	int clockdiv;		
};


#define OVCAMCHIP_CMD_Q_SUBTYPE     _IOR  (0x88, 0x00, int)
#define OVCAMCHIP_CMD_INITIALIZE    _IOW  (0x88, 0x01, int)

#define OVCAMCHIP_CMD_S_CTRL        _IOW  (0x88, 0x02, struct ovcamchip_control)
#define OVCAMCHIP_CMD_G_CTRL        _IOWR (0x88, 0x03, struct ovcamchip_control)
#define OVCAMCHIP_CMD_S_MODE        _IOW  (0x88, 0x04, struct ovcamchip_window)
#define OVCAMCHIP_MAX_CMD           _IO   (0x88, 0x3f)

#endif
