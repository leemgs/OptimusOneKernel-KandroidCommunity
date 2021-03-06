#ifndef _SCREEN_INFO_H
#define _SCREEN_INFO_H

#include <linux/types.h>



struct screen_info {
	__u8  orig_x;		
	__u8  orig_y;		
	__u16 ext_mem_k;	
	__u16 orig_video_page;	
	__u8  orig_video_mode;	
	__u8  orig_video_cols;	
	__u16 unused2;		
	__u16 orig_video_ega_bx;
	__u16 unused3;		
	__u8  orig_video_lines;	
	__u8  orig_video_isVGA;	
	__u16 orig_video_points;

	
	__u16 lfb_width;	
	__u16 lfb_height;	
	__u16 lfb_depth;	
	__u32 lfb_base;		
	__u32 lfb_size;		
	__u16 cl_magic, cl_offset; 
	__u16 lfb_linelength;	
	__u8  red_size;		
	__u8  red_pos;		
	__u8  green_size;	
	__u8  green_pos;	
	__u8  blue_size;	
	__u8  blue_pos;		
	__u8  rsvd_size;	
	__u8  rsvd_pos;		
	__u16 vesapm_seg;	
	__u16 vesapm_off;	
	__u16 pages;		
	__u16 vesa_attributes;	
	__u32 capabilities;     
	__u8  _reserved[6];	
} __attribute__((packed));

#define VIDEO_TYPE_MDA		0x10	
#define VIDEO_TYPE_CGA		0x11	
#define VIDEO_TYPE_EGAM		0x20	
#define VIDEO_TYPE_EGAC		0x21	
#define VIDEO_TYPE_VGAC		0x22	
#define VIDEO_TYPE_VLFB		0x23	

#define VIDEO_TYPE_PICA_S3	0x30	
#define VIDEO_TYPE_MIPS_G364	0x31    
#define VIDEO_TYPE_SGI          0x33    

#define VIDEO_TYPE_TGAC		0x40	

#define VIDEO_TYPE_SUN          0x50    
#define VIDEO_TYPE_SUNPCI       0x51    

#define VIDEO_TYPE_PMAC		0x60	

#define VIDEO_TYPE_EFI		0x70	

#ifdef __KERNEL__
extern struct screen_info screen_info;

#define ORIG_X			(screen_info.orig_x)
#define ORIG_Y			(screen_info.orig_y)
#define ORIG_VIDEO_MODE		(screen_info.orig_video_mode)
#define ORIG_VIDEO_COLS 	(screen_info.orig_video_cols)
#define ORIG_VIDEO_EGA_BX	(screen_info.orig_video_ega_bx)
#define ORIG_VIDEO_LINES	(screen_info.orig_video_lines)
#define ORIG_VIDEO_ISVGA	(screen_info.orig_video_isVGA)
#define ORIG_VIDEO_POINTS       (screen_info.orig_video_points)
#endif 

#endif 
