

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/backlight.h>
#include <linux/ctype.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <acpi/acpi_bus.h>
#include <acpi/acpi_drivers.h>
#include <linux/input.h>


#ifndef ACPI_HOTKEY_COMPONENT
#define ACPI_HOTKEY_COMPONENT	0x10000000
#endif

#define _COMPONENT		ACPI_HOTKEY_COMPONENT

MODULE_AUTHOR("Hiroshi Miura, David Bronaugh and Harald Welte");
MODULE_DESCRIPTION("ACPI HotKey driver for Panasonic Let's Note laptops");
MODULE_LICENSE("GPL");

#define LOGPREFIX "pcc_acpi: "



#define METHOD_HKEY_QUERY	"HINF"
#define METHOD_HKEY_SQTY	"SQTY"
#define METHOD_HKEY_SINF	"SINF"
#define METHOD_HKEY_SSET	"SSET"
#define HKEY_NOTIFY		 0x80

#define ACPI_PCC_DRIVER_NAME	"Panasonic Laptop Support"
#define ACPI_PCC_DEVICE_NAME	"Hotkey"
#define ACPI_PCC_CLASS		"pcc"

#define ACPI_PCC_INPUT_PHYS	"panasonic/hkey0"


enum SINF_BITS { SINF_NUM_BATTERIES = 0,
		 SINF_LCD_TYPE,
		 SINF_AC_MAX_BRIGHT,
		 SINF_AC_MIN_BRIGHT,
		 SINF_AC_CUR_BRIGHT,
		 SINF_DC_MAX_BRIGHT,
		 SINF_DC_MIN_BRIGHT,
		 SINF_DC_CUR_BRIGHT,
		 SINF_MUTE,
		 SINF_RESERVED,
		 SINF_ENV_STATE,
		 SINF_STICKY_KEY = 0x80,
	};


static int acpi_pcc_hotkey_add(struct acpi_device *device);
static int acpi_pcc_hotkey_remove(struct acpi_device *device, int type);
static int acpi_pcc_hotkey_resume(struct acpi_device *device);
static void acpi_pcc_hotkey_notify(struct acpi_device *device, u32 event);

static const struct acpi_device_id pcc_device_ids[] = {
	{ "MAT0012", 0},
	{ "MAT0013", 0},
	{ "MAT0018", 0},
	{ "MAT0019", 0},
	{ "", 0},
};
MODULE_DEVICE_TABLE(acpi, pcc_device_ids);

static struct acpi_driver acpi_pcc_driver = {
	.name =		ACPI_PCC_DRIVER_NAME,
	.class =	ACPI_PCC_CLASS,
	.ids =		pcc_device_ids,
	.ops =		{
				.add =		acpi_pcc_hotkey_add,
				.remove =	acpi_pcc_hotkey_remove,
				.resume =       acpi_pcc_hotkey_resume,
				.notify =	acpi_pcc_hotkey_notify,
			},
};

#define KEYMAP_SIZE		11
static const int initial_keymap[KEYMAP_SIZE] = {
	 KEY_RESERVED,
	 KEY_BRIGHTNESSDOWN,
	 KEY_BRIGHTNESSUP,
	 KEY_DISPLAYTOGGLE,
	 KEY_MUTE,
	 KEY_VOLUMEDOWN,
	 KEY_VOLUMEUP,
	 KEY_SLEEP,
	 KEY_PROG1, 
	 KEY_BATTERY,
	 KEY_SUSPEND,
};

struct pcc_acpi {
	acpi_handle		handle;
	unsigned long		num_sifr;
	int			sticky_mode;
	u32 			*sinf;
	struct acpi_device	*device;
	struct input_dev	*input_dev;
	struct backlight_device	*backlight;
	int			keymap[KEYMAP_SIZE];
};

struct pcc_keyinput {
	struct acpi_hotkey      *hotkey;
};


static int acpi_pcc_write_sset(struct pcc_acpi *pcc, int func, int val)
{
	union acpi_object in_objs[] = {
		{ .integer.type  = ACPI_TYPE_INTEGER,
		  .integer.value = func, },
		{ .integer.type  = ACPI_TYPE_INTEGER,
		  .integer.value = val, },
	};
	struct acpi_object_list params = {
		.count   = ARRAY_SIZE(in_objs),
		.pointer = in_objs,
	};
	acpi_status status = AE_OK;

	status = acpi_evaluate_object(pcc->handle, METHOD_HKEY_SSET,
				      &params, NULL);

	return status == AE_OK;
}

