

static const char version[] = "0.24";

#include <linux/module.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/pagemap.h>
#include <linux/usb.h>
#include "se401.h"

static int flickerless;
static int video_nr = -1;

static struct usb_device_id device_table[] = {
	{ USB_DEVICE(0x03e8, 0x0004) },
	{ USB_DEVICE(0x0471, 0x030b) },
	{ USB_DEVICE(0x047d, 0x5001) },
	{ USB_DEVICE(0x047d, 0x5002) },
	{ USB_DEVICE(0x047d, 0x5003) },
	{ }
};

MODULE_DEVICE_TABLE(usb, device_table);

MODULE_AUTHOR("Jeroen Vreeken <pe1rxq@amsat.org>");
MODULE_DESCRIPTION("SE401 USB Camera Driver");
MODULE_LICENSE("GPL");
module_param(flickerless, int, 0);
MODULE_PARM_DESC(flickerless,
		"Net frequency to adjust exposure time to (0/50/60)");
module_param(video_nr, int, 0);

static struct usb_driver se401_driver;



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
		adr +=  PAGE_SIZE;
		size -=  PAGE_SIZE;
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
		adr +=  PAGE_SIZE;
		size -=  PAGE_SIZE;
	}
	vfree(mem);
}





static int se401_sndctrl(int set, struct usb_se401 *se401, unsigned short req,
			 unsigned short value, unsigned char *cp, int size)
{
	return usb_control_msg(
		se401->dev,
		set ? usb_sndctrlpipe(se401->dev, 0) : usb_rcvctrlpipe(se401->dev, 0),
		req,
		(set ? USB_DIR_OUT : USB_DIR_IN) | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		value,
		0,
		cp,
		size,
		1000
	);
}

static int se401_set_feature(struct usb_se401 *se401, unsigned short selector,
			     unsigned short param)
{
	
	return usb_control_msg(
		se401->dev,
		usb_sndctrlpipe(se401->dev, 0),
		SE401_REQ_SET_EXT_FEATURE,
		USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		param,
		selector,
		NULL,
		0,
		1000
	);
}

static unsigned short se401_get_feature(struct usb_se401 *se401,
					unsigned short selector)
{
	
	unsigned char cp[2];
	usb_control_msg(
		se401->dev,
		usb_rcvctrlpipe(se401->dev, 0),
		SE401_REQ_GET_EXT_FEATURE,
		USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
		0,
		selector,
		cp,
		2,
		1000
	);
	return cp[0]+cp[1]*256;
}




static int se401_send_pict(struct usb_se401 *se401)
{
	
	se401_set_feature(se401, HV7131_REG_TITL, se401->expose_l);
	
	se401_set_feature(se401, HV7131_REG_TITM, se401->expose_m);
	
	se401_set_feature(se401, HV7131_REG_TITU, se401->expose_h);
	
	se401_set_feature(se401, HV7131_REG_ARLV, se401->resetlevel);
	
	se401_set_feature(se401, HV7131_REG_ARCG, se401->rgain);
	
	se401_set_feature(se401, HV7131_REG_AGCG, se401->ggain);
	
	se401_set_feature(se401, HV7131_REG_ABCG, se401->bgain);

	return 0;
}

static void se401_set_exposure(struct usb_se401 *se401, int brightness)
{
	int integration = brightness << 5;

	if (flickerless == 50)
		integration = integration-integration % 106667;
	if (flickerless == 60)
		integration = integration-integration % 88889;
	se401->brightness = integration >> 5;
	se401->expose_h = (integration >> 16) & 0xff;
	se401->expose_m = (integration >> 8) & 0xff;
	se401->expose_l = integration & 0xff;
}

static int se401_get_pict(struct usb_se401 *se401, struct video_picture *p)
{
	p->brightness = se401->brightness;
	if (se401->enhance)
		p->whiteness = 32768;
	else
		p->whiteness = 0;

	p->colour = 65535;
	p->contrast = 65535;
	p->hue = se401->rgain << 10;
	p->palette = se401->palette;
	p->depth = 3; 
	return 0;
}


static int se401_set_pict(struct usb_se401 *se401, struct video_picture *p)
{
	if (p->palette != VIDEO_PALETTE_RGB24)
		return 1;
	se401->palette = p->palette;
	if (p->hue != se401->hue) {
		se401->rgain =  p->hue >> 10;
		se401->bgain =  0x40-(p->hue >> 10);
		se401->hue = p->hue;
	}
	if (p->brightness != se401->brightness)
		se401_set_exposure(se401, p->brightness);

	if (p->whiteness >= 32768)
		se401->enhance = 1;
	else
		se401->enhance = 0;
	se401_send_pict(se401);
	se401_send_pict(se401);
	return 0;
}


