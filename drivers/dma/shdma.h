
#ifndef __DMA_SHDMA_H
#define __DMA_SHDMA_H

#include <linux/device.h>
#include <linux/dmapool.h>
#include <linux/dmaengine.h>

#define SH_DMA_TCR_MAX 0x00FFFFFF	

struct sh_dmae_regs {
	u32 sar; 
	u32 dar; 
	u32 tcr; 
};

struct sh_desc {
	struct list_head tx_list;
	struct sh_dmae_regs hw;
	struct list_head node;
	struct dma_async_tx_descriptor async_tx;
	int mark;
};

struct sh_dmae_chan {
	dma_cookie_t completed_cookie;	
	spinlock_t desc_lock;			
	struct list_head ld_queue;		
	struct list_head ld_free;		
	struct dma_chan common;			
	struct device *dev;				
	struct tasklet_struct tasklet;	
	int descs_allocated;			
	int id;				
	char dev_id[16];	

	
	int (*set_chcr)(struct sh_dmae_chan *sh_chan, u32 regs);
	
	int (*set_dmars)(struct sh_dmae_chan *sh_chan, u16 res);
};

struct sh_dmae_device {
	struct dma_device common;
	struct sh_dmae_chan *chan[MAX_DMA_CHANNELS];
	struct sh_dmae_pdata pdata;
};

#define to_sh_chan(chan) container_of(chan, struct sh_dmae_chan, common)
#define to_sh_desc(lh) container_of(lh, struct sh_desc, node)
#define tx_to_sh_desc(tx) container_of(tx, struct sh_desc, async_tx)

#endif	
