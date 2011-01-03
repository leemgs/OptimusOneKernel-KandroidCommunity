

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/backlight.h>
#include <acpi/acpi_drivers.h>
#include <acpi/acpi_bus.h>
#include <asm/uaccess.h>

#define ASUS_ACPI_VERSION "0.30"

#define PROC_ASUS       "asus"	
#define PROC_MLED       "mled"
#define PROC_WLED       "wled"
#define PROC_TLED       "tled"
#define PROC_BT         "bluetooth"
#define PROC_LEDD       "ledd"
#define PROC_INFO       "info"
#define PROC_LCD        "lcd"
#define PROC_BRN        "brn"
#define PROC_DISP       "disp"

#define ACPI_HOTK_NAME          "Asus Laptop ACPI Extras Driver"
#define ACPI_HOTK_CLASS         "hotkey"
#define ACPI_HOTK_DEVICE_NAME   "Hotkey"


#define BR_UP       0x10
#define BR_DOWN     0x20


#define MLED_ON     0x01	
#define WLED_ON     0x02	
#define TLED_ON     0x04	
#define BT_ON       0x08	

MODULE_AUTHOR("Julien Lerouge, Karol Kozimor");
MODULE_DESCRIPTION(ACPI_HOTK_NAME);
MODULE_LICENSE("GPL");

static uid_t asus_uid;
static gid_t asus_gid;
module_param(asus_uid, uint, 0);
MODULE_PARM_DESC(asus_uid, "UID for entries in /proc/acpi/asus");
module_param(asus_gid, uint, 0);
MODULE_PARM_DESC(asus_gid, "GID for entries in /proc/acpi/asus");


struct model_data {
	char *name;		
	char *mt_mled;		
	char *mled_status;	
	char *mt_wled;		
	char *wled_status;	
	char *mt_tled;		
	char *tled_status;	
	char *mt_ledd;		
	char *mt_bt_switch;	
	char *bt_status;	
	char *mt_lcd_switch;	
	char *lcd_status;	
	char *brightness_up;	
	char *brightness_down;	
	char *brightness_set;	
	char *brightness_get;	
	char *brightness_status;
	char *display_set;	
	char *display_get;	
};


struct asus_hotk {
	struct acpi_device *device;	
	acpi_handle handle;		
	char status;			
	u32 ledd_status;		
	struct model_data *methods;	
	u8 brightness;			
	enum {
		A1x = 0,	
		A2x,		
		A4G,		
		D1x,		
		L2D,		
		L3C,		
		L3D,		
		L3H,		
		L4R,		
		L5x,		
		L8L,		
		M1A,		
		M2E,		
		M6N,		
		M6R,		
		P30,		
		S1x,		
		S2x,		
		W1N,		
		W5A,		
		W3V,            
		xxN,		
		A4S,            
		F3Sa,		
		R1F,
		END_MODEL
	} model;		
	u16 event_count[128];	
};


#define A1x_PREFIX "\\_SB.PCI0.ISA.EC0."
#define L3C_PREFIX "\\_SB.PCI0.PX40.ECD0."
#define M1A_PREFIX "\\_SB.PCI0.PX40.EC0."
#define P30_PREFIX "\\_SB.PCI0.LPCB.EC0."
#define S1x_PREFIX "\\_SB.PCI0.PX40."
#define S2x_PREFIX A1x_PREFIX
#define xxN_PREFIX "\\_SB.PCI0.SBRG.EC0."

static struct model_data model_conf[END_MODEL] = {
	

	{
	 .name = "A1x",
	 .mt_mled = "MLED",
	 .mled_status = "\\MAIL",
	 .mt_lcd_switch = A1x_PREFIX "_Q10",
	 .lcd_status = "\\BKLI",
	 .brightness_up = A1x_PREFIX "_Q0E",
	 .brightness_down = A1x_PREFIX "_Q0F"},

