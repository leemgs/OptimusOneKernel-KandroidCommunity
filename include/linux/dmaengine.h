
#ifndef DMAENGINE_H
#define DMAENGINE_H

#include <linux/device.h>
#include <linux/uio.h>
#include <linux/dma-mapping.h>


typedef s32 dma_cookie_t;

#define dma_submit_error(cookie) ((cookie) < 0 ? 1 : 0)


enum dma_status {
	DMA_SUCCESS,
	DMA_IN_PROGRESS,
	DMA_ERROR,
};


enum dma_transaction_type {
	DMA_MEMCPY,
	DMA_XOR,
	DMA_PQ,
	DMA_XOR_VAL,
	DMA_PQ_VAL,
	DMA_MEMSET,
	DMA_INTERRUPT,
	DMA_PRIVATE,
	DMA_ASYNC_TX,
	DMA_SLAVE,
};


#define DMA_TX_TYPE_END (DMA_SLAVE + 1)



enum dma_ctrl_flags {
	DMA_PREP_INTERRUPT = (1 << 0),
	DMA_CTRL_ACK = (1 << 1),
	DMA_COMPL_SKIP_SRC_UNMAP = (1 << 2),
	DMA_COMPL_SKIP_DEST_UNMAP = (1 << 3),
	DMA_COMPL_SRC_UNMAP_SINGLE = (1 << 4),
	DMA_COMPL_DEST_UNMAP_SINGLE = (1 << 5),
	DMA_PREP_PQ_DISABLE_P = (1 << 6),
	DMA_PREP_PQ_DISABLE_Q = (1 << 7),
	DMA_PREP_CONTINUE = (1 << 8),
	DMA_PREP_FENCE = (1 << 9),
};


enum sum_check_bits {
	SUM_CHECK_P = 0,
	SUM_CHECK_Q = 1,
};


enum sum_check_flags {
	SUM_CHECK_P_RESULT = (1 << SUM_CHECK_P),
	SUM_CHECK_Q_RESULT = (1 << SUM_CHECK_Q),
};



typedef struct { DECLARE_BITMAP(bits, DMA_TX_TYPE_END); } dma_cap_mask_t;



struct dma_chan_percpu {
	
	unsigned long memcpy_count;
	unsigned long bytes_transferred;
};


struct dma_chan {
	struct dma_device *device;
	dma_cookie_t cookie;

	
	int chan_id;
	struct dma_chan_dev *dev;

	struct list_head device_node;
	struct dma_chan_percpu *local;
	int client_count;
	int table_count;
	void *private;
};


struct dma_chan_dev {
	struct dma_chan *chan;
	struct device device;
	int dev_id;
	atomic_t *idr_ref;
};

static inline const char *dma_chan_name(struct dma_chan *chan)
{
	return dev_name(&chan->dev->device);
}

void dma_chan_cleanup(struct kref *kref);


typedef bool (*dma_filter_fn)(struct dma_chan *chan, void *filter_param);

typedef void (*dma_async_tx_callback)(void *dma_async_param);

struct dma_async_tx_descriptor {
	dma_cookie_t cookie;
	enum dma_ctrl_flags flags; 
	dma_addr_t phys;
	struct dma_chan *chan;
	dma_cookie_t (*tx_submit)(struct dma_async_tx_descriptor *tx);
	dma_async_tx_callback callback;
	void *callback_param;
	struct dma_async_tx_descriptor *next;
	struct dma_async_tx_descriptor *parent;
	spinlock_t lock;
};


struct dma_device {

	unsigned int chancnt;
	unsigned int privatecnt;
	struct list_head channels;
	struct list_head global_node;
	dma_cap_mask_t  cap_mask;
	unsigned short max_xor;
	unsigned short max_pq;
	u8 copy_align;
	u8 xor_align;
	u8 pq_align;
	u8 fill_align;
	#define DMA_HAS_PQ_CONTINUE (1 << 15)

	int dev_id;
	struct device *dev;

