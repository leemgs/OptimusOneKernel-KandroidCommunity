

#include <linux/kernel.h>
#include <linux/percpu.h>
#include <asm/pvclock.h>


struct pvclock_shadow_time {
	u64 tsc_timestamp;     
	u64 system_timestamp;  
	u32 tsc_to_nsec_mul;
	int tsc_shift;
	u32 version;
};


static inline u64 scale_delta(u64 delta, u32 mul_frac, int shift)
{
	u64 product;
#ifdef __i386__
	u32 tmp1, tmp2;
#endif

	if (shift < 0)
		delta >>= -shift;
	else
		delta <<= shift;

#ifdef __i386__
	__asm__ (
		"mul  %5       ; "
		"mov  %4,%%eax ; "
		"mov  %%edx,%4 ; "
		"mul  %5       ; "
		"xor  %5,%5    ; "
		"add  %4,%%eax ; "
		"adc  %5,%%edx ; "
		: "=A" (product), "=r" (tmp1), "=r" (tmp2)
		: "a" ((u32)delta), "1" ((u32)(delta >> 32)), "2" (mul_frac) );
#elif defined(__x86_64__)
	__asm__ (
		"mul %%rdx ; shrd $32,%%rdx,%%rax"
		: "=a" (product) : "0" (delta), "d" ((u64)mul_frac) );
#else
#error implement me!
#endif

	return product;
}

static u64 pvclock_get_nsec_offset(struct pvclock_shadow_time *shadow)
{
	u64 delta = native_read_tsc() - shadow->tsc_timestamp;
	return scale_delta(delta, shadow->tsc_to_nsec_mul, shadow->tsc_shift);
}


static unsigned pvclock_get_time_values(struct pvclock_shadow_time *dst,
					struct pvclock_vcpu_time_info *src)
{
	do {
		dst->version = src->version;
		rmb();		
		dst->tsc_timestamp     = src->tsc_timestamp;
		dst->system_timestamp  = src->system_time;
		dst->tsc_to_nsec_mul   = src->tsc_to_system_mul;
		dst->tsc_shift         = src->tsc_shift;
		rmb();		
	} while ((src->version & 1) || (dst->version != src->version));

	return dst->version;
}

unsigned long pvclock_tsc_khz(struct pvclock_vcpu_time_info *src)
{
	u64 pv_tsc_khz = 1000000ULL << 32;

	do_div(pv_tsc_khz, src->tsc_to_system_mul);
	if (src->tsc_shift < 0)
		pv_tsc_khz <<= -src->tsc_shift;
	else
		pv_tsc_khz >>= src->tsc_shift;
	return pv_tsc_khz;
}

cycle_t pvclock_clocksource_read(struct pvclock_vcpu_time_info *src)
{
	struct pvclock_shadow_time shadow;
	unsigned version;
	cycle_t ret, offset;

	do {
		version = pvclock_get_time_values(&shadow, src);
		barrier();
		offset = pvclock_get_nsec_offset(&shadow);
		ret = shadow.system_timestamp + offset;
		barrier();
	} while (version != src->version);

	return ret;
}

void pvclock_read_wallclock(struct pvclock_wall_clock *wall_clock,
			    struct pvclock_vcpu_time_info *vcpu_time,
			    struct timespec *ts)
{
	u32 version;
	u64 delta;
	struct timespec now;

	
	do {
		version = wall_clock->version;
		rmb();		
		now.tv_sec  = wall_clock->sec;
		now.tv_nsec = wall_clock->nsec;
		rmb();		
	} while ((wall_clock->version & 1) || (version != wall_clock->version));

	delta = pvclock_clocksource_read(vcpu_time);	
	delta += now.tv_sec * (u64)NSEC_PER_SEC + now.tv_nsec;

	now.tv_nsec = do_div(delta, NSEC_PER_SEC);
	now.tv_sec = delta;

	set_normalized_timespec(ts, now.tv_sec, now.tv_nsec);
}
