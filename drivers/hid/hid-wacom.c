



#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>

#include "hid-ids.h"

struct wacom_data {
	__u16 tool;
	unsigned char butstate;
};

static int wacom_raw_event(struct hid_device *hdev, struct hid_report *report,
		u8 *raw_data, int size)
{
	struct wacom_data *wdata = hid_get_drvdata(hdev);
	struct hid_input *hidinput;
	struct input_dev *input;
	unsigned char *data = (unsigned char *) raw_data;
	int tool, x, y, rw;

	if (!(hdev->claimed & HID_CLAIMED_INPUT))
		return 0;

	tool = 0;
	hidinput = list_entry(hdev->inputs.next, struct hid_input, list);
	input = hidinput->input;

	
	if (data[0] != 0x03)
		return 0;

	
	x = le16_to_cpu(*(__le16 *) &data[2]);
	y = le16_to_cpu(*(__le16 *) &data[4]);

	
	if (data[1] & 0x90) { 
		switch ((data[1] >> 5) & 3) {
		case 0:	
			tool = BTN_TOOL_PEN;
			break;

		case 1: 
			tool = BTN_TOOL_RUBBER;
			break;

		case 2: 
		case 3: 
			tool = BTN_TOOL_MOUSE;
			break;
		}

		
		if (!(data[1] & 0x10))
			tool = 0;
	}

	
	if (wdata->tool != tool) {
		if (wdata->tool) {
			
			if (wdata->tool == BTN_TOOL_MOUSE) {
				input_report_key(input, BTN_LEFT, 0);
				input_report_key(input, BTN_RIGHT, 0);
				input_report_key(input, BTN_MIDDLE, 0);
				input_report_abs(input, ABS_DISTANCE,
						input->absmax[ABS_DISTANCE]);
			} else {
				input_report_key(input, BTN_TOUCH, 0);
				input_report_key(input, BTN_STYLUS, 0);
				input_report_key(input, BTN_STYLUS2, 0);
				input_report_abs(input, ABS_PRESSURE, 0);
			}
			input_report_key(input, wdata->tool, 0);
			input_sync(input);
		}
		wdata->tool = tool;
		if (tool)
			input_report_key(input, tool, 1);
	}

	if (tool) {
		input_report_abs(input, ABS_X, x);
		input_report_abs(input, ABS_Y, y);

		switch ((data[1] >> 5) & 3) {
		case 2: 
			input_report_key(input, BTN_MIDDLE, data[1] & 0x04);
			rw = (data[6] & 0x01) ? -1 :
				(data[6] & 0x02) ? 1 : 0;
			input_report_rel(input, REL_WHEEL, rw);
			

		case 3: 
			input_report_key(input, BTN_LEFT, data[1] & 0x01);
			input_report_key(input, BTN_RIGHT, data[1] & 0x02);
			
			rw = 44 - (data[6] >> 2);
			if (rw < 0)
				rw = 0;
			else if (rw > 31)
				rw = 31;
			input_report_abs(input, ABS_DISTANCE, rw);
			break;

		default:
			input_report_abs(input, ABS_PRESSURE,
					data[6] | (((__u16) (data[1] & 0x08)) << 5));
			input_report_key(input, BTN_TOUCH, data[1] & 0x01);
			input_report_key(input, BTN_STYLUS, data[1] & 0x02);
			input_report_key(input, BTN_STYLUS2, (tool == BTN_TOOL_PEN) && data[1] & 0x04);
			break;
		}

		input_sync(input);
	}

	
	rw = data[7] & 0x03;
	if (rw != wdata->butstate) {
		wdata->butstate = rw;
		input_report_key(input, BTN_0, rw & 0x02);
		input_report_key(input, BTN_1, rw & 0x01);
		input_event(input, EV_MSC, MSC_SERIAL, 0xf0);
		input_sync(input);
	}

	return 1;
}

static int wacom_probe(struct hid_device *hdev,
		const struct hid_device_id *id)
{
	struct hid_input *hidinput;
	struct input_dev *input;
	struct wacom_data *wdata;
	int ret;

	wdata = kzalloc(sizeof(*wdata), GFP_KERNEL);
	if (wdata == NULL) {
		dev_err(&hdev->dev, "can't alloc wacom descriptor\n");
		return -ENOMEM;
	}

	hid_set_drvdata(hdev, wdata);

	ret = hid_parse(hdev);
	if (ret) {
		dev_err(&hdev->dev, "parse failed\n");
		goto err_free;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		dev_err(&hdev->dev, "hw start failed\n");
		goto err_free;
	}

	hidinput = list_entry(hdev->inputs.next, struct hid_input, list);
	input = hidinput->input;

	
	input->evbit[0] |= BIT(EV_KEY) | BIT(EV_ABS) | BIT(EV_REL);
	input->absbit[0] |= BIT(ABS_X) | BIT(ABS_Y) |
		BIT(ABS_PRESSURE) | BIT(ABS_DISTANCE);
	input->relbit[0] |= BIT(REL_WHEEL);
	set_bit(BTN_TOOL_PEN, input->keybit);
	set_bit(BTN_TOUCH, input->keybit);
	set_bit(BTN_STYLUS, input->keybit);
	set_bit(BTN_STYLUS2, input->keybit);
	set_bit(BTN_LEFT, input->keybit);
	set_bit(BTN_RIGHT, input->keybit);
	set_bit(BTN_MIDDLE, input->keybit);

	
	input->evbit[0] |= BIT(EV_MSC);
	input->mscbit[0] |= BIT(MSC_SERIAL);

	
	input->absbit[0] |= BIT(ABS_DISTANCE);
	set_bit(BTN_TOOL_RUBBER, input->keybit);
	set_bit(BTN_TOOL_MOUSE, input->keybit);

	input->absmax[ABS_PRESSURE] = 511;
	input->absmax[ABS_DISTANCE] = 32;

	input->absmax[ABS_X] = 16704;
	input->absmax[ABS_Y] = 12064;
	input->absfuzz[ABS_X] = 4;
	input->absfuzz[ABS_Y] = 4;

	return 0;
err_free:
	kfree(wdata);
	return ret;
}

static void wacom_remove(struct hid_device *hdev)
{
	hid_hw_stop(hdev);
	kfree(hid_get_drvdata(hdev));
}

static const struct hid_device_id wacom_devices[] = {
	{ HID_BLUETOOTH_DEVICE(USB_VENDOR_ID_WACOM, USB_DEVICE_ID_WACOM_GRAPHIRE_BLUETOOTH) },

	{ }
};
MODULE_DEVICE_TABLE(hid, wacom_devices);

static struct hid_driver wacom_driver = {
	.name = "wacom",
	.id_table = wacom_devices,
	.probe = wacom_probe,
	.remove = wacom_remove,
	.raw_event = wacom_raw_event,
};

static int __init wacom_init(void)
{
	int ret;

	ret = hid_register_driver(&wacom_driver);
	if (ret)
		printk(KERN_ERR "can't register wacom driver\n");
	printk(KERN_ERR "wacom driver registered\n");
	return ret;
}

static void __exit wacom_exit(void)
{
	hid_unregister_driver(&wacom_driver);
}

module_init(wacom_init);
module_exit(wacom_exit);
MODULE_LICENSE("GPL");

