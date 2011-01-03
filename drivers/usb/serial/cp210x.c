

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/usb.h>
#include <linux/uaccess.h>
#include <linux/usb/serial.h>


#define DRIVER_VERSION "v0.09"
#define DRIVER_DESC "Silicon Labs CP210x RS232 serial adaptor driver"


static int cp210x_open(struct tty_struct *tty, struct usb_serial_port *);
static void cp210x_cleanup(struct usb_serial_port *);
static void cp210x_close(struct usb_serial_port *);
static void cp210x_get_termios(struct tty_struct *,
	struct usb_serial_port *port);
static void cp210x_get_termios_port(struct usb_serial_port *port,
	unsigned int *cflagp, unsigned int *baudp);
static void cp210x_set_termios(struct tty_struct *, struct usb_serial_port *,
							struct ktermios*);
static int cp210x_tiocmget(struct tty_struct *, struct file *);
static int cp210x_tiocmset(struct tty_struct *, struct file *,
		unsigned int, unsigned int);
static int cp210x_tiocmset_port(struct usb_serial_port *port, struct file *,
		unsigned int, unsigned int);
static void cp210x_break_ctl(struct tty_struct *, int);
static int cp210x_startup(struct usb_serial *);
static void cp210x_disconnect(struct usb_serial *);
static void cp210x_dtr_rts(struct usb_serial_port *p, int on);
static int cp210x_carrier_raised(struct usb_serial_port *p);

static int debug;

static struct usb_device_id id_table [] = {
	{ USB_DEVICE(0x0471, 0x066A) }, 
	{ USB_DEVICE(0x0489, 0xE000) }, 
	{ USB_DEVICE(0x0745, 0x1000) }, 
	{ USB_DEVICE(0x08e6, 0x5501) }, 
	{ USB_DEVICE(0x08FD, 0x000A) }, 
	{ USB_DEVICE(0x0FCF, 0x1003) }, 
	{ USB_DEVICE(0x0FCF, 0x1004) }, 
	{ USB_DEVICE(0x0FCF, 0x1006) }, 
	{ USB_DEVICE(0x10A6, 0xAA26) }, 
	{ USB_DEVICE(0x10AB, 0x10C5) }, 
	{ USB_DEVICE(0x10B5, 0xAC70) }, 
	{ USB_DEVICE(0x10C4, 0x0F91) }, 
	{ USB_DEVICE(0x10C4, 0x1101) }, 
	{ USB_DEVICE(0x10C4, 0x1601) }, 
	{ USB_DEVICE(0x10C4, 0x800A) }, 
	{ USB_DEVICE(0x10C4, 0x803B) }, 
	{ USB_DEVICE(0x10C4, 0x8053) }, 
	{ USB_DEVICE(0x10C4, 0x8054) }, 
	{ USB_DEVICE(0x10C4, 0x8066) }, 
	{ USB_DEVICE(0x10C4, 0x807A) }, 
	{ USB_DEVICE(0x10C4, 0x80CA) }, 
	{ USB_DEVICE(0x10C4, 0x80DD) }, 
	{ USB_DEVICE(0x10C4, 0x80F6) }, 
	{ USB_DEVICE(0x10C4, 0x8115) }, 
	{ USB_DEVICE(0x10C4, 0x813D) }, 
	{ USB_DEVICE(0x10C4, 0x813F) }, 
	{ USB_DEVICE(0x10C4, 0x814A) }, 
	{ USB_DEVICE(0x10C4, 0x814B) }, 
	{ USB_DEVICE(0x10C4, 0x815E) }, 
	{ USB_DEVICE(0x10C4, 0x819F) }, 
	{ USB_DEVICE(0x10C4, 0x81A6) }, 
	{ USB_DEVICE(0x10C4, 0x81AC) }, 
	{ USB_DEVICE(0x10C4, 0x81C8) }, 
	{ USB_DEVICE(0x10C4, 0x81E2) }, 
	{ USB_DEVICE(0x10C4, 0x81E7) }, 
	{ USB_DEVICE(0x10C4, 0x81F2) }, 
	{ USB_DEVICE(0x10C4, 0x8218) }, 
	{ USB_DEVICE(0x10C4, 0x822B) }, 
	{ USB_DEVICE(0x10C4, 0x826B) }, 
	{ USB_DEVICE(0x10c4, 0x8293) }, 
	{ USB_DEVICE(0x10C4, 0x82F9) }, 
	{ USB_DEVICE(0x10C4, 0x8341) }, 
	{ USB_DEVICE(0x10C4, 0x8382) }, 
	{ USB_DEVICE(0x10C4, 0x83A8) }, 
	{ USB_DEVICE(0x10C4, 0x8411) }, 
	{ USB_DEVICE(0x10C4, 0x846E) }, 
	{ USB_DEVICE(0x10C4, 0xEA60) }, 
	{ USB_DEVICE(0x10C4, 0xEA61) }, 
	{ USB_DEVICE(0x10C4, 0xF001) }, 
	{ USB_DEVICE(0x10C4, 0xF002) }, 
	{ USB_DEVICE(0x10C4, 0xF003) }, 
	{ USB_DEVICE(0x10C4, 0xF004) }, 
	{ USB_DEVICE(0x10C5, 0xEA61) }, 
	{ USB_DEVICE(0x10CE, 0xEA6A) }, 
	{ USB_DEVICE(0x13AD, 0x9999) }, 
	{ USB_DEVICE(0x1555, 0x0004) }, 
	{ USB_DEVICE(0x166A, 0x0303) }, 
	{ USB_DEVICE(0x16D6, 0x0001) }, 
	{ USB_DEVICE(0x18EF, 0xE00F) }, 
	{ USB_DEVICE(0x413C, 0x9500) }, 
	{ } 
};

