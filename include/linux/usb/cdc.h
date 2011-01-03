

#ifndef __LINUX_USB_CDC_H
#define __LINUX_USB_CDC_H

#include <linux/types.h>

#define USB_CDC_SUBCLASS_ACM			0x02
#define USB_CDC_SUBCLASS_ETHERNET		0x06
#define USB_CDC_SUBCLASS_WHCM			0x08
#define USB_CDC_SUBCLASS_DMM			0x09
#define USB_CDC_SUBCLASS_MDLM			0x0a
#define USB_CDC_SUBCLASS_OBEX			0x0b
#define USB_CDC_SUBCLASS_EEM			0x0c

#define USB_CDC_PROTO_NONE			0

#define USB_CDC_ACM_PROTO_AT_V25TER		1
#define USB_CDC_ACM_PROTO_AT_PCCA101		2
#define USB_CDC_ACM_PROTO_AT_PCCA101_WAKE	3
#define USB_CDC_ACM_PROTO_AT_GSM		4
#define USB_CDC_ACM_PROTO_AT_3G			5
#define USB_CDC_ACM_PROTO_AT_CDMA		6
#define USB_CDC_ACM_PROTO_VENDOR		0xff

#define USB_CDC_PROTO_EEM			7





#define USB_CDC_HEADER_TYPE		0x00	
#define USB_CDC_CALL_MANAGEMENT_TYPE	0x01	
#define USB_CDC_ACM_TYPE		0x02	
#define USB_CDC_UNION_TYPE		0x06	
#define USB_CDC_COUNTRY_TYPE		0x07
#define USB_CDC_NETWORK_TERMINAL_TYPE	0x0a	
#define USB_CDC_ETHERNET_TYPE		0x0f	
#define USB_CDC_WHCM_TYPE		0x11
#define USB_CDC_MDLM_TYPE		0x12	
#define USB_CDC_MDLM_DETAIL_TYPE	0x13	
#define USB_CDC_DMM_TYPE		0x14
#define USB_CDC_OBEX_TYPE		0x15


struct usb_cdc_header_desc {
	__u8	bLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubType;

	__le16	bcdCDC;
} __attribute__ ((packed));


struct usb_cdc_call_mgmt_descriptor {
	__u8	bLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubType;

	__u8	bmCapabilities;
#define USB_CDC_CALL_MGMT_CAP_CALL_MGMT		0x01
#define USB_CDC_CALL_MGMT_CAP_DATA_INTF		0x02

	__u8	bDataInterface;
} __attribute__ ((packed));


struct usb_cdc_acm_descriptor {
	__u8	bLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubType;

	__u8	bmCapabilities;
} __attribute__ ((packed));



#define USB_CDC_COMM_FEATURE	0x01
#define USB_CDC_CAP_LINE	0x02
#define USB_CDC_CAP_BRK	0x04
#define USB_CDC_CAP_NOTIFY	0x08


struct usb_cdc_union_desc {
	__u8	bLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubType;

	__u8	bMasterInterface0;
	__u8	bSlaveInterface0;
	
} __attribute__ ((packed));


struct usb_cdc_country_functional_desc {
	__u8	bLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubType;

	__u8	iCountryCodeRelDate;
	__le16	wCountyCode0;
	
} __attribute__ ((packed));


struct usb_cdc_network_terminal_desc {
	__u8	bLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubType;

	__u8	bEntityId;
	__u8	iName;
	__u8	bChannelIndex;
	__u8	bPhysicalInterface;
} __attribute__ ((packed));


struct usb_cdc_ether_desc {
	__u8	bLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubType;

	__u8	iMACAddress;
	__le32	bmEthernetStatistics;
	__le16	wMaxSegmentSize;
	__le16	wNumberMCFilters;
	__u8	bNumberPowerFilters;
} __attribute__ ((packed));


struct usb_cdc_dmm_desc {
	__u8	bFunctionLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubtype;
	__u16	bcdVersion;
	__le16	wMaxCommand;
} __attribute__ ((packed));


struct usb_cdc_mdlm_desc {
	__u8	bLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubType;

	__le16	bcdVersion;
	__u8	bGUID[16];
} __attribute__ ((packed));


struct usb_cdc_mdlm_detail_desc {
	__u8	bLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubType;

	
	__u8	bGuidDescriptorType;
	__u8	bDetailData[0];
} __attribute__ ((packed));


struct usb_cdc_obex_desc {
	__u8	bLength;
	__u8	bDescriptorType;
	__u8	bDescriptorSubType;

	__le16	bcdVersion;
} __attribute__ ((packed));





#define USB_CDC_SEND_ENCAPSULATED_COMMAND	0x00
#define USB_CDC_GET_ENCAPSULATED_RESPONSE	0x01
#define USB_CDC_REQ_SET_LINE_CODING		0x20
#define USB_CDC_REQ_GET_LINE_CODING		0x21
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE	0x22
#define USB_CDC_REQ_SEND_BREAK			0x23
#define USB_CDC_SET_ETHERNET_MULTICAST_FILTERS	0x40
#define USB_CDC_SET_ETHERNET_PM_PATTERN_FILTER	0x41
#define USB_CDC_GET_ETHERNET_PM_PATTERN_FILTER	0x42
#define USB_CDC_SET_ETHERNET_PACKET_FILTER	0x43
#define USB_CDC_GET_ETHERNET_STATISTIC		0x44


struct usb_cdc_line_coding {
	__le32	dwDTERate;
	__u8	bCharFormat;
#define USB_CDC_1_STOP_BITS			0
#define USB_CDC_1_5_STOP_BITS			1
#define USB_CDC_2_STOP_BITS			2

	__u8	bParityType;
#define USB_CDC_NO_PARITY			0
#define USB_CDC_ODD_PARITY			1
#define USB_CDC_EVEN_PARITY			2
#define USB_CDC_MARK_PARITY			3
#define USB_CDC_SPACE_PARITY			4

	__u8	bDataBits;
} __attribute__ ((packed));


#define	USB_CDC_PACKET_TYPE_PROMISCUOUS		(1 << 0)
#define	USB_CDC_PACKET_TYPE_ALL_MULTICAST	(1 << 1) 
#define	USB_CDC_PACKET_TYPE_DIRECTED		(1 << 2)
#define	USB_CDC_PACKET_TYPE_BROADCAST		(1 << 3)
#define	USB_CDC_PACKET_TYPE_MULTICAST		(1 << 4) 






#define USB_CDC_NOTIFY_NETWORK_CONNECTION	0x00
#define USB_CDC_NOTIFY_RESPONSE_AVAILABLE	0x01
#define USB_CDC_NOTIFY_SERIAL_STATE		0x20
#define USB_CDC_NOTIFY_SPEED_CHANGE		0x2a

struct usb_cdc_notification {
	__u8	bmRequestType;
	__u8	bNotificationType;
	__le16	wValue;
	__le16	wIndex;
	__le16	wLength;
} __attribute__ ((packed));

#endif 
