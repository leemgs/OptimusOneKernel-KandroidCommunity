

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/usb.h>
#include <linux/poll.h>
#include <linux/compat.h>
#include <linux/mm.h>
#include <linux/smp_lock.h>

#include <asm/uaccess.h>

#include "usb_mon.h"


#define SETUP_LEN  8


#define MON_IOC_MAGIC 0x92

#define MON_IOCQ_URB_LEN _IO(MON_IOC_MAGIC, 1)

#define MON_IOCG_STATS _IOR(MON_IOC_MAGIC, 3, struct mon_bin_stats)
#define MON_IOCT_RING_SIZE _IO(MON_IOC_MAGIC, 4)
#define MON_IOCQ_RING_SIZE _IO(MON_IOC_MAGIC, 5)
#define MON_IOCX_GET   _IOW(MON_IOC_MAGIC, 6, struct mon_bin_get)
#define MON_IOCX_MFETCH _IOWR(MON_IOC_MAGIC, 7, struct mon_bin_mfetch)
#define MON_IOCH_MFLUSH _IO(MON_IOC_MAGIC, 8)

#define MON_IOCX_GETX   _IOW(MON_IOC_MAGIC, 10, struct mon_bin_get)

#ifdef CONFIG_COMPAT
#define MON_IOCX_GET32 _IOW(MON_IOC_MAGIC, 6, struct mon_bin_get32)
#define MON_IOCX_MFETCH32 _IOWR(MON_IOC_MAGIC, 7, struct mon_bin_mfetch32)
#define MON_IOCX_GETX32   _IOW(MON_IOC_MAGIC, 10, struct mon_bin_get32)
#endif


#define CHUNK_SIZE   PAGE_SIZE
#define CHUNK_ALIGN(x)   (((x)+CHUNK_SIZE-1) & ~(CHUNK_SIZE-1))


#define BUFF_MAX  CHUNK_ALIGN(1200*1024)
#define BUFF_DFL   CHUNK_ALIGN(300*1024)
#define BUFF_MIN     CHUNK_ALIGN(8*1024)


struct mon_bin_hdr {
	u64 id;			
	unsigned char type;	
	unsigned char xfer_type;	
	unsigned char epnum;	
	unsigned char devnum;	
	unsigned short busnum;	
	char flag_setup;
	char flag_data;
	s64 ts_sec;		
	s32 ts_usec;		
	int status;
	unsigned int len_urb;	
	unsigned int len_cap;	
	union {
		unsigned char setup[SETUP_LEN];	
		struct iso_rec {
			int error_count;
			int numdesc;
		} iso;
	} s;
	int interval;
	int start_frame;
	unsigned int xfer_flags;
	unsigned int ndesc;	
};


struct mon_bin_isodesc {
	int          iso_status;
	unsigned int iso_off;
	unsigned int iso_len;
	u32 _pad;
};


struct mon_bin_stats {
	u32 queued;
	u32 dropped;
};

struct mon_bin_get {
	struct mon_bin_hdr __user *hdr;	
	void __user *data;
	size_t alloc;		
};

struct mon_bin_mfetch {
	u32 __user *offvec;	
	u32 nfetch;		
	u32 nflush;		
};

#ifdef CONFIG_COMPAT
struct mon_bin_get32 {
	u32 hdr32;
	u32 data32;
	u32 alloc32;
};

struct mon_bin_mfetch32 {
        u32 offvec32;
        u32 nfetch32;
        u32 nflush32;
};
#endif


#define PKT_ALIGN   64
#define PKT_SIZE    64

#define PKT_SZ_API0 48	
#define PKT_SZ_API1 64	

#define ISODESC_MAX   128	


#define MON_BIN_MAX_MINOR 128


struct mon_pgmap {
	struct page *pg;
	unsigned char *ptr;	
};


struct mon_reader_bin {
	
	spinlock_t b_lock;		
	unsigned int b_size;		
	unsigned int b_cnt;		
	unsigned int b_in, b_out;	
	unsigned int b_read;		
	struct mon_pgmap *b_vec;	
	wait_queue_head_t b_wait;	

	struct mutex fetch_lock;	
	int mmap_active;

	
	struct mon_reader r;

	
	unsigned int cnt_lost;
};

static inline struct mon_bin_hdr *MON_OFF2HDR(const struct mon_reader_bin *rp,
    unsigned int offset)
{
	return (struct mon_bin_hdr *)
	    (rp->b_vec[offset / CHUNK_SIZE].ptr + offset % CHUNK_SIZE);
}

#define MON_RING_EMPTY(rp)	((rp)->b_cnt == 0)

static unsigned char xfer_to_pipe[4] = {
	PIPE_CONTROL, PIPE_ISOCHRONOUS, PIPE_BULK, PIPE_INTERRUPT
};

