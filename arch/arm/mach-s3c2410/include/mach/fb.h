

#ifndef __ASM_ARM_FB_H
#define __ASM_ARM_FB_H

#include <mach/regs-lcd.h>

struct s3c2410fb_hw {
	unsigned long	lcdcon1;
	unsigned long	lcdcon2;
	unsigned long	lcdcon3;
	unsigned long	lcdcon4;
	unsigned long	lcdcon5;
};


struct s3c2410fb_display {
	
	unsigned type;

	
	unsigned short width;
	unsigned short height;

	
	unsigned short xres;
	unsigned short yres;
	unsigned short bpp;

	unsigned pixclock;		
	unsigned short left_margin;  
	unsigned short right_margin; 
	unsigned short hsync_len;    
	unsigned short upper_margin;	
	unsigned short lower_margin;	
	unsigned short vsync_len;	

	
	unsigned long	lcdcon5;
};

struct s3c2410fb_mach_info {

	struct s3c2410fb_display *displays;	
	unsigned num_displays;			
	unsigned default_display;

	

	unsigned long	gpcup;
	unsigned long	gpcup_mask;
	unsigned long	gpccon;
	unsigned long	gpccon_mask;
	unsigned long	gpdup;
	unsigned long	gpdup_mask;
	unsigned long	gpdcon;
	unsigned long	gpdcon_mask;

	
	unsigned long	lpcsel;
};

extern void __init s3c24xx_fb_set_platdata(struct s3c2410fb_mach_info *);

#endif 
