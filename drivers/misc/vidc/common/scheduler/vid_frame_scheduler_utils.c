

#include "vidc_type.h"
#include "vid_frame_scheduler_utils.h"


SCHED_INLINE void SCHED_ASSERT(int val)
{

}				


SCHED_INLINE int SCHED_MIN(int n_x, int n_y)
{
	if (n_x < n_y)
		return n_x;
	else
		return n_y;

}				


SCHED_INLINE void *SCHED_MALLOC(int size)
{
	return kmalloc(size, GFP_KERNEL);
}				


SCHED_INLINE void SCHED_FREE(void *p_ptr)
{
	kfree(p_ptr);
}				


SCHED_INLINE void *SCHED_MEMSET(void *ptr, int val, int size)
{
	return memset(ptr, val, size);
}				


SCHED_INLINE enum sched_status_type SCHED_GET_CURRENT_TIME(u32 *pn_time)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	*pn_time = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	return SCHED_S_OK;

}				


SCHED_INLINE enum sched_status_type SCHED_CRITSEC_CREATE(u32 **p_cs)
{
	return SCHED_S_OK;

}				


SCHED_INLINE enum sched_status_type SCHED_CRITSEC_RELEASE(u32 *cs)
{
	return SCHED_S_OK;

}				


SCHED_INLINE enum sched_status_type SCHED_CRITSEC_ENTER(u32 *cs)
{
	return SCHED_S_OK;

}				


SCHED_INLINE enum sched_status_type SCHED_CRITSEC_LEAVE(u32 *cs)
{
	return SCHED_S_OK;

}				
