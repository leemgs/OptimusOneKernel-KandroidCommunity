#ifndef LG_DIAG_KEYPRESS_H
#define LG_DIAG_KEYPRESS_H


#include "lg_comdef.h"

#if defined __GNUC__
  #define PACK(x)       x __attribute__((__packed__))
  #define PACKED        __attribute__((__packed__))
#elif defined __arm
  #define PACK(x)       __packed x
  #define PACKED        __packed
#else
  #error No PACK() macro defined for this compiler
#endif


typedef struct DIAG_HS_KEY_F_req_tag
{
  unsigned char command_code;	
  unsigned char hold;		
  unsigned char key;		
  unsigned long magic1;	
  unsigned long magic2;	
  unsigned short ext_key;    
}PACKED DIAG_HS_KEY_F_req_type;

typedef DIAG_HS_KEY_F_req_type DIAG_HS_KEY_F_rsp_type;

#endif 
