




#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include "drmP.h"






drm_dma_handle_t *drm_pci_alloc(struct drm_device * dev, size_t size, size_t align)
{
	drm_dma_handle_t *dmah;
#if 1
	unsigned long addr;
	size_t sz;
#endif

	
	if (align > size)
		return NULL;

	dmah = kmalloc(sizeof(drm_dma_handle_t), GFP_KERNEL);
	if (!dmah)
		return NULL;

	dmah->size = size;
	dmah->vaddr = dma_alloc_coherent(&dev->pdev->dev, size, &dmah->busaddr, GFP_KERNEL | __GFP_COMP);

	if (dmah->vaddr == NULL) {
		kfree(dmah);
		return NULL;
	}

	memset(dmah->vaddr, 0, size);

	
	
	for (addr = (unsigned long)dmah->vaddr, sz = size;
	     sz > 0; addr += PAGE_SIZE, sz -= PAGE_SIZE) {
		SetPageReserved(virt_to_page(addr));
	}

	return dmah;
}

EXPORT_SYMBOL(drm_pci_alloc);


void __drm_pci_free(struct drm_device * dev, drm_dma_handle_t * dmah)
{
#if 1
	unsigned long addr;
	size_t sz;
#endif

	if (dmah->vaddr) {
		
		
		for (addr = (unsigned long)dmah->vaddr, sz = dmah->size;
		     sz > 0; addr += PAGE_SIZE, sz -= PAGE_SIZE) {
			ClearPageReserved(virt_to_page(addr));
		}
		dma_free_coherent(&dev->pdev->dev, dmah->size, dmah->vaddr,
				  dmah->busaddr);
	}
}


void drm_pci_free(struct drm_device * dev, drm_dma_handle_t * dmah)
{
	__drm_pci_free(dev, dmah);
	kfree(dmah);
}

EXPORT_SYMBOL(drm_pci_free);

#ifdef CONFIG_PCI

int drm_get_pci_dev(struct pci_dev *pdev, const struct pci_device_id *ent,
		    struct drm_driver *driver)
{
	struct drm_device *dev;
	int ret;

	DRM_DEBUG("\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	ret = pci_enable_device(pdev);
	if (ret)
		goto err_g1;

	pci_set_master(pdev);

	dev->pdev = pdev;
	dev->pci_device = pdev->device;
	dev->pci_vendor = pdev->vendor;

#ifdef __alpha__
	dev->hose = pdev->sysdata;
#endif

	if ((ret = drm_fill_in_dev(dev, ent, driver))) {
		printk(KERN_ERR "DRM: Fill_in_dev failed.\n");
		goto err_g2;
	}

	if (drm_core_check_feature(dev, DRIVER_MODESET)) {
		pci_set_drvdata(pdev, dev);
		ret = drm_get_minor(dev, &dev->control, DRM_MINOR_CONTROL);
		if (ret)
			goto err_g2;
	}

	if ((ret = drm_get_minor(dev, &dev->primary, DRM_MINOR_LEGACY)))
		goto err_g3;

	if (dev->driver->load) {
		ret = dev->driver->load(dev, ent->driver_data);
		if (ret)
			goto err_g4;
	}

	
	if (drm_core_check_feature(dev, DRIVER_MODESET)) {
		ret = drm_mode_group_init_legacy_group(dev,
						&dev->primary->mode_group);
		if (ret)
			goto err_g4;
	}

	list_add_tail(&dev->driver_item, &driver->device_list);

	DRM_INFO("Initialized %s %d.%d.%d %s for %s on minor %d\n",
		 driver->name, driver->major, driver->minor, driver->patchlevel,
		 driver->date, pci_name(pdev), dev->primary->index);

	return 0;

err_g4:
	drm_put_minor(&dev->primary);
err_g3:
	if (drm_core_check_feature(dev, DRIVER_MODESET))
		drm_put_minor(&dev->control);
err_g2:
	pci_disable_device(pdev);
err_g1:
	kfree(dev);
	return ret;
}
EXPORT_SYMBOL(drm_get_pci_dev);


int drm_pci_init(struct drm_driver *driver)
{
	struct pci_dev *pdev = NULL;
	const struct pci_device_id *pid;
	int i;

	if (driver->driver_features & DRIVER_MODESET)
		return pci_register_driver(&driver->pci_driver);

	
	for (i = 0; driver->pci_driver.id_table[i].vendor != 0; i++) {
		pid = &driver->pci_driver.id_table[i];

		
		pdev = NULL;
		while ((pdev =
			pci_get_subsys(pid->vendor, pid->device, pid->subvendor,
				       pid->subdevice, pdev)) != NULL) {
			if ((pdev->class & pid->class_mask) != pid->class)
				continue;

			
			pci_dev_get(pdev);
			drm_get_pci_dev(pdev, pid, driver);
		}
	}
	return 0;
}

#else

int drm_pci_init(struct drm_driver *driver)
{
	return -1;
}

#endif

