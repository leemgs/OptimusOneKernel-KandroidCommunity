

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>
#include <linux/uaccess.h>


#define DRIVER_VERSION	"v0.3"
#define DRIVER_AUTHOR	"Roelf Diedericks"
#define DRIVER_DESC	"IPWireless tty driver"

#define IPW_TTY_MAJOR	240	
#define IPW_TTY_MINORS	256	

#define USB_IPW_MAGIC	0x6d02	



#define EVENT_BUFFER_SIZE	0xFF
#define CHAR2INT16(c1, c0)	(((u32)((c1) & 0xff) << 8) + (u32)((c0) & 0xff))
#define NUM_BULK_URBS		24
#define NUM_CONTROL_URBS	16


#define IPW_VID		0x0bc3
#define IPW_PID		0x0001





enum {
	ipw_sio_b256000 = 0x000e,
	ipw_sio_b128000 = 0x001d,
	ipw_sio_b115200 = 0x0020,
	ipw_sio_b57600  = 0x0040,
	ipw_sio_b56000  = 0x0042,
	ipw_sio_b38400  = 0x0060,
	ipw_sio_b19200  = 0x00c0,
	ipw_sio_b14400  = 0x0100,
	ipw_sio_b9600   = 0x0180,
	ipw_sio_b4800   = 0x0300,
	ipw_sio_b2400   = 0x0600,
	ipw_sio_b1200   = 0x0c00,
	ipw_sio_b600    = 0x1800
};


#define ipw_dtb_7		0x700
#define ipw_dtb_8		0x810	
					


#define IPW_SIO_RXCTL		0x00	
#define IPW_SIO_SET_BAUD	0x01	
#define IPW_SIO_SET_LINE	0x03	
#define IPW_SIO_SET_PIN		0x03	
#define IPW_SIO_POLL		0x08	
#define IPW_SIO_INIT		0x11	
#define IPW_SIO_PURGE		0x12	
#define IPW_SIO_HANDFLOW	0x13	
#define IPW_SIO_SETCHARS	0x13	
					


#define IPW_PIN_SETDTR		0x101
#define IPW_PIN_SETRTS		0x202
#define IPW_PIN_CLRDTR		0x100
#define IPW_PIN_CLRRTS		0x200 


#define IPW_RXBULK_ON		1
#define IPW_RXBULK_OFF		0


#define IPW_BYTES_FLOWINIT	{ 0x01, 0, 0, 0, 0x40, 0, 0, 0, \
					0, 0, 0, 0, 0, 0, 0, 0 }



#define IPW_DSR			((1<<4) | (1<<5))
#define IPW_CTS			((1<<5) | (1<<4))

#define IPW_WANTS_TO_SEND	0x30

static struct usb_device_id usb_ipw_ids[] = {
	{ USB_DEVICE(IPW_VID, IPW_PID) },
	{ },
};

MODULE_DEVICE_TABLE(usb, usb_ipw_ids);

static struct usb_driver usb_ipw_driver = {
	.name =		"ipwtty",
	.probe =	usb_serial_probe,
	.disconnect =	usb_serial_disconnect,
	.id_table =	usb_ipw_ids,
	.no_dynamic_id = 	1,
};

static int debug;

static void ipw_read_bulk_callback(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	unsigned char *data = urb->transfer_buffer;
	struct tty_struct *tty;
	int result;
	int status = urb->status;

	dbg("%s - port %d", __func__, port->number);

	if (status) {
		dbg("%s - nonzero read bulk status received: %d",
		    __func__, status);
		return;
	}

	usb_serial_debug_data(debug, &port->dev, __func__,
					urb->actual_length, data);

	tty = tty_port_tty_get(&port->port);
	if (tty && urb->actual_length) {
		tty_buffer_request_room(tty, urb->actual_length);
		tty_insert_flip_string(tty, data, urb->actual_length);
		tty_flip_buffer_push(tty);
	}
	tty_kref_put(tty);

	
	usb_fill_bulk_urb(port->read_urb, port->serial->dev,
			  usb_rcvbulkpipe(port->serial->dev,
					   port->bulk_in_endpointAddress),
			  port->read_urb->transfer_buffer,
			  port->read_urb->transfer_buffer_length,
			  ipw_read_bulk_callback, port);
	result = usb_submit_urb(port->read_urb, GFP_ATOMIC);
	if (result)
		dev_err(&port->dev,
			"%s - failed resubmitting read urb, error %d\n",
							__func__, result);
	return;
}

