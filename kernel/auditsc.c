

#include <linux/init.h>
#include <asm/types.h>
#include <asm/atomic.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mount.h>
#include <linux/socket.h>
#include <linux/mqueue.h>
#include <linux/audit.h>
#include <linux/personality.h>
#include <linux/time.h>
#include <linux/netlink.h>
#include <linux/compiler.h>
#include <asm/unistd.h>
#include <linux/security.h>
#include <linux/list.h>
#include <linux/tty.h>
#include <linux/binfmts.h>
#include <linux/highmem.h>
#include <linux/syscalls.h>
#include <linux/inotify.h>
#include <linux/capability.h>
#include <linux/fs_struct.h>

#include "audit.h"


#define AUDIT_NAMES    20


#define AUDIT_NAME_FULL -1


#define MAX_EXECVE_AUDIT_LEN 7500


int audit_n_rules;


int audit_signals;

struct audit_cap_data {
	kernel_cap_t		permitted;
	kernel_cap_t		inheritable;
	union {
		unsigned int	fE;		
		kernel_cap_t	effective;	
	};
};


struct audit_names {
	const char	*name;
	int		name_len;	
	unsigned	name_put;	
	unsigned long	ino;
	dev_t		dev;
	umode_t		mode;
	uid_t		uid;
	gid_t		gid;
	dev_t		rdev;
	u32		osid;
	struct audit_cap_data fcap;
	unsigned int	fcap_ver;
};

struct audit_aux_data {
	struct audit_aux_data	*next;
	int			type;
};

#define AUDIT_AUX_IPCPERM	0


#define AUDIT_AUX_PIDS	16

struct audit_aux_data_execve {
	struct audit_aux_data	d;
	int argc;
	int envc;
	struct mm_struct *mm;
};

struct audit_aux_data_pids {
	struct audit_aux_data	d;
	pid_t			target_pid[AUDIT_AUX_PIDS];
	uid_t			target_auid[AUDIT_AUX_PIDS];
	uid_t			target_uid[AUDIT_AUX_PIDS];
	unsigned int		target_sessionid[AUDIT_AUX_PIDS];
	u32			target_sid[AUDIT_AUX_PIDS];
	char 			target_comm[AUDIT_AUX_PIDS][TASK_COMM_LEN];
	int			pid_count;
};

struct audit_aux_data_bprm_fcaps {
	struct audit_aux_data	d;
	struct audit_cap_data	fcap;
	unsigned int		fcap_ver;
	struct audit_cap_data	old_pcap;
	struct audit_cap_data	new_pcap;
};

struct audit_aux_data_capset {
	struct audit_aux_data	d;
	pid_t			pid;
	struct audit_cap_data	cap;
};

struct audit_tree_refs {
	struct audit_tree_refs *next;
	struct audit_chunk *c[31];
};


struct audit_context {
	int		    dummy;	
	int		    in_syscall;	
	enum audit_state    state, current_state;
	unsigned int	    serial;     
	int		    major;      
	struct timespec	    ctime;      
	unsigned long	    argv[4];    
	long		    return_code;
	u64		    prio;
	int		    return_valid; 
	int		    name_count;
	struct audit_names  names[AUDIT_NAMES];
	char *		    filterkey;	
	struct path	    pwd;
	struct audit_context *previous; 
	struct audit_aux_data *aux;
	struct audit_aux_data *aux_pids;
	struct sockaddr_storage *sockaddr;
	size_t sockaddr_len;
				
	pid_t		    pid, ppid;
	uid_t		    uid, euid, suid, fsuid;
	gid_t		    gid, egid, sgid, fsgid;
	unsigned long	    personality;
	int		    arch;

	pid_t		    target_pid;
	uid_t		    target_auid;
	uid_t		    target_uid;
	unsigned int	    target_sessionid;
	u32		    target_sid;
	char		    target_comm[TASK_COMM_LEN];

	struct audit_tree_refs *trees, *first_trees;
	struct list_head killed_trees;
	int tree_count;

	int type;
	union {
		struct {
			int nargs;
			long args[6];
		} socketcall;
		struct {
			uid_t			uid;
			gid_t			gid;
			mode_t			mode;
			u32			osid;
			int			has_perm;
			uid_t			perm_uid;
			gid_t			perm_gid;
			mode_t			perm_mode;
			unsigned long		qbytes;
		} ipc;
		struct {
			mqd_t			mqdes;
			struct mq_attr 		mqstat;
		} mq_getsetattr;
		struct {
			mqd_t			mqdes;
			int			sigev_signo;
		} mq_notify;
		struct {
			mqd_t			mqdes;
			size_t			msg_len;
			unsigned int		msg_prio;
			struct timespec		abs_timeout;
		} mq_sendrecv;
		struct {
			int			oflag;
			mode_t			mode;
			struct mq_attr		attr;
		} mq_open;
		struct {
			pid_t			pid;
			struct audit_cap_data	cap;
		} capset;
	};
	int fds[2];

#if AUDIT_DEBUG
	int		    put_count;
	int		    ino_count;
#endif
};

#define ACC_MODE(x) ("\004\002\006\006"[(x)&O_ACCMODE])
static inline int open_arg(int flags, int mask)
{
	int n = ACC_MODE(flags);
	if (flags & (O_TRUNC | O_CREAT))
		n |= AUDIT_PERM_WRITE;
	return n & mask;
}

static int audit_match_perm(struct audit_context *ctx, int mask)
{
	unsigned n;
	if (unlikely(!ctx))
		return 0;
	n = ctx->major;

	switch (audit_classify_syscall(ctx->arch, n)) {
	case 0:	
		if ((mask & AUDIT_PERM_WRITE) &&
		     audit_match_class(AUDIT_CLASS_WRITE, n))
			return 1;
		if ((mask & AUDIT_PERM_READ) &&
		     audit_match_class(AUDIT_CLASS_READ, n))
			return 1;
		if ((mask & AUDIT_PERM_ATTR) &&
		     audit_match_class(AUDIT_CLASS_CHATTR, n))
			return 1;
		return 0;
	case 1: 
		if ((mask & AUDIT_PERM_WRITE) &&
		     audit_match_class(AUDIT_CLASS_WRITE_32, n))
			return 1;
		if ((mask & AUDIT_PERM_READ) &&
		     audit_match_class(AUDIT_CLASS_READ_32, n))
			return 1;
		if ((mask & AUDIT_PERM_ATTR) &&
		     audit_match_class(AUDIT_CLASS_CHATTR_32, n))
			return 1;
		return 0;
	case 2: 
		return mask & ACC_MODE(ctx->argv[1]);
	case 3: 
		return mask & ACC_MODE(ctx->argv[2]);
	case 4: 
		return ((mask & AUDIT_PERM_WRITE) && ctx->argv[0] == SYS_BIND);
	case 5: 
		return mask & AUDIT_PERM_EXEC;
	default:
		return 0;
	}
}

static int audit_match_filetype(struct audit_context *ctx, int which)
{
	unsigned index = which & ~S_IFMT;
	mode_t mode = which & S_IFMT;

	if (unlikely(!ctx))
		return 0;

	if (index >= ctx->name_count)
		return 0;
	if (ctx->names[index].ino == -1)
		return 0;
	if ((ctx->names[index].mode ^ mode) & S_IFMT)
		return 0;
	return 1;
}



#ifdef CONFIG_AUDIT_TREE
static void audit_set_auditable(struct audit_context *ctx)
{
	if (!ctx->prio) {
		ctx->prio = 1;
		ctx->current_state = AUDIT_RECORD_CONTEXT;
	}
}

static int put_tree_ref(struct audit_context *ctx, struct audit_chunk *chunk)
{
	struct audit_tree_refs *p = ctx->trees;
	int left = ctx->tree_count;
	if (likely(left)) {
		p->c[--left] = chunk;
		ctx->tree_count = left;
		return 1;
	}
	if (!p)
		return 0;
	p = p->next;
	if (p) {
		p->c[30] = chunk;
		ctx->trees = p;
		ctx->tree_count = 30;
		return 1;
	}
	return 0;
}

