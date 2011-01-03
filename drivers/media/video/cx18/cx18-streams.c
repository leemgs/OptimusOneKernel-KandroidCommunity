

#include "cx18-driver.h"
#include "cx18-io.h"
#include "cx18-fileops.h"
#include "cx18-mailbox.h"
#include "cx18-i2c.h"
#include "cx18-queue.h"
#include "cx18-ioctl.h"
#include "cx18-streams.h"
#include "cx18-cards.h"
#include "cx18-scb.h"
#include "cx18-dvb.h"

#define CX18_DSP0_INTERRUPT_MASK     	0xd0004C

static struct v4l2_file_operations cx18_v4l2_enc_fops = {
	.owner = THIS_MODULE,
	.read = cx18_v4l2_read,
	.open = cx18_v4l2_open,
	
	.ioctl = cx18_v4l2_ioctl,
	.release = cx18_v4l2_close,
	.poll = cx18_v4l2_enc_poll,
};


#define CX18_V4L2_ENC_TS_OFFSET   16

#define CX18_V4L2_ENC_PCM_OFFSET  24

#define CX18_V4L2_ENC_YUV_OFFSET  32

static struct {
	const char *name;
	int vfl_type;
	int num_offset;
	int dma;
	enum v4l2_buf_type buf_type;
} cx18_stream_info[] = {
	{	
		"encoder MPEG",
		VFL_TYPE_GRABBER, 0,
		PCI_DMA_FROMDEVICE, V4L2_BUF_TYPE_VIDEO_CAPTURE,
	},
	{	
		"TS",
		VFL_TYPE_GRABBER, -1,
		PCI_DMA_FROMDEVICE, V4L2_BUF_TYPE_VIDEO_CAPTURE,
	},
	{	
		"encoder YUV",
		VFL_TYPE_GRABBER, CX18_V4L2_ENC_YUV_OFFSET,
		PCI_DMA_FROMDEVICE, V4L2_BUF_TYPE_VIDEO_CAPTURE,
	},
	{	
		"encoder VBI",
		VFL_TYPE_VBI, 0,
		PCI_DMA_FROMDEVICE, V4L2_BUF_TYPE_VBI_CAPTURE,
	},
	{	
		"encoder PCM audio",
		VFL_TYPE_GRABBER, CX18_V4L2_ENC_PCM_OFFSET,
		PCI_DMA_FROMDEVICE, V4L2_BUF_TYPE_PRIVATE,
	},
	{	
		"encoder IDX",
		VFL_TYPE_GRABBER, -1,
		PCI_DMA_FROMDEVICE, V4L2_BUF_TYPE_VIDEO_CAPTURE,
	},
	{	
		"encoder radio",
		VFL_TYPE_RADIO, 0,
		PCI_DMA_NONE, V4L2_BUF_TYPE_PRIVATE,
	},
};

static void cx18_stream_init(struct cx18 *cx, int type)
{
	struct cx18_stream *s = &cx->streams[type];
	struct video_device *video_dev = s->video_dev;

	
	memset(s, 0, sizeof(*s));
	s->video_dev = video_dev;

	
	s->cx = cx;
	s->type = type;
	s->name = cx18_stream_info[type].name;
	s->handle = CX18_INVALID_TASK_HANDLE;

	s->dma = cx18_stream_info[type].dma;
	s->buffers = cx->stream_buffers[type];
	s->buf_size = cx->stream_buf_size[type];

	init_waitqueue_head(&s->waitq);
	s->id = -1;
	spin_lock_init(&s->q_free.lock);
	cx18_queue_init(&s->q_free);
	spin_lock_init(&s->q_busy.lock);
	cx18_queue_init(&s->q_busy);
	spin_lock_init(&s->q_full.lock);
	cx18_queue_init(&s->q_full);

	INIT_WORK(&s->out_work_order, cx18_out_work_handler);
}

