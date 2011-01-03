

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <asm/system.h>
#include <asm/irq.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/ss.h>
#include <pcmcia/cs.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/cisreg.h>
#include <pcmcia/ds.h>
#include "cs_internal.h"




MODULE_AUTHOR("David Hinds <dahinds@users.sourceforge.net>");
MODULE_DESCRIPTION("Linux Kernel Card Services");
MODULE_LICENSE("GPL");

#define INT_MODULE_PARM(n, v) static int n = v; module_param(n, int, 0444)

INT_MODULE_PARM(setup_delay,	10);		
INT_MODULE_PARM(resume_delay,	20);		
INT_MODULE_PARM(shutdown_delay,	3);		
INT_MODULE_PARM(vcc_settle,	40);		
INT_MODULE_PARM(reset_time,	10);		
INT_MODULE_PARM(unreset_delay,	10);		
INT_MODULE_PARM(unreset_check,	10);		
INT_MODULE_PARM(unreset_limit,	30);		


INT_MODULE_PARM(cis_speed,	300);		

#ifdef CONFIG_PCMCIA_DEBUG
static int pc_debug;

module_param(pc_debug, int, 0644);

int cs_debug_level(int level)
{
	return pc_debug > level;
}
#endif


socket_state_t dead_socket = {
	.csc_mask	= SS_DETECT,
};
EXPORT_SYMBOL(dead_socket);



LIST_HEAD(pcmcia_socket_list);
EXPORT_SYMBOL(pcmcia_socket_list);

DECLARE_RWSEM(pcmcia_socket_list_rwsem);
EXPORT_SYMBOL(pcmcia_socket_list_rwsem);



static int socket_early_resume(struct pcmcia_socket *skt);
static int socket_late_resume(struct pcmcia_socket *skt);
static int socket_resume(struct pcmcia_socket *skt);
static int socket_suspend(struct pcmcia_socket *skt);

static void pcmcia_socket_dev_run(struct device *dev,
				  int (*cb)(struct pcmcia_socket *))
{
	struct pcmcia_socket *socket;

	down_read(&pcmcia_socket_list_rwsem);
	list_for_each_entry(socket, &pcmcia_socket_list, socket_list) {
		if (socket->dev.parent != dev)
			continue;
		mutex_lock(&socket->skt_mutex);
		cb(socket);
		mutex_unlock(&socket->skt_mutex);
	}
	up_read(&pcmcia_socket_list_rwsem);
}

int pcmcia_socket_dev_suspend(struct device *dev)
{
	pcmcia_socket_dev_run(dev, socket_suspend);
	return 0;
}
EXPORT_SYMBOL(pcmcia_socket_dev_suspend);

void pcmcia_socket_dev_early_resume(struct device *dev)
{
	pcmcia_socket_dev_run(dev, socket_early_resume);
}
EXPORT_SYMBOL(pcmcia_socket_dev_early_resume);

void pcmcia_socket_dev_late_resume(struct device *dev)
{
	pcmcia_socket_dev_run(dev, socket_late_resume);
}
EXPORT_SYMBOL(pcmcia_socket_dev_late_resume);

int pcmcia_socket_dev_resume(struct device *dev)
{
	pcmcia_socket_dev_run(dev, socket_resume);
	return 0;
}
EXPORT_SYMBOL(pcmcia_socket_dev_resume);


struct pcmcia_socket * pcmcia_get_socket(struct pcmcia_socket *skt)
{
	struct device *dev = get_device(&skt->dev);
	if (!dev)
		return NULL;
	skt = dev_get_drvdata(dev);
	if (!try_module_get(skt->owner)) {
		put_device(&skt->dev);
		return NULL;
	}
	return (skt);
}
EXPORT_SYMBOL(pcmcia_get_socket);


void pcmcia_put_socket(struct pcmcia_socket *skt)
{
	module_put(skt->owner);
	put_device(&skt->dev);
}
EXPORT_SYMBOL(pcmcia_put_socket);


