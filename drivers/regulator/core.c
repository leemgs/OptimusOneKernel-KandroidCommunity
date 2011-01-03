

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/suspend.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#define REGULATOR_VERSION "0.5"

static DEFINE_MUTEX(regulator_list_mutex);
static LIST_HEAD(regulator_list);
static LIST_HEAD(regulator_map_list);
static int has_full_constraints;


struct regulator_map {
	struct list_head list;
	const char *dev_name;   
	const char *supply;
	struct regulator_dev *regulator;
};


struct regulator {
	struct device *dev;
	struct list_head list;
	int uA_load;
	int min_uV;
	int max_uV;
	char *supply_name;
	struct device_attribute dev_attr;
	struct regulator_dev *rdev;
};

static int _regulator_is_enabled(struct regulator_dev *rdev);
static int _regulator_disable(struct regulator_dev *rdev);
static int _regulator_get_voltage(struct regulator_dev *rdev);
static int _regulator_get_current_limit(struct regulator_dev *rdev);
static unsigned int _regulator_get_mode(struct regulator_dev *rdev);
static void _notifier_call_chain(struct regulator_dev *rdev,
				  unsigned long event, void *data);


static struct regulator *get_device_regulator(struct device *dev)
{
	struct regulator *regulator = NULL;
	struct regulator_dev *rdev;

	mutex_lock(&regulator_list_mutex);
	list_for_each_entry(rdev, &regulator_list, list) {
		mutex_lock(&rdev->mutex);
		list_for_each_entry(regulator, &rdev->consumer_list, list) {
			if (regulator->dev == dev) {
				mutex_unlock(&rdev->mutex);
				mutex_unlock(&regulator_list_mutex);
				return regulator;
			}
		}
		mutex_unlock(&rdev->mutex);
	}
	mutex_unlock(&regulator_list_mutex);
	return NULL;
}


static int regulator_check_voltage(struct regulator_dev *rdev,
				   int *min_uV, int *max_uV)
{
	BUG_ON(*min_uV > *max_uV);

	if (!rdev->constraints) {
		printk(KERN_ERR "%s: no constraints for %s\n", __func__,
		       rdev->desc->name);
		return -ENODEV;
	}
	if (!(rdev->constraints->valid_ops_mask & REGULATOR_CHANGE_VOLTAGE)) {
		printk(KERN_ERR "%s: operation not allowed for %s\n",
		       __func__, rdev->desc->name);
		return -EPERM;
	}

	if (*max_uV > rdev->constraints->max_uV)
		*max_uV = rdev->constraints->max_uV;
	if (*min_uV < rdev->constraints->min_uV)
		*min_uV = rdev->constraints->min_uV;

	if (*min_uV > *max_uV)
		return -EINVAL;

	return 0;
}


static int regulator_check_current_limit(struct regulator_dev *rdev,
					int *min_uA, int *max_uA)
{
	BUG_ON(*min_uA > *max_uA);

	if (!rdev->constraints) {
		printk(KERN_ERR "%s: no constraints for %s\n", __func__,
		       rdev->desc->name);
		return -ENODEV;
	}
	if (!(rdev->constraints->valid_ops_mask & REGULATOR_CHANGE_CURRENT)) {
		printk(KERN_ERR "%s: operation not allowed for %s\n",
		       __func__, rdev->desc->name);
		return -EPERM;
	}

	if (*max_uA > rdev->constraints->max_uA)
		*max_uA = rdev->constraints->max_uA;
	if (*min_uA < rdev->constraints->min_uA)
		*min_uA = rdev->constraints->min_uA;

	if (*min_uA > *max_uA)
		return -EINVAL;

	return 0;
}


static int regulator_check_mode(struct regulator_dev *rdev, int mode)
{
	switch (mode) {
	case REGULATOR_MODE_FAST:
	case REGULATOR_MODE_NORMAL:
	case REGULATOR_MODE_IDLE:
	case REGULATOR_MODE_STANDBY:
		break;
	default:
		return -EINVAL;
	}

	if (!rdev->constraints) {
		printk(KERN_ERR "%s: no constraints for %s\n", __func__,
		       rdev->desc->name);
		return -ENODEV;
	}
	if (!(rdev->constraints->valid_ops_mask & REGULATOR_CHANGE_MODE)) {
		printk(KERN_ERR "%s: operation not allowed for %s\n",
		       __func__, rdev->desc->name);
		return -EPERM;
	}
	if (!(rdev->constraints->valid_modes_mask & mode)) {
		printk(KERN_ERR "%s: invalid mode %x for %s\n",
		       __func__, mode, rdev->desc->name);
		return -EINVAL;
	}
	return 0;
}


static int regulator_check_drms(struct regulator_dev *rdev)
{
	if (!rdev->constraints) {
		printk(KERN_ERR "%s: no constraints for %s\n", __func__,
		       rdev->desc->name);
		return -ENODEV;
	}
	if (!(rdev->constraints->valid_ops_mask & REGULATOR_CHANGE_DRMS)) {
		printk(KERN_ERR "%s: operation not allowed for %s\n",
		       __func__, rdev->desc->name);
		return -EPERM;
	}
	return 0;
}

static ssize_t device_requested_uA_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct regulator *regulator;

	regulator = get_device_regulator(dev);
	if (regulator == NULL)
		return 0;

	return sprintf(buf, "%d\n", regulator->uA_load);
}

static ssize_t regulator_uV_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	ssize_t ret;

	mutex_lock(&rdev->mutex);
	ret = sprintf(buf, "%d\n", _regulator_get_voltage(rdev));
	mutex_unlock(&rdev->mutex);

	return ret;
}
static DEVICE_ATTR(microvolts, 0444, regulator_uV_show, NULL);

static ssize_t regulator_uA_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", _regulator_get_current_limit(rdev));
}
static DEVICE_ATTR(microamps, 0444, regulator_uA_show, NULL);

static ssize_t regulator_name_show(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	const char *name;

	if (rdev->constraints && rdev->constraints->name)
		name = rdev->constraints->name;
	else if (rdev->desc->name)
		name = rdev->desc->name;
	else
		name = "";

	return sprintf(buf, "%s\n", name);
}

static ssize_t regulator_print_opmode(char *buf, int mode)
{
	switch (mode) {
	case REGULATOR_MODE_FAST:
		return sprintf(buf, "fast\n");
	case REGULATOR_MODE_NORMAL:
		return sprintf(buf, "normal\n");
	case REGULATOR_MODE_IDLE:
		return sprintf(buf, "idle\n");
	case REGULATOR_MODE_STANDBY:
		return sprintf(buf, "standby\n");
	}
	return sprintf(buf, "unknown\n");
}

static ssize_t regulator_opmode_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return regulator_print_opmode(buf, _regulator_get_mode(rdev));
}
static DEVICE_ATTR(opmode, 0444, regulator_opmode_show, NULL);

