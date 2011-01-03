#ifndef _REISER_FS_I
#define _REISER_FS_I

#include <linux/list.h>

struct reiserfs_journal_list;


typedef enum {
    
	i_item_key_version_mask = 0x0001,
    
	i_stat_data_version_mask = 0x0002,
    
	i_pack_on_close_mask = 0x0004,
    
	i_nopack_mask = 0x0008,
    
	i_link_saved_unlink_mask = 0x0010,
	i_link_saved_truncate_mask = 0x0020,
	i_has_xattr_dir = 0x0040,
	i_data_log = 0x0080,
	i_ever_mapped = 0x0100
} reiserfs_inode_flags;

struct reiserfs_inode_info {
	__u32 i_key[4];		
    
	__u32 i_flags;

	__u32 i_first_direct_byte;	

	
	__u32 i_attrs;

	int i_prealloc_block;	
	int i_prealloc_count;	
	struct list_head i_prealloc_list;	

	unsigned new_packing_locality:1;	

	
	unsigned int i_trans_id;
	struct reiserfs_journal_list *i_jl;
	struct mutex i_mmap;
#ifdef CONFIG_REISERFS_FS_XATTR
	struct rw_semaphore i_xattr_sem;
#endif
	struct inode vfs_inode;
};

#endif
