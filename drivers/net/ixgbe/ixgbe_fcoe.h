

#ifndef _IXGBE_FCOE_H
#define _IXGBE_FCOE_H

#include <scsi/fc/fc_fs.h>
#include <scsi/fc/fc_fcoe.h>


#define IXGBE_RXDADV_FCSTAT_SHIFT	4


#define IXGBE_BUFFCNT_MAX	256	
#define IXGBE_FCPTR_ALIGN	16
#define IXGBE_FCPTR_MAX	(IXGBE_BUFFCNT_MAX * sizeof(dma_addr_t))
#define IXGBE_FCBUFF_4KB	0x0
#define IXGBE_FCBUFF_8KB	0x1
#define IXGBE_FCBUFF_16KB	0x2
#define IXGBE_FCBUFF_64KB	0x3
#define IXGBE_FCBUFF_MAX	65536	
#define IXGBE_FCBUFF_MIN	4096	
#define IXGBE_FCOE_DDP_MAX	512	


#define IXGBE_FCOE_DEFTC	3


#define IXGBE_FCERR_BADCRC       0x00100000

struct ixgbe_fcoe_ddp {
	int len;
	u32 err;
	unsigned int sgc;
	struct scatterlist *sgl;
	dma_addr_t udp;
	u64 *udl;
};

struct ixgbe_fcoe {
	u8 tc;
	spinlock_t lock;
	struct pci_pool *pool;
	struct ixgbe_fcoe_ddp ddp[IXGBE_FCOE_DDP_MAX];
};

#endif 
