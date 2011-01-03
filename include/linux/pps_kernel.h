

#include <linux/pps.h>

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/time.h>




struct pps_source_info {
	char name[PPS_MAX_NAME_LEN];		
	char path[PPS_MAX_NAME_LEN];		
	int mode;				

	void (*echo)(int source, int event, void *data); 

	struct module *owner;
	struct device *dev;
};


struct pps_device {
	struct pps_source_info info;		

	struct pps_kparams params;		

	__u32 assert_sequence;			
	__u32 clear_sequence;			
	struct pps_ktime assert_tu;
	struct pps_ktime clear_tu;
	int current_mode;			

	int go;					
	wait_queue_head_t queue;		

	unsigned int id;			
	struct cdev cdev;
	struct device *dev;
	int devno;
	struct fasync_struct *async_queue;	
	spinlock_t lock;

	atomic_t usage;				
};



extern spinlock_t pps_idr_lock;
extern struct idr pps_idr;
extern struct timespec pps_irq_ts[];

extern struct device_attribute pps_attrs[];



struct pps_device *pps_get_source(int source);
extern void pps_put_source(struct pps_device *pps);
extern int pps_register_source(struct pps_source_info *info,
				int default_params);
extern void pps_unregister_source(int source);
extern int pps_register_cdev(struct pps_device *pps);
extern void pps_unregister_cdev(struct pps_device *pps);
extern void pps_event(int source, struct pps_ktime *ts, int event, void *data);
