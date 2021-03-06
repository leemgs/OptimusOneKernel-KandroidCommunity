

#include <scsi/scsi_dh.h>
#include "../scsi_priv.h"

struct scsi_dh_devinfo_list {
	struct list_head node;
	char vendor[9];
	char model[17];
	struct scsi_device_handler *handler;
};

static DEFINE_SPINLOCK(list_lock);
static LIST_HEAD(scsi_dh_list);
static LIST_HEAD(scsi_dh_dev_list);

static struct scsi_device_handler *get_device_handler(const char *name)
{
	struct scsi_device_handler *tmp, *found = NULL;

	spin_lock(&list_lock);
	list_for_each_entry(tmp, &scsi_dh_list, list) {
		if (!strncmp(tmp->name, name, strlen(tmp->name))) {
			found = tmp;
			break;
		}
	}
	spin_unlock(&list_lock);
	return found;
}


static struct scsi_device_handler *
scsi_dh_cache_lookup(struct scsi_device *sdev)
{
	struct scsi_dh_devinfo_list *tmp;
	struct scsi_device_handler *found_dh = NULL;

	spin_lock(&list_lock);
	list_for_each_entry(tmp, &scsi_dh_dev_list, node) {
		if (!strncmp(sdev->vendor, tmp->vendor, strlen(tmp->vendor)) &&
		    !strncmp(sdev->model, tmp->model, strlen(tmp->model))) {
			found_dh = tmp->handler;
			break;
		}
	}
	spin_unlock(&list_lock);

	return found_dh;
}

static int scsi_dh_handler_lookup(struct scsi_device_handler *scsi_dh,
				  struct scsi_device *sdev)
{
	int i, found = 0;

	for(i = 0; scsi_dh->devlist[i].vendor; i++) {
		if (!strncmp(sdev->vendor, scsi_dh->devlist[i].vendor,
			     strlen(scsi_dh->devlist[i].vendor)) &&
		    !strncmp(sdev->model, scsi_dh->devlist[i].model,
			     strlen(scsi_dh->devlist[i].model))) {
			found = 1;
			break;
		}
	}
	return found;
}


static struct scsi_device_handler *
device_handler_match(struct scsi_device_handler *scsi_dh,
		     struct scsi_device *sdev)
{
	struct scsi_device_handler *found_dh = NULL;
	struct scsi_dh_devinfo_list *tmp;

	found_dh = scsi_dh_cache_lookup(sdev);
	if (found_dh)
		return found_dh;

	if (scsi_dh) {
		if (scsi_dh_handler_lookup(scsi_dh, sdev))
			found_dh = scsi_dh;
	} else {
		struct scsi_device_handler *tmp_dh;

		spin_lock(&list_lock);
		list_for_each_entry(tmp_dh, &scsi_dh_list, list) {
			if (scsi_dh_handler_lookup(tmp_dh, sdev))
				found_dh = tmp_dh;
		}
		spin_unlock(&list_lock);
	}

	if (found_dh) { 
		tmp = kmalloc(sizeof(*tmp), GFP_KERNEL);
		if (tmp) {
			strncpy(tmp->vendor, sdev->vendor, 8);
			strncpy(tmp->model, sdev->model, 16);
			tmp->vendor[8] = '\0';
			tmp->model[16] = '\0';
			tmp->handler = found_dh;
			spin_lock(&list_lock);
			list_add(&tmp->node, &scsi_dh_dev_list);
			spin_unlock(&list_lock);
		} else {
			found_dh = NULL;
		}
	}

	return found_dh;
}


static int scsi_dh_handler_attach(struct scsi_device *sdev,
				  struct scsi_device_handler *scsi_dh)
{
	int err = 0;

	if (sdev->scsi_dh_data) {
		if (sdev->scsi_dh_data->scsi_dh != scsi_dh)
			err = -EBUSY;
		else
			kref_get(&sdev->scsi_dh_data->kref);
	} else if (scsi_dh->attach) {
		err = scsi_dh->attach(sdev);
		if (!err) {
			kref_init(&sdev->scsi_dh_data->kref);
			sdev->scsi_dh_data->sdev = sdev;
		}
	}
	return err;
}

static void __detach_handler (struct kref *kref)
{
	struct scsi_dh_data *scsi_dh_data = container_of(kref, struct scsi_dh_data, kref);
	scsi_dh_data->scsi_dh->detach(scsi_dh_data->sdev);
}


static void scsi_dh_handler_detach(struct scsi_device *sdev,
				   struct scsi_device_handler *scsi_dh)
{
	if (!sdev->scsi_dh_data)
		return;

	if (scsi_dh && scsi_dh != sdev->scsi_dh_data->scsi_dh)
		return;

	if (!scsi_dh)
		scsi_dh = sdev->scsi_dh_data->scsi_dh;

	if (scsi_dh && scsi_dh->detach)
		kref_put(&sdev->scsi_dh_data->kref, __detach_handler);
}