	int (*device_alloc_chan_resources)(struct dma_chan *chan);
	void (*device_free_chan_resources)(struct dma_chan *chan);

	struct dma_async_tx_descriptor *(*device_prep_dma_memcpy)(
		struct dma_chan *chan, dma_addr_t dest, dma_addr_t src,
		size_t len, unsigned long flags);
	struct dma_async_tx_descriptor *(*device_prep_dma_xor)(
		struct dma_chan *chan, dma_addr_t dest, dma_addr_t *src,
		unsigned int src_cnt, size_t len, unsigned long flags);
	struct dma_async_tx_descriptor *(*device_prep_dma_xor_val)(
		struct dma_chan *chan, dma_addr_t *src,	unsigned int src_cnt,
		size_t len, enum sum_check_flags *result, unsigned long flags);
	struct dma_async_tx_descriptor *(*device_prep_dma_pq)(
		struct dma_chan *chan, dma_addr_t *dst, dma_addr_t *src,
		unsigned int src_cnt, const unsigned char *scf,
		size_t len, unsigned long flags);
	struct dma_async_tx_descriptor *(*device_prep_dma_pq_val)(
		struct dma_chan *chan, dma_addr_t *pq, dma_addr_t *src,
		unsigned int src_cnt, const unsigned char *scf, size_t len,
		enum sum_check_flags *pqres, unsigned long flags);
	struct dma_async_tx_descriptor *(*device_prep_dma_memset)(
		struct dma_chan *chan, dma_addr_t dest, int value, size_t len,
		unsigned long flags);
	struct dma_async_tx_descriptor *(*device_prep_dma_interrupt)(
		struct dma_chan *chan, unsigned long flags);

	struct dma_async_tx_descriptor *(*device_prep_slave_sg)(
		struct dma_chan *chan, struct scatterlist *sgl,
		unsigned int sg_len, enum dma_data_direction direction,
		unsigned long flags);
	void (*device_terminate_all)(struct dma_chan *chan);

	enum dma_status (*device_is_tx_complete)(struct dma_chan *chan,
			dma_cookie_t cookie, dma_cookie_t *last,
			dma_cookie_t *used);
	void (*device_issue_pending)(struct dma_chan *chan);
};

static inline bool dmaengine_check_align(u8 align, size_t off1, size_t off2, size_t len)
{
	size_t mask;

	if (!align)
		return true;
	mask = (1 << align) - 1;
	if (mask & (off1 | off2 | len))
		return false;
	return true;
}

static inline bool is_dma_copy_aligned(struct dma_device *dev, size_t off1,
				       size_t off2, size_t len)
{
	return dmaengine_check_align(dev->copy_align, off1, off2, len);
}

static inline bool is_dma_xor_aligned(struct dma_device *dev, size_t off1,
				      size_t off2, size_t len)
{
	return dmaengine_check_align(dev->xor_align, off1, off2, len);
}

static inline bool is_dma_pq_aligned(struct dma_device *dev, size_t off1,
				     size_t off2, size_t len)
{
	return dmaengine_check_align(dev->pq_align, off1, off2, len);
}

static inline bool is_dma_fill_aligned(struct dma_device *dev, size_t off1,
				       size_t off2, size_t len)
{
	return dmaengine_check_align(dev->fill_align, off1, off2, len);
}

static inline void
dma_set_maxpq(struct dma_device *dma, int maxpq, int has_pq_continue)
{
	dma->max_pq = maxpq;
	if (has_pq_continue)
		dma->max_pq |= DMA_HAS_PQ_CONTINUE;
}

static inline bool dmaf_continue(enum dma_ctrl_flags flags)
{
	return (flags & DMA_PREP_CONTINUE) == DMA_PREP_CONTINUE;
}

static inline bool dmaf_p_disabled_continue(enum dma_ctrl_flags flags)
{
	enum dma_ctrl_flags mask = DMA_PREP_CONTINUE | DMA_PREP_PQ_DISABLE_P;

	return (flags & mask) == mask;
}

