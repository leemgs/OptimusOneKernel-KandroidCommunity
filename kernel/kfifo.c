

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/kfifo.h>
#include <linux/log2.h>


struct kfifo *kfifo_init(unsigned char *buffer, unsigned int size,
			 gfp_t gfp_mask, spinlock_t *lock)
{
	struct kfifo *fifo;

	
	BUG_ON(!is_power_of_2(size));

	fifo = kmalloc(sizeof(struct kfifo), gfp_mask);
	if (!fifo)
		return ERR_PTR(-ENOMEM);

	fifo->buffer = buffer;
	fifo->size = size;
	fifo->in = fifo->out = 0;
	fifo->lock = lock;

	return fifo;
}
EXPORT_SYMBOL(kfifo_init);


struct kfifo *kfifo_alloc(unsigned int size, gfp_t gfp_mask, spinlock_t *lock)
{
	unsigned char *buffer;
	struct kfifo *ret;

	
	if (!is_power_of_2(size)) {
		BUG_ON(size > 0x80000000);
		size = roundup_pow_of_two(size);
	}

	buffer = kmalloc(size, gfp_mask);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	ret = kfifo_init(buffer, size, gfp_mask, lock);

	if (IS_ERR(ret))
		kfree(buffer);

	return ret;
}
EXPORT_SYMBOL(kfifo_alloc);


void kfifo_free(struct kfifo *fifo)
{
	kfree(fifo->buffer);
	kfree(fifo);
}
EXPORT_SYMBOL(kfifo_free);


unsigned int __kfifo_put(struct kfifo *fifo,
			const unsigned char *buffer, unsigned int len)
{
	unsigned int l;

	len = min(len, fifo->size - fifo->in + fifo->out);

	

	smp_mb();

	
	l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));
	memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);

	
	memcpy(fifo->buffer, buffer + l, len - l);

	

	smp_wmb();

	fifo->in += len;

	return len;
}
EXPORT_SYMBOL(__kfifo_put);


unsigned int __kfifo_get(struct kfifo *fifo,
			 unsigned char *buffer, unsigned int len)
{
	unsigned int l;

	len = min(len, fifo->in - fifo->out);

	

	smp_rmb();

	
	l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));
	memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);

	
	memcpy(buffer + l, fifo->buffer, len - l);

	

	smp_mb();

	fifo->out += len;

	return len;
}
EXPORT_SYMBOL(__kfifo_get);
