#include <linux/spinlock.h>

struct cpustate_t {
	spinlock_t lock;
	int excl;
        int open_count;
	unsigned char cached_val;
	int inited;
	unsigned long *set_addr;
	unsigned long *clr_addr;
};


#define HDPU_CPUSTATE_NAME "hdpu cpustate"
#define HDPU_NEXUS_NAME "hdpu nexus"

#define CPUSTATE_KERNEL_MAJOR  0x10

#define CPUSTATE_KERNEL_INIT_DRV   0 
#define CPUSTATE_KERNEL_INIT_PCI   1 
#define CPUSTATE_KERNEL_INIT_REG   2 
#define CPUSTATE_KERNEL_CPU1_KICK  3 
#define CPUSTATE_KERNEL_CPU1_OK    4  
#define CPUSTATE_KERNEL_OK         5 
#define CPUSTATE_KERNEL_RESET   14 
#define CPUSTATE_KERNEL_HALT   15 
