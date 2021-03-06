#ifndef _LINUX_BITOPS_H
#define _LINUX_BITOPS_H
#include <asm/types.h>

#ifdef	__KERNEL__
#define BIT(nr)			(1UL << (nr))
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE		8
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))
#endif


#include <asm/bitops.h>

#define for_each_bit(bit, addr, size) \
	for ((bit) = find_first_bit((addr), (size)); \
	     (bit) < (size); \
	     (bit) = find_next_bit((addr), (size), (bit) + 1))


static __inline__ int get_bitmask_order(unsigned int count)
{
	int order;
	
	order = fls(count);
	return order;	
}

static __inline__ int get_count_order(unsigned int count)
{
	int order;
	
	order = fls(count) - 1;
	if (count & (count - 1))
		order++;
	return order;
}

static inline unsigned long hweight_long(unsigned long w)
{
	return sizeof(w) == 4 ? hweight32(w) : hweight64(w);
}


static inline __u32 rol32(__u32 word, unsigned int shift)
{
	return (word << shift) | (word >> (32 - shift));
}


static inline __u32 ror32(__u32 word, unsigned int shift)
{
	return (word >> shift) | (word << (32 - shift));
}


static inline __u16 rol16(__u16 word, unsigned int shift)
{
	return (word << shift) | (word >> (16 - shift));
}


static inline __u16 ror16(__u16 word, unsigned int shift)
{
	return (word >> shift) | (word << (16 - shift));
}


static inline __u8 rol8(__u8 word, unsigned int shift)
{
	return (word << shift) | (word >> (8 - shift));
}


static inline __u8 ror8(__u8 word, unsigned int shift)
{
	return (word >> shift) | (word << (8 - shift));
}

static inline unsigned fls_long(unsigned long l)
{
	if (sizeof(l) == 4)
		return fls(l);
	return fls64(l);
}


static inline unsigned long __ffs64(u64 word)
{
#if BITS_PER_LONG == 32
	if (((u32)word) == 0UL)
		return __ffs((u32)(word >> 32)) + 32;
#elif BITS_PER_LONG != 64
#error BITS_PER_LONG not 32 or 64
#endif
	return __ffs((unsigned long)word);
}

#ifdef __KERNEL__
#ifdef CONFIG_GENERIC_FIND_FIRST_BIT


extern unsigned long find_first_bit(const unsigned long *addr,
				    unsigned long size);


extern unsigned long find_first_zero_bit(const unsigned long *addr,
					 unsigned long size);
#endif 

#ifdef CONFIG_GENERIC_FIND_LAST_BIT

extern unsigned long find_last_bit(const unsigned long *addr,
				   unsigned long size);
#endif 

#ifdef CONFIG_GENERIC_FIND_NEXT_BIT


extern unsigned long find_next_bit(const unsigned long *addr,
				   unsigned long size, unsigned long offset);



extern unsigned long find_next_zero_bit(const unsigned long *addr,
					unsigned long size,
					unsigned long offset);

#endif 
#endif 
#endif
