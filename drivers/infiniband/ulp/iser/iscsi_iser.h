
#ifndef __ISCSI_ISER_H__
#define __ISCSI_ISER_H__

#include <linux/types.h>
#include <linux/net.h>
#include <scsi/libiscsi.h>
#include <scsi/scsi_transport_iscsi.h>

#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>
#include <linux/mempool.h>
#include <linux/uio.h>

#include <linux/socket.h>
#include <linux/in.h>
#include <linux/in6.h>

#include <rdma/ib_verbs.h>
#include <rdma/ib_fmr_pool.h>
#include <rdma/rdma_cm.h>

#define DRV_NAME	"iser"
#define PFX		DRV_NAME ": "
#define DRV_VER		"0.1"
#define DRV_DATE	"May 7th, 2006"

#define iser_dbg(fmt, arg...)				\
	do {						\
		if (iser_debug_level > 1)		\
			printk(KERN_DEBUG PFX "%s:" fmt,\
				__func__ , ## arg);	\
	} while (0)

#define iser_warn(fmt, arg...)				\
	do {						\
		if (iser_debug_level > 0)		\
			printk(KERN_DEBUG PFX "%s:" fmt,\
				__func__ , ## arg);	\
	} while (0)

#define iser_err(fmt, arg...)				\
	do {						\
		printk(KERN_ERR PFX "%s:" fmt,          \
		       __func__ , ## arg);		\
	} while (0)

#define SHIFT_4K	12
#define SIZE_4K	(1UL << SHIFT_4K)
#define MASK_4K	(~(SIZE_4K-1))

					
#define ISCSI_ISER_SG_TABLESIZE         (0x80000 >> SHIFT_4K)
#define ISER_DEF_CMD_PER_LUN		128



#define ISER_MAX_RX_MISC_PDUS		4 

#define ISER_MAX_TX_MISC_PDUS		6 

#define ISER_QP_MAX_RECV_DTOS		(ISCSI_DEF_XMIT_CMDS_MAX + \
					ISER_MAX_RX_MISC_PDUS    +  \
					ISER_MAX_TX_MISC_PDUS)



#define ISER_INFLIGHT_DATAOUTS		8

#define ISER_QP_MAX_REQ_DTOS		(ISCSI_DEF_XMIT_CMDS_MAX *    \
					(1 + ISER_INFLIGHT_DATAOUTS) + \
					ISER_MAX_TX_MISC_PDUS        + \
					ISER_MAX_RX_MISC_PDUS)

#define ISER_VER			0x10
#define ISER_WSV			0x08
#define ISER_RSV			0x04

struct iser_hdr {
	u8      flags;
	u8      rsvd[3];
	__be32  write_stag; 
	__be64  write_va;
	__be32  read_stag;  
	__be64  read_va;
} __attribute__((packed));



#define ISER_OBJECT_NAME_SIZE		    64

enum iser_ib_conn_state {
	ISER_CONN_INIT,		   
	ISER_CONN_PENDING,	   
	ISER_CONN_UP,		   
	ISER_CONN_TERMINATING,	   
	ISER_CONN_DOWN,		   
	ISER_CONN_STATES_NUM
};

enum iser_task_status {
	ISER_TASK_STATUS_INIT = 0,
	ISER_TASK_STATUS_STARTED,
	ISER_TASK_STATUS_COMPLETED
};

enum iser_data_dir {
	ISER_DIR_IN = 0,	   
	ISER_DIR_OUT,		   
	ISER_DIRS_NUM
};

struct iser_data_buf {
	void               *buf;      
	unsigned int       size;      
	unsigned long      data_len;  
	unsigned int       dma_nents; 
	char       	   *copy_buf; 
	struct scatterlist sg_single; 
  };


struct iser_device;
struct iscsi_iser_conn;
struct iscsi_iser_task;
struct iscsi_endpoint;

struct iser_mem_reg {
	u32  lkey;
	u32  rkey;
	u64  va;
	u64  len;
	void *mem_h;
	int  is_fmr;
};

struct iser_regd_buf {
	struct iser_mem_reg     reg;        
	void                    *virt_addr;
	struct iser_device      *device;    
	u64                     dma_addr;   
	enum dma_data_direction direction;  
	unsigned int            data_size;
	atomic_t                ref_count;  
};

#define MAX_REGD_BUF_VECTOR_LEN	2

struct iser_dto {
	struct iscsi_iser_task *task;
	struct iser_conn *ib_conn;
	int                        notify_enable;

	
	unsigned int               regd_vector_len;
	struct iser_regd_buf       *regd[MAX_REGD_BUF_VECTOR_LEN];

	
	unsigned int               offset[MAX_REGD_BUF_VECTOR_LEN];

	
	unsigned int               used_sz[MAX_REGD_BUF_VECTOR_LEN];
};

enum iser_desc_type {
	ISCSI_RX,
	ISCSI_TX_CONTROL ,
	ISCSI_TX_SCSI_COMMAND,
	ISCSI_TX_DATAOUT
};

struct iser_desc {
	struct iser_hdr              iser_header;
	struct iscsi_hdr             iscsi_header;
	struct iser_regd_buf         hdr_regd_buf;
	void                         *data;         
	struct iser_regd_buf         data_regd_buf; 
	enum   iser_desc_type        type;
	struct iser_dto              dto;
};

struct iser_device {
	struct ib_device             *ib_device;
	struct ib_pd	             *pd;
	struct ib_cq	             *cq;
	struct ib_mr	             *mr;
	struct tasklet_struct	     cq_tasklet;
	struct list_head             ig_list; 
	int                          refcount;
};

struct iser_conn {
	struct iscsi_iser_conn       *iser_conn; 
	struct iscsi_endpoint	     *ep;
	enum iser_ib_conn_state	     state;	    
	atomic_t		     refcount;
	spinlock_t		     lock;	    
	struct iser_device           *device;       
	struct rdma_cm_id            *cma_id;       
	struct ib_qp	             *qp;           
	struct ib_fmr_pool           *fmr_pool;     
	int                          disc_evt_flag; 
	wait_queue_head_t	     wait;          
	atomic_t                     post_recv_buf_count; 
	atomic_t                     post_send_buf_count; 
	atomic_t                     unexpected_pdu_count;
	char 			     name[ISER_OBJECT_NAME_SIZE];
	struct iser_page_vec         *page_vec;     
	struct list_head	     conn_list;       
};

struct iscsi_iser_conn {
	struct iscsi_conn            *iscsi_conn;
	struct iser_conn             *ib_conn;   
};

struct iscsi_iser_task {
	struct iser_desc             desc;
	struct iscsi_iser_conn	     *iser_conn;
	enum iser_task_status 	     status;
	int                          command_sent;  
	int                          dir[ISER_DIRS_NUM];      
	struct iser_regd_buf         rdma_regd[ISER_DIRS_NUM];
	struct iser_data_buf         data[ISER_DIRS_NUM];     
	struct iser_data_buf         data_copy[ISER_DIRS_NUM];
};

struct iser_page_vec {
	u64 *pages;
	int length;
	int offset;
	int data_size;
};

struct iser_global {
	struct mutex      device_list_mutex;
	struct list_head  device_list;	     
	struct mutex      connlist_mutex;
	struct list_head  connlist;		

	struct kmem_cache *desc_cache;
};

extern struct iser_global ig;
extern int iser_debug_level;


int iser_conn_set_full_featured_mode(struct iscsi_conn *conn);

int iser_send_control(struct iscsi_conn *conn,
		      struct iscsi_task *task);

int iser_send_command(struct iscsi_conn *conn,
		      struct iscsi_task *task);

int iser_send_data_out(struct iscsi_conn *conn,
		       struct iscsi_task *task,
		       struct iscsi_data *hdr);

void iscsi_iser_recv(struct iscsi_conn *conn,
		     struct iscsi_hdr       *hdr,
		     char                   *rx_data,
		     int                    rx_data_len);

void iser_conn_init(struct iser_conn *ib_conn);

void iser_conn_get(struct iser_conn *ib_conn);

void iser_conn_put(struct iser_conn *ib_conn);

void iser_conn_terminate(struct iser_conn *ib_conn);

void iser_rcv_completion(struct iser_desc *desc,
			 unsigned long    dto_xfer_len);

void iser_snd_completion(struct iser_desc *desc);

void iser_task_rdma_init(struct iscsi_iser_task *task);

void iser_task_rdma_finalize(struct iscsi_iser_task *task);

void iser_dto_buffs_release(struct iser_dto *dto);

int  iser_regd_buff_release(struct iser_regd_buf *regd_buf);

void iser_reg_single(struct iser_device      *device,
		     struct iser_regd_buf    *regd_buf,
		     enum dma_data_direction direction);

void iser_finalize_rdma_unaligned_sg(struct iscsi_iser_task *task,
				     enum iser_data_dir         cmd_dir);

int  iser_reg_rdma_mem(struct iscsi_iser_task *task,
		       enum   iser_data_dir        cmd_dir);

int  iser_connect(struct iser_conn   *ib_conn,
		  struct sockaddr_in *src_addr,
		  struct sockaddr_in *dst_addr,
		  int                non_blocking);

int  iser_reg_page_vec(struct iser_conn     *ib_conn,
		       struct iser_page_vec *page_vec,
		       struct iser_mem_reg  *mem_reg);

void iser_unreg_mem(struct iser_mem_reg *mem_reg);

int  iser_post_recv(struct iser_desc *rx_desc);
int  iser_post_send(struct iser_desc *tx_desc);

int iser_conn_state_comp(struct iser_conn *ib_conn,
			 enum iser_ib_conn_state comp);

int iser_dma_map_task_data(struct iscsi_iser_task *iser_task,
			    struct iser_data_buf       *data,
			    enum   iser_data_dir       iser_dir,
			    enum   dma_data_direction  dma_dir);

void iser_dma_unmap_task_data(struct iscsi_iser_task *iser_task);
#endif