static struct class *mon_bin_class;
static dev_t mon_bin_dev0;
static struct cdev mon_bin_cdev;

static void mon_buff_area_fill(const struct mon_reader_bin *rp,
    unsigned int offset, unsigned int size);
static int mon_bin_wait_event(struct file *file, struct mon_reader_bin *rp);
static int mon_alloc_buff(struct mon_pgmap *map, int npages);
static void mon_free_buff(struct mon_pgmap *map, int npages);


static void mon_copy_to_buff(const struct mon_reader_bin *this,
    unsigned int off, const unsigned char *from, unsigned int length)
{
	unsigned int step_len;
	unsigned char *buf;
	unsigned int in_page;

	while (length) {
		
		step_len = length;
		in_page = CHUNK_SIZE - (off & (CHUNK_SIZE-1));
		if (in_page < step_len)
			step_len = in_page;

		
		buf = this->b_vec[off / CHUNK_SIZE].ptr + off % CHUNK_SIZE;
		memcpy(buf, from, step_len);
		if ((off += step_len) >= this->b_size) off = 0;
		from += step_len;
		length -= step_len;
	}
}


static int copy_from_buf(const struct mon_reader_bin *this, unsigned int off,
    char __user *to, int length)
{
	unsigned int step_len;
	unsigned char *buf;
	unsigned int in_page;

	while (length) {
		
		step_len = length;
		in_page = CHUNK_SIZE - (off & (CHUNK_SIZE-1));
		if (in_page < step_len)
			step_len = in_page;

		
		buf = this->b_vec[off / CHUNK_SIZE].ptr + off % CHUNK_SIZE;
		if (copy_to_user(to, buf, step_len))
			return -EINVAL;
		if ((off += step_len) >= this->b_size) off = 0;
		to += step_len;
		length -= step_len;
	}
	return 0;
}


static unsigned int mon_buff_area_alloc(struct mon_reader_bin *rp,
    unsigned int size)
{
	unsigned int offset;

	size = (size + PKT_ALIGN-1) & ~(PKT_ALIGN-1);
	if (rp->b_cnt + size > rp->b_size)
		return ~0;
	offset = rp->b_in;
	rp->b_cnt += size;
	if ((rp->b_in += size) >= rp->b_size)
		rp->b_in -= rp->b_size;
	return offset;
}


static unsigned int mon_buff_area_alloc_contiguous(struct mon_reader_bin *rp,
    unsigned int size)
{
	unsigned int offset;
	unsigned int fill_size;

	size = (size + PKT_ALIGN-1) & ~(PKT_ALIGN-1);
	if (rp->b_cnt + size > rp->b_size)
		return ~0;
	if (rp->b_in + size > rp->b_size) {
		
		fill_size = rp->b_size - rp->b_in;
		if (rp->b_cnt + size + fill_size > rp->b_size)
			return ~0;
		mon_buff_area_fill(rp, rp->b_in, fill_size);

		offset = 0;
		rp->b_in = size;
		rp->b_cnt += size + fill_size;
	} else if (rp->b_in + size == rp->b_size) {
		offset = rp->b_in;
		rp->b_in = 0;
		rp->b_cnt += size;
	} else {
		offset = rp->b_in;
		rp->b_in += size;
		rp->b_cnt += size;
	}
	return offset;
}


static void mon_buff_area_shrink(struct mon_reader_bin *rp, unsigned int size)
{

	
	rp->b_cnt -= size;
	if (rp->b_in < size)
		rp->b_in += rp->b_size;
	rp->b_in -= size;
}


static void mon_buff_area_free(struct mon_reader_bin *rp, unsigned int size)
{

	size = (size + PKT_ALIGN-1) & ~(PKT_ALIGN-1);
	rp->b_cnt -= size;
	if ((rp->b_out += size) >= rp->b_size)
		rp->b_out -= rp->b_size;
}

static void mon_buff_area_fill(const struct mon_reader_bin *rp,
    unsigned int offset, unsigned int size)
{
	struct mon_bin_hdr *ep;

	ep = MON_OFF2HDR(rp, offset);
	memset(ep, 0, PKT_SIZE);
	ep->type = '@';
	ep->len_cap = size - PKT_SIZE;
}

static inline char mon_bin_get_setup(unsigned char *setupb,
    const struct urb *urb, char ev_type)
{

	if (urb->setup_packet == NULL)
		return 'Z';
	memcpy(setupb, urb->setup_packet, SETUP_LEN);
	return 0;
}

static char mon_bin_get_data(const struct mon_reader_bin *rp,
    unsigned int offset, struct urb *urb, unsigned int length)
{

	if (urb->transfer_buffer == NULL)
		return 'Z';
	mon_copy_to_buff(rp, offset, urb->transfer_buffer, length);
	return 0;
}

