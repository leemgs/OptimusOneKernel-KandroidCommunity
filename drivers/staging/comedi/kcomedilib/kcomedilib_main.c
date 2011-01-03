

#define __NO_VERSION__
#include <linux/module.h>

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fcntl.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <asm/io.h>

#include "../comedi.h"
#include "../comedilib.h"
#include "../comedidev.h"

MODULE_AUTHOR("David Schleef <ds@schleef.org>");
MODULE_DESCRIPTION("Comedi kernel library");
MODULE_LICENSE("GPL");

void *comedi_open(const char *filename)
{
	struct comedi_device_file_info *dev_file_info;
	struct comedi_device *dev;
	unsigned int minor;

	if (strncmp(filename, "/dev/comedi", 11) != 0)
		return NULL;

	minor = simple_strtoul(filename + 11, NULL, 0);

	if (minor >= COMEDI_NUM_BOARD_MINORS)
		return NULL;

	dev_file_info = comedi_get_device_file_info(minor);
	if (dev_file_info == NULL)
		return NULL;
	dev = dev_file_info->device;

	if (dev == NULL || !dev->attached)
		return NULL;

	if (!try_module_get(dev->driver->module))
		return NULL;

	return (void *)dev;
}

void *comedi_open_old(unsigned int minor)
{
	struct comedi_device_file_info *dev_file_info;
	struct comedi_device *dev;

	if (minor >= COMEDI_NUM_MINORS)
		return NULL;

	dev_file_info = comedi_get_device_file_info(minor);
	if (dev_file_info == NULL)
		return NULL;
	dev = dev_file_info->device;

	if (dev == NULL || !dev->attached)
		return NULL;

	return (void *)dev;
}

int comedi_close(void *d)
{
	struct comedi_device *dev = (struct comedi_device *)d;

	module_put(dev->driver->module);

	return 0;
}

int comedi_loglevel(int newlevel)
{
	return 0;
}

void comedi_perror(const char *message)
{
	printk("%s: unknown error\n", message);
}

char *comedi_strerror(int err)
{
	return "unknown error";
}

int comedi_fileno(void *d)
{
	struct comedi_device *dev = (struct comedi_device *)d;

	
	return dev->minor;
}

int comedi_command(void *d, struct comedi_cmd *cmd)
{
	struct comedi_device *dev = (struct comedi_device *)d;
	struct comedi_subdevice *s;
	struct comedi_async *async;
	unsigned runflags;

	if (cmd->subdev >= dev->n_subdevices)
		return -ENODEV;

	s = dev->subdevices + cmd->subdev;
	if (s->type == COMEDI_SUBD_UNUSED)
		return -EIO;

	async = s->async;
	if (async == NULL)
		return -ENODEV;

	if (s->busy)
		return -EBUSY;
	s->busy = d;

	if (async->cb_mask & COMEDI_CB_EOS)
		cmd->flags |= TRIG_WAKE_EOS;

	async->cmd = *cmd;

	runflags = SRF_RUNNING;

	comedi_set_subdevice_runflags(s, ~0, runflags);

	comedi_reset_async_buf(async);

	return s->do_cmd(dev, s);
}

int comedi_command_test(void *d, struct comedi_cmd *cmd)
{
	struct comedi_device *dev = (struct comedi_device *)d;
	struct comedi_subdevice *s;

	if (cmd->subdev >= dev->n_subdevices)
		return -ENODEV;

	s = dev->subdevices + cmd->subdev;
	if (s->type == COMEDI_SUBD_UNUSED)
		return -EIO;

	if (s->async == NULL)
		return -ENODEV;

	return s->do_cmdtest(dev, s, cmd);
}


int comedi_do_insn(void *d, struct comedi_insn *insn)
{
	struct comedi_device *dev = (struct comedi_device *)d;
	struct comedi_subdevice *s;
	int ret = 0;

	if (insn->insn & INSN_MASK_SPECIAL) {
		switch (insn->insn) {
		case INSN_GTOD:
			{
				struct timeval tv;

				do_gettimeofday(&tv);
				insn->data[0] = tv.tv_sec;
				insn->data[1] = tv.tv_usec;
				ret = 2;

				break;
			}
		case INSN_WAIT:
			
			if (insn->n != 1 || insn->data[0] >= 100) {
				ret = -EINVAL;
				break;
			}
			udelay(insn->data[0]);
			ret = 1;
			break;
		case INSN_INTTRIG:
			if (insn->n != 1) {
				ret = -EINVAL;
				break;
			}
			if (insn->subdev >= dev->n_subdevices) {
				printk("%d not usable subdevice\n",
				       insn->subdev);
				ret = -EINVAL;
				break;
			}
			s = dev->subdevices + insn->subdev;
			if (!s->async) {
				printk("no async\n");
				ret = -EINVAL;
				break;
			}
			if (!s->async->inttrig) {
				printk("no inttrig\n");
				ret = -EAGAIN;
				break;
			}
			ret = s->async->inttrig(dev, s, insn->data[0]);
			if (ret >= 0)
				ret = 1;
			break;
		default:
			ret = -EINVAL;
		}
	} else {
		
		if (insn->subdev >= dev->n_subdevices) {
			ret = -EINVAL;
			goto error;
		}
		s = dev->subdevices + insn->subdev;

		if (s->type == COMEDI_SUBD_UNUSED) {
			printk("%d not useable subdevice\n", insn->subdev);
			ret = -EIO;
			goto error;
		}

		

		ret = check_chanlist(s, 1, &insn->chanspec);
		if (ret < 0) {
			printk("bad chanspec\n");
			ret = -EINVAL;
			goto error;
		}

		if (s->busy) {
			ret = -EBUSY;
			goto error;
		}
		s->busy = d;

		switch (insn->insn) {
		case INSN_READ:
			ret = s->insn_read(dev, s, insn, insn->data);
			break;
		case INSN_WRITE:
			ret = s->insn_write(dev, s, insn, insn->data);
			break;
		case INSN_BITS:
			ret = s->insn_bits(dev, s, insn, insn->data);
			break;
		case INSN_CONFIG:
			
			ret = s->insn_config(dev, s, insn, insn->data);
			break;
		default:
			ret = -EINVAL;
			break;
		}

		s->busy = NULL;
	}
	if (ret < 0)
		goto error;
#if 0
	
	if (ret != insn->n) {
		printk("BUG: result of insn != insn.n\n");
		ret = -EINVAL;
		goto error;
	}
#endif
error:

	return ret;
}


