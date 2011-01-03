

#include <linux/pci.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mempolicy.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/cpu.h>
#include "pci.h"

struct pci_dynid {
	struct list_head node;
	struct pci_device_id id;
};


int pci_add_dynid(struct pci_driver *drv,
		  unsigned int vendor, unsigned int device,
		  unsigned int subvendor, unsigned int subdevice,
		  unsigned int class, unsigned int class_mask,
		  unsigned long driver_data)
{
	struct pci_dynid *dynid;
	int retval;

	dynid = kzalloc(sizeof(*dynid), GFP_KERNEL);
	if (!dynid)
		return -ENOMEM;

	dynid->id.vendor = vendor;
	dynid->id.device = device;
	dynid->id.subvendor = subvendor;
	dynid->id.subdevice = subdevice;
	dynid->id.class = class;
	dynid->id.class_mask = class_mask;
	dynid->id.driver_data = driver_data;

	spin_lock(&drv->dynids.lock);
	list_add_tail(&dynid->node, &drv->dynids.list);
	spin_unlock(&drv->dynids.lock);

	get_driver(&drv->driver);
	retval = driver_attach(&drv->driver);
	put_driver(&drv->driver);

	return retval;
}

static void pci_free_dynids(struct pci_driver *drv)
{
	struct pci_dynid *dynid, *n;

	spin_lock(&drv->dynids.lock);
	list_for_each_entry_safe(dynid, n, &drv->dynids.list, node) {
		list_del(&dynid->node);
		kfree(dynid);
	}
	spin_unlock(&drv->dynids.lock);
}


#ifdef CONFIG_HOTPLUG

static ssize_t
store_new_id(struct device_driver *driver, const char *buf, size_t count)
{
	struct pci_driver *pdrv = to_pci_driver(driver);
	const struct pci_device_id *ids = pdrv->id_table;
	__u32 vendor, device, subvendor=PCI_ANY_ID,
		subdevice=PCI_ANY_ID, class=0, class_mask=0;
	unsigned long driver_data=0;
	int fields=0;
	int retval;

	fields = sscanf(buf, "%x %x %x %x %x %x %lx",
			&vendor, &device, &subvendor, &subdevice,
			&class, &class_mask, &driver_data);
	if (fields < 2)
		return -EINVAL;

	
	if (ids) {
		retval = -EINVAL;
		while (ids->vendor || ids->subvendor || ids->class_mask) {
			if (driver_data == ids->driver_data) {
				retval = 0;
				break;
			}
			ids++;
		}
		if (retval)	
			return retval;
	}

	retval = pci_add_dynid(pdrv, vendor, device, subvendor, subdevice,
			       class, class_mask, driver_data);
	if (retval)
		return retval;
	return count;
}
static DRIVER_ATTR(new_id, S_IWUSR, NULL, store_new_id);


static ssize_t
store_remove_id(struct device_driver *driver, const char *buf, size_t count)
{
	struct pci_dynid *dynid, *n;
	struct pci_driver *pdrv = to_pci_driver(driver);
	__u32 vendor, device, subvendor = PCI_ANY_ID,
		subdevice = PCI_ANY_ID, class = 0, class_mask = 0;
	int fields = 0;
	int retval = -ENODEV;

	fields = sscanf(buf, "%x %x %x %x %x %x",
			&vendor, &device, &subvendor, &subdevice,
			&class, &class_mask);
	if (fields < 2)
		return -EINVAL;

	spin_lock(&pdrv->dynids.lock);
	list_for_each_entry_safe(dynid, n, &pdrv->dynids.list, node) {
		struct pci_device_id *id = &dynid->id;
		if ((id->vendor == vendor) &&
		    (id->device == device) &&
		    (subvendor == PCI_ANY_ID || id->subvendor == subvendor) &&
		    (subdevice == PCI_ANY_ID || id->subdevice == subdevice) &&
		    !((id->class ^ class) & class_mask)) {
			list_del(&dynid->node);
			kfree(dynid);
			retval = 0;
			break;
		}
	}
	spin_unlock(&pdrv->dynids.lock);

	if (retval)
		return retval;
	return count;
}
static DRIVER_ATTR(remove_id, S_IWUSR, NULL, store_remove_id);

