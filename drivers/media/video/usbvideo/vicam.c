

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/videodev.h>
#include <linux/usb.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/mutex.h>
#include <linux/firmware.h>
#include <linux/ihex.h>
#include "usbvideo.h"



#ifdef VICAM_DEBUG
#define ADBG(lineno,fmt,args...) printk(fmt, jiffies, __func__, lineno, ##args)
#define DBG(fmt,args...) ADBG((__LINE__),KERN_DEBUG __FILE__"(%ld):%s (%d):"fmt,##args)
#else
#define DBG(fmn,args...) do {} while(0)
#endif

#define DRIVER_AUTHOR           "Joe Burks, jburks@wavicle.org"
#define DRIVER_DESC             "ViCam WebCam Driver"


#define USB_VICAM_VENDOR_ID	0x04c1
#define USB_VICAM_PRODUCT_ID	0x009d
#define USB_COMPRO_VENDOR_ID	0x0602
#define USB_COMPRO_PRODUCT_ID	0x1001

#define VICAM_BYTES_PER_PIXEL   3
#define VICAM_MAX_READ_SIZE     (512*242+128)
#define VICAM_MAX_FRAME_SIZE    (VICAM_BYTES_PER_PIXEL*320*240)
#define VICAM_FRAMES            2

#define VICAM_HEADER_SIZE       64


