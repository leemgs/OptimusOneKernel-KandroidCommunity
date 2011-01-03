

#ifndef __LINUX_FSNOTIFY_BACKEND_H
#define __LINUX_FSNOTIFY_BACKEND_H

#ifdef __KERNEL__

#include <linux/idr.h> 
#include <linux/fs.h> 
#include <linux/list.h>
#include <linux/path.h> 
#include <linux/spinlock.h>
#include <linux/types.h>

#include <asm/atomic.h>


#define FS_ACCESS		0x00000001	
#define FS_MODIFY		0x00000002	
#define FS_ATTRIB		0x00000004	
#define FS_CLOSE_WRITE		0x00000008	
#define FS_CLOSE_NOWRITE	0x00000010	
#define FS_OPEN			0x00000020	
#define FS_MOVED_FROM		0x00000040	
#define FS_MOVED_TO		0x00000080	
#define FS_CREATE		0x00000100	
#define FS_DELETE		0x00000200	
#define FS_DELETE_SELF		0x00000400	
#define FS_MOVE_SELF		0x00000800	

#define FS_UNMOUNT		0x00002000	
#define FS_Q_OVERFLOW		0x00004000	
#define FS_IN_IGNORED		0x00008000	

#define FS_IN_ISDIR		0x40000000	
#define FS_IN_ONESHOT		0x80000000	

#define FS_DN_RENAME		0x10000000	
#define FS_DN_MULTISHOT		0x20000000	


#define FS_EVENT_ON_CHILD	0x08000000


#define FS_EVENTS_POSS_ON_CHILD   (FS_ACCESS | FS_MODIFY | FS_ATTRIB |\
				   FS_CLOSE_WRITE | FS_CLOSE_NOWRITE | FS_OPEN |\
				   FS_MOVED_FROM | FS_MOVED_TO | FS_CREATE |\
				   FS_DELETE)


#define DNOTIFY_GROUP_NUM	UINT_MAX
#define INOTIFY_GROUP_NUM	(DNOTIFY_GROUP_NUM-1)

struct fsnotify_group;
struct fsnotify_event;
struct fsnotify_mark_entry;
struct fsnotify_event_private_data;


struct fsnotify_ops {
	bool (*should_send_event)(struct fsnotify_group *group, struct inode *inode, __u32 mask);
	int (*handle_event)(struct fsnotify_group *group, struct fsnotify_event *event);
	void (*free_group_priv)(struct fsnotify_group *group);
	void (*freeing_mark)(struct fsnotify_mark_entry *entry, struct fsnotify_group *group);
	void (*free_event_priv)(struct fsnotify_event_private_data *priv);
};


struct fsnotify_group {
	
	struct list_head group_list;

	
	__u32 mask;

	
	atomic_t refcnt;		
	unsigned int group_num;		

	const struct fsnotify_ops *ops;	

	
	struct mutex notification_mutex;	
	struct list_head notification_list;	
	wait_queue_head_t notification_waitq;	
	unsigned int q_len;			
	unsigned int max_events;		

	
	spinlock_t mark_lock;		
	atomic_t num_marks;		
	struct list_head mark_entries;	

	
	bool on_group_list;

	
	union {
		void *private;
#ifdef CONFIG_INOTIFY_USER
		struct inotify_group_private_data {
			spinlock_t	idr_lock;
			struct idr      idr;
			u32             last_wd;
			struct fasync_struct    *fa;    
			struct user_struct      *user;
		} inotify_data;
#endif
	};
};


struct fsnotify_event_holder {
	struct fsnotify_event *event;
	struct list_head event_list;
};


struct fsnotify_event_private_data {
	struct fsnotify_group *group;
	struct list_head event_list;
};


struct fsnotify_event {
	
	struct fsnotify_event_holder holder;
	spinlock_t lock;	
	
	struct inode *to_tell;	
	
	union {
		struct path path;
		struct inode *inode;
	};

#define FSNOTIFY_EVENT_NONE	0
#define FSNOTIFY_EVENT_PATH	1
#define FSNOTIFY_EVENT_INODE	2
#define FSNOTIFY_EVENT_FILE	3
	int data_type;		
	atomic_t refcnt;	
	__u32 mask;		

	u32 sync_cookie;	
	char *file_name;
	size_t name_len;

	struct list_head private_data_list;	
};


struct fsnotify_mark_entry {
	__u32 mask;			
	