static int cx18_prep_dev(struct cx18 *cx, int type)
{
	struct cx18_stream *s = &cx->streams[type];
	u32 cap = cx->v4l2_cap;
	int num_offset = cx18_stream_info[type].num_offset;
	int num = cx->instance + cx18_first_minor + num_offset;

	
	s->video_dev = NULL;
	s->cx = cx;
	s->type = type;
	s->name = cx18_stream_info[type].name;

	
	if (type == CX18_ENC_STREAM_TYPE_RAD && !(cap & V4L2_CAP_RADIO))
		return 0;

	
	if (type == CX18_ENC_STREAM_TYPE_VBI &&
	    !(cap & (V4L2_CAP_VBI_CAPTURE | V4L2_CAP_SLICED_VBI_CAPTURE)))
		return 0;

	
	if (cx18_stream_info[type].dma != PCI_DMA_NONE &&
	    cx->stream_buffers[type] == 0) {
		CX18_INFO("Disabled %s device\n", cx18_stream_info[type].name);
		return 0;
	}

	cx18_stream_init(cx, type);

	if (num_offset == -1)
		return 0;

	
	s->video_dev = video_device_alloc();
	if (s->video_dev == NULL) {
		CX18_ERR("Couldn't allocate v4l2 video_device for %s\n",
				s->name);
		return -ENOMEM;
	}

	snprintf(s->video_dev->name, sizeof(s->video_dev->name), "%s %s",
		 cx->v4l2_dev.name, s->name);

	s->video_dev->num = num;
	s->video_dev->v4l2_dev = &cx->v4l2_dev;
	s->video_dev->fops = &cx18_v4l2_enc_fops;
	s->video_dev->release = video_device_release;
	s->video_dev->tvnorms = V4L2_STD_ALL;
	cx18_set_funcs(s->video_dev);
	return 0;
}


int cx18_streams_setup(struct cx18 *cx)
{
	int type, ret;

	
	for (type = 0; type < CX18_MAX_STREAMS; type++) {
		
		ret = cx18_prep_dev(cx, type);
		if (ret < 0)
			break;

		
		ret = cx18_stream_alloc(&cx->streams[type]);
		if (ret < 0)
			break;
	}
	if (type == CX18_MAX_STREAMS)
		return 0;

	
	cx18_streams_cleanup(cx, 0);
	return ret;
}

static int cx18_reg_dev(struct cx18 *cx, int type)
{
	struct cx18_stream *s = &cx->streams[type];
	int vfl_type = cx18_stream_info[type].vfl_type;
	int num, ret;

	
	if (strcmp("TS", s->name) == 0) {
		
		if ((cx->card->hw_all & CX18_HW_DVB) == 0)
			return 0;
		ret = cx18_dvb_register(s);
		if (ret < 0) {
			CX18_ERR("DVB failed to register\n");
			return ret;
		}
	}

	if (s->video_dev == NULL)
		return 0;

	num = s->video_dev->num;
	
	if (type != CX18_ENC_STREAM_TYPE_MPG) {
		struct cx18_stream *s_mpg = &cx->streams[CX18_ENC_STREAM_TYPE_MPG];

		if (s_mpg->video_dev)
			num = s_mpg->video_dev->num
			    + cx18_stream_info[type].num_offset;
	}
	video_set_drvdata(s->video_dev, s);

	
	ret = video_register_device_no_warn(s->video_dev, vfl_type, num);
	if (ret < 0) {
		CX18_ERR("Couldn't register v4l2 device for %s (device node number %d)\n",
			s->name, num);
		video_device_release(s->video_dev);
		s->video_dev = NULL;
		return ret;
	}
	num = s->video_dev->num;

	switch (vfl_type) {
	case VFL_TYPE_GRABBER:
		CX18_INFO("Registered device video%d for %s (%d x %d kB)\n",
			  num, s->name, cx->stream_buffers[type],
			  cx->stream_buf_size[type]/1024);
		break;

	case VFL_TYPE_RADIO:
		CX18_INFO("Registered device radio%d for %s\n",
			num, s->name);
		break;

	case VFL_TYPE_VBI:
		if (cx->stream_buffers[type])
			CX18_INFO("Registered device vbi%d for %s "
				  "(%d x %d bytes)\n",
				  num, s->name, cx->stream_buffers[type],
				  cx->stream_buf_size[type]);
		else
			CX18_INFO("Registered device vbi%d for %s\n",
				num, s->name);
		break;
	}

	return 0;
}


int cx18_streams_register(struct cx18 *cx)
{
	int type;
	int err;
	int ret = 0;

	
	for (type = 0; type < CX18_MAX_STREAMS; type++) {
		err = cx18_reg_dev(cx, type);
		if (err && ret == 0)
			ret = err;
	}

	if (ret == 0)
		return 0;

	
	cx18_streams_cleanup(cx, 1);
	return ret;
}