static void se401_auto_resetlevel(struct usb_se401 *se401)
{
	unsigned int ahrc, alrc;
	int oldreset = se401->resetlevel;

	
	se401_get_feature(se401, HV7131_REG_HIREFNOH);
	se401_get_feature(se401, HV7131_REG_HIREFNOL);
	se401_get_feature(se401, HV7131_REG_LOREFNOH);
	se401_get_feature(se401, HV7131_REG_LOREFNOL);
	ahrc = 256*se401_get_feature(se401, HV7131_REG_HIREFNOH) +
	    se401_get_feature(se401, HV7131_REG_HIREFNOL);
	alrc = 256*se401_get_feature(se401, HV7131_REG_LOREFNOH) +
	    se401_get_feature(se401, HV7131_REG_LOREFNOL);

	
	if (alrc > 10) {
		while (alrc >= 10 && se401->resetlevel < 63) {
			se401->resetlevel++;
			alrc /= 2;
		}
	} else if (ahrc > 20) {
		while (ahrc >= 20 && se401->resetlevel > 0) {
			se401->resetlevel--;
			ahrc /= 2;
		}
	}
	if (se401->resetlevel != oldreset)
		se401_set_feature(se401, HV7131_REG_ARLV, se401->resetlevel);

	return;
}


static void se401_button_irq(struct urb *urb)
{
	struct usb_se401 *se401 = urb->context;
	int status;

	if (!se401->dev) {
		dev_info(&urb->dev->dev, "device vapourished\n");
		return;
	}

	switch (urb->status) {
	case 0:
		
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		
		dbg("%s - urb shutting down with status: %d",
							__func__, urb->status);
		return;
	default:
		dbg("%s - nonzero urb status received: %d",
							__func__, urb->status);
		goto exit;
	}

	if (urb->actual_length  >= 2)
		if (se401->button)
			se401->buttonpressed = 1;
exit:
	status = usb_submit_urb(urb, GFP_ATOMIC);
	if (status)
		err("%s - usb_submit_urb failed with result %d",
		     __func__, status);
}

static void se401_video_irq(struct urb *urb)
{
	struct usb_se401 *se401 = urb->context;
	int length = urb->actual_length;

	
	if (!se401->streaming)
		return;

	if (!se401->dev) {
		dev_info(&urb->dev->dev, "device vapourished\n");
		return;
	}

	
	if (length && !urb->status) {
		se401->nullpackets = 0;
		switch (se401->scratch[se401->scratch_next].state) {
		case BUFFER_READY:
		case BUFFER_BUSY:
			se401->dropped++;
			break;
		case BUFFER_UNUSED:
			memcpy(se401->scratch[se401->scratch_next].data,
				(unsigned char *)urb->transfer_buffer, length);
			se401->scratch[se401->scratch_next].state
							= BUFFER_READY;
			se401->scratch[se401->scratch_next].offset
							= se401->bayeroffset;
			se401->scratch[se401->scratch_next].length = length;
			if (waitqueue_active(&se401->wq))
				wake_up_interruptible(&se401->wq);
			se401->scratch_overflow = 0;
			se401->scratch_next++;
			if (se401->scratch_next >= SE401_NUMSCRATCH)
				se401->scratch_next = 0;
			break;
		}
		se401->bayeroffset += length;
		if (se401->bayeroffset >= se401->cheight * se401->cwidth)
			se401->bayeroffset = 0;
	} else {
		se401->nullpackets++;
		if (se401->nullpackets > SE401_MAX_NULLPACKETS)
			if (waitqueue_active(&se401->wq))
				wake_up_interruptible(&se401->wq);
	}

	
	urb->status = 0;
	urb->dev = se401->dev;
	if (usb_submit_urb(urb, GFP_KERNEL))
		dev_info(&urb->dev->dev, "urb burned down\n");
	return;
}

static void se401_send_size(struct usb_se401 *se401, int width, int height)
{
	int i = 0;
	int mode = 0x03; 
	int sendheight = height;
	int sendwidth = width;

	
	while (i < se401->sizes && !(se401->width[i] == width &&
						se401->height[i] == height))
		i++;
	while (i < se401->sizes) {
		if (se401->width[i] == width * 2 &&
				se401->height[i] == height * 2) {
			sendheight = se401->height[i];
			sendwidth = se401->width[i];
			mode = 0x40;
		}
		if (se401->width[i] == width * 4 &&
				se401->height[i] == height * 4) {
			sendheight = se401->height[i];
			sendwidth = se401->width[i];
			mode = 0x42;
		}
		i++;
	}

	se401_sndctrl(1, se401, SE401_REQ_SET_WIDTH, sendwidth, NULL, 0);
	se401_sndctrl(1, se401, SE401_REQ_SET_HEIGHT, sendheight, NULL, 0);
	se401_set_feature(se401, SE401_OPERATINGMODE, mode);

	if (mode == 0x03)
		se401->format = FMT_BAYER;
	else
		se401->format = FMT_JANGGU;
}


