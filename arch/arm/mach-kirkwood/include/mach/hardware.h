

#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H

#include "kirkwood.h"

#define pcibios_assign_all_busses()	1

#define PCIBIOS_MIN_IO			0x00001000
#define PCIBIOS_MIN_MEM			0x01000000
#define PCIMEM_BASE			KIRKWOOD_PCIE_MEM_PHYS_BASE 


#endif