static inline bool dma_dev_has_pq_continue(struct dma_device *dma)
{
	return (dma->max_pq & DMA_HAS_PQ_CONTINUE) == DMA_HAS_PQ_CONTINUE;
}

static unsigned short dma_dev_to_maxpq(struct dma_device *dma)
{
	return dma->max_pq & ~DMA_HAS_PQ_CONTINUE;
}


static inline int dma_maxpq(struct dma_device *dma, enum dma_ctrl_flags flags)
{
	if (dma_dev_has_pq_continue(dma) || !dmaf_continue(flags))
		return dma_dev_to_maxpq(dma);
	else if (dmaf_p_disabled_continue(flags))
		return dma_dev_to_maxpq(dma) - 1;
	else if (dmaf_continue(flags))
		return dma_dev_to_maxpq(dma) - 3;
	BUG();
}



#ifdef CONFIG_DMA_ENGINE
void dmaengine_get(void);
void dmaengine_put(void);
#else
static inline void dmaengine_get(void)
{
}
static inline void dmaengine_put(void)
{
}
#endif

#ifdef CONFIG_NET_DMA
#define net_dmaengine_get()	dmaengine_get()
#define net_dmaengine_put()	dmaengine_put()
#else
static inline void net_dmaengine_get(void)
{
}
static inline void net_dmaengine_put(void)
{
}
#endif

#ifdef CONFIG_ASYNC_TX_DMA
#define async_dmaengine_get()	dmaengine_get()
#define async_dmaengine_put()	dmaengine_put()
#ifdef CONFIG_ASYNC_TX_DISABLE_CHANNEL_SWITCH
#define async_dma_find_channel(type) dma_find_channel(DMA_ASYNC_TX)
#else
#define async_dma_find_channel(type) dma_find_channel(type)
#endif 
#else
static inline void async_dmaengine_get(void)
{
}
static inline void async_dmaengine_put(void)
{
}
static inline struct dma_chan *
async_dma_find_channel(enum dma_transaction_type type)
{
	return NULL;
}
#endif 

dma_cookie_t dma_async_memcpy_buf_to_buf(struct dma_chan *chan,
	void *dest, void *src, size_t len);
dma_cookie_t dma_async_memcpy_buf_to_pg(struct dma_chan *chan,
	struct page *page, unsigned int offset, void *kdata, size_t len);
dma_cookie_t dma_async_memcpy_pg_to_pg(struct dma_chan *chan,
	struct page *dest_pg, unsigned int dest_off, struct page *src_pg,
	unsigned int src_off, size_t len);
void dma_async_tx_descriptor_init(struct dma_async_tx_descriptor *tx,
	struct dma_chan *chan);

static inline void async_tx_ack(struct dma_async_tx_descriptor *tx)
{
	tx->flags |= DMA_CTRL_ACK;
}

static inline void async_tx_clear_ack(struct dma_async_tx_descriptor *tx)
{
	tx->flags &= ~DMA_CTRL_ACK;
}

static inline bool async_tx_test_ack(struct dma_async_tx_descriptor *tx)
{
	return (tx->flags & DMA_CTRL_ACK) == DMA_CTRL_ACK;
}

#define first_dma_cap(mask) __first_dma_cap(&(mask))
static inline int __first_dma_cap(const dma_cap_mask_t *srcp)
{
	return min_t(int, DMA_TX_TYPE_END,
		find_first_bit(srcp->bits, DMA_TX_TYPE_END));
}

#define next_dma_cap(n, mask) __next_dma_cap((n), &(mask))
static inline int __next_dma_cap(int n, const dma_cap_mask_t *srcp)
{
	return min_t(int, DMA_TX_TYPE_END,
		find_next_bit(srcp->bits, DMA_TX_TYPE_END, n+1));
}

#define dma_cap_set(tx, mask) __dma_cap_set((tx), &(mask))
static inline void
__dma_cap_set(enum dma_transaction_type tx_type, dma_cap_mask_t *dstp)
{
	set_bit(tx_type, dstp->bits);
}

