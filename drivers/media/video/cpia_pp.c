





#include <linux/module.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/parport.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/sched.h>

#include <linux/kmod.h>


#include "cpia.h"

static int cpia_pp_open(void *privdata);
static int cpia_pp_registerCallback(void *privdata, void (*cb) (void *cbdata),
				    void *cbdata);
static int cpia_pp_transferCmd(void *privdata, u8 *command, u8 *data);
static int cpia_pp_streamStart(void *privdata);
static int cpia_pp_streamStop(void *privdata);
static int cpia_pp_streamRead(void *privdata, u8 *buffer, int noblock);
static int cpia_pp_close(void *privdata);


#define ABOUT "Parallel port driver for Vision CPiA based cameras"

#define PACKET_LENGTH  8


#define PPCPIA_PARPORT_UNSPEC -4
#define PPCPIA_PARPORT_AUTO -3
#define PPCPIA_PARPORT_OFF -2
#define PPCPIA_PARPORT_NONE -1

static int parport_nr[PARPORT_MAX] = {[0 ... PARPORT_MAX - 1] = PPCPIA_PARPORT_UNSPEC};
static char *parport[PARPORT_MAX] = {NULL,};

MODULE_AUTHOR("B. Huisman <bhuism@cs.utwente.nl> & Peter Pregler <Peter_Pregler@email.com>");
MODULE_DESCRIPTION("Parallel port driver for Vision CPiA based cameras");
MODULE_LICENSE("GPL");

module_param_array(parport, charp, NULL, 0);
MODULE_PARM_DESC(parport, "'auto' or a list of parallel port numbers. Just like lp.");

struct pp_cam_entry {
	struct pardevice *pdev;
	struct parport *port;
	struct work_struct cb_task;
	void (*cb_func)(void *cbdata);
	void *cb_data;
	int open_count;
	wait_queue_head_t wq_stream;
	
	int image_ready;	
	int image_complete;	

	int streaming; 
	int stream_irq;
};

static struct cpia_camera_ops cpia_pp_ops =
{
	cpia_pp_open,
	cpia_pp_registerCallback,
	cpia_pp_transferCmd,
	cpia_pp_streamStart,
	cpia_pp_streamStop,
	cpia_pp_streamRead,
	cpia_pp_close,
	1,
	THIS_MODULE
};

static LIST_HEAD(cam_list);
static spinlock_t cam_list_lock_pp;


static void cpia_parport_enable_irq( struct parport *port ) {
	parport_enable_irq(port);
	mdelay(10);
	return;
}

static void cpia_parport_disable_irq( struct parport *port ) {
	parport_disable_irq(port);
	mdelay(10);
	return;
}


#define UPLOAD_FLAG  0x08
#define NIBBLE_TRANSFER 0x01
#define ECP_TRANSFER 0x03

#define PARPORT_CHUNK_SIZE	PAGE_SIZE


static void cpia_pp_run_callback(struct work_struct *work)
{
	void (*cb_func)(void *cbdata);
	void *cb_data;
	struct pp_cam_entry *cam;

	cam = container_of(work, struct pp_cam_entry, cb_task);
	cb_func = cam->cb_func;
	cb_data = cam->cb_data;

	cb_func(cb_data);
}





