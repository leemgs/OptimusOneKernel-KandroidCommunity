

#ifndef __RT3370_H__
#define __RT3370_H__

#ifdef RT3370


#error "For RT3070, you should define the compile flag -DRTMP_USB_SUPPORT"

#error "For RT3070, you should define the compile flag -DRTMP_MAC_USB"

#ifndef RTMP_RF_RW_SUPPORT
#error "For RT3070, you should define the compile flag -DRTMP_RF_RW_SUPPORT"
#endif

#ifndef RT33xx
#error "For RT3070, you should define the compile flag -DRT30xx"
#endif

#include "mac_usb.h"
#include "rt33xx.h"





#endif 

#endif 