MODULE_DEVICE_TABLE(usb, id_table);

static struct usb_driver cp210x_driver = {
	.name		= "cp210x",
	.probe		= usb_serial_probe,
	.disconnect	= usb_serial_disconnect,
	.id_table	= id_table,
	.no_dynamic_id	= 	1,
};

static struct usb_serial_driver cp210x_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name = 	"cp210x",
	},
	.usb_driver		= &cp210x_driver,
	.id_table		= id_table,
	.num_ports		= 1,
	.open			= cp210x_open,
	.close			= cp210x_close,
	.break_ctl		= cp210x_break_ctl,
	.set_termios		= cp210x_set_termios,
	.tiocmget 		= cp210x_tiocmget,
	.tiocmset		= cp210x_tiocmset,
	.attach			= cp210x_startup,
	.disconnect		= cp210x_disconnect,
	.dtr_rts		= cp210x_dtr_rts,
	.carrier_raised		= cp210x_carrier_raised
};


#define REQTYPE_HOST_TO_DEVICE	0x41
#define REQTYPE_DEVICE_TO_HOST	0xc1


#define CP210X_IFC_ENABLE	0x00
#define CP210X_SET_BAUDDIV	0x01
#define CP210X_GET_BAUDDIV	0x02
#define CP210X_SET_LINE_CTL	0x03
#define CP210X_GET_LINE_CTL	0x04
#define CP210X_SET_BREAK	0x05
#define CP210X_IMM_CHAR		0x06
#define CP210X_SET_MHS		0x07
#define CP210X_GET_MDMSTS	0x08
#define CP210X_SET_XON		0x09
#define CP210X_SET_XOFF		0x0A
#define CP210X_SET_EVENTMASK	0x0B
#define CP210X_GET_EVENTMASK	0x0C
#define CP210X_SET_CHAR		0x0D
#define CP210X_GET_CHARS	0x0E
#define CP210X_GET_PROPS	0x0F
#define CP210X_GET_COMM_STATUS	0x10
#define CP210X_RESET		0x11
#define CP210X_PURGE		0x12
#define CP210X_SET_FLOW		0x13
#define CP210X_GET_FLOW		0x14
#define CP210X_EMBED_EVENTS	0x15
#define CP210X_GET_EVENTSTATE	0x16
#define CP210X_SET_CHARS	0x19


#define UART_ENABLE		0x0001
#define UART_DISABLE		0x0000


#define BAUD_RATE_GEN_FREQ	0x384000


#define BITS_DATA_MASK		0X0f00
#define BITS_DATA_5		0X0500
#define BITS_DATA_6		0X0600
#define BITS_DATA_7		0X0700
#define BITS_DATA_8		0X0800
#define BITS_DATA_9		0X0900

