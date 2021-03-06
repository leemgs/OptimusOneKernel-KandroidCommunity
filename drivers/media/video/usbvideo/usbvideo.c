

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/spinlock.h>

#include <asm/io.h>

#include "usbvideo.h"

#if defined(MAP_NR)
#define	virt_to_page(v)	MAP_NR(v)	
#endif

static int video_nr = -1;
module_param(video_nr, int, 0);


static void usbvideo_Disconnect(struct usb_interface *intf);
static void usbvideo_CameraRelease(struct uvd *uvd);

static long usbvideo_v4l_ioctl(struct file *file,
			      unsigned int cmd, unsigned long arg);
static int usbvideo_v4l_mmap(struct file *file, struct vm_area_struct *vma);
static int usbvideo_v4l_open(struct file *file);
static ssize_t usbvideo_v4l_read(struct file *file, char __user *buf,
			     size_t count, loff_t *ppos);
static int usbvideo_v4l_close(struct file *file);

static int usbvideo_StartDataPump(struct uvd *uvd);
static void usbvideo_StopDataPump(struct uvd *uvd);
static int usbvideo_GetFrame(struct uvd *uvd, int frameNum);
static int usbvideo_NewFrame(struct uvd *uvd, int framenum);
static void usbvideo_SoftwareContrastAdjustment(struct uvd *uvd,
						struct usbvideo_frame *frame);




static void *usbvideo_rvmalloc(unsigned long size)
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

static void usbvideo_rvfree(void *mem, unsigned long size)
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

static void RingQueue_Initialize(struct RingQueue *rq)
{
	assert(rq != NULL);
	init_waitqueue_head(&rq->wqh);
}

static void RingQueue_Allocate(struct RingQueue *rq, int rqLen)
{
	

	int i = 1;
	assert(rq != NULL);
	assert(rqLen > 0);

	while(rqLen >> i)
		i++;
	if(rqLen != 1 << (i-1))
		rqLen = 1 << i;

	rq->length = rqLen;
	rq->ri = rq->wi = 0;
	rq->queue = usbvideo_rvmalloc(rq->length);
	assert(rq->queue != NULL);
}

static int RingQueue_IsAllocated(const struct RingQueue *rq)
{
	if (rq == NULL)
		return 0;
	return (rq->queue != NULL) && (rq->length > 0);
}

static void RingQueue_Free(struct RingQueue *rq)
{
	assert(rq != NULL);
	if (RingQueue_IsAllocated(rq)) {
		usbvideo_rvfree(rq->queue, rq->length);
		rq->queue = NULL;
		rq->length = 0;
	}
}

int RingQueue_Dequeue(struct RingQueue *rq, unsigned char *dst, int len)
{
	int rql, toread;

	assert(rq != NULL);
	assert(dst != NULL);

	rql = RingQueue_GetLength(rq);
	if(!rql)
		return 0;

	
	if(len > rql)
		len = rql;

	toread = len;
	if(rq->ri > rq->wi) {
		
		int read = (toread < (rq->length - rq->ri)) ? toread : rq->length - rq->ri;
		memcpy(dst, rq->queue + rq->ri, read);
		toread -= read;
		dst += read;
		rq->ri = (rq->ri + read) & (rq->length-1);
	}
	if(toread) {
		
		memcpy(dst, rq->queue + rq->ri, toread);
		rq->ri = (rq->ri + toread) & (rq->length-1);
	}
	return len;
}

EXPORT_SYMBOL(RingQueue_Dequeue);

int RingQueue_Enqueue(struct RingQueue *rq, const unsigned char *cdata, int n)
{
	int enqueued = 0;

	assert(rq != NULL);
	assert(cdata != NULL);
	assert(rq->length > 0);
	while (n > 0) {
		int m, q_avail;

		
		q_avail = rq->length - rq->wi;
		if (q_avail <= 0) {
			rq->wi = 0;
			q_avail = rq->length;
		}
		m = n;
		assert(q_avail > 0);
		if (m > q_avail)
			m = q_avail;

		memcpy(rq->queue + rq->wi, cdata, m);
		RING_QUEUE_ADVANCE_INDEX(rq, wi, m);
		cdata += m;
		enqueued += m;
		n -= m;
	}
	return enqueued;
}

EXPORT_SYMBOL(RingQueue_Enqueue);

static void RingQueue_InterruptibleSleepOn(struct RingQueue *rq)
{
	assert(rq != NULL);
	interruptible_sleep_on(&rq->wqh);
}

void RingQueue_WakeUpInterruptible(struct RingQueue *rq)
{
	assert(rq != NULL);
	if (waitqueue_active(&rq->wqh))
		wake_up_interruptible(&rq->wqh);
}

EXPORT_SYMBOL(RingQueue_WakeUpInterruptible);

void RingQueue_Flush(struct RingQueue *rq)
{
	assert(rq != NULL);
	rq->ri = 0;
	rq->wi = 0;
}

EXPORT_SYMBOL(RingQueue_Flush);



static void usbvideo_VideosizeToString(char *buf, int bufLen, videosize_t vs)
{
	char tmp[40];
	int n;

	n = 1 + sprintf(tmp, "%ldx%ld", VIDEOSIZE_X(vs), VIDEOSIZE_Y(vs));
	assert(n < sizeof(tmp));
	if ((buf == NULL) || (bufLen < n))
		err("usbvideo_VideosizeToString: buffer is too small.");
	else
		memmove(buf, tmp, n);
}


static void usbvideo_OverlayChar(struct uvd *uvd, struct usbvideo_frame *frame,
				 int x, int y, int ch)
{
	static const unsigned short digits[16] = {
		0xF6DE, 
		0x2492, 
		0xE7CE, 
		0xE79E, 
		0xB792, 
		0xF39E, 
		0xF3DE, 
		0xF492, 
		0xF7DE, 
		0xF79E, 
		0x77DA, 
		0xD75C, 
		0xF24E, 
		0xD6DC, 
		0xF34E, 
		0xF348  
	};
	unsigned short digit;
	int ix, iy;

	if ((uvd == NULL) || (frame == NULL))
		return;

	if (ch >= '0' && ch <= '9')
		ch -= '0';
	else if (ch >= 'A' && ch <= 'F')
		ch = 10 + (ch - 'A');
	else if (ch >= 'a' && ch <= 'f')
		ch = 10 + (ch - 'a');
	else
		return;
	digit = digits[ch];

	for (iy=0; iy < 5; iy++) {
		for (ix=0; ix < 3; ix++) {
			if (digit & 0x8000) {
				if (uvd->paletteBits & (1L << VIDEO_PALETTE_RGB24)) {
				RGB24_PUTPIXEL(frame, x+ix, y+iy, 0xFF, 0xFF, 0xFF);
				}
			}
			digit = digit << 1;
		}
	}
}


static void usbvideo_OverlayString(struct uvd *uvd, struct usbvideo_frame *frame,
				   int x, int y, const char *str)
{
	while (*str) {
		usbvideo_OverlayChar(uvd, frame, x, y, *str);
		str++;
		x += 4; 
	}
}


