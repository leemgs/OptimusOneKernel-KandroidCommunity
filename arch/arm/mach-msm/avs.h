

#ifndef AVS_H
#define AVS_H

#define VOLTAGE_MIN  1000 
#define VOLTAGE_MAX  1250
#define VOLTAGE_STEP 25

int __init avs_init(int (*set_vdd)(int), u32 freq_cnt, u32 freq_idx);
void __exit avs_exit(void);

int avs_adjust_freq(u32 freq_index, int begin);


u32 avs_test_delays(void);
u32 avs_reset_delays(u32 avsdscr);
u32 avs_get_avscsr(void);
u32 avs_get_avsdscr(void);
u32 avs_get_tscsr(void);
void     avs_set_tscsr(u32 to_tscsr);


#define AVSDEBUG(...)

#endif 
