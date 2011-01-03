

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include "ring_sw.h"

static inline int __iio_init_sw_ring_buffer(struct iio_sw_ring_buffer *ring,
					    int bytes_per_datum, int length)
{
	if ((length == 0) || (bytes_per_datum == 0))
		return -EINVAL;

	__iio_init_ring_buffer(&ring->buf, bytes_per_datum, length);
	ring->use_lock = __SPIN_LOCK_UNLOCKED((ring)->use_lock);
	ring->data = kmalloc(length*ring->buf.bpd, GFP_KERNEL);
	ring->read_p = 0;
	ring->write_p = 0;
	ring->last_written_p = 0;
	ring->half_p = 0;
	return ring->data ? 0 : -ENOMEM;
}

static inline void __iio_free_sw_ring_buffer(struct iio_sw_ring_buffer *ring)
{
	kfree(ring->data);
}

void iio_mark_sw_rb_in_use(struct iio_ring_buffer *r)
{
	struct iio_sw_ring_buffer *ring = iio_to_sw_ring(r);
	spin_lock(&ring->use_lock);
	ring->use_count++;
	spin_unlock(&ring->use_lock);
}
EXPORT_SYMBOL(iio_mark_sw_rb_in_use);

void iio_unmark_sw_rb_in_use(struct iio_ring_buffer *r)
{
	struct iio_sw_ring_buffer *ring = iio_to_sw_ring(r);
	spin_lock(&ring->use_lock);
	ring->use_count--;
	spin_unlock(&ring->use_lock);
}
EXPORT_SYMBOL(iio_unmark_sw_rb_in_use);






int iio_store_to_sw_ring(struct iio_sw_ring_buffer *ring,
			 unsigned char *data,
			 s64 timestamp)
{
	int ret = 0;
	int code;
	unsigned char *temp_ptr, *change_test_ptr;

	
	if (unlikely(ring->write_p == 0)) {
		ring->write_p = ring->data;
		
		ring->half_p = ring->data - ring->buf.length*ring->buf.bpd/2;
	}
	
	memcpy(ring->write_p, data, ring->buf.bpd);
	barrier();
	
	ring->last_written_p = ring->write_p;
	barrier();
	
	temp_ptr = ring->write_p + ring->buf.bpd;
	
	if (temp_ptr == ring->data + ring->buf.length*ring->buf.bpd)
		temp_ptr = ring->data;
	
	ring->write_p = temp_ptr;

	if (ring->read_p == 0)
		ring->read_p = ring->data;
	
	
	else if (ring->write_p == ring->read_p) {
		change_test_ptr = ring->read_p;
		temp_ptr = change_test_ptr + ring->buf.bpd;
		if (temp_ptr
		    == ring->data + ring->buf.length*ring->buf.bpd) {
			temp_ptr = ring->data;
		}
		
		if (change_test_ptr == ring->read_p)
			ring->read_p = temp_ptr;

		spin_lock(&ring->buf.shared_ev_pointer.lock);

		ret = iio_push_or_escallate_ring_event(&ring->buf,
						       IIO_EVENT_CODE_RING_100_FULL,
						       timestamp);
		spin_unlock(&ring->buf.shared_ev_pointer.lock);
		if (ret)
			goto error_ret;
	}
	
	
	
	ring->half_p += ring->buf.bpd;
	if (ring->half_p == ring->data + ring->buf.length*ring->buf.bpd)
		ring->half_p = ring->data;
	if (ring->half_p == ring->read_p) {
		spin_lock(&ring->buf.shared_ev_pointer.lock);
		code = IIO_EVENT_CODE_RING_50_FULL;
		ret = __iio_push_event(&ring->buf.ev_int,
				       code,
				       timestamp,
				       &ring->buf.shared_ev_pointer);
		spin_unlock(&ring->buf.shared_ev_pointer.lock);
	}
error_ret:
	return ret;
}

