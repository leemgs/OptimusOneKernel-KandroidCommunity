
#ifndef ASMARM_HARDWARE_ICST525_H
#define ASMARM_HARDWARE_ICST525_H

struct icst525_params {
	unsigned long	ref;
	unsigned long	vco_max;	
	unsigned short	vd_min;		
	unsigned short	vd_max;		
	unsigned char	rd_min;		
	unsigned char	rd_max;		
};

struct icst525_vco {
	unsigned short	v;
	unsigned char	r;
	unsigned char	s;
};

unsigned long icst525_khz(const struct icst525_params *p, struct icst525_vco vco);
struct icst525_vco icst525_khz_to_vco(const struct icst525_params *p, unsigned long freq);
struct icst525_vco icst525_ps_to_vco(const struct icst525_params *p, unsigned long period);

#endif
