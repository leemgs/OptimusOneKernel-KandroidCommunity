

#ifndef _NFSD4_STATE_H
#define _NFSD4_STATE_H

#include <linux/list.h>
#include <linux/kref.h>
#include <linux/sunrpc/clnt.h>

typedef struct {
	u32             cl_boot;
	u32             cl_id;
} clientid_t;

typedef struct {
	u32             so_boot;
	u32             so_stateownerid;
	u32             so_fileid;
} stateid_opaque_t;

typedef struct {
	u32                     si_generation;
	stateid_opaque_t        si_opaque;
} stateid_t;
#define si_boot           si_opaque.so_boot
#define si_stateownerid   si_opaque.so_stateownerid
#define si_fileid         si_opaque.so_fileid

struct nfsd4_cb_sequence {
	
	u32			cbs_minorversion;
	struct nfs4_client	*cbs_clp;
};

struct nfs4_delegation {
	struct list_head	dl_perfile;
	struct list_head	dl_perclnt;
	struct list_head	dl_recall_lru;  
	atomic_t		dl_count;       
	struct nfs4_client	*dl_client;
	struct nfs4_file	*dl_file;
	struct file_lock	*dl_flock;
	struct file		*dl_vfs_file;
	u32			dl_type;
	time_t			dl_time;

	u32			dl_ident;
	stateid_t		dl_stateid;
	struct knfsd_fh		dl_fh;
	int			dl_retries;
};


struct nfs4_cb_conn {
	
	struct sockaddr_storage	cb_addr;
	size_t			cb_addrlen;
	u32                     cb_prog;
	u32			cb_minorversion;
	u32                     cb_ident;	
	
	atomic_t		cb_set;     
	struct rpc_clnt *       cb_client;
};


#define NFSD_MAX_SLOTS_PER_SESSION     160

#define NFSD_MAX_OPS_PER_COMPOUND	16

#define NFSD_SLOT_CACHE_SIZE		1024

#define NFSD_CACHE_SIZE_SLOTS_PER_SESSION	32
#define NFSD_MAX_MEM_PER_SESSION  \
		(NFSD_CACHE_SIZE_SLOTS_PER_SESSION * NFSD_SLOT_CACHE_SIZE)

struct nfsd4_slot {
	bool	sl_inuse;
	bool	sl_cachethis;
	u16	sl_opcnt;
	u32	sl_seqid;
	__be32	sl_status;
	u32	sl_datalen;
	char	sl_data[];
};

struct nfsd4_channel_attrs {
	u32		headerpadsz;
	u32		maxreq_sz;
	u32		maxresp_sz;
	u32		maxresp_cached;
	u32		maxops;
	u32		maxreqs;
	u32		nr_rdma_attrs;
	u32		rdma_attrs;
};

struct nfsd4_create_session {
	clientid_t			clientid;
	struct nfs4_sessionid		sessionid;
	u32				seqid;
	u32				flags;
	struct nfsd4_channel_attrs	fore_channel;
	struct nfsd4_channel_attrs	back_channel;
	u32				callback_prog;
	u32				uid;
	u32				gid;
};


struct nfsd4_clid_slot {
	u32				sl_seqid;
	__be32				sl_status;
	struct nfsd4_create_session	sl_cr_ses;
};

struct nfsd4_session {
	struct kref		se_ref;
	struct list_head	se_hash;	
	struct list_head	se_perclnt;
	u32			se_flags;
	struct nfs4_client	*se_client;	
	struct nfs4_sessionid	se_sessionid;
	struct nfsd4_channel_attrs se_fchannel;
	struct nfsd4_channel_attrs se_bchannel;
	struct nfsd4_slot	*se_slots[];	
};

static inline void
nfsd4_put_session(struct nfsd4_session *ses)
{
	extern void free_session(struct kref *kref);
	kref_put(&ses->se_ref, free_session);
}

static inline void
nfsd4_get_session(struct nfsd4_session *ses)
{
	kref_get(&ses->se_ref);
}


struct nfsd4_sessionid {
	clientid_t	clientid;
	u32		sequence;
	u32		reserved;
};

#define HEXDIR_LEN     33 


struct nfs4_client {
	struct list_head	cl_idhash; 	
	struct list_head	cl_strhash; 	
	struct list_head	cl_openowners;
	struct list_head	cl_delegations;
	struct list_head        cl_lru;         
	struct xdr_netobj	cl_name; 	
	char                    cl_recdir[HEXDIR_LEN]; 
	nfs4_verifier		cl_verifier; 	
	time_t                  cl_time;        
	struct sockaddr_storage	cl_addr; 	
	u32			cl_flavor;	
	char			*cl_principal;	
	struct svc_cred		cl_cred; 	
	clientid_t		cl_clientid;	
	nfs4_verifier		cl_confirm;	
	struct nfs4_cb_conn	cl_cb_conn;     
	atomic_t		cl_count;	
	u32			cl_firststate;	

	
	struct list_head	cl_sessions;
	struct nfsd4_clid_slot	cl_cs_slot;	
	u32			cl_exchange_flags;
	struct nfs4_sessionid	cl_sessionid;

	
	
