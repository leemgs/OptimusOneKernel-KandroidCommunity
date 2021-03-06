

#include <linux/pm_qos_params.h>
#include <linux/sched.h>
#include <linux/smp_lock.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/init.h>

#include <linux/uaccess.h>



static s32 max_compare(s32 v1, s32 v2);
static s32 min_compare(s32 v1, s32 v2);

struct pm_qos_power_user {
	int pm_qos_class;
	char name[sizeof("user_01234567")];
};

static BLOCKING_NOTIFIER_HEAD(cpu_dma_lat_notifier);
static struct pm_qos_object cpu_dma_pm_qos = {
	.requirements = {LIST_HEAD_INIT(cpu_dma_pm_qos.requirements.list)},
	.notifiers = &cpu_dma_lat_notifier,
	.name = "cpu_dma_latency",
	.default_value = 2000 * USEC_PER_SEC,
	.target_value = ATOMIC_INIT(2000 * USEC_PER_SEC),
	.comparitor = min_compare
};

static BLOCKING_NOTIFIER_HEAD(network_lat_notifier);
static struct pm_qos_object network_lat_pm_qos = {
	.requirements = {LIST_HEAD_INIT(network_lat_pm_qos.requirements.list)},
	.notifiers = &network_lat_notifier,
	.name = "network_latency",
	.default_value = 2000 * USEC_PER_SEC,
	.target_value = ATOMIC_INIT(2000 * USEC_PER_SEC),
	.comparitor = min_compare
};


static BLOCKING_NOTIFIER_HEAD(network_throughput_notifier);
static struct pm_qos_object network_throughput_pm_qos = {
	.requirements =
		{LIST_HEAD_INIT(network_throughput_pm_qos.requirements.list)},
	.notifiers = &network_throughput_notifier,
	.name = "network_throughput",
	.default_value = 0,
	.target_value = ATOMIC_INIT(0),
	.comparitor = max_compare
};

static BLOCKING_NOTIFIER_HEAD(system_bus_freq_notifier);
static struct pm_qos_object system_bus_freq_pm_qos = {
	.requirements =
		{LIST_HEAD_INIT(system_bus_freq_pm_qos.requirements.list)},
	.notifiers = &system_bus_freq_notifier,
	.name = "system_bus_freq",
	.default_value = 0,
	.target_value = ATOMIC_INIT(0),
	.comparitor = max_compare
};


static struct pm_qos_object *pm_qos_array[PM_QOS_NUM_CLASSES] = {
	[PM_QOS_RESERVED] = NULL,
	[PM_QOS_CPU_DMA_LATENCY] = &cpu_dma_pm_qos,
	[PM_QOS_NETWORK_LATENCY] = &network_lat_pm_qos,
	[PM_QOS_NETWORK_THROUGHPUT] = &network_throughput_pm_qos,
	[PM_QOS_SYSTEM_BUS_FREQ] = &system_bus_freq_pm_qos,
};

static DEFINE_SPINLOCK(pm_qos_lock);
static atomic_t pm_qos_user_id = ATOMIC_INIT(0);

static ssize_t pm_qos_power_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos);
static int pm_qos_power_open(struct inode *inode, struct file *filp);
static int pm_qos_power_release(struct inode *inode, struct file *filp);

static const struct file_operations pm_qos_power_fops = {
	.write = pm_qos_power_write,
	.open = pm_qos_power_open,
	.release = pm_qos_power_release,
};


static s32 max_compare(s32 v1, s32 v2)
{
	return max(v1, v2);
}

static s32 min_compare(s32 v1, s32 v2)
{
	return min(v1, v2);
}

int pm_qos_register_plugin(int pm_qos_class, struct pm_qos_plugin *plugin)
{
	if (pm_qos_class >= PM_QOS_NUM_CLASSES)
		return -EINVAL;

	
	if (pm_qos_array[pm_qos_class]->plugin)
		return -EPERM;

	pm_qos_array[pm_qos_class]->plugin = plugin;

	return 0;
}

