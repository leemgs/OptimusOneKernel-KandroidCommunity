#ifndef LG_DIAG_KERNEL_SERVICE_H
#define LG_DIAG_KERNEL_SERVICE_H

#include <mach/lg_comdef.h>



#define DIAGPKT_HDR_PATTERN (0xDEADD00DU)
#define DIAGPKT_OVERRUN_PATTERN (0xDEADU)
#define DIAGPKT_USER_TBL_SIZE 10
#define READ_BUF_SIZE 8004

#define DIAG_DATA_TYPE_EVENT         0
#define DIAG_DATA_TYPE_F3            1
#define DIAG_DATA_TYPE_LOG           2
#define DIAG_DATA_TYPE_RESPONSE      3
#define DIAG_DATA_TYPE_DELAYED_RESPONSE   4

#define DIAGPKT_NO_SUBSYS_ID 0xFF

#define TRUE 1
#define FALSE 0



#if defined __GNUC__
  #define PACK(x)       x __attribute__((__packed__))
  #define PACKED        __attribute__((__packed__))
#elif defined __arm
  #define PACK(x)       __packed x
  #define PACKED        __packed
#else
  #error No PACK() macro defined for this compiler
#endif


typedef struct
{
  word cmd_code_lo;
  word cmd_code_hi;
  PACK(void *)(*func_ptr) (PACK(void *)req_pkt_ptr, uint16 pkt_len);
} diagpkt_user_table_entry_type;

typedef struct
{
	 uint16 cmd_code;
	 uint16 subsys_id;
	 uint16 cmd_code_lo;
	 uint16 cmd_code_hi;
	 uint16 proc_id;
	 uint32 event_id;
	 uint32 log_code;
	 uint32 client_id;
} bindpkt_params;

#define MAX_SYNC_OBJ_NAME_SIZE 32
typedef struct
{
	 char sync_obj_name[MAX_SYNC_OBJ_NAME_SIZE]; 
	 uint32 count; 
	 bindpkt_params *params; 
}
bindpkt_params_per_process;




typedef struct
{ uint16 delay_flag;  
  uint16 cmd_code;
  word subsysid;
  word count;
  uint16 proc_id;
  const diagpkt_user_table_entry_type *user_table;
} diagpkt_user_table_type;

typedef struct
{
  uint8 command_code;
}
diagpkt_hdr_type;

typedef struct
{
  uint8 command_code;
  uint8 subsys_id;
  uint16 subsys_cmd_code;
}
diagpkt_subsys_hdr_type;

typedef struct
{
  uint8 command_code;
  uint8 subsys_id;
  uint16 subsys_cmd_code;
  uint32 status;  
  uint16 delayed_rsp_id;
  uint16 rsp_cnt; 
}
diagpkt_subsys_hdr_type_v2;

typedef struct
{
  unsigned int pattern;     
  unsigned int size;        
  unsigned int length;      



#if 1	
  byte pkt[4096];               
#else
  byte pkt[1024];               
#endif

} diagpkt_rsp_type;

typedef void (*diag_cmd_rsp) (const byte *rsp, unsigned int length, void *param);

typedef struct
{
  diag_cmd_rsp rsp_func; 
  void *rsp_func_param;

  diagpkt_rsp_type rsp; 
} diagpkt_lsm_rsp_type;

typedef struct
{
  uint32 diag_data_type; 
  uint8 rest_of_data;
} diag_data;

#define FPOS( type, field ) \
     ( (dword) &(( type *) 0)-> field ) 

#define DIAG_REST_OF_DATA_POS (FPOS(diag_data, rest_of_data))

#define DIAGPKT_DISPATCH_TABLE_REGISTER(xx_subsysid, xx_entry) \
	do { \
		static const diagpkt_user_table_type xx_entry##_table = { \
		 0, 0xFF, xx_subsysid, sizeof (xx_entry) / sizeof (xx_entry[0]), 1, xx_entry \
		}; \
	  \
		diagpkt_tbl_reg (&xx_entry##_table); \
	} while (0)

#define DIAGPKT_PKT2LSMITEM(p) \
		((diagpkt_lsm_rsp_type *) (((byte *) p) - FPOS (diagpkt_lsm_rsp_type, rsp.pkt)))

#endif 