static int se401_start_stream(struct usb_se401 *se401)
{
	struct urb *urb;
	int err = 0, i;
	se401->streaming = 1;

	se401_sndctrl(1, se401, SE401_REQ_CAMERA_POWER, 1, NULL, 0);
	se401_sndctrl(1, se401, SE401_REQ_LED_CONTROL, 1, NULL, 0);

	
	
	se401_set_feature(se401, HV7131_REG_MODE_B, 0x05);
	se401_send_pict(se401);

	se401_send_size(se401, se401->cwidth, se401->cheight);

	se401_sndctrl(1, se401, SE401_REQ_START_CONTINUOUS_CAPTURE,
								0, NULL, 0);

	
	for (i = 0; i < SE401_NUMFRAMES; i++) {
		se401->frame[i].data = se401->fbuf + i * se401->maxframesize;
		se401->frame[i].curpix = 0;
	}
	for (i = 0; i < SE401_NUMSBUF; i++) {
		se401->sbuf[i].data = kmalloc(SE401_PACKETSIZE, GFP_KERNEL);
		if (!se401->sbuf[i].data) {
			for (i = i - 1; i >= 0; i--) {
				kfree(se401->sbuf[i].data);
				se401->sbuf[i].data = NULL;
			}
			return -ENOMEM;
		}
	}

	se401->bayeroffset = 0;
	se401->scratch_next = 0;
	se401->scratch_use = 0;
	se401->scratch_overflow = 0;
	for (i = 0; i < SE401_NUMSCRATCH; i++) {
		se401->scratch[i].data = kmalloc(SE401_PACKETSIZE, GFP_KERNEL);
		if (!se401->scratch[i].data) {
			for (i = i - 1; i >= 0; i--) {
				kfree(se401->scratch[i].data);
				se401->scratch[i].data = NULL;
			}
			goto nomem_sbuf;
		}
		se401->scratch[i].state = BUFFER_UNUSED;
	}

	for (i = 0; i < SE401_NUMSBUF; i++) {
		urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!urb) {
			for (i = i - 1; i >= 0; i--) {
				usb_kill_urb(se401->urb[i]);
				usb_free_urb(se401->urb[i]);
				se401->urb[i] = NULL;
			}
			goto nomem_scratch;
		}

		usb_fill_bulk_urb(urb, se401->dev,
			usb_rcvbulkpipe(se401->dev, SE401_VIDEO_ENDPOINT),
			se401->sbuf[i].data, SE401_PACKETSIZE,
			se401_video_irq,
			se401);

		se401->urb[i] = urb;

		err = usb_submit_urb(se401->urb[i], GFP_KERNEL);
		if (err)
			err("urb burned down");
	}

	se401->framecount = 0;

	return 0;

 nomem_scratch:
	for (i = 0; i < SE401_NUMSCRATCH; i++) {
		kfree(se401->scratch[i].data);
		se401->scratch[i].data = NULL;
	}
 nomem_sbuf:
	for (i = 0; i < SE401_NUMSBUF; i++) {
		kfree(se401->sbuf[i].data);
		se401->sbuf[i].data = NULL;
	}
	return -ENOMEM;
}

static int se401_stop_stream(struct usb_se401 *se401)
{
	int i;

	if (!se401->streaming || !se401->dev)
		return 1;

	se401->streaming = 0;

	se401_sndctrl(1, se401, SE401_REQ_STOP_CONTINUOUS_CAPTURE, 0, NULL, 0);

	se401_sndctrl(1, se401, SE401_REQ_LED_CONTROL, 0, NULL, 0);
	se401_sndctrl(1, se401, SE401_REQ_CAMERA_POWER, 0, NULL, 0);

	for (i = 0; i < SE401_NUMSBUF; i++)
		if (se401->urb[i]) {
			usb_kill_urb(se401->urb[i]);
			usb_free_urb(se401->urb[i]);
			se401->urb[i] = NULL;
			kfree(se401->sbuf[i].data);
		}
	for (i = 0; i < SE401_NUMSCRATCH; i++) {
		kfree(se401->scratch[i].data);
		se401->scratch[i].data = NULL;
	}

	return 0;
}

static int se401_set_size(struct usb_se401 *se401, int width, int height)
{
	int wasstreaming = se401->streaming;
	
	if (se401->cwidth == width && se401->cheight == height)
		return 0;

	
	if (!width || !height)
		return 1;
	if ((width & 1) || (height & 1))
		return 1;
	if (width > se401->width[se401->sizes-1])
		return 1;
	if (height > se401->height[se401->sizes-1])
		return 1;

	
	if (wasstreaming)
		se401_stop_stream(se401);
	se401->cwidth = width;
	se401->cheight = height;
	if (wasstreaming)
		se401_start_stream(se401);
	return 0;
}





static inline void enhance_picture(unsigned char *frame, int len)
{
	while (len--) {
		*frame = (((*frame^255)*(*frame^255))/255)^255;
		frame++;
	}
}

static inline void decode_JangGu_integrate(struct usb_se401 *se401, int data)
{
	struct se401_frame *frame = &se401->frame[se401->curframe];
	int linelength = se401->cwidth * 3;

	if (frame->curlinepix >= linelength) {
		frame->curlinepix = 0;
		frame->curline += linelength;
	}

	
	if (frame->curlinepix < 3)
		*(frame->curline-frame->curlinepix) = 1 + data * 4;
	else
		*(frame->curline-frame->curlinepix) =
		    *(frame->curline-frame->curlinepix + 3) + data * 4;
	frame->curlinepix++;
}

