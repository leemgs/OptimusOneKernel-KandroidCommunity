

#ifndef __RT3390_H__
#define __RT3390_H__

#ifdef RT3390

#ifndef RTMP_PCI_SUPPORT
#error "For RT3390, you should define the compile flag -DRTMP_PCI_SUPPORT"
#endif

#ifndef RTMP_MAC_PCI
#error "For RT3390, you should define the compile flag -DRTMP_MAC_PCI"
#endif

#ifndef RTMP_RF_RW_SUPPORT
#error "For RT3390, you should define the compile flag -DRTMP_RF_RW_SUPPORT"
#endif

#ifndef RT30xx
#error "For RT3390, you should define the compile flag -DRT30xx"
#endif

#ifdef CARRIER_DETECTION_SUPPORT
#define TONE_RADAR_DETECT_SUPPORT
#define CARRIER_SENSE_NEW_ALGO
#endif 

#define PCIE_PS_SUPPORT

#include "mac_pci.h"
#include "rt33xx.h"




#define NIC3390_PCIe_DEVICE_ID  0x3090		
#define NIC3391_PCIe_DEVICE_ID  0x3091		
#define NIC3392_PCIe_DEVICE_ID  0x3092		

#endif 

#endif 
