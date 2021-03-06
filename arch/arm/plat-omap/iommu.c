

#include <linux/err.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <asm/cacheflush.h>

#include <mach/iommu.h>

#include "iopgtable.h"


static const struct iommu_functions *arch_iommu;

static struct platform_driver omap_iommu_driver;
static struct kmem_cache *iopte_cachep;


int install_iommu_arch(const struct iommu_functions *ops)
{
	if (arch_iommu)
		return -EBUSY;

	arch_iommu = ops;
	return 0;
}
EXPORT_SYMBOL_GPL(install_iommu_arch);


void uninstall_iommu_arch(const struct iommu_functions *ops)
{
	if (arch_iommu != ops)
		pr_err("%s: not your arch\n", __func__);

	arch_iommu = NULL;
}
EXPORT_SYMBOL_GPL(uninstall_iommu_arch);


void iommu_save_ctx(struct iommu *obj)
{
	arch_iommu->save_ctx(obj);
}
EXPORT_SYMBOL_GPL(iommu_save_ctx);


void iommu_restore_ctx(struct iommu *obj)
{
	arch_iommu->restore_ctx(obj);
}
EXPORT_SYMBOL_GPL(iommu_restore_ctx);


u32 iommu_arch_version(void)
{
	return arch_iommu->version;
}
EXPORT_SYMBOL_GPL(iommu_arch_version);

static int iommu_enable(struct iommu *obj)
{
	int err;

	if (!obj)
		return -EINVAL;

	clk_enable(obj->clk);

	err = arch_iommu->enable(obj);

	clk_disable(obj->clk);
	return err;
}

static void iommu_disable(struct iommu *obj)
{
	if (!obj)
		return;

	clk_enable(obj->clk);

	arch_iommu->disable(obj);

	clk_disable(obj->clk);
}


void iotlb_cr_to_e(struct cr_regs *cr, struct iotlb_entry *e)
{
	BUG_ON(!cr || !e);

	arch_iommu->cr_to_e(cr, e);
}
EXPORT_SYMBOL_GPL(iotlb_cr_to_e);

static inline int iotlb_cr_valid(struct cr_regs *cr)
{
	if (!cr)
		return -EINVAL;

	return arch_iommu->cr_valid(cr);
}

static inline struct cr_regs *iotlb_alloc_cr(struct iommu *obj,
					     struct iotlb_entry *e)
{
	if (!e)
		return NULL;

	return arch_iommu->alloc_cr(obj, e);
}

u32 iotlb_cr_to_virt(struct cr_regs *cr)
{
	return arch_iommu->cr_to_virt(cr);
}
EXPORT_SYMBOL_GPL(iotlb_cr_to_virt);

static u32 get_iopte_attr(struct iotlb_entry *e)
{
	return arch_iommu->get_pte_attr(e);
}

static u32 iommu_report_fault(struct iommu *obj, u32 *da)
{
	return arch_iommu->fault_isr(obj, da);
}

static void iotlb_lock_get(struct iommu *obj, struct iotlb_lock *l)
{
	u32 val;

	val = iommu_read_reg(obj, MMU_LOCK);

	l->base = MMU_LOCK_BASE(val);
	l->vict = MMU_LOCK_VICT(val);

	BUG_ON(l->base != 0); 
}

static void iotlb_lock_set(struct iommu *obj, struct iotlb_lock *l)
{
	u32 val;

	BUG_ON(l->base != 0); 

	val = (l->base << MMU_LOCK_BASE_SHIFT);
	val |= (l->vict << MMU_LOCK_VICT_SHIFT);

	iommu_write_reg(obj, val, MMU_LOCK);
}

static void iotlb_read_cr(struct iommu *obj, struct cr_regs *cr)
{
	arch_iommu->tlb_read_cr(obj, cr);
}

static void iotlb_load_cr(struct iommu *obj, struct cr_regs *cr)
{
	arch_iommu->tlb_load_cr(obj, cr);

	iommu_write_reg(obj, 1, MMU_FLUSH_ENTRY);
	iommu_write_reg(obj, 1, MMU_LD_TLB);
}


static inline ssize_t iotlb_dump_cr(struct iommu *obj, struct cr_regs *cr,
				    char *buf)
{
	BUG_ON(!cr || !buf);

	return arch_iommu->dump_cr(obj, cr, buf);
}


