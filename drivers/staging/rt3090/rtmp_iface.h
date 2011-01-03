

#ifndef __RTMP_IFACE_H__
#define __RTMP_IFACE_H__

#ifdef RTMP_PCI_SUPPORT
#include "rtmp_pci.h"
#endif 


typedef struct _INF_PCI_CONFIG_
{
	unsigned long	CSRBaseAddress;     
	unsigned int	irq_num;
}INF_PCI_CONFIG;


typedef struct _INF_USB_CONFIG_
{
	UINT8                BulkInEpAddr;		
	UINT8                BulkOutEpAddr[6];	
}INF_USB_CONFIG;


typedef struct _INF_RBUS_CONFIG_
{
	unsigned long		csr_addr;
	unsigned int		irq;
}INF_RBUS_CONFIG;


typedef enum _RTMP_INF_TYPE_
{
	RTMP_DEV_INF_UNKNOWN = 0,
	RTMP_DEV_INF_PCI = 1,
	RTMP_DEV_INF_USB = 2,
	RTMP_DEV_INF_RBUS = 4,
}RTMP_INF_TYPE;


typedef union _RTMP_INF_CONFIG_{
	struct _INF_PCI_CONFIG_			pciConfig;
	struct _INF_USB_CONFIG_			usbConfig;
	struct _INF_RBUS_CONFIG_		rbusConfig;
}RTMP_INF_CONFIG;

#endif 
