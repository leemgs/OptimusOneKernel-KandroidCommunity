



#define MODULE_NAME "sq905"

#include <linux/workqueue.h>
#include "gspca.h"

MODULE_AUTHOR("Adam Baker <linux@baker-net.org.uk>, "
		"Theodore Kilgore <kilgota@auburn.edu>");
MODULE_DESCRIPTION("GSPCA/SQ905 USB Camera Driver");
MODULE_LICENSE("GPL");


#define SQ905_CMD_TIMEOUT 500
#define SQ905_DATA_TIMEOUT 1000


#define SQ905_MAX_TRANSFER 0x8000
#define FRAME_HEADER_LEN 64





#define SQ905_BULK_READ	0x03	
#define SQ905_COMMAND	0x06	
#define SQ905_PING	0x07	
#define SQ905_READ_DONE 0xc0    


#define SQ905_HIRES_MASK	0x00000300
#define SQ905_ORIENTATION_MASK	0x00000100



#define SQ905_ID      0xf0	
#define SQ905_CONFIG  0x20	
#define SQ905_DATA    0x30	
#define SQ905_CLEAR   0xa0	
#define SQ905_CAPTURE_LOW  0x60	
#define SQ905_CAPTURE_MED  0x61	
#define SQ905_CAPTURE_HIGH 0x62	



struct sd {
	struct gspca_dev gspca_dev;	

	
	struct work_struct work_struct;
	struct workqueue_struct *work_thread;
};

static struct v4l2_pix_format sq905_mode[] = {
	{ 160, 120, V4L2_PIX_FMT_SBGGR8, V4L2_FIELD_NONE,
		.bytesperline = 160,
		.sizeimage = 160 * 120,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0},
	{ 320, 240, V4L2_PIX_FMT_SBGGR8, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0},
	{ 640, 480, V4L2_PIX_FMT_SBGGR8, V4L2_FIELD_NONE,
		.bytesperline = 640,
		.sizeimage = 640 * 480,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0}
};


static int sq905_command(struct gspca_dev *gspca_dev, u16 index)
{
	int ret;

	gspca_dev->usb_buf[0] = '\0';
	ret = usb_control_msg(gspca_dev->dev,
			      usb_sndctrlpipe(gspca_dev->dev, 0),
			      USB_REQ_SYNCH_FRAME,                
			      USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      SQ905_COMMAND, index, gspca_dev->usb_buf, 1,
			      SQ905_CMD_TIMEOUT);
	if (ret < 0) {
		PDEBUG(D_ERR, "%s: usb_control_msg failed (%d)",
			__func__, ret);
		return ret;
	}

	ret = usb_control_msg(gspca_dev->dev,
			      usb_sndctrlpipe(gspca_dev->dev, 0),
			      USB_REQ_SYNCH_FRAME,                
			      USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      SQ905_PING, 0, gspca_dev->usb_buf, 1,
			      SQ905_CMD_TIMEOUT);
	if (ret < 0) {
		PDEBUG(D_ERR, "%s: usb_control_msg failed 2 (%d)",
			__func__, ret);
		return ret;
	}

	return 0;
}


static int sq905_ack_frame(struct gspca_dev *gspca_dev)
{
	int ret;

	gspca_dev->usb_buf[0] = '\0';
	ret = usb_control_msg(gspca_dev->dev,
			      usb_sndctrlpipe(gspca_dev->dev, 0),
			      USB_REQ_SYNCH_FRAME,                
			      USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      SQ905_READ_DONE, 0, gspca_dev->usb_buf, 1,
			      SQ905_CMD_TIMEOUT);
	if (ret < 0) {
		PDEBUG(D_ERR, "%s: usb_control_msg failed (%d)", __func__, ret);
		return ret;
	}

	return 0;
}