static int pm_qos_update_target(struct pm_qos_object *class, char *request_name,
				s32 value, void **request_data)
{
	s32 extreme_value;
	struct requirement_list *node;
	unsigned long flags;
	int call_notifier = 0;

	spin_lock_irqsave(&pm_qos_lock, flags);
	extreme_value = class->default_value;
	list_for_each_entry(node, &class->requirements.list, list)
		extreme_value = class->comparitor(extreme_value, node->value);

	if (atomic_read(&class->target_value) != extreme_value) {
		call_notifier = 1;
		atomic_set(&class->target_value, extreme_value);
		pr_debug(KERN_ERR "new target for qos %s is %d\n",
			class->name, atomic_read(&class->target_value));
	}
	spin_unlock_irqrestore(&pm_qos_lock, flags);

	if (call_notifier)
		blocking_notifier_call_chain(class->notifiers,
			(unsigned long) extreme_value, NULL);

	return 0;
}

static struct pm_qos_plugin pm_qos_default_plugin = {
	.add_fn = pm_qos_update_target,
	.update_fn = pm_qos_update_target,
	.remove_fn = pm_qos_update_target,
};

static int register_pm_qos_misc(struct pm_qos_object *qos)
{
	qos->pm_qos_power_miscdev.minor = MISC_DYNAMIC_MINOR;
	qos->pm_qos_power_miscdev.name = qos->name;
	qos->pm_qos_power_miscdev.fops = &pm_qos_power_fops;

	return misc_register(&qos->pm_qos_power_miscdev);
}

static int find_pm_qos_object_by_minor(int minor)
{
	int pm_qos_class;

	for (pm_qos_class = 0;
		pm_qos_class < PM_QOS_NUM_CLASSES; pm_qos_class++) {
		if (!pm_qos_array[pm_qos_class])
			continue;
		if (minor ==
			pm_qos_array[pm_qos_class]->pm_qos_power_miscdev.minor)
			return pm_qos_class;
	}
	return -1;
}


int pm_qos_requirement(int pm_qos_class)
{
	return atomic_read(&pm_qos_array[pm_qos_class]->target_value);
}
EXPORT_SYMBOL_GPL(pm_qos_requirement);


int pm_qos_add_requirement(int pm_qos_class, char *name, s32 value)
{
	struct requirement_list *dep;
	struct pm_qos_object *class = pm_qos_array[pm_qos_class];
	unsigned long flags;
	int rc = 0;

	dep = kzalloc(sizeof(struct requirement_list), GFP_KERNEL);
	if (!dep) {
		rc = -ENOMEM;
		goto err_dep_alloc_failed;
	}

	if (value == PM_QOS_DEFAULT_VALUE)
		dep->value = class->default_value;
	else
		dep->value = value;
	dep->name = kstrdup(name, GFP_KERNEL);
	if (!dep->name) {
		rc = -ENOMEM;
		goto err_name_alloc_failed;
	}

	
	if (!class->plugin)
		class->plugin = &pm_qos_default_plugin;

	spin_lock_irqsave(&pm_qos_lock, flags);
	list_add(&dep->list, &class->requirements.list);
	spin_unlock_irqrestore(&pm_qos_lock, flags);

	rc = class->plugin->add_fn(class, name, dep->value, &dep->data);
	if (rc)
		goto err_add_fn_failed;

	return rc;

err_add_fn_failed:
	kfree(dep->name);
err_name_alloc_failed:
	kfree(dep);
err_dep_alloc_failed:
	return rc;
}
EXPORT_SYMBOL_GPL(pm_qos_add_requirement);


