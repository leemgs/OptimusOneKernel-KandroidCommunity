
#include "../sysfs.h"



#define IIO_DEV_ATTR_ACCEL_X_OFFSET(_mode, _show, _store, _addr)	\
	IIO_DEVICE_ATTR(accel_x_offset, _mode, _show, _store, _addr)

#define IIO_DEV_ATTR_ACCEL_Y_OFFSET(_mode, _show, _store, _addr)	\
	IIO_DEVICE_ATTR(accel_y_offset, _mode, _show, _store, _addr)

#define IIO_DEV_ATTR_ACCEL_Z_OFFSET(_mode, _show, _store, _addr)	\
	IIO_DEVICE_ATTR(accel_z_offset, _mode, _show, _store, _addr)

#define IIO_DEV_ATTR_ACCEL_X_GAIN(_mode, _show, _store, _addr)		\
	IIO_DEVICE_ATTR(accel_x_gain, _mode, _show, _store, _addr)

#define IIO_DEV_ATTR_ACCEL_Y_GAIN(_mode, _show, _store, _addr)		\
	IIO_DEVICE_ATTR(accel_y_gain, _mode, _show, _store, _addr)

#define IIO_DEV_ATTR_ACCEL_Z_GAIN(_mode, _show, _store, _addr)		\
	IIO_DEVICE_ATTR(accel_z_gain, _mode, _show, _store, _addr)

#define IIO_DEV_ATTR_ACCEL_X(_show, _addr)			\
	IIO_DEVICE_ATTR(accel_x, S_IRUGO, _show, NULL, _addr)

#define IIO_DEV_ATTR_ACCEL_Y(_show, _addr)			\
	IIO_DEVICE_ATTR(accel_y, S_IRUGO, _show, NULL, _addr)

#define IIO_DEV_ATTR_ACCEL_Z(_show, _addr)			\
	IIO_DEVICE_ATTR(accel_z, S_IRUGO, _show, NULL, _addr)





#define IIO_DEV_ATTR_ACCEL_THRESH(_mode, _show, _store, _addr)	\
	IIO_DEVICE_ATTR(thresh, _mode, _show, _store, _addr)


#define IIO_DEV_ATTR_ACCEL_THRESH_X(_mode, _show, _store, _addr)	\
	IIO_DEVICE_ATTR(thresh_accel_x, _mode, _show, _store, _addr)

#define IIO_DEV_ATTR_ACCEL_THRESH_Y(_mode, _show, _store, _addr)	\
	IIO_DEVICE_ATTR(thresh_accel_y, _mode, _show, _store, _addr)

#define IIO_DEV_ATTR_ACCEL_THRESH_Z(_mode, _show, _store, _addr)	\
	IIO_DEVICE_ATTR(thresh_accel_z, _mode, _show, _store, _addr)



#define IIO_EVENT_ATTR_ACCEL_X_HIGH(_show, _store, _mask, _handler)	\
	IIO_EVENT_ATTR(accel_x_high, _show, _store, _mask, _handler)


#define IIO_EVENT_ATTR_ACCEL_X_HIGH_SH(_evlist, _show, _store, _mask)	\
	IIO_EVENT_ATTR_SH(accel_x_high, _evlist, _show, _store, _mask)


#define IIO_EVENT_CODE_ACCEL_X_HIGH IIO_EVENT_CODE_ACCEL_BASE

#define IIO_EVENT_ATTR_ACCEL_Y_HIGH(_show, _store, _mask, _handler)	\
	IIO_EVENT_ATTR(accel_y_high, _show, _store, _mask, _handler)