static int
pci_create_newid_file(struct pci_driver *drv)
{
	int error = 0;
	if (drv->probe != NULL)
		error = driver_create_file(&drv->driver, &driver_attr_new_id);
	return error;
}

static void pci_remove_newid_file(struct pci_driver *drv)
{
	driver_remove_file(&drv->driver, &driver_attr_new_id);
}

static int
pci_create_removeid_file(struct pci_driver *drv)
{
	int error = 0;
	if (drv->probe != NULL)
		error = driver_create_file(&drv->driver,&driver_attr_remove_id);
	return error;
}

static void pci_remove_removeid_file(struct pci_driver *drv)
{
	driver_remove_file(&drv->driver, &driver_attr_remove_id);
}
#else 
static inline int pci_create_newid_file(struct pci_driver *drv)
{
	return 0;
}
static inline void pci_remove_newid_file(struct pci_driver *drv) {}
static inline int pci_create_removeid_file(struct pci_driver *drv)
{
	return 0;
}
static inline void pci_remove_removeid_file(struct pci_driver *drv) {}
#endif


const struct pci_device_id *pci_match_id(const struct pci_device_id *ids,
					 struct pci_dev *dev)
{
	if (ids) {
		while (ids->vendor || ids->subvendor || ids->class_mask) {
			if (pci_match_one_device(ids, dev))
				return ids;
			ids++;
		}
	}
	return NULL;
}


static const struct pci_device_id *pci_match_device(struct pci_driver *drv,
						    struct pci_dev *dev)
{
	struct pci_dynid *dynid;

	
	spin_lock(&drv->dynids.lock);
	list_for_each_entry(dynid, &drv->dynids.list, node) {
		if (pci_match_one_device(&dynid->id, dev)) {
			spin_unlock(&drv->dynids.lock);
			return &dynid->id;
		}
	}
	spin_unlock(&drv->dynids.lock);

	return pci_match_id(drv->id_table, dev);
}

struct drv_dev_and_id {
	struct pci_driver *drv;
	struct pci_dev *dev;
	const struct pci_device_id *id;
};

static long local_pci_probe(void *_ddi)
{
	struct drv_dev_and_id *ddi = _ddi;

	return ddi->drv->probe(ddi->dev, ddi->id);
}

static int pci_call_probe(struct pci_driver *drv, struct pci_dev *dev,
			  const struct pci_device_id *id)
{
	int error, node;
	struct drv_dev_and_id ddi = { drv, dev, id };

	
	node = dev_to_node(&dev->dev);
	if (node >= 0) {
		int cpu;

		get_online_cpus();
		cpu = cpumask_any_and(cpumask_of_node(node), cpu_online_mask);
		if (cpu < nr_cpu_ids)
			error = work_on_cpu(cpu, local_pci_probe, &ddi);
		else
			error = local_pci_probe(&ddi);
		put_online_cpus();
	} else
		error = local_pci_probe(&ddi);
	return error;
}


static int
__pci_device_probe(struct pci_driver *drv, struct pci_dev *pci_dev)
{
	const struct pci_device_id *id;
	int error = 0;

	if (!pci_dev->driver && drv->probe) {
		error = -ENODEV;

		id = pci_match_device(drv, pci_dev);
		if (id)
			error = pci_call_probe(drv, pci_dev, id);
		if (error >= 0) {
			pci_dev->driver = drv;
			error = 0;
		}
	}
	return error;
}

static int pci_device_probe(struct device * dev)
{
	int error = 0;
	struct pci_driver *drv;
	struct pci_dev *pci_dev;

	drv = to_pci_driver(dev->driver);
	pci_dev = to_pci_dev(dev);
	pci_dev_get(pci_dev);
	error = __pci_device_probe(drv, pci_dev);
	if (error)
		pci_dev_put(pci_dev);

	return error;
}

