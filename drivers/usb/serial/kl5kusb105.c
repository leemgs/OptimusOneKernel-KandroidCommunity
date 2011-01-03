






#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <asm/unaligned.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>
#include "kl5kusb105.h"

static int debug;


#define DRIVER_VERSION "v0.3a"
#define DRIVER_AUTHOR "Utz-Uwe Haus <haus@uuhaus.de>"
#define DRIVER_DESC "KLSI KL5KUSB105 chipset USB->Serial Converter driver"



static int  klsi_105_startup(struct usb_serial *serial);
static void klsi_105_disconnect(struct usb_serial *serial);
static void klsi_105_release(struct usb_serial *serial);
static int  klsi_105_open(struct tty_struct *tty, struct usb_serial_port *port);
static void klsi_105_close(struct usb_serial_port *port);
static int  klsi_105_write(struct tty_struct *tty,
	struct usb_serial_port *port, const unsigned char *buf, int count);
static void klsi_105_write_bulk_callback(struct urb *urb);
static int  klsi_105_chars_in_buffer(struct tty_struct *tty);
static int  klsi_105_write_room(struct tty_struct *tty);
static void klsi_105_read_bulk_callback(struct urb *urb);
static void klsi_105_set_termios(struct tty_struct *tty,
			struct usb_serial_port *port, struct ktermios *old);
static void klsi_105_throttle(struct tty_struct *tty);
static void klsi_105_unthrottle(struct tty_struct *tty);
static int  klsi_105_tiocmget(struct tty_struct *tty, struct file *file);
static int  klsi_105_tiocmset(struct tty_struct *tty, struct file *file,
			unsigned int set, unsigned int clear);


static struct usb_device_id id_table [] = {
	{ USB_DEVICE(PALMCONNECT_VID, PALMCONNECT_PID) },
	{ USB_DEVICE(KLSI_VID, KLSI_KL5KUSB105D_PID) },
	{ }		
};

MODULE_DEVICE_TABLE(usb, id_table);

static struct usb_driver kl5kusb105d_driver = {
	.name =		"kl5kusb105d",
	.probe =	usb_serial_probe,
	.disconnect =	usb_serial_disconnect,
	.id_table =	id_table,
	.no_dynamic_id = 	1,
};

static struct usb_serial_driver kl5kusb105d_device = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"kl5kusb105d",
	},
	.description =	     "KL5KUSB105D / PalmConnect",
	.usb_driver =	     &kl5kusb105d_driver,
	.id_table =	     id_table,
	.num_ports =	     1,
	.open =		     klsi_105_open,
	.close =	     klsi_105_close,
	.write =	     klsi_105_write,
	.write_bulk_callback = klsi_105_write_bulk_callback,
	.chars_in_buffer =   klsi_105_chars_in_buffer,
	.write_room =        klsi_105_write_room,
	.read_bulk_callback = klsi_105_read_bulk_callback,
	.set_termios =	     klsi_105_set_termios,
	
	.tiocmget =          klsi_105_tiocmget,
	.tiocmset =          klsi_105_tiocmset,
	.attach =	     klsi_105_startup,
	.disconnect =	     klsi_105_disconnect,
	.release =	     klsi_105_release,
	.throttle =	     klsi_105_throttle,
	.unthrottle =	     klsi_105_unthrottle,
};

struct klsi_105_port_settings {
	__u8	pktlen;		
	__u8	baudrate;
	__u8	databits;
	__u8	unknown1;
	__u8	unknown2;
} __attribute__ ((packed));


#define NUM_URBS			1
#define URB_TRANSFER_BUFFER_SIZE	64
struct klsi_105_private {
	struct klsi_105_port_settings	cfg;
	struct ktermios			termios;
	unsigned long			line_state; 
	
	struct urb			*write_urb_pool[NUM_URBS];
	spinlock_t			lock;
	unsigned long			bytes_in;
	unsigned long			bytes_out;
};





#define KLSI_TIMEOUT	 5000 

