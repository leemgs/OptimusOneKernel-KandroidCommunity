

#ifndef DIAGCHAR_SHARED
#define DIAGCHAR_SHARED

#define MSG_MASKS_TYPE			1
#define LOG_MASKS_TYPE			2
#define EVENT_MASKS_TYPE		4
#define PKT_TYPE			8
#define DEINIT_TYPE			16
#define MEMORY_DEVICE_LOG_TYPE		32
#define USB_MODE			1
#define MEMORY_DEVICE_MODE		2
#define NO_LOGGING_MODE			3
#define MAX_SYNC_OBJ_NAME_SIZE		32


#define DATA_TYPE_EVENT         	0
#define DATA_TYPE_F3            	1
#define DATA_TYPE_LOG           	2
#define DATA_TYPE_RESPONSE      	3


#define DIAG_IOCTL_COMMAND_REG  	0
#define DIAG_IOCTL_SWITCH_LOGGING	7
#define DIAG_IOCTL_GET_DELAYED_RSP_ID 	8
#define DIAG_IOCTL_LSM_DEINIT		9


struct bindpkt_params {
	uint16_t cmd_code;
	uint16_t subsys_id;
	uint16_t cmd_code_lo;
	uint16_t cmd_code_hi;
	uint16_t proc_id;
	uint32_t event_id;
	uint32_t log_code;
	uint32_t client_id;
};

struct bindpkt_params_per_process {
	
	char sync_obj_name[MAX_SYNC_OBJ_NAME_SIZE];
	uint32_t count;	
	struct bindpkt_params *params; 
};

struct diagpkt_delay_params{
	void *rsp_ptr;
	int size;
	int *num_bytes_ptr;
};

#endif