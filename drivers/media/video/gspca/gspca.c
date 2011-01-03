

#define MODULE_NAME "gspca"

#include <linux/init.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/pagemap.h>
#include <linux/io.h>
#include <asm/page.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <media/v4l2-ioctl.h>

#include "gspca.h"


#define DEF_NURBS 3		
#if DEF_NURBS > MAX_NURBS
#error "DEF_NURBS too big"
#endif

MODULE_AUTHOR("Jean-Francois Moine <http://moinejf.free.fr>");
MODULE_DESCRIPTION("GSPCA USB Camera Driver");
MODULE_LICENSE("GPL");

#define DRIVER_VERSION_NUMBER	KERNEL_VERSION(2, 7, 0)

#ifdef GSPCA_DEBUG
int gspca_debug = D_ERR | D_PROBE;
EXPORT_SYMBOL(gspca_debug);

static void PDEBUG_MODE(char *txt, __u32 pixfmt, int w, int h)
{
	if ((pixfmt >> 24) >= '0' && (pixfmt >> 24) <= 'z') {
		PDEBUG(D_CONF|D_STREAM, "%s %c%c%c%c %dx%d",
			txt,
			pixfmt & 0xff,
			(pixfmt >> 8) & 0xff,
			(pixfmt >> 16) & 0xff,
			pixfmt >> 24,
			w, h);
	} else {
		PDEBUG(D_CONF|D_STREAM, "%s 0x%08x %dx%d",
			txt,
			pixfmt,
			w, h);
	}
}
#else
#define PDEBUG_MODE(txt, pixfmt, w, h)
#endif


#define GSPCA_MEMORY_NO 0	
#define GSPCA_MEMORY_READ 7

#define BUF_ALL_FLAGS (V4L2_BUF_FLAG_QUEUED | V4L2_BUF_FLAG_DONE)


static void gspca_vm_open(struct vm_area_struct *vma)
{
	struct gspca_frame *frame = vma->vm_private_data;

	frame->vma_use_count++;
	frame->v4l2_buf.flags |= V4L2_BUF_FLAG_MAPPED;
}

static void gspca_vm_close(struct vm_area_struct *vma)
{
	struct gspca_frame *frame = vma->vm_private_data;

	if (--frame->vma_use_count <= 0)
		frame->v4l2_buf.flags &= ~V4L2_BUF_FLAG_MAPPED;
}

static const struct vm_operations_struct gspca_vm_ops = {
	.open		= gspca_vm_open,
	.close		= gspca_vm_close,
};


struct gspca_frame *gspca_get_i_frame(struct gspca_dev *gspca_dev)
{
	struct gspca_frame *frame;
	int i;

	i = gspca_dev->fr_i;
	i = gspca_dev->fr_queue[i];
	frame = &gspca_dev->frame[i];
	if ((frame->v4l2_buf.flags & BUF_ALL_FLAGS)
				!= V4L2_BUF_FLAG_QUEUED)
		return NULL;
	return frame;
}
EXPORT_SYMBOL(gspca_get_i_frame);


static void fill_frame(struct gspca_dev *gspca_dev,
			struct urb *urb)
{
	struct gspca_frame *frame;
	u8 *data;		
	int i, len, st;
	cam_pkt_op pkt_scan;

	if (urb->status != 0) {
		if (urb->status == -ESHUTDOWN)
			return;		
#ifdef CONFIG_PM
		if (!gspca_dev->frozen)
#endif
			PDEBUG(D_ERR|D_PACK, "urb status: %d", urb->status);
		return;
	}
	pkt_scan = gspca_dev->sd_desc->pkt_scan;
	for (i = 0; i < urb->number_of_packets; i++) {

		
		frame = gspca_get_i_frame(gspca_dev);
		if (!frame) {
			gspca_dev->last_packet_type = DISCARD_PACKET;
			break;
		}

		
		len = urb->iso_frame_desc[i].actual_length;
		if (len == 0) {
			if (gspca_dev->empty_packet == 0)
				gspca_dev->empty_packet = 1;
			continue;
		}
		st = urb->iso_frame_desc[i].status;
		if (st) {
			PDEBUG(D_ERR,
				"ISOC data error: [%d] len=%d, status=%d",
				i, len, st);
			gspca_dev->last_packet_type = DISCARD_PACKET;
			continue;
		}

		
		PDEBUG(D_PACK, "packet [%d] o:%d l:%d",
			i, urb->iso_frame_desc[i].offset, len);
		data = (u8 *) urb->transfer_buffer
					+ urb->iso_frame_desc[i].offset;
		pkt_scan(gspca_dev, frame, data, len);
	}

	
	st = usb_submit_urb(urb, GFP_ATOMIC);
	if (st < 0)
		PDEBUG(D_ERR|D_PACK, "usb_submit_urb() ret %d", st);
}


static void isoc_irq(struct urb *urb)
{
	struct gspca_dev *gspca_dev = (struct gspca_dev *) urb->context;

	PDEBUG(D_PACK, "isoc irq");
	if (!gspca_dev->streaming)
		return;
	fill_frame(gspca_dev, urb);
}


static void bulk_irq(struct urb *urb)
{
	struct gspca_dev *gspca_dev = (struct gspca_dev *) urb->context;
	struct gspca_frame *frame;
	int st;

	PDEBUG(D_PACK, "bulk irq");
	if (!gspca_dev->streaming)
		return;
	switch (urb->status) {
	case 0:
		break;
	case -ESHUTDOWN:
		return;		
	case -ECONNRESET:
		urb->status = 0;
		break;
	default:
#ifdef CONFIG_PM
		if (!gspca_dev->frozen)
#endif
			PDEBUG(D_ERR|D_PACK, "urb status: %d", urb->status);
		return;
	}

	
	frame = gspca_get_i_frame(gspca_dev);
	if (!frame) {
		gspca_dev->last_packet_type = DISCARD_PACKET;
	} else {
		PDEBUG(D_PACK, "packet l:%d", urb->actual_length);
		gspca_dev->sd_desc->pkt_scan(gspca_dev,
					frame,
					urb->transfer_buffer,
					urb->actual_length);
	}

	
	if (gspca_dev->cam.bulk_nurbs != 0) {
		st = usb_submit_urb(urb, GFP_ATOMIC);
		if (st < 0)
			PDEBUG(D_ERR|D_PACK, "usb_submit_urb() ret %d", st);
	}
}


struct gspca_frame *gspca_frame_add(struct gspca_dev *gspca_dev,
				    enum gspca_packet_type packet_type,
				    struct gspca_frame *frame,
				    const __u8 *data,
				    int len)
{
	int i, j;

	PDEBUG(D_PACK, "add t:%d l:%d",	packet_type, len);

	
	if (packet_type == FIRST_PACKET) {
		if ((frame->v4l2_buf.flags & BUF_ALL_FLAGS)
						!= V4L2_BUF_FLAG_QUEUED) {
			gspca_dev->last_packet_type = DISCARD_PACKET;
			return frame;
		}
		frame->data_end = frame->data;
		jiffies_to_timeval(get_jiffies_64(),
				   &frame->v4l2_buf.timestamp);
		frame->v4l2_buf.sequence = ++gspca_dev->sequence;
	} else if (gspca_dev->last_packet_type == DISCARD_PACKET) {
		if (packet_type == LAST_PACKET)
			gspca_dev->last_packet_type = packet_type;
		return frame;
	}

	
	if (len > 0) {
		if (frame->data_end - frame->data + len
						 > frame->v4l2_buf.length) {
			PDEBUG(D_ERR|D_PACK, "frame overflow %zd > %d",
				frame->data_end - frame->data + len,
				frame->v4l2_buf.length);
			packet_type = DISCARD_PACKET;
		} else {
			memcpy(frame->data_end, data, len);
			frame->data_end += len;
		}
	}
	gspca_dev->last_packet_type = packet_type;

	
	if (packet_type == LAST_PACKET) {
		frame->v4l2_buf.bytesused = frame->data_end - frame->data;
		frame->v4l2_buf.flags &= ~V4L2_BUF_FLAG_QUEUED;
		frame->v4l2_buf.flags |= V4L2_BUF_FLAG_DONE;
		wake_up_interruptible(&gspca_dev->wq);	
		i = (gspca_dev->fr_i + 1) % gspca_dev->nframes;
		gspca_dev->fr_i = i;
		PDEBUG(D_FRAM, "frame complete len:%d q:%d i:%d o:%d",
			frame->v4l2_buf.bytesused,
			gspca_dev->fr_q,
			i,
			gspca_dev->fr_o);
		j = gspca_dev->fr_queue[i];
		frame = &gspca_dev->frame[j];
	}
	return frame;
}
EXPORT_SYMBOL(gspca_frame_add);

