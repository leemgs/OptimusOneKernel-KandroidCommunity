

#ifndef VCM_TYPES_H
#define VCM_TYPES_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/genalloc.h>
#include <linux/vcm_alloc.h>
#include <linux/list.h>




#define VCM_ALIGN_SHIFT		10
#define VCM_ALIGN_MASK		0x1F
#define VCM_ALIGN_ATTR(order) 	(((order) & VCM_ALIGN_MASK) << VCM_ALIGN_SHIFT)

#define VCM_ALIGN_DEFAULT	0
#define VCM_ALIGN_4K		(VCM_ALIGN_ATTR(12))
#define VCM_ALIGN_8K		(VCM_ALIGN_ATTR(13))
#define VCM_ALIGN_16K		(VCM_ALIGN_ATTR(14))
#define VCM_ALIGN_32K		(VCM_ALIGN_ATTR(15))
#define VCM_ALIGN_64K		(VCM_ALIGN_ATTR(16))
#define VCM_ALIGN_128K		(VCM_ALIGN_ATTR(17))
#define VCM_ALIGN_256K		(VCM_ALIGN_ATTR(18))
#define VCM_ALIGN_512K		(VCM_ALIGN_ATTR(19))
#define VCM_ALIGN_1M		(VCM_ALIGN_ATTR(20))
#define VCM_ALIGN_2M		(VCM_ALIGN_ATTR(21))
#define VCM_ALIGN_4M		(VCM_ALIGN_ATTR(22))
#define VCM_ALIGN_8M		(VCM_ALIGN_ATTR(23))
#define VCM_ALIGN_16M		(VCM_ALIGN_ATTR(24))
#define VCM_ALIGN_32M		(VCM_ALIGN_ATTR(25))
#define VCM_ALIGN_64M		(VCM_ALIGN_ATTR(26))
#define VCM_ALIGN_128M		(VCM_ALIGN_ATTR(27))
#define VCM_ALIGN_256M		(VCM_ALIGN_ATTR(28))
#define VCM_ALIGN_512M		(VCM_ALIGN_ATTR(29))
#define VCM_ALIGN_1GB		(VCM_ALIGN_ATTR(30))


#define VCM_CACHE_POLICY	(0xF << 0)
#define VCM_READ		(1UL << 9)
#define VCM_WRITE		(1UL << 8)
#define VCM_EXECUTE		(1UL << 7)
#define VCM_USER		(1UL << 6)
#define VCM_SUPERVISOR		(1UL << 5)
#define VCM_SECURE		(1UL << 4)
#define VCM_NOTCACHED		(0UL << 0)
#define VCM_WB_WA		(1UL << 0)
#define VCM_WB_NWA		(2UL << 0)
#define VCM_WT			(3UL << 0)




#define VCM_4KB			(1UL << 5)
#define VCM_64KB		(1UL << 4)
#define VCM_1MB			(1UL << 3)
#define VCM_ALL			(1UL << 2)
#define VCM_PAGE_SEL_MASK       (0xFUL << 2)
#define VCM_PHYS_CONT		(1UL << 1)
#define VCM_COHERENT		(1UL << 0)


#define SHIFT_4KB               (12)

#define ALIGN_REQ_BYTES(attr) (1UL << (((attr & VCM_ALIGNMENT_MASK) >> 6) + 12))

#define SET_ALIGN_REQ_BYTES(attr, align) \
	((attr & ~VCM_ALIGNMENT_MASK) | ((align << 6) & VCM_ALIGNMENT_MASK))



#define VCM_SPLIT		(1UL << 3)
#define VCM_USE_LOW_BASE	(1UL << 2)
#define VCM_USE_HIGH_BASE	(1UL << 1)



#define VCM_PREBUILT_KERNEL		1
#define VCM_PREBUILT_USER		2


enum memtarget_t {
	VCM_START
};



enum memtype_t {
	VCM_INVALID,
	VCM_MEMTYPE_0,
	VCM_MEMTYPE_1,
	VCM_MEMTYPE_2,
};



typedef int (*vcm_handler)(size_t dev_id, void *data, void *fault_data);


enum vcm_type {
	VCM_DEVICE,
	VCM_EXT_KERNEL,
	VCM_EXT_USER,
	VCM_ONE_TO_ONE,
};



struct vcm {
	enum vcm_type type;

	size_t start_addr;
	size_t len;

	size_t dev_id; 

	
	struct gen_pool *pool;

	struct list_head res_head;

	
	struct list_head assoc_head;
};


struct avcm {
	struct vcm *vcm_id;
	size_t dev_id;
	uint32_t attr;

	struct list_head assoc_elm;

	int is_active; 
};


struct bound {
	struct vcm *vcm_id;
	size_t len;
};




struct physmem {
	enum memtype_t memtype;
	size_t len;
	uint32_t attr;

	struct phys_chunk alloc_head;

	
	int is_cont;
	struct res *res;
};


struct res {
	struct vcm *vcm_id;
	struct physmem *physmem_id;
	size_t len;
	uint32_t attr;

	
	size_t alignment_req;
	size_t aligned_len;
	unsigned long ptr;
	size_t aligned_ptr;

	struct list_head res_elm;


	
	struct vm_struct *vm_area;
	int mapped;
};

extern int chunk_sizes[NUM_CHUNK_SIZES];

#endif 
