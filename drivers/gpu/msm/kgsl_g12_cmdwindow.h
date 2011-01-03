
#ifndef _GSL_CMDWINDOW_H
#define _GSL_CMDWINDOW_H

struct kgsl_device;

int kgsl_g12_cmdwindow_init(struct kgsl_device *device);
int kgsl_g12_cmdwindow_close(struct kgsl_device *device);
int kgsl_g12_cmdwindow_write(struct kgsl_device *device,
		enum kgsl_cmdwindow_type target, unsigned int addr,
		unsigned int data);

#endif 
