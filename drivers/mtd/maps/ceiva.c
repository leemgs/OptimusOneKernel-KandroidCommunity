

#include <linux/module.h>
#include <linux/types.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/concat.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/sizes.h>


#define CONFIG_MTD_CEIVA_STATICMAP

#ifdef CONFIG_MTD_CEIVA_STATICMAP


#ifdef CONFIG_ARCH_CEIVA


#define MAX_SIZE_KiB                  (16 + 8 + 8 + 96 + (7*128))
#define BOOT_PARTITION_SIZE_KiB       (16)
#define PARAMS_PARTITION_SIZE_KiB     (8)
#define KERNEL_PARTITION_SIZE_KiB     (4*128)

#define ROOT_PARTITION_SIZE_KiB       (3*128) + (8*128)

static struct mtd_partition ceiva_partitions[] = {
	{
		.name = "Ceiva BOOT partition",
		.size   = BOOT_PARTITION_SIZE_KiB*1024,
		.offset = 0,

	},{
		.name = "Ceiva parameters partition",
		.size   = PARAMS_PARTITION_SIZE_KiB*1024,
		.offset = (16 + 8) * 1024,
	},{
		.name = "Ceiva kernel partition",
		.size = (KERNEL_PARTITION_SIZE_KiB)*1024,
		.offset = 0x20000,

	},{
		.name = "Ceiva root filesystem partition",
		.offset = MTDPART_OFS_APPEND,
		.size = (ROOT_PARTITION_SIZE_KiB)*1024,
	}
};
#endif

static int __init clps_static_partitions(struct mtd_partition **parts)
{
	int nb_parts = 0;

#ifdef CONFIG_ARCH_CEIVA
	if (machine_is_ceiva()) {
		*parts       = ceiva_partitions;
		nb_parts     = ARRAY_SIZE(ceiva_partitions);
	}
#endif
	return nb_parts;
}
#endif

struct clps_info {
	unsigned long base;
	unsigned long size;
	int width;
	void *vbase;
	struct map_info *map;
	struct mtd_info *mtd;
	struct resource *res;
};

#define NR_SUBMTD 4

static struct clps_info info[NR_SUBMTD];

static int __init clps_setup_mtd(struct clps_info *clps, int nr, struct mtd_info **rmtd)
{
	struct mtd_info *subdev[nr];
	struct map_info *maps;
	int i, found = 0, ret = 0;

	
	maps = kzalloc(sizeof(struct map_info) * nr, GFP_KERNEL);
	if (!maps)
		return -ENOMEM;
	
	for (i = 0; i < nr; i++) {
		if (clps[i].base == (unsigned long)-1)
			break;

		clps[i].res = request_mem_region(clps[i].base, clps[i].size, "clps flash");
		if (!clps[i].res) {
			ret = -EBUSY;
			break;
		}

		clps[i].map = maps + i;

		clps[i].map->name = "clps flash";
		clps[i].map->phys = clps[i].base;

		clps[i].vbase = ioremap(clps[i].base, clps[i].size);
		if (!clps[i].vbase) {
			ret = -ENOMEM;
			break;
		}

		clps[i].map->virt = (void __iomem *)clps[i].vbase;
		clps[i].map->bankwidth = clps[i].width;
		clps[i].map->size = clps[i].size;

		simple_map_init(&clps[i].map);

		clps[i].mtd = do_map_probe("jedec_probe", clps[i].map);
		if (clps[i].mtd == NULL) {
			ret = -ENXIO;
			break;
		}
		clps[i].mtd->owner = THIS_MODULE;
		subdev[i] = clps[i].mtd;

		printk(KERN_INFO "clps flash: JEDEC device at 0x%08lx, %dMiB, "
			"%d-bit\n", clps[i].base, clps[i].mtd->size >> 20,
			clps[i].width * 8);
		found += 1;
	}

	
	if (ret == -ENXIO) {
		iounmap(clps[i].vbase);
		clps[i].vbase = NULL;
		release_resource(clps[i].res);
		clps[i].res = NULL;
	}

	
	if (ret == 0 || ret == -ENXIO) {
		if (found == 1) {
			*rmtd = subdev[0];
			ret = 0;
		} else if (found > 1) {
			
#ifdef CONFIG_MTD_CONCAT
			*rmtd = mtd_concat_create(subdev, found,
						  "clps flash");
			if (*rmtd == NULL)
				ret = -ENXIO;
#else
			printk(KERN_ERR "clps flash: multiple devices "
			       "found but MTD concat support disabled.\n");
			ret = -ENXIO;
#endif
		}
	}

	
	if (ret) {
		do {
			if (clps[i].mtd)
				map_destroy(clps[i].mtd);
			if (clps[i].vbase)
				iounmap(clps[i].vbase);
			if (clps[i].res)
				release_resource(clps[i].res);
		} while (i--);

		kfree(maps);
	}

	return ret;
}

