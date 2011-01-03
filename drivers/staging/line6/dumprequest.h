

#ifndef DUMPREQUEST_H
#define DUMPREQUEST_H


#include <linux/usb.h>
#include <linux/wait.h>

#include <sound/core.h>


enum {
	LINE6_DUMP_NONE,
	LINE6_DUMP_CURRENT
};


struct line6_dump_reqbuf {
	
	unsigned char *buffer;

	
	size_t length;
};


struct line6_dump_request {
	
	wait_queue_head_t wait;

	
	int in_progress;

	
	struct timer_list timer;

	
	char ok;

	
	struct line6_dump_reqbuf reqbufs[1];
};

extern void line6_dump_finished(struct line6_dump_request *l6dr);
extern int line6_dump_request_async(struct line6_dump_request *l6dr,
				    struct usb_line6 *line6, int num);
extern void line6_dump_started(struct line6_dump_request *l6dr, int dest);
extern void line6_dumpreq_destruct(struct line6_dump_request *l6dr);
extern void line6_dumpreq_destructbuf(struct line6_dump_request *l6dr, int num);
extern int line6_dumpreq_init(struct line6_dump_request *l6dr, const void *buf,
			      size_t len);
extern int line6_dumpreq_initbuf(struct line6_dump_request *l6dr,
				 const void *buf, size_t len, int num);
extern void line6_invalidate_current(struct line6_dump_request *l6dr);
extern void line6_startup_delayed(struct line6_dump_request *l6dr, int seconds,
				  void (*function)(unsigned long), void *data);
extern int line6_wait_dump(struct line6_dump_request *l6dr, int nonblock);


#endif
