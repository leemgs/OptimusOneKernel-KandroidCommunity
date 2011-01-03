
#ifndef __HV_API_H
#define __HV_API_H





#define HV_STATUS_SUCCESS				((u16)0x0000)


#define HV_STATUS_INVALID_HYPERCALL_CODE		((u16)0x0002)


#define HV_STATUS_INVALID_HYPERCALL_INPUT		((u16)0x0003)


#define HV_STATUS_INVALID_ALIGNMENT			((u16)0x0004)


#define HV_STATUS_INVALID_PARAMETER			((u16)0x0005)


#define HV_STATUS_ACCESS_DENIED				((u16)0x0006)


#define HV_STATUS_INVALID_PARTITION_STATE		((u16)0x0007)


#define HV_STATUS_OPERATION_DENIED			((u16)0x0008)


#define HV_STATUS_UNKNOWN_PROPERTY			((u16)0x0009)


#define HV_STATUS_PROPERTY_VALUE_OUT_OF_RANGE		((u16)0x000A)


#define HV_STATUS_INSUFFICIENT_MEMORY			((u16)0x000B)


#define HV_STATUS_PARTITION_TOO_DEEP			((u16)0x000C)


#define HV_STATUS_INVALID_PARTITION_ID			((u16)0x000D)


#define HV_STATUS_INVALID_VP_INDEX			((u16)0x000E)


#define HV_STATUS_NOT_FOUND				((u16)0x0010)


#define HV_STATUS_INVALID_PORT_ID			((u16)0x0011)


#define HV_STATUS_INVALID_CONNECTION_ID			((u16)0x0012)


#define HV_STATUS_INSUFFICIENT_BUFFERS			((u16)0x0013)


#define HV_STATUS_NOT_ACKNOWLEDGED			((u16)0x0014)


#define HV_STATUS_INVALID_VP_STATE			((u16)0x0015)


#define HV_STATUS_ACKNOWLEDGED				((u16)0x0016)


#define HV_STATUS_INVALID_SAVE_RESTORE_STATE		((u16)0x0017)


#define HV_STATUS_INVALID_SYNIC_STATE			((u16)0x0018)


#define HV_STATUS_OBJECT_IN_USE				((u16)0x0019)


#define HV_STATUS_INVALID_PROXIMITY_DOMAIN_INFO		((u16)0x001A)


#define HV_STATUS_NO_DATA				((u16)0x001B)


#define HV_STATUS_INACTIVE				((u16)0x001C)


#define HV_STATUS_NO_RESOURCES				((u16)0x001D)


#define HV_STATUS_FEATURE_UNAVAILABLE			((u16)0x001E)


#define HV_STATUS_UNSUCCESSFUL				((u16)0x1001)


#define HV_STATUS_INSUFFICIENT_BUFFER			((u16)0x1002)


#define HV_STATUS_GPA_NOT_PRESENT			((u16)0x1003)


#define HV_STATUS_GUEST_PAGE_FAULT			((u16)0x1004)


#define HV_STATUS_RUNDOWN_DISABLED			((u16)0x1005)


#define HV_STATUS_KEY_ALREADY_EXISTS			((u16)0x1006)


#define HV_STATUS_GPA_INTERCEPT				((u16)0x1007)


#define HV_STATUS_GUEST_GENERAL_PROTECTION_FAULT	((u16)0x1008)


#define HV_STATUS_GUEST_STACK_FAULT			((u16)0x1009)


#define HV_STATUS_GUEST_INVALID_OPCODE_FAULT		((u16)0x100A)


#define HV_STATUS_FINALIZE_INCOMPLETE			((u16)0x100B)


#define HV_STATUS_GUEST_MACHINE_CHECK_ABORT		((u16)0x100C)


#define HV_STATUS_ILLEGAL_OVERLAY_ACCESS		((u16)0x100D)


#define HV_STATUS_INSUFFICIENT_SYSTEM_VA		((u16)0x100E)


#define HV_STATUS_VIRTUAL_ADDRESS_NOT_MAPPED		((u16)0x100F)


#define HV_STATUS_NOT_IMPLEMENTED			((u16)0x1010)


#define HV_STATUS_VMX_INSTRUCTION_FAILED		((u16)0x1011)