static void pcmcia_release_socket(struct device *dev)
{
	struct pcmcia_socket *socket = dev_get_drvdata(dev);

	complete(&socket->socket_released);
}

static int pccardd(void *__skt);


int pcmcia_register_socket(struct pcmcia_socket *socket)
{
	struct task_struct *tsk;
	int ret;

	if (!socket || !socket->ops || !socket->dev.parent || !socket->resource_ops)
		return -EINVAL;

	cs_dbg(socket, 0, "pcmcia_register_socket(0x%p)\n", socket->ops);

	spin_lock_init(&socket->lock);

	
	down_write(&pcmcia_socket_list_rwsem);
	if (list_empty(&pcmcia_socket_list))
		socket->sock = 0;
	else {
		unsigned int found, i = 1;
		struct pcmcia_socket *tmp;
		do {
			found = 1;
			list_for_each_entry(tmp, &pcmcia_socket_list, socket_list) {
				if (tmp->sock == i)
					found = 0;
			}
			i++;
		} while (!found);
		socket->sock = i - 1;
	}
	list_add_tail(&socket->socket_list, &pcmcia_socket_list);
	up_write(&pcmcia_socket_list_rwsem);

#ifndef CONFIG_CARDBUS
	
	socket->features &= ~SS_CAP_CARDBUS;
#endif

	
	dev_set_drvdata(&socket->dev, socket);
	socket->dev.class = &pcmcia_socket_class;
	dev_set_name(&socket->dev, "pcmcia_socket%u", socket->sock);

	
	socket->cis_mem.flags = 0;
	socket->cis_mem.speed = cis_speed;

	INIT_LIST_HEAD(&socket->cis_cache);

	init_completion(&socket->socket_released);
	init_completion(&socket->thread_done);
	mutex_init(&socket->skt_mutex);
	spin_lock_init(&socket->thread_lock);

	if (socket->resource_ops->init) {
		ret = socket->resource_ops->init(socket);
		if (ret)
			goto err;
	}

	tsk = kthread_run(pccardd, socket, "pccardd");
	if (IS_ERR(tsk)) {
		ret = PTR_ERR(tsk);
		goto err;
	}

	wait_for_completion(&socket->thread_done);
	if (!socket->thread) {
		dev_printk(KERN_WARNING, &socket->dev,
			   "PCMCIA: warning: socket thread did not start\n");
		return -EIO;
	}

	pcmcia_parse_events(socket, SS_DETECT);

	return 0;

 err:
	down_write(&pcmcia_socket_list_rwsem);
	list_del(&socket->socket_list);
	up_write(&pcmcia_socket_list_rwsem);
	return ret;
} 
EXPORT_SYMBOL(pcmcia_register_socket);



void pcmcia_unregister_socket(struct pcmcia_socket *socket)
{
	if (!socket)
		return;

	cs_dbg(socket, 0, "pcmcia_unregister_socket(0x%p)\n", socket->ops);

	if (socket->thread)
		kthread_stop(socket->thread);

	release_cis_mem(socket);

	
	down_write(&pcmcia_socket_list_rwsem);
	list_del(&socket->socket_list);
	up_write(&pcmcia_socket_list_rwsem);

	
	release_resource_db(socket);
	wait_for_completion(&socket->socket_released);
} 
EXPORT_SYMBOL(pcmcia_unregister_socket);


struct pcmcia_socket * pcmcia_get_socket_by_nr(unsigned int nr)
{
	struct pcmcia_socket *s;

	down_read(&pcmcia_socket_list_rwsem);
	list_for_each_entry(s, &pcmcia_socket_list, socket_list)
		if (s->sock == nr) {
			up_read(&pcmcia_socket_list_rwsem);
			return s;
		}
	up_read(&pcmcia_socket_list_rwsem);

	return NULL;

}
EXPORT_SYMBOL(pcmcia_get_socket_by_nr);





