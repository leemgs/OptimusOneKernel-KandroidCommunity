

#ifndef _ANDROID_PMEM_H_
#define _ANDROID_PMEM_H_

#include <linux/fs.h>

#define PMEM_KERNEL_TEST_MAGIC 0xc0
#define PMEM_KERNEL_TEST_NOMINAL_TEST_IOCTL \
	_IO(PMEM_KERNEL_TEST_MAGIC, 1)
#define PMEM_KERNEL_TEST_ADVERSARIAL_TEST_IOCTL \
	_IO(PMEM_KERNEL_TEST_MAGIC, 2)
#define PMEM_KERNEL_TEST_HUGE_ALLOCATION_TEST_IOCTL \
	_IO(PMEM_KERNEL_TEST_MAGIC, 3)
#define PMEM_KERNEL_TEST_FREE_UNALLOCATED_TEST_IOCTL \
	_IO(PMEM_KERNEL_TEST_MAGIC, 4)
#define PMEM_KERNEL_TEST_LARGE_REGION_NUMBER_TEST_IOCTL \
	_IO(PMEM_KERNEL_TEST_MAGIC, 5)

#define PMEM_IOCTL_MAGIC 'p'
#define PMEM_GET_PHYS		_IOW(PMEM_IOCTL_MAGIC, 1, unsigned int)
#define PMEM_MAP		_IOW(PMEM_IOCTL_MAGIC, 2, unsigned int)
#define PMEM_GET_SIZE		_IOW(PMEM_IOCTL_MAGIC, 3, unsigned int)
#define PMEM_UNMAP		_IOW(PMEM_IOCTL_MAGIC, 4, unsigned int)

#define PMEM_ALLOCATE		_IOW(PMEM_IOCTL_MAGIC, 5, unsigned int)

#define PMEM_CONNECT		_IOW(PMEM_IOCTL_MAGIC, 6, unsigned int)

#define PMEM_GET_TOTAL_SIZE	_IOW(PMEM_IOCTL_MAGIC, 7, unsigned int)

#define HW3D_REVOKE_GPU		_IOW(PMEM_IOCTL_MAGIC, 8, unsigned int)
#define HW3D_GRANT_GPU		_IOW(PMEM_IOCTL_MAGIC, 9, unsigned int)
#define HW3D_WAIT_FOR_INTERRUPT	_IOW(PMEM_IOCTL_MAGIC, 10, unsigned int)

#define PMEM_CLEAN_INV_CACHES	_IOW(PMEM_IOCTL_MAGIC, 11, unsigned int)
#define PMEM_CLEAN_CACHES	_IOW(PMEM_IOCTL_MAGIC, 12, unsigned int)
#define PMEM_INV_CACHES		_IOW(PMEM_IOCTL_MAGIC, 13, unsigned int)

#define PMEM_GET_FREE_SPACE	_IOW(PMEM_IOCTL_MAGIC, 14, unsigned int)
#define PMEM_ALLOCATE_ALIGNED	_IOW(PMEM_IOCTL_MAGIC, 15, unsigned int)
struct pmem_region {
	unsigned long offset;
	unsigned long len;
};

struct pmem_addr {
	unsigned long vaddr;
	unsigned long offset;
	unsigned long length;
};

struct pmem_freespace {
	unsigned long total;
	unsigned long largest;
};

struct pmem_allocation {
	unsigned long size;
	unsigned int align;
};

#ifdef __KERNEL__
int get_pmem_file(unsigned int fd, unsigned long *start, unsigned long *vstart,
		  unsigned long *end, struct file **filp);
int get_pmem_fd(int fd, unsigned long *start, unsigned long *end);
int get_pmem_user_addr(struct file *file, unsigned long *start,
		       unsigned long *end);
void put_pmem_file(struct file* file);
void put_pmem_fd(int fd);
void flush_pmem_fd(int fd, unsigned long start, unsigned long len);
void flush_pmem_file(struct file *file, unsigned long start, unsigned long len);
int pmem_cache_maint(struct file *file, unsigned int cmd,
		struct pmem_addr *pmem_addr);

enum pmem_allocator_type {
	
	PMEM_ALLOCATORTYPE_BITMAP = 0, 

	PMEM_ALLOCATORTYPE_ALLORNOTHING,
	PMEM_ALLOCATORTYPE_BUDDYBESTFIT,

	PMEM_ALLOCATORTYPE_MAX,
};

#define PMEM_MEMTYPE_MASK 0x7
#define PMEM_INVALID_MEMTYPE 0x0
#define PMEM_MEMTYPE_EBI1 0x1
#define PMEM_MEMTYPE_SMI  0x2
#define PMEM_MEMTYPE_RESERVED_INVALID2 0x3
#define PMEM_MEMTYPE_RESERVED_INVALID3 0x4
#define PMEM_MEMTYPE_RESERVED_INVALID4 0x5
#define PMEM_MEMTYPE_RESERVED_INVALID5 0x6
#define PMEM_MEMTYPE_RESERVED_INVALID6 0x7

#define PMEM_ALIGNMENT_MASK 0x18
#define PMEM_ALIGNMENT_RESERVED_INVALID1 0x0
#define PMEM_ALIGNMENT_4K 0x8 
#define PMEM_ALIGNMENT_1M 0x10
#define PMEM_ALIGNMENT_RESERVED_INVALID2 0x18


int32_t pmem_kalloc(const size_t size, const uint32_t flags);
int32_t pmem_kfree(const int32_t physaddr);


#define PMEM_KERNEL_EBI1_DATA_NAME "pmem_kernel_ebi1"
#define PMEM_KERNEL_SMI_DATA_NAME "pmem_kernel_smi"

struct android_pmem_platform_data
{
	const char* name;
	
	unsigned long start;
	
	unsigned long size;

	enum pmem_allocator_type allocator_type;
	
	unsigned int quantum;

	
	unsigned cached;
	
	unsigned buffered;
	
	unsigned unstable;
};

int pmem_setup(struct android_pmem_platform_data *pdata,
	       long (*ioctl)(struct file *, unsigned int, unsigned long),
	       int (*release)(struct inode *, struct file *));

int pmem_remap(struct pmem_region *region, struct file *file,
	       unsigned operation);
#endif 

#endif 