static size_t cpia_read_nibble (struct parport *port,
			 void *buffer, size_t len,
			 int flags)
{
	

	unsigned char *buf = buffer;
	int i;
	unsigned char byte = 0;

	len *= 2; 
	for (i=0; i < len; i++) {
		unsigned char nibble;

		

		

		
		if (((i ) == 0) &&
		    (parport_read_status(port) & PARPORT_STATUS_ERROR)) {
			DBG("%s: No more nibble data (%d bytes)\n",
			    port->name, i/2);
			goto end_of_data;
		}

		
		parport_frob_control (port,
				      PARPORT_CONTROL_AUTOFD,
				      PARPORT_CONTROL_AUTOFD);

		
		port->ieee1284.phase = IEEE1284_PH_REV_DATA;
		if (parport_wait_peripheral (port,
					     PARPORT_STATUS_ACK, 0)) {
			
				 DBG("%s: Nibble timeout at event 9 (%d bytes)\n",
				 port->name, i/2);
			parport_frob_control (port, PARPORT_CONTROL_AUTOFD, 0);
			break;
		}


		
		nibble = parport_read_status (port) >> 3;
		nibble &= ~8;
		if ((nibble & 0x10) == 0)
			nibble |= 8;
		nibble &= 0xf;

		
		parport_frob_control (port, PARPORT_CONTROL_AUTOFD, 0);

		
		if (parport_wait_peripheral (port,
					     PARPORT_STATUS_ACK,
					     PARPORT_STATUS_ACK)) {
			
			DBG("%s: Nibble timeout at event 11\n",
				 port->name);
			break;
		}

		if (i & 1) {
			
			byte |= nibble << 4;
			*buf++ = byte;
		} else
			byte = nibble;
	}

	if (i == len) {
		
		if (parport_read_status (port) & PARPORT_STATUS_ERROR) {
		end_of_data:
			
			parport_frob_control (port,
					      PARPORT_CONTROL_AUTOFD,
					      PARPORT_CONTROL_AUTOFD);
			port->physport->ieee1284.phase = IEEE1284_PH_REV_IDLE;
		}
		else
			port->physport->ieee1284.phase = IEEE1284_PH_HBUSY_DAVAIL;
	}

	return i/2;
}



static size_t cpia_read_nibble_stream(struct parport *port,
			       void *buffer, size_t len,
			       int flags)
{
	int i;
	unsigned char *buf = buffer;
	int endseen = 0;

	for (i=0; i < len; i++) {
		unsigned char nibble[2], byte = 0;
		int j;

		
		if (endseen > 3 )
			break;

		
		parport_frob_control (port,
				      PARPORT_CONTROL_AUTOFD,
				      PARPORT_CONTROL_AUTOFD);

		
		port->ieee1284.phase = IEEE1284_PH_REV_DATA;
		if (parport_wait_peripheral (port,
					     PARPORT_STATUS_ACK, 0)) {
			
				 DBG("%s: Nibble timeout at event 9 (%d bytes)\n",
				 port->name, i/2);
			parport_frob_control (port, PARPORT_CONTROL_AUTOFD, 0);
			break;
		}

		
		nibble[0] = parport_read_status (port) >>3;

		
		parport_frob_control (port, PARPORT_CONTROL_AUTOFD, 0);

		
		if (parport_wait_peripheral (port,
					     PARPORT_STATUS_ACK,
					     PARPORT_STATUS_ACK)) {
			
			DBG("%s: Nibble timeout at event 11\n",
				 port->name);
			break;
		}

		
		nibble[1] = parport_read_status (port) >>3;

		
		for (j = 0; j < 2 ; j++ ) {
			nibble[j] &= ~8;
			if ((nibble[j] & 0x10) == 0)
				nibble[j] |= 8;
			nibble[j] &= 0xf;
		}
		byte = (nibble[0] |(nibble[1] << 4));
		*buf++ = byte;

		if(byte == EOI)
		  endseen++;
		else
		  endseen = 0;
	}
	return i;
}


static void EndTransferMode(struct pp_cam_entry *cam)
{
	parport_negotiate(cam->port, IEEE1284_MODE_COMPAT);
}


static int ForwardSetup(struct pp_cam_entry *cam)
{
	int retry;

	

	

	for(retry = 0; retry < 4; ++retry) {
		if(!parport_negotiate(cam->port, IEEE1284_MODE_ECP)) {
			break;
		}
		mdelay(10);
	}
	if(retry == 4) {
		DBG("Unable to negotiate IEEE1284 ECP Download mode\n");
		return -1;
	}
	return 0;
}

