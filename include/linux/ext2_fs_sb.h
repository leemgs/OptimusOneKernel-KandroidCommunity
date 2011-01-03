

#ifndef _LINUX_EXT2_FS_SB
#define _LINUX_EXT2_FS_SB

#include <linux/blockgroup_lock.h>
#include <linux/percpu_counter.h>
#include <linux/rbtree.h>




typedef int ext2_grpblk_t;


typedef unsigned long ext2_fsblk_t;

#define E2FSBLK "%lu"

struct ext2_reserve_window {
	ext2_fsblk_t		_rsv_start;	
	ext2_fsblk_t		_rsv_end;	
};

struct ext2_reserve_window_node {
	struct rb_node	 	rsv_node;
	__u32			rsv_goal_size;
	__u32			rsv_alloc_hit;
	struct ext2_reserve_window	rsv_window;
};

struct ext2_block_alloc_info {
	
	struct ext2_reserve_window_node	rsv_window_node;
	
	__u32			last_alloc_logical_block;
	
	ext2_fsblk_t		last_alloc_physical_block;
};

#define rsv_start rsv_window._rsv_start
#define rsv_end rsv_window._rsv_end


struct ext2_sb_info {
	unsigned long s_frag_size;	
	unsigned long s_frags_per_block;
	unsigned long s_inodes_per_block;
	unsigned long s_frags_per_group;
	unsigned long s_blocks_per_group;
	unsigned long s_inodes_per_group;
	unsigned long s_itb_per_group;	
	unsigned long s_gdb_count;	
	unsigned long s_desc_per_block;	
	unsigned long s_groups_count;	
	unsigned long s_overhead_last;  
	unsigned long s_blocks_last;    
	struct buffer_head * s_sbh;	
	struct ext2_super_block * s_es;	
	struct buffer_head ** s_group_desc;
	unsigned long  s_mount_opt;
	unsigned long s_sb_block;
	uid_t s_resuid;
	gid_t s_resgid;
	unsigned short s_mount_state;
	unsigned short s_pad;
	int s_addr_per_block_bits;
	int s_desc_per_block_bits;
	int s_inode_size;
	int s_first_ino;
	spinlock_t s_next_gen_lock;
	u32 s_next_generation;
	unsigned long s_dir_count;
	u8 *s_debts;
	struct percpu_counter s_freeblocks_counter;
	struct percpu_counter s_freeinodes_counter;
	struct percpu_counter s_dirs_counter;
	struct blockgroup_lock *s_blockgroup_lock;
	
	spinlock_t s_rsv_window_lock;
	struct rb_root s_rsv_window_root;
	struct ext2_reserve_window_node s_rsv_window_head;
};

static inline spinlock_t *
sb_bgl_lock(struct ext2_sb_info *sbi, unsigned int block_group)
{
	return bgl_lock_ptr(sbi->s_blockgroup_lock, block_group);
}

#endif	
