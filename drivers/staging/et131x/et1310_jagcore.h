

#ifndef __ET1310_JAGCORE_H__
#define __ET1310_JAGCORE_H__

#include "et1310_address_map.h"


#define INTERNAL_MEM_SIZE       0x400	
#define INTERNAL_MEM_RX_OFFSET  0x1FF	


#define INT_MASK_DISABLE            0xffffffff


#define INT_MASK_ENABLE             0xfffebf17
#define INT_MASK_ENABLE_NO_FLOW     0xfffebfd7


struct et131x_adapter;

void ConfigGlobalRegs(struct et131x_adapter *pAdapter);
void ConfigMMCRegs(struct et131x_adapter *pAdapter);
void et131x_enable_interrupts(struct et131x_adapter *adapter);
void et131x_disable_interrupts(struct et131x_adapter *adapter);

#endif 