static void mon_bin_get_isodesc(const struct mon_reader_bin *rp,
    unsigned int offset, struct urb *urb, char ev_type, unsigned int ndesc)
{
	struct mon_bin_isodesc *dp;
	struct usb_iso_packet_descriptor *fp;

	fp = urb->iso_frame_desc;
	while (ndesc-- != 0) {
		dp = (struct mon_bin_isodesc *)
		    (rp->b_vec[offset / CHUNK_SIZE].ptr + offset % CHUNK_SIZE);
		dp->iso_status = fp->status;
		dp->iso_off = fp->offset;
		dp->iso_len = (ev_type == 'S') ? fp->length : fp->actual_length;
		dp->_pad = 0;
		if ((offset += sizeof(struct mon_bin_isodesc)) >= rp->b_size)
			offset = 0;
		fp++;
	}
}

static void mon_bin_event(struct mon_reader_bin *rp, struct urb *urb,
    char ev_type, int status)
{
	const struct usb_endpoint_descriptor *epd = &urb->ep->desc;
	unsigned long flags;
	struct timeval ts;
	unsigned int urb_length;
	unsigned int offset;
	unsigned int length;
	unsigned int delta;
	unsigned int ndesc, lendesc;
	unsigned char dir;
	struct mon_bin_hdr *ep;
	char data_tag = 0;

	do_gettimeofday(&ts);

	spin_lock_irqsave(&rp->b_lock, flags);

	
	if (usb_endpoint_xfer_isoc(epd)) {
		if (urb->number_of_packets < 0) {
			ndesc = 0;
		} else if (urb->number_of_packets >= ISODESC_MAX) {
			ndesc = ISODESC_MAX;
		} else {
			ndesc = urb->number_of_packets;
		}
	} else {
		ndesc = 0;
	}
	lendesc = ndesc*sizeof(struct mon_bin_isodesc);

	urb_length = (ev_type == 'S') ?
	    urb->transfer_buffer_length : urb->actual_length;
	length = urb_length;

	if (length >= rp->b_size/5)
		length = rp->b_size/5;

	if (usb_urb_dir_in(urb)) {
		if (ev_type == 'S') {
			length = 0;
			data_tag = '<';
		}
		
		dir = USB_DIR_IN;
	} else {
		if (ev_type == 'C') {
			length = 0;
			data_tag = '>';
		}
		dir = 0;
	}

	if (rp->mmap_active) {
		offset = mon_buff_area_alloc_contiguous(rp,
						 length + PKT_SIZE + lendesc);
	} else {
		offset = mon_buff_area_alloc(rp, length + PKT_SIZE + lendesc);
	}
	if (offset == ~0) {
		rp->cnt_lost++;
		spin_unlock_irqrestore(&rp->b_lock, flags);
		return;
	}

	ep = MON_OFF2HDR(rp, offset);
	if ((offset += PKT_SIZE) >= rp->b_size) offset = 0;

	
	memset(ep, 0, PKT_SIZE);
	ep->type = ev_type;
	ep->xfer_type = xfer_to_pipe[usb_endpoint_type(epd)];
	ep->epnum = dir | usb_endpoint_num(epd);
	ep->devnum = urb->dev->devnum;
	ep->busnum = urb->dev->bus->busnum;
	ep->id = (unsigned long) urb;
	ep->ts_sec = ts.tv_sec;
	ep->ts_usec = ts.tv_usec;
	ep->status = status;
	ep->len_urb = urb_length;
	ep->len_cap = length + lendesc;
	ep->xfer_flags = urb->transfer_flags;

	if (usb_endpoint_xfer_int(epd)) {
		ep->interval = urb->interval;
	} else if (usb_endpoint_xfer_isoc(epd)) {
		ep->interval = urb->interval;
		ep->start_frame = urb->start_frame;
		ep->s.iso.error_count = urb->error_count;
		ep->s.iso.numdesc = urb->number_of_packets;
	}

	if (usb_endpoint_xfer_control(epd) && ev_type == 'S') {
		ep->flag_setup = mon_bin_get_setup(ep->s.setup, urb, ev_type);
	} else {
		ep->flag_setup = '-';
	}

	if (ndesc != 0) {
		ep->ndesc = ndesc;
		mon_bin_get_isodesc(rp, offset, urb, ev_type, ndesc);
		if ((offset += lendesc) >= rp->b_size)
			offset -= rp->b_size;
	}

	if (length != 0) {
		ep->flag_data = mon_bin_get_data(rp, offset, urb, length);
		if (ep->flag_data != 0) {	
			delta = (ep->len_cap + PKT_ALIGN-1) & ~(PKT_ALIGN-1);
			ep->len_cap -= length;
			delta -= (ep->len_cap + PKT_ALIGN-1) & ~(PKT_ALIGN-1);
			mon_buff_area_shrink(rp, delta);
		}
	} else {
		ep->flag_data = data_tag;
	}

	spin_unlock_irqrestore(&rp->b_lock, flags);

	wake_up(&rp->b_wait);
}

