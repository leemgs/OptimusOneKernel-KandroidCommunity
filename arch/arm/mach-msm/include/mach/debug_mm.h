
#ifndef __ARCH_ARM_MACH_MSM_DEBUG_MM_H_
#define __ARCH_ARM_MACH_MSM_DEBUG_MM_H_


#define __MM_FILE__ strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/')+1) : \
	__FILE__

#define MM_DBG(fmt, args...) pr_debug("[%s] " fmt,\
		__func__, ##args)

#define MM_INFO(fmt, args...) pr_info("[%s:%s] " fmt,\
	       __MM_FILE__, __func__, ##args)

#define MM_ERR(fmt, args...) pr_err("[%s:%s] " fmt,\
	       __MM_FILE__, __func__, ##args)
#endif 