static void usbvideo_OverlayStats(struct uvd *uvd, struct usbvideo_frame *frame)
{
	const int y_diff = 8;
	char tmp[16];
	int x = 10, y=10;
	long i, j, barLength;
	const int qi_x1 = 60, qi_y1 = 10;
	const int qi_x2 = VIDEOSIZE_X(frame->request) - 10, qi_h = 10;

	
	if (VALID_CALLBACK(uvd, overlayHook)) {
		if (GET_CALLBACK(uvd, overlayHook)(uvd, frame) < 0)
			return;
	}

	
	barLength = qi_x2 - qi_x1 - 2;
	if ((barLength > 10) && (uvd->paletteBits & (1L << VIDEO_PALETTE_RGB24))) {
	long u_lo, u_hi, q_used;
		long m_ri, m_wi, m_lo, m_hi;

		

		q_used = RingQueue_GetLength(&uvd->dp);
		if ((uvd->dp.ri + q_used) >= uvd->dp.length) {
			u_hi = uvd->dp.length;
			u_lo = (q_used + uvd->dp.ri) & (uvd->dp.length-1);
		} else {
			u_hi = (q_used + uvd->dp.ri);
			u_lo = -1;
		}

		
		m_ri = qi_x1 + ((barLength * uvd->dp.ri) / uvd->dp.length);
		m_wi = qi_x1 + ((barLength * uvd->dp.wi) / uvd->dp.length);
		m_lo = (u_lo > 0) ? (qi_x1 + ((barLength * u_lo) / uvd->dp.length)) : -1;
		m_hi = qi_x1 + ((barLength * u_hi) / uvd->dp.length);

		for (j=qi_y1; j < (qi_y1 + qi_h); j++) {
			for (i=qi_x1; i < qi_x2; i++) {
				
				if ((j == qi_y1) || (j == (qi_y1 + qi_h - 1)) ||
				    (i == qi_x1) || (i == (qi_x2 - 1))) {
					RGB24_PUTPIXEL(frame, i, j, 0xFF, 0xFF, 0xFF);
					continue;
				}
				
				if ((i >= m_ri) && (i <= (m_ri + 3))) {
					RGB24_PUTPIXEL(frame, i, j, 0x00, 0xFF, 0x00);
				} else if ((i >= m_wi) && (i <= (m_wi + 3))) {
					RGB24_PUTPIXEL(frame, i, j, 0xFF, 0x00, 0x00);
				} else if ((i < m_lo) || ((i > m_ri) && (i < m_hi)))
					RGB24_PUTPIXEL(frame, i, j, 0x00, 0x00, 0xFF);
			}
		}
	}

	sprintf(tmp, "%8lx", uvd->stats.frame_num);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8lx", uvd->stats.urb_count);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8lx", uvd->stats.urb_length);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8lx", uvd->stats.data_count);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8lx", uvd->stats.header_count);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8lx", uvd->stats.iso_skip_count);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8lx", uvd->stats.iso_err_count);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8x", uvd->vpic.colour);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8x", uvd->vpic.hue);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8x", uvd->vpic.brightness >> 8);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8x", uvd->vpic.contrast >> 12);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;

	sprintf(tmp, "%8d", uvd->vpic.whiteness >> 8);
	usbvideo_OverlayString(uvd, frame, x, y, tmp);
	y += y_diff;
}


static void usbvideo_ReportStatistics(const struct uvd *uvd)
{
	if ((uvd != NULL) && (uvd->stats.urb_count > 0)) {
		unsigned long allPackets, badPackets, goodPackets, percent;
		allPackets = uvd->stats.urb_count * CAMERA_URB_FRAMES;
		badPackets = uvd->stats.iso_skip_count + uvd->stats.iso_err_count;
		goodPackets = allPackets - badPackets;
		
		assert(allPackets != 0);
		if (goodPackets < (((unsigned long)-1)/100))
			percent = (100 * goodPackets) / allPackets;
		else
			percent = goodPackets / (allPackets / 100);
		dev_info(&uvd->dev->dev,
			 "Packet Statistics: Total=%lu. Empty=%lu. Usage=%lu%%\n",
			 allPackets, badPackets, percent);
		if (uvd->iso_packet_len > 0) {
			unsigned long allBytes, xferBytes;
			char multiplier = ' ';
			allBytes = allPackets * uvd->iso_packet_len;
			xferBytes = uvd->stats.data_count;
			assert(allBytes != 0);
			if (xferBytes < (((unsigned long)-1)/100))
				percent = (100 * xferBytes) / allBytes;
			else
				percent = xferBytes / (allBytes / 100);
			
			if (xferBytes > 10*1024) {
				xferBytes /= 1024;
				multiplier = 'K';
				if (xferBytes > 10*1024) {
					xferBytes /= 1024;
					multiplier = 'M';
					if (xferBytes > 10*1024) {
						xferBytes /= 1024;
						multiplier = 'G';
						if (xferBytes > 10*1024) {
							xferBytes /= 1024;
							multiplier = 'T';
						}
					}
				}
			}
			dev_info(&uvd->dev->dev,
				 "Transfer Statistics: Transferred=%lu%cB Usage=%lu%%\n",
				 xferBytes, multiplier, percent);
		}
	}
}


void usbvideo_TestPattern(struct uvd *uvd, int fullframe, int pmode)
{
	struct usbvideo_frame *frame;
	int num_cell = 0;
	int scan_length = 0;
	static int num_pass;

	if (uvd == NULL) {
		err("%s: uvd == NULL", __func__);
		return;
	}
	if ((uvd->curframe < 0) || (uvd->curframe >= USBVIDEO_NUMFRAMES)) {
		err("%s: uvd->curframe=%d.", __func__, uvd->curframe);
		return;
	}

	
	frame = &uvd->frame[uvd->curframe];

	
	if (fullframe) {
		frame->curline = 0;
		frame->seqRead_Length = 0;
	}
#if 0
	{	
		char tmp[20];
		usbvideo_VideosizeToString(tmp, sizeof(tmp), frame->request);
		dev_info(&uvd->dev->dev, "testpattern: frame=%s\n", tmp);
	}
#endif
	
	for (; frame->curline < VIDEOSIZE_Y(frame->request); frame->curline++) {
		int i;
		unsigned char *f = frame->data +
			(VIDEOSIZE_X(frame->request) * V4L_BYTES_PER_PIXEL * frame->curline);
		for (i=0; i < VIDEOSIZE_X(frame->request); i++) {
			unsigned char cb=0x80;
			unsigned char cg = 0;
			unsigned char cr = 0;

			if (pmode == 1) {
				if (frame->curline % 32 == 0)
					cb = 0, cg = cr = 0xFF;
				else if (i % 32 == 0) {
					if (frame->curline % 32 == 1)
						num_cell++;
					cb = 0, cg = cr = 0xFF;
				} else {
					cb = ((num_cell*7) + num_pass) & 0xFF;
					cg = ((num_cell*5) + num_pass*2) & 0xFF;
					cr = ((num_cell*3) + num_pass*3) & 0xFF;
				}
			} else {
				
			}

			*f++ = cb;
			*f++ = cg;
			*f++ = cr;
			scan_length += 3;
		}
	}

	frame->frameState = FrameState_Done;
	frame->seqRead_Length += scan_length;
	++num_pass;

	
	usbvideo_OverlayStats(uvd, frame);
}

