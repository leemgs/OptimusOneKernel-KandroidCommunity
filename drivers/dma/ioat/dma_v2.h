
#ifndef IOATDMA_V2_H
#define IOATDMA_V2_H

#include <linux/dmaengine.h>
#include "dma.h"
#include "hw.h"


extern int ioat_pending_level;
extern int ioat_ring_alloc_order;


#define NULL_DESC_BUFFER_SIZE 1

#define IOAT_MAX_ORDER 16
#define ioat_get_alloc_order() \
	(min(ioat_ring_alloc_order, IOAT_MAX_ORDER))
#define ioat_get_max_alloc_order() \
	(min(ioat_ring_max_alloc_order, IOAT_MAX_ORDER))


struct ioat2_dma_chan {
	struct ioat_chan_common base;
	size_t xfercap_log;
	u16 head;
	u16 issued;
	u16 tail;
	u16 dmacount;
	u16 alloc_order;
	int pending;
	struct ioat_ring_ent **ring;
	spinlock_t ring_lock;
};

static inline struct ioat2_dma_chan *to_ioat2_chan(struct dma_chan *c)
{
	struct ioat_chan_common *chan = to_chan_common(c);

	return container_of(chan, struct ioat2_dma_chan, base);
}

static inline u16 ioat2_ring_mask(struct ioat2_dma_chan *ioat)
{
	return (1 << ioat->alloc_order) - 1;
}


static inline u16 ioat2_ring_active(struct ioat2_dma_chan *ioat)
{
	return (ioat->head - ioat->tail) & ioat2_ring_mask(ioat);
}


static inline u16 ioat2_ring_pending(struct ioat2_dma_chan *ioat)
{
	return (ioat->head - ioat->issued) & ioat2_ring_mask(ioat);
}

static inline u16 ioat2_ring_space(struct ioat2_dma_chan *ioat)
{
	u16 num_descs = ioat2_ring_mask(ioat) + 1;
	u16 active = ioat2_ring_active(ioat);

	BUG_ON(active > num_descs);

	return num_descs - active;
}


static inline u16 ioat2_desc_alloc(struct ioat2_dma_chan *ioat, u16 len)
{
	ioat->head += len;
	return ioat->head - len;
}

static inline u16 ioat2_xferlen_to_descs(struct ioat2_dma_chan *ioat, size_t len)
{
	u16 num_descs = len >> ioat->xfercap_log;

	num_descs += !!(len & ((1 << ioat->xfercap_log) - 1));
	return num_descs;
}



struct ioat_ring_ent {
	union {
		struct ioat_dma_descriptor *hw;
		struct ioat_fill_descriptor *fill;
		struct ioat_xor_descriptor *xor;
		struct ioat_xor_ext_descriptor *xor_ex;
		struct ioat_pq_descriptor *pq;
		struct ioat_pq_ext_descriptor *pq_ex;
		struct ioat_pq_update_descriptor *pqu;
		struct ioat_raw_descriptor *raw;
	};
	size_t len;
	struct dma_async_tx_descriptor txd;
	enum sum_check_flags *result;
	#ifdef DEBUG
	int id;
	#endif
};

static inline struct ioat_ring_ent *
ioat2_get_ring_ent(struct ioat2_dma_chan *ioat, u16 idx)
{
	return ioat->ring[idx & ioat2_ring_mask(ioat)];
}

static inline void ioat2_set_chainaddr(struct ioat2_dma_chan *ioat, u64 addr)
{
	struct ioat_chan_common *chan = &ioat->base;

	writel(addr & 0x00000000FFFFFFFF,
	       chan->reg_base + IOAT2_CHAINADDR_OFFSET_LOW);
	writel(addr >> 32,
	       chan->reg_base + IOAT2_CHAINADDR_OFFSET_HIGH);
}

int __devinit ioat2_dma_probe(struct ioatdma_device *dev, int dca);
int __devinit ioat3_dma_probe(struct ioatdma_device *dev, int dca);
struct dca_provider * __devinit ioat2_dca_init(struct pci_dev *pdev, void __iomem *iobase);
struct dca_provider * __devinit ioat3_dca_init(struct pci_dev *pdev, void __iomem *iobase);
int ioat2_alloc_and_lock(u16 *idx, struct ioat2_dma_chan *ioat, int num_descs);
int ioat2_enumerate_channels(struct ioatdma_device *device);
struct dma_async_tx_descriptor *
ioat2_dma_prep_memcpy_lock(struct dma_chan *c, dma_addr_t dma_dest,
			   dma_addr_t dma_src, size_t len, unsigned long flags);
void ioat2_issue_pending(struct dma_chan *chan);
int ioat2_alloc_chan_resources(struct dma_chan *c);
void ioat2_free_chan_resources(struct dma_chan *c);
enum dma_status ioat2_is_complete(struct dma_chan *c, dma_cookie_t cookie,
				  dma_cookie_t *done, dma_cookie_t *used);
void __ioat2_restart_chan(struct ioat2_dma_chan *ioat);
bool reshape_ring(struct ioat2_dma_chan *ioat, int order);
void __ioat2_issue_pending(struct ioat2_dma_chan *ioat);
void ioat2_cleanup_tasklet(unsigned long data);
void ioat2_timer_event(unsigned long data);
int ioat2_quiesce(struct ioat_chan_common *chan, unsigned long tmo);
int ioat2_reset_sync(struct ioat_chan_common *chan, unsigned long tmo);
extern struct kobj_type ioat2_ktype;
extern struct kmem_cache *ioat2_cache;
#endif 
