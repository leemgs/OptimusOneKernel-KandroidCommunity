

#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/sysfs.h>
#include "../iio.h"
#include "../sysfs.h"
#include "../ring_generic.h"

#include "accel.h"
#include "sca3000.h"

enum sca3000_variant {
	d01,
	d03,
	e02,
	e04,
	e05,
	l01,
};


static const struct sca3000_chip_info sca3000_spi_chip_info_tbl[] = {
	{
		.name = "sca3000-d01",
		.temp_output = true,
		.measurement_mode_freq = 250,
		.option_mode_1 = SCA3000_OP_MODE_BYPASS,
		.option_mode_1_freq = 250,
	}, {
		
		.name = "sca3000-d03",
		.temp_output = true,
	}, {
		.name = "sca3000-e02",
		.measurement_mode_freq = 125,
		.option_mode_1 = SCA3000_OP_MODE_NARROW,
		.option_mode_1_freq = 63,
	}, {
		.name = "sca3000-e04",
		.measurement_mode_freq = 100,
		.option_mode_1 = SCA3000_OP_MODE_NARROW,
		.option_mode_1_freq = 50,
		.option_mode_2 = SCA3000_OP_MODE_WIDE,
		.option_mode_2_freq = 400,
	}, {
		.name = "sca3000-e05",
		.measurement_mode_freq = 200,
		.option_mode_1 = SCA3000_OP_MODE_NARROW,
		.option_mode_1_freq = 50,
		.option_mode_2 = SCA3000_OP_MODE_WIDE,
		.option_mode_2_freq = 400,
	}, {
		
		.name = "sca3000-l01",
		.temp_output = true,
		.option_mode_1 = SCA3000_OP_MODE_BYPASS,
	},
};


int sca3000_write_reg(struct sca3000_state *st, u8 address, u8 val)
{
	struct spi_transfer xfer = {
		.bits_per_word = 8,
		.len = 2,
		.cs_change = 1,
		.tx_buf = st->tx,
	};
	struct spi_message msg;

	st->tx[0] = SCA3000_WRITE_REG(address);
	st->tx[1] = val;
	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	return spi_sync(st->us, &msg);
}

int sca3000_read_data(struct sca3000_state *st,
		      uint8_t reg_address_high,
		      u8 **rx_p,
		      int len)
{
	int ret;
	struct spi_message msg;
	struct spi_transfer xfer = {
		.bits_per_word = 8,
		.len = len + 1,
		.cs_change = 1,
		.tx_buf = st->tx,
	};

	*rx_p = kmalloc(len + 1, GFP_KERNEL);
	if (*rx_p == NULL) {
		ret = -ENOMEM;
		goto error_ret;
	}
	xfer.rx_buf = *rx_p;
	st->tx[0] = SCA3000_READ_REG(reg_address_high);
	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	ret = spi_sync(st->us, &msg);

	if (ret) {
		dev_err(get_device(&st->us->dev), "problem reading register");
		goto error_free_rx;
	}

	return 0;
error_free_rx:
	kfree(*rx_p);
error_ret:
	return ret;

}

static int sca3000_reg_lock_on(struct sca3000_state *st)
{
	u8 *rx;
	int ret;

	ret = sca3000_read_data(st, SCA3000_REG_ADDR_STATUS, &rx, 1);

	if (ret < 0)
		return ret;
	ret = !(rx[1] & SCA3000_LOCKED);
	kfree(rx);

	return ret;
}


static int __sca3000_unlock_reg_lock(struct sca3000_state *st)
{
	struct spi_message msg;
	struct spi_transfer xfer[3] = {
		{
			.bits_per_word = 8,
			.len = 2,
			.cs_change = 1,
			.tx_buf = st->tx,
		}, {
			.bits_per_word = 8,
			.len = 2,
			.cs_change = 1,
			.tx_buf = st->tx + 2,
		}, {
			.bits_per_word = 8,
			.len = 2,
			.cs_change = 1,
			.tx_buf = st->tx + 4,
		},
	};
	st->tx[0] = SCA3000_WRITE_REG(SCA3000_REG_ADDR_UNLOCK);
	st->tx[1] = 0x00;
	st->tx[2] = SCA3000_WRITE_REG(SCA3000_REG_ADDR_UNLOCK);
	st->tx[3] = 0x50;
	st->tx[4] = SCA3000_WRITE_REG(SCA3000_REG_ADDR_UNLOCK);
	st->tx[5] = 0xA0;
	spi_message_init(&msg);
	spi_message_add_tail(&xfer[0], &msg);
	spi_message_add_tail(&xfer[1], &msg);
	spi_message_add_tail(&xfer[2], &msg);

	return spi_sync(st->us, &msg);
}


static int sca3000_write_ctrl_reg(struct sca3000_state *st,
				  uint8_t sel,
				  uint8_t val)
{

	int ret;

	ret = sca3000_reg_lock_on(st);
	if (ret < 0)
		goto error_ret;
	if (ret) {
		ret = __sca3000_unlock_reg_lock(st);
		if (ret)
			goto error_ret;
	}

	
	ret = sca3000_write_reg(st, SCA3000_REG_ADDR_CTRL_SEL, sel);
	if (ret)
		goto error_ret;

	
	ret = sca3000_write_reg(st, SCA3000_REG_ADDR_CTRL_DATA, val);

error_ret:
	return ret;
}



