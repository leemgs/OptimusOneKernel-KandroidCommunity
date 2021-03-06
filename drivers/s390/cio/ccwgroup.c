
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/ctype.h>
#include <linux/dcache.h>

#include <asm/ccwdev.h>
#include <asm/ccwgroup.h>

#define CCW_BUS_ID_SIZE		20




static int
ccwgroup_bus_match (struct device * dev, struct device_driver * drv)
{
	struct ccwgroup_device *gdev;
	struct ccwgroup_driver *gdrv;

	gdev = to_ccwgroupdev(dev);
	gdrv = to_ccwgroupdrv(drv);

	if (gdev->creator_id == gdrv->driver_id)
		return 1;

	return 0;
}
static int
ccwgroup_uevent (struct device *dev, struct kobj_uevent_env *env)
{
	
	return 0;
}

static struct bus_type ccwgroup_bus_type;

static void
__ccwgroup_remove_symlinks(struct ccwgroup_device *gdev)
{
	int i;
	char str[8];

	for (i = 0; i < gdev->count; i++) {
		sprintf(str, "cdev%d", i);
		sysfs_remove_link(&gdev->dev.kobj, str);
		sysfs_remove_link(&gdev->cdev[i]->dev.kobj, "group_device");
	}
	
}


static void ccwgroup_ungroup_callback(struct device *dev)
{
	struct ccwgroup_device *gdev = to_ccwgroupdev(dev);

	mutex_lock(&gdev->reg_mutex);
	if (device_is_registered(&gdev->dev)) {
		__ccwgroup_remove_symlinks(gdev);
		device_unregister(dev);
	}
	mutex_unlock(&gdev->reg_mutex);
}

static ssize_t
ccwgroup_ungroup_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct ccwgroup_device *gdev;
	int rc;

	gdev = to_ccwgroupdev(dev);

	
	if (atomic_cmpxchg(&gdev->onoff, 0, 1) != 0)
		return -EAGAIN;
	if (gdev->state != CCWGROUP_OFFLINE) {
		rc = -EINVAL;
		goto out;
	}
	
	rc = device_schedule_callback(dev, ccwgroup_ungroup_callback);
out:
	if (rc) {
		if (rc != -EAGAIN)
			
			atomic_set(&gdev->onoff, 0);
		return rc;
	}
	return count;
}

static DEVICE_ATTR(ungroup, 0200, NULL, ccwgroup_ungroup_store);

static void
ccwgroup_release (struct device *dev)
{
	struct ccwgroup_device *gdev;
	int i;

	gdev = to_ccwgroupdev(dev);

	for (i = 0; i < gdev->count; i++) {
		if (gdev->cdev[i]) {
			if (dev_get_drvdata(&gdev->cdev[i]->dev) == gdev)
				dev_set_drvdata(&gdev->cdev[i]->dev, NULL);
			put_device(&gdev->cdev[i]->dev);
		}
	}
	kfree(gdev);
}

static int
__ccwgroup_create_symlinks(struct ccwgroup_device *gdev)
{
	char str[8];
	int i, rc;

	for (i = 0; i < gdev->count; i++) {
		rc = sysfs_create_link(&gdev->cdev[i]->dev.kobj, &gdev->dev.kobj,
				       "group_device");
		if (rc) {
			for (--i; i >= 0; i--)
				sysfs_remove_link(&gdev->cdev[i]->dev.kobj,
						  "group_device");
			return rc;
		}
	}
	for (i = 0; i < gdev->count; i++) {
		sprintf(str, "cdev%d", i);
		rc = sysfs_create_link(&gdev->dev.kobj, &gdev->cdev[i]->dev.kobj,
				       str);
		if (rc) {
			for (--i; i >= 0; i--) {
				sprintf(str, "cdev%d", i);
				sysfs_remove_link(&gdev->dev.kobj, str);
			}
			for (i = 0; i < gdev->count; i++)
				sysfs_remove_link(&gdev->cdev[i]->dev.kobj,
						  "group_device");
			return rc;
		}
	}
	return 0;
}

static int __get_next_bus_id(const char **buf, char *bus_id)
{
	int rc, len;
	char *start, *end;

	start = (char *)*buf;
	end = strchr(start, ',');
	if (!end) {
		
		end = strchr(start, '\n');
		if (end)
			*end = '\0';
		len = strlen(start) + 1;
	} else {
		len = end - start + 1;
		end++;
	}
	if (len < CCW_BUS_ID_SIZE) {
		strlcpy(bus_id, start, len);
		rc = 0;
	} else
		rc = -EINVAL;
	*buf = end;
	return rc;
}