static int
sq905_read_data(struct gspca_dev *gspca_dev, u8 *data, int size)
{
	int ret;
	int act_len;

	gspca_dev->usb_buf[0] = '\0';
	ret = usb_control_msg(gspca_dev->dev,
			      usb_sndctrlpipe(gspca_dev->dev, 0),
			      USB_REQ_SYNCH_FRAME,                
			      USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      SQ905_BULK_READ, size, gspca_dev->usb_buf,
			      1, SQ905_CMD_TIMEOUT);
	if (ret < 0) {
		PDEBUG(D_ERR, "%s: usb_control_msg failed (%d)", __func__, ret);
		return ret;
	}
	ret = usb_bulk_msg(gspca_dev->dev,
			   usb_rcvbulkpipe(gspca_dev->dev, 0x81),
			   data, size, &act_len, SQ905_DATA_TIMEOUT);

	
	if (ret < 0 || act_len != size) {
		PDEBUG(D_ERR, "bulk read fail (%d) len %d/%d",
			ret, act_len, size);
		return -EIO;
	}
	return 0;
}


static void sq905_dostream(struct work_struct *work)
{
	struct sd *dev = container_of(work, struct sd, work_struct);
	struct gspca_dev *gspca_dev = &dev->gspca_dev;
	struct gspca_frame *frame;
	int bytes_left; 
	int data_len;   
	int header_read; 
	int discarding; 
	int packet_type;
	int frame_sz;
	int ret;
	u8 *data;
	u8 *buffer;

	buffer = kmalloc(SQ905_MAX_TRANSFER, GFP_KERNEL | GFP_DMA);
	mutex_lock(&gspca_dev->usb_lock);
	if (!buffer) {
		PDEBUG(D_ERR, "Couldn't allocate USB buffer");
		goto quit_stream;
	}

	frame_sz = gspca_dev->cam.cam_mode[gspca_dev->curr_mode].sizeimage
			+ FRAME_HEADER_LEN;

	while (gspca_dev->present && gspca_dev->streaming) {
		
		mutex_unlock(&gspca_dev->usb_lock);
		msleep(1);

		
		bytes_left = frame_sz;
		header_read = 0;
		discarding = 0;

		while (bytes_left > 0) {
			data_len = bytes_left > SQ905_MAX_TRANSFER ?
				SQ905_MAX_TRANSFER : bytes_left;
			mutex_lock(&gspca_dev->usb_lock);
			if (!gspca_dev->present)
				goto quit_stream;
			ret = sq905_read_data(gspca_dev, buffer, data_len);
			if (ret < 0)
				goto quit_stream;
			mutex_unlock(&gspca_dev->usb_lock);
			PDEBUG(D_STREAM,
				"Got %d bytes out of %d for frame",
				data_len, bytes_left);
			bytes_left -= data_len;
			data = buffer;
			if (!header_read) {
				packet_type = FIRST_PACKET;
				
				data += FRAME_HEADER_LEN;
				data_len -= FRAME_HEADER_LEN;
				header_read = 1;
			} else if (bytes_left == 0) {
				packet_type = LAST_PACKET;
			} else {
				packet_type = INTER_PACKET;
			}
			frame = gspca_get_i_frame(gspca_dev);
			if (frame && !discarding) {
				frame = gspca_frame_add(gspca_dev, packet_type,
						frame, data, data_len);
				
				if (packet_type == FIRST_PACKET &&
				    bytes_left == 0)
					frame = gspca_frame_add(gspca_dev,
							LAST_PACKET,
							frame, data, 0);
			} else {
				discarding = 1;
			}
		}
		
		mutex_lock(&gspca_dev->usb_lock);
		if (!gspca_dev->present)
			goto quit_stream;
		ret = sq905_ack_frame(gspca_dev);
		if (ret < 0)
			goto quit_stream;
	}
quit_stream:
	
	if (gspca_dev->present)
		sq905_command(gspca_dev, SQ905_CLEAR);
	mutex_unlock(&gspca_dev->usb_lock);
	kfree(buffer);
}


static int sd_config(struct gspca_dev *gspca_dev,
		const struct usb_device_id *id)
{
	struct cam *cam = &gspca_dev->cam;
	struct sd *dev = (struct sd *) gspca_dev;

	
	cam->bulk = 1;
	cam->bulk_size = 64;

	INIT_WORK(&dev->work_struct, sq905_dostream);

	return 0;
}