static int sca3000_read_ctrl_reg(struct sca3000_state *st,
				 u8 ctrl_reg,
				 u8 **rx_p)
{
	int ret;

	ret = sca3000_reg_lock_on(st);
	if (ret < 0)
		goto error_ret;
	if (ret) {
		ret = __sca3000_unlock_reg_lock(st);
		if (ret)
			goto error_ret;
	}
	
	ret = sca3000_write_reg(st, SCA3000_REG_ADDR_CTRL_SEL, ctrl_reg);
	if (ret)
		goto error_ret;
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_CTRL_DATA, rx_p, 1);

error_ret:
	return ret;
}

#ifdef SCA3000_DEBUG

static int sca3000_check_status(struct device *dev)
{
	u8 *rx;
	int ret;
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;

	mutex_lock(&st->lock);
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_STATUS, &rx, 1);
	if (ret < 0)
		goto error_ret;
	if (rx[1] & SCA3000_EEPROM_CS_ERROR)
		dev_err(dev, "eeprom error \n");
	if (rx[1] & SCA3000_SPI_FRAME_ERROR)
		dev_err(dev, "Previous SPI Frame was corrupt\n");
	kfree(rx);

error_ret:
	mutex_unlock(&st->lock);
	return ret;
}
#endif 


static ssize_t sca3000_read_13bit_signed(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	int len = 0, ret;
	int val;
	struct iio_dev_attr *this_attr = to_iio_dev_attr(attr);
	u8 *rx;
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;

	mutex_lock(&st->lock);
	ret = sca3000_read_data(st, this_attr->address, &rx, 2);
	if (ret < 0)
		goto error_ret;
	val = sca3000_13bit_convert(rx[1], rx[2]);
	len += sprintf(buf + len, "%d\n", val);
	kfree(rx);
error_ret:
	mutex_unlock(&st->lock);

	return ret ? ret : len;
}


static ssize_t sca3000_show_name(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct iio_dev *dev_info = dev_get_drvdata(dev);
	struct sca3000_state *st = dev_info->dev_data;
	return sprintf(buf, "%s\n", st->info->name);
}

static ssize_t sca3000_show_rev(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	int len = 0, ret;
	struct iio_dev *dev_info = dev_get_drvdata(dev);
	struct sca3000_state *st = dev_info->dev_data;

	u8 *rx;

	mutex_lock(&st->lock);
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_REVID, &rx, 1);
	if (ret < 0)
		goto error_ret;
	len += sprintf(buf + len,
		       "major=%d, minor=%d\n",
		       rx[1] & SCA3000_REVID_MAJOR_MASK,
		       rx[1] & SCA3000_REVID_MINOR_MASK);
	kfree(rx);

error_ret:
	mutex_unlock(&st->lock);

	return ret ? ret : len;
}


static ssize_t
sca3000_show_available_measurement_modes(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	struct iio_dev *dev_info = dev_get_drvdata(dev);
	struct sca3000_state *st = dev_info->dev_data;
	int len = 0;

	len += sprintf(buf + len, "0 - normal mode");
	switch (st->info->option_mode_1) {
	case SCA3000_OP_MODE_NARROW:
		len += sprintf(buf + len, ", 1 - narrow mode");
		break;
	case SCA3000_OP_MODE_BYPASS:
		len += sprintf(buf + len, ", 1 - bypass mode");
		break;
	};
	switch (st->info->option_mode_2) {
	case SCA3000_OP_MODE_WIDE:
		len += sprintf(buf + len, ", 2 - wide mode");
		break;
	}
	
	len += sprintf(buf + len, " 3 - motion detection \n");

	return len;
}


static ssize_t
sca3000_show_measurement_mode(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	struct iio_dev *dev_info = dev_get_drvdata(dev);
	struct sca3000_state *st = dev_info->dev_data;
	int len = 0, ret;
	u8 *rx;

	mutex_lock(&st->lock);
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_MODE, &rx, 1);
	if (ret)
		goto error_ret;
	
	rx[1] &= 0x03;
	switch (rx[1]) {
	case SCA3000_MEAS_MODE_NORMAL:
		len += sprintf(buf + len, "0 - normal mode\n");
		break;
	case SCA3000_MEAS_MODE_MOT_DET:
		len += sprintf(buf + len, "3 - motion detection\n");
		break;
	case SCA3000_MEAS_MODE_OP_1:
		switch (st->info->option_mode_1) {
		case SCA3000_OP_MODE_NARROW:
			len += sprintf(buf + len, "1 - narrow mode\n");
			break;
		case SCA3000_OP_MODE_BYPASS:
			len += sprintf(buf + len, "1 - bypass mode\n");
			break;
		};
		break;
	case SCA3000_MEAS_MODE_OP_2:
		switch (st->info->option_mode_2) {
		case SCA3000_OP_MODE_WIDE:
			len += sprintf(buf + len, "2 - wide mode\n");
			break;
		}
		break;
	};

