


#ifndef	__BFA_LOG_WDRV_H__
#define	__BFA_LOG_WDRV_H__
#include  <cs/bfa_log.h>
#define BFA_LOG_WDRV_IOC_INIT_ERROR 	\
	(((u32) BFA_LOG_WDRV_ID << BFA_LOG_MODID_OFFSET) | 1)
#define BFA_LOG_WDRV_IOC_INTERNAL_ERROR \
	(((u32) BFA_LOG_WDRV_ID << BFA_LOG_MODID_OFFSET) | 2)
#define BFA_LOG_WDRV_IOC_START_ERROR 	\
	(((u32) BFA_LOG_WDRV_ID << BFA_LOG_MODID_OFFSET) | 3)
#define BFA_LOG_WDRV_IOC_STOP_ERROR 	\
	(((u32) BFA_LOG_WDRV_ID << BFA_LOG_MODID_OFFSET) | 4)
#define BFA_LOG_WDRV_INSUFFICIENT_RESOURCES \
	(((u32) BFA_LOG_WDRV_ID << BFA_LOG_MODID_OFFSET) | 5)
#define BFA_LOG_WDRV_BASE_ADDRESS_MAP_ERROR \
	(((u32) BFA_LOG_WDRV_ID << BFA_LOG_MODID_OFFSET) | 6)
#endif
