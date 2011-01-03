

#ifndef __XILINXFB_H__
#define __XILINXFB_H__

#include <linux/types.h>


struct xilinxfb_platform_data {
	u32 rotate_screen;	
	u32 screen_height_mm;	
	u32 screen_width_mm;
	u32 xres, yres;		
	u32 xvirt, yvirt;	

	
	u32 fb_phys;
};

#endif  