static int pci_device_remove(struct device * dev)
{
	struct pci_dev * pci_dev = to_pci_dev(dev);
	struct pci_driver * drv = pci_dev->driver;

	if (drv) {
		if (drv->remove)
			drv->remove(pci_dev);
		pci_dev->driver = NULL;
	}

	
	if (pci_dev->current_state == PCI_D0)
		pci_dev->current_state = PCI_UNKNOWN;

	

	pci_dev_put(pci_dev);
	return 0;
}

static void pci_device_shutdown(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	struct pci_driver *drv = pci_dev->driver;

	if (drv && drv->shutdown)
		drv->shutdown(pci_dev);
	pci_msi_shutdown(pci_dev);
	pci_msix_shutdown(pci_dev);
}

#ifdef CONFIG_PM_SLEEP


static void pci_pm_set_unknown_state(struct pci_dev *pci_dev)
{
	
	if (pci_dev->current_state == PCI_D0)
		pci_dev->current_state = PCI_UNKNOWN;
}


static int pci_pm_reenable_device(struct pci_dev *pci_dev)
{
	int retval;

	
	retval = pci_reenable_device(pci_dev);
	
	if (pci_dev->is_busmaster)
		pci_set_master(pci_dev);

	return retval;
}

static int pci_legacy_suspend(struct device *dev, pm_message_t state)
{
	struct pci_dev * pci_dev = to_pci_dev(dev);
	struct pci_driver * drv = pci_dev->driver;

	if (drv && drv->suspend) {
		pci_power_t prev = pci_dev->current_state;
		int error;

		error = drv->suspend(pci_dev, state);
		suspend_report_result(drv->suspend, error);
		if (error)
			return error;

		if (!pci_dev->state_saved && pci_dev->current_state != PCI_D0
		    && pci_dev->current_state != PCI_UNKNOWN) {
			WARN_ONCE(pci_dev->current_state != prev,
				"PCI PM: Device state not saved by %pF\n",
				drv->suspend);
		}
	}

	pci_fixup_device(pci_fixup_suspend, pci_dev);

	return 0;
}

static int pci_legacy_suspend_late(struct device *dev, pm_message_t state)
{
	struct pci_dev * pci_dev = to_pci_dev(dev);
	struct pci_driver * drv = pci_dev->driver;

	if (drv && drv->suspend_late) {
		pci_power_t prev = pci_dev->current_state;
		int error;

		error = drv->suspend_late(pci_dev, state);
		suspend_report_result(drv->suspend_late, error);
		if (error)
			return error;

		if (!pci_dev->state_saved && pci_dev->current_state != PCI_D0
		    && pci_dev->current_state != PCI_UNKNOWN) {
			WARN_ONCE(pci_dev->current_state != prev,
				"PCI PM: Device state not saved by %pF\n",
				drv->suspend_late);
			return 0;
		}
	}

	if (!pci_dev->state_saved)
		pci_save_state(pci_dev);

	pci_pm_set_unknown_state(pci_dev);

	return 0;
}

static int pci_legacy_resume_early(struct device *dev)
{
	struct pci_dev * pci_dev = to_pci_dev(dev);
	struct pci_driver * drv = pci_dev->driver;

	return drv && drv->resume_early ?
			drv->resume_early(pci_dev) : 0;
}

static int pci_legacy_resume(struct device *dev)
{
	struct pci_dev * pci_dev = to_pci_dev(dev);
	struct pci_driver * drv = pci_dev->driver;

	pci_fixup_device(pci_fixup_resume, pci_dev);

	return drv && drv->resume ?
			drv->resume(pci_dev) : pci_pm_reenable_device(pci_dev);
}




static int pci_restore_standard_config(struct pci_dev *pci_dev)
{
	pci_update_current_state(pci_dev, PCI_UNKNOWN);

	if (pci_dev->current_state != PCI_D0) {
		int error = pci_set_power_state(pci_dev, PCI_D0);
		if (error)
			return error;
	}

	return pci_restore_state(pci_dev);
}

static void pci_pm_default_resume_noirq(struct pci_dev *pci_dev)
{
	pci_restore_standard_config(pci_dev);
	pci_fixup_device(pci_fixup_resume_early, pci_dev);
}

