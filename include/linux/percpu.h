#ifndef __LINUX_PERCPU_H
#define __LINUX_PERCPU_H

#include <linux/preempt.h>
#include <linux/slab.h> 
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <linux/pfn.h>

#include <asm/percpu.h>


#ifdef CONFIG_MODULES
#define PERCPU_MODULE_RESERVE		(8 << 10)
#else
#define PERCPU_MODULE_RESERVE		0
#endif

#ifndef PERCPU_ENOUGH_ROOM
#define PERCPU_ENOUGH_ROOM						\
	(ALIGN(__per_cpu_end - __per_cpu_start, SMP_CACHE_BYTES) +	\
	 PERCPU_MODULE_RESERVE)
#endif


#define get_cpu_var(var) (*({				\
	extern int simple_identifier_##var(void);	\
	preempt_disable();				\
	&__get_cpu_var(var); }))
#define put_cpu_var(var) preempt_enable()

#ifdef CONFIG_SMP

#ifndef CONFIG_HAVE_LEGACY_PER_CPU_AREA


#define PCPU_MIN_UNIT_SIZE		PFN_ALIGN(64 << 10)


#if BITS_PER_LONG > 32
#define PERCPU_DYNAMIC_RESERVE		(20 << 10)
#else
#define PERCPU_DYNAMIC_RESERVE		(12 << 10)
#endif

extern void *pcpu_base_addr;
extern const unsigned long *pcpu_unit_offsets;

struct pcpu_group_info {
	int			nr_units;	
	unsigned long		base_offset;	
	unsigned int		*cpu_map;	
};

struct pcpu_alloc_info {
	size_t			static_size;
	size_t			reserved_size;
	size_t			dyn_size;
	size_t			unit_size;
	size_t			atom_size;
	size_t			alloc_size;
	size_t			__ai_size;	
	int			nr_groups;	
	struct pcpu_group_info	groups[];
};

enum pcpu_fc {
	PCPU_FC_AUTO,
	PCPU_FC_EMBED,
	PCPU_FC_PAGE,

	PCPU_FC_NR,
};
extern const char *pcpu_fc_names[PCPU_FC_NR];

extern enum pcpu_fc pcpu_chosen_fc;

typedef void * (*pcpu_fc_alloc_fn_t)(unsigned int cpu, size_t size,
				     size_t align);
typedef void (*pcpu_fc_free_fn_t)(void *ptr, size_t size);
typedef void (*pcpu_fc_populate_pte_fn_t)(unsigned long addr);
typedef int (pcpu_fc_cpu_distance_fn_t)(unsigned int from, unsigned int to);

extern struct pcpu_alloc_info * __init pcpu_alloc_alloc_info(int nr_groups,
							     int nr_units);
extern void __init pcpu_free_alloc_info(struct pcpu_alloc_info *ai);

extern struct pcpu_alloc_info * __init pcpu_build_alloc_info(
				size_t reserved_size, ssize_t dyn_size,
				size_t atom_size,
				pcpu_fc_cpu_distance_fn_t cpu_distance_fn);

extern int __init pcpu_setup_first_chunk(const struct pcpu_alloc_info *ai,
					 void *base_addr);

#ifdef CONFIG_NEED_PER_CPU_EMBED_FIRST_CHUNK
extern int __init pcpu_embed_first_chunk(size_t reserved_size, ssize_t dyn_size,
				size_t atom_size,
				pcpu_fc_cpu_distance_fn_t cpu_distance_fn,
				pcpu_fc_alloc_fn_t alloc_fn,
				pcpu_fc_free_fn_t free_fn);
#endif

#ifdef CONFIG_NEED_PER_CPU_PAGE_FIRST_CHUNK
extern int __init pcpu_page_first_chunk(size_t reserved_size,
				pcpu_fc_alloc_fn_t alloc_fn,
				pcpu_fc_free_fn_t free_fn,
				pcpu_fc_populate_pte_fn_t populate_pte_fn);
#endif


#define per_cpu_ptr(ptr, cpu)	SHIFT_PERCPU_PTR((ptr), per_cpu_offset((cpu)))

extern void *__alloc_reserved_percpu(size_t size, size_t align);

#else 

struct percpu_data {
	void *ptrs[1];
};


#ifndef CONFIG_DEBUG_KMEMLEAK
#define __percpu_disguise(pdata) (struct percpu_data *)~(unsigned long)(pdata)
#else
#define __percpu_disguise(pdata) (struct percpu_data *)(pdata)
#endif

#define per_cpu_ptr(ptr, cpu)						\
({									\
        struct percpu_data *__p = __percpu_disguise(ptr);		\
        (__typeof__(ptr))__p->ptrs[(cpu)];				\
})

#endif 

extern void *__alloc_percpu(size_t size, size_t align);
extern void free_percpu(void *__pdata);

#ifndef CONFIG_HAVE_SETUP_PER_CPU_AREA
extern void __init setup_per_cpu_areas(void);
#endif

#else 

#define per_cpu_ptr(ptr, cpu) ({ (void)(cpu); (ptr); })

static inline void *__alloc_percpu(size_t size, size_t align)
{
	
	WARN_ON_ONCE(align > SMP_CACHE_BYTES);
	return kzalloc(size, GFP_KERNEL);
}

static inline void free_percpu(void *p)
{
	kfree(p);
}

static inline void __init setup_per_cpu_areas(void) { }

static inline void *pcpu_lpage_remapped(void *kaddr)
{
	return NULL;
}

#endif 

#define alloc_percpu(type)	(type *)__alloc_percpu(sizeof(type), \
						       __alignof__(type))


#ifndef percpu_read
# define percpu_read(var)						\
  ({									\
	typeof(per_cpu_var(var)) __tmp_var__;				\
	__tmp_var__ = get_cpu_var(var);					\
	put_cpu_var(var);						\
	__tmp_var__;							\
  })
#endif

#define __percpu_generic_to_op(var, val, op)				\
do {									\
	get_cpu_var(var) op val;					\
	put_cpu_var(var);						\
} while (0)

#ifndef percpu_write
# define percpu_write(var, val)		__percpu_generic_to_op(var, (val), =)
#endif

#ifndef percpu_add
# define percpu_add(var, val)		__percpu_generic_to_op(var, (val), +=)
#endif

#ifndef percpu_sub
# define percpu_sub(var, val)		__percpu_generic_to_op(var, (val), -=)
#endif

#ifndef percpu_and
# define percpu_and(var, val)		__percpu_generic_to_op(var, (val), &=)
#endif

#ifndef percpu_or
# define percpu_or(var, val)		__percpu_generic_to_op(var, (val), |=)
#endif

#ifndef percpu_xor
# define percpu_xor(var, val)		__percpu_generic_to_op(var, (val), ^=)
#endif

#endif 