#define HV_STATUS_VMX_INSTRUCTION_FAILED_WITH_STATUS	((u16)0x1012)


#define HV_STATUS_MSR_ACCESS_FAILED		((u16)0x1013)


#define HV_STATUS_CR_ACCESS_FAILED		((u16)0x1014)


#define HV_STATUS_TIMEOUT			((u16)0x1016)


#define HV_STATUS_MSR_INTERCEPT			((u16)0x1017)


#define HV_STATUS_CPUID_INTERCEPT		((u16)0x1018)


#define HV_STATUS_REPEAT_INSTRUCTION		((u16)0x1019)


#define HV_STATUS_PAGE_PROTECTION_VIOLATION	((u16)0x101A)


#define HV_STATUS_PAGE_TABLE_INVALID		((u16)0x101B)


#define HV_STATUS_PAGE_NOT_PRESENT		((u16)0x101C)


#define HV_STATUS_IO_INTERCEPT				((u16)0x101D)


#define HV_STATUS_NOTHING_TO_DO				((u16)0x101E)


#define HV_STATUS_THREAD_TERMINATING			((u16)0x101F)


#define HV_STATUS_SECTION_ALREADY_CONSTRUCTED		((u16)0x1020)


#define HV_STATUS_SECTION_NOT_ALREADY_CONSTRUCTED	((u16)0x1021)


#define HV_STATUS_PAGE_ALREADY_COMMITTED		((u16)0x1022)


#define HV_STATUS_PAGE_NOT_ALREADY_COMMITTED		((u16)0x1023)


#define HV_STATUS_COMMITTED_PAGES_REMAIN		((u16)0x1024)


#define HV_STATUS_NO_REMAINING_COMMITTED_PAGES		((u16)0x1025)


#define HV_STATUS_INSUFFICIENT_COMPARTMENT_VA		((u16)0x1026)


#define HV_STATUS_DEREF_SPA_LIST_FULL			((u16)0x1027)


#define HV_STATUS_GPA_OUT_OF_RANGE			((u16)0x1027)


#define HV_STATUS_NONVOLATILE_XMM_STALE			((u16)0x1028)


#define HV_STATUS_UNSUPPORTED_PROCESSOR			((u16)0x1029)


#define HV_STATUS_INSUFFICIENT_CROM_SPACE		((u16)0x2000)


#define HV_STATUS_BAD_CROM_FORMAT			((u16)0x2001)


#define HV_STATUS_UNSUPPORTED_CROM_FORMAT		((u16)0x2002)


#define HV_STATUS_UNSUPPORTED_CONTROLLER		((u16)0x2003)


#define HV_STATUS_CROM_TOO_LARGE			((u16)0x2004)


#define HV_STATUS_CONTROLLER_IN_USE			((u16)0x2005)



enum hv_cpuid_function {
	HvCpuIdFunctionVersionAndFeatures		= 0x00000001,
	HvCpuIdFunctionHvVendorAndMaxFunction		= 0x40000000,
	HvCpuIdFunctionHvInterface			= 0x40000001,

	
	HvCpuIdFunctionMsHvVersion			= 0x40000002,
	HvCpuIdFunctionMsHvFeatures			= 0x40000003,
	HvCpuIdFunctionMsHvEnlightenmentInformation	= 0x40000004,
	HvCpuIdFunctionMsHvImplementationLimits		= 0x40000005,
};


#define HV_X64_MSR_EOI			(0x40000070)
#define HV_X64_MSR_ICR			(0x40000071)
#define HV_X64_MSR_TPR			(0x40000072)
#define HV_X64_MSR_APIC_ASSIST_PAGE	(0x40000073)


#define HV_SYNIC_VERSION		(1)


