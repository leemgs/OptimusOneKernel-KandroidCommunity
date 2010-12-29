

#define ACDB_DAL_DEVICE		0x02000069
#define ACDB_DAL_PORT		"DAL_AM_AUD"
#define ACDB_DAL_VERSION	0x00010000

#define ACDB_OP_IOCTL		DAL_OP_FIRST_DEVICE_API


#define ACDB_GET_DEVICE		0x0108bb92
#define ACDB_SET_DEVICE		0x0108bb93
#define ACDB_GET_STREAM		0x0108bb95
#define ACDB_SET_STREAM		0x0108bb96
#define ACDB_GET_DEVICE_TABLE	0x0108bb97
#define ACDB_GET_STREAM_TABLE	0x0108bb98

#define ACDB_RES_SUCCESS	0
#define ACDB_RES_FAILURE	-1
#define ACDB_RES_BADPARM	-2
#define ACDB_RES_BADSTATE	-3

struct acdb_cmd_device {
	uint32_t size;

	uint32_t command_id;
	uint32_t device_id;
	uint32_t network_id;
	uint32_t sample_rate_id;
	uint32_t interface_id;
	uint32_t algorithm_block_id;

	
	uint32_t total_bytes;
	uint32_t unmapped_buf;
} __attribute__((packed));

struct acdb_cmd_device_table {
	uint32_t size;

	uint32_t command_id;
	uint32_t device_id;
	uint32_t network_id;
	uint32_t sample_rate_id;

	
	uint32_t total_bytes;
	uint32_t unmapped_buf;

	uint32_t res_size;
} __attribute__((packed));

struct acdb_result {
	uint32_t dal_status;
	uint32_t size;

	uint32_t total_devices;
	uint32_t unmapped_buf;
	uint32_t used_bytes;
	uint32_t result;
} __attribute__((packed));