static int send_event(struct pcmcia_socket *s, event_t event, int priority)
{
	int ret;

	if (s->state & SOCKET_CARDBUS)
		return 0;

	cs_dbg(s, 1, "send_event(event %d, pri %d, callback 0x%p)\n",
	   event, priority, s->callback);

	if (!s->callback)
		return 0;
	if (!try_module_get(s->callback->owner))
		return 0;

	ret = s->callback->event(s, event, priority);

	module_put(s->callback->owner);

	return ret;
}

static void socket_remove_drivers(struct pcmcia_socket *skt)
{
	cs_dbg(skt, 4, "remove_drivers\n");

	send_event(skt, CS_EVENT_CARD_REMOVAL, CS_EVENT_PRI_HIGH);
}

static int socket_reset(struct pcmcia_socket *skt)
{
	int status, i;

	cs_dbg(skt, 4, "reset\n");

	skt->socket.flags |= SS_OUTPUT_ENA | SS_RESET;
	skt->ops->set_socket(skt, &skt->socket);
	udelay((long)reset_time);

	skt->socket.flags &= ~SS_RESET;
	skt->ops->set_socket(skt, &skt->socket);

	msleep(unreset_delay * 10);
	for (i = 0; i < unreset_limit; i++) {
		skt->ops->get_status(skt, &status);

		if (!(status & SS_DETECT))
			return -ENODEV;

		if (status & SS_READY)
			return 0;

		msleep(unreset_check * 10);
	}

	cs_err(skt, "time out after reset.\n");
	return -ETIMEDOUT;
}


static void socket_shutdown(struct pcmcia_socket *s)
{
	int status;

	cs_dbg(s, 4, "shutdown\n");

	socket_remove_drivers(s);
	s->state &= SOCKET_INUSE | SOCKET_PRESENT;
	msleep(shutdown_delay * 10);
	s->state &= SOCKET_INUSE;

	
	s->socket = dead_socket;
	s->ops->init(s);
	s->ops->set_socket(s, &s->socket);
	s->irq.AssignedIRQ = s->irq.Config = 0;
	s->lock_count = 0;
	destroy_cis_cache(s);
#ifdef CONFIG_CARDBUS
	cb_free(s);
#endif
	s->functions = 0;

	
	msleep(100);

	s->ops->get_status(s, &status);
	if (status & SS_POWERON) {
		dev_printk(KERN_ERR, &s->dev,
			   "*** DANGER *** unable to remove socket power\n");
	}

	cs_socket_put(s);
}

static int socket_setup(struct pcmcia_socket *skt, int initial_delay)
{
	int status, i;

	cs_dbg(skt, 4, "setup\n");

	skt->ops->get_status(skt, &status);
	if (!(status & SS_DETECT))
		return -ENODEV;

	msleep(initial_delay * 10);

	for (i = 0; i < 100; i++) {
		skt->ops->get_status(skt, &status);
		if (!(status & SS_DETECT))
			return -ENODEV;

		if (!(status & SS_PENDING))
			break;

		msleep(100);
	}

	if (status & SS_PENDING) {
		cs_err(skt, "voltage interrogation timed out.\n");
		return -ETIMEDOUT;
	}

	if (status & SS_CARDBUS) {
		if (!(skt->features & SS_CAP_CARDBUS)) {
			cs_err(skt, "cardbus cards are not supported.\n");
			return -EINVAL;
		}
		skt->state |= SOCKET_CARDBUS;
	}

	
	if (status & SS_3VCARD)
		skt->socket.Vcc = skt->socket.Vpp = 33;
	else if (!(status & SS_XVCARD))
		skt->socket.Vcc = skt->socket.Vpp = 50;
	else {
		cs_err(skt, "unsupported voltage key.\n");
		return -EIO;
	}

	if (skt->power_hook)
		skt->power_hook(skt, HOOK_POWER_PRE);

	skt->socket.flags = 0;
	skt->ops->set_socket(skt, &skt->socket);

	
	msleep(vcc_settle * 10);

	skt->ops->get_status(skt, &status);
	if (!(status & SS_POWERON)) {
		cs_err(skt, "unable to apply power.\n");
		return -EIO;
	}

	status = socket_reset(skt);

	if (skt->power_hook)
		skt->power_hook(skt, HOOK_POWER_POST);

	return status;
}


