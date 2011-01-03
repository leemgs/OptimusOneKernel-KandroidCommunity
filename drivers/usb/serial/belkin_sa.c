

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
#include "belkin_sa.h"

static int debug;


#define DRIVER_VERSION "v1.2"
#define DRIVER_AUTHOR "William Greathouse <wgreathouse@smva.com>"
#define DRIVER_DESC "USB Belkin Serial converter driver"


static int  belkin_sa_startup(struct usb_serial *serial);
static void belkin_sa_release(struct usb_serial *serial);
static int  belkin_sa_open(struct tty_struct *tty,
			struct usb_serial_port *port);
static void belkin_sa_close(struct usb_serial_port *port);
static void belkin_sa_read_int_callback(struct urb *urb);
static void belkin_sa_set_termios(struct tty_struct *tty,
			struct usb_serial_port *port, struct ktermios * old);
static void belkin_sa_break_ctl(struct tty_struct *tty, int break_state);
static int  belkin_sa_tiocmget(struct tty_struct *tty, struct file *file);
static int  belkin_sa_tiocmset(struct tty_struct *tty, struct file *file,
					unsigned int set, unsigned int clear);


static struct usb_device_id id_table_combined [] = {
	{ USB_DEVICE(BELKIN_SA_VID, BELKIN_SA_PID) },
	{ USB_DEVICE(BELKIN_OLD_VID, BELKIN_OLD_PID) },
	{ USB_DEVICE(PERACOM_VID, PERACOM_PID) },
	{ USB_DEVICE(GOHUBS_VID, GOHUBS_PID) },
	{ USB_DEVICE(GOHUBS_VID, HANDYLINK_PID) },
	{ USB_DEVICE(BELKIN_DOCKSTATION_VID, BELKIN_DOCKSTATION_PID) },
	{ }	
};

MODULE_DEVICE_TABLE(usb, id_table_combined);

static struct usb_driver belkin_driver = {
	.name =		"belkin",
	.probe =	usb_serial_probe,
	.disconnect =	usb_serial_disconnect,
	.id_table =	id_table_combined,
	.no_dynamic_id = 	1,
};


static struct usb_serial_driver belkin_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"belkin",
	},
	.description =		"Belkin / Peracom / GoHubs USB Serial Adapter",
	.usb_driver =		&belkin_driver,
	.id_table =		id_table_combined,
	.num_ports =		1,
	.open =			belkin_sa_open,
	.close =		belkin_sa_close,
	.read_int_callback =	belkin_sa_read_int_callback,
					
	.set_termios =		belkin_sa_set_termios,
	.break_ctl =		belkin_sa_break_ctl,
	.tiocmget =		belkin_sa_tiocmget,
	.tiocmset =		belkin_sa_tiocmset,
	.attach =		belkin_sa_startup,
	.release =		belkin_sa_release,
};


struct belkin_sa_private {
	spinlock_t		lock;
	unsigned long		control_state;
	unsigned char		last_lsr;
	unsigned char		last_msr;
	int			bad_flow_control;
};




#define WDR_TIMEOUT 5000 


#define BSA_USB_CMD(c, v) usb_control_msg(serial->dev, usb_sndctrlpipe(serial->dev, 0), \
					    (c), BELKIN_SA_SET_REQUEST_TYPE, \
					    (v), 0, NULL, 0, WDR_TIMEOUT)


static int belkin_sa_startup(struct usb_serial *serial)
{
	struct usb_device *dev = serial->dev;
	struct belkin_sa_private *priv;

	
	priv = kmalloc(sizeof(struct belkin_sa_private), GFP_KERNEL);
	if (!priv)
		return -1; 
	
	spin_lock_init(&priv->lock);
	priv->control_state = 0;
	priv->last_lsr = 0;
	priv->last_msr = 0;
	
	priv->bad_flow_control =
		(le16_to_cpu(dev->descriptor.bcdDevice) <= 0x0206) ? 1 : 0;
	dev_info(&dev->dev, "bcdDevice: %04x, bfc: %d\n",
					le16_to_cpu(dev->descriptor.bcdDevice),
					priv->bad_flow_control);

	init_waitqueue_head(&serial->port[0]->write_wait);
	usb_set_serial_port_data(serial->port[0], priv);

	return 0;
}