EXPORT_SYMBOL(usbvideo_TestPattern);


#ifdef DEBUG

void usbvideo_HexDump(const unsigned char *data, int len)
{
	const int bytes_per_line = 32;
	char tmp[128]; 
	int i, k;

	for (i=k=0; len > 0; i++, len--) {
		if (i > 0 && ((i % bytes_per_line) == 0)) {
			printk("%s\n", tmp);
			k=0;
		}
		if ((i % bytes_per_line) == 0)
			k += sprintf(&tmp[k], "%04x: ", i);
		k += sprintf(&tmp[k], "%02x ", data[i]);
	}
	if (k > 0)
		printk("%s\n", tmp);
}

EXPORT_SYMBOL(usbvideo_HexDump);

#endif




static int usbvideo_ClientIncModCount(struct uvd *uvd)
{
	if (uvd == NULL) {
		err("%s: uvd == NULL", __func__);
		return -EINVAL;
	}
	if (uvd->handle == NULL) {
		err("%s: uvd->handle == NULL", __func__);
		return -EINVAL;
	}
	if (!try_module_get(uvd->handle->md_module)) {
		err("%s: try_module_get() == 0", __func__);
		return -ENODEV;
	}
	return 0;
}

static void usbvideo_ClientDecModCount(struct uvd *uvd)
{
	if (uvd == NULL) {
		err("%s: uvd == NULL", __func__);
		return;
	}
	if (uvd->handle == NULL) {
		err("%s: uvd->handle == NULL", __func__);
		return;
	}
	if (uvd->handle->md_module == NULL) {
		err("%s: uvd->handle->md_module == NULL", __func__);
		return;
	}
	module_put(uvd->handle->md_module);
}

int usbvideo_register(
	struct usbvideo **pCams,
	const int num_cams,
	const int num_extra,
	const char *driverName,
	const struct usbvideo_cb *cbTbl,
	struct module *md,
	const struct usb_device_id *id_table)
{
	struct usbvideo *cams;
	int i, base_size, result;

	
	if ((num_cams <= 0) || (pCams == NULL) || (cbTbl == NULL)) {
		err("%s: Illegal call", __func__);
		return -EINVAL;
	}

	
	if (cbTbl->probe == NULL) {
		err("%s: probe() is required!", __func__);
		return -EINVAL;
	}

	base_size = num_cams * sizeof(struct uvd) + sizeof(struct usbvideo);
	cams = kzalloc(base_size, GFP_KERNEL);
	if (cams == NULL) {
		err("Failed to allocate %d. bytes for usbvideo struct", base_size);
		return -ENOMEM;
	}
	dbg("%s: Allocated $%p (%d. bytes) for %d. cameras",
	    __func__, cams, base_size, num_cams);

	
	memmove(&cams->cb, cbTbl, sizeof(cams->cb));
	if (cams->cb.getFrame == NULL)
		cams->cb.getFrame = usbvideo_GetFrame;
	if (cams->cb.disconnect == NULL)
		cams->cb.disconnect = usbvideo_Disconnect;
	if (cams->cb.startDataPump == NULL)
		cams->cb.startDataPump = usbvideo_StartDataPump;
	if (cams->cb.stopDataPump == NULL)
		cams->cb.stopDataPump = usbvideo_StopDataPump;

	cams->num_cameras = num_cams;
	cams->cam = (struct uvd *) &cams[1];
	cams->md_module = md;
	mutex_init(&cams->lock);	

	for (i = 0; i < num_cams; i++) {
		struct uvd *up = &cams->cam[i];

		up->handle = cams;

		
		if (num_extra > 0) {
			up->user_size = num_cams * num_extra;
			up->user_data = kmalloc(up->user_size, GFP_KERNEL);
			if (up->user_data == NULL) {
				err("%s: Failed to allocate user_data (%d. bytes)",
				    __func__, up->user_size);
				while (i) {
					up = &cams->cam[--i];
					kfree(up->user_data);
				}
				kfree(cams);
				return -ENOMEM;
			}
			dbg("%s: Allocated cams[%d].user_data=$%p (%d. bytes)",
			     __func__, i, up->user_data, up->user_size);
		}
	}

	
	strcpy(cams->drvName, (driverName != NULL) ? driverName : "Unknown");
	cams->usbdrv.name = cams->drvName;
	cams->usbdrv.probe = cams->cb.probe;
	cams->usbdrv.disconnect = cams->cb.disconnect;
	cams->usbdrv.id_table = id_table;

	
	*pCams = cams;
	result = usb_register(&cams->usbdrv);
	if (result) {
		for (i = 0; i < num_cams; i++) {
			struct uvd *up = &cams->cam[i];
			kfree(up->user_data);
		}
		kfree(cams);
	}

	return result;
}

EXPORT_SYMBOL(usbvideo_register);


void usbvideo_Deregister(struct usbvideo **pCams)
{
	struct usbvideo *cams;
	int i;

	if (pCams == NULL) {
		err("%s: pCams == NULL", __func__);
		return;
	}
	cams = *pCams;
	if (cams == NULL) {
		err("%s: cams == NULL", __func__);
		return;
	}

	dbg("%s: Deregistering %s driver.", __func__, cams->drvName);
	usb_deregister(&cams->usbdrv);

	dbg("%s: Deallocating cams=$%p (%d. cameras)", __func__, cams, cams->num_cameras);
	for (i=0; i < cams->num_cameras; i++) {
		struct uvd *up = &cams->cam[i];
		int warning = 0;

		if (up->user_data != NULL) {
			if (up->user_size <= 0)
				++warning;
		} else {
			if (up->user_size > 0)
				++warning;
		}
		if (warning) {
			err("%s: Warning: user_data=$%p user_size=%d.",
			    __func__, up->user_data, up->user_size);
		} else {
			dbg("%s: Freeing %d. $%p->user_data=$%p",
			    __func__, i, up, up->user_data);
			kfree(up->user_data);
		}
	}
	
	dbg("%s: Freed %d uvd structures",
	    __func__, cams->num_cameras);
	kfree(cams);
	*pCams = NULL;
}

EXPORT_SYMBOL(usbvideo_Deregister);


static void usbvideo_Disconnect(struct usb_interface *intf)
{
	struct uvd *uvd = usb_get_intfdata (intf);
	int i;

	if (uvd == NULL) {
		err("%s($%p): Illegal call.", __func__, intf);
		return;
	}

	usb_set_intfdata (intf, NULL);

	usbvideo_ClientIncModCount(uvd);
	if (uvd->debug > 0)
		dev_info(&intf->dev, "%s(%p.)\n", __func__, intf);

	mutex_lock(&uvd->lock);
	uvd->remove_pending = 1; 

	
	GET_CALLBACK(uvd, stopDataPump)(uvd);

	for (i=0; i < USBVIDEO_NUMSBUF; i++)
		usb_free_urb(uvd->sbuf[i].urb);

	usb_put_dev(uvd->dev);
	uvd->dev = NULL;    	    

	video_unregister_device(&uvd->vdev);
	if (uvd->debug > 0)
		dev_info(&intf->dev, "%s: Video unregistered.\n", __func__);

	if (uvd->user)
		dev_info(&intf->dev, "%s: In use, disconnect pending.\n",
			 __func__);
	else
		usbvideo_CameraRelease(uvd);
	mutex_unlock(&uvd->lock);
	dev_info(&intf->dev, "USB camera disconnected.\n");

	usbvideo_ClientDecModCount(uvd);
}