#define BITS_PARITY_MASK	0x00f0
#define BITS_PARITY_NONE	0x0000
#define BITS_PARITY_ODD		0x0010
#define BITS_PARITY_EVEN	0x0020
#define BITS_PARITY_MARK	0x0030
#define BITS_PARITY_SPACE	0x0040

#define BITS_STOP_MASK		0x000f
#define BITS_STOP_1		0x0000
#define BITS_STOP_1_5		0x0001
#define BITS_STOP_2		0x0002


#define BREAK_ON		0x0000
#define BREAK_OFF		0x0001


#define CONTROL_DTR		0x0001
#define CONTROL_RTS		0x0002
#define CONTROL_CTS		0x0010
#define CONTROL_DSR		0x0020
#define CONTROL_RING		0x0040
#define CONTROL_DCD		0x0080
#define CONTROL_WRITE_DTR	0x0100
#define CONTROL_WRITE_RTS	0x0200


static int cp210x_get_config(struct usb_serial_port *port, u8 request,
		unsigned int *data, int size)
{
	struct usb_serial *serial = port->serial;
	__le32 *buf;
	int result, i, length;

	
	length = (((size - 1) | 3) + 1)/4;

	buf = kcalloc(length, sizeof(__le32), GFP_KERNEL);
	if (!buf) {
		dev_err(&port->dev, "%s - out of memory.\n", __func__);
		return -ENOMEM;
	}

	
	result = usb_control_msg(serial->dev, usb_rcvctrlpipe(serial->dev, 0),
				request, REQTYPE_DEVICE_TO_HOST, 0x0000,
				0, buf, size, 300);

	
	for (i = 0; i < length; i++)
		data[i] = le32_to_cpu(buf[i]);

	kfree(buf);

	if (result != size) {
		dbg("%s - Unable to send config request, "
				"request=0x%x size=%d result=%d\n",
				__func__, request, size, result);
		return -EPROTO;
	}

	return 0;
}


static int cp210x_set_config(struct usb_serial_port *port, u8 request,
		unsigned int *data, int size)
{
	struct usb_serial *serial = port->serial;
	__le32 *buf;
	int result, i, length;

	
	length = (((size - 1) | 3) + 1)/4;

	buf = kmalloc(length * sizeof(__le32), GFP_KERNEL);
	if (!buf) {
		dev_err(&port->dev, "%s - out of memory.\n",
				__func__);
		return -ENOMEM;
	}

	
	for (i = 0; i < length; i++)
		buf[i] = cpu_to_le32(data[i]);

	if (size > 2) {
		result = usb_control_msg(serial->dev,
				usb_sndctrlpipe(serial->dev, 0),
				request, REQTYPE_HOST_TO_DEVICE, 0x0000,
				0, buf, size, 300);
	} else {
		result = usb_control_msg(serial->dev,
				usb_sndctrlpipe(serial->dev, 0),
				request, REQTYPE_HOST_TO_DEVICE, data[0],
				0, NULL, 0, 300);
	}

	kfree(buf);

	if ((size > 2 && result != size) || result < 0) {
		dbg("%s - Unable to send request, "
				"request=0x%x size=%d result=%d\n",
				__func__, request, size, result);
		return -EPROTO;
	}

	
	result = usb_control_msg(serial->dev,
			usb_sndctrlpipe(serial->dev, 0),
			request, REQTYPE_HOST_TO_DEVICE, data[0],
			0, NULL, 0, 300);
	return 0;
}


static inline int cp210x_set_config_single(struct usb_serial_port *port,
		u8 request, unsigned int data)
{
	return cp210x_set_config(port, request, &data, 2);
}