static ssize_t regulator_print_state(char *buf, int state)
{
	if (state > 0)
		return sprintf(buf, "enabled\n");
	else if (state == 0)
		return sprintf(buf, "disabled\n");
	else
		return sprintf(buf, "unknown\n");
}

static ssize_t regulator_state_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	ssize_t ret;

	mutex_lock(&rdev->mutex);
	ret = regulator_print_state(buf, _regulator_is_enabled(rdev));
	mutex_unlock(&rdev->mutex);

	return ret;
}
static DEVICE_ATTR(state, 0444, regulator_state_show, NULL);

static ssize_t regulator_status_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	int status;
	char *label;

	status = rdev->desc->ops->get_status(rdev);
	if (status < 0)
		return status;

	switch (status) {
	case REGULATOR_STATUS_OFF:
		label = "off";
		break;
	case REGULATOR_STATUS_ON:
		label = "on";
		break;
	case REGULATOR_STATUS_ERROR:
		label = "error";
		break;
	case REGULATOR_STATUS_FAST:
		label = "fast";
		break;
	case REGULATOR_STATUS_NORMAL:
		label = "normal";
		break;
	case REGULATOR_STATUS_IDLE:
		label = "idle";
		break;
	case REGULATOR_STATUS_STANDBY:
		label = "standby";
		break;
	default:
		return -ERANGE;
	}

	return sprintf(buf, "%s\n", label);
}
static DEVICE_ATTR(status, 0444, regulator_status_show, NULL);

static ssize_t regulator_min_uA_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "constraint not defined\n");

	return sprintf(buf, "%d\n", rdev->constraints->min_uA);
}
static DEVICE_ATTR(min_microamps, 0444, regulator_min_uA_show, NULL);

static ssize_t regulator_max_uA_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "constraint not defined\n");

	return sprintf(buf, "%d\n", rdev->constraints->max_uA);
}
static DEVICE_ATTR(max_microamps, 0444, regulator_max_uA_show, NULL);

static ssize_t regulator_min_uV_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "constraint not defined\n");

	return sprintf(buf, "%d\n", rdev->constraints->min_uV);
}
static DEVICE_ATTR(min_microvolts, 0444, regulator_min_uV_show, NULL);

static ssize_t regulator_max_uV_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	if (!rdev->constraints)
		return sprintf(buf, "constraint not defined\n");

	return sprintf(buf, "%d\n", rdev->constraints->max_uV);
}
static DEVICE_ATTR(max_microvolts, 0444, regulator_max_uV_show, NULL);

static ssize_t regulator_total_uA_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	struct regulator *regulator;
	int uA = 0;

	mutex_lock(&rdev->mutex);
	list_for_each_entry(regulator, &rdev->consumer_list, list)
	    uA += regulator->uA_load;
	mutex_unlock(&rdev->mutex);
	return sprintf(buf, "%d\n", uA);
}
static DEVICE_ATTR(requested_microamps, 0444, regulator_total_uA_show, NULL);

static ssize_t regulator_num_users_show(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", rdev->use_count);
}

static ssize_t regulator_type_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	switch (rdev->desc->type) {
	case REGULATOR_VOLTAGE:
		return sprintf(buf, "voltage\n");
	case REGULATOR_CURRENT:
		return sprintf(buf, "current\n");
	}
	return sprintf(buf, "unknown\n");
}

static ssize_t regulator_suspend_mem_uV_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", rdev->constraints->state_mem.uV);
}
static DEVICE_ATTR(suspend_mem_microvolts, 0444,
		regulator_suspend_mem_uV_show, NULL);

static ssize_t regulator_suspend_disk_uV_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", rdev->constraints->state_disk.uV);
}
static DEVICE_ATTR(suspend_disk_microvolts, 0444,
		regulator_suspend_disk_uV_show, NULL);

static ssize_t regulator_suspend_standby_uV_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", rdev->constraints->state_standby.uV);
}
static DEVICE_ATTR(suspend_standby_microvolts, 0444,
		regulator_suspend_standby_uV_show, NULL);

static ssize_t regulator_suspend_mem_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return regulator_print_opmode(buf,
		rdev->constraints->state_mem.mode);
}
static DEVICE_ATTR(suspend_mem_mode, 0444,
		regulator_suspend_mem_mode_show, NULL);

static ssize_t regulator_suspend_disk_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return regulator_print_opmode(buf,
		rdev->constraints->state_disk.mode);
}
static DEVICE_ATTR(suspend_disk_mode, 0444,
		regulator_suspend_disk_mode_show, NULL);

static ssize_t regulator_suspend_standby_mode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return regulator_print_opmode(buf,
		rdev->constraints->state_standby.mode);
}
static DEVICE_ATTR(suspend_standby_mode, 0444,
		regulator_suspend_standby_mode_show, NULL);

static ssize_t regulator_suspend_mem_state_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return regulator_print_state(buf,
			rdev->constraints->state_mem.enabled);
}
static DEVICE_ATTR(suspend_mem_state, 0444,
		regulator_suspend_mem_state_show, NULL);

static ssize_t regulator_suspend_disk_state_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return regulator_print_state(buf,
			rdev->constraints->state_disk.enabled);
}
static DEVICE_ATTR(suspend_disk_state, 0444,
		regulator_suspend_disk_state_show, NULL);

static ssize_t regulator_suspend_standby_state_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);

	return regulator_print_state(buf,
			rdev->constraints->state_standby.enabled);
}
static DEVICE_ATTR(suspend_standby_state, 0444,
		regulator_suspend_standby_state_show, NULL);



static struct device_attribute regulator_dev_attrs[] = {
	__ATTR(name, 0444, regulator_name_show, NULL),
	__ATTR(num_users, 0444, regulator_num_users_show, NULL),
	__ATTR(type, 0444, regulator_type_show, NULL),
	__ATTR_NULL,
};

static void regulator_dev_release(struct device *dev)
{
	struct regulator_dev *rdev = dev_get_drvdata(dev);
	kfree(rdev);
}

static struct class regulator_class = {
	.name = "regulator",
	.dev_release = regulator_dev_release,
	.dev_attrs = regulator_dev_attrs,
};


static void drms_uA_update(struct regulator_dev *rdev)
{
	struct regulator *sibling;
	int current_uA = 0, output_uV, input_uV, err;
	unsigned int mode;

	err = regulator_check_drms(rdev);
	if (err < 0 || !rdev->desc->ops->get_optimum_mode ||
	    !rdev->desc->ops->get_voltage || !rdev->desc->ops->set_mode)
		return;

	
	output_uV = rdev->desc->ops->get_voltage(rdev);
	if (output_uV <= 0)
		return;

	
	if (rdev->supply && rdev->supply->desc->ops->get_voltage)
		input_uV = rdev->supply->desc->ops->get_voltage(rdev->supply);
	else
		input_uV = rdev->constraints->input_uV;
	if (input_uV <= 0)
		return;

	
	list_for_each_entry(sibling, &rdev->consumer_list, list)
	    current_uA += sibling->uA_load;

	
	mode = rdev->desc->ops->get_optimum_mode(rdev, input_uV,
						  output_uV, current_uA);

	
	err = regulator_check_mode(rdev, mode);
	if (err == 0)
		rdev->desc->ops->set_mode(rdev, mode);
}

