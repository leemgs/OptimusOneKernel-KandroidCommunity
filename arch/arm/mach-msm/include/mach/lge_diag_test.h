

#ifndef __ARCH_MSM_LGE_DIAG_TEST_H
#define __ARCH_MSM_LGE_DIAG_TEST_H

extern uint8_t if_condition_is_on_key_buffering;
extern uint8_t lgf_factor_key_test_rsp(char);
extern unsigned long int ats_mtc_log_mask;

extern int eta_execute(char *);
extern int base64_encode(char *, int, char *);

#endif
