
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/serial.h>
#include <linux/serial_reg.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>
#include <linux/uaccess.h>



#define DRIVER_VERSION "1.0.0.4F"
#define DRIVER_AUTHOR "Aspire Communications pvt Ltd."
#define DRIVER_DESC "Moschip USB Serial Driver"


#define MOS_WDR_TIMEOUT	(HZ * 5)

#define MOS_PORT1	0x0200
#define MOS_PORT2	0x0300
#define MOS_VENREG	0x0000
#define MOS_MAX_PORT	0x02
#define MOS_WRITE	0x0E
#define MOS_READ	0x0D


#define SERIAL_IIR_RLS	0x06
#define SERIAL_IIR_RDA	0x04
#define SERIAL_IIR_CTI	0x0c
#define SERIAL_IIR_THR	0x02
#define SERIAL_IIR_MS	0x00

#define NUM_URBS			16	
#define URB_TRANSFER_BUFFER_SIZE	32	


struct moschip_port {
	__u8	shadowLCR;		
	__u8	shadowMCR;		
	__u8	shadowMSR;		
	char			open;
	struct async_icount	icount;
	struct usb_serial_port	*port;	
	struct urb		*write_urb_pool[NUM_URBS];
};


struct moschip_serial {
	int interrupt_started;
};

static int debug;

#define USB_VENDOR_ID_MOSCHIP		0x9710
#define MOSCHIP_DEVICE_ID_7720		0x7720
#define MOSCHIP_DEVICE_ID_7715		0x7715

static struct usb_device_id moschip_port_id_table[] = {
	{ USB_DEVICE(USB_VENDOR_ID_MOSCHIP, MOSCHIP_DEVICE_ID_7720) },
	{ } 
};
MODULE_DEVICE_TABLE(usb, moschip_port_id_table);



static void mos7720_interrupt_callback(struct urb *urb)
{
	int result;
	int length;
	int status = urb->status;
	__u8 *data;
	__u8 sp1;
	__u8 sp2;

	dbg("%s", " : Entering\n");

	switch (status) {
	case 0:
		
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		
		dbg("%s - urb shutting down with status: %d", __func__,
		    status);
		return;
	default:
		dbg("%s - nonzero urb status received: %d", __func__,
		    status);
		goto exit;
	}

	length = urb->actual_length;
	data = urb->transfer_buffer;

	

	

	if (unlikely(length != 4)) {
		dbg("Wrong data !!!");
		return;
	}

	sp1 = data[3];
	sp2 = data[2];

	if ((sp1 | sp2) & 0x01) {
		
		dbg("No Interrupt !!!");
	} else {
		switch (sp1 & 0x0f) {
		case SERIAL_IIR_RLS:
			dbg("Serial Port 1: Receiver status error or address "
			    "bit detected in 9-bit mode\n");
			break;
		case SERIAL_IIR_CTI:
			dbg("Serial Port 1: Receiver time out");
			break;
		case SERIAL_IIR_MS:
			dbg("Serial Port 1: Modem status change");
			break;
		}

		switch (sp2 & 0x0f) {
		case SERIAL_IIR_RLS:
			dbg("Serial Port 2: Receiver status error or address "
			    "bit detected in 9-bit mode");
			break;
		case SERIAL_IIR_CTI:
			dbg("Serial Port 2: Receiver time out");
			break;
		case SERIAL_IIR_MS:
			dbg("Serial Port 2: Modem status change");
			break;
		}
	}

exit:
	result = usb_submit_urb(urb, GFP_ATOMIC);
	if (result)
		dev_err(&urb->dev->dev,
			"%s - Error %d submitting control urb\n",
			__func__, result);
	return;
}


static void mos7720_bulk_in_callback(struct urb *urb)
{
	int retval;
	unsigned char *data ;
	struct usb_serial_port *port;
	struct moschip_port *mos7720_port;
	struct tty_struct *tty;
	int status = urb->status;

	if (status) {
		dbg("nonzero read bulk status received: %d", status);
		return;
	}

	mos7720_port = urb->context;
	if (!mos7720_port) {
		dbg("%s", "NULL mos7720_port pointer \n");
		return ;
	}

	port = mos7720_port->port;

	dbg("Entering...%s", __func__);

	data = urb->transfer_buffer;

	tty = tty_port_tty_get(&port->port);
	if (tty && urb->actual_length) {
		tty_buffer_request_room(tty, urb->actual_length);
		tty_insert_flip_string(tty, data, urb->actual_length);
		tty_flip_buffer_push(tty);
	}
	tty_kref_put(tty);

	if (!port->read_urb) {
		dbg("URB KILLED !!!");
		return;
	}

	if (port->read_urb->status != -EINPROGRESS) {
		port->read_urb->dev = port->serial->dev;

		retval = usb_submit_urb(port->read_urb, GFP_ATOMIC);
		if (retval)
			dbg("usb_submit_urb(read bulk) failed, retval = %d",
			    retval);
	}
}


static void mos7720_bulk_out_data_callback(struct urb *urb)
{
	struct moschip_port *mos7720_port;
	struct tty_struct *tty;
	int status = urb->status;

	if (status) {
		dbg("nonzero write bulk status received:%d", status);
		return;
	}

	mos7720_port = urb->context;
	if (!mos7720_port) {
		dbg("NULL mos7720_port pointer");
		return ;
	}

	dbg("Entering .........");

	tty = tty_port_tty_get(&mos7720_port->port->port);

	if (tty && mos7720_port->open)
		tty_wakeup(tty);
	tty_kref_put(tty);
}


