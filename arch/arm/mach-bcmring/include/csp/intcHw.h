






#ifndef _INTCHW_H
#define _INTCHW_H


#include <mach/csp/intcHw_reg.h>




static inline void intcHw_irq_disable(void *basep, uint32_t mask);
static inline void intcHw_irq_enable(void *basep, uint32_t mask);

#endif 

