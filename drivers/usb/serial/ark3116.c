

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/tty.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>
#include <linux/serial.h>
#include <linux/uaccess.h>


static int debug;

static struct usb_device_id id_table [] = {
	{ USB_DEVICE(0x6547, 0x0232) },
	{ USB_DEVICE(0x18ec, 0x3118) },		
	{ },
};
MODULE_DEVICE_TABLE(usb, id_table);

static int is_irda(struct usb_serial *serial)
{
	struct usb_device *dev = serial->dev;
	if (le16_to_cpu(dev->descriptor.idVendor) == 0x18ec &&
			le16_to_cpu(dev->descriptor.idProduct) == 0x3118)
		return 1;
	return 0;
}

static inline void ARK3116_SND(struct usb_serial *serial, int seq,
			       __u8 request, __u8 requesttype,
			       __u16 value, __u16 index)
{
	int result;
	result = usb_control_msg(serial->dev,
				 usb_sndctrlpipe(serial->dev, 0),
				 request, requesttype, value, index,
				 NULL, 0x00, 1000);
	dbg("%03d > ok", seq);
}

static inline void ARK3116_RCV(struct usb_serial *serial, int seq,
			       __u8 request, __u8 requesttype,
			       __u16 value, __u16 index, __u8 expected,
			       char *buf)
{
	int result;
	result = usb_control_msg(serial->dev,
				 usb_rcvctrlpipe(serial->dev, 0),
				 request, requesttype, value, index,
				 buf, 0x0000001, 1000);
	if (result)
		dbg("%03d < %d bytes [0x%02X]", seq, result,
		    ((unsigned char *)buf)[0]);
	else
		dbg("%03d < 0 bytes", seq);
}

static inline void ARK3116_RCV_QUIET(struct usb_serial *serial,
				     __u8 request, __u8 requesttype,
				     __u16 value, __u16 index, char *buf)
{
	usb_control_msg(serial->dev,
			usb_rcvctrlpipe(serial->dev, 0),
			request, requesttype, value, index,
			buf, 0x0000001, 1000);
}

static int ark3116_attach(struct usb_serial *serial)
{
	char *buf;

	buf = kmalloc(1, GFP_KERNEL);
	if (!buf) {
		dbg("error kmalloc -> out of mem?");
		return -ENOMEM;
	}

	if (is_irda(serial))
		dbg("IrDA mode");

	
	ARK3116_SND(serial, 3, 0xFE, 0x40, 0x0008, 0x0002);
	ARK3116_SND(serial, 4, 0xFE, 0x40, 0x0008, 0x0001);
	ARK3116_SND(serial, 5, 0xFE, 0x40, 0x0000, 0x0008);
	ARK3116_SND(serial, 6, 0xFE, 0x40, is_irda(serial) ? 0x0001 : 0x0000,
		    0x000B);

	if (is_irda(serial)) {
		ARK3116_SND(serial, 1001, 0xFE, 0x40, 0x0000, 0x000C);
		ARK3116_SND(serial, 1002, 0xFE, 0x40, 0x0041, 0x000D);
		ARK3116_SND(serial, 1003, 0xFE, 0x40, 0x0001, 0x000A);
	}

	
	ARK3116_RCV(serial,  7, 0xFE, 0xC0, 0x0000, 0x0003, 0x00, buf);
	ARK3116_SND(serial,  8, 0xFE, 0x40, 0x0080, 0x0003);
	ARK3116_SND(serial,  9, 0xFE, 0x40, 0x001A, 0x0000);
	ARK3116_SND(serial, 10, 0xFE, 0x40, 0x0000, 0x0001);
	ARK3116_SND(serial, 11, 0xFE, 0x40, 0x0000, 0x0003);

	
	ARK3116_RCV(serial, 12, 0xFE, 0xC0, 0x0000, 0x0004, 0x00, buf);
	ARK3116_SND(serial, 13, 0xFE, 0x40, 0x0000, 0x0004);

	
	ARK3116_RCV(serial, 14, 0xFE, 0xC0, 0x0000, 0x0004, 0x00, buf);
	ARK3116_SND(serial, 15, 0xFE, 0x40, 0x0000, 0x0004);

	
	ARK3116_RCV(serial, 16, 0xFE, 0xC0, 0x0000, 0x0004, 0x00, buf);
	
	ARK3116_SND(serial, 17, 0xFE, 0x40, 0x0001, 0x0004);

	
	ARK3116_RCV(serial, 18, 0xFE, 0xC0, 0x0000, 0x0004, 0x01, buf);

	
	ARK3116_SND(serial, 19, 0xFE, 0x40, 0x0003, 0x0004);

	
	
	
	ARK3116_RCV(serial, 20, 0xFE, 0xC0, 0x0000, 0x0006, 0xFF, buf);

	
	ARK3116_SND(serial, 147, 0xFE, 0x40, 0x0083, 0x0003);
	ARK3116_SND(serial, 148, 0xFE, 0x40, 0x0038, 0x0000);
	ARK3116_SND(serial, 149, 0xFE, 0x40, 0x0001, 0x0001);
	if (is_irda(serial))
		ARK3116_SND(serial, 1004, 0xFE, 0x40, 0x0000, 0x0009);
	ARK3116_SND(serial, 150, 0xFE, 0x40, 0x0003, 0x0003);
	ARK3116_RCV(serial, 151, 0xFE, 0xC0, 0x0000, 0x0004, 0x03, buf);
	ARK3116_SND(serial, 152, 0xFE, 0x40, 0x0000, 0x0003);
	ARK3116_RCV(serial, 153, 0xFE, 0xC0, 0x0000, 0x0003, 0x00, buf);
	ARK3116_SND(serial, 154, 0xFE, 0x40, 0x0003, 0x0003);

	kfree(buf);
	return 0;
}

