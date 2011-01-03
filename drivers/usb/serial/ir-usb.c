

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>
#include <linux/usb/irda.h>


#define DRIVER_VERSION "v0.4"
#define DRIVER_AUTHOR "Greg Kroah-Hartman <greg@kroah.com>"
#define DRIVER_DESC "USB IR Dongle driver"

static int debug;


static int buffer_size;


static int xbof = -1;

static int  ir_startup (struct usb_serial *serial);
static int  ir_open(struct tty_struct *tty, struct usb_serial_port *port);
static void ir_close(struct usb_serial_port *port);
static int  ir_write(struct tty_struct *tty, struct usb_serial_port *port,
					const unsigned char *buf, int count);
static void ir_write_bulk_callback (struct urb *urb);
static void ir_read_bulk_callback (struct urb *urb);
static void ir_set_termios(struct tty_struct *tty,
		struct usb_serial_port *port, struct ktermios *old_termios);


static u8 ir_baud;
static u8 ir_xbof;
static u8 ir_add_bof;

static struct usb_device_id ir_id_table[] = {
	{ USB_DEVICE(0x050f, 0x0180) },		
	{ USB_DEVICE(0x08e9, 0x0100) },		
	{ USB_DEVICE(0x09c4, 0x0011) },		
	{ USB_INTERFACE_INFO(USB_CLASS_APP_SPEC, USB_SUBCLASS_IRDA, 0) },
	{ }					
};

MODULE_DEVICE_TABLE(usb, ir_id_table);

static struct usb_driver ir_driver = {
	.name		= "ir-usb",
	.probe		= usb_serial_probe,
	.disconnect	= usb_serial_disconnect,
	.id_table	= ir_id_table,
	.no_dynamic_id	= 1,
};

static struct usb_serial_driver ir_device = {
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "ir-usb",
	},
	.description		= "IR Dongle",
	.usb_driver		= &ir_driver,
	.id_table		= ir_id_table,
	.num_ports		= 1,
	.set_termios		= ir_set_termios,
	.attach			= ir_startup,
	.open			= ir_open,
	.close			= ir_close,
	.write			= ir_write,
	.write_bulk_callback	= ir_write_bulk_callback,
	.read_bulk_callback	= ir_read_bulk_callback,
};

static inline void irda_usb_dump_class_desc(struct usb_irda_cs_descriptor *desc)
{
	dbg("bLength=%x", desc->bLength);
	dbg("bDescriptorType=%x", desc->bDescriptorType);
	dbg("bcdSpecRevision=%x", __le16_to_cpu(desc->bcdSpecRevision));
	dbg("bmDataSize=%x", desc->bmDataSize);
	dbg("bmWindowSize=%x", desc->bmWindowSize);
	dbg("bmMinTurnaroundTime=%d", desc->bmMinTurnaroundTime);
	dbg("wBaudRate=%x", __le16_to_cpu(desc->wBaudRate));
	dbg("bmAdditionalBOFs=%x", desc->bmAdditionalBOFs);
	dbg("bIrdaRateSniff=%x", desc->bIrdaRateSniff);
	dbg("bMaxUnicastList=%x", desc->bMaxUnicastList);
}



static struct usb_irda_cs_descriptor *
irda_usb_find_class_desc(struct usb_device *dev, unsigned int ifnum)
{
	struct usb_irda_cs_descriptor *desc;
	int ret;

	desc = kzalloc(sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return NULL;

	ret = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
			USB_REQ_CS_IRDA_GET_CLASS_DESC,
			USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
			0, ifnum, desc, sizeof(*desc), 1000);

	dbg("%s -  ret=%d", __func__, ret);
	if (ret < sizeof(*desc)) {
		dbg("%s - class descriptor read %s (%d)",
				__func__,
				(ret < 0) ? "failed" : "too short",
				ret);
		goto error;
	}
	if (desc->bDescriptorType != USB_DT_CS_IRDA) {
		dbg("%s - bad class descriptor type", __func__);
		goto error;
	}

	irda_usb_dump_class_desc(desc);
	return desc;

error:
	kfree(desc);
	return NULL;
}


static u8 ir_xbof_change(u8 xbof)
{
	u8 result;

	
	switch (xbof) {
	case 48:
		result = 0x10;
		break;
	case 28:
	case 24:
		result = 0x20;
		break;
	default:
	case 12:
		result = 0x30;
		break;
	case  5:
	case  6:
		result = 0x40;
		break;
	case  3:
		result = 0x50;
		break;
	case  2:
		result = 0x60;
		break;
	case  1:
		result = 0x70;
		break;
	case  0:
		result = 0x80;
		break;
	}

	return(result);
}


