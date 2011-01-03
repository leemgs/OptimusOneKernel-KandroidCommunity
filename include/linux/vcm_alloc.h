

#ifndef VCM_ALLOC_H
#define VCM_ALLOC_H

#include <linux/list.h>

#define NUM_CHUNK_SIZES 3

enum chunk_size_idx {
	IDX_1M = 0,
	IDX_64K,
	IDX_4K
};

struct phys_chunk {
	struct list_head list;
	struct list_head allocated; 

	struct list_head refers_to;

	
	int pa;
	int size_idx;
};

int vcm_alloc_get_mem_size(void);
int vcm_alloc_blocks_avail(enum chunk_size_idx idx);
int vcm_alloc_get_num_chunks(void);
int vcm_alloc_all_blocks_avail(void);
int vcm_alloc_count_allocated(void);
void vcm_alloc_print_list(int just_allocated);
int vcm_alloc_idx_to_size(int idx);
int vcm_alloc_destroy(void);
int vcm_alloc_init(unsigned int set_base_pa);
int vcm_alloc_free_blocks(struct phys_chunk *alloc_head);
int vcm_alloc_num_blocks(int num,
			 enum chunk_size_idx idx, 
			 struct phys_chunk *alloc_head);
int vcm_alloc_max_munch(int len,
			struct phys_chunk *alloc_head);

#endif 