error_ret:
	mutex_unlock(&st->lock);

	return ret ? ret : len;
}


static ssize_t
sca3000_store_measurement_mode(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf,
			       size_t len)
{
	struct iio_dev *dev_info = dev_get_drvdata(dev);
	struct sca3000_state *st = dev_info->dev_data;
	int ret;
	u8 *rx;
	int mask = 0x03;
	long val;

	mutex_lock(&st->lock);
	ret = strict_strtol(buf, 10, &val);
	if (ret)
		goto error_ret;
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_MODE, &rx, 1);
	if (ret)
		goto error_ret;
	rx[1] &= ~mask;
	rx[1] |= (val & mask);
	ret = sca3000_write_reg(st, SCA3000_REG_ADDR_MODE, rx[1]);
	if (ret)
		goto error_free_rx;
	mutex_unlock(&st->lock);

	return len;

error_free_rx:
	kfree(rx);
error_ret:
	mutex_unlock(&st->lock);

	return ret;
}



static IIO_DEVICE_ATTR(available_measurement_modes, S_IRUGO,
		       sca3000_show_available_measurement_modes,
		       NULL, 0);

static IIO_DEVICE_ATTR(measurement_mode, S_IRUGO | S_IWUSR,
		       sca3000_show_measurement_mode,
		       sca3000_store_measurement_mode,
		       0);



static IIO_DEV_ATTR_NAME(sca3000_show_name);
static IIO_DEV_ATTR_REV(sca3000_show_rev);

static IIO_DEV_ATTR_ACCEL_X(sca3000_read_13bit_signed,
			    SCA3000_REG_ADDR_X_MSB);
static IIO_DEV_ATTR_ACCEL_Y(sca3000_read_13bit_signed,
			    SCA3000_REG_ADDR_Y_MSB);
static IIO_DEV_ATTR_ACCEL_Z(sca3000_read_13bit_signed,
			    SCA3000_REG_ADDR_Z_MSB);



static ssize_t sca3000_read_av_freq(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	int len = 0, ret;
	u8 *rx;
	mutex_lock(&st->lock);
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_MODE, &rx, 1);
	mutex_unlock(&st->lock);
	if (ret)
		goto error_ret;
	rx[1] &= 0x03;
	switch (rx[1]) {
	case SCA3000_MEAS_MODE_NORMAL:
		len += sprintf(buf + len, "%d %d %d\n",
			       st->info->measurement_mode_freq,
			       st->info->measurement_mode_freq/2,
			       st->info->measurement_mode_freq/4);
		break;
	case SCA3000_MEAS_MODE_OP_1:
		len += sprintf(buf + len, "%d %d %d\n",
			       st->info->option_mode_1_freq,
			       st->info->option_mode_1_freq/2,
			       st->info->option_mode_1_freq/4);
		break;
	case SCA3000_MEAS_MODE_OP_2:
		len += sprintf(buf + len, "%d %d %d\n",
			       st->info->option_mode_2_freq,
			       st->info->option_mode_2_freq/2,
			       st->info->option_mode_2_freq/4);
		break;
	};
	kfree(rx);
	return len;
error_ret:
	return ret;
}

static inline int __sca3000_get_base_freq(struct sca3000_state *st,
					  const struct sca3000_chip_info *info,
					  int *base_freq)
{
	int ret;
	u8 *rx;

	ret = sca3000_read_data(st, SCA3000_REG_ADDR_MODE, &rx, 1);
	if (ret)
		goto error_ret;
	switch (0x03 & rx[1]) {
	case SCA3000_MEAS_MODE_NORMAL:
		*base_freq = info->measurement_mode_freq;
		break;
	case SCA3000_MEAS_MODE_OP_1:
		*base_freq = info->option_mode_1_freq;
		break;
	case SCA3000_MEAS_MODE_OP_2:
		*base_freq = info->option_mode_2_freq;
		break;
	};
	kfree(rx);
error_ret:
	return ret;
}


static ssize_t sca3000_read_frequency(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	int ret, len = 0, base_freq = 0;
	u8 *rx;
	mutex_lock(&st->lock);
	ret = __sca3000_get_base_freq(st, st->info, &base_freq);
	if (ret)
		goto error_ret_mut;
	ret = sca3000_read_ctrl_reg(st, SCA3000_REG_CTRL_SEL_OUT_CTRL, &rx);
	mutex_unlock(&st->lock);
	if (ret)
		goto error_ret;
	if (base_freq > 0)
		switch (rx[1]&0x03) {
		case 0x00:
		case 0x03:
			len = sprintf(buf, "%d\n", base_freq);
			break;
		case 0x01:
			len = sprintf(buf, "%d\n", base_freq/2);
			break;
		case 0x02:
			len = sprintf(buf, "%d\n", base_freq/4);
			break;
	};
			kfree(rx);
	return len;
error_ret_mut:
	mutex_unlock(&st->lock);
error_ret:
	return ret;
}


