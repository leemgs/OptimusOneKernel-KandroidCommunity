

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/plat-ram.h>

#include <asm/io.h>



struct platram_info {
	struct device		*dev;
	struct mtd_info		*mtd;
	struct map_info		 map;
	struct mtd_partition	*partitions;
	bool			free_partitions;
	struct resource		*area;
	struct platdata_mtd_ram	*pdata;
};



static inline struct platram_info *to_platram_info(struct platform_device *dev)
{
	return (struct platram_info *)platform_get_drvdata(dev);
}



static inline void platram_setrw(struct platram_info *info, int to)
{
	if (info->pdata == NULL)
		return;

	if (info->pdata->set_rw != NULL)
		(info->pdata->set_rw)(info->dev, to);
}



static int platram_remove(struct platform_device *pdev)
{
	struct platram_info *info = to_platram_info(pdev);

	platform_set_drvdata(pdev, NULL);

	dev_dbg(&pdev->dev, "removing device\n");

	if (info == NULL)
		return 0;

	if (info->mtd) {
#ifdef CONFIG_MTD_PARTITIONS
		if (info->partitions) {
			del_mtd_partitions(info->mtd);
			if (info->free_partitions)
				kfree(info->partitions);
		}
#endif
		del_mtd_device(info->mtd);
		map_destroy(info->mtd);
	}

	

	platram_setrw(info, PLATRAM_RO);

	

	if (info->area) {
		release_resource(info->area);
		kfree(info->area);
	}

	if (info->map.virt != NULL)
		iounmap(info->map.virt);

	kfree(info);

	return 0;
}



static int platram_probe(struct platform_device *pdev)
{
	struct platdata_mtd_ram	*pdata;
	struct platram_info *info;
	struct resource *res;
	int err = 0;

	dev_dbg(&pdev->dev, "probe entered\n");

	if (pdev->dev.platform_data == NULL) {
		dev_err(&pdev->dev, "no platform data supplied\n");
		err = -ENOENT;
		goto exit_error;
	}

	pdata = pdev->dev.platform_data;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL) {
		dev_err(&pdev->dev, "no memory for flash info\n");
		err = -ENOMEM;
		goto exit_error;
	}

	platform_set_drvdata(pdev, info);

	info->dev = &pdev->dev;
	info->pdata = pdata;

	

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

	if (res == NULL) {
		dev_err(&pdev->dev, "no memory resource specified\n");
		err = -ENOENT;
		goto exit_free;
	}

	dev_dbg(&pdev->dev, "got platform resource %p (0x%llx)\n", res,
		(unsigned long long)res->start);

	

	info->map.phys = res->start;
	info->map.size = resource_size(res);
	info->map.name = pdata->mapname != NULL ?
			(char *)pdata->mapname : (char *)pdev->name;
	info->map.bankwidth = pdata->bankwidth;

	

	info->area = request_mem_region(res->start, info->map.size, pdev->name);
	if (info->area == NULL) {
		dev_err(&pdev->dev, "failed to request memory region\n");
		err = -EIO;
		goto exit_free;
	}

	

	info->map.virt = ioremap(res->start, info->map.size);
	dev_dbg(&pdev->dev, "virt %p, %lu bytes\n", info->map.virt, info->map.size);

	if (info->map.virt == NULL) {
		dev_err(&pdev->dev, "failed to ioremap() region\n");
		err = -EIO;
		goto exit_free;
	}

	simple_map_init(&info->map);

	dev_dbg(&pdev->dev, "initialised map, probing for mtd\n");

	

	if (pdata->map_probes) {
		const char **map_probes = pdata->map_probes;

		for ( ; !info->mtd && *map_probes; map_probes++)
			info->mtd = do_map_probe(*map_probes , &info->map);
	}
	
	else
		info->mtd = do_map_probe("map_ram", &info->map);

	if (info->mtd == NULL) {
		dev_err(&pdev->dev, "failed to probe for map_ram\n");
		err = -ENOMEM;
		goto exit_free;
	}

	info->mtd->owner = THIS_MODULE;
	info->mtd->dev.parent = &pdev->dev;

	platram_setrw(info, PLATRAM_RW);

	

#ifdef CONFIG_MTD_PARTITIONS
	if (!pdata->nr_partitions) {
		
		if (pdata->probes) {
			err = parse_mtd_partitions(info->mtd, pdata->probes,
					   &info->partitions, 0);
			info->free_partitions = 1;
			if (err > 0)
				err = add_mtd_partitions(info->mtd,
					info->partitions, err);
		}
	}
	
	else
		err = add_mtd_partitions(info->mtd, pdata->partitions,
				pdata->nr_partitions);
#endif 

	if (add_mtd_device(info->mtd)) {
		dev_err(&pdev->dev, "add_mtd_device() failed\n");
		err = -ENOMEM;
	}

	if (!err)
		dev_info(&pdev->dev, "registered mtd device\n");

	return err;

 exit_free:
	platram_remove(pdev);
 exit_error:
	return err;
}




MODULE_ALIAS("platform:mtd-ram");

static struct platform_driver platram_driver = {
	.probe		= platram_probe,
	.remove		= platram_remove,
	.driver		= {
		.name	= "mtd-ram",
		.owner	= THIS_MODULE,
	},
};



static int __init platram_init(void)
{
	printk("Generic platform RAM MTD, (c) 2004 Simtec Electronics\n");
	return platform_driver_register(&platram_driver);
}

static void __exit platram_exit(void)
{
	platform_driver_unregister(&platram_driver);
}

module_init(platram_init);
module_exit(platram_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ben Dooks <ben@simtec.co.uk>");
MODULE_DESCRIPTION("MTD platform RAM map driver");
