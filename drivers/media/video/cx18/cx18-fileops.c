

#include "cx18-driver.h"
#include "cx18-fileops.h"
#include "cx18-i2c.h"
#include "cx18-queue.h"
#include "cx18-vbi.h"
#include "cx18-audio.h"
#include "cx18-mailbox.h"
#include "cx18-scb.h"
#include "cx18-streams.h"
#include "cx18-controls.h"
#include "cx18-ioctl.h"
#include "cx18-cards.h"


static int cx18_claim_stream(struct cx18_open_id *id, int type)
{
	struct cx18 *cx = id->cx;
	struct cx18_stream *s = &cx->streams[type];
	struct cx18_stream *s_vbi;
	int vbi_type;

	if (test_and_set_bit(CX18_F_S_CLAIMED, &s->s_flags)) {
		
		if (s->id == id->open_id) {
			
			return 0;
		}
		if (s->id == -1 && type == CX18_ENC_STREAM_TYPE_VBI) {
			
			s->id = id->open_id;
			CX18_DEBUG_INFO("Start Read VBI\n");
			return 0;
		}
		
		CX18_DEBUG_INFO("Stream %d is busy\n", type);
		return -EBUSY;
	}
	s->id = id->open_id;

	
	if (type == CX18_ENC_STREAM_TYPE_MPG &&
	    cx->vbi.insert_mpeg && !cx18_raw_vbi(cx)) {
		vbi_type = CX18_ENC_STREAM_TYPE_VBI;
	} else {
		return 0;
	}
	s_vbi = &cx->streams[vbi_type];

	set_bit(CX18_F_S_CLAIMED, &s_vbi->s_flags);

	
	set_bit(CX18_F_S_INTERNAL_USE, &s_vbi->s_flags);
	return 0;
}


static void cx18_release_stream(struct cx18_stream *s)
{
	struct cx18 *cx = s->cx;
	struct cx18_stream *s_vbi;

	s->id = -1;
	if (s->type == CX18_ENC_STREAM_TYPE_VBI &&
		test_bit(CX18_F_S_INTERNAL_USE, &s->s_flags)) {
		
		return;
	}
	if (!test_and_clear_bit(CX18_F_S_CLAIMED, &s->s_flags)) {
		CX18_DEBUG_WARN("Release stream %s not in use!\n", s->name);
		return;
	}

	cx18_flush_queues(s);

	
	if (s->type == CX18_ENC_STREAM_TYPE_MPG)
		s_vbi = &cx->streams[CX18_ENC_STREAM_TYPE_VBI];
	else
		return;

	
	if (!test_and_clear_bit(CX18_F_S_INTERNAL_USE, &s_vbi->s_flags)) {
		
		return;
	}
	if (s_vbi->id != -1) {
		
		return;
	}
	clear_bit(CX18_F_S_CLAIMED, &s_vbi->s_flags);
	cx18_flush_queues(s_vbi);
}

static void cx18_dualwatch(struct cx18 *cx)
{
	struct v4l2_tuner vt;
	u32 new_bitmap;
	u32 new_stereo_mode;
	const u32 stereo_mask = 0x0300;
	const u32 dual = 0x0200;
	u32 h;

	new_stereo_mode = cx->params.audio_properties & stereo_mask;
	memset(&vt, 0, sizeof(vt));
	cx18_call_all(cx, tuner, g_tuner, &vt);
	if (vt.audmode == V4L2_TUNER_MODE_LANG1_LANG2 &&
			(vt.rxsubchans & V4L2_TUNER_SUB_LANG2))
		new_stereo_mode = dual;

	if (new_stereo_mode == cx->dualwatch_stereo_mode)
		return;

	new_bitmap = new_stereo_mode
			| (cx->params.audio_properties & ~stereo_mask);

	CX18_DEBUG_INFO("dualwatch: change stereo flag from 0x%x to 0x%x. "
			"new audio_bitmask=0x%ux\n",
			cx->dualwatch_stereo_mode, new_stereo_mode, new_bitmap);

	h = cx18_find_handle(cx);
	if (h == CX18_INVALID_TASK_HANDLE) {
		CX18_DEBUG_INFO("dualwatch: can't find valid task handle\n");
		return;
	}

	if (cx18_vapi(cx,
		      CX18_CPU_SET_AUDIO_PARAMETERS, 2, h, new_bitmap) == 0) {
		cx->dualwatch_stereo_mode = new_stereo_mode;
		return;
	}
	CX18_DEBUG_INFO("dualwatch: changing stereo flag failed\n");
}


