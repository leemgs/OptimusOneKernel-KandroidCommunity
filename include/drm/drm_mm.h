


#ifndef _DRM_MM_H_
#define _DRM_MM_H_


#include <linux/list.h>
#ifdef CONFIG_DEBUG_FS
#include <linux/seq_file.h>
#endif

struct drm_mm_node {
	struct list_head fl_entry;
	struct list_head ml_entry;
	int free;
	unsigned long start;
	unsigned long size;
	struct drm_mm *mm;
	void *private;
};

struct drm_mm {
	struct list_head fl_entry;
	struct list_head ml_entry;
	struct list_head unused_nodes;
	int num_unused;
	spinlock_t unused_lock;
};


extern struct drm_mm_node *drm_mm_get_block_generic(struct drm_mm_node *node,
						    unsigned long size,
						    unsigned alignment,
						    int atomic);
static inline struct drm_mm_node *drm_mm_get_block(struct drm_mm_node *parent,
						   unsigned long size,
						   unsigned alignment)
{
	return drm_mm_get_block_generic(parent, size, alignment, 0);
}
static inline struct drm_mm_node *drm_mm_get_block_atomic(struct drm_mm_node *parent,
							  unsigned long size,
							  unsigned alignment)
{
	return drm_mm_get_block_generic(parent, size, alignment, 1);
}
extern void drm_mm_put_block(struct drm_mm_node *cur);
extern struct drm_mm_node *drm_mm_search_free(const struct drm_mm *mm,
					      unsigned long size,
					      unsigned alignment,
					      int best_match);
extern int drm_mm_init(struct drm_mm *mm, unsigned long start,
		       unsigned long size);
extern void drm_mm_takedown(struct drm_mm *mm);
extern int drm_mm_clean(struct drm_mm *mm);
extern unsigned long drm_mm_tail_space(struct drm_mm *mm);
extern int drm_mm_remove_space_from_tail(struct drm_mm *mm,
					 unsigned long size);
extern int drm_mm_add_space_to_tail(struct drm_mm *mm,
				    unsigned long size, int atomic);
extern int drm_mm_pre_get(struct drm_mm *mm);

static inline struct drm_mm *drm_get_mm(struct drm_mm_node *block)
{
	return block->mm;
}

#ifdef CONFIG_DEBUG_FS
int drm_mm_dump_table(struct seq_file *m, struct drm_mm *mm);
#endif

#endif