static int send_mos_cmd(struct usb_serial *serial, __u8 request, __u16 value,
			__u16 index, void *data)
{
	int status;
	unsigned int pipe;
	u16 product = le16_to_cpu(serial->dev->descriptor.idProduct);
	__u8 requesttype;
	__u16 size = 0x0000;

	if (value < MOS_MAX_PORT) {
		if (product == MOSCHIP_DEVICE_ID_7715)
			value = value*0x100+0x100;
		else
			value = value*0x100+0x200;
	} else {
		value = 0x0000;
		if ((product == MOSCHIP_DEVICE_ID_7715) &&
		    (index != 0x08)) {
			dbg("serial->product== MOSCHIP_DEVICE_ID_7715");
			
		}
	}

	if (request == MOS_WRITE) {
		request = (__u8)MOS_WRITE;
		requesttype = (__u8)0x40;
		value  = value + (__u16)*((unsigned char *)data);
		data = NULL;
		pipe = usb_sndctrlpipe(serial->dev, 0);
	} else {
		request = (__u8)MOS_READ;
		requesttype = (__u8)0xC0;
		size = 0x01;
		pipe = usb_rcvctrlpipe(serial->dev, 0);
	}

	status = usb_control_msg(serial->dev, pipe, request, requesttype,
				 value, index, data, size, MOS_WDR_TIMEOUT);

	if (status < 0)
		dbg("Command Write failed Value %x index %x\n", value, index);

	return status;
}

static int mos7720_open(struct tty_struct *tty, struct usb_serial_port *port)
{
	struct usb_serial *serial;
	struct usb_serial_port *port0;
	struct urb *urb;
	struct moschip_serial *mos7720_serial;
	struct moschip_port *mos7720_port;
	int response;
	int port_number;
	char data;
	int allocated_urbs = 0;
	int j;

	serial = port->serial;

	mos7720_port = usb_get_serial_port_data(port);
	if (mos7720_port == NULL)
		return -ENODEV;

	port0 = serial->port[0];

	mos7720_serial = usb_get_serial_data(serial);

	if (mos7720_serial == NULL || port0 == NULL)
		return -ENODEV;

	usb_clear_halt(serial->dev, port->write_urb->pipe);
	usb_clear_halt(serial->dev, port->read_urb->pipe);

	
	for (j = 0; j < NUM_URBS; ++j) {
		urb = usb_alloc_urb(0, GFP_KERNEL);
		mos7720_port->write_urb_pool[j] = urb;

		if (urb == NULL) {
			dev_err(&port->dev, "No more urbs???\n");
			continue;
		}

		urb->transfer_buffer = kmalloc(URB_TRANSFER_BUFFER_SIZE,
					       GFP_KERNEL);
		if (!urb->transfer_buffer) {
			dev_err(&port->dev,
				"%s-out of memory for urb buffers.\n",
				__func__);
			usb_free_urb(mos7720_port->write_urb_pool[j]);
			mos7720_port->write_urb_pool[j] = NULL;
			continue;
		}
		allocated_urbs++;
	}

	if (!allocated_urbs)
		return -ENOMEM;

	 
	port_number = port->number - port->serial->minor;
	send_mos_cmd(port->serial, MOS_READ, port_number, UART_LSR, &data);
	dbg("SS::%p LSR:%x\n", mos7720_port, data);

	dbg("Check:Sending Command ..........");

	data = 0x02;
	send_mos_cmd(serial, MOS_WRITE, MOS_MAX_PORT, 0x01, &data);
	data = 0x02;
	send_mos_cmd(serial, MOS_WRITE, MOS_MAX_PORT, 0x02, &data);

	data = 0x00;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x01, &data);
	data = 0x00;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x02, &data);

	data = 0xCF;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x02, &data);
	data = 0x03;
	mos7720_port->shadowLCR  = data;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x03, &data);
	data = 0x0b;
	mos7720_port->shadowMCR  = data;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x04, &data);
	data = 0x0b;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x04, &data);

	data = 0x00;
	send_mos_cmd(serial, MOS_READ, MOS_MAX_PORT, 0x08, &data);
	data = 0x00;
	send_mos_cmd(serial, MOS_WRITE, MOS_MAX_PORT, 0x08, &data);


	data = 0x00;
	send_mos_cmd(serial, MOS_READ, MOS_MAX_PORT, 0x08, &data);

	data = data | (port->number - port->serial->minor + 1);
	send_mos_cmd(serial, MOS_WRITE, MOS_MAX_PORT, 0x08, &data);

	data = 0x83;
	mos7720_port->shadowLCR  = data;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x03, &data);
	data = 0x0c;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x00, &data);
	data = 0x00;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x01, &data);
	data = 0x03;
	mos7720_port->shadowLCR  = data;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x03, &data);
	data = 0x0c;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x01, &data);
	data = 0x0c;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x01, &data);

	
	if (!mos7720_serial->interrupt_started) {
		dbg("Interrupt buffer NULL !!!");

		
		mos7720_serial->interrupt_started = 1;

		dbg("To Submit URB !!!");

		
		usb_fill_int_urb(port0->interrupt_in_urb, serial->dev,
			 usb_rcvintpipe(serial->dev,
				port->interrupt_in_endpointAddress),
			 port0->interrupt_in_buffer,
			 port0->interrupt_in_urb->transfer_buffer_length,
			 mos7720_interrupt_callback, mos7720_port,
			 port0->interrupt_in_urb->interval);

		
		dbg("Submit URB over !!!");
		response = usb_submit_urb(port0->interrupt_in_urb, GFP_KERNEL);
		if (response)
			dev_err(&port->dev,
				"%s - Error %d submitting control urb\n",
				__func__, response);
	}

	
	usb_fill_bulk_urb(port->read_urb, serial->dev,
			  usb_rcvbulkpipe(serial->dev,
				port->bulk_in_endpointAddress),
			  port->bulk_in_buffer,
			  port->read_urb->transfer_buffer_length,
			  mos7720_bulk_in_callback, mos7720_port);
	response = usb_submit_urb(port->read_urb, GFP_KERNEL);
	if (response)
		dev_err(&port->dev, "%s - Error %d submitting read urb\n",
							__func__, response);

	
	memset(&(mos7720_port->icount), 0x00, sizeof(mos7720_port->icount));

	
	mos7720_port->shadowMCR = UART_MCR_OUT2; 

	
	mos7720_port->open = 1;

	return 0;
}