static int gspca_is_compressed(__u32 format)
{
	switch (format) {
	case V4L2_PIX_FMT_MJPEG:
	case V4L2_PIX_FMT_JPEG:
	case V4L2_PIX_FMT_SPCA561:
	case V4L2_PIX_FMT_PAC207:
	case V4L2_PIX_FMT_MR97310A:
		return 1;
	}
	return 0;
}

static void *rvmalloc(unsigned long size)
{
	void *mem;
	unsigned long adr;

	mem = vmalloc_32(size);
	if (mem != NULL) {
		adr = (unsigned long) mem;
		while ((long) size > 0) {
			SetPageReserved(vmalloc_to_page((void *) adr));
			adr += PAGE_SIZE;
			size -= PAGE_SIZE;
		}
	}
	return mem;
}

static void rvfree(void *mem, long size)
{
	unsigned long adr;

	adr = (unsigned long) mem;
	while (size > 0) {
		ClearPageReserved(vmalloc_to_page((void *) adr));
		adr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}
	vfree(mem);
}

static int frame_alloc(struct gspca_dev *gspca_dev,
			unsigned int count)
{
	struct gspca_frame *frame;
	unsigned int frsz;
	int i;

	i = gspca_dev->curr_mode;
	frsz = gspca_dev->cam.cam_mode[i].sizeimage;
	PDEBUG(D_STREAM, "frame alloc frsz: %d", frsz);
	frsz = PAGE_ALIGN(frsz);
	gspca_dev->frsz = frsz;
	if (count > GSPCA_MAX_FRAMES)
		count = GSPCA_MAX_FRAMES;
	gspca_dev->frbuf = rvmalloc(frsz * count);
	if (!gspca_dev->frbuf) {
		err("frame alloc failed");
		return -ENOMEM;
	}
	gspca_dev->nframes = count;
	for (i = 0; i < count; i++) {
		frame = &gspca_dev->frame[i];
		frame->v4l2_buf.index = i;
		frame->v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		frame->v4l2_buf.flags = 0;
		frame->v4l2_buf.field = V4L2_FIELD_NONE;
		frame->v4l2_buf.length = frsz;
		frame->v4l2_buf.memory = gspca_dev->memory;
		frame->v4l2_buf.sequence = 0;
		frame->data = frame->data_end =
					gspca_dev->frbuf + i * frsz;
		frame->v4l2_buf.m.offset = i * frsz;
	}
	gspca_dev->fr_i = gspca_dev->fr_o = gspca_dev->fr_q = 0;
	gspca_dev->last_packet_type = DISCARD_PACKET;
	gspca_dev->sequence = 0;
	return 0;
}

static void frame_free(struct gspca_dev *gspca_dev)
{
	int i;

	PDEBUG(D_STREAM, "frame free");
	if (gspca_dev->frbuf != NULL) {
		rvfree(gspca_dev->frbuf,
			gspca_dev->nframes * gspca_dev->frsz);
		gspca_dev->frbuf = NULL;
		for (i = 0; i < gspca_dev->nframes; i++)
			gspca_dev->frame[i].data = NULL;
	}
	gspca_dev->nframes = 0;
}

static void destroy_urbs(struct gspca_dev *gspca_dev)
{
	struct urb *urb;
	unsigned int i;

	PDEBUG(D_STREAM, "kill transfer");
	for (i = 0; i < MAX_NURBS; i++) {
		urb = gspca_dev->urb[i];
		if (urb == NULL)
			break;

		gspca_dev->urb[i] = NULL;
		usb_kill_urb(urb);
		if (urb->transfer_buffer != NULL)
			usb_buffer_free(gspca_dev->dev,
					urb->transfer_buffer_length,
					urb->transfer_buffer,
					urb->transfer_dma);
		usb_free_urb(urb);
	}
}


static struct usb_host_endpoint *alt_xfer(struct usb_host_interface *alt,
					  int xfer)
{
	struct usb_host_endpoint *ep;
	int i, attr;

	for (i = 0; i < alt->desc.bNumEndpoints; i++) {
		ep = &alt->endpoint[i];
		attr = ep->desc.bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
		if (attr == xfer
		    && ep->desc.wMaxPacketSize != 0)
			return ep;
	}
	return NULL;
}


static struct usb_host_endpoint *get_ep(struct gspca_dev *gspca_dev)
{
	struct usb_interface *intf;
	struct usb_host_endpoint *ep;
	int xfer, i, ret;

	intf = usb_ifnum_to_if(gspca_dev->dev, gspca_dev->iface);
	ep = NULL;
	xfer = gspca_dev->cam.bulk ? USB_ENDPOINT_XFER_BULK
				   : USB_ENDPOINT_XFER_ISOC;
	i = gspca_dev->alt;			
	while (--i >= 0) {
		ep = alt_xfer(&intf->altsetting[i], xfer);
		if (ep)
			break;
	}
	if (ep == NULL) {
		err("no transfer endpoint found");
		return NULL;
	}
	PDEBUG(D_STREAM, "use alt %d ep 0x%02x",
			i, ep->desc.bEndpointAddress);
	gspca_dev->alt = i;		
	if (gspca_dev->nbalt > 1) {
		ret = usb_set_interface(gspca_dev->dev, gspca_dev->iface, i);
		if (ret < 0) {
			err("set alt %d err %d", i, ret);
			return NULL;
		}
	}
	return ep;
}


static int create_urbs(struct gspca_dev *gspca_dev,
			struct usb_host_endpoint *ep)
{
	struct urb *urb;
	int n, nurbs, i, psize, npkt, bsize;

	
	psize = le16_to_cpu(ep->desc.wMaxPacketSize);

	if (!gspca_dev->cam.bulk) {		

		
		if (gspca_dev->pkt_size == 0)
			psize = (psize & 0x07ff) * (1 + ((psize >> 11) & 3));
		else
			psize = gspca_dev->pkt_size;
		npkt = gspca_dev->cam.npkt;
		if (npkt == 0)
			npkt = 32;		
		bsize = psize * npkt;
		PDEBUG(D_STREAM,
			"isoc %d pkts size %d = bsize:%d",
			npkt, psize, bsize);
		nurbs = DEF_NURBS;
	} else {				
		npkt = 0;
		bsize = gspca_dev->cam.bulk_size;
		if (bsize == 0)
			bsize = psize;
		PDEBUG(D_STREAM, "bulk bsize:%d", bsize);
		if (gspca_dev->cam.bulk_nurbs != 0)
			nurbs = gspca_dev->cam.bulk_nurbs;
		else
			nurbs = 1;
	}

	gspca_dev->nurbs = nurbs;
	for (n = 0; n < nurbs; n++) {
		urb = usb_alloc_urb(npkt, GFP_KERNEL);
		if (!urb) {
			err("usb_alloc_urb failed");
			destroy_urbs(gspca_dev);
			return -ENOMEM;
		}
		urb->transfer_buffer = usb_buffer_alloc(gspca_dev->dev,
						bsize,
						GFP_KERNEL,
						&urb->transfer_dma);

		if (urb->transfer_buffer == NULL) {
			usb_free_urb(urb);
			err("usb_buffer_urb failed");
			destroy_urbs(gspca_dev);
			return -ENOMEM;
		}
		gspca_dev->urb[n] = urb;
		urb->dev = gspca_dev->dev;
		urb->context = gspca_dev;
		urb->transfer_buffer_length = bsize;
		if (npkt != 0) {		
			urb->pipe = usb_rcvisocpipe(gspca_dev->dev,
						    ep->desc.bEndpointAddress);
			urb->transfer_flags = URB_ISO_ASAP
					| URB_NO_TRANSFER_DMA_MAP;
			urb->interval = ep->desc.bInterval;
			urb->complete = isoc_irq;
			urb->number_of_packets = npkt;
			for (i = 0; i < npkt; i++) {
				urb->iso_frame_desc[i].length = psize;
				urb->iso_frame_desc[i].offset = psize * i;
			}
		} else {		
			urb->pipe = usb_rcvbulkpipe(gspca_dev->dev,
						ep->desc.bEndpointAddress),
			urb->transfer_flags = URB_NO_TRANSFER_DMA_MAP;
			urb->complete = bulk_irq;
		}
	}
	return 0;
}


