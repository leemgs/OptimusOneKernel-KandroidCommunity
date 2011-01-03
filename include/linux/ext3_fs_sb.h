

#ifndef _LINUX_EXT3_FS_SB
#define _LINUX_EXT3_FS_SB

#ifdef __KERNEL__
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/blockgroup_lock.h>
#include <linux/percpu_counter.h>
#endif
#include <linux/rbtree.h>


struct ext3_sb_info {
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
	struct ext3_super_block * s_es;	
	struct buffer_head ** s_group_desc;
	unsigned long  s_mount_opt;
	ext3_fsblk_t s_sb_block;
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
	u32 s_hash_seed[4];
	int s_def_hash_version;
	int s_hash_unsigned;	
	struct percpu_counter s_freeblocks_counter;
	struct percpu_counter s_freeinodes_counter;
	struct percpu_counter s_dirs_counter;
	struct blockgroup_lock *s_blockgroup_lock;

	
	spinlock_t s_rsv_window_lock;
	struct rb_root s_rsv_window_root;
	struct ext3_reserve_window_node s_rsv_window_head;

	
	struct inode * s_journal_inode;
	struct journal_s * s_journal;
	struct list_head s_orphan;
	unsigned long s_commit_interval;
	struct block_device *journal_bdev;
#ifdef CONFIG_JBD_DEBUG
	struct timer_list turn_ro_timer;	
	wait_queue_head_t ro_wait_queue;	
#endif
#ifdef CONFIG_QUOTA
	char *s_qf_names[MAXQUOTAS];		
	int s_jquota_fmt;			
#endif
};

static inline spinlock_t *
sb_bgl_lock(struct ext3_sb_info *sbi, unsigned int block_group)
{
	return bgl_lock_ptr(sbi->s_blockgroup_lock, block_group);
}

#endif	