static void ark3116_init_termios(struct tty_struct *tty)
{
	struct ktermios *termios = tty->termios;
	*termios = tty_std_termios;
	termios->c_cflag = B9600 | CS8
				      | CREAD | HUPCL | CLOCAL;
	termios->c_ispeed = 9600;
	termios->c_ospeed = 9600;
}

static void ark3116_set_termios(struct tty_struct *tty,
				struct usb_serial_port *port,
				struct ktermios *old_termios)
{
	struct usb_serial *serial = port->serial;
	struct ktermios *termios = tty->termios;
	unsigned int cflag = termios->c_cflag;
	int baud;
	int ark3116_baud;
	char *buf;
	char config;

	config = 0;

	dbg("%s - port %d", __func__, port->number);


	cflag = termios->c_cflag;
	termios->c_cflag &= ~(CMSPAR|CRTSCTS);

	buf = kmalloc(1, GFP_KERNEL);
	if (!buf) {
		dbg("error kmalloc");
		*termios = *old_termios;
		return;
	}

	
	if (cflag & CSIZE) {
		switch (cflag & CSIZE) {
		case CS5:
			config |= 0x00;
			dbg("setting CS5");
			break;
		case CS6:
			config |= 0x01;
			dbg("setting CS6");
			break;
		case CS7:
			config |= 0x02;
			dbg("setting CS7");
			break;
		default:
			dbg("CSIZE was set but not CS5-CS8, using CS8!");
			
		case CS8:
			config |= 0x03;
			dbg("setting CS8");
			break;
		}
	}

	
	if (cflag & PARENB) {
		if (cflag & PARODD) {
			config |= 0x08;
			dbg("setting parity to ODD");
		} else {
			config |= 0x18;
			dbg("setting parity to EVEN");
		}
	} else {
		dbg("setting parity to NONE");
	}

	
	if (cflag & CSTOPB) {
		config |= 0x04;
		dbg("setting 2 stop bits");
	} else {
		dbg("setting 1 stop bit");
	}

	
	baud = tty_get_baud_rate(tty);

	switch (baud) {
	case 75:
	case 150:
	case 300:
	case 600:
	case 1200:
	case 1800:
	case 2400:
	case 4800:
	case 9600:
	case 19200:
	case 38400:
	case 57600:
	case 115200:
	case 230400:
	case 460800:
		
		tty_encode_baud_rate(tty, baud, baud);
		break;
	
	default:
		tty_encode_baud_rate(tty, 9600, 9600);
	case 0:
		baud = 9600;
	}

	
	if (baud == 460800)
		
		ark3116_baud = 7;
	else
		ark3116_baud = 3000000 / baud;

	
	ARK3116_RCV(serial, 0, 0xFE, 0xC0, 0x0000, 0x0003, 0x03, buf);

	
	
	

	
	dbg("setting baudrate to %d (->reg=%d)", baud, ark3116_baud);
	ARK3116_SND(serial, 147, 0xFE, 0x40, 0x0083, 0x0003);
	ARK3116_SND(serial, 148, 0xFE, 0x40,
			    (ark3116_baud & 0x00FF), 0x0000);
	ARK3116_SND(serial, 149, 0xFE, 0x40,
			    (ark3116_baud & 0xFF00) >> 8, 0x0001);
	ARK3116_SND(serial, 150, 0xFE, 0x40, 0x0003, 0x0003);

	
	ARK3116_RCV(serial, 151, 0xFE, 0xC0, 0x0000, 0x0004, 0x03, buf);
	ARK3116_SND(serial, 152, 0xFE, 0x40, 0x0000, 0x0003);

	
	dbg("updating bit count, stop bit or parity (cfg=0x%02X)", config);
	ARK3116_RCV(serial, 153, 0xFE, 0xC0, 0x0000, 0x0003, 0x00, buf);
	ARK3116_SND(serial, 154, 0xFE, 0x40, config, 0x0003);

	if (cflag & CRTSCTS)
		dbg("CRTSCTS not supported by chipset?!");

	

	kfree(buf);

	return;
}