static void mon_bin_submit(void *data, struct urb *urb)
{
	struct mon_reader_bin *rp = data;
	mon_bin_event(rp, urb, 'S', -EINPROGRESS);
}

static void mon_bin_complete(void *data, struct urb *urb, int status)
{
	struct mon_reader_bin *rp = data;
	mon_bin_event(rp, urb, 'C', status);
}

static void mon_bin_error(void *data, struct urb *urb, int error)
{
	struct mon_reader_bin *rp = data;
	unsigned long flags;
	unsigned int offset;
	struct mon_bin_hdr *ep;

	spin_lock_irqsave(&rp->b_lock, flags);

	offset = mon_buff_area_alloc(rp, PKT_SIZE);
	if (offset == ~0) {
		
		spin_unlock_irqrestore(&rp->b_lock, flags);
		return;
	}

	ep = MON_OFF2HDR(rp, offset);

	memset(ep, 0, PKT_SIZE);
	ep->type = 'E';
	ep->xfer_type = xfer_to_pipe[usb_endpoint_type(&urb->ep->desc)];
	ep->epnum = usb_urb_dir_in(urb) ? USB_DIR_IN : 0;
	ep->epnum |= usb_endpoint_num(&urb->ep->desc);
	ep->devnum = urb->dev->devnum;
	ep->busnum = urb->dev->bus->busnum;
	ep->id = (unsigned long) urb;
	ep->status = error;

	ep->flag_setup = '-';
	ep->flag_data = 'E';

	spin_unlock_irqrestore(&rp->b_lock, flags);

	wake_up(&rp->b_wait);
}

static int mon_bin_open(struct inode *inode, struct file *file)
{
	struct mon_bus *mbus;
	struct mon_reader_bin *rp;
	size_t size;
	int rc;

	lock_kernel();
	mutex_lock(&mon_lock);
	if ((mbus = mon_bus_lookup(iminor(inode))) == NULL) {
		mutex_unlock(&mon_lock);
		unlock_kernel();
		return -ENODEV;
	}
	if (mbus != &mon_bus0 && mbus->u_bus == NULL) {
		printk(KERN_ERR TAG ": consistency error on open\n");
		mutex_unlock(&mon_lock);
		unlock_kernel();
		return -ENODEV;
	}

	rp = kzalloc(sizeof(struct mon_reader_bin), GFP_KERNEL);
	if (rp == NULL) {
		rc = -ENOMEM;
		goto err_alloc;
	}
	spin_lock_init(&rp->b_lock);
	init_waitqueue_head(&rp->b_wait);
	mutex_init(&rp->fetch_lock);
	rp->b_size = BUFF_DFL;

	size = sizeof(struct mon_pgmap) * (rp->b_size/CHUNK_SIZE);
	if ((rp->b_vec = kzalloc(size, GFP_KERNEL)) == NULL) {
		rc = -ENOMEM;
		goto err_allocvec;
	}

	if ((rc = mon_alloc_buff(rp->b_vec, rp->b_size/CHUNK_SIZE)) < 0)
		goto err_allocbuff;

	rp->r.m_bus = mbus;
	rp->r.r_data = rp;
	rp->r.rnf_submit = mon_bin_submit;
	rp->r.rnf_error = mon_bin_error;
	rp->r.rnf_complete = mon_bin_complete;

	mon_reader_add(mbus, &rp->r);

	file->private_data = rp;
	mutex_unlock(&mon_lock);
	unlock_kernel();
	return 0;

err_allocbuff:
	kfree(rp->b_vec);
err_allocvec:
	kfree(rp);
err_alloc:
	mutex_unlock(&mon_lock);
	unlock_kernel();
	return rc;
}


