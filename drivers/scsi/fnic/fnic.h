
#ifndef _FNIC_H_
#define _FNIC_H_

#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/workqueue.h>
#include <scsi/libfc.h>
#include "fnic_io.h"
#include "fnic_res.h"
#include "vnic_dev.h"
#include "vnic_wq.h"
#include "vnic_rq.h"
#include "vnic_cq.h"
#include "vnic_wq_copy.h"
#include "vnic_intr.h"
#include "vnic_stats.h"
#include "vnic_scsi.h"

#define DRV_NAME		"fnic"
#define DRV_DESCRIPTION		"Cisco FCoE HBA Driver"
#define DRV_VERSION		"1.0.0.1121"
#define PFX			DRV_NAME ": "
#define DFX                     DRV_NAME "%d: "

#define DESC_CLEAN_LOW_WATERMARK 8
#define FNIC_MAX_IO_REQ		2048 
#define	FNIC_IO_LOCKS		64 
#define FNIC_DFLT_QUEUE_DEPTH	32
#define	FNIC_STATS_RATE_LIMIT	4 


#define BIT(nr)			(1UL << (nr))
#define FNIC_TAG_ABORT		BIT(30)		
#define FNIC_TAG_DEV_RST	BIT(29)		
#define FNIC_TAG_MASK		(BIT(24) - 1)	
#define FNIC_NO_TAG             -1


#define CMD_SP(Cmnd)		((Cmnd)->SCp.ptr)
#define CMD_STATE(Cmnd)		((Cmnd)->SCp.phase)
#define CMD_ABTS_STATUS(Cmnd)	((Cmnd)->SCp.Message)
#define CMD_LR_STATUS(Cmnd)	((Cmnd)->SCp.have_data_in)
#define CMD_TAG(Cmnd)           ((Cmnd)->SCp.sent_command)

#define FCPIO_INVALID_CODE 0x100 

#define FNIC_LUN_RESET_TIMEOUT	     10000	
#define FNIC_HOST_RESET_TIMEOUT	     10000	
#define FNIC_RMDEVICE_TIMEOUT        1000       
#define FNIC_HOST_RESET_SETTLE_TIME  30         

#define FNIC_MAX_FCP_TARGET     256

extern unsigned int fnic_log_level;

#define FNIC_MAIN_LOGGING 0x01
#define FNIC_FCS_LOGGING 0x02
#define FNIC_SCSI_LOGGING 0x04
#define FNIC_ISR_LOGGING 0x08

#define FNIC_CHECK_LOGGING(LEVEL, CMD)				\
do {								\
	if (unlikely(fnic_log_level & LEVEL))			\
		do {						\
			CMD;					\
		} while (0);					\
} while (0)

