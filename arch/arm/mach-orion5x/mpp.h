#ifndef __ARCH_ORION5X_MPP_H
#define __ARCH_ORION5X_MPP_H

enum orion5x_mpp_type {
	
	MPP_UNUSED,

	
	MPP_GPIO,

	
	MPP_PCIE_RST_OUTn,

	
	MPP_PCI_ARB,

	
	MPP_PCI_PMEn,

	
	MPP_GIGE,

	
	MPP_NAND,

	
	MPP_PCI_CLK,

	
	MPP_SATA_LED,

	
	MPP_UART,
};

struct orion5x_mpp_mode {
	int			mpp;
	enum orion5x_mpp_type	type;
};

void orion5x_mpp_conf(struct orion5x_mpp_mode *mode);


#endif
