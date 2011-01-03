

#ifndef __BFA_DEFS_DRIVER_H__
#define __BFA_DEFS_DRIVER_H__


	u16    tm_io_abort;
    u16    tm_io_abort_comp;
    u16    tm_lun_reset;
    u16    tm_lun_reset_comp;
    u16    tm_target_reset;
    u16    tm_bus_reset;
    u16    ioc_restart;        
    u16    io_pending;         
    u64    control_req;
    u64    input_req;
    u64    output_req;
    u64    input_words;
    u64    output_words;
} bfa_driver_stats_t;


#endif 
