

#include <linux/bootmem.h>
#include <linux/blkdev.h>
#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/limits.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/types.h>

#include <asm/mmzone.h>


struct ibft_table_header *ibft_addr;
EXPORT_SYMBOL_GPL(ibft_addr);

#define IBFT_SIGN "iBFT"
#define IBFT_SIGN_LEN 4
#define IBFT_START 0x80000 
#define IBFT_END 0x100000 
#define VGA_MEM 0xA0000 
#define VGA_SIZE 0x20000 



void __init reserve_ibft_region(void)
{
	unsigned long pos;
	unsigned int len = 0;
	void *virt;

	ibft_addr = NULL;

	for (pos = IBFT_START; pos < IBFT_END; pos += 16) {
		
		if (pos == VGA_MEM)
			pos += VGA_SIZE;
		virt = isa_bus_to_virt(pos);
		if (memcmp(virt, IBFT_SIGN, IBFT_SIGN_LEN) == 0) {
			unsigned long *addr =
			    (unsigned long *)isa_bus_to_virt(pos + 4);
			len = *addr;
			
			if (pos + len <= (IBFT_END-1)) {
				ibft_addr = (struct ibft_table_header *)virt;
				break;
			}
		}
	}
	if (ibft_addr)
		reserve_bootmem(pos, PAGE_ALIGN(len), BOOTMEM_DEFAULT);
}
