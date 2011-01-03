

#ifndef LINUX_HWRANDOM_H_
#define LINUX_HWRANDOM_H_

#include <linux/types.h>
#include <linux/list.h>


struct hwrng {
	const char *name;
	int (*init)(struct hwrng *rng);
	void (*cleanup)(struct hwrng *rng);
	int (*data_present)(struct hwrng *rng, int wait);
	int (*data_read)(struct hwrng *rng, u32 *data);
	unsigned long priv;

	
	struct list_head list;
};


extern int hwrng_register(struct hwrng *rng);

extern void hwrng_unregister(struct hwrng *rng);

#endif 
