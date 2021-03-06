

#ifndef _SR_H
#define _SR_H

#include <linux/genhd.h>
#include <linux/kref.h>

#define MAX_RETRIES	3
#define SR_TIMEOUT	(30 * HZ)

struct scsi_device;



#define IOCTL_TIMEOUT 30*HZ


typedef struct scsi_cd {
	struct scsi_driver *driver;
	unsigned capacity;	
	struct scsi_device *device;
	unsigned int vendor;	
	unsigned long ms_offset;	
	unsigned use:1;		
	unsigned xa_flag:1;	
	unsigned readcd_known:1;	
	unsigned readcd_cdda:1;	
	unsigned previous_state:1;	
	struct cdrom_device_info cdi;
	
	struct kref kref;
	struct gendisk *disk;
} Scsi_CD;

int sr_do_ioctl(Scsi_CD *, struct packet_command *);

int sr_lock_door(struct cdrom_device_info *, int);
int sr_tray_move(struct cdrom_device_info *, int);
int sr_drive_status(struct cdrom_device_info *, int);
int sr_disk_status(struct cdrom_device_info *);
int sr_get_last_session(struct cdrom_device_info *, struct cdrom_multisession *);
int sr_get_mcn(struct cdrom_device_info *, struct cdrom_mcn *);
int sr_reset(struct cdrom_device_info *);
int sr_select_speed(struct cdrom_device_info *cdi, int speed);
int sr_audio_ioctl(struct cdrom_device_info *, unsigned int, void *);

int sr_is_xa(Scsi_CD *);
int sr_test_unit_ready(struct scsi_device *sdev, struct scsi_sense_hdr *sshdr);


void sr_vendor_init(Scsi_CD *);
int sr_cd_check(struct cdrom_device_info *);
int sr_set_blocklength(Scsi_CD *, int blocklength);

#endif
