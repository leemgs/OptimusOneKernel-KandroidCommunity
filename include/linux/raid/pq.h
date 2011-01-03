

#ifndef LINUX_RAID_RAID6_H
#define LINUX_RAID_RAID6_H

#ifdef __KERNEL__


#define RAID6_USE_EMPTY_ZERO_PAGE 0
#include <linux/blkdev.h>


#if RAID6_USE_EMPTY_ZERO_PAGE
# define raid6_empty_zero_page empty_zero_page
#else
extern const char raid6_empty_zero_page[PAGE_SIZE];
#endif

#else 


#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/types.h>


#define BITS_PER_LONG __WORDSIZE

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#ifndef PAGE_SIZE
# define PAGE_SIZE 4096
#endif
extern const char raid6_empty_zero_page[PAGE_SIZE];

#define __init
#define __exit
#define __attribute_const__ __attribute__((const))
#define noinline __attribute__((noinline))

#define preempt_enable()
#define preempt_disable()
#define cpu_has_feature(x) 1
#define enable_kernel_altivec()
#define disable_kernel_altivec()

#define EXPORT_SYMBOL(sym)
#define MODULE_LICENSE(licence)
#define subsys_initcall(x)
#define module_exit(x)
#endif 


struct raid6_calls {
	void (*gen_syndrome)(int, size_t, void **);
	int  (*valid)(void);	
	const char *name;	
	int prefer;		
};


extern struct raid6_calls raid6_call;


extern const struct raid6_calls * const raid6_algos[];
int raid6_select_algo(void);


#define RAID6_OK	0
#define RAID6_P_BAD	1
#define RAID6_Q_BAD	2
#define RAID6_PQ_BAD	3


extern const u8 raid6_gfmul[256][256] __attribute__((aligned(256)));
extern const u8 raid6_gfexp[256]      __attribute__((aligned(256)));
extern const u8 raid6_gfinv[256]      __attribute__((aligned(256)));
extern const u8 raid6_gfexi[256]      __attribute__((aligned(256)));


void raid6_2data_recov(int disks, size_t bytes, int faila, int failb,
		       void **ptrs);
void raid6_datap_recov(int disks, size_t bytes, int faila, void **ptrs);
void raid6_dual_recov(int disks, size_t bytes, int faila, int failb,
		      void **ptrs);


#ifndef __KERNEL__

# define jiffies	raid6_jiffies()
# define printk 	printf
# define GFP_KERNEL	0
# define __get_free_pages(x, y)	((unsigned long)mmap(NULL, PAGE_SIZE << (y), \
						     PROT_READ|PROT_WRITE,   \
						     MAP_PRIVATE|MAP_ANONYMOUS,\
						     0, 0))
# define free_pages(x, y)	munmap((void *)(x), (y)*PAGE_SIZE)

static inline void cpu_relax(void)
{
	
}

#undef  HZ
#define HZ 1000
static inline uint32_t raid6_jiffies(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*1000 + tv.tv_usec/1000;
}

#endif 

#endif 