#define dma_cap_clear(tx, mask) __dma_cap_clear((tx), &(mask))
static inline void
__dma_cap_clear(enum dma_transaction_type tx_type, dma_cap_mask_t *dstp)
{
	clear_bit(tx_type, dstp->bits);
}

#define dma_cap_zero(mask) __dma_cap_zero(&(mask))
static inline void __dma_cap_zero(dma_cap_mask_t *dstp)
{
	bitmap_zero(dstp->bits, DMA_TX_TYPE_END);
}

#define dma_has_cap(tx, mask) __dma_has_cap((tx), &(mask))
static inline int
__dma_has_cap(enum dma_transaction_type tx_type, dma_cap_mask_t *srcp)
{
	return test_bit(tx_type, srcp->bits);
}

#define for_each_dma_cap_mask(cap, mask) \
	for ((cap) = first_dma_cap(mask);	\
		(cap) < DMA_TX_TYPE_END;	\
		(cap) = next_dma_cap((cap), (mask)))


static inline void dma_async_issue_pending(struct dma_chan *chan)
{
	chan->device->device_issue_pending(chan);
}

#define dma_async_memcpy_issue_pending(chan) dma_async_issue_pending(chan)


static inline enum dma_status dma_async_is_tx_complete(struct dma_chan *chan,
	dma_cookie_t cookie, dma_cookie_t *last, dma_cookie_t *used)
{
	return chan->device->device_is_tx_complete(chan, cookie, last, used);
}

#define dma_async_memcpy_complete(chan, cookie, last, used)\
	dma_async_is_tx_complete(chan, cookie, last, used)


static inline enum dma_status dma_async_is_complete(dma_cookie_t cookie,
			dma_cookie_t last_complete, dma_cookie_t last_used)
{
	if (last_complete <= last_used) {
		if ((cookie <= last_complete) || (cookie > last_used))
			return DMA_SUCCESS;
	} else {
		if ((cookie <= last_complete) && (cookie > last_used))
			return DMA_SUCCESS;
	}
	return DMA_IN_PROGRESS;
}

enum dma_status dma_sync_wait(struct dma_chan *chan, dma_cookie_t cookie);
#ifdef CONFIG_DMA_ENGINE
enum dma_status dma_wait_for_async_tx(struct dma_async_tx_descriptor *tx);
void dma_issue_pending_all(void);
#else
static inline enum dma_status dma_wait_for_async_tx(struct dma_async_tx_descriptor *tx)
{
	return DMA_SUCCESS;
}
static inline void dma_issue_pending_all(void)
{
	do { } while (0);
}
#endif



int dma_async_device_register(struct dma_device *device);
void dma_async_device_unregister(struct dma_device *device);
void dma_run_dependencies(struct dma_async_tx_descriptor *tx);
struct dma_chan *dma_find_channel(enum dma_transaction_type tx_type);
#define dma_request_channel(mask, x, y) __dma_request_channel(&(mask), x, y)
struct dma_chan *__dma_request_channel(dma_cap_mask_t *mask, dma_filter_fn fn, void *fn_param);
void dma_release_channel(struct dma_chan *chan);



struct dma_page_list {
	char __user *base_address;
	int nr_pages;
	struct page **pages;
};

struct dma_pinned_list {
	int nr_iovecs;
	struct dma_page_list page_list[0];
};

struct dma_pinned_list *dma_pin_iovec_pages(struct iovec *iov, size_t len);
void dma_unpin_iovec_pages(struct dma_pinned_list* pinned_list);

dma_cookie_t dma_memcpy_to_iovec(struct dma_chan *chan, struct iovec *iov,
	struct dma_pinned_list *pinned_list, unsigned char *kdata, size_t len);
dma_cookie_t dma_memcpy_pg_to_iovec(struct dma_chan *chan, struct iovec *iov,
	struct dma_pinned_list *pinned_list, struct page *page,
	unsigned int offset, size_t len);

#endif 