void cx18_streams_cleanup(struct cx18 *cx, int unregister)
{
	struct video_device *vdev;
	int type;

	
	for (type = 0; type < CX18_MAX_STREAMS; type++) {
		if (cx->streams[type].dvb.enabled) {
			cx18_dvb_unregister(&cx->streams[type]);
			cx->streams[type].dvb.enabled = false;
		}

		vdev = cx->streams[type].video_dev;

		cx->streams[type].video_dev = NULL;
		if (vdev == NULL)
			continue;

		cx18_stream_free(&cx->streams[type]);

		
		if (unregister)
			video_unregister_device(vdev);
		else
			video_device_release(vdev);
	}
}

static void cx18_vbi_setup(struct cx18_stream *s)
{
	struct cx18 *cx = s->cx;
	int raw = cx18_raw_vbi(cx);
	u32 data[CX2341X_MBOX_MAX_DATA];
	int lines;

	if (cx->is_60hz) {
		cx->vbi.count = 12;
		cx->vbi.start[0] = 10;
		cx->vbi.start[1] = 273;
	} else {        
		cx->vbi.count = 18;
		cx->vbi.start[0] = 6;
		cx->vbi.start[1] = 318;
	}

	
	v4l2_subdev_call(cx->sd_av, video, s_fmt, &cx->vbi.in);

	
	if (raw) {
		lines = cx->vbi.count * 2;
	} else {
		
		lines = cx->is_60hz ? (21 - 4 + 1) * 2 : (23 - 2 + 1) * 2;
	}

	data[0] = s->handle;
	
	data[1] = (lines / 2) | ((lines / 2) << 16);
	
	data[2] = (raw ? vbi_active_samples
		       : (cx->is_60hz ? vbi_hblank_samples_60Hz
				      : vbi_hblank_samples_50Hz));
	
	data[3] = 1;
	
	if (raw) {
		
		data[4] = 0x20602060;
		
		data[5] = 0x307090d0;
	} else {
		
		data[4] = 0xB0F0B0F0;
		
		data[5] = 0xA0E0A0E0;
	}

	CX18_DEBUG_INFO("Setup VBI h: %d lines %x bpl %d fr %d %x %x\n",
			data[0], data[1], data[2], data[3], data[4], data[5]);

	cx18_api(cx, CX18_CPU_SET_RAW_VBI_PARAM, 6, data);
}

static
struct cx18_queue *_cx18_stream_put_buf_fw(struct cx18_stream *s,
					   struct cx18_buffer *buf)
{
	struct cx18 *cx = s->cx;
	struct cx18_queue *q;

	
	if (s->handle == CX18_INVALID_TASK_HANDLE ||
	    test_bit(CX18_F_S_STOPPING, &s->s_flags) ||
	    !test_bit(CX18_F_S_STREAMING, &s->s_flags))
		return cx18_enqueue(s, buf, &s->q_free);

	q = cx18_enqueue(s, buf, &s->q_busy);
	if (q != &s->q_busy)
		return q; 

	cx18_buf_sync_for_device(s, buf);
	cx18_vapi(cx, CX18_CPU_DE_SET_MDL, 5, s->handle,
		  (void __iomem *) &cx->scb->cpu_mdl[buf->id] - cx->enc_mem,
		  1, buf->id, s->buf_size);
	return q;
}

static
void _cx18_stream_load_fw_queue(struct cx18_stream *s)
{
	struct cx18_queue *q;
	struct cx18_buffer *buf;

	if (atomic_read(&s->q_free.buffers) == 0 ||
	    atomic_read(&s->q_busy.buffers) >= CX18_MAX_FW_MDLS_PER_STREAM)
		return;

	
	do {
		buf = cx18_dequeue(s, &s->q_free);
		if (buf == NULL)
			break;
		q = _cx18_stream_put_buf_fw(s, buf);
	} while (atomic_read(&s->q_busy.buffers) < CX18_MAX_FW_MDLS_PER_STREAM
		 && q == &s->q_busy);
}

void cx18_out_work_handler(struct work_struct *work)
{
	struct cx18_stream *s =
			 container_of(work, struct cx18_stream, out_work_order);

	_cx18_stream_load_fw_queue(s);
}