int pm_qos_update_requirement(int pm_qos_class, char *name, s32 new_value)
{
	struct pm_qos_object *class = pm_qos_array[pm_qos_class];
	unsigned long flags;
	struct requirement_list *node;
	int pending_update = 0;
	int rc = 0;

	spin_lock_irqsave(&pm_qos_lock, flags);
	list_for_each_entry(node,
		&class->requirements.list, list) {
		if (strcmp(node->name, name) == 0) {
			if (new_value == PM_QOS_DEFAULT_VALUE)
				node->value = class->default_value;
			else
				node->value = new_value;
			pending_update = 1;
			break;
		}
	}
	spin_unlock_irqrestore(&pm_qos_lock, flags);

	if (pending_update && class->plugin)
		rc = class->plugin->update_fn(class, name,
			node->value, &node->data);

	return rc;
}
EXPORT_SYMBOL_GPL(pm_qos_update_requirement);


void pm_qos_remove_requirement(int pm_qos_class, char *name)
{
	unsigned long flags;
	struct pm_qos_object *class = pm_qos_array[pm_qos_class];
	struct requirement_list *node;
	int pending_update = 0;
	void *node_data = NULL;

	spin_lock_irqsave(&pm_qos_lock, flags);
	list_for_each_entry(node,
		&class->requirements.list, list) {
		if (strcmp(node->name, name) == 0) {
			node_data = node->data;
			kfree(node->name);
			list_del(&node->list);
			kfree(node);
			pending_update = 1;
			break;
		}
	}
	spin_unlock_irqrestore(&pm_qos_lock, flags);

	if (pending_update && class->plugin->remove_fn)
		class->plugin->remove_fn(class, name,
			class->default_value, &node_data);
}
EXPORT_SYMBOL_GPL(pm_qos_remove_requirement);


int pm_qos_add_notifier(int pm_qos_class, struct notifier_block *notifier)
{
	int retval;

	retval = blocking_notifier_chain_register(
			pm_qos_array[pm_qos_class]->notifiers, notifier);

	return retval;
}
EXPORT_SYMBOL_GPL(pm_qos_add_notifier);


int pm_qos_remove_notifier(int pm_qos_class, struct notifier_block *notifier)
{
	int retval;

	retval = blocking_notifier_chain_unregister(
			pm_qos_array[pm_qos_class]->notifiers, notifier);

	return retval;
}
EXPORT_SYMBOL_GPL(pm_qos_remove_notifier);


static int pm_qos_power_open(struct inode *inode, struct file *filp)
{
	int ret;
	int pm_qos_class;
	struct pm_qos_power_user *usr;

	usr = kzalloc(sizeof(struct pm_qos_power_user), GFP_KERNEL);
	if (!usr)
		return -ENOMEM;

	lock_kernel();
	pm_qos_class = find_pm_qos_object_by_minor(iminor(inode));
	if (pm_qos_class < 0) {
		unlock_kernel();
		kfree(usr);
		return -EPERM;
	}

	usr->pm_qos_class = pm_qos_class;
	snprintf(usr->name, sizeof(usr->name),
		"user_%08x", (unsigned)atomic_inc_return(&pm_qos_user_id));

	ret = pm_qos_add_requirement(usr->pm_qos_class, usr->name,
			PM_QOS_DEFAULT_VALUE);
	unlock_kernel();

	if (ret < 0) {
		kfree(usr);
		return ret;
	}

	filp->private_data = usr;
	return 0;
}

static int pm_qos_power_release(struct inode *inode, struct file *filp)
{
	struct pm_qos_power_user *usr;

	usr = (struct pm_qos_power_user *)filp->private_data;
	pm_qos_remove_requirement(usr->pm_qos_class, usr->name);

	filp->private_data = NULL;
	kfree(usr);
	return 0;
}

static ssize_t pm_qos_power_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	struct pm_qos_power_user *usr;
	s32 value;

	usr = (struct pm_qos_power_user *)filp->private_data;

	if (count != sizeof(s32))
		return -EINVAL;

	if (get_user(value, (s32 *)buf))
		return -EFAULT;

	pm_qos_update_requirement(usr->pm_qos_class, usr->name, value);
	return  sizeof(s32);
}