static ssize_t sca3000_set_frequency(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf,
			      size_t len)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	int ret, base_freq = 0;
	u8 *rx;
	long val;

	ret = strict_strtol(buf, 10, &val);
	if (ret)
		return ret;

	mutex_lock(&st->lock);
	
	ret = __sca3000_get_base_freq(st, st->info, &base_freq);
	if (ret)
		goto error_free_lock;

	ret = sca3000_read_ctrl_reg(st, SCA3000_REG_CTRL_SEL_OUT_CTRL, &rx);
	if (ret)
		goto error_free_lock;
	
	rx[1] &= ~0x03;

	if (val == base_freq/2) {
		rx[1] |= SCA3000_OUT_CTRL_BUF_DIV_2;
	} else if (val == base_freq/4) {
		rx[1] |= SCA3000_OUT_CTRL_BUF_DIV_4;
	} else if (val != base_freq) {
		ret = -EINVAL;
		goto error_free_lock;
	}
	ret = sca3000_write_ctrl_reg(st, SCA3000_REG_CTRL_SEL_OUT_CTRL, rx[1]);
error_free_lock:
	mutex_unlock(&st->lock);

	return ret ? ret : len;
}


static IIO_DEV_ATTR_AVAIL_SAMP_FREQ(sca3000_read_av_freq);

static IIO_DEV_ATTR_SAMP_FREQ(S_IWUSR | S_IRUGO,
			      sca3000_read_frequency,
			      sca3000_set_frequency);



static ssize_t sca3000_read_temp(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	int len = 0, ret;
	int val;
	u8 *rx;
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_TEMP_MSB, &rx, 2);
	if (ret < 0)
		goto error_ret;
	val = ((rx[1]&0x3F) << 3) | ((rx[2] & 0xE0) >> 5);
	len += sprintf(buf + len, "%d\n", val);
	kfree(rx);

	return len;

error_ret:
	return ret;
}
static IIO_DEV_ATTR_TEMP(sca3000_read_temp);


static ssize_t sca3000_show_thresh(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	struct iio_dev_attr *this_attr = to_iio_dev_attr(attr);
	int len = 0, ret;
	u8 *rx;

	mutex_lock(&st->lock);
	ret = sca3000_read_ctrl_reg(st,
				    this_attr->address,
				    &rx);
	mutex_unlock(&st->lock);
	if (ret)
		return ret;
	len += sprintf(buf + len, "%d\n", rx[1]);
	kfree(rx);

	return len;
}


static ssize_t sca3000_write_thresh(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf,
				    size_t len)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	struct iio_dev_attr *this_attr = to_iio_dev_attr(attr);
	int ret;
	long val;

	ret = strict_strtol(buf, 10, &val);
	if (ret)
		return ret;
	mutex_lock(&st->lock);
	ret = sca3000_write_ctrl_reg(st, this_attr->address, val);
	mutex_unlock(&st->lock);

	return ret ? ret : len;
}

static IIO_DEV_ATTR_ACCEL_THRESH_X(S_IRUGO | S_IWUSR,
				   sca3000_show_thresh,
				   sca3000_write_thresh,
				   SCA3000_REG_CTRL_SEL_MD_X_TH);
static IIO_DEV_ATTR_ACCEL_THRESH_Y(S_IRUGO | S_IWUSR,
				   sca3000_show_thresh,
				   sca3000_write_thresh,
				   SCA3000_REG_CTRL_SEL_MD_Y_TH);
static IIO_DEV_ATTR_ACCEL_THRESH_Z(S_IRUGO | S_IWUSR,
				   sca3000_show_thresh,
				   sca3000_write_thresh,
				   SCA3000_REG_CTRL_SEL_MD_Z_TH);

static struct attribute *sca3000_attributes[] = {
	&iio_dev_attr_name.dev_attr.attr,
	&iio_dev_attr_revision.dev_attr.attr,
	&iio_dev_attr_accel_x.dev_attr.attr,
	&iio_dev_attr_accel_y.dev_attr.attr,
	&iio_dev_attr_accel_z.dev_attr.attr,
	&iio_dev_attr_thresh_accel_x.dev_attr.attr,
	&iio_dev_attr_thresh_accel_y.dev_attr.attr,
	&iio_dev_attr_thresh_accel_z.dev_attr.attr,
	&iio_dev_attr_available_measurement_modes.dev_attr.attr,
	&iio_dev_attr_measurement_mode.dev_attr.attr,
	&iio_dev_attr_available_sampling_frequency.dev_attr.attr,
	&iio_dev_attr_sampling_frequency.dev_attr.attr,
	NULL,
};

static struct attribute *sca3000_attributes_with_temp[] = {
	&iio_dev_attr_name.dev_attr.attr,
	&iio_dev_attr_revision.dev_attr.attr,
	&iio_dev_attr_accel_x.dev_attr.attr,
	&iio_dev_attr_accel_y.dev_attr.attr,
	&iio_dev_attr_accel_z.dev_attr.attr,
	&iio_dev_attr_thresh_accel_x.dev_attr.attr,
	&iio_dev_attr_thresh_accel_y.dev_attr.attr,
	&iio_dev_attr_thresh_accel_z.dev_attr.attr,
	&iio_dev_attr_available_measurement_modes.dev_attr.attr,
	&iio_dev_attr_measurement_mode.dev_attr.attr,
	&iio_dev_attr_available_sampling_frequency.dev_attr.attr,
	&iio_dev_attr_sampling_frequency.dev_attr.attr,
	
