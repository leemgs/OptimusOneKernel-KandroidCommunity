

#ifndef _LINUX_REISER_FS_SB
#define _LINUX_REISER_FS_SB

#ifdef __KERNEL__
#include <linux/workqueue.h>
#include <linux/rwsem.h>
#endif

typedef enum {
	reiserfs_attrs_cleared = 0x00000001,
} reiserfs_super_block_flags;


#define sb_block_count(sbp)         (le32_to_cpu((sbp)->s_v1.s_block_count))
#define set_sb_block_count(sbp,v)   ((sbp)->s_v1.s_block_count = cpu_to_le32(v))
#define sb_free_blocks(sbp)         (le32_to_cpu((sbp)->s_v1.s_free_blocks))
#define set_sb_free_blocks(sbp,v)   ((sbp)->s_v1.s_free_blocks = cpu_to_le32(v))
#define sb_root_block(sbp)          (le32_to_cpu((sbp)->s_v1.s_root_block))
#define set_sb_root_block(sbp,v)    ((sbp)->s_v1.s_root_block = cpu_to_le32(v))

#define sb_jp_journal_1st_block(sbp)  \
              (le32_to_cpu((sbp)->s_v1.s_journal.jp_journal_1st_block))
#define set_sb_jp_journal_1st_block(sbp,v) \
              ((sbp)->s_v1.s_journal.jp_journal_1st_block = cpu_to_le32(v))
#define sb_jp_journal_dev(sbp) \
              (le32_to_cpu((sbp)->s_v1.s_journal.jp_journal_dev))
#define set_sb_jp_journal_dev(sbp,v) \
              ((sbp)->s_v1.s_journal.jp_journal_dev = cpu_to_le32(v))
#define sb_jp_journal_size(sbp) \
              (le32_to_cpu((sbp)->s_v1.s_journal.jp_journal_size))
#define set_sb_jp_journal_size(sbp,v) \
              ((sbp)->s_v1.s_journal.jp_journal_size = cpu_to_le32(v))
#define sb_jp_journal_trans_max(sbp) \
              (le32_to_cpu((sbp)->s_v1.s_journal.jp_journal_trans_max))
#define set_sb_jp_journal_trans_max(sbp,v) \
              ((sbp)->s_v1.s_journal.jp_journal_trans_max = cpu_to_le32(v))
#define sb_jp_journal_magic(sbp) \
              (le32_to_cpu((sbp)->s_v1.s_journal.jp_journal_magic))
#define set_sb_jp_journal_magic(sbp,v) \
              ((sbp)->s_v1.s_journal.jp_journal_magic = cpu_to_le32(v))
#define sb_jp_journal_max_batch(sbp) \
              (le32_to_cpu((sbp)->s_v1.s_journal.jp_journal_max_batch))
#define set_sb_jp_journal_max_batch(sbp,v) \
              ((sbp)->s_v1.s_journal.jp_journal_max_batch = cpu_to_le32(v))
#define sb_jp_jourmal_max_commit_age(sbp) \
              (le32_to_cpu((sbp)->s_v1.s_journal.jp_journal_max_commit_age))
#define set_sb_jp_journal_max_commit_age(sbp,v) \
              ((sbp)->s_v1.s_journal.jp_journal_max_commit_age = cpu_to_le32(v))