static int gspca_init_transfer(struct gspca_dev *gspca_dev)
{
	struct usb_host_endpoint *ep;
	int n, ret;

	if (mutex_lock_interruptible(&gspca_dev->usb_lock))
		return -ERESTARTSYS;

	if (!gspca_dev->present) {
		ret = -ENODEV;
		goto out;
	}

	
	gspca_dev->alt = gspca_dev->nbalt;
	if (gspca_dev->sd_desc->isoc_init) {
		ret = gspca_dev->sd_desc->isoc_init(gspca_dev);
		if (ret < 0)
			goto out;
	}
	ep = get_ep(gspca_dev);
	if (ep == NULL) {
		ret = -EIO;
		goto out;
	}
	for (;;) {
		PDEBUG(D_STREAM, "init transfer alt %d", gspca_dev->alt);
		ret = create_urbs(gspca_dev, ep);
		if (ret < 0)
			goto out;

		
		if (gspca_dev->cam.bulk)
			usb_clear_halt(gspca_dev->dev,
					gspca_dev->urb[0]->pipe);

		
		ret = gspca_dev->sd_desc->start(gspca_dev);
		if (ret < 0) {
			destroy_urbs(gspca_dev);
			goto out;
		}
		gspca_dev->streaming = 1;

		
		if (gspca_dev->cam.bulk && gspca_dev->cam.bulk_nurbs == 0)
			break;

		
		for (n = 0; n < gspca_dev->nurbs; n++) {
			ret = usb_submit_urb(gspca_dev->urb[n], GFP_KERNEL);
			if (ret < 0)
				break;
		}
		if (ret >= 0)
			break;
		PDEBUG(D_ERR|D_STREAM,
			"usb_submit_urb alt %d err %d", gspca_dev->alt, ret);
		gspca_dev->streaming = 0;
		destroy_urbs(gspca_dev);
		if (ret != -ENOSPC)
			goto out;

		
		msleep(20);	
		if (gspca_dev->sd_desc->isoc_nego) {
			ret = gspca_dev->sd_desc->isoc_nego(gspca_dev);
			if (ret < 0)
				goto out;
		} else {
			ep = get_ep(gspca_dev);
			if (ep == NULL) {
				ret = -EIO;
				goto out;
			}
		}
	}
out:
	mutex_unlock(&gspca_dev->usb_lock);
	return ret;
}

static int gspca_set_alt0(struct gspca_dev *gspca_dev)
{
	int ret;

	if (gspca_dev->alt == 0)
		return 0;
	ret = usb_set_interface(gspca_dev->dev, gspca_dev->iface, 0);
	if (ret < 0)
		PDEBUG(D_ERR|D_STREAM, "set alt 0 err %d", ret);
	return ret;
}


static void gspca_stream_off(struct gspca_dev *gspca_dev)
{
	gspca_dev->streaming = 0;
	if (gspca_dev->present) {
		if (gspca_dev->sd_desc->stopN)
			gspca_dev->sd_desc->stopN(gspca_dev);
		destroy_urbs(gspca_dev);
		gspca_set_alt0(gspca_dev);
	}

	
	if (gspca_dev->sd_desc->stop0)
		gspca_dev->sd_desc->stop0(gspca_dev);
	PDEBUG(D_STREAM, "stream off OK");
}

static void gspca_set_default_mode(struct gspca_dev *gspca_dev)
{
	int i;

	i = gspca_dev->cam.nmodes - 1;	
	gspca_dev->curr_mode = i;
	gspca_dev->width = gspca_dev->cam.cam_mode[i].width;
	gspca_dev->height = gspca_dev->cam.cam_mode[i].height;
	gspca_dev->pixfmt = gspca_dev->cam.cam_mode[i].pixelformat;
}

static int wxh_to_mode(struct gspca_dev *gspca_dev,
			int width, int height)
{
	int i;

	for (i = gspca_dev->cam.nmodes; --i > 0; ) {
		if (width >= gspca_dev->cam.cam_mode[i].width
		    && height >= gspca_dev->cam.cam_mode[i].height)
			break;
	}
	return i;
}


