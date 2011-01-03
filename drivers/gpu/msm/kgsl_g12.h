
#ifndef _KGSL_G12_H
#define _KGSL_G12_H

struct kgsl_g12_device {
	struct kgsl_device dev;    
	int current_timestamp;
	int timestamp;
	wait_queue_head_t wait_timestamp_wq;
};

struct kgsl_g12_z1xx {
	unsigned int prevctx;
	unsigned int numcontext;
	struct kgsl_memdesc      cmdbufdesc;
};

extern struct kgsl_g12_z1xx g_z1xx;

irqreturn_t kgsl_g12_isr(int irq, void *data);
int kgsl_g12_setstate(struct kgsl_device *device, uint32_t flags);
struct kgsl_device *kgsl_get_g12_generic_device(void);
int kgsl_g12_regread(struct kgsl_device *device, unsigned int offsetwords,
				unsigned int *value);
int kgsl_g12_regwrite(struct kgsl_device *device, unsigned int offsetwords,
			unsigned int value);
int __init kgsl_g12_config(struct kgsl_devconfig *,
				struct platform_device *pdev);
int kgsl_g12_getfunctable(struct kgsl_functable *ftbl);


#endif 
