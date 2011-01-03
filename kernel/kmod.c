
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/completion.h>
#include <linux/file.h>
#include <linux/fdtable.h>
#include <linux/workqueue.h>
#include <linux/security.h>
#include <linux/mount.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <asm/uaccess.h>

#include <trace/events/module.h>

extern int max_threads;

static struct workqueue_struct *khelper_wq;

#ifdef CONFIG_MODULES


char modprobe_path[KMOD_PATH_LEN] = "/sbin/modprobe";


int __request_module(bool wait, const char *fmt, ...)
{
	va_list args;
	char module_name[MODULE_NAME_LEN];
	unsigned int max_modprobes;
	int ret;
	char *argv[] = { modprobe_path, "-q", "--", module_name, NULL };
	static char *envp[] = { "HOME=/",
				"TERM=linux",
				"PATH=/sbin:/usr/sbin:/bin:/usr/bin",
				NULL };
	static atomic_t kmod_concurrent = ATOMIC_INIT(0);
#define MAX_KMOD_CONCURRENT 50	
	static int kmod_loop_msg;

	ret = security_kernel_module_request();
	if (ret)
		return ret;

	va_start(args, fmt);
	ret = vsnprintf(module_name, MODULE_NAME_LEN, fmt, args);
	va_end(args);
	if (ret >= MODULE_NAME_LEN)
		return -ENAMETOOLONG;

	
	max_modprobes = min(max_threads/2, MAX_KMOD_CONCURRENT);
	atomic_inc(&kmod_concurrent);
	if (atomic_read(&kmod_concurrent) > max_modprobes) {
		
		if (kmod_loop_msg++ < 5)
			printk(KERN_ERR
			       "request_module: runaway loop modprobe %s\n",
			       module_name);
		atomic_dec(&kmod_concurrent);
		return -ENOMEM;
	}

	trace_module_request(module_name, wait, _RET_IP_);

	ret = call_usermodehelper(modprobe_path, argv, envp,
			wait ? UMH_WAIT_PROC : UMH_WAIT_EXEC);
	atomic_dec(&kmod_concurrent);
	return ret;
}
EXPORT_SYMBOL(__request_module);
#endif 

struct subprocess_info {
	struct work_struct work;
	struct completion *complete;
	struct cred *cred;
	char *path;
	char **argv;
	char **envp;
	enum umh_wait wait;
	int retval;
	struct file *stdin;
	void (*cleanup)(char **argv, char **envp);
};


static int ____call_usermodehelper(void *data)
{
	struct subprocess_info *sub_info = data;
	int retval;

	BUG_ON(atomic_read(&sub_info->cred->usage) != 1);

	
	spin_lock_irq(&current->sighand->siglock);
	flush_signal_handlers(current, 1);
	sigemptyset(&current->blocked);
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	
	commit_creds(sub_info->cred);
	sub_info->cred = NULL;

	
	if (sub_info->stdin) {
		struct files_struct *f = current->files;
		struct fdtable *fdt;
		
		sys_close(0);
		fd_install(0, sub_info->stdin);
		spin_lock(&f->file_lock);
		fdt = files_fdtable(f);
		FD_SET(0, fdt->open_fds);
		FD_CLR(0, fdt->close_on_exec);
		spin_unlock(&f->file_lock);

		
		current->signal->rlim[RLIMIT_CORE] = (struct rlimit){0, 0};
	}

	
	set_cpus_allowed_ptr(current, cpu_all_mask);

	
	set_user_nice(current, 0);

	retval = kernel_execve(sub_info->path, sub_info->argv, sub_info->envp);

	
	sub_info->retval = retval;
	do_exit(0);
}

void call_usermodehelper_freeinfo(struct subprocess_info *info)
{
	if (info->cleanup)
		(*info->cleanup)(info->argv, info->envp);
	if (info->cred)
		put_cred(info->cred);
	kfree(info);
}
EXPORT_SYMBOL(call_usermodehelper_freeinfo);


static int wait_for_helper(void *data)
{
	struct subprocess_info *sub_info = data;
	pid_t pid;

	
	allow_signal(SIGCHLD);

	pid = kernel_thread(____call_usermodehelper, sub_info, SIGCHLD);
	if (pid < 0) {
		sub_info->retval = pid;
	} else {
		int ret;

		
		sys_wait4(pid, (int __user *)&ret, 0, NULL);

		
		if (ret)
			sub_info->retval = ret;
	}

	if (sub_info->wait == UMH_NO_WAIT)
		call_usermodehelper_freeinfo(sub_info);
	else
		complete(sub_info->complete);
	return 0;
}


static void __call_usermodehelper(struct work_struct *work)
{
	struct subprocess_info *sub_info =
		container_of(work, struct subprocess_info, work);
	pid_t pid;
	enum umh_wait wait = sub_info->wait;

	BUG_ON(atomic_read(&sub_info->cred->usage) != 1);

	
	if (wait == UMH_WAIT_PROC || wait == UMH_NO_WAIT)
		pid = kernel_thread(wait_for_helper, sub_info,
				    CLONE_FS | CLONE_FILES | SIGCHLD);
	else
		pid = kernel_thread(____call_usermodehelper, sub_info,
				    CLONE_VFORK | SIGCHLD);

	switch (wait) {
	case UMH_NO_WAIT:
		break;

	case UMH_WAIT_PROC:
		if (pid > 0)
			break;
		sub_info->retval = pid;
		

	case UMH_WAIT_EXEC:
		complete(sub_info->complete);
	}
}

