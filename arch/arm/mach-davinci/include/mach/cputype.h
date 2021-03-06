
#ifndef _ASM_ARCH_CPU_H
#define _ASM_ARCH_CPU_H

#include <mach/common.h>

struct davinci_id {
	u8	variant;	
	u16	part_no;	
	u16	manufacturer;	
	u32	cpu_id;
	char	*name;
};


#define	DAVINCI_CPU_ID_DM6446		0x64460000
#define	DAVINCI_CPU_ID_DM6467		0x64670000
#define	DAVINCI_CPU_ID_DM355		0x03550000
#define	DAVINCI_CPU_ID_DM365		0x03650000
#define	DAVINCI_CPU_ID_DA830		0x08300000
#define	DAVINCI_CPU_ID_DA850		0x08500000

#define IS_DAVINCI_CPU(type, id)					\
static inline int is_davinci_ ##type(void)				\
{									\
	return (davinci_soc_info.cpu_id == (id));			\
}

IS_DAVINCI_CPU(dm644x, DAVINCI_CPU_ID_DM6446)
IS_DAVINCI_CPU(dm646x, DAVINCI_CPU_ID_DM6467)
IS_DAVINCI_CPU(dm355, DAVINCI_CPU_ID_DM355)
IS_DAVINCI_CPU(dm365, DAVINCI_CPU_ID_DM365)
IS_DAVINCI_CPU(da830, DAVINCI_CPU_ID_DA830)
IS_DAVINCI_CPU(da850, DAVINCI_CPU_ID_DA850)

#ifdef CONFIG_ARCH_DAVINCI_DM644x
#define cpu_is_davinci_dm644x() is_davinci_dm644x()
#else
#define cpu_is_davinci_dm644x() 0
#endif

#ifdef CONFIG_ARCH_DAVINCI_DM646x
#define cpu_is_davinci_dm646x() is_davinci_dm646x()
#else
#define cpu_is_davinci_dm646x() 0
#endif

#ifdef CONFIG_ARCH_DAVINCI_DM355
#define cpu_is_davinci_dm355() is_davinci_dm355()
#else
#define cpu_is_davinci_dm355() 0
#endif

#ifdef CONFIG_ARCH_DAVINCI_DM365
#define cpu_is_davinci_dm365() is_davinci_dm365()
#else
#define cpu_is_davinci_dm365() 0
#endif

#ifdef CONFIG_ARCH_DAVINCI_DA830
#define cpu_is_davinci_da830() is_davinci_da830()
#else
#define cpu_is_davinci_da830() 0
#endif

#ifdef CONFIG_ARCH_DAVINCI_DA850
#define cpu_is_davinci_da850() is_davinci_da850()
#else
#define cpu_is_davinci_da850() 0
#endif

#endif