static int suspend_set_state(struct regulator_dev *rdev,
	struct regulator_state *rstate)
{
	int ret = 0;

	
	if (!rdev->desc->ops->set_suspend_enable ||
		!rdev->desc->ops->set_suspend_disable) {
		printk(KERN_ERR "%s: no way to set suspend state\n",
			__func__);
		return -EINVAL;
	}

	if (rstate->enabled)
		ret = rdev->desc->ops->set_suspend_enable(rdev);
	else
		ret = rdev->desc->ops->set_suspend_disable(rdev);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to enabled/disable\n", __func__);
		return ret;
	}

	if (rdev->desc->ops->set_suspend_voltage && rstate->uV > 0) {
		ret = rdev->desc->ops->set_suspend_voltage(rdev, rstate->uV);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to set voltage\n",
				__func__);
			return ret;
		}
	}

	if (rdev->desc->ops->set_suspend_mode && rstate->mode > 0) {
		ret = rdev->desc->ops->set_suspend_mode(rdev, rstate->mode);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to set mode\n", __func__);
			return ret;
		}
	}
	return ret;
}


static int suspend_prepare(struct regulator_dev *rdev, suspend_state_t state)
{
	if (!rdev->constraints)
		return -EINVAL;

	switch (state) {
	case PM_SUSPEND_STANDBY:
		return suspend_set_state(rdev,
			&rdev->constraints->state_standby);
	case PM_SUSPEND_MEM:
		return suspend_set_state(rdev,
			&rdev->constraints->state_mem);
	case PM_SUSPEND_MAX:
		return suspend_set_state(rdev,
			&rdev->constraints->state_disk);
	default:
		return -EINVAL;
	}
}

static void print_constraints(struct regulator_dev *rdev)
{
	struct regulation_constraints *constraints = rdev->constraints;
	char buf[80] = "";
	int count;

	if (rdev->desc->type == REGULATOR_VOLTAGE) {
		if (constraints->min_uV == constraints->max_uV)
			count = sprintf(buf, "%d mV ",
					constraints->min_uV / 1000);
		else
			count = sprintf(buf, "%d <--> %d mV ",
					constraints->min_uV / 1000,
					constraints->max_uV / 1000);
	} else {
		if (constraints->min_uA == constraints->max_uA)
			count = sprintf(buf, "%d mA ",
					constraints->min_uA / 1000);
		else
			count = sprintf(buf, "%d <--> %d mA ",
					constraints->min_uA / 1000,
					constraints->max_uA / 1000);
	}
	if (constraints->valid_modes_mask & REGULATOR_MODE_FAST)
		count += sprintf(buf + count, "fast ");
	if (constraints->valid_modes_mask & REGULATOR_MODE_NORMAL)
		count += sprintf(buf + count, "normal ");
	if (constraints->valid_modes_mask & REGULATOR_MODE_IDLE)
		count += sprintf(buf + count, "idle ");
	if (constraints->valid_modes_mask & REGULATOR_MODE_STANDBY)
		count += sprintf(buf + count, "standby");

	printk(KERN_INFO "regulator: %s: %s\n", rdev->desc->name, buf);
}


static int set_machine_constraints(struct regulator_dev *rdev,
	struct regulation_constraints *constraints)
{
	int ret = 0;
	const char *name;
	struct regulator_ops *ops = rdev->desc->ops;

	if (constraints->name)
		name = constraints->name;
	else if (rdev->desc->name)
		name = rdev->desc->name;
	else
		name = "regulator";

	
	if (ops->list_voltage && rdev->desc->n_voltages) {
		int	count = rdev->desc->n_voltages;
		int	i;
		int	min_uV = INT_MAX;
		int	max_uV = INT_MIN;
		int	cmin = constraints->min_uV;
		int	cmax = constraints->max_uV;

		
		if (count == 1 && !cmin) {
			cmin = 1;
			cmax = INT_MAX;
			constraints->min_uV = cmin;
			constraints->max_uV = cmax;
		}

		
		if ((cmin == 0) && (cmax == 0))
			goto out;

		
		if (cmin <= 0 || cmax <= 0 || cmax < cmin) {
			pr_err("%s: %s '%s' voltage constraints\n",
				       __func__, "invalid", name);
			ret = -EINVAL;
			goto out;
		}

		
		for (i = 0; i < count; i++) {
			int	value;

			value = ops->list_voltage(rdev, i);
			if (value <= 0)
				continue;

			
			if (value >= cmin && value < min_uV)
				min_uV = value;
			if (value <= cmax && value > max_uV)
				max_uV = value;
		}

		
		if (max_uV < min_uV) {
			pr_err("%s: %s '%s' voltage constraints\n",
				       __func__, "unsupportable", name);
			ret = -EINVAL;
			goto out;
		}

		
		if (constraints->min_uV < min_uV) {
			pr_debug("%s: override '%s' %s, %d -> %d\n",
				       __func__, name, "min_uV",
					constraints->min_uV, min_uV);
			constraints->min_uV = min_uV;
		}
		if (constraints->max_uV > max_uV) {
			pr_debug("%s: override '%s' %s, %d -> %d\n",
				       __func__, name, "max_uV",
					constraints->max_uV, max_uV);
			constraints->max_uV = max_uV;
		}
	}

	rdev->constraints = constraints;

	
	if (rdev->constraints->apply_uV &&
		rdev->constraints->min_uV == rdev->constraints->max_uV &&
		ops->set_voltage) {
		ret = ops->set_voltage(rdev,
			rdev->constraints->min_uV, rdev->constraints->max_uV);
			if (ret < 0) {
				printk(KERN_ERR "%s: failed to apply %duV constraint to %s\n",
				       __func__,
				       rdev->constraints->min_uV, name);
				rdev->constraints = NULL;
				goto out;
			}
	}

	
	if (constraints->initial_state) {
		ret = suspend_prepare(rdev, constraints->initial_state);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to set suspend state for %s\n",
			       __func__, name);
			rdev->constraints = NULL;
			goto out;
		}
	}

	if (constraints->initial_mode) {
		if (!ops->set_mode) {
			printk(KERN_ERR "%s: no set_mode operation for %s\n",
			       __func__, name);
			ret = -EINVAL;
			goto out;
		}

		ret = ops->set_mode(rdev, constraints->initial_mode);
		if (ret < 0) {
			printk(KERN_ERR
			       "%s: failed to set initial mode for %s: %d\n",
			       __func__, name, ret);
			goto out;
		}
	}

	
	if ((constraints->always_on || constraints->boot_on) && ops->enable) {
		ret = ops->enable(rdev);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to enable %s\n",
			       __func__, name);
			rdev->constraints = NULL;
			goto out;
		}
	}

	print_constraints(rdev);