static inline int acpi_pcc_get_sqty(struct acpi_device *device)
{
	unsigned long long s;
	acpi_status status;

	status = acpi_evaluate_integer(device->handle, METHOD_HKEY_SQTY,
				       NULL, &s);
	if (ACPI_SUCCESS(status))
		return s;
	else {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
				  "evaluation error HKEY.SQTY\n"));
		return -EINVAL;
	}
}

static int acpi_pcc_retrieve_biosdata(struct pcc_acpi *pcc, u32 *sinf)
{
	acpi_status status;
	struct acpi_buffer buffer = {ACPI_ALLOCATE_BUFFER, NULL};
	union acpi_object *hkey = NULL;
	int i;

	status = acpi_evaluate_object(pcc->handle, METHOD_HKEY_SINF, NULL,
				      &buffer);
	if (ACPI_FAILURE(status)) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
				  "evaluation error HKEY.SINF\n"));
		return 0;
	}

	hkey = buffer.pointer;
	if (!hkey || (hkey->type != ACPI_TYPE_PACKAGE)) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR, "Invalid HKEY.SINF\n"));
		goto end;
	}

	if (pcc->num_sifr < hkey->package.count) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
				 "SQTY reports bad SINF length\n"));
		status = AE_ERROR;
		goto end;
	}

	for (i = 0; i < hkey->package.count; i++) {
		union acpi_object *element = &(hkey->package.elements[i]);
		if (likely(element->type == ACPI_TYPE_INTEGER)) {
			sinf[i] = element->integer.value;
		} else
			ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
					 "Invalid HKEY.SINF data\n"));
	}
	sinf[hkey->package.count] = -1;

end:
	kfree(buffer.pointer);
	return status == AE_OK;
}





static int bl_get(struct backlight_device *bd)
{
	struct pcc_acpi *pcc = bl_get_data(bd);

	if (!acpi_pcc_retrieve_biosdata(pcc, pcc->sinf))
		return -EIO;

	return pcc->sinf[SINF_AC_CUR_BRIGHT];
}

static int bl_set_status(struct backlight_device *bd)
{
	struct pcc_acpi *pcc = bl_get_data(bd);
	int bright = bd->props.brightness;
	int rc;

	if (!acpi_pcc_retrieve_biosdata(pcc, pcc->sinf))
		return -EIO;

	if (bright < pcc->sinf[SINF_AC_MIN_BRIGHT])
		bright = pcc->sinf[SINF_AC_MIN_BRIGHT];

	if (bright < pcc->sinf[SINF_DC_MIN_BRIGHT])
		bright = pcc->sinf[SINF_DC_MIN_BRIGHT];

	if (bright < pcc->sinf[SINF_AC_MIN_BRIGHT] ||
	    bright > pcc->sinf[SINF_AC_MAX_BRIGHT])
		return -EINVAL;

	rc = acpi_pcc_write_sset(pcc, SINF_AC_CUR_BRIGHT, bright);
	if (rc < 0)
		return rc;

	return acpi_pcc_write_sset(pcc, SINF_DC_CUR_BRIGHT, bright);
}

static struct backlight_ops pcc_backlight_ops = {
	.get_brightness	= bl_get,
	.update_status	= bl_set_status,
};




static ssize_t show_numbatt(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct acpi_device *acpi = to_acpi_device(dev);
	struct pcc_acpi *pcc = acpi_driver_data(acpi);

	if (!acpi_pcc_retrieve_biosdata(pcc, pcc->sinf))
		return -EIO;

	return snprintf(buf, PAGE_SIZE, "%u\n", pcc->sinf[SINF_NUM_BATTERIES]);
}

static ssize_t show_lcdtype(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct acpi_device *acpi = to_acpi_device(dev);
	struct pcc_acpi *pcc = acpi_driver_data(acpi);

	if (!acpi_pcc_retrieve_biosdata(pcc, pcc->sinf))
		return -EIO;

	return snprintf(buf, PAGE_SIZE, "%u\n", pcc->sinf[SINF_LCD_TYPE]);
}

static ssize_t show_mute(struct device *dev, struct device_attribute *attr,
			 char *buf)
{
	struct acpi_device *acpi = to_acpi_device(dev);
	struct pcc_acpi *pcc = acpi_driver_data(acpi);

	if (!acpi_pcc_retrieve_biosdata(pcc, pcc->sinf))
		return -EIO;

	return snprintf(buf, PAGE_SIZE, "%u\n", pcc->sinf[SINF_MUTE]);
}

static ssize_t show_sticky(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct acpi_device *acpi = to_acpi_device(dev);
	struct pcc_acpi *pcc = acpi_driver_data(acpi);

	if (!acpi_pcc_retrieve_biosdata(pcc, pcc->sinf))
		return -EIO;

	return snprintf(buf, PAGE_SIZE, "%u\n", pcc->sinf[SINF_STICKY_KEY]);
}

