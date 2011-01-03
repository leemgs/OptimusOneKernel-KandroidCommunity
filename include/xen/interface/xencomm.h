

#ifndef _XEN_XENCOMM_H_
#define _XEN_XENCOMM_H_



#define XENCOMM_MAGIC 0x58434F4D 
#define XENCOMM_INVALID (~0UL)

struct xencomm_desc {
    uint32_t magic;
    uint32_t nr_addrs; 
    uint64_t address[0];
};

#endif 