int iio_rip_sw_rb(struct iio_ring_buffer *r,
		  size_t count, u8 **data, int *dead_offset)
{
	struct iio_sw_ring_buffer *ring = iio_to_sw_ring(r);

	u8 *initial_read_p, *initial_write_p, *current_read_p, *end_read_p;
	int ret, max_copied;
	int bytes_to_rip;

	
	if (count % ring->buf.bpd) {
		ret = -EINVAL;
		printk(KERN_INFO "Ring buffer read request not whole number of"
		       "samples: Request bytes %zd, Current bpd %d\n",
		       count, ring->buf.bpd);
		goto error_ret;
	}
	
	bytes_to_rip = min((size_t)(ring->buf.bpd*ring->buf.length), count);

	*data = kmalloc(bytes_to_rip, GFP_KERNEL);
	if (*data == NULL) {
		ret = -ENOMEM;
		goto error_ret;
	}

	
	initial_read_p = ring->read_p;
	if (unlikely(initial_read_p == 0)) { 
		ret = 0;
		goto error_free_data_cpy;
	}

	initial_write_p = ring->write_p;

	
	while ((initial_read_p != ring->read_p)
	       || (initial_write_p != ring->write_p)) {
		initial_read_p = ring->read_p;
		initial_write_p = ring->write_p;
	}
	if (initial_write_p == initial_read_p) {
		
		ret = 0;
		goto error_free_data_cpy;
	}

	if (initial_write_p >= initial_read_p + bytes_to_rip) {
		
		max_copied = bytes_to_rip;
		memcpy(*data, initial_read_p, max_copied);
		end_read_p = initial_read_p + max_copied;
	} else if (initial_write_p > initial_read_p) {
		
		max_copied = initial_write_p - initial_read_p;
		memcpy(*data, initial_read_p, max_copied);
		end_read_p = initial_write_p;
	} else {
		
		max_copied = ring->data
			+ ring->buf.length*ring->buf.bpd - initial_read_p;
		memcpy(*data, initial_read_p, max_copied);
		
		if (max_copied == bytes_to_rip)
			end_read_p = ring->data;
		else if (initial_write_p
			 > ring->data + bytes_to_rip - max_copied) {
			
			memcpy(*data + max_copied, ring->data,
			       bytes_to_rip - max_copied);
			max_copied = bytes_to_rip;
			end_read_p = ring->data + (bytes_to_rip - max_copied);
		} else {  
			memcpy(*data + max_copied, ring->data,
			       initial_write_p - ring->data);
			max_copied += initial_write_p - ring->data;
			end_read_p = initial_write_p;
		}
	}
	
	current_read_p = ring->read_p;

	if (initial_read_p <= current_read_p)
		*dead_offset = current_read_p - initial_read_p;
	else
		*dead_offset = ring->buf.length*ring->buf.bpd
			- (initial_read_p - current_read_p);

	
	
	if (max_copied - *dead_offset < 0) {
		ret = 0;
		goto error_free_data_cpy;
	}

	
	
	

	while (ring->read_p != end_read_p)
		ring->read_p = end_read_p;

	return max_copied - *dead_offset;

error_free_data_cpy:
	kfree(*data);
error_ret:
	return ret;
}
EXPORT_SYMBOL(iio_rip_sw_rb);

int iio_store_to_sw_rb(struct iio_ring_buffer *r, u8 *data, s64 timestamp)
{
	struct iio_sw_ring_buffer *ring = iio_to_sw_ring(r);
	return iio_store_to_sw_ring(ring, data, timestamp);
}
EXPORT_SYMBOL(iio_store_to_sw_rb);

int iio_read_last_from_sw_ring(struct iio_sw_ring_buffer *ring,
			       unsigned char *data)
{
	unsigned char *last_written_p_copy;

	iio_mark_sw_rb_in_use(&ring->buf);
again:
	barrier();
	last_written_p_copy = ring->last_written_p;
	barrier(); 
	
	if (last_written_p_copy == 0)
		return -EAGAIN;
	memcpy(data, last_written_p_copy, ring->buf.bpd);

	if (unlikely(ring->last_written_p >= last_written_p_copy))
		goto again;

	iio_unmark_sw_rb_in_use(&ring->buf);
	return 0;
}

int iio_read_last_from_sw_rb(struct iio_ring_buffer *r,
			     unsigned char *data)
{
	return iio_read_last_from_sw_ring(iio_to_sw_ring(r), data);
}
EXPORT_SYMBOL(iio_read_last_from_sw_rb);

