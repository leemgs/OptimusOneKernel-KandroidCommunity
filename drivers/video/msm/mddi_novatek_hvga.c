



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

static uint32 mddi_novatek_curr_vpos;
static boolean mddi_novatek_monitor_refresh_value = FALSE;
static boolean mddi_novatek_report_refresh_measurements = FALSE;
static boolean is_lcd_on = -1;


static uint32 mddi_novatek_rows_per_second = 31250;
static uint32 mddi_novatek_rows_per_refresh = 480;
static uint32 mddi_novatek_usecs_per_refresh = 15360; 
extern boolean mddi_vsync_detect_enabled;

static msm_fb_vsync_handler_type mddi_novatek_vsync_handler = NULL;
static void *mddi_novatek_vsync_handler_arg;
static uint16 mddi_novatek_vsync_attempts;




static struct msm_panel_novatek_pdata *mddi_novatek_pdata;

static int mddi_novatek_lcd_on(struct platform_device *pdev);
static int mddi_novatek_lcd_off(struct platform_device *pdev);

static int mddi_novatek_lcd_init(void);
static void mddi_novatek_lcd_panel_poweron(void);
static void mddi_novatek_lcd_panel_poweroff(void);

#define DEBUG 1
#if DEBUG
#define EPRINTK(fmt, args...) printk(fmt, ##args)
#else
#define EPRINTK(fmt, args...) do { } while (0)
#endif

struct display_table {
    unsigned reg;
    unsigned char count;
    unsigned val_list[256];
};

#define REGFLAG_DELAY             0XFFFE
#define REGFLAG_END_OF_TABLE      0xFFFF   

static struct display_table mddi_novatek_position_table[] = {
	
	{0x2a00, 1, {0x0000}}, 
	{0x2a01, 1, {0x0000}}, 
	{0x2a02, 1, {0x0000}}, 
	{0x2a03, 1, {0x013f}}, 
	
