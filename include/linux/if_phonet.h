
#ifndef LINUX_IF_PHONET_H
#define LINUX_IF_PHONET_H

#define PHONET_MIN_MTU		6	
#define PHONET_MAX_MTU		65541	
#define PHONET_DEV_MTU		PHONET_MAX_MTU

#ifdef __KERNEL__
extern struct header_ops phonet_header_ops;
#endif

#endif
