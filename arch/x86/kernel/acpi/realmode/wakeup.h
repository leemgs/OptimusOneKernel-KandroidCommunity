

#ifndef ARCH_X86_KERNEL_ACPI_RM_WAKEUP_H
#define ARCH_X86_KERNEL_ACPI_RM_WAKEUP_H

#ifndef __ASSEMBLY__
#include <linux/types.h>


struct wakeup_header {
	u16 video_mode;		
	u16 _jmp1;		
	u32 pmode_entry;	
	u16 _jmp2;		
	u32 pmode_cr0;		
	u32 pmode_cr3;		
	u32 pmode_cr4;		
	u32 pmode_efer_low;	
	u32 pmode_efer_high;
	u64 pmode_gdt;
	u32 realmode_flags;
	u32 real_magic;
	u16 trampoline_segment;	
	u8  _pad1;
	u8  wakeup_jmp;
	u16 wakeup_jmp_off;
	u16 wakeup_jmp_seg;
	u64 wakeup_gdt[3];
	u32 signature;		
} __attribute__((__packed__));

extern struct wakeup_header wakeup_header;
#endif

#define HEADER_OFFSET 0x3f00
#define WAKEUP_SIZE   0x4000

#endif 
