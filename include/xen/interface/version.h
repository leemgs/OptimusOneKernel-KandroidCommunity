

#ifndef __XEN_PUBLIC_VERSION_H__
#define __XEN_PUBLIC_VERSION_H__




#define XENVER_version      0


#define XENVER_extraversion 1
struct xen_extraversion {
    char extraversion[16];
};
#define XEN_EXTRAVERSION_LEN (sizeof(struct xen_extraversion))


#define XENVER_compile_info 2
struct xen_compile_info {
    char compiler[64];
    char compile_by[16];
    char compile_domain[32];
    char compile_date[32];
};

#define XENVER_capabilities 3
struct xen_capabilities_info {
    char info[1024];
};
#define XEN_CAPABILITIES_INFO_LEN (sizeof(struct xen_capabilities_info))

#define XENVER_changeset 4
struct xen_changeset_info {
    char info[64];
};
#define XEN_CHANGESET_INFO_LEN (sizeof(struct xen_changeset_info))

#define XENVER_platform_parameters 5
struct xen_platform_parameters {
    unsigned long virt_start;
};

#define XENVER_get_features 6
struct xen_feature_info {
    unsigned int submap_idx;    
    uint32_t     submap;        
};


#include "features.h"


#define XENVER_pagesize 7

#endif 