static inline void decode_JangGu_vlc(struct usb_se401 *se401,
			unsigned char *data, int bit_exp, int packetlength)
{
	int pos = 0;
	int vlc_cod = 0;
	int vlc_size = 0;
	int vlc_data = 0;
	int bit_cur;
	int bit;
	data += 4;
	while (pos < packetlength) {
		bit_cur = 8;
		while (bit_cur && bit_exp) {
			bit = ((*data) >> (bit_cur-1))&1;
			if (!vlc_cod) {
				if (bit) {
					vlc_size++;
				} else {
					if (!vlc_size)
						decode_JangGu_integrate(se401, 0);
					else {
						vlc_cod = 2;
						vlc_data = 0;
					}
				}
			} else {
				if (vlc_cod == 2) {
					if (!bit)
						vlc_data =  -(1 << vlc_size) + 1;
					vlc_cod--;
				}
				vlc_size--;
				vlc_data += bit << vlc_size;
				if (!vlc_size) {
					decode_JangGu_integrate(se401, vlc_data);
					vlc_cod = 0;
				}
			}
			bit_cur--;
			bit_exp--;
		}
		pos++;
		data++;
	}
}

static inline void decode_JangGu(struct usb_se401 *se401,
						struct se401_scratch *buffer)
{
	unsigned char *data = buffer->data;
	int len = buffer->length;
	int bit_exp = 0, pix_exp = 0, frameinfo = 0, packetlength = 0, size;
	int datapos = 0;

	
	if (!se401->frame[se401->curframe].curpix) {
		se401->frame[se401->curframe].curlinepix = 0;
		se401->frame[se401->curframe].curline =
		    se401->frame[se401->curframe].data+
		    se401->cwidth * 3 - 1;
		if (se401->frame[se401->curframe].grabstate == FRAME_READY)
			se401->frame[se401->curframe].grabstate = FRAME_GRABBING;
		se401->vlcdatapos = 0;
	}
	while (datapos < len) {
		size = 1024 - se401->vlcdatapos;
		if (size+datapos > len)
			size = len-datapos;
		memcpy(se401->vlcdata+se401->vlcdatapos, data+datapos, size);
		se401->vlcdatapos += size;
		packetlength = 0;
		if (se401->vlcdatapos >= 4) {
			bit_exp = se401->vlcdata[3] + (se401->vlcdata[2] << 8);
			pix_exp = se401->vlcdata[1] +
					((se401->vlcdata[0] & 0x3f) << 8);
			frameinfo = se401->vlcdata[0] & 0xc0;
			packetlength = ((bit_exp + 47) >> 4) << 1;
			if (packetlength > 1024) {
				se401->vlcdatapos = 0;
				datapos = len;
				packetlength = 0;
				se401->error++;
				se401->frame[se401->curframe].curpix = 0;
			}
		}
		if (packetlength && se401->vlcdatapos >= packetlength) {
			decode_JangGu_vlc(se401, se401->vlcdata, bit_exp,
								packetlength);
			se401->frame[se401->curframe].curpix += pix_exp * 3;
			datapos += size-(se401->vlcdatapos-packetlength);
			se401->vlcdatapos = 0;
			if (se401->frame[se401->curframe].curpix >= se401->cwidth * se401->cheight * 3) {
				if (se401->frame[se401->curframe].curpix == se401->cwidth * se401->cheight * 3) {
					if (se401->frame[se401->curframe].grabstate == FRAME_GRABBING) {
						se401->frame[se401->curframe].grabstate = FRAME_DONE;
						se401->framecount++;
						se401->readcount++;
					}
					if (se401->frame[(se401->curframe + 1) & (SE401_NUMFRAMES - 1)].grabstate == FRAME_READY)
						se401->curframe = (se401->curframe + 1) & (SE401_NUMFRAMES - 1);
				} else
					se401->error++;
				se401->frame[se401->curframe].curpix = 0;
				datapos = len;
			}
		} else
			datapos += size;
	}
}

static inline void decode_bayer(struct usb_se401 *se401,
						struct se401_scratch *buffer)
{
	unsigned char *data = buffer->data;
	int len = buffer->length;
	int offset = buffer->offset;
	int datasize = se401->cwidth * se401->cheight;
	struct se401_frame *frame = &se401->frame[se401->curframe];
	unsigned char *framedata = frame->data, *curline, *nextline;
	int width = se401->cwidth;
	int blineoffset = 0, bline;
	int linelength = width * 3, i;


	if (frame->curpix == 0) {
		if (frame->grabstate == FRAME_READY)
			frame->grabstate = FRAME_GRABBING;

		frame->curline = framedata + linelength;
		frame->curlinepix = 0;
	}

	if (offset != frame->curpix) {
		
		frame->curpix = 0;
		se401->error++;
		return;
	}

	
	if (frame->curpix + len > datasize)
		len = datasize-frame->curpix;

	if (se401->cheight % 4)
		blineoffset = 1;
	bline = frame->curpix / se401->cwidth+blineoffset;