static int mos7720_chars_in_buffer(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	int i;
	int chars = 0;
	struct moschip_port *mos7720_port;

	dbg("%s:entering ...........", __func__);

	mos7720_port = usb_get_serial_port_data(port);
	if (mos7720_port == NULL) {
		dbg("%s:leaving ...........", __func__);
		return 0;
	}

	for (i = 0; i < NUM_URBS; ++i) {
		if (mos7720_port->write_urb_pool[i] &&
		    mos7720_port->write_urb_pool[i]->status == -EINPROGRESS)
			chars += URB_TRANSFER_BUFFER_SIZE;
	}
	dbg("%s - returns %d", __func__, chars);
	return chars;
}

static void mos7720_close(struct usb_serial_port *port)
{
	struct usb_serial *serial;
	struct moschip_port *mos7720_port;
	char data;
	int j;

	dbg("mos7720_close:entering...");

	serial = port->serial;

	mos7720_port = usb_get_serial_port_data(port);
	if (mos7720_port == NULL)
		return;

	for (j = 0; j < NUM_URBS; ++j)
		usb_kill_urb(mos7720_port->write_urb_pool[j]);

	
	for (j = 0; j < NUM_URBS; ++j) {
		if (mos7720_port->write_urb_pool[j]) {
			kfree(mos7720_port->write_urb_pool[j]->transfer_buffer);
			usb_free_urb(mos7720_port->write_urb_pool[j]);
		}
	}

	
	dbg("Shutdown bulk write");
	usb_kill_urb(port->write_urb);
	dbg("Shutdown bulk read");
	usb_kill_urb(port->read_urb);

	mutex_lock(&serial->disc_mutex);
	
	if (!serial->disconnected) {
		data = 0x00;
		send_mos_cmd(serial, MOS_WRITE,
			port->number - port->serial->minor, 0x04, &data);

		data = 0x00;
		send_mos_cmd(serial, MOS_WRITE,
			port->number - port->serial->minor, 0x01, &data);
	}
	mutex_unlock(&serial->disc_mutex);
	mos7720_port->open = 0;

	dbg("Leaving %s", __func__);
}

static void mos7720_break(struct tty_struct *tty, int break_state)
{
	struct usb_serial_port *port = tty->driver_data;
	unsigned char data;
	struct usb_serial *serial;
	struct moschip_port *mos7720_port;

	dbg("Entering %s", __func__);

	serial = port->serial;

	mos7720_port = usb_get_serial_port_data(port);
	if (mos7720_port == NULL)
		return;

	if (break_state == -1)
		data = mos7720_port->shadowLCR | UART_LCR_SBC;
	else
		data = mos7720_port->shadowLCR & ~UART_LCR_SBC;

	mos7720_port->shadowLCR  = data;
	send_mos_cmd(serial, MOS_WRITE, port->number - port->serial->minor,
		     0x03, &data);

	return;
}


static int mos7720_write_room(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	struct moschip_port *mos7720_port;
	int room = 0;
	int i;

	dbg("%s:entering ...........", __func__);

	mos7720_port = usb_get_serial_port_data(port);
	if (mos7720_port == NULL) {
		dbg("%s:leaving ...........", __func__);
		return -ENODEV;
	}

	
	for (i = 0; i < NUM_URBS; ++i) {
		if (mos7720_port->write_urb_pool[i] &&
		    mos7720_port->write_urb_pool[i]->status != -EINPROGRESS)
			room += URB_TRANSFER_BUFFER_SIZE;
	}

	dbg("%s - returns %d", __func__, room);
	return room;
}

