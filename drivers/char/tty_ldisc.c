#include <linux/types.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/fcntl.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/devpts_fs.h>
#include <linux/file.h>
#include <linux/console.h>
#include <linux/timer.h>
#include <linux/ctype.h>
#include <linux/kd.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/seq_file.h>

#include <linux/uaccess.h>
#include <asm/system.h>

#include <linux/kbd_kern.h>
#include <linux/vt_kern.h>
#include <linux/selection.h>

#include <linux/kmod.h>
#include <linux/nsproxy.h>



static DEFINE_SPINLOCK(tty_ldisc_lock);
static DECLARE_WAIT_QUEUE_HEAD(tty_ldisc_wait);

static struct tty_ldisc_ops *tty_ldiscs[NR_LDISCS];

static inline struct tty_ldisc *get_ldisc(struct tty_ldisc *ld)
{
	if (ld)
		atomic_inc(&ld->users);
	return ld;
}

static void put_ldisc(struct tty_ldisc *ld)
{
	unsigned long flags;

	if (WARN_ON_ONCE(!ld))
		return;

	
	local_irq_save(flags);
	if (atomic_dec_and_lock(&ld->users, &tty_ldisc_lock)) {
		struct tty_ldisc_ops *ldo = ld->ops;

		ldo->refcount--;
		module_put(ldo->owner);
		spin_unlock_irqrestore(&tty_ldisc_lock, flags);

		kfree(ld);
		return;
	}
	local_irq_restore(flags);
}



int tty_register_ldisc(int disc, struct tty_ldisc_ops *new_ldisc)
{
	unsigned long flags;
	int ret = 0;

	if (disc < N_TTY || disc >= NR_LDISCS)
		return -EINVAL;

	spin_lock_irqsave(&tty_ldisc_lock, flags);
	tty_ldiscs[disc] = new_ldisc;
	new_ldisc->num = disc;
	new_ldisc->refcount = 0;
	spin_unlock_irqrestore(&tty_ldisc_lock, flags);

	return ret;
}
EXPORT_SYMBOL(tty_register_ldisc);



int tty_unregister_ldisc(int disc)
{
	unsigned long flags;
	int ret = 0;

	if (disc < N_TTY || disc >= NR_LDISCS)
		return -EINVAL;

	spin_lock_irqsave(&tty_ldisc_lock, flags);
	if (tty_ldiscs[disc]->refcount)
		ret = -EBUSY;
	else
		tty_ldiscs[disc] = NULL;
	spin_unlock_irqrestore(&tty_ldisc_lock, flags);

	return ret;
}
EXPORT_SYMBOL(tty_unregister_ldisc);

static struct tty_ldisc_ops *get_ldops(int disc)
{
	unsigned long flags;
	struct tty_ldisc_ops *ldops, *ret;

	spin_lock_irqsave(&tty_ldisc_lock, flags);
	ret = ERR_PTR(-EINVAL);
	ldops = tty_ldiscs[disc];
	if (ldops) {
		ret = ERR_PTR(-EAGAIN);
		if (try_module_get(ldops->owner)) {
			ldops->refcount++;
			ret = ldops;
		}
	}
	spin_unlock_irqrestore(&tty_ldisc_lock, flags);
	return ret;
}

static void put_ldops(struct tty_ldisc_ops *ldops)
{
	unsigned long flags;

	spin_lock_irqsave(&tty_ldisc_lock, flags);
	ldops->refcount--;
	module_put(ldops->owner);
	spin_unlock_irqrestore(&tty_ldisc_lock, flags);
}



static struct tty_ldisc *tty_ldisc_get(int disc)
{
	struct tty_ldisc *ld;
	struct tty_ldisc_ops *ldops;

	if (disc < N_TTY || disc >= NR_LDISCS)
		return ERR_PTR(-EINVAL);

	
	ldops = get_ldops(disc);
	if (IS_ERR(ldops)) {
		request_module("tty-ldisc-%d", disc);
		ldops = get_ldops(disc);
		if (IS_ERR(ldops))
			return ERR_CAST(ldops);
	}

