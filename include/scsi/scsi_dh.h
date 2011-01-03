

#include <scsi/scsi_device.h>

enum {
	SCSI_DH_OK = 0,
	
	SCSI_DH_DEV_FAILED,	
	SCSI_DH_DEV_TEMP_BUSY,
	SCSI_DH_DEV_UNSUPP,	
	SCSI_DH_DEVICE_MAX,	

	
	SCSI_DH_NOTCONN = SCSI_DH_DEVICE_MAX + 1,
	SCSI_DH_CONN_FAILURE,
	SCSI_DH_TRANSPORT_MAX,	

	
	SCSI_DH_IO = SCSI_DH_TRANSPORT_MAX + 1,	
	SCSI_DH_INVALID_IO,
	SCSI_DH_RETRY,		
	SCSI_DH_IMM_RETRY,	
	SCSI_DH_TIMED_OUT,
	SCSI_DH_RES_TEMP_UNAVAIL,
	SCSI_DH_DEV_OFFLINED,
	SCSI_DH_NOSYS,
	SCSI_DH_DRIVER_MAX,
};
#if defined(CONFIG_SCSI_DH) || defined(CONFIG_SCSI_DH_MODULE)
extern int scsi_dh_activate(struct request_queue *);
extern int scsi_dh_handler_exist(const char *);
extern int scsi_dh_attach(struct request_queue *, const char *);
extern void scsi_dh_detach(struct request_queue *);
extern int scsi_dh_set_params(struct request_queue *, const char *);
#else
static inline int scsi_dh_activate(struct request_queue *req)
{
	return 0;
}
static inline int scsi_dh_handler_exist(const char *name)
{
	return 0;
}
static inline int scsi_dh_attach(struct request_queue *req, const char *name)
{
	return SCSI_DH_NOSYS;
}
static inline void scsi_dh_detach(struct request_queue *q)
{
	return;
}
static inline int scsi_dh_set_params(struct request_queue *req, const char *params)
{
	return -SCSI_DH_NOSYS;
}
#endif