	curline = frame->curline;
	nextline = curline + linelength;
	if (nextline >= framedata+datasize * 3)
		nextline = curline;
	while (len) {
		if (frame->curlinepix >= width) {
			frame->curlinepix -= width;
			bline = frame->curpix / width + blineoffset;
			curline += linelength*2;
			nextline += linelength*2;
			if (curline >= framedata+datasize * 3) {
				frame->curlinepix++;
				curline -= 3;
				nextline -= 3;
				len--;
				data++;
				frame->curpix++;
			}
			if (nextline >= framedata+datasize*3)
				nextline = curline;
		}
		if (bline & 1) {
			if (frame->curlinepix & 1) {
				*(curline + 2) = *data;
				*(curline - 1) = *data;
				*(nextline + 2) = *data;
				*(nextline - 1) = *data;
			} else {
				*(curline + 1) =
					(*(curline + 1) + *data) / 2;
				*(curline-2) =
					(*(curline - 2) + *data) / 2;
				*(nextline + 1) = *data;
				*(nextline - 2) = *data;
			}
		} else {
			if (frame->curlinepix & 1) {
				*(curline + 1) =
					(*(curline + 1) + *data) / 2;
				*(curline - 2) =
					(*(curline - 2) + *data) / 2;
				*(nextline + 1) = *data;
				*(nextline - 2) = *data;
			} else {
				*curline = *data;
				*(curline - 3) = *data;
				*nextline = *data;
				*(nextline - 3) = *data;
			}
		}
		frame->curlinepix++;
		curline -= 3;
		nextline -= 3;
		len--;
		data++;
		frame->curpix++;
	}
	frame->curline = curline;

	if (frame->curpix >= datasize) {
		
		framedata += linelength;
		for (i = 0; i < linelength; i++) {
			framedata--;
			*framedata = *(framedata + linelength);
		}
		
		for (i = 0; i < se401->cheight; i++) {
			*framedata = *(framedata + 3);
			*(framedata + 1) = *(framedata + 4);
			*(framedata + 2) = *(framedata + 5);
			framedata += linelength;
		}
		frame->curpix = 0;
		frame->grabstate = FRAME_DONE;
		se401->framecount++;
		se401->readcount++;
		if (se401->frame[(se401->curframe + 1) &
		    (SE401_NUMFRAMES - 1)].grabstate == FRAME_READY) {
			se401->curframe = (se401->curframe+1) &
							(SE401_NUMFRAMES-1);
		}
	}
}

static int se401_newframe(struct usb_se401 *se401, int framenr)
{
	DECLARE_WAITQUEUE(wait, current);
	int errors = 0;

	while (se401->streaming &&
	    (se401->frame[framenr].grabstate == FRAME_READY ||
	     se401->frame[framenr].grabstate == FRAME_GRABBING)) {
		if (!se401->frame[framenr].curpix)
			errors++;

		wait_interruptible(
		    se401->scratch[se401->scratch_use].state != BUFFER_READY,
						    &se401->wq, &wait);
		if (se401->nullpackets > SE401_MAX_NULLPACKETS) {
			se401->nullpackets = 0;
			dev_info(&se401->dev->dev,
			 "too many null length packets, restarting capture\n");
			se401_stop_stream(se401);
			se401_start_stream(se401);
		} else {
			if (se401->scratch[se401->scratch_use].state !=
								BUFFER_READY) {
				se401->frame[framenr].grabstate = FRAME_ERROR;
				return -EIO;
			}
			se401->scratch[se401->scratch_use].state = BUFFER_BUSY;
			if (se401->format == FMT_JANGGU)
				decode_JangGu(se401,
					&se401->scratch[se401->scratch_use]);
			else
				decode_bayer(se401,
					&se401->scratch[se401->scratch_use]);

			se401->scratch[se401->scratch_use].state =
							BUFFER_UNUSED;
			se401->scratch_use++;
			if (se401->scratch_use >= SE401_NUMSCRATCH)
				se401->scratch_use = 0;
			if (errors > SE401_MAX_ERRORS) {
				errors = 0;
				dev_info(&se401->dev->dev,
				      "too many errors, restarting capture\n");
				se401_stop_stream(se401);
				se401_start_stream(se401);
			}
		}
	}

	if (se401->frame[framenr].grabstate == FRAME_DONE)
		if (se401->enhance)
			enhance_picture(se401->frame[framenr].data,
					se401->cheight * se401->cwidth * 3);
	return 0;
}

static void usb_se401_remove_disconnected(struct usb_se401 *se401)
{
	int i;

	se401->dev = NULL;

	for (i = 0; i < SE401_NUMSBUF; i++)
		if (se401->urb[i]) {
			usb_kill_urb(se401->urb[i]);
			usb_free_urb(se401->urb[i]);
			se401->urb[i] = NULL;
			kfree(se401->sbuf[i].data);
		}

	for (i = 0; i < SE401_NUMSCRATCH; i++)
		kfree(se401->scratch[i].data);

	if (se401->inturb) {
		usb_kill_urb(se401->inturb);
		usb_free_urb(se401->inturb);
	}
	dev_info(&se401->dev->dev, "%s disconnected", se401->camera_name);

	
	kfree(se401->width);
	kfree(se401->height);
	kfree(se401);
}






static int se401_open(struct file *file)
{
	struct video_device *dev = video_devdata(file);
	struct usb_se401 *se401 = (struct usb_se401 *)dev;
	int err = 0;

	lock_kernel();
	if (se401->user) {
		unlock_kernel();
		return -EBUSY;
	}
	se401->fbuf = rvmalloc(se401->maxframesize * SE401_NUMFRAMES);
	if (se401->fbuf)
		file->private_data = dev;
	else
		err = -ENOMEM;
	se401->user = !err;
	unlock_kernel();

	return err;
}