out:
	return ret;
}


static int set_supply(struct regulator_dev *rdev,
	struct regulator_dev *supply_rdev)
{
	int err;

	err = sysfs_create_link(&rdev->dev.kobj, &supply_rdev->dev.kobj,
				"supply");
	if (err) {
		printk(KERN_ERR
		       "%s: could not add device link %s err %d\n",
		       __func__, supply_rdev->dev.kobj.name, err);
		       goto out;
	}
	rdev->supply = supply_rdev;
	list_add(&rdev->slist, &supply_rdev->supply_list);
out:
	return err;
}


static int set_consumer_device_supply(struct regulator_dev *rdev,
	struct device *consumer_dev, const char *consumer_dev_name,
	const char *supply)
{
	struct regulator_map *node;
	int has_dev;

	if (consumer_dev && consumer_dev_name)
		return -EINVAL;

	if (!consumer_dev_name && consumer_dev)
		consumer_dev_name = dev_name(consumer_dev);

	if (supply == NULL)
		return -EINVAL;

	if (consumer_dev_name != NULL)
		has_dev = 1;
	else
		has_dev = 0;

	list_for_each_entry(node, &regulator_map_list, list) {
		if (consumer_dev_name != node->dev_name)
			continue;
		if (strcmp(node->supply, supply) != 0)
			continue;

		dev_dbg(consumer_dev, "%s/%s is '%s' supply; fail %s/%s\n",
				dev_name(&node->regulator->dev),
				node->regulator->desc->name,
				supply,
				dev_name(&rdev->dev), rdev->desc->name);
		return -EBUSY;
	}

	node = kzalloc(sizeof(struct regulator_map), GFP_KERNEL);
	if (node == NULL)
		return -ENOMEM;

	node->regulator = rdev;
	node->supply = supply;

	if (has_dev) {
		node->dev_name = kstrdup(consumer_dev_name, GFP_KERNEL);
		if (node->dev_name == NULL) {
			kfree(node);
			return -ENOMEM;
		}
	}

	list_add(&node->list, &regulator_map_list);
	return 0;
}

static void unset_consumer_device_supply(struct regulator_dev *rdev,
	const char *consumer_dev_name, struct device *consumer_dev)
{
	struct regulator_map *node, *n;

	if (consumer_dev && !consumer_dev_name)
		consumer_dev_name = dev_name(consumer_dev);

	list_for_each_entry_safe(node, n, &regulator_map_list, list) {
		if (rdev != node->regulator)
			continue;

		if (consumer_dev_name && node->dev_name &&
		    strcmp(consumer_dev_name, node->dev_name))
			continue;

		list_del(&node->list);
		kfree(node->dev_name);
		kfree(node);
		return;
	}
}

static void unset_regulator_supplies(struct regulator_dev *rdev)
{
	struct regulator_map *node, *n;

	list_for_each_entry_safe(node, n, &regulator_map_list, list) {
		if (rdev == node->regulator) {
			list_del(&node->list);
			kfree(node->dev_name);
			kfree(node);
			return;
		}
	}
}

#define REG_STR_SIZE	32

static struct regulator *create_regulator(struct regulator_dev *rdev,
					  struct device *dev,
					  const char *supply_name)
{
	struct regulator *regulator;
	char buf[REG_STR_SIZE];
	int err, size;

	regulator = kzalloc(sizeof(*regulator), GFP_KERNEL);
	if (regulator == NULL)
		return NULL;

	mutex_lock(&rdev->mutex);
	regulator->rdev = rdev;
	list_add(&regulator->list, &rdev->consumer_list);

	if (dev) {
		
		size = scnprintf(buf, REG_STR_SIZE, "microamps_requested_%s",
			supply_name);
		if (size >= REG_STR_SIZE)
			goto overflow_err;

		regulator->dev = dev;
		regulator->dev_attr.attr.name = kstrdup(buf, GFP_KERNEL);
		if (regulator->dev_attr.attr.name == NULL)
			goto attr_name_err;

		regulator->dev_attr.attr.owner = THIS_MODULE;
		regulator->dev_attr.attr.mode = 0444;
		regulator->dev_attr.show = device_requested_uA_show;
		err = device_create_file(dev, &regulator->dev_attr);
		if (err < 0) {
			printk(KERN_WARNING "%s: could not add regulator_dev"
				" load sysfs\n", __func__);
			goto attr_name_err;
		}

		
		size = scnprintf(buf, REG_STR_SIZE, "%s-%s",
				 dev->kobj.name, supply_name);
		if (size >= REG_STR_SIZE)
			goto attr_err;

		regulator->supply_name = kstrdup(buf, GFP_KERNEL);
		if (regulator->supply_name == NULL)
			goto attr_err;

		err = sysfs_create_link(&rdev->dev.kobj, &dev->kobj,
					buf);
		if (err) {
			printk(KERN_WARNING
			       "%s: could not add device link %s err %d\n",
			       __func__, dev->kobj.name, err);
			device_remove_file(dev, &regulator->dev_attr);
			goto link_name_err;
		}
	}
	mutex_unlock(&rdev->mutex);
	return regulator;
link_name_err:
	kfree(regulator->supply_name);
attr_err:
	device_remove_file(regulator->dev, &regulator->dev_attr);
attr_name_err:
	kfree(regulator->dev_attr.attr.name);
overflow_err:
	list_del(&regulator->list);
	kfree(regulator);
	mutex_unlock(&rdev->mutex);
	return NULL;
}


static struct regulator *_regulator_get(struct device *dev, const char *id,
					int exclusive)
{
	struct regulator_dev *rdev;
	struct regulator_map *map;
	struct regulator *regulator = ERR_PTR(-ENODEV);
	const char *devname = NULL;
	int ret;

	if (id == NULL) {
		printk(KERN_ERR "regulator: get() with no identifier\n");
		return regulator;
	}

	if (dev)
		devname = dev_name(dev);

	mutex_lock(&regulator_list_mutex);

	list_for_each_entry(map, &regulator_map_list, list) {
		
		if (map->dev_name &&
		    (!devname || strcmp(map->dev_name, devname)))
			continue;

		if (strcmp(map->supply, id) == 0) {
			rdev = map->regulator;
			goto found;
		}
	}
	mutex_unlock(&regulator_list_mutex);
	return regulator;

found:
	if (rdev->exclusive) {
		regulator = ERR_PTR(-EPERM);
		goto out;
	}

	if (exclusive && rdev->open_count) {
		regulator = ERR_PTR(-EBUSY);
		goto out;
	}

	if (!try_module_get(rdev->owner))
		goto out;

	regulator = create_regulator(rdev, dev, id);
	if (regulator == NULL) {
		regulator = ERR_PTR(-ENOMEM);
		module_put(rdev->owner);
	}

	rdev->open_count++;
	if (exclusive) {
		rdev->exclusive = 1;

		ret = _regulator_is_enabled(rdev);
		if (ret > 0)
			rdev->use_count = 1;
		else
			rdev->use_count = 0;
	}

out:
	mutex_unlock(&regulator_list_mutex);

	return regulator;
}