static int ipw_open(struct tty_struct *tty, struct usb_serial_port *port)
{
	struct usb_device *dev = port->serial->dev;
	u8 buf_flow_static[16] = IPW_BYTES_FLOWINIT;
	u8 *buf_flow_init;
	int result;

	dbg("%s", __func__);

	buf_flow_init = kmemdup(buf_flow_static, 16, GFP_KERNEL);
	if (!buf_flow_init)
		return -ENOMEM;

	
	dbg("%s: Sending SIO_INIT (we guess)", __func__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			 IPW_SIO_INIT,
			 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
			 0,
			 0, 
			 NULL,
			 0,
			 100000);
	if (result < 0)
		dev_err(&port->dev,
			"Init of modem failed (error = %d)\n", result);

	
	usb_clear_halt(dev,
			usb_rcvbulkpipe(dev, port->bulk_in_endpointAddress));
	usb_clear_halt(dev,
			usb_sndbulkpipe(dev, port->bulk_out_endpointAddress));

	
	dbg("%s: setting up bulk read callback", __func__);
	usb_fill_bulk_urb(port->read_urb, dev,
			  usb_rcvbulkpipe(dev, port->bulk_in_endpointAddress),
			  port->bulk_in_buffer,
			  port->bulk_in_size,
			  ipw_read_bulk_callback, port);
	result = usb_submit_urb(port->read_urb, GFP_KERNEL);
	if (result < 0)
		dbg("%s - usb_submit_urb(read bulk) failed with status %d",
							__func__, result);

	
	dbg("%s:asking modem for RxRead (RXBULK_ON)", __func__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			 IPW_SIO_RXCTL,
			 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
			 IPW_RXBULK_ON,
			 0, 
			 NULL,
			 0,
			 100000);
	if (result < 0)
		dev_err(&port->dev,
			"Enabling bulk RxRead failed (error = %d)\n", result);

	
	dbg("%s:setting init flowcontrol (%s)", __func__, buf_flow_init);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			 IPW_SIO_HANDFLOW,
			 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
			 0,
			 0,
			 buf_flow_init,
			 0x10,
			 200000);
	if (result < 0)
		dev_err(&port->dev,
			"initial flowcontrol failed (error = %d)\n", result);


	
	dbg("%s:raising dtr", __func__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			 IPW_SIO_SET_PIN,
			 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
			 IPW_PIN_SETDTR,
			 0,
			 NULL,
			 0,
			 200000);
	if (result < 0)
		dev_err(&port->dev,
				"setting dtr failed (error = %d)\n", result);

	
	dbg("%s:raising rts", __func__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			 IPW_SIO_SET_PIN,
			 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
			 IPW_PIN_SETRTS,
			 0,
			 NULL,
			 0,
			 200000);
	if (result < 0)
		dev_err(&port->dev,
				"setting dtr failed (error = %d)\n", result);

	kfree(buf_flow_init);
	return 0;
}

static void ipw_dtr_rts(struct usb_serial_port *port, int on)
{
	struct usb_device *dev = port->serial->dev;
	int result;

	
	dbg("%s:dropping dtr", __func__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			 IPW_SIO_SET_PIN,
			 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
			 on ? IPW_PIN_SETDTR : IPW_PIN_CLRDTR,
			 0,
			 NULL,
			 0,
			 200000);
	if (result < 0)
		dev_err(&port->dev, "dropping dtr failed (error = %d)\n",
								result);

	
	dbg("%s:dropping rts", __func__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			 IPW_SIO_SET_PIN, USB_TYPE_VENDOR |
			 		USB_RECIP_INTERFACE | USB_DIR_OUT,
			 on ? IPW_PIN_SETRTS : IPW_PIN_CLRRTS,
			 0,
			 NULL,
			 0,
			 200000);
	if (result < 0)
		dev_err(&port->dev,
				"dropping rts failed (error = %d)\n", result);
}

