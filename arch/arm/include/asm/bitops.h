

#ifndef __ASM_ARM_BITOPS_H
#define __ASM_ARM_BITOPS_H

#ifdef __KERNEL__

#ifndef _LINUX_BITOPS_H
#error only <linux/bitops.h> can be included directly
#endif

#include <linux/compiler.h>
#include <asm/system.h>

#define smp_mb__before_clear_bit()	mb()
#define smp_mb__after_clear_bit()	mb()


static inline void ____atomic_set_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long flags;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	raw_local_irq_save(flags);
	*p |= mask;
	raw_local_irq_restore(flags);
}

static inline void ____atomic_clear_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long flags;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	raw_local_irq_save(flags);
	*p &= ~mask;
	raw_local_irq_restore(flags);
}

static inline void ____atomic_change_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long flags;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	raw_local_irq_save(flags);
	*p ^= mask;
	raw_local_irq_restore(flags);
}

static inline int
____atomic_test_and_set_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long flags;
	unsigned int res;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	raw_local_irq_save(flags);
	res = *p;
	*p = res | mask;
	raw_local_irq_restore(flags);

	return (res & mask) != 0;
}

static inline int
____atomic_test_and_clear_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long flags;
	unsigned int res;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	raw_local_irq_save(flags);
	res = *p;
	*p = res & ~mask;
	raw_local_irq_restore(flags);

	return (res & mask) != 0;
}

static inline int
____atomic_test_and_change_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long flags;
	unsigned int res;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	raw_local_irq_save(flags);
	res = *p;
	*p = res ^ mask;
	raw_local_irq_restore(flags);

	return (res & mask) != 0;
}

#include <asm-generic/bitops/non-atomic.h>




extern void _set_bit_le(int nr, volatile unsigned long * p);
extern void _clear_bit_le(int nr, volatile unsigned long * p);
extern void _change_bit_le(int nr, volatile unsigned long * p);
extern int _test_and_set_bit_le(int nr, volatile unsigned long * p);
extern int _test_and_clear_bit_le(int nr, volatile unsigned long * p);
extern int _test_and_change_bit_le(int nr, volatile unsigned long * p);
extern int _find_first_zero_bit_le(const void * p, unsigned size);
extern int _find_next_zero_bit_le(const void * p, int size, int offset);
extern int _find_first_bit_le(const unsigned long *p, unsigned size);
extern int _find_next_bit_le(const unsigned long *p, int size, int offset);


extern void _set_bit_be(int nr, volatile unsigned long * p);
extern void _clear_bit_be(int nr, volatile unsigned long * p);
extern void _change_bit_be(int nr, volatile unsigned long * p);
extern int _test_and_set_bit_be(int nr, volatile unsigned long * p);
extern int _test_and_clear_bit_be(int nr, volatile unsigned long * p);
extern int _test_and_change_bit_be(int nr, volatile unsigned long * p);
extern int _find_first_zero_bit_be(const void * p, unsigned size);
extern int _find_next_zero_bit_be(const void * p, int size, int offset);
extern int _find_first_bit_be(const unsigned long *p, unsigned size);
extern int _find_next_bit_be(const unsigned long *p, int size, int offset);

#ifndef CONFIG_SMP

