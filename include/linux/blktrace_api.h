#ifndef BLKTRACE_H
#define BLKTRACE_H

#include <linux/types.h>
#ifdef __KERNEL__
#include <linux/blkdev.h>
#include <linux/relay.h>
#endif


enum blktrace_cat {
	BLK_TC_READ	= 1 << 0,	
	BLK_TC_WRITE	= 1 << 1,	
	BLK_TC_BARRIER	= 1 << 2,	
	BLK_TC_SYNC	= 1 << 3,	
	BLK_TC_SYNCIO	= BLK_TC_SYNC,
	BLK_TC_QUEUE	= 1 << 4,	
	BLK_TC_REQUEUE	= 1 << 5,	
	BLK_TC_ISSUE	= 1 << 6,	
	BLK_TC_COMPLETE	= 1 << 7,	
	BLK_TC_FS	= 1 << 8,	
	BLK_TC_PC	= 1 << 9,	
	BLK_TC_NOTIFY	= 1 << 10,	
	BLK_TC_AHEAD	= 1 << 11,	
	BLK_TC_META	= 1 << 12,	
	BLK_TC_DISCARD	= 1 << 13,	
	BLK_TC_DRV_DATA	= 1 << 14,	

	BLK_TC_END	= 1 << 15,	
};

#define BLK_TC_SHIFT		(16)
#define BLK_TC_ACT(act)		((act) << BLK_TC_SHIFT)


enum blktrace_act {
	__BLK_TA_QUEUE = 1,		
	__BLK_TA_BACKMERGE,		
	__BLK_TA_FRONTMERGE,		
	__BLK_TA_GETRQ,			
	__BLK_TA_SLEEPRQ,		
	__BLK_TA_REQUEUE,		
	__BLK_TA_ISSUE,			
	__BLK_TA_COMPLETE,		
	__BLK_TA_PLUG,			
	__BLK_TA_UNPLUG_IO,		
	__BLK_TA_UNPLUG_TIMER,		
	__BLK_TA_INSERT,		
	__BLK_TA_SPLIT,			
	__BLK_TA_BOUNCE,		
	__BLK_TA_REMAP,			
	__BLK_TA_ABORT,			
	__BLK_TA_DRV_DATA,		
};


enum blktrace_notify {
	__BLK_TN_PROCESS = 0,		
	__BLK_TN_TIMESTAMP,		
	__BLK_TN_MESSAGE,		
};



#define BLK_TA_QUEUE		(__BLK_TA_QUEUE | BLK_TC_ACT(BLK_TC_QUEUE))
#define BLK_TA_BACKMERGE	(__BLK_TA_BACKMERGE | BLK_TC_ACT(BLK_TC_QUEUE))
#define BLK_TA_FRONTMERGE	(__BLK_TA_FRONTMERGE | BLK_TC_ACT(BLK_TC_QUEUE))
#define	BLK_TA_GETRQ		(__BLK_TA_GETRQ | BLK_TC_ACT(BLK_TC_QUEUE))
#define	BLK_TA_SLEEPRQ		(__BLK_TA_SLEEPRQ | BLK_TC_ACT(BLK_TC_QUEUE))
#define	BLK_TA_REQUEUE		(__BLK_TA_REQUEUE | BLK_TC_ACT(BLK_TC_REQUEUE))
#define BLK_TA_ISSUE		(__BLK_TA_ISSUE | BLK_TC_ACT(BLK_TC_ISSUE))
#define BLK_TA_COMPLETE		(__BLK_TA_COMPLETE| BLK_TC_ACT(BLK_TC_COMPLETE))
#define BLK_TA_PLUG		(__BLK_TA_PLUG | BLK_TC_ACT(BLK_TC_QUEUE))
#define BLK_TA_UNPLUG_IO	(__BLK_TA_UNPLUG_IO | BLK_TC_ACT(BLK_TC_QUEUE))
#define BLK_TA_UNPLUG_TIMER	(__BLK_TA_UNPLUG_TIMER | BLK_TC_ACT(BLK_TC_QUEUE))
#define BLK_TA_INSERT		(__BLK_TA_INSERT | BLK_TC_ACT(BLK_TC_QUEUE))
#define BLK_TA_SPLIT		(__BLK_TA_SPLIT)
#define BLK_TA_BOUNCE		(__BLK_TA_BOUNCE)
#define BLK_TA_REMAP		(__BLK_TA_REMAP | BLK_TC_ACT(BLK_TC_QUEUE))
#define BLK_TA_ABORT		(__BLK_TA_ABORT | BLK_TC_ACT(BLK_TC_QUEUE))
#define BLK_TA_DRV_DATA	(__BLK_TA_DRV_DATA | BLK_TC_ACT(BLK_TC_DRV_DATA))