int iio_request_update_sw_rb(struct iio_ring_buffer *r)
{
	int ret = 0;
	struct iio_sw_ring_buffer *ring = iio_to_sw_ring(r);

	spin_lock(&ring->use_lock);
	if (!ring->update_needed)
		goto error_ret;
	if (ring->use_count) {
		ret = -EAGAIN;
		goto error_ret;
	}
	__iio_free_sw_ring_buffer(ring);
	ret = __iio_init_sw_ring_buffer(ring, ring->buf.bpd, ring->buf.length);
error_ret:
	spin_unlock(&ring->use_lock);
	return ret;
}
EXPORT_SYMBOL(iio_request_update_sw_rb);

int iio_get_bpd_sw_rb(struct iio_ring_buffer *r)
{
	struct iio_sw_ring_buffer *ring = iio_to_sw_ring(r);
	return ring->buf.bpd;
}
EXPORT_SYMBOL(iio_get_bpd_sw_rb);

int iio_set_bpd_sw_rb(struct iio_ring_buffer *r, size_t bpd)
{
	if (r->bpd != bpd) {
		r->bpd = bpd;
		if (r->access.mark_param_change)
			r->access.mark_param_change(r);
	}
	return 0;
}
EXPORT_SYMBOL(iio_set_bpd_sw_rb);

int iio_get_length_sw_rb(struct iio_ring_buffer *r)
{
	return r->length;
}
EXPORT_SYMBOL(iio_get_length_sw_rb);

int iio_set_length_sw_rb(struct iio_ring_buffer *r, int length)
{
	if (r->length != length) {
		r->length = length;
		if (r->access.mark_param_change)
			r->access.mark_param_change(r);
	}
	return 0;
}
EXPORT_SYMBOL(iio_set_length_sw_rb);

int iio_mark_update_needed_sw_rb(struct iio_ring_buffer *r)
{
	struct iio_sw_ring_buffer *ring = iio_to_sw_ring(r);
	ring->update_needed = true;
	return 0;
}
EXPORT_SYMBOL(iio_mark_update_needed_sw_rb);

static void iio_sw_rb_release(struct device *dev)
{
	struct iio_ring_buffer *r = to_iio_ring_buffer(dev);
	kfree(iio_to_sw_ring(r));
}

static IIO_RING_ENABLE_ATTR;
static IIO_RING_BPS_ATTR;
static IIO_RING_LENGTH_ATTR;


static struct attribute *iio_ring_attributes[] = {
	&dev_attr_length.attr,
	&dev_attr_bps.attr,
	&dev_attr_ring_enable.attr,
	NULL,
};

static struct attribute_group iio_ring_attribute_group = {
	.attrs = iio_ring_attributes,
};

static const struct attribute_group *iio_ring_attribute_groups[] = {
	&iio_ring_attribute_group,
	NULL
};

static struct device_type iio_sw_ring_type = {
	.release = iio_sw_rb_release,
	.groups = iio_ring_attribute_groups,
};

struct iio_ring_buffer *iio_sw_rb_allocate(struct iio_dev *indio_dev)
{
	struct iio_ring_buffer *buf;
	struct iio_sw_ring_buffer *ring;

	ring = kzalloc(sizeof *ring, GFP_KERNEL);
	if (!ring)
		return 0;
	buf = &ring->buf;

	iio_ring_buffer_init(buf, indio_dev);
	buf->dev.type = &iio_sw_ring_type;
	device_initialize(&buf->dev);
	buf->dev.parent = &indio_dev->dev;
	buf->dev.class = &iio_class;
	dev_set_drvdata(&buf->dev, (void *)buf);

	return buf;
}
EXPORT_SYMBOL(iio_sw_rb_allocate);

void iio_sw_rb_free(struct iio_ring_buffer *r)
{
	if (r)
		iio_put_ring_buffer(r);
}
EXPORT_SYMBOL(iio_sw_rb_free);
MODULE_DESCRIPTION("Industrialio I/O software ring buffer");
MODULE_LICENSE("GPL");
