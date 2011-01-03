


#ifndef __RTMP_OS_H__
#define __RTMP_OS_H__

#ifdef LINUX
#include "rt_linux.h"
#endif 



typedef struct _RTMP_OS_NETDEV_OP_HOOK_
{
	const struct net_device_ops *netdev_ops;
	void			*priv;
	int			priv_flags;
	unsigned char devAddr[6];
	unsigned char	devName[16];
	unsigned char	needProtcted;
}RTMP_OS_NETDEV_OP_HOOK, *PRTMP_OS_NETDEV_OP_HOOK;


typedef enum _RTMP_TASK_STATUS_
{
	RTMP_TASK_STAT_UNKNOWN = 0,
	RTMP_TASK_STAT_INITED = 1,
	RTMP_TASK_STAT_RUNNING = 2,
	RTMP_TASK_STAT_STOPED = 4,
}RTMP_TASK_STATUS;
#define RTMP_TASK_CAN_DO_INSERT		(RTMP_TASK_STAT_INITED |RTMP_TASK_STAT_RUNNING)

#define RTMP_OS_TASK_NAME_LEN	16
typedef struct _RTMP_OS_TASK_
{
	char					taskName[RTMP_OS_TASK_NAME_LEN];
	void					*priv;
	
	RTMP_TASK_STATUS	taskStatus;
#ifndef KTHREAD_SUPPORT
	RTMP_OS_SEM			taskSema;
	RTMP_OS_PID			taskPID;
	struct completion		taskComplete;
#endif
	unsigned char			task_killed;
#ifdef KTHREAD_SUPPORT
	struct task_struct	*kthread_task;
	wait_queue_head_t		kthread_q;
	BOOLEAN					kthread_running;
#endif
}RTMP_OS_TASK;

#endif 