static int mos7720_write(struct tty_struct *tty, struct usb_serial_port *port,
				 const unsigned char *data, int count)
{
	int status;
	int i;
	int bytes_sent = 0;
	int transfer_size;

	struct moschip_port *mos7720_port;
	struct usb_serial *serial;
	struct urb    *urb;
	const unsigned char *current_position = data;

	dbg("%s:entering ...........", __func__);

	serial = port->serial;

	mos7720_port = usb_get_serial_port_data(port);
	if (mos7720_port == NULL) {
		dbg("mos7720_port is NULL");
		return -ENODEV;
	}

	
	urb = NULL;

	for (i = 0; i < NUM_URBS; ++i) {
		if (mos7720_port->write_urb_pool[i] &&
		    mos7720_port->write_urb_pool[i]->status != -EINPROGRESS) {
			urb = mos7720_port->write_urb_pool[i];
			dbg("URB:%d", i);
			break;
		}
	}

	if (urb == NULL) {
		dbg("%s - no more free urbs", __func__);
		goto exit;
	}

	if (urb->transfer_buffer == NULL) {
		urb->transfer_buffer = kmalloc(URB_TRANSFER_BUFFER_SIZE,
					       GFP_KERNEL);
		if (urb->transfer_buffer == NULL) {
			dev_err(&port->dev, "%s no more kernel memory...\n",
				__func__);
			goto exit;
		}
	}
	transfer_size = min(count, URB_TRANSFER_BUFFER_SIZE);

	memcpy(urb->transfer_buffer, current_position, transfer_size);
	usb_serial_debug_data(debug, &port->dev, __func__, transfer_size,
			      urb->transfer_buffer);

	
	usb_fill_bulk_urb(urb, serial->dev,
			  usb_sndbulkpipe(serial->dev,
					port->bulk_out_endpointAddress),
			  urb->transfer_buffer, transfer_size,
			  mos7720_bulk_out_data_callback, mos7720_port);

	
	status = usb_submit_urb(urb, GFP_ATOMIC);
	if (status) {
		dev_err(&port->dev, "%s - usb_submit_urb(write bulk) failed "
			"with status = %d\n", __func__, status);
		bytes_sent = status;
		goto exit;
	}
	bytes_sent = transfer_size;

exit:
	return bytes_sent;
}

static void mos7720_throttle(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	struct moschip_port *mos7720_port;
	int status;

	dbg("%s- port %d\n", __func__, port->number);

	mos7720_port = usb_get_serial_port_data(port);

	if (mos7720_port == NULL)
		return;

	if (!mos7720_port->open) {
		dbg("port not opened");
		return;
	}

	dbg("%s: Entering ..........", __func__);

	
	if (I_IXOFF(tty)) {
		unsigned char stop_char = STOP_CHAR(tty);
		status = mos7720_write(tty, port, &stop_char, 1);
		if (status <= 0)
			return;
	}

	
	if (tty->termios->c_cflag & CRTSCTS) {
		mos7720_port->shadowMCR &= ~UART_MCR_RTS;
		status = send_mos_cmd(port->serial, MOS_WRITE,
				      port->number - port->serial->minor,
				      UART_MCR, &mos7720_port->shadowMCR);
		if (status != 0)
			return;
	}
}

static void mos7720_unthrottle(struct tty_struct *tty)
{
	struct usb_serial_port *port = tty->driver_data;
	struct moschip_port *mos7720_port = usb_get_serial_port_data(port);
	int status;

	if (mos7720_port == NULL)
		return;

	if (!mos7720_port->open) {
		dbg("%s - port not opened", __func__);
		return;
	}

	dbg("%s: Entering ..........", __func__);

	
	if (I_IXOFF(tty)) {
		unsigned char start_char = START_CHAR(tty);
		status = mos7720_write(tty, port, &start_char, 1);
		if (status <= 0)
			return;
	}

	
	if (tty->termios->c_cflag & CRTSCTS) {
		mos7720_port->shadowMCR |= UART_MCR_RTS;
		status = send_mos_cmd(port->serial, MOS_WRITE,
				      port->number - port->serial->minor,
				      UART_MCR, &mos7720_port->shadowMCR);
		if (status != 0)
			return;
	}
}

static int set_higher_rates(struct moschip_port *mos7720_port,
			    unsigned int baud)
{
	unsigned char data;
	struct usb_serial_port *port;
	struct usb_serial *serial;
	int port_number;

	if (mos7720_port == NULL)
		return -EINVAL;

	port = mos7720_port->port;
	serial = port->serial;

	 
	dbg("Sending Setting Commands ..........");
	port_number = port->number - port->serial->minor;

	data = 0x000;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x01, &data);
	data = 0x000;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x02, &data);
	data = 0x0CF;
	send_mos_cmd(serial, MOS_WRITE, port->number, 0x02, &data);
	data = 0x00b;
	mos7720_port->shadowMCR  = data;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x04, &data);
	data = 0x00b;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x04, &data);

	data = 0x000;
	send_mos_cmd(serial, MOS_READ, MOS_MAX_PORT, 0x08, &data);
	data = 0x000;
	send_mos_cmd(serial, MOS_WRITE, MOS_MAX_PORT, 0x08, &data);


	

	data = baud * 0x10;
	send_mos_cmd(serial, MOS_WRITE, MOS_MAX_PORT, port_number + 1, &data);

	data = 0x003;
	send_mos_cmd(serial, MOS_READ, MOS_MAX_PORT, 0x08, &data);
	data = 0x003;
	send_mos_cmd(serial, MOS_WRITE, MOS_MAX_PORT, 0x08, &data);

	data = 0x02b;
	mos7720_port->shadowMCR  = data;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x04, &data);
	data = 0x02b;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x04, &data);

	

	data = mos7720_port->shadowLCR | UART_LCR_DLAB;
	mos7720_port->shadowLCR  = data;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x03, &data);

	data =  0x001; 
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x00, &data);
	data =  0x000; 
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x01, &data);

	data = mos7720_port->shadowLCR & ~UART_LCR_DLAB;
	mos7720_port->shadowLCR  = data;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x03, &data);

	return 0;
}


struct divisor_table_entry {
	__u32  baudrate;
	__u16  divisor;
};