static void usbvideo_CameraRelease(struct uvd *uvd)
{
	if (uvd == NULL) {
		err("%s: Illegal call", __func__);
		return;
	}

	RingQueue_Free(&uvd->dp);
	if (VALID_CALLBACK(uvd, userFree))
		GET_CALLBACK(uvd, userFree)(uvd);
	uvd->uvd_used = 0;	
}


static int usbvideo_find_struct(struct usbvideo *cams)
{
	int u, rv = -1;

	if (cams == NULL) {
		err("No usbvideo handle?");
		return -1;
	}
	mutex_lock(&cams->lock);
	for (u = 0; u < cams->num_cameras; u++) {
		struct uvd *uvd = &cams->cam[u];
		if (!uvd->uvd_used) 
		{
			uvd->uvd_used = 1;	
			mutex_init(&uvd->lock);	
			uvd->dev = NULL;
			rv = u;
			break;
		}
	}
	mutex_unlock(&cams->lock);
	return rv;
}

static const struct v4l2_file_operations usbvideo_fops = {
	.owner =  THIS_MODULE,
	.open =   usbvideo_v4l_open,
	.release =usbvideo_v4l_close,
	.read =   usbvideo_v4l_read,
	.mmap =   usbvideo_v4l_mmap,
	.ioctl =  usbvideo_v4l_ioctl,
};
static const struct video_device usbvideo_template = {
	.fops =       &usbvideo_fops,
};

struct uvd *usbvideo_AllocateDevice(struct usbvideo *cams)
{
	int i, devnum;
	struct uvd *uvd = NULL;

	if (cams == NULL) {
		err("No usbvideo handle?");
		return NULL;
	}

	devnum = usbvideo_find_struct(cams);
	if (devnum == -1) {
		err("IBM USB camera driver: Too many devices!");
		return NULL;
	}
	uvd = &cams->cam[devnum];
	dbg("Device entry #%d. at $%p", devnum, uvd);

	
	usbvideo_ClientIncModCount(uvd);

	mutex_lock(&uvd->lock);
	for (i=0; i < USBVIDEO_NUMSBUF; i++) {
		uvd->sbuf[i].urb = usb_alloc_urb(FRAMES_PER_DESC, GFP_KERNEL);
		if (uvd->sbuf[i].urb == NULL) {
			err("usb_alloc_urb(%d.) failed.", FRAMES_PER_DESC);
			uvd->uvd_used = 0;
			uvd = NULL;
			goto allocate_done;
		}
	}
	uvd->user=0;
	uvd->remove_pending = 0;
	uvd->last_error = 0;
	RingQueue_Initialize(&uvd->dp);

	
	uvd->vdev = usbvideo_template;
	sprintf(uvd->vdev.name, "%.20s USB Camera", cams->drvName);
	
allocate_done:
	mutex_unlock(&uvd->lock);
	usbvideo_ClientDecModCount(uvd);
	return uvd;
}

EXPORT_SYMBOL(usbvideo_AllocateDevice);

int usbvideo_RegisterVideoDevice(struct uvd *uvd)
{
	char tmp1[20], tmp2[20];	

	if (uvd == NULL) {
		err("%s: Illegal call.", __func__);
		return -EINVAL;
	}
	if (uvd->video_endp == 0) {
		dev_info(&uvd->dev->dev,
			 "%s: No video endpoint specified; data pump disabled.\n",
			 __func__);
	}
	if (uvd->paletteBits == 0) {
		err("%s: No palettes specified!", __func__);
		return -EINVAL;
	}
	if (uvd->defaultPalette == 0) {
		dev_info(&uvd->dev->dev, "%s: No default palette!\n",
			 __func__);
	}

	uvd->max_frame_size = VIDEOSIZE_X(uvd->canvas) *
		VIDEOSIZE_Y(uvd->canvas) * V4L_BYTES_PER_PIXEL;
	usbvideo_VideosizeToString(tmp1, sizeof(tmp1), uvd->videosize);
	usbvideo_VideosizeToString(tmp2, sizeof(tmp2), uvd->canvas);

	if (uvd->debug > 0) {
		dev_info(&uvd->dev->dev,
			 "%s: iface=%d. endpoint=$%02x paletteBits=$%08lx\n",
			 __func__, uvd->iface, uvd->video_endp,
			 uvd->paletteBits);
	}
	if (uvd->dev == NULL) {
		err("%s: uvd->dev == NULL", __func__);
		return -EINVAL;
	}
	uvd->vdev.parent = &uvd->dev->dev;
	uvd->vdev.release = video_device_release_empty;
	if (video_register_device(&uvd->vdev, VFL_TYPE_GRABBER, video_nr) < 0) {
		err("%s: video_register_device failed", __func__);
		return -EPIPE;
	}
	if (uvd->debug > 1) {
		dev_info(&uvd->dev->dev,
			 "%s: video_register_device() successful\n", __func__);
	}

	dev_info(&uvd->dev->dev, "%s on /dev/video%d: canvas=%s videosize=%s\n",
		 (uvd->handle != NULL) ? uvd->handle->drvName : "???",
		 uvd->vdev.num, tmp2, tmp1);

	usb_get_dev(uvd->dev);
	return 0;
}

EXPORT_SYMBOL(usbvideo_RegisterVideoDevice);



static int usbvideo_v4l_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct uvd *uvd = file->private_data;
	unsigned long start = vma->vm_start;
	unsigned long size  = vma->vm_end-vma->vm_start;
	unsigned long page, pos;

	if (!CAMERA_IS_OPERATIONAL(uvd))
		return -EFAULT;

	if (size > (((USBVIDEO_NUMFRAMES * uvd->max_frame_size) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)))
		return -EINVAL;

	pos = (unsigned long) uvd->fbuf;
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