static int klsi_105_chg_port_settings(struct usb_serial_port *port,
				      struct klsi_105_port_settings *settings)
{
	int rc;

	rc = usb_control_msg(port->serial->dev,
			usb_sndctrlpipe(port->serial->dev, 0),
			KL5KUSB105A_SIO_SET_DATA,
			USB_TYPE_VENDOR | USB_DIR_OUT | USB_RECIP_INTERFACE,
			0, 
			0, 
			settings,
			sizeof(struct klsi_105_port_settings),
			KLSI_TIMEOUT);
	if (rc < 0)
		dev_err(&port->dev,
			"Change port settings failed (error = %d)\n", rc);
	dev_info(&port->serial->dev->dev,
		 "%d byte block, baudrate %x, databits %d, u1 %d, u2 %d\n",
		 settings->pktlen, settings->baudrate, settings->databits,
		 settings->unknown1, settings->unknown2);
	return rc;
} 


static unsigned long klsi_105_status2linestate(const __u16 status)
{
	unsigned long res = 0;

	res =   ((status & KL5KUSB105A_DSR) ? TIOCM_DSR : 0)
	      | ((status & KL5KUSB105A_CTS) ? TIOCM_CTS : 0)
	      ;

	return res;
}


#define KLSI_STATUSBUF_LEN	2
static int klsi_105_get_line_state(struct usb_serial_port *port,
				   unsigned long *line_state_p)
{
	int rc;
	__u8 status_buf[KLSI_STATUSBUF_LEN] = { -1, -1};
	__u16 status;

	dev_info(&port->serial->dev->dev, "sending SIO Poll request\n");
	rc = usb_control_msg(port->serial->dev,
			     usb_rcvctrlpipe(port->serial->dev, 0),
			     KL5KUSB105A_SIO_POLL,
			     USB_TYPE_VENDOR | USB_DIR_IN,
			     0, 
			     0, 
			     status_buf, KLSI_STATUSBUF_LEN,
			     10000
			     );
	if (rc < 0)
		dev_err(&port->dev, "Reading line status failed (error = %d)\n",
			rc);
	else {
		status = get_unaligned_le16(status_buf);

		dev_info(&port->serial->dev->dev, "read status %x %x",
			 status_buf[0], status_buf[1]);

		*line_state_p = klsi_105_status2linestate(status);
	}
	return rc;
}




static int klsi_105_startup(struct usb_serial *serial)
{
	struct klsi_105_private *priv;
	int i, j;

	

	
	for (i = 0; i < serial->num_ports; i++) {
		priv = kmalloc(sizeof(struct klsi_105_private),
						   GFP_KERNEL);
		if (!priv) {
			dbg("%skmalloc for klsi_105_private failed.", __func__);
			i--;
			goto err_cleanup;
		}
		
		priv->cfg.pktlen    = 5;
		priv->cfg.baudrate  = kl5kusb105a_sio_b9600;
		priv->cfg.databits  = kl5kusb105a_dtb_8;
		priv->cfg.unknown1  = 0;
		priv->cfg.unknown2  = 1;

		priv->line_state    = 0;

		priv->bytes_in	    = 0;
		priv->bytes_out	    = 0;
		usb_set_serial_port_data(serial->port[i], priv);

		spin_lock_init(&priv->lock);
		for (j = 0; j < NUM_URBS; j++) {
			struct urb *urb = usb_alloc_urb(0, GFP_KERNEL);

			priv->write_urb_pool[j] = urb;
			if (urb == NULL) {
				dev_err(&serial->dev->dev, "No more urbs???\n");
				goto err_cleanup;
			}

			urb->transfer_buffer =
				kmalloc(URB_TRANSFER_BUFFER_SIZE, GFP_KERNEL);
			if (!urb->transfer_buffer) {
				dev_err(&serial->dev->dev,
					"%s - out of memory for urb buffers.\n",
					__func__);
				goto err_cleanup;
			}
		}

		
		init_waitqueue_head(&serial->port[i]->write_wait);
	}

	return 0;

err_cleanup:
	for (; i >= 0; i--) {
		priv = usb_get_serial_port_data(serial->port[i]);
		for (j = 0; j < NUM_URBS; j++) {
			if (priv->write_urb_pool[j]) {
				kfree(priv->write_urb_pool[j]->transfer_buffer);
				usb_free_urb(priv->write_urb_pool[j]);
			}
		}
		usb_set_serial_port_data(serial->port[i], NULL);
	}
	return -ENOMEM;
} 