static unsigned int cp210x_quantise_baudrate(unsigned int baud) {
	if      (baud <= 56)       baud = 0;
	else if (baud <= 300)      baud = 300;
	else if (baud <= 600)      baud = 600;
	else if (baud <= 1200)     baud = 1200;
	else if (baud <= 1800)     baud = 1800;
	else if (baud <= 2400)     baud = 2400;
	else if (baud <= 4000)     baud = 4000;
	else if (baud <= 4803)     baud = 4800;
	else if (baud <= 7207)     baud = 7200;
	else if (baud <= 9612)     baud = 9600;
	else if (baud <= 14428)    baud = 14400;
	else if (baud <= 16062)    baud = 16000;
	else if (baud <= 19250)    baud = 19200;
	else if (baud <= 28912)    baud = 28800;
	else if (baud <= 38601)    baud = 38400;
	else if (baud <= 51558)    baud = 51200;
	else if (baud <= 56280)    baud = 56000;
	else if (baud <= 58053)    baud = 57600;
	else if (baud <= 64111)    baud = 64000;
	else if (baud <= 77608)    baud = 76800;
	else if (baud <= 117028)   baud = 115200;
	else if (baud <= 129347)   baud = 128000;
	else if (baud <= 156868)   baud = 153600;
	else if (baud <= 237832)   baud = 230400;
	else if (baud <= 254234)   baud = 250000;
	else if (baud <= 273066)   baud = 256000;
	else if (baud <= 491520)   baud = 460800;
	else if (baud <= 567138)   baud = 500000;
	else if (baud <= 670254)   baud = 576000;
	else if (baud <= 1053257)  baud = 921600;
	else if (baud <= 1474560)  baud = 1228800;
	else if (baud <= 2457600)  baud = 1843200;
	else                       baud = 3686400;
	return baud;
}

static int cp210x_open(struct tty_struct *tty, struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;
	int result;

	dbg("%s - port %d", __func__, port->number);

	if (cp210x_set_config_single(port, CP210X_IFC_ENABLE, UART_ENABLE)) {
		dev_err(&port->dev, "%s - Unable to enable UART\n",
				__func__);
		return -EPROTO;
	}

	
	usb_fill_bulk_urb(port->read_urb, serial->dev,
			usb_rcvbulkpipe(serial->dev,
			port->bulk_in_endpointAddress),
			port->read_urb->transfer_buffer,
			port->read_urb->transfer_buffer_length,
			serial->type->read_bulk_callback,
			port);
	result = usb_submit_urb(port->read_urb, GFP_KERNEL);
	if (result) {
		dev_err(&port->dev, "%s - failed resubmitting read urb, "
				"error %d\n", __func__, result);
		return result;
	}

	
	cp210x_get_termios(tty, port);
	return 0;
}

static void cp210x_cleanup(struct usb_serial_port *port)
{
	struct usb_serial *serial = port->serial;

	dbg("%s - port %d", __func__, port->number);

	if (serial->dev) {
		
		if (serial->num_bulk_out)
			usb_kill_urb(port->write_urb);
		if (serial->num_bulk_in)
			usb_kill_urb(port->read_urb);
	}
}

static void cp210x_close(struct usb_serial_port *port)
{
	dbg("%s - port %d", __func__, port->number);

	
	dbg("%s - shutting down urbs", __func__);
	usb_kill_urb(port->write_urb);
	usb_kill_urb(port->read_urb);

	mutex_lock(&port->serial->disc_mutex);
	if (!port->serial->disconnected)
		cp210x_set_config_single(port, CP210X_IFC_ENABLE, UART_DISABLE);
	mutex_unlock(&port->serial->disc_mutex);
}


static void cp210x_get_termios(struct tty_struct *tty,
	struct usb_serial_port *port)
{
	unsigned int baud;

	if (tty) {
		cp210x_get_termios_port(tty->driver_data,
			&tty->termios->c_cflag, &baud);
		tty_encode_baud_rate(tty, baud, baud);
	}

	else {
		unsigned int cflag;
		cflag = 0;
		cp210x_get_termios_port(port, &cflag, &baud);
	}
}


