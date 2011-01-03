

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/device.h>

#include "i1480u-wlp.h"



ssize_t uwb_phy_rate_show(const struct wlp_options *options, char *buf)
{
	return sprintf(buf, "%u\n",
		       wlp_tx_hdr_phy_rate(&options->def_tx_hdr));
}
EXPORT_SYMBOL_GPL(uwb_phy_rate_show);


ssize_t uwb_phy_rate_store(struct wlp_options *options,
			   const char *buf, size_t size)
{
	ssize_t result;
	unsigned rate;

	result = sscanf(buf, "%u\n", &rate);
	if (result != 1) {
		result = -EINVAL;
		goto out;
	}
	result = -EINVAL;
	if (rate >= UWB_PHY_RATE_INVALID)
		goto out;
	wlp_tx_hdr_set_phy_rate(&options->def_tx_hdr, rate);
	result = 0;
out:
	return result < 0 ? result : size;
}
EXPORT_SYMBOL_GPL(uwb_phy_rate_store);


ssize_t uwb_rts_cts_show(const struct wlp_options *options, char *buf)
{
	return sprintf(buf, "%u\n",
		       wlp_tx_hdr_rts_cts(&options->def_tx_hdr));
}
EXPORT_SYMBOL_GPL(uwb_rts_cts_show);


ssize_t uwb_rts_cts_store(struct wlp_options *options,
			  const char *buf, size_t size)
{
	ssize_t result;
	unsigned value;

	result = sscanf(buf, "%u\n", &value);
	if (result != 1) {
		result = -EINVAL;
		goto out;
	}
	result = -EINVAL;
	wlp_tx_hdr_set_rts_cts(&options->def_tx_hdr, !!value);
	result = 0;
out:
	return result < 0 ? result : size;
}
EXPORT_SYMBOL_GPL(uwb_rts_cts_store);


ssize_t uwb_ack_policy_show(const struct wlp_options *options, char *buf)
{
	return sprintf(buf, "%u\n",
		       wlp_tx_hdr_ack_policy(&options->def_tx_hdr));
}
EXPORT_SYMBOL_GPL(uwb_ack_policy_show);


ssize_t uwb_ack_policy_store(struct wlp_options *options,
			     const char *buf, size_t size)
{
	ssize_t result;
	unsigned value;

	result = sscanf(buf, "%u\n", &value);
	if (result != 1 || value > UWB_ACK_B_REQ) {
		result = -EINVAL;
		goto out;
	}
	wlp_tx_hdr_set_ack_policy(&options->def_tx_hdr, value);
	result = 0;
out:
	return result < 0 ? result : size;
}
EXPORT_SYMBOL_GPL(uwb_ack_policy_store);



ssize_t uwb_pca_base_priority_show(const struct wlp_options *options,
				   char *buf)
{
	return sprintf(buf, "%u\n",
		       options->pca_base_priority);
}
EXPORT_SYMBOL_GPL(uwb_pca_base_priority_show);



ssize_t uwb_pca_base_priority_store(struct wlp_options *options,
				    const char *buf, size_t size)
{
	ssize_t result = -EINVAL;
	u8 pca_base_priority;

	result = sscanf(buf, "%hhu\n", &pca_base_priority);
	if (result != 1) {
		result = -EINVAL;
		goto out;
	}
	result = -EINVAL;
	if (pca_base_priority >= 8)
		goto out;
	options->pca_base_priority = pca_base_priority;
	
	if (result >= 0 && (wlp_tx_hdr_delivery_id_type(&options->def_tx_hdr) & WLP_DRP) == 0)
		wlp_tx_hdr_set_delivery_id_type(&options->def_tx_hdr, options->pca_base_priority);
	result = 0;
out:
	return result < 0 ? result : size;
}
EXPORT_SYMBOL_GPL(uwb_pca_base_priority_store);


static ssize_t wlp_tx_inflight_show(struct i1480u_tx_inflight *inflight,
				    char *buf)
{
	ssize_t result;
	unsigned long sec_elapsed = (jiffies - inflight->restart_ts)/HZ;
	unsigned long restart_count = atomic_read(&inflight->restart_count);

	result = scnprintf(buf, PAGE_SIZE, "%lu %lu %d %lu %lu %lu\n"
			   "#read: threshold max inflight_count restarts "
			   "seconds restarts/sec\n"
			   "#write: threshold max\n",
			   inflight->threshold, inflight->max,
			   atomic_read(&inflight->count),
			   restart_count, sec_elapsed,
			   sec_elapsed == 0 ? 0 : restart_count/sec_elapsed);
	inflight->restart_ts = jiffies;
	atomic_set(&inflight->restart_count, 0);
	return result;
}

