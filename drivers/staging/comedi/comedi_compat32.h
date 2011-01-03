

#ifndef _COMEDI_COMPAT32_H
#define _COMEDI_COMPAT32_H

#include <linux/compat.h>
#include <linux/fs.h>		

#ifdef CONFIG_COMPAT

#ifdef HAVE_COMPAT_IOCTL

extern long comedi_compat_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg);
#define comedi_register_ioctl32() do {} while (0)
#define comedi_unregister_ioctl32() do {} while (0)

#else 

#define comedi_compat_ioctl 0	
extern void comedi_register_ioctl32(void);
extern void comedi_unregister_ioctl32(void);

#endif 

#else 

#define comedi_compat_ioctl 0	
#define comedi_register_ioctl32() do {} while (0)
#define comedi_unregister_ioctl32() do {} while (0)

#endif 

#endif 
