

#ifndef BEISCSI_H
#define BEISCSI_H

#include <linux/pci.h>
#include <linux/if_vlan.h>

#define FW_VER_LEN 32

struct be_dma_mem {
	void *va;
	dma_addr_t dma;
	u32 size;
};

struct be_queue_info {
	struct be_dma_mem dma_mem;
	u16 len;
	u16 entry_size;		
	u16 id;
	u16 tail, head;
	bool created;
	atomic_t used;		
};

static inline u32 MODULO(u16 val, u16 limit)
{
	WARN_ON(limit & (limit - 1));
	return val & (limit - 1);
}

static inline void index_inc(u16 *index, u16 limit)
{
	*index = MODULO((*index + 1), limit);
}

static inline void *queue_head_node(struct be_queue_info *q)
{
	return q->dma_mem.va + q->head * q->entry_size;
}

static inline void *queue_tail_node(struct be_queue_info *q)
{
	return q->dma_mem.va + q->tail * q->entry_size;
}

static inline void queue_head_inc(struct be_queue_info *q)
{
	index_inc(&q->head, q->len);
}

static inline void queue_tail_inc(struct be_queue_info *q)
{
	index_inc(&q->tail, q->len);
}



struct be_eq_obj {
	struct be_queue_info q;
	char desc[32];

	
	bool enable_aic;
	u16 min_eqd;		
	u16 max_eqd;		
	u16 cur_eqd;		
};

struct be_mcc_obj {
	struct be_queue_info *q;
	struct be_queue_info *cq;
};

struct be_ctrl_info {
	u8 __iomem *csr;
	u8 __iomem *db;		
	u8 __iomem *pcicfg;	
	struct pci_dev *pdev;

	
	spinlock_t mbox_lock;	
	struct be_dma_mem mbox_mem;
	
	struct be_dma_mem mbox_mem_alloced;

	
	struct be_mcc_obj mcc_obj;
	spinlock_t mcc_lock;	
	spinlock_t mcc_cq_lock;

	
	void (*async_cb) (void *adapter, bool link_up);
	void *adapter_ctxt;
};

#include "be_cmds.h"

#define PAGE_SHIFT_4K 12
#define PAGE_SIZE_4K (1 << PAGE_SHIFT_4K)


#define PAGES_4K_SPANNED(_address, size) 				\
		((u32)((((size_t)(_address) & (PAGE_SIZE_4K - 1)) + 	\
			(size) + (PAGE_SIZE_4K - 1)) >> PAGE_SHIFT_4K))


#define OFFSET_IN_PAGE(addr)						\
		((size_t)(addr) & (PAGE_SIZE_4K-1))


#define AMAP_BIT_OFFSET(_struct, field)  				\
		(((size_t)&(((_struct *)0)->field))%32)


static inline u32 amap_mask(u32 bitsize)
{
	return (bitsize == 32 ? 0xFFFFFFFF : (1 << bitsize) - 1);
}

static inline void amap_set(void *ptr, u32 dw_offset, u32 mask,
					u32 offset, u32 value)
{
	u32 *dw = (u32 *) ptr + dw_offset;
	*dw &= ~(mask << offset);
	*dw |= (mask & value) << offset;
}

#define AMAP_SET_BITS(_struct, field, ptr, val)				\
		amap_set(ptr,						\
			offsetof(_struct, field)/32,			\
			amap_mask(sizeof(((_struct *)0)->field)),	\
			AMAP_BIT_OFFSET(_struct, field),		\
			val)

static inline u32 amap_get(void *ptr, u32 dw_offset, u32 mask, u32 offset)
{
	u32 *dw = ptr;
	return mask & (*(dw + dw_offset) >> offset);
}

#define AMAP_GET_BITS(_struct, field, ptr)				\
		amap_get(ptr,						\
			offsetof(_struct, field)/32,			\
			amap_mask(sizeof(((_struct *)0)->field)),	\
			AMAP_BIT_OFFSET(_struct, field))

#define be_dws_cpu_to_le(wrb, len) swap_dws(wrb, len)
#define be_dws_le_to_cpu(wrb, len) swap_dws(wrb, len)
static inline void swap_dws(void *wrb, int len)
{
#ifdef __BIG_ENDIAN
	u32 *dw = wrb;
	WARN_ON(len % 4);
	do {
		*dw = cpu_to_le32(*dw);
		dw++;
		len -= 4;
	} while (len);
#endif 
}

extern void beiscsi_cq_notify(struct be_ctrl_info *ctrl, u16 qid, bool arm,
			      u16 num_popped);

#endif 
