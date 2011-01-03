

#include <linux/completion.h>

struct timeriomem_rng_data {
	struct completion	completion;
	unsigned int		present:1;

	void __iomem		*address;

	
	unsigned int		period;
};
