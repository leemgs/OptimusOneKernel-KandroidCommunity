

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/kdev_t.h>
#include <linux/random.h>

#include "uwb-internal.h"






unsigned long beacon_timeout_ms = 500;

static
ssize_t beacon_timeout_ms_show(struct class *class, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%lu\n", beacon_timeout_ms);
}

static
ssize_t beacon_timeout_ms_store(struct class *class,
				const char *buf, size_t size)
{
	unsigned long bt;
	ssize_t result;
	result = sscanf(buf, "%lu", &bt);
	if (result != 1)
		return -EINVAL;
	beacon_timeout_ms = bt;
	return size;
}

static struct class_attribute uwb_class_attrs[] = {
	__ATTR(beacon_timeout_ms, S_IWUSR | S_IRUGO,
	       beacon_timeout_ms_show, beacon_timeout_ms_store),
	__ATTR_NULL,
};


struct class uwb_rc_class = {
	.name        = "uwb_rc",
	.class_attrs = uwb_class_attrs,
};


static int __init uwb_subsys_init(void)
{
	int result = 0;

	result = uwb_est_create();
	if (result < 0) {
		printk(KERN_ERR "uwb: Can't initialize EST subsystem\n");
		goto error_est_init;
	}

	result = class_register(&uwb_rc_class);
	if (result < 0)
		goto error_uwb_rc_class_register;
	uwb_dbg_init();
	return 0;

error_uwb_rc_class_register:
	uwb_est_destroy();
error_est_init:
	return result;
}
module_init(uwb_subsys_init);

static void __exit uwb_subsys_exit(void)
{
	uwb_dbg_exit();
	class_unregister(&uwb_rc_class);
	uwb_est_destroy();
	return;
}
module_exit(uwb_subsys_exit);

MODULE_AUTHOR("Inaky Perez-Gonzalez <inaky.perez-gonzalez@intel.com>");
MODULE_DESCRIPTION("Ultra Wide Band core");
MODULE_LICENSE("GPL");
