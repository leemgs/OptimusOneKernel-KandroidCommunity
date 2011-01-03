

#ifndef __ASM_ARM_ARCH_OMAP_PRCM_H
#define __ASM_ARM_ARCH_OMAP_PRCM_H

u32 omap_prcm_get_reset_sources(void);
void omap_prcm_arch_reset(char mode);
int omap2_cm_wait_idlest(void __iomem *reg, u32 mask, const char *name);

#endif