int load_iotlb_entry(struct iommu *obj, struct iotlb_entry *e)
{
	int i;
	int err = 0;
	struct iotlb_lock l;
	struct cr_regs *cr;

	if (!obj || !obj->nr_tlb_entries || !e)
		return -EINVAL;

	clk_enable(obj->clk);

	for (i = 0; i < obj->nr_tlb_entries; i++) {
		struct cr_regs tmp;

		iotlb_lock_get(obj, &l);
		l.vict = i;
		iotlb_lock_set(obj, &l);
		iotlb_read_cr(obj, &tmp);
		if (!iotlb_cr_valid(&tmp))
			break;
	}

	if (i == obj->nr_tlb_entries) {
		dev_dbg(obj->dev, "%s: full: no entry\n", __func__);
		err = -EBUSY;
		goto out;
	}

	cr = iotlb_alloc_cr(obj, e);
	if (IS_ERR(cr)) {
		clk_disable(obj->clk);
		return PTR_ERR(cr);
	}

	iotlb_load_cr(obj, cr);
	kfree(cr);

	
	if (++l.vict == obj->nr_tlb_entries)
		l.vict = 0;
	iotlb_lock_set(obj, &l);
out:
	clk_disable(obj->clk);
	return err;
}
EXPORT_SYMBOL_GPL(load_iotlb_entry);


void flush_iotlb_page(struct iommu *obj, u32 da)
{
	struct iotlb_lock l;
	int i;

	clk_enable(obj->clk);

	for (i = 0; i < obj->nr_tlb_entries; i++) {
		struct cr_regs cr;
		u32 start;
		size_t bytes;

		iotlb_lock_get(obj, &l);
		l.vict = i;
		iotlb_lock_set(obj, &l);
		iotlb_read_cr(obj, &cr);
		if (!iotlb_cr_valid(&cr))
			continue;

		start = iotlb_cr_to_virt(&cr);
		bytes = iopgsz_to_bytes(cr.cam & 3);

		if ((start <= da) && (da < start + bytes)) {
			dev_dbg(obj->dev, "%s: %08x<=%08x(%x)\n",
				__func__, start, da, bytes);
			iotlb_load_cr(obj, &cr);
			iommu_write_reg(obj, 1, MMU_FLUSH_ENTRY);
		}
	}
	clk_disable(obj->clk);

	if (i == obj->nr_tlb_entries)
		dev_dbg(obj->dev, "%s: no page for %08x\n", __func__, da);
}
EXPORT_SYMBOL_GPL(flush_iotlb_page);


void flush_iotlb_range(struct iommu *obj, u32 start, u32 end)
{
	u32 da = start;

	while (da < end) {
		flush_iotlb_page(obj, da);
		
		da += IOPTE_SIZE;
	}
}
EXPORT_SYMBOL_GPL(flush_iotlb_range);


void flush_iotlb_all(struct iommu *obj)
{
	struct iotlb_lock l;

	clk_enable(obj->clk);

	l.base = 0;
	l.vict = 0;
	iotlb_lock_set(obj, &l);

	iommu_write_reg(obj, 1, MMU_GFLUSH);

	clk_disable(obj->clk);
}
EXPORT_SYMBOL_GPL(flush_iotlb_all);

#if defined(CONFIG_OMAP_IOMMU_DEBUG_MODULE)

ssize_t iommu_dump_ctx(struct iommu *obj, char *buf, ssize_t bytes)
{
	if (!obj || !buf)
		return -EINVAL;

	clk_enable(obj->clk);

	bytes = arch_iommu->dump_ctx(obj, buf, bytes);

	clk_disable(obj->clk);

	return bytes;
}
EXPORT_SYMBOL_GPL(iommu_dump_ctx);

