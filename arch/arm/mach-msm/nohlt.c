


#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <asm/system.h>

static int set_nohalt(void *data, u64 val)
{
	if (val)
		disable_hlt();
	else
		enable_hlt();
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(nohalt_ops, NULL, set_nohalt, "%llu\n");

static int __init init_hlt_debug(void)
{
	debugfs_create_file("nohlt", 0200, NULL, NULL, &nohalt_ops);

	return 0;
}

late_initcall(init_hlt_debug);
