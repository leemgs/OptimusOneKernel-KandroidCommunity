
#ifndef _LINUX_MOUNT_H
#define _LINUX_MOUNT_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/nodemask.h>
#include <linux/spinlock.h>
#include <asm/atomic.h>

struct super_block;
struct vfsmount;
struct dentry;
struct mnt_namespace;

#define MNT_NOSUID	0x01
#define MNT_NODEV	0x02
#define MNT_NOEXEC	0x04
#define MNT_NOATIME	0x08
#define MNT_NODIRATIME	0x10
#define MNT_RELATIME	0x20
#define MNT_READONLY	0x40	
#define MNT_STRICTATIME 0x80

#define MNT_SHRINKABLE	0x100
#define MNT_WRITE_HOLD	0x200

#define MNT_SHARED	0x1000	
#define MNT_UNBINDABLE	0x2000	
#define MNT_PNODE_MASK	0x3000	

struct vfsmount {
	struct list_head mnt_hash;
	struct vfsmount *mnt_parent;	
	struct dentry *mnt_mountpoint;	
	struct dentry *mnt_root;	
	struct super_block *mnt_sb;	
	struct list_head mnt_mounts;	
	struct list_head mnt_child;	
	int mnt_flags;
	
	const char *mnt_devname;	
	struct list_head mnt_list;
	struct list_head mnt_expire;	
	struct list_head mnt_share;	
	struct list_head mnt_slave_list;
	struct list_head mnt_slave;	
	struct vfsmount *mnt_master;	
	struct mnt_namespace *mnt_ns;	
	int mnt_id;			
	int mnt_group_id;		
	
	atomic_t mnt_count;
	int mnt_expiry_mark;		
	int mnt_pinned;
	int mnt_ghosts;
#ifdef CONFIG_SMP
	int *mnt_writers;
#else
	int mnt_writers;
#endif
};

static inline int *get_mnt_writers_ptr(struct vfsmount *mnt)
{
#ifdef CONFIG_SMP
	return mnt->mnt_writers;
#else
	return &mnt->mnt_writers;
#endif
}

static inline struct vfsmount *mntget(struct vfsmount *mnt)
{
	if (mnt)
		atomic_inc(&mnt->mnt_count);
	return mnt;
}

struct file; 

extern int mnt_want_write(struct vfsmount *mnt);
extern int mnt_want_write_file(struct file *file);
extern int mnt_clone_write(struct vfsmount *mnt);
extern void mnt_drop_write(struct vfsmount *mnt);
extern void mntput_no_expire(struct vfsmount *mnt);
extern void mnt_pin(struct vfsmount *mnt);
extern void mnt_unpin(struct vfsmount *mnt);
extern int __mnt_is_readonly(struct vfsmount *mnt);

static inline void mntput(struct vfsmount *mnt)
{
	if (mnt) {
		mnt->mnt_expiry_mark = 0;
		mntput_no_expire(mnt);
	}
}

extern struct vfsmount *do_kern_mount(const char *fstype, int flags,
				      const char *name, void *data);

struct file_system_type;
extern struct vfsmount *vfs_kern_mount(struct file_system_type *type,
				      int flags, const char *name,
				      void *data);

struct nameidata;

struct path;
extern int do_add_mount(struct vfsmount *newmnt, struct path *path,
			int mnt_flags, struct list_head *fslist);

extern void mark_mounts_for_expiry(struct list_head *mounts);

extern spinlock_t vfsmount_lock;
extern dev_t name_to_dev_t(char *name);

#endif 