static int socket_insert(struct pcmcia_socket *skt)
{
	int ret;

	cs_dbg(skt, 4, "insert\n");

	if (!cs_socket_get(skt))
		return -ENODEV;

	ret = socket_setup(skt, setup_delay);
	if (ret == 0) {
		skt->state |= SOCKET_PRESENT;

		dev_printk(KERN_NOTICE, &skt->dev,
			   "pccard: %s card inserted into slot %d\n",
			   (skt->state & SOCKET_CARDBUS) ? "CardBus" : "PCMCIA",
			   skt->sock);

#ifdef CONFIG_CARDBUS
		if (skt->state & SOCKET_CARDBUS) {
			cb_alloc(skt);
			skt->state |= SOCKET_CARDBUS_CONFIG;
		}
#endif
		cs_dbg(skt, 4, "insert done\n");

		send_event(skt, CS_EVENT_CARD_INSERTION, CS_EVENT_PRI_LOW);
	} else {
		socket_shutdown(skt);
	}

	return ret;
}

static int socket_suspend(struct pcmcia_socket *skt)
{
	if (skt->state & SOCKET_SUSPEND)
		return -EBUSY;

	send_event(skt, CS_EVENT_PM_SUSPEND, CS_EVENT_PRI_LOW);
	skt->socket = dead_socket;
	skt->ops->set_socket(skt, &skt->socket);
	if (skt->ops->suspend)
		skt->ops->suspend(skt);
	skt->state |= SOCKET_SUSPEND;

	return 0;
}

static int socket_early_resume(struct pcmcia_socket *skt)
{
	skt->socket = dead_socket;
	skt->ops->init(skt);
	skt->ops->set_socket(skt, &skt->socket);
	if (skt->state & SOCKET_PRESENT)
		skt->resume_status = socket_setup(skt, resume_delay);
	return 0;
}

static int socket_late_resume(struct pcmcia_socket *skt)
{
	if (!(skt->state & SOCKET_PRESENT)) {
		skt->state &= ~SOCKET_SUSPEND;
		return socket_insert(skt);
	}

	if (skt->resume_status == 0) {
		
		if (verify_cis_cache(skt) != 0) {
			cs_dbg(skt, 4, "cis mismatch - different card\n");
			socket_remove_drivers(skt);
			destroy_cis_cache(skt);
			
			msleep(200);
			send_event(skt, CS_EVENT_CARD_INSERTION, CS_EVENT_PRI_LOW);
		} else {
			cs_dbg(skt, 4, "cis matches cache\n");
			send_event(skt, CS_EVENT_PM_RESUME, CS_EVENT_PRI_LOW);
		}
	} else {
		socket_shutdown(skt);
	}

	skt->state &= ~SOCKET_SUSPEND;

	return 0;
}


static int socket_resume(struct pcmcia_socket *skt)
{
	if (!(skt->state & SOCKET_SUSPEND))
		return -EBUSY;

	socket_early_resume(skt);
	return socket_late_resume(skt);
}

static void socket_remove(struct pcmcia_socket *skt)
{
	dev_printk(KERN_NOTICE, &skt->dev,
		   "pccard: card ejected from slot %d\n", skt->sock);
	socket_shutdown(skt);
}


static void socket_detect_change(struct pcmcia_socket *skt)
{
	if (!(skt->state & SOCKET_SUSPEND)) {
		int status;

		if (!(skt->state & SOCKET_PRESENT))
			msleep(20);

		skt->ops->get_status(skt, &status);
		if ((skt->state & SOCKET_PRESENT) &&
		     !(status & SS_DETECT))
			socket_remove(skt);
		if (!(skt->state & SOCKET_PRESENT) &&
		    (status & SS_DETECT))
			socket_insert(skt);
	}
}

