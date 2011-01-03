

#ifndef _ASM_X86_AMD_IOMMU_H
#define _ASM_X86_AMD_IOMMU_H

#include <linux/irqreturn.h>

#ifdef CONFIG_AMD_IOMMU
extern int amd_iommu_init(void);
extern int amd_iommu_init_dma_ops(void);
extern int amd_iommu_init_passthrough(void);
extern void amd_iommu_detect(void);
extern irqreturn_t amd_iommu_int_handler(int irq, void *data);
extern void amd_iommu_flush_all_domains(void);
extern void amd_iommu_flush_all_devices(void);
extern void amd_iommu_shutdown(void);
extern void amd_iommu_apply_erratum_63(u16 devid);
extern void amd_iommu_init_api(void);
#else
static inline int amd_iommu_init(void) { return -ENODEV; }
static inline void amd_iommu_detect(void) { }
static inline void amd_iommu_shutdown(void) { }
#endif

#endif 
