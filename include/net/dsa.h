

#ifndef __LINUX_NET_DSA_H
#define __LINUX_NET_DSA_H

#define DSA_MAX_SWITCHES	4
#define DSA_MAX_PORTS		12

struct dsa_chip_data {
	
	struct device	*mii_bus;
	int		sw_addr;

	
	char		*port_names[DSA_MAX_PORTS];

	
	s8		*rtable;
};

struct dsa_platform_data {
	
	struct device	*netdev;

	
	int		nr_chips;
	struct dsa_chip_data	*chip;
};

extern bool dsa_uses_dsa_tags(void *dsa_ptr);
extern bool dsa_uses_trailer_tags(void *dsa_ptr);


#endif
