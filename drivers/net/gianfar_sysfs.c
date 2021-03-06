

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/device.h>

#include <asm/uaccess.h>
#include <linux/module.h>

#include "gianfar.h"

static ssize_t gfar_show_bd_stash(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));

	return sprintf(buf, "%s\n", priv->bd_stash_en ? "on" : "off");
}

static ssize_t gfar_set_bd_stash(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));
	int new_setting = 0;
	u32 temp;
	unsigned long flags;

	if (!(priv->device_flags & FSL_GIANFAR_DEV_HAS_BD_STASHING))
		return count;

	
	if (!strncmp("on", buf, count - 1) || !strncmp("1", buf, count - 1))
		new_setting = 1;
	else if (!strncmp("off", buf, count - 1)
		 || !strncmp("0", buf, count - 1))
		new_setting = 0;
	else
		return count;

	spin_lock_irqsave(&priv->rxlock, flags);

	
	priv->bd_stash_en = new_setting;

	temp = gfar_read(&priv->regs->attr);

	if (new_setting)
		temp |= ATTR_BDSTASH;
	else
		temp &= ~(ATTR_BDSTASH);

	gfar_write(&priv->regs->attr, temp);

	spin_unlock_irqrestore(&priv->rxlock, flags);

	return count;
}

static DEVICE_ATTR(bd_stash, 0644, gfar_show_bd_stash, gfar_set_bd_stash);

static ssize_t gfar_show_rx_stash_size(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));

	return sprintf(buf, "%d\n", priv->rx_stash_size);
}

static ssize_t gfar_set_rx_stash_size(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));
	unsigned int length = simple_strtoul(buf, NULL, 0);
	u32 temp;
	unsigned long flags;

	if (!(priv->device_flags & FSL_GIANFAR_DEV_HAS_BUF_STASHING))
		return count;

	spin_lock_irqsave(&priv->rxlock, flags);
	if (length > priv->rx_buffer_size)
		goto out;

	if (length == priv->rx_stash_size)
		goto out;

	priv->rx_stash_size = length;

	temp = gfar_read(&priv->regs->attreli);
	temp &= ~ATTRELI_EL_MASK;
	temp |= ATTRELI_EL(length);
	gfar_write(&priv->regs->attreli, temp);

	
	temp = gfar_read(&priv->regs->attr);

	if (length)
		temp |= ATTR_BUFSTASH;
	else
		temp &= ~(ATTR_BUFSTASH);

	gfar_write(&priv->regs->attr, temp);

out:
	spin_unlock_irqrestore(&priv->rxlock, flags);

	return count;
}

static DEVICE_ATTR(rx_stash_size, 0644, gfar_show_rx_stash_size,
		   gfar_set_rx_stash_size);


static ssize_t gfar_show_rx_stash_index(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));

	return sprintf(buf, "%d\n", priv->rx_stash_index);
}

static ssize_t gfar_set_rx_stash_index(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));
	unsigned short index = simple_strtoul(buf, NULL, 0);
	u32 temp;
	unsigned long flags;

	if (!(priv->device_flags & FSL_GIANFAR_DEV_HAS_BUF_STASHING))
		return count;

	spin_lock_irqsave(&priv->rxlock, flags);
	if (index > priv->rx_stash_size)
		goto out;

	if (index == priv->rx_stash_index)
		goto out;

	priv->rx_stash_index = index;

	temp = gfar_read(&priv->regs->attreli);
	temp &= ~ATTRELI_EI_MASK;
	temp |= ATTRELI_EI(index);
	gfar_write(&priv->regs->attreli, flags);

out:
	spin_unlock_irqrestore(&priv->rxlock, flags);

	return count;
}

static DEVICE_ATTR(rx_stash_index, 0644, gfar_show_rx_stash_index,
		   gfar_set_rx_stash_index);

static ssize_t gfar_show_fifo_threshold(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));

	return sprintf(buf, "%d\n", priv->fifo_threshold);
}

static ssize_t gfar_set_fifo_threshold(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t count)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));
	unsigned int length = simple_strtoul(buf, NULL, 0);
	u32 temp;
	unsigned long flags;

	if (length > GFAR_MAX_FIFO_THRESHOLD)
		return count;

	spin_lock_irqsave(&priv->txlock, flags);

	priv->fifo_threshold = length;

	temp = gfar_read(&priv->regs->fifo_tx_thr);
	temp &= ~FIFO_TX_THR_MASK;
	temp |= length;
	gfar_write(&priv->regs->fifo_tx_thr, temp);

	spin_unlock_irqrestore(&priv->txlock, flags);

	return count;
}

static DEVICE_ATTR(fifo_threshold, 0644, gfar_show_fifo_threshold,
		   gfar_set_fifo_threshold);

static ssize_t gfar_show_fifo_starve(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));

	return sprintf(buf, "%d\n", priv->fifo_starve);
}

static ssize_t gfar_set_fifo_starve(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));
	unsigned int num = simple_strtoul(buf, NULL, 0);
	u32 temp;
	unsigned long flags;

	if (num > GFAR_MAX_FIFO_STARVE)
		return count;

	spin_lock_irqsave(&priv->txlock, flags);

	priv->fifo_starve = num;

	temp = gfar_read(&priv->regs->fifo_tx_starve);
	temp &= ~FIFO_TX_STARVE_MASK;
	temp |= num;
	gfar_write(&priv->regs->fifo_tx_starve, temp);

	spin_unlock_irqrestore(&priv->txlock, flags);

	return count;
}

static DEVICE_ATTR(fifo_starve, 0644, gfar_show_fifo_starve,
		   gfar_set_fifo_starve);

static ssize_t gfar_show_fifo_starve_off(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));

	return sprintf(buf, "%d\n", priv->fifo_starve_off);
}

static ssize_t gfar_set_fifo_starve_off(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct gfar_private *priv = netdev_priv(to_net_dev(dev));
	unsigned int num = simple_strtoul(buf, NULL, 0);
	u32 temp;
	unsigned long flags;

	if (num > GFAR_MAX_FIFO_STARVE_OFF)
		return count;

	spin_lock_irqsave(&priv->txlock, flags);

	priv->fifo_starve_off = num;

	temp = gfar_read(&priv->regs->fifo_tx_starve_shutoff);
	temp &= ~FIFO_TX_STARVE_OFF_MASK;
	temp |= num;
	gfar_write(&priv->regs->fifo_tx_starve_shutoff, temp);

	spin_unlock_irqrestore(&priv->txlock, flags);

	return count;
}

static DEVICE_ATTR(fifo_starve_off, 0644, gfar_show_fifo_starve_off,
		   gfar_set_fifo_starve_off);

void gfar_init_sysfs(struct net_device *dev)
{
	struct gfar_private *priv = netdev_priv(dev);
	int rc;

	
	priv->fifo_threshold = DEFAULT_FIFO_TX_THR;
	priv->fifo_starve = DEFAULT_FIFO_TX_STARVE;
	priv->fifo_starve_off = DEFAULT_FIFO_TX_STARVE_OFF;

	
	rc = device_create_file(&dev->dev, &dev_attr_bd_stash);
	rc |= device_create_file(&dev->dev, &dev_attr_rx_stash_size);
	rc |= device_create_file(&dev->dev, &dev_attr_rx_stash_index);
	rc |= device_create_file(&dev->dev, &dev_attr_fifo_threshold);
	rc |= device_create_file(&dev->dev, &dev_attr_fifo_starve);
	rc |= device_create_file(&dev->dev, &dev_attr_fifo_starve_off);
	if (rc)
		dev_err(&dev->dev, "Error creating gianfar sysfs files.\n");
}