static int __is_valid_bus_id(char bus_id[CCW_BUS_ID_SIZE])
{
	int cssid, ssid, devno;

	
	if (sscanf(bus_id, "%x.%1x.%04x", &cssid, &ssid, &devno) != 3)
		return 0;
	return 1;
}


int ccwgroup_create_from_string(struct device *root, unsigned int creator_id,
				struct ccw_driver *cdrv, int num_devices,
				const char *buf)
{
	struct ccwgroup_device *gdev;
	int rc, i;
	char tmp_bus_id[CCW_BUS_ID_SIZE];
	const char *curr_buf;

	gdev = kzalloc(sizeof(*gdev) + num_devices * sizeof(gdev->cdev[0]),
		       GFP_KERNEL);
	if (!gdev)
		return -ENOMEM;

	atomic_set(&gdev->onoff, 0);
	mutex_init(&gdev->reg_mutex);
	mutex_lock(&gdev->reg_mutex);
	gdev->creator_id = creator_id;
	gdev->count = num_devices;
	gdev->dev.bus = &ccwgroup_bus_type;
	gdev->dev.parent = root;
	gdev->dev.release = ccwgroup_release;
	device_initialize(&gdev->dev);

	curr_buf = buf;
	for (i = 0; i < num_devices && curr_buf; i++) {
		rc = __get_next_bus_id(&curr_buf, tmp_bus_id);
		if (rc != 0)
			goto error;
		if (!__is_valid_bus_id(tmp_bus_id)) {
			rc = -EINVAL;
			goto error;
		}
		gdev->cdev[i] = get_ccwdev_by_busid(cdrv, tmp_bus_id);
		
		if (!gdev->cdev[i]
		    || gdev->cdev[i]->id.driver_info !=
		    gdev->cdev[0]->id.driver_info) {
			rc = -EINVAL;
			goto error;
		}
		
		if (dev_get_drvdata(&gdev->cdev[i]->dev)) {
			rc = -EINVAL;
			goto error;
		}
		dev_set_drvdata(&gdev->cdev[i]->dev, gdev);
	}
	
	if (i < num_devices && !curr_buf) {
		rc = -EINVAL;
		goto error;
	}
	
	if (i == num_devices && strlen(curr_buf) > 0) {
		rc = -EINVAL;
		goto error;
	}

	dev_set_name(&gdev->dev, "%s", dev_name(&gdev->cdev[0]->dev));

	rc = device_add(&gdev->dev);
	if (rc)
		goto error;
	get_device(&gdev->dev);
	rc = device_create_file(&gdev->dev, &dev_attr_ungroup);

	if (rc) {
		device_unregister(&gdev->dev);
		goto error;
	}

	rc = __ccwgroup_create_symlinks(gdev);
	if (!rc) {
		mutex_unlock(&gdev->reg_mutex);
		put_device(&gdev->dev);
		return 0;
	}
	device_remove_file(&gdev->dev, &dev_attr_ungroup);
	device_unregister(&gdev->dev);
error:
	for (i = 0; i < num_devices; i++)
		if (gdev->cdev[i]) {
			if (dev_get_drvdata(&gdev->cdev[i]->dev) == gdev)
				dev_set_drvdata(&gdev->cdev[i]->dev, NULL);
			put_device(&gdev->cdev[i]->dev);
			gdev->cdev[i] = NULL;
		}
	mutex_unlock(&gdev->reg_mutex);
	put_device(&gdev->dev);
	return rc;
}
EXPORT_SYMBOL(ccwgroup_create_from_string);

static int ccwgroup_notifier(struct notifier_block *nb, unsigned long action,
			     void *data);

static struct notifier_block ccwgroup_nb = {
	.notifier_call = ccwgroup_notifier
};

static int __init init_ccwgroup(void)
{
	int ret;

	ret = bus_register(&ccwgroup_bus_type);
	if (ret)
		return ret;

	ret = bus_register_notifier(&ccwgroup_bus_type, &ccwgroup_nb);
	if (ret)
		bus_unregister(&ccwgroup_bus_type);

	return ret;
}

static void __exit cleanup_ccwgroup(void)
{
	bus_unregister_notifier(&ccwgroup_bus_type, &ccwgroup_nb);
	bus_unregister(&ccwgroup_bus_type);
}

module_init(init_ccwgroup);
module_exit(cleanup_ccwgroup);



static int
ccwgroup_set_online(struct ccwgroup_device *gdev)
{
	struct ccwgroup_driver *gdrv;
	int ret;

	if (atomic_cmpxchg(&gdev->onoff, 0, 1) != 0)
		return -EAGAIN;
	if (gdev->state == CCWGROUP_ONLINE) {
		ret = 0;
		goto out;
	}
	if (!gdev->dev.driver) {
		ret = -EINVAL;
		goto out;
	}
	gdrv = to_ccwgroupdrv (gdev->dev.driver);
	if ((ret = gdrv->set_online ? gdrv->set_online(gdev) : 0))
		goto out;

	gdev->state = CCWGROUP_ONLINE;
 out:
	atomic_set(&gdev->onoff, 0);
	return ret;
}

