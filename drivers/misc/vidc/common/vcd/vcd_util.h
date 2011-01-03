
#ifndef _VCD_UTIL_H_
#define _VCD_UTIL_H_

#include "vcd_api.h"

#if DEBUG

#define VCD_MSG_LOW(xx_fmt...)		printk(KERN_INFO "\n\t* " xx_fmt)
#define VCD_MSG_MED(xx_fmt...)		printk(KERN_INFO "\n  * " xx_fmt)
#define VCD_MSG_HIGH(xx_fmt...)		printk(KERN_WARNING "\n" xx_fmt)

#else

#define VCD_MSG_LOW(xx_fmt...)
#define VCD_MSG_MED(xx_fmt...)
#define VCD_MSG_HIGH(xx_fmt...)

#endif

#define VCD_MSG_ERROR(xx_fmt...)	printk(KERN_ERR "\n err: " xx_fmt)
#define VCD_MSG_FATAL(xx_fmt...)	printk(KERN_ERR "\n<FATAL> " xx_fmt)

#define VCD_FAILED_RETURN(rc, xx_fmt...)		\
	do {						\
		if (VCD_FAILED(rc)) {			\
			printk(KERN_ERR  xx_fmt);	\
			return rc;			\
		}					\
	} while	(0)

#define VCD_FAILED_DEVICE_FATAL(rc) \
	(rc == VCD_ERR_HW_FATAL ? TRUE : FALSE)
#define VCD_FAILED_CLIENT_FATAL(rc) \
	(rc == VCD_ERR_CLIENT_FATAL ? TRUE : FALSE)

#define VCD_FAILED_FATAL(rc)  \
	((VCD_FAILED_DEVICE_FATAL(rc) || VCD_FAILED_CLIENT_FATAL(rc)) \
	? TRUE : FALSE)


#define vcd_assert()                     VCD_MSG_FATAL("ASSERT")
#define vcd_malloc(n_bytes)              kmalloc(n_bytes, GFP_KERNEL)
#define vcd_free(p_mem)                  kfree(p_mem)

#ifdef NO_IN_KERNEL_PMEM
	#define VCD_PMEM_malloc(n_bytes)         kmalloc(n_bytes, GFP_KERNEL)
	#define VCD_PMEM_free(p_mem)             kfree(p_mem)
	#define VCD_PMEM_get_physical(p_mem)     virt_to_phys(p_mem)
#else
	int vcd_pmem_alloc(u32 size, u8 **kernel_vaddr, u8 **phy_addr);
	int vcd_pmem_free(u8 *kernel_vaddr, u8 *phy_addr);
#endif

u32 vcd_critical_section_create(u32 **p_cs);
u32 vcd_critical_section_release(u32 *cs);
u32 vcd_critical_section_enter(u32 *cs);
u32 vcd_critical_section_leave(u32 *cs);

#endif
