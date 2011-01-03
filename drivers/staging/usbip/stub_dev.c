

#include "usbip_common.h"
#include "stub.h"



static int stub_probe(struct usb_interface *interface,
				const struct usb_device_id *id);
static void stub_disconnect(struct usb_interface *interface);



static struct usb_device_id stub_table[] = {
#if 0
	
	{ USB_DEVICE(0x05ac, 0x0301) },   
	{ USB_DEVICE(0x0430, 0x0009) },   
	{ USB_DEVICE(0x059b, 0x0001) },   
	{ USB_DEVICE(0x04b3, 0x4427) },   
	{ USB_DEVICE(0x05a9, 0xa511) },   
	{ USB_DEVICE(0x55aa, 0x0201) },   
	{ USB_DEVICE(0x046d, 0x0870) },   
	{ USB_DEVICE(0x04bb, 0x0101) },   
	{ USB_DEVICE(0x04bb, 0x0904) },   
	{ USB_DEVICE(0x04bb, 0x0201) },   
	{ USB_DEVICE(0x08bb, 0x2702) },   
	{ USB_DEVICE(0x046d, 0x08b2) },   
#endif
	
	{ .driver_info = 1 },
	{ 0, }                                     
};
MODULE_DEVICE_TABLE(usb, stub_table);

struct usb_driver stub_driver = {
	.name		= "usbip",
	.probe		= stub_probe,
	.disconnect	= stub_disconnect,
	.id_table	= stub_table,
};








static ssize_t show_status(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct stub_device *sdev = dev_get_drvdata(dev);
	int status;

	if (!sdev) {
		dev_err(dev, "sdev is null\n");
		return -ENODEV;
	}

	spin_lock(&sdev->ud.lock);
	status = sdev->ud.status;
	spin_unlock(&sdev->ud.lock);

	return snprintf(buf, PAGE_SIZE, "%d\n", status);
}
static DEVICE_ATTR(usbip_status, S_IRUGO, show_status, NULL);


static ssize_t store_sockfd(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct stub_device *sdev = dev_get_drvdata(dev);
	int sockfd = 0;
	struct socket *socket;

	if (!sdev) {
		dev_err(dev, "sdev is null\n");
		return -ENODEV;
	}

	sscanf(buf, "%d", &sockfd);

	if (sockfd != -1) {
		dev_info(dev, "stub up\n");

		spin_lock(&sdev->ud.lock);

		if (sdev->ud.status != SDEV_ST_AVAILABLE) {
			dev_err(dev, "not ready\n");
			spin_unlock(&sdev->ud.lock);
			return -EINVAL;
		}

		socket = sockfd_to_socket(sockfd);
		if (!socket) {
			spin_unlock(&sdev->ud.lock);
			return -EINVAL;
		}

#if 0
		setnodelay(socket);
		setkeepalive(socket);
		setreuse(socket);
#endif

		sdev->ud.tcp_socket = socket;

		spin_unlock(&sdev->ud.lock);

		usbip_start_threads(&sdev->ud);

		spin_lock(&sdev->ud.lock);
		sdev->ud.status = SDEV_ST_USED;
		spin_unlock(&sdev->ud.lock);

	} else {
		dev_info(dev, "stub down\n");

		spin_lock(&sdev->ud.lock);
		if (sdev->ud.status != SDEV_ST_USED) {
			spin_unlock(&sdev->ud.lock);
			return -EINVAL;
		}
		spin_unlock(&sdev->ud.lock);

		usbip_event_add(&sdev->ud, SDEV_EVENT_DOWN);
	}

	return count;
}
static DEVICE_ATTR(usbip_sockfd, S_IWUSR, NULL, store_sockfd);

static int stub_add_files(struct device *dev)
{
	int err = 0;

	err = device_create_file(dev, &dev_attr_usbip_status);
	if (err)
		goto err_status;

	err = device_create_file(dev, &dev_attr_usbip_sockfd);
	if (err)
		goto err_sockfd;

	err = device_create_file(dev, &dev_attr_usbip_debug);
	if (err)
		goto err_debug;

	return 0;

err_debug:
	device_remove_file(dev, &dev_attr_usbip_sockfd);

err_sockfd:
	device_remove_file(dev, &dev_attr_usbip_status);

err_status:
	return err;
}

static void stub_remove_files(struct device *dev)
{
	device_remove_file(dev, &dev_attr_usbip_status);
	device_remove_file(dev, &dev_attr_usbip_sockfd);
	device_remove_file(dev, &dev_attr_usbip_debug);
}







static void stub_shutdown_connection(struct usbip_device *ud)
{
	struct stub_device *sdev = container_of(ud, struct stub_device, ud);

	
	if (ud->tcp_socket) {
		usbip_udbg("shutdown tcp_socket %p\n", ud->tcp_socket);
		kernel_sock_shutdown(ud->tcp_socket, SHUT_RDWR);
	}

	
	usbip_stop_threads(ud);

	
	
	if (ud->tcp_socket) {
		sock_release(ud->tcp_socket);
		ud->tcp_socket = NULL;
	}

	
	stub_device_cleanup_urbs(sdev);

	
	{
		unsigned long flags;
		struct stub_unlink *unlink, *tmp;

		spin_lock_irqsave(&sdev->priv_lock, flags);

		list_for_each_entry_safe(unlink, tmp, &sdev->unlink_tx, list) {
			list_del(&unlink->list);
			kfree(unlink);
		}

		list_for_each_entry_safe(unlink, tmp,
						 &sdev->unlink_free, list) {
			list_del(&unlink->list);
			kfree(unlink);
		}

		spin_unlock_irqrestore(&sdev->priv_lock, flags);
	}
}