static ssize_t set_sticky(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct acpi_device *acpi = to_acpi_device(dev);
	struct pcc_acpi *pcc = acpi_driver_data(acpi);
	int val;

	if (count && sscanf(buf, "%i", &val) == 1 &&
	    (val == 0 || val == 1)) {
		acpi_pcc_write_sset(pcc, SINF_STICKY_KEY, val);
		pcc->sticky_mode = val;
	}

	return count;
}

static DEVICE_ATTR(numbatt, S_IRUGO, show_numbatt, NULL);
static DEVICE_ATTR(lcdtype, S_IRUGO, show_lcdtype, NULL);
static DEVICE_ATTR(mute, S_IRUGO, show_mute, NULL);
static DEVICE_ATTR(sticky_key, S_IRUGO | S_IWUSR, show_sticky, set_sticky);

static struct attribute *pcc_sysfs_entries[] = {
	&dev_attr_numbatt.attr,
	&dev_attr_lcdtype.attr,
	&dev_attr_mute.attr,
	&dev_attr_sticky_key.attr,
	NULL,
};

static struct attribute_group pcc_attr_group = {
	.name	= NULL,		
	.attrs	= pcc_sysfs_entries,
};




static int pcc_getkeycode(struct input_dev *dev, int scancode, int *keycode)
{
	struct pcc_acpi *pcc = input_get_drvdata(dev);

	if (scancode >= ARRAY_SIZE(pcc->keymap))
		return -EINVAL;

	*keycode = pcc->keymap[scancode];

	return 0;
}

static int keymap_get_by_keycode(struct pcc_acpi *pcc, int keycode)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(pcc->keymap); i++) {
		if (pcc->keymap[i] == keycode)
			return i+1;
	}

	return 0;
}

static int pcc_setkeycode(struct input_dev *dev, int scancode, int keycode)
{
	struct pcc_acpi *pcc = input_get_drvdata(dev);
	int oldkeycode;

	if (scancode >= ARRAY_SIZE(pcc->keymap))
		return -EINVAL;

	if (keycode < 0 || keycode > KEY_MAX)
		return -EINVAL;

	oldkeycode = pcc->keymap[scancode];
	pcc->keymap[scancode] = keycode;

	set_bit(keycode, dev->keybit);

	if (!keymap_get_by_keycode(pcc, oldkeycode))
		clear_bit(oldkeycode, dev->keybit);

	return 0;
}

static void acpi_pcc_generate_keyinput(struct pcc_acpi *pcc)
{
	struct input_dev *hotk_input_dev = pcc->input_dev;
	int rc;
	int key_code, hkey_num;
	unsigned long long result;

	rc = acpi_evaluate_integer(pcc->handle, METHOD_HKEY_QUERY,
				   NULL, &result);
	if (!ACPI_SUCCESS(rc)) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
				 "error getting hotkey status\n"));
		return;
	}

	acpi_bus_generate_proc_event(pcc->device, HKEY_NOTIFY, result);

	hkey_num = result & 0xf;

	if (hkey_num < 0 || hkey_num >= ARRAY_SIZE(pcc->keymap)) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
				  "hotkey number out of range: %d\n",
				  hkey_num));
		return;
	}

	key_code = pcc->keymap[hkey_num];

	if (key_code != KEY_RESERVED) {
		int pushed = (result & 0x80) ? TRUE : FALSE;

		input_report_key(hotk_input_dev, key_code, pushed);
		input_sync(hotk_input_dev);
	}

	return;
}

static void acpi_pcc_hotkey_notify(struct acpi_device *device, u32 event)
{
	struct pcc_acpi *pcc = acpi_driver_data(device);

	switch (event) {
	case HKEY_NOTIFY:
		acpi_pcc_generate_keyinput(pcc);
		break;
	default:
		
		break;
	}
}