#define HV_X64_MSR_SCONTROL		(0x40000080)
#define HV_X64_MSR_SVERSION		(0x40000081)
#define HV_X64_MSR_SIEFP		(0x40000082)
#define HV_X64_MSR_SIMP			(0x40000083)
#define HV_X64_MSR_EOM			(0x40000084)
#define HV_X64_MSR_SINT0		(0x40000090)
#define HV_X64_MSR_SINT1		(0x40000091)
#define HV_X64_MSR_SINT2		(0x40000092)
#define HV_X64_MSR_SINT3		(0x40000093)
#define HV_X64_MSR_SINT4		(0x40000094)
#define HV_X64_MSR_SINT5		(0x40000095)
#define HV_X64_MSR_SINT6		(0x40000096)
#define HV_X64_MSR_SINT7		(0x40000097)
#define HV_X64_MSR_SINT8		(0x40000098)
#define HV_X64_MSR_SINT9		(0x40000099)
#define HV_X64_MSR_SINT10		(0x4000009A)
#define HV_X64_MSR_SINT11		(0x4000009B)
#define HV_X64_MSR_SINT12		(0x4000009C)
#define HV_X64_MSR_SINT13		(0x4000009D)
#define HV_X64_MSR_SINT14		(0x4000009E)
#define HV_X64_MSR_SINT15		(0x4000009F)


#define HV_SYNIC_VERSION_1		(0x1)


#define HV_MESSAGE_SIZE			(256)
#define HV_MESSAGE_PAYLOAD_BYTE_COUNT	(240)
#define HV_MESSAGE_PAYLOAD_QWORD_COUNT	(30)
#define HV_ANY_VP			(0xFFFFFFFF)


#define HV_EVENT_FLAGS_COUNT		(256 * 8)
#define HV_EVENT_FLAGS_BYTE_COUNT	(256)
#define HV_EVENT_FLAGS_DWORD_COUNT	(256 / sizeof(u32))


enum hv_message_type {
	HvMessageTypeNone			= 0x00000000,

	
	HvMessageTypeUnmappedGpa		= 0x80000000,
	HvMessageTypeGpaIntercept		= 0x80000001,

	
	HvMessageTimerExpired			= 0x80000010,

	
	HvMessageTypeInvalidVpRegisterValue	= 0x80000020,
	HvMessageTypeUnrecoverableException	= 0x80000021,
	HvMessageTypeUnsupportedFeature		= 0x80000022,

	
	HvMessageTypeEventLogBufferComplete	= 0x80000040,

	
	HvMessageTypeX64IoPortIntercept		= 0x80010000,
	HvMessageTypeX64MsrIntercept		= 0x80010001,
	HvMessageTypeX64CpuidIntercept		= 0x80010002,
	HvMessageTypeX64ExceptionIntercept	= 0x80010003,
	HvMessageTypeX64ApicEoi			= 0x80010004,
	HvMessageTypeX64LegacyFpError		= 0x80010005
};


#define HV_SYNIC_SINT_COUNT		(16)
#define HV_SYNIC_STIMER_COUNT		(4)


#define HV_PARTITION_ID_INVALID		((u64)0x0)


union hv_connection_id {
	u32 Asu32;
	struct {
		u32 Id:24;
		u32 Reserved:8;
	} u;
};


union hv_port_id {
	u32 Asu32;
	struct {
		u32 Id:24;
		u32 Reserved:8;
	} u ;
};


enum hv_port_type {
	HvPortTypeMessage	= 1,
	HvPortTypeEvent		= 2,
	HvPortTypeMonitor	= 3
};


struct hv_port_info {
	enum hv_port_type PortType;
	u32 Padding;
	union {
		struct {
			u32 TargetSint;
			u32 TargetVp;
			u64 RsvdZ;
		} MessagePortInfo;
		struct {
			u32 TargetSint;
			u32 TargetVp;
			u16 BaseFlagNumber;
			u16 FlagCount;
			u32 RsvdZ;
		} EventPortInfo;
		struct {
			u64 MonitorAddress;
			u64 RsvdZ;
		} MonitorPortInfo;
	};
};

struct hv_connection_info {
	enum hv_port_type PortType;
	u32 Padding;
	union {
		struct {
			u64 RsvdZ;
		} MessageConnectionInfo;
		struct {
			u64 RsvdZ;
		} EventConnectionInfo;
		struct {
			u64 MonitorAddress;
		} MonitorConnectionInfo;
	};
};


union hv_message_flags {
	u8 Asu8;
	struct {
		u8 MessagePending:1;
		u8 Reserved:7;
	};
};


struct hv_message_header {
	enum hv_message_type MessageType;
	u8 PayloadSize;
	union hv_message_flags MessageFlags;
	u8 Reserved[2];
	union {
		u64 Sender;
		union hv_port_id Port;
	};
};


