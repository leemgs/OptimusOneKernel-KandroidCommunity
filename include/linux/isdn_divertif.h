





#define DIVERT_IF_MAGIC 0x25873401
#define DIVERT_CMD_REG  0x00  
#define DIVERT_CMD_REL  0x01  
#define DIVERT_NO_ERR   0x00  
#define DIVERT_CMD_ERR  0x01  
#define DIVERT_VER_ERR  0x02  
#define DIVERT_REG_ERR  0x03  
#define DIVERT_REL_ERR  0x04  
#define DIVERT_REG_NAME isdn_register_divert

#ifdef __KERNEL__
#include <linux/isdnif.h>
#include <linux/types.h>



 
typedef struct
  { ulong if_magic; 
    int cmd; 
    int (*stat_callback)(isdn_ctrl *); 
    int (*ll_cmd)(isdn_ctrl *); 
    char * (*drv_to_name)(int); 
    int (*name_to_drv)(char *); 
  } isdn_divert_if;




extern int DIVERT_REG_NAME(isdn_divert_if *);
#endif