	&iio_dev_attr_temp.dev_attr.attr,
	NULL,
};

static const struct attribute_group sca3000_attribute_group = {
	.attrs = sca3000_attributes,
};

static const struct attribute_group sca3000_attribute_group_with_temp = {
	.attrs = sca3000_attributes_with_temp,
};





static void sca3000_interrupt_handler_bh(struct work_struct *work_s)
{
	struct sca3000_state *st
		= container_of(work_s, struct sca3000_state,
			       interrupt_handler_ws);
	u8 *rx;
	int ret;

	
	enable_irq(st->us->irq);
	mutex_lock(&st->lock);
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_INT_STATUS,
				&rx, 1);
	mutex_unlock(&st->lock);
	if (ret)
		goto done;

	sca3000_ring_int_process(rx[1], st->indio_dev->ring);

	if (rx[1] & SCA3000_INT_STATUS_FREE_FALL)
		iio_push_event(st->indio_dev, 0,
			       IIO_EVENT_CODE_FREE_FALL,
			       st->last_timestamp);

	if (rx[1] & SCA3000_INT_STATUS_Y_TRIGGER)
		iio_push_event(st->indio_dev, 0,
			       IIO_EVENT_CODE_ACCEL_Y_HIGH,
			       st->last_timestamp);

	if (rx[1] & SCA3000_INT_STATUS_X_TRIGGER)
		iio_push_event(st->indio_dev, 0,
			       IIO_EVENT_CODE_ACCEL_X_HIGH,
			       st->last_timestamp);

	if (rx[1] & SCA3000_INT_STATUS_Z_TRIGGER)
		iio_push_event(st->indio_dev, 0,
			       IIO_EVENT_CODE_ACCEL_Z_HIGH,
			       st->last_timestamp);

done:
	kfree(rx);
	return;
}


static int sca3000_handler_th(struct iio_dev *dev_info,
			      int index,
			      s64 timestamp,
			      int no_test)
{
	struct sca3000_state *st = dev_info->dev_data;

	st->last_timestamp = timestamp;
	schedule_work(&st->interrupt_handler_ws);

	return 0;
}


static ssize_t sca3000_query_mo_det(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	struct iio_event_attr *this_attr = to_iio_event_attr(attr);
	int ret, len = 0;
	u8 *rx;
	u8 protect_mask = 0x03;

	
	mutex_lock(&st->lock);
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_MODE, &rx, 1);
	if (ret)
		goto error_ret;

	if ((rx[1]&protect_mask) != SCA3000_MEAS_MODE_MOT_DET)
		len += sprintf(buf + len, "0\n");
	else {
		kfree(rx);
		ret = sca3000_read_ctrl_reg(st,
					    SCA3000_REG_CTRL_SEL_MD_CTRL,
					    &rx);
		if (ret)
			goto error_ret;
		
		len += sprintf(buf + len, "%d\n",
			       (rx[1] & this_attr->mask) ? 1 : 0);
	}
	kfree(rx);
error_ret:
	mutex_unlock(&st->lock);

	return ret ? ret : len;
}

static ssize_t sca3000_query_free_fall_mode(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	int ret, len;
	u8 *rx;
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;

	mutex_lock(&st->lock);
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_MODE, &rx, 1);
	mutex_unlock(&st->lock);
	if (ret)
		return ret;
	len = sprintf(buf, "%d\n",
		      !!(rx[1] & SCA3000_FREE_FALL_DETECT));
	kfree(rx);

	return len;
}

static ssize_t sca3000_query_ring_int(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct iio_event_attr *this_attr = to_iio_event_attr(attr);
	int ret, len;
	u8 *rx;
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	mutex_lock(&st->lock);
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_INT_MASK, &rx, 1);
	mutex_unlock(&st->lock);
	if (ret)
		return ret;
	len = sprintf(buf, "%d\n", (rx[1] & this_attr->mask) ? 1 : 0);
	kfree(rx);

	return len;
}

static ssize_t sca3000_set_ring_int(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf,
				      size_t len)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	struct iio_event_attr *this_attr = to_iio_event_attr(attr);

	long val;
	int ret;
	u8 *rx;

	mutex_lock(&st->lock);
	ret = strict_strtol(buf, 10, &val);
	if (ret)
		goto error_ret;
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_INT_MASK, &rx, 1);
	if (ret)
		goto error_ret;
	if (val)
		ret = sca3000_write_reg(st,
					SCA3000_REG_ADDR_INT_MASK,
					rx[1] | this_attr->mask);
	else
		ret = sca3000_write_reg(st,
					SCA3000_REG_ADDR_INT_MASK,
					rx[1] & ~this_attr->mask);
	kfree(rx);
error_ret:
	mutex_unlock(&st->lock);

	return ret ? ret : len;
}



