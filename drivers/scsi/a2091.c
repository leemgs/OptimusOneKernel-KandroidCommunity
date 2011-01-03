#include <linux/types.h>
#include <linux/mm.h>
#include <linux/blkdev.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#include <asm/setup.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/amigaints.h>
#include <asm/amigahw.h>
#include <linux/zorro.h>
#include <asm/irq.h>
#include <linux/spinlock.h>

#include "scsi.h"
#include <scsi/scsi_host.h>
#include "wd33c93.h"
#include "a2091.h"

#include<linux/stat.h>

#define DMA(ptr) ((a2091_scsiregs *)((ptr)->base))
#define HDATA(ptr) ((struct WD33C93_hostdata *)((ptr)->hostdata))

static int a2091_release(struct Scsi_Host *instance);

static irqreturn_t a2091_intr (int irq, void *_instance)
{
    unsigned long flags;
    unsigned int status;
    struct Scsi_Host *instance = (struct Scsi_Host *)_instance;

    status = DMA(instance)->ISTR;
    if (!(status & (ISTR_INT_F|ISTR_INT_P)) || !(status & ISTR_INTS))
	return IRQ_NONE;

    spin_lock_irqsave(instance->host_lock, flags);
    wd33c93_intr(instance);
    spin_unlock_irqrestore(instance->host_lock, flags);
    return IRQ_HANDLED;
}

static int dma_setup(struct scsi_cmnd *cmd, int dir_in)
{
    unsigned short cntr = CNTR_PDMD | CNTR_INTEN;
    unsigned long addr = virt_to_bus(cmd->SCp.ptr);
    struct Scsi_Host *instance = cmd->device->host;

    
    if (addr & A2091_XFER_MASK)
    {
	HDATA(instance)->dma_bounce_len = (cmd->SCp.this_residual + 511)
	    & ~0x1ff;
	HDATA(instance)->dma_bounce_buffer =
	    kmalloc (HDATA(instance)->dma_bounce_len, GFP_KERNEL);
	
	
	if (!HDATA(instance)->dma_bounce_buffer) {
	    HDATA(instance)->dma_bounce_len = 0;
	    return 1;
	}

	
	addr = virt_to_bus(HDATA(instance)->dma_bounce_buffer);

	
	if (addr & A2091_XFER_MASK) {
	    
	    kfree (HDATA(instance)->dma_bounce_buffer);
	    HDATA(instance)->dma_bounce_buffer = NULL;
	    HDATA(instance)->dma_bounce_len = 0;
	    return 1;
	}

	if (!dir_in) {
		
		memcpy (HDATA(instance)->dma_bounce_buffer,
			cmd->SCp.ptr, cmd->SCp.this_residual);
	}
    }

    
    if (!dir_in)
	cntr |= CNTR_DDIR;

    
    HDATA(cmd->device->host)->dma_dir = dir_in;

    DMA(cmd->device->host)->CNTR = cntr;

    
    DMA(cmd->device->host)->ACR = addr;

    if (dir_in){
	
	cache_clear (addr, cmd->SCp.this_residual);
    }else{
	
	cache_push (addr, cmd->SCp.this_residual);
      }
    
    DMA(cmd->device->host)->ST_DMA = 1;

    
    return 0;
}

static void dma_stop(struct Scsi_Host *instance, struct scsi_cmnd *SCpnt,
		      int status)
{
    
    unsigned short cntr = CNTR_PDMD;

    if (!HDATA(instance)->dma_dir)
	    cntr |= CNTR_DDIR;

    
    DMA(instance)->CNTR = cntr;

    
    if (HDATA(instance)->dma_dir) {
	DMA(instance)->FLUSH = 1;
	while (!(DMA(instance)->ISTR & ISTR_FE_FLG))
	    ;
    }

    
    DMA(instance)->CINT = 1;

    
    DMA(instance)->SP_DMA = 1;

    
    DMA(instance)->CNTR = CNTR_PDMD | CNTR_INTEN;

    
    if (status && HDATA(instance)->dma_bounce_buffer) {
	if( HDATA(instance)->dma_dir )
		memcpy (SCpnt->SCp.ptr, 
			HDATA(instance)->dma_bounce_buffer,
			SCpnt->SCp.this_residual);
	kfree (HDATA(instance)->dma_bounce_buffer);
	HDATA(instance)->dma_bounce_buffer = NULL;
	HDATA(instance)->dma_bounce_len = 0;
    }
}

