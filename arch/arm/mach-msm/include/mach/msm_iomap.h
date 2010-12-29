

#ifndef __ASM_ARCH_MSM_IOMAP_H
#define __ASM_ARCH_MSM_IOMAP_H

#include <asm/sizes.h>



#ifdef __ASSEMBLY__
#define IOMEM(x)	x
#else
#define IOMEM(x)	((void __force __iomem *)(x))
#endif

#if defined(CONFIG_ARCH_MSM7X30)
#include "msm_iomap-7x30.h"
#elif defined(CONFIG_ARCH_QSD8X50)
#include "msm_iomap-8x50.h"
#elif defined(CONFIG_ARCH_MSM8X60)
#include "msm_iomap-8x60.h"
#else
#include "msm_iomap-7xxx.h"
#endif

#endif