	{
	 .name = "A2x",
	 .mt_mled = "MLED",
	 .mt_wled = "WLED",
	 .wled_status = "\\SG66",
	 .mt_lcd_switch = "\\Q10",
	 .lcd_status = "\\BAOF",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\INFB"},

	{
	 .name = "A4G",
	 .mt_mled = "MLED",

	 .mt_lcd_switch = xxN_PREFIX "_Q10",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\ADVG"},

	{
	 .name = "D1x",
	 .mt_mled = "MLED",
	 .mt_lcd_switch = "\\Q0D",
	 .lcd_status = "\\GP11",
	 .brightness_up = "\\Q0C",
	 .brightness_down = "\\Q0B",
	 .brightness_status = "\\BLVL",
	 .display_set = "SDSP",
	 .display_get = "\\INFB"},

	{
	 .name = "L2D",
	 .mt_mled = "MLED",
	 .mled_status = "\\SGP6",
	 .mt_wled = "WLED",
	 .wled_status = "\\RCP3",
	 .mt_lcd_switch = "\\Q10",
	 .lcd_status = "\\SGP0",
	 .brightness_up = "\\Q0E",
	 .brightness_down = "\\Q0F",
	 .display_set = "SDSP",
	 .display_get = "\\INFB"},

	{
	 .name = "L3C",
	 .mt_mled = "MLED",
	 .mt_wled = "WLED",
	 .mt_lcd_switch = L3C_PREFIX "_Q10",
	 .lcd_status = "\\GL32",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\_SB.PCI0.PCI1.VGAC.NMAP"},

	{
	 .name = "L3D",
	 .mt_mled = "MLED",
	 .mled_status = "\\MALD",
	 .mt_wled = "WLED",
	 .mt_lcd_switch = "\\Q10",
	 .lcd_status = "\\BKLG",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\INFB"},

	{
	 .name = "L3H",
	 .mt_mled = "MLED",
	 .mt_wled = "WLED",
	 .mt_lcd_switch = "EHK",
	 .lcd_status = "\\_SB.PCI0.PM.PBC",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\INFB"},

	{
	 .name = "L4R",
	 .mt_mled = "MLED",
	 .mt_wled = "WLED",
	 .wled_status = "\\_SB.PCI0.SBRG.SG13",
	 .mt_lcd_switch = xxN_PREFIX "_Q10",
	 .lcd_status = "\\_SB.PCI0.SBSM.SEO4",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\_SB.PCI0.P0P1.VGA.GETD"},

	{
	 .name = "L5x",
	 .mt_mled = "MLED",

	 .mt_tled = "TLED",
	 .mt_lcd_switch = "\\Q0D",
	 .lcd_status = "\\BAOF",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\INFB"},

	{
	 .name = "L8L"

	 },

	{
	 .name = "M1A",
	 .mt_mled = "MLED",
	 .mt_lcd_switch = M1A_PREFIX "Q10",
	 .lcd_status = "\\PNOF",
	 .brightness_up = M1A_PREFIX "Q0E",
	 .brightness_down = M1A_PREFIX "Q0F",
	 .brightness_status = "\\BRIT",
	 .display_set = "SDSP",
	 .display_get = "\\INFB"},

	{
	 .name = "M2E",
	 .mt_mled = "MLED",
	 .mt_wled = "WLED",
	 .mt_lcd_switch = "\\Q10",
	 .lcd_status = "\\GP06",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\INFB"},

	{
	 .name = "M6N",
	 .mt_mled = "MLED",
	 .mt_wled = "WLED",
	 .wled_status = "\\_SB.PCI0.SBRG.SG13",
	 .mt_lcd_switch = xxN_PREFIX "_Q10",
	 .lcd_status = "\\_SB.BKLT",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\SSTE"},

	{
	 .name = "M6R",
	 .mt_mled = "MLED",
	 .mt_wled = "WLED",
	 .mt_lcd_switch = xxN_PREFIX "_Q10",
	 .lcd_status = "\\_SB.PCI0.SBSM.SEO4",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\_SB.PCI0.P0P1.VGA.GETD"},

	{
	 .name = "P30",
	 .mt_wled = "WLED",
	 .mt_lcd_switch = P30_PREFIX "_Q0E",
	 .lcd_status = "\\BKLT",
	 .brightness_up = P30_PREFIX "_Q68",
	 .brightness_down = P30_PREFIX "_Q69",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\DNXT"},

	{
	 .name = "S1x",
	 .mt_mled = "MLED",
	 .mled_status = "\\EMLE",
	 .mt_wled = "WLED",
	 .mt_lcd_switch = S1x_PREFIX "Q10",
	 .lcd_status = "\\PNOF",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV"},

	{
	 .name = "S2x",
	 .mt_mled = "MLED",
	 .mled_status = "\\MAIL",
	 .mt_lcd_switch = S2x_PREFIX "_Q10",
	 .lcd_status = "\\BKLI",
	 .brightness_up = S2x_PREFIX "_Q0B",
	 .brightness_down = S2x_PREFIX "_Q0A"},

	{
	 .name = "W1N",
	 .mt_mled = "MLED",
	 .mt_wled = "WLED",
	 .mt_ledd = "SLCM",
	 .mt_lcd_switch = xxN_PREFIX "_Q10",
	 .lcd_status = "\\BKLT",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\ADVG"},

	{
	 .name = "W5A",
	 .mt_bt_switch = "BLED",
	 .mt_wled = "WLED",
	 .mt_lcd_switch = xxN_PREFIX "_Q10",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\ADVG"},

	{
	 .name = "W3V",
	 .mt_mled = "MLED",
	 .mt_wled = "WLED",
	 .mt_lcd_switch = xxN_PREFIX "_Q10",
	 .lcd_status = "\\BKLT",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	 .display_get = "\\INFB"},

       {
	 .name = "xxN",
	 .mt_mled = "MLED",

	 .mt_lcd_switch = xxN_PREFIX "_Q10",
	 .lcd_status = "\\BKLT",
	 .brightness_set = "SPLV",
	 .brightness_get = "GPLV",
	 .display_set = "SDSP",
	.display_get = "\\ADVG"},

	{
		.name              = "A4S",
		.brightness_set    = "SPLV",
		.brightness_get    = "GPLV",
		.mt_bt_switch      = "BLED",
		.mt_wled           = "WLED"
	},

	{
		.name		= "F3Sa",
		.mt_bt_switch	= "BLED",
		.mt_wled	= "WLED",
		.mt_mled	= "MLED",
		.brightness_get	= "GPLV",
		.brightness_set	= "SPLV",
		.mt_lcd_switch	= "\\_SB.PCI0.SBRG.EC0._Q10",
		.lcd_status	= "\\_SB.PCI0.SBRG.EC0.RPIN",
		.display_get	= "\\ADVG",
		.display_set	= "SDSP",
	},
	{
		.name = "R1F",
		.mt_bt_switch = "BLED",
		.mt_mled = "MLED",
		.mt_wled = "WLED",
		.mt_lcd_switch = "\\Q10",
		.lcd_status = "\\GP06",
		.brightness_set = "SPLV",
		.brightness_get = "GPLV",
		.display_set = "SDSP",
		.display_get = "\\INFB"
	}
};


static struct proc_dir_entry *asus_proc_dir;

static struct backlight_device *asus_backlight_device;


static struct acpi_table_header *asus_info;


static struct asus_hotk *hotk;


static int asus_hotk_add(struct acpi_device *device);
static int asus_hotk_remove(struct acpi_device *device, int type);
static void asus_hotk_notify(struct acpi_device *device, u32 event);

static const struct acpi_device_id asus_device_ids[] = {
	{"ATK0100", 0},
	{"", 0},
};
MODULE_DEVICE_TABLE(acpi, asus_device_ids);

static struct acpi_driver asus_hotk_driver = {
	.name = "asus_acpi",
	.class = ACPI_HOTK_CLASS,
	.ids = asus_device_ids,
	.flags = ACPI_DRIVER_ALL_NOTIFY_EVENTS,
	.ops = {
		.add = asus_hotk_add,
		.remove = asus_hotk_remove,
		.notify = asus_hotk_notify,
		},
};


static int write_acpi_int(acpi_handle handle, const char *method, int val,
			  struct acpi_buffer *output)
{
	struct acpi_object_list params;	
	union acpi_object in_obj;	
	acpi_status status;

	params.count = 1;
	params.pointer = &in_obj;
	in_obj.type = ACPI_TYPE_INTEGER;
	in_obj.integer.value = val;

	status = acpi_evaluate_object(handle, (char *)method, &params, output);
	return (status == AE_OK);
}

static int read_acpi_int(acpi_handle handle, const char *method, int *val)
{
	struct acpi_buffer output;
	union acpi_object out_obj;
	acpi_status status;

	output.length = sizeof(out_obj);
	output.pointer = &out_obj;

	status = acpi_evaluate_object(handle, (char *)method, NULL, &output);
	*val = out_obj.integer.value;
	return (status == AE_OK) && (out_obj.type == ACPI_TYPE_INTEGER);
}


static int
proc_read_info(char *page, char **start, off_t off, int count, int *eof,
	       void *data)
{
	int len = 0;
	int temp;
	char buf[16];		
	

	len += sprintf(page, ACPI_HOTK_NAME " " ASUS_ACPI_VERSION "\n");
	len += sprintf(page + len, "Model reference    : %s\n",
		       hotk->methods->name);
	
	if (read_acpi_int(hotk->handle, "SFUN", &temp))
		len +=
		    sprintf(page + len, "SFUN value         : 0x%04x\n", temp);
	
	if (read_acpi_int(hotk->handle, "ASYM", &temp))
		len +=
		    sprintf(page + len, "ASYM value         : 0x%04x\n", temp);
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




static int read_led(const char *ledname, int ledmask)
{
	if (ledname) {
		int led_status;

		if (read_acpi_int(NULL, ledname, &led_status))
			return led_status;
		else
			printk(KERN_WARNING "Asus ACPI: Error reading LED "
			       "status\n");
	}
	return (hotk->status & ledmask) ? 1 : 0;
}

static int parse_arg(const char __user *buf, unsigned long count, int *val)
{
	char s[32];
	if (!count)
		return 0;
	if (count > 31)
		return -EINVAL;
	if (copy_from_user(s, buf, count))
		return -EFAULT;
	s[count] = 0;
	if (sscanf(s, "%i", val) != 1)
		return -EINVAL;
	return count;
}


static int
write_led(const char __user *buffer, unsigned long count,
	  char *ledname, int ledmask, int invert)
{
	int rv, value;
	int led_out = 0;

	rv = parse_arg(buffer, count, &value);
	if (rv > 0)
		led_out = value ? 1 : 0;

	hotk->status =
	    (led_out) ? (hotk->status | ledmask) : (hotk->status & ~ledmask);

	if (invert)		
		led_out = !led_out;

	if (!write_acpi_int(hotk->handle, ledname, led_out, NULL))
		printk(KERN_WARNING "Asus ACPI: LED (%s) write failed\n",
		       ledname);

	return rv;
}


static int
proc_read_mled(char *page, char **start, off_t off, int count, int *eof,
	       void *data)
{
	return sprintf(page, "%d\n",
		       read_led(hotk->methods->mled_status, MLED_ON));
}

static int
proc_write_mled(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
	return write_led(buffer, count, hotk->methods->mt_mled, MLED_ON, 1);
}


static int
proc_read_ledd(char *page, char **start, off_t off, int count, int *eof,
	       void *data)
{
	return sprintf(page, "0x%08x\n", hotk->ledd_status);
}

static int
proc_write_ledd(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
	int rv, value;

	rv = parse_arg(buffer, count, &value);
	if (rv > 0) {
		if (!write_acpi_int
		    (hotk->handle, hotk->methods->mt_ledd, value, NULL))
			printk(KERN_WARNING
			       "Asus ACPI: LED display write failed\n");
		else
			hotk->ledd_status = (u32) value;
	}
	return rv;
}


static int
proc_read_wled(char *page, char **start, off_t off, int count, int *eof,
	       void *data)
{
	return sprintf(page, "%d\n",
		       read_led(hotk->methods->wled_status, WLED_ON));
}

static int
proc_write_wled(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
	return write_led(buffer, count, hotk->methods->mt_wled, WLED_ON, 0);
}


static int
proc_read_bluetooth(char *page, char **start, off_t off, int count, int *eof,
		    void *data)
{
	return sprintf(page, "%d\n", read_led(hotk->methods->bt_status, BT_ON));
}

static int
proc_write_bluetooth(struct file *file, const char __user *buffer,
		     unsigned long count, void *data)
{
	
	return write_led(buffer, count, hotk->methods->mt_bt_switch, BT_ON, 0);
}


static int
proc_read_tled(char *page, char **start, off_t off, int count, int *eof,
	       void *data)
{
	return sprintf(page, "%d\n",
		       read_led(hotk->methods->tled_status, TLED_ON));
}

static int
proc_write_tled(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
	return write_led(buffer, count, hotk->methods->mt_tled, TLED_ON, 0);
}

static int get_lcd_state(void)
{
	int lcd = 0;

	if (hotk->model == L3H) {
		
		acpi_status status = 0;
		struct acpi_object_list input;
		union acpi_object mt_params[2];
		struct acpi_buffer output;
		union acpi_object out_obj;

		input.count = 2;
		input.pointer = mt_params;
		
		mt_params[0].type = ACPI_TYPE_INTEGER;
		mt_params[0].integer.value = 0x02;
		mt_params[1].type = ACPI_TYPE_INTEGER;
		mt_params[1].integer.value = 0x02;

		output.length = sizeof(out_obj);
		output.pointer = &out_obj;

		status =
		    acpi_evaluate_object(NULL, hotk->methods->lcd_status,
					 &input, &output);
		if (status != AE_OK)
			return -1;
		if (out_obj.type == ACPI_TYPE_INTEGER)
			
			lcd = out_obj.integer.value >> 8;
	} else if (hotk->model == F3Sa) {
		unsigned long long tmp;
		union acpi_object param;
		struct acpi_object_list input;
		acpi_status status;

		
		param.type = ACPI_TYPE_INTEGER;
		param.integer.value = 0x11;
		input.count = 1;
		input.pointer = &param;

		status = acpi_evaluate_integer(NULL, hotk->methods->lcd_status,
						&input, &tmp);
		if (status != AE_OK)
			return -1;

		lcd = tmp;
	} else {
		
		if (!read_acpi_int(NULL, hotk->methods->lcd_status, &lcd))
			printk(KERN_WARNING
			       "Asus ACPI: Error reading LCD status\n");

		if (hotk->model == L2D)
			lcd = ~lcd;
	}

	return (lcd & 1);
}

static int set_lcd_state(int value)
{
	int lcd = 0;
	acpi_status status = 0;

	lcd = value ? 1 : 0;
	if (lcd != get_lcd_state()) {
		
		if (hotk->model != L3H) {
			status =
			    acpi_evaluate_object(NULL,
						 hotk->methods->mt_lcd_switch,
						 NULL, NULL);
		} else {
			
			if (!write_acpi_int
			    (hotk->handle, hotk->methods->mt_lcd_switch, 0x07,
			     NULL))
				status = AE_ERROR;
			
		}
		if (ACPI_FAILURE(status))
			printk(KERN_WARNING "Asus ACPI: Error switching LCD\n");
	}
	return 0;

}

static int
proc_read_lcd(char *page, char **start, off_t off, int count, int *eof,
	      void *data)
{
	return sprintf(page, "%d\n", get_lcd_state());
}

static int
proc_write_lcd(struct file *file, const char __user *buffer,
	       unsigned long count, void *data)
{
	int rv, value;

	rv = parse_arg(buffer, count, &value);
	if (rv > 0)
		set_lcd_state(value);
	return rv;
}

static int read_brightness(struct backlight_device *bd)
{
	int value;

	if (hotk->methods->brightness_get) {	
		if (!read_acpi_int(hotk->handle, hotk->methods->brightness_get,
				   &value))
			printk(KERN_WARNING
			       "Asus ACPI: Error reading brightness\n");
	} else if (hotk->methods->brightness_status) {	
		if (!read_acpi_int(NULL, hotk->methods->brightness_status,
				   &value))
			printk(KERN_WARNING
			       "Asus ACPI: Error reading brightness\n");
	} else			
		value = hotk->brightness;
	return value;
}


static int set_brightness(int value)
{
	acpi_status status = 0;
	int ret = 0;

	
	if (hotk->methods->brightness_set) {
		if (!write_acpi_int(hotk->handle, hotk->methods->brightness_set,
				    value, NULL))
			printk(KERN_WARNING
			       "Asus ACPI: Error changing brightness\n");
			ret = -EIO;
		goto out;
	}

	
	value -= read_brightness(NULL);
	while (value != 0) {
		status = acpi_evaluate_object(NULL, (value > 0) ?
					      hotk->methods->brightness_up :
					      hotk->methods->brightness_down,
					      NULL, NULL);
		(value > 0) ? value-- : value++;
		if (ACPI_FAILURE(status))
			printk(KERN_WARNING
			       "Asus ACPI: Error changing brightness\n");
			ret = -EIO;
	}
out:
	return ret;
}

static int set_brightness_status(struct backlight_device *bd)
{
	return set_brightness(bd->props.brightness);
}

static int
proc_read_brn(char *page, char **start, off_t off, int count, int *eof,
	      void *data)
{
	return sprintf(page, "%d\n", read_brightness(NULL));
}

static int
proc_write_brn(struct file *file, const char __user *buffer,
	       unsigned long count, void *data)
{
	int rv, value;

	rv = parse_arg(buffer, count, &value);
	if (rv > 0) {
		value = (0 < value) ? ((15 < value) ? 15 : value) : 0;
		
		set_brightness(value);
	}
	return rv;
}

static void set_display(int value)
{
	
	if (!write_acpi_int(hotk->handle, hotk->methods->display_set,
			    value, NULL))
		printk(KERN_WARNING "Asus ACPI: Error setting display\n");
	return;
}


static int
proc_read_disp(char *page, char **start, off_t off, int count, int *eof,
	       void *data)
{
	int value = 0;

	if (!read_acpi_int(hotk->handle, hotk->methods->display_get, &value))
		printk(KERN_WARNING
		       "Asus ACPI: Error reading display status\n");
	value &= 0x07;	
	return sprintf(page, "%d\n", value);
}


static int
proc_write_disp(struct file *file, const char __user *buffer,
		unsigned long count, void *data)
{
	int rv, value;

	rv = parse_arg(buffer, count, &value);
	if (rv > 0)
		set_display(value);
	return rv;
}

typedef int (proc_readfunc) (char *page, char **start, off_t off, int count,
			     int *eof, void *data);
typedef int (proc_writefunc) (struct file *file, const char __user *buffer,
			      unsigned long count, void *data);

static int
asus_proc_add(char *name, proc_writefunc *writefunc,
		     proc_readfunc *readfunc, mode_t mode,
		     struct acpi_device *device)
{
	struct proc_dir_entry *proc =
	    create_proc_entry(name, mode, acpi_device_dir(device));
	if (!proc) {
		printk(KERN_WARNING "  Unable to create %s fs entry\n", name);
		return -1;
	}
	proc->write_proc = writefunc;
	proc->read_proc = readfunc;
	proc->data = acpi_driver_data(device);
	proc->uid = asus_uid;
	proc->gid = asus_gid;
	return 0;
}

static int asus_hotk_add_fs(struct acpi_device *device)
{
	struct proc_dir_entry *proc;
	mode_t mode;

	

	if ((asus_uid == 0) && (asus_gid == 0)) {
		mode = S_IFREG | S_IRUGO | S_IWUGO;
	} else {
		mode = S_IFREG | S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP;
		printk(KERN_WARNING "  asus_uid and asus_gid parameters are "
		       "deprecated, use chown and chmod instead!\n");
	}

	acpi_device_dir(device) = asus_proc_dir;
	if (!acpi_device_dir(device))
		return -ENODEV;

	proc = create_proc_entry(PROC_INFO, mode, acpi_device_dir(device));
	if (proc) {
		proc->read_proc = proc_read_info;
		proc->data = acpi_driver_data(device);
		proc->uid = asus_uid;
		proc->gid = asus_gid;
	} else {
		printk(KERN_WARNING "  Unable to create " PROC_INFO
		       " fs entry\n");
	}

	if (hotk->methods->mt_wled) {
		asus_proc_add(PROC_WLED, &proc_write_wled, &proc_read_wled,
			      mode, device);
	}

	if (hotk->methods->mt_ledd) {
		asus_proc_add(PROC_LEDD, &proc_write_ledd, &proc_read_ledd,
			      mode, device);
	}

	if (hotk->methods->mt_mled) {
		asus_proc_add(PROC_MLED, &proc_write_mled, &proc_read_mled,
			      mode, device);
	}

	if (hotk->methods->mt_tled) {
		asus_proc_add(PROC_TLED, &proc_write_tled, &proc_read_tled,
			      mode, device);
	}

	if (hotk->methods->mt_bt_switch) {
		asus_proc_add(PROC_BT, &proc_write_bluetooth,
			      &proc_read_bluetooth, mode, device);
	}

	
	if (hotk->methods->mt_lcd_switch && hotk->methods->lcd_status) {
		asus_proc_add(PROC_LCD, &proc_write_lcd, &proc_read_lcd, mode,
			      device);
	}

	if ((hotk->methods->brightness_up && hotk->methods->brightness_down) ||
	    (hotk->methods->brightness_get && hotk->methods->brightness_set)) {
		asus_proc_add(PROC_BRN, &proc_write_brn, &proc_read_brn, mode,
			      device);
	}

	if (hotk->methods->display_set) {
		asus_proc_add(PROC_DISP, &proc_write_disp, &proc_read_disp,
			      mode, device);
	}

	return 0;
}

static int asus_hotk_remove_fs(struct acpi_device *device)
{
	if (acpi_device_dir(device)) {
		remove_proc_entry(PROC_INFO, acpi_device_dir(device));
		if (hotk->methods->mt_wled)
			remove_proc_entry(PROC_WLED, acpi_device_dir(device));
		if (hotk->methods->mt_mled)
			remove_proc_entry(PROC_MLED, acpi_device_dir(device));
		if (hotk->methods->mt_tled)
			remove_proc_entry(PROC_TLED, acpi_device_dir(device));
		if (hotk->methods->mt_ledd)
			remove_proc_entry(PROC_LEDD, acpi_device_dir(device));
		if (hotk->methods->mt_bt_switch)
			remove_proc_entry(PROC_BT, acpi_device_dir(device));
		if (hotk->methods->mt_lcd_switch && hotk->methods->lcd_status)
			remove_proc_entry(PROC_LCD, acpi_device_dir(device));
		if ((hotk->methods->brightness_up
		     && hotk->methods->brightness_down)
		    || (hotk->methods->brightness_get
			&& hotk->methods->brightness_set))
			remove_proc_entry(PROC_BRN, acpi_device_dir(device));
		if (hotk->methods->display_set)
			remove_proc_entry(PROC_DISP, acpi_device_dir(device));
	}
	return 0;
}

static void asus_hotk_notify(struct acpi_device *device, u32 event)
{
	
	if (!hotk)
		return;

	
	if (event > ACPI_MAX_SYS_NOTIFY)
		return;

	if ((event & ~((u32) BR_UP)) < 16)
		hotk->brightness = (event & ~((u32) BR_UP));
	else if ((event & ~((u32) BR_DOWN)) < 16)
		hotk->brightness = (event & ~((u32) BR_DOWN));

	acpi_bus_generate_proc_event(hotk->device, event,
				hotk->event_count[event % 128]++);

	return;
}


static int asus_model_match(char *model)
{
	if (model == NULL)
		return END_MODEL;

	if (strncmp(model, "L3D", 3) == 0)
		return L3D;
	else if (strncmp(model, "L2E", 3) == 0 ||
		 strncmp(model, "L3H", 3) == 0 || strncmp(model, "L5D", 3) == 0)
		return L3H;
	else if (strncmp(model, "L3", 2) == 0 || strncmp(model, "L2B", 3) == 0)
		return L3C;
	else if (strncmp(model, "L8L", 3) == 0)
		return L8L;
	else if (strncmp(model, "L4R", 3) == 0)
		return L4R;
	else if (strncmp(model, "M6N", 3) == 0 || strncmp(model, "W3N", 3) == 0)
		return M6N;
	else if (strncmp(model, "M6R", 3) == 0 || strncmp(model, "A3G", 3) == 0)
		return M6R;
	else if (strncmp(model, "M2N", 3) == 0 ||
		 strncmp(model, "M3N", 3) == 0 ||
		 strncmp(model, "M5N", 3) == 0 ||
		 strncmp(model, "M6N", 3) == 0 ||
		 strncmp(model, "S1N", 3) == 0 ||
		 strncmp(model, "S5N", 3) == 0 || strncmp(model, "W1N", 3) == 0)
		return xxN;
	else if (strncmp(model, "M1", 2) == 0)
		return M1A;
	else if (strncmp(model, "M2", 2) == 0 || strncmp(model, "L4E", 3) == 0)
		return M2E;
	else if (strncmp(model, "L2", 2) == 0)
		return L2D;
	else if (strncmp(model, "L8", 2) == 0)
		return S1x;
	else if (strncmp(model, "D1", 2) == 0)
		return D1x;
	else if (strncmp(model, "A1", 2) == 0)
		return A1x;
	else if (strncmp(model, "A2", 2) == 0)
		return A2x;
	else if (strncmp(model, "J1", 2) == 0)
		return S2x;
	else if (strncmp(model, "L5", 2) == 0)
		return L5x;
	else if (strncmp(model, "A4G", 3) == 0)
		return A4G;
	else if (strncmp(model, "W1N", 3) == 0)
		return W1N;
	else if (strncmp(model, "W3V", 3) == 0)
		return W3V;
	else if (strncmp(model, "W5A", 3) == 0)
		return W5A;
	else if (strncmp(model, "R1F", 3) == 0)
		return R1F;
	else if (strncmp(model, "A4S", 3) == 0)
		return A4S;
	else if (strncmp(model, "F3Sa", 4) == 0)
		return F3Sa;
	else
		return END_MODEL;
}


static int asus_hotk_get_info(void)
{
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	union acpi_object *model = NULL;
	int bsts_result;
	char *string = NULL;
	acpi_status status;

	
	status = acpi_get_table(ACPI_SIG_DSDT, 1, &asus_info);
	if (ACPI_FAILURE(status))
		printk(KERN_WARNING "  Couldn't get the DSDT table header\n");

	
	if (!write_acpi_int(hotk->handle, "INIT", 0, &buffer)) {
		printk(KERN_ERR "  Hotkey initialization failed\n");
		return -ENODEV;
	}

	
	if (!read_acpi_int(hotk->handle, "BSTS", &bsts_result))
		printk(KERN_WARNING "  Error calling BSTS\n");
	else if (bsts_result)
		printk(KERN_NOTICE "  BSTS called, 0x%02x returned\n",
		       bsts_result);

	
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
			kfree(model);
			model = NULL;
			break;
		}
	}
	hotk->model = asus_model_match(string);
	if (hotk->model == END_MODEL) {	
		if (asus_info &&
		    strncmp(asus_info->oem_table_id, "ODEM", 4) == 0) {
			hotk->model = P30;
			printk(KERN_NOTICE
			       "  Samsung P30 detected, supported\n");
		} else {
			hotk->model = M2E;
			printk(KERN_NOTICE "  unsupported model %s, trying "
			       "default values\n", string);
			printk(KERN_NOTICE
			       "  send /proc/acpi/dsdt to the developers\n");
			kfree(model);
			return -ENODEV;
		}
		hotk->methods = &model_conf[hotk->model];
		return AE_OK;
	}
	hotk->methods = &model_conf[hotk->model];
	printk(KERN_NOTICE "  %s model detected, supported\n", string);

	
	if (strncmp(string, "L2B", 3) == 0)
		hotk->methods->lcd_status = NULL;
	
	else if (strncmp(string, "A3G", 3) == 0)
		hotk->methods->lcd_status = "\\BLFG";
	
	else if (strncmp(string, "S5N", 3) == 0 ||
		 strncmp(string, "M5N", 3) == 0 ||
		 strncmp(string, "W3N", 3) == 0)
		hotk->methods->mt_mled = NULL;
	
	else if (strncmp(string, "L5D", 3) == 0)
		hotk->methods->mt_wled = NULL;
	
	else if (strncmp(string, "M2N", 3) == 0 ||
		 strncmp(string, "W3V", 3) == 0 ||
		 strncmp(string, "S1N", 3) == 0)
		hotk->methods->mt_wled = "WLED";
	
	else if (asus_info) {
		if (strncmp(asus_info->oem_table_id, "L1", 2) == 0)
			hotk->methods->mled_status = NULL;
		
	}

	kfree(model);

	return AE_OK;
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
		printk(KERN_ERR "  Hotkey device not present, aborting\n");
		return -EINVAL;
	}

	return result;
}

static int asus_hotk_found;

static int asus_hotk_add(struct acpi_device *device)
{
	acpi_status status = AE_OK;
	int result;

	if (!device)
		return -EINVAL;

	printk(KERN_NOTICE "Asus Laptop ACPI Extras version %s\n",
	       ASUS_ACPI_VERSION);

	hotk = kzalloc(sizeof(struct asus_hotk), GFP_KERNEL);
	if (!hotk)
		return -ENOMEM;

	hotk->handle = device->handle;
	strcpy(acpi_device_name(device), ACPI_HOTK_DEVICE_NAME);
	strcpy(acpi_device_class(device), ACPI_HOTK_CLASS);
	device->driver_data = hotk;
	hotk->device = device;

	result = asus_hotk_check();
	if (result)
		goto end;

	result = asus_hotk_add_fs(device);
	if (result)
		goto end;

	
	if ((!hotk->methods->brightness_get)
	    && (!hotk->methods->brightness_status)
	    && (hotk->methods->brightness_up && hotk->methods->brightness_down)) {
		status =
		    acpi_evaluate_object(NULL, hotk->methods->brightness_down,
					 NULL, NULL);
		if (ACPI_FAILURE(status))
			printk(KERN_WARNING "  Error changing brightness\n");
		else {
			status =
			    acpi_evaluate_object(NULL,
						 hotk->methods->brightness_up,
						 NULL, NULL);
			if (ACPI_FAILURE(status))
				printk(KERN_WARNING "  Strange, error changing"
				       " brightness\n");
		}
	}

	asus_hotk_found = 1;

	
	hotk->ledd_status = 0xFFF;

end:
	if (result)
		kfree(hotk);

	return result;
}

static int asus_hotk_remove(struct acpi_device *device, int type)
{
	if (!device || !acpi_driver_data(device))
		return -EINVAL;

	asus_hotk_remove_fs(device);

	kfree(hotk);

	return 0;
}

static struct backlight_ops asus_backlight_data = {
	.get_brightness = read_brightness,
	.update_status  = set_brightness_status,
};

static void asus_acpi_exit(void)
{
	if (asus_backlight_device)
		backlight_device_unregister(asus_backlight_device);

	acpi_bus_unregister_driver(&asus_hotk_driver);
	remove_proc_entry(PROC_ASUS, acpi_root_dir);

	return;
}

static int __init asus_acpi_init(void)
{
	int result;

	if (acpi_disabled)
		return -ENODEV;

	asus_proc_dir = proc_mkdir(PROC_ASUS, acpi_root_dir);
	if (!asus_proc_dir) {
		printk(KERN_ERR "Asus ACPI: Unable to create /proc entry\n");
		return -ENODEV;
	}

	result = acpi_bus_register_driver(&asus_hotk_driver);
	if (result < 0) {
		remove_proc_entry(PROC_ASUS, acpi_root_dir);
		return result;
	}

	
	if (!asus_hotk_found) {
		acpi_bus_unregister_driver(&asus_hotk_driver);
		remove_proc_entry(PROC_ASUS, acpi_root_dir);
		return -ENODEV;
	}

	asus_backlight_device = backlight_device_register("asus", NULL, NULL,
							  &asus_backlight_data);
	if (IS_ERR(asus_backlight_device)) {
		printk(KERN_ERR "Could not register asus backlight device\n");
		asus_backlight_device = NULL;
		asus_acpi_exit();
		return -ENODEV;
	}
	asus_backlight_device->props.max_brightness = 15;

	return 0;
}

module_init(asus_acpi_init);
module_exit(asus_acpi_exit);
