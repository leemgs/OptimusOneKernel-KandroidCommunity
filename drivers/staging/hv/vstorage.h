





#define REVISION_STRING(REVISION_) #REVISION_
#define FILL_VMSTOR_REVISION(RESULT_LVALUE_)				\
{									\
	char *revisionString = REVISION_STRING($Revision: 6 $) + 11;	\
	RESULT_LVALUE_ = 0;						\
	while (*revisionString >= '0' && *revisionString <= '9') {	\
		RESULT_LVALUE_ *= 10;					\
		RESULT_LVALUE_ += *revisionString - '0';		\
		revisionString++;					\
	}								\
}



#define VMSTOR_PROTOCOL_MAJOR(VERSION_)		(((VERSION_) >> 8) & 0xff)
#define VMSTOR_PROTOCOL_MINOR(VERSION_)		(((VERSION_))      & 0xff)
#define VMSTOR_PROTOCOL_VERSION(MAJOR_, MINOR_)	((((MAJOR_) & 0xff) << 8) | \
						 (((MINOR_) & 0xff)))
#define VMSTOR_INVALID_PROTOCOL_VERSION		(-1)





#define VMSTOR_PROTOCOL_VERSION_CURRENT VMSTOR_PROTOCOL_VERSION(2, 0)







#define MAX_TRANSFER_LENGTH	0x40000
#define DEFAULT_PACKET_SIZE (sizeof(struct vmdata_gpa_direct) +	\
			sizeof(struct vstor_packet) +		\
			sizesizeof(u64) * (MAX_TRANSFER_LENGTH / PAGE_SIZE)))



enum vstor_packet_operation {
	VStorOperationCompleteIo            = 1,
	VStorOperationRemoveDevice          = 2,
	VStorOperationExecuteSRB            = 3,
	VStorOperationResetLun              = 4,
	VStorOperationResetAdapter          = 5,
	VStorOperationResetBus              = 6,
	VStorOperationBeginInitialization   = 7,
	VStorOperationEndInitialization     = 8,
	VStorOperationQueryProtocolVersion  = 9,
	VStorOperationQueryProperties       = 10,
	VStorOperationMaximum               = 10
};


#define CDB16GENERIC_LENGTH			0x10

#ifndef SENSE_BUFFER_SIZE
#define SENSE_BUFFER_SIZE			0x12
#endif

#define MAX_DATA_BUFFER_LENGTH_WITH_PADDING	0x14

struct vmscsi_request {
	unsigned short Length;
	unsigned char SrbStatus;
	unsigned char ScsiStatus;

	unsigned char PortNumber;
	unsigned char PathId;
	unsigned char TargetId;
	unsigned char Lun;

	unsigned char CdbLength;
	unsigned char SenseInfoLength;
	unsigned char DataIn;
	unsigned char Reserved;

	unsigned int DataTransferLength;

	union {
	unsigned char Cdb[CDB16GENERIC_LENGTH];

	unsigned char SenseData[SENSE_BUFFER_SIZE];

	unsigned char ReservedArray[MAX_DATA_BUFFER_LENGTH_WITH_PADDING];
	};
} __attribute((packed));



struct vmstorage_channel_properties {
	unsigned short ProtocolVersion;
	unsigned char  PathId;
	unsigned char  TargetId;

	
	unsigned int  PortNumber;
	unsigned int  Flags;
	unsigned int  MaxTransferBytes;

	
	
	unsigned long long UniqueId;
} __attribute__((packed));


struct vmstorage_protocol_version {
	
	unsigned short MajorMinor;

	
	unsigned short Revision;
} __attribute__((packed));


#define STORAGE_CHANNEL_REMOVABLE_FLAG		0x1
#define STORAGE_CHANNEL_EMULATED_IDE_FLAG	0x2

struct vstor_packet {
	
	enum vstor_packet_operation Operation;

	
	unsigned int     Flags;

	
	unsigned int     Status;

	
	union {
		
		struct vmscsi_request VmSrb;

		
		struct vmstorage_channel_properties StorageChannelProperties;

		
		struct vmstorage_protocol_version Version;
	};
} __attribute__((packed));



#define REQUEST_COMPLETION_FLAG	0x1


#define VSC_LEGAL_FLAGS		(REQUEST_COMPLETION_FLAG)