static int grow_tree_refs(struct audit_context *ctx)
{
	struct audit_tree_refs *p = ctx->trees;
	ctx->trees = kzalloc(sizeof(struct audit_tree_refs), GFP_KERNEL);
	if (!ctx->trees) {
		ctx->trees = p;
		return 0;
	}
	if (p)
		p->next = ctx->trees;
	else
		ctx->first_trees = ctx->trees;
	ctx->tree_count = 31;
	return 1;
}
#endif

static void unroll_tree_refs(struct audit_context *ctx,
		      struct audit_tree_refs *p, int count)
{
#ifdef CONFIG_AUDIT_TREE
	struct audit_tree_refs *q;
	int n;
	if (!p) {
		
		p = ctx->first_trees;
		count = 31;
		
		if (!p)
			return;
	}
	n = count;
	for (q = p; q != ctx->trees; q = q->next, n = 31) {
		while (n--) {
			audit_put_chunk(q->c[n]);
			q->c[n] = NULL;
		}
	}
	while (n-- > ctx->tree_count) {
		audit_put_chunk(q->c[n]);
		q->c[n] = NULL;
	}
	ctx->trees = p;
	ctx->tree_count = count;
#endif
}

static void free_tree_refs(struct audit_context *ctx)
{
	struct audit_tree_refs *p, *q;
	for (p = ctx->first_trees; p; p = q) {
		q = p->next;
		kfree(p);
	}
}

static int match_tree_refs(struct audit_context *ctx, struct audit_tree *tree)
{
#ifdef CONFIG_AUDIT_TREE
	struct audit_tree_refs *p;
	int n;
	if (!tree)
		return 0;
	
	for (p = ctx->first_trees; p != ctx->trees; p = p->next) {
		for (n = 0; n < 31; n++)
			if (audit_tree_match(p->c[n], tree))
				return 1;
	}
	
	if (p) {
		for (n = ctx->tree_count; n < 31; n++)
			if (audit_tree_match(p->c[n], tree))
				return 1;
	}
#endif
	return 0;
}



static int audit_filter_rules(struct task_struct *tsk,
			      struct audit_krule *rule,
			      struct audit_context *ctx,
			      struct audit_names *name,
			      enum audit_state *state)
{
	const struct cred *cred = get_task_cred(tsk);
	int i, j, need_sid = 1;
	u32 sid;

	for (i = 0; i < rule->field_count; i++) {
		struct audit_field *f = &rule->fields[i];
		int result = 0;

		switch (f->type) {
		case AUDIT_PID:
			result = audit_comparator(tsk->pid, f->op, f->val);
			break;
		case AUDIT_PPID:
			if (ctx) {
				if (!ctx->ppid)
					ctx->ppid = sys_getppid();
				result = audit_comparator(ctx->ppid, f->op, f->val);
			}
			break;
		case AUDIT_UID:
			result = audit_comparator(cred->uid, f->op, f->val);
			break;
		case AUDIT_EUID:
			result = audit_comparator(cred->euid, f->op, f->val);
			break;
		case AUDIT_SUID:
			result = audit_comparator(cred->suid, f->op, f->val);
			break;
		case AUDIT_FSUID:
			result = audit_comparator(cred->fsuid, f->op, f->val);
			break;
		case AUDIT_GID:
			result = audit_comparator(cred->gid, f->op, f->val);
			break;
		case AUDIT_EGID:
			result = audit_comparator(cred->egid, f->op, f->val);
			break;
		case AUDIT_SGID:
			result = audit_comparator(cred->sgid, f->op, f->val);
			break;
		case AUDIT_FSGID:
			result = audit_comparator(cred->fsgid, f->op, f->val);
			break;
		case AUDIT_PERS:
			result = audit_comparator(tsk->personality, f->op, f->val);
			break;
		case AUDIT_ARCH:
			if (ctx)
				result = audit_comparator(ctx->arch, f->op, f->val);
			break;

		case AUDIT_EXIT:
			if (ctx && ctx->return_valid)
				result = audit_comparator(ctx->return_code, f->op, f->val);
			break;
		case AUDIT_SUCCESS:
			if (ctx && ctx->return_valid) {
				if (f->val)
					result = audit_comparator(ctx->return_valid, f->op, AUDITSC_SUCCESS);
				else
					result = audit_comparator(ctx->return_valid, f->op, AUDITSC_FAILURE);
			}
			break;
		case AUDIT_DEVMAJOR:
			if (name)
				result = audit_comparator(MAJOR(name->dev),
							  f->op, f->val);
			else if (ctx) {
				for (j = 0; j < ctx->name_count; j++) {
					if (audit_comparator(MAJOR(ctx->names[j].dev),	f->op, f->val)) {
						++result;
						break;
					}
				}
			}
			break;
		case AUDIT_DEVMINOR:
			if (name)
				result = audit_comparator(MINOR(name->dev),
							  f->op, f->val);
			else if (ctx) {
				for (j = 0; j < ctx->name_count; j++) {
					if (audit_comparator(MINOR(ctx->names[j].dev), f->op, f->val)) {
						++result;
						break;
					}
				}
			}
			break;
		case AUDIT_INODE:
			if (name)
				result = (name->ino == f->val);
			else if (ctx) {
				for (j = 0; j < ctx->name_count; j++) {
					if (audit_comparator(ctx->names[j].ino, f->op, f->val)) {
						++result;
						break;
					}
				}
			}
			break;
		case AUDIT_WATCH:
			if (name && audit_watch_inode(rule->watch) != (unsigned long)-1)
				result = (name->dev == audit_watch_dev(rule->watch) &&
					  name->ino == audit_watch_inode(rule->watch));
			break;
		case AUDIT_DIR:
			if (ctx)
				result = match_tree_refs(ctx, rule->tree);
			break;
		case AUDIT_LOGINUID:
			result = 0;
			if (ctx)
				result = audit_comparator(tsk->loginuid, f->op, f->val);
			break;
		case AUDIT_SUBJ_USER:
		case AUDIT_SUBJ_ROLE:
		case AUDIT_SUBJ_TYPE:
		case AUDIT_SUBJ_SEN:
		case AUDIT_SUBJ_CLR:
			
			if (f->lsm_rule) {
				if (need_sid) {
					security_task_getsecid(tsk, &sid);
					need_sid = 0;
				}
				result = security_audit_rule_match(sid, f->type,
				                                  f->op,
				                                  f->lsm_rule,
				                                  ctx);
			}
			break;
		case AUDIT_OBJ_USER:
		case AUDIT_OBJ_ROLE:
		case AUDIT_OBJ_TYPE:
		case AUDIT_OBJ_LEV_LOW:
		case AUDIT_OBJ_LEV_HIGH:
			
			if (f->lsm_rule) {
				
				if (name) {
					result = security_audit_rule_match(
					           name->osid, f->type, f->op,
					           f->lsm_rule, ctx);
				} else if (ctx) {
					for (j = 0; j < ctx->name_count; j++) {
						if (security_audit_rule_match(
						      ctx->names[j].osid,
						      f->type, f->op,
						      f->lsm_rule, ctx)) {
							++result;
							break;
						}
					}
				}
				
				if (!ctx || ctx->type != AUDIT_IPC)
					break;
				if (security_audit_rule_match(ctx->ipc.osid,
							      f->type, f->op,
							      f->lsm_rule, ctx))
					++result;
			}
			break;
		case AUDIT_ARG0:
		case AUDIT_ARG1:
		case AUDIT_ARG2:
		case AUDIT_ARG3:
			if (ctx)
				result = audit_comparator(ctx->argv[f->type-AUDIT_ARG0], f->op, f->val);
			break;
		case AUDIT_FILTERKEY:
			
			result = 1;
			break;
		case AUDIT_PERM:
			result = audit_match_perm(ctx, f->val);
			break;
		case AUDIT_FILETYPE:
			result = audit_match_filetype(ctx, f->val);
			break;
		}

		if (!result) {
			put_cred(cred);
			return 0;
		}
	}

	if (ctx) {
		if (rule->prio <= ctx->prio)
			return 0;
		if (rule->filterkey) {
			kfree(ctx->filterkey);
			ctx->filterkey = kstrdup(rule->filterkey, GFP_ATOMIC);
		}
		ctx->prio = rule->prio;
	}
	switch (rule->action) {
	case AUDIT_NEVER:    *state = AUDIT_DISABLED;	    break;
	case AUDIT_ALWAYS:   *state = AUDIT_RECORD_CONTEXT; break;
	}
	put_cred(cred);
	return 1;
}