static int ReverseSetup(struct pp_cam_entry *cam, int extensibility)
{
	int retry;
	int upload_mode, mode = IEEE1284_MODE_ECP;
	int transfer_mode = ECP_TRANSFER;

	if (!(cam->port->modes & PARPORT_MODE_ECP) &&
	     !(cam->port->modes & PARPORT_MODE_TRISTATE)) {
		mode = IEEE1284_MODE_NIBBLE;
		transfer_mode = NIBBLE_TRANSFER;
	}

	upload_mode = mode;
	if(extensibility) mode = UPLOAD_FLAG|transfer_mode|IEEE1284_EXT_LINK;

	

	for(retry = 0; retry < 4; ++retry) {
		if(!parport_negotiate(cam->port, mode)) {
			break;
		}
		mdelay(10);
	}
	if(retry == 4) {
		if(extensibility)
			DBG("Unable to negotiate upload extensibility mode\n");
		else
			DBG("Unable to negotiate upload mode\n");
		return -1;
	}
	if(extensibility) cam->port->ieee1284.mode = upload_mode;
	return 0;
}


static int WritePacket(struct pp_cam_entry *cam, const u8 *packet, size_t size)
{
	int retval=0;
	int size_written;

	if (packet == NULL) {
		return -EINVAL;
	}
	if (ForwardSetup(cam)) {
		DBG("Write failed in setup\n");
		return -EIO;
	}
	size_written = parport_write(cam->port, packet, size);
	if(size_written != size) {
		DBG("Write failed, wrote %d/%d\n", size_written, size);
		retval = -EIO;
	}
	EndTransferMode(cam);
	return retval;
}


static int ReadPacket(struct pp_cam_entry *cam, u8 *packet, size_t size)
{
	int retval=0;

	if (packet == NULL) {
		return -EINVAL;
	}
	if (ReverseSetup(cam, 0)) {
		return -EIO;
	}

	
	if(cam->port->ieee1284.mode == IEEE1284_MODE_NIBBLE) {
		if(cpia_read_nibble(cam->port, packet, size, 0) != size)
			retval = -EIO;
	} else {
		if(parport_read(cam->port, packet, size) != size)
			retval = -EIO;
	}
	EndTransferMode(cam);
	return retval;
}


static int cpia_pp_streamStart(void *privdata)
{
	struct pp_cam_entry *cam = privdata;
	DBG("\n");
	cam->streaming=1;
	cam->image_ready=0;
	
	if(cam->stream_irq) cpia_parport_enable_irq(cam->port);
	return 0;
}


static int cpia_pp_streamStop(void *privdata)
{
	struct pp_cam_entry *cam = privdata;

	DBG("\n");
	cam->streaming=0;
	cpia_parport_disable_irq(cam->port);
	

	return 0;
}


static int cpia_pp_read(struct parport *port, u8 *buffer, int len)
{
	int bytes_read;

	
	if(port->ieee1284.mode == IEEE1284_MODE_NIBBLE)
		bytes_read = cpia_read_nibble_stream(port,buffer,len,0);
	else {
		int new_bytes;
		for(bytes_read=0; bytes_read<len; bytes_read += new_bytes) {
			new_bytes = parport_read(port, buffer+bytes_read,
						 len-bytes_read);
			if(new_bytes < 0) break;
		}
	}
	return bytes_read;
}

static int cpia_pp_streamRead(void *privdata, u8 *buffer, int noblock)
{
	struct pp_cam_entry *cam = privdata;
	int read_bytes = 0;
	int i, endseen, block_size, new_bytes;

	if(cam == NULL) {
		DBG("Internal driver error: cam is NULL\n");
		return -EINVAL;
	}
	if(buffer == NULL) {
		DBG("Internal driver error: buffer is NULL\n");
		return -EINVAL;
	}
	
	if( cam->stream_irq ) {
		DBG("%d\n", cam->image_ready);
		cam->image_ready--;
	}
	cam->image_complete=0;
	if (0) {
		if(!cam->image_ready) {
			if(noblock) return -EWOULDBLOCK;
			interruptible_sleep_on(&cam->wq_stream);
			if( signal_pending(current) ) return -EINTR;
			DBG("%d\n", cam->image_ready);
		}
	} else {
		if (ReverseSetup(cam, 1)) {
			DBG("unable to ReverseSetup\n");
			return -EIO;
		}
	}
	endseen = 0;
	block_size = PARPORT_CHUNK_SIZE;
	while( !cam->image_complete ) {
		cond_resched();

		new_bytes = cpia_pp_read(cam->port, buffer, block_size );
		if( new_bytes <= 0 ) {
			break;
		}
		i=-1;
		while(++i<new_bytes && endseen<4) {
			if(*buffer==EOI) {
				endseen++;
			} else {
				endseen=0;
			}
			buffer++;
		}
		read_bytes += i;
		if( endseen==4 ) {
			cam->image_complete=1;
			break;
		}
		if( CPIA_MAX_IMAGE_SIZE-read_bytes <= PARPORT_CHUNK_SIZE ) {
			block_size=CPIA_MAX_IMAGE_SIZE-read_bytes;
		}
	}
	EndTransferMode(cam);
	return cam->image_complete ? read_bytes : -EIO;
}