static int __dump_tlb_entries(struct iommu *obj, struct cr_regs *crs, int num)
{
	int i;
	struct iotlb_lock saved, l;
	struct cr_regs *p = crs;

	clk_enable(obj->clk);

	iotlb_lock_get(obj, &saved);
	memcpy(&l, &saved, sizeof(saved));

	for (i = 0; i < num; i++) {
		struct cr_regs tmp;

		iotlb_lock_get(obj, &l);
		l.vict = i;
		iotlb_lock_set(obj, &l);
		iotlb_read_cr(obj, &tmp);
		if (!iotlb_cr_valid(&tmp))
			continue;

		*p++ = tmp;
	}
	iotlb_lock_set(obj, &saved);
	clk_disable(obj->clk);

	return  p - crs;
}


size_t dump_tlb_entries(struct iommu *obj, char *buf, ssize_t bytes)
{
	int i, num;
	struct cr_regs *cr;
	char *p = buf;

	num = bytes / sizeof(*cr);
	num = min(obj->nr_tlb_entries, num);

	cr = kcalloc(num, sizeof(*cr), GFP_KERNEL);
	if (!cr)
		return 0;

	num = __dump_tlb_entries(obj, cr, num);
	for (i = 0; i < num; i++)
		p += iotlb_dump_cr(obj, cr + i, p);
	kfree(cr);

	return p - buf;
}
EXPORT_SYMBOL_GPL(dump_tlb_entries);

int foreach_iommu_device(void *data, int (*fn)(struct device *, void *))
{
	return driver_for_each_device(&omap_iommu_driver.driver,
				      NULL, data, fn);
}
EXPORT_SYMBOL_GPL(foreach_iommu_device);

#endif 


static void flush_iopgd_range(u32 *first, u32 *last)
{
	
	do {
		asm("mcr	p15, 0, %0, c7, c10, 1 @ flush_pgd"
		    : : "r" (first));
		first += L1_CACHE_BYTES / sizeof(*first);
	} while (first <= last);
}

static void flush_iopte_range(u32 *first, u32 *last)
{
	
	do {
		asm("mcr	p15, 0, %0, c7, c10, 1 @ flush_pte"
		    : : "r" (first));
		first += L1_CACHE_BYTES / sizeof(*first);
	} while (first <= last);
}

static void iopte_free(u32 *iopte)
{
	
	kmem_cache_free(iopte_cachep, iopte);
}

static u32 *iopte_alloc(struct iommu *obj, u32 *iopgd, u32 da)
{
	u32 *iopte;

	
	if (*iopgd)
		goto pte_ready;

	
	spin_unlock(&obj->page_table_lock);
	iopte = kmem_cache_zalloc(iopte_cachep, GFP_KERNEL);
	spin_lock(&obj->page_table_lock);

	if (!*iopgd) {
		if (!iopte)
			return ERR_PTR(-ENOMEM);

		*iopgd = virt_to_phys(iopte) | IOPGD_TABLE;
		flush_iopgd_range(iopgd, iopgd);

		dev_vdbg(obj->dev, "%s: a new pte:%p\n", __func__, iopte);
	} else {
		
		iopte_free(iopte);
	}

pte_ready:
	iopte = iopte_offset(iopgd, da);

	dev_vdbg(obj->dev,
		 "%s: da:%08x pgd:%p *pgd:%08x pte:%p *pte:%08x\n",
		 __func__, da, iopgd, *iopgd, iopte, *iopte);

	return iopte;
}

static int iopgd_alloc_section(struct iommu *obj, u32 da, u32 pa, u32 prot)
{
	u32 *iopgd = iopgd_offset(obj, da);

	*iopgd = (pa & IOSECTION_MASK) | prot | IOPGD_SECTION;
	flush_iopgd_range(iopgd, iopgd);
	return 0;
}

static int iopgd_alloc_super(struct iommu *obj, u32 da, u32 pa, u32 prot)
{
	u32 *iopgd = iopgd_offset(obj, da);
	int i;

	for (i = 0; i < 16; i++)
		*(iopgd + i) = (pa & IOSUPER_MASK) | prot | IOPGD_SUPER;
	flush_iopgd_range(iopgd, iopgd + 15);
	return 0;
}

static int iopte_alloc_page(struct iommu *obj, u32 da, u32 pa, u32 prot)
{
	u32 *iopgd = iopgd_offset(obj, da);
	u32 *iopte = iopte_alloc(obj, iopgd, da);

	if (IS_ERR(iopte))
		return PTR_ERR(iopte);

	*iopte = (pa & IOPAGE_MASK) | prot | IOPTE_SMALL;
	flush_iopte_range(iopte, iopte);

	dev_vdbg(obj->dev, "%s: da:%08x pa:%08x pte:%p *pte:%08x\n",
		 __func__, da, pa, iopte, *iopte);

	return 0;
}