static void cp210x_get_termios_port(struct usb_serial_port *port,
	unsigned int *cflagp, unsigned int *baudp)
{
	unsigned int cflag, modem_ctl[4];
	unsigned int baud;
	unsigned int bits;

	dbg("%s - port %d", __func__, port->number);

	cp210x_get_config(port, CP210X_GET_BAUDDIV, &baud, 2);
	
	if (baud)
		baud = cp210x_quantise_baudrate((BAUD_RATE_GEN_FREQ + baud/2)/ baud);

	dbg("%s - baud rate = %d", __func__, baud);
	*baudp = baud;

	cflag = *cflagp;

	cp210x_get_config(port, CP210X_GET_LINE_CTL, &bits, 2);
	cflag &= ~CSIZE;
	switch (bits & BITS_DATA_MASK) {
	case BITS_DATA_5:
		dbg("%s - data bits = 5", __func__);
		cflag |= CS5;
		break;
	case BITS_DATA_6:
		dbg("%s - data bits = 6", __func__);
		cflag |= CS6;
		break;
	case BITS_DATA_7:
		dbg("%s - data bits = 7", __func__);
		cflag |= CS7;
		break;
	case BITS_DATA_8:
		dbg("%s - data bits = 8", __func__);
		cflag |= CS8;
		break;
	case BITS_DATA_9:
		dbg("%s - data bits = 9 (not supported, using 8 data bits)",
								__func__);
		cflag |= CS8;
		bits &= ~BITS_DATA_MASK;
		bits |= BITS_DATA_8;
		cp210x_set_config(port, CP210X_SET_LINE_CTL, &bits, 2);
		break;
	default:
		dbg("%s - Unknown number of data bits, using 8", __func__);
		cflag |= CS8;
		bits &= ~BITS_DATA_MASK;
		bits |= BITS_DATA_8;
		cp210x_set_config(port, CP210X_SET_LINE_CTL, &bits, 2);
		break;
	}

	switch (bits & BITS_PARITY_MASK) {
	case BITS_PARITY_NONE:
		dbg("%s - parity = NONE", __func__);
		cflag &= ~PARENB;
		break;
	case BITS_PARITY_ODD:
		dbg("%s - parity = ODD", __func__);
		cflag |= (PARENB|PARODD);
		break;
	case BITS_PARITY_EVEN:
		dbg("%s - parity = EVEN", __func__);
		cflag &= ~PARODD;
		cflag |= PARENB;
		break;
	case BITS_PARITY_MARK:
		dbg("%s - parity = MARK (not supported, disabling parity)",
				__func__);
		cflag &= ~PARENB;
		bits &= ~BITS_PARITY_MASK;
		cp210x_set_config(port, CP210X_SET_LINE_CTL, &bits, 2);
		break;
	case BITS_PARITY_SPACE:
		dbg("%s - parity = SPACE (not supported, disabling parity)",
				__func__);
		cflag &= ~PARENB;
		bits &= ~BITS_PARITY_MASK;
		cp210x_set_config(port, CP210X_SET_LINE_CTL, &bits, 2);
		break;
	default:
		dbg("%s - Unknown parity mode, disabling parity", __func__);
		cflag &= ~PARENB;
		bits &= ~BITS_PARITY_MASK;
		cp210x_set_config(port, CP210X_SET_LINE_CTL, &bits, 2);
		break;
	}

	cflag &= ~CSTOPB;
	switch (bits & BITS_STOP_MASK) {
	case BITS_STOP_1:
		dbg("%s - stop bits = 1", __func__);
		break;
	case BITS_STOP_1_5:
		dbg("%s - stop bits = 1.5 (not supported, using 1 stop bit)",
								__func__);
		bits &= ~BITS_STOP_MASK;
		cp210x_set_config(port, CP210X_SET_LINE_CTL, &bits, 2);
		break;
	case BITS_STOP_2:
		dbg("%s - stop bits = 2", __func__);
		cflag |= CSTOPB;
		break;
	default:
		dbg("%s - Unknown number of stop bits, using 1 stop bit",
								__func__);
		bits &= ~BITS_STOP_MASK;
		cp210x_set_config(port, CP210X_SET_LINE_CTL, &bits, 2);
		break;
	}

	cp210x_get_config(port, CP210X_GET_FLOW, modem_ctl, 16);
	if (modem_ctl[0] & 0x0008) {
		dbg("%s - flow control = CRTSCTS", __func__);
		cflag |= CRTSCTS;
	} else {
		dbg("%s - flow control = NONE", __func__);
		cflag &= ~CRTSCTS;
	}

	*cflagp = cflag;
}

static void cp210x_set_termios(struct tty_struct *tty,
		struct usb_serial_port *port, struct ktermios *old_termios)
{
	unsigned int cflag, old_cflag;
	unsigned int baud = 0, bits;
	unsigned int modem_ctl[4];

	dbg("%s - port %d", __func__, port->number);

