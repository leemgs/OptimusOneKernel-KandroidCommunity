

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/backlight.h>
#include <linux/fb.h>
#include <linux/leds.h>
#include <linux/platform_device.h>
#include <acpi/acpi_drivers.h>
#include <acpi/acpi_bus.h>
#include <asm/uaccess.h>
#include <linux/input.h>

#define ASUS_LAPTOP_VERSION "0.42"

#define ASUS_HOTK_NAME          "Asus Laptop Support"
#define ASUS_HOTK_CLASS         "hotkey"
#define ASUS_HOTK_DEVICE_NAME   "Hotkey"
#define ASUS_HOTK_FILE          KBUILD_MODNAME
#define ASUS_HOTK_PREFIX        "\\_SB.ATKD."



#define ATKD_BR_UP       0x10
#define ATKD_BR_DOWN     0x20
#define ATKD_LCD_ON      0x33
#define ATKD_LCD_OFF     0x34


#define WL_HWRS     0x80
#define BT_HWRS     0x100


#define WL_ON       0x01	
#define BT_ON       0x02	
#define MLED_ON     0x04	
#define TLED_ON     0x08	
#define RLED_ON     0x10	
#define PLED_ON     0x20	
#define GLED_ON     0x40	
#define LCD_ON      0x80	
#define GPS_ON      0x100	
#define KEY_ON      0x200	

#define ASUS_LOG    ASUS_HOTK_FILE ": "
#define ASUS_ERR    KERN_ERR    ASUS_LOG
#define ASUS_WARNING    KERN_WARNING    ASUS_LOG
#define ASUS_NOTICE KERN_NOTICE ASUS_LOG
#define ASUS_INFO   KERN_INFO   ASUS_LOG
#define ASUS_DEBUG  KERN_DEBUG  ASUS_LOG

MODULE_AUTHOR("Julien Lerouge, Karol Kozimor, Corentin Chary");
MODULE_DESCRIPTION(ASUS_HOTK_NAME);
MODULE_LICENSE("GPL");


static uint wapf = 1;
module_param(wapf, uint, 0644);
MODULE_PARM_DESC(wapf, "WAPF value");

#define ASUS_HANDLE(object, paths...)					\
	static acpi_handle  object##_handle = NULL;			\
	static char *object##_paths[] = { paths }


ASUS_HANDLE(mled_set, ASUS_HOTK_PREFIX "MLED");
ASUS_HANDLE(tled_set, ASUS_HOTK_PREFIX "TLED");
ASUS_HANDLE(rled_set, ASUS_HOTK_PREFIX "RLED");	
ASUS_HANDLE(pled_set, ASUS_HOTK_PREFIX "PLED");	
ASUS_HANDLE(gled_set, ASUS_HOTK_PREFIX "GLED");	


ASUS_HANDLE(ledd_set, ASUS_HOTK_PREFIX "SLCM");


ASUS_HANDLE(wl_switch, ASUS_HOTK_PREFIX "WLED");
ASUS_HANDLE(bt_switch, ASUS_HOTK_PREFIX "BLED");
ASUS_HANDLE(wireless_status, ASUS_HOTK_PREFIX "RSTS");	


ASUS_HANDLE(brightness_set, ASUS_HOTK_PREFIX "SPLV");
ASUS_HANDLE(brightness_get, ASUS_HOTK_PREFIX "GPLV");


ASUS_HANDLE(lcd_switch, "\\_SB.PCI0.SBRG.EC0._Q10",	
	    "\\_SB.PCI0.ISA.EC0._Q10",	
	    "\\_SB.PCI0.PX40.ECD0._Q10",	
	    "\\_SB.PCI0.PX40.EC0.Q10",	
	    "\\_SB.PCI0.LPCB.EC0._Q10",	
	    "\\_SB.PCI0.LPCB.EC0._Q0E", 
	    "\\_SB.PCI0.PX40.Q10",	
	    "\\Q10");		


ASUS_HANDLE(display_set, ASUS_HOTK_PREFIX "SDSP");
ASUS_HANDLE(display_get,
	    
	    "\\_SB.PCI0.P0P1.VGA.GETD",
	    
	    "\\_SB.PCI0.P0P2.VGA.GETD",
	    
	    "\\_SB.PCI0.P0P3.VGA.GETD",
	    
	    "\\_SB.PCI0.P0PA.VGA.GETD",
	    
	    "\\_SB.PCI0.PCI1.VGAC.NMAP",
	    
	    "\\_SB.PCI0.VGA.GETD",
	    
	    "\\ACTD",
	    
	    "\\ADVG",
	    
	    "\\DNXT",
	    
	    "\\INFB",
	    
	    "\\SSTE");

ASUS_HANDLE(ls_switch, ASUS_HOTK_PREFIX "ALSC"); 
ASUS_HANDLE(ls_level, ASUS_HOTK_PREFIX "ALSL");	 



