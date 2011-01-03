

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>

#define RIM_VENDOR		0x0fca
#define BLACKBERRY		0x0001
#define BLACKBERRY_PEARL_DUAL   0x0004
#define BLACKBERRY_PEARL        0x0006

static int debug;
static int pearl_dual_mode = 1;

#ifdef dbg
#undef dbg
#endif
#define dbg(dev, format, arg...)				\
	if (debug)						\
		dev_printk(KERN_DEBUG , dev , format , ## arg)

static struct usb_device_id id_table [] = {
	{ USB_DEVICE(RIM_VENDOR, BLACKBERRY) },
	{ USB_DEVICE(RIM_VENDOR, BLACKBERRY_PEARL) },
	{ USB_DEVICE(RIM_VENDOR, BLACKBERRY_PEARL_DUAL) },
	{ },					
};
MODULE_DEVICE_TABLE(usb, id_table);

static int magic_charge(struct usb_device *udev)
{
	char *dummy_buffer = kzalloc(2, GFP_KERNEL);
	int retval;

	if (!dummy_buffer)
		return -ENOMEM;

	

	
	dbg(&udev->dev, "Sending first magic command\n");
	retval = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
				 0xa5, 0xc0, 0, 1, dummy_buffer, 2, 100);
	if (retval != 2) {
		dev_err(&udev->dev, "First magic command failed: %d.\n",
			retval);
		goto exit;
	}

	dbg(&udev->dev, "Sending second magic command\n");
	retval = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
				 0xa2, 0x40, 0, 1, dummy_buffer, 0, 100);
	if (retval != 0) {
		dev_err(&udev->dev, "Second magic command failed: %d.\n",
			retval);
		goto exit;
	}

	dbg(&udev->dev, "Calling set_configuration\n");
	retval = usb_driver_set_configuration(udev, 1);
	if (retval)
		dev_err(&udev->dev, "Set Configuration failed :%d.\n", retval);

exit:
	kfree(dummy_buffer);
	return retval;
}

static int magic_dual_mode(struct usb_device *udev)
{
	char *dummy_buffer = kzalloc(2, GFP_KERNEL);
	int retval;

	if (!dummy_buffer)
		return -ENOMEM;

	
	dbg(&udev->dev, "Sending magic pearl command\n");
	retval = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
				 0xa9, 0xc0, 1, 1, dummy_buffer, 2, 100);
	dbg(&udev->dev, "Magic pearl command returned %d\n", retval);

	dbg(&udev->dev, "Calling set_configuration\n");
	retval = usb_driver_set_configuration(udev, 1);
	if (retval)
		dev_err(&udev->dev, "Set Configuration failed :%d.\n", retval);

	kfree(dummy_buffer);
	return retval;
}

static int berry_probe(struct usb_interface *intf,
		       const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(intf);

	if (udev->bus_mA < 500) {
		dbg(&udev->dev, "Not enough power to charge available\n");
		return -ENODEV;
	}

	dbg(&udev->dev, "Power is set to %dmA\n",
	    udev->actconfig->desc.bMaxPower * 2);

	
	if ((udev->actconfig->desc.bMaxPower * 2) == 500) {
		dbg(&udev->dev, "device is already charging, power is "
		    "set to %dmA\n", udev->actconfig->desc.bMaxPower * 2);
		return -ENODEV;
	}

	
	magic_charge(udev);

	if ((le16_to_cpu(udev->descriptor.idProduct) == BLACKBERRY_PEARL) &&
	    (pearl_dual_mode))
		magic_dual_mode(udev);

	
	return -ENODEV;
}

static void berry_disconnect(struct usb_interface *intf)
{
}

static struct usb_driver berry_driver = {
	.name =		"berry_charge",
	.probe =	berry_probe,
	.disconnect =	berry_disconnect,
	.id_table =	id_table,
};

static int __init berry_init(void)
{
	return usb_register(&berry_driver);
}

static void __exit berry_exit(void)
{
	usb_deregister(&berry_driver);
}

module_init(berry_init);
module_exit(berry_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Greg Kroah-Hartman <gregkh@suse.de>");
module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug enabled or not");
module_param(pearl_dual_mode, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(pearl_dual_mode, "Change Blackberry Pearl to run in dual mode");