static int mon_bin_get_event(struct file *file, struct mon_reader_bin *rp,
    struct mon_bin_hdr __user *hdr, unsigned int hdrbytes,
    void __user *data, unsigned int nbytes)
{
	unsigned long flags;
	struct mon_bin_hdr *ep;
	size_t step_len;
	unsigned int offset;
	int rc;

	mutex_lock(&rp->fetch_lock);

	if ((rc = mon_bin_wait_event(file, rp)) < 0) {
		mutex_unlock(&rp->fetch_lock);
		return rc;
	}

	ep = MON_OFF2HDR(rp, rp->b_out);

	if (copy_to_user(hdr, ep, hdrbytes)) {
		mutex_unlock(&rp->fetch_lock);
		return -EFAULT;
	}

	step_len = min(ep->len_cap, nbytes);
	if ((offset = rp->b_out + PKT_SIZE) >= rp->b_size) offset = 0;

	if (copy_from_buf(rp, offset, data, step_len)) {
		mutex_unlock(&rp->fetch_lock);
		return -EFAULT;
	}

	spin_lock_irqsave(&rp->b_lock, flags);
	mon_buff_area_free(rp, PKT_SIZE + ep->len_cap);
	spin_unlock_irqrestore(&rp->b_lock, flags);
	rp->b_read = 0;

	mutex_unlock(&rp->fetch_lock);
	return 0;
}

static int mon_bin_release(struct inode *inode, struct file *file)
{
	struct mon_reader_bin *rp = file->private_data;
	struct mon_bus* mbus = rp->r.m_bus;

	mutex_lock(&mon_lock);

	if (mbus->nreaders <= 0) {
		printk(KERN_ERR TAG ": consistency error on close\n");
		mutex_unlock(&mon_lock);
		return 0;
	}
	mon_reader_del(mbus, &rp->r);

	mon_free_buff(rp->b_vec, rp->b_size/CHUNK_SIZE);
	kfree(rp->b_vec);
	kfree(rp);

	mutex_unlock(&mon_lock);
	return 0;
}

static ssize_t mon_bin_read(struct file *file, char __user *buf,
    size_t nbytes, loff_t *ppos)
{
	struct mon_reader_bin *rp = file->private_data;
	unsigned int hdrbytes = PKT_SZ_API0;
	unsigned long flags;
	struct mon_bin_hdr *ep;
	unsigned int offset;
	size_t step_len;
	char *ptr;
	ssize_t done = 0;
	int rc;

	mutex_lock(&rp->fetch_lock);

	if ((rc = mon_bin_wait_event(file, rp)) < 0) {
		mutex_unlock(&rp->fetch_lock);
		return rc;
	}

	ep = MON_OFF2HDR(rp, rp->b_out);

	if (rp->b_read < hdrbytes) {
		step_len = min(nbytes, (size_t)(hdrbytes - rp->b_read));
		ptr = ((char *)ep) + rp->b_read;
		if (step_len && copy_to_user(buf, ptr, step_len)) {
			mutex_unlock(&rp->fetch_lock);
			return -EFAULT;
		}
		nbytes -= step_len;
		buf += step_len;
		rp->b_read += step_len;
		done += step_len;
	}

	if (rp->b_read >= hdrbytes) {
		step_len = ep->len_cap;
		step_len -= rp->b_read - hdrbytes;
		if (step_len > nbytes)
			step_len = nbytes;
		offset = rp->b_out + PKT_SIZE;
		offset += rp->b_read - hdrbytes;
		if (offset >= rp->b_size)
			offset -= rp->b_size;
		if (copy_from_buf(rp, offset, buf, step_len)) {
			mutex_unlock(&rp->fetch_lock);
			return -EFAULT;
		}
		nbytes -= step_len;
		buf += step_len;
		rp->b_read += step_len;
		done += step_len;
	}

	
	if (rp->b_read >= hdrbytes + ep->len_cap) {
		spin_lock_irqsave(&rp->b_lock, flags);
		mon_buff_area_free(rp, PKT_SIZE + ep->len_cap);
		spin_unlock_irqrestore(&rp->b_lock, flags);
		rp->b_read = 0;
	}

	mutex_unlock(&rp->fetch_lock);
	return done;
}


static int mon_bin_flush(struct mon_reader_bin *rp, unsigned nevents)
{
	unsigned long flags;
	struct mon_bin_hdr *ep;
	int i;

	mutex_lock(&rp->fetch_lock);
	spin_lock_irqsave(&rp->b_lock, flags);
	for (i = 0; i < nevents; ++i) {
		if (MON_RING_EMPTY(rp))
			break;

		ep = MON_OFF2HDR(rp, rp->b_out);
		mon_buff_area_free(rp, PKT_SIZE + ep->len_cap);
	}
	spin_unlock_irqrestore(&rp->b_lock, flags);
	rp->b_read = 0;
	mutex_unlock(&rp->fetch_lock);
	return i;
}


