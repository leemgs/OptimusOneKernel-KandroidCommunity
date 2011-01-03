

#ifndef _ASM_X86_XEN_HYPERVISOR_H
#define _ASM_X86_XEN_HYPERVISOR_H


extern struct shared_info *HYPERVISOR_shared_info;
extern struct start_info *xen_start_info;

enum xen_domain_type {
	XEN_NATIVE,		
	XEN_PV_DOMAIN,		
	XEN_HVM_DOMAIN,		
};

#ifdef CONFIG_XEN
extern enum xen_domain_type xen_domain_type;
#else
#define xen_domain_type		XEN_NATIVE
#endif

#define xen_domain()		(xen_domain_type != XEN_NATIVE)
#define xen_pv_domain()		(xen_domain() &&			\
				 xen_domain_type == XEN_PV_DOMAIN)
#define xen_hvm_domain()	(xen_domain() &&			\
				 xen_domain_type == XEN_HVM_DOMAIN)

#ifdef CONFIG_XEN_DOM0
#include <xen/interface/xen.h>

#define xen_initial_domain()	(xen_pv_domain() && \
				 xen_start_info->flags & SIF_INITDOMAIN)
#else  
#define xen_initial_domain()	(0)
#endif	

#endif 
