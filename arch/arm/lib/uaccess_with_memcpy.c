

#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>
#include <linux/rwsem.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/hardirq.h> 
#include <asm/current.h>
#include <asm/page.h>

static int
pin_page_for_write(const void __user *_addr, pte_t **ptep, spinlock_t **ptlp)
{
	unsigned long addr = (unsigned long)_addr;
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *pte;
	spinlock_t *ptl;

	pgd = pgd_offset(current->mm, addr);
	if (unlikely(pgd_none(*pgd) || pgd_bad(*pgd)))
		return 0;

	pmd = pmd_offset(pgd, addr);
	if (unlikely(pmd_none(*pmd) || pmd_bad(*pmd)))
		return 0;

	pte = pte_offset_map_lock(current->mm, pmd, addr, &ptl);
	if (unlikely(!pte_present(*pte) || !pte_young(*pte) ||
	    !pte_write(*pte) || !pte_dirty(*pte))) {
		pte_unmap_unlock(pte, ptl);
		return 0;
	}

	*ptep = pte;
	*ptlp = ptl;

	return 1;
}

static unsigned long noinline
__copy_to_user_memcpy(void __user *to, const void *from, unsigned long n)
{
	int atomic;

	if (unlikely(segment_eq(get_fs(), KERNEL_DS))) {
		memcpy((void *)to, from, n);
		return 0;
	}

	
	atomic = in_atomic();

	if (!atomic)
		down_read(&current->mm->mmap_sem);
	while (n) {
		pte_t *pte;
		spinlock_t *ptl;
		int tocopy;

		while (!pin_page_for_write(to, &pte, &ptl)) {
			if (!atomic)
				up_read(&current->mm->mmap_sem);
			if (__put_user(0, (char __user *)to))
				goto out;
			if (!atomic)
				down_read(&current->mm->mmap_sem);
		}

		tocopy = (~(unsigned long)to & ~PAGE_MASK) + 1;
		if (tocopy > n)
			tocopy = n;

		memcpy((void *)to, from, tocopy);
		to += tocopy;
		from += tocopy;
		n -= tocopy;

		pte_unmap_unlock(pte, ptl);
	}
	if (!atomic)
		up_read(&current->mm->mmap_sem);

out:
	return n;
}

unsigned long
__copy_to_user(void __user *to, const void *from, unsigned long n)
{
	
	if (n < 64)
		return __copy_to_user_std(to, from, n);
	return __copy_to_user_memcpy(to, from, n);
}
	
static unsigned long noinline
__clear_user_memset(void __user *addr, unsigned long n)
{
	if (unlikely(segment_eq(get_fs(), KERNEL_DS))) {
		memset((void *)addr, 0, n);
		return 0;
	}

	down_read(&current->mm->mmap_sem);
	while (n) {
		pte_t *pte;
		spinlock_t *ptl;
		int tocopy;

		while (!pin_page_for_write(addr, &pte, &ptl)) {
			up_read(&current->mm->mmap_sem);
			if (__put_user(0, (char __user *)addr))
				goto out;
			down_read(&current->mm->mmap_sem);
		}

		tocopy = (~(unsigned long)addr & ~PAGE_MASK) + 1;
		if (tocopy > n)
			tocopy = n;

		memset((void *)addr, 0, tocopy);
		addr += tocopy;
		n -= tocopy;

		pte_unmap_unlock(pte, ptl);
	}
	up_read(&current->mm->mmap_sem);

out:
	return n;
}

unsigned long __clear_user(void __user *addr, unsigned long n)
{
	
	if (n < 64)
		return __clear_user_std(addr, n);
	return __clear_user_memset(addr, n);
}

#if 0



#include <linux/vmalloc.h>

static int __init test_size_treshold(void)
{
	struct page *src_page, *dst_page;
	void *user_ptr, *kernel_ptr;
	unsigned long long t0, t1, t2;
	int size, ret;

	ret = -ENOMEM;
	src_page = alloc_page(GFP_KERNEL);
	if (!src_page)
		goto no_src;
	dst_page = alloc_page(GFP_KERNEL);
	if (!dst_page)
		goto no_dst;
	kernel_ptr = page_address(src_page);
	user_ptr = vmap(&dst_page, 1, VM_IOREMAP, __pgprot(__P010));
	if (!user_ptr)
		goto no_vmap;

	
	ret = __copy_to_user_memcpy(user_ptr, kernel_ptr, PAGE_SIZE);

	for (size = PAGE_SIZE; size >= 4; size /= 2) {
		t0 = sched_clock();
		ret |= __copy_to_user_memcpy(user_ptr, kernel_ptr, size);
		t1 = sched_clock();
		ret |= __copy_to_user_std(user_ptr, kernel_ptr, size);
		t2 = sched_clock();
		printk("copy_to_user: %d %llu %llu\n", size, t1 - t0, t2 - t1);
	}

	for (size = PAGE_SIZE; size >= 4; size /= 2) {
		t0 = sched_clock();
		ret |= __clear_user_memset(user_ptr, size);
		t1 = sched_clock();
		ret |= __clear_user_std(user_ptr, size);
		t2 = sched_clock();
		printk("clear_user: %d %llu %llu\n", size, t1 - t0, t2 - t1);
	}

	if (ret)
		ret = -EFAULT;

	vunmap(user_ptr);
no_vmap:
	put_page(dst_page);
no_dst:
	put_page(src_page);
no_src:
	return ret;
}

subsys_initcall(test_size_treshold);

#endif
