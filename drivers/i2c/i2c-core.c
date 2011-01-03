






#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/idr.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/hardirq.h>
#include <linux/irqflags.h>
#include <linux/rwsem.h>
#include <asm/uaccess.h>

#include "i2c-core.h"



static DEFINE_MUTEX(core_lock);
static DEFINE_IDR(i2c_adapter_idr);
static LIST_HEAD(userspace_devices);

static struct device_type i2c_client_type;
static int i2c_check_addr(struct i2c_adapter *adapter, int addr);
static int i2c_detect(struct i2c_adapter *adapter, struct i2c_driver *driver);



static const struct i2c_device_id *i2c_match_id(const struct i2c_device_id *id,
						const struct i2c_client *client)
{
	while (id->name[0]) {
		if (strcmp(client->name, id->name) == 0)
			return id;
		id++;
	}
	return NULL;
}

static int i2c_device_match(struct device *dev, struct device_driver *drv)
{
	struct i2c_client	*client = i2c_verify_client(dev);
	struct i2c_driver	*driver;

	if (!client)
		return 0;

	driver = to_i2c_driver(drv);
	
	if (driver->id_table)
		return i2c_match_id(driver->id_table, client) != NULL;

	return 0;
}

#ifdef	CONFIG_HOTPLUG


static int i2c_device_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	struct i2c_client	*client = to_i2c_client(dev);

	if (add_uevent_var(env, "MODALIAS=%s%s",
			   I2C_MODULE_PREFIX, client->name))
		return -ENOMEM;
	dev_dbg(dev, "uevent\n");
	return 0;
}

#else
#define i2c_device_uevent	NULL
#endif	

static int i2c_device_probe(struct device *dev)
{
	struct i2c_client	*client = i2c_verify_client(dev);
	struct i2c_driver	*driver;
	int status;

	if (!client)
		return 0;

	driver = to_i2c_driver(dev->driver);
	if (!driver->probe || !driver->id_table)
		return -ENODEV;
	client->driver = driver;
	if (!device_can_wakeup(&client->dev))
		device_init_wakeup(&client->dev,
					client->flags & I2C_CLIENT_WAKE);
	dev_dbg(dev, "probe\n");

	status = driver->probe(client, i2c_match_id(driver->id_table, client));
	if (status)
		client->driver = NULL;
	return status;
}

static int i2c_device_remove(struct device *dev)
{
	struct i2c_client	*client = i2c_verify_client(dev);
	struct i2c_driver	*driver;
	int			status;

	if (!client || !dev->driver)
		return 0;

	driver = to_i2c_driver(dev->driver);
	if (driver->remove) {
		dev_dbg(dev, "remove\n");
		status = driver->remove(client);
	} else {
		dev->driver = NULL;
		status = 0;
	}
	if (status == 0)
		client->driver = NULL;
	return status;
}

static void i2c_device_shutdown(struct device *dev)
{
	struct i2c_client *client = i2c_verify_client(dev);
	struct i2c_driver *driver;

	if (!client || !dev->driver)
		return;
	driver = to_i2c_driver(dev->driver);
	if (driver->shutdown)
		driver->shutdown(client);
}

#ifdef CONFIG_SUSPEND
static int i2c_device_pm_suspend(struct device *dev)
{
	const struct dev_pm_ops *pm;

	if (!dev->driver)
		return 0;
	pm = dev->driver->pm;
	if (!pm || !pm->suspend)
		return 0;
	return pm->suspend(dev);
}

static int i2c_device_pm_resume(struct device *dev)
{
	const struct dev_pm_ops *pm;

	if (!dev->driver)
		return 0;
	pm = dev->driver->pm;
	if (!pm || !pm->resume)
		return 0;
	return pm->resume(dev);
}
#else
#define i2c_device_pm_suspend	NULL
#define i2c_device_pm_resume	NULL
#endif

static int i2c_device_suspend(struct device *dev, pm_message_t mesg)
{
	struct i2c_client *client = i2c_verify_client(dev);
	struct i2c_driver *driver;

	if (!client || !dev->driver)
		return 0;
	driver = to_i2c_driver(dev->driver);
	if (!driver->suspend)
		return 0;
	return driver->suspend(client, mesg);
}

static int i2c_device_resume(struct device *dev)
{
	struct i2c_client *client = i2c_verify_client(dev);
	struct i2c_driver *driver;

	if (!client || !dev->driver)
		return 0;
	driver = to_i2c_driver(dev->driver);
	if (!driver->resume)
		return 0;
	return driver->resume(client);
}

static void i2c_client_dev_release(struct device *dev)
{
	kfree(to_i2c_client(dev));
}

static ssize_t
show_name(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", dev->type == &i2c_client_type ?
		       to_i2c_client(dev)->name : to_i2c_adapter(dev)->name);
}

static ssize_t
show_modalias(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	return sprintf(buf, "%s%s\n", I2C_MODULE_PREFIX, client->name);
}

static DEVICE_ATTR(name, S_IRUGO, show_name, NULL);
static DEVICE_ATTR(modalias, S_IRUGO, show_modalias, NULL);

static struct attribute *i2c_dev_attrs[] = {
	&dev_attr_name.attr,
	
	&dev_attr_modalias.attr,
	NULL
};

static struct attribute_group i2c_dev_attr_group = {
	.attrs		= i2c_dev_attrs,
};

static const struct attribute_group *i2c_dev_attr_groups[] = {
	&i2c_dev_attr_group,
	NULL
};

const static struct dev_pm_ops i2c_device_pm_ops = {
	.suspend = i2c_device_pm_suspend,
	.resume = i2c_device_pm_resume,
};

struct bus_type i2c_bus_type = {
	.name		= "i2c",
	.match		= i2c_device_match,
	.probe		= i2c_device_probe,
	.remove		= i2c_device_remove,
	.shutdown	= i2c_device_shutdown,
	.suspend	= i2c_device_suspend,
	.resume		= i2c_device_resume,
	.pm		= &i2c_device_pm_ops,
};
EXPORT_SYMBOL_GPL(i2c_bus_type);

