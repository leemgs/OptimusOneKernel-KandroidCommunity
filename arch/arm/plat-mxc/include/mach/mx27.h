

#ifndef __ASM_ARCH_MXC_MX27_H__
#define __ASM_ARCH_MXC_MX27_H__


#define IRAM_BASE_ADDR          0xFFFF4C00	

#define MSHC_BASE_ADDR          (AIPI_BASE_ADDR + 0x18000)
#define GPT5_BASE_ADDR          (AIPI_BASE_ADDR + 0x19000)
#define GPT4_BASE_ADDR          (AIPI_BASE_ADDR + 0x1A000)
#define UART5_BASE_ADDR         (AIPI_BASE_ADDR + 0x1B000)
#define UART6_BASE_ADDR         (AIPI_BASE_ADDR + 0x1C000)
#define I2C2_BASE_ADDR          (AIPI_BASE_ADDR + 0x1D000)
#define SDHC3_BASE_ADDR         (AIPI_BASE_ADDR + 0x1E000)
#define GPT6_BASE_ADDR          (AIPI_BASE_ADDR + 0x1F000)
#define VPU_BASE_ADDR           (AIPI_BASE_ADDR + 0x23000)
#define OTG_BASE_ADDR           USBOTG_BASE_ADDR
#define SAHARA_BASE_ADDR        (AIPI_BASE_ADDR + 0x25000)
#define IIM_BASE_ADDR           (AIPI_BASE_ADDR + 0x28000)
#define RTIC_BASE_ADDR          (AIPI_BASE_ADDR + 0x2A000)
#define FEC_BASE_ADDR           (AIPI_BASE_ADDR + 0x2B000)
#define SCC_BASE_ADDR           (AIPI_BASE_ADDR + 0x2C000)
#define ETB_BASE_ADDR           (AIPI_BASE_ADDR + 0x3B000)
#define ETB_RAM_BASE_ADDR       (AIPI_BASE_ADDR + 0x3C000)


#define ROMP_BASE_ADDR          0x10041000

#define ATA_BASE_ADDR           (SAHB1_BASE_ADDR + 0x1000)


#define SDRAM_BASE_ADDR         0xA0000000
#define CSD1_BASE_ADDR          0xB0000000

#define CS0_BASE_ADDR           0xC0000000
#define CS1_BASE_ADDR           0xC8000000
#define CS2_BASE_ADDR           0xD0000000
#define CS3_BASE_ADDR           0xD2000000
#define CS4_BASE_ADDR           0xD4000000
#define CS5_BASE_ADDR           0xD6000000
#define PCMCIA_MEM_BASE_ADDR    0xDC000000


#define X_MEMC_BASE_ADDR        0xD8000000
#define X_MEMC_BASE_ADDR_VIRT   0xF4200000
#define X_MEMC_SIZE             SZ_1M

#define NFC_BASE_ADDR           (X_MEMC_BASE_ADDR)
#define SDRAMC_BASE_ADDR        (X_MEMC_BASE_ADDR + 0x1000)
#define WEIM_BASE_ADDR          (X_MEMC_BASE_ADDR + 0x2000)
#define M3IF_BASE_ADDR          (X_MEMC_BASE_ADDR + 0x3000)
#define PCMCIA_CTL_BASE_ADDR    (X_MEMC_BASE_ADDR + 0x4000)


#define MXC_INT_CCM		63
#define MXC_INT_IIM		62
#define MXC_INT_SAHARA		59
#define MXC_INT_SCC_SCM		58
#define MXC_INT_SCC_SMN		57
#define MXC_INT_USB3		56
#define MXC_INT_USB2		55
#define MXC_INT_USB1		54
#define MXC_INT_VPU		53
#define MXC_INT_FEC		50
#define MXC_INT_UART5		49
#define MXC_INT_UART6		48
#define MXC_INT_ATA		30
#define MXC_INT_SDHC3		9
#define MXC_INT_SDHC		7
#define MXC_INT_RTIC		5
#define MXC_INT_GPT4		4
#define MXC_INT_GPT5		3
#define MXC_INT_GPT6		2
#define MXC_INT_I2C2		1


#define DMA_REQ_NFC             37
#define DMA_REQ_SDHC3           36
#define DMA_REQ_UART6_RX        35
#define DMA_REQ_UART6_TX        34
#define DMA_REQ_UART5_RX        33
#define DMA_REQ_UART5_TX        32
#define DMA_REQ_ATA_RCV         29
#define DMA_REQ_ATA_TX          28
#define DMA_REQ_MSHC            4


#define CHIP_REV_1_0		0x00
#define CHIP_REV_2_0		0x01

#ifndef __ASSEMBLY__
extern int mx27_revision(void);
#endif



#endif 
