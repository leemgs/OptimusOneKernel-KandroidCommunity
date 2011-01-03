



#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>

#include "hid-ids.h"


static void samsung_report_fixup(struct hid_device *hdev, __u8 *rdesc,
		unsigned int rsize)
{
	if (rsize == 184 && rdesc[175] == 0x25 && rdesc[176] == 0x40 &&
			rdesc[177] == 0x75 && rdesc[178] == 0x30 &&
			rdesc[179] == 0x95 && rdesc[180] == 0x01 &&
			rdesc[182] == 0x40) {
		dev_info(&hdev->dev, "fixing up Samsung IrDA %d byte report "
				"descriptor\n", 184);
		rdesc[176] = 0xff;
		rdesc[178] = 0x08;
		rdesc[180] = 0x06;
		rdesc[182] = 0x42;
	} else
	if (rsize == 203 && rdesc[192] == 0x15 && rdesc[193] == 0x0 &&
			rdesc[194] == 0x25 && rdesc[195] == 0x12) {
		dev_info(&hdev->dev, "fixing up Samsung IrDA %d byte report "
				"descriptor\n", 203);
		rdesc[193] = 0x1;
		rdesc[195] = 0xf;
	} else
	if (rsize == 135 && rdesc[124] == 0x15 && rdesc[125] == 0x0 &&
			rdesc[126] == 0x25 && rdesc[127] == 0x11) {
		dev_info(&hdev->dev, "fixing up Samsung IrDA %d byte report "
				"descriptor\n", 135);
		rdesc[125] = 0x1;
		rdesc[127] = 0xe;
	}
}

static int samsung_probe(struct hid_device *hdev,
		const struct hid_device_id *id)
{
	int ret;
	unsigned int cmask = HID_CONNECT_DEFAULT;

	ret = hid_parse(hdev);
	if (ret) {
		dev_err(&hdev->dev, "parse failed\n");
		goto err_free;
	}

	if (hdev->rsize == 184) {
		
		cmask = (cmask & ~HID_CONNECT_HIDINPUT) |
			HID_CONNECT_HIDDEV_FORCE;
	}

	ret = hid_hw_start(hdev, cmask);
	if (ret) {
		dev_err(&hdev->dev, "hw start failed\n");
		goto err_free;
	}

	return 0;
err_free:
	return ret;
}

static const struct hid_device_id samsung_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_SAMSUNG, USB_DEVICE_ID_SAMSUNG_IR_REMOTE) },
	{ }
};
MODULE_DEVICE_TABLE(hid, samsung_devices);

static struct hid_driver samsung_driver = {
	.name = "samsung",
	.id_table = samsung_devices,
	.report_fixup = samsung_report_fixup,
	.probe = samsung_probe,
};

static int __init samsung_init(void)
{
	return hid_register_driver(&samsung_driver);
}

static void __exit samsung_exit(void)
{
	hid_unregister_driver(&samsung_driver);
}

module_init(samsung_init);
module_exit(samsung_exit);
MODULE_LICENSE("GPL");