static void belkin_sa_release(struct usb_serial *serial)
{
	struct belkin_sa_private *priv;
	int i;

	dbg("%s", __func__);

	for (i = 0; i < serial->num_ports; ++i) {
		
		priv = usb_get_serial_port_data(serial->port[i]);
		kfree(priv);
	}
}


static int  belkin_sa_open(struct tty_struct *tty,
					struct usb_serial_port *port)
{
	int retval = 0;

	dbg("%s port %d", __func__, port->number);

	
	
	port->read_urb->dev = port->serial->dev;
	retval = usb_submit_urb(port->read_urb, GFP_KERNEL);
	if (retval) {
		dev_err(&port->dev, "usb_submit_urb(read bulk) failed\n");
		goto exit;
	}

	port->interrupt_in_urb->dev = port->serial->dev;
	retval = usb_submit_urb(port->interrupt_in_urb, GFP_KERNEL);
	if (retval) {
		usb_kill_urb(port->read_urb);
		dev_err(&port->dev, "usb_submit_urb(read int) failed\n");
	}

exit:
	return retval;
} 


static void belkin_sa_close(struct usb_serial_port *port)
{
	dbg("%s port %d", __func__, port->number);

	
	usb_kill_urb(port->write_urb);
	usb_kill_urb(port->read_urb);
	usb_kill_urb(port->interrupt_in_urb);
} 


static void belkin_sa_read_int_callback(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	struct belkin_sa_private *priv;
	unsigned char *data = urb->transfer_buffer;
	int retval;
	int status = urb->status;
	unsigned long flags;

	switch (status) {
	case 0:
		
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		
		dbg("%s - urb shutting down with status: %d",
		    __func__, status);
		return;
	default:
		dbg("%s - nonzero urb status received: %d",
		    __func__, status);
		goto exit;
	}

	usb_serial_debug_data(debug, &port->dev, __func__,
					urb->actual_length, data);

	
	

	priv = usb_get_serial_port_data(port);
	spin_lock_irqsave(&priv->lock, flags);
	priv->last_msr = data[BELKIN_SA_MSR_INDEX];

	
	if (priv->last_msr & BELKIN_SA_MSR_DSR)
		priv->control_state |= TIOCM_DSR;
	else
		priv->control_state &= ~TIOCM_DSR;

	if (priv->last_msr & BELKIN_SA_MSR_CTS)
		priv->control_state |= TIOCM_CTS;
	else
		priv->control_state &= ~TIOCM_CTS;

	if (priv->last_msr & BELKIN_SA_MSR_RI)
		priv->control_state |= TIOCM_RI;
	else
		priv->control_state &= ~TIOCM_RI;

	if (priv->last_msr & BELKIN_SA_MSR_CD)
		priv->control_state |= TIOCM_CD;
	else
		priv->control_state &= ~TIOCM_CD;

	
	priv->last_lsr = data[BELKIN_SA_LSR_INDEX];
#if 0
	
	if (priv->last_lsr & BELKIN_SA_LSR_ERR) {
		tty = tty_port_tty_get(&port->port);
		
		if (priv->last_lsr & BELKIN_SA_LSR_OE) {
		}
		
		if (priv->last_lsr & BELKIN_SA_LSR_PE) {
		}
		
		if (priv->last_lsr & BELKIN_SA_LSR_FE) {
		}
		
		if (priv->last_lsr & BELKIN_SA_LSR_BI) {
		}
		tty_kref_put(tty);
	}
#endif
	spin_unlock_irqrestore(&priv->lock, flags);
exit:
	retval = usb_submit_urb(urb, GFP_ATOMIC);
	if (retval)
		dev_err(&port->dev, "%s - usb_submit_urb failed with "
			"result %d\n", __func__, retval);
}

static void belkin_sa_set_termios(struct tty_struct *tty,
		struct usb_serial_port *port, struct ktermios *old_termios)
{
	struct usb_serial *serial = port->serial;
	struct belkin_sa_private *priv = usb_get_serial_port_data(port);
	unsigned int iflag;
	unsigned int cflag;
	unsigned int old_iflag = 0;
	unsigned int old_cflag = 0;
	__u16 urb_value = 0; 
	unsigned long flags;
	unsigned long control_state;
	int bad_flow_control;
	speed_t baud;
	struct ktermios *termios = tty->termios;

