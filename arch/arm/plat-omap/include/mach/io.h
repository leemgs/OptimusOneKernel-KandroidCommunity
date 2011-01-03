

#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

#include <mach/hardware.h>

#define IO_SPACE_LIMIT 0xffffffff


#define __io(a)		__typesafe_io(a)
#define __mem_pci(a)	(a)



#ifdef __ASSEMBLER__
#define IOMEM(x)		(x)
#else
#define IOMEM(x)		((void __force __iomem *)(x))
#endif

#define OMAP1_IO_OFFSET		0x01000000	
#define OMAP1_IO_ADDRESS(pa)	IOMEM((pa) - OMAP1_IO_OFFSET)

#define OMAP2_IO_OFFSET		0x90000000
#define OMAP2_IO_ADDRESS(pa)	IOMEM((pa) + OMAP2_IO_OFFSET) 



#define OMAP1_IO_PHYS		0xFFFB0000
#define OMAP1_IO_SIZE		0x40000
#define OMAP1_IO_VIRT		(OMAP1_IO_PHYS - OMAP1_IO_OFFSET)




#define L3_24XX_PHYS	L3_24XX_BASE	
#define L3_24XX_VIRT	0xf8000000
#define L3_24XX_SIZE	SZ_1M		
#define L4_24XX_PHYS	L4_24XX_BASE	
#define L4_24XX_VIRT	0xd8000000
#define L4_24XX_SIZE	SZ_1M		

#define L4_WK_243X_PHYS		L4_WK_243X_BASE		
#define L4_WK_243X_VIRT		0xd9000000
#define L4_WK_243X_SIZE		SZ_1M
#define OMAP243X_GPMC_PHYS	OMAP243X_GPMC_BASE	
#define OMAP243X_GPMC_VIRT	0xFE000000
#define OMAP243X_GPMC_SIZE	SZ_1M
#define OMAP243X_SDRC_PHYS	OMAP243X_SDRC_BASE
#define OMAP243X_SDRC_VIRT	0xFD000000
#define OMAP243X_SDRC_SIZE	SZ_1M
#define OMAP243X_SMS_PHYS	OMAP243X_SMS_BASE
#define OMAP243X_SMS_VIRT	0xFC000000
#define OMAP243X_SMS_SIZE	SZ_1M


#define DSP_MEM_24XX_PHYS	OMAP2420_DSP_MEM_BASE	
#define DSP_MEM_24XX_VIRT	0xe0000000
#define DSP_MEM_24XX_SIZE	0x28000
#define DSP_IPI_24XX_PHYS	OMAP2420_DSP_IPI_BASE	
#define DSP_IPI_24XX_VIRT	0xe1000000
#define DSP_IPI_24XX_SIZE	SZ_4K
#define DSP_MMU_24XX_PHYS	OMAP2420_DSP_MMU_BASE	
#define DSP_MMU_24XX_VIRT	0xe2000000
#define DSP_MMU_24XX_SIZE	SZ_4K




#define L3_34XX_PHYS		L3_34XX_BASE	
#define L3_34XX_VIRT		0xf8000000
#define L3_34XX_SIZE		SZ_1M   

#define L4_34XX_PHYS		L4_34XX_BASE	
#define L4_34XX_VIRT		0xd8000000
#define L4_34XX_SIZE		SZ_4M   



#define L4_WK_34XX_PHYS		L4_WK_34XX_BASE 
#define L4_WK_34XX_VIRT		0xd8300000
#define L4_WK_34XX_SIZE		SZ_1M

#define L4_PER_34XX_PHYS	L4_PER_34XX_BASE 
#define L4_PER_34XX_VIRT	0xd9000000
#define L4_PER_34XX_SIZE	SZ_1M

#define L4_EMU_34XX_PHYS	L4_EMU_34XX_BASE 
#define L4_EMU_34XX_VIRT	0xe4000000
#define L4_EMU_34XX_SIZE	SZ_64M

#define OMAP34XX_GPMC_PHYS	OMAP34XX_GPMC_BASE 
#define OMAP34XX_GPMC_VIRT	0xFE000000
#define OMAP34XX_GPMC_SIZE	SZ_1M

#define OMAP343X_SMS_PHYS	OMAP343X_SMS_BASE 
#define OMAP343X_SMS_VIRT	0xFC000000
#define OMAP343X_SMS_SIZE	SZ_1M

#define OMAP343X_SDRC_PHYS	OMAP343X_SDRC_BASE 
#define OMAP343X_SDRC_VIRT	0xFD000000
#define OMAP343X_SDRC_SIZE	SZ_1M


#define DSP_MEM_34XX_PHYS	OMAP34XX_DSP_MEM_BASE	
#define DSP_MEM_34XX_VIRT	0xe0000000
#define DSP_MEM_34XX_SIZE	0x28000
#define DSP_IPI_34XX_PHYS	OMAP34XX_DSP_IPI_BASE	
#define DSP_IPI_34XX_VIRT	0xe1000000
#define DSP_IPI_34XX_SIZE	SZ_4K
#define DSP_MMU_34XX_PHYS	OMAP34XX_DSP_MMU_BASE	
#define DSP_MMU_34XX_VIRT	0xe2000000
#define DSP_MMU_34XX_SIZE	SZ_4K




#define L3_44XX_PHYS		L3_44XX_BASE
#define L3_44XX_VIRT		0xd4000000
#define L3_44XX_SIZE		SZ_1M

#define L4_44XX_PHYS		L4_44XX_BASE
#define L4_44XX_VIRT		0xda000000
#define L4_44XX_SIZE		SZ_4M


#define L4_WK_44XX_PHYS		L4_WK_44XX_BASE
#define L4_WK_44XX_VIRT		0xda300000
#define L4_WK_44XX_SIZE		SZ_1M

#define L4_PER_44XX_PHYS	L4_PER_44XX_BASE
#define L4_PER_44XX_VIRT	0xd8000000
#define L4_PER_44XX_SIZE	SZ_4M

#define L4_EMU_44XX_PHYS	L4_EMU_44XX_BASE
#define L4_EMU_44XX_VIRT	0xe4000000
#define L4_EMU_44XX_SIZE	SZ_64M

#define OMAP44XX_GPMC_PHYS	OMAP44XX_GPMC_BASE
#define OMAP44XX_GPMC_VIRT	0xe0000000
#define OMAP44XX_GPMC_SIZE	SZ_1M




#ifndef __ASSEMBLER__



extern u8 omap_readb(u32 pa);
extern u16 omap_readw(u32 pa);
extern u32 omap_readl(u32 pa);
extern void omap_writeb(u8 v, u32 pa);
extern void omap_writew(u16 v, u32 pa);
extern void omap_writel(u32 v, u32 pa);

struct omap_sdrc_params;

extern void omap1_map_common_io(void);
extern void omap1_init_common_hw(void);

extern void omap2_map_common_io(void);
extern void omap2_init_common_hw(struct omap_sdrc_params *sdrc_cs0,
				 struct omap_sdrc_params *sdrc_cs1);

#define __arch_ioremap(p,s,t)	omap_ioremap(p,s,t)
#define __arch_iounmap(v)	omap_iounmap(v)

void __iomem *omap_ioremap(unsigned long phys, size_t size, unsigned int type);
void omap_iounmap(volatile void __iomem *addr);

#endif

#endif