static void klsi_105_disconnect(struct usb_serial *serial)
{
	int i;

	dbg("%s", __func__);

	
	for (i = 0; i < serial->num_ports; ++i) {
		struct klsi_105_private *priv =
				usb_get_serial_port_data(serial->port[i]);

		if (priv) {
			
			int j;
			struct urb **write_urbs = priv->write_urb_pool;

			for (j = 0; j < NUM_URBS; j++) {
				if (write_urbs[j]) {
					usb_kill_urb(write_urbs[j]);
					usb_free_urb(write_urbs[j]);
				}
			}
		}
	}
} 


static void klsi_105_release(struct usb_serial *serial)
{
	int i;

	dbg("%s", __func__);

	for (i = 0; i < serial->num_ports; ++i) {
		struct klsi_105_private *priv =
				usb_get_serial_port_data(serial->port[i]);

		kfree(priv);
	}
} 

static int  klsi_105_open(struct tty_struct *tty, struct usb_serial_port *port)
{
	struct klsi_105_private *priv = usb_get_serial_port_data(port);
	int retval = 0;
	int rc;
	int i;
	unsigned long line_state;
	struct klsi_105_port_settings cfg;
	unsigned long flags;

	dbg("%s port %d", __func__, port->number);

	
	cfg.pktlen   = 5;
	cfg.baudrate = kl5kusb105a_sio_b9600;
	cfg.databits = kl5kusb105a_dtb_8;
	cfg.unknown1 = 0;
	cfg.unknown2 = 1;
	klsi_105_chg_port_settings(port, &cfg);

	
	spin_lock_irqsave(&priv->lock, flags);
	priv->termios.c_iflag = tty->termios->c_iflag;
	priv->termios.c_oflag = tty->termios->c_oflag;
	priv->termios.c_cflag = tty->termios->c_cflag;
	priv->termios.c_lflag = tty->termios->c_lflag;
	for (i = 0; i < NCCS; i++)
		priv->termios.c_cc[i] = tty->termios->c_cc[i];
	priv->cfg.pktlen   = cfg.pktlen;
	priv->cfg.baudrate = cfg.baudrate;
	priv->cfg.databits = cfg.databits;
	priv->cfg.unknown1 = cfg.unknown1;
	priv->cfg.unknown2 = cfg.unknown2;
	spin_unlock_irqrestore(&priv->lock, flags);

	
	usb_fill_bulk_urb(port->read_urb, port->serial->dev,
		      usb_rcvbulkpipe(port->serial->dev,
				      port->bulk_in_endpointAddress),
		      port->read_urb->transfer_buffer,
		      port->read_urb->transfer_buffer_length,
		      klsi_105_read_bulk_callback,
		      port);

	rc = usb_submit_urb(port->read_urb, GFP_KERNEL);
	if (rc) {
		dev_err(&port->dev, "%s - failed submitting read urb, "
			"error %d\n", __func__, rc);
		retval = rc;
		goto exit;
	}

	rc = usb_control_msg(port->serial->dev,
			     usb_sndctrlpipe(port->serial->dev, 0),
			     KL5KUSB105A_SIO_CONFIGURE,
			     USB_TYPE_VENDOR|USB_DIR_OUT|USB_RECIP_INTERFACE,
			     KL5KUSB105A_SIO_CONFIGURE_READ_ON,
			     0, 
			     NULL,
			     0,
			     KLSI_TIMEOUT);
	if (rc < 0) {
		dev_err(&port->dev, "Enabling read failed (error = %d)\n", rc);
		retval = rc;
	} else
		dbg("%s - enabled reading", __func__);

	rc = klsi_105_get_line_state(port, &line_state);
	if (rc >= 0) {
		spin_lock_irqsave(&priv->lock, flags);
		priv->line_state = line_state;
		spin_unlock_irqrestore(&priv->lock, flags);
		dbg("%s - read line state 0x%lx", __func__, line_state);
		retval = 0;
	} else
		retval = rc;

exit:
	return retval;
} 


