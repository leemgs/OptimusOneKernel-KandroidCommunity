



#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>

#include "hid-ids.h"

#define NTRIG_DUPLICATE_USAGES	0x001

#define nt_map_key_clear(c)	hid_map_usage_clear(hi, usage, bit, max, \
					EV_KEY, (c))

struct ntrig_data {
	__s32 x, y, id, w, h;
	char reading_a_point, found_contact_id;
	char pen_active;
	char finger_active;
	char inverted;
};



static int ntrig_input_mapping(struct hid_device *hdev, struct hid_input *hi,
		struct hid_field *field, struct hid_usage *usage,
		unsigned long **bit, int *max)
{
	switch (usage->hid & HID_USAGE_PAGE) {

	case HID_UP_GENDESK:
		switch (usage->hid) {
		case HID_GD_X:
			hid_map_usage(hi, usage, bit, max,
					EV_ABS, ABS_MT_POSITION_X);
			input_set_abs_params(hi->input, ABS_X,
					field->logical_minimum,
					field->logical_maximum, 0, 0);
			return 1;
		case HID_GD_Y:
			hid_map_usage(hi, usage, bit, max,
					EV_ABS, ABS_MT_POSITION_Y);
			input_set_abs_params(hi->input, ABS_Y,
					field->logical_minimum,
					field->logical_maximum, 0, 0);
			return 1;
		}
		return 0;

	case HID_UP_DIGITIZER:
		switch (usage->hid) {
		
		case HID_DG_CONTACTID: 
		case HID_DG_INPUTMODE:
		case HID_DG_DEVICEINDEX:
		case HID_DG_CONTACTCOUNT:
		case HID_DG_CONTACTMAX:
			return -1;

		
		case HID_DG_CONFIDENCE:
			nt_map_key_clear(BTN_TOOL_DOUBLETAP);
			return 1;

		
		case HID_DG_WIDTH:
			hid_map_usage(hi, usage, bit, max,
					EV_ABS, ABS_MT_TOUCH_MAJOR);
			return 1;
		case HID_DG_HEIGHT:
			hid_map_usage(hi, usage, bit, max,
					EV_ABS, ABS_MT_TOUCH_MINOR);
			input_set_abs_params(hi->input, ABS_MT_ORIENTATION,
					0, 1, 0, 0);
			return 1;
		}
		return 0;

	case 0xff000000:
		
		return -1;
	}

	return 0;
}

static int ntrig_input_mapped(struct hid_device *hdev, struct hid_input *hi,
		struct hid_field *field, struct hid_usage *usage,
		unsigned long **bit, int *max)
{
	if (usage->type == EV_KEY || usage->type == EV_REL
			|| usage->type == EV_ABS)
		clear_bit(usage->code, *bit);

	return 0;
}


static int ntrig_event (struct hid_device *hid, struct hid_field *field,
		                        struct hid_usage *usage, __s32 value)
{
	struct input_dev *input = field->hidinput->input;
	struct ntrig_data *nd = hid_get_drvdata(hid);

