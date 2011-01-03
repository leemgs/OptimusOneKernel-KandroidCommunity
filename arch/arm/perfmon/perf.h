


#ifndef PERF_H
#define PERF_H
extern unsigned long perf_get_cycles(void);
extern void perf_set_count1(unsigned long val);
extern void perf_set_count0(unsigned long val);
extern unsigned long perf_get_count1(void);
extern unsigned long perf_get_count0(void);
extern unsigned long  perf_get_ctrl(void);
extern void perf_set_ctrl(void);
extern void perf_set_ctrl_with(unsigned long v);
extern void perf_enable_counting(void);
extern void perf_disable_counting(void);
extern void perf_set_divider(int d);
extern unsigned long perf_get_overflow(void);
extern void perf_clear_overflow(unsigned long bit);
extern void perf_export_event(unsigned long bit);
extern void perf_reset_counts(void);
extern int perf_set_event(unsigned long index, unsigned long val);
extern unsigned long perf_get_count(unsigned long index);
extern void perf_set_cycles(unsigned long c);

extern void pm_stop_all(void);
extern void pm_start_all(void);
extern void pm_reset_all(void);
extern void pm_set_event(unsigned long monitorIndex, unsigned long eventIndex);
extern unsigned long pm_get_count(unsigned long monitorIndex);
extern unsigned long pm_get_cycle_count(void);
extern char *pm_find_event_name(unsigned long index);
extern  void pm_set_local_iu(unsigned long events);
extern  void pm_set_local_xu(unsigned long events);
extern  void pm_set_local_su(unsigned long events);
extern  void pm_set_local_l2(unsigned long events);
extern  void pm_set_local_vu(unsigned long events);
extern void pm_initialize(void);
extern void pm_deinitialize(void);
extern void pm_free_irq(void);

extern int per_process_perf_init(void);
extern void per_process_perf_exit(void);
int per_process_read(char *page, char **start, off_t off, int count,
   int *eof, void *data);
int per_process_write_hex(struct file *file, const char *buff,
    unsigned long cnt, void *data);
int per_process_read_decimal(char *page, char **start, off_t off, int count,
   int *eof, void *data);
int per_process_write_dec(struct file *file, const char *buff,
    unsigned long cnt, void *data);
void perfmon_register_callback(void);
void _per_process_switch(unsigned long oldPid, unsigned long newPid);
extern unsigned int pp_loaded;
extern atomic_t pm_op_lock;
#endif 