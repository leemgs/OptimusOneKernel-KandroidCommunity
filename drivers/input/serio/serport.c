



#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/tty.h>

MODULE_AUTHOR("Vojtech Pavlik <vojtech@ucw.cz>");
MODULE_DESCRIPTION("Input device TTY line discipline");
MODULE_LICENSE("GPL");
MODULE_ALIAS_LDISC(N_MOUSE);

#define SERPORT_BUSY	1
#define SERPORT_ACTIVE	2
#define SERPORT_DEAD	3

struct serport {
	struct tty_struct *tty;
	wait_queue_head_t wait;
	struct serio *serio;
	struct serio_device_id id;
	spinlock_t lock;
	unsigned long flags;
};



static int serport_serio_write(struct serio *serio, unsigned char data)
{
	struct serport *serport = serio->port_data;
	return -(serport->tty->ops->write(serport->tty, &data, 1) != 1);
}

static int serport_serio_open(struct serio *serio)
{
	struct serport *serport = serio->port_data;
	unsigned long flags;

	spin_lock_irqsave(&serport->lock, flags);
	set_bit(SERPORT_ACTIVE, &serport->flags);
	spin_unlock_irqrestore(&serport->lock, flags);

	return 0;
}


static void serport_serio_close(struct serio *serio)
{
	struct serport *serport = serio->port_data;
	unsigned long flags;

	spin_lock_irqsave(&serport->lock, flags);
	clear_bit(SERPORT_ACTIVE, &serport->flags);
	set_bit(SERPORT_DEAD, &serport->flags);
	spin_unlock_irqrestore(&serport->lock, flags);

	wake_up_interruptible(&serport->wait);
}



static int serport_ldisc_open(struct tty_struct *tty)
{
	struct serport *serport;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	serport = kzalloc(sizeof(struct serport), GFP_KERNEL);
	if (!serport)
		return -ENOMEM;

	serport->tty = tty;
	spin_lock_init(&serport->lock);
	init_waitqueue_head(&serport->wait);

	tty->disc_data = serport;
	tty->receive_room = 256;
	set_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);

	return 0;
}



static void serport_ldisc_close(struct tty_struct *tty)
{
	struct serport *serport = (struct serport *) tty->disc_data;

	kfree(serport);
}



static void serport_ldisc_receive(struct tty_struct *tty, const unsigned char *cp, char *fp, int count)
{
	struct serport *serport = (struct serport*) tty->disc_data;
	unsigned long flags;
	int i;

	spin_lock_irqsave(&serport->lock, flags);

	if (!test_bit(SERPORT_ACTIVE, &serport->flags))
		goto out;

	for (i = 0; i < count; i++)
		serio_interrupt(serport->serio, cp[i], 0);

out:
	spin_unlock_irqrestore(&serport->lock, flags);
}



static ssize_t serport_ldisc_read(struct tty_struct * tty, struct file * file, unsigned char __user * buf, size_t nr)
{
	struct serport *serport = (struct serport*) tty->disc_data;
	struct serio *serio;
	char name[64];

	if (test_and_set_bit(SERPORT_BUSY, &serport->flags))
		return -EBUSY;

	serport->serio = serio = kzalloc(sizeof(struct serio), GFP_KERNEL);
	if (!serio)
		return -ENOMEM;

	strlcpy(serio->name, "Serial port", sizeof(serio->name));
	snprintf(serio->phys, sizeof(serio->phys), "%s/serio0", tty_name(tty, name));
	serio->id = serport->id;
	serio->id.type = SERIO_RS232;
	serio->write = serport_serio_write;
	serio->open = serport_serio_open;
	serio->close = serport_serio_close;
	serio->port_data = serport;

	serio_register_port(serport->serio);
	printk(KERN_INFO "serio: Serial port %s\n", tty_name(tty, name));

	wait_event_interruptible(serport->wait, test_bit(SERPORT_DEAD, &serport->flags));
	serio_unregister_port(serport->serio);
	serport->serio = NULL;

	clear_bit(SERPORT_DEAD, &serport->flags);
	clear_bit(SERPORT_BUSY, &serport->flags);

	return 0;
}



static int serport_ldisc_ioctl(struct tty_struct * tty, struct file * file, unsigned int cmd, unsigned long arg)
{
	struct serport *serport = (struct serport*) tty->disc_data;
	unsigned long type;

	if (cmd == SPIOCSTYPE) {
		if (get_user(type, (unsigned long __user *) arg))
			return -EFAULT;

		serport->id.proto = type & 0x000000ff;
		serport->id.id	  = (type & 0x0000ff00) >> 8;
		serport->id.extra = (type & 0x00ff0000) >> 16;

		return 0;
	}

	return -EINVAL;
}

static void serport_ldisc_write_wakeup(struct tty_struct * tty)
{
	struct serport *serport = (struct serport *) tty->disc_data;
	unsigned long flags;

	spin_lock_irqsave(&serport->lock, flags);
	if (test_bit(SERPORT_ACTIVE, &serport->flags))
		serio_drv_write_wakeup(serport->serio);
	spin_unlock_irqrestore(&serport->lock, flags);
}



static struct tty_ldisc_ops serport_ldisc = {
	.owner =	THIS_MODULE,
	.name =		"input",
	.open =		serport_ldisc_open,
	.close =	serport_ldisc_close,
	.read =		serport_ldisc_read,
	.ioctl =	serport_ldisc_ioctl,
	.receive_buf =	serport_ldisc_receive,
	.write_wakeup =	serport_ldisc_write_wakeup
};



static int __init serport_init(void)
{
	int retval;
	retval = tty_register_ldisc(N_MOUSE, &serport_ldisc);
	if (retval)
		printk(KERN_ERR "serport.c: Error registering line discipline.\n");

	return  retval;
}

static void __exit serport_exit(void)
{
	tty_unregister_ldisc(N_MOUSE);
}

module_init(serport_init);
module_exit(serport_exit);
