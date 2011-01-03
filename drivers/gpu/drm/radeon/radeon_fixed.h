
#ifndef RADEON_FIXED_H
#define RADEON_FIXED_H

typedef union rfixed {
	u32 full;
} fixed20_12;


#define rfixed_const(A) (u32)(((A) << 12))
#define rfixed_const_half(A) (u32)(((A) << 12) + 2048)
#define rfixed_const_666(A) (u32)(((A) << 12) + 2731)
#define rfixed_const_8(A) (u32)(((A) << 12) + 3277)
#define rfixed_mul(A, B) ((u64)((u64)(A).full * (B).full + 2048) >> 12)
#define fixed_init(A) { .full = rfixed_const((A)) }
#define fixed_init_half(A) { .full = rfixed_const_half((A)) }
#define rfixed_trunc(A) ((A).full >> 12)

static inline u32 rfixed_div(fixed20_12 A, fixed20_12 B)
{
	u64 tmp = ((u64)A.full << 13);

	do_div(tmp, B.full);
	tmp += 1;
	tmp /= 2;
	return lower_32_bits(tmp);
}
#endif