	iflag = termios->c_iflag;
	cflag = termios->c_cflag;

	termios->c_cflag &= ~CMSPAR;

	
	spin_lock_irqsave(&priv->lock, flags);
	control_state = priv->control_state;
	bad_flow_control = priv->bad_flow_control;
	spin_unlock_irqrestore(&priv->lock, flags);

	old_iflag = old_termios->c_iflag;
	old_cflag = old_termios->c_cflag;

	
	if ((cflag & CBAUD) != (old_cflag & CBAUD)) {
		
		if ((old_cflag & CBAUD) == B0) {
			control_state |= (TIOCM_DTR|TIOCM_RTS);
			if (BSA_USB_CMD(BELKIN_SA_SET_DTR_REQUEST, 1) < 0)
				dev_err(&port->dev, "Set DTR error\n");
			
			if (!(old_cflag & CRTSCTS))
				if (BSA_USB_CMD(BELKIN_SA_SET_RTS_REQUEST
								, 1) < 0)
					dev_err(&port->dev, "Set RTS error\n");
		}
	}

	baud = tty_get_baud_rate(tty);
	if (baud) {
		urb_value = BELKIN_SA_BAUD(baud);
		
		if (urb_value == 0)
			urb_value = 1;
		
		baud = BELKIN_SA_BAUD(urb_value);

		
		tty_encode_baud_rate(tty, baud, baud);
		if (BSA_USB_CMD(BELKIN_SA_SET_BAUDRATE_REQUEST, urb_value) < 0)
			dev_err(&port->dev, "Set baudrate error\n");
	} else {
		
		if (BSA_USB_CMD(BELKIN_SA_SET_FLOW_CTRL_REQUEST,
						BELKIN_SA_FLOW_NONE) < 0)
			dev_err(&port->dev, "Disable flowcontrol error\n");
		
		control_state &= ~(TIOCM_DTR | TIOCM_RTS);
		if (BSA_USB_CMD(BELKIN_SA_SET_DTR_REQUEST, 0) < 0)
			dev_err(&port->dev, "DTR LOW error\n");
		if (BSA_USB_CMD(BELKIN_SA_SET_RTS_REQUEST, 0) < 0)
			dev_err(&port->dev, "RTS LOW error\n");
	}

	
	if ((cflag ^ old_cflag) & (PARENB | PARODD)) {
		if (cflag & PARENB)
			urb_value = (cflag & PARODD) ?  BELKIN_SA_PARITY_ODD
						: BELKIN_SA_PARITY_EVEN;
		else
			urb_value = BELKIN_SA_PARITY_NONE;
		if (BSA_USB_CMD(BELKIN_SA_SET_PARITY_REQUEST, urb_value) < 0)
			dev_err(&port->dev, "Set parity error\n");
	}

	
	if ((cflag & CSIZE) != (old_cflag & CSIZE)) {
		switch (cflag & CSIZE) {
		case CS5:
			urb_value = BELKIN_SA_DATA_BITS(5);
			break;
		case CS6:
			urb_value = BELKIN_SA_DATA_BITS(6);
			break;
		case CS7:
			urb_value = BELKIN_SA_DATA_BITS(7);
			break;
		case CS8:
			urb_value = BELKIN_SA_DATA_BITS(8);
			break;
		default: dbg("CSIZE was not CS5-CS8, using default of 8");
			urb_value = BELKIN_SA_DATA_BITS(8);
			break;
		}
		if (BSA_USB_CMD(BELKIN_SA_SET_DATA_BITS_REQUEST, urb_value) < 0)
			dev_err(&port->dev, "Set data bits error\n");
	}

	
	if ((cflag & CSTOPB) != (old_cflag & CSTOPB)) {
		urb_value = (cflag & CSTOPB) ? BELKIN_SA_STOP_BITS(2)
						: BELKIN_SA_STOP_BITS(1);
		if (BSA_USB_CMD(BELKIN_SA_SET_STOP_BITS_REQUEST,
							urb_value) < 0)
			dev_err(&port->dev, "Set stop bits error\n");
	}

	
	if (((iflag ^ old_iflag) & (IXOFF | IXON)) ||
		((cflag ^ old_cflag) & CRTSCTS)) {
		urb_value = 0;
		if ((iflag & IXOFF) || (iflag & IXON))
			urb_value |= (BELKIN_SA_FLOW_OXON | BELKIN_SA_FLOW_IXON);
		else
			urb_value &= ~(BELKIN_SA_FLOW_OXON | BELKIN_SA_FLOW_IXON);

		if (cflag & CRTSCTS)
			urb_value |=  (BELKIN_SA_FLOW_OCTS | BELKIN_SA_FLOW_IRTS);
		else
			urb_value &= ~(BELKIN_SA_FLOW_OCTS | BELKIN_SA_FLOW_IRTS);

		if (bad_flow_control)
			urb_value &= ~(BELKIN_SA_FLOW_IRTS);

		if (BSA_USB_CMD(BELKIN_SA_SET_FLOW_CTRL_REQUEST, urb_value) < 0)
			dev_err(&port->dev, "Set flow control error\n");
	}

	
	spin_lock_irqsave(&priv->lock, flags);
	priv->control_state = control_state;
	spin_unlock_irqrestore(&priv->lock, flags);
} 


