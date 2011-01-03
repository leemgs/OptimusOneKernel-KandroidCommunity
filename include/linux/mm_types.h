#ifndef _LINUX_MM_TYPES_H
#define _LINUX_MM_TYPES_H

#include <linux/auxvec.h>
#include <linux/types.h>
#include <linux/threads.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/prio_tree.h>
#include <linux/rbtree.h>
#include <linux/rwsem.h>
#include <linux/completion.h>
#include <linux/cpumask.h>
#include <linux/page-debug-flags.h>
#include <asm/page.h>
#include <asm/mmu.h>

#ifndef AT_VECTOR_SIZE_ARCH
#define AT_VECTOR_SIZE_ARCH 0
#endif
#define AT_VECTOR_SIZE (2*(AT_VECTOR_SIZE_ARCH + AT_VECTOR_SIZE_BASE + 1))

struct address_space;

#define USE_SPLIT_PTLOCKS	(NR_CPUS >= CONFIG_SPLIT_PTLOCK_CPUS)

#if USE_SPLIT_PTLOCKS
typedef atomic_long_t mm_counter_t;
#else  
typedef unsigned long mm_counter_t;
#endif 


struct page {
	unsigned long flags;		
	atomic_t _count;		
	union {
		atomic_t _mapcount;	
		struct {		
			u16 inuse;
			u16 objects;
		};
	};
	union {
	    struct {
		unsigned long private;		
		struct address_space *mapping;	
	    };
#if USE_SPLIT_PTLOCKS
	    spinlock_t ptl;
#endif
	    struct kmem_cache *slab;	
	    struct page *first_page;	
	};
	union {
		pgoff_t index;		
		void *freelist;		
	};
	struct list_head lru;		
	
#if defined(WANT_PAGE_VIRTUAL)
	void *virtual;			
#endif 
#ifdef CONFIG_WANT_PAGE_DEBUG_FLAGS
	unsigned long debug_flags;	
#endif

#ifdef CONFIG_KMEMCHECK
	
	void *shadow;
#endif
};


struct vm_region {
	struct rb_node	vm_rb;		
	unsigned long	vm_flags;	
	unsigned long	vm_start;	
	unsigned long	vm_end;		
	unsigned long	vm_top;		
	unsigned long	vm_pgoff;	
	struct file	*vm_file;	

	atomic_t	vm_usage;	
};


struct vm_area_struct {
	struct mm_struct * vm_mm;	
	unsigned long vm_start;		
	unsigned long vm_end;		

	
	struct vm_area_struct *vm_next;

	pgprot_t vm_page_prot;		
	unsigned long vm_flags;		

	struct rb_node vm_rb;

	
	union {
		struct {
			struct list_head list;
			void *parent;	
			struct vm_area_struct *head;
		} vm_set;

		struct raw_prio_tree_node prio_tree_node;
	} shared;

	
	struct list_head anon_vma_node;	
	struct anon_vma *anon_vma;	

	
	const struct vm_operations_struct *vm_ops;

	
	unsigned long vm_pgoff;		
	struct file * vm_file;		
	void * vm_private_data;		
	unsigned long vm_truncate_count;

#ifndef CONFIG_MMU
	struct vm_region *vm_region;	
#endif
#ifdef CONFIG_NUMA
	struct mempolicy *vm_policy;	
#endif
};

struct core_thread {
	struct task_struct *task;
	struct core_thread *next;
};

struct core_state {
	atomic_t nr_threads;
	struct core_thread dumper;
	struct completion startup;
};

struct mm_struct {
	struct vm_area_struct * mmap;		
	struct rb_root mm_rb;
	struct vm_area_struct * mmap_cache;	
	unsigned long (*get_unmapped_area) (struct file *filp,
				unsigned long addr, unsigned long len,
				unsigned long pgoff, unsigned long flags);
	void (*unmap_area) (struct mm_struct *mm, unsigned long addr);
	unsigned long mmap_base;		
	unsigned long task_size;		
	unsigned long cached_hole_size; 	
	unsigned long free_area_cache;		
	pgd_t * pgd;
	atomic_t mm_users;			
	atomic_t mm_count;			
	int map_count;				
	struct rw_semaphore mmap_sem;
	spinlock_t page_table_lock;		

	struct list_head mmlist;		

	
	mm_counter_t _file_rss;
	mm_counter_t _anon_rss;

	unsigned long hiwater_rss;	
	unsigned long hiwater_vm;	

	unsigned long total_vm, locked_vm, shared_vm, exec_vm;
	unsigned long stack_vm, reserved_vm, def_flags, nr_ptes;
	unsigned long start_code, end_code, start_data, end_data;
	unsigned long start_brk, brk, start_stack;
	unsigned long arg_start, arg_end, env_start, env_end;

	unsigned long saved_auxv[AT_VECTOR_SIZE]; 

	struct linux_binfmt *binfmt;

	cpumask_t cpu_vm_mask;

	
	mm_context_t context;

	
	
	unsigned int faultstamp;
	unsigned int token_priority;
	unsigned int last_interval;

	unsigned long flags; 

	struct core_state *core_state; 
#ifdef CONFIG_AIO
	spinlock_t		ioctx_lock;
	struct hlist_head	ioctx_list;
#endif
#ifdef CONFIG_MM_OWNER
	
	struct task_struct *owner;
#endif

#ifdef CONFIG_PROC_FS
	
	struct file *exe_file;
	unsigned long num_exe_file_vmas;
#endif
#ifdef CONFIG_MMU_NOTIFIER
	struct mmu_notifier_mm *mmu_notifier_mm;
#endif
};


#define mm_cpumask(mm) (&(mm)->cpu_vm_mask)

#endif 