static
ssize_t wlp_tx_inflight_store(struct i1480u_tx_inflight *inflight,
				const char *buf, size_t size)
{
	unsigned long in_threshold, in_max;
	ssize_t result;
	result = sscanf(buf, "%lu %lu", &in_threshold, &in_max);
	if (result != 2)
		return -EINVAL;
	if (in_max <= in_threshold)
		return -EINVAL;
	inflight->max = in_max;
	inflight->threshold = in_threshold;
	return size;
}


#define i1480u_SHOW(name, fn, param)				\
static ssize_t i1480u_show_##name(struct device *dev,		\
				  struct device_attribute *attr,\
				  char *buf)			\
{								\
	struct i1480u *i1480u = netdev_priv(to_net_dev(dev));	\
	return fn(&i1480u->param, buf);				\
}

#define i1480u_STORE(name, fn, param)				\
static ssize_t i1480u_store_##name(struct device *dev,		\
				   struct device_attribute *attr,\
				   const char *buf, size_t size)\
{								\
	struct i1480u *i1480u = netdev_priv(to_net_dev(dev));	\
	return fn(&i1480u->param, buf, size);			\
}

#define i1480u_ATTR(name, perm) static DEVICE_ATTR(name, perm,  \
					     i1480u_show_##name,\
					     i1480u_store_##name)

#define i1480u_ATTR_SHOW(name) static DEVICE_ATTR(name,		\
					S_IRUGO,		\
					i1480u_show_##name, NULL)

#define i1480u_ATTR_NAME(a) (dev_attr_##a)



i1480u_SHOW(uwb_phy_rate, uwb_phy_rate_show, options);
i1480u_STORE(uwb_phy_rate, uwb_phy_rate_store, options);
i1480u_ATTR(uwb_phy_rate, S_IRUGO | S_IWUSR);

i1480u_SHOW(uwb_rts_cts, uwb_rts_cts_show, options);
i1480u_STORE(uwb_rts_cts, uwb_rts_cts_store, options);
i1480u_ATTR(uwb_rts_cts, S_IRUGO | S_IWUSR);

i1480u_SHOW(uwb_ack_policy, uwb_ack_policy_show, options);
i1480u_STORE(uwb_ack_policy, uwb_ack_policy_store, options);
i1480u_ATTR(uwb_ack_policy, S_IRUGO | S_IWUSR);

i1480u_SHOW(uwb_pca_base_priority, uwb_pca_base_priority_show, options);
i1480u_STORE(uwb_pca_base_priority, uwb_pca_base_priority_store, options);
i1480u_ATTR(uwb_pca_base_priority, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_eda, wlp_eda_show, wlp);
i1480u_STORE(wlp_eda, wlp_eda_store, wlp);
i1480u_ATTR(wlp_eda, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_uuid, wlp_uuid_show, wlp);
i1480u_STORE(wlp_uuid, wlp_uuid_store, wlp);
i1480u_ATTR(wlp_uuid, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_dev_name, wlp_dev_name_show, wlp);
i1480u_STORE(wlp_dev_name, wlp_dev_name_store, wlp);
i1480u_ATTR(wlp_dev_name, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_dev_manufacturer, wlp_dev_manufacturer_show, wlp);
i1480u_STORE(wlp_dev_manufacturer, wlp_dev_manufacturer_store, wlp);
i1480u_ATTR(wlp_dev_manufacturer, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_dev_model_name, wlp_dev_model_name_show, wlp);
i1480u_STORE(wlp_dev_model_name, wlp_dev_model_name_store, wlp);
i1480u_ATTR(wlp_dev_model_name, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_dev_model_nr, wlp_dev_model_nr_show, wlp);
i1480u_STORE(wlp_dev_model_nr, wlp_dev_model_nr_store, wlp);
i1480u_ATTR(wlp_dev_model_nr, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_dev_serial, wlp_dev_serial_show, wlp);
i1480u_STORE(wlp_dev_serial, wlp_dev_serial_store, wlp);
i1480u_ATTR(wlp_dev_serial, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_dev_prim_category, wlp_dev_prim_category_show, wlp);
i1480u_STORE(wlp_dev_prim_category, wlp_dev_prim_category_store, wlp);
i1480u_ATTR(wlp_dev_prim_category, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_dev_prim_OUI, wlp_dev_prim_OUI_show, wlp);
i1480u_STORE(wlp_dev_prim_OUI, wlp_dev_prim_OUI_store, wlp);
i1480u_ATTR(wlp_dev_prim_OUI, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_dev_prim_OUI_sub, wlp_dev_prim_OUI_sub_show, wlp);
i1480u_STORE(wlp_dev_prim_OUI_sub, wlp_dev_prim_OUI_sub_store, wlp);
i1480u_ATTR(wlp_dev_prim_OUI_sub, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_dev_prim_subcat, wlp_dev_prim_subcat_show, wlp);
i1480u_STORE(wlp_dev_prim_subcat, wlp_dev_prim_subcat_store, wlp);
i1480u_ATTR(wlp_dev_prim_subcat, S_IRUGO | S_IWUSR);