        if (hid->claimed & HID_CLAIMED_INPUT) {
		switch (usage->hid) {

		case HID_DG_INRANGE:
			if (field->application & 0x3)
				nd->pen_active = (value != 0);
			else
				nd->finger_active = (value != 0);
			return 0;

		case HID_DG_INVERT:
			nd->inverted = value;
			return 0;

		case HID_GD_X:
			nd->x = value;
			nd->reading_a_point = 1;
			break;
		case HID_GD_Y:
			nd->y = value;
			break;
		case HID_DG_CONTACTID:
			nd->id = value;
			
			nd->found_contact_id = 1;
			break;
		case HID_DG_WIDTH:
			nd->w = value;
			break;
		case HID_DG_HEIGHT:
			nd->h = value;
			
			if (!nd->found_contact_id) {
				if (nd->pen_active && nd->finger_active) {
					input_report_key(input, BTN_TOOL_DOUBLETAP, 0);
					input_report_key(input, BTN_TOOL_DOUBLETAP, 1);
				}
				input_event(input, EV_ABS, ABS_X, nd->x);
				input_event(input, EV_ABS, ABS_Y, nd->y);
			}
			break;
		case HID_DG_TIPPRESSURE:
			
			if (! nd->found_contact_id) {
				if (nd->pen_active && nd->finger_active) {
					input_report_key(input,
							nd->inverted ? BTN_TOOL_RUBBER : BTN_TOOL_PEN
							, 0);
					input_report_key(input,
							nd->inverted ? BTN_TOOL_RUBBER : BTN_TOOL_PEN
							, 1);
				}
				input_event(input, EV_ABS, ABS_X, nd->x);
				input_event(input, EV_ABS, ABS_Y, nd->y);
				input_event(input, EV_ABS, ABS_PRESSURE, value);
			}
			break;
		case 0xff000002:
			
			if (!nd->reading_a_point || value != 1)
				break;
			
			if (nd->id == 0) {
				input_event(input, EV_ABS, ABS_X, nd->x);
				input_event(input, EV_ABS, ABS_Y, nd->y);
			}
			input_event(input, EV_ABS, ABS_MT_POSITION_X, nd->x);
			input_event(input, EV_ABS, ABS_MT_POSITION_Y, nd->y);
			if (nd->w > nd->h) {
				input_event(input, EV_ABS,
						ABS_MT_ORIENTATION, 1);
				input_event(input, EV_ABS,
						ABS_MT_TOUCH_MAJOR, nd->w);
				input_event(input, EV_ABS,
						ABS_MT_TOUCH_MINOR, nd->h);
			} else {
				input_event(input, EV_ABS,
						ABS_MT_ORIENTATION, 0);
				input_event(input, EV_ABS,
						ABS_MT_TOUCH_MAJOR, nd->h);
				input_event(input, EV_ABS,
						ABS_MT_TOUCH_MINOR, nd->w);
			}
			input_mt_sync(field->hidinput->input);
			nd->reading_a_point = 0;
			nd->found_contact_id = 0;
			break;

		default:
			
			return 0;
		}
	}

	
        if (hid->claimed & HID_CLAIMED_HIDDEV && hid->hiddev_hid_event)
                hid->hiddev_hid_event(hid, field, usage, value);

	return 1;
}

static int ntrig_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret;
	struct ntrig_data *nd;

	nd = kmalloc(sizeof(struct ntrig_data), GFP_KERNEL);
	if (!nd) {
		dev_err(&hdev->dev, "cannot allocate N-Trig data\n");
		return -ENOMEM;
	}
	nd->reading_a_point = 0;
	nd->found_contact_id = 0;
	hid_set_drvdata(hdev, nd);

	ret = hid_parse(hdev);
	if (!ret)
		ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);

	if (ret)
		kfree (nd);

	return ret;
}

static void ntrig_remove(struct hid_device *hdev)
{
	hid_hw_stop(hdev);
	kfree(hid_get_drvdata(hdev));
}

static const struct hid_device_id ntrig_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_NTRIG, USB_DEVICE_ID_NTRIG_TOUCH_SCREEN),
		.driver_data = NTRIG_DUPLICATE_USAGES },
	{ }
};
MODULE_DEVICE_TABLE(hid, ntrig_devices);

static const struct hid_usage_id ntrig_grabbed_usages[] = {
	{ HID_ANY_ID, HID_ANY_ID, HID_ANY_ID },
	{ HID_ANY_ID - 1, HID_ANY_ID - 1, HID_ANY_ID - 1}
};

static struct hid_driver ntrig_driver = {
	.name = "ntrig",
	.id_table = ntrig_devices,
	.probe = ntrig_probe,
	.remove = ntrig_remove,
	.input_mapping = ntrig_input_mapping,
	.input_mapped = ntrig_input_mapped,
	.usage_table = ntrig_grabbed_usages,
	.event = ntrig_event,
};

static int __init ntrig_init(void)
{
	return hid_register_driver(&ntrig_driver);
}

static void __exit ntrig_exit(void)
{
	hid_unregister_driver(&ntrig_driver);
}

module_init(ntrig_init);
module_exit(ntrig_exit);
MODULE_LICENSE("GPL");