static int usbvideo_v4l_open(struct file *file)
{
	struct video_device *dev = video_devdata(file);
	struct uvd *uvd = (struct uvd *) dev;
	const int sb_size = FRAMES_PER_DESC * uvd->iso_packet_len;
	int i, errCode = 0;

	if (uvd->debug > 1)
		dev_info(&uvd->dev->dev, "%s($%p)\n", __func__, dev);

	if (usbvideo_ClientIncModCount(uvd) < 0)
		return -ENODEV;
	mutex_lock(&uvd->lock);

	if (uvd->user) {
		err("%s: Someone tried to open an already opened device!", __func__);
		errCode = -EBUSY;
	} else {
		
		memset(&uvd->stats, 0, sizeof(uvd->stats));

		
		for (i=0; i < USBVIDEO_NUMSBUF; i++)
			uvd->sbuf[i].data = NULL;

		
		uvd->fbuf_size = USBVIDEO_NUMFRAMES * uvd->max_frame_size;
		uvd->fbuf = usbvideo_rvmalloc(uvd->fbuf_size);
		RingQueue_Allocate(&uvd->dp, RING_QUEUE_SIZE);
		if ((uvd->fbuf == NULL) ||
		    (!RingQueue_IsAllocated(&uvd->dp))) {
			err("%s: Failed to allocate fbuf or dp", __func__);
			errCode = -ENOMEM;
		} else {
			
			for (i=0; i < USBVIDEO_NUMFRAMES; i++) {
				uvd->frame[i].frameState = FrameState_Unused;
				uvd->frame[i].data = uvd->fbuf + i*(uvd->max_frame_size);
				
				uvd->frame[i].canvas = uvd->canvas;
				uvd->frame[i].seqRead_Index = 0;
			}
			for (i=0; i < USBVIDEO_NUMSBUF; i++) {
				uvd->sbuf[i].data = kmalloc(sb_size, GFP_KERNEL);
				if (uvd->sbuf[i].data == NULL) {
					errCode = -ENOMEM;
					break;
				}
			}
		}
		if (errCode != 0) {
			
			if (uvd->fbuf != NULL) {
				usbvideo_rvfree(uvd->fbuf, uvd->fbuf_size);
				uvd->fbuf = NULL;
			}
			RingQueue_Free(&uvd->dp);
			for (i=0; i < USBVIDEO_NUMSBUF; i++) {
				kfree(uvd->sbuf[i].data);
				uvd->sbuf[i].data = NULL;
			}
		}
	}

	
	if (errCode == 0) {
		
		if (uvd->video_endp != 0)
			errCode = GET_CALLBACK(uvd, startDataPump)(uvd);
		if (errCode == 0) {
			if (VALID_CALLBACK(uvd, setupOnOpen)) {
				if (uvd->debug > 1)
					dev_info(&uvd->dev->dev,
						 "%s: setupOnOpen callback\n",
						 __func__);
				errCode = GET_CALLBACK(uvd, setupOnOpen)(uvd);
				if (errCode < 0) {
					err("%s: setupOnOpen callback failed (%d.).",
					    __func__, errCode);
				} else if (uvd->debug > 1) {
					dev_info(&uvd->dev->dev,
						 "%s: setupOnOpen callback successful\n",
						 __func__);
				}
			}
			if (errCode == 0) {
				uvd->settingsAdjusted = 0;
				if (uvd->debug > 1)
					dev_info(&uvd->dev->dev,
						 "%s: Open succeeded.\n",
						 __func__);
				uvd->user++;
				file->private_data = uvd;
			}
		}
	}
	mutex_unlock(&uvd->lock);
	if (errCode != 0)
		usbvideo_ClientDecModCount(uvd);
	if (uvd->debug > 0)
		dev_info(&uvd->dev->dev, "%s: Returning %d.\n", __func__,
			 errCode);
	return errCode;
}


static int usbvideo_v4l_close(struct file *file)
{
	struct video_device *dev = file->private_data;
	struct uvd *uvd = (struct uvd *) dev;
	int i;

	if (uvd->debug > 1)
		dev_info(&uvd->dev->dev, "%s($%p)\n", __func__, dev);

	mutex_lock(&uvd->lock);
	GET_CALLBACK(uvd, stopDataPump)(uvd);
	usbvideo_rvfree(uvd->fbuf, uvd->fbuf_size);
	uvd->fbuf = NULL;
	RingQueue_Free(&uvd->dp);

	for (i=0; i < USBVIDEO_NUMSBUF; i++) {
		kfree(uvd->sbuf[i].data);
		uvd->sbuf[i].data = NULL;
	}

#if USBVIDEO_REPORT_STATS
	usbvideo_ReportStatistics(uvd);
#endif

	uvd->user--;
	if (uvd->remove_pending) {
		if (uvd->debug > 0)
			dev_info(&uvd->dev->dev, "%s: Final disconnect.\n",
				 __func__);
		usbvideo_CameraRelease(uvd);
	}
	mutex_unlock(&uvd->lock);
	usbvideo_ClientDecModCount(uvd);

	if (uvd->debug > 1)
		dev_info(&uvd->dev->dev, "%s: Completed.\n", __func__);
	file->private_data = NULL;
	return 0;
}


static long usbvideo_v4l_do_ioctl(struct file *file, unsigned int cmd, void *arg)
{
	struct uvd *uvd = file->private_data;

	if (!CAMERA_IS_OPERATIONAL(uvd))
		return -EIO;

	switch (cmd) {
		case VIDIOCGCAP:
		{
			struct video_capability *b = arg;
			*b = uvd->vcap;
			return 0;
		}
		case VIDIOCGCHAN:
		{
			struct video_channel *v = arg;
			*v = uvd->vchan;
			return 0;
		}
		case VIDIOCSCHAN:
		{
			struct video_channel *v = arg;
			if (v->channel != 0)
				return -EINVAL;
			return 0;
		}
		case VIDIOCGPICT:
		{
			struct video_picture *pic = arg;
			*pic = uvd->vpic;
			return 0;
		}
		case VIDIOCSPICT:
		{
			struct video_picture *pic = arg;
			
			uvd->vpic.brightness = pic->brightness;
			uvd->vpic.hue = pic->hue;
			uvd->vpic.colour = pic->colour;
			uvd->vpic.contrast = pic->contrast;
			uvd->settingsAdjusted = 0;	
			return 0;
		}
		case VIDIOCSWIN:
		{
			struct video_window *vw = arg;

			if(VALID_CALLBACK(uvd, setVideoMode)) {
				return GET_CALLBACK(uvd, setVideoMode)(uvd, vw);
			}

			if (vw->flags)
				return -EINVAL;
			if (vw->clipcount)
				return -EINVAL;
			if (vw->width != VIDEOSIZE_X(uvd->canvas))
				return -EINVAL;
			if (vw->height != VIDEOSIZE_Y(uvd->canvas))
				return -EINVAL;

			return 0;
		}
		case VIDIOCGWIN:
		{
			struct video_window *vw = arg;

			vw->x = 0;
			vw->y = 0;
			vw->width = VIDEOSIZE_X(uvd->videosize);
			vw->height = VIDEOSIZE_Y(uvd->videosize);
			vw->chromakey = 0;
			if (VALID_CALLBACK(uvd, getFPS))
				vw->flags = GET_CALLBACK(uvd, getFPS)(uvd);
			else
				vw->flags = 10; 
			return 0;
		}
		case VIDIOCGMBUF:
		{
			struct video_mbuf *vm = arg;
			int i;

			memset(vm, 0, sizeof(*vm));
			vm->size = uvd->max_frame_size * USBVIDEO_NUMFRAMES;
			vm->frames = USBVIDEO_NUMFRAMES;
			for(i = 0; i < USBVIDEO_NUMFRAMES; i++)
			  vm->offsets[i] = i * uvd->max_frame_size;

			return 0;
		}
		case VIDIOCMCAPTURE:
		{
			struct video_mmap *vm = arg;

			if (uvd->debug >= 1) {
				dev_info(&uvd->dev->dev,
					 "VIDIOCMCAPTURE: frame=%d. size=%dx%d, format=%d.\n",
					 vm->frame, vm->width, vm->height, vm->format);
			}
			
			if ((vm->width > VIDEOSIZE_X(uvd->canvas)) ||
			    (vm->height > VIDEOSIZE_Y(uvd->canvas))) {
				if (uvd->debug > 0) {
					dev_info(&uvd->dev->dev,
						 "VIDIOCMCAPTURE: Size=%dx%d "
						 "too large; allowed only up "
						 "to %ldx%ld\n", vm->width,
						 vm->height,
						 VIDEOSIZE_X(uvd->canvas),
						 VIDEOSIZE_Y(uvd->canvas));
				}
				return -EINVAL;
			}
			
			if (((1L << vm->format) & uvd->paletteBits) == 0) {
				if (uvd->debug > 0) {
					dev_info(&uvd->dev->dev,
						 "VIDIOCMCAPTURE: format=%d. "
						 "not supported "
						 "(paletteBits=$%08lx)\n",
						 vm->format, uvd->paletteBits);
				}
				return -EINVAL;
			}
			if ((vm->frame < 0) || (vm->frame >= USBVIDEO_NUMFRAMES)) {
				err("VIDIOCMCAPTURE: vm.frame=%d. !E [0-%d]", vm->frame, USBVIDEO_NUMFRAMES-1);
				return -EINVAL;
			}
			if (uvd->frame[vm->frame].frameState == FrameState_Grabbing) {
				
			}
			uvd->frame[vm->frame].request = VIDEOSIZE(vm->width, vm->height);
			uvd->frame[vm->frame].palette = vm->format;

			
			uvd->frame[vm->frame].frameState = FrameState_Ready;

			return usbvideo_NewFrame(uvd, vm->frame);
		}
		case VIDIOCSYNC:
		{
			int *frameNum = arg;
			int ret;

			if (*frameNum < 0 || *frameNum >= USBVIDEO_NUMFRAMES)
				return -EINVAL;

			if (uvd->debug >= 1)
				dev_info(&uvd->dev->dev,
					 "VIDIOCSYNC: syncing to frame %d.\n",
					 *frameNum);
			if (uvd->flags & FLAGS_NO_DECODING)
				ret = usbvideo_GetFrame(uvd, *frameNum);
			else if (VALID_CALLBACK(uvd, getFrame)) {
				ret = GET_CALLBACK(uvd, getFrame)(uvd, *frameNum);
				if ((ret < 0) && (uvd->debug >= 1)) {
					err("VIDIOCSYNC: getFrame() returned %d.", ret);
				}
			} else {
				err("VIDIOCSYNC: getFrame is not set");
				ret = -EFAULT;
			}

			
			uvd->frame[*frameNum].frameState = FrameState_Unused;
			return ret;
		}
		case VIDIOCGFBUF:
		{
			struct video_buffer *vb = arg;

			memset(vb, 0, sizeof(*vb));
			return 0;
		}
		case VIDIOCKEY:
			return 0;

		case VIDIOCCAPTURE:
			return -EINVAL;

		case VIDIOCSFBUF:

		case VIDIOCGTUNER:
		case VIDIOCSTUNER:

		case VIDIOCGFREQ:
		case VIDIOCSFREQ:

		case VIDIOCGAUDIO:
		case VIDIOCSAUDIO:
			return -EINVAL;

		default:
			return -ENOIOCTLCMD;
	}
	return 0;
}