static int ark3116_open(struct tty_struct *tty, struct usb_serial_port *port)
{
	struct ktermios tmp_termios;
	struct usb_serial *serial = port->serial;
	char *buf;
	int result = 0;

	dbg("%s - port %d", __func__, port->number);

	buf = kmalloc(1, GFP_KERNEL);
	if (!buf) {
		dbg("error kmalloc -> out of mem?");
		return -ENOMEM;
	}

	result = usb_serial_generic_open(tty, port);
	if (result)
		goto err_out;

	
	ARK3116_RCV(serial, 111, 0xFE, 0xC0, 0x0000, 0x0003, 0x02, buf);

	ARK3116_SND(serial, 112, 0xFE, 0x40, 0x0082, 0x0003);
	ARK3116_SND(serial, 113, 0xFE, 0x40, 0x001A, 0x0000);
	ARK3116_SND(serial, 114, 0xFE, 0x40, 0x0000, 0x0001);
	ARK3116_SND(serial, 115, 0xFE, 0x40, 0x0002, 0x0003);

	ARK3116_RCV(serial, 116, 0xFE, 0xC0, 0x0000, 0x0004, 0x03, buf);
	ARK3116_SND(serial, 117, 0xFE, 0x40, 0x0002, 0x0004);

	ARK3116_RCV(serial, 118, 0xFE, 0xC0, 0x0000, 0x0004, 0x02, buf);
	ARK3116_SND(serial, 119, 0xFE, 0x40, 0x0000, 0x0004);

	ARK3116_RCV(serial, 120, 0xFE, 0xC0, 0x0000, 0x0004, 0x00, buf);

	ARK3116_SND(serial, 121, 0xFE, 0x40, 0x0001, 0x0004);

	ARK3116_RCV(serial, 122, 0xFE, 0xC0, 0x0000, 0x0004, 0x01, buf);

	ARK3116_SND(serial, 123, 0xFE, 0x40, 0x0003, 0x0004);

	
	ARK3116_RCV(serial, 124, 0xFE, 0xC0, 0x0000, 0x0006, 0xFF, buf);

	
	if (tty)
		ark3116_set_termios(tty, port, &tmp_termios);

err_out:
	kfree(buf);

	return result;
}

static int ark3116_ioctl(struct tty_struct *tty, struct file *file,
			 unsigned int cmd, unsigned long arg)
{
	struct usb_serial_port *port = tty->driver_data;
	struct serial_struct serstruct;
	void __user *user_arg = (void __user *)arg;

	switch (cmd) {
	case TIOCGSERIAL:
		
		memset(&serstruct, 0, sizeof(serstruct));
		serstruct.type = PORT_16654;
		serstruct.line = port->serial->minor;
		serstruct.port = port->number;
		serstruct.custom_divisor = 0;
		serstruct.baud_base = 460800;

		if (copy_to_user(user_arg, &serstruct, sizeof(serstruct)))
			return -EFAULT;

		return 0;
	case TIOCSSERIAL:
		if (copy_from_user(&serstruct, user_arg, sizeof(serstruct)))
			return -EFAULT;
		return 0;
	default:
		dbg("%s cmd 0x%04x not supported", __func__, cmd);
		break;
	}

	return -ENOIOCTLCMD;
}

static int ark3116_tiocmget(struct tty_struct *tty, struct file *file)
{
	struct usb_serial_port *port = tty->driver_data;
	struct usb_serial *serial = port->serial;
	char *buf;
	char temp;

	

	buf = kmalloc(1, GFP_KERNEL);
	if (!buf) {
		dbg("error kmalloc");
		return -ENOMEM;
	}

	
	ARK3116_RCV_QUIET(serial, 0xFE, 0xC0, 0x0000, 0x0006, buf);
	temp = buf[0];
	kfree(buf);

	
	return (temp & (1<<4) ? TIOCM_CTS : 0)
	       | (temp & (1<<6) ? TIOCM_DSR : 0);
}

static struct usb_driver ark3116_driver = {
	.name =		"ark3116",
	.probe =	usb_serial_probe,
	.disconnect =	usb_serial_disconnect,
	.id_table =	id_table,
	.no_dynamic_id =	1,
};

static struct usb_serial_driver ark3116_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"ark3116",
	},
	.id_table =		id_table,
	.usb_driver =		&ark3116_driver,
	.num_ports =		1,
	.attach =		ark3116_attach,
	.set_termios =		ark3116_set_termios,
	.init_termios =		ark3116_init_termios,
	.ioctl =		ark3116_ioctl,
	.tiocmget =		ark3116_tiocmget,
	.open =			ark3116_open,
};

static int __init ark3116_init(void)
{
	int retval;

	retval = usb_serial_register(&ark3116_device);
	if (retval)
		return retval;
	retval = usb_register(&ark3116_driver);
	if (retval)
		usb_serial_deregister(&ark3116_device);
	return retval;
}

static void __exit ark3116_exit(void)
{
	usb_deregister(&ark3116_driver);
	usb_serial_deregister(&ark3116_device);
}

module_init(ark3116_init);
module_exit(ark3116_exit);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug enabled or not");

