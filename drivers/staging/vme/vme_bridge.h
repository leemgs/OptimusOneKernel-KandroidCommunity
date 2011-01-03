#ifndef _VME_BRIDGE_H_
#define _VME_BRIDGE_H_

#define VME_CRCSR_BUF_SIZE (508*1024)
#define VME_SLOTS_MAX 32

struct vme_master_resource {
	struct list_head list;
	struct vme_bridge *parent;
	
	spinlock_t lock;
	int locked;
	int number;
	vme_address_t address_attr;
	vme_cycle_t cycle_attr;
	vme_width_t width_attr;
	struct resource pci_resource;	
	void *kern_base;
};

struct vme_slave_resource {
	struct list_head list;
	struct vme_bridge *parent;
	struct mutex mtx;
	int locked;
	int number;
	vme_address_t address_attr;
	vme_cycle_t cycle_attr;
};

struct vme_dma_pattern {
	u32 pattern;
	vme_pattern_t type;
};

struct vme_dma_pci {
	dma_addr_t address;
};

struct vme_dma_vme {
	unsigned long long address;
	vme_address_t aspace;
	vme_cycle_t cycle;
	vme_width_t dwidth;
};

struct vme_dma_list {
	struct list_head list;
	struct vme_dma_resource *parent;
	struct list_head entries;
	struct mutex mtx;
};

struct vme_dma_resource {
	struct list_head list;
	struct vme_bridge *parent;
	struct mutex mtx;
	int locked;
	int number;
	struct list_head pending;
	struct list_head running;
};

struct vme_lm_resource {
	struct list_head list;
	struct vme_bridge *parent;
	struct mutex mtx;
	int locked;
	int number;
	int monitors;
};

struct vme_bus_error {
	struct list_head list;
	unsigned long long address;
	u32 attributes;
};

struct vme_callback {
	void (*func)(int, int, void*);
	void *priv_data;
};

struct vme_irq {
	int count;
	struct vme_callback callback[255];
};


#define VMENAMSIZ 16


struct vme_bridge {
        char name[VMENAMSIZ];
	int num;
	struct list_head master_resources;
	struct list_head slave_resources;
	struct list_head dma_resources;
	struct list_head lm_resources;

	struct list_head vme_errors;	

	
	struct device *parent;	
	void * base;		

	struct device dev[VME_SLOTS_MAX];	

	
	struct vme_irq irq[7];

	
	int (*slave_get) (struct vme_slave_resource *, int *,
		unsigned long long *, unsigned long long *, dma_addr_t *,
		vme_address_t *, vme_cycle_t *);
	int (*slave_set) (struct vme_slave_resource *, int, unsigned long long,
		unsigned long long, dma_addr_t, vme_address_t, vme_cycle_t);

	
	int (*master_get) (struct vme_master_resource *, int *,
		unsigned long long *, unsigned long long *, vme_address_t *,
		vme_cycle_t *, vme_width_t *);
	int (*master_set) (struct vme_master_resource *, int,
		unsigned long long, unsigned long long,  vme_address_t,
		vme_cycle_t, vme_width_t);
	ssize_t (*master_read) (struct vme_master_resource *, void *, size_t,
		loff_t);
	ssize_t (*master_write) (struct vme_master_resource *, void *, size_t,
		loff_t);
	unsigned int (*master_rmw) (struct vme_master_resource *, unsigned int,
		unsigned int, unsigned int, loff_t);

	
	int (*dma_list_add) (struct vme_dma_list *, struct vme_dma_attr *,
		struct vme_dma_attr *, size_t);
	int (*dma_list_exec) (struct vme_dma_list *);
	int (*dma_list_empty) (struct vme_dma_list *);

	
	int (*request_irq) (int, int, void (*cback)(int, int, void*), void *);
	void (*free_irq) (int, int);
	int (*generate_irq) (int, int);

	
	int (*lm_set) (struct vme_lm_resource *, unsigned long long,
		vme_address_t, vme_cycle_t);
	int (*lm_get) (struct vme_lm_resource *, unsigned long long *,
		vme_address_t *, vme_cycle_t *);
	int (*lm_attach) (struct vme_lm_resource *, int, void (*callback)(int));
	int (*lm_detach) (struct vme_lm_resource *, int);

	
	int (*slot_get) (void);
	

#if 0
	int (*set_prefetch) (void);
	int (*get_prefetch) (void);
	int (*set_arbiter) (void);
	int (*get_arbiter) (void);
	int (*set_requestor) (void);
	int (*get_requestor) (void);
#endif
};

int vme_register_bridge (struct vme_bridge *);
void vme_unregister_bridge (struct vme_bridge *);

#endif 

#if 0

struct vmeInfoCfg {
	int vmeSlotNum;		
	int boardResponded;	
	char sysConFlag;	
	int vmeControllerID;	
	int vmeControllerRev;	
	char osName[8];		
	int vmeSharedDataValid;	
	int vmeDriverRev;	
	unsigned int vmeAddrHi[8];	
	unsigned int vmeAddrLo[8];	
	unsigned int vmeSize[8];	
	unsigned int vmeAm[8];	
	int reserved;		
};
typedef struct vmeInfoCfg vmeInfoCfg_t;


struct vmeRequesterCfg {
	int requestLevel;	
	char fairMode;		
	int releaseMode;	
	int timeonTimeoutTimer;	
	int timeoffTimeoutTimer;	
	int reserved;		
};
typedef struct vmeRequesterCfg vmeRequesterCfg_t;


struct vmeArbiterCfg {
	vme_arbitration_t arbiterMode;	
	char arbiterTimeoutFlag;	
	int globalTimeoutTimer;	
	char noEarlyReleaseFlag;	
	int reserved;		
};
typedef struct vmeArbiterCfg vmeArbiterCfg_t;



struct vmeRmwCfg {
	unsigned int targetAddrU;	
	unsigned int targetAddr;	
	vme_address_t addrSpace;	
	int enableMask;		
	int compareData;	
	int swapData;		
	int maxAttempts;	
	int numAttempts;	
	int reserved;		

};
typedef struct vmeRmwCfg vmeRmwCfg_t;


struct vmeLmCfg {
	unsigned int addrU;	
	unsigned int addr;	
	vme_address_t addrSpace;	
	int userAccessType;	
	int dataAccessType;	
	int lmWait;		
	int lmEvents;		
	int reserved;		
};
typedef struct vmeLmCfg vmeLmCfg_t;
#endif
