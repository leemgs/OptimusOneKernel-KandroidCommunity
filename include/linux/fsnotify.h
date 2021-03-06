#ifndef _LINUX_FS_NOTIFY_H
#define _LINUX_FS_NOTIFY_H



#include <linux/dnotify.h>
#include <linux/inotify.h>
#include <linux/fsnotify_backend.h>
#include <linux/audit.h>


static inline void fsnotify_d_instantiate(struct dentry *entry,
						struct inode *inode)
{
	__fsnotify_d_instantiate(entry, inode);

	inotify_d_instantiate(entry, inode);
}


static inline void fsnotify_parent(struct dentry *dentry, __u32 mask)
{
	__fsnotify_parent(dentry, mask);

	inotify_dentry_parent_queue_event(dentry, mask, 0, dentry->d_name.name);
}


static inline void fsnotify_d_move(struct dentry *entry)
{
	
	__fsnotify_update_dcache_flags(entry);

	inotify_d_move(entry);
}


static inline void fsnotify_link_count(struct inode *inode)
{
	inotify_inode_queue_event(inode, IN_ATTRIB, 0, NULL, NULL);

	fsnotify(inode, FS_ATTRIB, inode, FSNOTIFY_EVENT_INODE, NULL, 0);
}


static inline void fsnotify_move(struct inode *old_dir, struct inode *new_dir,
				 const char *old_name, const char *new_name,
				 int isdir, struct inode *target, struct dentry *moved)
{
	struct inode *source = moved->d_inode;
	u32 in_cookie = inotify_get_cookie();
	u32 fs_cookie = fsnotify_get_cookie();
	__u32 old_dir_mask = (FS_EVENT_ON_CHILD | FS_MOVED_FROM);
	__u32 new_dir_mask = (FS_EVENT_ON_CHILD | FS_MOVED_TO);

	if (old_dir == new_dir)
		old_dir_mask |= FS_DN_RENAME;

	if (isdir) {
		isdir = IN_ISDIR;
		old_dir_mask |= FS_IN_ISDIR;
		new_dir_mask |= FS_IN_ISDIR;
	}

	inotify_inode_queue_event(old_dir, IN_MOVED_FROM|isdir, in_cookie, old_name,
				  source);
	inotify_inode_queue_event(new_dir, IN_MOVED_TO|isdir, in_cookie, new_name,
				  source);

	fsnotify(old_dir, old_dir_mask, old_dir, FSNOTIFY_EVENT_INODE, old_name, fs_cookie);
	fsnotify(new_dir, new_dir_mask, new_dir, FSNOTIFY_EVENT_INODE, new_name, fs_cookie);

	if (target) {
		inotify_inode_queue_event(target, IN_DELETE_SELF, 0, NULL, NULL);
		inotify_inode_is_dead(target);

		
		fsnotify_link_count(target);
	}

	if (source) {
		inotify_inode_queue_event(source, IN_MOVE_SELF, 0, NULL, NULL);
		fsnotify(source, FS_MOVE_SELF, moved->d_inode, FSNOTIFY_EVENT_INODE, NULL, 0);
	}
	audit_inode_child(new_name, moved, new_dir);
}


static inline void fsnotify_inode_delete(struct inode *inode)
{
	__fsnotify_inode_delete(inode);
}


static inline void fsnotify_nameremove(struct dentry *dentry, int isdir)
{
	__u32 mask = FS_DELETE;

	if (isdir)
		mask |= FS_IN_ISDIR;

	fsnotify_parent(dentry, mask);
}


static inline void fsnotify_inoderemove(struct inode *inode)
{
	inotify_inode_queue_event(inode, IN_DELETE_SELF, 0, NULL, NULL);
	inotify_inode_is_dead(inode);

	fsnotify(inode, FS_DELETE_SELF, inode, FSNOTIFY_EVENT_INODE, NULL, 0);
	__fsnotify_inode_delete(inode);
}


static inline void fsnotify_create(struct inode *inode, struct dentry *dentry)
{
	inotify_inode_queue_event(inode, IN_CREATE, 0, dentry->d_name.name,
				  dentry->d_inode);
	audit_inode_child(dentry->d_name.name, dentry, inode);

	fsnotify(inode, FS_CREATE, dentry->d_inode, FSNOTIFY_EVENT_INODE, dentry->d_name.name, 0);
}


static inline void fsnotify_link(struct inode *dir, struct inode *inode, struct dentry *new_dentry)
{
	inotify_inode_queue_event(dir, IN_CREATE, 0, new_dentry->d_name.name,
				  inode);
	fsnotify_link_count(inode);
	audit_inode_child(new_dentry->d_name.name, new_dentry, dir);

	fsnotify(dir, FS_CREATE, inode, FSNOTIFY_EVENT_INODE, new_dentry->d_name.name, 0);
}