static int __init a2091_detect(struct scsi_host_template *tpnt)
{
    static unsigned char called = 0;
    struct Scsi_Host *instance;
    unsigned long address;
    struct zorro_dev *z = NULL;
    wd33c93_regs regs;
    int num_a2091 = 0;

    if (!MACH_IS_AMIGA || called)
	return 0;
    called = 1;

    tpnt->proc_name = "A2091";
    tpnt->proc_info = &wd33c93_proc_info;

    while ((z = zorro_find_device(ZORRO_WILDCARD, z))) {
	if (z->id != ZORRO_PROD_CBM_A590_A2091_1 &&
	    z->id != ZORRO_PROD_CBM_A590_A2091_2)
	    continue;
	address = z->resource.start;
	if (!request_mem_region(address, 256, "wd33c93"))
	    continue;

	instance = scsi_register (tpnt, sizeof (struct WD33C93_hostdata));
	if (instance == NULL)
	    goto release;
	instance->base = ZTWO_VADDR(address);
	instance->irq = IRQ_AMIGA_PORTS;
	instance->unique_id = z->slotaddr;
	DMA(instance)->DAWR = DAWR_A2091;
	regs.SASR = &(DMA(instance)->SASR);
	regs.SCMD = &(DMA(instance)->SCMD);
	HDATA(instance)->no_sync = 0xff;
	HDATA(instance)->fast = 0;
	HDATA(instance)->dma_mode = CTRL_DMA;
	wd33c93_init(instance, regs, dma_setup, dma_stop, WD33C93_FS_8_10);
	if (request_irq(IRQ_AMIGA_PORTS, a2091_intr, IRQF_SHARED, "A2091 SCSI",
			instance))
	    goto unregister;
	DMA(instance)->CNTR = CNTR_PDMD | CNTR_INTEN;
	num_a2091++;
	continue;

unregister:
	scsi_unregister(instance);
	wd33c93_release();
release:
	release_mem_region(address, 256);
    }

    return num_a2091;
}

static int a2091_bus_reset(struct scsi_cmnd *cmd)
{
	

	

	spin_lock_irq(cmd->device->host->host_lock);
	wd33c93_host_reset(cmd);
	spin_unlock_irq(cmd->device->host->host_lock);

	return SUCCESS;
}

#define HOSTS_C

static struct scsi_host_template driver_template = {
	.proc_name		= "A2901",
	.name			= "Commodore A2091/A590 SCSI",
	.detect			= a2091_detect,
	.release		= a2091_release,
	.queuecommand		= wd33c93_queuecommand,
	.eh_abort_handler	= wd33c93_abort,
	.eh_bus_reset_handler	= a2091_bus_reset,
	.eh_host_reset_handler	= wd33c93_host_reset,
	.can_queue		= CAN_QUEUE,
	.this_id		= 7,
	.sg_tablesize		= SG_ALL,
	.cmd_per_lun		= CMD_PER_LUN,
	.use_clustering		= DISABLE_CLUSTERING
};


#include "scsi_module.c"

static int a2091_release(struct Scsi_Host *instance)
{
#ifdef MODULE
	DMA(instance)->CNTR = 0;
	release_mem_region(ZTWO_PADDR(instance->base), 256);
	free_irq(IRQ_AMIGA_PORTS, instance);
	wd33c93_release();
#endif
	return 1;
}

MODULE_LICENSE("GPL");