static void sd_stop0(struct gspca_dev *gspca_dev)
{
	struct sd *dev = (struct sd *) gspca_dev;

	
	mutex_unlock(&gspca_dev->usb_lock);
	
	destroy_workqueue(dev->work_thread);
	dev->work_thread = NULL;
	mutex_lock(&gspca_dev->usb_lock);
}


static int sd_init(struct gspca_dev *gspca_dev)
{
	u32 ident;
	int ret;

	
	ret = sq905_command(gspca_dev, SQ905_CLEAR);
	if (ret < 0)
		return ret;
	ret = sq905_command(gspca_dev, SQ905_ID);
	if (ret < 0)
		return ret;
	ret = sq905_read_data(gspca_dev, gspca_dev->usb_buf, 4);
	if (ret < 0)
		return ret;
	
	ident = be32_to_cpup((__be32 *)gspca_dev->usb_buf);
	ret = sq905_command(gspca_dev, SQ905_CLEAR);
	if (ret < 0)
		return ret;
	PDEBUG(D_CONF, "SQ905 camera ID %08x detected", ident);
	gspca_dev->cam.cam_mode = sq905_mode;
	gspca_dev->cam.nmodes = ARRAY_SIZE(sq905_mode);
	if (!(ident & SQ905_HIRES_MASK))
		gspca_dev->cam.nmodes--;

	if (ident & SQ905_ORIENTATION_MASK)
		gspca_dev->cam.input_flags = V4L2_IN_ST_VFLIP;
	else
		gspca_dev->cam.input_flags = V4L2_IN_ST_VFLIP |
					     V4L2_IN_ST_HFLIP;
	return 0;
}


static int sd_start(struct gspca_dev *gspca_dev)
{
	struct sd *dev = (struct sd *) gspca_dev;
	int ret;

	
	switch (gspca_dev->curr_mode) {
	default:

		PDEBUG(D_STREAM, "Start streaming at high resolution");
		ret = sq905_command(&dev->gspca_dev, SQ905_CAPTURE_HIGH);
		break;
	case 1:
		PDEBUG(D_STREAM, "Start streaming at medium resolution");
		ret = sq905_command(&dev->gspca_dev, SQ905_CAPTURE_MED);
		break;
	case 0:
		PDEBUG(D_STREAM, "Start streaming at low resolution");
		ret = sq905_command(&dev->gspca_dev, SQ905_CAPTURE_LOW);
	}

	if (ret < 0) {
		PDEBUG(D_ERR, "Start streaming command failed");
		return ret;
	}
	
	dev->work_thread = create_singlethread_workqueue(MODULE_NAME);
	queue_work(dev->work_thread, &dev->work_struct);

	return 0;
}


static const __devinitdata struct usb_device_id device_table[] = {
	{USB_DEVICE(0x2770, 0x9120)},
	{}
};

MODULE_DEVICE_TABLE(usb, device_table);


static const struct sd_desc sd_desc = {
	.name   = MODULE_NAME,
	.config = sd_config,
	.init   = sd_init,
	.start  = sd_start,
	.stop0  = sd_stop0,
};


static int sd_probe(struct usb_interface *intf,
		const struct usb_device_id *id)
{
	return gspca_dev_probe(intf, id,
			&sd_desc,
			sizeof(struct sd),
			THIS_MODULE);
}

static struct usb_driver sd_driver = {
	.name       = MODULE_NAME,
	.id_table   = device_table,
	.probe      = sd_probe,
	.disconnect = gspca_disconnect,
#ifdef CONFIG_PM
	.suspend = gspca_suspend,
	.resume  = gspca_resume,
#endif
};


static int __init sd_mod_init(void)
{
	int ret;

	ret = usb_register(&sd_driver);
	if (ret < 0)
		return ret;
	PDEBUG(D_PROBE, "registered");
	return 0;
}

static void __exit sd_mod_exit(void)
{
	usb_deregister(&sd_driver);
	PDEBUG(D_PROBE, "deregistered");
}

module_init(sd_mod_init);
module_exit(sd_mod_exit);