struct regulator *regulator_get(struct device *dev, const char *id)
{
	return _regulator_get(dev, id, 0);
}
EXPORT_SYMBOL_GPL(regulator_get);


struct regulator *regulator_get_exclusive(struct device *dev, const char *id)
{
	return _regulator_get(dev, id, 1);
}
EXPORT_SYMBOL_GPL(regulator_get_exclusive);


void regulator_put(struct regulator *regulator)
{
	struct regulator_dev *rdev;

	if (regulator == NULL || IS_ERR(regulator))
		return;

	mutex_lock(&regulator_list_mutex);
	rdev = regulator->rdev;

	
	if (regulator->dev) {
		sysfs_remove_link(&rdev->dev.kobj, regulator->supply_name);
		kfree(regulator->supply_name);
		device_remove_file(regulator->dev, &regulator->dev_attr);
		kfree(regulator->dev_attr.attr.name);
	}
	list_del(&regulator->list);
	kfree(regulator);

	rdev->open_count--;
	rdev->exclusive = 0;

	module_put(rdev->owner);
	mutex_unlock(&regulator_list_mutex);
}
EXPORT_SYMBOL_GPL(regulator_put);

static int _regulator_can_change_status(struct regulator_dev *rdev)
{
	if (!rdev->constraints)
		return 0;

	if (rdev->constraints->valid_ops_mask & REGULATOR_CHANGE_STATUS)
		return 1;
	else
		return 0;
}


static int _regulator_enable(struct regulator_dev *rdev)
{
	int ret;

	
	if (rdev->supply) {
		ret = _regulator_enable(rdev->supply);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to enable %s: %d\n",
			       __func__, rdev->desc->name, ret);
			return ret;
		}
	}

	
	if (rdev->constraints &&
	    (rdev->constraints->valid_ops_mask & REGULATOR_CHANGE_DRMS))
		drms_uA_update(rdev);

	if (rdev->use_count == 0) {
		
		ret = _regulator_is_enabled(rdev);
		if (ret == -EINVAL || ret == 0) {
			if (!_regulator_can_change_status(rdev))
				return -EPERM;

			if (rdev->desc->ops->enable) {
				ret = rdev->desc->ops->enable(rdev);
				if (ret < 0)
					return ret;
			} else {
				return -EINVAL;
			}
		} else if (ret < 0) {
			printk(KERN_ERR "%s: is_enabled() failed for %s: %d\n",
			       __func__, rdev->desc->name, ret);
			return ret;
		}
		
	}

	rdev->use_count++;

	return 0;
}