static int se401_close(struct file *file)
{
	struct video_device *dev = file->private_data;
	struct usb_se401 *se401 = (struct usb_se401 *)dev;
	int i;

	rvfree(se401->fbuf, se401->maxframesize * SE401_NUMFRAMES);
	if (se401->removed) {
		dev_info(&se401->dev->dev, "device unregistered\n");
		usb_se401_remove_disconnected(se401);
	} else {
		for (i = 0; i < SE401_NUMFRAMES; i++)
			se401->frame[i].grabstate = FRAME_UNUSED;
		if (se401->streaming)
			se401_stop_stream(se401);
		se401->user = 0;
	}
	file->private_data = NULL;
	return 0;
}

static long se401_do_ioctl(struct file *file, unsigned int cmd, void *arg)
{
	struct video_device *vdev = file->private_data;
	struct usb_se401 *se401 = (struct usb_se401 *)vdev;

	if (!se401->dev)
		return -EIO;

	switch (cmd) {
	case VIDIOCGCAP:
	{
		struct video_capability *b = arg;
		strcpy(b->name, se401->camera_name);
		b->type = VID_TYPE_CAPTURE;
		b->channels = 1;
		b->audios = 0;
		b->maxwidth = se401->width[se401->sizes-1];
		b->maxheight = se401->height[se401->sizes-1];
		b->minwidth = se401->width[0];
		b->minheight = se401->height[0];
		return 0;
	}
	case VIDIOCGCHAN:
	{
		struct video_channel *v = arg;

		if (v->channel != 0)
			return -EINVAL;
		v->flags = 0;
		v->tuners = 0;
		v->type = VIDEO_TYPE_CAMERA;
		strcpy(v->name, "Camera");
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
		struct video_picture *p = arg;

		se401_get_pict(se401, p);
		return 0;
	}
	case VIDIOCSPICT:
	{
		struct video_picture *p = arg;

		if (se401_set_pict(se401, p))
			return -EINVAL;
		return 0;
	}
	case VIDIOCSWIN:
	{
		struct video_window *vw = arg;

		if (vw->flags)
			return -EINVAL;
		if (vw->clipcount)
			return -EINVAL;
		if (se401_set_size(se401, vw->width, vw->height))
			return -EINVAL;
		return 0;
	}
	case VIDIOCGWIN:
	{
		struct video_window *vw = arg;

		vw->x = 0;               
		vw->y = 0;
		vw->chromakey = 0;
		vw->flags = 0;
		vw->clipcount = 0;
		vw->width = se401->cwidth;
		vw->height = se401->cheight;
		return 0;
	}
	case VIDIOCGMBUF:
	{
		struct video_mbuf *vm = arg;
		int i;

		memset(vm, 0, sizeof(*vm));
		vm->size = SE401_NUMFRAMES * se401->maxframesize;
		vm->frames = SE401_NUMFRAMES;
		for (i = 0; i < SE401_NUMFRAMES; i++)
			vm->offsets[i] = se401->maxframesize * i;
		return 0;
	}
	case VIDIOCMCAPTURE:
	{
		struct video_mmap *vm = arg;

		if (vm->format != VIDEO_PALETTE_RGB24)
			return -EINVAL;
		if (vm->frame >= SE401_NUMFRAMES)
			return -EINVAL;
		if (se401->frame[vm->frame].grabstate != FRAME_UNUSED)
			return -EBUSY;

		
		if (se401_set_size(se401, vm->width, vm->height))
			return -EINVAL;
		se401->frame[vm->frame].grabstate = FRAME_READY;

		if (!se401->streaming)
			se401_start_stream(se401);

		
		if (se401->framecount == 0)
			se401_send_pict(se401);
		
		if (se401->framecount % 20 == 1)
			se401_auto_resetlevel(se401);

		return 0;
	}
	case VIDIOCSYNC:
	{
		int *frame = arg;
		int ret = 0;

		if (*frame < 0 || *frame >= SE401_NUMFRAMES)
			return -EINVAL;

		ret = se401_newframe(se401, *frame);
		se401->frame[*frame].grabstate = FRAME_UNUSED;
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
		return -EINVAL;
	case VIDIOCGTUNER:
	case VIDIOCSTUNER:
		return -EINVAL;
	case VIDIOCGFREQ:
	case VIDIOCSFREQ:
		return -EINVAL;
	case VIDIOCGAUDIO:
	case VIDIOCSAUDIO:
		return -EINVAL;
	default:
		return -ENOIOCTLCMD;
	} 

	return 0;
}

static long se401_ioctl(struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	return video_usercopy(file, cmd, arg, se401_do_ioctl);
}

static ssize_t se401_read(struct file *file, char __user *buf,
		     size_t count, loff_t *ppos)
{
	int realcount = count, ret = 0;
	struct video_device *dev = file->private_data;
	struct usb_se401 *se401 = (struct usb_se401 *)dev;


	if (se401->dev ==  NULL)
		return -EIO;
	if (realcount > se401->cwidth*se401->cheight*3)
		realcount = se401->cwidth*se401->cheight*3;

	
	if (se401->frame[0].grabstate == FRAME_GRABBING)
		return -EBUSY;
	se401->frame[0].grabstate = FRAME_READY;
	se401->frame[1].grabstate = FRAME_UNUSED;
	se401->curframe = 0;

	if (!se401->streaming)
		se401_start_stream(se401);

	
	if (se401->framecount == 0)
		se401_send_pict(se401);
	
	if (se401->framecount%20 == 1)
		se401_auto_resetlevel(se401);

	ret = se401_newframe(se401, 0);

	se401->frame[0].grabstate = FRAME_UNUSED;
	if (ret)
		return ret;
	if (copy_to_user(buf, se401->frame[0].data, realcount))
		return -EFAULT;

	return realcount;
}

static int se401_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct video_device *dev = file->private_data;
	struct usb_se401 *se401 = (struct usb_se401 *)dev;
	unsigned long start = vma->vm_start;
	unsigned long size  = vma->vm_end-vma->vm_start;
	unsigned long page, pos;

	mutex_lock(&se401->lock);

	if (se401->dev ==  NULL) {
		mutex_unlock(&se401->lock);
		return -EIO;
	}
	if (size > (((SE401_NUMFRAMES * se401->maxframesize) + PAGE_SIZE - 1)
							& ~(PAGE_SIZE - 1))) {
		mutex_unlock(&se401->lock);
		return -EINVAL;
	}
	pos = (unsigned long)se401->fbuf;
	while (size > 0) {
		page = vmalloc_to_pfn((void *)pos);
		if (remap_pfn_range(vma, start, page, PAGE_SIZE, PAGE_SHARED)) {
			mutex_unlock(&se401->lock);
			return -EAGAIN;
		}
		start +=  PAGE_SIZE;
		pos +=  PAGE_SIZE;
		if (size > PAGE_SIZE)
			size -=  PAGE_SIZE;
		else
			size = 0;
	}
	mutex_unlock(&se401->lock);

	return 0;
}