static int
ccwgroup_set_offline(struct ccwgroup_device *gdev)
{
	struct ccwgroup_driver *gdrv;
	int ret;

	if (atomic_cmpxchg(&gdev->onoff, 0, 1) != 0)
		return -EAGAIN;
	if (gdev->state == CCWGROUP_OFFLINE) {
		ret = 0;
		goto out;
	}
	if (!gdev->dev.driver) {
		ret = -EINVAL;
		goto out;
	}
	gdrv = to_ccwgroupdrv (gdev->dev.driver);
	if ((ret = gdrv->set_offline ? gdrv->set_offline(gdev) : 0))
		goto out;

	gdev->state = CCWGROUP_OFFLINE;
 out:
	atomic_set(&gdev->onoff, 0);
	return ret;
}

static ssize_t
ccwgroup_online_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct ccwgroup_device *gdev;
	struct ccwgroup_driver *gdrv;
	unsigned long value;
	int ret;

	if (!dev->driver)
		return -ENODEV;

	gdev = to_ccwgroupdev(dev);
	gdrv = to_ccwgroupdrv(dev->driver);

	if (!try_module_get(gdrv->owner))
		return -EINVAL;

	ret = strict_strtoul(buf, 0, &value);
	if (ret)
		goto out;

	if (value == 1)
		ret = ccwgroup_set_online(gdev);
	else if (value == 0)
		ret = ccwgroup_set_offline(gdev);
	else
		ret = -EINVAL;
out:
	module_put(gdrv->owner);
	return (ret == 0) ? count : ret;
}

static ssize_t
ccwgroup_online_show (struct device *dev, struct device_attribute *attr, char *buf)
{
	int online;

	online = (to_ccwgroupdev(dev)->state == CCWGROUP_ONLINE);

	return sprintf(buf, online ? "1\n" : "0\n");
}

static DEVICE_ATTR(online, 0644, ccwgroup_online_show, ccwgroup_online_store);

static int
ccwgroup_probe (struct device *dev)
{
	struct ccwgroup_device *gdev;
	struct ccwgroup_driver *gdrv;

	int ret;

	gdev = to_ccwgroupdev(dev);
	gdrv = to_ccwgroupdrv(dev->driver);

	if ((ret = device_create_file(dev, &dev_attr_online)))
		return ret;

	ret = gdrv->probe ? gdrv->probe(gdev) : -ENODEV;
	if (ret)
		device_remove_file(dev, &dev_attr_online);

	return ret;
}

static int
ccwgroup_remove (struct device *dev)
{
	struct ccwgroup_device *gdev;
	struct ccwgroup_driver *gdrv;

	device_remove_file(dev, &dev_attr_online);
	device_remove_file(dev, &dev_attr_ungroup);

	if (!dev->driver)
		return 0;

	gdev = to_ccwgroupdev(dev);
	gdrv = to_ccwgroupdrv(dev->driver);

	if (gdrv->remove)
		gdrv->remove(gdev);

	return 0;
}

static void ccwgroup_shutdown(struct device *dev)
{
	struct ccwgroup_device *gdev;
	struct ccwgroup_driver *gdrv;

	if (!dev->driver)
		return;

	gdev = to_ccwgroupdev(dev);
	gdrv = to_ccwgroupdrv(dev->driver);

	if (gdrv->shutdown)
		gdrv->shutdown(gdev);
}

static int ccwgroup_pm_prepare(struct device *dev)
{
	struct ccwgroup_device *gdev = to_ccwgroupdev(dev);
	struct ccwgroup_driver *gdrv = to_ccwgroupdrv(gdev->dev.driver);

	
	if (atomic_read(&gdev->onoff))
		return -EAGAIN;

	if (!gdev->dev.driver || gdev->state != CCWGROUP_ONLINE)
		return 0;

	return gdrv->prepare ? gdrv->prepare(gdev) : 0;
}

static void ccwgroup_pm_complete(struct device *dev)
{
	struct ccwgroup_device *gdev = to_ccwgroupdev(dev);
	struct ccwgroup_driver *gdrv = to_ccwgroupdrv(dev->driver);

	if (!gdev->dev.driver || gdev->state != CCWGROUP_ONLINE)
		return;

	if (gdrv->complete)
		gdrv->complete(gdev);
}