static enum audit_state audit_filter_task(struct task_struct *tsk, char **key)
{
	struct audit_entry *e;
	enum audit_state   state;

	rcu_read_lock();
	list_for_each_entry_rcu(e, &audit_filter_list[AUDIT_FILTER_TASK], list) {
		if (audit_filter_rules(tsk, &e->rule, NULL, NULL, &state)) {
			if (state == AUDIT_RECORD_CONTEXT)
				*key = kstrdup(e->rule.filterkey, GFP_ATOMIC);
			rcu_read_unlock();
			return state;
		}
	}
	rcu_read_unlock();
	return AUDIT_BUILD_CONTEXT;
}


static enum audit_state audit_filter_syscall(struct task_struct *tsk,
					     struct audit_context *ctx,
					     struct list_head *list)
{
	struct audit_entry *e;
	enum audit_state state;

	if (audit_pid && tsk->tgid == audit_pid)
		return AUDIT_DISABLED;

	rcu_read_lock();
	if (!list_empty(list)) {
		int word = AUDIT_WORD(ctx->major);
		int bit  = AUDIT_BIT(ctx->major);

		list_for_each_entry_rcu(e, list, list) {
			if ((e->rule.mask[word] & bit) == bit &&
			    audit_filter_rules(tsk, &e->rule, ctx, NULL,
					       &state)) {
				rcu_read_unlock();
				ctx->current_state = state;
				return state;
			}
		}
	}
	rcu_read_unlock();
	return AUDIT_BUILD_CONTEXT;
}


void audit_filter_inodes(struct task_struct *tsk, struct audit_context *ctx)
{
	int i;
	struct audit_entry *e;
	enum audit_state state;

	if (audit_pid && tsk->tgid == audit_pid)
		return;

	rcu_read_lock();
	for (i = 0; i < ctx->name_count; i++) {
		int word = AUDIT_WORD(ctx->major);
		int bit  = AUDIT_BIT(ctx->major);
		struct audit_names *n = &ctx->names[i];
		int h = audit_hash_ino((u32)n->ino);
		struct list_head *list = &audit_inode_hash[h];

		if (list_empty(list))
			continue;

		list_for_each_entry_rcu(e, list, list) {
			if ((e->rule.mask[word] & bit) == bit &&
			    audit_filter_rules(tsk, &e->rule, ctx, n, &state)) {
				rcu_read_unlock();
				ctx->current_state = state;
				return;
			}
		}
	}
	rcu_read_unlock();
}

static inline struct audit_context *audit_get_context(struct task_struct *tsk,
						      int return_valid,
						      long return_code)
{
	struct audit_context *context = tsk->audit_context;

	if (likely(!context))
		return NULL;
	context->return_valid = return_valid;

	
	if (unlikely(return_code <= -ERESTARTSYS) &&
	    (return_code >= -ERESTART_RESTARTBLOCK) &&
	    (return_code != -ENOIOCTLCMD))
		context->return_code = -EINTR;
	else
		context->return_code  = return_code;

	if (context->in_syscall && !context->dummy) {
		audit_filter_syscall(tsk, context, &audit_filter_list[AUDIT_FILTER_EXIT]);
		audit_filter_inodes(tsk, context);
	}

	tsk->audit_context = NULL;
	return context;
}

static inline void audit_free_names(struct audit_context *context)
{
	int i;

#if AUDIT_DEBUG == 2
	if (context->put_count + context->ino_count != context->name_count) {
		printk(KERN_ERR "%s:%d(:%d): major=%d in_syscall=%d"
		       " name_count=%d put_count=%d"
		       " ino_count=%d [NOT freeing]\n",
		       __FILE__, __LINE__,
		       context->serial, context->major, context->in_syscall,
		       context->name_count, context->put_count,
		       context->ino_count);
		for (i = 0; i < context->name_count; i++) {
			printk(KERN_ERR "names[%d] = %p = %s\n", i,
			       context->names[i].name,
			       context->names[i].name ?: "(null)");
		}
		dump_stack();
		return;
	}
#endif
#if AUDIT_DEBUG
	context->put_count  = 0;
	context->ino_count  = 0;
#endif

	for (i = 0; i < context->name_count; i++) {
		if (context->names[i].name && context->names[i].name_put)
			__putname(context->names[i].name);
	}
	context->name_count = 0;
	path_put(&context->pwd);
	context->pwd.dentry = NULL;
	context->pwd.mnt = NULL;
}

static inline void audit_free_aux(struct audit_context *context)
{
	struct audit_aux_data *aux;

	while ((aux = context->aux)) {
		context->aux = aux->next;
		kfree(aux);
	}
	while ((aux = context->aux_pids)) {
		context->aux_pids = aux->next;
		kfree(aux);
	}
}

static inline void audit_zero_context(struct audit_context *context,
				      enum audit_state state)
{
	memset(context, 0, sizeof(*context));
	context->state      = state;
	context->prio = state == AUDIT_RECORD_CONTEXT ? ~0ULL : 0;
}

static inline struct audit_context *audit_alloc_context(enum audit_state state)
{
	struct audit_context *context;

	if (!(context = kmalloc(sizeof(*context), GFP_KERNEL)))
		return NULL;
	audit_zero_context(context, state);
	INIT_LIST_HEAD(&context->killed_trees);
	return context;
}


int audit_alloc(struct task_struct *tsk)
{
	struct audit_context *context;
	enum audit_state     state;
	char *key = NULL;

	if (likely(!audit_ever_enabled))
		return 0; 

	state = audit_filter_task(tsk, &key);
	if (likely(state == AUDIT_DISABLED))
		return 0;

	if (!(context = audit_alloc_context(state))) {
		kfree(key);
		audit_log_lost("out of memory in audit_alloc");
		return -ENOMEM;
	}
	context->filterkey = key;

	tsk->audit_context  = context;
	set_tsk_thread_flag(tsk, TIF_SYSCALL_AUDIT);
	return 0;
}

static inline void audit_free_context(struct audit_context *context)
{
	struct audit_context *previous;
	int		     count = 0;

	do {
		previous = context->previous;
		if (previous || (count &&  count < 10)) {
			++count;
			printk(KERN_ERR "audit(:%d): major=%d name_count=%d:"
			       " freeing multiple contexts (%d)\n",
			       context->serial, context->major,
			       context->name_count, count);
		}
		audit_free_names(context);
		unroll_tree_refs(context, NULL, 0);
		free_tree_refs(context);
		audit_free_aux(context);
		kfree(context->filterkey);
		kfree(context->sockaddr);
		kfree(context);
		context  = previous;
	} while (context);
	if (count >= 10)
		printk(KERN_ERR "audit: freed %d contexts\n", count);
}

void audit_log_task_context(struct audit_buffer *ab)
{
	char *ctx = NULL;
	unsigned len;
	int error;
	u32 sid;

	security_task_getsecid(current, &sid);
	if (!sid)
		return;

	error = security_secid_to_secctx(sid, &ctx, &len);
	if (error) {
		if (error != -EINVAL)
			goto error_path;
		return;
	}

	audit_log_format(ab, " subj=%s", ctx);
	security_release_secctx(ctx, len);
	return;

error_path:
	audit_panic("error in audit_log_task_context");
	return;
}

EXPORT_SYMBOL(audit_log_task_context);

static void audit_log_task_info(struct audit_buffer *ab, struct task_struct *tsk)
{
	char name[sizeof(tsk->comm)];
	struct mm_struct *mm = tsk->mm;
	struct vm_area_struct *vma;

	

	get_task_comm(name, tsk);
	audit_log_format(ab, " comm=");
	audit_log_untrustedstring(ab, name);

	if (mm) {
		down_read(&mm->mmap_sem);
		vma = mm->mmap;
		while (vma) {
			if ((vma->vm_flags & VM_EXECUTABLE) &&
			    vma->vm_file) {
				audit_log_d_path(ab, "exe=",
						 &vma->vm_file->f_path);
				break;
			}
			vma = vma->vm_next;
		}
		up_read(&mm->mmap_sem);
	}
	audit_log_task_context(ab);
}