struct hv_timer_message_payload {
	u32 TimerIndex;
	u32 Reserved;
	u64 ExpirationTime;	
	u64 DeliveryTime;	
};


struct hv_message {
	struct hv_message_header Header;
	union {
		u64 Payload[HV_MESSAGE_PAYLOAD_QWORD_COUNT];
	} u ;
};


#define HV_PORT_MESSAGE_BUFFER_COUNT	(16)


struct hv_message_page {
	struct hv_message SintMessage[HV_SYNIC_SINT_COUNT];
};


union hv_synic_event_flags {
	u8 Flags8[HV_EVENT_FLAGS_BYTE_COUNT];
	u32 Flags32[HV_EVENT_FLAGS_DWORD_COUNT];
};


struct hv_synic_event_flags_page {
	union hv_synic_event_flags SintEventFlags[HV_SYNIC_SINT_COUNT];
};


union hv_synic_scontrol {
	u64 AsUINT64;
	struct {
		u64 Enable:1;
		u64 Reserved:63;
	};
};


union hv_synic_sint {
	u64 AsUINT64;
	struct {
		u64 Vector:8;
		u64 Reserved1:8;
		u64 Masked:1;
		u64 AutoEoi:1;
		u64 Reserved2:46;
	};
};


union hv_synic_simp {
	u64 AsUINT64;
	struct {
		u64 SimpEnabled:1;
		u64 Preserved:11;
		u64 BaseSimpGpa:52;
	};
};


union hv_synic_siefp {
	u64 AsUINT64;
	struct {
		u64 SiefpEnabled:1;
		u64 Preserved:11;
		u64 BaseSiefpGpa:52;
	};
};


union hv_monitor_trigger_group {
	u64 AsUINT64;
	struct {
		u32 Pending;
		u32 Armed;
	};
};

struct hv_monitor_parameter {
	union hv_connection_id ConnectionId;
	u16 FlagNumber;
	u16 RsvdZ;
};

union hv_monitor_trigger_state {
	u32 Asu32;

	struct {
		u32 GroupEnable:4;
		u32 RsvdZ:28;
	};
};




















struct hv_monitor_page {
	union hv_monitor_trigger_state TriggerState;
	u32 RsvdZ1;

	union hv_monitor_trigger_group TriggerGroup[4];
	u64 RsvdZ2[3];

	s32 NextCheckTime[4][32];

	u16 Latency[4][32];
	u64 RsvdZ3[32];

	struct hv_monitor_parameter Parameter[4][32];

	u8 RsvdZ4[1984];
};


enum hv_call_code {
	HvCallPostMessage	= 0x005c,
	HvCallSignalEvent	= 0x005d,
};


struct hv_input_post_message {
	union hv_connection_id ConnectionId;
	u32 Reserved;
	enum hv_message_type MessageType;
	u32 PayloadSize;
	u64 Payload[HV_MESSAGE_PAYLOAD_QWORD_COUNT];
};


struct hv_input_signal_event {
	union hv_connection_id ConnectionId;
	u16 FlagNumber;
	u16 RsvdZ;
};




enum hv_guest_os_vendor {
	HvGuestOsVendorMicrosoft	= 0x0001
};

enum hv_guest_os_microsoft_ids {
	HvGuestOsMicrosoftUndefined	= 0x00,
	HvGuestOsMicrosoftMSDOS		= 0x01,
	HvGuestOsMicrosoftWindows3x	= 0x02,
	HvGuestOsMicrosoftWindows9x	= 0x03,
	HvGuestOsMicrosoftWindowsNT	= 0x04,
	HvGuestOsMicrosoftWindowsCE	= 0x05
};


#define HV_X64_MSR_GUEST_OS_ID	0x40000000

union hv_x64_msr_guest_os_id_contents {
	u64 AsUINT64;
	struct {
		u64 BuildNumber:16;
		u64 ServiceVersion:8; 
		u64 MinorVersion:8;
		u64 MajorVersion:8;
		u64 OsId:8; 
		u64 VendorId:16; 
	};
};


#define HV_X64_MSR_HYPERCALL	0x40000001

union hv_x64_msr_hypercall_contents {
	u64 AsUINT64;
	struct {
		u64 Enable:1;
		u64 Reserved:11;
		u64 GuestPhysicalAddress:52;
	};
};

#endif
