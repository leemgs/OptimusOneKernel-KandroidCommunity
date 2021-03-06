

#ifndef __LINUX_OF_SPI_H
#define __LINUX_OF_SPI_H

#include <linux/of.h>
#include <linux/spi/spi.h>

extern void of_register_spi_devices(struct spi_master *master,
				    struct device_node *np);

#endif 