static long usbvideo_v4l_ioctl(struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	return video_usercopy(file, cmd, arg, usbvideo_v4l_do_ioctl);
}


static ssize_t usbvideo_v4l_read(struct file *file, char __user *buf,
		      size_t count, loff_t *ppos)
{
	struct uvd *uvd = file->private_data;
	int noblock = file->f_flags & O_NONBLOCK;
	int frmx = -1, i;
	struct usbvideo_frame *frame;

	if (!CAMERA_IS_OPERATIONAL(uvd) || (buf == NULL))
		return -EFAULT;

	if (uvd->debug >= 1)
		dev_info(&uvd->dev->dev,
			 "%s: %Zd. bytes, noblock=%d.\n",
			 __func__, count, noblock);

	mutex_lock(&uvd->lock);

	
	for(i = 0; i < USBVIDEO_NUMFRAMES; i++) {
		if ((uvd->frame[i].frameState == FrameState_Done) ||
		    (uvd->frame[i].frameState == FrameState_Done_Hold) ||
		    (uvd->frame[i].frameState == FrameState_Error)) {
			frmx = i;
			break;
		}
	}

	
	if (noblock && (frmx == -1)) {
		count = -EAGAIN;
		goto read_done;
	}

	
	if (frmx == -1) {
		for(i = 0; i < USBVIDEO_NUMFRAMES; i++) {
			if (uvd->frame[i].frameState == FrameState_Grabbing) {
				frmx = i;
				break;
			}
		}
	}

	
	if (frmx == -1) {
		if (uvd->defaultPalette == 0) {
			err("%s: No default palette; don't know what to do!", __func__);
			count = -EFAULT;
			goto read_done;
		}
		frmx = 0;
		
		uvd->frame[frmx].request = uvd->videosize;
		uvd->frame[frmx].palette = uvd->defaultPalette;
		uvd->frame[frmx].frameState = FrameState_Ready;
		usbvideo_NewFrame(uvd, frmx);
		
	}

	
	frame = &uvd->frame[frmx];

	
	if (frame->frameState != FrameState_Done_Hold) {
		long rv = -EFAULT;
		if (uvd->flags & FLAGS_NO_DECODING)
			rv = usbvideo_GetFrame(uvd, frmx);
		else if (VALID_CALLBACK(uvd, getFrame))
			rv = GET_CALLBACK(uvd, getFrame)(uvd, frmx);
		else
			err("getFrame is not set");
		if ((rv != 0) || (frame->frameState != FrameState_Done_Hold)) {
			count = rv;
			goto read_done;
		}
	}

	
	if ((count + frame->seqRead_Index) > frame->seqRead_Length)
		count = frame->seqRead_Length - frame->seqRead_Index;

	
	if (copy_to_user(buf, frame->data + frame->seqRead_Index, count)) {
		count = -EFAULT;
		goto read_done;
	}

	
	frame->seqRead_Index += count;
	if (uvd->debug >= 1) {
		err("%s: {copy} count used=%Zd, new seqRead_Index=%ld",
			__func__, count, frame->seqRead_Index);
	}

	
	if (frame->seqRead_Index >= frame->seqRead_Length) {
		
		frame->seqRead_Index = 0;

		
		uvd->frame[frmx].frameState = FrameState_Unused;
		if (usbvideo_NewFrame(uvd, (frmx + 1) % USBVIDEO_NUMFRAMES)) {
			err("%s: usbvideo_NewFrame failed.", __func__);
		}
	}
read_done:
	mutex_unlock(&uvd->lock);
	return count;
}


static int usbvideo_CompressIsochronous(struct uvd *uvd, struct urb *urb)
{
	char *cdata;
	int i, totlen = 0;

	for (i = 0; i < urb->number_of_packets; i++) {
		int n = urb->iso_frame_desc[i].actual_length;
		int st = urb->iso_frame_desc[i].status;

		cdata = urb->transfer_buffer + urb->iso_frame_desc[i].offset;

		
		if (st < 0) {
			if (uvd->debug >= 1)
				err("Data error: packet=%d. len=%d. status=%d.", i, n, st);
			uvd->stats.iso_err_count++;
			continue;
		}

		
		if (n <= 0) {
			uvd->stats.iso_skip_count++;
			continue;
		}
		totlen += n;	
		RingQueue_Enqueue(&uvd->dp, cdata, n);
	}
	return totlen;
}