static void klsi_105_close(struct usb_serial_port *port)
{
	struct klsi_105_private *priv = usb_get_serial_port_data(port);
	int rc;

	dbg("%s port %d", __func__, port->number);

	mutex_lock(&port->serial->disc_mutex);
	if (!port->serial->disconnected) {
		
		rc = usb_control_msg(port->serial->dev,
				     usb_sndctrlpipe(port->serial->dev, 0),
				     KL5KUSB105A_SIO_CONFIGURE,
				     USB_TYPE_VENDOR | USB_DIR_OUT,
				     KL5KUSB105A_SIO_CONFIGURE_READ_OFF,
				     0, 
				     NULL, 0,
				     KLSI_TIMEOUT);
		if (rc < 0)
			dev_err(&port->dev,
				"Disabling read failed (error = %d)\n", rc);
	}
	mutex_unlock(&port->serial->disc_mutex);

	
	usb_kill_urb(port->write_urb);
	usb_kill_urb(port->read_urb);
	
	
	
	usb_kill_urb(port->interrupt_in_urb);
	dev_info(&port->serial->dev->dev,
		 "port stats: %ld bytes in, %ld bytes out\n",
		 priv->bytes_in, priv->bytes_out);
} 



#define KLSI_105_DATA_OFFSET	2   


static int klsi_105_write(struct tty_struct *tty,
	struct usb_serial_port *port, const unsigned char *buf, int count)
{
	struct klsi_105_private *priv = usb_get_serial_port_data(port);
	int result, size;
	int bytes_sent = 0;

	dbg("%s - port %d", __func__, port->number);

	while (count > 0) {
		
		struct urb *urb = NULL;
		unsigned long flags;
		int i;
		
		spin_lock_irqsave(&priv->lock, flags);
		for (i = 0; i < NUM_URBS; i++) {
			if (priv->write_urb_pool[i]->status != -EINPROGRESS) {
				urb = priv->write_urb_pool[i];
				dbg("%s - using pool URB %d", __func__, i);
				break;
			}
		}
		spin_unlock_irqrestore(&priv->lock, flags);

		if (urb == NULL) {
			dbg("%s - no more free urbs", __func__);
			goto exit;
		}

		if (urb->transfer_buffer == NULL) {
			urb->transfer_buffer =
				kmalloc(URB_TRANSFER_BUFFER_SIZE, GFP_ATOMIC);
			if (urb->transfer_buffer == NULL) {
				dev_err(&port->dev,
					"%s - no more kernel memory...\n",
					__func__);
				goto exit;
			}
		}

		size = min(count, port->bulk_out_size - KLSI_105_DATA_OFFSET);
		size = min(size, URB_TRANSFER_BUFFER_SIZE -
							KLSI_105_DATA_OFFSET);

		memcpy(urb->transfer_buffer + KLSI_105_DATA_OFFSET, buf, size);

		
		((__u8 *)urb->transfer_buffer)[0] = (__u8) (size & 0xFF);
		((__u8 *)urb->transfer_buffer)[1] = (__u8) ((size & 0xFF00)>>8);

		
		usb_fill_bulk_urb(urb, port->serial->dev,
			      usb_sndbulkpipe(port->serial->dev,
					      port->bulk_out_endpointAddress),
			      urb->transfer_buffer,
			      URB_TRANSFER_BUFFER_SIZE,
			      klsi_105_write_bulk_callback,
			      port);

		
		result = usb_submit_urb(urb, GFP_ATOMIC);
		if (result) {
			dev_err(&port->dev,
				"%s - failed submitting write urb, error %d\n",
				__func__, result);
			goto exit;
		}
		buf += size;
		bytes_sent += size;
		count -= size;
	}
exit:
	