int regulator_enable(struct regulator *regulator)
{
	struct regulator_dev *rdev = regulator->rdev;
	int ret = 0;

	mutex_lock(&rdev->mutex);
	ret = _regulator_enable(rdev);
	mutex_unlock(&rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_enable);


static int _regulator_disable(struct regulator_dev *rdev)
{
	int ret = 0;

	if (WARN(rdev->use_count <= 0,
			"unbalanced disables for %s\n",
			rdev->desc->name))
		return -EIO;

	
	if (rdev->use_count == 1 &&
	    (rdev->constraints && !rdev->constraints->always_on)) {

		
		if (_regulator_can_change_status(rdev) &&
		    rdev->desc->ops->disable) {
			ret = rdev->desc->ops->disable(rdev);
			if (ret < 0) {
				printk(KERN_ERR "%s: failed to disable %s\n",
				       __func__, rdev->desc->name);
				return ret;
			}
		}

		
		if (rdev->supply)
			_regulator_disable(rdev->supply);

		rdev->use_count = 0;
	} else if (rdev->use_count > 1) {

		if (rdev->constraints &&
			(rdev->constraints->valid_ops_mask &
			REGULATOR_CHANGE_DRMS))
			drms_uA_update(rdev);

		rdev->use_count--;
	}
	return ret;
}


int regulator_disable(struct regulator *regulator)
{
	struct regulator_dev *rdev = regulator->rdev;
	int ret = 0;

	mutex_lock(&rdev->mutex);
	ret = _regulator_disable(rdev);
	mutex_unlock(&rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_disable);


static int _regulator_force_disable(struct regulator_dev *rdev)
{
	int ret = 0;

	
	if (rdev->desc->ops->disable) {
		
		ret = rdev->desc->ops->disable(rdev);
		if (ret < 0) {
			printk(KERN_ERR "%s: failed to force disable %s\n",
			       __func__, rdev->desc->name);
			return ret;
		}
		
		_notifier_call_chain(rdev, REGULATOR_EVENT_FORCE_DISABLE,
			NULL);
	}

	
	if (rdev->supply)
		_regulator_disable(rdev->supply);

	rdev->use_count = 0;
	return ret;
}


int regulator_force_disable(struct regulator *regulator)
{
	int ret;

	mutex_lock(&regulator->rdev->mutex);
	regulator->uA_load = 0;
	ret = _regulator_force_disable(regulator->rdev);
	mutex_unlock(&regulator->rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_force_disable);

static int _regulator_is_enabled(struct regulator_dev *rdev)
{
	
	if (!rdev->desc->ops->is_enabled)
		return -EINVAL;

	return rdev->desc->ops->is_enabled(rdev);
}


int regulator_is_enabled(struct regulator *regulator)
{
	int ret;

	mutex_lock(&regulator->rdev->mutex);
	ret = _regulator_is_enabled(regulator->rdev);
	mutex_unlock(&regulator->rdev->mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(regulator_is_enabled);


int regulator_count_voltages(struct regulator *regulator)
{
	struct regulator_dev	*rdev = regulator->rdev;

	return rdev->desc->n_voltages ? : -EINVAL;
}
EXPORT_SYMBOL_GPL(regulator_count_voltages);


int regulator_list_voltage(struct regulator *regulator, unsigned selector)
{
	struct regulator_dev	*rdev = regulator->rdev;
	struct regulator_ops	*ops = rdev->desc->ops;
	int			ret;

	if (!ops->list_voltage || selector >= rdev->desc->n_voltages)
		return -EINVAL;

	mutex_lock(&rdev->mutex);
	ret = ops->list_voltage(rdev, selector);
	mutex_unlock(&rdev->mutex);

	if (ret > 0) {
		if (ret < rdev->constraints->min_uV)
			ret = 0;
		else if (ret > rdev->constraints->max_uV)
			ret = 0;
	}

	return ret;
}
EXPORT_SYMBOL_GPL(regulator_list_voltage);


int regulator_is_supported_voltage(struct regulator *regulator,
				   int min_uV, int max_uV)
{
	int i, voltages, ret;

	ret = regulator_count_voltages(regulator);
	if (ret < 0)
		return ret;
	voltages = ret;

	for (i = 0; i < voltages; i++) {
		ret = regulator_list_voltage(regulator, i);

		if (ret >= min_uV && ret <= max_uV)
			return 1;
	}

	return 0;
}


int regulator_set_voltage(struct regulator *regulator, int min_uV, int max_uV)
{
	struct regulator_dev *rdev = regulator->rdev;
	int ret;

	mutex_lock(&rdev->mutex);

	
	if (!rdev->desc->ops->set_voltage) {
		ret = -EINVAL;
		goto out;
	}

	
	ret = regulator_check_voltage(rdev, &min_uV, &max_uV);
	if (ret < 0)
		goto out;
	regulator->min_uV = min_uV;
	regulator->max_uV = max_uV;
	ret = rdev->desc->ops->set_voltage(rdev, min_uV, max_uV);

out:
	_notifier_call_chain(rdev, REGULATOR_EVENT_VOLTAGE_CHANGE, NULL);
	mutex_unlock(&rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_set_voltage);

static int _regulator_get_voltage(struct regulator_dev *rdev)
{
	
	if (rdev->desc->ops->get_voltage)
		return rdev->desc->ops->get_voltage(rdev);
	else
		return -EINVAL;
}


int regulator_get_voltage(struct regulator *regulator)
{
	int ret;

	mutex_lock(&regulator->rdev->mutex);

	ret = _regulator_get_voltage(regulator->rdev);

	mutex_unlock(&regulator->rdev->mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(regulator_get_voltage);


int regulator_set_current_limit(struct regulator *regulator,
			       int min_uA, int max_uA)
{
	struct regulator_dev *rdev = regulator->rdev;
	int ret;

	mutex_lock(&rdev->mutex);

	
	if (!rdev->desc->ops->set_current_limit) {
		ret = -EINVAL;
		goto out;
	}

	
	ret = regulator_check_current_limit(rdev, &min_uA, &max_uA);
	if (ret < 0)
		goto out;

	ret = rdev->desc->ops->set_current_limit(rdev, min_uA, max_uA);
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_set_current_limit);

static int _regulator_get_current_limit(struct regulator_dev *rdev)
{
	int ret;

	mutex_lock(&rdev->mutex);

	
	if (!rdev->desc->ops->get_current_limit) {
		ret = -EINVAL;
		goto out;
	}

	ret = rdev->desc->ops->get_current_limit(rdev);
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}


int regulator_get_current_limit(struct regulator *regulator)
{
	return _regulator_get_current_limit(regulator->rdev);
}
EXPORT_SYMBOL_GPL(regulator_get_current_limit);


int regulator_set_mode(struct regulator *regulator, unsigned int mode)
{
	struct regulator_dev *rdev = regulator->rdev;
	int ret;

	mutex_lock(&rdev->mutex);

	
	if (!rdev->desc->ops->set_mode) {
		ret = -EINVAL;
		goto out;
	}

	
	ret = regulator_check_mode(rdev, mode);
	if (ret < 0)
		goto out;

	ret = rdev->desc->ops->set_mode(rdev, mode);
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_set_mode);

static unsigned int _regulator_get_mode(struct regulator_dev *rdev)
{
	int ret;

	mutex_lock(&rdev->mutex);

	
	if (!rdev->desc->ops->get_mode) {
		ret = -EINVAL;
		goto out;
	}

	ret = rdev->desc->ops->get_mode(rdev);
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}


unsigned int regulator_get_mode(struct regulator *regulator)
{
	return _regulator_get_mode(regulator->rdev);
}
EXPORT_SYMBOL_GPL(regulator_get_mode);


int regulator_set_optimum_mode(struct regulator *regulator, int uA_load)
{
	struct regulator_dev *rdev = regulator->rdev;
	struct regulator *consumer;
	int ret, output_uV, input_uV, total_uA_load = 0;
	unsigned int mode;

	mutex_lock(&rdev->mutex);

	regulator->uA_load = uA_load;
	ret = regulator_check_drms(rdev);
	if (ret < 0)
		goto out;
	ret = -EINVAL;

	
	if (!rdev->desc->ops->get_optimum_mode)
		goto out;

	
	output_uV = rdev->desc->ops->get_voltage(rdev);
	if (output_uV <= 0) {
		printk(KERN_ERR "%s: invalid output voltage found for %s\n",
			__func__, rdev->desc->name);
		goto out;
	}

	
	if (rdev->supply && rdev->supply->desc->ops->get_voltage)
		input_uV = rdev->supply->desc->ops->get_voltage(rdev->supply);
	else
		input_uV = rdev->constraints->input_uV;
	if (input_uV <= 0) {
		printk(KERN_ERR "%s: invalid input voltage found for %s\n",
			__func__, rdev->desc->name);
		goto out;
	}

	
	list_for_each_entry(consumer, &rdev->consumer_list, list)
	    total_uA_load += consumer->uA_load;

	mode = rdev->desc->ops->get_optimum_mode(rdev,
						 input_uV, output_uV,
						 total_uA_load);
	ret = regulator_check_mode(rdev, mode);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to get optimum mode for %s @"
			" %d uA %d -> %d uV\n", __func__, rdev->desc->name,
			total_uA_load, input_uV, output_uV);
		goto out;
	}

	ret = rdev->desc->ops->set_mode(rdev, mode);
	if (ret < 0) {
		printk(KERN_ERR "%s: failed to set optimum mode %x for %s\n",
			__func__, mode, rdev->desc->name);
		goto out;
	}
	ret = mode;
out:
	mutex_unlock(&rdev->mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_set_optimum_mode);


int regulator_register_notifier(struct regulator *regulator,
			      struct notifier_block *nb)
{
	return blocking_notifier_chain_register(&regulator->rdev->notifier,
						nb);
}
EXPORT_SYMBOL_GPL(regulator_register_notifier);


int regulator_unregister_notifier(struct regulator *regulator,
				struct notifier_block *nb)
{
	return blocking_notifier_chain_unregister(&regulator->rdev->notifier,
						  nb);
}
EXPORT_SYMBOL_GPL(regulator_unregister_notifier);


static void _notifier_call_chain(struct regulator_dev *rdev,
				  unsigned long event, void *data)
{
	struct regulator_dev *_rdev;

	
	blocking_notifier_call_chain(&rdev->notifier, event, NULL);

	
	list_for_each_entry(_rdev, &rdev->supply_list, slist) {
	  mutex_lock(&_rdev->mutex);
	  _notifier_call_chain(_rdev, event, data);
	  mutex_unlock(&_rdev->mutex);
	}
}


int regulator_bulk_get(struct device *dev, int num_consumers,
		       struct regulator_bulk_data *consumers)
{
	int i;
	int ret;

	for (i = 0; i < num_consumers; i++)
		consumers[i].consumer = NULL;

	for (i = 0; i < num_consumers; i++) {
		consumers[i].consumer = regulator_get(dev,
						      consumers[i].supply);
		if (IS_ERR(consumers[i].consumer)) {
			dev_err(dev, "Failed to get supply '%s'\n",
				consumers[i].supply);
			ret = PTR_ERR(consumers[i].consumer);
			consumers[i].consumer = NULL;
			goto err;
		}
	}

	return 0;

err:
	for (i = 0; i < num_consumers && consumers[i].consumer; i++)
		regulator_put(consumers[i].consumer);

	return ret;
}
EXPORT_SYMBOL_GPL(regulator_bulk_get);


int regulator_bulk_enable(int num_consumers,
			  struct regulator_bulk_data *consumers)
{
	int i;
	int ret;

	for (i = 0; i < num_consumers; i++) {
		ret = regulator_enable(consumers[i].consumer);
		if (ret != 0)
			goto err;
	}

	return 0;

err:
	printk(KERN_ERR "Failed to enable %s\n", consumers[i].supply);
	for (i = 0; i < num_consumers; i++)
		regulator_disable(consumers[i].consumer);

	return ret;
}
EXPORT_SYMBOL_GPL(regulator_bulk_enable);


int regulator_bulk_disable(int num_consumers,
			   struct regulator_bulk_data *consumers)
{
	int i;
	int ret;

	for (i = 0; i < num_consumers; i++) {
		ret = regulator_disable(consumers[i].consumer);
		if (ret != 0)
			goto err;
	}

	return 0;

err:
	printk(KERN_ERR "Failed to disable %s\n", consumers[i].supply);
	for (i = 0; i < num_consumers; i++)
		regulator_enable(consumers[i].consumer);

	return ret;
}
EXPORT_SYMBOL_GPL(regulator_bulk_disable);


void regulator_bulk_free(int num_consumers,
			 struct regulator_bulk_data *consumers)
{
	int i;

	for (i = 0; i < num_consumers; i++) {
		regulator_put(consumers[i].consumer);
		consumers[i].consumer = NULL;
	}
}
EXPORT_SYMBOL_GPL(regulator_bulk_free);


int regulator_notifier_call_chain(struct regulator_dev *rdev,
				  unsigned long event, void *data)
{
	_notifier_call_chain(rdev, event, data);
	return NOTIFY_DONE;

}
EXPORT_SYMBOL_GPL(regulator_notifier_call_chain);


int regulator_mode_to_status(unsigned int mode)
{
	switch (mode) {
	case REGULATOR_MODE_FAST:
		return REGULATOR_STATUS_FAST;
	case REGULATOR_MODE_NORMAL:
		return REGULATOR_STATUS_NORMAL;
	case REGULATOR_MODE_IDLE:
		return REGULATOR_STATUS_IDLE;
	case REGULATOR_STATUS_STANDBY:
		return REGULATOR_STATUS_STANDBY;
	default:
		return 0;
	}
}
EXPORT_SYMBOL_GPL(regulator_mode_to_status);


static int add_regulator_attributes(struct regulator_dev *rdev)
{
	struct device		*dev = &rdev->dev;
	struct regulator_ops	*ops = rdev->desc->ops;
	int			status = 0;

	
	if (ops->get_voltage) {
		status = device_create_file(dev, &dev_attr_microvolts);
		if (status < 0)
			return status;
	}
	if (ops->get_current_limit) {
		status = device_create_file(dev, &dev_attr_microamps);
		if (status < 0)
			return status;
	}
	if (ops->get_mode) {
		status = device_create_file(dev, &dev_attr_opmode);
		if (status < 0)
			return status;
	}
	if (ops->is_enabled) {
		status = device_create_file(dev, &dev_attr_state);
		if (status < 0)
			return status;
	}
	if (ops->get_status) {
		status = device_create_file(dev, &dev_attr_status);
		if (status < 0)
			return status;
	}

	
	if (rdev->desc->type == REGULATOR_CURRENT) {
		status = device_create_file(dev, &dev_attr_requested_microamps);
		if (status < 0)
			return status;
	}

	
	if (!rdev->constraints)
		return status;

	
	if (ops->set_voltage) {
		status = device_create_file(dev, &dev_attr_min_microvolts);
		if (status < 0)
			return status;
		status = device_create_file(dev, &dev_attr_max_microvolts);
		if (status < 0)
			return status;
	}
	if (ops->set_current_limit) {
		status = device_create_file(dev, &dev_attr_min_microamps);
		if (status < 0)
			return status;
		status = device_create_file(dev, &dev_attr_max_microamps);
		if (status < 0)
			return status;
	}

	
	if (!(ops->set_suspend_enable && ops->set_suspend_disable))
		return status;

	status = device_create_file(dev, &dev_attr_suspend_standby_state);
	if (status < 0)
		return status;
	status = device_create_file(dev, &dev_attr_suspend_mem_state);
	if (status < 0)
		return status;
	status = device_create_file(dev, &dev_attr_suspend_disk_state);
	if (status < 0)
		return status;

	if (ops->set_suspend_voltage) {
		status = device_create_file(dev,
				&dev_attr_suspend_standby_microvolts);
		if (status < 0)
			return status;
		status = device_create_file(dev,
				&dev_attr_suspend_mem_microvolts);
		if (status < 0)
			return status;
		status = device_create_file(dev,
				&dev_attr_suspend_disk_microvolts);
		if (status < 0)
			return status;
	}

	if (ops->set_suspend_mode) {
		status = device_create_file(dev,
				&dev_attr_suspend_standby_mode);
		if (status < 0)
			return status;
		status = device_create_file(dev,
				&dev_attr_suspend_mem_mode);
		if (status < 0)
			return status;
		status = device_create_file(dev,
				&dev_attr_suspend_disk_mode);
		if (status < 0)
			return status;
	}

	return status;
}


struct regulator_dev *regulator_register(struct regulator_desc *regulator_desc,
	struct device *dev, struct regulator_init_data *init_data,
	void *driver_data)
{
	static atomic_t regulator_no = ATOMIC_INIT(0);
	struct regulator_dev *rdev;
	int ret, i;

	if (regulator_desc == NULL)
		return ERR_PTR(-EINVAL);

	if (regulator_desc->name == NULL || regulator_desc->ops == NULL)
		return ERR_PTR(-EINVAL);

	if (regulator_desc->type != REGULATOR_VOLTAGE &&
	    regulator_desc->type != REGULATOR_CURRENT)
		return ERR_PTR(-EINVAL);

	if (!init_data)
		return ERR_PTR(-EINVAL);

	rdev = kzalloc(sizeof(struct regulator_dev), GFP_KERNEL);
	if (rdev == NULL)
		return ERR_PTR(-ENOMEM);

	mutex_lock(&regulator_list_mutex);

	mutex_init(&rdev->mutex);
	rdev->reg_data = driver_data;
	rdev->owner = regulator_desc->owner;
	rdev->desc = regulator_desc;
	INIT_LIST_HEAD(&rdev->consumer_list);
	INIT_LIST_HEAD(&rdev->supply_list);
	INIT_LIST_HEAD(&rdev->list);
	INIT_LIST_HEAD(&rdev->slist);
	BLOCKING_INIT_NOTIFIER_HEAD(&rdev->notifier);

	
	if (init_data->regulator_init) {
		ret = init_data->regulator_init(rdev->reg_data);
		if (ret < 0)
			goto clean;
	}

	
	rdev->dev.class = &regulator_class;
	rdev->dev.parent = dev;
	dev_set_name(&rdev->dev, "regulator.%d",
		     atomic_inc_return(&regulator_no) - 1);
	ret = device_register(&rdev->dev);
	if (ret != 0)
		goto clean;

	dev_set_drvdata(&rdev->dev, rdev);

	
	ret = set_machine_constraints(rdev, &init_data->constraints);
	if (ret < 0)
		goto scrub;

	
	ret = add_regulator_attributes(rdev);
	if (ret < 0)
		goto scrub;

	
	if (init_data->supply_regulator_dev) {
		ret = set_supply(rdev,
			dev_get_drvdata(init_data->supply_regulator_dev));
		if (ret < 0)
			goto scrub;
	}

	
	for (i = 0; i < init_data->num_consumer_supplies; i++) {
		ret = set_consumer_device_supply(rdev,
			init_data->consumer_supplies[i].dev,
			init_data->consumer_supplies[i].dev_name,
			init_data->consumer_supplies[i].supply);
		if (ret < 0) {
			for (--i; i >= 0; i--)
				unset_consumer_device_supply(rdev,
				    init_data->consumer_supplies[i].dev_name,
				    init_data->consumer_supplies[i].dev);
			goto scrub;
		}
	}

	list_add(&rdev->list, &regulator_list);
out:
	mutex_unlock(&regulator_list_mutex);
	return rdev;

scrub:
	device_unregister(&rdev->dev);
	
	rdev = ERR_PTR(ret);
	goto out;

clean:
	kfree(rdev);
	rdev = ERR_PTR(ret);
	goto out;
}
EXPORT_SYMBOL_GPL(regulator_register);


void regulator_unregister(struct regulator_dev *rdev)
{
	if (rdev == NULL)
		return;

	mutex_lock(&regulator_list_mutex);
	WARN_ON(rdev->open_count);
	unset_regulator_supplies(rdev);
	list_del(&rdev->list);
	if (rdev->supply)
		sysfs_remove_link(&rdev->dev.kobj, "supply");
	device_unregister(&rdev->dev);
	mutex_unlock(&regulator_list_mutex);
}
EXPORT_SYMBOL_GPL(regulator_unregister);


int regulator_suspend_prepare(suspend_state_t state)
{
	struct regulator_dev *rdev;
	int ret = 0;

	
	if (state == PM_SUSPEND_ON)
		return -EINVAL;

	mutex_lock(&regulator_list_mutex);
	list_for_each_entry(rdev, &regulator_list, list) {

		mutex_lock(&rdev->mutex);
		ret = suspend_prepare(rdev, state);
		mutex_unlock(&rdev->mutex);

		if (ret < 0) {
			printk(KERN_ERR "%s: failed to prepare %s\n",
				__func__, rdev->desc->name);
			goto out;
		}
	}
out:
	mutex_unlock(&regulator_list_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(regulator_suspend_prepare);


void regulator_has_full_constraints(void)
{
	has_full_constraints = 1;
}
EXPORT_SYMBOL_GPL(regulator_has_full_constraints);


void *rdev_get_drvdata(struct regulator_dev *rdev)
{
	return rdev->reg_data;
}
EXPORT_SYMBOL_GPL(rdev_get_drvdata);


void *regulator_get_drvdata(struct regulator *regulator)
{
	return regulator->rdev->reg_data;
}
EXPORT_SYMBOL_GPL(regulator_get_drvdata);


void regulator_set_drvdata(struct regulator *regulator, void *data)
{
	regulator->rdev->reg_data = data;
}
EXPORT_SYMBOL_GPL(regulator_set_drvdata);


int rdev_get_id(struct regulator_dev *rdev)
{
	return rdev->desc->id;
}
EXPORT_SYMBOL_GPL(rdev_get_id);

struct device *rdev_get_dev(struct regulator_dev *rdev)
{
	return &rdev->dev;
}
EXPORT_SYMBOL_GPL(rdev_get_dev);

void *regulator_get_init_drvdata(struct regulator_init_data *reg_init_data)
{
	return reg_init_data->driver_data;
}
EXPORT_SYMBOL_GPL(regulator_get_init_drvdata);

static int __init regulator_init(void)
{
	printk(KERN_INFO "regulator: core version %s\n", REGULATOR_VERSION);
	return class_register(&regulator_class);
}


core_initcall(regulator_init);

static int __init regulator_init_complete(void)
{
	struct regulator_dev *rdev;
	struct regulator_ops *ops;
	struct regulation_constraints *c;
	int enabled, ret;
	const char *name;

	mutex_lock(&regulator_list_mutex);

	
	list_for_each_entry(rdev, &regulator_list, list) {
		ops = rdev->desc->ops;
		c = rdev->constraints;

		if (c && c->name)
			name = c->name;
		else if (rdev->desc->name)
			name = rdev->desc->name;
		else
			name = "regulator";

		if (!ops->disable || (c && c->always_on))
			continue;

		mutex_lock(&rdev->mutex);

		if (rdev->use_count)
			goto unlock;

		
		if (ops->is_enabled)
			enabled = ops->is_enabled(rdev);
		else
			enabled = 1;

		if (!enabled)
			goto unlock;

		if (has_full_constraints) {
			
			printk(KERN_INFO "%s: disabling %s\n",
			       __func__, name);
			ret = ops->disable(rdev);
			if (ret != 0) {
				printk(KERN_ERR
				       "%s: couldn't disable %s: %d\n",
				       __func__, name, ret);
			}
		} else {
			
			printk(KERN_WARNING
			       "%s: incomplete constraints, leaving %s on\n",
			       __func__, name);
		}

unlock:
		mutex_unlock(&rdev->mutex);
	}

	mutex_unlock(&regulator_list_mutex);

	return 0;
}
late_initcall(regulator_init_complete);