static void usbvideo_IsocIrq(struct urb *urb)
{
	int i, ret, len;
	struct uvd *uvd = urb->context;

	
	if (!CAMERA_IS_OPERATIONAL(uvd))
		return;
#if 0
	if (urb->actual_length > 0) {
		dev_info(&uvd->dev->dev,
			 "urb=$%p status=%d. errcount=%d. length=%d.\n",
			 urb, urb->status, urb->error_count,
			 urb->actual_length);
	} else {
		static int c = 0;
		if (c++ % 100 == 0)
			dev_info(&uvd->dev->dev, "No Isoc data\n");
	}
#endif

	if (!uvd->streaming) {
		if (uvd->debug >= 1)
			dev_info(&uvd->dev->dev,
				 "Not streaming, but interrupt!\n");
		return;
	}

	uvd->stats.urb_count++;
	if (urb->actual_length <= 0)
		goto urb_done_with;

	
	len = usbvideo_CompressIsochronous(uvd, urb);
	uvd->stats.urb_length = len;
	if (len <= 0)
		goto urb_done_with;

	
	uvd->stats.data_count += len;
	RingQueue_WakeUpInterruptible(&uvd->dp);

urb_done_with:
	for (i = 0; i < FRAMES_PER_DESC; i++) {
		urb->iso_frame_desc[i].status = 0;
		urb->iso_frame_desc[i].actual_length = 0;
	}
	urb->status = 0;
	urb->dev = uvd->dev;
	ret = usb_submit_urb (urb, GFP_KERNEL);
	if(ret)
		err("usb_submit_urb error (%d)", ret);
	return;
}


static int usbvideo_StartDataPump(struct uvd *uvd)
{
	struct usb_device *dev = uvd->dev;
	int i, errFlag;

	if (uvd->debug > 1)
		dev_info(&uvd->dev->dev, "%s($%p)\n", __func__, uvd);

	if (!CAMERA_IS_OPERATIONAL(uvd)) {
		err("%s: Camera is not operational", __func__);
		return -EFAULT;
	}
	uvd->curframe = -1;

	
	i = usb_set_interface(dev, uvd->iface, uvd->ifaceAltActive);
	if (i < 0) {
		err("%s: usb_set_interface error", __func__);
		uvd->last_error = i;
		return -EBUSY;
	}
	if (VALID_CALLBACK(uvd, videoStart))
		GET_CALLBACK(uvd, videoStart)(uvd);
	else
		err("%s: videoStart not set", __func__);

	
	for (i=0; i < USBVIDEO_NUMSBUF; i++) {
		int j, k;
		struct urb *urb = uvd->sbuf[i].urb;
		urb->dev = dev;
		urb->context = uvd;
		urb->pipe = usb_rcvisocpipe(dev, uvd->video_endp);
		urb->interval = 1;
		urb->transfer_flags = URB_ISO_ASAP;
		urb->transfer_buffer = uvd->sbuf[i].data;
		urb->complete = usbvideo_IsocIrq;
		urb->number_of_packets = FRAMES_PER_DESC;
		urb->transfer_buffer_length = uvd->iso_packet_len * FRAMES_PER_DESC;
		for (j=k=0; j < FRAMES_PER_DESC; j++, k += uvd->iso_packet_len) {
			urb->iso_frame_desc[j].offset = k;
			urb->iso_frame_desc[j].length = uvd->iso_packet_len;
		}
	}

	
	for (i=0; i < USBVIDEO_NUMSBUF; i++) {
		errFlag = usb_submit_urb(uvd->sbuf[i].urb, GFP_KERNEL);
		if (errFlag)
			err("%s: usb_submit_isoc(%d) ret %d", __func__, i, errFlag);
	}

	uvd->streaming = 1;
	if (uvd->debug > 1)
		dev_info(&uvd->dev->dev,
			 "%s: streaming=1 video_endp=$%02x\n", __func__,
			 uvd->video_endp);
	return 0;
}


static void usbvideo_StopDataPump(struct uvd *uvd)
{
	int i, j;

	if ((uvd == NULL) || (!uvd->streaming) || (uvd->dev == NULL))
		return;

	if (uvd->debug > 1)
		dev_info(&uvd->dev->dev, "%s($%p)\n", __func__, uvd);

	
	for (i=0; i < USBVIDEO_NUMSBUF; i++) {
		usb_kill_urb(uvd->sbuf[i].urb);
	}
	if (uvd->debug > 1)
		dev_info(&uvd->dev->dev, "%s: streaming=0\n", __func__);
	uvd->streaming = 0;

	if (!uvd->remove_pending) {
		
		if (VALID_CALLBACK(uvd, videoStop))
			GET_CALLBACK(uvd, videoStop)(uvd);
		else
			err("%s: videoStop not set", __func__);

		
		j = usb_set_interface(uvd->dev, uvd->iface, uvd->ifaceAltInactive);
		if (j < 0) {
			err("%s: usb_set_interface() error %d.", __func__, j);
			uvd->last_error = j;
		}
	}
}


static int usbvideo_NewFrame(struct uvd *uvd, int framenum)
{
	struct usbvideo_frame *frame;
	int n;

	if (uvd->debug > 1)
		dev_info(&uvd->dev->dev, "usbvideo_NewFrame($%p,%d.)\n", uvd,
			 framenum);

	
	
	if (uvd->curframe != -1)
		return 0;

	
	if (!uvd->settingsAdjusted) {
		if (VALID_CALLBACK(uvd, adjustPicture))
			GET_CALLBACK(uvd, adjustPicture)(uvd);
		uvd->settingsAdjusted = 1;
	}

	n = (framenum + 1) % USBVIDEO_NUMFRAMES;
	if (uvd->frame[n].frameState == FrameState_Ready)
		framenum = n;

	frame = &uvd->frame[framenum];

	frame->frameState = FrameState_Grabbing;
	frame->scanstate = ScanState_Scanning;
	frame->seqRead_Length = 0;	
	frame->deinterlace = Deinterlace_None;
	frame->flags = 0; 
	uvd->curframe = framenum;

	
	if (!(uvd->flags & FLAGS_SEPARATE_FRAMES)) {
		
		int prev = (framenum - 1 + USBVIDEO_NUMFRAMES) % USBVIDEO_NUMFRAMES;
		memmove(frame->data, uvd->frame[prev].data, uvd->max_frame_size);
	} else {
		if (uvd->flags & FLAGS_CLEAN_FRAMES) {
			
			memset(frame->data, 0, uvd->max_frame_size);
		}
	}
	return 0;
}


static void usbvideo_CollectRawData(struct uvd *uvd, struct usbvideo_frame *frame)
{
	int n;

	assert(uvd != NULL);
	assert(frame != NULL);

	
	n = RingQueue_GetLength(&uvd->dp);
	if (n > 0) {
		int m;
		
		m = uvd->max_frame_size - frame->seqRead_Length;
		if (n > m)
			n = m;
		
		RingQueue_Dequeue(
			&uvd->dp,
			frame->data + frame->seqRead_Length,
			m);
		frame->seqRead_Length += m;
	}
	
	if (frame->seqRead_Length >= uvd->max_frame_size) {
		frame->frameState = FrameState_Done;
		uvd->curframe = -1;
		uvd->stats.frame_num++;
	}
}