static struct cx18_buffer *cx18_get_buffer(struct cx18_stream *s, int non_block, int *err)
{
	struct cx18 *cx = s->cx;
	struct cx18_stream *s_vbi = &cx->streams[CX18_ENC_STREAM_TYPE_VBI];
	struct cx18_buffer *buf;
	DEFINE_WAIT(wait);

	*err = 0;
	while (1) {
		if (s->type == CX18_ENC_STREAM_TYPE_MPG) {
			

			if (time_after(jiffies, cx->dualwatch_jiffies + msecs_to_jiffies(1000))) {
				cx->dualwatch_jiffies = jiffies;
				cx18_dualwatch(cx);
			}
			if (test_bit(CX18_F_S_INTERNAL_USE, &s_vbi->s_flags) &&
			    !test_bit(CX18_F_S_APPL_IO, &s_vbi->s_flags)) {
				while ((buf = cx18_dequeue(s_vbi, &s_vbi->q_full))) {
					
					cx18_process_vbi_data(cx, buf,
							      s_vbi->type);
					cx18_stream_put_buf_fw(s_vbi, buf);
				}
			}
			buf = &cx->vbi.sliced_mpeg_buf;
			if (buf->readpos != buf->bytesused)
				return buf;
		}

		
		buf = cx18_dequeue(s, &s->q_full);
		if (buf) {
			if (!test_and_clear_bit(CX18_F_B_NEED_BUF_SWAP,
						&buf->b_flags))
				return buf;
			if (s->type == CX18_ENC_STREAM_TYPE_MPG)
				
				cx18_buf_swap(buf);
			else {
				
				cx18_process_vbi_data(cx, buf, s->type);
			}
			return buf;
		}

		
		if (!test_bit(CX18_F_S_STREAMING, &s->s_flags)) {
			CX18_DEBUG_INFO("EOS %s\n", s->name);
			return NULL;
		}

		
		if (non_block) {
			*err = -EAGAIN;
			return NULL;
		}

		
		prepare_to_wait(&s->waitq, &wait, TASK_INTERRUPTIBLE);
		
		if (!atomic_read(&s->q_full.buffers))
			schedule();
		finish_wait(&s->waitq, &wait);
		if (signal_pending(current)) {
			
			CX18_DEBUG_INFO("User stopped %s\n", s->name);
			*err = -EINTR;
			return NULL;
		}
	}
}

static void cx18_setup_sliced_vbi_buf(struct cx18 *cx)
{
	int idx = cx->vbi.inserted_frame % CX18_VBI_FRAMES;

	cx->vbi.sliced_mpeg_buf.buf = cx->vbi.sliced_mpeg_data[idx];
	cx->vbi.sliced_mpeg_buf.bytesused = cx->vbi.sliced_mpeg_size[idx];
	cx->vbi.sliced_mpeg_buf.readpos = 0;
}

static size_t cx18_copy_buf_to_user(struct cx18_stream *s,
		struct cx18_buffer *buf, char __user *ubuf, size_t ucount)
{
	struct cx18 *cx = s->cx;
	size_t len = buf->bytesused - buf->readpos;

	if (len > ucount)
		len = ucount;
	if (cx->vbi.insert_mpeg && s->type == CX18_ENC_STREAM_TYPE_MPG &&
	    !cx18_raw_vbi(cx) && buf != &cx->vbi.sliced_mpeg_buf) {
		
		
		const char *start = buf->buf + buf->readpos;
		const char *p = start + 1;
		const u8 *q;
		u8 ch = cx->search_pack_header ? 0xba : 0xe0;
		int stuffing, i;

		while (start + len > p) {
			
			q = memchr(p, 0, start + len - p);
			if (q == NULL)
				break;
			p = q + 1;
			
			if ((char *)q + 15 >= buf->buf + buf->bytesused ||
			    q[1] != 0 || q[2] != 1 || q[3] != ch)
				continue;

			
			if (!cx->search_pack_header) {
				
				if ((q[6] & 0xc0) != 0x80)
					continue;
				
				if (((q[7] & 0xc0) == 0x80 &&  
				     (q[9] & 0xf0) == 0x20) || 
				    ((q[7] & 0xc0) == 0xc0 &&  
				     (q[9] & 0xf0) == 0x30)) { 
					
					ch = 0xba; 
					cx->search_pack_header = 1;
					p = q + 9; 
				}
				continue;
			}

			

			
			stuffing = q[13] & 7;
			
			for (i = 0; i < stuffing; i++)
				if (q[14 + i] != 0xff)
					break;
			if (i == stuffing && 
			    (q[4] & 0xc4) == 0x44 && 
			    (q[12] & 3) == 3 &&  
			    q[14 + stuffing] == 0 && 
			    q[15 + stuffing] == 0 &&
			    q[16 + stuffing] == 1) {
				
				cx->search_pack_header = 0; 
				len = (char *)q - start;
				cx18_setup_sliced_vbi_buf(cx);
				break;
			}
		}
	}
	if (copy_to_user(ubuf, (u8 *)buf->buf + buf->readpos, len)) {
		CX18_DEBUG_WARN("copy %zd bytes to user failed for %s\n",
				len, s->name);
		return -EFAULT;
	}
	buf->readpos += len;
	if (s->type == CX18_ENC_STREAM_TYPE_MPG &&
	    buf != &cx->vbi.sliced_mpeg_buf)
		cx->mpg_data_received += len;
	return len;
}

