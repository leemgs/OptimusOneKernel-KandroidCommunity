#include <linux/compiler.h>
#include <linux/module.h>
#include <linux/types.h>

#include <asm/processor.h>
#include <asm/cmpxchg.h>
#include <asm/atomic.h>

static noinline u64 cmpxchg8b(u64 *ptr, u64 old, u64 new)
{
	u32 low = new;
	u32 high = new >> 32;

	asm volatile(
		LOCK_PREFIX "cmpxchg8b %1\n"
		     : "+A" (old), "+m" (*ptr)
		     :  "b" (low),  "c" (high)
		     );
	return old;
}

u64 atomic64_cmpxchg(atomic64_t *ptr, u64 old_val, u64 new_val)
{
	return cmpxchg8b(&ptr->counter, old_val, new_val);
}
EXPORT_SYMBOL(atomic64_cmpxchg);


u64 atomic64_xchg(atomic64_t *ptr, u64 new_val)
{
	
	u64 old_val, real_val = 0;

	do {
		old_val = real_val;

		real_val = atomic64_cmpxchg(ptr, old_val, new_val);

	} while (real_val != old_val);

	return old_val;
}
EXPORT_SYMBOL(atomic64_xchg);


void atomic64_set(atomic64_t *ptr, u64 new_val)
{
	atomic64_xchg(ptr, new_val);
}
EXPORT_SYMBOL(atomic64_set);


noinline u64 atomic64_add_return(u64 delta, atomic64_t *ptr)
{
	
	u64 old_val, new_val, real_val = 0;

	do {
		old_val = real_val;
		new_val = old_val + delta;

		real_val = atomic64_cmpxchg(ptr, old_val, new_val);

	} while (real_val != old_val);

	return new_val;
}
EXPORT_SYMBOL(atomic64_add_return);

u64 atomic64_sub_return(u64 delta, atomic64_t *ptr)
{
	return atomic64_add_return(-delta, ptr);
}
EXPORT_SYMBOL(atomic64_sub_return);

u64 atomic64_inc_return(atomic64_t *ptr)
{
	return atomic64_add_return(1, ptr);
}
EXPORT_SYMBOL(atomic64_inc_return);

u64 atomic64_dec_return(atomic64_t *ptr)
{
	return atomic64_sub_return(1, ptr);
}
EXPORT_SYMBOL(atomic64_dec_return);


void atomic64_add(u64 delta, atomic64_t *ptr)
{
	atomic64_add_return(delta, ptr);
}
EXPORT_SYMBOL(atomic64_add);


void atomic64_sub(u64 delta, atomic64_t *ptr)
{
	atomic64_add(-delta, ptr);
}
EXPORT_SYMBOL(atomic64_sub);


int atomic64_sub_and_test(u64 delta, atomic64_t *ptr)
{
	u64 new_val = atomic64_sub_return(delta, ptr);

	return new_val == 0;
}
EXPORT_SYMBOL(atomic64_sub_and_test);


void atomic64_inc(atomic64_t *ptr)
{
	atomic64_add(1, ptr);
}
EXPORT_SYMBOL(atomic64_inc);


void atomic64_dec(atomic64_t *ptr)
{
	atomic64_sub(1, ptr);
}
EXPORT_SYMBOL(atomic64_dec);


int atomic64_dec_and_test(atomic64_t *ptr)
{
	return atomic64_sub_and_test(1, ptr);
}
EXPORT_SYMBOL(atomic64_dec_and_test);


int atomic64_inc_and_test(atomic64_t *ptr)
{
	return atomic64_sub_and_test(-1, ptr);
}
EXPORT_SYMBOL(atomic64_inc_and_test);


int atomic64_add_negative(u64 delta, atomic64_t *ptr)
{
	s64 new_val = atomic64_add_return(delta, ptr);

	return new_val < 0;
}
EXPORT_SYMBOL(atomic64_add_negative);
