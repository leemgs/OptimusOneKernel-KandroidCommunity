

#ifndef __ASM_ARCH_MXC_MX21_H__
#define __ASM_ARCH_MXC_MX21_H__


#define SDRAM_BASE_ADDR         0xC0000000
#define CSD1_BASE_ADDR          0xC4000000

#define CS0_BASE_ADDR           0xC8000000
#define CS1_BASE_ADDR           0xCC000000
#define CS2_BASE_ADDR           0xD0000000
#define CS3_BASE_ADDR           0xD1000000
#define CS4_BASE_ADDR           0xD2000000
#define CS5_BASE_ADDR           0xDD000000
#define PCMCIA_MEM_BASE_ADDR    0xD4000000


#define X_MEMC_BASE_ADDR        0xDF000000
#define X_MEMC_BASE_ADDR_VIRT   0xF4200000
#define X_MEMC_SIZE             SZ_256K

#define SDRAMC_BASE_ADDR        (X_MEMC_BASE_ADDR + 0x0000)
#define EIM_BASE_ADDR           (X_MEMC_BASE_ADDR + 0x1000)
#define PCMCIA_CTL_BASE_ADDR    (X_MEMC_BASE_ADDR + 0x2000)
#define NFC_BASE_ADDR           (X_MEMC_BASE_ADDR + 0x3000)

#define IRAM_BASE_ADDR          0xFFFFE800	


#define MXC_INT_USBCTRL         58
#define MXC_INT_USBCTRL         58
#define MXC_INT_USBMNP          57
#define MXC_INT_USBFUNC         56
#define MXC_INT_USBHOST         55
#define MXC_INT_USBDMA          54
#define MXC_INT_USBWKUP         53
#define MXC_INT_EMMADEC         50
#define MXC_INT_EMMAENC         49
#define MXC_INT_BMI             30
#define MXC_INT_FIRI            9


#define DMA_REQ_BMI_RX          29
#define DMA_REQ_BMI_TX          28
#define DMA_REQ_FIRI_RX         4

#endif 