static ssize_t cx18_read(struct cx18_stream *s, char __user *ubuf,
		size_t tot_count, int non_block)
{
	struct cx18 *cx = s->cx;
	size_t tot_written = 0;
	int single_frame = 0;

	if (atomic_read(&cx->ana_capturing) == 0 && s->id == -1) {
		
		CX18_DEBUG_WARN("Stream %s not initialized before read\n",
				s->name);
		return -EIO;
	}

	
	if (s->type == CX18_ENC_STREAM_TYPE_VBI && !cx18_raw_vbi(cx))
		single_frame = 1;

	for (;;) {
		struct cx18_buffer *buf;
		int rc;

		buf = cx18_get_buffer(s, non_block, &rc);
		
		if (buf == NULL) {
			
			if (tot_written)
				break;
			
			if (rc == 0) {
				clear_bit(CX18_F_S_STREAMOFF, &s->s_flags);
				clear_bit(CX18_F_S_APPL_IO, &s->s_flags);
				cx18_release_stream(s);
			}
			
			return rc;
		}

		rc = cx18_copy_buf_to_user(s, buf, ubuf + tot_written,
				tot_count - tot_written);

		if (buf != &cx->vbi.sliced_mpeg_buf) {
			if (buf->readpos == buf->bytesused)
				cx18_stream_put_buf_fw(s, buf);
			else
				cx18_push(s, buf, &s->q_full);
		} else if (buf->readpos == buf->bytesused) {
			int idx = cx->vbi.inserted_frame % CX18_VBI_FRAMES;

			cx->vbi.sliced_mpeg_size[idx] = 0;
			cx->vbi.inserted_frame++;
			cx->vbi_data_inserted += buf->bytesused;
		}
		if (rc < 0)
			return rc;
		tot_written += rc;

		if (tot_written == tot_count || single_frame)
			break;
	}
	return tot_written;
}

static ssize_t cx18_read_pos(struct cx18_stream *s, char __user *ubuf,
		size_t count, loff_t *pos, int non_block)
{
	ssize_t rc = count ? cx18_read(s, ubuf, count, non_block) : 0;
	struct cx18 *cx = s->cx;

	CX18_DEBUG_HI_FILE("read %zd from %s, got %zd\n", count, s->name, rc);
	if (rc > 0)
		pos += rc;
	return rc;
}

