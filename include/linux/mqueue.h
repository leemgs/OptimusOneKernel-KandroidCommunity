

#ifndef _LINUX_MQUEUE_H
#define _LINUX_MQUEUE_H

#define MQ_PRIO_MAX 	32768

#define MQ_BYTES_MAX	819200

struct mq_attr {
	long	mq_flags;	
	long	mq_maxmsg;	
	long	mq_msgsize;	
	long	mq_curmsgs;	
	long	__reserved[4];	
};


#define NOTIFY_NONE	0
#define NOTIFY_WOKENUP	1
#define NOTIFY_REMOVED	2

#define NOTIFY_COOKIE_LEN	32

#endif