	{0x2b00, 1, {0x0000}}, 
	{0x2b01, 1, {0x0000}}, 
	{0x2b02, 1, {0x0000}}, 
	{0x2b03, 1, {0x01df}}, 
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct display_table mddi_novatek_display_on[] = {
	
	{0x1100, 1, {0x0000}}, 
	{REGFLAG_DELAY, 150, {}},
	{0x2c00, 1, {0x0000}},
	{0x3800, 1, {0x0000}}, 
	{0x2900, 1, {0x0000}}, 
	{0x2c00, 1, {0x0000}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#if 0
static struct display_table2 mddi_novatek_img[] = {
	{0x2c, 16384, {}},
};
static struct display_table mddi_novatek_img_end[] = {
	{0x00, 0, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif
static struct display_table mddi_novatek_display_off[] = {
	
	{0x3900, 1, {0x0000}}, 
	{0x2800, 1, {0x0000}}, 
	{REGFLAG_DELAY, 50, {}},
	{0x1000, 1, {0x0000}},
	{REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
static struct display_table mddi_novatek_sleep_mode_on_data[] = {
	
	{0x3900, 1, {0x0000}}, 
	{0x2800, 1, {0x0000}},
	{REGFLAG_DELAY, 50, {}},
	{0x1000, 4, {0x0000}},
	{REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct display_table mddi_novatek_initialize[] = {
	
	{0x3900, 1, {0x0000}}, 
#if defined(USE_TENTATIVE_COMMAND)
	{0x1100, 1, {0x0000}}, 
#endif
	{REGFLAG_DELAY, 100, {}},
	{0xF300, 1, {0x00AA}}, 
	{0xF280, 1, {0x0002}}, 
	{0x0280, 1, {0x0011}}, 
	{0x0380, 1, {0x0000}}, 

  
  
  #if 1
  
	{0x0480, 1, {0x0056}}, 
	{0x0580, 1, {0x0056}}, 
	{0x0680, 1, {0x0056}}, 
	#else
	
	{0x0480, 1, {0x005b}}, 
	{0x0580, 1, {0x005b}}, 
	{0x0680, 1, {0x005b}}, 
  #endif
	
	{0x0780, 1, {0x0000}}, 
	{0x0880, 1, {0x0033}}, 
	{0x0980, 1, {0x0043}}, 
	{0x0A80, 1, {0x0030}}, 
	{0x0B80, 1, {0x0044}}, 
	{0x0C80, 1, {0x0054}}, 
	{0x0D80, 1, {0x0030}}, 
	{0x0E80, 1, {0x0033}}, 
	{0x0F80, 1, {0x0043}}, 
	{0x1080, 1, {0x0030}}, 
	{0x1180, 1, {0x0000}}, 
	{0x1280, 1, {0x000C}}, 
	{0x1380, 1, {0x0004}}, 
	{0x1480, 1, {0x0058}}, 
	{0x1680, 1, {0x0070}}, 
	{0x1780, 1, {0x00CC}}, 
	{0x1880, 1, {0x0080}}, 
	{0x1980, 1, {0x0000}}, 
	{0x1A80, 1, {0x0078}}, 
	{0x1B80, 1, {0x0050}}, 
	{0x1C80, 1, {0x0080}}, 
	{0x9480, 1, {0x0017}}, 
	{0x9580, 1, {0x0021}}, 
	{0x9680, 1, {0x0005}}, 
	{0x9780, 1, {0x000C}}, 
	{0x9880, 1, {0x0072}}, 
	{0x9980, 1, {0x0012}}, 
	{0x9A80, 1, {0x0088}}, 
	{0x9B80, 1, {0x0001}}, 
	{0x9C80, 1, {0x0005}}, 
	{0x9D80, 1, {0x0016}}, 
	{0x9E80, 1, {0x0000}}, 
	{0x9F80, 1, {0x0000}}, 
	{0xA380, 1, {0x00F8}}, 
	{0xA480, 1, {0x003F}}, 
	{0xA680, 1, {0x0008}}, 
	
	{0x2880, 1, {0x0009}}, 
	{0x2980, 1, {0x001E}}, 
	{0x2A80, 1, {0x0045}}, 
	{0x2B80, 1, {0x0063}}, 
	{0x2C80, 1, {0x000D}}, 
	{0x2D80, 1, {0x002E}}, 
	{0x2E80, 1, {0x0061}}, 
	{0x2F80, 1, {0x0063}}, 
	{0x3080, 1, {0x0020}}, 
	{0x3180, 1, {0x0026}}, 
	{0x3280, 1, {0x00A5}}, 
	{0x3380, 1, {0x001E}}, 
	{0x3480, 1, {0x0048}}, 
	{0x3580, 1, {0x0067}}, 
	{0x3680, 1, {0x0078}}, 
	{0x3780, 1, {0x0088}}, 
	{0x3880, 1, {0x0025}}, 
	{0x3890, 1, {0x0053}}, 
	{0x3A80, 1, {0x0009}}, 
	{0x3B80, 1, {0x0037}}, 
	{0x3C80, 1, {0x0056}}, 
	{0x3D80, 1, {0x0068}}, 
	{0x3E80, 1, {0x0018}}, 
	{0x3F80, 1, {0x0037}}, 
	{0x4080, 1, {0x0061}}, 
	{0x4180, 1, {0x0040}}, 
	{0x4280, 1, {0x0019}}, 
	{0x4380, 1, {0x001F}}, 
	{0x4480, 1, {0x0085}}, 
	{0x4580, 1, {0x001E}}, 
	{0x4680, 1, {0x0051}}, 
	{0x4780, 1, {0x0072}}, 
	{0x4880, 1, {0x0087}}, 
	{0x4980, 1, {0x00A6}}, 
	{0x4A80, 1, {0x004D}}, 
	{0x4B80, 1, {0x0062}}, 
	{0x4C80, 1, {0x003D}}, 
	{0x4D80, 1, {0x0050}}, 
	{0x4E80, 1, {0x006E}}, 
	{0x4F80, 1, {0x007E}}, 
	{0x5080, 1, {0x0009}}, 
	{0x5180, 1, {0x0028}}, 
	{0x5280, 1, {0x005C}}, 
	{0x5380, 1, {0x006A}},
	{0x5480, 1, {0x001F}},
	{0x5580, 1, {0x0026}},
	{0x5680, 1, {0x00AA}},
	{0x5780, 1, {0x001D}},
	{0x5880, 1, {0x0048}},
	{0x5980, 1, {0x0065}},
	{0x5A80, 1, {0x007A}},
	{0x5B80, 1, {0x008A}},
	{0x5C80, 1, {0x0026}},
	{0x5D80, 1, {0x0053}},
	{0x5E80, 1, {0x0009}},
	{0x5F80, 1, {0x0036}},
	{0x6080, 1, {0x0053}},
	{0x6180, 1, {0x0066}},
	{0x6280, 1, {0x001A}},
	{0x6380, 1, {0x0037}},
	{0x6480, 1, {0x0062}},
	{0x6580, 1, {0x003B}},
	{0x6680, 1, {0x0019}},
	{0x6780, 1, {0x0020}},
	{0x6880, 1, {0x007E}},
	{0x6980, 1, {0x0023}},
	{0x6A80, 1, {0x0057}},
	{0x6B80, 1, {0x0076}},
	{0x6C80, 1, {0x006C}},
	{0x6D80, 1, {0x007C}},
	{0x6E80, 1, {0x001A}},
	{0x6F80, 1, {0x002D}},
	{0x7080, 1, {0x0009}}, 
	{0x7180, 1, {0x0023}},
	{0x7280, 1, {0x004F}},
	{0x7380, 1, {0x0069}},
	{0x7480, 1, {0x0015}},
	{0x7580, 1, {0x003E}},
	{0x7680, 1, {0x0069}},
	{0x7780, 1, {0x0074}},
	{0x7880, 1, {0x0020}},
	{0x7980, 1, {0x0026}},
	{0x7A80, 1, {0x00AD}},
	{0x7B80, 1, {0x001E}},
	{0x7C80, 1, {0x004E}},
	{0x7D80, 1, {0x0067}},
	{0x7E80, 1, {0x0079}},
	{0x7F80, 1, {0x0086}},
	{0x8080, 1, {0x0028}},
	{0x8180, 1, {0x0053}},
	{0x8280, 1, {0x0009}},
	{0x8380, 1, {0x0034}},
	{0x8480, 1, {0x0058}},
	{0x8580, 1, {0x0067}},
	{0x8680, 1, {0x0018}},
	{0x8780, 1, {0x0031}},
	{0x8880, 1, {0x0061}},
	{0x8980, 1, {0x0038}},
	{0x8A80, 1, {0x0019}},
	{0x8B80, 1, {0x001F}},
	{0x8C80, 1, {0x0074}},
	{0x8D80, 1, {0x0016}},
	{0x8E80, 1, {0x0041}},
	{0x8F80, 1, {0x006A}},
	{0x9080, 1, {0x0081}},
	{0x9180, 1, {0x009B}},
	{0x9280, 1, {0x0048}},
	{0x9380, 1, {0x0062}},
#if defined(USE_TENTATIVE_COMMAND)
	{0x2780, 1, {0x0033}},
#endif
	{0x1580, 1, {0x00AA}}, 
	{0xF200, 1, {0x0001}}, 
#if defined(USE_TENTATIVE_COMMAND)
	
	
#endif
	{0x3B00, 1, {0x0043}}, 
	{0x3B01, 1, {0x0004}},
	{0x3B02, 1, {0x0004}},
	{0x3B03, 1, {0x0008}},
	{0x3B04, 1, {0x0007}},
	
	
	{0x5100, 1, {0x0000}}, 
	{0x5300, 1, {0x002C}}, 

	
	{0x2a00, 1, {0x0000}}, 
	{0x2a01, 1, {0x0000}}, 
	{0x2a02, 1, {0x0000}}, 
	{0x2a03, 1, {0x013f}}, 
	
	{0x2b00, 1, {0x0000}}, 
	{0x2b01, 1, {0x0000}}, 
	{0x2b02, 1, {0x0000}}, 
	{0x2b03, 1, {0x01df}}, 

	{0x3600, 1, {0x0008}}, 
	{0x3800, 1, {0x0000}}, 
	{0x3A00, 1, {0x0055}}, 

	{0x3500, 1, {0x0000}}, 

	{0x2900, 1, {0x0000}}, 
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
                mddi_host_register_cmds_write32(reg, table[i].count, table[i].val_list, 1, 0, 0);
		
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
                mddi_host_register_cmds_write32(reg, table[i].count, table[i].val_list, 0, 0, 0);
		EPRINTK("%s: reg : %x, val : %x.\n", __func__, reg, table[i].val_list[0]);
       	}
    }	
}


static void mddi_novatek_vsync_set_handler(msm_fb_vsync_handler_type handler,	
					 void *arg)
{
	boolean error = FALSE;
	unsigned long flags;

	
	printk("%s : handler = %x\n", 
			__func__, (unsigned int)handler);

	
	spin_lock_irqsave(&mddi_host_spin_lock, flags);
	

	if (mddi_novatek_vsync_handler != NULL) {
		error = TRUE;
	} else {
		
		mddi_novatek_vsync_handler = handler;
		mddi_novatek_vsync_handler_arg = arg;
	}
	
	
	spin_unlock_irqrestore(&mddi_host_spin_lock, flags);
	
	if (error) {
		printk("MDDI: Previous Vsync handler never called\n");
	} else {
		
		
		mddi_novatek_vsync_attempts = 1;
		mddi_vsync_detect_enabled = TRUE;
	}
}

static void mddi_novatek_lcd_vsync_detected(boolean detected)
{
	
	static struct timeval start_time;
	static boolean first_time = TRUE;
	
	
	struct timeval now;
	uint32 elapsed_us;
	uint32 num_vsyncs;

	mddi_vsync_detect_enabled = TRUE;;

#if 0 
	mddi_queue_register_write_int(0x2C00, 0);

	if ((detected) || (mddi_novatek_vsync_attempts > 5)) {
		if ((detected) || (mddi_novatek_monitor_refresh_value)) {
			
			if (!first_time) {
				jiffies_to_timeval(jiffies, &now);
				elapsed_us =
					(now.tv_sec - start_time.tv_sec) * 1000000 +
					now.tv_usec - start_time.tv_usec;
				
				num_vsyncs = (elapsed_us +
						(mddi_novatek_rows_per_refresh >>
						 1))/
					mddi_novatek_rows_per_refresh;
				
				mddi_novatek_rows_per_second =
					(mddi_novatek_rows_per_refresh * 1000 *
					 num_vsyncs) / (elapsed_us / 1000);
			}
			
			first_time = FALSE;
			jiffies_to_timeval(jiffies, &start_time);
			if (mddi_novatek_report_refresh_measurements) {
				(void)mddi_queue_register_read_int(VPOS,
									&mddi_novatek_curr_vpos);
				
			}
		}
		
		if (mddi_novatek_vsync_handler != NULL) {
			(*mddi_novatek_vsync_handler)
				(mddi_novatek_vsync_handler_arg);
			mddi_novatek_vsync_handler = NULL;
		}
		mddi_vsync_detect_enabled = FALSE;
		mddi_novatek_vsync_attempts = 0;
		
		if (!mddi_queue_register_write_int(INTMSK, 0x0001))
			printk("Vsync interrupt disable failed!\n");
		if (!detected) {
			
			printk("Vsync detection failed!\n");
		} else if ((mddi_novatek_monitor_refresh_value) &&
				(mddi_novatek_report_refresh_measurements)) {
			printk("  Last Line Counter=%d!\n",
					mddi_novatek_curr_vpos);
			
			printk("  Lines Per Second=%d!\n",
					mddi_novatek_rows_per_second);
		}
		
		if (!mddi_queue_register_write_int(INTFLG, 0x0001))
			printk("Vsync interrupt clear failed!\n");
	} else {
		
		mddi_novatek_vsync_attempts++;
	}
#endif
}

static int mddi_novatek_lcd_on(struct platform_device *pdev)
{
	EPRINTK("%s: started.\n", __func__);

	
	if(is_lcd_on == -1) {
		is_lcd_on = TRUE;
		return 0;
	}
	

	
	
	if (system_state == SYSTEM_BOOTING && mddi_novatek_pdata->initialized) {
		is_lcd_on = TRUE;
	}

	
	mddi_novatek_lcd_panel_poweron();	
	display_table(mddi_novatek_initialize, sizeof(mddi_novatek_initialize)/sizeof(struct display_table));
	display_table(mddi_novatek_display_on, sizeof(mddi_novatek_display_on) / sizeof(struct display_table));
	is_lcd_on = TRUE;
	return 0;
}

static int mddi_novatek_lcd_off(struct platform_device *pdev)
{
	display_table(mddi_novatek_sleep_mode_on_data, sizeof(mddi_novatek_sleep_mode_on_data)/sizeof(struct display_table));
	mddi_novatek_lcd_panel_poweroff();
	is_lcd_on = FALSE;
	return 0;
}

ssize_t mddi_novatek_lcd_show_onoff(struct platform_device *pdev)
{
	EPRINTK("%s : strat\n", __func__);
	return 0;
}

ssize_t mddi_novatek_lcd_store_onoff(struct platform_device *pdev, struct device_attribute *attr, const char *buf, size_t count)
{
	int onoff; 
	sscanf(buf, "%d", &onoff);

	EPRINTK("%s: onoff : %d\n", __func__, onoff);
	
	if(onoff) {
		is_lcd_on = TRUE;
		display_table(mddi_novatek_display_on, sizeof(mddi_novatek_display_on) / sizeof(struct display_table));
	}
	else {
		is_lcd_on = FALSE;
		display_table(mddi_novatek_display_off, sizeof(mddi_novatek_display_off) / sizeof(struct display_table));
	}

	return 0;
}

int mddi_novatek_position(void)
{
	display_table(mddi_novatek_position_table, ARRAY_SIZE(mddi_novatek_position_table));
	return 0;
}
EXPORT_SYMBOL(mddi_novatek_position);

DEVICE_ATTR(lcd_onoff, 0666, mddi_novatek_lcd_show_onoff, mddi_novatek_lcd_store_onoff);

struct msm_fb_panel_data novatek_panel_data0 = {
	.on = mddi_novatek_lcd_on,
	.off = mddi_novatek_lcd_off,
	.set_backlight = NULL,
	.set_vsync_notifier = mddi_novatek_vsync_set_handler,
};

static struct platform_device this_device_0 = {
	.name   = "mddi_novatek_hvga",
	.id	= MDDI_LCD_NOVATEK_NT35451,
	.dev	= {
		.platform_data = &novatek_panel_data0,
	}
};

static int __init mddi_novatek_lcd_probe(struct platform_device *pdev)
{
	int ret;
	EPRINTK("%s: started.\n", __func__);

	if (pdev->id == 0) {
		mddi_novatek_pdata = pdev->dev.platform_data;
		return 0;
	}

	msm_fb_add_device(pdev);

	ret = device_create_file(&pdev->dev, &dev_attr_lcd_onoff);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mddi_novatek_lcd_probe,
	.driver = {
		.name   = "mddi_novatek_hvga",
	},
};

static int mddi_novatek_lcd_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;

#ifdef CONFIG_FB_MSM_MDDI_AUTO_DETECT
	u32 id;
	id = mddi_get_client_id();

	

#endif
	ret = platform_driver_register(&this_driver);
	if (!ret) {
		pinfo = &novatek_panel_data0.panel_info;
		EPRINTK("%s: setting up panel info.\n", __func__);
		pinfo->xres = 320;
		pinfo->yres = 480;
		pinfo->type = MDDI_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->mddi.vdopkt = MDDI_DEFAULT_PRIM_PIX_ATTR;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 16;
	
		
		pinfo->lcd.vsync_enable = TRUE;
		pinfo->lcd.refx100 = (mddi_novatek_rows_per_second * 100) /
                        		mddi_novatek_rows_per_refresh;

		pinfo->lcd.v_back_porch = 200;
		pinfo->lcd.v_front_porch = 200;
		pinfo->lcd.v_pulse_width = 30;

		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_notifier_period = (1 * HZ);

		pinfo->bl_max = 4;
		pinfo->bl_min = 1;

    
		pinfo->clk_rate = 122880000;
		pinfo->clk_min =   120000000;
		pinfo->clk_max =   130000000;
		pinfo->fb_num = 2;

		ret = platform_device_register(&this_device_0);
		if (ret) {
			EPRINTK("%s: this_device_0 register success\n", __func__);
			platform_driver_unregister(&this_driver);
		}
	}

	if(!ret) {
		mddi_lcd.vsync_detected = mddi_novatek_lcd_vsync_detected;
	}

	return ret;
}

extern unsigned fb_width;
extern unsigned fb_height;

static void mddi_novatek_lcd_panel_poweron(void)
{
	
	
	
	struct msm_panel_novatek_pdata *pdata = mddi_novatek_pdata;

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
		gpio_set_value(pdata->gpio, 1);
		mdelay(10);
		gpio_set_value(pdata->gpio, 0);
		mdelay(10);
		gpio_set_value(pdata->gpio, 1);
		mdelay(20);
	}
}


static void mddi_novatek_lcd_panel_poweroff(void)
{
	struct msm_panel_novatek_pdata *pdata = mddi_novatek_pdata;

	EPRINTK("%s: started.\n", __func__);

	fb_width = 320;
	fb_height = 480;

	if(pdata && pdata->gpio) {
		gpio_set_value(pdata->gpio, 0);
		mdelay(10);
	}
}

module_init(mddi_novatek_lcd_init);