static int mon_bin_fetch(struct file *file, struct mon_reader_bin *rp,
    u32 __user *vec, unsigned int max)
{
	unsigned int cur_out;
	unsigned int bytes, avail;
	unsigned int size;
	unsigned int nevents;
	struct mon_bin_hdr *ep;
	unsigned long flags;
	int rc;

	mutex_lock(&rp->fetch_lock);

	if ((rc = mon_bin_wait_event(file, rp)) < 0) {
		mutex_unlock(&rp->fetch_lock);
		return rc;
	}

	spin_lock_irqsave(&rp->b_lock, flags);
	avail = rp->b_cnt;
	spin_unlock_irqrestore(&rp->b_lock, flags);

	cur_out = rp->b_out;
	nevents = 0;
	bytes = 0;
	while (bytes < avail) {
		if (nevents >= max)
			break;

		ep = MON_OFF2HDR(rp, cur_out);
		if (put_user(cur_out, &vec[nevents])) {
			mutex_unlock(&rp->fetch_lock);
			return -EFAULT;
		}

		nevents++;
		size = ep->len_cap + PKT_SIZE;
		size = (size + PKT_ALIGN-1) & ~(PKT_ALIGN-1);
		if ((cur_out += size) >= rp->b_size)
			cur_out -= rp->b_size;
		bytes += size;
	}

	mutex_unlock(&rp->fetch_lock);
	return nevents;
}


static int mon_bin_queued(struct mon_reader_bin *rp)
{
	unsigned int cur_out;
	unsigned int bytes, avail;
	unsigned int size;
	unsigned int nevents;
	struct mon_bin_hdr *ep;
	unsigned long flags;

	mutex_lock(&rp->fetch_lock);

	spin_lock_irqsave(&rp->b_lock, flags);
	avail = rp->b_cnt;
	spin_unlock_irqrestore(&rp->b_lock, flags);

	cur_out = rp->b_out;
	nevents = 0;
	bytes = 0;
	while (bytes < avail) {
		ep = MON_OFF2HDR(rp, cur_out);

		nevents++;
		size = ep->len_cap + PKT_SIZE;
		size = (size + PKT_ALIGN-1) & ~(PKT_ALIGN-1);
		if ((cur_out += size) >= rp->b_size)
			cur_out -= rp->b_size;
		bytes += size;
	}

	mutex_unlock(&rp->fetch_lock);
	return nevents;
}