static const struct v4l2_file_operations se401_fops = {
	.owner  = 	THIS_MODULE,
	.open =         se401_open,
	.release =      se401_close,
	.read =         se401_read,
	.mmap =         se401_mmap,
	.ioctl =        se401_ioctl,
};
static struct video_device se401_template = {
	.name =         "se401 USB camera",
	.fops =         &se401_fops,
	.release = video_device_release_empty,
};




static int se401_init(struct usb_se401 *se401, int button)
{
	int i = 0, rc;
	unsigned char cp[0x40];
	char temp[200];
	int slen;

	
	se401_sndctrl(1, se401, SE401_REQ_LED_CONTROL, 1, NULL, 0);

	
	rc = se401_sndctrl(0, se401, SE401_REQ_GET_CAMERA_DESCRIPTOR, 0,
							cp, sizeof(cp));
	if (cp[1] != 0x41) {
		err("Wrong descriptor type");
		return 1;
	}
	slen = snprintf(temp, 200, "ExtraFeatures: %d", cp[3]);

	se401->sizes = cp[4] + cp[5] * 256;
	se401->width = kmalloc(se401->sizes*sizeof(int), GFP_KERNEL);
	if (!se401->width)
		return 1;
	se401->height = kmalloc(se401->sizes*sizeof(int), GFP_KERNEL);
	if (!se401->height) {
		kfree(se401->width);
		return 1;
	}
	for (i = 0; i < se401->sizes; i++) {
		se401->width[i] = cp[6 + i * 4 + 0] + cp[6 + i*4 + 1] * 256;
		se401->height[i] = cp[6 + i * 4 + 2] + cp[6 + i * 4 + 3] * 256;
	}
	slen += snprintf(temp + slen, 200 - slen, " Sizes:");
	for (i = 0; i < se401->sizes; i++) {
		slen +=  snprintf(temp + slen, 200 - slen,
			" %dx%d", se401->width[i], se401->height[i]);
	}
	dev_info(&se401->dev->dev, "%s\n", temp);
	se401->maxframesize = se401->width[se401->sizes-1] *
					se401->height[se401->sizes - 1] * 3;

	rc = se401_sndctrl(0, se401, SE401_REQ_GET_WIDTH, 0, cp, sizeof(cp));
	se401->cwidth = cp[0]+cp[1]*256;
	rc = se401_sndctrl(0, se401, SE401_REQ_GET_HEIGHT, 0, cp, sizeof(cp));
	se401->cheight = cp[0]+cp[1]*256;

	if (!(cp[2] & SE401_FORMAT_BAYER)) {
		err("Bayer format not supported!");
		return 1;
	}
	
	se401_sndctrl(1, se401, SE401_REQ_SET_OUTPUT_MODE,
						SE401_FORMAT_BAYER, NULL, 0);

	rc = se401_sndctrl(0, se401, SE401_REQ_GET_BRT, 0, cp, sizeof(cp));
	se401->brightness = cp[0]+cp[1]*256;
	
	se401->resetlevel = 0x2d;
	se401->rgain = 0x20;
	se401->ggain = 0x20;
	se401->bgain = 0x20;
	se401_set_exposure(se401, 20000);
	se401->palette = VIDEO_PALETTE_RGB24;
	se401->enhance = 1;
	se401->dropped = 0;
	se401->error = 0;
	se401->framecount = 0;
	se401->readcount = 0;

	
	if (button) {
		se401->inturb = usb_alloc_urb(0, GFP_KERNEL);
		if (!se401->inturb) {
			dev_info(&se401->dev->dev,
				 "Allocation of inturb failed\n");
			return 1;
		}
		usb_fill_int_urb(se401->inturb, se401->dev,
		    usb_rcvintpipe(se401->dev, SE401_BUTTON_ENDPOINT),
		    &se401->button, sizeof(se401->button),
		    se401_button_irq,
		    se401,
		    8
		);
		if (usb_submit_urb(se401->inturb, GFP_KERNEL)) {
			dev_info(&se401->dev->dev, "int urb burned down\n");
			return 1;
		}
	} else
		se401->inturb = NULL;

	
	se401_sndctrl(1, se401, SE401_REQ_CAMERA_POWER, 1, NULL, 0);
	se401_sndctrl(1, se401, SE401_REQ_LED_CONTROL, 1, NULL, 0);
	se401_sndctrl(1, se401, SE401_REQ_CAMERA_POWER, 0, NULL, 0);
	se401_sndctrl(1, se401, SE401_REQ_LED_CONTROL, 0, NULL, 0);

	return 0;
}

