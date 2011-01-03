

#ifndef CX25821_VIDEO_H_
#define CX25821_VIDEO_H_

#include <linux/init.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kmod.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <asm/div64.h>

#include "cx25821.h"
#include <media/v4l2-common.h>
#include <media/v4l2-ioctl.h>

#ifdef CONFIG_VIDEO_V4L1_COMPAT

#include <linux/videodev.h>
#endif

#define TUNER_FLAG

#define VIDEO_DEBUG 0

#define dprintk(level, fmt, arg...)\
    do { if (VIDEO_DEBUG >= level)\
	printk(KERN_DEBUG "%s/0: " fmt, dev->name, ## arg);\
    } while (0)


#define UPSTREAM_START_VIDEO        700
#define UPSTREAM_STOP_VIDEO         701
#define UPSTREAM_START_AUDIO        702
#define UPSTREAM_STOP_AUDIO         703
#define UPSTREAM_DUMP_REGISTERS     702
#define SET_VIDEO_STD               800
#define SET_PIXEL_FORMAT            1000
#define ENABLE_CIF_RESOLUTION       1001

#define REG_READ		    900
#define REG_WRITE		    901
#define MEDUSA_READ		    910
#define MEDUSA_WRITE		911

extern struct sram_channel *channel0;
extern struct sram_channel *channel1;
extern struct sram_channel *channel2;
extern struct sram_channel *channel3;
extern struct sram_channel *channel4;
extern struct sram_channel *channel5;
extern struct sram_channel *channel6;
extern struct sram_channel *channel7;
extern struct sram_channel *channel9;
extern struct sram_channel *channel10;
extern struct sram_channel *channel11;
extern struct video_device cx25821_video_template0;
extern struct video_device cx25821_video_template1;
extern struct video_device cx25821_video_template2;
extern struct video_device cx25821_video_template3;
extern struct video_device cx25821_video_template4;
extern struct video_device cx25821_video_template5;
extern struct video_device cx25821_video_template6;
extern struct video_device cx25821_video_template7;
extern struct video_device cx25821_video_template9;
extern struct video_device cx25821_video_template10;
extern struct video_device cx25821_video_template11;
extern struct video_device cx25821_videoioctl_template;


extern unsigned int vid_limit;

#define FORMAT_FLAGS_PACKED       0x01
extern struct cx25821_fmt formats[];
extern struct cx25821_fmt *format_by_fourcc(unsigned int fourcc);
extern struct cx25821_data timeout_data[MAX_VID_CHANNEL_NUM];

extern void dump_video_queue(struct cx25821_dev *dev,
			     struct cx25821_dmaqueue *q);
extern void cx25821_video_wakeup(struct cx25821_dev *dev,
				 struct cx25821_dmaqueue *q, u32 count);

#ifdef TUNER_FLAG
extern int cx25821_set_tvnorm(struct cx25821_dev *dev, v4l2_std_id norm);
#endif

extern int res_get(struct cx25821_dev *dev, struct cx25821_fh *fh,
		   unsigned int bit);
extern int res_check(struct cx25821_fh *fh, unsigned int bit);
extern int res_locked(struct cx25821_dev *dev, unsigned int bit);
extern void res_free(struct cx25821_dev *dev, struct cx25821_fh *fh,
		     unsigned int bits);
extern int cx25821_video_mux(struct cx25821_dev *dev, unsigned int input);
extern int cx25821_start_video_dma(struct cx25821_dev *dev,
				   struct cx25821_dmaqueue *q,
				   struct cx25821_buffer *buf,
				   struct sram_channel *channel);

extern int cx25821_set_scale(struct cx25821_dev *dev, unsigned int width,
			     unsigned int height, enum v4l2_field field);
extern int cx25821_video_irq(struct cx25821_dev *dev, int chan_num, u32 status);
extern void cx25821_video_unregister(struct cx25821_dev *dev, int chan_num);
extern int cx25821_video_register(struct cx25821_dev *dev, int chan_num,
				  struct video_device *video_template);
extern int get_format_size(void);

extern int buffer_setup(struct videobuf_queue *q, unsigned int *count,
			unsigned int *size);
extern int buffer_prepare(struct videobuf_queue *q, struct videobuf_buffer *vb,
			  enum v4l2_field field);
extern void buffer_release(struct videobuf_queue *q,
			   struct videobuf_buffer *vb);
extern struct videobuf_queue *get_queue(struct cx25821_fh *fh);
extern int get_resource(struct cx25821_fh *fh, int resource);
extern int video_mmap(struct file *file, struct vm_area_struct *vma);
extern int vidioc_try_fmt_vid_cap(struct file *file, void *priv,
				  struct v4l2_format *f);
extern int vidioc_querycap(struct file *file, void *priv,
			   struct v4l2_capability *cap);
extern int vidioc_enum_fmt_vid_cap(struct file *file, void *priv,
				   struct v4l2_fmtdesc *f);
extern int vidiocgmbuf(struct file *file, void *priv, struct video_mbuf *mbuf);
extern int vidioc_reqbufs(struct file *file, void *priv,
			  struct v4l2_requestbuffers *p);
extern int vidioc_querybuf(struct file *file, void *priv,
			   struct v4l2_buffer *p);
extern int vidioc_qbuf(struct file *file, void *priv, struct v4l2_buffer *p);
extern int vidioc_s_std(struct file *file, void *priv, v4l2_std_id * tvnorms);
extern int cx25821_enum_input(struct cx25821_dev *dev, struct v4l2_input *i);
extern int vidioc_enum_input(struct file *file, void *priv,
			     struct v4l2_input *i);
extern int vidioc_g_input(struct file *file, void *priv, unsigned int *i);
extern int vidioc_s_input(struct file *file, void *priv, unsigned int i);
extern int vidioc_g_ctrl(struct file *file, void *priv,
			 struct v4l2_control *ctl);
extern int vidioc_g_fmt_vid_cap(struct file *file, void *priv,
				struct v4l2_format *f);
extern int vidioc_g_frequency(struct file *file, void *priv,
			      struct v4l2_frequency *f);
extern int cx25821_set_freq(struct cx25821_dev *dev, struct v4l2_frequency *f);
extern int vidioc_s_frequency(struct file *file, void *priv,
			      struct v4l2_frequency *f);
extern int vidioc_g_register(struct file *file, void *fh,
			     struct v4l2_dbg_register *reg);
extern int vidioc_s_register(struct file *file, void *fh,
			     struct v4l2_dbg_register *reg);
extern int vidioc_g_tuner(struct file *file, void *priv, struct v4l2_tuner *t);
extern int vidioc_s_tuner(struct file *file, void *priv, struct v4l2_tuner *t);

extern int is_valid_width(u32 width, v4l2_std_id tvnorm);
extern int is_valid_height(u32 height, v4l2_std_id tvnorm);

extern int vidioc_g_priority(struct file *file, void *f, enum v4l2_priority *p);
extern int vidioc_s_priority(struct file *file, void *f,
			     enum v4l2_priority prio);

extern int vidioc_queryctrl(struct file *file, void *priv,
			    struct v4l2_queryctrl *qctrl);
extern int cx25821_set_control(struct cx25821_dev *dev,
			       struct v4l2_control *ctrl, int chan_num);

extern int vidioc_cropcap(struct file *file, void *fh,
			  struct v4l2_cropcap *cropcap);
extern int vidioc_s_crop(struct file *file, void *priv, struct v4l2_crop *crop);
extern int vidioc_g_crop(struct file *file, void *priv, struct v4l2_crop *crop);

extern int vidioc_querystd(struct file *file, void *priv, v4l2_std_id * norm);
#endif