	ld = kmalloc(sizeof(struct tty_ldisc), GFP_KERNEL);
	if (ld == NULL) {
		put_ldops(ldops);
		return ERR_PTR(-ENOMEM);
	}

	ld->ops = ldops;
	atomic_set(&ld->users, 1);
	return ld;
}

static void *tty_ldiscs_seq_start(struct seq_file *m, loff_t *pos)
{
	return (*pos < NR_LDISCS) ? pos : NULL;
}

static void *tty_ldiscs_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	(*pos)++;
	return (*pos < NR_LDISCS) ? pos : NULL;
}

static void tty_ldiscs_seq_stop(struct seq_file *m, void *v)
{
}

static int tty_ldiscs_seq_show(struct seq_file *m, void *v)
{
	int i = *(loff_t *)v;
	struct tty_ldisc_ops *ldops;

	ldops = get_ldops(i);
	if (IS_ERR(ldops))
		return 0;
	seq_printf(m, "%-10s %2d\n", ldops->name ? ldops->name : "???", i);
	put_ldops(ldops);
	return 0;
}

static const struct seq_operations tty_ldiscs_seq_ops = {
	.start	= tty_ldiscs_seq_start,
	.next	= tty_ldiscs_seq_next,
	.stop	= tty_ldiscs_seq_stop,
	.show	= tty_ldiscs_seq_show,
};

static int proc_tty_ldiscs_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &tty_ldiscs_seq_ops);
}

const struct file_operations tty_ldiscs_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= proc_tty_ldiscs_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};



static void tty_ldisc_assign(struct tty_struct *tty, struct tty_ldisc *ld)
{
	tty->ldisc = ld;
}



static struct tty_ldisc *tty_ldisc_try(struct tty_struct *tty)
{
	unsigned long flags;
	struct tty_ldisc *ld;

	spin_lock_irqsave(&tty_ldisc_lock, flags);
	ld = NULL;
	if (test_bit(TTY_LDISC, &tty->flags))
		ld = get_ldisc(tty->ldisc);
	spin_unlock_irqrestore(&tty_ldisc_lock, flags);
	return ld;
}



struct tty_ldisc *tty_ldisc_ref_wait(struct tty_struct *tty)
{
	struct tty_ldisc *ld;

	
	wait_event(tty_ldisc_wait, (ld = tty_ldisc_try(tty)) != NULL);
	return ld;
}
EXPORT_SYMBOL_GPL(tty_ldisc_ref_wait);



struct tty_ldisc *tty_ldisc_ref(struct tty_struct *tty)
{
	return tty_ldisc_try(tty);
}
EXPORT_SYMBOL_GPL(tty_ldisc_ref);



void tty_ldisc_deref(struct tty_ldisc *ld)
{
	put_ldisc(ld);
}
EXPORT_SYMBOL_GPL(tty_ldisc_deref);

static inline void tty_ldisc_put(struct tty_ldisc *ld)
{
	put_ldisc(ld);
}



void tty_ldisc_enable(struct tty_struct *tty)
{
	set_bit(TTY_LDISC, &tty->flags);
	clear_bit(TTY_LDISC_CHANGING, &tty->flags);
	wake_up(&tty_ldisc_wait);
}



void tty_ldisc_flush(struct tty_struct *tty)
{
	struct tty_ldisc *ld = tty_ldisc_ref(tty);
	if (ld) {
		if (ld->ops->flush_buffer)
			ld->ops->flush_buffer(tty);
		tty_ldisc_deref(ld);
	}
	tty_buffer_flush(tty);
}
EXPORT_SYMBOL_GPL(tty_ldisc_flush);



static void tty_set_termios_ldisc(struct tty_struct *tty, int num)
{
	mutex_lock(&tty->termios_mutex);
	tty->termios->c_line = num;
	mutex_unlock(&tty->termios_mutex);
}



