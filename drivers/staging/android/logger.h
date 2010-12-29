

#ifndef _LINUX_LOGGER_H
#define _LINUX_LOGGER_H

#include <linux/types.h>
#include <linux/ioctl.h>

struct logger_entry {
	__u16		len;	
	__u16		__pad;	
	__s32		pid;	
	__s32		tid;	
	__s32		sec;	
	__s32		nsec;	
	char		msg[0];	
};

#define LOGGER_LOG_RADIO	"log_radio"	
#define LOGGER_LOG_EVENTS	"log_events"	
#define LOGGER_LOG_SYSTEM	"log_system"	
#define LOGGER_LOG_MAIN		"log_main"	

#define LOGGER_ENTRY_MAX_LEN		(4*1024)
#define LOGGER_ENTRY_MAX_PAYLOAD	\
	(LOGGER_ENTRY_MAX_LEN - sizeof(struct logger_entry))

#define __LOGGERIO	0xAE

#define LOGGER_GET_LOG_BUF_SIZE		_IO(__LOGGERIO, 1) 
#define LOGGER_GET_LOG_LEN		_IO(__LOGGERIO, 2) 
#define LOGGER_GET_NEXT_ENTRY_LEN	_IO(__LOGGERIO, 3) 
#define LOGGER_FLUSH_LOG		_IO(__LOGGERIO, 4) 

#endif 
