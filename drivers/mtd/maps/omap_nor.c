

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/slab.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>
#include <mach/hardware.h>
#include <asm/mach/flash.h>
#include <mach/tc.h>

#ifdef CONFIG_MTD_PARTITIONS
static const char *part_probes[] = {  "cmdlinepart", NULL };
#endif

struct omapflash_info {
	struct mtd_partition	*parts;
	struct mtd_info		*mtd;
	struct map_info		map;
};

static void omap_set_vpp(struct map_info *map, int enable)
{
	static int	count;
	u32 l;

	if (cpu_class_is_omap1()) {
		if (enable) {
			if (count++ == 0) {
				l = omap_readl(EMIFS_CONFIG);
				l |= OMAP_EMIFS_CONFIG_WP;
				omap_writel(l, EMIFS_CONFIG);
			}
		} else {
			if (count && (--count == 0)) {
				l = omap_readl(EMIFS_CONFIG);
				l &= ~OMAP_EMIFS_CONFIG_WP;
				omap_writel(l, EMIFS_CONFIG);
			}
		}
	}
}

static int __init omapflash_probe(struct platform_device *pdev)
{
	int err;
	struct omapflash_info *info;
	struct flash_platform_data *pdata = pdev->dev.platform_data;
	struct resource *res = pdev->resource;
	unsigned long size = res->end - res->start + 1;

	info = kzalloc(sizeof(struct omapflash_info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	if (!request_mem_region(res->start, size, "flash")) {
		err = -EBUSY;
		goto out_free_info;
	}

	info->map.virt		= ioremap(res->start, size);
	if (!info->map.virt) {
		err = -ENOMEM;
		goto out_release_mem_region;
	}
	info->map.name		= dev_name(&pdev->dev);
	info->map.phys		= res->start;
	info->map.size		= size;
	info->map.bankwidth	= pdata->width;
	info->map.set_vpp	= omap_set_vpp;

	simple_map_init(&info->map);
	info->mtd = do_map_probe(pdata->map_name, &info->map);
	if (!info->mtd) {
		err = -EIO;
		goto out_iounmap;
	}
	info->mtd->owner = THIS_MODULE;

	info->mtd->dev.parent = &pdev->dev;

#ifdef CONFIG_MTD_PARTITIONS
	err = parse_mtd_partitions(info->mtd, part_probes, &info->parts, 0);
	if (err > 0)
		add_mtd_partitions(info->mtd, info->parts, err);
	else if (err <= 0 && pdata->parts)
		add_mtd_partitions(info->mtd, pdata->parts, pdata->nr_parts);
	else
#endif
		add_mtd_device(info->mtd);

	platform_set_drvdata(pdev, info);

	return 0;

out_iounmap:
	iounmap(info->map.virt);
out_release_mem_region:
	release_mem_region(res->start, size);
out_free_info:
	kfree(info);

	return err;
}

static int __exit omapflash_remove(struct platform_device *pdev)
{
	struct omapflash_info *info = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	if (info) {
		if (info->parts) {
			del_mtd_partitions(info->mtd);
			kfree(info->parts);
		} else
			del_mtd_device(info->mtd);
		map_destroy(info->mtd);
		release_mem_region(info->map.phys, info->map.size);
		iounmap((void __iomem *) info->map.virt);
		kfree(info);
	}

	return 0;
}

static struct platform_driver omapflash_driver = {
	.remove	= __exit_p(omapflash_remove),
	.driver = {
		.name	= "omapflash",
		.owner	= THIS_MODULE,
	},
};

static int __init omapflash_init(void)
{
	return platform_driver_probe(&omapflash_driver, omapflash_probe);
}

static void __exit omapflash_exit(void)
{
	platform_driver_unregister(&omapflash_driver);
}

module_init(omapflash_init);
module_exit(omapflash_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MTD NOR map driver for TI OMAP boards");
MODULE_ALIAS("platform:omapflash");