int comedi_lock(void *d, unsigned int subdevice)
{
	struct comedi_device *dev = (struct comedi_device *)d;
	struct comedi_subdevice *s;
	unsigned long flags;
	int ret = 0;

	if (subdevice >= dev->n_subdevices)
		return -EINVAL;

	s = dev->subdevices + subdevice;

	spin_lock_irqsave(&s->spin_lock, flags);

	if (s->busy) {
		ret = -EBUSY;
	} else {
		if (s->lock) {
			ret = -EBUSY;
		} else {
			s->lock = d;
		}
	}

	spin_unlock_irqrestore(&s->spin_lock, flags);

	return ret;
}


int comedi_unlock(void *d, unsigned int subdevice)
{
	struct comedi_device *dev = (struct comedi_device *)d;
	struct comedi_subdevice *s;
	unsigned long flags;
	struct comedi_async *async;
	int ret;

	if (subdevice >= dev->n_subdevices)
		return -EINVAL;

	s = dev->subdevices + subdevice;

	async = s->async;

	spin_lock_irqsave(&s->spin_lock, flags);

	if (s->busy) {
		ret = -EBUSY;
	} else if (s->lock && s->lock != (void *)d) {
		ret = -EACCES;
	} else {
		s->lock = NULL;

		if (async) {
			async->cb_mask = 0;
			async->cb_func = NULL;
			async->cb_arg = NULL;
		}

		ret = 0;
	}

	spin_unlock_irqrestore(&s->spin_lock, flags);

	return ret;
}


int comedi_cancel(void *d, unsigned int subdevice)
{
	struct comedi_device *dev = (struct comedi_device *)d;
	struct comedi_subdevice *s;
	int ret = 0;

	if (subdevice >= dev->n_subdevices)
		return -EINVAL;

	s = dev->subdevices + subdevice;

	if (s->lock && s->lock != d)
		return -EACCES;

#if 0
	if (!s->busy)
		return 0;

	if (s->busy != d)
		return -EBUSY;
#endif

	if (!s->cancel || !s->async)
		return -EINVAL;

	ret = s->cancel(dev, s);

	if (ret)
		return ret;

	comedi_set_subdevice_runflags(s, SRF_RUNNING | SRF_RT, 0);
	s->async->inttrig = NULL;
	s->busy = NULL;

	return 0;
}


int comedi_register_callback(void *d, unsigned int subdevice,
			     unsigned int mask, int (*cb) (unsigned int,
							   void *), void *arg)
{
	struct comedi_device *dev = (struct comedi_device *)d;
	struct comedi_subdevice *s;
	struct comedi_async *async;

	if (subdevice >= dev->n_subdevices)
		return -EINVAL;

	s = dev->subdevices + subdevice;

	async = s->async;
	if (s->type == COMEDI_SUBD_UNUSED || !async)
		return -EIO;

	
	if (s->lock && s->lock != d)
		return -EACCES;

	
	if (s->busy)
		return -EBUSY;

	if (!mask) {
		async->cb_mask = 0;
		async->cb_func = NULL;
		async->cb_arg = NULL;
	} else {
		async->cb_mask = mask;
		async->cb_func = cb;
		async->cb_arg = arg;
	}

	return 0;
}

int comedi_poll(void *d, unsigned int subdevice)
{
	struct comedi_device *dev = (struct comedi_device *)d;
	struct comedi_subdevice *s = dev->subdevices;
	struct comedi_async *async;

	if (subdevice >= dev->n_subdevices)
		return -EINVAL;

	s = dev->subdevices + subdevice;

	async = s->async;
	if (s->type == COMEDI_SUBD_UNUSED || !async)
		return -EIO;

	
	if (s->lock && s->lock != d)
		return -EACCES;

	
	if (!s->busy)
		return -EIO;

	return s->poll(dev, s);
}


int comedi_map(void *d, unsigned int subdevice, void *ptr)
{
	struct comedi_device *dev = (struct comedi_device *)d;
	struct comedi_subdevice *s;

	if (subdevice >= dev->n_subdevices)
		return -EINVAL;

	s = dev->subdevices + subdevice;

	if (!s->async)
		return -EINVAL;

	if (ptr)
		*((void **)ptr) = s->async->prealloc_buf;

	

	return 0;
}


int comedi_unmap(void *d, unsigned int subdevice)
{
	struct comedi_device *dev = (struct comedi_device *)d;
	struct comedi_subdevice *s;

	if (subdevice >= dev->n_subdevices)
		return -EINVAL;

	s = dev->subdevices + subdevice;

	if (!s->async)
		return -EINVAL;

	

	return 0;
}
