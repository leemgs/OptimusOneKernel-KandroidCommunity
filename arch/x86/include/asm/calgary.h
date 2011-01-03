

#ifndef _ASM_X86_CALGARY_H
#define _ASM_X86_CALGARY_H

#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/timer.h>
#include <asm/types.h>

struct iommu_table {
	struct cal_chipset_ops *chip_ops; 
	unsigned long  it_base;      
	unsigned long  it_hint;      
	unsigned long *it_map;       
	void __iomem  *bbar;         
	u64	       tar_val;      
	struct timer_list watchdog_timer;
	spinlock_t     it_lock;      
	unsigned int   it_size;      
	unsigned char  it_busno;     
};

struct cal_chipset_ops {
	void (*handle_quirks)(struct iommu_table *tbl, struct pci_dev *dev);
	void (*tce_cache_blast)(struct iommu_table *tbl);
	void (*dump_error_regs)(struct iommu_table *tbl);
};

#define TCE_TABLE_SIZE_UNSPECIFIED	~0
#define TCE_TABLE_SIZE_64K		0
#define TCE_TABLE_SIZE_128K		1
#define TCE_TABLE_SIZE_256K		2
#define TCE_TABLE_SIZE_512K		3
#define TCE_TABLE_SIZE_1M		4
#define TCE_TABLE_SIZE_2M		5
#define TCE_TABLE_SIZE_4M		6
#define TCE_TABLE_SIZE_8M		7

extern int use_calgary;

#ifdef CONFIG_CALGARY_IOMMU
extern int calgary_iommu_init(void);
extern void detect_calgary(void);
#else
static inline int calgary_iommu_init(void) { return 1; }
static inline void detect_calgary(void) { return; }
#endif

#endif 