static int acpi_pcc_init_input(struct pcc_acpi *pcc)
{
	int i, rc;

	pcc->input_dev = input_allocate_device();
	if (!pcc->input_dev) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
				  "Couldn't allocate input device for hotkey"));
		return -ENOMEM;
	}

	pcc->input_dev->evbit[0] = BIT(EV_KEY);

	pcc->input_dev->name = ACPI_PCC_DRIVER_NAME;
	pcc->input_dev->phys = ACPI_PCC_INPUT_PHYS;
	pcc->input_dev->id.bustype = BUS_HOST;
	pcc->input_dev->id.vendor = 0x0001;
	pcc->input_dev->id.product = 0x0001;
	pcc->input_dev->id.version = 0x0100;
	pcc->input_dev->getkeycode = pcc_getkeycode;
	pcc->input_dev->setkeycode = pcc_setkeycode;

	
	memcpy(pcc->keymap, initial_keymap, sizeof(pcc->keymap));

	for (i = 0; i < ARRAY_SIZE(pcc->keymap); i++)
		__set_bit(pcc->keymap[i], pcc->input_dev->keybit);
	__clear_bit(KEY_RESERVED, pcc->input_dev->keybit);

	input_set_drvdata(pcc->input_dev, pcc);

	rc = input_register_device(pcc->input_dev);
	if (rc < 0)
		input_free_device(pcc->input_dev);

	return rc;
}



static int acpi_pcc_hotkey_resume(struct acpi_device *device)
{
	struct pcc_acpi *pcc = acpi_driver_data(device);
	acpi_status status = AE_OK;

	if (device == NULL || pcc == NULL)
		return -EINVAL;

	ACPI_DEBUG_PRINT((ACPI_DB_ERROR, "Sticky mode restore: %d\n",
			  pcc->sticky_mode));

	status = acpi_pcc_write_sset(pcc, SINF_STICKY_KEY, pcc->sticky_mode);

	return status == AE_OK ? 0 : -EINVAL;
}

static int acpi_pcc_hotkey_add(struct acpi_device *device)
{
	struct pcc_acpi *pcc;
	int num_sifr, result;

	if (!device)
		return -EINVAL;

	num_sifr = acpi_pcc_get_sqty(device);

	if (num_sifr > 255) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR, "num_sifr too large"));
		return -ENODEV;
	}

	pcc = kzalloc(sizeof(struct pcc_acpi), GFP_KERNEL);
	if (!pcc) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
				  "Couldn't allocate mem for pcc"));
		return -ENOMEM;
	}

	pcc->sinf = kzalloc(sizeof(u32) * (num_sifr + 1), GFP_KERNEL);
	if (!pcc->sinf) {
		result = -ENOMEM;
		goto out_hotkey;
	}

	pcc->device = device;
	pcc->handle = device->handle;
	pcc->num_sifr = num_sifr;
	device->driver_data = pcc;
	strcpy(acpi_device_name(device), ACPI_PCC_DEVICE_NAME);
	strcpy(acpi_device_class(device), ACPI_PCC_CLASS);

	result = acpi_pcc_init_input(pcc);
	if (result) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
				  "Error installing keyinput handler\n"));
		goto out_sinf;
	}

	
	pcc->backlight = backlight_device_register("panasonic", NULL, pcc,
						   &pcc_backlight_ops);
	if (IS_ERR(pcc->backlight))
		goto out_input;

	if (!acpi_pcc_retrieve_biosdata(pcc, pcc->sinf)) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
				 "Couldn't retrieve BIOS data\n"));
		goto out_backlight;
	}

	
	pcc->backlight->props.max_brightness =
					pcc->sinf[SINF_AC_MAX_BRIGHT];
	pcc->backlight->props.brightness = pcc->sinf[SINF_AC_CUR_BRIGHT];

	
	pcc->sticky_mode = pcc->sinf[SINF_STICKY_KEY];

	
	result = sysfs_create_group(&device->dev.kobj, &pcc_attr_group);
	if (result)
		goto out_backlight;

	return 0;

out_backlight:
	backlight_device_unregister(pcc->backlight);
out_input:
	input_unregister_device(pcc->input_dev);
	
out_sinf:
	kfree(pcc->sinf);
out_hotkey:
	kfree(pcc);

	return result;
}

static int __init acpi_pcc_init(void)
{
	int result = 0;

	if (acpi_disabled)
		return -ENODEV;

	result = acpi_bus_register_driver(&acpi_pcc_driver);
	if (result < 0) {
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR,
				  "Error registering hotkey driver\n"));
		return -ENODEV;
	}

	return 0;
}

static int acpi_pcc_hotkey_remove(struct acpi_device *device, int type)
{
	struct pcc_acpi *pcc = acpi_driver_data(device);

	if (!device || !pcc)
		return -EINVAL;

	sysfs_remove_group(&device->dev.kobj, &pcc_attr_group);

	backlight_device_unregister(pcc->backlight);

	input_unregister_device(pcc->input_dev);
	

	kfree(pcc->sinf);
	kfree(pcc);

	return 0;
}

static void __exit acpi_pcc_exit(void)
{
	acpi_bus_unregister_driver(&acpi_pcc_driver);
}

module_init(acpi_pcc_init);
module_exit(acpi_pcc_exit);