ASUS_HANDLE(gps_on, ASUS_HOTK_PREFIX "SDON");	
ASUS_HANDLE(gps_off, ASUS_HOTK_PREFIX "SDOF");	
ASUS_HANDLE(gps_status, ASUS_HOTK_PREFIX "GPST");


ASUS_HANDLE(kled_set, ASUS_HOTK_PREFIX "SLKB");
ASUS_HANDLE(kled_get, ASUS_HOTK_PREFIX "GLKB");


struct asus_hotk {
	char *name;		
	struct acpi_device *device;	
	acpi_handle handle;	
	char status;		
	u32 ledd_status;	
	u8 light_level;		
	u8 light_switch;	
	u16 event_count[128];	
	struct input_dev *inputdev;
	u16 *keycode_map;
};


static struct acpi_table_header *asus_info;


static struct asus_hotk *hotk;


static const struct acpi_device_id asus_device_ids[] = {
	{"ATK0100", 0},
	{"ATK0101", 0},
	{"", 0},
};
MODULE_DEVICE_TABLE(acpi, asus_device_ids);

static int asus_hotk_add(struct acpi_device *device);
static int asus_hotk_remove(struct acpi_device *device, int type);
static void asus_hotk_notify(struct acpi_device *device, u32 event);

static struct acpi_driver asus_hotk_driver = {
	.name = ASUS_HOTK_NAME,
	.class = ASUS_HOTK_CLASS,
	.ids = asus_device_ids,
	.flags = ACPI_DRIVER_ALL_NOTIFY_EVENTS,
	.ops = {
		.add = asus_hotk_add,
		.remove = asus_hotk_remove,
		.notify = asus_hotk_notify,
		},
};


static struct backlight_device *asus_backlight_device;


static int read_brightness(struct backlight_device *bd);
static int update_bl_status(struct backlight_device *bd);
static struct backlight_ops asusbl_ops = {
	.get_brightness = read_brightness,
	.update_status = update_bl_status,
};


static struct workqueue_struct *led_workqueue;

#define ASUS_LED(object, ledname, max)					\
	static void object##_led_set(struct led_classdev *led_cdev,	\
				     enum led_brightness value);	\
	static enum led_brightness object##_led_get(			\
		struct led_classdev *led_cdev);				\
	static void object##_led_update(struct work_struct *ignored);	\
	static int object##_led_wk;					\
	static DECLARE_WORK(object##_led_work, object##_led_update);	\
	static struct led_classdev object##_led = {			\
		.name           = "asus::" ledname,			\
		.brightness_set = object##_led_set,			\
		.brightness_get = object##_led_get,			\
		.max_brightness = max					\
	}

ASUS_LED(mled, "mail", 1);
ASUS_LED(tled, "touchpad", 1);
ASUS_LED(rled, "record", 1);
ASUS_LED(pled, "phone", 1);
ASUS_LED(gled, "gaming", 1);
ASUS_LED(kled, "kbd_backlight", 3);

struct key_entry {
	char type;
	u8 code;
	u16 keycode;
};

enum { KE_KEY, KE_END };

static struct key_entry asus_keymap[] = {
	{KE_KEY, 0x02, KEY_SCREENLOCK},
	{KE_KEY, 0x05, KEY_WLAN},
	{KE_KEY, 0x08, BTN_TOUCH},
	{KE_KEY, 0x17, KEY_ZOOM},
	{KE_KEY, 0x1f, KEY_BATTERY},
	{KE_KEY, 0x30, KEY_VOLUMEUP},
	{KE_KEY, 0x31, KEY_VOLUMEDOWN},
	{KE_KEY, 0x32, KEY_MUTE},
	{KE_KEY, 0x33, KEY_SWITCHVIDEOMODE},
	{KE_KEY, 0x34, KEY_SWITCHVIDEOMODE},
	{KE_KEY, 0x40, KEY_PREVIOUSSONG},
	{KE_KEY, 0x41, KEY_NEXTSONG},
	{KE_KEY, 0x43, KEY_STOPCD},
	{KE_KEY, 0x45, KEY_PLAYPAUSE},
	{KE_KEY, 0x4c, KEY_MEDIA},
	{KE_KEY, 0x50, KEY_EMAIL},
	{KE_KEY, 0x51, KEY_WWW},
	{KE_KEY, 0x55, KEY_CALC},
	{KE_KEY, 0x5C, KEY_SCREENLOCK},  
	{KE_KEY, 0x5D, KEY_WLAN},
	{KE_KEY, 0x5E, KEY_WLAN},
	{KE_KEY, 0x5F, KEY_WLAN},
	{KE_KEY, 0x60, KEY_SWITCHVIDEOMODE},
	{KE_KEY, 0x61, KEY_SWITCHVIDEOMODE},
	{KE_KEY, 0x62, KEY_SWITCHVIDEOMODE},
	{KE_KEY, 0x63, KEY_SWITCHVIDEOMODE},
	{KE_KEY, 0x6B, BTN_TOUCH}, 
	{KE_KEY, 0x82, KEY_CAMERA},
	{KE_KEY, 0x8A, KEY_PROG1},
	{KE_KEY, 0x95, KEY_MEDIA},
	{KE_KEY, 0x99, KEY_PHONE},
	{KE_KEY, 0xc4, KEY_KBDILLUMUP},
	{KE_KEY, 0xc5, KEY_KBDILLUMDOWN},
	{KE_END, 0},
};