	if (!tty)
		return;

	tty->termios->c_cflag &= ~CMSPAR;
	cflag = tty->termios->c_cflag;
	old_cflag = old_termios->c_cflag;
	baud = cp210x_quantise_baudrate(tty_get_baud_rate(tty));

	
	if (baud != tty_termios_baud_rate(old_termios) && baud != 0) {
		dbg("%s - Setting baud rate to %d baud", __func__,
				baud);
		if (cp210x_set_config_single(port, CP210X_SET_BAUDDIV,
					((BAUD_RATE_GEN_FREQ + baud/2) / baud))) {
			dbg("Baud rate requested not supported by device\n");
			baud = tty_termios_baud_rate(old_termios);
		}
	}
	
	tty_encode_baud_rate(tty, baud, baud);

	
	if ((cflag & CSIZE) != (old_cflag & CSIZE)) {
		cp210x_get_config(port, CP210X_GET_LINE_CTL, &bits, 2);
		bits &= ~BITS_DATA_MASK;
		switch (cflag & CSIZE) {
		case CS5:
			bits |= BITS_DATA_5;
			dbg("%s - data bits = 5", __func__);
			break;
		case CS6:
			bits |= BITS_DATA_6;
			dbg("%s - data bits = 6", __func__);
			break;
		case CS7:
			bits |= BITS_DATA_7;
			dbg("%s - data bits = 7", __func__);
			break;
		case CS8:
			bits |= BITS_DATA_8;
			dbg("%s - data bits = 8", __func__);
			break;
		
		default:
			dbg("cp210x driver does not "
					"support the number of bits requested,"
					" using 8 bit mode\n");
				bits |= BITS_DATA_8;
				break;
		}
		if (cp210x_set_config(port, CP210X_SET_LINE_CTL, &bits, 2))
			dbg("Number of data bits requested "
					"not supported by device\n");
	}

	if ((cflag & (PARENB|PARODD)) != (old_cflag & (PARENB|PARODD))) {
		cp210x_get_config(port, CP210X_GET_LINE_CTL, &bits, 2);
		bits &= ~BITS_PARITY_MASK;
		if (cflag & PARENB) {
			if (cflag & PARODD) {
				bits |= BITS_PARITY_ODD;
				dbg("%s - parity = ODD", __func__);
			} else {
				bits |= BITS_PARITY_EVEN;
				dbg("%s - parity = EVEN", __func__);
			}
		}
		if (cp210x_set_config(port, CP210X_SET_LINE_CTL, &bits, 2))
			dbg("Parity mode not supported "
					"by device\n");
	}

	if ((cflag & CSTOPB) != (old_cflag & CSTOPB)) {
		cp210x_get_config(port, CP210X_GET_LINE_CTL, &bits, 2);
		bits &= ~BITS_STOP_MASK;
		if (cflag & CSTOPB) {
			bits |= BITS_STOP_2;
			dbg("%s - stop bits = 2", __func__);
		} else {
			bits |= BITS_STOP_1;
			dbg("%s - stop bits = 1", __func__);
		}
		if (cp210x_set_config(port, CP210X_SET_LINE_CTL, &bits, 2))
			dbg("Number of stop bits requested "
					"not supported by device\n");
	}

	if ((cflag & CRTSCTS) != (old_cflag & CRTSCTS)) {
		cp210x_get_config(port, CP210X_GET_FLOW, modem_ctl, 16);
		dbg("%s - read modem controls = 0x%.4x 0x%.4x 0x%.4x 0x%.4x",
				__func__, modem_ctl[0], modem_ctl[1],
				modem_ctl[2], modem_ctl[3]);

		if (cflag & CRTSCTS) {
			modem_ctl[0] &= ~0x7B;
			modem_ctl[0] |= 0x09;
			modem_ctl[1] = 0x80;
			dbg("%s - flow control = CRTSCTS", __func__);
		} else {
			modem_ctl[0] &= ~0x7B;
			modem_ctl[0] |= 0x01;
			modem_ctl[1] |= 0x40;
			dbg("%s - flow control = NONE", __func__);
		}

		dbg("%s - write modem controls = 0x%.4x 0x%.4x 0x%.4x 0x%.4x",
				__func__, modem_ctl[0], modem_ctl[1],
				modem_ctl[2], modem_ctl[3]);
		cp210x_set_config(port, CP210X_SET_FLOW, modem_ctl, 16);
	}

}