static ssize_t
store_dh_state(struct device *dev, struct device_attribute *attr,
	       const char *buf, size_t count)
{
	struct scsi_device *sdev = to_scsi_device(dev);
	struct scsi_device_handler *scsi_dh;
	int err = -EINVAL;

	if (!sdev->scsi_dh_data) {
		
		if (!(scsi_dh = get_device_handler(buf)))
			return err;
		err = scsi_dh_handler_attach(sdev, scsi_dh);
	} else {
		scsi_dh = sdev->scsi_dh_data->scsi_dh;
		if (!strncmp(buf, "detach", 6)) {
			
			scsi_dh_handler_detach(sdev, scsi_dh);
			err = 0;
		} else if (!strncmp(buf, "activate", 8)) {
			
			if (scsi_dh->activate)
				err = scsi_dh->activate(sdev);
			else
				err = 0;
		}
	}

	return err<0?err:count;
}

static ssize_t
show_dh_state(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct scsi_device *sdev = to_scsi_device(dev);

	if (!sdev->scsi_dh_data)
		return snprintf(buf, 20, "detached\n");

	return snprintf(buf, 20, "%s\n", sdev->scsi_dh_data->scsi_dh->name);
}

static struct device_attribute scsi_dh_state_attr =
	__ATTR(dh_state, S_IRUGO | S_IWUSR, show_dh_state,
	       store_dh_state);


static int scsi_dh_sysfs_attr_add(struct device *dev, void *data)
{
	struct scsi_device *sdev;
	int err;

	if (!scsi_is_sdev_device(dev))
		return 0;

	sdev = to_scsi_device(dev);

	err = device_create_file(&sdev->sdev_gendev,
				 &scsi_dh_state_attr);

	return 0;
}


static int scsi_dh_sysfs_attr_remove(struct device *dev, void *data)
{
	struct scsi_device *sdev;

	if (!scsi_is_sdev_device(dev))
		return 0;

	sdev = to_scsi_device(dev);

	device_remove_file(&sdev->sdev_gendev,
			   &scsi_dh_state_attr);

	return 0;
}


static int scsi_dh_notifier(struct notifier_block *nb,
			    unsigned long action, void *data)
{
	struct device *dev = data;
	struct scsi_device *sdev;
	int err = 0;
	struct scsi_device_handler *devinfo = NULL;

	if (!scsi_is_sdev_device(dev))
		return 0;

	sdev = to_scsi_device(dev);

	if (action == BUS_NOTIFY_ADD_DEVICE) {
		err = device_create_file(dev, &scsi_dh_state_attr);
		
		devinfo = device_handler_match(NULL, sdev);
		if (devinfo)
			err = scsi_dh_handler_attach(sdev, devinfo);
	} else if (action == BUS_NOTIFY_DEL_DEVICE) {
		device_remove_file(dev, &scsi_dh_state_attr);
		scsi_dh_handler_detach(sdev, NULL);
	}
	return err;
}


static int scsi_dh_notifier_add(struct device *dev, void *data)
{
	struct scsi_device_handler *scsi_dh = data;
	struct scsi_device *sdev;

	if (!scsi_is_sdev_device(dev))
		return 0;

	if (!get_device(dev))
		return 0;

	sdev = to_scsi_device(dev);

	if (device_handler_match(scsi_dh, sdev))
		scsi_dh_handler_attach(sdev, scsi_dh);

	put_device(dev);

	return 0;
}


static int scsi_dh_notifier_remove(struct device *dev, void *data)
{
	struct scsi_device_handler *scsi_dh = data;
	struct scsi_device *sdev;

	if (!scsi_is_sdev_device(dev))
		return 0;

	if (!get_device(dev))
		return 0;

	sdev = to_scsi_device(dev);

	scsi_dh_handler_detach(sdev, scsi_dh);

	put_device(dev);

	return 0;
}


int scsi_register_device_handler(struct scsi_device_handler *scsi_dh)
{
	if (get_device_handler(scsi_dh->name))
		return -EBUSY;

	spin_lock(&list_lock);
	list_add(&scsi_dh->list, &scsi_dh_list);
	spin_unlock(&list_lock);
	bus_for_each_dev(&scsi_bus_type, NULL, scsi_dh, scsi_dh_notifier_add);
	printk(KERN_INFO "%s: device handler registered\n", scsi_dh->name);

	return SCSI_DH_OK;
}
EXPORT_SYMBOL_GPL(scsi_register_device_handler);


