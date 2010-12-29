#ifndef DIAGMTC_H
#define DIAGMTC_H



#include <mach/lg_comdef.h>






#if defined __GNUC__
  #define PACK(x)       x __attribute__((__packed__))
  #define PACKED        __attribute__((__packed__))
#elif defined __arm
  #define PACK(x)       __packed x
  #define PACKED        __packed
#else
  #error No PACK() macro defined for this compiler
#endif



#if 1	
#define LG_FW_ATS_ETA_MTC_KEY_LOGGING
#endif


typedef enum
{
  MTC_INFO_REQ_CMD				= 0x00, 
  MTC_CAPTURE_REQ_CMD 			= 0x01, 
  MTC_KEY_EVENT_REQ_CMD			= 0x03, 
  MTC_TOUCH_REQ_CMD				= 0x04,

#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined (LG_FW_ATS_ETA_MTC_KEY_LOGGING)
  MTC_LOGGING_MASK_REQ_CMD		= 0x07,
  MTC_LOG_REQ_CMD 					= 0x08,
#endif 

  MTC_SERIALIZED_DATA_REQ_CMD       = 0x0A,
  MTC_SERIALIZED_CAPTURE_REQ_CMD = 0x0B,
  MTC_PHONE_RESTART_REQ_CMD		= 0x10, 
  MTC_FACTORY_RESET				= 0x11,
  MTC_PHONE_REPORT				= 0x20,
  MTC_PHONE_STATE 					= 0x21,
  MTC_CAPTURE_PROP				= 0x22,
  MTC_NOTIFICATION_REQUEST		= 0x30,
  MTC_CUR_PROC_NAME_REQ_CMD		= 0x32, 
  MTC_KEY_EVENT_UNIV_REQ_CMD		= 0x33, 
  MTC_MEMORY_DUMP 				= 0x40,
  MTC_BATTERY_POWER				= 0x41,
  MTC_BACKLIGHT_INFO				= 0x42,
  MTC_FLASH_MODE					= 0x60,
  MTC_MODEM_MODE					= 0x61,
  MTC_CELL_INFORMATION			= 0x80, 
  MTC_HANDOVER						= 0x81,
  MTC_ERROR_CMD					= 0x7F,
  MTC_MAX_CMD						= 0xFF,
} mtc_sub_cmd_type;


#if defined __GNUC__
  #define PACK(x)       x __attribute__((__packed__))
  #define PACKED        __attribute__((__packed__))
#elif defined __arm
  #define PACK(x)       __packed x
  #define PACKED        __packed
#else
  #error No PACK() macro defined for this compiler
#endif


#define MTC_262K_CMASK_RED			0x0000FC00
#define MTC_262K_CMASK_GREEN			0x000003F0
#define MTC_262K_CMASK_BLUE 			0xC000000F

#define MTC_65K_CMASK_RED				0xF800
#define MTC_65K_CMASK_GREEN 			0x07E0
#define MTC_65K_CMASK_BLUE			0x001F

typedef  struct
{
  unsigned char cmd_code;
  unsigned char sub_cmd;
} PACKED mtc_req_hdr_type;

typedef enum
{
  MTC_MAIN_LCD = 0, 
  MTC_SUB_LCD,	     	
  MTC_LCD_PADDING = 0x7F
}PACKED mtc_scrn_id_type;

typedef struct
{
  mtc_req_hdr_type hdr;
  mtc_scrn_id_type screen_id;
} PACKED mtc_info_req_type;

typedef struct
{
  mtc_req_hdr_type hdr;
  mtc_scrn_id_type screen_id;
  unsigned short left;      
  unsigned short top;       
  unsigned short width;     
  unsigned short height;    
} PACKED mtc_capture_req_type;

typedef enum
{
  MTC_KEY_UP = 0,
  MTC_KEY_DOWN
} PACKED mtc_key_hold_type;

typedef struct
{
  mtc_req_hdr_type hdr;
  mtc_key_hold_type hold;
  unsigned char key_code;
} PACKED mtc_key_req_type;

typedef enum
{
  MTC_TOUCH_MOVETO	= 0,
  MTC_TOUCH_MOVEBY,
  MTC_TOUCH_TAB,
  MTC_TOUCH_DOUBLETAB,
  MTC_TOUCH_DOWN,
  MTC_TOUCH_UP
} PACKED mtc_touch_action_type;

typedef struct
{
  mtc_req_hdr_type hdr;
  mtc_scrn_id_type screen_id;
  mtc_touch_action_type action;
  unsigned short x;
  unsigned short y;
} PACKED mtc_touch_req_type;

#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined (LG_FW_ATS_ETA_MTC_KEY_LOGGING)
typedef struct
{
  mtc_req_hdr_type hdr;
  unsigned long mask;
} PACKED mtc_log_req_type;
#endif 

typedef struct
{
  mtc_req_hdr_type hdr;
  unsigned short seqeunce;
} PACKED mtc_serialized_data_req_type;

typedef struct
{
  mtc_req_hdr_type hdr;
  mtc_scrn_id_type screen_id;
  unsigned short left;      
  unsigned short top;       
  unsigned short width;     
  unsigned short height;    
} PACKED mtc_serialized_capture_req_type;

typedef union
{
  mtc_info_req_type info;
  mtc_capture_req_type capture;
  mtc_key_req_type key;	
  mtc_touch_req_type touch;
#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined (LG_FW_ATS_ETA_MTC_KEY_LOGGING)
  mtc_log_req_type log;
#endif 
  mtc_serialized_data_req_type serial_data;
  mtc_serialized_capture_req_type serial_capture;
} PACKED mtc_req_type;