int cx18_start_v4l2_encode_stream(struct cx18_stream *s)
{
	u32 data[MAX_MB_ARGUMENTS];
	struct cx18 *cx = s->cx;
	struct cx18_buffer *buf;
	int captype = 0;
	struct cx18_api_func_private priv;

	if (s->video_dev == NULL && s->dvb.enabled == 0)
		return -EINVAL;

	CX18_DEBUG_INFO("Start encoder stream %s\n", s->name);

	switch (s->type) {
	case CX18_ENC_STREAM_TYPE_MPG:
		captype = CAPTURE_CHANNEL_TYPE_MPEG;
		cx->mpg_data_received = cx->vbi_data_inserted = 0;
		cx->dualwatch_jiffies = jiffies;
		cx->dualwatch_stereo_mode = cx->params.audio_properties & 0x300;
		cx->search_pack_header = 0;
		break;

	case CX18_ENC_STREAM_TYPE_TS:
		captype = CAPTURE_CHANNEL_TYPE_TS;
		break;
	case CX18_ENC_STREAM_TYPE_YUV:
		captype = CAPTURE_CHANNEL_TYPE_YUV;
		break;
	case CX18_ENC_STREAM_TYPE_PCM:
		captype = CAPTURE_CHANNEL_TYPE_PCM;
		break;
	case CX18_ENC_STREAM_TYPE_VBI:
#ifdef CX18_ENCODER_PARSES_SLICED
		captype = cx18_raw_vbi(cx) ?
		     CAPTURE_CHANNEL_TYPE_VBI : CAPTURE_CHANNEL_TYPE_SLICED_VBI;
#else
		
		captype = CAPTURE_CHANNEL_TYPE_VBI;
#endif
		cx->vbi.frame = 0;
		cx->vbi.inserted_frame = 0;
		memset(cx->vbi.sliced_mpeg_size,
			0, sizeof(cx->vbi.sliced_mpeg_size));
		break;
	default:
		return -EINVAL;
	}

	
	clear_bit(CX18_F_S_STREAMOFF, &s->s_flags);

	cx18_vapi_result(cx, data, CX18_CREATE_TASK, 1, CPU_CMD_MASK_CAPTURE);
	s->handle = data[0];
	cx18_vapi(cx, CX18_CPU_SET_CHANNEL_TYPE, 2, s->handle, captype);

	
	if (captype != CAPTURE_CHANNEL_TYPE_TS) {
		cx18_vapi(cx, CX18_CPU_SET_VER_CROP_LINE, 2, s->handle, 0);
		cx18_vapi(cx, CX18_CPU_SET_MISC_PARAMETERS, 3, s->handle, 3, 1);
		cx18_vapi(cx, CX18_CPU_SET_MISC_PARAMETERS, 3, s->handle, 8, 0);
		cx18_vapi(cx, CX18_CPU_SET_MISC_PARAMETERS, 3, s->handle, 4, 1);

		
		if (atomic_read(&cx->ana_capturing) == 0)
			cx18_vapi(cx, CX18_CPU_SET_MISC_PARAMETERS, 2,
				  s->handle, 12);

		
		cx18_vapi(cx, CX18_CPU_SET_CAPTURE_LINE_NO, 3,
			  s->handle, 312, 313);

		if (cx->v4l2_cap & V4L2_CAP_VBI_CAPTURE)
			cx18_vbi_setup(s);

		
		cx18_vapi_result(cx, data, CX18_CPU_SET_INDEXTABLE, 1, 0);

		
		priv.cx = cx;
		priv.s = s;
		cx2341x_update(&priv, cx18_api_func, NULL, &cx->params);

		
		if (!cx->params.video_mute &&
		    test_bit(CX18_F_I_RADIO_USER, &cx->i_flags))
			cx18_vapi(cx, CX18_CPU_SET_VIDEO_MUTE, 2, s->handle,
				  (cx->params.video_mute_yuv << 8) | 1);
	}

	if (atomic_read(&cx->tot_capturing) == 0) {
		clear_bit(CX18_F_I_EOS, &cx->i_flags);
		cx18_write_reg(cx, 7, CX18_DSP0_INTERRUPT_MASK);
	}

	cx18_vapi(cx, CX18_CPU_DE_SET_MDL_ACK, 3, s->handle,
		(void __iomem *)&cx->scb->cpu_mdl_ack[s->type][0] - cx->enc_mem,
		(void __iomem *)&cx->scb->cpu_mdl_ack[s->type][1] - cx->enc_mem);

	
	cx18_flush_queues(s);
	spin_lock(&s->q_free.lock);
	list_for_each_entry(buf, &s->q_free.list, list) {
		cx18_writel(cx, buf->dma_handle,
					&cx->scb->cpu_mdl[buf->id].paddr);
		cx18_writel(cx, s->buf_size, &cx->scb->cpu_mdl[buf->id].length);
	}
	spin_unlock(&s->q_free.lock);
	_cx18_stream_load_fw_queue(s);

	
	if (cx18_vapi(cx, CX18_CPU_CAPTURE_START, 1, s->handle)) {
		CX18_DEBUG_WARN("Error starting capture!\n");
		
		set_bit(CX18_F_S_STOPPING, &s->s_flags);
		if (s->type == CX18_ENC_STREAM_TYPE_MPG)
			cx18_vapi(cx, CX18_CPU_CAPTURE_STOP, 2, s->handle, 1);
		else
			cx18_vapi(cx, CX18_CPU_CAPTURE_STOP, 1, s->handle);
		clear_bit(CX18_F_S_STREAMING, &s->s_flags);
		
		cx18_vapi(cx, CX18_CPU_DE_RELEASE_MDL, 1, s->handle);
		cx18_vapi(cx, CX18_DESTROY_TASK, 1, s->handle);
		s->handle = CX18_INVALID_TASK_HANDLE;
		clear_bit(CX18_F_S_STOPPING, &s->s_flags);
		if (atomic_read(&cx->tot_capturing) == 0) {
			set_bit(CX18_F_I_EOS, &cx->i_flags);
			cx18_write_reg(cx, 5, CX18_DSP0_INTERRUPT_MASK);
		}
		return -EINVAL;
	}

	
	if (captype != CAPTURE_CHANNEL_TYPE_TS)
		atomic_inc(&cx->ana_capturing);
	atomic_inc(&cx->tot_capturing);
	return 0;
}