#define sb_blocksize(sbp)          (le16_to_cpu((sbp)->s_v1.s_blocksize))
#define set_sb_blocksize(sbp,v)    ((sbp)->s_v1.s_blocksize = cpu_to_le16(v))
#define sb_oid_maxsize(sbp)        (le16_to_cpu((sbp)->s_v1.s_oid_maxsize))
#define set_sb_oid_maxsize(sbp,v)  ((sbp)->s_v1.s_oid_maxsize = cpu_to_le16(v))
#define sb_oid_cursize(sbp)        (le16_to_cpu((sbp)->s_v1.s_oid_cursize))
#define set_sb_oid_cursize(sbp,v)  ((sbp)->s_v1.s_oid_cursize = cpu_to_le16(v))
#define sb_umount_state(sbp)       (le16_to_cpu((sbp)->s_v1.s_umount_state))
#define set_sb_umount_state(sbp,v) ((sbp)->s_v1.s_umount_state = cpu_to_le16(v))
#define sb_fs_state(sbp)           (le16_to_cpu((sbp)->s_v1.s_fs_state))
#define set_sb_fs_state(sbp,v)     ((sbp)->s_v1.s_fs_state = cpu_to_le16(v))
#define sb_hash_function_code(sbp) \
              (le32_to_cpu((sbp)->s_v1.s_hash_function_code))
#define set_sb_hash_function_code(sbp,v) \
              ((sbp)->s_v1.s_hash_function_code = cpu_to_le32(v))
#define sb_tree_height(sbp)        (le16_to_cpu((sbp)->s_v1.s_tree_height))
#define set_sb_tree_height(sbp,v)  ((sbp)->s_v1.s_tree_height = cpu_to_le16(v))
#define sb_bmap_nr(sbp)            (le16_to_cpu((sbp)->s_v1.s_bmap_nr))
#define set_sb_bmap_nr(sbp,v)      ((sbp)->s_v1.s_bmap_nr = cpu_to_le16(v))
#define sb_version(sbp)            (le16_to_cpu((sbp)->s_v1.s_version))
#define set_sb_version(sbp,v)      ((sbp)->s_v1.s_version = cpu_to_le16(v))

#define sb_mnt_count(sbp)	   (le16_to_cpu((sbp)->s_mnt_count))
#define set_sb_mnt_count(sbp, v)   ((sbp)->s_mnt_count = cpu_to_le16(v))

#define sb_reserved_for_journal(sbp) \
              (le16_to_cpu((sbp)->s_v1.s_reserved_for_journal))
#define set_sb_reserved_for_journal(sbp,v) \
              ((sbp)->s_v1.s_reserved_for_journal = cpu_to_le16(v))






				
#define JOURNAL_BLOCK_SIZE  4096	
#define JOURNAL_MAX_CNODE   1500	
#define JOURNAL_HASH_SIZE 8192
#define JOURNAL_NUM_BITMAPS 5	


struct reiserfs_journal_cnode {
	struct buffer_head *bh;	
	struct super_block *sb;	
	__u32 blocknr;		
	unsigned long state;
	struct reiserfs_journal_list *jlist;	
	struct reiserfs_journal_cnode *next;	
	struct reiserfs_journal_cnode *prev;	
	struct reiserfs_journal_cnode *hprev;	
	struct reiserfs_journal_cnode *hnext;	
};

struct reiserfs_bitmap_node {
	int id;
	char *data;
	struct list_head list;
};

struct reiserfs_list_bitmap {
	struct reiserfs_journal_list *journal_list;
	struct reiserfs_bitmap_node **bitmaps;
};


struct reiserfs_journal_list {
	unsigned long j_start;
	unsigned long j_state;
	unsigned long j_len;
	atomic_t j_nonzerolen;
	atomic_t j_commit_left;
	atomic_t j_older_commits_done;	
	struct mutex j_commit_mutex;
	unsigned int j_trans_id;
	time_t j_timestamp;
	struct reiserfs_list_bitmap *j_list_bitmap;
	struct buffer_head *j_commit_bh;	
	struct reiserfs_journal_cnode *j_realblock;
	struct reiserfs_journal_cnode *j_freedlist;	
	
	struct list_head j_list;

	
	struct list_head j_working_list;

	
	struct list_head j_tail_bh_list;
	
	struct list_head j_bh_list;
	int j_refcount;
};

struct reiserfs_journal {
	struct buffer_head **j_ap_blocks;	
	struct reiserfs_journal_cnode *j_last;	
	struct reiserfs_journal_cnode *j_first;	

	struct block_device *j_dev_bd;
	fmode_t j_dev_mode;
	int j_1st_reserved_block;	

