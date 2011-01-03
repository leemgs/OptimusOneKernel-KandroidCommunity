



#define MODULE_NAME "sq905c"

#include <linux/workqueue.h>
#include "gspca.h"

MODULE_AUTHOR("Theodore Kilgore <kilgota@auburn.edu>");
MODULE_DESCRIPTION("GSPCA/SQ905C USB Camera Driver");
MODULE_LICENSE("GPL");


#define SQ905C_CMD_TIMEOUT 500
#define SQ905C_DATA_TIMEOUT 1000


#define SQ905C_MAX_TRANSFER 0x8000

#define FRAME_HEADER_LEN 0x50


#define SQ905C_CLEAR   0xa0		
#define SQ905C_CAPTURE_LOW 0xa040	
#define SQ905C_CAPTURE_MED 0x1440	
#define SQ905C_CAPTURE_HI 0x2840	


#define SQ905C_CAPTURE_INDEX 0x110f


struct sd {
	struct gspca_dev gspca_dev;	
	const struct v4l2_pix_format *cap_mode;
	
	struct work_struct work_struct;
	struct workqueue_struct *work_thread;
};


static struct v4l2_pix_format sq905c_mode[] = {
	{ 320, 240, V4L2_PIX_FMT_SQ905C, V4L2_FIELD_NONE,
		.bytesperline = 320,
		.sizeimage = 320 * 240,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0},
	{ 640, 480, V4L2_PIX_FMT_SQ905C, V4L2_FIELD_NONE,
		.bytesperline = 640,
		.sizeimage = 640 * 480,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0}
};


static int sq905c_command(struct gspca_dev *gspca_dev, u16 command, u16 index)
{
	int ret;

	ret = usb_control_msg(gspca_dev->dev,
			      usb_sndctrlpipe(gspca_dev->dev, 0),
			      USB_REQ_SYNCH_FRAME,                
			      USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			      command, index, NULL, 0,
			      SQ905C_CMD_TIMEOUT);
	if (ret < 0) {
		PDEBUG(D_ERR, "%s: usb_control_msg failed (%d)",
			__func__, ret);
		return ret;
	}

	return 0;
}


static void sq905c_dostream(struct work_struct *work)
{
	struct sd *dev = container_of(work, struct sd, work_struct);
	struct gspca_dev *gspca_dev = &dev->gspca_dev;
	struct gspca_frame *frame;
	int bytes_left; 
	int data_len;   
	int act_len;
	int discarding = 0; 
	int packet_type;
	int ret;
	u8 *buffer;

	buffer = kmalloc(SQ905C_MAX_TRANSFER, GFP_KERNEL | GFP_DMA);
	if (!buffer) {
		PDEBUG(D_ERR, "Couldn't allocate USB buffer");
		goto quit_stream;
	}

	while (gspca_dev->present && gspca_dev->streaming) {
		if (!gspca_dev->present)
			goto quit_stream;
		
		ret = usb_bulk_msg(gspca_dev->dev,
				usb_rcvbulkpipe(gspca_dev->dev, 0x81),
				buffer, FRAME_HEADER_LEN, &act_len,
				SQ905C_DATA_TIMEOUT);
		PDEBUG(D_STREAM,
			"Got %d bytes out of %d for header",
			act_len, FRAME_HEADER_LEN);
		if (ret < 0 || act_len < FRAME_HEADER_LEN)
			goto quit_stream;
		
		bytes_left = buffer[0x40]|(buffer[0x41]<<8)|(buffer[0x42]<<16)
					|(buffer[0x43]<<24);
		PDEBUG(D_STREAM, "bytes_left = 0x%x", bytes_left);
		
		packet_type = FIRST_PACKET;
		frame = gspca_get_i_frame(gspca_dev);
		if (frame && !discarding) {
			gspca_frame_add(gspca_dev, packet_type,
				frame, buffer, FRAME_HEADER_LEN);
			} else
				discarding = 1;
		while (bytes_left > 0) {
			data_len = bytes_left > SQ905C_MAX_TRANSFER ?
				SQ905C_MAX_TRANSFER : bytes_left;
			if (!gspca_dev->present)
				goto quit_stream;
			ret = usb_bulk_msg(gspca_dev->dev,
				usb_rcvbulkpipe(gspca_dev->dev, 0x81),
				buffer, data_len, &act_len,
				SQ905C_DATA_TIMEOUT);
			if (ret < 0 || act_len < data_len)
				goto quit_stream;
			PDEBUG(D_STREAM,
				"Got %d bytes out of %d for frame",
				data_len, bytes_left);
			bytes_left -= data_len;
			if (bytes_left == 0)
				packet_type = LAST_PACKET;
			else
				packet_type = INTER_PACKET;
			frame = gspca_get_i_frame(gspca_dev);
			if (frame && !discarding)
				gspca_frame_add(gspca_dev, packet_type,
						frame, buffer, data_len);
			else
				discarding = 1;
		}
	}
quit_stream:
	mutex_lock(&gspca_dev->usb_lock);
	if (gspca_dev->present)
		sq905c_command(gspca_dev, SQ905C_CLEAR, 0);
	mutex_unlock(&gspca_dev->usb_lock);
	kfree(buffer);
}


static int sd_config(struct gspca_dev *gspca_dev,
		const struct usb_device_id *id)
{
	struct cam *cam = &gspca_dev->cam;
	struct sd *dev = (struct sd *) gspca_dev;

	PDEBUG(D_PROBE,
		"SQ9050 camera detected"
		" (vid/pid 0x%04X:0x%04X)", id->idVendor, id->idProduct);
	cam->cam_mode = sq905c_mode;
	cam->nmodes = 2;
	if (id->idProduct == 0x9050)
		cam->nmodes = 1;
	
	cam->bulk_size = 32;
	cam->bulk = 1;
	INIT_WORK(&dev->work_struct, sq905c_dostream);
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
	int ret;

	
	ret = sq905c_command(gspca_dev, SQ905C_CLEAR, 0);
	return ret;
}


static int sd_start(struct gspca_dev *gspca_dev)
{
	struct sd *dev = (struct sd *) gspca_dev;
	int ret;

	dev->cap_mode = gspca_dev->cam.cam_mode;
	
	switch (gspca_dev->width) {
	case 640:
		PDEBUG(D_STREAM, "Start streaming at high resolution");
		dev->cap_mode++;
		ret = sq905c_command(gspca_dev, SQ905C_CAPTURE_HI,
						SQ905C_CAPTURE_INDEX);
		break;
	default: 
	PDEBUG(D_STREAM, "Start streaming at medium resolution");
		ret = sq905c_command(gspca_dev, SQ905C_CAPTURE_MED,
						SQ905C_CAPTURE_INDEX);
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
	{USB_DEVICE(0x2770, 0x905c)},
	{USB_DEVICE(0x2770, 0x9050)},
	{USB_DEVICE(0x2770, 0x913d)},
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