static struct device_type i2c_client_type = {
	.groups		= i2c_dev_attr_groups,
	.uevent		= i2c_device_uevent,
	.release	= i2c_client_dev_release,
};



struct i2c_client *i2c_verify_client(struct device *dev)
{
	return (dev->type == &i2c_client_type)
			? to_i2c_client(dev)
			: NULL;
}
EXPORT_SYMBOL(i2c_verify_client);



struct i2c_client *
i2c_new_device(struct i2c_adapter *adap, struct i2c_board_info const *info)
{
	struct i2c_client	*client;
	int			status;

	client = kzalloc(sizeof *client, GFP_KERNEL);
	if (!client)
		return NULL;

	client->adapter = adap;

	client->dev.platform_data = info->platform_data;

	if (info->archdata)
		client->dev.archdata = *info->archdata;

	client->flags = info->flags;
	client->addr = info->addr;
	client->irq = info->irq;

	strlcpy(client->name, info->type, sizeof(client->name));

	
	status = i2c_check_addr(adap, client->addr);
	if (status)
		goto out_err;

	client->dev.parent = &client->adapter->dev;
	client->dev.bus = &i2c_bus_type;
	client->dev.type = &i2c_client_type;

	dev_set_name(&client->dev, "%d-%04x", i2c_adapter_id(adap),
		     client->addr);
	status = device_register(&client->dev);
	if (status)
		goto out_err;

	dev_dbg(&adap->dev, "client [%s] registered with bus id %s\n",
		client->name, dev_name(&client->dev));

	return client;

out_err:
	dev_err(&adap->dev, "Failed to register i2c client %s at 0x%02x "
		"(%d)\n", client->name, client->addr, status);
	kfree(client);
	return NULL;
}
EXPORT_SYMBOL_GPL(i2c_new_device);



void i2c_unregister_device(struct i2c_client *client)
{
	device_unregister(&client->dev);
}
EXPORT_SYMBOL_GPL(i2c_unregister_device);


static const struct i2c_device_id dummy_id[] = {
	{ "dummy", 0 },
	{ },
};

static int dummy_probe(struct i2c_client *client,
		       const struct i2c_device_id *id)
{
	return 0;
}

static int dummy_remove(struct i2c_client *client)
{
	return 0;
}

static struct i2c_driver dummy_driver = {
	.driver.name	= "dummy",
	.probe		= dummy_probe,
	.remove		= dummy_remove,
	.id_table	= dummy_id,
};


struct i2c_client *i2c_new_dummy(struct i2c_adapter *adapter, u16 address)
{
	struct i2c_board_info info = {
		I2C_BOARD_INFO("dummy", address),
	};

	return i2c_new_device(adapter, &info);
}
EXPORT_SYMBOL_GPL(i2c_new_dummy);





static void i2c_adapter_dev_release(struct device *dev)
{
	struct i2c_adapter *adap = to_i2c_adapter(dev);
	complete(&adap->dev_released);
}


static ssize_t
i2c_sysfs_new_device(struct device *dev, struct device_attribute *attr,
		     const char *buf, size_t count)
{
	struct i2c_adapter *adap = to_i2c_adapter(dev);
	struct i2c_board_info info;
	struct i2c_client *client;
	char *blank, end;
	int res;

	dev_warn(dev, "The new_device interface is still experimental "
		 "and may change in a near future\n");
	memset(&info, 0, sizeof(struct i2c_board_info));

	blank = strchr(buf, ' ');
	if (!blank) {
		dev_err(dev, "%s: Missing parameters\n", "new_device");
		return -EINVAL;
	}
	if (blank - buf > I2C_NAME_SIZE - 1) {
		dev_err(dev, "%s: Invalid device name\n", "new_device");
		return -EINVAL;
	}
	memcpy(info.type, buf, blank - buf);

	
	res = sscanf(++blank, "%hi%c", &info.addr, &end);
	if (res < 1) {
		dev_err(dev, "%s: Can't parse I2C address\n", "new_device");
		return -EINVAL;
	}
	if (res > 1  && end != '\n') {
		dev_err(dev, "%s: Extra parameters\n", "new_device");
		return -EINVAL;
	}

	if (info.addr < 0x03 || info.addr > 0x77) {
		dev_err(dev, "%s: Invalid I2C address 0x%hx\n", "new_device",
			info.addr);
		return -EINVAL;
	}

	client = i2c_new_device(adap, &info);
	if (!client)
		return -EEXIST;

	
	mutex_lock(&core_lock);
	list_add_tail(&client->detected, &userspace_devices);
	mutex_unlock(&core_lock);
	dev_info(dev, "%s: Instantiated device %s at 0x%02hx\n", "new_device",
		 info.type, info.addr);

	return count;
}


static ssize_t
i2c_sysfs_delete_device(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct i2c_adapter *adap = to_i2c_adapter(dev);
	struct i2c_client *client, *next;
	unsigned short addr;
	char end;
	int res;

	
	res = sscanf(buf, "%hi%c", &addr, &end);
	if (res < 1) {
		dev_err(dev, "%s: Can't parse I2C address\n", "delete_device");
		return -EINVAL;
	}
	if (res > 1  && end != '\n') {
		dev_err(dev, "%s: Extra parameters\n", "delete_device");
		return -EINVAL;
	}

	
	res = -ENOENT;
	mutex_lock(&core_lock);
	list_for_each_entry_safe(client, next, &userspace_devices, detected) {
		if (client->addr == addr && client->adapter == adap) {
			dev_info(dev, "%s: Deleting device %s at 0x%02hx\n",
				 "delete_device", client->name, client->addr);

			list_del(&client->detected);
			i2c_unregister_device(client);
			res = count;
			break;
		}
	}
	mutex_unlock(&core_lock);

	if (res < 0)
		dev_err(dev, "%s: Can't find device in list\n",
			"delete_device");
	return res;
}