static int se401_probe(struct usb_interface *intf,
	const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_interface_descriptor *interface;
	struct usb_se401 *se401;
	char *camera_name = NULL;
	int button = 1;

	
	if (dev->descriptor.bNumConfigurations != 1)
		return -ENODEV;

	interface = &intf->cur_altsetting->desc;

	
	if (le16_to_cpu(dev->descriptor.idVendor) ==  0x03e8 &&
	    le16_to_cpu(dev->descriptor.idProduct) ==  0x0004) {
		camera_name = "Endpoints/Aox SE401";
	} else if (le16_to_cpu(dev->descriptor.idVendor) ==  0x0471 &&
	    le16_to_cpu(dev->descriptor.idProduct) ==  0x030b) {
		camera_name = "Philips PCVC665K";
	} else if (le16_to_cpu(dev->descriptor.idVendor) ==  0x047d &&
	    le16_to_cpu(dev->descriptor.idProduct) ==  0x5001) {
		camera_name = "Kensington VideoCAM 67014";
	} else if (le16_to_cpu(dev->descriptor.idVendor) ==  0x047d &&
	    le16_to_cpu(dev->descriptor.idProduct) ==  0x5002) {
		camera_name = "Kensington VideoCAM 6701(5/7)";
	} else if (le16_to_cpu(dev->descriptor.idVendor) ==  0x047d &&
	    le16_to_cpu(dev->descriptor.idProduct) ==  0x5003) {
		camera_name = "Kensington VideoCAM 67016";
		button = 0;
	} else
		return -ENODEV;

	
	if (interface->bInterfaceClass != 0x00)
		return -ENODEV;
	if (interface->bInterfaceSubClass != 0x00)
		return -ENODEV;

	
	dev_info(&intf->dev, "SE401 camera found: %s\n", camera_name);

	se401 = kzalloc(sizeof(*se401), GFP_KERNEL);
	if (se401 ==  NULL) {
		err("couldn't kmalloc se401 struct");
		return -ENOMEM;
	}

	se401->dev = dev;
	se401->iface = interface->bInterfaceNumber;
	se401->camera_name = camera_name;

	dev_info(&intf->dev, "firmware version: %02x\n",
		 le16_to_cpu(dev->descriptor.bcdDevice) & 255);

	if (se401_init(se401, button)) {
		kfree(se401);
		return -EIO;
	}

	memcpy(&se401->vdev, &se401_template, sizeof(se401_template));
	memcpy(se401->vdev.name, se401->camera_name,
					strlen(se401->camera_name));
	init_waitqueue_head(&se401->wq);
	mutex_init(&se401->lock);
	wmb();

	if (video_register_device(&se401->vdev,
					VFL_TYPE_GRABBER, video_nr) < 0) {
		kfree(se401);
		err("video_register_device failed");
		return -EIO;
	}
	dev_info(&intf->dev, "registered new video device: video%d\n",
		 se401->vdev.num);

	usb_set_intfdata(intf, se401);
	return 0;
}

static void se401_disconnect(struct usb_interface *intf)
{
	struct usb_se401 *se401 = usb_get_intfdata(intf);

	usb_set_intfdata(intf, NULL);
	if (se401) {
		video_unregister_device(&se401->vdev);
		if (!se401->user)
			usb_se401_remove_disconnected(se401);
		else {
			se401->frame[0].grabstate = FRAME_ERROR;
			se401->frame[0].grabstate = FRAME_ERROR;

			se401->streaming = 0;

			wake_up_interruptible(&se401->wq);
			se401->removed = 1;
		}
	}
}

static struct usb_driver se401_driver = {
	.name		 =  "se401",
	.id_table	 =  device_table,
	.probe		 =  se401_probe,
	.disconnect	 =  se401_disconnect,
};





static int __init usb_se401_init(void)
{
	printk(KERN_INFO "SE401 usb camera driver version %s registering\n",
								version);
	if (flickerless)
		if (flickerless != 50 && flickerless != 60) {
			printk(KERN_ERR "Invallid flickerless value, use 0, 50 or 60.\n");
			return -1;
	}
	return usb_register(&se401_driver);
}

static void __exit usb_se401_exit(void)
{
	usb_deregister(&se401_driver);
	printk(KERN_INFO "SE401 driver deregistered\frame");
}

module_init(usb_se401_init);
module_exit(usb_se401_exit);