#ifdef CONFIG_PM_SLEEP

static int usermodehelper_disabled;


static atomic_t running_helpers = ATOMIC_INIT(0);


static DECLARE_WAIT_QUEUE_HEAD(running_helpers_waitq);


#define RUNNING_HELPERS_TIMEOUT	(5 * HZ)


int usermodehelper_disable(void)
{
	long retval;

	usermodehelper_disabled = 1;
	smp_mb();
	
	retval = wait_event_timeout(running_helpers_waitq,
					atomic_read(&running_helpers) == 0,
					RUNNING_HELPERS_TIMEOUT);
	if (retval)
		return 0;

	usermodehelper_disabled = 0;
	return -EAGAIN;
}


void usermodehelper_enable(void)
{
	usermodehelper_disabled = 0;
}

static void helper_lock(void)
{
	atomic_inc(&running_helpers);
	smp_mb__after_atomic_inc();
}

static void helper_unlock(void)
{
	if (atomic_dec_and_test(&running_helpers))
		wake_up(&running_helpers_waitq);
}
#else 
#define usermodehelper_disabled	0

static inline void helper_lock(void) {}
static inline void helper_unlock(void) {}
#endif 


struct subprocess_info *call_usermodehelper_setup(char *path, char **argv,
						  char **envp, gfp_t gfp_mask)
{
	struct subprocess_info *sub_info;
	sub_info = kzalloc(sizeof(struct subprocess_info), gfp_mask);
	if (!sub_info)
		goto out;

	INIT_WORK(&sub_info->work, __call_usermodehelper);
	sub_info->path = path;
	sub_info->argv = argv;
	sub_info->envp = envp;
	sub_info->cred = prepare_usermodehelper_creds();
	if (!sub_info->cred) {
		kfree(sub_info);
		return NULL;
	}

  out:
	return sub_info;
}
EXPORT_SYMBOL(call_usermodehelper_setup);


void call_usermodehelper_setkeys(struct subprocess_info *info,
				 struct key *session_keyring)
{
#ifdef CONFIG_KEYS
	struct thread_group_cred *tgcred = info->cred->tgcred;
	key_put(tgcred->session_keyring);
	tgcred->session_keyring = key_get(session_keyring);
#else
	BUG();
#endif
}
EXPORT_SYMBOL(call_usermodehelper_setkeys);


void call_usermodehelper_setcleanup(struct subprocess_info *info,
				    void (*cleanup)(char **argv, char **envp))
{
	info->cleanup = cleanup;
}
EXPORT_SYMBOL(call_usermodehelper_setcleanup);


int call_usermodehelper_stdinpipe(struct subprocess_info *sub_info,
				  struct file **filp)
{
	struct file *f;

	f = create_write_pipe(0);
	if (IS_ERR(f))
		return PTR_ERR(f);
	*filp = f;

	f = create_read_pipe(f, 0);
	if (IS_ERR(f)) {
		free_write_pipe(*filp);
		return PTR_ERR(f);
	}
	sub_info->stdin = f;

	return 0;
}
EXPORT_SYMBOL(call_usermodehelper_stdinpipe);


int call_usermodehelper_exec(struct subprocess_info *sub_info,
			     enum umh_wait wait)
{
	DECLARE_COMPLETION_ONSTACK(done);
	int retval = 0;

	BUG_ON(atomic_read(&sub_info->cred->usage) != 1);
	validate_creds(sub_info->cred);

	helper_lock();
	if (sub_info->path[0] == '\0')
		goto out;

	if (!khelper_wq || usermodehelper_disabled) {
		retval = -EBUSY;
		goto out;
	}

	sub_info->complete = &done;
	sub_info->wait = wait;

	queue_work(khelper_wq, &sub_info->work);
	if (wait == UMH_NO_WAIT)	
		goto unlock;
	wait_for_completion(&done);
	retval = sub_info->retval;

out:
	call_usermodehelper_freeinfo(sub_info);
unlock:
	helper_unlock();
	return retval;
}
EXPORT_SYMBOL(call_usermodehelper_exec);


int call_usermodehelper_pipe(char *path, char **argv, char **envp,
			     struct file **filp)
{
	struct subprocess_info *sub_info;
	int ret;

	sub_info = call_usermodehelper_setup(path, argv, envp, GFP_KERNEL);
	if (sub_info == NULL)
		return -ENOMEM;

	ret = call_usermodehelper_stdinpipe(sub_info, filp);
	if (ret < 0)
		goto out;

	return call_usermodehelper_exec(sub_info, UMH_WAIT_EXEC);

  out:
	call_usermodehelper_freeinfo(sub_info);
	return ret;
}
EXPORT_SYMBOL(call_usermodehelper_pipe);

void __init usermodehelper_init(void)
{
	khelper_wq = create_singlethread_workqueue("khelper");
	BUG_ON(!khelper_wq);
}
