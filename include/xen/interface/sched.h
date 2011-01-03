

#ifndef __XEN_PUBLIC_SCHED_H__
#define __XEN_PUBLIC_SCHED_H__

#include "event_channel.h"




#define SCHEDOP_yield       0


#define SCHEDOP_block       1


#define SCHEDOP_shutdown    2
struct sched_shutdown {
    unsigned int reason; 
};
DEFINE_GUEST_HANDLE_STRUCT(sched_shutdown);


#define SCHEDOP_poll        3
struct sched_poll {
    GUEST_HANDLE(evtchn_port_t) ports;
    unsigned int nr_ports;
    uint64_t timeout;
};
DEFINE_GUEST_HANDLE_STRUCT(sched_poll);


#define SHUTDOWN_poweroff   0  
#define SHUTDOWN_reboot     1  
#define SHUTDOWN_suspend    2  
#define SHUTDOWN_crash      3  

#endif 