static int cp210x_tiocmset (struct tty_struct *tty, struct file *file,
		unsigned int set, unsigned int clear)
{
	struct usb_serial_port *port = tty->driver_data;
	return cp210x_tiocmset_port(port, file, set, clear);
}

static int cp210x_tiocmset_port(struct usb_serial_port *port, struct file *file,
		unsigned int set, unsigned int clear)
{
	unsigned int control = 0;

	dbg("%s - port %d", __func__, port->number);

	if (set & TIOCM_RTS) {
		control |= CONTROL_RTS;
		control |= CONTROL_WRITE_RTS;
	}
	if (set & TIOCM_DTR) {
		control |= CONTROL_DTR;
		control |= CONTROL_WRITE_DTR;
	}
	if (clear & TIOCM_RTS) {
		control &= ~CONTROL_RTS;
		control |= CONTROL_WRITE_RTS;
	}
	if (clear & TIOCM_DTR) {
		control &= ~CONTROL_DTR;
		control |= CONTROL_WRITE_DTR;
	}

	dbg("%s - control = 0x%.4x", __func__, control);

	return cp210x_set_config(port, CP210X_SET_MHS, &control, 2);
}

static void cp210x_dtr_rts(struct usb_serial_port *p, int on)
{
	if (on)
		cp210x_tiocmset_port(p, NULL,  TIOCM_DTR|TIOCM_RTS, 0);
	else
		cp210x_tiocmset_port(p, NULL,  0, TIOCM_DTR|TIOCM_RTS);
}

static int cp210x_tiocmget (struct tty_struct *tty, struct file *file)
{
	struct usb_serial_port *port = tty->driver_data;
	unsigned int control;
	int result;

	dbg("%s - port %d", __func__, port->number);

	cp210x_get_config(port, CP210X_GET_MDMSTS, &control, 1);

	result = ((control & CONTROL_DTR) ? TIOCM_DTR : 0)
		|((control & CONTROL_RTS) ? TIOCM_RTS : 0)
		|((control & CONTROL_CTS) ? TIOCM_CTS : 0)
		|((control & CONTROL_DSR) ? TIOCM_DSR : 0)
		|((control & CONTROL_RING)? TIOCM_RI  : 0)
		|((control & CONTROL_DCD) ? TIOCM_CD  : 0);

	dbg("%s - control = 0x%.2x", __func__, control);

	return result;
}

static int cp210x_carrier_raised(struct usb_serial_port *p)
{
	unsigned int control;
	cp210x_get_config(p, CP210X_GET_MDMSTS, &control, 1);
	if (control & CONTROL_DCD)
		return 1;
	return 0;
}

static void cp210x_break_ctl (struct tty_struct *tty, int break_state)
{
	struct usb_serial_port *port = tty->driver_data;
	unsigned int state;

	dbg("%s - port %d", __func__, port->number);
	if (break_state == 0)
		state = BREAK_OFF;
	else
		state = BREAK_ON;
	dbg("%s - turning break %s", __func__,
			state == BREAK_OFF ? "off" : "on");
	cp210x_set_config(port, CP210X_SET_BREAK, &state, 2);
}

static int cp210x_startup(struct usb_serial *serial)
{
	
	usb_reset_device(serial->dev);
	return 0;
}

static void cp210x_disconnect(struct usb_serial *serial)
{
	int i;

	dbg("%s", __func__);

	
	for (i = 0; i < serial->num_ports; ++i)
		cp210x_cleanup(serial->port[i]);
}

static int __init cp210x_init(void)
{
	int retval;

	retval = usb_serial_register(&cp210x_device);
	if (retval)
		return retval; 

	retval = usb_register(&cp210x_driver);
	if (retval) {
		
		usb_serial_deregister(&cp210x_device);
		return retval;
	}

	
	printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
	       DRIVER_DESC "\n");
	return 0;
}

static void __exit cp210x_exit(void)
{
	usb_deregister(&cp210x_driver);
	usb_serial_deregister(&cp210x_device);
}

module_init(cp210x_init);
module_exit(cp210x_exit);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Enable verbose debugging messages");