	unsigned long j_state;
	unsigned int j_trans_id;
	unsigned long j_mount_id;
	unsigned long j_start;	
	unsigned long j_len;	
	unsigned long j_len_alloc;	
	atomic_t j_wcount;	
	unsigned long j_bcount;	
	unsigned long j_first_unflushed_offset;	
	unsigned j_last_flush_trans_id;	
	struct buffer_head *j_header_bh;

	time_t j_trans_start_time;	
	struct mutex j_mutex;
	struct mutex j_flush_mutex;
	wait_queue_head_t j_join_wait;	
	atomic_t j_jlock;	
	int j_list_bitmap_index;	
	int j_must_wait;	
	int j_next_full_flush;	
	int j_next_async_flush;	

	int j_cnode_used;	
	int j_cnode_free;	

	unsigned int j_trans_max;	
	unsigned int j_max_batch;	
	unsigned int j_max_commit_age;	
	unsigned int j_max_trans_age;	
	unsigned int j_default_max_commit_age;	

	struct reiserfs_journal_cnode *j_cnode_free_list;
	struct reiserfs_journal_cnode *j_cnode_free_orig;	

	struct reiserfs_journal_list *j_current_jl;
	int j_free_bitmap_nodes;
	int j_used_bitmap_nodes;

	int j_num_lists;	
	int j_num_work_lists;	

	
	unsigned int j_last_flush_id;

	
	unsigned int j_last_commit_id;

	struct list_head j_bitmap_nodes;
	struct list_head j_dirty_buffers;
	spinlock_t j_dirty_buffers_lock;	

	
	struct list_head j_journal_list;
	
	struct list_head j_working_list;

	struct reiserfs_list_bitmap j_list_bitmap[JOURNAL_NUM_BITMAPS];	
	struct reiserfs_journal_cnode *j_hash_table[JOURNAL_HASH_SIZE];	
	struct reiserfs_journal_cnode *j_list_hash_table[JOURNAL_HASH_SIZE];	
	struct list_head j_prealloc_list;	
	int j_persistent_trans;
	unsigned long j_max_trans_size;
	unsigned long j_max_batch_size;

	int j_errno;

	
	struct delayed_work j_work;
	struct super_block *j_work_sb;
	atomic_t j_async_throttle;
};

enum journal_state_bits {
	J_WRITERS_BLOCKED = 1,	
	J_WRITERS_QUEUED,	
	J_ABORTED,		
};

#define JOURNAL_DESC_MAGIC "ReIsErLB"	

typedef __u32(*hashf_t) (const signed char *, int);

struct reiserfs_bitmap_info {
	__u32 free_count;
};

struct proc_dir_entry;

