#ifndef CCISS_H
#define CCISS_H

#include <linux/genhd.h>
#include <linux/mutex.h>

#include "cciss_cmd.h"


#define NWD_SHIFT	4
#define MAX_PART	(1 << NWD_SHIFT)

#define IO_OK		0
#define IO_ERROR	1
#define IO_NEEDS_RETRY  3

#define VENDOR_LEN	8
#define MODEL_LEN	16
#define REV_LEN		4

struct ctlr_info;
typedef struct ctlr_info ctlr_info_t;

struct access_method {
	void (*submit_command)(ctlr_info_t *h, CommandList_struct *c);
	void (*set_intr_mask)(ctlr_info_t *h, unsigned long val);
	unsigned long (*fifo_full)(ctlr_info_t *h);
	unsigned long (*intr_pending)(ctlr_info_t *h);
	unsigned long (*command_completed)(ctlr_info_t *h);
};
typedef struct _drive_info_struct
{
	unsigned char LunID[8];
	int 	usage_count;
	struct request_queue *queue;
	sector_t nr_blocks;
	int	block_size;
	int 	heads;
	int	sectors;
	int 	cylinders;
	int	raid_level; 
	int	busy_configuring; 
	struct	device dev;
	__u8 serial_no[16]; 
	char vendor[VENDOR_LEN + 1]; 
	char model[MODEL_LEN + 1];   
	char rev[REV_LEN + 1];       
	char device_initialized;     
} drive_info_struct;

struct ctlr_info 
{
	int	ctlr;
	char	devname[8];
	char    *product_name;
	char	firm_ver[4]; 
	struct pci_dev *pdev;
	__u32	board_id;
	void __iomem *vaddr;
	unsigned long paddr;
	int 	nr_cmds; 
	CfgTable_struct __iomem *cfgtable;
	int	interrupts_enabled;
	int	major;
	int 	max_commands;
	int	commands_outstanding;
	int 	max_outstanding;  
	int	num_luns;
	int 	highest_lun;
	int	usage_count;  
#	define DOORBELL_INT	0
#	define PERF_MODE_INT	1
#	define SIMPLE_MODE_INT	2
#	define MEMQ_MODE_INT	3
	unsigned int intr[4];
	unsigned int msix_vector;
	unsigned int msi_vector;
	int 	cciss_max_sectors;
	BYTE	cciss_read;
	BYTE	cciss_write;
	BYTE	cciss_read_capacity;

	
	drive_info_struct *drv[CISS_MAX_LUN];

	struct access_method access;

	 
	struct hlist_head reqQ;
	struct hlist_head cmpQ;
	unsigned int Qdepth;
	unsigned int maxQsinceinit;
	unsigned int maxSG;
	spinlock_t lock;

	
	CommandList_struct 	*cmd_pool;
	dma_addr_t		cmd_pool_dhandle; 
	ErrorInfo_struct 	*errinfo_pool;
	dma_addr_t		errinfo_pool_dhandle; 
        unsigned long  		*cmd_pool_bits;
	int			nr_allocs;
	int			nr_frees; 
	int			busy_configuring;
	int			busy_initializing;
	int			busy_scanning;
	struct mutex		busy_shutting_down;

	
	int			next_to_run;

	
	struct gendisk   *gendisk[CISS_MAX_LUN];
#ifdef CONFIG_CISS_SCSI_TAPE
	void *scsi_ctlr; 
	
	
#endif
	unsigned char alive;
	struct list_head scan_list;
	struct completion scan_wait;
	struct device dev;
};



#define SA5_DOORBELL	0x20
#define SA5_REQUEST_PORT_OFFSET	0x40
#define SA5_REPLY_INTR_MASK_OFFSET	0x34
#define SA5_REPLY_PORT_OFFSET		0x44
#define SA5_INTR_STATUS		0x30
#define SA5_SCRATCHPAD_OFFSET	0xB0