static int audit_log_pid_context(struct audit_context *context, pid_t pid,
				 uid_t auid, uid_t uid, unsigned int sessionid,
				 u32 sid, char *comm)
{
	struct audit_buffer *ab;
	char *ctx = NULL;
	u32 len;
	int rc = 0;

	ab = audit_log_start(context, GFP_KERNEL, AUDIT_OBJ_PID);
	if (!ab)
		return rc;

	audit_log_format(ab, "opid=%d oauid=%d ouid=%d oses=%d", pid, auid,
			 uid, sessionid);
	if (security_secid_to_secctx(sid, &ctx, &len)) {
		audit_log_format(ab, " obj=(none)");
		rc = 1;
	} else {
		audit_log_format(ab, " obj=%s", ctx);
		security_release_secctx(ctx, len);
	}
	audit_log_format(ab, " ocomm=");
	audit_log_untrustedstring(ab, comm);
	audit_log_end(ab);

	return rc;
}


static int audit_log_single_execve_arg(struct audit_context *context,
					struct audit_buffer **ab,
					int arg_num,
					size_t *len_sent,
					const char __user *p,
					char *buf)
{
	char arg_num_len_buf[12];
	const char __user *tmp_p = p;
	
	size_t arg_num_len = snprintf(arg_num_len_buf, 12, "%d", arg_num) + 5;
	size_t len, len_left, to_send;
	size_t max_execve_audit_len = MAX_EXECVE_AUDIT_LEN;
	unsigned int i, has_cntl = 0, too_long = 0;
	int ret;

	
	len_left = len = strnlen_user(p, MAX_ARG_STRLEN) - 1;

	
	if (unlikely((len == -1) || len > MAX_ARG_STRLEN - 1)) {
		WARN_ON(1);
		send_sig(SIGKILL, current, 0);
		return -1;
	}

	
	do {
		if (len_left > MAX_EXECVE_AUDIT_LEN)
			to_send = MAX_EXECVE_AUDIT_LEN;
		else
			to_send = len_left;
		ret = copy_from_user(buf, tmp_p, to_send);
		
		if (ret) {
			WARN_ON(1);
			send_sig(SIGKILL, current, 0);
			return -1;
		}
		buf[to_send] = '\0';
		has_cntl = audit_string_contains_control(buf, to_send);
		if (has_cntl) {
			
			max_execve_audit_len = MAX_EXECVE_AUDIT_LEN / 2;
			break;
		}
		len_left -= to_send;
		tmp_p += to_send;
	} while (len_left > 0);

	len_left = len;

	if (len > max_execve_audit_len)
		too_long = 1;

	
	for (i = 0; len_left > 0; i++) {
		int room_left;

		if (len_left > max_execve_audit_len)
			to_send = max_execve_audit_len;
		else
			to_send = len_left;

		
		room_left = MAX_EXECVE_AUDIT_LEN - arg_num_len - *len_sent;
		if (has_cntl)
			room_left -= (to_send * 2);
		else
			room_left -= to_send;
		if (room_left < 0) {
			*len_sent = 0;
			audit_log_end(*ab);
			*ab = audit_log_start(context, GFP_KERNEL, AUDIT_EXECVE);
			if (!*ab)
				return 0;
		}

		
		if ((i == 0) && (too_long))
			audit_log_format(*ab, " a%d_len=%zu", arg_num,
					 has_cntl ? 2*len : len);

		
		if (len >= max_execve_audit_len)
			ret = copy_from_user(buf, p, to_send);
		else
			ret = 0;
		if (ret) {
			WARN_ON(1);
			send_sig(SIGKILL, current, 0);
			return -1;
		}
		buf[to_send] = '\0';

		
		audit_log_format(*ab, " a%d", arg_num);
		if (too_long)
			audit_log_format(*ab, "[%d]", i);
		audit_log_format(*ab, "=");
		if (has_cntl)
			audit_log_n_hex(*ab, buf, to_send);
		else
			audit_log_string(*ab, buf);

		p += to_send;
		len_left -= to_send;
		*len_sent += arg_num_len;
		if (has_cntl)
			*len_sent += to_send * 2;
		else
			*len_sent += to_send;
	}
	
	return len + 1;
}

static void audit_log_execve_info(struct audit_context *context,
				  struct audit_buffer **ab,
				  struct audit_aux_data_execve *axi)
{
	int i;
	size_t len, len_sent = 0;
	const char __user *p;
	char *buf;

	if (axi->mm != current->mm)
		return; 

	p = (const char __user *)axi->mm->arg_start;

	audit_log_format(*ab, "argc=%d", axi->argc);

	
	buf = kmalloc(MAX_EXECVE_AUDIT_LEN + 1, GFP_KERNEL);
	if (!buf) {
		audit_panic("out of memory for argv string\n");
		return;
	}

	for (i = 0; i < axi->argc; i++) {
		len = audit_log_single_execve_arg(context, ab, i,
						  &len_sent, p, buf);
		if (len <= 0)
			break;
		p += len;
	}
	kfree(buf);
}

static void audit_log_cap(struct audit_buffer *ab, char *prefix, kernel_cap_t *cap)
{
	int i;

	audit_log_format(ab, " %s=", prefix);
	CAP_FOR_EACH_U32(i) {
		audit_log_format(ab, "%08x", cap->cap[(_KERNEL_CAPABILITY_U32S-1) - i]);
	}
}

static void audit_log_fcaps(struct audit_buffer *ab, struct audit_names *name)
{
	kernel_cap_t *perm = &name->fcap.permitted;
	kernel_cap_t *inh = &name->fcap.inheritable;
	int log = 0;

	if (!cap_isclear(*perm)) {
		audit_log_cap(ab, "cap_fp", perm);
		log = 1;
	}
	if (!cap_isclear(*inh)) {
		audit_log_cap(ab, "cap_fi", inh);
		log = 1;
	}

	if (log)
		audit_log_format(ab, " cap_fe=%d cap_fver=%x", name->fcap.fE, name->fcap_ver);
}

static void show_special(struct audit_context *context, int *call_panic)
{
	struct audit_buffer *ab;
	int i;

	ab = audit_log_start(context, GFP_KERNEL, context->type);
	if (!ab)
		return;

	switch (context->type) {
	case AUDIT_SOCKETCALL: {
		int nargs = context->socketcall.nargs;
		audit_log_format(ab, "nargs=%d", nargs);
		for (i = 0; i < nargs; i++)
			audit_log_format(ab, " a%d=%lx", i,
				context->socketcall.args[i]);
		break; }
	case AUDIT_IPC: {
		u32 osid = context->ipc.osid;

		audit_log_format(ab, "ouid=%u ogid=%u mode=%#o",
			 context->ipc.uid, context->ipc.gid, context->ipc.mode);
		if (osid) {
			char *ctx = NULL;
			u32 len;
			if (security_secid_to_secctx(osid, &ctx, &len)) {
				audit_log_format(ab, " osid=%u", osid);
				*call_panic = 1;
			} else {
				audit_log_format(ab, " obj=%s", ctx);
				security_release_secctx(ctx, len);
			}
		}
		if (context->ipc.has_perm) {
			audit_log_end(ab);
			ab = audit_log_start(context, GFP_KERNEL,
					     AUDIT_IPC_SET_PERM);
			audit_log_format(ab,
				"qbytes=%lx ouid=%u ogid=%u mode=%#o",
				context->ipc.qbytes,
				context->ipc.perm_uid,
				context->ipc.perm_gid,
				context->ipc.perm_mode);
			if (!ab)
				return;
		}
		break; }
	case AUDIT_MQ_OPEN: {
		audit_log_format(ab,
			"oflag=0x%x mode=%#o mq_flags=0x%lx mq_maxmsg=%ld "
			"mq_msgsize=%ld mq_curmsgs=%ld",
			context->mq_open.oflag, context->mq_open.mode,
			context->mq_open.attr.mq_flags,
			context->mq_open.attr.mq_maxmsg,
			context->mq_open.attr.mq_msgsize,
			context->mq_open.attr.mq_curmsgs);
		break; }
	case AUDIT_MQ_SENDRECV: {
		audit_log_format(ab,
			"mqdes=%d msg_len=%zd msg_prio=%u "
			"abs_timeout_sec=%ld abs_timeout_nsec=%ld",
			context->mq_sendrecv.mqdes,
			context->mq_sendrecv.msg_len,
			context->mq_sendrecv.msg_prio,
			context->mq_sendrecv.abs_timeout.tv_sec,
			context->mq_sendrecv.abs_timeout.tv_nsec);
		break; }
	case AUDIT_MQ_NOTIFY: {
		audit_log_format(ab, "mqdes=%d sigev_signo=%d",
				context->mq_notify.mqdes,
				context->mq_notify.sigev_signo);
		break; }
	case AUDIT_MQ_GETSETATTR: {
		struct mq_attr *attr = &context->mq_getsetattr.mqstat;
		audit_log_format(ab,
			"mqdes=%d mq_flags=0x%lx mq_maxmsg=%ld mq_msgsize=%ld "
			"mq_curmsgs=%ld ",
			context->mq_getsetattr.mqdes,
			attr->mq_flags, attr->mq_maxmsg,
			attr->mq_msgsize, attr->mq_curmsgs);
		break; }
	case AUDIT_CAPSET: {
		audit_log_format(ab, "pid=%d", context->capset.pid);
		audit_log_cap(ab, "cap_pi", &context->capset.cap.inheritable);
		audit_log_cap(ab, "cap_pp", &context->capset.cap.permitted);
		audit_log_cap(ab, "cap_pe", &context->capset.cap.effective);
		break; }
	}
	audit_log_end(ab);
}

