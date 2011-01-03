

#include <linux/module.h>
#include <linux/io.h>

#include <asm/tlb.h>

#define BETWEEN(p, st, sz)	((p) >= (st) && (p) < ((st) + (sz)))
#define XLATE(p, pst, vst)	((void __iomem *)((p) - (pst) + (vst)))


void __iomem *davinci_ioremap(unsigned long p, size_t size, unsigned int type)
{
	if (BETWEEN(p, IO_PHYS, IO_SIZE))
		return XLATE(p, IO_PHYS, IO_VIRT);

	return __arm_ioremap(p, size, type);
}
EXPORT_SYMBOL(davinci_ioremap);

void davinci_iounmap(volatile void __iomem *addr)
{
	unsigned long virt = (unsigned long)addr;

	if (virt >= VMALLOC_START && virt < VMALLOC_END)
		__iounmap(addr);
}
EXPORT_SYMBOL(davinci_iounmap);
