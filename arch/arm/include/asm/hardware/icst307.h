
#ifndef ASMARM_HARDWARE_ICST307_H
#define ASMARM_HARDWARE_ICST307_H

struct icst307_params {
	unsigned long	ref;
	unsigned long	vco_max;	
	unsigned short	vd_min;		
	unsigned short	vd_max;		
	unsigned char	rd_min;		
	unsigned char	rd_max;		
};

struct icst307_vco {
	unsigned short	v;
	unsigned char	r;
	unsigned char	s;
};

unsigned long icst307_khz(const struct icst307_params *p, struct icst307_vco vco);
struct icst307_vco icst307_khz_to_vco(const struct icst307_params *p, unsigned long freq);
struct icst307_vco icst307_ps_to_vco(const struct icst307_params *p, unsigned long period);

#endif
