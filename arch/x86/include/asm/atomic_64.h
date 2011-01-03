#ifndef _ASM_X86_ATOMIC_64_H
#define _ASM_X86_ATOMIC_64_H

#include <linux/types.h>
#include <asm/alternative.h>
#include <asm/cmpxchg.h>



#define ATOMIC_INIT(i)	{ (i) }


static inline int atomic_read(const atomic_t *v)
{
	return v->counter;
}


static inline void atomic_set(atomic_t *v, int i)
{
	v->counter = i;
}


static inline void atomic_add(int i, atomic_t *v)
{
	asm volatile(LOCK_PREFIX "addl %1,%0"
		     : "=m" (v->counter)
		     : "ir" (i), "m" (v->counter));
}


static inline void atomic_sub(int i, atomic_t *v)
{
	asm volatile(LOCK_PREFIX "subl %1,%0"
		     : "=m" (v->counter)
		     : "ir" (i), "m" (v->counter));
}


static inline int atomic_sub_and_test(int i, atomic_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "subl %2,%0; sete %1"
		     : "=m" (v->counter), "=qm" (c)
		     : "ir" (i), "m" (v->counter) : "memory");
	return c;
}


static inline void atomic_inc(atomic_t *v)
{
	asm volatile(LOCK_PREFIX "incl %0"
		     : "=m" (v->counter)
		     : "m" (v->counter));
}


static inline void atomic_dec(atomic_t *v)
{
	asm volatile(LOCK_PREFIX "decl %0"
		     : "=m" (v->counter)
		     : "m" (v->counter));
}


static inline int atomic_dec_and_test(atomic_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "decl %0; sete %1"
		     : "=m" (v->counter), "=qm" (c)
		     : "m" (v->counter) : "memory");
	return c != 0;
}


static inline int atomic_inc_and_test(atomic_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "incl %0; sete %1"
		     : "=m" (v->counter), "=qm" (c)
		     : "m" (v->counter) : "memory");
	return c != 0;
}


static inline int atomic_add_negative(int i, atomic_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "addl %2,%0; sets %1"
		     : "=m" (v->counter), "=qm" (c)
		     : "ir" (i), "m" (v->counter) : "memory");
	return c;
}


static inline int atomic_add_return(int i, atomic_t *v)
{
	int __i = i;
	asm volatile(LOCK_PREFIX "xaddl %0, %1"
		     : "+r" (i), "+m" (v->counter)
		     : : "memory");
	return i + __i;
}

static inline int atomic_sub_return(int i, atomic_t *v)
{
	return atomic_add_return(-i, v);
}

#define atomic_inc_return(v)  (atomic_add_return(1, v))
#define atomic_dec_return(v)  (atomic_sub_return(1, v))



#define ATOMIC64_INIT(i)	{ (i) }


static inline long atomic64_read(const atomic64_t *v)
{
	return v->counter;
}


static inline void atomic64_set(atomic64_t *v, long i)
{
	v->counter = i;
}


static inline void atomic64_add(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "addq %1,%0"
		     : "=m" (v->counter)
		     : "er" (i), "m" (v->counter));
}


static inline void atomic64_sub(long i, atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "subq %1,%0"
		     : "=m" (v->counter)
		     : "er" (i), "m" (v->counter));
}


static inline int atomic64_sub_and_test(long i, atomic64_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "subq %2,%0; sete %1"
		     : "=m" (v->counter), "=qm" (c)
		     : "er" (i), "m" (v->counter) : "memory");
	return c;
}


static inline void atomic64_inc(atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "incq %0"
		     : "=m" (v->counter)
		     : "m" (v->counter));
}


static inline void atomic64_dec(atomic64_t *v)
{
	asm volatile(LOCK_PREFIX "decq %0"
		     : "=m" (v->counter)
		     : "m" (v->counter));
}


static inline int atomic64_dec_and_test(atomic64_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "decq %0; sete %1"
		     : "=m" (v->counter), "=qm" (c)
		     : "m" (v->counter) : "memory");
	return c != 0;
}


static inline int atomic64_inc_and_test(atomic64_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "incq %0; sete %1"
		     : "=m" (v->counter), "=qm" (c)
		     : "m" (v->counter) : "memory");
	return c != 0;
}


static inline int atomic64_add_negative(long i, atomic64_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "addq %2,%0; sets %1"
		     : "=m" (v->counter), "=qm" (c)
		     : "er" (i), "m" (v->counter) : "memory");
	return c;
}


static inline long atomic64_add_return(long i, atomic64_t *v)
{
	long __i = i;
	asm volatile(LOCK_PREFIX "xaddq %0, %1;"
		     : "+r" (i), "+m" (v->counter)
		     : : "memory");
	return i + __i;
}

static inline long atomic64_sub_return(long i, atomic64_t *v)
{
	return atomic64_add_return(-i, v);
}

#define atomic64_inc_return(v)  (atomic64_add_return(1, (v)))
#define atomic64_dec_return(v)  (atomic64_sub_return(1, (v)))

static inline long atomic64_cmpxchg(atomic64_t *v, long old, long new)
{
	return cmpxchg(&v->counter, old, new);
}

static inline long atomic64_xchg(atomic64_t *v, long new)
{
	return xchg(&v->counter, new);
}

static inline long atomic_cmpxchg(atomic_t *v, int old, int new)
{
	return cmpxchg(&v->counter, old, new);
}

static inline long atomic_xchg(atomic_t *v, int new)
{
	return xchg(&v->counter, new);
}


static inline int atomic_add_unless(atomic_t *v, int a, int u)
{
	int c, old;
	c = atomic_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic_cmpxchg((v), c, c + (a));
		if (likely(old == c))
			break;
		c = old;
	}
	return c != (u);
}

#define atomic_inc_not_zero(v) atomic_add_unless((v), 1, 0)


static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
			break;
		c = old;
	}
	return c != (u);
}


static inline short int atomic_inc_short(short int *v)
{
	asm(LOCK_PREFIX "addw $1, %0" : "+m" (*v));
	return *v;
}


static inline void atomic_or_long(unsigned long *v1, unsigned long v2)
{
	asm(LOCK_PREFIX "orq %1, %0" : "+m" (*v1) : "r" (v2));
}

#define atomic64_inc_not_zero(v) atomic64_add_unless((v), 1, 0)


#define atomic_clear_mask(mask, addr)					\
	asm volatile(LOCK_PREFIX "andl %0,%1"				\
		     : : "r" (~(mask)), "m" (*(addr)) : "memory")

#define atomic_set_mask(mask, addr)					\
	asm volatile(LOCK_PREFIX "orl %0,%1"				\
		     : : "r" ((unsigned)(mask)), "m" (*(addr))		\
		     : "memory")


#define smp_mb__before_atomic_dec()	barrier()
#define smp_mb__after_atomic_dec()	barrier()
#define smp_mb__before_atomic_inc()	barrier()
#define smp_mb__after_atomic_inc()	barrier()

#include <asm-generic/atomic-long.h>
#endif 
