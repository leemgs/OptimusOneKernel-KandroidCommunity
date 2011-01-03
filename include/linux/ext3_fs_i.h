

#ifndef _LINUX_EXT3_FS_I
#define _LINUX_EXT3_FS_I

#include <linux/rwsem.h>
#include <linux/rbtree.h>
#include <linux/seqlock.h>
#include <linux/mutex.h>


typedef int ext3_grpblk_t;


typedef unsigned long ext3_fsblk_t;

#define E3FSBLK "%lu"

struct ext3_reserve_window {
	ext3_fsblk_t	_rsv_start;	
	ext3_fsblk_t	_rsv_end;	
};

struct ext3_reserve_window_node {
	struct rb_node		rsv_node;
	__u32			rsv_goal_size;
	__u32			rsv_alloc_hit;
	struct ext3_reserve_window	rsv_window;
};

struct ext3_block_alloc_info {
	
	struct ext3_reserve_window_node	rsv_window_node;
	
	__u32                   last_alloc_logical_block;
	
	ext3_fsblk_t		last_alloc_physical_block;
};

#define rsv_start rsv_window._rsv_start
#define rsv_end rsv_window._rsv_end


struct ext3_inode_info {
	__le32	i_data[15];	
	__u32	i_flags;
#ifdef EXT3_FRAGMENTS
	__u32	i_faddr;
	__u8	i_frag_no;
	__u8	i_frag_size;
#endif
	ext3_fsblk_t	i_file_acl;
	__u32	i_dir_acl;
	__u32	i_dtime;

	
	__u32	i_block_group;
	__u32	i_state;		

	
	struct ext3_block_alloc_info *i_block_alloc_info;

	__u32	i_dir_start_lookup;
#ifdef CONFIG_EXT3_FS_XATTR
	
	struct rw_semaphore xattr_sem;
#endif

	struct list_head i_orphan;	

	
	loff_t	i_disksize;

	
	__u16 i_extra_isize;

	
	struct mutex truncate_mutex;

	
	atomic_t i_sync_tid;
	atomic_t i_datasync_tid;

	struct inode vfs_inode;
};

#endif	