static struct divisor_table_entry divisor_table[] = {
	{   50,		2304},
	{   110,	1047},	
	{   134,	857},	
	{   150,	768},
	{   300,	384},
	{   600,	192},
	{   1200,	96},
	{   1800,	64},
	{   2400,	48},
	{   4800,	24},
	{   7200,	16},
	{   9600,	12},
	{   19200,	6},
	{   38400,	3},
	{   57600,	2},
	{   115200,	1},
};


static int calc_baud_rate_divisor(int baudrate, int *divisor)
{
	int i;
	__u16 custom;
	__u16 round1;
	__u16 round;


	dbg("%s - %d", __func__, baudrate);

	for (i = 0; i < ARRAY_SIZE(divisor_table); i++) {
		if (divisor_table[i].baudrate == baudrate) {
			*divisor = divisor_table[i].divisor;
			return 0;
		}
	}

	
	if (baudrate > 75 &&  baudrate < 230400) {
		
		custom = (__u16)(230400L  / baudrate);

		
		round1 = (__u16)(2304000L / baudrate);
		round = (__u16)(round1 - (custom * 10));
		if (round > 4)
			custom++;
		*divisor = custom;

		dbg("Baud %d = %d", baudrate, custom);
		return 0;
	}

	dbg("Baud calculation Failed...");
	return -EINVAL;
}


static int send_cmd_write_baud_rate(struct moschip_port *mos7720_port,
				    int baudrate)
{
	struct usb_serial_port *port;
	struct usb_serial *serial;
	int divisor;
	int status;
	unsigned char data;
	unsigned char number;

	if (mos7720_port == NULL)
		return -1;

	port = mos7720_port->port;
	serial = port->serial;

	dbg("%s: Entering ..........", __func__);

	number = port->number - port->serial->minor;
	dbg("%s - port = %d, baud = %d", __func__, port->number, baudrate);

	
	status = calc_baud_rate_divisor(baudrate, &divisor);
	if (status) {
		dev_err(&port->dev, "%s - bad baud rate\n", __func__);
		return status;
	}

	
	data = mos7720_port->shadowLCR | UART_LCR_DLAB;
	mos7720_port->shadowLCR  = data;
	send_mos_cmd(serial, MOS_WRITE, number, UART_LCR, &data);

	
	data = ((unsigned char)(divisor & 0xff));
	send_mos_cmd(serial, MOS_WRITE, number, 0x00, &data);

	data = ((unsigned char)((divisor & 0xff00) >> 8));
	send_mos_cmd(serial, MOS_WRITE, number, 0x01, &data);

	
	data = mos7720_port->shadowLCR & ~UART_LCR_DLAB;
	mos7720_port->shadowLCR = data;
	send_mos_cmd(serial, MOS_WRITE, number, 0x03, &data);

	return status;
}


static void change_port_settings(struct tty_struct *tty,
				 struct moschip_port *mos7720_port,
				 struct ktermios *old_termios)
{
	struct usb_serial_port *port;
	struct usb_serial *serial;
	int baud;
	unsigned cflag;
	unsigned iflag;
	__u8 mask = 0xff;
	__u8 lData;
	__u8 lParity;
	__u8 lStop;
	int status;
	int port_number;
	char data;

	if (mos7720_port == NULL)
		return ;

	port = mos7720_port->port;
	serial = port->serial;
	port_number = port->number - port->serial->minor;

	dbg("%s - port %d", __func__, port->number);

	if (!mos7720_port->open) {
		dbg("%s - port not opened", __func__);
		return;
	}

	dbg("%s: Entering ..........", __func__);

	lData = UART_LCR_WLEN8;
	lStop = 0x00;	
	lParity = 0x00;	

	cflag = tty->termios->c_cflag;
	iflag = tty->termios->c_iflag;

	
	switch (cflag & CSIZE) {
	case CS5:
		lData = UART_LCR_WLEN5;
		mask = 0x1f;
		break;

	case CS6:
		lData = UART_LCR_WLEN6;
		mask = 0x3f;
		break;

	case CS7:
		lData = UART_LCR_WLEN7;
		mask = 0x7f;
		break;
	default:
	case CS8:
		lData = UART_LCR_WLEN8;
		break;
	}

	
	if (cflag & PARENB) {
		if (cflag & PARODD) {
			lParity = UART_LCR_PARITY;
			dbg("%s - parity = odd", __func__);
		} else {
			lParity = (UART_LCR_EPAR | UART_LCR_PARITY);
			dbg("%s - parity = even", __func__);
		}

	} else {
		dbg("%s - parity = none", __func__);
	}

	if (cflag & CMSPAR)
		lParity = lParity | 0x20;

	
	if (cflag & CSTOPB) {
		lStop = UART_LCR_STOP;
		dbg("%s - stop bits = 2", __func__);
	} else {
		lStop = 0x00;
		dbg("%s - stop bits = 1", __func__);
	}

#define LCR_BITS_MASK		0x03	
#define LCR_STOP_MASK		0x04	
#define LCR_PAR_MASK		0x38	

	
	mos7720_port->shadowLCR &=
			~(LCR_BITS_MASK | LCR_STOP_MASK | LCR_PAR_MASK);
	mos7720_port->shadowLCR |= (lData | lParity | lStop);


	
	data = 0x00;
	send_mos_cmd(serial, MOS_WRITE, port->number - port->serial->minor,
							UART_IER, &data);

	data = 0x00;
	send_mos_cmd(serial, MOS_WRITE, port_number, UART_FCR, &data);