	priv->bytes_out += bytes_sent;

	return bytes_sent;	
} 

static void klsi_105_write_bulk_callback(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	int status = urb->status;

	dbg("%s - port %d", __func__, port->number);

	if (status) {
		dbg("%s - nonzero write bulk status received: %d", __func__,
		    status);
		return;
	}

	usb_serial_port_softint(port);
} 



static int klsi_105_chars_in_buffer(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	int chars = 0;
	int i;
	unsigned long flags;
	struct klsi_105_private *priv = usb_get_serial_port_data(port);

	spin_lock_irqsave(&priv->lock, flags);

	for (i = 0; i < NUM_URBS; ++i) {
		if (priv->write_urb_pool[i]->status == -EINPROGRESS)
			chars += URB_TRANSFER_BUFFER_SIZE;
	}

	spin_unlock_irqrestore(&priv->lock, flags);

	dbg("%s - returns %d", __func__, chars);
	return chars;
}

static int klsi_105_write_room(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	unsigned long flags;
	int i;
	int room = 0;
	struct klsi_105_private *priv = usb_get_serial_port_data(port);

	spin_lock_irqsave(&priv->lock, flags);
	for (i = 0; i < NUM_URBS; ++i) {
		if (priv->write_urb_pool[i]->status != -EINPROGRESS)
			room += URB_TRANSFER_BUFFER_SIZE;
	}

	spin_unlock_irqrestore(&priv->lock, flags);

	dbg("%s - returns %d", __func__, room);
	return room;
}



static void klsi_105_read_bulk_callback(struct urb *urb)
{
	struct usb_serial_port *port = urb->context;
	struct klsi_105_private *priv = usb_get_serial_port_data(port);
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int rc;
	int status = urb->status;

	dbg("%s - port %d", __func__, port->number);

	
	if (status) {
		dbg("%s - nonzero read bulk status received: %d", __func__,
		    status);
		return;
	}

	
	if (urb->actual_length == 0) {
		
		
	       ;
	} else if (urb->actual_length <= 2) {
		dbg("%s - size %d URB not understood", __func__,
		    urb->actual_length);
		usb_serial_debug_data(debug, &port->dev, __func__,
				      urb->actual_length, data);
	} else {
		int bytes_sent = ((__u8 *) data)[0] +
				 ((unsigned int) ((__u8 *) data)[1] << 8);
		tty = tty_port_tty_get(&port->port);
		
		usb_serial_debug_data(debug, &port->dev, __func__,
				      urb->actual_length, data);

		if (bytes_sent + 2 > urb->actual_length) {
			dbg("%s - trying to read more data than available"
			    " (%d vs. %d)", __func__,
			    bytes_sent+2, urb->actual_length);
			
			bytes_sent = urb->actual_length - 2;
		}

		tty_buffer_request_room(tty, bytes_sent);
		tty_insert_flip_string(tty, data + 2, bytes_sent);
		tty_flip_buffer_push(tty);
		tty_kref_put(tty);

		
		priv->bytes_in += bytes_sent;
	}
	
	usb_fill_bulk_urb(port->read_urb, port->serial->dev,
		      usb_rcvbulkpipe(port->serial->dev,
				      port->bulk_in_endpointAddress),
		      port->read_urb->transfer_buffer,
		      port->read_urb->transfer_buffer_length,
		      klsi_105_read_bulk_callback,
		      port);
	rc = usb_submit_urb(port->read_urb, GFP_ATOMIC);
	if (rc)
		dev_err(&port->dev,
			"%s - failed resubmitting read urb, error %d\n",
			__func__, rc);
} 


