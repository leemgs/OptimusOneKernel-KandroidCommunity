

#ifndef __THERMAL_H__
#define __THERMAL_H__

#include <linux/idr.h>
#include <linux/device.h>
#include <linux/workqueue.h>

struct thermal_zone_device;
struct thermal_cooling_device;

enum thermal_device_mode {
	THERMAL_DEVICE_DISABLED = 0,
	THERMAL_DEVICE_ENABLED,
};

enum thermal_trip_type {
	THERMAL_TRIP_ACTIVE = 0,
	THERMAL_TRIP_PASSIVE,
	THERMAL_TRIP_HOT,
	THERMAL_TRIP_CRITICAL,
};

struct thermal_zone_device_ops {
	int (*bind) (struct thermal_zone_device *,
		     struct thermal_cooling_device *);
	int (*unbind) (struct thermal_zone_device *,
		       struct thermal_cooling_device *);
	int (*get_temp) (struct thermal_zone_device *, unsigned long *);
	int (*get_mode) (struct thermal_zone_device *,
			 enum thermal_device_mode *);
	int (*set_mode) (struct thermal_zone_device *,
		enum thermal_device_mode);
	int (*get_trip_type) (struct thermal_zone_device *, int,
		enum thermal_trip_type *);
	int (*get_trip_temp) (struct thermal_zone_device *, int,
			      unsigned long *);
	int (*get_crit_temp) (struct thermal_zone_device *, unsigned long *);
	int (*notify) (struct thermal_zone_device *, int,
		       enum thermal_trip_type);
};

struct thermal_cooling_device_ops {
	int (*get_max_state) (struct thermal_cooling_device *, unsigned long *);
	int (*get_cur_state) (struct thermal_cooling_device *, unsigned long *);
	int (*set_cur_state) (struct thermal_cooling_device *, unsigned long);
};

#define THERMAL_TRIPS_NONE -1
#define THERMAL_MAX_TRIPS 12
#define THERMAL_NAME_LENGTH 20
struct thermal_cooling_device {
	int id;
	char type[THERMAL_NAME_LENGTH];
	struct device device;
	void *devdata;
	struct thermal_cooling_device_ops *ops;
	struct list_head node;
};

#define KELVIN_TO_CELSIUS(t)	(long)(((long)t-2732 >= 0) ?	\
				((long)t-2732+5)/10 : ((long)t-2732-5)/10)
#define CELSIUS_TO_KELVIN(t)	((t)*10+2732)

#if defined(CONFIG_THERMAL_HWMON)

struct thermal_hwmon_device {
	char type[THERMAL_NAME_LENGTH];
	struct device *device;
	int count;
	struct list_head tz_list;
	struct list_head node;
};

struct thermal_hwmon_attr {
	struct device_attribute attr;
	char name[16];
};
#endif

struct thermal_zone_device {
	int id;
	char type[THERMAL_NAME_LENGTH];
	struct device device;
	void *devdata;
	int trips;
	int tc1;
	int tc2;
	int passive_delay;
	int polling_delay;
	int last_temperature;
	bool passive;
	unsigned int forced_passive;
	struct thermal_zone_device_ops *ops;
	struct list_head cooling_devices;
	struct idr idr;
	struct mutex lock;	
	struct list_head node;
	struct delayed_work poll_queue;
#if defined(CONFIG_THERMAL_HWMON)
	struct list_head hwmon_node;
	struct thermal_hwmon_device *hwmon;
	struct thermal_hwmon_attr temp_input;	
	struct thermal_hwmon_attr temp_crit;	
#endif
};

struct thermal_zone_device *thermal_zone_device_register(char *, int, void *,
							 struct
							 thermal_zone_device_ops
							 *, int tc1, int tc2,
							 int passive_freq,
							 int polling_freq);
void thermal_zone_device_unregister(struct thermal_zone_device *);

int thermal_zone_bind_cooling_device(struct thermal_zone_device *, int,
				     struct thermal_cooling_device *);
int thermal_zone_unbind_cooling_device(struct thermal_zone_device *, int,
				       struct thermal_cooling_device *);
void thermal_zone_device_update(struct thermal_zone_device *);
struct thermal_cooling_device *thermal_cooling_device_register(char *, void *,
							       struct
							       thermal_cooling_device_ops
							       *);
void thermal_cooling_device_unregister(struct thermal_cooling_device *);

#endif 
