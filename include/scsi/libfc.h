

#ifndef _LIBFC_H_
#define _LIBFC_H_

#include <linux/timer.h>
#include <linux/if.h>
#include <linux/percpu.h>

#include <scsi/scsi_transport.h>
#include <scsi/scsi_transport_fc.h>

#include <scsi/fc/fc_fcp.h>
#include <scsi/fc/fc_ns.h>
#include <scsi/fc/fc_els.h>
#include <scsi/fc/fc_gs.h>

#include <scsi/fc_frame.h>

#define FC_LIBFC_LOGGING 0x01 
#define FC_LPORT_LOGGING 0x02 
#define FC_DISC_LOGGING  0x04 
#define FC_RPORT_LOGGING 0x08 
#define FC_FCP_LOGGING   0x10 
#define FC_EM_LOGGING    0x20 
#define FC_EXCH_LOGGING  0x40 
#define FC_SCSI_LOGGING  0x80 

extern unsigned int fc_debug_logging;

#define FC_CHECK_LOGGING(LEVEL, CMD)				\
do {								\
	if (unlikely(fc_debug_logging & LEVEL))			\
		do {						\
			CMD;					\
		} while (0);					\
} while (0)

#define FC_LIBFC_DBG(fmt, args...)					\
	FC_CHECK_LOGGING(FC_LIBFC_LOGGING,				\
			 printk(KERN_INFO "libfc: " fmt, ##args))

#define FC_LPORT_DBG(lport, fmt, args...)				\
	FC_CHECK_LOGGING(FC_LPORT_LOGGING,				\
			 printk(KERN_INFO "host%u: lport %6x: " fmt,	\
				(lport)->host->host_no,			\
				fc_host_port_id((lport)->host), ##args))

#define FC_DISC_DBG(disc, fmt, args...)					\
	FC_CHECK_LOGGING(FC_DISC_LOGGING,				\
			 printk(KERN_INFO "host%u: disc: " fmt,		\
				(disc)->lport->host->host_no,		\
				##args))

#define FC_RPORT_ID_DBG(lport, port_id, fmt, args...)			\
	FC_CHECK_LOGGING(FC_RPORT_LOGGING,				\
			 printk(KERN_INFO "host%u: rport %6x: " fmt,	\
				(lport)->host->host_no,			\
				(port_id), ##args))

#define FC_RPORT_DBG(rdata, fmt, args...)				\
	FC_RPORT_ID_DBG((rdata)->local_port, (rdata)->ids.port_id, fmt, ##args)

#define FC_FCP_DBG(pkt, fmt, args...)					\
	FC_CHECK_LOGGING(FC_FCP_LOGGING,				\
			 printk(KERN_INFO "host%u: fcp: %6x: " fmt,	\
				(pkt)->lp->host->host_no,		\
				pkt->rport->port_id, ##args))

#define FC_EXCH_DBG(exch, fmt, args...)					\
	FC_CHECK_LOGGING(FC_EXCH_LOGGING,				\
			 printk(KERN_INFO "host%u: xid %4x: " fmt,	\
				(exch)->lp->host->host_no,		\
				exch->xid, ##args))

#define FC_SCSI_DBG(lport, fmt, args...)				\
	FC_CHECK_LOGGING(FC_SCSI_LOGGING,                               \
			 printk(KERN_INFO "host%u: scsi: " fmt,		\
				(lport)->host->host_no,	##args))


#define	FC_NO_ERR	0	
#define	FC_EX_TIMEOUT	1	
#define	FC_EX_CLOSED	2	



#define ntohll(x) be64_to_cpu(x)
#define htonll(x) cpu_to_be64(x)

#define ntoh24(p) (((p)[0] << 16) | ((p)[1] << 8) | ((p)[2]))

#define hton24(p, v)	do {			\
		p[0] = (((v) >> 16) & 0xFF);	\
		p[1] = (((v) >> 8) & 0xFF);	\
		p[2] = ((v) & 0xFF);		\
	} while (0)


enum fc_lport_state {
	LPORT_ST_DISABLED = 0,
	LPORT_ST_FLOGI,
	LPORT_ST_DNS,
	LPORT_ST_RPN_ID,
	LPORT_ST_RFT_ID,
	LPORT_ST_SCR,
	LPORT_ST_READY,
	LPORT_ST_LOGO,
	LPORT_ST_RESET
};

enum fc_disc_event {
	DISC_EV_NONE = 0,
	DISC_EV_SUCCESS,
	DISC_EV_FAILED
};

enum fc_rport_state {
	RPORT_ST_INIT,		
	RPORT_ST_PLOGI,		
	RPORT_ST_PRLI,		
	RPORT_ST_RTV,		
	RPORT_ST_READY,		
	RPORT_ST_LOGO,		
	RPORT_ST_ADISC,		
	RPORT_ST_DELETE,	
	RPORT_ST_RESTART,       
};


struct fc_disc_port {
	struct fc_lport             *lp;
	struct list_head            peers;
	struct work_struct	    rport_work;
	u32                         port_id;
};

enum fc_rport_event {
	RPORT_EV_NONE = 0,
	RPORT_EV_READY,
	RPORT_EV_FAILED,
	RPORT_EV_STOP,
	RPORT_EV_LOGO
};

struct fc_rport_priv;

struct fc_rport_operations {
	void (*event_callback)(struct fc_lport *, struct fc_rport_priv *,
			       enum fc_rport_event);
};


struct fc_rport_libfc_priv {
	struct fc_lport		   *local_port;
	enum fc_rport_state        rp_state;
	u16			   flags;
	#define FC_RP_FLAGS_REC_SUPPORTED	(1 << 0)
	#define FC_RP_FLAGS_RETRY		(1 << 1)
	unsigned int	           e_d_tov;
	unsigned int	           r_a_tov;
};


struct fc_rport_priv {
	struct fc_lport		   *local_port;
	struct fc_rport		   *rport;
	struct kref		   kref;
	enum fc_rport_state        rp_state;
	struct fc_rport_identifiers ids;
	u16			   flags;
	u16		           max_seq;
	u16			   disc_id;
	u16			   maxframe_size;
	unsigned int	           retries;
	unsigned int	           e_d_tov;
	unsigned int	           r_a_tov;
	struct mutex               rp_mutex;
	struct delayed_work	   retry_work;
	enum fc_rport_event        event;
	struct fc_rport_operations *ops;
	struct list_head           peers;
	struct work_struct         event_work;
	u32			   supported_classes;
};


struct fcoe_dev_stats {
	u64		SecondsSinceLastReset;
	u64		TxFrames;
	u64		TxWords;
	u64		RxFrames;
	u64		RxWords;
	u64		ErrorFrames;
	u64		DumpedFrames;
	u64		LinkFailureCount;
	u64		LossOfSignalCount;
	u64		InvalidTxWordCount;
	u64		InvalidCRCCount;
	u64		InputRequests;
	u64		OutputRequests;
	u64		ControlRequests;
	u64		InputMegabytes;
	u64		OutputMegabytes;
};


struct fc_seq_els_data {
	struct fc_frame *fp;
	enum fc_els_rjt_reason reason;
	enum fc_els_rjt_explan explan;
};


struct fc_fcp_pkt {
	
	struct fc_lport *lp;	
	u16		state;		
	u16		tgt_flags;	
	atomic_t	ref_cnt;	
	spinlock_t	scsi_pkt_lock;	
	
	struct scsi_cmnd *cmd;		
	struct list_head list;		
	
	struct timer_list timer;	
	struct completion tm_done;
	int	wait_for_comp;
	unsigned long	start_time;	
	unsigned long	end_time;	
	unsigned long	last_pkt_time;	 

	
	u32		data_len;
	
	struct fcp_cmnd cdb_cmd;
	size_t		xfer_len;
	u16		xfer_ddp;	
	u32		xfer_contig_end; 
	u16		max_payload;	

	
	u32		io_status;	
	u8		cdb_status;
	u8		status_code;	
	
	u8		scsi_comp_flags;
	u32		req_flags;	
	u32		scsi_resid;	

	struct fc_rport	*rport;		
	struct fc_seq	*seq_ptr;	
	
	u8		recov_retry;	
	struct fc_seq	*recov_seq;	
};

static inline bool fc_fcp_is_read(const struct fc_fcp_pkt *fsp)
{
	if (fsp && fsp->cmd)
		return fsp->cmd->sc_data_direction == DMA_FROM_DEVICE;
	return false;
}



struct fc_exch_mgr;
struct fc_exch_mgr_anchor;
extern u16	fc_cpu_mask;	


struct fc_seq {
	u8	id;		
	u16	ssb_stat;	
	u16	cnt;		
	u32	rec_data;	
};

#define FC_EX_DONE		(1 << 0) 
#define FC_EX_RST_CLEANUP	(1 << 1) 


struct fc_exch {
	struct fc_exch_mgr *em;		
	struct fc_exch_pool *pool;	
	u32		state;		
	u16		xid;		
	struct list_head	ex_list;	
	spinlock_t	ex_lock;	
	atomic_t	ex_refcnt;	
	struct delayed_work timeout_work; 
	struct fc_lport	*lp;		
	u16		oxid;		
	u16		rxid;		
	u32		oid;		
	u32		sid;		
	u32		did;		
	u32		esb_stat;	
	u32		r_a_tov;	
	u8		seq_id;		
	u32		f_ctl;		
	u8		fh_type;	
	enum fc_class	class;		
	struct fc_seq	seq;		
	
	void		(*resp)(struct fc_seq *, struct fc_frame *, void *);
	void		(*destructor)(struct fc_seq *, void *);
	
	void		*arg;
};
#define	fc_seq_exch(sp) container_of(sp, struct fc_exch, seq)

struct libfc_function_template {

	
	int (*frame_send)(struct fc_lport *lp, struct fc_frame *fp);

	
	struct fc_seq *(*elsct_send)(struct fc_lport *lport,
				     u32 did,
				     struct fc_frame *fp,
				     unsigned int op,
				     void (*resp)(struct fc_seq *,
					     struct fc_frame *fp,
					     void *arg),
				     void *arg, u32 timer_msec);

	
	struct fc_seq *(*exch_seq_send)(struct fc_lport *lp,
					struct fc_frame *fp,
					void (*resp)(struct fc_seq *sp,
						     struct fc_frame *fp,
						     void *arg),
					void (*destructor)(struct fc_seq *sp,
							   void *arg),
					void *arg, unsigned int timer_msec);

	
	int (*ddp_setup)(struct fc_lport *lp, u16 xid,
			 struct scatterlist *sgl, unsigned int sgc);
	
	int (*ddp_done)(struct fc_lport *lp, u16 xid);
	
	int (*seq_send)(struct fc_lport *lp, struct fc_seq *sp,
			struct fc_frame *fp);

	
	void (*seq_els_rsp_send)(struct fc_seq *sp, enum fc_els_cmd els_cmd,
				 struct fc_seq_els_data *els_data);

	
	int (*seq_exch_abort)(const struct fc_seq *req_sp,
			      unsigned int timer_msec);

	
	void (*exch_done)(struct fc_seq *sp);

	
	struct fc_seq *(*seq_start_next)(struct fc_seq *sp);

	
	void (*exch_mgr_reset)(struct fc_lport *,
			       u32 s_id, u32 d_id);

	
	void (*rport_flush_queue)(void);

	
	void (*lport_recv)(struct fc_lport *lp, struct fc_seq *sp,
			   struct fc_frame *fp);

	
	int (*lport_reset)(struct fc_lport *);

	
	struct fc_rport_priv *(*rport_create)(struct fc_lport *, u32);

	
	int (*rport_login)(struct fc_rport_priv *);

	
	int (*rport_logoff)(struct fc_rport_priv *);

	
	void (*rport_recv_req)(struct fc_seq *, struct fc_frame *,
			       struct fc_lport *);

	
	struct fc_rport_priv *(*rport_lookup)(const struct fc_lport *, u32);

	
	void (*rport_destroy)(struct kref *);

	
	int (*fcp_cmd_send)(struct fc_lport *lp, struct fc_fcp_pkt *fsp,
			    void (*resp)(struct fc_seq *, struct fc_frame *fp,
					 void *arg));

	
	void (*fcp_cleanup)(struct fc_lport *lp);

	
	void (*fcp_abort_io)(struct fc_lport *lp);

	
	void (*disc_recv_req)(struct fc_seq *,
			      struct fc_frame *, struct fc_lport *);

	
	void (*disc_start)(void (*disc_callback)(struct fc_lport *,
						 enum fc_disc_event),
			   struct fc_lport *);

	
	void (*disc_stop) (struct fc_lport *);

	
	void (*disc_stop_final) (struct fc_lport *);
};


struct fc_disc {
	unsigned char		retry_count;
	unsigned char		pending;
	unsigned char		requested;
	unsigned short		seq_count;
	unsigned char		buf_len;
	u16			disc_id;

	void (*disc_callback)(struct fc_lport *,
			      enum fc_disc_event);

	struct list_head	 rports;
	struct fc_lport		*lport;
	struct mutex		disc_mutex;
	struct fc_gpn_ft_resp	partial_buf;	
	struct delayed_work	disc_work;
};

struct fc_lport {
	struct list_head list;

	
	struct Scsi_Host	*host;
	struct list_head	ema_list;
	struct fc_rport_priv	*dns_rp;
	struct fc_rport_priv	*ptp_rp;
	void			*scsi_priv;
	struct fc_disc          disc;

	
	struct libfc_function_template tt;
	u8			link_up;
	u8			qfull;
	enum fc_lport_state	state;
	unsigned long		boot_time;

	struct fc_host_statistics host_stats;
	struct fcoe_dev_stats	*dev_stats;

	u64			wwpn;
	u64			wwnn;
	u8			retry_count;

	
	u32			sg_supp:1;	
	u32			seq_offload:1;	
	u32			crc_offload:1;	
	u32			lro_enabled:1;	
	u32			mfs;	        
	unsigned int		service_params;
	unsigned int		e_d_tov;
	unsigned int		r_a_tov;
	u8			max_retry_count;
	u8			max_rport_retry_count;
	u16			link_speed;
	u16			link_supported_speeds;
	u16			lro_xid;	
	unsigned int		lso_max;	
	struct fc_ns_fts	fcts;	        
	struct fc_els_rnid_gen	rnid_gen;	

	
	struct mutex lp_mutex;

	
	struct delayed_work	retry_work;
	struct delayed_work	disc_work;
};


static inline int fc_lport_test_ready(struct fc_lport *lp)
{
	return lp->state == LPORT_ST_READY;
}

static inline void fc_set_wwnn(struct fc_lport *lp, u64 wwnn)
{
	lp->wwnn = wwnn;
}

static inline void fc_set_wwpn(struct fc_lport *lp, u64 wwnn)
{
	lp->wwpn = wwnn;
}

static inline void fc_lport_state_enter(struct fc_lport *lp,
					enum fc_lport_state state)
{
	if (state != lp->state)
		lp->retry_count = 0;
	lp->state = state;
}

static inline int fc_lport_init_stats(struct fc_lport *lp)
{
	
	lp->dev_stats = alloc_percpu(struct fcoe_dev_stats);
	if (!lp->dev_stats)
		return -ENOMEM;
	return 0;
}

static inline void fc_lport_free_stats(struct fc_lport *lp)
{
	free_percpu(lp->dev_stats);
}

static inline struct fcoe_dev_stats *fc_lport_get_stats(struct fc_lport *lp)
{
	return per_cpu_ptr(lp->dev_stats, smp_processor_id());
}

static inline void *lport_priv(const struct fc_lport *lp)
{
	return (void *)(lp + 1);
}


static inline struct Scsi_Host *
libfc_host_alloc(struct scsi_host_template *sht, int priv_size)
{
	return scsi_host_alloc(sht, sizeof(struct fc_lport) + priv_size);
}


int fc_lport_init(struct fc_lport *lp);


int fc_lport_destroy(struct fc_lport *lp);


int fc_fabric_logoff(struct fc_lport *lp);


int fc_fabric_login(struct fc_lport *lp);


void fc_linkup(struct fc_lport *);


void fc_linkdown(struct fc_lport *);


int fc_lport_config(struct fc_lport *);


int fc_lport_reset(struct fc_lport *);


int fc_set_mfs(struct fc_lport *lp, u32 mfs);



int fc_rport_init(struct fc_lport *lp);
void fc_rport_terminate_io(struct fc_rport *rp);


int fc_disc_init(struct fc_lport *lp);




int fc_fcp_init(struct fc_lport *);


int fc_queuecommand(struct scsi_cmnd *sc_cmd,
		    void (*done)(struct scsi_cmnd *));


void fc_fcp_complete(struct fc_fcp_pkt *fsp);


int fc_eh_abort(struct scsi_cmnd *sc_cmd);


int fc_eh_device_reset(struct scsi_cmnd *sc_cmd);


int fc_eh_host_reset(struct scsi_cmnd *sc_cmd);


int fc_slave_alloc(struct scsi_device *sdev);


int fc_change_queue_depth(struct scsi_device *sdev, int qdepth);


int fc_change_queue_type(struct scsi_device *sdev, int tag_type);


void fc_fcp_destroy(struct fc_lport *);


void fc_fcp_ddp_setup(struct fc_fcp_pkt *fsp, u16 xid);



int fc_elsct_init(struct fc_lport *lp);




int fc_exch_init(struct fc_lport *lp);


struct fc_exch_mgr_anchor *fc_exch_mgr_add(struct fc_lport *lport,
					   struct fc_exch_mgr *mp,
					   bool (*match)(struct fc_frame *));


void fc_exch_mgr_del(struct fc_exch_mgr_anchor *ema);


struct fc_exch_mgr *fc_exch_mgr_alloc(struct fc_lport *lp,
				      enum fc_class class,
				      u16 min_xid,
				      u16 max_xid,
				      bool (*match)(struct fc_frame *));


void fc_exch_mgr_free(struct fc_lport *lport);


void fc_exch_recv(struct fc_lport *lp, struct fc_frame *fp);


struct fc_seq *fc_exch_seq_send(struct fc_lport *lp,
				struct fc_frame *fp,
				void (*resp)(struct fc_seq *sp,
					     struct fc_frame *fp,
					     void *arg),
				void (*destructor)(struct fc_seq *sp,
						   void *arg),
				void *arg, u32 timer_msec);


int fc_seq_send(struct fc_lport *lp, struct fc_seq *sp, struct fc_frame *fp);


void fc_seq_els_rsp_send(struct fc_seq *sp, enum fc_els_cmd els_cmd,
			 struct fc_seq_els_data *els_data);


int fc_seq_exch_abort(const struct fc_seq *req_sp, unsigned int timer_msec);


void fc_exch_done(struct fc_seq *sp);


struct fc_exch *fc_exch_alloc(struct fc_lport *lport, struct fc_frame *fp);

struct fc_seq *fc_seq_start_next(struct fc_seq *sp);



void fc_exch_mgr_reset(struct fc_lport *, u32 s_id, u32 d_id);


void fc_get_host_speed(struct Scsi_Host *shost);
void fc_get_host_port_type(struct Scsi_Host *shost);
void fc_get_host_port_state(struct Scsi_Host *shost);
void fc_set_rport_loss_tmo(struct fc_rport *rport, u32 timeout);
struct fc_host_statistics *fc_get_host_stats(struct Scsi_Host *);


int fc_setup_exch_mgr(void);
void fc_destroy_exch_mgr(void);
int fc_setup_rport(void);
void fc_destroy_rport(void);


const char *fc_els_resp_type(struct fc_frame *);

#endif 
