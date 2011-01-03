

#ifndef __XEN_PUBLIC_MEMORY_H__
#define __XEN_PUBLIC_MEMORY_H__


#define XENMEM_increase_reservation 0
#define XENMEM_decrease_reservation 1
#define XENMEM_populate_physmap     6
struct xen_memory_reservation {

    
    GUEST_HANDLE(ulong) extent_start;

    
    unsigned long  nr_extents;
    unsigned int   extent_order;

    
    unsigned int   address_bits;

    
    domid_t        domid;

};
DEFINE_GUEST_HANDLE_STRUCT(xen_memory_reservation);


#define XENMEM_maximum_ram_page     2


#define XENMEM_current_reservation  3
#define XENMEM_maximum_reservation  4


#define XENMEM_machphys_mfn_list    5
struct xen_machphys_mfn_list {
    
    unsigned int max_extents;

    
    GUEST_HANDLE(ulong) extent_start;

    
    unsigned int nr_extents;
};
DEFINE_GUEST_HANDLE_STRUCT(xen_machphys_mfn_list);


#define XENMEM_add_to_physmap      7
struct xen_add_to_physmap {
    
    domid_t domid;

    
#define XENMAPSPACE_shared_info 0 
#define XENMAPSPACE_grant_table 1 
    unsigned int space;

    
    unsigned long idx;

    
    unsigned long gpfn;
};
DEFINE_GUEST_HANDLE_STRUCT(xen_add_to_physmap);


#define XENMEM_translate_gpfn_list  8
struct xen_translate_gpfn_list {
    
    domid_t domid;

    
    unsigned long nr_gpfns;

    
    GUEST_HANDLE(ulong) gpfn_list;

    
    GUEST_HANDLE(ulong) mfn_list;
};
DEFINE_GUEST_HANDLE_STRUCT(xen_translate_gpfn_list);

#endif 