#if defined( CONFIG_PROC_FS ) && defined( CONFIG_REISERFS_PROC_INFO )
typedef unsigned long int stat_cnt_t;
typedef struct reiserfs_proc_info_data {
	spinlock_t lock;
	int exiting;
	int max_hash_collisions;

	stat_cnt_t breads;
	stat_cnt_t bread_miss;
	stat_cnt_t search_by_key;
	stat_cnt_t search_by_key_fs_changed;
	stat_cnt_t search_by_key_restarted;

	stat_cnt_t insert_item_restarted;
	stat_cnt_t paste_into_item_restarted;
	stat_cnt_t cut_from_item_restarted;
	stat_cnt_t delete_solid_item_restarted;
	stat_cnt_t delete_item_restarted;

	stat_cnt_t leaked_oid;
	stat_cnt_t leaves_removable;

	
	stat_cnt_t balance_at[5];	
	
	stat_cnt_t sbk_read_at[5];	
	stat_cnt_t sbk_fs_changed[5];
	stat_cnt_t sbk_restarted[5];
	stat_cnt_t items_at[5];	
	stat_cnt_t free_at[5];	
	stat_cnt_t can_node_be_removed[5];	
	long int lnum[5];	
	long int rnum[5];	
	long int lbytes[5];	
	long int rbytes[5];	
	stat_cnt_t get_neighbors[5];
	stat_cnt_t get_neighbors_restart[5];
	stat_cnt_t need_l_neighbor[5];
	stat_cnt_t need_r_neighbor[5];

	stat_cnt_t free_block;
	struct __scan_bitmap_stats {
		stat_cnt_t call;
		stat_cnt_t wait;
		stat_cnt_t bmap;
		stat_cnt_t retry;
		stat_cnt_t in_journal_hint;
		stat_cnt_t in_journal_nohint;
		stat_cnt_t stolen;
	} scan_bitmap;
	struct __journal_stats {
		stat_cnt_t in_journal;
		stat_cnt_t in_journal_bitmap;
		stat_cnt_t in_journal_reusable;
		stat_cnt_t lock_journal;
		stat_cnt_t lock_journal_wait;
		stat_cnt_t journal_being;
		stat_cnt_t journal_relock_writers;
		stat_cnt_t journal_relock_wcount;
		stat_cnt_t mark_dirty;
		stat_cnt_t mark_dirty_already;
		stat_cnt_t mark_dirty_notjournal;
		stat_cnt_t restore_prepared;
		stat_cnt_t prepare;
		stat_cnt_t prepare_retry;
	} journal;
} reiserfs_proc_info_data_t;
#else
typedef struct reiserfs_proc_info_data {
} reiserfs_proc_info_data_t;
#endif


struct reiserfs_sb_info {
	struct buffer_head *s_sbh;	
	
	struct reiserfs_super_block *s_rs;	
	struct reiserfs_bitmap_info *s_ap_bitmap;
	struct reiserfs_journal *s_journal;	
	unsigned short s_mount_state;	

	
	void (*end_io_handler) (struct buffer_head *, int);
	hashf_t s_hash_function;	
	unsigned long s_mount_opt;	

	struct {		
		unsigned long bits;	
		unsigned long large_file_size;	
		int border;	
		int preallocmin;	
		int preallocsize;	
	} s_alloc_options;

	
	wait_queue_head_t s_wait;
	
	atomic_t s_generation_counter;	
	
	unsigned long s_properties;	

	
	int s_disk_reads;
	int s_disk_writes;
	int s_fix_nodes;
	int s_do_balance;
	int s_unneeded_left_neighbor;
	int s_good_search_by_key_reada;
	int s_bmaps;
	int s_bmaps_without_search;
	int s_direct2indirect;
	int s_indirect2direct;
	
	int s_is_unlinked_ok;
	reiserfs_proc_info_data_t s_proc_info_data;
	struct proc_dir_entry *procdir;
	int reserved_blocks;	
	spinlock_t bitmap_lock;	
	struct dentry *priv_root;	
	struct dentry *xattr_root;	
	int j_errno;
#ifdef CONFIG_QUOTA
	char *s_qf_names[MAXQUOTAS];
	int s_jquota_fmt;
#endif
};


#define REISERFS_3_5 0
#define REISERFS_3_6 1
#define REISERFS_OLD_FORMAT 2

enum reiserfs_mount_options {

	REISERFS_LARGETAIL,	
	REISERFS_SMALLTAIL,	
	REPLAYONLY,		
	REISERFS_CONVERT,	


	FORCE_TEA_HASH,		
	FORCE_RUPASOV_HASH,	
	FORCE_R5_HASH,		
	FORCE_HASH_DETECT,	

	REISERFS_DATA_LOG,
	REISERFS_DATA_ORDERED,
	REISERFS_DATA_WRITEBACK,



	REISERFS_NO_BORDER,
	REISERFS_NO_UNHASHED_RELOCATION,
	REISERFS_HASHED_RELOCATION,
	REISERFS_ATTRS,
	REISERFS_XATTRS_USER,
	REISERFS_POSIXACL,
	REISERFS_EXPOSE_PRIVROOT,
	REISERFS_BARRIER_NONE,
	REISERFS_BARRIER_FLUSH,

	
	REISERFS_ERROR_PANIC,
	REISERFS_ERROR_RO,
	REISERFS_ERROR_CONTINUE,