#define SA5_CTCFG_OFFSET	0xB4
#define SA5_CTMEM_OFFSET	0xB8

#define SA5_INTR_OFF		0x08
#define SA5B_INTR_OFF		0x04
#define SA5_INTR_PENDING	0x08
#define SA5B_INTR_PENDING	0x04
#define FIFO_EMPTY		0xffffffff	
#define CCISS_FIRMWARE_READY	0xffff0000 

#define  CISS_ERROR_BIT		0x02

#define CCISS_INTR_ON 	1 
#define CCISS_INTR_OFF	0

static void SA5_submit_command( ctlr_info_t *h, CommandList_struct *c) 
{
#ifdef CCISS_DEBUG
	 printk("Sending %x - down to controller\n", c->busaddr );
#endif  
         writel(c->busaddr, h->vaddr + SA5_REQUEST_PORT_OFFSET);
	 h->commands_outstanding++;
	 if ( h->commands_outstanding > h->max_outstanding)
		h->max_outstanding = h->commands_outstanding;
}


static void SA5_intr_mask(ctlr_info_t *h, unsigned long val)
{
	if (val) 
	{ 
		h->interrupts_enabled = 1;
		writel(0, h->vaddr + SA5_REPLY_INTR_MASK_OFFSET);
	} else 
	{
		h->interrupts_enabled = 0;
        	writel( SA5_INTR_OFF, 
			h->vaddr + SA5_REPLY_INTR_MASK_OFFSET);
	}
}

static void SA5B_intr_mask(ctlr_info_t *h, unsigned long val)
{
        if (val)
        { 
		h->interrupts_enabled = 1;
                writel(0, h->vaddr + SA5_REPLY_INTR_MASK_OFFSET);
        } else 
        {
		h->interrupts_enabled = 0;
                writel( SA5B_INTR_OFF,
                        h->vaddr + SA5_REPLY_INTR_MASK_OFFSET);
        }
}
 
static unsigned long SA5_fifo_full(ctlr_info_t *h)
{
	if( h->commands_outstanding >= h->max_commands)
		return(1);
	else 
		return(0);

}
 
static unsigned long SA5_completed(ctlr_info_t *h)
{
	unsigned long register_value 
		= readl(h->vaddr + SA5_REPLY_PORT_OFFSET);
	if(register_value != FIFO_EMPTY)
	{
		h->commands_outstanding--;
#ifdef CCISS_DEBUG
		printk("cciss:  Read %lx back from board\n", register_value);
#endif  
	} 
#ifdef CCISS_DEBUG
	else
	{
		printk("cciss:  FIFO Empty read\n");
	}
#endif 
	return ( register_value); 

}

static unsigned long SA5_intr_pending(ctlr_info_t *h)
{
	unsigned long register_value  = 
		readl(h->vaddr + SA5_INTR_STATUS);
#ifdef CCISS_DEBUG
	printk("cciss: intr_pending %lx\n", register_value);
#endif  
	if( register_value &  SA5_INTR_PENDING) 
		return  1;	
	return 0 ;
}


static unsigned long SA5B_intr_pending(ctlr_info_t *h)
{
        unsigned long register_value  =
                readl(h->vaddr + SA5_INTR_STATUS);
#ifdef CCISS_DEBUG
        printk("cciss: intr_pending %lx\n", register_value);
#endif  
        if( register_value &  SA5B_INTR_PENDING)
                return  1;
        return 0 ;
}


static struct access_method SA5_access = {
	SA5_submit_command,
	SA5_intr_mask,
	SA5_fifo_full,
	SA5_intr_pending,
	SA5_completed,
};

static struct access_method SA5B_access = {
        SA5_submit_command,
        SA5B_intr_mask,
        SA5_fifo_full,
        SA5B_intr_pending,
        SA5_completed,
};

struct board_type {
	__u32	board_id;
	char	*product_name;
	struct access_method *access;
	int nr_cmds; 
};

#define CCISS_LOCK(i)	(&hba[i]->lock)

#endif 