static int cpia_pp_transferCmd(void *privdata, u8 *command, u8 *data)
{
	int err;
	int retval=0;
	int databytes;
	struct pp_cam_entry *cam = privdata;

	if(cam == NULL) {
		DBG("Internal driver error: cam is NULL\n");
		return -EINVAL;
	}
	if(command == NULL) {
		DBG("Internal driver error: command is NULL\n");
		return -EINVAL;
	}
	databytes = (((int)command[7])<<8) | command[6];
	if ((err = WritePacket(cam, command, PACKET_LENGTH)) < 0) {
		DBG("Error writing command\n");
		return err;
	}
	if(command[0] == DATA_IN) {
		u8 buffer[8];
		if(data == NULL) {
			DBG("Internal driver error: data is NULL\n");
			return -EINVAL;
		}
		if((err = ReadPacket(cam, buffer, 8)) < 0) {
			DBG("Error reading command result\n");
		       return err;
		}
		memcpy(data, buffer, databytes);
	} else if(command[0] == DATA_OUT) {
		if(databytes > 0) {
			if(data == NULL) {
				DBG("Internal driver error: data is NULL\n");
				retval = -EINVAL;
			} else {
				if((err=WritePacket(cam, data, databytes)) < 0){
					DBG("Error writing command data\n");
					return err;
				}
			}
		}
	} else {
		DBG("Unexpected first byte of command: %x\n", command[0]);
		retval = -EINVAL;
	}
	return retval;
}


static int cpia_pp_open(void *privdata)
{
	struct pp_cam_entry *cam = (struct pp_cam_entry *)privdata;

	if (cam == NULL)
		return -EINVAL;

	if(cam->open_count == 0) {
		if (parport_claim(cam->pdev)) {
			DBG("failed to claim the port\n");
			return -EBUSY;
		}
		parport_negotiate(cam->port, IEEE1284_MODE_COMPAT);
		parport_data_forward(cam->port);
		parport_write_control(cam->port, PARPORT_CONTROL_SELECT);
		udelay(50);
		parport_write_control(cam->port,
				      PARPORT_CONTROL_SELECT
				      | PARPORT_CONTROL_INIT);
	}

	++cam->open_count;

	return 0;
}


static int cpia_pp_registerCallback(void *privdata, void (*cb)(void *cbdata), void *cbdata)
{
	struct pp_cam_entry *cam = privdata;
	int retval = 0;

	if(cam->port->irq != PARPORT_IRQ_NONE) {
		cam->cb_func = cb;
		cam->cb_data = cbdata;
		INIT_WORK(&cam->cb_task, cpia_pp_run_callback);
	} else {
		retval = -1;
	}
	return retval;
}


static int cpia_pp_close(void *privdata)
{
	struct pp_cam_entry *cam = privdata;
	if (--cam->open_count == 0) {
		parport_release(cam->pdev);
	}
	return 0;
}


static int cpia_pp_register(struct parport *port)
{
	struct pardevice *pdev = NULL;
	struct pp_cam_entry *cam;
	struct cam_data *cpia;

	if (!(port->modes & PARPORT_MODE_PCSPP)) {
		LOG("port is not supported by CPiA driver\n");
		return -ENXIO;
	}

	cam = kzalloc(sizeof(struct pp_cam_entry), GFP_KERNEL);
	if (cam == NULL) {
		LOG("failed to allocate camera structure\n");
		return -ENOMEM;
	}

	pdev = parport_register_device(port, "cpia_pp", NULL, NULL,
				       NULL, 0, cam);

	if (!pdev) {
		LOG("failed to parport_register_device\n");
		kfree(cam);
		return -ENXIO;
	}

	cam->pdev = pdev;
	cam->port = port;
	init_waitqueue_head(&cam->wq_stream);

	cam->streaming = 0;
	cam->stream_irq = 0;

	if((cpia = cpia_register_camera(&cpia_pp_ops, cam)) == NULL) {
		LOG("failed to cpia_register_camera\n");
		parport_unregister_device(pdev);
		kfree(cam);
		return -ENXIO;
	}
	spin_lock( &cam_list_lock_pp );
	list_add( &cpia->cam_data_list, &cam_list );
	spin_unlock( &cam_list_lock_pp );

	return 0;
}