static int iopte_alloc_large(struct iommu *obj, u32 da, u32 pa, u32 prot)
{
	u32 *iopgd = iopgd_offset(obj, da);
	u32 *iopte = iopte_alloc(obj, iopgd, da);
	int i;

	if (IS_ERR(iopte))
		return PTR_ERR(iopte);

	for (i = 0; i < 16; i++)
		*(iopte + i) = (pa & IOLARGE_MASK) | prot | IOPTE_LARGE;
	flush_iopte_range(iopte, iopte + 15);
	return 0;
}

static int iopgtable_store_entry_core(struct iommu *obj, struct iotlb_entry *e)
{
	int (*fn)(struct iommu *, u32, u32, u32);
	u32 prot;
	int err;

	if (!obj || !e)
		return -EINVAL;

	switch (e->pgsz) {
	case MMU_CAM_PGSZ_16M:
		fn = iopgd_alloc_super;
		break;
	case MMU_CAM_PGSZ_1M:
		fn = iopgd_alloc_section;
		break;
	case MMU_CAM_PGSZ_64K:
		fn = iopte_alloc_large;
		break;
	case MMU_CAM_PGSZ_4K:
		fn = iopte_alloc_page;
		break;
	default:
		fn = NULL;
		BUG();
		break;
	}

	prot = get_iopte_attr(e);

	spin_lock(&obj->page_table_lock);
	err = fn(obj, e->da, e->pa, prot);
	spin_unlock(&obj->page_table_lock);

	return err;
}


int iopgtable_store_entry(struct iommu *obj, struct iotlb_entry *e)
{
	int err;

	flush_iotlb_page(obj, e->da);
	err = iopgtable_store_entry_core(obj, e);
#ifdef PREFETCH_IOTLB
	if (!err)
		load_iotlb_entry(obj, e);
#endif
	return err;
}
EXPORT_SYMBOL_GPL(iopgtable_store_entry);


void iopgtable_lookup_entry(struct iommu *obj, u32 da, u32 **ppgd, u32 **ppte)
{
	u32 *iopgd, *iopte = NULL;

	iopgd = iopgd_offset(obj, da);
	if (!*iopgd)
		goto out;

	if (*iopgd & IOPGD_TABLE)
		iopte = iopte_offset(iopgd, da);
out:
	*ppgd = iopgd;
	*ppte = iopte;
}
EXPORT_SYMBOL_GPL(iopgtable_lookup_entry);

static size_t iopgtable_clear_entry_core(struct iommu *obj, u32 da)
{
	size_t bytes;
	u32 *iopgd = iopgd_offset(obj, da);
	int nent = 1;

	if (!*iopgd)
		return 0;

	if (*iopgd & IOPGD_TABLE) {
		int i;
		u32 *iopte = iopte_offset(iopgd, da);

		bytes = IOPTE_SIZE;
		if (*iopte & IOPTE_LARGE) {
			nent *= 16;
			
			iopte = (u32 *)((u32)iopte & IOLARGE_MASK);
		}
		bytes *= nent;
		memset(iopte, 0, nent * sizeof(*iopte));
		flush_iopte_range(iopte, iopte + (nent - 1) * sizeof(*iopte));

		
		iopte = iopte_offset(iopgd, 0);
		for (i = 0; i < PTRS_PER_IOPTE; i++)
			if (iopte[i])
				goto out;

		iopte_free(iopte);
		nent = 1; 
	} else {
		bytes = IOPGD_SIZE;
		if ((*iopgd & IOPGD_SUPER) == IOPGD_SUPER) {
			nent *= 16;
			
			iopgd = (u32 *)((u32)iopgd & IOSUPER_MASK);
		}
		bytes *= nent;
	}
	memset(iopgd, 0, nent * sizeof(*iopgd));
	flush_iopgd_range(iopgd, iopgd + (nent - 1) * sizeof(*iopgd));
out:
	return bytes;
}