#define BLK_TN_PROCESS		(__BLK_TN_PROCESS | BLK_TC_ACT(BLK_TC_NOTIFY))
#define BLK_TN_TIMESTAMP	(__BLK_TN_TIMESTAMP | BLK_TC_ACT(BLK_TC_NOTIFY))
#define BLK_TN_MESSAGE		(__BLK_TN_MESSAGE | BLK_TC_ACT(BLK_TC_NOTIFY))

#define BLK_IO_TRACE_MAGIC	0x65617400
#define BLK_IO_TRACE_VERSION	0x07


struct blk_io_trace {
	__u32 magic;		
	__u32 sequence;		
	__u64 time;		
	__u64 sector;		
	__u32 bytes;		
	__u32 action;		
	__u32 pid;		
	__u32 device;		
	__u32 cpu;		
	__u16 error;		
	__u16 pdu_len;		
};


struct blk_io_trace_remap {
	__be32 device_from;
	__be32 device_to;
	__be64 sector_from;
};

enum {
	Blktrace_setup = 1,
	Blktrace_running,
	Blktrace_stopped,
};

#define BLKTRACE_BDEV_SIZE	32


struct blk_user_trace_setup {
	char name[BLKTRACE_BDEV_SIZE];	
	__u16 act_mask;			
	__u32 buf_size;			
	__u32 buf_nr;			
	__u64 start_lba;
	__u64 end_lba;
	__u32 pid;
};

#ifdef __KERNEL__
#if defined(CONFIG_BLK_DEV_IO_TRACE)

#include <linux/sysfs.h>

struct blk_trace {
	int trace_state;
	struct rchan *rchan;
	unsigned long *sequence;
	unsigned char *msg_data;
	u16 act_mask;
	u64 start_lba;
	u64 end_lba;
	u32 pid;
	u32 dev;
	struct dentry *dir;
	struct dentry *dropped_file;
	struct dentry *msg_file;
	atomic_t dropped;
};

extern int blk_trace_ioctl(struct block_device *, unsigned, char __user *);
extern void blk_trace_shutdown(struct request_queue *);
extern int do_blk_trace_setup(struct request_queue *q, char *name,
			      dev_t dev, struct block_device *bdev,
			      struct blk_user_trace_setup *buts);
extern void __trace_note_message(struct blk_trace *, const char *fmt, ...);


#define blk_add_trace_msg(q, fmt, ...)					\
	do {								\
		struct blk_trace *bt = (q)->blk_trace;			\
		if (unlikely(bt))					\
			__trace_note_message(bt, fmt, ##__VA_ARGS__);	\
	} while (0)
#define BLK_TN_MAX_MSG		128

extern void blk_add_driver_data(struct request_queue *q, struct request *rq,
				void *data, size_t len);
extern int blk_trace_setup(struct request_queue *q, char *name, dev_t dev,
			   struct block_device *bdev,
			   char __user *arg);
extern int blk_trace_startstop(struct request_queue *q, int start);
extern int blk_trace_remove(struct request_queue *q);
extern void blk_trace_remove_sysfs(struct device *dev);
extern int blk_trace_init_sysfs(struct device *dev);

extern struct attribute_group blk_trace_attr_group;

#else 
# define blk_trace_ioctl(bdev, cmd, arg)		(-ENOTTY)
# define blk_trace_shutdown(q)				do { } while (0)
# define do_blk_trace_setup(q, name, dev, bdev, buts)	(-ENOTTY)
# define blk_add_driver_data(q, rq, data, len)		do {} while (0)
# define blk_trace_setup(q, name, dev, bdev, arg)	(-ENOTTY)
# define blk_trace_startstop(q, start)			(-ENOTTY)
# define blk_trace_remove(q)				(-ENOTTY)
# define blk_add_trace_msg(q, fmt, ...)			do { } while (0)
# define blk_trace_remove_sysfs(dev)			do { } while (0)
static inline int blk_trace_init_sysfs(struct device *dev)
{
	return 0;
}

#endif 

#if defined(CONFIG_EVENT_TRACING) && defined(CONFIG_BLOCK)

static inline int blk_cmd_buf_len(struct request *rq)
{
	return blk_pc_request(rq) ? rq->cmd_len * 3 : 1;
}

extern void blk_dump_cmd(char *buf, struct request *rq);
extern void blk_fill_rwbs(char *rwbs, u32 rw, int bytes);
extern void blk_fill_rwbs_rq(char *rwbs, struct request *rq);

#endif 

#endif 
#endif
