

#ifndef __VIAFBDEV_H__
#define __VIAFBDEV_H__

#include <linux/proc_fs.h>
#include <linux/fb.h>

#include "ioctl.h"
#include "share.h"
#include "chip.h"
#include "hw.h"
#include "via_i2c.h"

#define VERSION_MAJOR       2
#define VERSION_KERNEL      6	

#define VERSION_OS          0	
#define VERSION_MINOR       4

struct viafb_shared {
	struct proc_dir_entry *proc_entry;	

	
	struct via_i2c_stuff i2c_stuff;

	
	struct tmds_setting_information tmds_setting_info;
	struct crt_setting_information crt_setting_info;
	struct lvds_setting_information lvds_setting_info;
	struct lvds_setting_information lvds_setting_info2;
	struct chip_information chip_info;

	
	void __iomem *engine_mmio;
	u32 cursor_vram_addr;
	u32 vq_vram_addr;	
	int (*hw_bitblt)(void __iomem *engine, u8 op, u32 width, u32 height,
		u8 dst_bpp, u32 dst_addr, u32 dst_pitch, u32 dst_x, u32 dst_y,
		u32 *src_mem, u32 src_addr, u32 src_pitch, u32 src_x, u32 src_y,
		u32 fg_color, u32 bg_color, u8 fill_rop);
};

struct viafb_par {
	u8 depth;
	u32 vram_addr;

	unsigned int fbmem;	
	unsigned int memsize;	
	u32 fbmem_free;		
	u32 fbmem_used;		
	u32 iga_path;

	struct viafb_shared *shared;

	
	
	struct tmds_setting_information *tmds_setting_info;
	struct crt_setting_information *crt_setting_info;
	struct lvds_setting_information *lvds_setting_info;
	struct lvds_setting_information *lvds_setting_info2;
	struct chip_information *chip_info;
};

extern unsigned int viafb_second_virtual_yres;
extern unsigned int viafb_second_virtual_xres;
extern unsigned int viafb_second_offset;
extern int viafb_second_size;
extern int viafb_SAMM_ON;
extern int viafb_dual_fb;
extern int viafb_LCD2_ON;
extern int viafb_LCD_ON;
extern int viafb_DVI_ON;
extern int viafb_hotplug;
extern int viafb_memsize;

extern int strict_strtoul(const char *cp, unsigned int base,
	unsigned long *res);

void viafb_fill_var_timing_info(struct fb_var_screeninfo *var, int refresh,
			  int mode_index);
int viafb_get_mode_index(int hres, int vres);
u8 viafb_gpio_i2c_read_lvds(struct lvds_setting_information
	*plvds_setting_info, struct lvds_chip_information
	*plvds_chip_info, u8 index);
void viafb_gpio_i2c_write_mask_lvds(struct lvds_setting_information
			      *plvds_setting_info, struct lvds_chip_information
			      *plvds_chip_info, struct IODATA io_data);
#endif 