	data = 0xcf;
	send_mos_cmd(serial, MOS_WRITE, port_number, UART_FCR, &data);

	
	data = mos7720_port->shadowLCR;
	send_mos_cmd(serial, MOS_WRITE, port_number, UART_LCR, &data);

	data = 0x00b;
	mos7720_port->shadowMCR = data;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x04, &data);
	data = 0x00b;
	send_mos_cmd(serial, MOS_WRITE, port_number, 0x04, &data);

	
	mos7720_port->shadowMCR = UART_MCR_OUT2;
	if (cflag & CBAUD)
		mos7720_port->shadowMCR |= (UART_MCR_DTR | UART_MCR_RTS);

	if (cflag & CRTSCTS) {
		mos7720_port->shadowMCR |= (UART_MCR_XONANY);
		
		if (port->number) {
			data = 0x001;
			send_mos_cmd(serial, MOS_WRITE, MOS_MAX_PORT,
				     0x08, &data);
		} else {
			data = 0x002;
			send_mos_cmd(serial, MOS_WRITE, MOS_MAX_PORT,
				     0x08, &data);
		}
	} else {
		mos7720_port->shadowMCR &= ~(UART_MCR_XONANY);
	}

	data = mos7720_port->shadowMCR;
	send_mos_cmd(serial, MOS_WRITE, port_number, UART_MCR, &data);

	
	baud = tty_get_baud_rate(tty);
	if (!baud) {
		
		dbg("Picked default baud...");
		baud = 9600;
	}

	if (baud >= 230400) {
		set_higher_rates(mos7720_port, baud);
		
		data = 0x0c;
		send_mos_cmd(serial, MOS_WRITE, port_number, UART_IER, &data);
		return;
	}

	dbg("%s - baud rate = %d", __func__, baud);
	status = send_cmd_write_baud_rate(mos7720_port, baud);
	
	if (cflag & CBAUD)
		tty_encode_baud_rate(tty, baud, baud);
	
	data = 0x0c;
	send_mos_cmd(serial, MOS_WRITE, port_number, UART_IER, &data);

	if (port->read_urb->status != -EINPROGRESS) {
		port->read_urb->dev = serial->dev;

		status = usb_submit_urb(port->read_urb, GFP_ATOMIC);
		if (status)
			dbg("usb_submit_urb(read bulk) failed, status = %d",
			    status);
	}
	return;
}


static void mos7720_set_termios(struct tty_struct *tty,
		struct usb_serial_port *port, struct ktermios *old_termios)
{
	int status;
	unsigned int cflag;
	struct usb_serial *serial;
	struct moschip_port *mos7720_port;

	serial = port->serial;

	mos7720_port = usb_get_serial_port_data(port);

	if (mos7720_port == NULL)
		return;

	if (!mos7720_port->open) {
		dbg("%s - port not opened", __func__);
		return;
	}

	dbg("%s\n", "setting termios - ASPIRE");

	cflag = tty->termios->c_cflag;

	dbg("%s - cflag %08x iflag %08x", __func__,
	    tty->termios->c_cflag,
	    RELEVANT_IFLAG(tty->termios->c_iflag));

	dbg("%s - old cflag %08x old iflag %08x", __func__,
	    old_termios->c_cflag,
	    RELEVANT_IFLAG(old_termios->c_iflag));

	dbg("%s - port %d", __func__, port->number);

	
	change_port_settings(tty, mos7720_port, old_termios);

	if (!port->read_urb) {
		dbg("%s", "URB KILLED !!!!!\n");
		return;
	}

	if (port->read_urb->status != -EINPROGRESS) {
		port->read_urb->dev = serial->dev;
		status = usb_submit_urb(port->read_urb, GFP_ATOMIC);
		if (status)
			dbg("usb_submit_urb(read bulk) failed, status = %d",
			    status);
	}
	return;
}


static int get_lsr_info(struct tty_struct *tty,
		struct moschip_port *mos7720_port, unsigned int __user *value)
{
	struct usb_serial_port *port = tty->driver_data;
	unsigned int result = 0;
	unsigned char data = 0;
	int port_number = port->number - port->serial->minor;
	int count;

	count = mos7720_chars_in_buffer(tty);
	if (count == 0) {
		send_mos_cmd(port->serial, MOS_READ, port_number,
							UART_LSR, &data);
		if ((data & (UART_LSR_TEMT | UART_LSR_THRE))
					== (UART_LSR_TEMT | UART_LSR_THRE)) {
			dbg("%s -- Empty", __func__);
			result = TIOCSER_TEMT;
		}
	}
	if (copy_to_user(value, &result, sizeof(int)))
		return -EFAULT;
	return 0;
}

static int mos7720_tiocmget(struct tty_struct *tty, struct file *file)
{
	struct usb_serial_port *port = tty->driver_data;
	struct moschip_port *mos7720_port = usb_get_serial_port_data(port);
	unsigned int result = 0;
	unsigned int mcr ;
	unsigned int msr ;

	dbg("%s - port %d", __func__, port->number);

	mcr = mos7720_port->shadowMCR;
	msr = mos7720_port->shadowMSR;

	result = ((mcr & UART_MCR_DTR)  ? TIOCM_DTR : 0)   
	  | ((mcr & UART_MCR_RTS)   ? TIOCM_RTS : 0)   
	  | ((msr & UART_MSR_CTS)   ? TIOCM_CTS : 0)   
	  | ((msr & UART_MSR_DCD)   ? TIOCM_CAR : 0)   
	  | ((msr & UART_MSR_RI)    ? TIOCM_RI :  0)   
	  | ((msr & UART_MSR_DSR)   ? TIOCM_DSR : 0);  

	dbg("%s -- %x", __func__, result);

	return result;
}

