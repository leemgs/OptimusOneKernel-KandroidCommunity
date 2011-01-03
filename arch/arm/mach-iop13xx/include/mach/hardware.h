#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H
#include <asm/types.h>

#define pcibios_assign_all_busses() 1

#ifndef __ASSEMBLY__
extern unsigned long iop13xx_pcibios_min_io;
extern unsigned long iop13xx_pcibios_min_mem;
extern u16 iop13xx_dev_id(void);
extern void iop13xx_set_atu_mmr_bases(void);
#endif

#define PCIBIOS_MIN_IO      (iop13xx_pcibios_min_io)
#define PCIBIOS_MIN_MEM     (iop13xx_pcibios_min_mem)


#include "iop13xx.h"


#include "iq81340.h"

#endif  
