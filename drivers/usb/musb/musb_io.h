

#ifndef __MUSB_LINUX_PLATFORM_ARCH_H__
#define __MUSB_LINUX_PLATFORM_ARCH_H__

#include <linux/io.h>

#if !defined(CONFIG_ARM) && !defined(CONFIG_SUPERH) \
	&& !defined(CONFIG_AVR32) && !defined(CONFIG_PPC32) \
	&& !defined(CONFIG_PPC64) && !defined(CONFIG_BLACKFIN)
static inline void readsl(const void __iomem *addr, void *buf, int len)
	{ insl((unsigned long)addr, buf, len); }
static inline void readsw(const void __iomem *addr, void *buf, int len)
	{ insw((unsigned long)addr, buf, len); }
static inline void readsb(const void __iomem *addr, void *buf, int len)
	{ insb((unsigned long)addr, buf, len); }

static inline void writesl(const void __iomem *addr, const void *buf, int len)
	{ outsl((unsigned long)addr, buf, len); }
static inline void writesw(const void __iomem *addr, const void *buf, int len)
	{ outsw((unsigned long)addr, buf, len); }
static inline void writesb(const void __iomem *addr, const void *buf, int len)
	{ outsb((unsigned long)addr, buf, len); }

#endif

#ifndef CONFIG_BLACKFIN



static inline u16 musb_readw(const void __iomem *addr, unsigned offset)
	{ return __raw_readw(addr + offset); }

static inline u32 musb_readl(const void __iomem *addr, unsigned offset)
	{ return __raw_readl(addr + offset); }


static inline void musb_writew(void __iomem *addr, unsigned offset, u16 data)
	{ __raw_writew(data, addr + offset); }

static inline void musb_writel(void __iomem *addr, unsigned offset, u32 data)
	{ __raw_writel(data, addr + offset); }


#ifdef CONFIG_USB_TUSB6010


static inline u8 musb_readb(const void __iomem *addr, unsigned offset)
{
	u16 tmp;
	u8 val;

	tmp = __raw_readw(addr + (offset & ~1));
	if (offset & 1)
		val = (tmp >> 8);
	else
		val = tmp & 0xff;

	return val;
}

static inline void musb_writeb(void __iomem *addr, unsigned offset, u8 data)
{
	u16 tmp;

	tmp = __raw_readw(addr + (offset & ~1));
	if (offset & 1)
		tmp = (data << 8) | (tmp & 0xff);
	else
		tmp = (tmp & 0xff00) | data;

	__raw_writew(tmp, addr + (offset & ~1));
}

#else

static inline u8 musb_readb(const void __iomem *addr, unsigned offset)
	{ return __raw_readb(addr + offset); }

static inline void musb_writeb(void __iomem *addr, unsigned offset, u8 data)
	{ __raw_writeb(data, addr + offset); }

#endif	

#else

static inline u8 musb_readb(const void __iomem *addr, unsigned offset)
	{ return (u8) (bfin_read16(addr + offset)); }

static inline u16 musb_readw(const void __iomem *addr, unsigned offset)
	{ return bfin_read16(addr + offset); }

static inline u32 musb_readl(const void __iomem *addr, unsigned offset)
	{ return (u32) (bfin_read16(addr + offset)); }

static inline void musb_writeb(void __iomem *addr, unsigned offset, u8 data)
	{ bfin_write16(addr + offset, (u16) data); }

static inline void musb_writew(void __iomem *addr, unsigned offset, u16 data)
	{ bfin_write16(addr + offset, data); }

static inline void musb_writel(void __iomem *addr, unsigned offset, u32 data)
	{ bfin_write16(addr + offset, (u16) data); }

#endif 

#endif