static int mos7720_tiocmset(struct tty_struct *tty, struct file *file,
					unsigned int set, unsigned int clear)
{
	struct usb_serial_port *port = tty->driver_data;
	struct moschip_port *mos7720_port = usb_get_serial_port_data(port);
	unsigned int mcr ;
	unsigned char lmcr;

	dbg("%s - port %d", __func__, port->number);
	dbg("he was at tiocmget");

	mcr = mos7720_port->shadowMCR;

	if (set & TIOCM_RTS)
		mcr |= UART_MCR_RTS;
	if (set & TIOCM_DTR)
		mcr |= UART_MCR_DTR;
	if (set & TIOCM_LOOP)
		mcr |= UART_MCR_LOOP;

	if (clear & TIOCM_RTS)
		mcr &= ~UART_MCR_RTS;
	if (clear & TIOCM_DTR)
		mcr &= ~UART_MCR_DTR;
	if (clear & TIOCM_LOOP)
		mcr &= ~UART_MCR_LOOP;

	mos7720_port->shadowMCR = mcr;
	lmcr = mos7720_port->shadowMCR;

	send_mos_cmd(port->serial, MOS_WRITE,
		port->number - port->serial->minor, UART_MCR, &lmcr);

	return 0;
}

static int set_modem_info(struct moschip_port *mos7720_port, unsigned int cmd,
			  unsigned int __user *value)
{
	unsigned int mcr ;
	unsigned int arg;
	unsigned char data;

	struct usb_serial_port *port;

	if (mos7720_port == NULL)
		return -1;

	port = (struct usb_serial_port *)mos7720_port->port;
	mcr = mos7720_port->shadowMCR;

	if (copy_from_user(&arg, value, sizeof(int)))
		return -EFAULT;

	switch (cmd) {
	case TIOCMBIS:
		if (arg & TIOCM_RTS)
			mcr |= UART_MCR_RTS;
		if (arg & TIOCM_DTR)
			mcr |= UART_MCR_RTS;
		if (arg & TIOCM_LOOP)
			mcr |= UART_MCR_LOOP;
		break;

	case TIOCMBIC:
		if (arg & TIOCM_RTS)
			mcr &= ~UART_MCR_RTS;
		if (arg & TIOCM_DTR)
			mcr &= ~UART_MCR_RTS;
		if (arg & TIOCM_LOOP)
			mcr &= ~UART_MCR_LOOP;
		break;

	}

	mos7720_port->shadowMCR = mcr;

	data = mos7720_port->shadowMCR;
	send_mos_cmd(port->serial, MOS_WRITE,
		     port->number - port->serial->minor, UART_MCR, &data);

	return 0;
}

static int get_serial_info(struct moschip_port *mos7720_port,
			   struct serial_struct __user *retinfo)
{
	struct serial_struct tmp;

	if (!retinfo)
		return -EFAULT;

	memset(&tmp, 0, sizeof(tmp));

	tmp.type		= PORT_16550A;
	tmp.line		= mos7720_port->port->serial->minor;
	tmp.port		= mos7720_port->port->number;
	tmp.irq			= 0;
	tmp.flags		= ASYNC_SKIP_TEST | ASYNC_AUTO_IRQ;
	tmp.xmit_fifo_size	= NUM_URBS * URB_TRANSFER_BUFFER_SIZE;
	tmp.baud_base		= 9600;
	tmp.close_delay		= 5*HZ;
	tmp.closing_wait	= 30*HZ;

	if (copy_to_user(retinfo, &tmp, sizeof(*retinfo)))
		return -EFAULT;
	return 0;
}

static int mos7720_ioctl(struct tty_struct *tty, struct file *file,
			 unsigned int cmd, unsigned long arg)
{
	struct usb_serial_port *port = tty->driver_data;
	struct moschip_port *mos7720_port;
	struct async_icount cnow;
	struct async_icount cprev;
	struct serial_icounter_struct icount;

	mos7720_port = usb_get_serial_port_data(port);
	if (mos7720_port == NULL)
		return -ENODEV;

	dbg("%s - port %d, cmd = 0x%x", __func__, port->number, cmd);

	switch (cmd) {
	case TIOCSERGETLSR:
		dbg("%s (%d) TIOCSERGETLSR", __func__,  port->number);
		return get_lsr_info(tty, mos7720_port,
					(unsigned int __user *)arg);
		return 0;

	
	case TIOCMBIS:
	case TIOCMBIC:
		dbg("%s (%d) TIOCMSET/TIOCMBIC/TIOCMSET",
					__func__, port->number);
		return set_modem_info(mos7720_port, cmd,
				      (unsigned int __user *)arg);

	case TIOCGSERIAL:
		dbg("%s (%d) TIOCGSERIAL", __func__,  port->number);
		return get_serial_info(mos7720_port,
				       (struct serial_struct __user *)arg);

	case TIOCMIWAIT:
		dbg("%s (%d) TIOCMIWAIT", __func__,  port->number);
		cprev = mos7720_port->icount;
		while (1) {
			if (signal_pending(current))
				return -ERESTARTSYS;
			cnow = mos7720_port->icount;
			if (cnow.rng == cprev.rng && cnow.dsr == cprev.dsr &&
			    cnow.dcd == cprev.dcd && cnow.cts == cprev.cts)
				return -EIO; 
			if (((arg & TIOCM_RNG) && (cnow.rng != cprev.rng)) ||
			    ((arg & TIOCM_DSR) && (cnow.dsr != cprev.dsr)) ||
			    ((arg & TIOCM_CD)  && (cnow.dcd != cprev.dcd)) ||
			    ((arg & TIOCM_CTS) && (cnow.cts != cprev.cts))) {
				return 0;
			}
			cprev = cnow;
		}
		
		break;

	case TIOCGICOUNT:
		cnow = mos7720_port->icount;
		icount.cts = cnow.cts;
		icount.dsr = cnow.dsr;
		icount.rng = cnow.rng;
		icount.dcd = cnow.dcd;
		icount.rx = cnow.rx;
		icount.tx = cnow.tx;
		icount.frame = cnow.frame;
		icount.overrun = cnow.overrun;
		icount.parity = cnow.parity;
		icount.brk = cnow.brk;
		icount.buf_overrun = cnow.buf_overrun;

		dbg("%s (%d) TIOCGICOUNT RX=%d, TX=%d", __func__,
		    port->number, icount.rx, icount.tx);
		if (copy_to_user((void __user *)arg, &icount, sizeof(icount)))
			return -EFAULT;
		return 0;
	}

	return -ENOIOCTLCMD;
}