static void klsi_105_set_termios(struct tty_struct *tty,
				 struct usb_serial_port *port,
				 struct ktermios *old_termios)
{
	struct klsi_105_private *priv = usb_get_serial_port_data(port);
	unsigned int iflag = tty->termios->c_iflag;
	unsigned int old_iflag = old_termios->c_iflag;
	unsigned int cflag = tty->termios->c_cflag;
	unsigned int old_cflag = old_termios->c_cflag;
	struct klsi_105_port_settings cfg;
	unsigned long flags;
	speed_t baud;

	
	spin_lock_irqsave(&priv->lock, flags);

	
	baud = tty_get_baud_rate(tty);

	if ((cflag & CBAUD) != (old_cflag & CBAUD)) {
		
		if ((old_cflag & CBAUD) == B0) {
			dbg("%s: baud was B0", __func__);
#if 0
			priv->control_state |= TIOCM_DTR;
			
			if (!(old_cflag & CRTSCTS))
				priv->control_state |= TIOCM_RTS;
			mct_u232_set_modem_ctrl(serial, priv->control_state);
#endif
		}
	}
	switch (baud) {
	case 0: 
		break;
	case 1200:
		priv->cfg.baudrate = kl5kusb105a_sio_b1200;
		break;
	case 2400:
		priv->cfg.baudrate = kl5kusb105a_sio_b2400;
		break;
	case 4800:
		priv->cfg.baudrate = kl5kusb105a_sio_b4800;
		break;
	case 9600:
		priv->cfg.baudrate = kl5kusb105a_sio_b9600;
		break;
	case 19200:
		priv->cfg.baudrate = kl5kusb105a_sio_b19200;
		break;
	case 38400:
		priv->cfg.baudrate = kl5kusb105a_sio_b38400;
		break;
	case 57600:
		priv->cfg.baudrate = kl5kusb105a_sio_b57600;
		break;
	case 115200:
		priv->cfg.baudrate = kl5kusb105a_sio_b115200;
		break;
	default:
		dbg("KLSI USB->Serial converter:"
		    " unsupported baudrate request, using default of 9600");
			priv->cfg.baudrate = kl5kusb105a_sio_b9600;
		baud = 9600;
		break;
	}
	if ((cflag & CBAUD) == B0) {
		dbg("%s: baud is B0", __func__);
		
		
		;
#if 0
		priv->control_state &= ~(TIOCM_DTR | TIOCM_RTS);
		mct_u232_set_modem_ctrl(serial, priv->control_state);
#endif
	}
	tty_encode_baud_rate(tty, baud, baud);

	if ((cflag & CSIZE) != (old_cflag & CSIZE)) {
		
		switch (cflag & CSIZE) {
		case CS5:
			dbg("%s - 5 bits/byte not supported", __func__);
			spin_unlock_irqrestore(&priv->lock, flags);
			return ;
		case CS6:
			dbg("%s - 6 bits/byte not supported", __func__);
			spin_unlock_irqrestore(&priv->lock, flags);
			return ;
		case CS7:
			priv->cfg.databits = kl5kusb105a_dtb_7;
			break;
		case CS8:
			priv->cfg.databits = kl5kusb105a_dtb_8;
			break;
		default:
			dev_err(&port->dev,
				"CSIZE was not CS5-CS8, using default of 8\n");
			priv->cfg.databits = kl5kusb105a_dtb_8;
			break;
		}
	}

	
	if ((cflag & (PARENB|PARODD)) != (old_cflag & (PARENB|PARODD))
	    || (cflag & CSTOPB) != (old_cflag & CSTOPB)) {
		
		tty->termios->c_cflag &= ~(PARENB|PARODD|CSTOPB);
#if 0
		priv->last_lcr = 0;

		
		if (cflag & PARENB)
			priv->last_lcr |= (cflag & PARODD) ?
				MCT_U232_PARITY_ODD : MCT_U232_PARITY_EVEN;
		else
			priv->last_lcr |= MCT_U232_PARITY_NONE;

		
		priv->last_lcr |= (cflag & CSTOPB) ?
			MCT_U232_STOP_BITS_2 : MCT_U232_STOP_BITS_1;

		mct_u232_set_line_ctrl(serial, priv->last_lcr);
#endif
		;
	}
	