static int gspca_get_mode(struct gspca_dev *gspca_dev,
			int mode,
			int pixfmt)
{
	int modeU, modeD;

	modeU = modeD = mode;
	while ((modeU < gspca_dev->cam.nmodes) || modeD >= 0) {
		if (--modeD >= 0) {
			if (gspca_dev->cam.cam_mode[modeD].pixelformat
								== pixfmt)
				return modeD;
		}
		if (++modeU < gspca_dev->cam.nmodes) {
			if (gspca_dev->cam.cam_mode[modeU].pixelformat
								== pixfmt)
				return modeU;
		}
	}
	return -EINVAL;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int vidioc_g_register(struct file *file, void *priv,
			struct v4l2_dbg_register *reg)
{
	int ret;
	struct gspca_dev *gspca_dev = priv;

	if (!gspca_dev->sd_desc->get_chip_ident)
		return -EINVAL;

	if (!gspca_dev->sd_desc->get_register)
		return -EINVAL;

	if (mutex_lock_interruptible(&gspca_dev->usb_lock))
		return -ERESTARTSYS;
	if (gspca_dev->present)
		ret = gspca_dev->sd_desc->get_register(gspca_dev, reg);
	else
		ret = -ENODEV;
	mutex_unlock(&gspca_dev->usb_lock);

	return ret;
}

static int vidioc_s_register(struct file *file, void *priv,
			struct v4l2_dbg_register *reg)
{
	int ret;
	struct gspca_dev *gspca_dev = priv;

	if (!gspca_dev->sd_desc->get_chip_ident)
		return -EINVAL;

	if (!gspca_dev->sd_desc->set_register)
		return -EINVAL;

	if (mutex_lock_interruptible(&gspca_dev->usb_lock))
		return -ERESTARTSYS;
	if (gspca_dev->present)
		ret = gspca_dev->sd_desc->set_register(gspca_dev, reg);
	else
		ret = -ENODEV;
	mutex_unlock(&gspca_dev->usb_lock);

	return ret;
}
#endif

static int vidioc_g_chip_ident(struct file *file, void *priv,
			struct v4l2_dbg_chip_ident *chip)
{
	int ret;
	struct gspca_dev *gspca_dev = priv;

	if (!gspca_dev->sd_desc->get_chip_ident)
		return -EINVAL;

	if (mutex_lock_interruptible(&gspca_dev->usb_lock))
		return -ERESTARTSYS;
	if (gspca_dev->present)
		ret = gspca_dev->sd_desc->get_chip_ident(gspca_dev, chip);
	else
		ret = -ENODEV;
	mutex_unlock(&gspca_dev->usb_lock);

	return ret;
}

static int vidioc_enum_fmt_vid_cap(struct file *file, void  *priv,
				struct v4l2_fmtdesc *fmtdesc)
{
	struct gspca_dev *gspca_dev = priv;
	int i, j, index;
	__u32 fmt_tb[8];

	
	index = 0;
	j = 0;
	for (i = gspca_dev->cam.nmodes; --i >= 0; ) {
		fmt_tb[index] = gspca_dev->cam.cam_mode[i].pixelformat;
		j = 0;
		for (;;) {
			if (fmt_tb[j] == fmt_tb[index])
				break;
			j++;
		}
		if (j == index) {
			if (fmtdesc->index == index)
				break;		
			index++;
			if (index >= ARRAY_SIZE(fmt_tb))
				return -EINVAL;
		}
	}
	if (i < 0)
		return -EINVAL;		

	fmtdesc->pixelformat = fmt_tb[index];
	if (gspca_is_compressed(fmt_tb[index]))
		fmtdesc->flags = V4L2_FMT_FLAG_COMPRESSED;
	fmtdesc->description[0] = fmtdesc->pixelformat & 0xff;
	fmtdesc->description[1] = (fmtdesc->pixelformat >> 8) & 0xff;
	fmtdesc->description[2] = (fmtdesc->pixelformat >> 16) & 0xff;
	fmtdesc->description[3] = fmtdesc->pixelformat >> 24;
	fmtdesc->description[4] = '\0';
	return 0;
}

static int vidioc_g_fmt_vid_cap(struct file *file, void *priv,
			    struct v4l2_format *fmt)
{
	struct gspca_dev *gspca_dev = priv;
	int mode;

	mode = gspca_dev->curr_mode;
	memcpy(&fmt->fmt.pix, &gspca_dev->cam.cam_mode[mode],
		sizeof fmt->fmt.pix);
	return 0;
}

static int try_fmt_vid_cap(struct gspca_dev *gspca_dev,
			struct v4l2_format *fmt)
{
	int w, h, mode, mode2;

	w = fmt->fmt.pix.width;
	h = fmt->fmt.pix.height;

#ifdef GSPCA_DEBUG
	if (gspca_debug & D_CONF)
		PDEBUG_MODE("try fmt cap", fmt->fmt.pix.pixelformat, w, h);
#endif
	
	mode = wxh_to_mode(gspca_dev, w, h);

	
	if (gspca_dev->cam.cam_mode[mode].pixelformat
						!= fmt->fmt.pix.pixelformat) {

		
		mode2 = gspca_get_mode(gspca_dev, mode,
					fmt->fmt.pix.pixelformat);
		if (mode2 >= 0)
			mode = mode2;

	}
	memcpy(&fmt->fmt.pix, &gspca_dev->cam.cam_mode[mode],
		sizeof fmt->fmt.pix);
	return mode;			
}

static int vidioc_try_fmt_vid_cap(struct file *file,
			      void *priv,
			      struct v4l2_format *fmt)
{
	struct gspca_dev *gspca_dev = priv;
	int ret;

	ret = try_fmt_vid_cap(gspca_dev, fmt);
	if (ret < 0)
		return ret;
	return 0;
}

static int vidioc_s_fmt_vid_cap(struct file *file, void *priv,
			    struct v4l2_format *fmt)
{
	struct gspca_dev *gspca_dev = priv;
	int ret;

	if (mutex_lock_interruptible(&gspca_dev->queue_lock))
		return -ERESTARTSYS;

	ret = try_fmt_vid_cap(gspca_dev, fmt);
	if (ret < 0)
		goto out;

	if (gspca_dev->nframes != 0
	    && fmt->fmt.pix.sizeimage > gspca_dev->frsz) {
		ret = -EINVAL;
		goto out;
	}

	if (ret == gspca_dev->curr_mode) {
		ret = 0;
		goto out;			
	}

	if (gspca_dev->streaming) {
		ret = -EBUSY;
		goto out;
	}
	gspca_dev->width = fmt->fmt.pix.width;
	gspca_dev->height = fmt->fmt.pix.height;
	gspca_dev->pixfmt = fmt->fmt.pix.pixelformat;
	gspca_dev->curr_mode = ret;

	ret = 0;
out:
	mutex_unlock(&gspca_dev->queue_lock);
	return ret;
}

static int vidioc_enum_framesizes(struct file *file, void *priv,
				  struct v4l2_frmsizeenum *fsize)
{
	struct gspca_dev *gspca_dev = priv;
	int i;
	__u32 index = 0;

	for (i = 0; i < gspca_dev->cam.nmodes; i++) {
		if (fsize->pixel_format !=
				gspca_dev->cam.cam_mode[i].pixelformat)
			continue;

		if (fsize->index == index) {
			fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
			fsize->discrete.width =
				gspca_dev->cam.cam_mode[i].width;
			fsize->discrete.height =
				gspca_dev->cam.cam_mode[i].height;
			return 0;
		}
		index++;
	}

	return -EINVAL;
}

static void gspca_release(struct video_device *vfd)
{
	struct gspca_dev *gspca_dev = container_of(vfd, struct gspca_dev, vdev);

	PDEBUG(D_STREAM, "device released");

	kfree(gspca_dev->usb_buf);
	kfree(gspca_dev);
}

static int dev_open(struct file *file)
{
	struct gspca_dev *gspca_dev;
	int ret;

	PDEBUG(D_STREAM, "%s open", current->comm);
	gspca_dev = (struct gspca_dev *) video_devdata(file);
	if (mutex_lock_interruptible(&gspca_dev->queue_lock))
		return -ERESTARTSYS;
	if (!gspca_dev->present) {
		ret = -ENODEV;
		goto out;
	}

	if (gspca_dev->users > 4) {	
		ret = -EBUSY;
		goto out;
	}

	
	if (!try_module_get(gspca_dev->module)) {
		ret = -ENODEV;
		goto out;
	}

	gspca_dev->users++;

	file->private_data = gspca_dev;
#ifdef GSPCA_DEBUG
	
	if (gspca_debug & D_V4L2)
		gspca_dev->vdev.debug |= V4L2_DEBUG_IOCTL
					| V4L2_DEBUG_IOCTL_ARG;
	else
		gspca_dev->vdev.debug &= ~(V4L2_DEBUG_IOCTL
					| V4L2_DEBUG_IOCTL_ARG);
#endif
	ret = 0;
out:
	mutex_unlock(&gspca_dev->queue_lock);
	if (ret != 0)
		PDEBUG(D_ERR|D_STREAM, "open failed err %d", ret);
	else
		PDEBUG(D_STREAM, "open done");
	return ret;
}

static int dev_close(struct file *file)
{
	struct gspca_dev *gspca_dev = file->private_data;

	PDEBUG(D_STREAM, "%s close", current->comm);
	if (mutex_lock_interruptible(&gspca_dev->queue_lock))
		return -ERESTARTSYS;
	gspca_dev->users--;

	
	if (gspca_dev->capt_file == file) {
		if (gspca_dev->streaming) {
			mutex_lock(&gspca_dev->usb_lock);
			gspca_stream_off(gspca_dev);
			mutex_unlock(&gspca_dev->usb_lock);
		}
		frame_free(gspca_dev);
		gspca_dev->capt_file = NULL;
		gspca_dev->memory = GSPCA_MEMORY_NO;
	}
	file->private_data = NULL;
	module_put(gspca_dev->module);
	mutex_unlock(&gspca_dev->queue_lock);

	PDEBUG(D_STREAM, "close done");

	return 0;
}

static int vidioc_querycap(struct file *file, void  *priv,
			   struct v4l2_capability *cap)
{
	struct gspca_dev *gspca_dev = priv;
	int ret;

	
	if (mutex_lock_interruptible(&gspca_dev->usb_lock))
		return -ERESTARTSYS;
	if (!gspca_dev->present) {
		ret = -ENODEV;
		goto out;
	}
	strncpy(cap->driver, gspca_dev->sd_desc->name, sizeof cap->driver);
	if (gspca_dev->dev->product != NULL) {
		strncpy(cap->card, gspca_dev->dev->product,
			sizeof cap->card);
	} else {
		snprintf(cap->card, sizeof cap->card,
			"USB Camera (%04x:%04x)",
			le16_to_cpu(gspca_dev->dev->descriptor.idVendor),
			le16_to_cpu(gspca_dev->dev->descriptor.idProduct));
	}
	usb_make_path(gspca_dev->dev, cap->bus_info, sizeof(cap->bus_info));
	cap->version = DRIVER_VERSION_NUMBER;
	cap->capabilities = V4L2_CAP_VIDEO_CAPTURE
			  | V4L2_CAP_STREAMING
			  | V4L2_CAP_READWRITE;
	ret = 0;
out:
	mutex_unlock(&gspca_dev->usb_lock);
	return ret;
}

static const struct ctrl *get_ctrl(struct gspca_dev *gspca_dev,
				   int id)
{
	const struct ctrl *ctrls;
	int i;

	for (i = 0, ctrls = gspca_dev->sd_desc->ctrls;
	     i < gspca_dev->sd_desc->nctrls;
	     i++, ctrls++) {
		if (gspca_dev->ctrl_dis & (1 << i))
			continue;
		if (id == ctrls->qctrl.id)
			return ctrls;
	}
	return NULL;
}

static int vidioc_queryctrl(struct file *file, void *priv,
			   struct v4l2_queryctrl *q_ctrl)
{
	struct gspca_dev *gspca_dev = priv;
	const struct ctrl *ctrls;
	int i;
	u32 id;

	ctrls = NULL;
	id = q_ctrl->id;
	if (id & V4L2_CTRL_FLAG_NEXT_CTRL) {
		id &= V4L2_CTRL_ID_MASK;
		id++;
		for (i = 0; i < gspca_dev->sd_desc->nctrls; i++) {
			if (gspca_dev->ctrl_dis & (1 << i))
				continue;
			if (gspca_dev->sd_desc->ctrls[i].qctrl.id < id)
				continue;
			if (ctrls && gspca_dev->sd_desc->ctrls[i].qctrl.id
					    > ctrls->qctrl.id)
				continue;
			ctrls = &gspca_dev->sd_desc->ctrls[i];
		}
	} else {
		ctrls = get_ctrl(gspca_dev, id);
	}
	if (ctrls == NULL)
		return -EINVAL;
	memcpy(q_ctrl, ctrls, sizeof *q_ctrl);
	return 0;
}

static int vidioc_s_ctrl(struct file *file, void *priv,
			 struct v4l2_control *ctrl)
{
	struct gspca_dev *gspca_dev = priv;
	const struct ctrl *ctrls;
	int ret;

	ctrls = get_ctrl(gspca_dev, ctrl->id);
	if (ctrls == NULL)
		return -EINVAL;

	if (ctrl->value < ctrls->qctrl.minimum
	    || ctrl->value > ctrls->qctrl.maximum)
		return -ERANGE;
	PDEBUG(D_CONF, "set ctrl [%08x] = %d", ctrl->id, ctrl->value);
	if (mutex_lock_interruptible(&gspca_dev->usb_lock))
		return -ERESTARTSYS;
	if (gspca_dev->present)
		ret = ctrls->set(gspca_dev, ctrl->value);
	else
		ret = -ENODEV;
	mutex_unlock(&gspca_dev->usb_lock);
	return ret;
}

static int vidioc_g_ctrl(struct file *file, void *priv,
			 struct v4l2_control *ctrl)
{
	struct gspca_dev *gspca_dev = priv;
	const struct ctrl *ctrls;
	int ret;

	ctrls = get_ctrl(gspca_dev, ctrl->id);
	if (ctrls == NULL)
		return -EINVAL;

	if (mutex_lock_interruptible(&gspca_dev->usb_lock))
		return -ERESTARTSYS;
	if (gspca_dev->present)
		ret = ctrls->get(gspca_dev, &ctrl->value);
	else
		ret = -ENODEV;
	mutex_unlock(&gspca_dev->usb_lock);
	return ret;
}


static int vidioc_s_audio(struct file *file, void *priv,
			 struct v4l2_audio *audio)
{
	if (audio->index != 0)
		return -EINVAL;
	return 0;
}

static int vidioc_g_audio(struct file *file, void *priv,
			 struct v4l2_audio *audio)
{
	strcpy(audio->name, "Microphone");
	return 0;
}

static int vidioc_enumaudio(struct file *file, void *priv,
			 struct v4l2_audio *audio)
{
	if (audio->index != 0)
		return -EINVAL;

	strcpy(audio->name, "Microphone");
	audio->capability = 0;
	audio->mode = 0;
	return 0;
}

static int vidioc_querymenu(struct file *file, void *priv,
			    struct v4l2_querymenu *qmenu)
{
	struct gspca_dev *gspca_dev = priv;

	if (!gspca_dev->sd_desc->querymenu)
		return -EINVAL;
	return gspca_dev->sd_desc->querymenu(gspca_dev, qmenu);
}

static int vidioc_enum_input(struct file *file, void *priv,
				struct v4l2_input *input)
{
	struct gspca_dev *gspca_dev = priv;

	if (input->index != 0)
		return -EINVAL;
	input->type = V4L2_INPUT_TYPE_CAMERA;
	input->status = gspca_dev->cam.input_flags;
	strncpy(input->name, gspca_dev->sd_desc->name,
		sizeof input->name);
	return 0;
}

static int vidioc_g_input(struct file *file, void *priv, unsigned int *i)
{
	*i = 0;
	return 0;
}

static int vidioc_s_input(struct file *file, void *priv, unsigned int i)
{
	if (i > 0)
		return -EINVAL;
	return (0);
}

static int vidioc_reqbufs(struct file *file, void *priv,
			  struct v4l2_requestbuffers *rb)
{
	struct gspca_dev *gspca_dev = priv;
	int i, ret = 0;

	switch (rb->memory) {
	case GSPCA_MEMORY_READ:			
	case V4L2_MEMORY_MMAP:
	case V4L2_MEMORY_USERPTR:
		break;
	default:
		return -EINVAL;
	}
	if (mutex_lock_interruptible(&gspca_dev->queue_lock))
		return -ERESTARTSYS;

	if (gspca_dev->memory != GSPCA_MEMORY_NO
	    && gspca_dev->memory != rb->memory) {
		ret = -EBUSY;
		goto out;
	}

	
	if (gspca_dev->capt_file != NULL
	    && gspca_dev->capt_file != file) {
		ret = -EBUSY;
		goto out;
	}

	
	for (i = 0; i < gspca_dev->nframes; i++) {
		if (gspca_dev->frame[i].vma_use_count) {
			ret = -EBUSY;
			goto out;
		}
	}

	
	if (gspca_dev->streaming) {
		mutex_lock(&gspca_dev->usb_lock);
		gspca_stream_off(gspca_dev);
		mutex_unlock(&gspca_dev->usb_lock);
	}

	
	if (gspca_dev->nframes != 0) {
		frame_free(gspca_dev);
		gspca_dev->capt_file = NULL;
	}
	if (rb->count == 0)			
		goto out;
	gspca_dev->memory = rb->memory;
	ret = frame_alloc(gspca_dev, rb->count);
	if (ret == 0) {
		rb->count = gspca_dev->nframes;
		gspca_dev->capt_file = file;
	}
out:
	mutex_unlock(&gspca_dev->queue_lock);
	PDEBUG(D_STREAM, "reqbufs st:%d c:%d", ret, rb->count);
	return ret;
}

static int vidioc_querybuf(struct file *file, void *priv,
			   struct v4l2_buffer *v4l2_buf)
{
	struct gspca_dev *gspca_dev = priv;
	struct gspca_frame *frame;

	if (v4l2_buf->index < 0
	    || v4l2_buf->index >= gspca_dev->nframes)
		return -EINVAL;

	frame = &gspca_dev->frame[v4l2_buf->index];
	memcpy(v4l2_buf, &frame->v4l2_buf, sizeof *v4l2_buf);
	return 0;
}

static int vidioc_streamon(struct file *file, void *priv,
			   enum v4l2_buf_type buf_type)
{
	struct gspca_dev *gspca_dev = priv;
	int ret;

	if (buf_type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	if (mutex_lock_interruptible(&gspca_dev->queue_lock))
		return -ERESTARTSYS;

	if (gspca_dev->nframes == 0
	    || !(gspca_dev->frame[0].v4l2_buf.flags & V4L2_BUF_FLAG_QUEUED)) {
		ret = -EINVAL;
		goto out;
	}
	if (!gspca_dev->streaming) {
		ret = gspca_init_transfer(gspca_dev);
		if (ret < 0)
			goto out;
	}
#ifdef GSPCA_DEBUG
	if (gspca_debug & D_STREAM) {
		PDEBUG_MODE("stream on OK",
			gspca_dev->pixfmt,
			gspca_dev->width,
			gspca_dev->height);
	}
#endif
	ret = 0;
out:
	mutex_unlock(&gspca_dev->queue_lock);
	return ret;
}

static int vidioc_streamoff(struct file *file, void *priv,
				enum v4l2_buf_type buf_type)
{
	struct gspca_dev *gspca_dev = priv;
	int i, ret;

	if (buf_type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
	if (!gspca_dev->streaming)
		return 0;
	if (mutex_lock_interruptible(&gspca_dev->queue_lock))
		return -ERESTARTSYS;

	
	if (mutex_lock_interruptible(&gspca_dev->usb_lock)) {
		ret = -ERESTARTSYS;
		goto out;
	}
	gspca_stream_off(gspca_dev);
	mutex_unlock(&gspca_dev->usb_lock);

	
	for (i = 0; i < gspca_dev->nframes; i++)
		gspca_dev->frame[i].v4l2_buf.flags &= ~BUF_ALL_FLAGS;
	gspca_dev->fr_i = gspca_dev->fr_o = gspca_dev->fr_q = 0;
	gspca_dev->last_packet_type = DISCARD_PACKET;
	gspca_dev->sequence = 0;
	ret = 0;
out:
	mutex_unlock(&gspca_dev->queue_lock);
	return ret;
}

static int vidioc_g_jpegcomp(struct file *file, void *priv,
			struct v4l2_jpegcompression *jpegcomp)
{
	struct gspca_dev *gspca_dev = priv;
	int ret;

	if (!gspca_dev->sd_desc->get_jcomp)
		return -EINVAL;
	if (mutex_lock_interruptible(&gspca_dev->usb_lock))
		return -ERESTARTSYS;
	if (gspca_dev->present)
		ret = gspca_dev->sd_desc->get_jcomp(gspca_dev, jpegcomp);
	else
		ret = -ENODEV;
	mutex_unlock(&gspca_dev->usb_lock);
	return ret;
}

static int vidioc_s_jpegcomp(struct file *file, void *priv,
			struct v4l2_jpegcompression *jpegcomp)
{
	struct gspca_dev *gspca_dev = priv;
	int ret;

	if (!gspca_dev->sd_desc->set_jcomp)
		return -EINVAL;
	if (mutex_lock_interruptible(&gspca_dev->usb_lock))
		return -ERESTARTSYS;
	if (gspca_dev->present)
		ret = gspca_dev->sd_desc->set_jcomp(gspca_dev, jpegcomp);
	else
		ret = -ENODEV;
	mutex_unlock(&gspca_dev->usb_lock);
	return ret;
}

static int vidioc_g_parm(struct file *filp, void *priv,
			struct v4l2_streamparm *parm)
{
	struct gspca_dev *gspca_dev = priv;

	parm->parm.capture.readbuffers = gspca_dev->nbufread;

	if (gspca_dev->sd_desc->get_streamparm) {
		int ret;

		if (mutex_lock_interruptible(&gspca_dev->usb_lock))
			return -ERESTARTSYS;
		if (gspca_dev->present)
			ret = gspca_dev->sd_desc->get_streamparm(gspca_dev,
								 parm);
		else
			ret = -ENODEV;
		mutex_unlock(&gspca_dev->usb_lock);
		return ret;
	}

	return 0;
}

static int vidioc_s_parm(struct file *filp, void *priv,
			struct v4l2_streamparm *parm)
{
	struct gspca_dev *gspca_dev = priv;
	int n;

	n = parm->parm.capture.readbuffers;
	if (n == 0 || n > GSPCA_MAX_FRAMES)
		parm->parm.capture.readbuffers = gspca_dev->nbufread;
	else
		gspca_dev->nbufread = n;

	if (gspca_dev->sd_desc->set_streamparm) {
		int ret;

		if (mutex_lock_interruptible(&gspca_dev->usb_lock))
			return -ERESTARTSYS;
		if (gspca_dev->present)
			ret = gspca_dev->sd_desc->set_streamparm(gspca_dev,
								 parm);
		else
			ret = -ENODEV;
		mutex_unlock(&gspca_dev->usb_lock);
		return ret;
	}

	return 0;
}

#ifdef CONFIG_VIDEO_V4L1_COMPAT
static int vidiocgmbuf(struct file *file, void *priv,
			struct video_mbuf *mbuf)
{
	struct gspca_dev *gspca_dev = file->private_data;
	int i;

	PDEBUG(D_STREAM, "cgmbuf");
	if (gspca_dev->nframes == 0) {
		int ret;

		{
			struct v4l2_format fmt;

			fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			i = gspca_dev->cam.nmodes - 1;	
			fmt.fmt.pix.width = gspca_dev->cam.cam_mode[i].width;
			fmt.fmt.pix.height = gspca_dev->cam.cam_mode[i].height;
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_BGR24;
			ret = vidioc_s_fmt_vid_cap(file, priv, &fmt);
			if (ret != 0)
				return ret;
		}
		{
			struct v4l2_requestbuffers rb;

			memset(&rb, 0, sizeof rb);
			rb.count = 4;
			rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			rb.memory = V4L2_MEMORY_MMAP;
			ret = vidioc_reqbufs(file, priv, &rb);
			if (ret != 0)
				return ret;
		}
	}
	mbuf->frames = gspca_dev->nframes;
	mbuf->size = gspca_dev->frsz * gspca_dev->nframes;
	for (i = 0; i < mbuf->frames; i++)
		mbuf->offsets[i] = gspca_dev->frame[i].v4l2_buf.m.offset;
	return 0;
}
#endif

static int dev_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct gspca_dev *gspca_dev = file->private_data;
	struct gspca_frame *frame;
	struct page *page;
	unsigned long addr, start, size;
	int i, ret;

	start = vma->vm_start;
	size = vma->vm_end - vma->vm_start;
	PDEBUG(D_STREAM, "mmap start:%08x size:%d", (int) start, (int) size);

	if (mutex_lock_interruptible(&gspca_dev->queue_lock))
		return -ERESTARTSYS;
	if (!gspca_dev->present) {
		ret = -ENODEV;
		goto out;
	}
	if (gspca_dev->capt_file != file) {
		ret = -EINVAL;
		goto out;
	}

	frame = NULL;
	for (i = 0; i < gspca_dev->nframes; ++i) {
		if (gspca_dev->frame[i].v4l2_buf.memory != V4L2_MEMORY_MMAP) {
			PDEBUG(D_STREAM, "mmap bad memory type");
			break;
		}
		if ((gspca_dev->frame[i].v4l2_buf.m.offset >> PAGE_SHIFT)
						== vma->vm_pgoff) {
			frame = &gspca_dev->frame[i];
			break;
		}
	}
	if (frame == NULL) {
		PDEBUG(D_STREAM, "mmap no frame buffer found");
		ret = -EINVAL;
		goto out;
	}
#ifdef CONFIG_VIDEO_V4L1_COMPAT
	
	if (i != 0
	    || size != frame->v4l2_buf.length * gspca_dev->nframes)
#endif
	    if (size != frame->v4l2_buf.length) {
		PDEBUG(D_STREAM, "mmap bad size");
		ret = -EINVAL;
		goto out;
	}

	
	vma->vm_flags |= VM_IO;

	addr = (unsigned long) frame->data;
	while (size > 0) {
		page = vmalloc_to_page((void *) addr);
		ret = vm_insert_page(vma, start, page);
		if (ret < 0)
			goto out;
		start += PAGE_SIZE;
		addr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	vma->vm_ops = &gspca_vm_ops;
	vma->vm_private_data = frame;
	gspca_vm_open(vma);
	ret = 0;
out:
	mutex_unlock(&gspca_dev->queue_lock);
	return ret;
}


static int frame_wait(struct gspca_dev *gspca_dev,
			int nonblock_ing)
{
	struct gspca_frame *frame;
	int i, j, ret;

	
	i = gspca_dev->fr_o;
	j = gspca_dev->fr_queue[i];
	frame = &gspca_dev->frame[j];

	if (!(frame->v4l2_buf.flags & V4L2_BUF_FLAG_DONE)) {
		if (nonblock_ing)
			return -EAGAIN;

		
		ret = wait_event_interruptible_timeout(gspca_dev->wq,
			(frame->v4l2_buf.flags & V4L2_BUF_FLAG_DONE) ||
			!gspca_dev->streaming || !gspca_dev->present,
			msecs_to_jiffies(3000));
		if (ret < 0)
			return ret;
		if (ret == 0 || !gspca_dev->streaming || !gspca_dev->present)
			return -EIO;
	}

	gspca_dev->fr_o = (i + 1) % gspca_dev->nframes;
	PDEBUG(D_FRAM, "frame wait q:%d i:%d o:%d",
		gspca_dev->fr_q,
		gspca_dev->fr_i,
		gspca_dev->fr_o);

	if (gspca_dev->sd_desc->dq_callback) {
		mutex_lock(&gspca_dev->usb_lock);
		if (gspca_dev->present)
			gspca_dev->sd_desc->dq_callback(gspca_dev);
		mutex_unlock(&gspca_dev->usb_lock);
	}
	return j;
}


static int vidioc_dqbuf(struct file *file, void *priv,
			struct v4l2_buffer *v4l2_buf)
{
	struct gspca_dev *gspca_dev = priv;
	struct gspca_frame *frame;
	int i, ret;

	PDEBUG(D_FRAM, "dqbuf");
	if (v4l2_buf->memory != gspca_dev->memory)
		return -EINVAL;

	if (!gspca_dev->present)
		return -ENODEV;

	
	if (!(file->f_flags & O_NONBLOCK)
	    && !gspca_dev->streaming && gspca_dev->users == 1)
		return -EINVAL;

	
	if (gspca_dev->capt_file != file)
		return -EINVAL;

	
	if (mutex_lock_interruptible(&gspca_dev->read_lock))
		return -ERESTARTSYS;

	ret = frame_wait(gspca_dev, file->f_flags & O_NONBLOCK);
	if (ret < 0)
		goto out;
	i = ret;				
	frame = &gspca_dev->frame[i];
	if (gspca_dev->memory == V4L2_MEMORY_USERPTR) {
		if (copy_to_user((__u8 __user *) frame->v4l2_buf.m.userptr,
				 frame->data,
				 frame->v4l2_buf.bytesused)) {
			PDEBUG(D_ERR|D_STREAM,
				"dqbuf cp to user failed");
			ret = -EFAULT;
			goto out;
		}
	}
	frame->v4l2_buf.flags &= ~V4L2_BUF_FLAG_DONE;
	memcpy(v4l2_buf, &frame->v4l2_buf, sizeof *v4l2_buf);
	PDEBUG(D_FRAM, "dqbuf %d", i);
	ret = 0;
out:
	mutex_unlock(&gspca_dev->read_lock);
	return ret;
}


static int vidioc_qbuf(struct file *file, void *priv,
			struct v4l2_buffer *v4l2_buf)
{
	struct gspca_dev *gspca_dev = priv;
	struct gspca_frame *frame;
	int i, index, ret;

	PDEBUG(D_FRAM, "qbuf %d", v4l2_buf->index);

	if (mutex_lock_interruptible(&gspca_dev->queue_lock))
		return -ERESTARTSYS;

	index = v4l2_buf->index;
	if ((unsigned) index >= gspca_dev->nframes) {
		PDEBUG(D_FRAM,
			"qbuf idx %d >= %d", index, gspca_dev->nframes);
		ret = -EINVAL;
		goto out;
	}
	if (v4l2_buf->memory != gspca_dev->memory) {
		PDEBUG(D_FRAM, "qbuf bad memory type");
		ret = -EINVAL;
		goto out;
	}

	frame = &gspca_dev->frame[index];
	if (frame->v4l2_buf.flags & BUF_ALL_FLAGS) {
		PDEBUG(D_FRAM, "qbuf bad state");
		ret = -EINVAL;
		goto out;
	}

	frame->v4l2_buf.flags |= V4L2_BUF_FLAG_QUEUED;

	if (frame->v4l2_buf.memory == V4L2_MEMORY_USERPTR) {
		frame->v4l2_buf.m.userptr = v4l2_buf->m.userptr;
		frame->v4l2_buf.length = v4l2_buf->length;
	}

	
	i = gspca_dev->fr_q;
	gspca_dev->fr_queue[i] = index;
	gspca_dev->fr_q = (i + 1) % gspca_dev->nframes;
	PDEBUG(D_FRAM, "qbuf q:%d i:%d o:%d",
		gspca_dev->fr_q,
		gspca_dev->fr_i,
		gspca_dev->fr_o);

	v4l2_buf->flags |= V4L2_BUF_FLAG_QUEUED;
	v4l2_buf->flags &= ~V4L2_BUF_FLAG_DONE;
	ret = 0;
out:
	mutex_unlock(&gspca_dev->queue_lock);
	return ret;
}


static int read_alloc(struct gspca_dev *gspca_dev,
			struct file *file)
{
	struct v4l2_buffer v4l2_buf;
	int i, ret;

	PDEBUG(D_STREAM, "read alloc");
	if (gspca_dev->nframes == 0) {
		struct v4l2_requestbuffers rb;

		memset(&rb, 0, sizeof rb);
		rb.count = gspca_dev->nbufread;
		rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		rb.memory = GSPCA_MEMORY_READ;
		ret = vidioc_reqbufs(file, gspca_dev, &rb);
		if (ret != 0) {
			PDEBUG(D_STREAM, "read reqbuf err %d", ret);
			return ret;
		}
		memset(&v4l2_buf, 0, sizeof v4l2_buf);
		v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l2_buf.memory = GSPCA_MEMORY_READ;
		for (i = 0; i < gspca_dev->nbufread; i++) {
			v4l2_buf.index = i;
			ret = vidioc_qbuf(file, gspca_dev, &v4l2_buf);
			if (ret != 0) {
				PDEBUG(D_STREAM, "read qbuf err: %d", ret);
				return ret;
			}
		}
		gspca_dev->memory = GSPCA_MEMORY_READ;
	}

	
	ret = vidioc_streamon(file, gspca_dev, V4L2_BUF_TYPE_VIDEO_CAPTURE);
	if (ret != 0)
		PDEBUG(D_STREAM, "read streamon err %d", ret);
	return ret;
}

static unsigned int dev_poll(struct file *file, poll_table *wait)
{
	struct gspca_dev *gspca_dev = file->private_data;
	int i, ret;

	PDEBUG(D_FRAM, "poll");

	poll_wait(file, &gspca_dev->wq, wait);

	
	if (gspca_dev->nframes == 0) {
		if (gspca_dev->memory != GSPCA_MEMORY_NO)
			return POLLERR;		
		ret = read_alloc(gspca_dev, file);
		if (ret != 0)
			return POLLERR;
	}

	if (mutex_lock_interruptible(&gspca_dev->queue_lock) != 0)
		return POLLERR;

	
	i = gspca_dev->fr_o;
	i = gspca_dev->fr_queue[i];
	if (gspca_dev->frame[i].v4l2_buf.flags & V4L2_BUF_FLAG_DONE)
		ret = POLLIN | POLLRDNORM;	
	else
		ret = 0;
	mutex_unlock(&gspca_dev->queue_lock);
	if (!gspca_dev->present)
		return POLLHUP;
	return ret;
}

static ssize_t dev_read(struct file *file, char __user *data,
		    size_t count, loff_t *ppos)
{
	struct gspca_dev *gspca_dev = file->private_data;
	struct gspca_frame *frame;
	struct v4l2_buffer v4l2_buf;
	struct timeval timestamp;
	int n, ret, ret2;

	PDEBUG(D_FRAM, "read (%zd)", count);
	if (!gspca_dev->present)
		return -ENODEV;
	switch (gspca_dev->memory) {
	case GSPCA_MEMORY_NO:			
		ret = read_alloc(gspca_dev, file);
		if (ret != 0)
			return ret;
		break;
	case GSPCA_MEMORY_READ:
		if (gspca_dev->capt_file == file)
			break;
		
	default:
		return -EINVAL;
	}

	
	jiffies_to_timeval(get_jiffies_64(), &timestamp);
	timestamp.tv_sec--;
	n = 2;
	for (;;) {
		memset(&v4l2_buf, 0, sizeof v4l2_buf);
		v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		v4l2_buf.memory = GSPCA_MEMORY_READ;
		ret = vidioc_dqbuf(file, gspca_dev, &v4l2_buf);
		if (ret != 0) {
			PDEBUG(D_STREAM, "read dqbuf err %d", ret);
			return ret;
		}

		
		frame = &gspca_dev->frame[v4l2_buf.index];
		if (--n < 0)
			break;			
		if (frame->v4l2_buf.timestamp.tv_sec >= timestamp.tv_sec)
			break;
		ret = vidioc_qbuf(file, gspca_dev, &v4l2_buf);
		if (ret != 0) {
			PDEBUG(D_STREAM, "read qbuf err %d", ret);
			return ret;
		}
	}

	
	if (count > frame->v4l2_buf.bytesused)
		count = frame->v4l2_buf.bytesused;
	ret = copy_to_user(data, frame->data, count);
	if (ret != 0) {
		PDEBUG(D_ERR|D_STREAM,
			"read cp to user lack %d / %zd", ret, count);
		ret = -EFAULT;
		goto out;
	}
	ret = count;
out:
	
	ret2 = vidioc_qbuf(file, gspca_dev, &v4l2_buf);
	if (ret2 != 0)
		return ret2;
	return ret;
}

static struct v4l2_file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_close,
	.read = dev_read,
	.mmap = dev_mmap,
	.unlocked_ioctl = video_ioctl2,
	.poll	= dev_poll,
};

static const struct v4l2_ioctl_ops dev_ioctl_ops = {
	.vidioc_querycap	= vidioc_querycap,
	.vidioc_dqbuf		= vidioc_dqbuf,
	.vidioc_qbuf		= vidioc_qbuf,
	.vidioc_enum_fmt_vid_cap = vidioc_enum_fmt_vid_cap,
	.vidioc_try_fmt_vid_cap	= vidioc_try_fmt_vid_cap,
	.vidioc_g_fmt_vid_cap	= vidioc_g_fmt_vid_cap,
	.vidioc_s_fmt_vid_cap	= vidioc_s_fmt_vid_cap,
	.vidioc_streamon	= vidioc_streamon,
	.vidioc_queryctrl	= vidioc_queryctrl,
	.vidioc_g_ctrl		= vidioc_g_ctrl,
	.vidioc_s_ctrl		= vidioc_s_ctrl,
	.vidioc_g_audio		= vidioc_g_audio,
	.vidioc_s_audio		= vidioc_s_audio,
	.vidioc_enumaudio	= vidioc_enumaudio,
	.vidioc_querymenu	= vidioc_querymenu,
	.vidioc_enum_input	= vidioc_enum_input,
	.vidioc_g_input		= vidioc_g_input,
	.vidioc_s_input		= vidioc_s_input,
	.vidioc_reqbufs		= vidioc_reqbufs,
	.vidioc_querybuf	= vidioc_querybuf,
	.vidioc_streamoff	= vidioc_streamoff,
	.vidioc_g_jpegcomp	= vidioc_g_jpegcomp,
	.vidioc_s_jpegcomp	= vidioc_s_jpegcomp,
	.vidioc_g_parm		= vidioc_g_parm,
	.vidioc_s_parm		= vidioc_s_parm,
	.vidioc_enum_framesizes = vidioc_enum_framesizes,
#ifdef CONFIG_VIDEO_ADV_DEBUG
	.vidioc_g_register	= vidioc_g_register,
	.vidioc_s_register	= vidioc_s_register,
#endif
	.vidioc_g_chip_ident	= vidioc_g_chip_ident,
#ifdef CONFIG_VIDEO_V4L1_COMPAT
	.vidiocgmbuf          = vidiocgmbuf,
#endif
};

static struct video_device gspca_template = {
	.name = "gspca main driver",
	.fops = &dev_fops,
	.ioctl_ops = &dev_ioctl_ops,
	.release = gspca_release,
	.minor = -1,
};


int gspca_dev_probe(struct usb_interface *intf,
		const struct usb_device_id *id,
		const struct sd_desc *sd_desc,
		int dev_size,
		struct module *module)
{
	struct usb_interface_descriptor *interface;
	struct gspca_dev *gspca_dev;
	struct usb_device *dev = interface_to_usbdev(intf);
	int ret;

	PDEBUG(D_PROBE, "probing %04x:%04x", id->idVendor, id->idProduct);

	
	if (dev->descriptor.bNumConfigurations != 1)
		return -ENODEV;
	interface = &intf->cur_altsetting->desc;
	if (interface->bInterfaceNumber > 0)
		return -ENODEV;

	
	if (dev_size < sizeof *gspca_dev)
		dev_size = sizeof *gspca_dev;
	gspca_dev = kzalloc(dev_size, GFP_KERNEL);
	if (!gspca_dev) {
		err("couldn't kzalloc gspca struct");
		return -ENOMEM;
	}
	gspca_dev->usb_buf = kmalloc(USB_BUF_SZ, GFP_KERNEL);
	if (!gspca_dev->usb_buf) {
		err("out of memory");
		ret = -ENOMEM;
		goto out;
	}
	gspca_dev->dev = dev;
	gspca_dev->iface = interface->bInterfaceNumber;
	gspca_dev->nbalt = intf->num_altsetting;
	gspca_dev->sd_desc = sd_desc;
	gspca_dev->nbufread = 2;
	gspca_dev->empty_packet = -1;	

	
	ret = sd_desc->config(gspca_dev, id);
	if (ret < 0)
		goto out;
	ret = sd_desc->init(gspca_dev);
	if (ret < 0)
		goto out;
	ret = gspca_set_alt0(gspca_dev);
	if (ret < 0)
		goto out;
	gspca_set_default_mode(gspca_dev);

	mutex_init(&gspca_dev->usb_lock);
	mutex_init(&gspca_dev->read_lock);
	mutex_init(&gspca_dev->queue_lock);
	init_waitqueue_head(&gspca_dev->wq);

	
	memcpy(&gspca_dev->vdev, &gspca_template, sizeof gspca_template);
	gspca_dev->vdev.parent = &intf->dev;
	gspca_dev->module = module;
	gspca_dev->present = 1;
	ret = video_register_device(&gspca_dev->vdev,
				  VFL_TYPE_GRABBER,
				  -1);
	if (ret < 0) {
		err("video_register_device err %d", ret);
		goto out;
	}

	usb_set_intfdata(intf, gspca_dev);
	PDEBUG(D_PROBE, "probe ok");
	return 0;
out:
	kfree(gspca_dev->usb_buf);
	kfree(gspca_dev);
	return ret;
}
EXPORT_SYMBOL(gspca_dev_probe);


void gspca_disconnect(struct usb_interface *intf)
{
	struct gspca_dev *gspca_dev = usb_get_intfdata(intf);

	mutex_lock(&gspca_dev->usb_lock);
	gspca_dev->present = 0;

	if (gspca_dev->streaming) {
		destroy_urbs(gspca_dev);
		wake_up_interruptible(&gspca_dev->wq);
	}

	
	gspca_dev->dev = NULL;
	mutex_unlock(&gspca_dev->usb_lock);

	usb_set_intfdata(intf, NULL);

	
	
	video_unregister_device(&gspca_dev->vdev);

	PDEBUG(D_PROBE, "disconnect complete");
}
EXPORT_SYMBOL(gspca_disconnect);

#ifdef CONFIG_PM
int gspca_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct gspca_dev *gspca_dev = usb_get_intfdata(intf);

	if (!gspca_dev->streaming)
		return 0;
	gspca_dev->frozen = 1;		
	if (gspca_dev->sd_desc->stopN)
		gspca_dev->sd_desc->stopN(gspca_dev);
	destroy_urbs(gspca_dev);
	gspca_set_alt0(gspca_dev);
	if (gspca_dev->sd_desc->stop0)
		gspca_dev->sd_desc->stop0(gspca_dev);
	return 0;
}
EXPORT_SYMBOL(gspca_suspend);

int gspca_resume(struct usb_interface *intf)
{
	struct gspca_dev *gspca_dev = usb_get_intfdata(intf);

	gspca_dev->frozen = 0;
	gspca_dev->sd_desc->init(gspca_dev);
	if (gspca_dev->streaming)
		return gspca_init_transfer(gspca_dev);
	return 0;
}
EXPORT_SYMBOL(gspca_resume);
#endif



int gspca_auto_gain_n_exposure(struct gspca_dev *gspca_dev, int avg_lum,
	int desired_avg_lum, int deadzone, int gain_knee, int exposure_knee)
{
	int i, steps, gain, orig_gain, exposure, orig_exposure, autogain;
	const struct ctrl *gain_ctrl = NULL;
	const struct ctrl *exposure_ctrl = NULL;
	const struct ctrl *autogain_ctrl = NULL;
	int retval = 0;

	for (i = 0; i < gspca_dev->sd_desc->nctrls; i++) {
		if (gspca_dev->sd_desc->ctrls[i].qctrl.id == V4L2_CID_GAIN)
			gain_ctrl = &gspca_dev->sd_desc->ctrls[i];
		if (gspca_dev->sd_desc->ctrls[i].qctrl.id == V4L2_CID_EXPOSURE)
			exposure_ctrl = &gspca_dev->sd_desc->ctrls[i];
		if (gspca_dev->sd_desc->ctrls[i].qctrl.id == V4L2_CID_AUTOGAIN)
			autogain_ctrl = &gspca_dev->sd_desc->ctrls[i];
	}
	if (!gain_ctrl || !exposure_ctrl || !autogain_ctrl) {
		PDEBUG(D_ERR, "Error: gspca_auto_gain_n_exposure called "
			"on cam without (auto)gain/exposure");
		return 0;
	}

	if (gain_ctrl->get(gspca_dev, &gain) ||
			exposure_ctrl->get(gspca_dev, &exposure) ||
			autogain_ctrl->get(gspca_dev, &autogain) || !autogain)
		return 0;

	orig_gain = gain;
	orig_exposure = exposure;

	
	steps = abs(desired_avg_lum - avg_lum) / deadzone;

	PDEBUG(D_FRAM, "autogain: lum: %d, desired: %d, steps: %d",
		avg_lum, desired_avg_lum, steps);

	for (i = 0; i < steps; i++) {
		if (avg_lum > desired_avg_lum) {
			if (gain > gain_knee)
				gain--;
			else if (exposure > exposure_knee)
				exposure--;
			else if (gain > gain_ctrl->qctrl.default_value)
				gain--;
			else if (exposure > exposure_ctrl->qctrl.minimum)
				exposure--;
			else if (gain > gain_ctrl->qctrl.minimum)
				gain--;
			else
				break;
		} else {
			if (gain < gain_ctrl->qctrl.default_value)
				gain++;
			else if (exposure < exposure_knee)
				exposure++;
			else if (gain < gain_knee)
				gain++;
			else if (exposure < exposure_ctrl->qctrl.maximum)
				exposure++;
			else if (gain < gain_ctrl->qctrl.maximum)
				gain++;
			else
				break;
		}
	}

	if (gain != orig_gain) {
		gain_ctrl->set(gspca_dev, gain);
		retval = 1;
	}
	if (exposure != orig_exposure) {
		exposure_ctrl->set(gspca_dev, exposure);
		retval = 1;
	}

	return retval;
}
EXPORT_SYMBOL(gspca_auto_gain_n_exposure);


static int __init gspca_init(void)
{
	info("main v%d.%d.%d registered",
		(DRIVER_VERSION_NUMBER >> 16) & 0xff,
		(DRIVER_VERSION_NUMBER >> 8) & 0xff,
		DRIVER_VERSION_NUMBER & 0xff);
	return 0;
}
static void __exit gspca_exit(void)
{
	info("main deregistered");
}

module_init(gspca_init);
module_exit(gspca_exit);

#ifdef GSPCA_DEBUG
module_param_named(debug, gspca_debug, int, 0644);
MODULE_PARM_DESC(debug,
		"Debug (bit) 0x01:error 0x02:probe 0x04:config"
		" 0x08:stream 0x10:frame 0x20:packet 0x40:USBin 0x80:USBout"
		" 0x0100: v4l2");
#endif
