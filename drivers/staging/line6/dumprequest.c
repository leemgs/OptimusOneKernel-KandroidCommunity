

#include "driver.h"
#include "dumprequest.h"



void line6_dump_started(struct line6_dump_request *l6dr, int dest)
{
	l6dr->in_progress = dest;
}


void line6_invalidate_current(struct line6_dump_request *l6dr)
{
	line6_dump_started(l6dr, LINE6_DUMP_CURRENT);
}


void line6_dump_finished(struct line6_dump_request *l6dr)
{
	l6dr->in_progress = LINE6_DUMP_NONE;
	wake_up_interruptible(&l6dr->wait);
}


int line6_dump_request_async(struct line6_dump_request *l6dr,
			     struct usb_line6 *line6, int num)
{
	int ret;
	line6_invalidate_current(l6dr);
	ret = line6_send_raw_message_async(line6, l6dr->reqbufs[num].buffer,
					   l6dr->reqbufs[num].length);

	if (ret < 0)
		line6_dump_finished(l6dr);

	return ret;
}


void line6_startup_delayed(struct line6_dump_request *l6dr, int seconds,
			   void (*function)(unsigned long), void *data)
{
	l6dr->timer.expires = jiffies + seconds * HZ;
	l6dr->timer.function = function;
	l6dr->timer.data = (unsigned long)data;
	add_timer(&l6dr->timer);
}


int line6_wait_dump(struct line6_dump_request *l6dr, int nonblock)
{
	int retval = 0;
	DECLARE_WAITQUEUE(wait, current);
	add_wait_queue(&l6dr->wait, &wait);
	current->state = TASK_INTERRUPTIBLE;

	while (l6dr->in_progress) {
		if (nonblock) {
			retval = -EAGAIN;
			break;
		}

		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			break;
		} else
			schedule();
	}

	current->state = TASK_RUNNING;
	remove_wait_queue(&l6dr->wait, &wait);
	return retval;
}


int line6_dumpreq_initbuf(struct line6_dump_request *l6dr, const void *buf,
			  size_t len, int num)
{
	l6dr->reqbufs[num].buffer = kmalloc(len, GFP_KERNEL);
	if (l6dr->reqbufs[num].buffer == NULL)
		return -ENOMEM;
	memcpy(l6dr->reqbufs[num].buffer, buf, len);
	l6dr->reqbufs[num].length = len;
	return 0;
}


int line6_dumpreq_init(struct line6_dump_request *l6dr, const void *buf,
		       size_t len)
{
	int ret;
	ret = line6_dumpreq_initbuf(l6dr, buf, len, 0);
	if (ret < 0)
		return ret;
	init_waitqueue_head(&l6dr->wait);
	init_timer(&l6dr->timer);
	return 0;
}


void line6_dumpreq_destructbuf(struct line6_dump_request *l6dr, int num)
{
	if (l6dr == NULL)
		return;
	if (l6dr->reqbufs[num].buffer == NULL)
		return;
	kfree(l6dr->reqbufs[num].buffer);
	l6dr->reqbufs[num].buffer = NULL;
}


void line6_dumpreq_destruct(struct line6_dump_request *l6dr)
{
	if (l6dr->reqbufs[0].buffer == NULL)
		return;
	line6_dumpreq_destructbuf(l6dr, 0);
	l6dr->ok = 1;
	del_timer_sync(&l6dr->timer);
}