	if ((iflag & IXOFF) != (old_iflag & IXOFF)
	    || (iflag & IXON) != (old_iflag & IXON)
	    ||  (cflag & CRTSCTS) != (old_cflag & CRTSCTS)) {
		
		tty->termios->c_cflag &= ~CRTSCTS;
		
#if 0
		if ((iflag & IXOFF) || (iflag & IXON) || (cflag & CRTSCTS))
			priv->control_state |= TIOCM_DTR | TIOCM_RTS;
		else
			priv->control_state &= ~(TIOCM_DTR | TIOCM_RTS);
		mct_u232_set_modem_ctrl(serial, priv->control_state);
#endif
		;
	}
	memcpy(&cfg, &priv->cfg, sizeof(cfg));
	spin_unlock_irqrestore(&priv->lock, flags);

	
	klsi_105_chg_port_settings(port, &cfg);
} 


#if 0
static void mct_u232_break_ctl(struct tty_struct *tty, int break_state)
{
	struct usb_serial_port *port = tty->driver_data;
	struct usb_serial *serial = port->serial;
	struct mct_u232_private *priv =
				(struct mct_u232_private *)port->private;
	unsigned char lcr = priv->last_lcr;

	dbg("%sstate=%d", __func__, break_state);

	
	if (break_state)
		lcr |= MCT_U232_SET_BREAK;

	mct_u232_set_line_ctrl(serial, lcr);
} 
#endif

static int klsi_105_tiocmget(struct tty_struct *tty, struct file *file)
{
	struct usb_serial_port *port = tty->driver_data;
	struct klsi_105_private *priv = usb_get_serial_port_data(port);
	unsigned long flags;
	int rc;
	unsigned long line_state;
	dbg("%s - request, just guessing", __func__);

	rc = klsi_105_get_line_state(port, &line_state);
	if (rc < 0) {
		dev_err(&port->dev,
			"Reading line control failed (error = %d)\n", rc);
		
		return rc;
	}

	spin_lock_irqsave(&priv->lock, flags);
	priv->line_state = line_state;
	spin_unlock_irqrestore(&priv->lock, flags);
	dbg("%s - read line state 0x%lx", __func__, line_state);
	return (int)line_state;
}

static int klsi_105_tiocmset(struct tty_struct *tty, struct file *file,
			     unsigned int set, unsigned int clear)
{
	int retval = -EINVAL;

	dbg("%s", __func__);


	return retval;
}

static void klsi_105_throttle(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	dbg("%s - port %d", __func__, port->number);
	usb_kill_urb(port->read_urb);
}

static void klsi_105_unthrottle(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	int result;

	dbg("%s - port %d", __func__, port->number);

	port->read_urb->dev = port->serial->dev;
	result = usb_submit_urb(port->read_urb, GFP_KERNEL);
	if (result)
		dev_err(&port->dev,
			"%s - failed submitting read urb, error %d\n",
			__func__, result);
}



static int __init klsi_105_init(void)
{
	int retval;
	retval = usb_serial_register(&kl5kusb105d_device);
	if (retval)
		goto failed_usb_serial_register;
	retval = usb_register(&kl5kusb105d_driver);
	if (retval)
		goto failed_usb_register;

	printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
	       DRIVER_DESC "\n");
	return 0;
failed_usb_register:
	usb_serial_deregister(&kl5kusb105d_device);
failed_usb_serial_register:
	return retval;
}


static void __exit klsi_105_exit(void)
{
	usb_deregister(&kl5kusb105d_driver);
	usb_serial_deregister(&kl5kusb105d_device);
}


module_init(klsi_105_init);
module_exit(klsi_105_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");


module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "enable extensive debugging messages");