size_t iopgtable_clear_entry(struct iommu *obj, u32 da)
{
	size_t bytes;

	spin_lock(&obj->page_table_lock);

	bytes = iopgtable_clear_entry_core(obj, da);
	flush_iotlb_page(obj, da);

	spin_unlock(&obj->page_table_lock);

	return bytes;
}
EXPORT_SYMBOL_GPL(iopgtable_clear_entry);

static void iopgtable_clear_entry_all(struct iommu *obj)
{
	int i;

	spin_lock(&obj->page_table_lock);

	for (i = 0; i < PTRS_PER_IOPGD; i++) {
		u32 da;
		u32 *iopgd;

		da = i << IOPGD_SHIFT;
		iopgd = iopgd_offset(obj, da);

		if (!*iopgd)
			continue;

		if (*iopgd & IOPGD_TABLE)
			iopte_free(iopte_offset(iopgd, 0));

		*iopgd = 0;
		flush_iopgd_range(iopgd, iopgd);
	}

	flush_iotlb_all(obj);

	spin_unlock(&obj->page_table_lock);
}


static irqreturn_t iommu_fault_handler(int irq, void *data)
{
	u32 stat, da;
	u32 *iopgd, *iopte;
	int err = -EIO;
	struct iommu *obj = data;

	if (!obj->refcount)
		return IRQ_NONE;

	
	if (obj->isr)
		err = obj->isr(obj);

	if (!err)
		return IRQ_HANDLED;

	clk_enable(obj->clk);
	stat = iommu_report_fault(obj, &da);
	clk_disable(obj->clk);
	if (!stat)
		return IRQ_HANDLED;

	iopgd = iopgd_offset(obj, da);

	if (!(*iopgd & IOPGD_TABLE)) {
		dev_err(obj->dev, "%s: da:%08x pgd:%p *pgd:%08x\n", __func__,
			da, iopgd, *iopgd);
		return IRQ_NONE;
	}

	iopte = iopte_offset(iopgd, da);

	dev_err(obj->dev, "%s: da:%08x pgd:%p *pgd:%08x pte:%p *pte:%08x\n",
		__func__, da, iopgd, *iopgd, iopte, *iopte);

	return IRQ_NONE;
}

static int device_match_by_alias(struct device *dev, void *data)
{
	struct iommu *obj = to_iommu(dev);
	const char *name = data;

	pr_debug("%s: %s %s\n", __func__, obj->name, name);

	return strcmp(obj->name, name) == 0;
}


struct iommu *iommu_get(const char *name)
{
	int err = -ENOMEM;
	struct device *dev;
	struct iommu *obj;

	dev = driver_find_device(&omap_iommu_driver.driver, NULL, (void *)name,
				 device_match_by_alias);
	if (!dev)
		return ERR_PTR(-ENODEV);

	obj = to_iommu(dev);

	mutex_lock(&obj->iommu_lock);

	if (obj->refcount++ == 0) {
		err = iommu_enable(obj);
		if (err)
			goto err_enable;
		flush_iotlb_all(obj);
	}

	if (!try_module_get(obj->owner))
		goto err_module;

	mutex_unlock(&obj->iommu_lock);

	dev_dbg(obj->dev, "%s: %s\n", __func__, obj->name);
	return obj;

err_module:
	if (obj->refcount == 1)
		iommu_disable(obj);
err_enable:
	obj->refcount--;
	mutex_unlock(&obj->iommu_lock);
	return ERR_PTR(err);
}
EXPORT_SYMBOL_GPL(iommu_get);


void iommu_put(struct iommu *obj)
{
	if (!obj && IS_ERR(obj))
		return;

	mutex_lock(&obj->iommu_lock);

	if (--obj->refcount == 0)
		iommu_disable(obj);

	module_put(obj->owner);

	mutex_unlock(&obj->iommu_lock);

	dev_dbg(obj->dev, "%s: %s\n", __func__, obj->name);
}
EXPORT_SYMBOL_GPL(iommu_put);


