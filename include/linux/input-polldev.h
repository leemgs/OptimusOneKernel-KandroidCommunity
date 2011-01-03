#ifndef _INPUT_POLLDEV_H
#define _INPUT_POLLDEV_H



#include <linux/input.h>
#include <linux/workqueue.h>


struct input_polled_dev {
	void *private;

	void (*flush)(struct input_polled_dev *dev);
	void (*poll)(struct input_polled_dev *dev);
	unsigned int poll_interval; 

	struct input_dev *input;
	struct delayed_work work;
};

struct input_polled_dev *input_allocate_polled_device(void);
void input_free_polled_device(struct input_polled_dev *dev);
int input_register_polled_device(struct input_polled_dev *dev);
void input_unregister_polled_device(struct input_polled_dev *dev);

#endif