static void belkin_sa_break_ctl(struct tty_struct *tty, int break_state)
{
	struct usb_serial_port *port = tty->driver_data;
	struct usb_serial *serial = port->serial;

	if (BSA_USB_CMD(BELKIN_SA_SET_BREAK_REQUEST, break_state ? 1 : 0) < 0)
		dev_err(&port->dev, "Set break_ctl %d\n", break_state);
}


static int belkin_sa_tiocmget(struct tty_struct *tty, struct file *file)
{
	struct usb_serial_port *port = tty->driver_data;
	struct belkin_sa_private *priv = usb_get_serial_port_data(port);
	unsigned long control_state;
	unsigned long flags;

	dbg("%s", __func__);

	spin_lock_irqsave(&priv->lock, flags);
	control_state = priv->control_state;
	spin_unlock_irqrestore(&priv->lock, flags);

	return control_state;
}


static int belkin_sa_tiocmset(struct tty_struct *tty, struct file *file,
			       unsigned int set, unsigned int clear)
{
	struct usb_serial_port *port = tty->driver_data;
	struct usb_serial *serial = port->serial;
	struct belkin_sa_private *priv = usb_get_serial_port_data(port);
	unsigned long control_state;
	unsigned long flags;
	int retval;
	int rts = 0;
	int dtr = 0;

	dbg("%s", __func__);

	spin_lock_irqsave(&priv->lock, flags);
	control_state = priv->control_state;

	if (set & TIOCM_RTS) {
		control_state |= TIOCM_RTS;
		rts = 1;
	}
	if (set & TIOCM_DTR) {
		control_state |= TIOCM_DTR;
		dtr = 1;
	}
	if (clear & TIOCM_RTS) {
		control_state &= ~TIOCM_RTS;
		rts = 0;
	}
	if (clear & TIOCM_DTR) {
		control_state &= ~TIOCM_DTR;
		dtr = 0;
	}

	priv->control_state = control_state;
	spin_unlock_irqrestore(&priv->lock, flags);

	retval = BSA_USB_CMD(BELKIN_SA_SET_RTS_REQUEST, rts);
	if (retval < 0) {
		dev_err(&port->dev, "Set RTS error %d\n", retval);
		goto exit;
	}

	retval = BSA_USB_CMD(BELKIN_SA_SET_DTR_REQUEST, dtr);
	if (retval < 0) {
		dev_err(&port->dev, "Set DTR error %d\n", retval);
		goto exit;
	}
exit:
	return retval;
}


static int __init belkin_sa_init(void)
{
	int retval;
	retval = usb_serial_register(&belkin_device);
	if (retval)
		goto failed_usb_serial_register;
	retval = usb_register(&belkin_driver);
	if (retval)
		goto failed_usb_register;
	printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
	       DRIVER_DESC "\n");
	return 0;
failed_usb_register:
	usb_serial_deregister(&belkin_device);
failed_usb_serial_register:
	return retval;
}


static void __exit belkin_sa_exit (void)
{
	usb_deregister(&belkin_driver);
	usb_serial_deregister(&belkin_device);
}


module_init(belkin_sa_init);
module_exit(belkin_sa_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug enabled or not");
