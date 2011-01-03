

#include <linux/kernel.h>
#include <linux/audit.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/netlink.h>
#include <linux/sched.h>
#include <linux/inotify.h>
#include <linux/security.h>
#include "audit.h"



struct audit_watch {
	atomic_t		count;	
	dev_t			dev;	
	char			*path;	
	unsigned long		ino;	
	struct audit_parent	*parent; 
	struct list_head	wlist;	
	struct list_head	rules;	
};

struct audit_parent {
	struct list_head	ilist;	
	struct list_head	watches; 
	struct inotify_watch	wdata;	
	unsigned		flags;	
};


struct inotify_handle *audit_ih;


#define AUDIT_PARENT_INVALID	0x001


#define AUDIT_IN_WATCH IN_MOVE|IN_CREATE|IN_DELETE|IN_DELETE_SELF|IN_MOVE_SELF

static void audit_free_parent(struct inotify_watch *i_watch)
{
	struct audit_parent *parent;

	parent = container_of(i_watch, struct audit_parent, wdata);
	WARN_ON(!list_empty(&parent->watches));
	kfree(parent);
}

void audit_get_watch(struct audit_watch *watch)
{
	atomic_inc(&watch->count);
}

void audit_put_watch(struct audit_watch *watch)
{
	if (atomic_dec_and_test(&watch->count)) {
		WARN_ON(watch->parent);
		WARN_ON(!list_empty(&watch->rules));
		kfree(watch->path);
		kfree(watch);
	}
}

void audit_remove_watch(struct audit_watch *watch)
{
	list_del(&watch->wlist);
	put_inotify_watch(&watch->parent->wdata);
	watch->parent = NULL;
	audit_put_watch(watch); 
}

char *audit_watch_path(struct audit_watch *watch)
{
	return watch->path;
}

struct list_head *audit_watch_rules(struct audit_watch *watch)
{
	return &watch->rules;
}

unsigned long audit_watch_inode(struct audit_watch *watch)
{
	return watch->ino;
}

dev_t audit_watch_dev(struct audit_watch *watch)
{
	return watch->dev;
}


static struct audit_parent *audit_init_parent(struct nameidata *ndp)
{
	struct audit_parent *parent;
	s32 wd;

	parent = kzalloc(sizeof(*parent), GFP_KERNEL);
	if (unlikely(!parent))
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&parent->watches);
	parent->flags = 0;

	inotify_init_watch(&parent->wdata);
	
	get_inotify_watch(&parent->wdata);
	wd = inotify_add_watch(audit_ih, &parent->wdata,
			       ndp->path.dentry->d_inode, AUDIT_IN_WATCH);
	if (wd < 0) {
		audit_free_parent(&parent->wdata);
		return ERR_PTR(wd);
	}

	return parent;
}


static struct audit_watch *audit_init_watch(char *path)
{
	struct audit_watch *watch;

	watch = kzalloc(sizeof(*watch), GFP_KERNEL);
	if (unlikely(!watch))
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&watch->rules);
	atomic_set(&watch->count, 1);
	watch->path = path;
	watch->dev = (dev_t)-1;
	watch->ino = (unsigned long)-1;

	return watch;
}


int audit_to_watch(struct audit_krule *krule, char *path, int len, u32 op)
{
	struct audit_watch *watch;

	if (!audit_ih)
		return -EOPNOTSUPP;

	if (path[0] != '/' || path[len-1] == '/' ||
	    krule->listnr != AUDIT_FILTER_EXIT ||
	    op != Audit_equal ||
	    krule->inode_f || krule->watch || krule->tree)
		return -EINVAL;

	watch = audit_init_watch(path);
	if (IS_ERR(watch))
		return PTR_ERR(watch);

	audit_get_watch(watch);
	krule->watch = watch;

	return 0;
}


static struct audit_watch *audit_dupe_watch(struct audit_watch *old)
{
	char *path;
	struct audit_watch *new;

	path = kstrdup(old->path, GFP_KERNEL);
	if (unlikely(!path))
		return ERR_PTR(-ENOMEM);

	new = audit_init_watch(path);
	if (IS_ERR(new)) {
		kfree(path);
		goto out;
	}

	new->dev = old->dev;
	new->ino = old->ino;
	get_inotify_watch(&old->parent->wdata);
	new->parent = old->parent;

out:
	return new;
}