static ssize_t sca3000_set_free_fall_mode(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf,
					  size_t len)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	long val;
	int ret;
	u8 *rx;
	u8 protect_mask = SCA3000_FREE_FALL_DETECT;

	mutex_lock(&st->lock);
	ret = strict_strtol(buf, 10, &val);
	if (ret)
		goto error_ret;

	
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_MODE, &rx, 1);
	if (ret)
		goto error_ret;

	
	if (val && !(rx[1] & protect_mask))
		ret = sca3000_write_reg(st, SCA3000_REG_ADDR_MODE,
					(rx[1] | SCA3000_FREE_FALL_DETECT));
	
	else if (!val && (rx[1]&protect_mask))
		ret = sca3000_write_reg(st, SCA3000_REG_ADDR_MODE,
					(rx[1] & ~protect_mask));

	kfree(rx);
error_ret:
	mutex_unlock(&st->lock);

	return ret ? ret : len;
}


static ssize_t sca3000_set_mo_det(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf,
				  size_t len)
{
	struct iio_dev *indio_dev = dev_get_drvdata(dev);
	struct sca3000_state *st = indio_dev->dev_data;
	struct iio_event_attr *this_attr = to_iio_event_attr(attr);
	long val;
	int ret;
	u8 *rx;
	u8 protect_mask = 0x03;
	ret = strict_strtol(buf, 10, &val);
	if (ret)
		return ret;

	mutex_lock(&st->lock);
	
	ret = sca3000_read_ctrl_reg(st,
				    SCA3000_REG_CTRL_SEL_MD_CTRL,
				    &rx);
	if (ret)
		goto exit_point;
	
	if (val && !(rx[1] & this_attr->mask)) {
		ret = sca3000_write_ctrl_reg(st,
					     SCA3000_REG_CTRL_SEL_MD_CTRL,
					     rx[1] | this_attr->mask);
		if (ret)
			goto exit_point_free_rx;
		st->mo_det_use_count++;
	} else if (!val && (rx[1]&this_attr->mask)) {
		ret = sca3000_write_ctrl_reg(st,
					     SCA3000_REG_CTRL_SEL_MD_CTRL,
					     rx[1] & ~(this_attr->mask));
		if (ret)
			goto exit_point_free_rx;
		st->mo_det_use_count--;
	} else 
		goto exit_point_free_rx;
	kfree(rx);
	
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_MODE, &rx, 1);
	if (ret)
		goto exit_point;
	
	if ((st->mo_det_use_count)
	    && ((rx[1]&protect_mask) != SCA3000_MEAS_MODE_MOT_DET))
		ret = sca3000_write_reg(st, SCA3000_REG_ADDR_MODE,
					(rx[1] & ~protect_mask)
					| SCA3000_MEAS_MODE_MOT_DET);
	
	else if (!(st->mo_det_use_count)
		 && ((rx[1]&protect_mask) == SCA3000_MEAS_MODE_MOT_DET))
		ret = sca3000_write_reg(st, SCA3000_REG_ADDR_MODE,
					(rx[1] & ~protect_mask));
exit_point_free_rx:
	kfree(rx);
exit_point:
	mutex_unlock(&st->lock);

	return ret ? ret : len;
}


IIO_EVENT_SH(all, &sca3000_handler_th);


IIO_EVENT_ATTR_FREE_FALL_DETECT_SH(iio_event_all,
				   sca3000_query_free_fall_mode,
				   sca3000_set_free_fall_mode,
				   0)


IIO_EVENT_ATTR_ACCEL_X_HIGH_SH(iio_event_all,
			       sca3000_query_mo_det,
			       sca3000_set_mo_det,
			       SCA3000_MD_CTRL_OR_X);

IIO_EVENT_ATTR_ACCEL_Y_HIGH_SH(iio_event_all,
			       sca3000_query_mo_det,
			       sca3000_set_mo_det,
			       SCA3000_MD_CTRL_OR_Y);

IIO_EVENT_ATTR_ACCEL_Z_HIGH_SH(iio_event_all,
			       sca3000_query_mo_det,
			       sca3000_set_mo_det,
			       SCA3000_MD_CTRL_OR_Z);


IIO_EVENT_ATTR_RING_50_FULL_SH(iio_event_all,
			       sca3000_query_ring_int,
			       sca3000_set_ring_int,
			       SCA3000_INT_MASK_RING_HALF);

IIO_EVENT_ATTR_RING_75_FULL_SH(iio_event_all,
			       sca3000_query_ring_int,
			       sca3000_set_ring_int,
			       SCA3000_INT_MASK_RING_THREE_QUARTER);

static struct attribute *sca3000_event_attributes[] = {
	&iio_event_attr_free_fall.dev_attr.attr,
	&iio_event_attr_accel_x_high.dev_attr.attr,
	&iio_event_attr_accel_y_high.dev_attr.attr,
	&iio_event_attr_accel_z_high.dev_attr.attr,
	&iio_event_attr_ring_50_full.dev_attr.attr,
	&iio_event_attr_ring_75_full.dev_attr.attr,
	NULL,
};

static struct attribute_group sca3000_event_attribute_group = {
	.attrs = sca3000_event_attributes,
};


