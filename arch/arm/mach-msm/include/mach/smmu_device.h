

#ifndef SMMU_DEVICE_H
#define SMMU_DEVICE_H

#include <mach/smmu_driver.h>


#define MAX_NUM_MIDS	16


struct smmu_device {
	char *name;
	char *clk;
	unsigned long clk_rate;
	unsigned int num_ctx;
};


struct smmu_ctx {
	char *name;
	int num;
	int mids[MAX_NUM_MIDS];
};


struct smmu_dev *smmu_get_ctx_instance(char *ctx_name);


int smmu_free_ctx_instance(struct smmu_dev *dev);



unsigned long smmu_get_base_addr(struct smmu_dev *dev);

#endif