static int write_acpi_int(acpi_handle handle, const char *method, int val,
			  struct acpi_buffer *output)
{
	struct acpi_object_list params;	
	union acpi_object in_obj;	
	acpi_status status;

	if (!handle)
		return 0;

	params.count = 1;
	params.pointer = &in_obj;
	in_obj.type = ACPI_TYPE_INTEGER;
	in_obj.integer.value = val;

	status = acpi_evaluate_object(handle, (char *)method, &params, output);
	if (status == AE_OK)
		return 0;
	else
		return -1;
}

static int read_wireless_status(int mask)
{
	unsigned long long status;
	acpi_status rv = AE_OK;

	if (!wireless_status_handle)
		return (hotk->status & mask) ? 1 : 0;

	rv = acpi_evaluate_integer(wireless_status_handle, NULL, NULL, &status);
	if (ACPI_FAILURE(rv))
		pr_warning("Error reading Wireless status\n");
	else
		return (status & mask) ? 1 : 0;

	return (hotk->status & mask) ? 1 : 0;
}

static int read_gps_status(void)
{
	unsigned long long status;
	acpi_status rv = AE_OK;

	rv = acpi_evaluate_integer(gps_status_handle, NULL, NULL, &status);
	if (ACPI_FAILURE(rv))
		pr_warning("Error reading GPS status\n");
	else
		return status ? 1 : 0;

	return (hotk->status & GPS_ON) ? 1 : 0;
}


static int read_status(int mask)
{
	
	if (mask == BT_ON || mask == WL_ON)
		return read_wireless_status(mask);
	else if (mask == GPS_ON)
		return read_gps_status();

	return (hotk->status & mask) ? 1 : 0;
}

static void write_status(acpi_handle handle, int out, int mask)
{
	hotk->status = (out) ? (hotk->status | mask) : (hotk->status & ~mask);

	switch (mask) {
	case MLED_ON:
		out = !(out & 0x1);
		break;
	case GLED_ON:
		out = (out & 0x1) + 1;
		break;
	case GPS_ON:
		handle = (out) ? gps_on_handle : gps_off_handle;
		out = 0x02;
		break;
	default:
		out &= 0x1;
		break;
	}

	if (write_acpi_int(handle, NULL, out, NULL))
		pr_warning(" write failed %x\n", mask);
}