static int sca3000_clean_setup(struct sca3000_state *st)
{
	int ret;
	u8 *rx;

	mutex_lock(&st->lock);
	
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_INT_STATUS, &rx, 1);
	if (ret)
		goto error_ret;
	kfree(rx);

	
	ret = sca3000_read_ctrl_reg(st,
				    SCA3000_REG_CTRL_SEL_MD_CTRL,
				    &rx);
	if (ret)
		goto error_ret;
	ret = sca3000_write_ctrl_reg(st,
				     SCA3000_REG_CTRL_SEL_MD_CTRL,
				     rx[1] & SCA3000_MD_CTRL_PROT_MASK);
	kfree(rx);
	if (ret)
		goto error_ret;

	
	sca3000_read_ctrl_reg(st,
			      SCA3000_REG_CTRL_SEL_OUT_CTRL,
			      &rx);
	
	ret = sca3000_write_ctrl_reg(st,
				     SCA3000_REG_CTRL_SEL_OUT_CTRL,
				     (rx[1] & SCA3000_OUT_CTRL_PROT_MASK)
				     | SCA3000_OUT_CTRL_BUF_X_EN
				     | SCA3000_OUT_CTRL_BUF_Y_EN
				     | SCA3000_OUT_CTRL_BUF_Z_EN
				     | SCA3000_OUT_CTRL_BUF_DIV_4);
	kfree(rx);

	if (ret)
		goto error_ret;
	
	ret = sca3000_read_data(st,
			  SCA3000_REG_ADDR_INT_MASK,
			  &rx, 1);
	if (ret)
		goto error_ret;
	ret = sca3000_write_reg(st,
				SCA3000_REG_ADDR_INT_MASK,
				(rx[1] & SCA3000_INT_MASK_PROT_MASK)
				| SCA3000_INT_MASK_ACTIVE_LOW);
	kfree(rx);
	if (ret)
		goto error_ret;
	
	
	ret = sca3000_read_data(st,
			  SCA3000_REG_ADDR_MODE,
			  &rx, 1);
	if (ret)
		goto error_ret;
	ret = sca3000_write_reg(st,
				SCA3000_REG_ADDR_MODE,
				(rx[1] & SCA3000_MODE_PROT_MASK));
	kfree(rx);
	st->bpse = 11;

error_ret:
	mutex_unlock(&st->lock);
	return ret;
}

static int __devinit __sca3000_probe(struct spi_device *spi,
				     enum sca3000_variant variant)
{
	int ret, regdone = 0;
	struct sca3000_state *st;

	st = kzalloc(sizeof(struct sca3000_state), GFP_KERNEL);
	if (st == NULL) {
		ret = -ENOMEM;
		goto error_ret;
	}
	spi_set_drvdata(spi, st);

	st->tx = kmalloc(sizeof(*st->tx)*6, GFP_KERNEL);
	if (st->tx == NULL) {
		ret = -ENOMEM;
		goto error_clear_st;
	}
	st->rx = kmalloc(sizeof(*st->rx)*3, GFP_KERNEL);
	if (st->rx == NULL) {
		ret = -ENOMEM;
		goto error_free_tx;
	}
	st->us = spi;
	mutex_init(&st->lock);
	st->info = &sca3000_spi_chip_info_tbl[variant];

	st->indio_dev = iio_allocate_device();
	if (st->indio_dev == NULL) {
		ret = -ENOMEM;
		goto error_free_rx;
	}

	st->indio_dev->dev.parent = &spi->dev;
	st->indio_dev->num_interrupt_lines = 1;
	st->indio_dev->event_attrs = &sca3000_event_attribute_group;
	if (st->info->temp_output)
		st->indio_dev->attrs = &sca3000_attribute_group_with_temp;
	else
		st->indio_dev->attrs = &sca3000_attribute_group;
	st->indio_dev->dev_data = (void *)(st);
	st->indio_dev->modes = INDIO_DIRECT_MODE;

	sca3000_configure_ring(st->indio_dev);

	ret = iio_device_register(st->indio_dev);
	if (ret < 0)
		goto error_free_dev;
	regdone = 1;
	ret = iio_ring_buffer_register(st->indio_dev->ring);
	if (ret < 0)
		goto error_unregister_dev;
	if (spi->irq && gpio_is_valid(irq_to_gpio(spi->irq)) > 0) {
		INIT_WORK(&st->interrupt_handler_ws,
			  sca3000_interrupt_handler_bh);
		ret = iio_register_interrupt_line(spi->irq,
						  st->indio_dev,
						  0,
						  IRQF_TRIGGER_FALLING,
						  "sca3000");
		if (ret)
			goto error_unregister_ring;
		
		iio_add_event_to_list(iio_event_attr_accel_z_high.listel,
					    &st->indio_dev
					    ->interrupts[0]->ev_list);
	}
	sca3000_register_ring_funcs(st->indio_dev);
	ret = sca3000_clean_setup(st);
	if (ret)
		goto error_unregister_interrupt_line;
	return 0;

error_unregister_interrupt_line:
	if (spi->irq && gpio_is_valid(irq_to_gpio(spi->irq)) > 0)
		iio_unregister_interrupt_line(st->indio_dev, 0);
error_unregister_ring:
	iio_ring_buffer_unregister(st->indio_dev->ring);
error_unregister_dev:
error_free_dev:
	if (regdone)
		iio_device_unregister(st->indio_dev);
	else
		iio_free_device(st->indio_dev);
error_free_rx:
	kfree(st->rx);
error_free_tx:
	kfree(st->tx);
error_clear_st:
	kfree(st);
error_ret:
	return ret;
}