#define	ATOMIC_BITOP_LE(name,nr,p)		\
	(__builtin_constant_p(nr) ?		\
	 ____atomic_##name(nr, p) :		\
	 _##name##_le(nr,p))

#define	ATOMIC_BITOP_BE(name,nr,p)		\
	(__builtin_constant_p(nr) ?		\
	 ____atomic_##name(nr, p) :		\
	 _##name##_be(nr,p))
#else
#define ATOMIC_BITOP_LE(name,nr,p)	_##name##_le(nr,p)
#define ATOMIC_BITOP_BE(name,nr,p)	_##name##_be(nr,p)
#endif

#define NONATOMIC_BITOP(name,nr,p)		\
	(____nonatomic_##name(nr, p))

#ifndef __ARMEB__

#define set_bit(nr,p)			ATOMIC_BITOP_LE(set_bit,nr,p)
#define clear_bit(nr,p)			ATOMIC_BITOP_LE(clear_bit,nr,p)
#define change_bit(nr,p)		ATOMIC_BITOP_LE(change_bit,nr,p)
#define test_and_set_bit(nr,p)		ATOMIC_BITOP_LE(test_and_set_bit,nr,p)
#define test_and_clear_bit(nr,p)	ATOMIC_BITOP_LE(test_and_clear_bit,nr,p)
#define test_and_change_bit(nr,p)	ATOMIC_BITOP_LE(test_and_change_bit,nr,p)
#define find_first_zero_bit(p,sz)	_find_first_zero_bit_le(p,sz)
#define find_next_zero_bit(p,sz,off)	_find_next_zero_bit_le(p,sz,off)
#define find_first_bit(p,sz)		_find_first_bit_le(p,sz)
#define find_next_bit(p,sz,off)		_find_next_bit_le(p,sz,off)

#define WORD_BITOFF_TO_LE(x)		((x))

#else


#define set_bit(nr,p)			ATOMIC_BITOP_BE(set_bit,nr,p)
#define clear_bit(nr,p)			ATOMIC_BITOP_BE(clear_bit,nr,p)
#define change_bit(nr,p)		ATOMIC_BITOP_BE(change_bit,nr,p)
#define test_and_set_bit(nr,p)		ATOMIC_BITOP_BE(test_and_set_bit,nr,p)
#define test_and_clear_bit(nr,p)	ATOMIC_BITOP_BE(test_and_clear_bit,nr,p)
#define test_and_change_bit(nr,p)	ATOMIC_BITOP_BE(test_and_change_bit,nr,p)
#define find_first_zero_bit(p,sz)	_find_first_zero_bit_be(p,sz)
#define find_next_zero_bit(p,sz,off)	_find_next_zero_bit_be(p,sz,off)
#define find_first_bit(p,sz)		_find_first_bit_be(p,sz)
#define find_next_bit(p,sz,off)		_find_next_bit_be(p,sz,off)

#define WORD_BITOFF_TO_LE(x)		((x) ^ 0x18)

#endif

#if __LINUX_ARM_ARCH__ < 5

#include <asm-generic/bitops/ffz.h>
#include <asm-generic/bitops/__fls.h>
#include <asm-generic/bitops/__ffs.h>
#include <asm-generic/bitops/fls.h>
#include <asm-generic/bitops/ffs.h>

#else

static inline int constant_fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}



static inline int fls(int x)
{
	int ret;

	if (__builtin_constant_p(x))
	       return constant_fls(x);

	asm("clz\t%0, %1" : "=r" (ret) : "r" (x) : "cc");
       	ret = 32 - ret;
	return ret;
}

#define __fls(x) (fls(x) - 1)
#define ffs(x) ({ unsigned long __t = (x); fls(__t & -__t); })
#define __ffs(x) (ffs(x) - 1)
#define ffz(x) __ffs( ~(x) )

#endif

#include <asm-generic/bitops/fls64.h>

#include <asm-generic/bitops/sched.h>
#include <asm-generic/bitops/hweight.h>
#include <asm-generic/bitops/lock.h>


#define ext2_set_bit(nr,p)			\
		__test_and_set_bit(WORD_BITOFF_TO_LE(nr), (unsigned long *)(p))
#define ext2_set_bit_atomic(lock,nr,p)          \
                test_and_set_bit(WORD_BITOFF_TO_LE(nr), (unsigned long *)(p))
#define ext2_clear_bit(nr,p)			\
		__test_and_clear_bit(WORD_BITOFF_TO_LE(nr), (unsigned long *)(p))
#define ext2_clear_bit_atomic(lock,nr,p)        \
                test_and_clear_bit(WORD_BITOFF_TO_LE(nr), (unsigned long *)(p))
#define ext2_test_bit(nr,p)			\
		test_bit(WORD_BITOFF_TO_LE(nr), (unsigned long *)(p))
#define ext2_find_first_zero_bit(p,sz)		\
		_find_first_zero_bit_le(p,sz)
#define ext2_find_next_zero_bit(p,sz,off)	\
		_find_next_zero_bit_le(p,sz,off)
#define ext2_find_next_bit(p, sz, off) \
		_find_next_bit_le(p, sz, off)


#define minix_set_bit(nr,p)			\
		__set_bit(WORD_BITOFF_TO_LE(nr), (unsigned long *)(p))
#define minix_test_bit(nr,p)			\
		test_bit(WORD_BITOFF_TO_LE(nr), (unsigned long *)(p))
#define minix_test_and_set_bit(nr,p)		\
		__test_and_set_bit(WORD_BITOFF_TO_LE(nr), (unsigned long *)(p))
#define minix_test_and_clear_bit(nr,p)		\
		__test_and_clear_bit(WORD_BITOFF_TO_LE(nr), (unsigned long *)(p))
#define minix_find_first_zero_bit(p,sz)		\
		_find_first_zero_bit_le(p,sz)

#endif 

#endif 
