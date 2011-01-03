

#include "msm_fb.h"
#include "mddihost.h"
#include "mddihosti.h"
#include <asm/gpio.h>
#include <mach/vreg.h>
#include <mach/board_lge.h>

#define PANEL_DEBUG 0

#define LCD_CONTROL_BLOCK_BASE	0x110000
#define INTFLG		LCD_CONTROL_BLOCK_BASE|(0x18)
#define INTMSK		LCD_CONTROL_BLOCK_BASE|(0x1c)
#define VPOS		LCD_CONTROL_BLOCK_BASE|(0xc0)

static uint32 mddi_hitachi_curr_vpos;
static boolean mddi_hitachi_monitor_refresh_value = FALSE;
static boolean mddi_hitachi_report_refresh_measurements = FALSE;
static boolean is_lcd_on = -1;


static uint32 mddi_hitachi_rows_per_second = 31250;
static uint32 mddi_hitachi_rows_per_refresh = 480;
static uint32 mddi_hitachi_usecs_per_refresh = 15360; 
extern boolean mddi_vsync_detect_enabled;

static msm_fb_vsync_handler_type mddi_hitachi_vsync_handler = NULL;
static void *mddi_hitachi_vsync_handler_arg;
static uint16 mddi_hitachi_vsync_attempts;

#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA)	\
|| defined(CONFIG_MACH_MSM7X27_SU370) || defined(CONFIG_MACH_MSM7X27_KU3700) || defined(CONFIG_MACH_MSM7X27_LU3700)

static struct msm_panel_hitachi_pdata *mddi_hitachi_pdata;
#else
static struct msm_panel_common_pdata *mddi_hitachi_pdata;
#endif

static int mddi_hitachi_lcd_on(struct platform_device *pdev);
static int mddi_hitachi_lcd_off(struct platform_device *pdev);

static int mddi_hitachi_lcd_init(void);
static void mddi_hitachi_lcd_panel_poweron(void);
static void mddi_hitachi_lcd_panel_poweroff(void);
static void mddi_hitachi_lcd_panel_store_poweron(void);

#define DEBUG 1
#if DEBUG
#define EPRINTK(fmt, args...) printk(fmt, ##args)
#else
#define EPRINTK(fmt, args...) do { } while (0)
#endif

struct display_table {
    unsigned reg;
    unsigned char count;
    unsigned char val_list[20];
};

struct display_table2 {
    unsigned reg;
    unsigned char count;
    unsigned char val_list[16384];
};

#define REGFLAG_DELAY             0XFFFE
#define REGFLAG_END_OF_TABLE      0xFFFF   

static struct display_table mddi_hitachi_position_table[] = {
	
	{0x2a,  4, {0x00, 0x00, 0x01, 0x3f}},
	