int cx18_start_capture(struct cx18_open_id *id)
{
	struct cx18 *cx = id->cx;
	struct cx18_stream *s = &cx->streams[id->type];
	struct cx18_stream *s_vbi;

	if (s->type == CX18_ENC_STREAM_TYPE_RAD) {
		
		return -EPERM;
	}

	
	if (cx18_claim_stream(id, s->type))
		return -EBUSY;

	
	if (test_bit(CX18_F_S_STREAMOFF, &s->s_flags) ||
	    test_and_set_bit(CX18_F_S_STREAMING, &s->s_flags)) {
		set_bit(CX18_F_S_APPL_IO, &s->s_flags);
		return 0;
	}

	
	s_vbi = &cx->streams[CX18_ENC_STREAM_TYPE_VBI];
	if (s->type == CX18_ENC_STREAM_TYPE_MPG &&
	    test_bit(CX18_F_S_INTERNAL_USE, &s_vbi->s_flags) &&
	    !test_and_set_bit(CX18_F_S_STREAMING, &s_vbi->s_flags)) {
		
		if (cx18_start_v4l2_encode_stream(s_vbi)) {
			CX18_DEBUG_WARN("VBI capture start failed\n");

			
			clear_bit(CX18_F_S_STREAMING, &s_vbi->s_flags);
			clear_bit(CX18_F_S_STREAMING, &s->s_flags);
			
			cx18_release_stream(s);
			return -EIO;
		}
		CX18_DEBUG_INFO("VBI insertion started\n");
	}

	
	if (!cx18_start_v4l2_encode_stream(s)) {
		
		set_bit(CX18_F_S_APPL_IO, &s->s_flags);
		
		if (test_and_clear_bit(CX18_F_I_ENC_PAUSED, &cx->i_flags))
			cx18_vapi(cx, CX18_CPU_CAPTURE_PAUSE, 1, s->handle);
		return 0;
	}

	
	CX18_DEBUG_WARN("Failed to start capturing for stream %s\n", s->name);

	
	if (s->type == CX18_ENC_STREAM_TYPE_MPG &&
	    test_bit(CX18_F_S_STREAMING, &s_vbi->s_flags)) {
		cx18_stop_v4l2_encode_stream(s_vbi, 0);
		clear_bit(CX18_F_S_STREAMING, &s_vbi->s_flags);
	}
	clear_bit(CX18_F_S_STREAMING, &s->s_flags);
	cx18_release_stream(s);
	return -EIO;
}

ssize_t cx18_v4l2_read(struct file *filp, char __user *buf, size_t count,
		loff_t *pos)
{
	struct cx18_open_id *id = filp->private_data;
	struct cx18 *cx = id->cx;
	struct cx18_stream *s = &cx->streams[id->type];
	int rc;

	CX18_DEBUG_HI_FILE("read %zd bytes from %s\n", count, s->name);

	mutex_lock(&cx->serialize_lock);
	rc = cx18_start_capture(id);
	mutex_unlock(&cx->serialize_lock);
	if (rc)
		return rc;
	return cx18_read_pos(s, buf, count, pos, filp->f_flags & O_NONBLOCK);
}

unsigned int cx18_v4l2_enc_poll(struct file *filp, poll_table *wait)
{
	struct cx18_open_id *id = filp->private_data;
	struct cx18 *cx = id->cx;
	struct cx18_stream *s = &cx->streams[id->type];
	int eof = test_bit(CX18_F_S_STREAMOFF, &s->s_flags);

	
	if (!eof && !test_bit(CX18_F_S_STREAMING, &s->s_flags)) {
		int rc;

		mutex_lock(&cx->serialize_lock);
		rc = cx18_start_capture(id);
		mutex_unlock(&cx->serialize_lock);
		if (rc) {
			CX18_DEBUG_INFO("Could not start capture for %s (%d)\n",
					s->name, rc);
			return POLLERR;
		}
		CX18_DEBUG_FILE("Encoder poll started capture\n");
	}

	
	CX18_DEBUG_HI_FILE("Encoder poll\n");
	poll_wait(filp, &s->waitq, wait);

	if (atomic_read(&s->q_full.buffers))
		return POLLIN | POLLRDNORM;
	if (eof)
		return POLLHUP;
	return 0;
}

void cx18_stop_capture(struct cx18_open_id *id, int gop_end)
{
	struct cx18 *cx = id->cx;
	struct cx18_stream *s = &cx->streams[id->type];

	CX18_DEBUG_IOCTL("close() of %s\n", s->name);

	

	
	if (test_bit(CX18_F_S_STREAMING, &s->s_flags)) {
		struct cx18_stream *s_vbi =
			&cx->streams[CX18_ENC_STREAM_TYPE_VBI];

		CX18_DEBUG_INFO("close stopping capture\n");
		
		if (id->type == CX18_ENC_STREAM_TYPE_MPG &&
		    test_bit(CX18_F_S_STREAMING, &s_vbi->s_flags) &&
		    !test_bit(CX18_F_S_APPL_IO, &s_vbi->s_flags)) {
			CX18_DEBUG_INFO("close stopping embedded VBI capture\n");
			cx18_stop_v4l2_encode_stream(s_vbi, 0);
		}
		if (id->type == CX18_ENC_STREAM_TYPE_VBI &&
		    test_bit(CX18_F_S_INTERNAL_USE, &s->s_flags))
			
			s->id = -1;
		else
			cx18_stop_v4l2_encode_stream(s, gop_end);
	}
	if (!gop_end) {
		clear_bit(CX18_F_S_APPL_IO, &s->s_flags);
		clear_bit(CX18_F_S_STREAMOFF, &s->s_flags);
		cx18_release_stream(s);
	}
}