static void pci_pm_default_resume(struct pci_dev *pci_dev)
{
	pci_fixup_device(pci_fixup_resume, pci_dev);

	if (!pci_is_bridge(pci_dev))
		pci_enable_wake(pci_dev, PCI_D0, false);
}

static void pci_pm_default_suspend(struct pci_dev *pci_dev)
{
	
	if (!pci_is_bridge(pci_dev))
		pci_disable_enabled_device(pci_dev);
}

static bool pci_has_legacy_pm_support(struct pci_dev *pci_dev)
{
	struct pci_driver *drv = pci_dev->driver;
	bool ret = drv && (drv->suspend || drv->suspend_late || drv->resume
		|| drv->resume_early);

	
	WARN_ON(ret && drv->driver.pm);

	return ret;
}



static int pci_pm_prepare(struct device *dev)
{
	struct device_driver *drv = dev->driver;
	int error = 0;

	if (drv && drv->pm && drv->pm->prepare)
		error = drv->pm->prepare(dev);

	return error;
}

static void pci_pm_complete(struct device *dev)
{
	struct device_driver *drv = dev->driver;

	if (drv && drv->pm && drv->pm->complete)
		drv->pm->complete(dev);
}

#ifdef CONFIG_SUSPEND

static int pci_pm_suspend(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_suspend(dev, PMSG_SUSPEND);

	if (!pm) {
		pci_pm_default_suspend(pci_dev);
		goto Fixup;
	}

	if (pm->suspend) {
		pci_power_t prev = pci_dev->current_state;
		int error;

		error = pm->suspend(dev);
		suspend_report_result(pm->suspend, error);
		if (error)
			return error;

		if (!pci_dev->state_saved && pci_dev->current_state != PCI_D0
		    && pci_dev->current_state != PCI_UNKNOWN) {
			WARN_ONCE(pci_dev->current_state != prev,
				"PCI PM: State of device not saved by %pF\n",
				pm->suspend);
		}
	}

 Fixup:
	pci_fixup_device(pci_fixup_suspend, pci_dev);

	return 0;
}

static int pci_pm_suspend_noirq(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_suspend_late(dev, PMSG_SUSPEND);

	if (!pm) {
		pci_save_state(pci_dev);
		return 0;
	}

	if (pm->suspend_noirq) {
		pci_power_t prev = pci_dev->current_state;
		int error;

		error = pm->suspend_noirq(dev);
		suspend_report_result(pm->suspend_noirq, error);
		if (error)
			return error;

		if (!pci_dev->state_saved && pci_dev->current_state != PCI_D0
		    && pci_dev->current_state != PCI_UNKNOWN) {
			WARN_ONCE(pci_dev->current_state != prev,
				"PCI PM: State of device not saved by %pF\n",
				pm->suspend_noirq);
			return 0;
		}
	}

	if (!pci_dev->state_saved) {
		pci_save_state(pci_dev);
		if (!pci_is_bridge(pci_dev))
			pci_prepare_to_sleep(pci_dev);
	}

	pci_pm_set_unknown_state(pci_dev);

	return 0;
}

static int pci_pm_resume_noirq(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	struct device_driver *drv = dev->driver;
	int error = 0;

	pci_pm_default_resume_noirq(pci_dev);

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_resume_early(dev);

	if (drv && drv->pm && drv->pm->resume_noirq)
		error = drv->pm->resume_noirq(dev);

	return error;
}

static int pci_pm_resume(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;
	int error = 0;

	
	if (pci_dev->state_saved)
		pci_restore_standard_config(pci_dev);

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_resume(dev);

	pci_pm_default_resume(pci_dev);

	if (pm) {
		if (pm->resume)
			error = pm->resume(dev);
	} else {
		pci_pm_reenable_device(pci_dev);
	}

	return error;
}

#else 

#define pci_pm_suspend		NULL
#define pci_pm_suspend_noirq	NULL
#define pci_pm_resume		NULL
#define pci_pm_resume_noirq	NULL

#endif 