static DEVICE_ATTR(new_device, S_IWUSR, NULL, i2c_sysfs_new_device);
static DEVICE_ATTR(delete_device, S_IWUSR, NULL, i2c_sysfs_delete_device);

static struct attribute *i2c_adapter_attrs[] = {
	&dev_attr_name.attr,
	&dev_attr_new_device.attr,
	&dev_attr_delete_device.attr,
	NULL
};

static struct attribute_group i2c_adapter_attr_group = {
	.attrs		= i2c_adapter_attrs,
};

static const struct attribute_group *i2c_adapter_attr_groups[] = {
	&i2c_adapter_attr_group,
	NULL
};

static struct device_type i2c_adapter_type = {
	.groups		= i2c_adapter_attr_groups,
	.release	= i2c_adapter_dev_release,
};

#ifdef CONFIG_I2C_COMPAT
static struct class_compat *i2c_adapter_compat_class;
#endif

static void i2c_scan_static_board_info(struct i2c_adapter *adapter)
{
	struct i2c_devinfo	*devinfo;

	down_read(&__i2c_board_lock);
	list_for_each_entry(devinfo, &__i2c_board_list, list) {
		if (devinfo->busnum == adapter->nr
				&& !i2c_new_device(adapter,
						&devinfo->board_info))
			dev_err(&adapter->dev,
				"Can't create device at 0x%02x\n",
				devinfo->board_info.addr);
	}
	up_read(&__i2c_board_lock);
}

static int i2c_do_add_adapter(struct device_driver *d, void *data)
{
	struct i2c_driver *driver = to_i2c_driver(d);
	struct i2c_adapter *adap = data;

	
	i2c_detect(adap, driver);

	
	if (driver->attach_adapter) {
		
		driver->attach_adapter(adap);
	}
	return 0;
}

static int i2c_register_adapter(struct i2c_adapter *adap)
{
	int res = 0, dummy;

	
	if (unlikely(WARN_ON(!i2c_bus_type.p))) {
		res = -EAGAIN;
		goto out_list;
	}

	mutex_init(&adap->bus_lock);

	
	if (adap->timeout == 0)
		adap->timeout = HZ;

	dev_set_name(&adap->dev, "i2c-%d", adap->nr);
	adap->dev.bus = &i2c_bus_type;
	adap->dev.type = &i2c_adapter_type;
	res = device_register(&adap->dev);
	if (res)
		goto out_list;

	dev_dbg(&adap->dev, "adapter [%s] registered\n", adap->name);

#ifdef CONFIG_I2C_COMPAT
	res = class_compat_create_link(i2c_adapter_compat_class, &adap->dev,
				       adap->dev.parent);
	if (res)
		dev_warn(&adap->dev,
			 "Failed to create compatibility class link\n");
#endif

	
	if (adap->nr < __i2c_first_dynamic_bus_num)
		i2c_scan_static_board_info(adap);

	
	mutex_lock(&core_lock);
	dummy = bus_for_each_drv(&i2c_bus_type, NULL, adap,
				 i2c_do_add_adapter);
	mutex_unlock(&core_lock);

	return 0;

out_list:
	mutex_lock(&core_lock);
	idr_remove(&i2c_adapter_idr, adap->nr);
	mutex_unlock(&core_lock);
	return res;
}


int i2c_add_adapter(struct i2c_adapter *adapter)
{
	int	id, res = 0;

retry:
	if (idr_pre_get(&i2c_adapter_idr, GFP_KERNEL) == 0)
		return -ENOMEM;

	mutex_lock(&core_lock);
	
	res = idr_get_new_above(&i2c_adapter_idr, adapter,
				__i2c_first_dynamic_bus_num, &id);
	mutex_unlock(&core_lock);

	if (res < 0) {
		if (res == -EAGAIN)
			goto retry;
		return res;
	}

	adapter->nr = id;
	return i2c_register_adapter(adapter);
}
EXPORT_SYMBOL(i2c_add_adapter);


int i2c_add_numbered_adapter(struct i2c_adapter *adap)
{
	int	id;
	int	status;

	if (adap->nr & ~MAX_ID_MASK)
		return -EINVAL;

retry:
	if (idr_pre_get(&i2c_adapter_idr, GFP_KERNEL) == 0)
		return -ENOMEM;

	mutex_lock(&core_lock);
	
	status = idr_get_new_above(&i2c_adapter_idr, adap, adap->nr, &id);
	if (status == 0 && id != adap->nr) {
		status = -EBUSY;
		idr_remove(&i2c_adapter_idr, id);
	}
	mutex_unlock(&core_lock);
	if (status == -EAGAIN)
		goto retry;

	if (status == 0)
		status = i2c_register_adapter(adap);
	return status;
}
EXPORT_SYMBOL_GPL(i2c_add_numbered_adapter);

static int i2c_do_del_adapter(struct device_driver *d, void *data)
{
	struct i2c_driver *driver = to_i2c_driver(d);
	struct i2c_adapter *adapter = data;
	struct i2c_client *client, *_n;
	int res;

	
	list_for_each_entry_safe(client, _n, &driver->clients, detected) {
		if (client->adapter == adapter) {
			dev_dbg(&adapter->dev, "Removing %s at 0x%x\n",
				client->name, client->addr);
			list_del(&client->detected);
			i2c_unregister_device(client);
		}
	}

	if (!driver->detach_adapter)
		return 0;
	res = driver->detach_adapter(adapter);
	if (res)
		dev_err(&adapter->dev, "detach_adapter failed (%d) "
			"for driver [%s]\n", res, driver->driver.name);
	return res;
}

static int __unregister_client(struct device *dev, void *dummy)
{
	struct i2c_client *client = i2c_verify_client(dev);
	if (client)
		i2c_unregister_device(client);
	return 0;
}