static inline void fsnotify_mkdir(struct inode *inode, struct dentry *dentry)
{
	__u32 mask = (FS_CREATE | FS_IN_ISDIR);
	struct inode *d_inode = dentry->d_inode;

	inotify_inode_queue_event(inode, mask, 0, dentry->d_name.name, d_inode);
	audit_inode_child(dentry->d_name.name, dentry, inode);

	fsnotify(inode, mask, d_inode, FSNOTIFY_EVENT_INODE, dentry->d_name.name, 0);
}


static inline void fsnotify_access(struct dentry *dentry)
{
	struct inode *inode = dentry->d_inode;
	__u32 mask = FS_ACCESS;

	if (S_ISDIR(inode->i_mode))
		mask |= FS_IN_ISDIR;

	inotify_inode_queue_event(inode, mask, 0, NULL, NULL);

	fsnotify_parent(dentry, mask);
	fsnotify(inode, mask, inode, FSNOTIFY_EVENT_INODE, NULL, 0);
}


static inline void fsnotify_modify(struct dentry *dentry)
{
	struct inode *inode = dentry->d_inode;
	__u32 mask = FS_MODIFY;

	if (S_ISDIR(inode->i_mode))
		mask |= FS_IN_ISDIR;

	inotify_inode_queue_event(inode, mask, 0, NULL, NULL);

	fsnotify_parent(dentry, mask);
	fsnotify(inode, mask, inode, FSNOTIFY_EVENT_INODE, NULL, 0);
}


static inline void fsnotify_open(struct dentry *dentry)
{
	struct inode *inode = dentry->d_inode;
	__u32 mask = FS_OPEN;

	if (S_ISDIR(inode->i_mode))
		mask |= FS_IN_ISDIR;

	inotify_inode_queue_event(inode, mask, 0, NULL, NULL);

	fsnotify_parent(dentry, mask);
	fsnotify(inode, mask, inode, FSNOTIFY_EVENT_INODE, NULL, 0);
}


static inline void fsnotify_close(struct file *file)
{
	struct dentry *dentry = file->f_path.dentry;
	struct inode *inode = dentry->d_inode;
	fmode_t mode = file->f_mode;
	__u32 mask = (mode & FMODE_WRITE) ? FS_CLOSE_WRITE : FS_CLOSE_NOWRITE;

	if (S_ISDIR(inode->i_mode))
		mask |= FS_IN_ISDIR;

	inotify_inode_queue_event(inode, mask, 0, NULL, NULL);

	fsnotify_parent(dentry, mask);
	fsnotify(inode, mask, file, FSNOTIFY_EVENT_FILE, NULL, 0);
}


static inline void fsnotify_xattr(struct dentry *dentry)
{
	struct inode *inode = dentry->d_inode;
	__u32 mask = FS_ATTRIB;

	if (S_ISDIR(inode->i_mode))
		mask |= FS_IN_ISDIR;

	inotify_inode_queue_event(inode, mask, 0, NULL, NULL);

	fsnotify_parent(dentry, mask);
	fsnotify(inode, mask, inode, FSNOTIFY_EVENT_INODE, NULL, 0);
}


static inline void fsnotify_change(struct dentry *dentry, unsigned int ia_valid)
{
	struct inode *inode = dentry->d_inode;
	__u32 mask = 0;

	if (ia_valid & ATTR_UID)
		mask |= FS_ATTRIB;
	if (ia_valid & ATTR_GID)
		mask |= FS_ATTRIB;
	if (ia_valid & ATTR_SIZE)
		mask |= FS_MODIFY;

	
	if ((ia_valid & (ATTR_ATIME | ATTR_MTIME)) == (ATTR_ATIME | ATTR_MTIME))
		mask |= FS_ATTRIB;
	else if (ia_valid & ATTR_ATIME)
		mask |= FS_ACCESS;
	else if (ia_valid & ATTR_MTIME)
		mask |= FS_MODIFY;

	if (ia_valid & ATTR_MODE)
		mask |= FS_ATTRIB;

	if (mask) {
		if (S_ISDIR(inode->i_mode))
			mask |= FS_IN_ISDIR;
		inotify_inode_queue_event(inode, mask, 0, NULL, NULL);

		fsnotify_parent(dentry, mask);
		fsnotify(inode, mask, inode, FSNOTIFY_EVENT_INODE, NULL, 0);
	}
}

#if defined(CONFIG_INOTIFY) || defined(CONFIG_FSNOTIFY)	


static inline const char *fsnotify_oldname_init(const char *name)
{
	return kstrdup(name, GFP_KERNEL);
}


static inline void fsnotify_oldname_free(const char *old_name)
{
	kfree(old_name);
}

#else	

static inline const char *fsnotify_oldname_init(const char *name)
{
	return NULL;
}

static inline void fsnotify_oldname_free(const char *old_name)
{
}

#endif	

#endif	