static void audit_log_exit(struct audit_context *context, struct task_struct *tsk)
{
	const struct cred *cred;
	int i, call_panic = 0;
	struct audit_buffer *ab;
	struct audit_aux_data *aux;
	const char *tty;

	
	context->pid = tsk->pid;
	if (!context->ppid)
		context->ppid = sys_getppid();
	cred = current_cred();
	context->uid   = cred->uid;
	context->gid   = cred->gid;
	context->euid  = cred->euid;
	context->suid  = cred->suid;
	context->fsuid = cred->fsuid;
	context->egid  = cred->egid;
	context->sgid  = cred->sgid;
	context->fsgid = cred->fsgid;
	context->personality = tsk->personality;

	ab = audit_log_start(context, GFP_KERNEL, AUDIT_SYSCALL);
	if (!ab)
		return;		
	audit_log_format(ab, "arch=%x syscall=%d",
			 context->arch, context->major);
	if (context->personality != PER_LINUX)
		audit_log_format(ab, " per=%lx", context->personality);
	if (context->return_valid)
		audit_log_format(ab, " success=%s exit=%ld",
				 (context->return_valid==AUDITSC_SUCCESS)?"yes":"no",
				 context->return_code);

	spin_lock_irq(&tsk->sighand->siglock);
	if (tsk->signal && tsk->signal->tty && tsk->signal->tty->name)
		tty = tsk->signal->tty->name;
	else
		tty = "(none)";
	spin_unlock_irq(&tsk->sighand->siglock);

	audit_log_format(ab,
		  " a0=%lx a1=%lx a2=%lx a3=%lx items=%d"
		  " ppid=%d pid=%d auid=%u uid=%u gid=%u"
		  " euid=%u suid=%u fsuid=%u"
		  " egid=%u sgid=%u fsgid=%u tty=%s ses=%u",
		  context->argv[0],
		  context->argv[1],
		  context->argv[2],
		  context->argv[3],
		  context->name_count,
		  context->ppid,
		  context->pid,
		  tsk->loginuid,
		  context->uid,
		  context->gid,
		  context->euid, context->suid, context->fsuid,
		  context->egid, context->sgid, context->fsgid, tty,
		  tsk->sessionid);


	audit_log_task_info(ab, tsk);
	audit_log_key(ab, context->filterkey);
	audit_log_end(ab);

	for (aux = context->aux; aux; aux = aux->next) {

		ab = audit_log_start(context, GFP_KERNEL, aux->type);
		if (!ab)
			continue; 

		switch (aux->type) {

		case AUDIT_EXECVE: {
			struct audit_aux_data_execve *axi = (void *)aux;
			audit_log_execve_info(context, &ab, axi);
			break; }

		case AUDIT_BPRM_FCAPS: {
			struct audit_aux_data_bprm_fcaps *axs = (void *)aux;
			audit_log_format(ab, "fver=%x", axs->fcap_ver);
			audit_log_cap(ab, "fp", &axs->fcap.permitted);
			audit_log_cap(ab, "fi", &axs->fcap.inheritable);
			audit_log_format(ab, " fe=%d", axs->fcap.fE);
			audit_log_cap(ab, "old_pp", &axs->old_pcap.permitted);
			audit_log_cap(ab, "old_pi", &axs->old_pcap.inheritable);
			audit_log_cap(ab, "old_pe", &axs->old_pcap.effective);
			audit_log_cap(ab, "new_pp", &axs->new_pcap.permitted);
			audit_log_cap(ab, "new_pi", &axs->new_pcap.inheritable);
			audit_log_cap(ab, "new_pe", &axs->new_pcap.effective);
			break; }

		}
		audit_log_end(ab);
	}

	if (context->type)
		show_special(context, &call_panic);

	if (context->fds[0] >= 0) {
		ab = audit_log_start(context, GFP_KERNEL, AUDIT_FD_PAIR);
		if (ab) {
			audit_log_format(ab, "fd0=%d fd1=%d",
					context->fds[0], context->fds[1]);
			audit_log_end(ab);
		}
	}

	if (context->sockaddr_len) {
		ab = audit_log_start(context, GFP_KERNEL, AUDIT_SOCKADDR);
		if (ab) {
			audit_log_format(ab, "saddr=");
			audit_log_n_hex(ab, (void *)context->sockaddr,
					context->sockaddr_len);
			audit_log_end(ab);
		}
	}

	for (aux = context->aux_pids; aux; aux = aux->next) {
		struct audit_aux_data_pids *axs = (void *)aux;

		for (i = 0; i < axs->pid_count; i++)
			if (audit_log_pid_context(context, axs->target_pid[i],
						  axs->target_auid[i],
						  axs->target_uid[i],
						  axs->target_sessionid[i],
						  axs->target_sid[i],
						  axs->target_comm[i]))
				call_panic = 1;
	}

	if (context->target_pid &&
	    audit_log_pid_context(context, context->target_pid,
				  context->target_auid, context->target_uid,
				  context->target_sessionid,
				  context->target_sid, context->target_comm))
			call_panic = 1;

	if (context->pwd.dentry && context->pwd.mnt) {
		ab = audit_log_start(context, GFP_KERNEL, AUDIT_CWD);
		if (ab) {
			audit_log_d_path(ab, "cwd=", &context->pwd);
			audit_log_end(ab);
		}
	}
	for (i = 0; i < context->name_count; i++) {
		struct audit_names *n = &context->names[i];

		ab = audit_log_start(context, GFP_KERNEL, AUDIT_PATH);
		if (!ab)
			continue; 

		audit_log_format(ab, "item=%d", i);

		if (n->name) {
			switch(n->name_len) {
			case AUDIT_NAME_FULL:
				
				audit_log_format(ab, " name=");
				audit_log_untrustedstring(ab, n->name);
				break;
			case 0:
				
				audit_log_d_path(ab, "name=", &context->pwd);
				break;
			default:
				
				audit_log_format(ab, " name=");
				audit_log_n_untrustedstring(ab, n->name,
							    n->name_len);
			}
		} else
			audit_log_format(ab, " name=(null)");

		if (n->ino != (unsigned long)-1) {
			audit_log_format(ab, " inode=%lu"
					 " dev=%02x:%02x mode=%#o"
					 " ouid=%u ogid=%u rdev=%02x:%02x",
					 n->ino,
					 MAJOR(n->dev),
					 MINOR(n->dev),
					 n->mode,
					 n->uid,
					 n->gid,
					 MAJOR(n->rdev),
					 MINOR(n->rdev));
		}
		if (n->osid != 0) {
			char *ctx = NULL;
			u32 len;
			if (security_secid_to_secctx(
				n->osid, &ctx, &len)) {
				audit_log_format(ab, " osid=%u", n->osid);
				call_panic = 2;
			} else {
				audit_log_format(ab, " obj=%s", ctx);
				security_release_secctx(ctx, len);
			}
		}

		audit_log_fcaps(ab, n);

		audit_log_end(ab);
	}

	
	ab = audit_log_start(context, GFP_KERNEL, AUDIT_EOE);
	if (ab)
		audit_log_end(ab);
	if (call_panic)
		audit_panic("error converting sid to string");
}