static int ccwgroup_pm_freeze(struct device *dev)
{
	struct ccwgroup_device *gdev = to_ccwgroupdev(dev);
	struct ccwgroup_driver *gdrv = to_ccwgroupdrv(gdev->dev.driver);

	if (!gdev->dev.driver || gdev->state != CCWGROUP_ONLINE)
		return 0;

	return gdrv->freeze ? gdrv->freeze(gdev) : 0;
}

static int ccwgroup_pm_thaw(struct device *dev)
{
	struct ccwgroup_device *gdev = to_ccwgroupdev(dev);
	struct ccwgroup_driver *gdrv = to_ccwgroupdrv(gdev->dev.driver);

	if (!gdev->dev.driver || gdev->state != CCWGROUP_ONLINE)
		return 0;

	return gdrv->thaw ? gdrv->thaw(gdev) : 0;
}

static int ccwgroup_pm_restore(struct device *dev)
{
	struct ccwgroup_device *gdev = to_ccwgroupdev(dev);
	struct ccwgroup_driver *gdrv = to_ccwgroupdrv(gdev->dev.driver);

	if (!gdev->dev.driver || gdev->state != CCWGROUP_ONLINE)
		return 0;

	return gdrv->restore ? gdrv->restore(gdev) : 0;
}

static struct dev_pm_ops ccwgroup_pm_ops = {
	.prepare = ccwgroup_pm_prepare,
	.complete = ccwgroup_pm_complete,
	.freeze = ccwgroup_pm_freeze,
	.thaw = ccwgroup_pm_thaw,
	.restore = ccwgroup_pm_restore,
};

static struct bus_type ccwgroup_bus_type = {
	.name   = "ccwgroup",
	.match  = ccwgroup_bus_match,
	.uevent = ccwgroup_uevent,
	.probe  = ccwgroup_probe,
	.remove = ccwgroup_remove,
	.shutdown = ccwgroup_shutdown,
	.pm = &ccwgroup_pm_ops,
};


static int ccwgroup_notifier(struct notifier_block *nb, unsigned long action,
			     void *data)
{
	struct device *dev = data;

	if (action == BUS_NOTIFY_UNBIND_DRIVER)
		device_schedule_callback(dev, ccwgroup_ungroup_callback);

	return NOTIFY_OK;
}



int ccwgroup_driver_register(struct ccwgroup_driver *cdriver)
{
	
	cdriver->driver.bus = &ccwgroup_bus_type;
	cdriver->driver.name = cdriver->name;
	cdriver->driver.owner = cdriver->owner;

	return driver_register(&cdriver->driver);
}

static int
__ccwgroup_match_all(struct device *dev, void *data)
{
	return 1;
}


void ccwgroup_driver_unregister(struct ccwgroup_driver *cdriver)
{
	struct device *dev;

	
	get_driver(&cdriver->driver);
	while ((dev = driver_find_device(&cdriver->driver, NULL, NULL,
					 __ccwgroup_match_all))) {
		struct ccwgroup_device *gdev = to_ccwgroupdev(dev);

		mutex_lock(&gdev->reg_mutex);
		__ccwgroup_remove_symlinks(gdev);
		device_unregister(dev);
		mutex_unlock(&gdev->reg_mutex);
		put_device(dev);
	}
	put_driver(&cdriver->driver);
	driver_unregister(&cdriver->driver);
}


int ccwgroup_probe_ccwdev(struct ccw_device *cdev)
{
	return 0;
}

static struct ccwgroup_device *
__ccwgroup_get_gdev_by_cdev(struct ccw_device *cdev)
{
	struct ccwgroup_device *gdev;

	gdev = dev_get_drvdata(&cdev->dev);
	if (gdev) {
		if (get_device(&gdev->dev)) {
			mutex_lock(&gdev->reg_mutex);
			if (device_is_registered(&gdev->dev))
				return gdev;
			mutex_unlock(&gdev->reg_mutex);
			put_device(&gdev->dev);
		}
		return NULL;
	}
	return NULL;
}


void ccwgroup_remove_ccwdev(struct ccw_device *cdev)
{
	struct ccwgroup_device *gdev;

	
	ccw_device_set_offline(cdev);
	
	gdev = __ccwgroup_get_gdev_by_cdev(cdev);
	if (gdev) {
		__ccwgroup_remove_symlinks(gdev);
		device_unregister(&gdev->dev);
		mutex_unlock(&gdev->reg_mutex);
		put_device(&gdev->dev);
	}
}

MODULE_LICENSE("GPL");
EXPORT_SYMBOL(ccwgroup_driver_register);
EXPORT_SYMBOL(ccwgroup_driver_unregister);
EXPORT_SYMBOL(ccwgroup_probe_ccwdev);
EXPORT_SYMBOL(ccwgroup_remove_ccwdev);