static int pccardd(void *__skt)
{
	struct pcmcia_socket *skt = __skt;
	int ret;

	skt->thread = current;
	skt->socket = dead_socket;
	skt->ops->init(skt);
	skt->ops->set_socket(skt, &skt->socket);

	
	ret = device_register(&skt->dev);
	if (ret) {
		dev_printk(KERN_WARNING, &skt->dev,
			   "PCMCIA: unable to register socket\n");
		skt->thread = NULL;
		complete(&skt->thread_done);
		return 0;
	}
	ret = pccard_sysfs_add_socket(&skt->dev);
	if (ret)
		dev_warn(&skt->dev, "err %d adding socket attributes\n", ret);

	complete(&skt->thread_done);

	set_freezable();
	for (;;) {
		unsigned long flags;
		unsigned int events;

		set_current_state(TASK_INTERRUPTIBLE);

		spin_lock_irqsave(&skt->thread_lock, flags);
		events = skt->thread_events;
		skt->thread_events = 0;
		spin_unlock_irqrestore(&skt->thread_lock, flags);

		if (events) {
			mutex_lock(&skt->skt_mutex);
			if (events & SS_DETECT)
				socket_detect_change(skt);
			if (events & SS_BATDEAD)
				send_event(skt, CS_EVENT_BATTERY_DEAD, CS_EVENT_PRI_LOW);
			if (events & SS_BATWARN)
				send_event(skt, CS_EVENT_BATTERY_LOW, CS_EVENT_PRI_LOW);
			if (events & SS_READY)
				send_event(skt, CS_EVENT_READY_CHANGE, CS_EVENT_PRI_LOW);
			mutex_unlock(&skt->skt_mutex);
			continue;
		}

		if (kthread_should_stop())
			break;

		schedule();
		try_to_freeze();
	}
	
	set_current_state(TASK_RUNNING);

	
	pccard_sysfs_remove_socket(&skt->dev);
	device_unregister(&skt->dev);

	return 0;
}


void pcmcia_parse_events(struct pcmcia_socket *s, u_int events)
{
	unsigned long flags;
	cs_dbg(s, 4, "parse_events: events %08x\n", events);
	if (s->thread) {
		spin_lock_irqsave(&s->thread_lock, flags);
		s->thread_events |= events;
		spin_unlock_irqrestore(&s->thread_lock, flags);

		wake_up_process(s->thread);
	}
} 
EXPORT_SYMBOL(pcmcia_parse_events);



int pccard_register_pcmcia(struct pcmcia_socket *s, struct pcmcia_callback *c)
{
        int ret = 0;

	
	mutex_lock(&s->skt_mutex);

	if (c) {
		
		if (s->callback) {
			ret = -EBUSY;
			goto err;
		}

		s->callback = c;

		if ((s->state & (SOCKET_PRESENT|SOCKET_CARDBUS)) == SOCKET_PRESENT)
			send_event(s, CS_EVENT_CARD_INSERTION, CS_EVENT_PRI_LOW);
	} else
		s->callback = NULL;
 err:
	mutex_unlock(&s->skt_mutex);

	return ret;
}
EXPORT_SYMBOL(pccard_register_pcmcia);




int pcmcia_reset_card(struct pcmcia_socket *skt)
{
	int ret;

	cs_dbg(skt, 1, "resetting socket\n");

	mutex_lock(&skt->skt_mutex);
	do {
		if (!(skt->state & SOCKET_PRESENT)) {
			ret = -ENODEV;
			break;
		}
		if (skt->state & SOCKET_SUSPEND) {
			ret = -EBUSY;
			break;
		}
		if (skt->state & SOCKET_CARDBUS) {
			ret = -EPERM;
			break;
		}

		ret = send_event(skt, CS_EVENT_RESET_REQUEST, CS_EVENT_PRI_LOW);
		if (ret == 0) {
			send_event(skt, CS_EVENT_RESET_PHYSICAL, CS_EVENT_PRI_LOW);
			if (skt->callback)
				skt->callback->suspend(skt);
			if (socket_reset(skt) == 0) {
				send_event(skt, CS_EVENT_CARD_RESET, CS_EVENT_PRI_LOW);
				if (skt->callback)
					skt->callback->resume(skt);
			}
		}

		ret = 0;
	} while (0);
	mutex_unlock(&skt->skt_mutex);

	return ret;
} 
EXPORT_SYMBOL(pcmcia_reset_card);