void audit_free(struct task_struct *tsk)
{
	struct audit_context *context;

	context = audit_get_context(tsk, 0, 0);
	if (likely(!context))
		return;

	
	
	if (context->in_syscall && context->current_state == AUDIT_RECORD_CONTEXT)
		audit_log_exit(context, tsk);
	if (!list_empty(&context->killed_trees))
		audit_kill_trees(&context->killed_trees);

	audit_free_context(context);
}


void audit_syscall_entry(int arch, int major,
			 unsigned long a1, unsigned long a2,
			 unsigned long a3, unsigned long a4)
{
	struct task_struct *tsk = current;
	struct audit_context *context = tsk->audit_context;
	enum audit_state     state;

	if (unlikely(!context))
		return;

	
	if (context->in_syscall) {
		struct audit_context *newctx;

#if AUDIT_DEBUG
		printk(KERN_ERR
		       "audit(:%d) pid=%d in syscall=%d;"
		       " entering syscall=%d\n",
		       context->serial, tsk->pid, context->major, major);
#endif
		newctx = audit_alloc_context(context->state);
		if (newctx) {
			newctx->previous   = context;
			context		   = newctx;
			tsk->audit_context = newctx;
		} else	{
			
			audit_zero_context(context, context->state);
		}
	}
	BUG_ON(context->in_syscall || context->name_count);

	if (!audit_enabled)
		return;

	context->arch	    = arch;
	context->major      = major;
	context->argv[0]    = a1;
	context->argv[1]    = a2;
	context->argv[2]    = a3;
	context->argv[3]    = a4;

	state = context->state;
	context->dummy = !audit_n_rules;
	if (!context->dummy && state == AUDIT_BUILD_CONTEXT) {
		context->prio = 0;
		state = audit_filter_syscall(tsk, context, &audit_filter_list[AUDIT_FILTER_ENTRY]);
	}
	if (likely(state == AUDIT_DISABLED))
		return;

	context->serial     = 0;
	context->ctime      = CURRENT_TIME;
	context->in_syscall = 1;
	context->current_state  = state;
	context->ppid       = 0;
}

void audit_finish_fork(struct task_struct *child)
{
	struct audit_context *ctx = current->audit_context;
	struct audit_context *p = child->audit_context;
	if (!p || !ctx)
		return;
	if (!ctx->in_syscall || ctx->current_state != AUDIT_RECORD_CONTEXT)
		return;
	p->arch = ctx->arch;
	p->major = ctx->major;
	memcpy(p->argv, ctx->argv, sizeof(ctx->argv));
	p->ctime = ctx->ctime;
	p->dummy = ctx->dummy;
	p->in_syscall = ctx->in_syscall;
	p->filterkey = kstrdup(ctx->filterkey, GFP_KERNEL);
	p->ppid = current->pid;
	p->prio = ctx->prio;
	p->current_state = ctx->current_state;
}


void audit_syscall_exit(int valid, long return_code)
{
	struct task_struct *tsk = current;
	struct audit_context *context;

	context = audit_get_context(tsk, valid, return_code);

	if (likely(!context))
		return;

	if (context->in_syscall && context->current_state == AUDIT_RECORD_CONTEXT)
		audit_log_exit(context, tsk);

	context->in_syscall = 0;
	context->prio = context->state == AUDIT_RECORD_CONTEXT ? ~0ULL : 0;

	if (!list_empty(&context->killed_trees))
		audit_kill_trees(&context->killed_trees);

	if (context->previous) {
		struct audit_context *new_context = context->previous;
		context->previous  = NULL;
		audit_free_context(context);
		tsk->audit_context = new_context;
	} else {
		audit_free_names(context);
		unroll_tree_refs(context, NULL, 0);
		audit_free_aux(context);
		context->aux = NULL;
		context->aux_pids = NULL;
		context->target_pid = 0;
		context->target_sid = 0;
		context->sockaddr_len = 0;
		context->type = 0;
		context->fds[0] = -1;
		if (context->state != AUDIT_RECORD_CONTEXT) {
			kfree(context->filterkey);
			context->filterkey = NULL;
		}
		tsk->audit_context = context;
	}
}

static inline void handle_one(const struct inode *inode)
{
#ifdef CONFIG_AUDIT_TREE
	struct audit_context *context;
	struct audit_tree_refs *p;
	struct audit_chunk *chunk;
	int count;
	if (likely(list_empty(&inode->inotify_watches)))
		return;
	context = current->audit_context;
	p = context->trees;
	count = context->tree_count;
	rcu_read_lock();
	chunk = audit_tree_lookup(inode);
	rcu_read_unlock();
	if (!chunk)
		return;
	if (likely(put_tree_ref(context, chunk)))
		return;
	if (unlikely(!grow_tree_refs(context))) {
		printk(KERN_WARNING "out of memory, audit has lost a tree reference\n");
		audit_set_auditable(context);
		audit_put_chunk(chunk);
		unroll_tree_refs(context, p, count);
		return;
	}
	put_tree_ref(context, chunk);
#endif
}

static void handle_path(const struct dentry *dentry)
{
#ifdef CONFIG_AUDIT_TREE
	struct audit_context *context;
	struct audit_tree_refs *p;
	const struct dentry *d, *parent;
	struct audit_chunk *drop;
	unsigned long seq;
	int count;

	context = current->audit_context;
	p = context->trees;
	count = context->tree_count;
retry:
	drop = NULL;
	d = dentry;
	rcu_read_lock();
	seq = read_seqbegin(&rename_lock);
	for(;;) {
		struct inode *inode = d->d_inode;
		if (inode && unlikely(!list_empty(&inode->inotify_watches))) {
			struct audit_chunk *chunk;
			chunk = audit_tree_lookup(inode);
			if (chunk) {
				if (unlikely(!put_tree_ref(context, chunk))) {
					drop = chunk;
					break;
				}
			}
		}
		parent = d->d_parent;
		if (parent == d)
			break;
		d = parent;
	}
	if (unlikely(read_seqretry(&rename_lock, seq) || drop)) {  
		rcu_read_unlock();
		if (!drop) {
			
			unroll_tree_refs(context, p, count);
			goto retry;
		}
		audit_put_chunk(drop);
		if (grow_tree_refs(context)) {
			
			unroll_tree_refs(context, p, count);
			goto retry;
		}
		
		printk(KERN_WARNING
			"out of memory, audit has lost a tree reference\n");
		unroll_tree_refs(context, p, count);
		audit_set_auditable(context);
		return;
	}
	rcu_read_unlock();
#endif
}


void __audit_getname(const char *name)
{
	struct audit_context *context = current->audit_context;

	if (IS_ERR(name) || !name)
		return;

	if (!context->in_syscall) {
#if AUDIT_DEBUG == 2
		printk(KERN_ERR "%s:%d(:%d): ignoring getname(%p)\n",
		       __FILE__, __LINE__, context->serial, name);
		dump_stack();
#endif
		return;
	}
	BUG_ON(context->name_count >= AUDIT_NAMES);
	context->names[context->name_count].name = name;
	context->names[context->name_count].name_len = AUDIT_NAME_FULL;
	context->names[context->name_count].name_put = 1;
	context->names[context->name_count].ino  = (unsigned long)-1;
	context->names[context->name_count].osid = 0;
	++context->name_count;
	if (!context->pwd.dentry) {
		read_lock(&current->fs->lock);
		context->pwd = current->fs->pwd;
		path_get(&current->fs->pwd);
		read_unlock(&current->fs->lock);
	}

}


void audit_putname(const char *name)
{
	struct audit_context *context = current->audit_context;

	BUG_ON(!context);
	if (!context->in_syscall) {
#if AUDIT_DEBUG == 2
		printk(KERN_ERR "%s:%d(:%d): __putname(%p)\n",
		       __FILE__, __LINE__, context->serial, name);
		if (context->name_count) {
			int i;
			for (i = 0; i < context->name_count; i++)
				printk(KERN_ERR "name[%d] = %p = %s\n", i,
				       context->names[i].name,
				       context->names[i].name ?: "(null)");
		}
#endif
		__putname(name);
	}
#if AUDIT_DEBUG
	else {
		++context->put_count;
		if (context->put_count > context->name_count) {
			printk(KERN_ERR "%s:%d(:%d): major=%d"
			       " in_syscall=%d putname(%p) name_count=%d"
			       " put_count=%d\n",
			       __FILE__, __LINE__,
			       context->serial, context->major,
			       context->in_syscall, name, context->name_count,
			       context->put_count);
			dump_stack();
		}
	}
#endif
}

