

#ifndef _LINUX_TIMED_OUTPUT_H
#define _LINUX_TIMED_OUTPUT_H

struct timed_output_dev {
	const char	*name;

	
	void	(*enable)(struct timed_output_dev *sdev, int timeout);

	
	int		(*get_time)(struct timed_output_dev *sdev);

	
	struct device	*dev;
	int		index;
	int		state;
};

extern int timed_output_dev_register(struct timed_output_dev *dev);
extern void timed_output_dev_unregister(struct timed_output_dev *dev);

#endif
