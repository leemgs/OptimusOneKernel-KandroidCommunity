#ifndef __ASM_SH_MOBILE_CEU_H__
#define __ASM_SH_MOBILE_CEU_H__

#define SH_CEU_FLAG_USE_8BIT_BUS	(1 << 0) 
#define SH_CEU_FLAG_USE_16BIT_BUS	(1 << 1) 

struct sh_mobile_ceu_info {
	unsigned long flags;
};

#endif 
