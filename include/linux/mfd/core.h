

#ifndef MFD_CORE_H
#define MFD_CORE_H

#include <linux/platform_device.h>


struct mfd_cell {
	const char		*name;
	int			id;

	int			(*enable)(struct platform_device *dev);
	int			(*disable)(struct platform_device *dev);
	int			(*suspend)(struct platform_device *dev);
	int			(*resume)(struct platform_device *dev);

	
	void			*driver_data;

	
	void			*platform_data;
	size_t			data_size;

	
	int			num_resources;
	const struct resource	*resources;
};

extern int mfd_add_devices(struct device *parent, int id,
			   const struct mfd_cell *cells, int n_devs,
			   struct resource *mem_base,
			   int irq_base);

extern void mfd_remove_devices(struct device *parent);

#endif