static void audit_watch_log_rule_change(struct audit_krule *r, struct audit_watch *w, char *op)
{
	if (audit_enabled) {
		struct audit_buffer *ab;
		ab = audit_log_start(NULL, GFP_NOFS, AUDIT_CONFIG_CHANGE);
		audit_log_format(ab, "auid=%u ses=%u op=",
				 audit_get_loginuid(current),
				 audit_get_sessionid(current));
		audit_log_string(ab, op);
		audit_log_format(ab, " path=");
		audit_log_untrustedstring(ab, w->path);
		audit_log_key(ab, r->filterkey);
		audit_log_format(ab, " list=%d res=1", r->listnr);
		audit_log_end(ab);
	}
}


static void audit_update_watch(struct audit_parent *parent,
			       const char *dname, dev_t dev,
			       unsigned long ino, unsigned invalidating)
{
	struct audit_watch *owatch, *nwatch, *nextw;
	struct audit_krule *r, *nextr;
	struct audit_entry *oentry, *nentry;

	mutex_lock(&audit_filter_mutex);
	list_for_each_entry_safe(owatch, nextw, &parent->watches, wlist) {
		if (audit_compare_dname_path(dname, owatch->path, NULL))
			continue;

		
		if (invalidating && current->audit_context)
			audit_filter_inodes(current, current->audit_context);

		nwatch = audit_dupe_watch(owatch);
		if (IS_ERR(nwatch)) {
			mutex_unlock(&audit_filter_mutex);
			audit_panic("error updating watch, skipping");
			return;
		}
		nwatch->dev = dev;
		nwatch->ino = ino;

		list_for_each_entry_safe(r, nextr, &owatch->rules, rlist) {

			oentry = container_of(r, struct audit_entry, rule);
			list_del(&oentry->rule.rlist);
			list_del_rcu(&oentry->list);

			nentry = audit_dupe_rule(&oentry->rule, nwatch);
			if (IS_ERR(nentry)) {
				list_del(&oentry->rule.list);
				audit_panic("error updating watch, removing");
			} else {
				int h = audit_hash_ino((u32)ino);
				list_add(&nentry->rule.rlist, &nwatch->rules);
				list_add_rcu(&nentry->list, &audit_inode_hash[h]);
				list_replace(&oentry->rule.list,
					     &nentry->rule.list);
			}

			audit_watch_log_rule_change(r, owatch, "updated rules");

			call_rcu(&oentry->rcu, audit_free_rule_rcu);
		}

		audit_remove_watch(owatch);
		goto add_watch_to_parent; 
	}
	mutex_unlock(&audit_filter_mutex);
	return;

add_watch_to_parent:
	list_add(&nwatch->wlist, &parent->watches);
	mutex_unlock(&audit_filter_mutex);
	return;
}


static void audit_remove_parent_watches(struct audit_parent *parent)
{
	struct audit_watch *w, *nextw;
	struct audit_krule *r, *nextr;
	struct audit_entry *e;

	mutex_lock(&audit_filter_mutex);
	parent->flags |= AUDIT_PARENT_INVALID;
	list_for_each_entry_safe(w, nextw, &parent->watches, wlist) {
		list_for_each_entry_safe(r, nextr, &w->rules, rlist) {
			e = container_of(r, struct audit_entry, rule);
			audit_watch_log_rule_change(r, w, "remove rule");
			list_del(&r->rlist);
			list_del(&r->list);
			list_del_rcu(&e->list);
			call_rcu(&e->rcu, audit_free_rule_rcu);
		}
		audit_remove_watch(w);
	}
	mutex_unlock(&audit_filter_mutex);
}


void audit_inotify_unregister(struct list_head *in_list)
{
	struct audit_parent *p, *n;

	list_for_each_entry_safe(p, n, in_list, ilist) {
		list_del(&p->ilist);
		inotify_rm_watch(audit_ih, &p->wdata);
		
		unpin_inotify_watch(&p->wdata);
	}
}


static int audit_get_nd(char *path, struct nameidata **ndp, struct nameidata **ndw)
{
	struct nameidata *ndparent, *ndwatch;
	int err;

	ndparent = kmalloc(sizeof(*ndparent), GFP_KERNEL);
	if (unlikely(!ndparent))
		return -ENOMEM;

	ndwatch = kmalloc(sizeof(*ndwatch), GFP_KERNEL);
	if (unlikely(!ndwatch)) {
		kfree(ndparent);
		return -ENOMEM;
	}

	err = path_lookup(path, LOOKUP_PARENT, ndparent);
	if (err) {
		kfree(ndparent);
		kfree(ndwatch);
		return err;
	}

	err = path_lookup(path, 0, ndwatch);
	if (err) {
		kfree(ndwatch);
		ndwatch = NULL;
	}

	*ndp = ndparent;
	*ndw = ndwatch;

	return 0;
}


