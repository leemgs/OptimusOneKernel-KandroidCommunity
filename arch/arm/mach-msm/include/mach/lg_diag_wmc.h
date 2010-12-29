#ifndef LG_DIAG_WMC_H
#define LG_DIAG_WMC_H

#include "lg_comdef.h"

#if defined __GNUC__
  #define PACK(x)       x __attribute__((__packed__))
  #define PACKED        __attribute__((__packed__))
#elif defined __arm
  #define PACK(x)       __packed x
  #define PACKED        __packed
#else
  #error No PACK() macro defined for this compiler
#endif



#endif 