static int audit_inc_name_count(struct audit_context *context,
				const struct inode *inode)
{
	if (context->name_count >= AUDIT_NAMES) {
		if (inode)
			printk(KERN_DEBUG "name_count maxed, losing inode data: "
			       "dev=%02x:%02x, inode=%lu\n",
			       MAJOR(inode->i_sb->s_dev),
			       MINOR(inode->i_sb->s_dev),
			       inode->i_ino);

		else
			printk(KERN_DEBUG "name_count maxed, losing inode data\n");
		return 1;
	}
	context->name_count++;
#if AUDIT_DEBUG
	context->ino_count++;
#endif
	return 0;
}


static inline int audit_copy_fcaps(struct audit_names *name, const struct dentry *dentry)
{
	struct cpu_vfs_cap_data caps;
	int rc;

	memset(&name->fcap.permitted, 0, sizeof(kernel_cap_t));
	memset(&name->fcap.inheritable, 0, sizeof(kernel_cap_t));
	name->fcap.fE = 0;
	name->fcap_ver = 0;

	if (!dentry)
		return 0;

	rc = get_vfs_caps_from_disk(dentry, &caps);
	if (rc)
		return rc;

	name->fcap.permitted = caps.permitted;
	name->fcap.inheritable = caps.inheritable;
	name->fcap.fE = !!(caps.magic_etc & VFS_CAP_FLAGS_EFFECTIVE);
	name->fcap_ver = (caps.magic_etc & VFS_CAP_REVISION_MASK) >> VFS_CAP_REVISION_SHIFT;

	return 0;
}



static void audit_copy_inode(struct audit_names *name, const struct dentry *dentry,
			     const struct inode *inode)
{
	name->ino   = inode->i_ino;
	name->dev   = inode->i_sb->s_dev;
	name->mode  = inode->i_mode;
	name->uid   = inode->i_uid;
	name->gid   = inode->i_gid;
	name->rdev  = inode->i_rdev;
	security_inode_getsecid(inode, &name->osid);
	audit_copy_fcaps(name, dentry);
}


void __audit_inode(const char *name, const struct dentry *dentry)
{
	int idx;
	struct audit_context *context = current->audit_context;
	const struct inode *inode = dentry->d_inode;

	if (!context->in_syscall)
		return;
	if (context->name_count
	    && context->names[context->name_count-1].name
	    && context->names[context->name_count-1].name == name)
		idx = context->name_count - 1;
	else if (context->name_count > 1
		 && context->names[context->name_count-2].name
		 && context->names[context->name_count-2].name == name)
		idx = context->name_count - 2;
	else {
		
		if (audit_inc_name_count(context, inode))
			return;
		idx = context->name_count - 1;
		context->names[idx].name = NULL;
	}
	handle_path(dentry);
	audit_copy_inode(&context->names[idx], dentry, inode);
}


void __audit_inode_child(const char *dname, const struct dentry *dentry,
			 const struct inode *parent)
{
	int idx;
	struct audit_context *context = current->audit_context;
	const char *found_parent = NULL, *found_child = NULL;
	const struct inode *inode = dentry->d_inode;
	int dirlen = 0;

	if (!context->in_syscall)
		return;

	if (inode)
		handle_one(inode);
	
	if (!dname)
		goto add_names;

	
	for (idx = 0; idx < context->name_count; idx++) {
		struct audit_names *n = &context->names[idx];

		if (!n->name)
			continue;

		if (n->ino == parent->i_ino &&
		    !audit_compare_dname_path(dname, n->name, &dirlen)) {
			n->name_len = dirlen; 
			found_parent = n->name;
			goto add_names;
		}
	}

	
	for (idx = 0; idx < context->name_count; idx++) {
		struct audit_names *n = &context->names[idx];

		if (!n->name)
			continue;

		
		if (!strcmp(dname, n->name) ||
		     !audit_compare_dname_path(dname, n->name, &dirlen)) {
			if (inode)
				audit_copy_inode(n, NULL, inode);
			else
				n->ino = (unsigned long)-1;
			found_child = n->name;
			goto add_names;
		}
	}

add_names:
	if (!found_parent) {
		if (audit_inc_name_count(context, parent))
			return;
		idx = context->name_count - 1;
		context->names[idx].name = NULL;
		audit_copy_inode(&context->names[idx], NULL, parent);
	}

	if (!found_child) {
		if (audit_inc_name_count(context, inode))
			return;
		idx = context->name_count - 1;

		
		if (found_parent) {
			context->names[idx].name = found_parent;
			context->names[idx].name_len = AUDIT_NAME_FULL;
			
			context->names[idx].name_put = 0;
		} else {
			context->names[idx].name = NULL;
		}

		if (inode)
			audit_copy_inode(&context->names[idx], NULL, inode);
		else
			context->names[idx].ino = (unsigned long)-1;
	}
}
EXPORT_SYMBOL_GPL(__audit_inode_child);


int auditsc_get_stamp(struct audit_context *ctx,
		       struct timespec *t, unsigned int *serial)
{
	if (!ctx->in_syscall)
		return 0;
	if (!ctx->serial)
		ctx->serial = audit_serial();
	t->tv_sec  = ctx->ctime.tv_sec;
	t->tv_nsec = ctx->ctime.tv_nsec;
	*serial    = ctx->serial;
	if (!ctx->prio) {
		ctx->prio = 1;
		ctx->current_state = AUDIT_RECORD_CONTEXT;
	}
	return 1;
}


static atomic_t session_id = ATOMIC_INIT(0);


int audit_set_loginuid(struct task_struct *task, uid_t loginuid)
{
	unsigned int sessionid = atomic_inc_return(&session_id);
	struct audit_context *context = task->audit_context;

	if (context && context->in_syscall) {
		struct audit_buffer *ab;

		ab = audit_log_start(NULL, GFP_KERNEL, AUDIT_LOGIN);
		if (ab) {
			audit_log_format(ab, "login pid=%d uid=%u "
				"old auid=%u new auid=%u"
				" old ses=%u new ses=%u",
				task->pid, task_uid(task),
				task->loginuid, loginuid,
				task->sessionid, sessionid);
			audit_log_end(ab);
		}
	}
	task->sessionid = sessionid;
	task->loginuid = loginuid;
	return 0;
}


void __audit_mq_open(int oflag, mode_t mode, struct mq_attr *attr)
{
	struct audit_context *context = current->audit_context;

	if (attr)
		memcpy(&context->mq_open.attr, attr, sizeof(struct mq_attr));
	else
		memset(&context->mq_open.attr, 0, sizeof(struct mq_attr));

	context->mq_open.oflag = oflag;
	context->mq_open.mode = mode;

	context->type = AUDIT_MQ_OPEN;
}


void __audit_mq_sendrecv(mqd_t mqdes, size_t msg_len, unsigned int msg_prio,
			const struct timespec *abs_timeout)
{
	struct audit_context *context = current->audit_context;
	struct timespec *p = &context->mq_sendrecv.abs_timeout;

	if (abs_timeout)
		memcpy(p, abs_timeout, sizeof(struct timespec));
	else
		memset(p, 0, sizeof(struct timespec));

	context->mq_sendrecv.mqdes = mqdes;
	context->mq_sendrecv.msg_len = msg_len;
	context->mq_sendrecv.msg_prio = msg_prio;

	context->type = AUDIT_MQ_SENDRECV;
}



void __audit_mq_notify(mqd_t mqdes, const struct sigevent *notification)
{
	struct audit_context *context = current->audit_context;

	if (notification)
		context->mq_notify.sigev_signo = notification->sigev_signo;
	else
		context->mq_notify.sigev_signo = 0;

	context->mq_notify.mqdes = mqdes;
	context->type = AUDIT_MQ_NOTIFY;
}


void __audit_mq_getsetattr(mqd_t mqdes, struct mq_attr *mqstat)
{
	struct audit_context *context = current->audit_context;
	context->mq_getsetattr.mqdes = mqdes;
	context->mq_getsetattr.mqstat = *mqstat;
	context->type = AUDIT_MQ_GETSETATTR;
}


