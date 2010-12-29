

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/debugfs.h>
#include <linux/clk.h>
#include "clock.h"

static struct clk *msm_clock_get_nth(unsigned index)
{
	if (index < msm_num_clocks)
		return msm_clocks + index;
	else
		return 0;
}

static int clock_debug_rate_set(void *data, u64 val)
{
	struct clk *clock = data;
	int ret;

	
	if (clock->flags & CLK_MAX)
		clk_set_max_rate(clock, val);
	if (clock->flags & CLK_MIN)
		ret = clk_set_min_rate(clock, val);
	else
		ret = clk_set_rate(clock, val);
	if (ret != 0)
		printk(KERN_ERR "clk_set%s_rate failed (%d)\n",
			(clock->flags & CLK_MIN) ? "_min" : "", ret);
	return ret;
}

static int clock_debug_rate_get(void *data, u64 *val)
{
	struct clk *clock = data;
	*val = clk_get_rate(clock);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_rate_fops, clock_debug_rate_get,
			clock_debug_rate_set, "%llu\n");

static int clock_debug_measure_get(void *data, u64 *val)
{
	struct clk *clock = data;
	*val = clock->ops->measure_rate(clock->id);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_measure_fops, clock_debug_measure_get,
			NULL, "%lld\n");

static int clock_debug_enable_set(void *data, u64 val)
{
	struct clk *clock = data;
	int rc = 0;

	if (val)
		rc = clock->ops->enable(clock->id);
	else
		clock->ops->disable(clock->id);

	return rc;
}

static int clock_debug_enable_get(void *data, u64 *val)
{
	struct clk *clock = data;

	*val = clock->ops->is_enabled(clock->id);

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_enable_fops, clock_debug_enable_get,
			clock_debug_enable_set, "%llu\n");

static int clock_debug_local_get(void *data, u64 *val)
{
	struct clk *clock = data;

	*val = clock->ops != &clk_ops_remote;

	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(clock_local_fops, clock_debug_local_get,
			NULL, "%llu\n");

static int __init clock_debug_init(void)
{
	struct dentry *base_dir;
	struct clk *clock;
	unsigned n = 0;
	char temp[50], *ptr;

	base_dir = debugfs_create_dir("clk", NULL);
	if (!base_dir)
		return -ENOMEM;

	while ((clock = msm_clock_get_nth(n++)) != 0) {
		struct dentry *clk_dir;

		strncpy(temp, clock->dbg_name, ARRAY_SIZE(temp)-1);
		for (ptr = temp; *ptr; ptr++)
			*ptr = tolower(*ptr);

		clk_dir = debugfs_create_dir(temp, base_dir);
		if (!clk_dir)
			return -ENOMEM;

		if (!debugfs_create_file("rate", S_IRUGO | S_IWUSR, clk_dir,
					clock, &clock_rate_fops))
			return -ENOMEM;

		if (!debugfs_create_file("enable", S_IRUGO | S_IWUSR, clk_dir,
					clock, &clock_enable_fops))
			return -ENOMEM;

		if (!debugfs_create_file("is_local", S_IRUGO, clk_dir, clock,
					&clock_local_fops))
			return -ENOMEM;

		if (!debugfs_create_file("measure", S_IRUGO, clk_dir,
					clock, &clock_measure_fops))
			return -ENOMEM;
	}
	return 0;
}
device_initcall(clock_debug_init);