#define FNIC_MAIN_DBG(kern_level, host, fmt, args...)		\
	FNIC_CHECK_LOGGING(FNIC_MAIN_LOGGING,			\
			 shost_printk(kern_level, host, fmt, ##args);)

#define FNIC_FCS_DBG(kern_level, host, fmt, args...)		\
	FNIC_CHECK_LOGGING(FNIC_FCS_LOGGING,			\
			 shost_printk(kern_level, host, fmt, ##args);)

#define FNIC_SCSI_DBG(kern_level, host, fmt, args...)		\
	FNIC_CHECK_LOGGING(FNIC_SCSI_LOGGING,			\
			 shost_printk(kern_level, host, fmt, ##args);)

#define FNIC_ISR_DBG(kern_level, host, fmt, args...)		\
	FNIC_CHECK_LOGGING(FNIC_ISR_LOGGING,			\
			 shost_printk(kern_level, host, fmt, ##args);)

extern const char *fnic_state_str[];

enum fnic_intx_intr_index {
	FNIC_INTX_WQ_RQ_COPYWQ,
	FNIC_INTX_ERR,
	FNIC_INTX_NOTIFY,
	FNIC_INTX_INTR_MAX,
};

enum fnic_msix_intr_index {
	FNIC_MSIX_RQ,
	FNIC_MSIX_WQ,
	FNIC_MSIX_WQ_COPY,
	FNIC_MSIX_ERR_NOTIFY,
	FNIC_MSIX_INTR_MAX,
};

struct fnic_msix_entry {
	int requested;
	char devname[IFNAMSIZ];
	irqreturn_t (*isr)(int, void *);
	void *devid;
};

enum fnic_state {
	FNIC_IN_FC_MODE = 0,
	FNIC_IN_FC_TRANS_ETH_MODE,
	FNIC_IN_ETH_MODE,
	FNIC_IN_ETH_TRANS_FC_MODE,
};

#define FNIC_WQ_COPY_MAX 1
#define FNIC_WQ_MAX 1
#define FNIC_RQ_MAX 1
#define FNIC_CQ_MAX (FNIC_WQ_COPY_MAX + FNIC_WQ_MAX + FNIC_RQ_MAX)

struct mempool;


struct fnic {
	struct fc_lport *lport;
	struct vnic_dev_bar bar0;

	struct msix_entry msix_entry[FNIC_MSIX_INTR_MAX];
	struct fnic_msix_entry msix[FNIC_MSIX_INTR_MAX];

	struct vnic_stats *stats;
	unsigned long stats_time;	
	struct vnic_nic_cfg *nic_cfg;
	char name[IFNAMSIZ];
	struct timer_list notify_timer; 

	unsigned int err_intr_offset;
	unsigned int link_intr_offset;

	unsigned int wq_count;
	unsigned int cq_count;

	u32 fcoui_mode:1;		
	u32 vlan_hw_insert:1;	        
	u32 in_remove:1;                
	u32 stop_rx_link_events:1;      

	struct completion *remove_wait; 

	struct fc_frame *flogi;
	struct fc_frame *flogi_resp;
	u16 flogi_oxid;
	unsigned long s_id;
	enum fnic_state state;
	spinlock_t fnic_lock;

	u16 vlan_id;	                
	u8 mac_addr[ETH_ALEN];
	u8 dest_addr[ETH_ALEN];
	u8 data_src_addr[ETH_ALEN];
	u64 fcp_input_bytes;		
	u64 fcp_output_bytes;		
	u32 link_down_cnt;
	int link_status;

	struct list_head list;
	struct pci_dev *pdev;
	struct vnic_fc_config config;
	struct vnic_dev *vdev;
	unsigned int raw_wq_count;
	unsigned int wq_copy_count;
	unsigned int rq_count;
	int fw_ack_index[FNIC_WQ_COPY_MAX];
	unsigned short fw_ack_recd[FNIC_WQ_COPY_MAX];
	unsigned short wq_copy_desc_low[FNIC_WQ_COPY_MAX];
	unsigned int intr_count;
	u32 __iomem *legacy_pba;
	struct fnic_host_tag *tags;
	mempool_t *io_req_pool;
	mempool_t *io_sgl_pool[FNIC_SGL_NUM_CACHES];
	spinlock_t io_req_lock[FNIC_IO_LOCKS];	

	struct work_struct link_work;
	struct work_struct frame_work;
	struct sk_buff_head frame_queue;

	
	____cacheline_aligned struct vnic_wq_copy wq_copy[FNIC_WQ_COPY_MAX];
	
	____cacheline_aligned struct vnic_cq cq[FNIC_CQ_MAX];

	spinlock_t wq_copy_lock[FNIC_WQ_COPY_MAX];

	
	____cacheline_aligned struct vnic_wq wq[FNIC_WQ_MAX];
	spinlock_t wq_lock[FNIC_WQ_MAX];

	
	____cacheline_aligned struct vnic_rq rq[FNIC_RQ_MAX];

	
	____cacheline_aligned struct vnic_intr intr[FNIC_MSIX_INTR_MAX];
};

extern struct workqueue_struct *fnic_event_queue;
extern struct device_attribute *fnic_attrs[];

void fnic_clear_intr_mode(struct fnic *fnic);
int fnic_set_intr_mode(struct fnic *fnic);
void fnic_free_intr(struct fnic *fnic);
int fnic_request_intr(struct fnic *fnic);

int fnic_send(struct fc_lport *, struct fc_frame *);
void fnic_free_wq_buf(struct vnic_wq *wq, struct vnic_wq_buf *buf);
void fnic_handle_frame(struct work_struct *work);
void fnic_handle_link(struct work_struct *work);
int fnic_rq_cmpl_handler(struct fnic *fnic, int);
int fnic_alloc_rq_frame(struct vnic_rq *rq);
void fnic_free_rq_buf(struct vnic_rq *rq, struct vnic_rq_buf *buf);
int fnic_send_frame(struct fnic *fnic, struct fc_frame *fp);

int fnic_queuecommand(struct scsi_cmnd *, void (*done)(struct scsi_cmnd *));
int fnic_abort_cmd(struct scsi_cmnd *);
int fnic_device_reset(struct scsi_cmnd *);
int fnic_host_reset(struct scsi_cmnd *);
int fnic_reset(struct Scsi_Host *);
void fnic_scsi_cleanup(struct fc_lport *);
void fnic_scsi_abort_io(struct fc_lport *);
void fnic_empty_scsi_cleanup(struct fc_lport *);
void fnic_exch_mgr_reset(struct fc_lport *, u32, u32);
int fnic_wq_copy_cmpl_handler(struct fnic *fnic, int);
int fnic_wq_cmpl_handler(struct fnic *fnic, int);
int fnic_flogi_reg_handler(struct fnic *fnic);
void fnic_wq_copy_cleanup_handler(struct vnic_wq_copy *wq,
				  struct fcpio_host_req *desc);
int fnic_fw_reset_handler(struct fnic *fnic);
void fnic_terminate_rport_io(struct fc_rport *);
const char *fnic_state_to_str(unsigned int state);

void fnic_log_q_error(struct fnic *fnic);
void fnic_handle_link_event(struct fnic *fnic);

#endif 
