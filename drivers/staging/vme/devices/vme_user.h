#ifndef _VME_USER_H_
#define _VME_USER_H_

#define USER_BUS_MAX                  1


struct vme_master {
	int enable;			
	unsigned long long vme_addr;	
	unsigned long long size;	
	vme_address_t aspace;		
	vme_cycle_t cycle;		
	vme_width_t dwidth;		
#if 0
	char prefetchEnable;		
	int prefetchSize;		
	char wrPostEnable;		
#endif
};





#define VME_IOC_MAGIC 0xAE



struct vme_slave {
	int enable;			
	unsigned long long vme_addr;	
	unsigned long long size;	
	vme_address_t aspace;		
	vme_cycle_t cycle;		
#if 0
	char wrPostEnable;		
	char rmwLock;			
	char data64BitCapable;		
#endif
};

#define VME_GET_SLAVE _IOR(VME_IOC_MAGIC, 1, struct vme_slave)
#define VME_SET_SLAVE _IOW(VME_IOC_MAGIC, 2, struct vme_slave)
#define VME_GET_MASTER _IOR(VME_IOC_MAGIC, 3, struct vme_master)
#define VME_SET_MASTER _IOW(VME_IOC_MAGIC, 4, struct vme_master)

#endif 