#ifdef CONFIG_HIBERNATION

static int pci_pm_freeze(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_suspend(dev, PMSG_FREEZE);

	if (!pm) {
		pci_pm_default_suspend(pci_dev);
		return 0;
	}

	if (pm->freeze) {
		int error;

		error = pm->freeze(dev);
		suspend_report_result(pm->freeze, error);
		if (error)
			return error;
	}

	return 0;
}

static int pci_pm_freeze_noirq(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	struct device_driver *drv = dev->driver;

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_suspend_late(dev, PMSG_FREEZE);

	if (drv && drv->pm && drv->pm->freeze_noirq) {
		int error;

		error = drv->pm->freeze_noirq(dev);
		suspend_report_result(drv->pm->freeze_noirq, error);
		if (error)
			return error;
	}

	if (!pci_dev->state_saved)
		pci_save_state(pci_dev);

	pci_pm_set_unknown_state(pci_dev);

	return 0;
}

static int pci_pm_thaw_noirq(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	struct device_driver *drv = dev->driver;
	int error = 0;

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_resume_early(dev);

	pci_update_current_state(pci_dev, PCI_D0);

	if (drv && drv->pm && drv->pm->thaw_noirq)
		error = drv->pm->thaw_noirq(dev);

	return error;
}

static int pci_pm_thaw(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;
	int error = 0;

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_resume(dev);

	if (pm) {
		if (pm->thaw)
			error = pm->thaw(dev);
	} else {
		pci_pm_reenable_device(pci_dev);
	}

	pci_dev->state_saved = false;

	return error;
}

static int pci_pm_poweroff(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_suspend(dev, PMSG_HIBERNATE);

	if (!pm) {
		pci_pm_default_suspend(pci_dev);
		goto Fixup;
	}

	if (pm->poweroff) {
		int error;

		error = pm->poweroff(dev);
		suspend_report_result(pm->poweroff, error);
		if (error)
			return error;
	}

 Fixup:
	pci_fixup_device(pci_fixup_suspend, pci_dev);

	return 0;
}

static int pci_pm_poweroff_noirq(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	struct device_driver *drv = dev->driver;

	if (pci_has_legacy_pm_support(to_pci_dev(dev)))
		return pci_legacy_suspend_late(dev, PMSG_HIBERNATE);

	if (!drv || !drv->pm)
		return 0;

	if (drv->pm->poweroff_noirq) {
		int error;

		error = drv->pm->poweroff_noirq(dev);
		suspend_report_result(drv->pm->poweroff_noirq, error);
		if (error)
			return error;
	}

	if (!pci_dev->state_saved && !pci_is_bridge(pci_dev))
		pci_prepare_to_sleep(pci_dev);

	return 0;
}

static int pci_pm_restore_noirq(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	struct device_driver *drv = dev->driver;
	int error = 0;

	pci_pm_default_resume_noirq(pci_dev);

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_resume_early(dev);

	if (drv && drv->pm && drv->pm->restore_noirq)
		error = drv->pm->restore_noirq(dev);

	return error;
}

static int pci_pm_restore(struct device *dev)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	const struct dev_pm_ops *pm = dev->driver ? dev->driver->pm : NULL;
	int error = 0;

	
	if (pci_dev->state_saved)
		pci_restore_standard_config(pci_dev);

	if (pci_has_legacy_pm_support(pci_dev))
		return pci_legacy_resume(dev);

	pci_pm_default_resume(pci_dev);

	if (pm) {
		if (pm->restore)
			error = pm->restore(dev);
	} else {
		pci_pm_reenable_device(pci_dev);
	}

	return error;
}

#else 

#define pci_pm_freeze		NULL
#define pci_pm_freeze_noirq	NULL
#define pci_pm_thaw		NULL
#define pci_pm_thaw_noirq	NULL
#define pci_pm_poweroff		NULL
#define pci_pm_poweroff_noirq	NULL
#define pci_pm_restore		NULL
#define pci_pm_restore_noirq	NULL

#endif 

