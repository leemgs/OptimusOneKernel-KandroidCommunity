


#ifndef __ASM_ARCH_HARDWARE_H
#define __ASM_ARCH_HARDWARE_H


#define IO_BASE			0xF0000000                 
#define IO_SIZE			0x00100000                 
#define IO_START		0x80000000                 


#define IO_ADDRESS(x) (((x) & 0x000fffff) | IO_BASE)

#endif