#define IIO_EVENT_ATTR_ACCEL_Y_HIGH_SH(_evlist, _show, _store, _mask)	\
	IIO_EVENT_ATTR_SH(accel_y_high, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_Y_HIGH (IIO_EVENT_CODE_ACCEL_BASE + 1)

#define IIO_EVENT_ATTR_ACCEL_Z_HIGH(_show, _store, _mask, _handler)	\
	IIO_EVENT_ATTR(accel_z_high, _show, _store, _mask, _handler)

#define IIO_EVENT_ATTR_ACCEL_Z_HIGH_SH(_evlist, _show, _store, _mask)	\
	IIO_EVENT_ATTR_SH(accel_z_high, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_Z_HIGH (IIO_EVENT_CODE_ACCEL_BASE + 2)

#define IIO_EVENT_ATTR_ACCEL_X_LOW(_show, _store, _mask, _handler)	\
	IIO_EVENT_ATTR(accel_x_low, _show, _store, _mask, _handler)

#define IIO_EVENT_ATTR_ACCEL_X_LOW_SH(_evlist, _show, _store, _mask)	\
	IIO_EVENT_ATTR_SH(accel_x_low, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_X_LOW (IIO_EVENT_CODE_ACCEL_BASE + 3)

#define IIO_EVENT_ATTR_ACCEL_Y_LOW(_show, _store, _mask, _handler) \
	IIO_EVENT_ATTR(accel_y_low, _show, _store, _mask, _handler)

#define IIO_EVENT_ATTR_ACCEL_Y_LOW_SH(_evlist, _show, _store, _mask)\
	IIO_EVENT_ATTR_SH(accel_y_low, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_Y_LOW (IIO_EVENT_CODE_ACCEL_BASE + 4)

#define IIO_EVENT_ATTR_ACCEL_Z_LOW(_show, _store, _mask, _handler)	\
	IIO_EVENT_ATTR(accel_z_low, _show, _store, _mask, _handler)

#define IIO_EVENT_ATTR_ACCEL_Z_LOW_SH(_evlist, _show, _store, _mask)	\
	IIO_EVENT_ATTR_SH(accel_z_low, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_Z_LOW (IIO_EVENT_CODE_ACCEL_BASE + 5)

#define IIO_EVENT_ATTR_FREE_FALL_DETECT(_show, _store, _mask, _handler)	\
	IIO_EVENT_ATTR(free_fall, _show, _store, _mask, _handler)

#define IIO_EVENT_ATTR_FREE_FALL_DETECT_SH(_evlist, _show, _store, _mask) \
	IIO_EVENT_ATTR_SH(free_fall, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_FREE_FALL (IIO_EVENT_CODE_ACCEL_BASE + 6)


#define IIO_EVENT_ATTR_ACCEL_X_ROC_HIGH_SH(_evlist, _show, _store, _mask) \
	IIO_EVENT_ATTR_SH(accel_x_roc_high, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_X_ROC_HIGH (IIO_EVENT_CODE_ACCEL_BASE + 10)

#define IIO_EVENT_ATTR_ACCEL_X_ROC_LOW_SH(_evlist, _show, _store, _mask) \
	IIO_EVENT_ATTR_SH(accel_x_roc_low, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_X_ROC_LOW (IIO_EVENT_CODE_ACCEL_BASE + 11)

#define IIO_EVENT_ATTR_ACCEL_Y_ROC_HIGH_SH(_evlist, _show, _store, _mask) \
	IIO_EVENT_ATTR_SH(accel_y_roc_high, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_Y_ROC_HIGH (IIO_EVENT_CODE_ACCEL_BASE + 12)

#define IIO_EVENT_ATTR_ACCEL_Y_ROC_LOW_SH(_evlist, _show, _store, _mask) \
	IIO_EVENT_ATTR_SH(accel_y_roc_low, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_Y_ROC_LOW (IIO_EVENT_CODE_ACCEL_BASE + 13)

#define IIO_EVENT_ATTR_ACCEL_Z_ROC_HIGH_SH(_evlist, _show, _store, _mask) \
	IIO_EVENT_ATTR_SH(accel_z_roc_high, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_Z_ROC_HIGH (IIO_EVENT_CODE_ACCEL_BASE + 14)

#define IIO_EVENT_ATTR_ACCEL_Z_ROC_LOW_SH(_evlist, _show, _store, _mask) \
	IIO_EVENT_ATTR_SH(accel_z_roc_low, _evlist, _show, _store, _mask)

#define IIO_EVENT_CODE_ACCEL_Z_ROC_LOW (IIO_EVENT_CODE_ACCEL_BASE + 15)