int i2c_del_adapter(struct i2c_adapter *adap)
{
	int res = 0;
	struct i2c_adapter *found;
	struct i2c_client *client, *next;

	
	mutex_lock(&core_lock);
	found = idr_find(&i2c_adapter_idr, adap->nr);
	mutex_unlock(&core_lock);
	if (found != adap) {
		pr_debug("i2c-core: attempting to delete unregistered "
			 "adapter [%s]\n", adap->name);
		return -EINVAL;
	}

	
	mutex_lock(&core_lock);
	res = bus_for_each_drv(&i2c_bus_type, NULL, adap,
			       i2c_do_del_adapter);
	mutex_unlock(&core_lock);
	if (res)
		return res;

	
	list_for_each_entry_safe(client, next, &userspace_devices, detected) {
		if (client->adapter == adap) {
			dev_dbg(&adap->dev, "Removing %s at 0x%x\n",
				client->name, client->addr);
			list_del(&client->detected);
			i2c_unregister_device(client);
		}
	}

	
	res = device_for_each_child(&adap->dev, NULL, __unregister_client);

#ifdef CONFIG_I2C_COMPAT
	class_compat_remove_link(i2c_adapter_compat_class, &adap->dev,
				 adap->dev.parent);
#endif

	
	dev_dbg(&adap->dev, "adapter [%s] unregistered\n", adap->name);

	
	init_completion(&adap->dev_released);
	device_unregister(&adap->dev);

	
	wait_for_completion(&adap->dev_released);

	
	mutex_lock(&core_lock);
	idr_remove(&i2c_adapter_idr, adap->nr);
	mutex_unlock(&core_lock);

	
	memset(&adap->dev, 0, sizeof(adap->dev));

	return 0;
}
EXPORT_SYMBOL(i2c_del_adapter);




static int __attach_adapter(struct device *dev, void *data)
{
	struct i2c_adapter *adapter;
	struct i2c_driver *driver = data;

	if (dev->type != &i2c_adapter_type)
		return 0;
	adapter = to_i2c_adapter(dev);

	i2c_detect(adapter, driver);

	
	if (driver->attach_adapter)
		driver->attach_adapter(adapter);

	return 0;
}



int i2c_register_driver(struct module *owner, struct i2c_driver *driver)
{
	int res;

	
	if (unlikely(WARN_ON(!i2c_bus_type.p)))
		return -EAGAIN;

	
	driver->driver.owner = owner;
	driver->driver.bus = &i2c_bus_type;

	
	res = driver_register(&driver->driver);
	if (res)
		return res;

	pr_debug("i2c-core: driver [%s] registered\n", driver->driver.name);

	INIT_LIST_HEAD(&driver->clients);
	
	mutex_lock(&core_lock);
	bus_for_each_dev(&i2c_bus_type, NULL, driver, __attach_adapter);
	mutex_unlock(&core_lock);

	return 0;
}
EXPORT_SYMBOL(i2c_register_driver);

static int __detach_adapter(struct device *dev, void *data)
{
	struct i2c_adapter *adapter;
	struct i2c_driver *driver = data;
	struct i2c_client *client, *_n;

	if (dev->type != &i2c_adapter_type)
		return 0;
	adapter = to_i2c_adapter(dev);

	
	list_for_each_entry_safe(client, _n, &driver->clients, detected) {
		dev_dbg(&adapter->dev, "Removing %s at 0x%x\n",
			client->name, client->addr);
		list_del(&client->detected);
		i2c_unregister_device(client);
	}

	if (driver->detach_adapter) {
		if (driver->detach_adapter(adapter))
			dev_err(&adapter->dev,
				"detach_adapter failed for driver [%s]\n",
				driver->driver.name);
	}

	return 0;
}


void i2c_del_driver(struct i2c_driver *driver)
{
	mutex_lock(&core_lock);
	bus_for_each_dev(&i2c_bus_type, NULL, driver, __detach_adapter);
	mutex_unlock(&core_lock);

	driver_unregister(&driver->driver);
	pr_debug("i2c-core: driver [%s] unregistered\n", driver->driver.name);
}
EXPORT_SYMBOL(i2c_del_driver);



static int __i2c_check_addr(struct device *dev, void *addrp)
{
	struct i2c_client	*client = i2c_verify_client(dev);
	int			addr = *(int *)addrp;

	if (client && client->addr == addr)
		return -EBUSY;
	return 0;
}

static int i2c_check_addr(struct i2c_adapter *adapter, int addr)
{
	return device_for_each_child(&adapter->dev, &addr, __i2c_check_addr);
}


struct i2c_client *i2c_use_client(struct i2c_client *client)
{
	if (client && get_device(&client->dev))
		return client;
	return NULL;
}
EXPORT_SYMBOL(i2c_use_client);


void i2c_release_client(struct i2c_client *client)
{
	if (client)
		put_device(&client->dev);
}
EXPORT_SYMBOL(i2c_release_client);

struct i2c_cmd_arg {
	unsigned	cmd;
	void		*arg;
};

static int i2c_cmd(struct device *dev, void *_arg)
{
	struct i2c_client	*client = i2c_verify_client(dev);
	struct i2c_cmd_arg	*arg = _arg;

	if (client && client->driver && client->driver->command)
		client->driver->command(client, arg->cmd, arg->arg);
	return 0;
}

void i2c_clients_command(struct i2c_adapter *adap, unsigned int cmd, void *arg)
{
	struct i2c_cmd_arg	cmd_arg;

	cmd_arg.cmd = cmd;
	cmd_arg.arg = arg;
	device_for_each_child(&adap->dev, &cmd_arg, i2c_cmd);
}
EXPORT_SYMBOL(i2c_clients_command);