static int __devinit omap_iommu_probe(struct platform_device *pdev)
{
	int err = -ENODEV;
	void *p;
	int irq;
	struct iommu *obj;
	struct resource *res;
	struct iommu_platform_data *pdata = pdev->dev.platform_data;

	if (pdev->num_resources != 2)
		return -EINVAL;

	obj = kzalloc(sizeof(*obj) + MMU_REG_SIZE, GFP_KERNEL);
	if (!obj)
		return -ENOMEM;

	obj->clk = clk_get(&pdev->dev, pdata->clk_name);
	if (IS_ERR(obj->clk))
		goto err_clk;

	obj->nr_tlb_entries = pdata->nr_tlb_entries;
	obj->name = pdata->name;
	obj->dev = &pdev->dev;
	obj->ctx = (void *)obj + sizeof(*obj);

	mutex_init(&obj->iommu_lock);
	mutex_init(&obj->mmap_lock);
	spin_lock_init(&obj->page_table_lock);
	INIT_LIST_HEAD(&obj->mmap);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		err = -ENODEV;
		goto err_mem;
	}
	obj->regbase = ioremap(res->start, resource_size(res));
	if (!obj->regbase) {
		err = -ENOMEM;
		goto err_mem;
	}

	res = request_mem_region(res->start, resource_size(res),
				 dev_name(&pdev->dev));
	if (!res) {
		err = -EIO;
		goto err_mem;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		err = -ENODEV;
		goto err_irq;
	}
	err = request_irq(irq, iommu_fault_handler, IRQF_SHARED,
			  dev_name(&pdev->dev), obj);
	if (err < 0)
		goto err_irq;
	platform_set_drvdata(pdev, obj);

	p = (void *)__get_free_pages(GFP_KERNEL, get_order(IOPGD_TABLE_SIZE));
	if (!p) {
		err = -ENOMEM;
		goto err_pgd;
	}
	memset(p, 0, IOPGD_TABLE_SIZE);
	clean_dcache_area(p, IOPGD_TABLE_SIZE);
	obj->iopgd = p;

	BUG_ON(!IS_ALIGNED((unsigned long)obj->iopgd, IOPGD_TABLE_SIZE));

	dev_info(&pdev->dev, "%s registered\n", obj->name);
	return 0;

err_pgd:
	free_irq(irq, obj);
err_irq:
	release_mem_region(res->start, resource_size(res));
	iounmap(obj->regbase);
err_mem:
	clk_put(obj->clk);
err_clk:
	kfree(obj);
	return err;
}

static int __devexit omap_iommu_remove(struct platform_device *pdev)
{
	int irq;
	struct resource *res;
	struct iommu *obj = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	iopgtable_clear_entry_all(obj);
	free_pages((unsigned long)obj->iopgd, get_order(IOPGD_TABLE_SIZE));

	irq = platform_get_irq(pdev, 0);
	free_irq(irq, obj);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, resource_size(res));
	iounmap(obj->regbase);

	clk_put(obj->clk);
	dev_info(&pdev->dev, "%s removed\n", obj->name);
	kfree(obj);
	return 0;
}

static struct platform_driver omap_iommu_driver = {
	.probe	= omap_iommu_probe,
	.remove	= __devexit_p(omap_iommu_remove),
	.driver	= {
		.name	= "omap-iommu",
	},
};

static void iopte_cachep_ctor(void *iopte)
{
	clean_dcache_area(iopte, IOPTE_TABLE_SIZE);
}

static int __init omap_iommu_init(void)
{
	struct kmem_cache *p;
	const unsigned long flags = SLAB_HWCACHE_ALIGN;
	size_t align = 1 << 10; 

	p = kmem_cache_create("iopte_cache", IOPTE_TABLE_SIZE, align, flags,
			      iopte_cachep_ctor);
	if (!p)
		return -ENOMEM;
	iopte_cachep = p;

	return platform_driver_register(&omap_iommu_driver);
}
module_init(omap_iommu_init);

static void __exit omap_iommu_exit(void)
{
	kmem_cache_destroy(iopte_cachep);

	platform_driver_unregister(&omap_iommu_driver);
}
module_exit(omap_iommu_exit);

MODULE_DESCRIPTION("omap iommu: tlb and pagetable primitives");
MODULE_ALIAS("platform:omap-iommu");
MODULE_AUTHOR("Hiroshi DOYU, Paul Mundt and Toshihiro Kobayashi");
MODULE_LICENSE("GPL v2");