	{0x2b,  4, {0x00, 0x00, 0x01, 0xdf}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct display_table mddi_hitachi_display_on_1st[] = {
	
	{0x11, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_DELAY, 80, {}},
	{0x2c, 4, {0x00, 0x00, 0x00, 0x00}},
	{0x29, 4, {0x00, 0x00, 0x00, 0x00}},
	{0x2c, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static struct display_table mddi_hitachi_display_on_3rd[] = {
	
	{0x11, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_DELAY, 80, {}},
	{0x29, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#if 0
static struct display_table2 mddi_hitachi_img[] = {
	{0x2c, 16384, {}},
};
static struct display_table mddi_hitachi_img_end[] = {
	{0x00, 0, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
static struct display_table mddi_hitachi_display_off[] = {
	
	{0x28, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_DELAY, 40, {}},
	{0x10, 4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_DELAY, 130, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static struct display_table mddi_hitachi_sleep_mode_on_data[] = {
	
	{0x28, 4, {0x00, 0x00, 0x00, 0x00}},
	
	{REGFLAG_DELAY, 20, {}},
	{0x10, 4, {0x00, 0x00, 0x00, 0x00}},
	
	{REGFLAG_DELAY, 40, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct display_table mddi_hitachi_initialize_1st[] = {

	
	{0xf0, 4, {0x5a, 0x5a, 0x00, 0x00}},
	{0xf1, 4, {0x5a, 0x5a, 0x00, 0x00}},
	{0xd0, 4, {0x06, 0x00, 0x00, 0x00}},

	
	{0xf4, 16, {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x04, 0x66, 0x02, 0x04, 0x66, 0x02, 0x00, 0x00}},

	
	{0xf5, 12, {0x00, 0x59, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x01, 0x01, 0x59, 0x45}},
	{REGFLAG_DELAY, 10, {}},

	
	{0xf3, 8,  {0x01, 0x6e, 0x15, 0x07, 0x03, 0x00, 0x00, 0x00}},
	
	
	
	{0xf2, 20, {0x3b, 0x54, 0x0f, 0x18, 0x18, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x3f, 0x18,
			    0x18, 0x18, 0x18, 0x00}},
	
	

	{0xf6, 12, {0x04, 0x00, 0x08, 0x03, 0x01, 0x00, 0x01, 0x00,
			    0x00, 0x00, 0x00, 0x00}},

	{0xf9, 4,  {0x27, 0x00, 0x00, 0x00}},

	
	
	
	
	{0xfa, 16, {0x03, 0x03, 0x08, 0x28, 0x2b, 0x2f, 0x32, 0x12,
				0x1d, 0x1f, 0x1c, 0x1c, 0x0f, 0x00, 0x00, 0x00}},

	
	
	
	
	{0xfb, 16, {0x03, 0x03, 0x08, 0x28, 0x2b, 0x2f, 0x32, 0x12,
				0x1d, 0x1f, 0x1c, 0x1c, 0x0f, 0x00, 0x00, 0x00}},

	
	{0x36,  4, {0x48, 0x00, 0x00, 0x00}},

	
	{0x35,  4, {0x00, 0x00, 0x00, 0x00}},

	
	{0x3a,  4, {0x55, 0x00, 0x00, 0x00}},

	
	{0x2a,  4, {0x00, 0x00, 0x01, 0x3f}},

	
	{0x2b,  4, {0x00, 0x00, 0x01, 0xdf}},

	{0x2c,  4, {0x00, 0x00, 0x00, 0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static struct display_table mddi_hitachi_initialize_3rd_vs660[] = {

	
	{0xf0, 4, {0x5a, 0x5a, 0x00, 0x00}},
	{0xf1, 4, {0x5a, 0x5a, 0x00, 0x00}},

	
	
	{0xf4, 16, {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x3f, 0x66, 0x02, 0x3f, 0x66, 0x02, 0x00, 0x00}},

	
	
	{0xf5, 12, {0x00, 0x59, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x59, 0x45}},
	{REGFLAG_DELAY, 10, {}},

	
	
	{0xf3, 8,  {0x01, 0x6e, 0x15, 0x07, 0x03, 0x00, 0x00, 0x00}},
	
	
	
	
	
	{0xf2, 20, {0x3b, 0x4d, 0x0f, 0x08, 0x08, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x4d, 0x08,
			    0x08, 0x08, 0x08, 0x00}},

	{0xf6, 12, {0x04, 0x00, 0x08, 0x03, 0x01, 0x00, 0x01, 0x00,
			    0x00, 0x00, 0x00, 0x00}},

	{0xf9, 4,  {0x27, 0x00, 0x00, 0x00}},

	
	{0xfa, 16, {0x03, 0x03, 0x08, 0x28, 0x2b, 0x2f, 0x32, 0x12,
			    0x1d, 0x1f, 0x1c, 0x1c, 0x0f, 0x00, 0x00, 0x00}},

	
	{0x36,  4, {0x48, 0x00, 0x00, 0x00}},

	
	{0x35,  4, {0x00, 0x00, 0x00, 0x00}},

	
	{0x3a,  4, {0x55, 0x00, 0x00, 0x00}},

	
	{0x2a,  4, {0x00, 0x00, 0x01, 0x3f}},

	
	{0x2b,  4, {0x00, 0x00, 0x01, 0xdf}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static struct display_table mddi_hitachi_initialize_3rd_p500[] = {

	
	{0xf0, 4, {0x5a, 0x5a, 0x00, 0x00}},
	{0xf1, 4, {0x5a, 0x5a, 0x00, 0x00}},

	
	{0xf4, 16, {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x04, 0x66, 0x02, 0x04, 0x66, 0x02, 0x00, 0x00}},

	
	
	{0xf5, 12, {0x00, 0x59, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x59, 0x45}},
	{REGFLAG_DELAY, 10, {}},

	
	
	{0xf3, 8,  {0x01, 0x6e, 0x15, 0x07, 0x03, 0x00, 0x00, 0x00}},
	
	
	
	
	
	{0xf2, 20, {0x3b, 0x4d, 0x0f, 0x08, 0x08, 0x00, 0x00, 0x00,
			    0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x4d, 0x08,
			    0x08, 0x08, 0x08, 0x00}},

	{0xf6, 12, {0x04, 0x00, 0x08, 0x03, 0x01, 0x00, 0x01, 0x00,
			    0x00, 0x00, 0x00, 0x00}},

	{0xf9, 4,  {0x27, 0x00, 0x00, 0x00}},

	
	{0xfa, 16, {0x03, 0x03, 0x08, 0x28, 0x2b, 0x2f, 0x32, 0x12,
			    0x1d, 0x1f, 0x1c, 0x1c, 0x0f, 0x00, 0x00, 0x00}},

	
	{0x36,  4, {0x48, 0x00, 0x00, 0x00}},

	
	{0x35,  4, {0x00, 0x00, 0x00, 0x00}},

	
	{0x3a,  4, {0x55, 0x00, 0x00, 0x00}},

	
	{0x2a,  4, {0x00, 0x00, 0x01, 0x3f}},

	
	{0x2b,  4, {0x00, 0x00, 0x01, 0xdf}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
void display_table(struct display_table *table, unsigned int count)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned reg;
        reg = table[i].reg;
		
        switch (reg) {
			
            case REGFLAG_DELAY :
                msleep(table[i].count);
				EPRINTK("%s() : delay %d msec\n", __func__, table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
                mddi_host_register_cmds_write8(reg, table[i].count, table[i].val_list, 1, 0, 0);
				
       	}
    }
	
}

static void compare_table(struct display_table *table, unsigned int count)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned reg;
        reg = table[i].reg;
		
        switch (reg) {
			
            case REGFLAG_DELAY :              
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
                mddi_host_register_cmds_write8(reg, table[i].count, table[i].val_list, 0, 0, 0);

					
				EPRINTK("%s: reg : %x, val : %x.\n", __func__, reg, table[i].val_list[0]);
       	}
    }	
}


static void mddi_hitachi_vsync_set_handler(msm_fb_vsync_handler_type handler,	
					 void *arg)
{
	boolean error = FALSE;
	unsigned long flags;

	
	printk("%s : handler = %x\n", 
			__func__, (unsigned int)handler);

	
	spin_lock_irqsave(&mddi_host_spin_lock, flags);
	

	if (mddi_hitachi_vsync_handler != NULL) {
		error = TRUE;
	} else {
		
		mddi_hitachi_vsync_handler = handler;
		mddi_hitachi_vsync_handler_arg = arg;
	}
	
	
	spin_unlock_irqrestore(&mddi_host_spin_lock, flags);
	
	if (error) {
		printk("MDDI: Previous Vsync handler never called\n");
	} else {
		
		
		mddi_hitachi_vsync_attempts = 1;
		mddi_vsync_detect_enabled = TRUE;
	}
}

static void mddi_hitachi_lcd_vsync_detected(boolean detected)
{
	
	static struct timeval start_time;
	static boolean first_time = TRUE;
	
	
	struct timeval now;
	uint32 elapsed_us;
	uint32 num_vsyncs;




#if 0 
	if ((detected) || (mddi_hitachi_vsync_attempts > 5)) {
		if ((detected) || (mddi_hitachi_monitor_refresh_value)) {
			
			if (!first_time) {
				jiffies_to_timeval(jiffies, &now);
				elapsed_us =
					(now.tv_sec - start_time.tv_sec) * 1000000 +
					now.tv_usec - start_time.tv_usec;
				
				num_vsyncs = (elapsed_us +
						(mddi_hitachi_rows_per_refresh >>
						 1))/
					mddi_hitachi_rows_per_refresh;
				
				mddi_hitachi_rows_per_second =
					(mddi_hitachi_rows_per_refresh * 1000 *
					 num_vsyncs) / (elapsed_us / 1000);
			}
			
			first_time = FALSE;
			jiffies_to_timeval(jiffies, &start_time);
			if (mddi_hitachi_report_refresh_measurements) {
				(void)mddi_queue_register_read_int(VPOS,
									&mddi_hitachi_curr_vpos);
				
			}
		}
		
		if (mddi_hitachi_vsync_handler != NULL) {
			(*mddi_hitachi_vsync_handler)
				(mddi_hitachi_vsync_handler_arg);
			mddi_hitachi_vsync_handler = NULL;
		}
		mddi_vsync_detect_enabled = FALSE;
		mddi_hitachi_vsync_attempts = 0;
		
		if (!mddi_queue_register_write_int(INTMSK, 0x0001))
			printk("Vsync interrupt disable failed!\n");
		if (!detected) {
			
			printk("Vsync detection failed!\n");
		} else if ((mddi_hitachi_monitor_refresh_value) &&
				(mddi_hitachi_report_refresh_measurements)) {
			printk("  Last Line Counter=%d!\n",
					mddi_hitachi_curr_vpos);
			
			printk("  Lines Per Second=%d!\n",
					mddi_hitachi_rows_per_second);
		}
		
		if (!mddi_queue_register_write_int(INTFLG, 0x0001))
			printk("Vsync interrupt clear failed!\n");
	} else {
		
		mddi_hitachi_vsync_attempts++;
	}
#endif
}

static int mddi_hitachi_lcd_on(struct platform_device *pdev)
{
	EPRINTK("%s: started.\n", __func__);

#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA)	\
|| defined(CONFIG_MACH_MSM7X27_SU370) || defined(CONFIG_MACH_MSM7X27_KU3700) || defined(CONFIG_MACH_MSM7X27_LU3700)
	if (system_state == SYSTEM_BOOTING && mddi_hitachi_pdata->initialized) {
		is_lcd_on = TRUE;
		return 0;
	}
#endif
	
	mddi_hitachi_lcd_panel_poweron();
#if defined(CONFIG_MACH_MSM7X27_THUNDERG)
	if (lge_bd_rev <= LGE_REV_E) {
		EPRINTK("ThunderG ==> lge_bd_rev = %d : 1st LCD initial\n", lge_bd_rev);
		display_table(mddi_hitachi_initialize_1st, sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
		display_table(mddi_hitachi_display_on_1st, sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
	} else {
		EPRINTK("ThunderG ==> lge_bd_rev = %d : 3rd LCD initial\n", lge_bd_rev);
		display_table(mddi_hitachi_initialize_3rd_p500, sizeof(mddi_hitachi_initialize_3rd_p500)/sizeof(struct display_table));
		display_table(mddi_hitachi_display_on_3rd, sizeof(mddi_hitachi_display_on_3rd) / sizeof(struct display_table));
	}
#elif defined(CONFIG_MACH_MSM7X27_THUNDERA)
	display_table(mddi_hitachi_initialize_1st, 
			sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
	display_table(mddi_hitachi_display_on_1st,
			sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));

#elif defined(CONFIG_MACH_MSM7X27_SU370) || defined(CONFIG_MACH_MSM7X27_KU3700) || defined(CONFIG_MACH_MSM7X27_LU3700)
	if (lge_bd_rev <= LGE_REV_C) {
		EPRINTK("U370 ==> lge_bd_rev = %d : 1st LCD initial\n", lge_bd_rev);
		display_table(mddi_hitachi_initialize_1st, sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
		display_table(mddi_hitachi_display_on_1st, sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
	} else {
		EPRINTK("U370 ==> lge_bd_rev = %d : 3rd LCD initial\n", lge_bd_rev);
		display_table(mddi_hitachi_initialize_3rd_p500, sizeof(mddi_hitachi_initialize_3rd_p500)/sizeof(struct display_table));
		display_table(mddi_hitachi_display_on_3rd, sizeof(mddi_hitachi_display_on_3rd) / sizeof(struct display_table));
	}

#else
	if (lge_bd_rev <= LGE_REV_D) {
		EPRINTK("ThunderC ==> lge_bd_rev = %d : 1st LCD initial\n", lge_bd_rev);
		display_table(mddi_hitachi_initialize_1st, sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
		display_table(mddi_hitachi_display_on_1st, sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
	} else {
		EPRINTK("ThunderC ==> lge_bd_rev = %d : 3rd LCD initial\n", lge_bd_rev);
		display_table(mddi_hitachi_initialize_3rd_vs660, sizeof(mddi_hitachi_initialize_3rd_vs660)/sizeof(struct display_table));
		display_table(mddi_hitachi_display_on_3rd, sizeof(mddi_hitachi_display_on_3rd) / sizeof(struct display_table));
	}
#endif
	is_lcd_on = TRUE;
	return 0;
}

static int mddi_hitachi_lcd_store_on(void)
{
	EPRINTK("%s: started.\n", __func__);

#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA) \
 || defined(CONFIG_MACH_MSM7X27_SU370) || defined(CONFIG_MACH_MSM7X27_KU3700) || defined(CONFIG_MACH_MSM7X27_LU3700)
	if (system_state == SYSTEM_BOOTING && mddi_hitachi_pdata->initialized) {
		is_lcd_on = TRUE;
		return 0;
	}
#endif
	
	mddi_hitachi_lcd_panel_store_poweron();
#if defined(CONFIG_MACH_MSM7X27_THUNDERG)
	if (lge_bd_rev <= LGE_REV_E) {
		display_table(mddi_hitachi_initialize_1st, sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
		mdelay(200);
		display_table(mddi_hitachi_display_on_1st, sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
	} else {
		display_table(mddi_hitachi_initialize_3rd_p500, sizeof(mddi_hitachi_initialize_3rd_p500)/sizeof(struct display_table));
		mdelay(200);
		display_table(mddi_hitachi_display_on_3rd, sizeof(mddi_hitachi_display_on_3rd) / sizeof(struct display_table));
	}
#elif defined(CONFIG_MACH_MSM7X27_THUNDERA)
	display_table(mddi_hitachi_initialize_1st,
			sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
	mdelay(200);
	display_table(mddi_hitachi_display_on_1st,
			sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));

#elif defined(CONFIG_MACH_MSM7X27_SU370) || defined(CONFIG_MACH_MSM7X27_KU3700) || defined(CONFIG_MACH_MSM7X27_LU3700)
	if (lge_bd_rev <= LGE_REV_C) {
		display_table(mddi_hitachi_initialize_1st, sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
		mdelay(200);
		display_table(mddi_hitachi_display_on_1st, sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
	} else {
		display_table(mddi_hitachi_initialize_3rd_p500, sizeof(mddi_hitachi_initialize_3rd_p500)/sizeof(struct display_table));
		mdelay(200);
		display_table(mddi_hitachi_display_on_3rd, sizeof(mddi_hitachi_display_on_3rd) / sizeof(struct display_table));
	}

#else
	if (lge_bd_rev <= LGE_REV_D) {
		display_table(mddi_hitachi_initialize_1st, sizeof(mddi_hitachi_initialize_1st)/sizeof(struct display_table));
		mdelay(200);
		display_table(mddi_hitachi_display_on_1st, sizeof(mddi_hitachi_display_on_1st) / sizeof(struct display_table));
	} else {
		display_table(mddi_hitachi_initialize_3rd_vs660, sizeof(mddi_hitachi_initialize_3rd_vs660)/sizeof(struct display_table));
		mdelay(200);
		display_table(mddi_hitachi_display_on_3rd, sizeof(mddi_hitachi_display_on_3rd) / sizeof(struct display_table));
	}
#endif
	is_lcd_on = TRUE;
	return 0;
}

static int mddi_hitachi_lcd_off(struct platform_device *pdev)
{
	display_table(mddi_hitachi_sleep_mode_on_data, sizeof(mddi_hitachi_sleep_mode_on_data)/sizeof(struct display_table));
	mddi_hitachi_lcd_panel_poweroff();
	is_lcd_on = FALSE;
	return 0;
}

ssize_t mddi_hitachi_lcd_show_onoff(struct device *dev, struct device_attribute *attr, char *buf)
{
	EPRINTK("%s : strat\n", __func__);
	return 0;
}

ssize_t mddi_hitachi_lcd_store_onoff(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct platform_device dummy_pdev;
	int onoff; 
	sscanf(buf, "%d", &onoff);

	EPRINTK("%s: onoff : %d\n", __func__, onoff);
	
	if(onoff) {

		mddi_hitachi_lcd_store_on();
		is_lcd_on = TRUE;
	}
	else {

		mddi_hitachi_lcd_off(&dummy_pdev);
		is_lcd_on = FALSE;
	}

	
	return count;
}

int mddi_hitachi_position(void)
{
	display_table(mddi_hitachi_position_table, ARRAY_SIZE(mddi_hitachi_position_table));
	return 0;
}
EXPORT_SYMBOL(mddi_hitachi_position);

DEVICE_ATTR(lcd_onoff, 0666, mddi_hitachi_lcd_show_onoff, mddi_hitachi_lcd_store_onoff);

struct msm_fb_panel_data hitachi_panel_data0 = {
	.on = mddi_hitachi_lcd_on,
	.off = mddi_hitachi_lcd_off,
	.set_backlight = NULL,
	.set_vsync_notifier = mddi_hitachi_vsync_set_handler,
};

static struct platform_device this_device_0 = {
	.name   = "mddi_hitachi_hvga",
	.id	= MDDI_LCD_HITACHI_TX08D39VM,
	.dev	= {
		.platform_data = &hitachi_panel_data0,
	}
};

static int __init mddi_hitachi_lcd_probe(struct platform_device *pdev)
{
	int ret;
	EPRINTK("%s: started.\n", __func__);

	if (pdev->id == 0) {
		mddi_hitachi_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);

	ret = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mddi_hitachi_lcd_probe,
	.driver = {
		.name   = "mddi_hitachi_hvga",
	},
};

static int mddi_hitachi_lcd_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;

#ifdef CONFIG_FB_MSM_MDDI_AUTO_DETECT
	u32 id;
	id = mddi_get_client_id();

	

#endif
	ret = platform_driver_register(&this_driver);
	if (!ret) {
		pinfo = &hitachi_panel_data0.panel_info;
		EPRINTK("%s: setting up panel info.\n", __func__);
		pinfo->xres = 320;
		pinfo->yres = 480;
		pinfo->type = MDDI_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->mddi.vdopkt = 0x23;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 16;
	
		
		pinfo->lcd.vsync_enable = TRUE;
		pinfo->lcd.refx100 = (mddi_hitachi_rows_per_second * 100) /
                        		mddi_hitachi_rows_per_refresh;


		pinfo->lcd.v_back_porch = 6;
		pinfo->lcd.v_front_porch = 6;
		pinfo->lcd.v_pulse_width = 4;

		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_notifier_period = (1 * HZ);

		pinfo->bl_max = 4;
		pinfo->bl_min = 1;

		pinfo->clk_rate = 10000000;
		pinfo->clk_min =  9000000;
		pinfo->clk_max =  11000000;
		pinfo->fb_num = 2;

		ret = platform_device_register(&this_device_0);
		if (ret) {
			EPRINTK("%s: this_device_0 register success\n", __func__);
			platform_driver_unregister(&this_driver);
		}
	}

	if(!ret) {
		mddi_lcd.vsync_detected = mddi_hitachi_lcd_vsync_detected;
	}

	return ret;
}

extern unsigned fb_width;
extern unsigned fb_height;

static void mddi_hitachi_lcd_panel_poweron(void)
{
#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA) \
 || defined(CONFIG_MACH_MSM7X27_SU370) || defined(CONFIG_MACH_MSM7X27_KU3700) || defined(CONFIG_MACH_MSM7X27_LU3700)
	struct msm_panel_hitachi_pdata *pdata = mddi_hitachi_pdata;
#else
	struct msm_panel_common_pdata *pdata = mddi_hitachi_pdata;
#endif

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
	
	
		gpio_set_value(pdata->gpio, 0);
		mdelay(10);
		gpio_set_value(pdata->gpio, 1);
		mdelay(2);
	}
}

static void mddi_hitachi_lcd_panel_store_poweron(void)
{
#if defined(CONFIG_MACH_MSM7X27_THUNDERG) || defined(CONFIG_MACH_MSM7X27_THUNDERC) || defined(CONFIG_MACH_MSM7X27_THUNDERA) \
 || defined(CONFIG_MACH_MSM7X27_SU370) || defined(CONFIG_MACH_MSM7X27_KU3700) || defined(CONFIG_MACH_MSM7X27_LU3700)
	struct msm_panel_hitachi_pdata *pdata = mddi_hitachi_pdata;
#else
	struct msm_panel_common_pdata *pdata = mddi_hitachi_pdata;
#endif

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
	
	
		gpio_set_value(pdata->gpio, 0);
		mdelay(50);
		gpio_set_value(pdata->gpio, 1);
		mdelay(50);
	}
}


static void mddi_hitachi_lcd_panel_poweroff(void)
{
	struct msm_panel_hitachi_pdata *pdata = mddi_hitachi_pdata;

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
		gpio_set_value(pdata->gpio, 0);
		mdelay(5);
	}
}
module_init(mddi_hitachi_lcd_init);