static int __init i2c_init(void)
{
	int retval;

	retval = bus_register(&i2c_bus_type);
	if (retval)
		return retval;
#ifdef CONFIG_I2C_COMPAT
	i2c_adapter_compat_class = class_compat_register("i2c-adapter");
	if (!i2c_adapter_compat_class) {
		retval = -ENOMEM;
		goto bus_err;
	}
#endif
	retval = i2c_add_driver(&dummy_driver);
	if (retval)
		goto class_err;
	return 0;

class_err:
#ifdef CONFIG_I2C_COMPAT
	class_compat_unregister(i2c_adapter_compat_class);
bus_err:
#endif
	bus_unregister(&i2c_bus_type);
	return retval;
}

static void __exit i2c_exit(void)
{
	i2c_del_driver(&dummy_driver);
#ifdef CONFIG_I2C_COMPAT
	class_compat_unregister(i2c_adapter_compat_class);
#endif
	bus_unregister(&i2c_bus_type);
}


postcore_initcall(i2c_init);
module_exit(i2c_exit);




int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	unsigned long orig_jiffies;
	int ret, try;

	

	if (adap->algo->master_xfer) {
#ifdef DEBUG
		for (ret = 0; ret < num; ret++) {
			dev_dbg(&adap->dev, "master_xfer[%d] %c, addr=0x%02x, "
				"len=%d%s\n", ret, (msgs[ret].flags & I2C_M_RD)
				? 'R' : 'W', msgs[ret].addr, msgs[ret].len,
				(msgs[ret].flags & I2C_M_RECV_LEN) ? "+" : "");
		}
#endif

		if (in_atomic() || irqs_disabled()) {
			ret = mutex_trylock(&adap->bus_lock);
			if (!ret)
				
				return -EAGAIN;
		} else {
			mutex_lock_nested(&adap->bus_lock, adap->level);
		}

		
		orig_jiffies = jiffies;
		for (ret = 0, try = 0; try <= adap->retries; try++) {
			ret = adap->algo->master_xfer(adap, msgs, num);
			if (ret != -EAGAIN)
				break;
			if (time_after(jiffies, orig_jiffies + adap->timeout))
				break;
		}
		mutex_unlock(&adap->bus_lock);

		return ret;
	} else {
		dev_dbg(&adap->dev, "I2C level transfers not supported\n");
		return -EOPNOTSUPP;
	}
}
EXPORT_SYMBOL(i2c_transfer);


int i2c_master_send(struct i2c_client *client,const char *buf ,int count)
{
	int ret;
	struct i2c_adapter *adap=client->adapter;
	struct i2c_msg msg;

	msg.addr = client->addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.len = count;
	msg.buf = (char *)buf;

	ret = i2c_transfer(adap, &msg, 1);

	
	return (ret == 1) ? count : ret;
}
EXPORT_SYMBOL(i2c_master_send);


int i2c_master_recv(struct i2c_client *client, char *buf ,int count)
{
	struct i2c_adapter *adap=client->adapter;
	struct i2c_msg msg;
	int ret;

	msg.addr = client->addr;
	msg.flags = client->flags & I2C_M_TEN;
	msg.flags |= I2C_M_RD;
	msg.len = count;
	msg.buf = buf;

	ret = i2c_transfer(adap, &msg, 1);

	
	return (ret == 1) ? count : ret;
}
EXPORT_SYMBOL(i2c_master_recv);



static int i2c_detect_address(struct i2c_client *temp_client, int kind,
			      struct i2c_driver *driver)
{
	struct i2c_board_info info;
	struct i2c_adapter *adapter = temp_client->adapter;
	int addr = temp_client->addr;
	int err;

	
	if (addr < 0x03 || addr > 0x77) {
		dev_warn(&adapter->dev, "Invalid probe address 0x%02x\n",
			 addr);
		return -EINVAL;
	}

	
	if (i2c_check_addr(adapter, addr))
		return 0;

	
	if (kind < 0) {
		if (i2c_smbus_xfer(adapter, addr, 0, 0, 0,
				   I2C_SMBUS_QUICK, NULL) < 0)
			return 0;

		
		if ((addr & ~0x0f) == 0x50)
			i2c_smbus_xfer(adapter, addr, 0, 0, 0,
				       I2C_SMBUS_QUICK, NULL);
	}

	
	memset(&info, 0, sizeof(struct i2c_board_info));
	info.addr = addr;
	err = driver->detect(temp_client, kind, &info);
	if (err) {
		
		return err == -ENODEV ? 0 : err;
	}

	
	if (info.type[0] == '\0') {
		dev_err(&adapter->dev, "%s detection function provided "
			"no name for 0x%x\n", driver->driver.name,
			addr);
	} else {
		struct i2c_client *client;

		
		dev_dbg(&adapter->dev, "Creating %s at 0x%02x\n",
			info.type, info.addr);
		client = i2c_new_device(adapter, &info);
		if (client)
			list_add_tail(&client->detected, &driver->clients);
		else
			dev_err(&adapter->dev, "Failed creating %s at 0x%02x\n",
				info.type, info.addr);
	}
	return 0;
}