static void *rvmalloc(unsigned long size)
{
	void *mem;
	unsigned long adr;

	size = PAGE_ALIGN(size);
	mem = vmalloc_32(size);
	if (!mem)
		return NULL;

	memset(mem, 0, size); 
	adr = (unsigned long) mem;
	while (size > 0) {
		SetPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	return mem;
}

static void rvfree(void *mem, unsigned long size)
{
	unsigned long adr;

	if (!mem)
		return;

	adr = (unsigned long) mem;
	while ((long) size > 0) {
		ClearPageReserved(vmalloc_to_page((void *)adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	vfree(mem);
}

struct vicam_camera {
	u16 shutter_speed;	
	u16 gain;		

	u8 *raw_image;		
	u8 *framebuf;		
	u8 *cntrlbuf;		

	struct video_device vdev;	
	struct usb_device *udev;	

	
	struct mutex cam_lock;

	int is_initialized;
	u8 open_count;
	u8 bulkEndpoint;
	int needsDummyRead;
};

static int vicam_probe( struct usb_interface *intf, const struct usb_device_id *id);
static void vicam_disconnect(struct usb_interface *intf);
static void read_frame(struct vicam_camera *cam, int framenum);
static void vicam_decode_color(const u8 *, u8 *);

static int __send_control_msg(struct vicam_camera *cam,
			      u8 request,
			      u16 value,
			      u16 index,
			      unsigned char *cp,
			      u16 size)
{
	int status;

	

	status = usb_control_msg(cam->udev,
				 usb_sndctrlpipe(cam->udev, 0),
				 request,
				 USB_DIR_OUT | USB_TYPE_VENDOR |
				 USB_RECIP_DEVICE, value, index,
				 cp, size, 1000);

	status = min(status, 0);

	if (status < 0) {
		printk(KERN_INFO "Failed sending control message, error %d.\n",
		       status);
	}

	return status;
}

static int send_control_msg(struct vicam_camera *cam,
			    u8 request,
			    u16 value,
			    u16 index,
			    unsigned char *cp,
			    u16 size)
{
	int status = -ENODEV;
	mutex_lock(&cam->cam_lock);
	if (cam->udev) {
		status = __send_control_msg(cam, request, value,
					    index, cp, size);
	}
	mutex_unlock(&cam->cam_lock);
	return status;
}
static int
initialize_camera(struct vicam_camera *cam)
{
	int err;
	const struct ihex_binrec *rec;
	const struct firmware *uninitialized_var(fw);

	err = request_ihex_firmware(&fw, "vicam/firmware.fw", &cam->udev->dev);
	if (err) {
		printk(KERN_ERR "Failed to load \"vicam/firmware.fw\": %d\n",
		       err);
		return err;
	}

	for (rec = (void *)fw->data; rec; rec = ihex_next_binrec(rec)) {
		memcpy(cam->cntrlbuf, rec->data, be16_to_cpu(rec->len));

		err = send_control_msg(cam, 0xff, 0, 0,
				       cam->cntrlbuf, be16_to_cpu(rec->len));
		if (err)
			break;
	}

	release_firmware(fw);

	return err;
}

static int
set_camera_power(struct vicam_camera *cam, int state)
{
	int status;

	if ((status = send_control_msg(cam, 0x50, state, 0, NULL, 0)) < 0)
		return status;

	if (state) {
		send_control_msg(cam, 0x55, 1, 0, NULL, 0);
	}

	return 0;
}

static long
vicam_ioctl(struct file *file, unsigned int ioctlnr, unsigned long arg)
{
	void __user *user_arg = (void __user *)arg;
	struct vicam_camera *cam = file->private_data;
	long retval = 0;

	if (!cam)
		return -ENODEV;

	switch (ioctlnr) {
		
	case VIDIOCGCAP:
		{
			struct video_capability b;

			DBG("VIDIOCGCAP\n");
			memset(&b, 0, sizeof(b));
			strcpy(b.name, "ViCam-based Camera");
			b.type = VID_TYPE_CAPTURE;
			b.channels = 1;
			b.audios = 0;
			b.maxwidth = 320;	
			b.maxheight = 240;
			b.minwidth = 320;	
			b.minheight = 240;

			if (copy_to_user(user_arg, &b, sizeof(b)))
				retval = -EFAULT;

			break;
		}
		
	case VIDIOCGCHAN:
		{
			struct video_channel v;

			DBG("VIDIOCGCHAN\n");
			if (copy_from_user(&v, user_arg, sizeof(v))) {
				retval = -EFAULT;
				break;
			}
			if (v.channel != 0) {
				retval = -EINVAL;
				break;
			}

			v.channel = 0;
			strcpy(v.name, "Camera");
			v.tuners = 0;
			v.flags = 0;
			v.type = VIDEO_TYPE_CAMERA;
			v.norm = 0;

			if (copy_to_user(user_arg, &v, sizeof(v)))
				retval = -EFAULT;
			break;
		}

	case VIDIOCSCHAN:
		{
			int v;

			if (copy_from_user(&v, user_arg, sizeof(v)))
				retval = -EFAULT;
			DBG("VIDIOCSCHAN %d\n", v);

			if (retval == 0 && v != 0)
				retval = -EINVAL;

			break;
		}

		
	case VIDIOCGPICT:
		{
			struct video_picture vp;
			DBG("VIDIOCGPICT\n");
			memset(&vp, 0, sizeof (struct video_picture));
			vp.brightness = cam->gain << 8;
			vp.depth = 24;
			vp.palette = VIDEO_PALETTE_RGB24;
			if (copy_to_user(user_arg, &vp, sizeof (struct video_picture)))
				retval = -EFAULT;
			break;
		}

	case VIDIOCSPICT:
		{
			struct video_picture vp;

			if (copy_from_user(&vp, user_arg, sizeof(vp))) {
				retval = -EFAULT;
				break;
			}

			DBG("VIDIOCSPICT depth = %d, pal = %d\n", vp.depth,
			    vp.palette);

			cam->gain = vp.brightness >> 8;

			if (vp.depth != 24
			    || vp.palette != VIDEO_PALETTE_RGB24)
				retval = -EINVAL;

			break;
		}

		
	case VIDIOCGWIN:
		{
			struct video_window vw;
			vw.x = 0;
			vw.y = 0;
			vw.width = 320;
			vw.height = 240;
			vw.chromakey = 0;
			vw.flags = 0;
			vw.clips = NULL;
			vw.clipcount = 0;

			DBG("VIDIOCGWIN\n");

			if (copy_to_user(user_arg, (void *)&vw, sizeof(vw)))
				retval = -EFAULT;

			
			
			break;
		}

	case VIDIOCSWIN:
		{

			struct video_window vw;

			if (copy_from_user(&vw, user_arg, sizeof(vw))) {
				retval = -EFAULT;
				break;
			}

			DBG("VIDIOCSWIN %d x %d\n", vw.width, vw.height);

			if ( vw.width != 320 || vw.height != 240 )
				retval = -EFAULT;

			break;
		}

		
	case VIDIOCGMBUF:
		{
			struct video_mbuf vm;
			int i;

			DBG("VIDIOCGMBUF\n");
			memset(&vm, 0, sizeof (vm));
			vm.size =
			    VICAM_MAX_FRAME_SIZE * VICAM_FRAMES;
			vm.frames = VICAM_FRAMES;
			for (i = 0; i < VICAM_FRAMES; i++)
				vm.offsets[i] = VICAM_MAX_FRAME_SIZE * i;

			if (copy_to_user(user_arg, (void *)&vm, sizeof(vm)))
				retval = -EFAULT;

			break;
		}

	case VIDIOCMCAPTURE:
		{
			struct video_mmap vm;
			

			if (copy_from_user((void *)&vm, user_arg, sizeof(vm))) {
				retval = -EFAULT;
				break;
			}

			DBG("VIDIOCMCAPTURE frame=%d, height=%d, width=%d, format=%d.\n",vm.frame,vm.width,vm.height,vm.format);

			if ( vm.frame >= VICAM_FRAMES || vm.format != VIDEO_PALETTE_RGB24 )
				retval = -EINVAL;

			
			
			
			
			

			break;
		}

	case VIDIOCSYNC:
		{
			int frame;

			if (copy_from_user((void *)&frame, user_arg, sizeof(int))) {
				retval = -EFAULT;
				break;
			}
			DBG("VIDIOCSYNC: %d\n", frame);

			read_frame(cam, frame);
			vicam_decode_color(cam->raw_image,
					   cam->framebuf +
					   frame * VICAM_MAX_FRAME_SIZE );

			break;
		}

		
	case VIDIOCCAPTURE:
	case VIDIOCGFBUF:
	case VIDIOCSFBUF:
	case VIDIOCKEY:
		retval = -EINVAL;
		break;

		
	case VIDIOCGTUNER:
	case VIDIOCSTUNER:
	case VIDIOCGFREQ:
	case VIDIOCSFREQ:
		retval = -EINVAL;
		break;

		
	case VIDIOCGAUDIO:
	case VIDIOCSAUDIO:
		retval = -EINVAL;
		break;
	default:
		retval = -ENOIOCTLCMD;
		break;
	}

	return retval;
}

static int
vicam_open(struct file *file)
{
	struct vicam_camera *cam = video_drvdata(file);

	DBG("open\n");

	if (!cam) {
		printk(KERN_ERR
		       "vicam video_device improperly initialized");
		return -EINVAL;
	}

	

	lock_kernel();
	if (cam->open_count > 0) {
		printk(KERN_INFO
		       "vicam_open called on already opened camera");
		unlock_kernel();
		return -EBUSY;
	}

	cam->raw_image = kmalloc(VICAM_MAX_READ_SIZE, GFP_KERNEL);
	if (!cam->raw_image) {
		unlock_kernel();
		return -ENOMEM;
	}

	cam->framebuf = rvmalloc(VICAM_MAX_FRAME_SIZE * VICAM_FRAMES);
	if (!cam->framebuf) {
		kfree(cam->raw_image);
		unlock_kernel();
		return -ENOMEM;
	}

	cam->cntrlbuf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!cam->cntrlbuf) {
		kfree(cam->raw_image);
		rvfree(cam->framebuf, VICAM_MAX_FRAME_SIZE * VICAM_FRAMES);
		unlock_kernel();
		return -ENOMEM;
	}

	

	if (!cam->is_initialized) {
		initialize_camera(cam);

		cam->is_initialized = 1;
	}

	set_camera_power(cam, 1);

	cam->needsDummyRead = 1;
	cam->open_count++;

	file->private_data = cam;
	unlock_kernel();

	return 0;
}

static int
vicam_close(struct file *file)
{
	struct vicam_camera *cam = file->private_data;
	int open_count;
	struct usb_device *udev;

	DBG("close\n");

	

	set_camera_power(cam, 0);

	kfree(cam->raw_image);
	rvfree(cam->framebuf, VICAM_MAX_FRAME_SIZE * VICAM_FRAMES);
	kfree(cam->cntrlbuf);

	mutex_lock(&cam->cam_lock);

	cam->open_count--;
	open_count = cam->open_count;
	udev = cam->udev;

	mutex_unlock(&cam->cam_lock);

	if (!open_count && !udev) {
		kfree(cam);
	}

	return 0;
}

static void vicam_decode_color(const u8 *data, u8 *rgb)
{
	

	int i, prevY, nextY;

	prevY = 512;
	nextY = 512;

	data += VICAM_HEADER_SIZE;

	for( i = 0; i < 240; i++, data += 512 ) {
		const int y = ( i * 242 ) / 240;

		int j, prevX, nextX;
		int Y, Cr, Cb;

		if ( y == 242 - 1 ) {
			nextY = -512;
		}

		prevX = 1;
		nextX = 1;

		for ( j = 0; j < 320; j++, rgb += 3 ) {
			const int x = ( j * 512 ) / 320;
			const u8 * const src = &data[x];

			if ( x == 512 - 1 ) {
				nextX = -1;
			}

			Cr = ( src[prevX] - src[0] ) +
				( src[nextX] - src[0] );
			Cr /= 2;

			Cb = ( src[prevY] - src[prevX + prevY] ) +
				( src[prevY] - src[nextX + prevY] ) +
				( src[nextY] - src[prevX + nextY] ) +
				( src[nextY] - src[nextX + nextY] );
			Cb /= 4;

			Y = 1160 * ( src[0] + ( Cr / 2 ) - 16 );

			if ( i & 1 ) {
				int Ct = Cr;
				Cr = Cb;
				Cb = Ct;
			}

			if ( ( x ^ i ) & 1 ) {
				Cr = -Cr;
				Cb = -Cb;
			}

			rgb[0] = clamp( ( ( Y + ( 2017 * Cb ) ) +
					500 ) / 900, 0, 255 );
			rgb[1] = clamp( ( ( Y - ( 392 * Cb ) -
					  ( 813 * Cr ) ) +
					  500 ) / 1000, 0, 255 );
			rgb[2] = clamp( ( ( Y + ( 1594 * Cr ) ) +
					500 ) / 1300, 0, 255 );

			prevX = -1;
		}

		prevY = -512;
	}
}

static void
read_frame(struct vicam_camera *cam, int framenum)
{
	unsigned char *request = cam->cntrlbuf;
	int realShutter;
	int n;
	int actual_length;

	if (cam->needsDummyRead) {
		cam->needsDummyRead = 0;
		read_frame(cam, framenum);
	}

	memset(request, 0, 16);
	request[0] = cam->gain;	

	request[1] = 0;	

	request[2] = 0x90;	
	request[3] = 0x07;	

	if (cam->shutter_speed > 60) {
		
		realShutter =
		    ((-15631900 / cam->shutter_speed) + 260533) / 1000;
		request[4] = realShutter & 0xFF;
		request[5] = (realShutter >> 8) & 0xFF;
		request[6] = 0x03;
		request[7] = 0x01;
	} else {
		
		realShutter = 15600 / cam->shutter_speed - 1;
		request[4] = 0;
		request[5] = 0;
		request[6] = realShutter & 0xFF;
		request[7] = realShutter >> 8;
	}

	
	request[8] = 0;
	

	mutex_lock(&cam->cam_lock);

	if (!cam->udev) {
		goto done;
	}

	n = __send_control_msg(cam, 0x51, 0x80, 0, request, 16);

	if (n < 0) {
		printk(KERN_ERR
		       " Problem sending frame capture control message");
		goto done;
	}

	n = usb_bulk_msg(cam->udev,
			 usb_rcvbulkpipe(cam->udev, cam->bulkEndpoint),
			 cam->raw_image,
			 512 * 242 + 128, &actual_length, 10000);

	if (n < 0) {
		printk(KERN_ERR "Problem during bulk read of frame data: %d\n",
		       n);
	}

 done:
	mutex_unlock(&cam->cam_lock);
}

static ssize_t
vicam_read( struct file *file, char __user *buf, size_t count, loff_t *ppos )
{
	struct vicam_camera *cam = file->private_data;

	DBG("read %d bytes.\n", (int) count);

	if (*ppos >= VICAM_MAX_FRAME_SIZE) {
		*ppos = 0;
		return 0;
	}

	if (*ppos == 0) {
		read_frame(cam, 0);
		vicam_decode_color(cam->raw_image,
				   cam->framebuf +
				   0 * VICAM_MAX_FRAME_SIZE);
	}

	count = min_t(size_t, count, VICAM_MAX_FRAME_SIZE - *ppos);

	if (copy_to_user(buf, &cam->framebuf[*ppos], count)) {
		count = -EFAULT;
	} else {
		*ppos += count;
	}

	if (count == VICAM_MAX_FRAME_SIZE) {
		*ppos = 0;
	}

	return count;
}


static int
vicam_mmap(struct file *file, struct vm_area_struct *vma)
{
	
	unsigned long page, pos;
	unsigned long start = vma->vm_start;
	unsigned long size  = vma->vm_end-vma->vm_start;
	struct vicam_camera *cam = file->private_data;

	if (!cam)
		return -ENODEV;

	DBG("vicam_mmap: %ld\n", size);

	

	pos = (unsigned long)cam->framebuf;
	while (size > 0) {
		page = vmalloc_to_pfn((void *)pos);
		if (remap_pfn_range(vma, start, page, PAGE_SIZE, PAGE_SHARED))
			return -EAGAIN;

		start += PAGE_SIZE;
		pos += PAGE_SIZE;
		if (size > PAGE_SIZE)
			size -= PAGE_SIZE;
		else
			size = 0;
	}

	return 0;
}

static const struct v4l2_file_operations vicam_fops = {
	.owner		= THIS_MODULE,
	.open		= vicam_open,
	.release	= vicam_close,
	.read		= vicam_read,
	.mmap		= vicam_mmap,
	.ioctl		= vicam_ioctl,
};

static struct video_device vicam_template = {
	.name 		= "ViCam-based USB Camera",
	.fops 		= &vicam_fops,
	.minor 		= -1,
	.release 	= video_device_release_empty,
};


static struct usb_device_id vicam_table[] = {
	{USB_DEVICE(USB_VICAM_VENDOR_ID, USB_VICAM_PRODUCT_ID)},
	{USB_DEVICE(USB_COMPRO_VENDOR_ID, USB_COMPRO_PRODUCT_ID)},
	{}			
};

MODULE_DEVICE_TABLE(usb, vicam_table);

static struct usb_driver vicam_driver = {
	.name		= "vicam",
	.probe		= vicam_probe,
	.disconnect	= vicam_disconnect,
	.id_table	= vicam_table
};


static int
vicam_probe( struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	int bulkEndpoint = 0;
	const struct usb_host_interface *interface;
	const struct usb_endpoint_descriptor *endpoint;
	struct vicam_camera *cam;

	printk(KERN_INFO "ViCam based webcam connected\n");

	interface = intf->cur_altsetting;

	DBG(KERN_DEBUG "Interface %d. has %u. endpoints!\n",
	       interface->desc.bInterfaceNumber, (unsigned) (interface->desc.bNumEndpoints));
	endpoint = &interface->endpoint[0].desc;

	if (usb_endpoint_is_bulk_in(endpoint)) {
		
		bulkEndpoint = endpoint->bEndpointAddress;
	} else {
		printk(KERN_ERR
		       "No bulk in endpoint was found ?! (this is bad)\n");
	}

	if ((cam =
	     kzalloc(sizeof (struct vicam_camera), GFP_KERNEL)) == NULL) {
		printk(KERN_WARNING
		       "could not allocate kernel memory for vicam_camera struct\n");
		return -ENOMEM;
	}


	cam->shutter_speed = 15;

	mutex_init(&cam->cam_lock);

	memcpy(&cam->vdev, &vicam_template, sizeof(vicam_template));
	video_set_drvdata(&cam->vdev, cam);

	cam->udev = dev;
	cam->bulkEndpoint = bulkEndpoint;

	if (video_register_device(&cam->vdev, VFL_TYPE_GRABBER, -1) < 0) {
		kfree(cam);
		printk(KERN_WARNING "video_register_device failed\n");
		return -EIO;
	}

	printk(KERN_INFO "ViCam webcam driver now controlling video device %d\n",
			cam->vdev.num);

	usb_set_intfdata (intf, cam);

	return 0;
}

static void
vicam_disconnect(struct usb_interface *intf)
{
	int open_count;
	struct vicam_camera *cam = usb_get_intfdata (intf);
	usb_set_intfdata (intf, NULL);

	

	video_unregister_device(&cam->vdev);

	

	mutex_lock(&cam->cam_lock);

	

	cam->udev = NULL;

	

	open_count = cam->open_count;

	mutex_unlock(&cam->cam_lock);

	if (!open_count) {
		kfree(cam);
	}

	printk(KERN_DEBUG "ViCam-based WebCam disconnected\n");
}


static int __init
usb_vicam_init(void)
{
	int retval;
	DBG(KERN_INFO "ViCam-based WebCam driver startup\n");
	retval = usb_register(&vicam_driver);
	if (retval)
		printk(KERN_WARNING "usb_register failed!\n");
	return retval;
}

static void __exit
usb_vicam_exit(void)
{
	DBG(KERN_INFO
	       "ViCam-based WebCam driver shutdown\n");

	usb_deregister(&vicam_driver);
}

module_init(usb_vicam_init);
module_exit(usb_vicam_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
MODULE_FIRMWARE("vicam/firmware.fw");