int cx18_v4l2_close(struct file *filp)
{
	struct cx18_open_id *id = filp->private_data;
	struct cx18 *cx = id->cx;
	struct cx18_stream *s = &cx->streams[id->type];

	CX18_DEBUG_IOCTL("close() of %s\n", s->name);

	v4l2_prio_close(&cx->prio, &id->prio);

	
	if (s->id != id->open_id) {
		kfree(id);
		return 0;
	}

	

	
	mutex_lock(&cx->serialize_lock);
	if (id->type == CX18_ENC_STREAM_TYPE_RAD) {
		
		cx18_mute(cx);
		
		clear_bit(CX18_F_I_RADIO_USER, &cx->i_flags);
		
		cx18_call_all(cx, core, s_std, cx->std);
		
		cx18_audio_set_io(cx);
		if (atomic_read(&cx->ana_capturing) > 0) {
			
			cx18_vapi(cx, CX18_CPU_SET_VIDEO_MUTE, 2, s->handle,
				cx->params.video_mute |
					(cx->params.video_mute_yuv << 8));
		}
		
		cx18_unmute(cx);
		cx18_release_stream(s);
	} else {
		cx18_stop_capture(id, 0);
	}
	kfree(id);
	mutex_unlock(&cx->serialize_lock);
	return 0;
}

static int cx18_serialized_open(struct cx18_stream *s, struct file *filp)
{
	struct cx18 *cx = s->cx;
	struct cx18_open_id *item;

	CX18_DEBUG_FILE("open %s\n", s->name);

	
	item = kmalloc(sizeof(struct cx18_open_id), GFP_KERNEL);
	if (NULL == item) {
		CX18_DEBUG_WARN("nomem on v4l2 open\n");
		return -ENOMEM;
	}
	item->cx = cx;
	item->type = s->type;
	v4l2_prio_open(&cx->prio, &item->prio);

	item->open_id = cx->open_id++;
	filp->private_data = item;

	if (item->type == CX18_ENC_STREAM_TYPE_RAD) {
		
		if (cx18_claim_stream(item, item->type)) {
			
			kfree(item);
			return -EBUSY;
		}

		if (!test_bit(CX18_F_I_RADIO_USER, &cx->i_flags)) {
			if (atomic_read(&cx->ana_capturing) > 0) {
				
				cx18_release_stream(s);
				kfree(item);
				return -EBUSY;
			}
		}

		
		set_bit(CX18_F_I_RADIO_USER, &cx->i_flags);
		
		cx18_mute(cx);
		
		cx18_call_all(cx, tuner, s_radio);
		
		cx18_audio_set_io(cx);
		
		cx18_unmute(cx);
	}
	return 0;
}

int cx18_v4l2_open(struct file *filp)
{
	int res;
	struct video_device *video_dev = video_devdata(filp);
	struct cx18_stream *s = video_get_drvdata(video_dev);
	struct cx18 *cx = s->cx;

	mutex_lock(&cx->serialize_lock);
	if (cx18_init_on_first_open(cx)) {
		CX18_ERR("Failed to initialize on minor %d\n",
			 video_dev->minor);
		mutex_unlock(&cx->serialize_lock);
		return -ENXIO;
	}
	res = cx18_serialized_open(s, filp);
	mutex_unlock(&cx->serialize_lock);
	return res;
}

void cx18_mute(struct cx18 *cx)
{
	u32 h;
	if (atomic_read(&cx->ana_capturing)) {
		h = cx18_find_handle(cx);
		if (h != CX18_INVALID_TASK_HANDLE)
			cx18_vapi(cx, CX18_CPU_SET_AUDIO_MUTE, 2, h, 1);
		else
			CX18_ERR("Can't find valid task handle for mute\n");
	}
	CX18_DEBUG_INFO("Mute\n");
}

void cx18_unmute(struct cx18 *cx)
{
	u32 h;
	if (atomic_read(&cx->ana_capturing)) {
		h = cx18_find_handle(cx);
		if (h != CX18_INVALID_TASK_HANDLE) {
			cx18_msleep_timeout(100, 0);
			cx18_vapi(cx, CX18_CPU_SET_MISC_PARAMETERS, 2, h, 12);
			cx18_vapi(cx, CX18_CPU_SET_AUDIO_MUTE, 2, h, 0);
		} else
			CX18_ERR("Can't find valid task handle for unmute\n");
	}
	CX18_DEBUG_INFO("Unmute\n");
}