static int mos7720_startup(struct usb_serial *serial)
{
	struct moschip_serial *mos7720_serial;
	struct moschip_port *mos7720_port;
	struct usb_device *dev;
	int i;
	char data;

	dbg("%s: Entering ..........", __func__);

	if (!serial) {
		dbg("Invalid Handler");
		return -ENODEV;
	}

	dev = serial->dev;

	
	mos7720_serial = kzalloc(sizeof(struct moschip_serial), GFP_KERNEL);
	if (mos7720_serial == NULL) {
		dev_err(&dev->dev, "%s - Out of memory\n", __func__);
		return -ENOMEM;
	}

	usb_set_serial_data(serial, mos7720_serial);

	

	
	for (i = 0; i < serial->num_ports; ++i) {
		mos7720_port = kzalloc(sizeof(struct moschip_port), GFP_KERNEL);
		if (mos7720_port == NULL) {
			dev_err(&dev->dev, "%s - Out of memory\n", __func__);
			usb_set_serial_data(serial, NULL);
			kfree(mos7720_serial);
			return -ENOMEM;
		}

		
		serial->port[i]->interrupt_in_endpointAddress =
				serial->port[0]->interrupt_in_endpointAddress;

		mos7720_port->port = serial->port[i];
		usb_set_serial_port_data(serial->port[i], mos7720_port);

		dbg("port number is %d", serial->port[i]->number);
		dbg("serial number is %d", serial->minor);
	}


	
	usb_control_msg(serial->dev, usb_sndctrlpipe(serial->dev, 0),
			(__u8)0x03, 0x00, 0x01, 0x00, NULL, 0x00, 5*HZ);

	
	send_mos_cmd(serial, MOS_READ, 0x00, UART_LSR, &data);
	dbg("LSR:%x", data);

	
	send_mos_cmd(serial, MOS_READ, 0x01, UART_LSR, &data);
	dbg("LSR:%x", data);

	return 0;
}

static void mos7720_release(struct usb_serial *serial)
{
	int i;

	
	for (i = 0; i < serial->num_ports; ++i)
		kfree(usb_get_serial_port_data(serial->port[i]));

	
	kfree(usb_get_serial_data(serial));
}

static struct usb_driver usb_driver = {
	.name =		"moschip7720",
	.probe =	usb_serial_probe,
	.disconnect =	usb_serial_disconnect,
	.id_table =	moschip_port_id_table,
	.no_dynamic_id =	1,
};

static struct usb_serial_driver moschip7720_2port_driver = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"moschip7720",
	},
	.description		= "Moschip 2 port adapter",
	.usb_driver		= &usb_driver,
	.id_table		= moschip_port_id_table,
	.num_ports		= 2,
	.open			= mos7720_open,
	.close			= mos7720_close,
	.throttle		= mos7720_throttle,
	.unthrottle		= mos7720_unthrottle,
	.attach			= mos7720_startup,
	.release		= mos7720_release,
	.ioctl			= mos7720_ioctl,
	.tiocmget		= mos7720_tiocmget,
	.tiocmset		= mos7720_tiocmset,
	.set_termios		= mos7720_set_termios,
	.write			= mos7720_write,
	.write_room		= mos7720_write_room,
	.chars_in_buffer	= mos7720_chars_in_buffer,
	.break_ctl		= mos7720_break,
	.read_bulk_callback	= mos7720_bulk_in_callback,
	.read_int_callback	= mos7720_interrupt_callback,
};

static int __init moschip7720_init(void)
{
	int retval;

	dbg("%s: Entering ..........", __func__);

	
	retval = usb_serial_register(&moschip7720_2port_driver);
	if (retval)
		goto failed_port_device_register;

	printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
	       DRIVER_DESC "\n");

	
	retval = usb_register(&usb_driver);
	if (retval)
		goto failed_usb_register;

	return 0;

failed_usb_register:
	usb_serial_deregister(&moschip7720_2port_driver);

failed_port_device_register:
	return retval;
}

static void __exit moschip7720_exit(void)
{
	usb_deregister(&usb_driver);
	usb_serial_deregister(&moschip7720_2port_driver);
}

module_init(moschip7720_init);
module_exit(moschip7720_exit);


MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

module_param(debug, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(debug, "Debug enabled or not");
