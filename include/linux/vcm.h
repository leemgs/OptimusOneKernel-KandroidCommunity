

#ifndef _VCM_H_
#define _VCM_H_



#include <linux/vcm_types.h>







struct vcm *vcm_create(size_t start_addr, size_t len);



struct vcm *vcm_create_from_prebuilt(size_t ext_vcm_id);



struct vcm *vcm_clone(struct vcm *vcm_id);



size_t vcm_get_start_addr(struct vcm *vcm_id);



size_t vcm_get_len(struct vcm *vcm_id);



int vcm_free(struct vcm *vcm_id);





struct res *vcm_reserve(struct vcm *vcm_id, size_t len, uint32_t attr);



struct res *vcm_reserve_at(enum memtarget_t memtarget, struct vcm *vcm_id,
		     size_t len, uint32_t attr);



struct vcm *vcm_get_vcm_from_res(struct res *res_id);



int vcm_unreserve(struct res *res_id);



size_t vcm_get_res_len(struct res *res_id);



int vcm_set_res_attr(struct res *res_id, uint32_t attr);



uint32_t vcm_get_res_attr(struct res *res_id);



size_t vcm_get_num_res(struct vcm *vcm_id);



struct res *vcm_get_next_res(struct vcm *vcm_id, struct res *res_id);



size_t vcm_res_copy(struct res *to, size_t to_off, struct res *from, size_t
		    from_off, size_t len);



size_t vcm_get_min_page_size(void);



int vcm_back(struct res *res_id, struct physmem *physmem_id);



int vcm_unback(struct res *res_id);



struct physmem *vcm_phys_alloc(enum memtype_t memtype, size_t len,
			       uint32_t attr);



int vcm_phys_free(struct physmem *physmem_id);



struct physmem *vcm_get_physmem_from_res(struct res *res_id);



enum memtype_t vcm_get_memtype_of_physalloc(struct physmem *physmem_id);





struct avcm *vcm_assoc(struct vcm *vcm_id, size_t dev_id, uint32_t attr);



int vcm_deassoc(struct avcm *avcm_id);



int vcm_set_assoc_attr(struct avcm *avcm_id, uint32_t attr);



uint32_t vcm_get_assoc_attr(struct avcm *avcm_id);



int vcm_activate(struct avcm *avcm_id);



int vcm_deactivate(struct avcm *avcm_id);



int vcm_is_active(struct avcm *avcm_id);






struct bound *vcm_create_bound(struct vcm *vcm_id, size_t len);



int vcm_free_bound(struct bound *bound_id);



struct res *vcm_reserve_from_bound(struct bound *bound_id, size_t len,
				   uint32_t attr);



size_t vcm_get_bound_start_addr(struct bound *bound_id);



size_t vcm_get_bound_len(struct bound *bound_id);






struct physmem *vcm_map_phys_addr(size_t phys, size_t len);



size_t vcm_get_next_phys_addr(struct physmem *physmem_id, size_t phys,
			      size_t *len);



size_t vcm_get_dev_addr(struct res *res_id);



struct res *vcm_get_res(size_t dev_addr, struct vcm *vcm_id);



size_t vcm_translate(size_t src_dev_id, struct vcm *src_vcm_id,
		     struct vcm *dst_vcm_id);



size_t vcm_get_phys_num_res(size_t phys);



struct res *vcm_get_next_phys_res(size_t phys, struct res *res_id, size_t *len);



size_t vcm_get_pgtbl_pa(struct vcm *vcm_id);



size_t vcm_get_cont_memtype_pa(enum memtype_t memtype);



size_t vcm_get_cont_memtype_len(enum memtype_t memtype);



size_t vcm_dev_addr_to_phys_addr(size_t dev_id, size_t dev_addr);





int vcm_hook(size_t dev_id, vcm_handler handler, void *data);






size_t vcm_hw_ver(size_t dev_id);




int vcm_sys_init(void);
int vcm_sys_destroy(void);

#endif 