void cx18_stop_all_captures(struct cx18 *cx)
{
	int i;

	for (i = CX18_MAX_STREAMS - 1; i >= 0; i--) {
		struct cx18_stream *s = &cx->streams[i];

		if (s->video_dev == NULL && s->dvb.enabled == 0)
			continue;
		if (test_bit(CX18_F_S_STREAMING, &s->s_flags))
			cx18_stop_v4l2_encode_stream(s, 0);
	}
}

int cx18_stop_v4l2_encode_stream(struct cx18_stream *s, int gop_end)
{
	struct cx18 *cx = s->cx;
	unsigned long then;

	if (s->video_dev == NULL && s->dvb.enabled == 0)
		return -EINVAL;

	

	CX18_DEBUG_INFO("Stop Capture\n");

	if (atomic_read(&cx->tot_capturing) == 0)
		return 0;

	set_bit(CX18_F_S_STOPPING, &s->s_flags);
	if (s->type == CX18_ENC_STREAM_TYPE_MPG)
		cx18_vapi(cx, CX18_CPU_CAPTURE_STOP, 2, s->handle, !gop_end);
	else
		cx18_vapi(cx, CX18_CPU_CAPTURE_STOP, 1, s->handle);

	then = jiffies;

	if (s->type == CX18_ENC_STREAM_TYPE_MPG && gop_end) {
		CX18_INFO("ignoring gop_end: not (yet?) supported by the firmware\n");
	}

	if (s->type != CX18_ENC_STREAM_TYPE_TS)
		atomic_dec(&cx->ana_capturing);
	atomic_dec(&cx->tot_capturing);

	
	clear_bit(CX18_F_S_STREAMING, &s->s_flags);

	
	cx18_vapi(cx, CX18_CPU_DE_RELEASE_MDL, 1, s->handle);

	cx18_vapi(cx, CX18_DESTROY_TASK, 1, s->handle);
	s->handle = CX18_INVALID_TASK_HANDLE;
	clear_bit(CX18_F_S_STOPPING, &s->s_flags);

	if (atomic_read(&cx->tot_capturing) > 0)
		return 0;

	cx18_write_reg(cx, 5, CX18_DSP0_INTERRUPT_MASK);
	wake_up(&s->waitq);

	return 0;
}

u32 cx18_find_handle(struct cx18 *cx)
{
	int i;

	
	for (i = 0; i < CX18_MAX_STREAMS; i++) {
		struct cx18_stream *s = &cx->streams[i];

		if (s->video_dev && (s->handle != CX18_INVALID_TASK_HANDLE))
			return s->handle;
	}
	return CX18_INVALID_TASK_HANDLE;
}

struct cx18_stream *cx18_handle_to_stream(struct cx18 *cx, u32 handle)
{
	int i;
	struct cx18_stream *s;

	if (handle == CX18_INVALID_TASK_HANDLE)
		return NULL;

	for (i = 0; i < CX18_MAX_STREAMS; i++) {
		s = &cx->streams[i];
		if (s->handle != handle)
			continue;
		if (s->video_dev || s->dvb.enabled)
			return s;
	}
	return NULL;
}