i1480u_SHOW(wlp_neighborhood, wlp_neighborhood_show, wlp);
i1480u_ATTR_SHOW(wlp_neighborhood);

i1480u_SHOW(wss_activate, wlp_wss_activate_show, wlp.wss);
i1480u_STORE(wss_activate, wlp_wss_activate_store, wlp.wss);
i1480u_ATTR(wss_activate, S_IRUGO | S_IWUSR);


i1480u_SHOW(wlp_lqe, stats_show, lqe_stats);
i1480u_STORE(wlp_lqe, stats_store, lqe_stats);
i1480u_ATTR(wlp_lqe, S_IRUGO | S_IWUSR);


i1480u_SHOW(wlp_rssi, stats_show, rssi_stats);
i1480u_STORE(wlp_rssi, stats_store, rssi_stats);
i1480u_ATTR(wlp_rssi, S_IRUGO | S_IWUSR);


i1480u_SHOW(wlp_tx_inflight, wlp_tx_inflight_show, tx_inflight);
i1480u_STORE(wlp_tx_inflight, wlp_tx_inflight_store, tx_inflight);
i1480u_ATTR(wlp_tx_inflight, S_IRUGO | S_IWUSR);

static struct attribute *i1480u_attrs[] = {
	&i1480u_ATTR_NAME(uwb_phy_rate).attr,
	&i1480u_ATTR_NAME(uwb_rts_cts).attr,
	&i1480u_ATTR_NAME(uwb_ack_policy).attr,
	&i1480u_ATTR_NAME(uwb_pca_base_priority).attr,
	&i1480u_ATTR_NAME(wlp_lqe).attr,
	&i1480u_ATTR_NAME(wlp_rssi).attr,
	&i1480u_ATTR_NAME(wlp_eda).attr,
	&i1480u_ATTR_NAME(wlp_uuid).attr,
	&i1480u_ATTR_NAME(wlp_dev_name).attr,
	&i1480u_ATTR_NAME(wlp_dev_manufacturer).attr,
	&i1480u_ATTR_NAME(wlp_dev_model_name).attr,
	&i1480u_ATTR_NAME(wlp_dev_model_nr).attr,
	&i1480u_ATTR_NAME(wlp_dev_serial).attr,
	&i1480u_ATTR_NAME(wlp_dev_prim_category).attr,
	&i1480u_ATTR_NAME(wlp_dev_prim_OUI).attr,
	&i1480u_ATTR_NAME(wlp_dev_prim_OUI_sub).attr,
	&i1480u_ATTR_NAME(wlp_dev_prim_subcat).attr,
	&i1480u_ATTR_NAME(wlp_neighborhood).attr,
	&i1480u_ATTR_NAME(wss_activate).attr,
	&i1480u_ATTR_NAME(wlp_tx_inflight).attr,
	NULL,
};

static struct attribute_group i1480u_attr_group = {
	.name = NULL,	
	.attrs = i1480u_attrs,
};

int i1480u_sysfs_setup(struct i1480u *i1480u)
{
	int result;
	struct device *dev = &i1480u->usb_iface->dev;
	result = sysfs_create_group(&i1480u->net_dev->dev.kobj,
				    &i1480u_attr_group);
	if (result < 0)
		dev_err(dev, "cannot initialize sysfs attributes: %d\n",
			result);
	return result;
}


void i1480u_sysfs_release(struct i1480u *i1480u)
{
	sysfs_remove_group(&i1480u->net_dev->dev.kobj,
			   &i1480u_attr_group);
}