void __audit_ipc_obj(struct kern_ipc_perm *ipcp)
{
	struct audit_context *context = current->audit_context;
	context->ipc.uid = ipcp->uid;
	context->ipc.gid = ipcp->gid;
	context->ipc.mode = ipcp->mode;
	context->ipc.has_perm = 0;
	security_ipc_getsecid(ipcp, &context->ipc.osid);
	context->type = AUDIT_IPC;
}


void __audit_ipc_set_perm(unsigned long qbytes, uid_t uid, gid_t gid, mode_t mode)
{
	struct audit_context *context = current->audit_context;

	context->ipc.qbytes = qbytes;
	context->ipc.perm_uid = uid;
	context->ipc.perm_gid = gid;
	context->ipc.perm_mode = mode;
	context->ipc.has_perm = 1;
}

int audit_bprm(struct linux_binprm *bprm)
{
	struct audit_aux_data_execve *ax;
	struct audit_context *context = current->audit_context;

	if (likely(!audit_enabled || !context || context->dummy))
		return 0;

	ax = kmalloc(sizeof(*ax), GFP_KERNEL);
	if (!ax)
		return -ENOMEM;

	ax->argc = bprm->argc;
	ax->envc = bprm->envc;
	ax->mm = bprm->mm;
	ax->d.type = AUDIT_EXECVE;
	ax->d.next = context->aux;
	context->aux = (void *)ax;
	return 0;
}



void audit_socketcall(int nargs, unsigned long *args)
{
	struct audit_context *context = current->audit_context;

	if (likely(!context || context->dummy))
		return;

	context->type = AUDIT_SOCKETCALL;
	context->socketcall.nargs = nargs;
	memcpy(context->socketcall.args, args, nargs * sizeof(unsigned long));
}


void __audit_fd_pair(int fd1, int fd2)
{
	struct audit_context *context = current->audit_context;
	context->fds[0] = fd1;
	context->fds[1] = fd2;
}


int audit_sockaddr(int len, void *a)
{
	struct audit_context *context = current->audit_context;

	if (likely(!context || context->dummy))
		return 0;

	if (!context->sockaddr) {
		void *p = kmalloc(sizeof(struct sockaddr_storage), GFP_KERNEL);
		if (!p)
			return -ENOMEM;
		context->sockaddr = p;
	}

	context->sockaddr_len = len;
	memcpy(context->sockaddr, a, len);
	return 0;
}

void __audit_ptrace(struct task_struct *t)
{
	struct audit_context *context = current->audit_context;

	context->target_pid = t->pid;
	context->target_auid = audit_get_loginuid(t);
	context->target_uid = task_uid(t);
	context->target_sessionid = audit_get_sessionid(t);
	security_task_getsecid(t, &context->target_sid);
	memcpy(context->target_comm, t->comm, TASK_COMM_LEN);
}


int __audit_signal_info(int sig, struct task_struct *t)
{
	struct audit_aux_data_pids *axp;
	struct task_struct *tsk = current;
	struct audit_context *ctx = tsk->audit_context;
	uid_t uid = current_uid(), t_uid = task_uid(t);

	if (audit_pid && t->tgid == audit_pid) {
		if (sig == SIGTERM || sig == SIGHUP || sig == SIGUSR1 || sig == SIGUSR2) {
			audit_sig_pid = tsk->pid;
			if (tsk->loginuid != -1)
				audit_sig_uid = tsk->loginuid;
			else
				audit_sig_uid = uid;
			security_task_getsecid(tsk, &audit_sig_sid);
		}
		if (!audit_signals || audit_dummy_context())
			return 0;
	}

	
	if (!ctx->target_pid) {
		ctx->target_pid = t->tgid;
		ctx->target_auid = audit_get_loginuid(t);
		ctx->target_uid = t_uid;
		ctx->target_sessionid = audit_get_sessionid(t);
		security_task_getsecid(t, &ctx->target_sid);
		memcpy(ctx->target_comm, t->comm, TASK_COMM_LEN);
		return 0;
	}

	axp = (void *)ctx->aux_pids;
	if (!axp || axp->pid_count == AUDIT_AUX_PIDS) {
		axp = kzalloc(sizeof(*axp), GFP_ATOMIC);
		if (!axp)
			return -ENOMEM;

		axp->d.type = AUDIT_OBJ_PID;
		axp->d.next = ctx->aux_pids;
		ctx->aux_pids = (void *)axp;
	}
	BUG_ON(axp->pid_count >= AUDIT_AUX_PIDS);

	axp->target_pid[axp->pid_count] = t->tgid;
	axp->target_auid[axp->pid_count] = audit_get_loginuid(t);
	axp->target_uid[axp->pid_count] = t_uid;
	axp->target_sessionid[axp->pid_count] = audit_get_sessionid(t);
	security_task_getsecid(t, &axp->target_sid[axp->pid_count]);
	memcpy(axp->target_comm[axp->pid_count], t->comm, TASK_COMM_LEN);
	axp->pid_count++;

	return 0;
}


int __audit_log_bprm_fcaps(struct linux_binprm *bprm,
			   const struct cred *new, const struct cred *old)
{
	struct audit_aux_data_bprm_fcaps *ax;
	struct audit_context *context = current->audit_context;
	struct cpu_vfs_cap_data vcaps;
	struct dentry *dentry;

	ax = kmalloc(sizeof(*ax), GFP_KERNEL);
	if (!ax)
		return -ENOMEM;

	ax->d.type = AUDIT_BPRM_FCAPS;
	ax->d.next = context->aux;
	context->aux = (void *)ax;

	dentry = dget(bprm->file->f_dentry);
	get_vfs_caps_from_disk(dentry, &vcaps);
	dput(dentry);

	ax->fcap.permitted = vcaps.permitted;
	ax->fcap.inheritable = vcaps.inheritable;
	ax->fcap.fE = !!(vcaps.magic_etc & VFS_CAP_FLAGS_EFFECTIVE);
	ax->fcap_ver = (vcaps.magic_etc & VFS_CAP_REVISION_MASK) >> VFS_CAP_REVISION_SHIFT;

	ax->old_pcap.permitted   = old->cap_permitted;
	ax->old_pcap.inheritable = old->cap_inheritable;
	ax->old_pcap.effective   = old->cap_effective;

	ax->new_pcap.permitted   = new->cap_permitted;
	ax->new_pcap.inheritable = new->cap_inheritable;
	ax->new_pcap.effective   = new->cap_effective;
	return 0;
}


void __audit_log_capset(pid_t pid,
		       const struct cred *new, const struct cred *old)
{
	struct audit_context *context = current->audit_context;
	context->capset.pid = pid;
	context->capset.cap.effective   = new->cap_effective;
	context->capset.cap.inheritable = new->cap_effective;
	context->capset.cap.permitted   = new->cap_permitted;
	context->type = AUDIT_CAPSET;
}


void audit_core_dumps(long signr)
{
	struct audit_buffer *ab;
	u32 sid;
	uid_t auid = audit_get_loginuid(current), uid;
	gid_t gid;
	unsigned int sessionid = audit_get_sessionid(current);

	if (!audit_enabled)
		return;

	if (signr == SIGQUIT)	
		return;

	ab = audit_log_start(NULL, GFP_KERNEL, AUDIT_ANOM_ABEND);
	current_uid_gid(&uid, &gid);
	audit_log_format(ab, "auid=%u uid=%u gid=%u ses=%u",
			 auid, uid, gid, sessionid);
	security_task_getsecid(current, &sid);
	if (sid) {
		char *ctx = NULL;
		u32 len;

		if (security_secid_to_secctx(sid, &ctx, &len))
			audit_log_format(ab, " ssid=%u", sid);
		else {
			audit_log_format(ab, " subj=%s", ctx);
			security_release_secctx(ctx, len);
		}
	}
	audit_log_format(ab, " pid=%d comm=", current->pid);
	audit_log_untrustedstring(ab, current->comm);
	audit_log_format(ab, " sig=%ld", signr);
	audit_log_end(ab);
}

struct list_head *audit_killed_trees(void)
{
	struct audit_context *ctx = current->audit_context;
	if (likely(!ctx || !ctx->in_syscall))
		return NULL;
	return &ctx->killed_trees;
}
