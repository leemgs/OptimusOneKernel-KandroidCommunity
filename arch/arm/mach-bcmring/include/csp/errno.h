

#ifndef CSP_ERRNO_H
#define CSP_ERRNO_H



#if   defined(__KERNEL__)
#include <linux/errno.h>
#elif defined(CSP_SIMULATION)
#include <asm-generic/errno.h>
#else
#include <errno.h>
#endif





#endif 
