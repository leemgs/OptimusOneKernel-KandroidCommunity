#ifndef _ARM_KEXEC_H
#define _ARM_KEXEC_H

#ifdef CONFIG_KEXEC


#define KEXEC_SOURCE_MEMORY_LIMIT (-1UL)

#define KEXEC_DESTINATION_MEMORY_LIMIT (-1UL)

#define KEXEC_CONTROL_MEMORY_LIMIT (-1UL)

#define KEXEC_CONTROL_PAGE_SIZE	4096

#define KEXEC_ARCH KEXEC_ARCH_ARM

#define KEXEC_ARM_ATAGS_OFFSET  0x1000
#define KEXEC_ARM_ZIMAGE_OFFSET 0x8000

#ifndef __ASSEMBLY__

struct kimage;

static inline void crash_setup_regs(struct pt_regs *newregs,
                                        struct pt_regs *oldregs) { }

#endif 

#endif 

#endif 
