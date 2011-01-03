

#ifndef _8255_H
#define _8255_H

#include "../comedidev.h"

#if defined(CONFIG_COMEDI_8255) || defined(CONFIG_COMEDI_8255_MODULE)

int subdev_8255_init(struct comedi_device *dev, struct comedi_subdevice *s,
		     int (*cb) (int, int, int, unsigned long),
		     unsigned long arg);
int subdev_8255_init_irq(struct comedi_device *dev, struct comedi_subdevice *s,
			 int (*cb) (int, int, int, unsigned long),
			 unsigned long arg);
void subdev_8255_cleanup(struct comedi_device *dev, struct comedi_subdevice *s);
void subdev_8255_interrupt(struct comedi_device *dev,
			   struct comedi_subdevice *s);

#else

static inline int subdev_8255_init(struct comedi_device *dev,
				   struct comedi_subdevice *s, void *x,
				   unsigned long y)
{
	printk("8255 support not configured -- disabling subdevice\n");

	s->type = COMEDI_SUBD_UNUSED;

	return 0;
}

static inline void subdev_8255_cleanup(struct comedi_device *dev,
				       struct comedi_subdevice *s)
{
}

#endif

#endif