static int ir_startup(struct usb_serial *serial)
{
	struct usb_irda_cs_descriptor *irda_desc;

	irda_desc = irda_usb_find_class_desc(serial->dev, 0);
	if (!irda_desc) {
		dev_err(&serial->dev->dev,
			"IRDA class descriptor not found, device not bound\n");
		return -ENODEV;
	}

	dbg("%s - Baud rates supported:%s%s%s%s%s%s%s%s%s",
		__func__,
		(irda_desc->wBaudRate & USB_IRDA_BR_2400) ? " 2400" : "",
		(irda_desc->wBaudRate & USB_IRDA_BR_9600) ? " 9600" : "",
		(irda_desc->wBaudRate & USB_IRDA_BR_19200) ? " 19200" : "",
		(irda_desc->wBaudRate & USB_IRDA_BR_38400) ? " 38400" : "",
		(irda_desc->wBaudRate & USB_IRDA_BR_57600) ? " 57600" : "",
		(irda_desc->wBaudRate & USB_IRDA_BR_115200) ? " 115200" : "",
		(irda_desc->wBaudRate & USB_IRDA_BR_576000) ? " 576000" : "",
		(irda_desc->wBaudRate & USB_IRDA_BR_1152000) ? " 1152000" : "",
		(irda_desc->wBaudRate & USB_IRDA_BR_4000000) ? " 4000000" : "");

	switch (irda_desc->bmAdditionalBOFs) {
	case USB_IRDA_AB_48:
		ir_add_bof = 48;
		break;
	case USB_IRDA_AB_24:
		ir_add_bof = 24;
		break;
	case USB_IRDA_AB_12:
		ir_add_bof = 12;
		break;
	case USB_IRDA_AB_6:
		ir_add_bof = 6;
		break;
	case USB_IRDA_AB_3:
		ir_add_bof = 3;
		break;
	case USB_IRDA_AB_2:
		ir_add_bof = 2;
		break;
	case USB_IRDA_AB_1:
		ir_add_bof = 1;
		break;
	case USB_IRDA_AB_0:
		ir_add_bof = 0;
		break;
	default:
		break;
	}

	kfree(irda_desc);

	return 0;
}

static int ir_open(struct tty_struct *tty, struct usb_serial_port *port)
{
	char *buffer;
	int result = 0;

	dbg("%s - port %d", __func__, port->number);

	if (buffer_size) {
		
		buffer = kmalloc(buffer_size, GFP_KERNEL);
		if (!buffer) {
			dev_err(&port->dev, "%s - out of memory.\n", __func__);
			return -ENOMEM;
		}
		kfree(port->read_urb->transfer_buffer);
		port->read_urb->transfer_buffer = buffer;
		port->read_urb->transfer_buffer_length = buffer_size;

		buffer = kmalloc(buffer_size, GFP_KERNEL);
		if (!buffer) {
			dev_err(&port->dev, "%s - out of memory.\n", __func__);
			return -ENOMEM;
		}
		kfree(port->write_urb->transfer_buffer);
		port->write_urb->transfer_buffer = buffer;
		port->write_urb->transfer_buffer_length = buffer_size;
		port->bulk_out_size = buffer_size;
	}

	
	usb_fill_bulk_urb(
		port->read_urb,
		port->serial->dev,
		usb_rcvbulkpipe(port->serial->dev,
			port->bulk_in_endpointAddress),
		port->read_urb->transfer_buffer,
		port->read_urb->transfer_buffer_length,
		ir_read_bulk_callback,
		port);
	result = usb_submit_urb(port->read_urb, GFP_KERNEL);
	if (result)
		dev_err(&port->dev,
			"%s - failed submitting read urb, error %d\n",
			__func__, result);

	return result;
}

static void ir_close(struct usb_serial_port *port)
{
	dbg("%s - port %d", __func__, port->number);

	
	usb_kill_urb(port->read_urb);
}

static int ir_write(struct tty_struct *tty, struct usb_serial_port *port,
					const unsigned char *buf, int count)
{
	unsigned char *transfer_buffer;
	int result;
	int transfer_size;

	dbg("%s - port = %d, count = %d", __func__, port->number, count);

	if (count == 0)
		return 0;

	spin_lock_bh(&port->lock);
	if (port->write_urb_busy) {
		spin_unlock_bh(&port->lock);
		dbg("%s - already writing", __func__);
		return 0;
	}
	port->write_urb_busy = 1;
	spin_unlock_bh(&port->lock);

	transfer_buffer = port->write_urb->transfer_buffer;
	transfer_size = min(count, port->bulk_out_size - 1);

	
	*transfer_buffer = ir_xbof | ir_baud;
	++transfer_buffer;

	memcpy(transfer_buffer, buf, transfer_size);

	usb_fill_bulk_urb(
		port->write_urb,
		port->serial->dev,
		usb_sndbulkpipe(port->serial->dev,
			port->bulk_out_endpointAddress),
		port->write_urb->transfer_buffer,
		transfer_size + 1,
		ir_write_bulk_callback,
		port);

	port->write_urb->transfer_flags = URB_ZERO_PACKET;

	result = usb_submit_urb(port->write_urb, GFP_ATOMIC);
	if (result) {
		port->write_urb_busy = 0;
		dev_err(&port->dev,
			"%s - failed submitting write urb, error %d\n",
			__func__, result);
	} else
		result = transfer_size;

	return result;
}