#define ASUS_LED_HANDLER(object, mask)					\
	static void object##_led_set(struct led_classdev *led_cdev,	\
				     enum led_brightness value)		\
	{								\
		object##_led_wk = (value > 0) ? 1 : 0;			\
		queue_work(led_workqueue, &object##_led_work);		\
	}								\
	static void object##_led_update(struct work_struct *ignored)	\
	{								\
		int value = object##_led_wk;				\
		write_status(object##_set_handle, value, (mask));	\
	}								\
	static enum led_brightness object##_led_get(			\
		struct led_classdev *led_cdev)				\
	{								\
		return led_cdev->brightness;				\
	}

ASUS_LED_HANDLER(mled, MLED_ON);
ASUS_LED_HANDLER(pled, PLED_ON);
ASUS_LED_HANDLER(rled, RLED_ON);
ASUS_LED_HANDLER(tled, TLED_ON);
ASUS_LED_HANDLER(gled, GLED_ON);


static int get_kled_lvl(void)
{
	unsigned long long kblv;
	struct acpi_object_list params;
	union acpi_object in_obj;
	acpi_status rv;

	params.count = 1;
	params.pointer = &in_obj;
	in_obj.type = ACPI_TYPE_INTEGER;
	in_obj.integer.value = 2;

	rv = acpi_evaluate_integer(kled_get_handle, NULL, &params, &kblv);
	if (ACPI_FAILURE(rv)) {
		pr_warning("Error reading kled level\n");
		return 0;
	}
	return kblv;
}

static int set_kled_lvl(int kblv)
{
	if (kblv > 0)
		kblv = (1 << 7) | (kblv & 0x7F);
	else
		kblv = 0;

	if (write_acpi_int(kled_set_handle, NULL, kblv, NULL)) {
		pr_warning("Keyboard LED display write failed\n");
		return -EINVAL;
	}
	return 0;
}

static void kled_led_set(struct led_classdev *led_cdev,
			 enum led_brightness value)
{
	kled_led_wk = value;
	queue_work(led_workqueue, &kled_led_work);
}

static void kled_led_update(struct work_struct *ignored)
{
	set_kled_lvl(kled_led_wk);
}

static enum led_brightness kled_led_get(struct led_classdev *led_cdev)
{
	return get_kled_lvl();
}

static int get_lcd_state(void)
{
	return read_status(LCD_ON);
}

static int set_lcd_state(int value)
{
	int lcd = 0;
	acpi_status status = 0;

	lcd = value ? 1 : 0;

	if (lcd == get_lcd_state())
		return 0;

	if (lcd_switch_handle) {
		status = acpi_evaluate_object(lcd_switch_handle,
					      NULL, NULL, NULL);

		if (ACPI_FAILURE(status))
			pr_warning("Error switching LCD\n");
	}

	write_status(NULL, lcd, LCD_ON);
	return 0;
}

static void lcd_blank(int blank)
{
	struct backlight_device *bd = asus_backlight_device;

	if (bd) {
		bd->props.power = blank;
		backlight_update_status(bd);
	}
}

static int read_brightness(struct backlight_device *bd)
{
	unsigned long long value;
	acpi_status rv = AE_OK;

	rv = acpi_evaluate_integer(brightness_get_handle, NULL, NULL, &value);
	if (ACPI_FAILURE(rv))
		pr_warning("Error reading brightness\n");

	return value;
}

static int set_brightness(struct backlight_device *bd, int value)
{
	int ret = 0;

	value = (0 < value) ? ((15 < value) ? 15 : value) : 0;
	

	if (write_acpi_int(brightness_set_handle, NULL, value, NULL)) {
		pr_warning("Error changing brightness\n");
		ret = -EIO;
	}

	return ret;
}

static int update_bl_status(struct backlight_device *bd)
{
	int rv;
	int value = bd->props.brightness;

	rv = set_brightness(bd, value);
	if (rv)
		return rv;

	value = (bd->props.power == FB_BLANK_UNBLANK) ? 1 : 0;
	return set_lcd_state(value);
}




static ssize_t show_infos(struct device *dev,
			  struct device_attribute *attr, char *page)
{
	int len = 0;
	unsigned long long temp;
	char buf[16];		
	acpi_status rv = AE_OK;

	

	len += sprintf(page, ASUS_HOTK_NAME " " ASUS_LAPTOP_VERSION "\n");
	len += sprintf(page + len, "Model reference    : %s\n", hotk->name);
	
	rv = acpi_evaluate_integer(hotk->handle, "SFUN", NULL, &temp);
	if (!ACPI_FAILURE(rv))
		len += sprintf(page + len, "SFUN value         : %#x\n",
			       (uint) temp);
	
	rv = acpi_evaluate_integer(hotk->handle, "HRWS", NULL, &temp);
	if (!ACPI_FAILURE(rv))
		len += sprintf(page + len, "HRWS value         : %#x\n",
			       (uint) temp);
	
	rv = acpi_evaluate_integer(hotk->handle, "ASYM", NULL, &temp);
	if (!ACPI_FAILURE(rv))
		len += sprintf(page + len, "ASYM value         : %#x\n",
			       (uint) temp);
	if (asus_info) {
		snprintf(buf, 16, "%d", asus_info->length);
		len += sprintf(page + len, "DSDT length        : %s\n", buf);
		snprintf(buf, 16, "%d", asus_info->checksum);
		len += sprintf(page + len, "DSDT checksum      : %s\n", buf);
		snprintf(buf, 16, "%d", asus_info->revision);
		len += sprintf(page + len, "DSDT revision      : %s\n", buf);
		snprintf(buf, 7, "%s", asus_info->oem_id);
		len += sprintf(page + len, "OEM id             : %s\n", buf);
		snprintf(buf, 9, "%s", asus_info->oem_table_id);
		len += sprintf(page + len, "OEM table id       : %s\n", buf);
		snprintf(buf, 16, "%x", asus_info->oem_revision);
		len += sprintf(page + len, "OEM revision       : 0x%s\n", buf);
		snprintf(buf, 5, "%s", asus_info->asl_compiler_id);
		len += sprintf(page + len, "ASL comp vendor id : %s\n", buf);
		snprintf(buf, 16, "%x", asus_info->asl_compiler_revision);
		len += sprintf(page + len, "ASL comp revision  : 0x%s\n", buf);
	}

	return len;
}

static int parse_arg(const char *buf, unsigned long count, int *val)
{
	if (!count)
		return 0;
	if (count > 31)
		return -EINVAL;
	if (sscanf(buf, "%i", val) != 1)
		return -EINVAL;
	return count;
}

static ssize_t store_status(const char *buf, size_t count,
			    acpi_handle handle, int mask)
{
	int rv, value;
	int out = 0;

	rv = parse_arg(buf, count, &value);
	if (rv > 0)
		out = value ? 1 : 0;

	write_status(handle, out, mask);

	return rv;
}


static ssize_t show_ledd(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "0x%08x\n", hotk->ledd_status);
}

static ssize_t store_ledd(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	int rv, value;

	rv = parse_arg(buf, count, &value);
	if (rv > 0) {
		if (write_acpi_int(ledd_set_handle, NULL, value, NULL))
			pr_warning("LED display write failed\n");
		else
			hotk->ledd_status = (u32) value;
	}
	return rv;
}


static ssize_t show_wlan(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", read_status(WL_ON));
}