static int mon_bin_ioctl(struct inode *inode, struct file *file,
    unsigned int cmd, unsigned long arg)
{
	struct mon_reader_bin *rp = file->private_data;
	
	int ret = 0;
	struct mon_bin_hdr *ep;
	unsigned long flags;

	switch (cmd) {

	case MON_IOCQ_URB_LEN:
		
		spin_lock_irqsave(&rp->b_lock, flags);
		if (!MON_RING_EMPTY(rp)) {
			ep = MON_OFF2HDR(rp, rp->b_out);
			ret = ep->len_cap;
		}
		spin_unlock_irqrestore(&rp->b_lock, flags);
		break;

	case MON_IOCQ_RING_SIZE:
		ret = rp->b_size;
		break;

	case MON_IOCT_RING_SIZE:
		
		{
		int size;
		struct mon_pgmap *vec;

		if (arg < BUFF_MIN || arg > BUFF_MAX)
			return -EINVAL;

		size = CHUNK_ALIGN(arg);
		if ((vec = kzalloc(sizeof(struct mon_pgmap) * (size/CHUNK_SIZE),
		    GFP_KERNEL)) == NULL) {
			ret = -ENOMEM;
			break;
		}

		ret = mon_alloc_buff(vec, size/CHUNK_SIZE);
		if (ret < 0) {
			kfree(vec);
			break;
		}

		mutex_lock(&rp->fetch_lock);
		spin_lock_irqsave(&rp->b_lock, flags);
		mon_free_buff(rp->b_vec, size/CHUNK_SIZE);
		kfree(rp->b_vec);
		rp->b_vec  = vec;
		rp->b_size = size;
		rp->b_read = rp->b_in = rp->b_out = rp->b_cnt = 0;
		rp->cnt_lost = 0;
		spin_unlock_irqrestore(&rp->b_lock, flags);
		mutex_unlock(&rp->fetch_lock);
		}
		break;

	case MON_IOCH_MFLUSH:
		ret = mon_bin_flush(rp, arg);
		break;

	case MON_IOCX_GET:
	case MON_IOCX_GETX:
		{
		struct mon_bin_get getb;

		if (copy_from_user(&getb, (void __user *)arg,
					    sizeof(struct mon_bin_get)))
			return -EFAULT;

		if (getb.alloc > 0x10000000)	
			return -EINVAL;
		ret = mon_bin_get_event(file, rp, getb.hdr,
		    (cmd == MON_IOCX_GET)? PKT_SZ_API0: PKT_SZ_API1,
		    getb.data, (unsigned int)getb.alloc);
		}
		break;

	case MON_IOCX_MFETCH:
		{
		struct mon_bin_mfetch mfetch;
		struct mon_bin_mfetch __user *uptr;

		uptr = (struct mon_bin_mfetch __user *)arg;

		if (copy_from_user(&mfetch, uptr, sizeof(mfetch)))
			return -EFAULT;

		if (mfetch.nflush) {
			ret = mon_bin_flush(rp, mfetch.nflush);
			if (ret < 0)
				return ret;
			if (put_user(ret, &uptr->nflush))
				return -EFAULT;
		}
		ret = mon_bin_fetch(file, rp, mfetch.offvec, mfetch.nfetch);
		if (ret < 0)
			return ret;
		if (put_user(ret, &uptr->nfetch))
			return -EFAULT;
		ret = 0;
		}
		break;

	case MON_IOCG_STATS: {
		struct mon_bin_stats __user *sp;
		unsigned int nevents;
		unsigned int ndropped;

		spin_lock_irqsave(&rp->b_lock, flags);
		ndropped = rp->cnt_lost;
		rp->cnt_lost = 0;
		spin_unlock_irqrestore(&rp->b_lock, flags);
		nevents = mon_bin_queued(rp);

		sp = (struct mon_bin_stats __user *)arg;
		if (put_user(rp->cnt_lost, &sp->dropped))
			return -EFAULT;
		if (put_user(nevents, &sp->queued))
			return -EFAULT;

		}
		break;

	default:
		return -ENOTTY;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long mon_bin_compat_ioctl(struct file *file,
    unsigned int cmd, unsigned long arg)
{
	struct mon_reader_bin *rp = file->private_data;
	int ret;

	switch (cmd) {

	case MON_IOCX_GET32:
	case MON_IOCX_GETX32:
		{
		struct mon_bin_get32 getb;

		if (copy_from_user(&getb, (void __user *)arg,
					    sizeof(struct mon_bin_get32)))
			return -EFAULT;

		ret = mon_bin_get_event(file, rp, compat_ptr(getb.hdr32),
		    (cmd == MON_IOCX_GET32)? PKT_SZ_API0: PKT_SZ_API1,
		    compat_ptr(getb.data32), getb.alloc32);
		if (ret < 0)
			return ret;
		}
		return 0;

	case MON_IOCX_MFETCH32:
		{
		struct mon_bin_mfetch32 mfetch;
		struct mon_bin_mfetch32 __user *uptr;

		uptr = (struct mon_bin_mfetch32 __user *) compat_ptr(arg);

		if (copy_from_user(&mfetch, uptr, sizeof(mfetch)))
			return -EFAULT;

		if (mfetch.nflush32) {
			ret = mon_bin_flush(rp, mfetch.nflush32);
			if (ret < 0)
				return ret;
			if (put_user(ret, &uptr->nflush32))
				return -EFAULT;
		}
		ret = mon_bin_fetch(file, rp, compat_ptr(mfetch.offvec32),
		    mfetch.nfetch32);
		if (ret < 0)
			return ret;
		if (put_user(ret, &uptr->nfetch32))
			return -EFAULT;
		}
		return 0;

	case MON_IOCG_STATS:
		return mon_bin_ioctl(NULL, file, cmd,
					    (unsigned long) compat_ptr(arg));

	case MON_IOCQ_URB_LEN:
	case MON_IOCQ_RING_SIZE:
	case MON_IOCT_RING_SIZE:
	case MON_IOCH_MFLUSH:
		return mon_bin_ioctl(NULL, file, cmd, arg);

	default:
		;
	}
	return -ENOTTY;
}
#endif 

static unsigned int
mon_bin_poll(struct file *file, struct poll_table_struct *wait)
{
	struct mon_reader_bin *rp = file->private_data;
	unsigned int mask = 0;
	unsigned long flags;

	if (file->f_mode & FMODE_READ)
		poll_wait(file, &rp->b_wait, wait);

	spin_lock_irqsave(&rp->b_lock, flags);
	if (!MON_RING_EMPTY(rp))
		mask |= POLLIN | POLLRDNORM;    
	spin_unlock_irqrestore(&rp->b_lock, flags);
	return mask;
}


static void mon_bin_vma_open(struct vm_area_struct *vma)
{
	struct mon_reader_bin *rp = vma->vm_private_data;
	rp->mmap_active++;
}

static void mon_bin_vma_close(struct vm_area_struct *vma)
{
	struct mon_reader_bin *rp = vma->vm_private_data;
	rp->mmap_active--;
}


static int mon_bin_vma_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct mon_reader_bin *rp = vma->vm_private_data;
	unsigned long offset, chunk_idx;
	struct page *pageptr;

	offset = vmf->pgoff << PAGE_SHIFT;
	if (offset >= rp->b_size)
		return VM_FAULT_SIGBUS;
	chunk_idx = offset / CHUNK_SIZE;
	pageptr = rp->b_vec[chunk_idx].pg;
	get_page(pageptr);
	vmf->page = pageptr;
	return 0;
}

static const struct vm_operations_struct mon_bin_vm_ops = {
	.open =     mon_bin_vma_open,
	.close =    mon_bin_vma_close,
	.fault =    mon_bin_vma_fault,
};

static int mon_bin_mmap(struct file *filp, struct vm_area_struct *vma)
{
	
	vma->vm_ops = &mon_bin_vm_ops;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_private_data = filp->private_data;
	mon_bin_vma_open(vma);
	return 0;
}

static const struct file_operations mon_fops_binary = {
	.owner =	THIS_MODULE,
	.open =		mon_bin_open,
	.llseek =	no_llseek,
	.read =		mon_bin_read,
	
	.poll =		mon_bin_poll,
	.ioctl =	mon_bin_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl =	mon_bin_compat_ioctl,
#endif
	.release =	mon_bin_release,
	.mmap =		mon_bin_mmap,
};

static int mon_bin_wait_event(struct file *file, struct mon_reader_bin *rp)
{
	DECLARE_WAITQUEUE(waita, current);
	unsigned long flags;

	add_wait_queue(&rp->b_wait, &waita);
	set_current_state(TASK_INTERRUPTIBLE);

	spin_lock_irqsave(&rp->b_lock, flags);
	while (MON_RING_EMPTY(rp)) {
		spin_unlock_irqrestore(&rp->b_lock, flags);

		if (file->f_flags & O_NONBLOCK) {
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&rp->b_wait, &waita);
			return -EWOULDBLOCK; 
		}
		schedule();
		if (signal_pending(current)) {
			remove_wait_queue(&rp->b_wait, &waita);
			return -EINTR;
		}
		set_current_state(TASK_INTERRUPTIBLE);

		spin_lock_irqsave(&rp->b_lock, flags);
	}
	spin_unlock_irqrestore(&rp->b_lock, flags);

	set_current_state(TASK_RUNNING);
	remove_wait_queue(&rp->b_wait, &waita);
	return 0;
}

