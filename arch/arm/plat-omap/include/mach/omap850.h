

#ifndef __ASM_ARCH_OMAP850_H
#define __ASM_ARCH_OMAP850_H





#define OMAP850_DSP_BASE	0xE0000000
#define OMAP850_DSP_SIZE	0x50000
#define OMAP850_DSP_START	0xE0000000

#define OMAP850_DSPREG_BASE	0xE1000000
#define OMAP850_DSPREG_SIZE	SZ_128K
#define OMAP850_DSPREG_START	0xE1000000


#define OMAP850_CONFIG_BASE	0xfffe1000
#define OMAP850_IO_CONF_0	0xfffe1070
#define OMAP850_IO_CONF_1	0xfffe1074
#define OMAP850_IO_CONF_2	0xfffe1078
#define OMAP850_IO_CONF_3	0xfffe107c
#define OMAP850_IO_CONF_4	0xfffe1080
#define OMAP850_IO_CONF_5	0xfffe1084
#define OMAP850_IO_CONF_6	0xfffe1088
#define OMAP850_IO_CONF_7	0xfffe108c
#define OMAP850_IO_CONF_8	0xfffe1090
#define OMAP850_IO_CONF_9	0xfffe1094
#define OMAP850_IO_CONF_10	0xfffe1098
#define OMAP850_IO_CONF_11	0xfffe109c
#define OMAP850_IO_CONF_12	0xfffe10a0
#define OMAP850_IO_CONF_13	0xfffe10a4

#define OMAP850_MODE_1		0xfffe1010
#define OMAP850_MODE_2		0xfffe1014


#define OMAP850_MODE2_OFFSET	0x14


#define OMAP850_FLASH_CFG_0	0xfffecc10
#define OMAP850_FLASH_ACFG_0	0xfffecc50
#define OMAP850_FLASH_CFG_1	0xfffecc14
#define OMAP850_FLASH_ACFG_1	0xfffecc54


#define OMAP850_ICR_BASE	0xfffbb800
#define OMAP850_DSP_M_CTL	0xfffbb804
#define OMAP850_DSP_MMU_BASE	0xfffed200


#define OMAP850_PCC_UPLD_CTRL_BASE	(0xfffe0900)
#define OMAP850_PCC_UPLD_CTRL		(OMAP850_PCC_UPLD_CTRL_BASE + 0x00)

#endif 