static void __exit clps_destroy_mtd(struct clps_info *clps, struct mtd_info *mtd)
{
	int i;

	del_mtd_partitions(mtd);

	if (mtd != clps[0].mtd)
		mtd_concat_destroy(mtd);

	for (i = NR_SUBMTD; i >= 0; i--) {
		if (clps[i].mtd)
			map_destroy(clps[i].mtd);
		if (clps[i].vbase)
			iounmap(clps[i].vbase);
		if (clps[i].res)
			release_resource(clps[i].res);
	}
	kfree(clps[0].map);
}



static int __init clps_setup_flash(void)
{
	int nr;

#ifdef CONFIG_ARCH_CEIVA
	if (machine_is_ceiva()) {
		info[0].base = CS0_PHYS_BASE;
		info[0].size = SZ_32M;
		info[0].width = CEIVA_FLASH_WIDTH;
		info[1].base = CS1_PHYS_BASE;
		info[1].size = SZ_32M;
		info[1].width = CEIVA_FLASH_WIDTH;
		nr = 2;
	}
#endif
	return nr;
}

static struct mtd_partition *parsed_parts;
static const char *probes[] = { "cmdlinepart", "RedBoot", NULL };

static void __init clps_locate_partitions(struct mtd_info *mtd)
{
	const char *part_type = NULL;
	int nr_parts = 0;
	do {
		
		nr_parts = parse_mtd_partitions(mtd, probes, &parsed_parts, 0);
		if (nr_parts > 0) {
			part_type = "command line";
			break;
		}
#ifdef CONFIG_MTD_CEIVA_STATICMAP
		nr_parts = clps_static_partitions(&parsed_parts);
		if (nr_parts > 0) {
			part_type = "static";
			break;
		}
		printk("found: %d partitions\n", nr_parts);
#endif
	} while (0);

	if (nr_parts == 0) {
		printk(KERN_NOTICE "clps flash: no partition info "
			"available, registering whole flash\n");
		add_mtd_device(mtd);
	} else {
		printk(KERN_NOTICE "clps flash: using %s partition "
			"definition\n", part_type);
		add_mtd_partitions(mtd, parsed_parts, nr_parts);
	}

	
}

static void __exit clps_destroy_partitions(void)
{
	kfree(parsed_parts);
}

static struct mtd_info *mymtd;

static int __init clps_mtd_init(void)
{
	int ret;
	int nr;

	nr = clps_setup_flash();
	if (nr < 0)
		return nr;

	ret = clps_setup_mtd(info, nr, &mymtd);
	if (ret)
		return ret;

	clps_locate_partitions(mymtd);

	return 0;
}

static void __exit clps_mtd_cleanup(void)
{
	clps_destroy_mtd(info, mymtd);
	clps_destroy_partitions();
}

module_init(clps_mtd_init);
module_exit(clps_mtd_cleanup);

MODULE_AUTHOR("Rob Scott");
MODULE_DESCRIPTION("Cirrus Logic JEDEC map driver");
MODULE_LICENSE("GPL");