static ssize_t store_wlan(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	return store_status(buf, count, wl_switch_handle, WL_ON);
}


static ssize_t show_bluetooth(struct device *dev,
			      struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", read_status(BT_ON));
}

static ssize_t store_bluetooth(struct device *dev,
			       struct device_attribute *attr, const char *buf,
			       size_t count)
{
	return store_status(buf, count, bt_switch_handle, BT_ON);
}


static void set_display(int value)
{
	
	if (write_acpi_int(display_set_handle, NULL, value, NULL))
		pr_warning("Error setting display\n");
	return;
}

static int read_display(void)
{
	unsigned long long value = 0;
	acpi_status rv = AE_OK;

	
	if (display_get_handle) {
		rv = acpi_evaluate_integer(display_get_handle, NULL,
					   NULL, &value);
		if (ACPI_FAILURE(rv))
			pr_warning("Error reading display status\n");
	}

	value &= 0x0F;		

	return value;
}


static ssize_t show_disp(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", read_display());
}


static ssize_t store_disp(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	int rv, value;

	rv = parse_arg(buf, count, &value);
	if (rv > 0)
		set_display(value);
	return rv;
}


static void set_light_sens_switch(int value)
{
	if (write_acpi_int(ls_switch_handle, NULL, value, NULL))
		pr_warning("Error setting light sensor switch\n");
	hotk->light_switch = value;
}

static ssize_t show_lssw(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", hotk->light_switch);
}

static ssize_t store_lssw(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	int rv, value;

	rv = parse_arg(buf, count, &value);
	if (rv > 0)
		set_light_sens_switch(value ? 1 : 0);

	return rv;
}

static void set_light_sens_level(int value)
{
	if (write_acpi_int(ls_level_handle, NULL, value, NULL))
		pr_warning("Error setting light sensor level\n");
	hotk->light_level = value;
}

static ssize_t show_lslvl(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", hotk->light_level);
}

static ssize_t store_lslvl(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	int rv, value;

	rv = parse_arg(buf, count, &value);
	if (rv > 0) {
		value = (0 < value) ? ((15 < value) ? 15 : value) : 0;
		
		set_light_sens_level(value);
	}

	return rv;
}


static ssize_t show_gps(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", read_status(GPS_ON));
}

static ssize_t store_gps(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	return store_status(buf, count, NULL, GPS_ON);
}


static struct key_entry *asus_get_entry_by_scancode(int code)
{
	struct key_entry *key;

	for (key = asus_keymap; key->type != KE_END; key++)
		if (code == key->code)
			return key;

	return NULL;
}

static struct key_entry *asus_get_entry_by_keycode(int code)
{
	struct key_entry *key;

	for (key = asus_keymap; key->type != KE_END; key++)
		if (code == key->keycode && key->type == KE_KEY)
			return key;

	return NULL;
}

static int asus_getkeycode(struct input_dev *dev, int scancode, int *keycode)
{
	struct key_entry *key = asus_get_entry_by_scancode(scancode);

	if (key && key->type == KE_KEY) {
		*keycode = key->keycode;
		return 0;
	}

	return -EINVAL;
}

static int asus_setkeycode(struct input_dev *dev, int scancode, int keycode)
{
	struct key_entry *key;
	int old_keycode;

	if (keycode < 0 || keycode > KEY_MAX)
		return -EINVAL;

	key = asus_get_entry_by_scancode(scancode);
	if (key && key->type == KE_KEY) {
		old_keycode = key->keycode;
		key->keycode = keycode;
		set_bit(keycode, dev->keybit);
		if (!asus_get_entry_by_keycode(old_keycode))
			clear_bit(old_keycode, dev->keybit);
		return 0;
	}

	return -EINVAL;
}

static void asus_hotk_notify(struct acpi_device *device, u32 event)
{
	static struct key_entry *key;
	u16 count;

	
	if (!hotk)
		return;

	
	if (event == ATKD_LCD_ON) {
		write_status(NULL, 1, LCD_ON);
		lcd_blank(FB_BLANK_UNBLANK);
	} else if (event == ATKD_LCD_OFF) {
		write_status(NULL, 0, LCD_ON);
		lcd_blank(FB_BLANK_POWERDOWN);
	}

	count = hotk->event_count[event % 128]++;
	acpi_bus_generate_proc_event(hotk->device, event, count);
	acpi_bus_generate_netlink_event(hotk->device->pnp.device_class,
					dev_name(&hotk->device->dev), event,
					count);

	if (hotk->inputdev) {
		key = asus_get_entry_by_scancode(event);
		if (!key)
			return ;

		switch (key->type) {
		case KE_KEY:
			input_report_key(hotk->inputdev, key->keycode, 1);
			input_sync(hotk->inputdev);
			input_report_key(hotk->inputdev, key->keycode, 0);
			input_sync(hotk->inputdev);
			break;
		}
	}
}

#define ASUS_CREATE_DEVICE_ATTR(_name)					\
	struct device_attribute dev_attr_##_name = {			\
		.attr = {						\
			.name = __stringify(_name),			\
			.mode = 0 },					\
		.show   = NULL,						\
		.store  = NULL,						\
	}

