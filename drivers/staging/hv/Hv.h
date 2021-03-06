


#ifndef __HV_H__
#define __HV_H__

#include "hv_api.h"

enum {
	VMBUS_MESSAGE_CONNECTION_ID	= 1,
	VMBUS_MESSAGE_PORT_ID		= 1,
	VMBUS_EVENT_CONNECTION_ID	= 2,
	VMBUS_EVENT_PORT_ID		= 2,
	VMBUS_MONITOR_CONNECTION_ID	= 3,
	VMBUS_MONITOR_PORT_ID		= 3,
	VMBUS_MESSAGE_SINT		= 2,
};



#define HV_PRESENT_BIT			0x80000000

#define HV_XENLINUX_GUEST_ID_LO		0x00000000
#define HV_XENLINUX_GUEST_ID_HI		0x0B00B135
#define HV_XENLINUX_GUEST_ID		(((u64)HV_XENLINUX_GUEST_ID_HI << 32) \
					  | HV_XENLINUX_GUEST_ID_LO)

#define HV_LINUX_GUEST_ID_LO		0x00000000
#define HV_LINUX_GUEST_ID_HI		0xB16B00B5
#define HV_LINUX_GUEST_ID		(((u64)HV_LINUX_GUEST_ID_HI << 32) | \
					   HV_LINUX_GUEST_ID_LO)

#define HV_CPU_POWER_MANAGEMENT		(1 << 0)
#define HV_RECOMMENDATIONS_MAX		4

#define HV_X64_MAX			5
#define HV_CAPS_MAX			8


#define HV_HYPERCALL_PARAM_ALIGN	sizeof(u64)




#define HV_SERVICE_PARENT_PORT				(0)
#define HV_SERVICE_PARENT_CONNECTION			(0)

#define HV_SERVICE_CONNECT_RESPONSE_SUCCESS		(0)
#define HV_SERVICE_CONNECT_RESPONSE_INVALID_PARAMETER	(1)
#define HV_SERVICE_CONNECT_RESPONSE_UNKNOWN_SERVICE	(2)
#define HV_SERVICE_CONNECT_RESPONSE_CONNECTION_REJECTED	(3)

#define HV_SERVICE_CONNECT_REQUEST_MESSAGE_ID		(1)
#define HV_SERVICE_CONNECT_RESPONSE_MESSAGE_ID		(2)
#define HV_SERVICE_DISCONNECT_REQUEST_MESSAGE_ID	(3)
#define HV_SERVICE_DISCONNECT_RESPONSE_MESSAGE_ID	(4)
#define HV_SERVICE_MAX_MESSAGE_ID				(4)

#define HV_SERVICE_PROTOCOL_VERSION (0x0010)
#define HV_CONNECT_PAYLOAD_BYTE_COUNT 64







static const struct hv_guid VMBUS_SERVICE_ID = {
	.data = {
		0xb8, 0x80, 0x81, 0x62, 0x8d, 0x30, 0x5e, 0x4c,
		0xb7, 0xdb, 0x1b, 0xeb, 0x62, 0xe6, 0x2e, 0xf4
	},
};

#define MAX_NUM_CPUS	32


struct hv_input_signal_event_buffer {
	u64 Align8;
	struct hv_input_signal_event Event;
};

struct hv_context {
	
	u64 GuestId;

	void *HypercallPage;

	bool SynICInitialized;

	
	struct hv_input_signal_event_buffer *SignalEventBuffer;
	
	struct hv_input_signal_event *SignalEventParam;

	void *synICMessagePage[MAX_NUM_CPUS];
	void *synICEventPage[MAX_NUM_CPUS];
};

extern struct hv_context gHvContext;




extern int HvInit(void);

extern void HvCleanup(void);

extern u16 HvPostMessage(union hv_connection_id connectionId,
			 enum hv_message_type messageType,
			 void *payload, size_t payloadSize);

extern u16 HvSignalEvent(void);

extern void HvSynicInit(void *irqarg);

extern void HvSynicCleanup(void *arg);

#endif 