static int i2c_detect(struct i2c_adapter *adapter, struct i2c_driver *driver)
{
	const struct i2c_client_address_data *address_data;
	struct i2c_client *temp_client;
	int i, err = 0;
	int adap_id = i2c_adapter_id(adapter);

	address_data = driver->address_data;
	if (!driver->detect || !address_data)
		return 0;

	
	temp_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
	if (!temp_client)
		return -ENOMEM;
	temp_client->adapter = adapter;

	
	if (address_data->forces) {
		const unsigned short * const *forces = address_data->forces;
		int kind;

		for (kind = 0; forces[kind]; kind++) {
			for (i = 0; forces[kind][i] != I2C_CLIENT_END;
			     i += 2) {
				if (forces[kind][i] == adap_id
				 || forces[kind][i] == ANY_I2C_BUS) {
					dev_dbg(&adapter->dev, "found force "
						"parameter for adapter %d, "
						"addr 0x%02x, kind %d\n",
						adap_id, forces[kind][i + 1],
						kind);
					temp_client->addr = forces[kind][i + 1];
					err = i2c_detect_address(temp_client,
						kind, driver);
					if (err)
						goto exit_free;
				}
			}
		}
	}

	
	if (!(adapter->class & driver->class))
		goto exit_free;

	
	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_QUICK)) {
		if (address_data->probe[0] == I2C_CLIENT_END
		 && address_data->normal_i2c[0] == I2C_CLIENT_END)
			goto exit_free;

		dev_warn(&adapter->dev, "SMBus Quick command not supported, "
			 "can't probe for chips\n");
		err = -EOPNOTSUPP;
		goto exit_free;
	}

	
	for (i = 0; address_data->probe[i] != I2C_CLIENT_END; i += 2) {
		if (address_data->probe[i] == adap_id
		 || address_data->probe[i] == ANY_I2C_BUS) {
			dev_dbg(&adapter->dev, "found probe parameter for "
				"adapter %d, addr 0x%02x\n", adap_id,
				address_data->probe[i + 1]);
			temp_client->addr = address_data->probe[i + 1];
			err = i2c_detect_address(temp_client, -1, driver);
			if (err)
				goto exit_free;
		}
	}

	
	for (i = 0; address_data->normal_i2c[i] != I2C_CLIENT_END; i += 1) {
		int j, ignore;

		ignore = 0;
		for (j = 0; address_data->ignore[j] != I2C_CLIENT_END;
		     j += 2) {
			if ((address_data->ignore[j] == adap_id ||
			     address_data->ignore[j] == ANY_I2C_BUS)
			 && address_data->ignore[j + 1]
			    == address_data->normal_i2c[i]) {
				dev_dbg(&adapter->dev, "found ignore "
					"parameter for adapter %d, "
					"addr 0x%02x\n", adap_id,
					address_data->ignore[j + 1]);
				ignore = 1;
				break;
			}
		}
		if (ignore)
			continue;

		dev_dbg(&adapter->dev, "found normal entry for adapter %d, "
			"addr 0x%02x\n", adap_id,
			address_data->normal_i2c[i]);
		temp_client->addr = address_data->normal_i2c[i];
		err = i2c_detect_address(temp_client, -1, driver);
		if (err)
			goto exit_free;
	}

 exit_free:
	kfree(temp_client);
	return err;
}

struct i2c_client *
i2c_new_probed_device(struct i2c_adapter *adap,
		      struct i2c_board_info *info,
		      unsigned short const *addr_list)
{
	int i;

	
	if (!i2c_check_functionality(adap, I2C_FUNC_SMBUS_READ_BYTE)) {
		dev_err(&adap->dev, "Probing not supported\n");
		return NULL;
	}

	for (i = 0; addr_list[i] != I2C_CLIENT_END; i++) {
		
		if (addr_list[i] < 0x03 || addr_list[i] > 0x77) {
			dev_warn(&adap->dev, "Invalid 7-bit address "
				 "0x%02x\n", addr_list[i]);
			continue;
		}

		
		if (i2c_check_addr(adap, addr_list[i])) {
			dev_dbg(&adap->dev, "Address 0x%02x already in "
				"use, not probing\n", addr_list[i]);
			continue;
		}

		
		if ((addr_list[i] & ~0x07) == 0x30
		 || (addr_list[i] & ~0x0f) == 0x50
		 || !i2c_check_functionality(adap, I2C_FUNC_SMBUS_QUICK)) {
			union i2c_smbus_data data;

			if (i2c_smbus_xfer(adap, addr_list[i], 0,
					   I2C_SMBUS_READ, 0,
					   I2C_SMBUS_BYTE, &data) >= 0)
				break;
		} else {
			if (i2c_smbus_xfer(adap, addr_list[i], 0,
					   I2C_SMBUS_WRITE, 0,
					   I2C_SMBUS_QUICK, NULL) >= 0)
				break;
		}
	}

	if (addr_list[i] == I2C_CLIENT_END) {
		dev_dbg(&adap->dev, "Probing failed, no device found\n");
		return NULL;
	}

	info->addr = addr_list[i];
	return i2c_new_device(adap, info);
}
EXPORT_SYMBOL_GPL(i2c_new_probed_device);

struct i2c_adapter* i2c_get_adapter(int id)
{
	struct i2c_adapter *adapter;

	mutex_lock(&core_lock);
	adapter = idr_find(&i2c_adapter_idr, id);
	if (adapter && !try_module_get(adapter->owner))
		adapter = NULL;

	mutex_unlock(&core_lock);
	return adapter;
}
EXPORT_SYMBOL(i2c_get_adapter);

void i2c_put_adapter(struct i2c_adapter *adap)
{
	module_put(adap->owner);
}
EXPORT_SYMBOL(i2c_put_adapter);



#define POLY    (0x1070U << 3)
static u8 crc8(u16 data)
{
	int i;

	for(i = 0; i < 8; i++) {
		if (data & 0x8000)
			data = data ^ POLY;
		data = data << 1;
	}
	return (u8)(data >> 8);
}


static u8 i2c_smbus_pec(u8 crc, u8 *p, size_t count)
{
	int i;

	for(i = 0; i < count; i++)
		crc = crc8((crc ^ p[i]) << 8);
	return crc;
}


static u8 i2c_smbus_msg_pec(u8 pec, struct i2c_msg *msg)
{
	
	u8 addr = (msg->addr << 1) | !!(msg->flags & I2C_M_RD);
	pec = i2c_smbus_pec(pec, &addr, 1);

	
	return i2c_smbus_pec(pec, msg->buf, msg->len);
}


static inline void i2c_smbus_add_pec(struct i2c_msg *msg)
{
	msg->buf[msg->len] = i2c_smbus_msg_pec(0, msg);
	msg->len++;
}


static int i2c_smbus_check_pec(u8 cpec, struct i2c_msg *msg)
{
	u8 rpec = msg->buf[--msg->len];
	cpec = i2c_smbus_msg_pec(cpec, msg);

	if (rpec != cpec) {
		pr_debug("i2c-core: Bad PEC 0x%02x vs. 0x%02x\n",
			rpec, cpec);
		return -EBADMSG;
	}
	return 0;
}


