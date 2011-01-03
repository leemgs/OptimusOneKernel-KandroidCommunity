#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/slab.h>

#include <asm/errno.h>


const struct of_device_id *of_match_device(const struct of_device_id *matches,
					const struct of_device *dev)
{
	if (!dev->node)
		return NULL;
	return of_match_node(matches, dev->node);
}
EXPORT_SYMBOL(of_match_device);

struct of_device *of_dev_get(struct of_device *dev)
{
	struct device *tmp;

	if (!dev)
		return NULL;
	tmp = get_device(&dev->dev);
	if (tmp)
		return to_of_device(tmp);
	else
		return NULL;
}
EXPORT_SYMBOL(of_dev_get);

void of_dev_put(struct of_device *dev)
{
	if (dev)
		put_device(&dev->dev);
}
EXPORT_SYMBOL(of_dev_put);

static ssize_t devspec_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct of_device *ofdev;

	ofdev = to_of_device(dev);
	return sprintf(buf, "%s\n", ofdev->node->full_name);
}

static ssize_t name_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct of_device *ofdev;

	ofdev = to_of_device(dev);
	return sprintf(buf, "%s\n", ofdev->node->name);
}

static ssize_t modalias_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct of_device *ofdev = to_of_device(dev);
	ssize_t len = 0;

	len = of_device_get_modalias(ofdev, buf, PAGE_SIZE - 2);
	buf[len] = '\n';
	buf[len+1] = 0;
	return len+1;
}

struct device_attribute of_platform_device_attrs[] = {
	__ATTR_RO(devspec),
	__ATTR_RO(name),
	__ATTR_RO(modalias),
	__ATTR_NULL
};


void of_release_dev(struct device *dev)
{
	struct of_device *ofdev;

	ofdev = to_of_device(dev);
	of_node_put(ofdev->node);
	kfree(ofdev);
}
EXPORT_SYMBOL(of_release_dev);

int of_device_register(struct of_device *ofdev)
{
	BUG_ON(ofdev->node == NULL);

	device_initialize(&ofdev->dev);

	
	if (!ofdev->dev.parent)
		set_dev_node(&ofdev->dev, of_node_to_nid(ofdev->node));

	return device_add(&ofdev->dev);
}
EXPORT_SYMBOL(of_device_register);

void of_device_unregister(struct of_device *ofdev)
{
	device_unregister(&ofdev->dev);
}
EXPORT_SYMBOL(of_device_unregister);

ssize_t of_device_get_modalias(struct of_device *ofdev,
				char *str, ssize_t len)
{
	const char *compat;
	int cplen, i;
	ssize_t tsize, csize, repend;

	
	csize = snprintf(str, len, "of:N%sT%s",
				ofdev->node->name, ofdev->node->type);

	
	compat = of_get_property(ofdev->node, "compatible", &cplen);
	if (!compat)
		return csize;

	
	for (i = (cplen - 1); i >= 0 && !compat[i]; i--)
		cplen--;
	if (!cplen)
		return csize;
	cplen++;

	
	tsize = csize + cplen;
	repend = tsize;

	if (csize >= len)		
		return tsize;

	if (tsize >= len) {		
		cplen = len - csize - 1;
		repend = len;
	}

	
	memcpy(&str[csize + 1], compat, cplen);
	for (i = csize; i < repend; i++) {
		char c = str[i];
		if (c == '\0')
			str[i] = 'C';
		else if (c == ' ')
			str[i] = '_';
	}

	return tsize;
}
