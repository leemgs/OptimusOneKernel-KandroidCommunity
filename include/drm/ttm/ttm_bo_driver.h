

#ifndef _TTM_BO_DRIVER_H_
#define _TTM_BO_DRIVER_H_

#include "ttm/ttm_bo_api.h"
#include "ttm/ttm_memory.h"
#include "ttm/ttm_module.h"
#include "drm_mm.h"
#include "linux/workqueue.h"
#include "linux/fs.h"
#include "linux/spinlock.h"

struct ttm_backend;

struct ttm_backend_func {
	
	int (*populate) (struct ttm_backend *backend,
			 unsigned long num_pages, struct page **pages,
			 struct page *dummy_read_page);
	
	void (*clear) (struct ttm_backend *backend);

	
	int (*bind) (struct ttm_backend *backend, struct ttm_mem_reg *bo_mem);

	
	int (*unbind) (struct ttm_backend *backend);

	
	void (*destroy) (struct ttm_backend *backend);
};



struct ttm_backend {
	struct ttm_bo_device *bdev;
	uint32_t flags;
	struct ttm_backend_func *func;
};

#define TTM_PAGE_FLAG_VMALLOC         (1 << 0)
#define TTM_PAGE_FLAG_USER            (1 << 1)
#define TTM_PAGE_FLAG_USER_DIRTY      (1 << 2)
#define TTM_PAGE_FLAG_WRITE           (1 << 3)
#define TTM_PAGE_FLAG_SWAPPED         (1 << 4)
#define TTM_PAGE_FLAG_PERSISTANT_SWAP (1 << 5)
#define TTM_PAGE_FLAG_ZERO_ALLOC      (1 << 6)
#define TTM_PAGE_FLAG_DMA32           (1 << 7)

enum ttm_caching_state {
	tt_uncached,
	tt_wc,
	tt_cached
};



struct ttm_tt {
	struct page *dummy_read_page;
	struct page **pages;
	long first_himem_page;
	long last_lomem_page;
	uint32_t page_flags;
	unsigned long num_pages;
	struct ttm_bo_global *glob;
	struct ttm_backend *be;
	struct task_struct *tsk;
	unsigned long start;
	struct file *swap_storage;
	enum ttm_caching_state caching_state;
	enum {
		tt_bound,
		tt_unbound,
		tt_unpopulated,
	} state;
};

#define TTM_MEMTYPE_FLAG_FIXED         (1 << 0)	
#define TTM_MEMTYPE_FLAG_MAPPABLE      (1 << 1)	
#define TTM_MEMTYPE_FLAG_NEEDS_IOREMAP (1 << 2)	
#define TTM_MEMTYPE_FLAG_CMA           (1 << 3)	



struct ttm_mem_type_manager {

	

	bool has_type;
	bool use_type;
	uint32_t flags;
	unsigned long gpu_offset;
	unsigned long io_offset;
	unsigned long io_size;
	void *io_addr;
	uint64_t size;
	uint32_t available_caching;
	uint32_t default_caching;

	

	struct drm_mm manager;
	struct list_head lru;
};



struct ttm_bo_driver {
	const uint32_t *mem_type_prio;
	const uint32_t *mem_busy_prio;
	uint32_t num_mem_type_prio;
	uint32_t num_mem_busy_prio;

	

	struct ttm_backend *(*create_ttm_backend_entry)
	 (struct ttm_bo_device *bdev);

	

	int (*invalidate_caches) (struct ttm_bo_device *bdev, uint32_t flags);
	int (*init_mem_type) (struct ttm_bo_device *bdev, uint32_t type,
			      struct ttm_mem_type_manager *man);
	

	 uint32_t(*evict_flags) (struct ttm_buffer_object *bo);
	
	int (*move) (struct ttm_buffer_object *bo,
		     bool evict, bool interruptible,
		     bool no_wait, struct ttm_mem_reg *new_mem);

	
	int (*verify_access) (struct ttm_buffer_object *bo,
			      struct file *filp);

	

	bool (*sync_obj_signaled) (void *sync_obj, void *sync_arg);
	int (*sync_obj_wait) (void *sync_obj, void *sync_arg,
			      bool lazy, bool interruptible);
	int (*sync_obj_flush) (void *sync_obj, void *sync_arg);
	void (*sync_obj_unref) (void **sync_obj);
	void *(*sync_obj_ref) (void *sync_obj);

	
	void (*move_notify)(struct ttm_buffer_object *bo,
			    struct ttm_mem_reg *new_mem);
	
	void (*fault_reserve_notify)(struct ttm_buffer_object *bo);
};



struct ttm_bo_global_ref {
	struct ttm_global_reference ref;
	struct ttm_mem_global *mem_glob;
};



struct ttm_bo_global {

	

	struct kobject kobj;
	struct ttm_mem_global *mem_glob;
	struct page *dummy_read_page;
	struct ttm_mem_shrink shrink;
	size_t ttm_bo_extra_size;
	size_t ttm_bo_size;
	struct mutex device_list_mutex;
	spinlock_t lru_lock;

	
	struct list_head device_list;

	
	struct list_head swap_lru;

	
	atomic_t bo_count;
};