static void cpia_pp_detach (struct parport *port)
{
	struct list_head *tmp;
	struct cam_data *cpia = NULL;
	struct pp_cam_entry *cam;

	spin_lock( &cam_list_lock_pp );
	list_for_each (tmp, &cam_list) {
		cpia = list_entry(tmp, struct cam_data, cam_data_list);
		cam = (struct pp_cam_entry *) cpia->lowlevel_data;
		if (cam && cam->port->number == port->number) {
			list_del(&cpia->cam_data_list);
			break;
		}
		cpia = NULL;
	}
	spin_unlock( &cam_list_lock_pp );

	if (!cpia) {
		DBG("cpia_pp_detach failed to find cam_data in cam_list\n");
		return;
	}

	cam = (struct pp_cam_entry *) cpia->lowlevel_data;
	cpia_unregister_camera(cpia);
	if(cam->open_count > 0)
		cpia_pp_close(cam);
	parport_unregister_device(cam->pdev);
	cpia->lowlevel_data = NULL;
	kfree(cam);
}

static void cpia_pp_attach (struct parport *port)
{
	unsigned int i;

	switch (parport_nr[0])
	{
	case PPCPIA_PARPORT_UNSPEC:
	case PPCPIA_PARPORT_AUTO:
		if (port->probe_info[0].class != PARPORT_CLASS_MEDIA ||
		    port->probe_info[0].cmdset == NULL ||
		    strncmp(port->probe_info[0].cmdset, "CPIA_1", 6) != 0)
			return;

		cpia_pp_register(port);

		break;

	default:
		for (i = 0; i < PARPORT_MAX; ++i) {
			if (port->number == parport_nr[i]) {
				cpia_pp_register(port);
				break;
			}
		}
		break;
	}
}

static struct parport_driver cpia_pp_driver = {
	.name = "cpia_pp",
	.attach = cpia_pp_attach,
	.detach = cpia_pp_detach,
};

static int __init cpia_pp_init(void)
{
	printk(KERN_INFO "%s v%d.%d.%d\n",ABOUT,
	       CPIA_PP_MAJ_VER,CPIA_PP_MIN_VER,CPIA_PP_PATCH_VER);

	if(parport_nr[0] == PPCPIA_PARPORT_OFF) {
		printk("  disabled\n");
		return 0;
	}

	spin_lock_init( &cam_list_lock_pp );

	if (parport_register_driver (&cpia_pp_driver)) {
		LOG ("unable to register with parport\n");
		return -EIO;
	}
	return 0;
}

static int __init cpia_init(void)
{
	if (parport[0]) {
		
		if (!strncmp(parport[0], "auto", 4)) {
			parport_nr[0] = PPCPIA_PARPORT_AUTO;
		} else {
			int n;
			for (n = 0; n < PARPORT_MAX && parport[n]; n++) {
				if (!strncmp(parport[n], "none", 4)) {
					parport_nr[n] = PPCPIA_PARPORT_NONE;
				} else {
					char *ep;
					unsigned long r = simple_strtoul(parport[n], &ep, 0);
					if (ep != parport[n]) {
						parport_nr[n] = r;
					} else {
						LOG("bad port specifier `%s'\n", parport[n]);
						return -ENODEV;
					}
				}
			}
		}
	}
	return cpia_pp_init();
}

static void __exit cpia_cleanup(void)
{
	parport_unregister_driver(&cpia_pp_driver);
	return;
}

module_init(cpia_init);
module_exit(cpia_cleanup);
