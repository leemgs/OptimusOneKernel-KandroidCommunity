

#ifndef __ASM_ARCH_MSM_IOMAP_7XXX_H
#define __ASM_ARCH_MSM_IOMAP_7XXX_H



#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_VIC_BASE          IOMEM(0xF0000000)
#else	
#define MSM_VIC_BASE          IOMEM(0xE0000000)
#endif
#define MSM_VIC_PHYS          0xC0000000
#define MSM_VIC_SIZE          SZ_4K

#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_CSR_BASE          IOMEM(0xF0001000)
#else	
#define MSM_CSR_BASE          IOMEM(0xE0001000)
#endif
#define MSM_CSR_PHYS          0xC0100000
#define MSM_CSR_SIZE          SZ_4K

#define MSM_TMR_PHYS          MSM_CSR_PHYS
#define MSM_TMR_BASE          MSM_CSR_BASE
#define MSM_TMR_SIZE          SZ_4K

#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_DMOV_BASE         IOMEM(0xF0002000)
#else	
#define MSM_DMOV_BASE         IOMEM(0xE0002000)
#endif
#define MSM_DMOV_PHYS         0xA9700000
#define MSM_DMOV_SIZE         SZ_4K

#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_GPIO1_BASE        IOMEM(0xF0003000)
#else	
#define MSM_GPIO1_BASE        IOMEM(0xE0003000)
#endif
#define MSM_GPIO1_PHYS        0xA9200000
#define MSM_GPIO1_SIZE        SZ_4K

#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_GPIO2_BASE        IOMEM(0xF0004000)
#else	
#define MSM_GPIO2_BASE        IOMEM(0xE0004000)
#endif
#define MSM_GPIO2_PHYS        0xA9300000
#define MSM_GPIO2_SIZE        SZ_4K

#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_CLK_CTL_BASE      IOMEM(0xF0005000)
#else	
#define MSM_CLK_CTL_BASE      IOMEM(0xE0005000)
#endif
#define MSM_CLK_CTL_PHYS      0xA8600000
#define MSM_CLK_CTL_SIZE      SZ_4K

#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_L2CC_BASE         IOMEM(0xF0006000)
#else	
#define MSM_L2CC_BASE         IOMEM(0xE0006000)
#endif
#define MSM_L2CC_PHYS         0xC0400000
#define MSM_L2CC_SIZE         SZ_4K

#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_SHARED_RAM_BASE   IOMEM(0xF0100000)
#else	
#define MSM_SHARED_RAM_BASE   IOMEM(0xE0100000)
#endif
#define MSM_SHARED_RAM_SIZE   SZ_1M

#define MSM_UART1_PHYS        0xA9A00000
#define MSM_UART1_SIZE        SZ_4K

#define MSM_UART2_PHYS        0xA9B00000
#define MSM_UART2_SIZE        SZ_4K

#define MSM_UART3_PHYS        0xA9C00000
#define MSM_UART3_SIZE        SZ_4K

#ifdef CONFIG_MSM_DEBUG_UART
#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_DEBUG_UART_BASE   0xF1000000
#else
#define MSM_DEBUG_UART_BASE   0xE1000000
#endif
#if CONFIG_MSM_DEBUG_UART == 1
#define MSM_DEBUG_UART_PHYS   MSM_UART1_PHYS
#elif CONFIG_MSM_DEBUG_UART == 2
#define MSM_DEBUG_UART_PHYS   MSM_UART2_PHYS
#elif CONFIG_MSM_DEBUG_UART == 3
#define MSM_DEBUG_UART_PHYS   MSM_UART3_PHYS
#endif
#define MSM_DEBUG_UART_SIZE   SZ_4K
#endif

#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_MDC_BASE	      IOMEM(0xF0200000)
#else	
#define MSM_MDC_BASE	      IOMEM(0xE0200000)
#endif
#define MSM_MDC_PHYS	      0xAA500000
#define MSM_MDC_SIZE	      SZ_1M

#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_AD5_BASE          IOMEM(0xF0300000)
#else	
#define MSM_AD5_BASE          IOMEM(0xE0300000)
#endif
#define MSM_AD5_PHYS          0xAC000000
#define MSM_AD5_SIZE          (SZ_1M*13)

#if defined(CONFIG_MACH_LGE)

#if !defined(CONFIG_VMSPLIT_2G) && defined (CONFIG_LGE_4G_DDR)


#define MSM_WEB_BASE          IOMEM(0xF100C000)
#else	
#define MSM_WEB_BASE          IOMEM(0xE100C000)
#endif
#define MSM_WEB_PHYS          0xA9D00040 
#define MSM_WEB_SIZE          SZ_4K
#endif 

#endif
