
#ifdef CONFIG_CISS_SCSI_TAPE
#ifndef _CCISS_SCSI_H_
#define _CCISS_SCSI_H_

#include <scsi/scsicam.h> 

		
#define SELF_SCSI_ID 15
		
		
		
		
		
		
		
		

#define SCSI_CCISS_CAN_QUEUE 2



struct cciss_scsi_dev_t {
	int devtype;
	int bus, target, lun;		
	unsigned char scsi3addr[8];	
	unsigned char device_id[16];	
	unsigned char vendor[8];	
	unsigned char model[16];	
	unsigned char revision[4];	
};

struct cciss_scsi_hba_t {
	char *name;
	int ndevices;
#define CCISS_MAX_SCSI_DEVS_PER_HBA 16
	struct cciss_scsi_dev_t dev[CCISS_MAX_SCSI_DEVS_PER_HBA];
};

#endif 
#endif 
