#include <linux/i2c.h>
#include <linux/input-polldev.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/of_device.h>

enum ams_irq {
	AMS_IRQ_FREEFALL = 0x01,
	AMS_IRQ_SHOCK = 0x02,
	AMS_IRQ_GLOBAL = 0x04,
	AMS_IRQ_ALL =
		AMS_IRQ_FREEFALL |
		AMS_IRQ_SHOCK |
		AMS_IRQ_GLOBAL,
};

struct ams {
	
	spinlock_t irq_lock;
	struct mutex lock;

	
	struct device_node *of_node;
	struct of_device *of_dev;
	char has_device;
	char vflag;
	u32 orient1;
	u32 orient2;

	
	struct work_struct worker;
	u8 worker_irqs;

	
	void (*exit)(void);

	void (*get_xyz)(s8 *x, s8 *y, s8 *z);
	u8 (*get_vendor)(void);

	void (*clear_irq)(enum ams_irq reg);

#ifdef CONFIG_SENSORS_AMS_I2C
	
	struct i2c_client *i2c_client;
#endif

	
	struct input_polled_dev *idev;
	__u16 bustype;

	
	int xcalib, ycalib, zcalib;
};

extern struct ams ams_info;

extern void ams_sensors(s8 *x, s8 *y, s8 *z);
extern int ams_sensor_attach(void);

extern int ams_pmu_init(struct device_node *np);
extern int ams_i2c_init(struct device_node *np);

extern int ams_input_init(void);
extern void ams_input_exit(void);