s32 i2c_smbus_read_byte(struct i2c_client *client)
{
	union i2c_smbus_data data;
	int status;

	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
				I2C_SMBUS_READ, 0,
				I2C_SMBUS_BYTE, &data);
	return (status < 0) ? status : data.byte;
}
EXPORT_SYMBOL(i2c_smbus_read_byte);


s32 i2c_smbus_write_byte(struct i2c_client *client, u8 value)
{
	return i2c_smbus_xfer(client->adapter,client->addr,client->flags,
	                      I2C_SMBUS_WRITE, value, I2C_SMBUS_BYTE, NULL);
}
EXPORT_SYMBOL(i2c_smbus_write_byte);


s32 i2c_smbus_read_byte_data(struct i2c_client *client, u8 command)
{
	union i2c_smbus_data data;
	int status;

	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
				I2C_SMBUS_READ, command,
				I2C_SMBUS_BYTE_DATA, &data);
	return (status < 0) ? status : data.byte;
}
EXPORT_SYMBOL(i2c_smbus_read_byte_data);


s32 i2c_smbus_write_byte_data(struct i2c_client *client, u8 command, u8 value)
{
	union i2c_smbus_data data;
	data.byte = value;
	return i2c_smbus_xfer(client->adapter,client->addr,client->flags,
	                      I2C_SMBUS_WRITE,command,
	                      I2C_SMBUS_BYTE_DATA,&data);
}
EXPORT_SYMBOL(i2c_smbus_write_byte_data);


s32 i2c_smbus_read_word_data(struct i2c_client *client, u8 command)
{
	union i2c_smbus_data data;
	int status;

	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
				I2C_SMBUS_READ, command,
				I2C_SMBUS_WORD_DATA, &data);
	return (status < 0) ? status : data.word;
}
EXPORT_SYMBOL(i2c_smbus_read_word_data);


s32 i2c_smbus_write_word_data(struct i2c_client *client, u8 command, u16 value)
{
	union i2c_smbus_data data;
	data.word = value;
	return i2c_smbus_xfer(client->adapter,client->addr,client->flags,
	                      I2C_SMBUS_WRITE,command,
	                      I2C_SMBUS_WORD_DATA,&data);
}
EXPORT_SYMBOL(i2c_smbus_write_word_data);


s32 i2c_smbus_process_call(struct i2c_client *client, u8 command, u16 value)
{
	union i2c_smbus_data data;
	int status;
	data.word = value;

	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
				I2C_SMBUS_WRITE, command,
				I2C_SMBUS_PROC_CALL, &data);
	return (status < 0) ? status : data.word;
}
EXPORT_SYMBOL(i2c_smbus_process_call);


s32 i2c_smbus_read_block_data(struct i2c_client *client, u8 command,
			      u8 *values)
{
	union i2c_smbus_data data;
	int status;

	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
				I2C_SMBUS_READ, command,
				I2C_SMBUS_BLOCK_DATA, &data);
	if (status)
		return status;

	memcpy(values, &data.block[1], data.block[0]);
	return data.block[0];
}
EXPORT_SYMBOL(i2c_smbus_read_block_data);


s32 i2c_smbus_write_block_data(struct i2c_client *client, u8 command,
			       u8 length, const u8 *values)
{
	union i2c_smbus_data data;

	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	data.block[0] = length;
	memcpy(&data.block[1], values, length);
	return i2c_smbus_xfer(client->adapter,client->addr,client->flags,
			      I2C_SMBUS_WRITE,command,
			      I2C_SMBUS_BLOCK_DATA,&data);
}
EXPORT_SYMBOL(i2c_smbus_write_block_data);


s32 i2c_smbus_read_i2c_block_data(struct i2c_client *client, u8 command,
				  u8 length, u8 *values)
{
	union i2c_smbus_data data;
	int status;

	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	data.block[0] = length;
	status = i2c_smbus_xfer(client->adapter, client->addr, client->flags,
				I2C_SMBUS_READ, command,
				I2C_SMBUS_I2C_BLOCK_DATA, &data);
	if (status < 0)
		return status;

	memcpy(values, &data.block[1], data.block[0]);
	return data.block[0];
}
EXPORT_SYMBOL(i2c_smbus_read_i2c_block_data);

s32 i2c_smbus_write_i2c_block_data(struct i2c_client *client, u8 command,
				   u8 length, const u8 *values)
{
	union i2c_smbus_data data;

	if (length > I2C_SMBUS_BLOCK_MAX)
		length = I2C_SMBUS_BLOCK_MAX;
	data.block[0] = length;
	memcpy(data.block + 1, values, length);
	return i2c_smbus_xfer(client->adapter, client->addr, client->flags,
			      I2C_SMBUS_WRITE, command,
			      I2C_SMBUS_I2C_BLOCK_DATA, &data);
}
EXPORT_SYMBOL(i2c_smbus_write_i2c_block_data);


static s32 i2c_smbus_xfer_emulated(struct i2c_adapter * adapter, u16 addr,
                                   unsigned short flags,
                                   char read_write, u8 command, int size,
                                   union i2c_smbus_data * data)
{
	
	unsigned char msgbuf0[I2C_SMBUS_BLOCK_MAX+3];
	unsigned char msgbuf1[I2C_SMBUS_BLOCK_MAX+2];
	int num = read_write == I2C_SMBUS_READ?2:1;
	struct i2c_msg msg[2] = { { addr, flags, 1, msgbuf0 },
	                          { addr, flags | I2C_M_RD, 0, msgbuf1 }
	                        };
	int i;
	u8 partial_pec = 0;
	int status;

