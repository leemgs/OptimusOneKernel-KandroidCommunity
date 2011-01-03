#ifndef __SHMEM_FS_H
#define __SHMEM_FS_H

#include <linux/swap.h>
#include <linux/mempolicy.h>



#define SHMEM_NR_DIRECT 16

struct shmem_inode_info {
	spinlock_t		lock;
	unsigned long		flags;
	unsigned long		alloced;	
	unsigned long		swapped;	
	unsigned long		next_index;	
	struct shared_policy	policy;		
	struct page		*i_indirect;	
	swp_entry_t		i_direct[SHMEM_NR_DIRECT]; 
	struct list_head	swaplist;	
	struct inode		vfs_inode;
};

struct shmem_sb_info {
	unsigned long max_blocks;   
	unsigned long free_blocks;  
	unsigned long max_inodes;   
	unsigned long free_inodes;  
	spinlock_t stat_lock;	    
	uid_t uid;		    
	gid_t gid;		    
	mode_t mode;		    
	struct mempolicy *mpol;     
};

static inline struct shmem_inode_info *SHMEM_I(struct inode *inode)
{
	return container_of(inode, struct shmem_inode_info, vfs_inode);
}

extern int init_tmpfs(void);
extern int shmem_fill_super(struct super_block *sb, void *data, int silent);

#ifdef CONFIG_TMPFS_POSIX_ACL
int shmem_check_acl(struct inode *, int);
int shmem_acl_init(struct inode *, struct inode *);

extern struct xattr_handler shmem_xattr_acl_access_handler;
extern struct xattr_handler shmem_xattr_acl_default_handler;

extern struct generic_acl_operations shmem_acl_ops;

#else
static inline int shmem_acl_init(struct inode *inode, struct inode *dir)
{
	return 0;
}
#endif  

#endif