#define ASUS_SET_DEVICE_ATTR(_name, _mode, _show, _store)		\
	do {								\
		dev_attr_##_name.attr.mode = _mode;			\
		dev_attr_##_name.show = _show;				\
		dev_attr_##_name.store = _store;			\
	} while(0)

static ASUS_CREATE_DEVICE_ATTR(infos);
static ASUS_CREATE_DEVICE_ATTR(wlan);
static ASUS_CREATE_DEVICE_ATTR(bluetooth);
static ASUS_CREATE_DEVICE_ATTR(display);
static ASUS_CREATE_DEVICE_ATTR(ledd);
static ASUS_CREATE_DEVICE_ATTR(ls_switch);
static ASUS_CREATE_DEVICE_ATTR(ls_level);
static ASUS_CREATE_DEVICE_ATTR(gps);

static struct attribute *asuspf_attributes[] = {
	&dev_attr_infos.attr,
	&dev_attr_wlan.attr,
	&dev_attr_bluetooth.attr,
	&dev_attr_display.attr,
	&dev_attr_ledd.attr,
	&dev_attr_ls_switch.attr,
	&dev_attr_ls_level.attr,
	&dev_attr_gps.attr,
	NULL
};

static struct attribute_group asuspf_attribute_group = {
	.attrs = asuspf_attributes
};

static struct platform_driver asuspf_driver = {
	.driver = {
		   .name = ASUS_HOTK_FILE,
		   .owner = THIS_MODULE,
		   }
};

static struct platform_device *asuspf_device;

static void asus_hotk_add_fs(void)
{
	ASUS_SET_DEVICE_ATTR(infos, 0444, show_infos, NULL);

	if (wl_switch_handle)
		ASUS_SET_DEVICE_ATTR(wlan, 0644, show_wlan, store_wlan);

	if (bt_switch_handle)
		ASUS_SET_DEVICE_ATTR(bluetooth, 0644,
				     show_bluetooth, store_bluetooth);

	if (display_set_handle && display_get_handle)
		ASUS_SET_DEVICE_ATTR(display, 0644, show_disp, store_disp);
	else if (display_set_handle)
		ASUS_SET_DEVICE_ATTR(display, 0200, NULL, store_disp);

	if (ledd_set_handle)
		ASUS_SET_DEVICE_ATTR(ledd, 0644, show_ledd, store_ledd);

	if (ls_switch_handle && ls_level_handle) {
		ASUS_SET_DEVICE_ATTR(ls_level, 0644, show_lslvl, store_lslvl);
		ASUS_SET_DEVICE_ATTR(ls_switch, 0644, show_lssw, store_lssw);
	}

	if (gps_status_handle && gps_on_handle && gps_off_handle)
		ASUS_SET_DEVICE_ATTR(gps, 0644, show_gps, store_gps);
}

static int asus_handle_init(char *name, acpi_handle * handle,
			    char **paths, int num_paths)
{
	int i;
	acpi_status status;

	for (i = 0; i < num_paths; i++) {
		status = acpi_get_handle(NULL, paths[i], handle);
		if (ACPI_SUCCESS(status))
			return 0;
	}

	*handle = NULL;
	return -ENODEV;
}