static int tty_ldisc_open(struct tty_struct *tty, struct tty_ldisc *ld)
{
	WARN_ON(test_and_set_bit(TTY_LDISC_OPEN, &tty->flags));
	if (ld->ops->open)
		return ld->ops->open(tty);
	return 0;
}



static void tty_ldisc_close(struct tty_struct *tty, struct tty_ldisc *ld)
{
	WARN_ON(!test_bit(TTY_LDISC_OPEN, &tty->flags));
	clear_bit(TTY_LDISC_OPEN, &tty->flags);
	if (ld->ops->close)
		ld->ops->close(tty);
}



static void tty_ldisc_restore(struct tty_struct *tty, struct tty_ldisc *old)
{
	char buf[64];
	struct tty_ldisc *new_ldisc;
	int r;

	
	old = tty_ldisc_get(old->ops->num);
	WARN_ON(IS_ERR(old));
	tty_ldisc_assign(tty, old);
	tty_set_termios_ldisc(tty, old->ops->num);
	if (tty_ldisc_open(tty, old) < 0) {
		tty_ldisc_put(old);
		
		new_ldisc = tty_ldisc_get(N_TTY);
		if (IS_ERR(new_ldisc))
			panic("n_tty: get");
		tty_ldisc_assign(tty, new_ldisc);
		tty_set_termios_ldisc(tty, N_TTY);
		r = tty_ldisc_open(tty, new_ldisc);
		if (r < 0)
			panic("Couldn't open N_TTY ldisc for "
			      "%s --- error %d.",
			      tty_name(tty, buf), r);
	}
}



static int tty_ldisc_halt(struct tty_struct *tty)
{
	clear_bit(TTY_LDISC, &tty->flags);
	return cancel_delayed_work_sync(&tty->buf.work);
}



int tty_set_ldisc(struct tty_struct *tty, int ldisc)
{
	int retval;
	struct tty_ldisc *o_ldisc, *new_ldisc;
	int work, o_work = 0;
	struct tty_struct *o_tty;

	new_ldisc = tty_ldisc_get(ldisc);
	if (IS_ERR(new_ldisc))
		return PTR_ERR(new_ldisc);

	

	o_tty = tty->link;	


	

	if (tty->ldisc->ops->num == ldisc) {
		tty_ldisc_put(new_ldisc);
		return 0;
	}

	

	tty_wait_until_sent(tty, 0);

	mutex_lock(&tty->ldisc_mutex);

	

	while (test_bit(TTY_LDISC_CHANGING, &tty->flags)) {
		mutex_unlock(&tty->ldisc_mutex);
		wait_event(tty_ldisc_wait,
			test_bit(TTY_LDISC_CHANGING, &tty->flags) == 0);
		mutex_lock(&tty->ldisc_mutex);
	}
	set_bit(TTY_LDISC_CHANGING, &tty->flags);

	

	tty->receive_room = 0;

	o_ldisc = tty->ldisc;
	

	work = tty_ldisc_halt(tty);
	if (o_tty)
		o_work = tty_ldisc_halt(o_tty);

	

	mutex_unlock(&tty->ldisc_mutex);

	flush_scheduled_work();

	mutex_lock(&tty->ldisc_mutex);
	if (test_bit(TTY_HUPPED, &tty->flags)) {
		
		clear_bit(TTY_LDISC_CHANGING, &tty->flags);
		mutex_unlock(&tty->ldisc_mutex);
		tty_ldisc_put(new_ldisc);
		return -EIO;
	}

	
	tty_ldisc_close(tty, o_ldisc);

	
	tty_ldisc_assign(tty, new_ldisc);
	tty_set_termios_ldisc(tty, ldisc);

	retval = tty_ldisc_open(tty, new_ldisc);
	if (retval < 0) {
		
		tty_ldisc_put(new_ldisc);
		tty_ldisc_restore(tty, o_ldisc);
	}

	

	if (tty->ldisc->ops->num != o_ldisc->ops->num && tty->ops->set_ldisc)
		tty->ops->set_ldisc(tty);

	tty_ldisc_put(o_ldisc);

	

	tty_ldisc_enable(tty);
	if (o_tty)
		tty_ldisc_enable(o_tty);

	
	if (work)
		schedule_delayed_work(&tty->buf.work, 1);
	if (o_work)
		schedule_delayed_work(&o_tty->buf.work, 1);
	mutex_unlock(&tty->ldisc_mutex);
	return retval;
}