const struct dev_pm_ops pci_dev_pm_ops = {
	.prepare = pci_pm_prepare,
	.complete = pci_pm_complete,
	.suspend = pci_pm_suspend,
	.resume = pci_pm_resume,
	.freeze = pci_pm_freeze,
	.thaw = pci_pm_thaw,
	.poweroff = pci_pm_poweroff,
	.restore = pci_pm_restore,
	.suspend_noirq = pci_pm_suspend_noirq,
	.resume_noirq = pci_pm_resume_noirq,
	.freeze_noirq = pci_pm_freeze_noirq,
	.thaw_noirq = pci_pm_thaw_noirq,
	.poweroff_noirq = pci_pm_poweroff_noirq,
	.restore_noirq = pci_pm_restore_noirq,
};

#define PCI_PM_OPS_PTR	(&pci_dev_pm_ops)

#else 

#define PCI_PM_OPS_PTR	NULL

#endif 


int __pci_register_driver(struct pci_driver *drv, struct module *owner,
			  const char *mod_name)
{
	int error;

	
	drv->driver.name = drv->name;
	drv->driver.bus = &pci_bus_type;
	drv->driver.owner = owner;
	drv->driver.mod_name = mod_name;

	spin_lock_init(&drv->dynids.lock);
	INIT_LIST_HEAD(&drv->dynids.list);

	
	error = driver_register(&drv->driver);
	if (error)
		goto out;

	error = pci_create_newid_file(drv);
	if (error)
		goto out_newid;

	error = pci_create_removeid_file(drv);
	if (error)
		goto out_removeid;
out:
	return error;

out_removeid:
	pci_remove_newid_file(drv);
out_newid:
	driver_unregister(&drv->driver);
	goto out;
}



void
pci_unregister_driver(struct pci_driver *drv)
{
	pci_remove_removeid_file(drv);
	pci_remove_newid_file(drv);
	driver_unregister(&drv->driver);
	pci_free_dynids(drv);
}

static struct pci_driver pci_compat_driver = {
	.name = "compat"
};


struct pci_driver *
pci_dev_driver(const struct pci_dev *dev)
{
	if (dev->driver)
		return dev->driver;
	else {
		int i;
		for(i=0; i<=PCI_ROM_RESOURCE; i++)
			if (dev->resource[i].flags & IORESOURCE_BUSY)
				return &pci_compat_driver;
	}
	return NULL;
}


static int pci_bus_match(struct device *dev, struct device_driver *drv)
{
	struct pci_dev *pci_dev = to_pci_dev(dev);
	struct pci_driver *pci_drv = to_pci_driver(drv);
	const struct pci_device_id *found_id;

	found_id = pci_match_device(pci_drv, pci_dev);
	if (found_id)
		return 1;

	return 0;
}


struct pci_dev *pci_dev_get(struct pci_dev *dev)
{
	if (dev)
		get_device(&dev->dev);
	return dev;
}


void pci_dev_put(struct pci_dev *dev)
{
	if (dev)
		put_device(&dev->dev);
}

#ifndef CONFIG_HOTPLUG
int pci_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	return -ENODEV;
}
#endif

struct bus_type pci_bus_type = {
	.name		= "pci",
	.match		= pci_bus_match,
	.uevent		= pci_uevent,
	.probe		= pci_device_probe,
	.remove		= pci_device_remove,
	.shutdown	= pci_device_shutdown,
	.dev_attrs	= pci_dev_attrs,
	.bus_attrs	= pci_bus_attrs,
	.pm		= PCI_PM_OPS_PTR,
};

static int __init pci_driver_init(void)
{
	return bus_register(&pci_bus_type);
}

postcore_initcall(pci_driver_init);

EXPORT_SYMBOL_GPL(pci_add_dynid);
EXPORT_SYMBOL(pci_match_id);
EXPORT_SYMBOL(__pci_register_driver);
EXPORT_SYMBOL(pci_unregister_driver);
EXPORT_SYMBOL(pci_dev_driver);
EXPORT_SYMBOL(pci_bus_type);
EXPORT_SYMBOL(pci_dev_get);
EXPORT_SYMBOL(pci_dev_put);
