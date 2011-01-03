


#ifndef CSP_DELAY_H
#define CSP_DELAY_H








#if defined(__KERNEL__) && !defined(STANDALONE)
   #include <linux/delay.h>
#else
   #include <mach/csp/delay.h>
#endif





#endif 