static int mon_alloc_buff(struct mon_pgmap *map, int npages)
{
	int n;
	unsigned long vaddr;

	for (n = 0; n < npages; n++) {
		vaddr = get_zeroed_page(GFP_KERNEL);
		if (vaddr == 0) {
			while (n-- != 0)
				free_page((unsigned long) map[n].ptr);
			return -ENOMEM;
		}
		map[n].ptr = (unsigned char *) vaddr;
		map[n].pg = virt_to_page((void *) vaddr);
	}
	return 0;
}

static void mon_free_buff(struct mon_pgmap *map, int npages)
{
	int n;

	for (n = 0; n < npages; n++)
		free_page((unsigned long) map[n].ptr);
}

int mon_bin_add(struct mon_bus *mbus, const struct usb_bus *ubus)
{
	struct device *dev;
	unsigned minor = ubus? ubus->busnum: 0;

	if (minor >= MON_BIN_MAX_MINOR)
		return 0;

	dev = device_create(mon_bin_class, ubus ? ubus->controller : NULL,
			    MKDEV(MAJOR(mon_bin_dev0), minor), NULL,
			    "usbmon%d", minor);
	if (IS_ERR(dev))
		return 0;

	mbus->classdev = dev;
	return 1;
}

void mon_bin_del(struct mon_bus *mbus)
{
	device_destroy(mon_bin_class, mbus->classdev->devt);
}

int __init mon_bin_init(void)
{
	int rc;

	mon_bin_class = class_create(THIS_MODULE, "usbmon");
	if (IS_ERR(mon_bin_class)) {
		rc = PTR_ERR(mon_bin_class);
		goto err_class;
	}

	rc = alloc_chrdev_region(&mon_bin_dev0, 0, MON_BIN_MAX_MINOR, "usbmon");
	if (rc < 0)
		goto err_dev;

	cdev_init(&mon_bin_cdev, &mon_fops_binary);
	mon_bin_cdev.owner = THIS_MODULE;

	rc = cdev_add(&mon_bin_cdev, mon_bin_dev0, MON_BIN_MAX_MINOR);
	if (rc < 0)
		goto err_add;

	return 0;

err_add:
	unregister_chrdev_region(mon_bin_dev0, MON_BIN_MAX_MINOR);
err_dev:
	class_destroy(mon_bin_class);
err_class:
	return rc;
}

void mon_bin_exit(void)
{
	cdev_del(&mon_bin_cdev);
	unregister_chrdev_region(mon_bin_dev0, MON_BIN_MAX_MINOR);
	class_destroy(mon_bin_class);
}
