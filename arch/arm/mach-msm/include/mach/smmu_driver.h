

#ifndef SMMU_DRIVER_H
#define SMMU_DRIVER_H

#include <linux/vcm_types.h>
#include <linux/list.h>

#define FL_OFFSET(va)	(((va) & 0xFFF00000) >> 20)
#define SL_OFFSET(va)	(((va) & 0xFF000) >> 12)

#define NUM_FL_PTE 	4096
#define NUM_SL_PTE	256


struct smmu_driver {
	unsigned long base;
	int irq;
	struct list_head list_active;
};



struct smmu_dev {
	int base;
	int context;
	vcm_handler smmu_vcm_handler;
	void *smmu_vcm_handler_data;
	unsigned long *fl_table;
	struct list_head dev_elm;
	struct smmu_driver *drv;
};

void v7_flush_kern_cache_all(void);


int smmu_drvdata_init(struct smmu_driver *drv, unsigned long base, int irq);



struct smmu_dev *smmu_ctx_init(int ctx);



int smmu_ctx_bind(struct smmu_dev *ctx, struct smmu_driver *drv);



int smmu_ctx_deinit(struct smmu_dev *dev);




struct smmu_driver *smmu_get_driver(struct smmu_dev *dev);



int smmu_activate(struct smmu_dev *dev);



int smmu_deactivate(struct smmu_dev *dev);



int smmu_is_active(struct smmu_dev *dev);



int smmu_update_start(struct smmu_dev *dev);



int smmu_update_done(struct smmu_dev *dev);



int smmu_map(struct smmu_dev *dev, unsigned long pa, unsigned long va,
	     unsigned long len, unsigned int attr);



int __smmu_map(struct smmu_dev *dev, unsigned long pa, unsigned long va,
	       unsigned long len, unsigned int attr);



int smmu_unmap(struct smmu_dev *dev, unsigned long va, unsigned long len);



unsigned long smmu_translate(struct smmu_dev *dev, unsigned long va);



int smmu_hook_irpt(struct smmu_dev *dev, vcm_handler handler, void *data);

#endif 