#define TTM_NUM_MEM_TYPES 8

#define TTM_BO_PRIV_FLAG_MOVING  0	
#define TTM_BO_PRIV_FLAG_MAX 1


struct ttm_bo_device {

	
	struct list_head device_list;
	struct ttm_bo_global *glob;
	struct ttm_bo_driver *driver;
	rwlock_t vm_lock;
	struct ttm_mem_type_manager man[TTM_NUM_MEM_TYPES];
	
	struct rb_root addr_space_rb;
	struct drm_mm addr_space_mm;

	
	struct list_head ddestroy;

	

	bool nice_mode;
	struct address_space *dev_mapping;

	

	struct delayed_work wq;

	bool need_dma32;
};



static inline uint32_t
ttm_flag_masked(uint32_t *old, uint32_t new, uint32_t mask)
{
	*old ^= (*old ^ new) & mask;
	return *old;
}


extern struct ttm_tt *ttm_tt_create(struct ttm_bo_device *bdev,
				    unsigned long size,
				    uint32_t page_flags,
				    struct page *dummy_read_page);



extern int ttm_tt_set_user(struct ttm_tt *ttm,
			   struct task_struct *tsk,
			   unsigned long start, unsigned long num_pages);


extern int ttm_tt_bind(struct ttm_tt *ttm, struct ttm_mem_reg *bo_mem);


extern void ttm_tt_destroy(struct ttm_tt *ttm);


extern void ttm_tt_unbind(struct ttm_tt *ttm);


extern struct page *ttm_tt_get_page(struct ttm_tt *ttm, int index);


extern void ttm_tt_cache_flush(struct page *pages[], unsigned long num_pages);


extern int ttm_tt_set_placement_caching(struct ttm_tt *ttm, uint32_t placement);
extern int ttm_tt_swapout(struct ttm_tt *ttm,
			  struct file *persistant_swap_storage);




extern bool ttm_mem_reg_is_pci(struct ttm_bo_device *bdev,
				   struct ttm_mem_reg *mem);


extern int ttm_bo_mem_space(struct ttm_buffer_object *bo,
			    uint32_t proposed_placement,
			    struct ttm_mem_reg *mem,
			    bool interruptible, bool no_wait);


extern int ttm_bo_wait_cpu(struct ttm_buffer_object *bo, bool no_wait);



extern int ttm_bo_pci_offset(struct ttm_bo_device *bdev,
			     struct ttm_mem_reg *mem,
			     unsigned long *bus_base,
			     unsigned long *bus_offset,
			     unsigned long *bus_size);

extern void ttm_bo_global_release(struct ttm_global_reference *ref);
extern int ttm_bo_global_init(struct ttm_global_reference *ref);

extern int ttm_bo_device_release(struct ttm_bo_device *bdev);


extern int ttm_bo_device_init(struct ttm_bo_device *bdev,
			      struct ttm_bo_global *glob,
			      struct ttm_bo_driver *driver,
			      uint64_t file_page_offset, bool need_dma32);


extern void ttm_bo_unmap_virtual(struct ttm_buffer_object *bo);


extern int ttm_bo_reserve(struct ttm_buffer_object *bo,
			  bool interruptible,
			  bool no_wait, bool use_sequence, uint32_t sequence);


extern void ttm_bo_unreserve(struct ttm_buffer_object *bo);


extern int ttm_bo_wait_unreserved(struct ttm_buffer_object *bo,
				  bool interruptible);


extern int ttm_bo_block_reservation(struct ttm_buffer_object *bo,
				    bool interruptible, bool no_wait);


extern void ttm_bo_unblock_reservation(struct ttm_buffer_object *bo);





extern int ttm_bo_move_ttm(struct ttm_buffer_object *bo,
			   bool evict, bool no_wait,
			   struct ttm_mem_reg *new_mem);



extern int ttm_bo_move_memcpy(struct ttm_buffer_object *bo,
			      bool evict,
			      bool no_wait, struct ttm_mem_reg *new_mem);


extern void ttm_bo_free_old_node(struct ttm_buffer_object *bo);



extern int ttm_bo_move_accel_cleanup(struct ttm_buffer_object *bo,
				     void *sync_obj,
				     void *sync_obj_arg,
				     bool evict, bool no_wait,
				     struct ttm_mem_reg *new_mem);

extern pgprot_t ttm_io_prot(enum ttm_caching_state c_state, pgprot_t tmp);

#if (defined(CONFIG_AGP) || (defined(CONFIG_AGP_MODULE) && defined(MODULE)))
#define TTM_HAS_AGP
#include <linux/agp_backend.h>


extern struct ttm_backend *ttm_agp_backend_init(struct ttm_bo_device *bdev,
						struct agp_bridge_data *bridge);
#endif

#endif