static void audit_put_nd(struct nameidata *ndp, struct nameidata *ndw)
{
	if (ndp) {
		path_put(&ndp->path);
		kfree(ndp);
	}
	if (ndw) {
		path_put(&ndw->path);
		kfree(ndw);
	}
}


static void audit_add_to_parent(struct audit_krule *krule,
				struct audit_parent *parent)
{
	struct audit_watch *w, *watch = krule->watch;
	int watch_found = 0;

	list_for_each_entry(w, &parent->watches, wlist) {
		if (strcmp(watch->path, w->path))
			continue;

		watch_found = 1;

		
		audit_put_watch(watch);
		audit_put_watch(watch);

		audit_get_watch(w);
		krule->watch = watch = w;
		break;
	}

	if (!watch_found) {
		get_inotify_watch(&parent->wdata);
		watch->parent = parent;

		list_add(&watch->wlist, &parent->watches);
	}
	list_add(&krule->rlist, &watch->rules);
}


int audit_add_watch(struct audit_krule *krule)
{
	struct audit_watch *watch = krule->watch;
	struct inotify_watch *i_watch;
	struct audit_parent *parent;
	struct nameidata *ndp = NULL, *ndw = NULL;
	int ret = 0;

	mutex_unlock(&audit_filter_mutex);

	
	ret = audit_get_nd(watch->path, &ndp, &ndw);
	if (ret) {
		
		mutex_lock(&audit_filter_mutex);
		goto error;
	}

	
	if (ndw) {
		watch->dev = ndw->path.dentry->d_inode->i_sb->s_dev;
		watch->ino = ndw->path.dentry->d_inode->i_ino;
	}

	
	if (inotify_find_watch(audit_ih, ndp->path.dentry->d_inode,
			       &i_watch) < 0) {
		parent = audit_init_parent(ndp);
		if (IS_ERR(parent)) {
			
			mutex_lock(&audit_filter_mutex);
			ret = PTR_ERR(parent);
			goto error;
		}
	} else
		parent = container_of(i_watch, struct audit_parent, wdata);

	mutex_lock(&audit_filter_mutex);

	
	if (parent->flags & AUDIT_PARENT_INVALID)
		ret = -ENOENT;
	else
		audit_add_to_parent(krule, parent);

	
	put_inotify_watch(&parent->wdata);

error:
	audit_put_nd(ndp, ndw);		
	return ret;

}

void audit_remove_watch_rule(struct audit_krule *krule, struct list_head *list)
{
	struct audit_watch *watch = krule->watch;
	struct audit_parent *parent = watch->parent;

	list_del(&krule->rlist);

	if (list_empty(&watch->rules)) {
		audit_remove_watch(watch);

		if (list_empty(&parent->watches)) {
			
			if (pin_inotify_watch(&parent->wdata))
				list_add(&parent->ilist, list);
		}
	}
}


static void audit_handle_ievent(struct inotify_watch *i_watch, u32 wd, u32 mask,
			 u32 cookie, const char *dname, struct inode *inode)
{
	struct audit_parent *parent;

	parent = container_of(i_watch, struct audit_parent, wdata);

	if (mask & (IN_CREATE|IN_MOVED_TO) && inode)
		audit_update_watch(parent, dname, inode->i_sb->s_dev,
				   inode->i_ino, 0);
	else if (mask & (IN_DELETE|IN_MOVED_FROM))
		audit_update_watch(parent, dname, (dev_t)-1, (unsigned long)-1, 1);
	
	else if (mask & (IN_DELETE_SELF|IN_UNMOUNT))
		audit_remove_parent_watches(parent);
	
	else if(mask & IN_MOVE_SELF) {
		audit_remove_parent_watches(parent);
		inotify_remove_watch_locked(audit_ih, i_watch);
	} else if (mask & IN_IGNORED)
		put_inotify_watch(i_watch);
}

static const struct inotify_operations audit_inotify_ops = {
	.handle_event   = audit_handle_ievent,
	.destroy_watch  = audit_free_parent,
};

static int __init audit_watch_init(void)
{
	audit_ih = inotify_init(&audit_inotify_ops);
	if (IS_ERR(audit_ih))
		audit_panic("cannot initialize inotify handle");
	return 0;
}
subsys_initcall(audit_watch_init);