static void ir_write_bulk_callback(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	int status = urb->status;

	dbg("%s - port %d", __func__, port->number);

	port->write_urb_busy = 0;
	if (status) {
		dbg("%s - nonzero write bulk status received: %d",
		    __func__, status);
		return;
	}

	usb_serial_debug_data(
		debug,
		&port->dev,
		__func__,
		urb->actual_length,
		urb->transfer_buffer);

	usb_serial_port_softint(port);
}

static void ir_read_bulk_callback(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int result;
	int status = urb->status;

	dbg("%s - port %d", __func__, port->number);

	if (!port->port.count) {
		dbg("%s - port closed.", __func__);
		return;
	}

	switch (status) {
	case 0: 
		
		if ((*data & 0x0f) > 0)
			ir_baud = *data & 0x0f;
		usb_serial_debug_data(debug, &port->dev, __func__,
						urb->actual_length, data);
		tty = tty_port_tty_get(&port->port);
		if (tty_buffer_request_room(tty, urb->actual_length - 1)) {
			tty_insert_flip_string(tty, data+1, urb->actual_length - 1);
			tty_flip_buffer_push(tty);
		}
		tty_kref_put(tty);

		

	case -EPROTO: 
			
		usb_fill_bulk_urb(
			port->read_urb,
			port->serial->dev, 
			usb_rcvbulkpipe(port->serial->dev,
				port->bulk_in_endpointAddress),
			port->read_urb->transfer_buffer,
			port->read_urb->transfer_buffer_length,
			ir_read_bulk_callback,
			port);

		result = usb_submit_urb(port->read_urb, GFP_ATOMIC);
		if (result)
			dev_err(&port->dev, "%s - failed resubmitting read urb, error %d\n",
				__func__, result);
			break ;
	default:
		dbg("%s - nonzero read bulk status received: %d",
			__func__, status);
		break ;
	}
	return;
}

static void ir_set_termios(struct tty_struct *tty,
		struct usb_serial_port *port, struct ktermios *old_termios)
{
	unsigned char *transfer_buffer;
	int result;
	speed_t baud;
	int ir_baud;

	dbg("%s - port %d", __func__, port->number);

	baud = tty_get_baud_rate(tty);

	

	switch (baud) {
	case 2400:
		ir_baud = USB_IRDA_BR_2400;
		break;
	case 9600:
		ir_baud = USB_IRDA_BR_9600;
		break;
	case 19200:
		ir_baud = USB_IRDA_BR_19200;
		break;
	case 38400:
		ir_baud = USB_IRDA_BR_38400;
		break;
	case 57600:
		ir_baud = USB_IRDA_BR_57600;
		break;
	case 115200:
		ir_baud = USB_IRDA_BR_115200;
		break;
	case 576000:
		ir_baud = USB_IRDA_BR_576000;
		break;
	case 1152000:
		ir_baud = USB_IRDA_BR_1152000;
		break;
	case 4000000:
		ir_baud = USB_IRDA_BR_4000000;
		break;
	default:
		ir_baud = USB_IRDA_BR_9600;
		baud = 9600;
	}

	if (xbof == -1)
		ir_xbof = ir_xbof_change(ir_add_bof);
	else
		ir_xbof = ir_xbof_change(xbof) ;

	
	transfer_buffer = port->write_urb->transfer_buffer;
	*transfer_buffer = ir_xbof | ir_baud;

	usb_fill_bulk_urb(
		port->write_urb,
		port->serial->dev,
		usb_sndbulkpipe(port->serial->dev,
			port->bulk_out_endpointAddress),
		port->write_urb->transfer_buffer,
		1,
		ir_write_bulk_callback,
		port);

	port->write_urb->transfer_flags = URB_ZERO_PACKET;

	result = usb_submit_urb(port->write_urb, GFP_KERNEL);
	if (result)
		dev_err(&port->dev,
				"%s - failed submitting write urb, error %d\n",
				__func__, result);

	
	tty_termios_copy_hw(tty->termios, old_termios);
	tty_encode_baud_rate(tty, baud, baud);
}

static int __init ir_init(void)
{
	int retval;

	retval = usb_serial_register(&ir_device);
	if (retval)
		goto failed_usb_serial_register;

	retval = usb_register(&ir_driver);
	if (retval)
		goto failed_usb_register;

	printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
	       DRIVER_DESC "\n");

	return 0;

failed_usb_register:
	usb_serial_deregister(&ir_device);

failed_usb_serial_register:
	return retval;
}

static void __exit ir_exit(void)
{
	usb_deregister(&ir_driver);
	usb_serial_deregister(&ir_device);
}


module_init(ir_init);
module_exit(ir_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug enabled or not");
module_param(xbof, int, 0);
MODULE_PARM_DESC(xbof, "Force specific number of XBOFs");
module_param(buffer_size, int, 0);
MODULE_PARM_DESC(buffer_size, "Size of the transfer buffers");

