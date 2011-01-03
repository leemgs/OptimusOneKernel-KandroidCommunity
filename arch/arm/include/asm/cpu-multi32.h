
#include <asm/page.h>

struct mm_struct;


extern struct processor {
	
	void (*_data_abort)(unsigned long pc);
	
	unsigned long (*_prefetch_abort)(unsigned long lr);
	
	void (*_proc_init)(void);
	
	void (*_proc_fin)(void);
	
	void (*reset)(unsigned long addr) __attribute__((noreturn));
	
	int (*_do_idle)(void);
	
	
	void (*dcache_clean_area)(void *addr, int size);

	
	void (*switch_mm)(unsigned long pgd_phys, struct mm_struct *mm);
	
	void (*set_pte_ext)(pte_t *ptep, pte_t pte, unsigned int ext);
} processor;

#define cpu_proc_init()			processor._proc_init()
#define cpu_proc_fin()			processor._proc_fin()
#define cpu_reset(addr)			processor.reset(addr)
#define cpu_do_idle()			processor._do_idle()
#define cpu_dcache_clean_area(addr,sz)	processor.dcache_clean_area(addr,sz)
#define cpu_set_pte_ext(ptep,pte,ext)	processor.set_pte_ext(ptep,pte,ext)
#define cpu_do_switch_mm(pgd,mm)	processor.switch_mm(pgd,mm)
