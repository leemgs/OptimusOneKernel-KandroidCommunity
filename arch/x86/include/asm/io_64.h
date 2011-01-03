#ifndef _ASM_X86_IO_64_H
#define _ASM_X86_IO_64_H






 

extern void native_io_delay(void);

extern int io_delay_type;
extern void io_delay_init(void);

#if defined(CONFIG_PARAVIRT)
#include <asm/paravirt.h>
#else

static inline void slow_down_io(void)
{
	native_io_delay();
#ifdef REALLY_SLOW_IO
	native_io_delay();
	native_io_delay();
	native_io_delay();
#endif
}
#endif


#define __OUT1(s, x)							\
static inline void out##s(unsigned x value, unsigned short port) {

#define __OUT2(s, s1, s2)				\
asm volatile ("out" #s " %" s1 "0,%" s2 "1"

#ifndef REALLY_SLOW_IO
#define REALLY_SLOW_IO
#define UNSET_REALLY_SLOW_IO
#endif

#define __OUT(s, s1, x)							\
	__OUT1(s, x) __OUT2(s, s1, "w") : : "a" (value), "Nd" (port));	\
	}								\
	__OUT1(s##_p, x) __OUT2(s, s1, "w") : : "a" (value), "Nd" (port)); \
	slow_down_io();							\
}

#define __IN1(s)							\
static inline RETURN_TYPE in##s(unsigned short port)			\
{									\
	RETURN_TYPE _v;

#define __IN2(s, s1, s2)						\
	asm volatile ("in" #s " %" s2 "1,%" s1 "0"

#define __IN(s, s1, i...)						\
	__IN1(s) __IN2(s, s1, "w") : "=a" (_v) : "Nd" (port), ##i);	\
	return _v;							\
	}								\
	__IN1(s##_p) __IN2(s, s1, "w") : "=a" (_v) : "Nd" (port), ##i);	\
	slow_down_io(); \
	return _v; }

#ifdef UNSET_REALLY_SLOW_IO
#undef REALLY_SLOW_IO
#endif

#define __INS(s)							\
static inline void ins##s(unsigned short port, void *addr,		\
			  unsigned long count)				\
{									\
	asm volatile ("rep ; ins" #s					\
		      : "=D" (addr), "=c" (count)			\
		      : "d" (port), "0" (addr), "1" (count));		\
}

#define __OUTS(s)							\
static inline void outs##s(unsigned short port, const void *addr,	\
			   unsigned long count)				\
{									\
	asm volatile ("rep ; outs" #s					\
		      : "=S" (addr), "=c" (count)			\
		      : "d" (port), "0" (addr), "1" (count));		\
}

#define RETURN_TYPE unsigned char
__IN(b, "")
#undef RETURN_TYPE
#define RETURN_TYPE unsigned short
__IN(w, "")
#undef RETURN_TYPE
#define RETURN_TYPE unsigned int
__IN(l, "")
#undef RETURN_TYPE

__OUT(b, "b", char)
__OUT(w, "w", short)
__OUT(l, , int)

__INS(b)
__INS(w)
__INS(l)

__OUTS(b)
__OUTS(w)
__OUTS(l)

#if defined(__KERNEL__) && defined(__x86_64__)

#include <linux/vmalloc.h>

#include <asm-generic/iomap.h>

void __memcpy_fromio(void *, unsigned long, unsigned);
void __memcpy_toio(unsigned long, const void *, unsigned);

static inline void memcpy_fromio(void *to, const volatile void __iomem *from,
				 unsigned len)
{
	__memcpy_fromio(to, (unsigned long)from, len);
}

static inline void memcpy_toio(volatile void __iomem *to, const void *from,
			       unsigned len)
{
	__memcpy_toio((unsigned long)to, from, len);
}

void memset_io(volatile void __iomem *a, int b, size_t c);


#define __ISA_IO_base ((char __iomem *)(PAGE_OFFSET))

#define flush_write_buffers()


#define xlate_dev_kmem_ptr(p)	p

#endif 

#endif 
