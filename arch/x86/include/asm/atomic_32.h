#ifndef _ASM_X86_ATOMIC_32_H
#define _ASM_X86_ATOMIC_32_H

#include <linux/compiler.h>
#include <linux/types.h>
#include <asm/processor.h>
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
		     : "+m" (v->counter)
		     : "ir" (i));
}


static inline void atomic_sub(int i, atomic_t *v)
{
	asm volatile(LOCK_PREFIX "subl %1,%0"
		     : "+m" (v->counter)
		     : "ir" (i));
}


static inline int atomic_sub_and_test(int i, atomic_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "subl %2,%0; sete %1"
		     : "+m" (v->counter), "=qm" (c)
		     : "ir" (i) : "memory");
	return c;
}


static inline void atomic_inc(atomic_t *v)
{
	asm volatile(LOCK_PREFIX "incl %0"
		     : "+m" (v->counter));
}


static inline void atomic_dec(atomic_t *v)
{
	asm volatile(LOCK_PREFIX "decl %0"
		     : "+m" (v->counter));
}


static inline int atomic_dec_and_test(atomic_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "decl %0; sete %1"
		     : "+m" (v->counter), "=qm" (c)
		     : : "memory");
	return c != 0;
}


static inline int atomic_inc_and_test(atomic_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "incl %0; sete %1"
		     : "+m" (v->counter), "=qm" (c)
		     : : "memory");
	return c != 0;
}


static inline int atomic_add_negative(int i, atomic_t *v)
{
	unsigned char c;

	asm volatile(LOCK_PREFIX "addl %2,%0; sets %1"
		     : "+m" (v->counter), "=qm" (c)
		     : "ir" (i) : "memory");
	return c;
}


static inline int atomic_add_return(int i, atomic_t *v)
{
	int __i;
#ifdef CONFIG_M386
	unsigned long flags;
	if (unlikely(boot_cpu_data.x86 <= 3))
		goto no_xadd;
#endif
	
	__i = i;
	asm volatile(LOCK_PREFIX "xaddl %0, %1"
		     : "+r" (i), "+m" (v->counter)
		     : : "memory");
	return i + __i;

#ifdef CONFIG_M386
no_xadd: 
	local_irq_save(flags);
	__i = atomic_read(v);
	atomic_set(v, i + __i);
	local_irq_restore(flags);
	return i + __i;
#endif
}


static inline int atomic_sub_return(int i, atomic_t *v)
{
	return atomic_add_return(-i, v);
}

static inline int atomic_cmpxchg(atomic_t *v, int old, int new)
{
	return cmpxchg(&v->counter, old, new);
}

static inline int atomic_xchg(atomic_t *v, int new)
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

#define atomic_inc_return(v)  (atomic_add_return(1, v))
#define atomic_dec_return(v)  (atomic_sub_return(1, v))


#define atomic_clear_mask(mask, addr)				\
	asm volatile(LOCK_PREFIX "andl %0,%1"			\
		     : : "r" (~(mask)), "m" (*(addr)) : "memory")

#define atomic_set_mask(mask, addr)				\
	asm volatile(LOCK_PREFIX "orl %0,%1"				\
		     : : "r" (mask), "m" (*(addr)) : "memory")


#define smp_mb__before_atomic_dec()	barrier()
#define smp_mb__after_atomic_dec()	barrier()
#define smp_mb__before_atomic_inc()	barrier()
#define smp_mb__after_atomic_inc()	barrier()



typedef struct {
	u64 __aligned(8) counter;
} atomic64_t;

#define ATOMIC64_INIT(val)	{ (val) }

extern u64 atomic64_cmpxchg(atomic64_t *ptr, u64 old_val, u64 new_val);


extern u64 atomic64_xchg(atomic64_t *ptr, u64 new_val);


extern void atomic64_set(atomic64_t *ptr, u64 new_val);


static inline u64 atomic64_read(atomic64_t *ptr)
{
	u64 res;

	
	asm volatile(
		"mov %%ebx, %%eax\n\t"
		"mov %%ecx, %%edx\n\t"
		LOCK_PREFIX "cmpxchg8b %1\n"
			: "=&A" (res)
			: "m" (*ptr)
		);

	return res;
}

extern u64 atomic64_read(atomic64_t *ptr);


extern u64 atomic64_add_return(u64 delta, atomic64_t *ptr);


extern u64 atomic64_sub_return(u64 delta, atomic64_t *ptr);
extern u64 atomic64_inc_return(atomic64_t *ptr);
extern u64 atomic64_dec_return(atomic64_t *ptr);


extern void atomic64_add(u64 delta, atomic64_t *ptr);


extern void atomic64_sub(u64 delta, atomic64_t *ptr);


extern int atomic64_sub_and_test(u64 delta, atomic64_t *ptr);


extern void atomic64_inc(atomic64_t *ptr);


extern void atomic64_dec(atomic64_t *ptr);


extern int atomic64_dec_and_test(atomic64_t *ptr);


extern int atomic64_inc_and_test(atomic64_t *ptr);


extern int atomic64_add_negative(u64 delta, atomic64_t *ptr);

#include <asm-generic/atomic-long.h>
#endif 
