

#ifndef _SMB_FS_SB
#define _SMB_FS_SB

#include <linux/types.h>
#include <linux/smb.h>


#define MAX_REQUEST_HARD       256

enum smb_receive_state {
	SMB_RECV_START,		
	SMB_RECV_HEADER,	
	SMB_RECV_HCOMPLETE,	
	SMB_RECV_PARAM,		
	SMB_RECV_DATA,		
	SMB_RECV_END,		
	SMB_RECV_DROP,		
	SMB_RECV_REQUEST,	
};


#define server_from_inode(inode) SMB_SB((inode)->i_sb)
#define server_from_dentry(dentry) SMB_SB((dentry)->d_sb)
#define SB_of(server) ((server)->super_block)

struct smb_sb_info {
	
	struct list_head entry;

        enum smb_conn_state state;
	struct file * sock_file;
	int conn_error;
	enum smb_receive_state rstate;

	atomic_t nr_requests;
	struct list_head xmitq;
	struct list_head recvq;
	u16 mid;

        struct smb_mount_data_kernel *mnt;

	
	unsigned int generation;
	struct pid *conn_pid;
	struct smb_conn_opt opt;
	wait_queue_head_t conn_wq;
	int conn_complete;
	struct semaphore sem;

	unsigned char      header[SMB_HEADER_LEN + 20*2 + 2];
	u32                header_len;
	u32                smb_len;
	u32                smb_read;

        
        void *data_ready;

	
	struct nls_table *remote_nls;
	struct nls_table *local_nls;

	struct smb_ops *ops;

	struct super_block *super_block;
};

static inline int
smb_lock_server_interruptible(struct smb_sb_info *server)
{
	return down_interruptible(&(server->sem));
}

static inline void
smb_lock_server(struct smb_sb_info *server)
{
	down(&(server->sem));
}

static inline void
smb_unlock_server(struct smb_sb_info *server)
{
	up(&(server->sem));
}

#endif
