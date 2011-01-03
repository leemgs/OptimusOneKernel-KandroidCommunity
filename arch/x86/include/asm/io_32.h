#ifndef _ASM_X86_IO_32_H
#define _ASM_X86_IO_32_H

#include <linux/string.h>
#include <linux/compiler.h>





 

#define XQUAD_PORTIO_BASE 0xfe400000
#define XQUAD_PORTIO_QUAD 0x40000  

#ifdef __KERNEL__

#include <asm-generic/iomap.h>

#include <linux/vmalloc.h>


#define xlate_dev_kmem_ptr(p)	p

static inline void
memset_io(volatile void __iomem *addr, unsigned char val, int count)
{
	memset((void __force *)addr, val, count);
}

static inline void
memcpy_fromio(void *dst, const volatile void __iomem *src, int count)
{
	__memcpy(dst, (const void __force *)src, count);
}

static inline void
memcpy_toio(volatile void __iomem *dst, const void *src, int count)
{
	__memcpy((void __force *)dst, src, count);
}


#define __ISA_IO_base ((char __iomem *)(PAGE_OFFSET))



#if defined(CONFIG_X86_OOSTORE) || defined(CONFIG_X86_PPRO_FENCE)

static inline void flush_write_buffers(void)
{
	asm volatile("lock; addl $0,0(%%esp)": : :"memory");
}

#else

#define flush_write_buffers() do { } while (0)

#endif

#endif 

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

#define __BUILDIO(bwl, bw, type)				\
static inline void out##bwl(unsigned type value, int port)	\
{								\
	out##bwl##_local(value, port);				\
}								\
								\
static inline unsigned type in##bwl(int port)			\
{								\
	return in##bwl##_local(port);				\
}

#define BUILDIO(bwl, bw, type)						\
static inline void out##bwl##_local(unsigned type value, int port)	\
{									\
	asm volatile("out" #bwl " %" #bw "0, %w1"		\
		     : : "a"(value), "Nd"(port));			\
}									\
									\
static inline unsigned type in##bwl##_local(int port)			\
{									\
	unsigned type value;						\
	asm volatile("in" #bwl " %w1, %" #bw "0"		\
		     : "=a"(value) : "Nd"(port));			\
	return value;							\
}									\
									\
static inline void out##bwl##_local_p(unsigned type value, int port)	\
{									\
	out##bwl##_local(value, port);					\
	slow_down_io();							\
}									\
									\
static inline unsigned type in##bwl##_local_p(int port)			\
{									\
	unsigned type value = in##bwl##_local(port);			\
	slow_down_io();							\
	return value;							\
}									\
									\
__BUILDIO(bwl, bw, type)						\
									\
static inline void out##bwl##_p(unsigned type value, int port)		\
{									\
	out##bwl(value, port);						\
	slow_down_io();							\
}									\
									\
static inline unsigned type in##bwl##_p(int port)			\
{									\
	unsigned type value = in##bwl(port);				\
	slow_down_io();							\
	return value;							\
}									\
									\
static inline void outs##bwl(int port, const void *addr, unsigned long count) \
{									\
	asm volatile("rep; outs" #bwl					\
		     : "+S"(addr), "+c"(count) : "d"(port));		\
}									\
									\
static inline void ins##bwl(int port, void *addr, unsigned long count)	\
{									\
	asm volatile("rep; ins" #bwl					\
		     : "+D"(addr), "+c"(count) : "d"(port));		\
}

BUILDIO(b, b, char)
BUILDIO(w, w, short)
BUILDIO(l, , int)

#endif 
