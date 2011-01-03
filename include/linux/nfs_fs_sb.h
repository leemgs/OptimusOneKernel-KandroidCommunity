#ifndef _NFS_FS_SB
#define _NFS_FS_SB

#include <linux/list.h>
#include <linux/backing-dev.h>
#include <linux/wait.h>
#include <linux/nfs_xdr.h>
#include <linux/sunrpc/xprt.h>

#include <asm/atomic.h>

struct nfs4_session;
struct nfs_iostats;
struct nlm_host;
struct nfs4_sequence_args;
struct nfs4_sequence_res;
struct nfs_server;


struct nfs_client {
	atomic_t		cl_count;
	int			cl_cons_state;	
#define NFS_CS_READY		0		
#define NFS_CS_INITING		1		
#define NFS_CS_SESSION_INITING	2		
	unsigned long		cl_res_state;	
#define NFS_CS_CALLBACK		1		
#define NFS_CS_IDMAP		2		
#define NFS_CS_RENEWD		3		
	struct sockaddr_storage	cl_addr;	
	size_t			cl_addrlen;
	char *			cl_hostname;	
	struct list_head	cl_share_link;	
	struct list_head	cl_superblocks;	

	struct rpc_clnt *	cl_rpcclient;
	const struct nfs_rpc_ops *rpc_ops;	
	int			cl_proto;	

	u32			cl_minorversion;
	struct rpc_cred		*cl_machine_cred;

#ifdef CONFIG_NFS_V4
	u64			cl_clientid;	
	nfs4_verifier		cl_confirm;
	unsigned long		cl_state;

	struct rb_root		cl_openowner_id;
	struct rb_root		cl_lockowner_id;

	struct list_head	cl_delegations;
	struct rb_root		cl_state_owners;
	spinlock_t		cl_lock;

	unsigned long		cl_lease_time;
	unsigned long		cl_last_renewal;
	struct delayed_work	cl_renewd;

	struct rpc_wait_queue	cl_rpcwaitq;

	
	struct timespec		cl_boot_time;

	
	struct idmap *		cl_idmap;

	
	char			cl_ipaddr[48];
	unsigned char		cl_id_uniquifier;
	int		     (* cl_call_sync)(struct nfs_server *server,
					      struct rpc_message *msg,
					      struct nfs4_sequence_args *args,
					      struct nfs4_sequence_res *res,
					      int cache_reply);
#endif 

#ifdef CONFIG_NFS_V4_1
	
	u64			cl_ex_clid;
	
	u32			cl_seqid;
	
	u32			cl_exchange_flags;
	struct nfs4_session	*cl_session; 	
#endif 

#ifdef CONFIG_NFS_FSCACHE
	struct fscache_cookie	*fscache;	
#endif
};


struct nfs_server {
	struct nfs_client *	nfs_client;	
	struct list_head	client_link;	
	struct list_head	master_link;	
	struct rpc_clnt *	client;		
	struct rpc_clnt *	client_acl;	
	struct nlm_host		*nlm_host;	
	struct nfs_iostats *	io_stats;	
	struct backing_dev_info	backing_dev_info;
	atomic_long_t		writeback;	
	int			flags;		
	unsigned int		caps;		
	unsigned int		rsize;		
	unsigned int		rpages;		
	unsigned int		wsize;		
	unsigned int		wpages;		
	unsigned int		wtmult;		
	unsigned int		dtsize;		
	unsigned short		port;		
	unsigned int		bsize;		
	unsigned int		acregmin;	
	unsigned int		acregmax;
	unsigned int		acdirmin;
	unsigned int		acdirmax;
	unsigned int		namelen;
	unsigned int		options;	
#define NFS_OPTION_FSCACHE	0x00000001	

	struct nfs_fsid		fsid;
	__u64			maxfilesize;	
	unsigned long		mount_time;	
	dev_t			s_dev;		

#ifdef CONFIG_NFS_FSCACHE
	struct nfs_fscache_key	*fscache_key;	
	struct fscache_cookie	*fscache;	
#endif

#ifdef CONFIG_NFS_V4
	u32			attr_bitmask[2];
	u32			cache_consistency_bitmask[2];
						
	u32			acl_bitmask;	
#endif
	void (*destroy)(struct nfs_server *);

	atomic_t active; 

	
	struct sockaddr_storage	mountd_address;
	size_t			mountd_addrlen;
	u32			mountd_version;
	unsigned short		mountd_port;
	unsigned short		mountd_protocol;
};


#define NFS_CAP_READDIRPLUS	(1U << 0)
#define NFS_CAP_HARDLINKS	(1U << 1)
#define NFS_CAP_SYMLINKS	(1U << 2)
#define NFS_CAP_ACLS		(1U << 3)
#define NFS_CAP_ATOMIC_OPEN	(1U << 4)
#define NFS_CAP_CHANGE_ATTR	(1U << 5)
#define NFS_CAP_FILEID		(1U << 6)
#define NFS_CAP_MODE		(1U << 7)
#define NFS_CAP_NLINK		(1U << 8)
#define NFS_CAP_OWNER		(1U << 9)
#define NFS_CAP_OWNER_GROUP	(1U << 10)
#define NFS_CAP_ATIME		(1U << 11)
#define NFS_CAP_CTIME		(1U << 12)
#define NFS_CAP_MTIME		(1U << 13)



#define NFS4_MAX_SLOT_TABLE RPC_MAX_SLOT_TABLE

#if defined(CONFIG_NFS_V4_1)


#define SLOT_TABLE_SZ (NFS4_MAX_SLOT_TABLE/(8*sizeof(long)))
struct nfs4_slot_table {
	struct nfs4_slot *slots;		
	unsigned long   used_slots[SLOT_TABLE_SZ]; 
	spinlock_t	slot_tbl_lock;
	struct rpc_wait_queue	slot_tbl_waitq;	
	int		max_slots;		
	int		highest_used_slotid;	
};

static inline int slot_idx(struct nfs4_slot_table *tbl, struct nfs4_slot *sp)
{
	return sp - tbl->slots;
}


struct nfs4_session {
	struct nfs4_sessionid		sess_id;
	u32				flags;
	unsigned long			session_state;
	u32				hash_alg;
	u32				ssv_len;

	
	struct nfs4_channel_attrs	fc_attrs;
	struct nfs4_slot_table		fc_slot_table;
	struct nfs4_channel_attrs	bc_attrs;
	struct nfs4_slot_table		bc_slot_table;
	struct nfs_client		*clp;
};

#endif 
#endif
