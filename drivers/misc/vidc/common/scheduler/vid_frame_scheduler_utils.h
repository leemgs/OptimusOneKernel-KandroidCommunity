
#ifndef _SCHEDULER_UTILS_H_
#define _SCHEDULER_UTILS_H_

#include "vid_frame_scheduler_api.h"

#define SCHED_INLINE

#if DEBUG

#define SCHED_MSG_LOW(xx_fmt, ...)	printk(KERN_INFO "\n " \
					xx_fmt, ## __VA_ARGS__)
#define SCHED_MSG_MED(xx_fmt, ...)	printk(KERN_INFO "\n" \
					xx_fmt, ## __VA_ARGS__)
#define SCHED_MSG_HIGH(xx_fmt, ...)	printk(KERN_WARNING "\n" \
					xx_fmt, ## __VA_ARGS__)

#else

#define SCHED_MSG_LOW(xx_fmt...)
#define SCHED_MSG_MED(xx_fmt...)
#define SCHED_MSG_HIGH(xx_fmt...)

#endif

#define SCHED_MSG_ERR(xx_fmt, ...)	printk(KERN_ERR "\n err: " \
					xx_fmt, ## __VA_ARGS__)
#define SCHED_MSG_FATAL(xx_fmt, ...)	printk(KERN_ERR "\n<FATAL> " \
					xx_fmt, ## __VA_ARGS__)

SCHED_INLINE void SCHED_ASSERT(int val);

SCHED_INLINE int SCHED_MIN(int x, int y);

SCHED_INLINE enum sched_status_type SCHED_CRITSEC_CREATE(u32 **p_cs);

SCHED_INLINE enum sched_status_type SCHED_CRITSEC_RELEASE(u32 *cs);

SCHED_INLINE enum sched_status_type SCHED_CRITSEC_ENTER(u32 *cs);

SCHED_INLINE enum sched_status_type SCHED_CRITSEC_LEAVE(u32 *cs);

SCHED_INLINE void *SCHED_MALLOC(int size);

SCHED_INLINE void SCHED_FREE(void *ptr);

SCHED_INLINE void *SCHED_MEMSET(void *ptr, int val, int size);

SCHED_INLINE enum sched_status_type SCHED_GET_CURRENT_TIME(u32 *pn_time);

#endif
