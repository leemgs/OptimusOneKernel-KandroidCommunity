

#ifndef __ARCH_ARM_MACH_OMAP_COMMON_H
#define __ARCH_ARM_MACH_OMAP_COMMON_H

#include <linux/i2c.h>

struct sys_timer;

extern void omap_map_common_io(void);
extern struct sys_timer omap_timer;
#if defined(CONFIG_I2C_OMAP) || defined(CONFIG_I2C_OMAP_MODULE)
extern int omap_register_i2c_bus(int bus_id, u32 clkrate,
				 struct i2c_board_info const *info,
				 unsigned len);
#else
static inline int omap_register_i2c_bus(int bus_id, u32 clkrate,
				 struct i2c_board_info const *info,
				 unsigned len)
{
	return 0;
}
#endif


struct omap_globals {
	u32		class;		
	void __iomem	*tap;		
	void __iomem	*sdrc;		
	void __iomem	*sms;		
	void __iomem	*ctrl;		
	void __iomem	*prm;		
	void __iomem	*cm;		
};

void omap2_set_globals_242x(void);
void omap2_set_globals_243x(void);
void omap2_set_globals_343x(void);
void omap2_set_globals_443x(void);


void omap2_set_globals_tap(struct omap_globals *);
void omap2_set_globals_sdrc(struct omap_globals *);
void omap2_set_globals_control(struct omap_globals *);
void omap2_set_globals_prcm(struct omap_globals *);

#endif 