typedef union{
  mtc_req_hdr_type hdr;
  mtc_req_type mtc_req;
} PACKED DIAG_MTC_F_req_type;


#define MTC_SCRN_BUF_SIZE_MAX (320*480*(sizeof(unsigned short))) 
#define MTC_SCRN_BUF_SIZE (1024*2)

typedef enum
{
  MTC_BIT_UNDEF   = 0,
  MTC_BIT_MONO    = 1,
  MTC_BIT_4       = 2,
  MTC_BIT_16      = 4,
  MTC_BIT_256     = 8,
  MTC_BIT_65K     = 16,
  MTC_BIT_262K    = 18,
  MTC_BIT_RGB24   = 24,
  MTC_BIT_RGB32   = 32,
  MBT_BIT_PADDING = 0x7F
} PACKED mtc_bits_pixel_type;

typedef struct
{
  mtc_req_hdr_type hdr;
  mtc_scrn_id_type scrn_id;
  unsigned short scrn_width;                 
  unsigned short scrn_height;                
  mtc_bits_pixel_type bits_pixel;  
} PACKED mtc_info_rsp_type;

typedef struct
{
  unsigned int blue;
  unsigned int green;
  unsigned int red;
} PACKED mtc_mask_type;

typedef struct
{
  mtc_req_hdr_type hdr;
  mtc_scrn_id_type scrn_id;
  unsigned short bmp_width;
  unsigned short bmp_height;
  mtc_bits_pixel_type bits_pixel;
  mtc_mask_type mask;
  unsigned char bmp_data[MTC_SCRN_BUF_SIZE_MAX];
} PACKED mtc_capture_rsp_type;

#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined (LG_FW_ATS_ETA_MTC_KEY_LOGGING)
typedef struct
{
  unsigned long long time; 
  unsigned char hold; 
  unsigned char keycode;
  unsigned long long active_uiid; 
} PACKED mtc_log_data_key_type;

typedef struct
{
  unsigned long long time; 
  unsigned char screen_id;
  unsigned char action;
  unsigned short x;
  unsigned short y;
  unsigned long long active_uiid; 
} PACKED mtc_log_data_touch_type;

typedef union{
  mtc_log_data_key_type log_data_key;
  mtc_log_data_touch_type log_data_touch;
} PACKED log_data_type;

typedef struct
{
  mtc_req_hdr_type hdr;
  unsigned char log_id;
  unsigned short log_len;
  log_data_type log_type;
} PACKED mtc_log_data_rsp_type;

#endif 

typedef struct
{
  mtc_req_hdr_type hdr;
  unsigned short seqeunce;
  unsigned long length;
  unsigned char bmp_data[MTC_SCRN_BUF_SIZE];
} PACKED mtc_serialized_data_rsp_type;

typedef struct
{
  mtc_req_hdr_type hdr;
  mtc_scrn_id_type screen_id;
  unsigned long bmp_size;
  unsigned short bmp_width;
  unsigned short bmp_height;
  mtc_bits_pixel_type bits_pixel;
  mtc_mask_type mask;
} PACKED mtc_serialized_capture_rsp_type;

typedef union
{
  mtc_info_rsp_type info;
  mtc_capture_rsp_type capture;
  mtc_key_req_type key;
  mtc_touch_req_type touch;
#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined (LG_FW_ATS_ETA_MTC_KEY_LOGGING)
  mtc_log_req_type log;
  mtc_log_data_rsp_type log_data;
  mtc_serialized_data_rsp_type serial_data;
  mtc_serialized_capture_rsp_type serial_capture;
#endif 
} PACKED mtc_rsp_type;

typedef union
{
  mtc_req_hdr_type hdr;
  mtc_rsp_type mtc_rsp;
} PACKED DIAG_MTC_F_rsp_type;

typedef struct
{
  mtc_scrn_id_type id;		
  int width_max;			
  int height_max;			
  int degrees;				
  mtc_bits_pixel_type bits_pixel;  
  mtc_mask_type mask;		
}PACKED mtc_lcd_info_type;

#define MTC_MSTR_TBL_SIZE   0xFF


#if !defined (ARM9_PROCESSOR) && !defined (ARM11_PROCESSOR)
typedef enum{
  MTC_ARM9_PROCESSOR = 0,
  MTC_ARM11_PROCESSOR = 1,
  MTC_ARM9_ARM11_BOTH = 2,
  MTC_NOT_SUPPORTED = 0xFF
}mtc_which_processor_type;
#endif

typedef DIAG_MTC_F_rsp_type*(* mtc_func_type)(DIAG_MTC_F_req_type *);


typedef struct
{
  unsigned short cmd_code;
  mtc_func_type func_ptr;
  unsigned char which_procesor;             
}mtc_user_table_entry_type;


#if defined (CONFIG_MACH_MSM7X27_THUNDERC) || defined (LG_FW_ATS_ETA_MTC_KEY_LOGGING)
struct ats_mtc_key_log_type{
	unsigned char log_id;
	unsigned short log_len;
	unsigned int x_hold;
	unsigned int y_code;
	unsigned char action;
};

enum ats_mtc_key_log_id_type{
	ATS_MTC_KEY_LOG_ID_KEY = 1,
	ATS_MTC_KEY_LOG_ID_TOUCH = 2,
	ATS_MTC_KEY_LOG_ID_MAX,
};
#endif 




#endif 
