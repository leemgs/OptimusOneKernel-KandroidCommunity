
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#include <asm/hardware/cache-l2x0.h>

#define CACHE_LINE_SIZE		32

static void __iomem *l2x0_base;
static uint32_t aux_ctrl_save;
static DEFINE_SPINLOCK(l2x0_lock);

static inline void sync_writel(unsigned long val, unsigned long reg,
			       unsigned long complete_mask)
{
	unsigned long flags;

	spin_lock_irqsave(&l2x0_lock, flags);
	writel(val, l2x0_base + reg);
	
	while (readl(l2x0_base + reg) & complete_mask)
		;
	spin_unlock_irqrestore(&l2x0_lock, flags);
}

static inline void cache_sync(void)
{
	sync_writel(0, L2X0_CACHE_SYNC, 1);
}

void l2x0_cache_sync(void)
{
	cache_sync();
}

static inline void l2x0_inv_all(void)
{
	
	sync_writel(0xff, L2X0_INV_WAY, 0xff);
	cache_sync();
}

static inline void l2x0_flush_all(void)
{
	
	sync_writel(0xff, L2X0_CLEAN_INV_WAY, 0xff);
	cache_sync();
}

static void l2x0_inv_range(unsigned long start, unsigned long end)
{
	unsigned long addr;

	if (start & (CACHE_LINE_SIZE - 1)) {
		start &= ~(CACHE_LINE_SIZE - 1);
		sync_writel(start, L2X0_CLEAN_INV_LINE_PA, 1);
		start += CACHE_LINE_SIZE;
	}

	if (end & (CACHE_LINE_SIZE - 1)) {
		end &= ~(CACHE_LINE_SIZE - 1);
		sync_writel(end, L2X0_CLEAN_INV_LINE_PA, 1);
	}

	for (addr = start; addr < end; addr += CACHE_LINE_SIZE)
		sync_writel(addr, L2X0_INV_LINE_PA, 1);
	cache_sync();
}

static void l2x0_inv_range_atomic(unsigned long start, unsigned long end)
{
	unsigned long addr;

	if (start & (CACHE_LINE_SIZE - 1)) {
		start &= ~(CACHE_LINE_SIZE - 1);
		writel(start, l2x0_base + L2X0_CLEAN_INV_LINE_PA);
		start += CACHE_LINE_SIZE;
	}

	if (end & (CACHE_LINE_SIZE - 1)) {
		end &= ~(CACHE_LINE_SIZE - 1);
		writel(end, l2x0_base + L2X0_CLEAN_INV_LINE_PA);
	}

	for (addr = start; addr < end; addr += CACHE_LINE_SIZE)
		writel(addr, l2x0_base + L2X0_INV_LINE_PA);
}

static void l2x0_clean_range(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start &= ~(CACHE_LINE_SIZE - 1);
	for (addr = start; addr < end; addr += CACHE_LINE_SIZE)
		sync_writel(addr, L2X0_CLEAN_LINE_PA, 1);
	cache_sync();
}

static void l2x0_clean_range_atomic(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start &= ~(CACHE_LINE_SIZE - 1);
	for (addr = start; addr < end; addr += CACHE_LINE_SIZE)
		writel(addr, l2x0_base + L2X0_CLEAN_LINE_PA);
}

static void l2x0_flush_range(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start &= ~(CACHE_LINE_SIZE - 1);
	for (addr = start; addr < end; addr += CACHE_LINE_SIZE)
		sync_writel(addr, L2X0_CLEAN_INV_LINE_PA, 1);
	cache_sync();
}

void l2x0_flush_range_atomic(unsigned long start, unsigned long end)
{
	unsigned long addr;

	start &= ~(CACHE_LINE_SIZE - 1);
	for (addr = start; addr < end; addr += CACHE_LINE_SIZE)
		writel(addr, l2x0_base + L2X0_CLEAN_INV_LINE_PA);
}

void __init l2x0_init(void __iomem *base, __u32 aux_val, __u32 aux_mask)
{
	__u32 bits;

	l2x0_base = base;

	
	bits = readl(l2x0_base + L2X0_CTRL);
	bits &= ~0x01;	
	writel(bits, l2x0_base + L2X0_CTRL);

	bits = readl(l2x0_base + L2X0_AUX_CTRL);
	bits &= aux_mask;
	bits |= aux_val;
	writel(bits, l2x0_base + L2X0_AUX_CTRL);

	l2x0_inv_all();

	
	bits = readl(l2x0_base + L2X0_CTRL);
	bits |= 0x01;	
	writel(bits, l2x0_base + L2X0_CTRL);

	bits = readl(l2x0_base + L2X0_CACHE_ID);
	bits >>= 6;	
	bits &= 0x0f;	

	if (bits == 2) {	
		outer_cache.inv_range = l2x0_inv_range;
		outer_cache.clean_range = l2x0_clean_range;
		outer_cache.flush_range = l2x0_flush_range;
		printk(KERN_INFO "L220 cache controller enabled\n");
	} else {		
		outer_cache.inv_range = l2x0_inv_range_atomic;
		outer_cache.clean_range = l2x0_clean_range_atomic;
		outer_cache.flush_range = l2x0_flush_range_atomic;
		printk(KERN_INFO "L210 cache controller enabled\n");
	}

}

void l2x0_suspend(void)
{
	
	aux_ctrl_save = readl(l2x0_base + L2X0_AUX_CTRL);
	
	l2x0_flush_all();
	
	writel(0, l2x0_base + L2X0_CTRL);

	
	dmb();
}

void l2x0_resume(int collapsed)
{
	if (collapsed) {
		
		writel(0, l2x0_base + L2X0_CTRL);

		
		writel(aux_ctrl_save, l2x0_base + L2X0_AUX_CTRL);

		
		l2x0_inv_all();
	}

	
	writel(1, l2x0_base + L2X0_CTRL);
}