static int __init pm_qos_power_init(void)
{
	int ret = 0;

	ret = register_pm_qos_misc(&cpu_dma_pm_qos);
	if (ret < 0) {
		printk(KERN_ERR "pm_qos_param: cpu_dma_latency setup failed\n");
		return ret;
	}
	ret = register_pm_qos_misc(&network_lat_pm_qos);
	if (ret < 0) {
		printk(KERN_ERR "pm_qos_param: network_latency setup failed\n");
		return ret;
	}
	ret = register_pm_qos_misc(&network_throughput_pm_qos);
	if (ret < 0) {
		printk(KERN_ERR
			"pm_qos_param: network_throughput setup failed\n");
		return ret;
	}
	ret = register_pm_qos_misc(&system_bus_freq_pm_qos);
	if (ret < 0)
		printk(KERN_ERR
			"pm_qos_param: system_bus_freq setup failed\n");

	return ret;
}

late_initcall(pm_qos_power_init);

#ifdef CONFIG_DEBUG_FS

#define PM_QOS_CLASS_COUNT ARRAY_SIZE(pm_qos_array)

static struct {
	int pm_qos_class;
	struct requirement_list *node;
	unsigned long flags;
} pvote;

static void *votes_start(struct seq_file *m, loff_t *pos)
{
	struct list_head *head;
	int n = 0;

	spin_lock_irqsave(&pm_qos_lock, pvote.flags);

	if (*pos < 0)
		return NULL;

	for (pvote.pm_qos_class = 0
				; pvote.pm_qos_class < PM_QOS_CLASS_COUNT
				; pvote.pm_qos_class++) {
		if (!pm_qos_array[pvote.pm_qos_class])
			continue;
		pvote.node = NULL;
		if (n == *pos)
			return &pvote;
		n++;
		head = &pm_qos_array[pvote.pm_qos_class]->requirements.list;
		list_for_each_entry(pvote.node, head, list) {
			if (n == *pos)
				return &pvote;
			n++;
		}
	}

	return NULL;
}

static void *votes_next(struct seq_file *m, void *p, loff_t *pos)
{
	struct pm_qos_object *class = pm_qos_array[pvote.pm_qos_class];

	(*pos)++;

	if (pvote.node == NULL) {
		pvote.node = list_prepare_entry(pvote.node,
					&class->requirements.list, list);
	}
	list_for_each_entry_continue(pvote.node,
					&class->requirements.list, list) {
		return &pvote;
	}

	pvote.node = NULL;
	do {
		pvote.pm_qos_class++;
		if (pvote.pm_qos_class >= PM_QOS_CLASS_COUNT)
			return NULL;
	} while (!pm_qos_array[pvote.pm_qos_class]);

	return &pvote;
}

static void votes_stop(struct seq_file *m, void *p)
{
	spin_unlock_irqrestore(&pm_qos_lock, pvote.flags);
}

static int votes_show(struct seq_file *m, void *p)
{
	struct pm_qos_object *class = pm_qos_array[pvote.pm_qos_class];

	if (pvote.node) {
		seq_printf(m, "\t%c %12d %s\n",
			(pvote.node->value == atomic_read(&class->target_value)
								? '*' : ' '),
			pvote.node->value,
			pvote.node->name);
	} else {
		if (pvote.pm_qos_class > 1)
			seq_printf(m, "\n");
		seq_printf(m, "%s target(%d) default(%d)\n",
				class->name,
				atomic_read(&class->target_value),
				class->default_value);
	}

	return 0;
}

static const struct seq_operations votes_op = {
	.start	= votes_start,
	.next	= votes_next,
	.stop	= votes_stop,
	.show	= votes_show,
};

static int votes_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &votes_op);
}

static const struct file_operations votes_fops = {
	.open		= votes_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

static __init int pm_qos_init_debugfs(void)
{
	struct dentry *entry;

	entry = debugfs_create_file("pm_qos", 0444, NULL, NULL, &votes_fops);
	if (!entry)
		pr_warning("pm_qos: Could not create debugfs node 'pm_qos'\n");

	return 0;
}

late_initcall(pm_qos_init_debugfs);

#endif 