static void tty_reset_termios(struct tty_struct *tty)
{
	mutex_lock(&tty->termios_mutex);
	*tty->termios = tty->driver->init_termios;
	tty->termios->c_ispeed = tty_termios_input_baud_rate(tty->termios);
	tty->termios->c_ospeed = tty_termios_baud_rate(tty->termios);
	mutex_unlock(&tty->termios_mutex);
}




static void tty_ldisc_reinit(struct tty_struct *tty)
{
	struct tty_ldisc *ld;

	tty_ldisc_close(tty, tty->ldisc);
	tty_ldisc_put(tty->ldisc);
	tty->ldisc = NULL;
	
	ld = tty_ldisc_get(N_TTY);
	BUG_ON(IS_ERR(ld));
	tty_ldisc_assign(tty, ld);
	tty_set_termios_ldisc(tty, N_TTY);
}



void tty_ldisc_hangup(struct tty_struct *tty)
{
	struct tty_ldisc *ld;

	
	ld = tty_ldisc_ref(tty);
	if (ld != NULL) {
		
		if (ld->ops->flush_buffer)
			ld->ops->flush_buffer(tty);
		tty_driver_flush_buffer(tty);
		if ((test_bit(TTY_DO_WRITE_WAKEUP, &tty->flags)) &&
		    ld->ops->write_wakeup)
			ld->ops->write_wakeup(tty);
		if (ld->ops->hangup)
			ld->ops->hangup(tty);
		tty_ldisc_deref(ld);
	}
	
	wake_up_interruptible_poll(&tty->write_wait, POLLOUT);
	wake_up_interruptible_poll(&tty->read_wait, POLLIN);
	
	if (tty->driver->flags & TTY_DRIVER_RESET_TERMIOS) {
		
		mutex_lock(&tty->ldisc_mutex);
		tty_ldisc_halt(tty);
		if (tty->ldisc) {	
			
			tty_ldisc_reinit(tty);
			
			WARN_ON(tty_ldisc_open(tty, tty->ldisc));
			tty_ldisc_enable(tty);
		}
		mutex_unlock(&tty->ldisc_mutex);
		tty_reset_termios(tty);
	}
}



int tty_ldisc_setup(struct tty_struct *tty, struct tty_struct *o_tty)
{
	struct tty_ldisc *ld = tty->ldisc;
	int retval;

	retval = tty_ldisc_open(tty, ld);
	if (retval)
		return retval;

	if (o_tty) {
		retval = tty_ldisc_open(o_tty, o_tty->ldisc);
		if (retval) {
			tty_ldisc_close(tty, ld);
			return retval;
		}
		tty_ldisc_enable(o_tty);
	}
	tty_ldisc_enable(tty);
	return 0;
}


void tty_ldisc_release(struct tty_struct *tty, struct tty_struct *o_tty)
{
	

	tty_ldisc_halt(tty);
	flush_scheduled_work();

	mutex_lock(&tty->ldisc_mutex);
	
	tty_ldisc_close(tty, tty->ldisc);
	tty_ldisc_put(tty->ldisc);
	
	tty->ldisc = NULL;

	
	tty_set_termios_ldisc(tty, N_TTY);
	mutex_unlock(&tty->ldisc_mutex);

	
	if (o_tty)
		tty_ldisc_release(o_tty, NULL);

	
}



void tty_ldisc_init(struct tty_struct *tty)
{
	struct tty_ldisc *ld = tty_ldisc_get(N_TTY);
	if (IS_ERR(ld))
		panic("n_tty: init_tty");
	tty_ldisc_assign(tty, ld);
}

void tty_ldisc_begin(void)
{
	
	(void) tty_register_ldisc(N_TTY, &tty_ldisc_N_TTY);
}
