

#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/platform_device.h>
#include <linux/types.h>

#define pr_devinit(fmt, args...) ({ static const __devinitconst char __fmt[] = fmt; printk(__fmt, ## args); })

#define DRIVER_NAME "gpio-addr-flash"
#define PFX DRIVER_NAME ": "


struct async_state {
	struct mtd_info *mtd;
	struct map_info map;
	size_t gpio_count;
	unsigned *gpio_addrs;
	int *gpio_values;
	unsigned long win_size;
};
#define gf_map_info_to_state(mi) ((struct async_state *)(mi)->map_priv_1)


static void gf_set_gpios(struct async_state *state, unsigned long ofs)
{
	size_t i = 0;
	int value;
	ofs /= state->win_size;
	do {
		value = ofs & (1 << i);
		if (state->gpio_values[i] != value) {
			gpio_set_value(state->gpio_addrs[i], value);
			state->gpio_values[i] = value;
		}
	} while (++i < state->gpio_count);
}


static map_word gf_read(struct map_info *map, unsigned long ofs)
{
	struct async_state *state = gf_map_info_to_state(map);
	uint16_t word;
	map_word test;

	gf_set_gpios(state, ofs);

	word = readw(map->virt + (ofs % state->win_size));
	test.x[0] = word;
	return test;
}


static void gf_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	struct async_state *state = gf_map_info_to_state(map);

	gf_set_gpios(state, from);

	
	BUG_ON(!((from + len) % state->win_size <= (from + len)));

	
	memcpy_fromio(to, map->virt + (from % state->win_size), len);
}


static void gf_write(struct map_info *map, map_word d1, unsigned long ofs)
{
	struct async_state *state = gf_map_info_to_state(map);
	uint16_t d;

	gf_set_gpios(state, ofs);

	d = d1.x[0];
	writew(d, map->virt + (ofs % state->win_size));
}


static void gf_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	struct async_state *state = gf_map_info_to_state(map);

	gf_set_gpios(state, to);

	
	BUG_ON(!((to + len) % state->win_size <= (to + len)));

	
	memcpy_toio(map->virt + (to % state->win_size), from, len);
}

#ifdef CONFIG_MTD_PARTITIONS
static const char *part_probe_types[] = { "cmdlinepart", "RedBoot", NULL };
#endif


static int __devinit gpio_flash_probe(struct platform_device *pdev)
{
	int ret;
	size_t i, arr_size;
	struct physmap_flash_data *pdata;
	struct resource *memory;
	struct resource *gpios;
	struct async_state *state;

	pdata = pdev->dev.platform_data;
	memory = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpios = platform_get_resource(pdev, IORESOURCE_IRQ, 0);

	if (!memory || !gpios || !gpios->end)
		return -EINVAL;

	arr_size = sizeof(int) * gpios->end;
	state = kzalloc(sizeof(*state) + arr_size, GFP_KERNEL);
	if (!state)
		return -ENOMEM;

	state->gpio_count     = gpios->end;
	state->gpio_addrs     = (void *)gpios->start;
	state->gpio_values    = (void *)(state + 1);
	state->win_size       = memory->end - memory->start + 1;
	memset(state->gpio_values, 0xff, arr_size);

	state->map.name       = DRIVER_NAME;
	state->map.read       = gf_read;
	state->map.copy_from  = gf_copy_from;
	state->map.write      = gf_write;
	state->map.copy_to    = gf_copy_to;
	state->map.bankwidth  = pdata->width;
	state->map.size       = state->win_size * (1 << state->gpio_count);
	state->map.virt       = (void __iomem *)memory->start;
	state->map.phys       = NO_XIP;
	state->map.map_priv_1 = (unsigned long)state;

	platform_set_drvdata(pdev, state);

	i = 0;
	do {
		if (gpio_request(state->gpio_addrs[i], DRIVER_NAME)) {
			pr_devinit(KERN_ERR PFX "failed to request gpio %d\n",
				state->gpio_addrs[i]);
			while (i--)
				gpio_free(state->gpio_addrs[i]);
			kfree(state);
			return -EBUSY;
		}
		gpio_direction_output(state->gpio_addrs[i], 0);
	} while (++i < state->gpio_count);

	pr_devinit(KERN_NOTICE PFX "probing %d-bit flash bus\n",
		state->map.bankwidth * 8);
	state->mtd = do_map_probe(memory->name, &state->map);
	if (!state->mtd) {
		for (i = 0; i < state->gpio_count; ++i)
			gpio_free(state->gpio_addrs[i]);
		kfree(state);
		return -ENXIO;
	}

#ifdef CONFIG_MTD_PARTITIONS
	ret = parse_mtd_partitions(state->mtd, part_probe_types, &pdata->parts, 0);
	if (ret > 0) {
		pr_devinit(KERN_NOTICE PFX "Using commandline partition definition\n");
		add_mtd_partitions(state->mtd, pdata->parts, ret);
		kfree(pdata->parts);

	} else if (pdata->nr_parts) {
		pr_devinit(KERN_NOTICE PFX "Using board partition definition\n");
		add_mtd_partitions(state->mtd, pdata->parts, pdata->nr_parts);

	} else
#endif
	{
		pr_devinit(KERN_NOTICE PFX "no partition info available, registering whole flash at once\n");
		add_mtd_device(state->mtd);
	}

	return 0;
}

static int __devexit gpio_flash_remove(struct platform_device *pdev)
{
	struct async_state *state = platform_get_drvdata(pdev);
	size_t i = 0;
	do {
		gpio_free(state->gpio_addrs[i]);
	} while (++i < state->gpio_count);
#ifdef CONFIG_MTD_PARTITIONS
	del_mtd_partitions(state->mtd);
#endif
	map_destroy(state->mtd);
	kfree(state);
	return 0;
}

static struct platform_driver gpio_flash_driver = {
	.probe		= gpio_flash_probe,
	.remove		= __devexit_p(gpio_flash_remove),
	.driver		= {
		.name	= DRIVER_NAME,
	},
};

static int __init gpio_flash_init(void)
{
	return platform_driver_register(&gpio_flash_driver);
}
module_init(gpio_flash_init);

static void __exit gpio_flash_exit(void)
{
	platform_driver_unregister(&gpio_flash_driver);
}
module_exit(gpio_flash_exit);

MODULE_AUTHOR("Mike Frysinger <vapier@gentoo.org>");
MODULE_DESCRIPTION("MTD map driver for flashes addressed physically and with gpios");
MODULE_LICENSE("GPL");