	REISERFS_QUOTA,		

	REISERFS_TEST1,
	REISERFS_TEST2,
	REISERFS_TEST3,
	REISERFS_TEST4,
	REISERFS_UNSUPPORTED_OPT,
};

#define reiserfs_r5_hash(s) (REISERFS_SB(s)->s_mount_opt & (1 << FORCE_R5_HASH))
#define reiserfs_rupasov_hash(s) (REISERFS_SB(s)->s_mount_opt & (1 << FORCE_RUPASOV_HASH))
#define reiserfs_tea_hash(s) (REISERFS_SB(s)->s_mount_opt & (1 << FORCE_TEA_HASH))
#define reiserfs_hash_detect(s) (REISERFS_SB(s)->s_mount_opt & (1 << FORCE_HASH_DETECT))
#define reiserfs_no_border(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_NO_BORDER))
#define reiserfs_no_unhashed_relocation(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_NO_UNHASHED_RELOCATION))
#define reiserfs_hashed_relocation(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_HASHED_RELOCATION))
#define reiserfs_test4(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_TEST4))

#define have_large_tails(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_LARGETAIL))
#define have_small_tails(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_SMALLTAIL))
#define replay_only(s) (REISERFS_SB(s)->s_mount_opt & (1 << REPLAYONLY))
#define reiserfs_attrs(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_ATTRS))
#define old_format_only(s) (REISERFS_SB(s)->s_properties & (1 << REISERFS_3_5))
#define convert_reiserfs(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_CONVERT))
#define reiserfs_data_log(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_DATA_LOG))
#define reiserfs_data_ordered(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_DATA_ORDERED))
#define reiserfs_data_writeback(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_DATA_WRITEBACK))
#define reiserfs_xattrs_user(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_XATTRS_USER))
#define reiserfs_posixacl(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_POSIXACL))
#define reiserfs_expose_privroot(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_EXPOSE_PRIVROOT))
#define reiserfs_xattrs_optional(s) (reiserfs_xattrs_user(s) || reiserfs_posixacl(s))
#define reiserfs_barrier_none(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_BARRIER_NONE))
#define reiserfs_barrier_flush(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_BARRIER_FLUSH))

#define reiserfs_error_panic(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_ERROR_PANIC))
#define reiserfs_error_ro(s) (REISERFS_SB(s)->s_mount_opt & (1 << REISERFS_ERROR_RO))

void reiserfs_file_buffer(struct buffer_head *bh, int list);
extern struct file_system_type reiserfs_fs_type;
int reiserfs_resize(struct super_block *, unsigned long);

#define CARRY_ON                0
#define SCHEDULE_OCCURRED       1

#define SB_BUFFER_WITH_SB(s) (REISERFS_SB(s)->s_sbh)
#define SB_JOURNAL(s) (REISERFS_SB(s)->s_journal)
#define SB_JOURNAL_1st_RESERVED_BLOCK(s) (SB_JOURNAL(s)->j_1st_reserved_block)
#define SB_JOURNAL_LEN_FREE(s) (SB_JOURNAL(s)->j_journal_len_free)
#define SB_AP_BITMAP(s) (REISERFS_SB(s)->s_ap_bitmap)

#define SB_DISK_JOURNAL_HEAD(s) (SB_JOURNAL(s)->j_header_bh->)


static inline char *reiserfs_bdevname(struct super_block *s)
{
	return (s == NULL) ? "Null superblock" : s->s_id;
}

#define reiserfs_is_journal_aborted(journal) (unlikely (__reiserfs_is_journal_aborted (journal)))
static inline int __reiserfs_is_journal_aborted(struct reiserfs_journal
						*journal)
{
	return test_bit(J_ABORTED, &journal->j_state);
}

#endif				