	unsigned long		cl_cb_slot_busy;
	u32			cl_cb_seq_nr;
	struct svc_xprt		*cl_cb_xprt;	
	struct rpc_wait_queue	cl_cb_waitq;	
						
};


struct nfs4_client_reclaim {
	struct list_head	cr_strhash;	
	char			cr_recdir[HEXDIR_LEN]; 
};

static inline void
update_stateid(stateid_t *stateid)
{
	stateid->si_generation++;
}



#define NFSD4_REPLAY_ISIZE       112 


struct nfs4_replay {
	__be32			rp_status;
	unsigned int		rp_buflen;
	char			*rp_buf;
	unsigned		intrp_allocated;
	struct knfsd_fh		rp_openfh;
	char			rp_ibuf[NFSD4_REPLAY_ISIZE];
};


struct nfs4_stateowner {
	struct kref		so_ref;
	struct list_head        so_idhash;   
	struct list_head        so_strhash;   
	struct list_head        so_perclient;
	struct list_head        so_stateids;
	struct list_head        so_perstateid; 
	struct list_head	so_close_lru; 
	time_t			so_time; 
	int			so_is_open_owner; 
	u32                     so_id;
	struct nfs4_client *    so_client;
	
	u32                     so_seqid;
	struct xdr_netobj       so_owner;     
	int                     so_confirmed; 
	struct nfs4_replay	so_replay;
};


struct nfs4_file {
	atomic_t		fi_ref;
	struct list_head        fi_hash;    
	struct list_head        fi_stateids;
	struct list_head	fi_delegations;
	struct inode		*fi_inode;
	u32                     fi_id;      
	bool			fi_had_conflict;
};



struct nfs4_stateid {
	struct list_head              st_hash; 
	struct list_head              st_perfile;
	struct list_head              st_perstateowner;
	struct list_head              st_lockowners;
	struct nfs4_stateowner      * st_stateowner;
	struct nfs4_file            * st_file;
	stateid_t                     st_stateid;
	struct file                 * st_vfs_file;
	unsigned long                 st_access_bmap;
	unsigned long                 st_deny_bmap;
	struct nfs4_stateid         * st_openstp;
};


#define HAS_SESSION             0x00000001
#define CONFIRM                 0x00000002
#define OPEN_STATE              0x00000004
#define LOCK_STATE              0x00000008
#define RD_STATE	        0x00000010
#define WR_STATE	        0x00000020
#define CLOSE_STATE             0x00000040

#define seqid_mutating_err(err)                       \
	(((err) != nfserr_stale_clientid) &&    \
	((err) != nfserr_bad_seqid) &&          \
	((err) != nfserr_stale_stateid) &&      \
	((err) != nfserr_bad_stateid))

struct nfsd4_compound_state;

extern __be32 nfs4_preprocess_stateid_op(struct nfsd4_compound_state *cstate,
		stateid_t *stateid, int flags, struct file **filp);
extern void nfs4_lock_state(void);
extern void nfs4_unlock_state(void);
extern int nfs4_in_grace(void);
extern __be32 nfs4_check_open_reclaim(clientid_t *clid);
extern void put_nfs4_client(struct nfs4_client *clp);
extern void nfs4_free_stateowner(struct kref *kref);
extern int set_callback_cred(void);
extern void nfsd4_probe_callback(struct nfs4_client *clp);
extern void nfsd4_cb_recall(struct nfs4_delegation *dp);
extern void nfs4_put_delegation(struct nfs4_delegation *dp);
extern __be32 nfs4_make_rec_clidname(char *clidname, struct xdr_netobj *clname);
extern void nfsd4_init_recdir(char *recdir_name);
extern int nfsd4_recdir_load(void);
extern void nfsd4_shutdown_recdir(void);
extern int nfs4_client_to_reclaim(const char *name);
extern int nfs4_has_reclaimed_state(const char *name, bool use_exchange_id);
extern void nfsd4_recdir_purge_old(void);
extern int nfsd4_create_clid_dir(struct nfs4_client *clp);
extern void nfsd4_remove_clid_dir(struct nfs4_client *clp);

static inline void
nfs4_put_stateowner(struct nfs4_stateowner *so)
{
	kref_put(&so->so_ref, nfs4_free_stateowner);
}

static inline void
nfs4_get_stateowner(struct nfs4_stateowner *so)
{
	kref_get(&so->so_ref);
}

#endif   
