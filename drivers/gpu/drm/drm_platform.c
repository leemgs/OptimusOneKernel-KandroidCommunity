

#include "drmP.h"



int drm_get_platform_dev(struct platform_device *platdev,
			 struct drm_driver *driver)
{
	struct drm_device *dev;
	int ret;

	DRM_DEBUG("\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->platformdev = platdev;

	ret = drm_fill_in_dev(dev, NULL, driver);

	if (ret) {
		printk(KERN_ERR "DRM: Fill_in_dev failed.\n");
		goto err_g1;
	}

	if (drm_core_check_feature(dev, DRIVER_MODESET)) {
		dev_set_drvdata(&platdev->dev, dev);
		ret = drm_get_minor(dev, &dev->control, DRM_MINOR_CONTROL);
		if (ret)
			goto err_g1;
	}

	ret = drm_get_minor(dev, &dev->primary, DRM_MINOR_LEGACY);
	if (ret)
		goto err_g2;

	if (dev->driver->load) {
		ret = dev->driver->load(dev, 0);
		if (ret)
			goto err_g3;
	}

	
	if (drm_core_check_feature(dev, DRIVER_MODESET)) {
		ret = drm_mode_group_init_legacy_group(dev,
				&dev->primary->mode_group);
		if (ret)
			goto err_g3;
	}

	list_add_tail(&dev->driver_item, &driver->device_list);

	DRM_INFO("Initialized %s %d.%d.%d %s on minor %d\n",
		 driver->name, driver->major, driver->minor, driver->patchlevel,
		 driver->date, dev->primary->index);

	return 0;

err_g3:
	drm_put_minor(&dev->primary);
err_g2:
	if (drm_core_check_feature(dev, DRIVER_MODESET))
		drm_put_minor(&dev->control);
err_g1:
	kfree(dev);
	return ret;
}
EXPORT_SYMBOL(drm_get_platform_dev);



int drm_platform_init(struct drm_driver *driver)
{
	return drm_get_platform_dev(driver->platform_device, driver);
}