static int sca3000_stop_all_interrupts(struct sca3000_state *st)
{
	int ret;
	u8 *rx;

	mutex_lock(&st->lock);
	ret = sca3000_read_data(st, SCA3000_REG_ADDR_INT_MASK, &rx, 1);
	if (ret)
		goto error_ret;
	ret = sca3000_write_reg(st, SCA3000_REG_ADDR_INT_MASK,
				(rx[1] & ~(SCA3000_INT_MASK_RING_THREE_QUARTER
					   | SCA3000_INT_MASK_RING_HALF
					   | SCA3000_INT_MASK_ALL_INTS)));
error_ret:
	kfree(rx);
	return ret;

}

static int sca3000_remove(struct spi_device *spi)
{
	struct sca3000_state *st =  spi_get_drvdata(spi);
	struct iio_dev *indio_dev = st->indio_dev;
	int ret;
	
	ret = sca3000_stop_all_interrupts(st);
	if (ret)
		return ret;
	if (spi->irq && gpio_is_valid(irq_to_gpio(spi->irq)) > 0)
		iio_unregister_interrupt_line(indio_dev, 0);
	iio_ring_buffer_unregister(indio_dev->ring);
	sca3000_unconfigure_ring(indio_dev);
	iio_device_unregister(indio_dev);

	kfree(st->tx);
	kfree(st->rx);
	kfree(st);

	return 0;
}


#define SCA3000_VARIANT_PROBE(_name)				\
	static int __devinit					\
	sca3000_##_name##_probe(struct spi_device *spi)		\
	{							\
		return __sca3000_probe(spi, _name);		\
	}

#define SCA3000_VARIANT_SPI_DRIVER(_name)			\
	struct spi_driver sca3000_##_name##_driver = {		\
		.driver = {					\
			.name = "sca3000_" #_name,		\
			.owner = THIS_MODULE,			\
		},						\
		.probe = sca3000_##_name##_probe,		\
		.remove = __devexit_p(sca3000_remove),		\
	}

SCA3000_VARIANT_PROBE(d01);
static SCA3000_VARIANT_SPI_DRIVER(d01);

SCA3000_VARIANT_PROBE(d03);
static SCA3000_VARIANT_SPI_DRIVER(d03);

SCA3000_VARIANT_PROBE(e02);
static SCA3000_VARIANT_SPI_DRIVER(e02);

SCA3000_VARIANT_PROBE(e04);
static SCA3000_VARIANT_SPI_DRIVER(e04);

SCA3000_VARIANT_PROBE(e05);
static SCA3000_VARIANT_SPI_DRIVER(e05);

SCA3000_VARIANT_PROBE(l01);
static SCA3000_VARIANT_SPI_DRIVER(l01);

static __init int sca3000_init(void)
{
	int ret;

	ret = spi_register_driver(&sca3000_d01_driver);
	if (ret)
		goto error_ret;
	ret = spi_register_driver(&sca3000_d03_driver);
	if (ret)
		goto error_unreg_d01;
	ret = spi_register_driver(&sca3000_e02_driver);
	if (ret)
		goto error_unreg_d03;
	ret = spi_register_driver(&sca3000_e04_driver);
	if (ret)
		goto error_unreg_e02;
	ret = spi_register_driver(&sca3000_e05_driver);
	if (ret)
		goto error_unreg_e04;
	ret = spi_register_driver(&sca3000_l01_driver);
	if (ret)
		goto error_unreg_e05;

	return 0;

error_unreg_e05:
	spi_unregister_driver(&sca3000_e05_driver);
error_unreg_e04:
	spi_unregister_driver(&sca3000_e04_driver);
error_unreg_e02:
	spi_unregister_driver(&sca3000_e02_driver);
error_unreg_d03:
	spi_unregister_driver(&sca3000_d03_driver);
error_unreg_d01:
	spi_unregister_driver(&sca3000_d01_driver);
error_ret:

	return ret;
}

static __exit void sca3000_exit(void)
{
	spi_unregister_driver(&sca3000_l01_driver);
	spi_unregister_driver(&sca3000_e05_driver);
	spi_unregister_driver(&sca3000_e04_driver);
	spi_unregister_driver(&sca3000_e02_driver);
	spi_unregister_driver(&sca3000_d03_driver);
	spi_unregister_driver(&sca3000_d01_driver);
}

module_init(sca3000_init);
module_exit(sca3000_exit);

MODULE_AUTHOR("Jonathan Cameron <jic23@cam.ac.uk>");
MODULE_DESCRIPTION("VTI SCA3000 Series Accelerometers SPI driver");
MODULE_LICENSE("GPL v2");
