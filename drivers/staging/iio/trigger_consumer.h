


#ifdef CONFIG_IIO_TRIGGER

int iio_device_register_trigger_consumer(struct iio_dev *dev_info);

int iio_device_unregister_trigger_consumer(struct iio_dev *dev_info);

#else


int iio_device_register_trigger_consumer(struct iio_dev *dev_info)
{
	return 0;
};

int iio_device_unregister_trigger_consumer(struct iio_dev *dev_info)
{
	return 0;
};

#endif 