	msgbuf0[0] = command;
	switch(size) {
	case I2C_SMBUS_QUICK:
		msg[0].len = 0;
		
		msg[0].flags = flags | (read_write == I2C_SMBUS_READ ?
					I2C_M_RD : 0);
		num = 1;
		break;
	case I2C_SMBUS_BYTE:
		if (read_write == I2C_SMBUS_READ) {
			
			msg[0].flags = I2C_M_RD | flags;
			num = 1;
		}
		break;
	case I2C_SMBUS_BYTE_DATA:
		if (read_write == I2C_SMBUS_READ)
			msg[1].len = 1;
		else {
			msg[0].len = 2;
			msgbuf0[1] = data->byte;
		}
		break;
	case I2C_SMBUS_WORD_DATA:
		if (read_write == I2C_SMBUS_READ)
			msg[1].len = 2;
		else {
			msg[0].len=3;
			msgbuf0[1] = data->word & 0xff;
			msgbuf0[2] = data->word >> 8;
		}
		break;
	case I2C_SMBUS_PROC_CALL:
		num = 2; 
		read_write = I2C_SMBUS_READ;
		msg[0].len = 3;
		msg[1].len = 2;
		msgbuf0[1] = data->word & 0xff;
		msgbuf0[2] = data->word >> 8;
		break;
	case I2C_SMBUS_BLOCK_DATA:
		if (read_write == I2C_SMBUS_READ) {
			msg[1].flags |= I2C_M_RECV_LEN;
			msg[1].len = 1; 
		} else {
			msg[0].len = data->block[0] + 2;
			if (msg[0].len > I2C_SMBUS_BLOCK_MAX + 2) {
				dev_err(&adapter->dev,
					"Invalid block write size %d\n",
					data->block[0]);
				return -EINVAL;
			}
			for (i = 1; i < msg[0].len; i++)
				msgbuf0[i] = data->block[i-1];
		}
		break;
	case I2C_SMBUS_BLOCK_PROC_CALL:
		num = 2; 
		read_write = I2C_SMBUS_READ;
		if (data->block[0] > I2C_SMBUS_BLOCK_MAX) {
			dev_err(&adapter->dev,
				"Invalid block write size %d\n",
				data->block[0]);
			return -EINVAL;
		}
		msg[0].len = data->block[0] + 2;
		for (i = 1; i < msg[0].len; i++)
			msgbuf0[i] = data->block[i-1];
		msg[1].flags |= I2C_M_RECV_LEN;
		msg[1].len = 1; 
		break;
	case I2C_SMBUS_I2C_BLOCK_DATA:
		if (read_write == I2C_SMBUS_READ) {
			msg[1].len = data->block[0];
		} else {
			msg[0].len = data->block[0] + 1;
			if (msg[0].len > I2C_SMBUS_BLOCK_MAX + 1) {
				dev_err(&adapter->dev,
					"Invalid block write size %d\n",
					data->block[0]);
				return -EINVAL;
			}
			for (i = 1; i <= data->block[0]; i++)
				msgbuf0[i] = data->block[i];
		}
		break;
	default:
		dev_err(&adapter->dev, "Unsupported transaction %d\n", size);
		return -EOPNOTSUPP;
	}

	i = ((flags & I2C_CLIENT_PEC) && size != I2C_SMBUS_QUICK
				      && size != I2C_SMBUS_I2C_BLOCK_DATA);
	if (i) {
		
		if (!(msg[0].flags & I2C_M_RD)) {
			if (num == 1) 
				i2c_smbus_add_pec(&msg[0]);
			else 
				partial_pec = i2c_smbus_msg_pec(0, &msg[0]);
		}
		
		if (msg[num-1].flags & I2C_M_RD)
			msg[num-1].len++;
	}

	status = i2c_transfer(adapter, msg, num);
	if (status < 0)
		return status;

	
	if (i && (msg[num-1].flags & I2C_M_RD)) {
		status = i2c_smbus_check_pec(partial_pec, &msg[num-1]);
		if (status < 0)
			return status;
	}

	if (read_write == I2C_SMBUS_READ)
		switch(size) {
			case I2C_SMBUS_BYTE:
				data->byte = msgbuf0[0];
				break;
			case I2C_SMBUS_BYTE_DATA:
				data->byte = msgbuf1[0];
				break;
			case I2C_SMBUS_WORD_DATA:
			case I2C_SMBUS_PROC_CALL:
				data->word = msgbuf1[0] | (msgbuf1[1] << 8);
				break;
			case I2C_SMBUS_I2C_BLOCK_DATA:
				for (i = 0; i < data->block[0]; i++)
					data->block[i+1] = msgbuf1[i];
				break;
			case I2C_SMBUS_BLOCK_DATA:
			case I2C_SMBUS_BLOCK_PROC_CALL:
				for (i = 0; i < msgbuf1[0] + 1; i++)
					data->block[i] = msgbuf1[i];
				break;
		}
	return 0;
}


s32 i2c_smbus_xfer(struct i2c_adapter *adapter, u16 addr, unsigned short flags,
		   char read_write, u8 command, int protocol,
		   union i2c_smbus_data *data)
{
	unsigned long orig_jiffies;
	int try;
	s32 res;

	flags &= I2C_M_TEN | I2C_CLIENT_PEC;

	if (adapter->algo->smbus_xfer) {
		mutex_lock(&adapter->bus_lock);

		
		orig_jiffies = jiffies;
		for (res = 0, try = 0; try <= adapter->retries; try++) {
			res = adapter->algo->smbus_xfer(adapter, addr, flags,
							read_write, command,
							protocol, data);
			if (res != -EAGAIN)
				break;
			if (time_after(jiffies,
				       orig_jiffies + adapter->timeout))
				break;
		}
		mutex_unlock(&adapter->bus_lock);
	} else
		res = i2c_smbus_xfer_emulated(adapter,addr,flags,read_write,
					      command, protocol, data);

	return res;
}
EXPORT_SYMBOL(i2c_smbus_xfer);

MODULE_AUTHOR("Simon G. Vogl <simon@tk.uni-linz.ac.at>");
MODULE_DESCRIPTION("I2C-Bus main module");
MODULE_LICENSE("GPL");
