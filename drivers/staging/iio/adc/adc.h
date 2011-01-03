

#define IIO_DEV_ATTR_ADC(_num, _show, _addr)			\
  IIO_DEVICE_ATTR(adc_##_num, S_IRUGO, _show, NULL, _addr)