#define ASUS_HANDLE_INIT(object)					\
	asus_handle_init(#object, &object##_handle, object##_paths,	\
			 ARRAY_SIZE(object##_paths))


static int asus_hotk_get_info(void)
{
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *model = NULL;
	unsigned long long bsts_result, hwrs_result;
	char *string = NULL;
	acpi_status status;

	
	status = acpi_get_table(ACPI_SIG_DSDT, 1, &asus_info);
	if (ACPI_FAILURE(status))
		pr_warning("Couldn't get the DSDT table header\n");

	
	if (write_acpi_int(hotk->handle, "INIT", 0, &buffer)) {
		pr_err("Hotkey initialization failed\n");
		return -ENODEV;
	}

	
	status =
	    acpi_evaluate_integer(hotk->handle, "BSTS", NULL, &bsts_result);
	if (ACPI_FAILURE(status))
		pr_warning("Error calling BSTS\n");
	else if (bsts_result)
		pr_notice("BSTS called, 0x%02x returned\n",
		       (uint) bsts_result);

	
	write_acpi_int(hotk->handle, "CWAP", wapf, NULL);

	
	if (buffer.pointer) {
		model = buffer.pointer;
		switch (model->type) {
		case ACPI_TYPE_STRING:
			string = model->string.pointer;
			break;
		case ACPI_TYPE_BUFFER:
			string = model->buffer.pointer;
			break;
		default:
			string = "";
			break;
		}
	}
	hotk->name = kstrdup(string, GFP_KERNEL);
	if (!hotk->name)
		return -ENOMEM;

	if (*string)
		pr_notice("  %s model detected\n", string);

	ASUS_HANDLE_INIT(mled_set);
	ASUS_HANDLE_INIT(tled_set);
	ASUS_HANDLE_INIT(rled_set);
	ASUS_HANDLE_INIT(pled_set);
	ASUS_HANDLE_INIT(gled_set);

	ASUS_HANDLE_INIT(ledd_set);

	ASUS_HANDLE_INIT(kled_set);
	ASUS_HANDLE_INIT(kled_get);

	
	status =
	    acpi_evaluate_integer(hotk->handle, "HRWS", NULL, &hwrs_result);
	if (ACPI_FAILURE(status))
		hwrs_result = WL_HWRS | BT_HWRS;

	if (hwrs_result & WL_HWRS)
		ASUS_HANDLE_INIT(wl_switch);
	if (hwrs_result & BT_HWRS)
		ASUS_HANDLE_INIT(bt_switch);

	ASUS_HANDLE_INIT(wireless_status);

	ASUS_HANDLE_INIT(brightness_set);
	ASUS_HANDLE_INIT(brightness_get);

	ASUS_HANDLE_INIT(lcd_switch);

	ASUS_HANDLE_INIT(display_set);
	ASUS_HANDLE_INIT(display_get);

	
	if (!ASUS_HANDLE_INIT(ls_switch))
		ASUS_HANDLE_INIT(ls_level);

	ASUS_HANDLE_INIT(gps_on);
	ASUS_HANDLE_INIT(gps_off);
	ASUS_HANDLE_INIT(gps_status);

	kfree(model);

	return AE_OK;
}

static int asus_input_init(void)
{
	const struct key_entry *key;
	int result;

	hotk->inputdev = input_allocate_device();
	if (!hotk->inputdev) {
		pr_info("Unable to allocate input device\n");
		return 0;
	}
	hotk->inputdev->name = "Asus Laptop extra buttons";
	hotk->inputdev->phys = ASUS_HOTK_FILE "/input0";
	hotk->inputdev->id.bustype = BUS_HOST;
	hotk->inputdev->getkeycode = asus_getkeycode;
	hotk->inputdev->setkeycode = asus_setkeycode;

	for (key = asus_keymap; key->type != KE_END; key++) {
		switch (key->type) {
		case KE_KEY:
			set_bit(EV_KEY, hotk->inputdev->evbit);
			set_bit(key->keycode, hotk->inputdev->keybit);
			break;
		}
	}
	result = input_register_device(hotk->inputdev);
	if (result) {
		pr_info("Unable to register input device\n");
		input_free_device(hotk->inputdev);
	}
	return result;
}

static int asus_hotk_check(void)
{
	int result = 0;

	result = acpi_bus_get_status(hotk->device);
	if (result)
		return result;

	if (hotk->device->status.present) {
		result = asus_hotk_get_info();
	} else {
		pr_err("Hotkey device not present, aborting\n");
		return -EINVAL;
	}

	return result;
}

static int asus_hotk_found;

static int asus_hotk_add(struct acpi_device *device)
{
	int result;

	if (!device)
		return -EINVAL;

	pr_notice("Asus Laptop Support version %s\n",
	       ASUS_LAPTOP_VERSION);

	hotk = kzalloc(sizeof(struct asus_hotk), GFP_KERNEL);
	if (!hotk)
		return -ENOMEM;

	hotk->handle = device->handle;
	strcpy(acpi_device_name(device), ASUS_HOTK_DEVICE_NAME);
	strcpy(acpi_device_class(device), ASUS_HOTK_CLASS);
	device->driver_data = hotk;
	hotk->device = device;

	result = asus_hotk_check();
	if (result)
		goto end;

	asus_hotk_add_fs();

	asus_hotk_found = 1;

	
	write_status(bt_switch_handle, 1, BT_ON);
	write_status(wl_switch_handle, 1, WL_ON);

	
	write_status(NULL, read_status(BT_ON), BT_ON);
	write_status(NULL, read_status(WL_ON), WL_ON);

	
	write_status(NULL, 1, LCD_ON);

	
	if (kled_set_handle)
		set_kled_lvl(1);

	
	hotk->ledd_status = 0xFFF;

	
	hotk->light_switch = 0;	
	hotk->light_level = 5;	

	if (ls_switch_handle)
		set_light_sens_switch(hotk->light_switch);

	if (ls_level_handle)
		set_light_sens_level(hotk->light_level);

	
	write_status(NULL, 1, GPS_ON);

end:
	if (result) {
		kfree(hotk->name);
		kfree(hotk);
	}

	return result;
}

static int asus_hotk_remove(struct acpi_device *device, int type)
{
	if (!device || !acpi_driver_data(device))
		return -EINVAL;

	kfree(hotk->name);
	kfree(hotk);

	return 0;
}

static void asus_backlight_exit(void)
{
	if (asus_backlight_device)
		backlight_device_unregister(asus_backlight_device);
}

#define  ASUS_LED_UNREGISTER(object)				\
	if (object##_led.dev)					\
		led_classdev_unregister(&object##_led)

static void asus_led_exit(void)
{
	destroy_workqueue(led_workqueue);
	ASUS_LED_UNREGISTER(mled);
	ASUS_LED_UNREGISTER(tled);
	ASUS_LED_UNREGISTER(pled);
	ASUS_LED_UNREGISTER(rled);
	ASUS_LED_UNREGISTER(gled);
	ASUS_LED_UNREGISTER(kled);
}

static void asus_input_exit(void)
{
	if (hotk->inputdev)
		input_unregister_device(hotk->inputdev);
}

static void __exit asus_laptop_exit(void)
{
	asus_backlight_exit();
	asus_led_exit();
	asus_input_exit();

	acpi_bus_unregister_driver(&asus_hotk_driver);
	sysfs_remove_group(&asuspf_device->dev.kobj, &asuspf_attribute_group);
	platform_device_unregister(asuspf_device);
	platform_driver_unregister(&asuspf_driver);
}

static int asus_backlight_init(struct device *dev)
{
	struct backlight_device *bd;

	if (brightness_set_handle && lcd_switch_handle) {
		bd = backlight_device_register(ASUS_HOTK_FILE, dev,
					       NULL, &asusbl_ops);
		if (IS_ERR(bd)) {
			pr_err("Could not register asus backlight device\n");
			asus_backlight_device = NULL;
			return PTR_ERR(bd);
		}

		asus_backlight_device = bd;

		bd->props.max_brightness = 15;
		bd->props.brightness = read_brightness(NULL);
		bd->props.power = FB_BLANK_UNBLANK;
		backlight_update_status(bd);
	}
	return 0;
}

static int asus_led_register(acpi_handle handle,
			     struct led_classdev *ldev, struct device *dev)
{
	if (!handle)
		return 0;

	return led_classdev_register(dev, ldev);
}

#define ASUS_LED_REGISTER(object, device)				\
	asus_led_register(object##_set_handle, &object##_led, device)

static int asus_led_init(struct device *dev)
{
	int rv;

	rv = ASUS_LED_REGISTER(mled, dev);
	if (rv)
		goto out;

	rv = ASUS_LED_REGISTER(tled, dev);
	if (rv)
		goto out1;

	rv = ASUS_LED_REGISTER(rled, dev);
	if (rv)
		goto out2;

	rv = ASUS_LED_REGISTER(pled, dev);
	if (rv)
		goto out3;

	rv = ASUS_LED_REGISTER(gled, dev);
	if (rv)
		goto out4;

	if (kled_set_handle && kled_get_handle)
		rv = ASUS_LED_REGISTER(kled, dev);
	if (rv)
		goto out5;

	led_workqueue = create_singlethread_workqueue("led_workqueue");
	if (!led_workqueue)
		goto out6;

	return 0;
out6:
	rv = -ENOMEM;
	ASUS_LED_UNREGISTER(kled);
out5:
	ASUS_LED_UNREGISTER(gled);
out4:
	ASUS_LED_UNREGISTER(pled);
out3:
	ASUS_LED_UNREGISTER(rled);
out2:
	ASUS_LED_UNREGISTER(tled);
out1:
	ASUS_LED_UNREGISTER(mled);
out:
	return rv;
}

static int __init asus_laptop_init(void)
{
	int result;

	if (acpi_disabled)
		return -ENODEV;

	result = acpi_bus_register_driver(&asus_hotk_driver);
	if (result < 0)
		return result;

	
	if (!asus_hotk_found) {
		acpi_bus_unregister_driver(&asus_hotk_driver);
		return -ENODEV;
	}

	result = asus_input_init();
	if (result)
		goto fail_input;

	
	result = platform_driver_register(&asuspf_driver);
	if (result)
		goto fail_platform_driver;

	asuspf_device = platform_device_alloc(ASUS_HOTK_FILE, -1);
	if (!asuspf_device) {
		result = -ENOMEM;
		goto fail_platform_device1;
	}

	result = platform_device_add(asuspf_device);
	if (result)
		goto fail_platform_device2;

	result = sysfs_create_group(&asuspf_device->dev.kobj,
				    &asuspf_attribute_group);
	if (result)
		goto fail_sysfs;

	result = asus_led_init(&asuspf_device->dev);
	if (result)
		goto fail_led;

	if (!acpi_video_backlight_support()) {
		result = asus_backlight_init(&asuspf_device->dev);
		if (result)
			goto fail_backlight;
	} else
		pr_info("Brightness ignored, must be controlled by "
		       "ACPI video driver\n");

	return 0;

fail_backlight:
       asus_led_exit();

fail_led:
       sysfs_remove_group(&asuspf_device->dev.kobj,
			  &asuspf_attribute_group);

fail_sysfs:
	platform_device_del(asuspf_device);

fail_platform_device2:
	platform_device_put(asuspf_device);

fail_platform_device1:
	platform_driver_unregister(&asuspf_driver);

fail_platform_driver:
	asus_input_exit();

fail_input:

	return result;
}

module_init(asus_laptop_init);
module_exit(asus_laptop_exit);