int scsi_unregister_device_handler(struct scsi_device_handler *scsi_dh)
{
	struct scsi_dh_devinfo_list *tmp, *pos;

	if (!get_device_handler(scsi_dh->name))
		return -ENODEV;

	bus_for_each_dev(&scsi_bus_type, NULL, scsi_dh,
			 scsi_dh_notifier_remove);

	spin_lock(&list_lock);
	list_del(&scsi_dh->list);
	list_for_each_entry_safe(pos, tmp, &scsi_dh_dev_list, node) {
		if (pos->handler == scsi_dh) {
			list_del(&pos->node);
			kfree(pos);
		}
	}
	spin_unlock(&list_lock);
	printk(KERN_INFO "%s: device handler unregistered\n", scsi_dh->name);

	return SCSI_DH_OK;
}
EXPORT_SYMBOL_GPL(scsi_unregister_device_handler);


int scsi_dh_activate(struct request_queue *q)
{
	int err = 0;
	unsigned long flags;
	struct scsi_device *sdev;
	struct scsi_device_handler *scsi_dh = NULL;

	spin_lock_irqsave(q->queue_lock, flags);
	sdev = q->queuedata;
	if (sdev && sdev->scsi_dh_data)
		scsi_dh = sdev->scsi_dh_data->scsi_dh;
	if (!scsi_dh || !get_device(&sdev->sdev_gendev))
		err = SCSI_DH_NOSYS;
	spin_unlock_irqrestore(q->queue_lock, flags);

	if (err)
		return err;

	if (scsi_dh->activate)
		err = scsi_dh->activate(sdev);
	put_device(&sdev->sdev_gendev);
	return err;
}
EXPORT_SYMBOL_GPL(scsi_dh_activate);


int scsi_dh_set_params(struct request_queue *q, const char *params)
{
	int err = -SCSI_DH_NOSYS;
	unsigned long flags;
	struct scsi_device *sdev;
	struct scsi_device_handler *scsi_dh = NULL;

	spin_lock_irqsave(q->queue_lock, flags);
	sdev = q->queuedata;
	if (sdev && sdev->scsi_dh_data)
		scsi_dh = sdev->scsi_dh_data->scsi_dh;
	if (scsi_dh && scsi_dh->set_params && get_device(&sdev->sdev_gendev))
		err = 0;
	spin_unlock_irqrestore(q->queue_lock, flags);

	if (err)
		return err;
	err = scsi_dh->set_params(sdev, params);
	put_device(&sdev->sdev_gendev);
	return err;
}
EXPORT_SYMBOL_GPL(scsi_dh_set_params);


int scsi_dh_handler_exist(const char *name)
{
	return (get_device_handler(name) != NULL);
}
EXPORT_SYMBOL_GPL(scsi_dh_handler_exist);


int scsi_dh_attach(struct request_queue *q, const char *name)
{
	unsigned long flags;
	struct scsi_device *sdev;
	struct scsi_device_handler *scsi_dh;
	int err = 0;

	scsi_dh = get_device_handler(name);
	if (!scsi_dh)
		return -EINVAL;

	spin_lock_irqsave(q->queue_lock, flags);
	sdev = q->queuedata;
	if (!sdev || !get_device(&sdev->sdev_gendev))
		err = -ENODEV;
	spin_unlock_irqrestore(q->queue_lock, flags);

	if (!err) {
		err = scsi_dh_handler_attach(sdev, scsi_dh);
		put_device(&sdev->sdev_gendev);
	}
	return err;
}
EXPORT_SYMBOL_GPL(scsi_dh_attach);


void scsi_dh_detach(struct request_queue *q)
{
	unsigned long flags;
	struct scsi_device *sdev;
	struct scsi_device_handler *scsi_dh = NULL;

	spin_lock_irqsave(q->queue_lock, flags);
	sdev = q->queuedata;
	if (!sdev || !get_device(&sdev->sdev_gendev))
		sdev = NULL;
	spin_unlock_irqrestore(q->queue_lock, flags);

	if (!sdev)
		return;

	if (sdev->scsi_dh_data) {
		scsi_dh = sdev->scsi_dh_data->scsi_dh;
		scsi_dh_handler_detach(sdev, scsi_dh);
	}
	put_device(&sdev->sdev_gendev);
}
EXPORT_SYMBOL_GPL(scsi_dh_detach);

static struct notifier_block scsi_dh_nb = {
	.notifier_call = scsi_dh_notifier
};

static int __init scsi_dh_init(void)
{
	int r;

	r = bus_register_notifier(&scsi_bus_type, &scsi_dh_nb);

	if (!r)
		bus_for_each_dev(&scsi_bus_type, NULL, NULL,
				 scsi_dh_sysfs_attr_add);

	return r;
}

static void __exit scsi_dh_exit(void)
{
	bus_for_each_dev(&scsi_bus_type, NULL, NULL,
			 scsi_dh_sysfs_attr_remove);
	bus_unregister_notifier(&scsi_bus_type, &scsi_dh_nb);
}

module_init(scsi_dh_init);
module_exit(scsi_dh_exit);

MODULE_DESCRIPTION("SCSI device handler");
MODULE_AUTHOR("Chandra Seetharaman <sekharan@us.ibm.com>");
MODULE_LICENSE("GPL");