static void stub_device_reset(struct usbip_device *ud)
{
	struct stub_device *sdev = container_of(ud, struct stub_device, ud);
	struct usb_device *udev = interface_to_usbdev(sdev->interface);
	int ret;

	usbip_udbg("device reset");
	ret = usb_lock_device_for_reset(udev, sdev->interface);
	if (ret < 0) {
		dev_err(&udev->dev, "lock for reset\n");

		spin_lock(&ud->lock);
		ud->status = SDEV_ST_ERROR;
		spin_unlock(&ud->lock);

		return;
	}

	
	ret = usb_reset_device(udev);

	usb_unlock_device(udev);

	spin_lock(&ud->lock);
	if (ret) {
		dev_err(&udev->dev, "device reset\n");
		ud->status = SDEV_ST_ERROR;

	} else {
		dev_info(&udev->dev, "device reset\n");
		ud->status = SDEV_ST_AVAILABLE;

	}
	spin_unlock(&ud->lock);

	return;
}

static void stub_device_unusable(struct usbip_device *ud)
{
	spin_lock(&ud->lock);
	ud->status = SDEV_ST_ERROR;
	spin_unlock(&ud->lock);
}





static struct stub_device *stub_device_alloc(struct usb_interface *interface)
{
	struct stub_device *sdev;
	int busnum = interface_to_busnum(interface);
	int devnum = interface_to_devnum(interface);

	dev_dbg(&interface->dev, "allocating stub device");

	
	sdev = kzalloc(sizeof(struct stub_device), GFP_KERNEL);
	if (!sdev) {
		dev_err(&interface->dev, "no memory for stub_device\n");
		return NULL;
	}

	sdev->interface = interface;

	
	sdev->devid     = (busnum << 16) | devnum;

	usbip_task_init(&sdev->ud.tcp_rx, "stub_rx", stub_rx_loop);
	usbip_task_init(&sdev->ud.tcp_tx, "stub_tx", stub_tx_loop);

	sdev->ud.side = USBIP_STUB;
	sdev->ud.status = SDEV_ST_AVAILABLE;
	
	spin_lock_init(&sdev->ud.lock);
	sdev->ud.tcp_socket = NULL;

	INIT_LIST_HEAD(&sdev->priv_init);
	INIT_LIST_HEAD(&sdev->priv_tx);
	INIT_LIST_HEAD(&sdev->priv_free);
	INIT_LIST_HEAD(&sdev->unlink_free);
	INIT_LIST_HEAD(&sdev->unlink_tx);
	
	spin_lock_init(&sdev->priv_lock);

	init_waitqueue_head(&sdev->tx_waitq);

	sdev->ud.eh_ops.shutdown = stub_shutdown_connection;
	sdev->ud.eh_ops.reset    = stub_device_reset;
	sdev->ud.eh_ops.unusable = stub_device_unusable;

	usbip_start_eh(&sdev->ud);

	usbip_udbg("register new interface\n");
	return sdev;
}

static int stub_device_free(struct stub_device *sdev)
{
	if (!sdev)
		return -EINVAL;

	kfree(sdev);
	usbip_udbg("kfree udev ok\n");

	return 0;
}





static int stub_probe(struct usb_interface *interface,
		      const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(interface);
	struct stub_device *sdev = NULL;
	const char *udev_busid = dev_name(interface->dev.parent);
	int err = 0;

	dev_dbg(&interface->dev, "Enter\n");

	
	if (match_busid(udev_busid)) {
		dev_info(&interface->dev,
			 "this device %s is not in match_busid table. skip!\n",
			 udev_busid);

		
		return -ENODEV;
	}

	if (udev->descriptor.bDeviceClass ==  USB_CLASS_HUB) {
		usbip_udbg("this device %s is a usb hub device. skip!\n",
								udev_busid);
		return -ENODEV;
	}

	if (!strcmp(udev->bus->bus_name, "vhci_hcd")) {
		usbip_udbg("this device %s is attached on vhci_hcd. skip!\n",
								udev_busid);
		return -ENODEV;
	}

	
	sdev = stub_device_alloc(interface);
	if (!sdev)
		return -ENOMEM;

	dev_info(&interface->dev, "USB/IP Stub: register a new interface "
		 "(bus %u dev %u ifn %u)\n", udev->bus->busnum, udev->devnum,
		 interface->cur_altsetting->desc.bInterfaceNumber);

	
	usb_set_intfdata(interface, sdev);

	err = stub_add_files(&interface->dev);
	if (err) {
		dev_err(&interface->dev, "create sysfs files for %s\n",
			udev_busid);
		return err;
	}

	return 0;
}



static void stub_disconnect(struct usb_interface *interface)
{
	struct stub_device *sdev = usb_get_intfdata(interface);

	usbip_udbg("Enter\n");

	
	if (!sdev) {
		err(" could not get device from inteface data");
		
		return;
	}

	usb_set_intfdata(interface, NULL);


	
	stub_remove_files(&interface->dev);

	
	usbip_event_add(&sdev->ud, SDEV_EVENT_REMOVED);

	
	usbip_stop_eh(&sdev->ud);

	
	stub_device_free(sdev);


	usbip_udbg("bye\n");
}
