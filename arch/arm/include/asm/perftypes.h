



#ifndef __PERFTYPES_H__
#define __PERFTYPES_H__

typedef void (*VPVF)(void);
typedef void (*VPULF)(unsigned long);
typedef void (*VPULULF)(unsigned long, unsigned long);

extern VPVF pp_interrupt_out_ptr;
extern VPVF pp_interrupt_in_ptr;
extern VPULF pp_process_remove_ptr;
extern void perf_mon_interrupt_in(void);
extern void perf_mon_interrupt_out(void);

#endif