static void ipw_close(struct usb_serial_port *port)
{
	struct usb_device *dev = port->serial->dev;
	int result;

	
	dbg("%s:sending purge", __func__);
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			 IPW_SIO_PURGE, USB_TYPE_VENDOR |
			 		USB_RECIP_INTERFACE | USB_DIR_OUT,
			 0x03,
			 0,
			 NULL,
			 0,
			 200000);
	if (result < 0)
		dev_err(&port->dev, "purge failed (error = %d)\n", result);


	
	result = usb_control_msg(dev, usb_sndctrlpipe(dev, 0),
			 IPW_SIO_RXCTL,
			 USB_TYPE_VENDOR | USB_RECIP_INTERFACE | USB_DIR_OUT,
			 IPW_RXBULK_OFF,
			 0, 
			 NULL,
			 0,
			 100000);

	if (result < 0)
		dev_err(&port->dev,
			"Disabling bulk RxRead failed (error = %d)\n", result);

	
	usb_kill_urb(port->read_urb);
	usb_kill_urb(port->write_urb);
}

static void ipw_write_bulk_callback(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	int status = urb->status;

	dbg("%s", __func__);

	port->write_urb_busy = 0;

	if (status)
		dbg("%s - nonzero write bulk status received: %d",
		    __func__, status);

	usb_serial_port_softint(port);
}

static int ipw_write(struct tty_struct *tty, struct usb_serial_port *port,
					const unsigned char *buf, int count)
{
	struct usb_device *dev = port->serial->dev;
	int ret;

	dbg("%s: TOP: count=%d, in_interrupt=%ld", __func__,
		count, in_interrupt());

	if (count == 0) {
		dbg("%s - write request of 0 bytes", __func__);
		return 0;
	}

	spin_lock_bh(&port->lock);
	if (port->write_urb_busy) {
		spin_unlock_bh(&port->lock);
		dbg("%s - already writing", __func__);
		return 0;
	}
	port->write_urb_busy = 1;
	spin_unlock_bh(&port->lock);

	count = min(count, port->bulk_out_size);
	memcpy(port->bulk_out_buffer, buf, count);

	dbg("%s count now:%d", __func__, count);

	usb_fill_bulk_urb(port->write_urb, dev,
			  usb_sndbulkpipe(dev, port->bulk_out_endpointAddress),
			  port->write_urb->transfer_buffer,
			  count,
			  ipw_write_bulk_callback,
			  port);

	ret = usb_submit_urb(port->write_urb, GFP_ATOMIC);
	if (ret != 0) {
		port->write_urb_busy = 0;
		dbg("%s - usb_submit_urb(write bulk) failed with error = %d",
								__func__, ret);
		return ret;
	}

	dbg("%s returning %d", __func__, count);
	return count;
}

static int ipw_probe(struct usb_serial_port *port)
{
	return 0;
}

static int ipw_disconnect(struct usb_serial_port *port)
{
	usb_set_serial_port_data(port, NULL);
	return 0;
}

static struct usb_serial_driver ipw_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"ipw",
	},
	.description =		"IPWireless converter",
	.usb_driver = 		&usb_ipw_driver,
	.id_table =		usb_ipw_ids,
	.num_ports =		1,
	.open =			ipw_open,
	.close =		ipw_close,
	.dtr_rts =		ipw_dtr_rts,
	.port_probe = 		ipw_probe,
	.port_remove =		ipw_disconnect,
	.write =		ipw_write,
	.write_bulk_callback =	ipw_write_bulk_callback,
	.read_bulk_callback =	ipw_read_bulk_callback,
};



static int __init usb_ipw_init(void)
{
	int retval;

	retval = usb_serial_register(&ipw_device);
	if (retval)
		return retval;
	retval = usb_register(&usb_ipw_driver);
	if (retval) {
		usb_serial_deregister(&ipw_device);
		return retval;
	}
	printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
	       DRIVER_DESC "\n");
	return 0;
}

static void __exit usb_ipw_exit(void)
{
	usb_deregister(&usb_ipw_driver);
	usb_serial_deregister(&ipw_device);
}

module_init(usb_ipw_init);
module_exit(usb_ipw_exit);


MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug enabled or not");