int pcmcia_suspend_card(struct pcmcia_socket *skt)
{
	int ret;

	cs_dbg(skt, 1, "suspending socket\n");

	mutex_lock(&skt->skt_mutex);
	do {
		if (!(skt->state & SOCKET_PRESENT)) {
			ret = -ENODEV;
			break;
		}
		if (skt->state & SOCKET_CARDBUS) {
			ret = -EPERM;
			break;
		}
		if (skt->callback) {
			ret = skt->callback->suspend(skt);
			if (ret)
				break;
		}
		ret = socket_suspend(skt);
	} while (0);
	mutex_unlock(&skt->skt_mutex);

	return ret;
} 
EXPORT_SYMBOL(pcmcia_suspend_card);


int pcmcia_resume_card(struct pcmcia_socket *skt)
{
	int ret;
    
	cs_dbg(skt, 1, "waking up socket\n");

	mutex_lock(&skt->skt_mutex);
	do {
		if (!(skt->state & SOCKET_PRESENT)) {
			ret = -ENODEV;
			break;
		}
		if (skt->state & SOCKET_CARDBUS) {
			ret = -EPERM;
			break;
		}
		ret = socket_resume(skt);
		if (!ret && skt->callback)
			skt->callback->resume(skt);
	} while (0);
	mutex_unlock(&skt->skt_mutex);

	return ret;
} 
EXPORT_SYMBOL(pcmcia_resume_card);



int pcmcia_eject_card(struct pcmcia_socket *skt)
{
	int ret;
    
	cs_dbg(skt, 1, "user eject request\n");

	mutex_lock(&skt->skt_mutex);
	do {
		if (!(skt->state & SOCKET_PRESENT)) {
			ret = -ENODEV;
			break;
		}

		ret = send_event(skt, CS_EVENT_EJECTION_REQUEST, CS_EVENT_PRI_LOW);
		if (ret != 0) {
			ret = -EINVAL;
			break;
		}

		socket_remove(skt);
		ret = 0;
	} while (0);
	mutex_unlock(&skt->skt_mutex);

	return ret;
} 
EXPORT_SYMBOL(pcmcia_eject_card);


int pcmcia_insert_card(struct pcmcia_socket *skt)
{
	int ret;

	cs_dbg(skt, 1, "user insert request\n");

	mutex_lock(&skt->skt_mutex);
	do {
		if (skt->state & SOCKET_PRESENT) {
			ret = -EBUSY;
			break;
		}
		if (socket_insert(skt) == -ENODEV) {
			ret = -ENODEV;
			break;
		}
		ret = 0;
	} while (0);
	mutex_unlock(&skt->skt_mutex);

	return ret;
} 
EXPORT_SYMBOL(pcmcia_insert_card);


static int pcmcia_socket_uevent(struct device *dev,
				struct kobj_uevent_env *env)
{
	struct pcmcia_socket *s = container_of(dev, struct pcmcia_socket, dev);

	if (add_uevent_var(env, "SOCKET_NO=%u", s->sock))
		return -ENOMEM;

	return 0;
}


static struct completion pcmcia_unload;

static void pcmcia_release_socket_class(struct class *data)
{
	complete(&pcmcia_unload);
}


struct class pcmcia_socket_class = {
	.name = "pcmcia_socket",
	.dev_uevent = pcmcia_socket_uevent,
	.dev_release = pcmcia_release_socket,
	.class_release = pcmcia_release_socket_class,
};
EXPORT_SYMBOL(pcmcia_socket_class);


static int __init init_pcmcia_cs(void)
{
	init_completion(&pcmcia_unload);
	return class_register(&pcmcia_socket_class);
}

static void __exit exit_pcmcia_cs(void)
{
	class_unregister(&pcmcia_socket_class);
	wait_for_completion(&pcmcia_unload);
}

subsys_initcall(init_pcmcia_cs);
module_exit(exit_pcmcia_cs);