static int usbvideo_GetFrame(struct uvd *uvd, int frameNum)
{
	struct usbvideo_frame *frame = &uvd->frame[frameNum];

	if (uvd->debug >= 2)
		dev_info(&uvd->dev->dev, "%s($%p,%d.)\n", __func__, uvd,
			 frameNum);

	switch (frame->frameState) {
	case FrameState_Unused:
		if (uvd->debug >= 2)
			dev_info(&uvd->dev->dev, "%s: FrameState_Unused\n",
				 __func__);
		return -EINVAL;
	case FrameState_Ready:
	case FrameState_Grabbing:
	case FrameState_Error:
	{
		int ntries, signalPending;
	redo:
		if (!CAMERA_IS_OPERATIONAL(uvd)) {
			if (uvd->debug >= 2)
				dev_info(&uvd->dev->dev,
					 "%s: Camera is not operational (1)\n",
					 __func__);
			return -EIO;
		}
		ntries = 0;
		do {
			RingQueue_InterruptibleSleepOn(&uvd->dp);
			signalPending = signal_pending(current);
			if (!CAMERA_IS_OPERATIONAL(uvd)) {
				if (uvd->debug >= 2)
					dev_info(&uvd->dev->dev,
						 "%s: Camera is not "
						 "operational (2)\n", __func__);
				return -EIO;
			}
			assert(uvd->fbuf != NULL);
			if (signalPending) {
				if (uvd->debug >= 2)
					dev_info(&uvd->dev->dev,
					"%s: Signal=$%08x\n", __func__,
					signalPending);
				if (uvd->flags & FLAGS_RETRY_VIDIOCSYNC) {
					usbvideo_TestPattern(uvd, 1, 0);
					uvd->curframe = -1;
					uvd->stats.frame_num++;
					if (uvd->debug >= 2)
						dev_info(&uvd->dev->dev,
							 "%s: Forced test "
							 "pattern screen\n",
							 __func__);
					return 0;
				} else {
					
					if (uvd->debug >= 2)
						dev_info(&uvd->dev->dev,
							 "%s: Interrupted!\n",
							 __func__);
					return -EINTR;
				}
			} else {
				
				if (uvd->flags & FLAGS_NO_DECODING)
					usbvideo_CollectRawData(uvd, frame);
				else if (VALID_CALLBACK(uvd, processData))
					GET_CALLBACK(uvd, processData)(uvd, frame);
				else
					err("%s: processData not set", __func__);
			}
		} while (frame->frameState == FrameState_Grabbing);
		if (uvd->debug >= 2) {
			dev_info(&uvd->dev->dev,
				 "%s: Grabbing done; state=%d. (%lu. bytes)\n",
				 __func__, frame->frameState,
				 frame->seqRead_Length);
		}
		if (frame->frameState == FrameState_Error) {
			int ret = usbvideo_NewFrame(uvd, frameNum);
			if (ret < 0) {
				err("%s: usbvideo_NewFrame() failed (%d.)", __func__, ret);
				return ret;
			}
			goto redo;
		}
		
	}
	case FrameState_Done:
		
		uvd->stats.frame_num++;
		if ((uvd->flags & FLAGS_NO_DECODING) == 0) {
			if (VALID_CALLBACK(uvd, postProcess))
				GET_CALLBACK(uvd, postProcess)(uvd, frame);
			if (frame->flags & USBVIDEO_FRAME_FLAG_SOFTWARE_CONTRAST)
				usbvideo_SoftwareContrastAdjustment(uvd, frame);
		}
		frame->frameState = FrameState_Done_Hold;
		if (uvd->debug >= 2)
			dev_info(&uvd->dev->dev,
				 "%s: Entered FrameState_Done_Hold state.\n",
				 __func__);
		return 0;

	case FrameState_Done_Hold:
		
		if (uvd->debug >= 2)
			dev_info(&uvd->dev->dev,
				 "%s: FrameState_Done_Hold state.\n",
				 __func__);
		return 0;
	}

	
	err("%s: Invalid state %d.", __func__, frame->frameState);
	frame->frameState = FrameState_Unused;
	return 0;
}


void usbvideo_DeinterlaceFrame(struct uvd *uvd, struct usbvideo_frame *frame)
{
	if ((uvd == NULL) || (frame == NULL))
		return;

	if ((frame->deinterlace == Deinterlace_FillEvenLines) ||
	    (frame->deinterlace == Deinterlace_FillOddLines))
	{
		const int v4l_linesize = VIDEOSIZE_X(frame->request) * V4L_BYTES_PER_PIXEL;
		int i = (frame->deinterlace == Deinterlace_FillEvenLines) ? 0 : 1;

		for (; i < VIDEOSIZE_Y(frame->request); i += 2) {
			const unsigned char *fs1, *fs2;
			unsigned char *fd;
			int ip, in, j;	

			
			ip = i - 1;	
			in = i + 1;

			
			if (ip < 0)
				ip = in;
			if (in >= VIDEOSIZE_Y(frame->request))
				in = ip;

			
			if ((ip < 0) || (in < 0) ||
			    (ip >= VIDEOSIZE_Y(frame->request)) ||
			    (in >= VIDEOSIZE_Y(frame->request)))
			{
				err("Error: ip=%d. in=%d. req.height=%ld.",
				    ip, in, VIDEOSIZE_Y(frame->request));
				break;
			}

			
			fs1 = frame->data + (v4l_linesize * ip);
			fs2 = frame->data + (v4l_linesize * in);
			fd = frame->data + (v4l_linesize * i);

			
			for (j=0; j < v4l_linesize; j++) {
				fd[j] = (unsigned char)((((unsigned) fs1[j]) +
							 ((unsigned)fs2[j])) >> 1);
			}
		}
	}

	
	if (uvd->flags & FLAGS_OVERLAY_STATS)
		usbvideo_OverlayStats(uvd, frame);
}

EXPORT_SYMBOL(usbvideo_DeinterlaceFrame);


static void usbvideo_SoftwareContrastAdjustment(struct uvd *uvd,
						struct usbvideo_frame *frame)
{
	int i, j, v4l_linesize;
	signed long adj;
	const int ccm = 128; 

	if ((uvd == NULL) || (frame == NULL)) {
		err("%s: Illegal call.", __func__);
		return;
	}
	adj = (uvd->vpic.contrast - 0x8000) >> 8; 
	RESTRICT_TO_RANGE(adj, -ccm, ccm+1);
	if (adj == 0) {
		
		return;
	}
	v4l_linesize = VIDEOSIZE_X(frame->request) * V4L_BYTES_PER_PIXEL;
	for (i=0; i < VIDEOSIZE_Y(frame->request); i++) {
		unsigned char *fd = frame->data + (v4l_linesize * i);
		for (j=0; j < v4l_linesize; j++) {
			signed long v = (signed long) fd[j];
			
			v = 128 + ((ccm + adj) * (v - 128)) / ccm;
			RESTRICT_TO_RANGE(v, 0, 0xFF); 
			fd[j] = (unsigned char) v;
		}
	}
}

MODULE_LICENSE("GPL");