	atomic_t refcnt;		
	struct inode *inode;		
	struct fsnotify_group *group;	
	struct hlist_node i_list;	
	struct list_head g_list;	
	spinlock_t lock;		
	struct list_head free_i_list;	
	struct list_head free_g_list;	
	void (*free_mark)(struct fsnotify_mark_entry *entry); 
};

#ifdef CONFIG_FSNOTIFY




extern void fsnotify(struct inode *to_tell, __u32 mask, void *data, int data_is,
		     const char *name, u32 cookie);
extern void __fsnotify_parent(struct dentry *dentry, __u32 mask);
extern void __fsnotify_inode_delete(struct inode *inode);
extern u32 fsnotify_get_cookie(void);

static inline int fsnotify_inode_watches_children(struct inode *inode)
{
	
	if (!(inode->i_fsnotify_mask & FS_EVENT_ON_CHILD))
		return 0;
	
	return inode->i_fsnotify_mask & FS_EVENTS_POSS_ON_CHILD;
}


static inline void __fsnotify_update_dcache_flags(struct dentry *dentry)
{
	struct dentry *parent;

	assert_spin_locked(&dcache_lock);
	assert_spin_locked(&dentry->d_lock);

	parent = dentry->d_parent;
	if (parent->d_inode && fsnotify_inode_watches_children(parent->d_inode))
		dentry->d_flags |= DCACHE_FSNOTIFY_PARENT_WATCHED;
	else
		dentry->d_flags &= ~DCACHE_FSNOTIFY_PARENT_WATCHED;
}


static inline void __fsnotify_d_instantiate(struct dentry *dentry, struct inode *inode)
{
	if (!inode)
		return;

	assert_spin_locked(&dcache_lock);

	spin_lock(&dentry->d_lock);
	__fsnotify_update_dcache_flags(dentry);
	spin_unlock(&dentry->d_lock);
}




extern void fsnotify_recalc_global_mask(void);

extern struct fsnotify_group *fsnotify_obtain_group(unsigned int group_num,
						    __u32 mask,
						    const struct fsnotify_ops *ops);

extern void fsnotify_recalc_group_mask(struct fsnotify_group *group);

extern void fsnotify_put_group(struct fsnotify_group *group);


extern void fsnotify_get_event(struct fsnotify_event *event);
extern void fsnotify_put_event(struct fsnotify_event *event);

extern struct fsnotify_event_private_data *fsnotify_remove_priv_from_event(struct fsnotify_group *group,
									   struct fsnotify_event *event);


extern int fsnotify_add_notify_event(struct fsnotify_group *group, struct fsnotify_event *event,
				     struct fsnotify_event_private_data *priv);

extern bool fsnotify_notify_queue_is_empty(struct fsnotify_group *group);

extern struct fsnotify_event *fsnotify_peek_notify_event(struct fsnotify_group *group);

extern struct fsnotify_event *fsnotify_remove_notify_event(struct fsnotify_group *group);




extern void fsnotify_recalc_inode_mask(struct inode *inode);
extern void fsnotify_init_mark(struct fsnotify_mark_entry *entry, void (*free_mark)(struct fsnotify_mark_entry *entry));

extern struct fsnotify_mark_entry *fsnotify_find_mark_entry(struct fsnotify_group *group, struct inode *inode);

extern int fsnotify_add_mark(struct fsnotify_mark_entry *entry, struct fsnotify_group *group, struct inode *inode);

extern void fsnotify_destroy_mark_by_entry(struct fsnotify_mark_entry *entry);

extern void fsnotify_clear_marks_by_group(struct fsnotify_group *group);
extern void fsnotify_get_mark(struct fsnotify_mark_entry *entry);
extern void fsnotify_put_mark(struct fsnotify_mark_entry *entry);
extern void fsnotify_unmount_inodes(struct list_head *list);


extern struct fsnotify_event *fsnotify_create_event(struct inode *to_tell, __u32 mask,
						    void *data, int data_is, const char *name,
						    u32 cookie, gfp_t gfp);

#else

static inline void fsnotify(struct inode *to_tell, __u32 mask, void *data, int data_is,
			    const char *name, u32 cookie)
{}

static inline void __fsnotify_parent(struct dentry *dentry, __u32 mask)
{}

static inline void __fsnotify_inode_delete(struct inode *inode)
{}

static inline void __fsnotify_update_dcache_flags(struct dentry *dentry)
{}

static inline void __fsnotify_d_instantiate(struct dentry *dentry, struct inode *inode)
{}

static inline u32 fsnotify_get_cookie(void)
{
	return 0;
}

static inline void fsnotify_unmount_inodes(struct list_head *list)
{}

#endif	

#endif	

#endif	
