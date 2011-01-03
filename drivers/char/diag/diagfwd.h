

#ifndef DIAGFWD_H
#define DIAGFWD_H

void diagfwd_init(void);
void diagfwd_exit(void);
void diag_process_hdlc(void *data, unsigned len);
void __diag_smd_send_req(int mode);
void __diag_smd_qdsp_send_req(int mode);
int diag_device_write(void *buf, int proc_num);
int diagfwd_connect(void);
int diagfwd_disconnect(void);
int mask_request_validate(unsigned char mask_buf[]);


extern int diag_debug_buf_idx;
extern unsigned char diag_debug_buf[1024];
static spinlock_t diagchar_smd_lock;
static spinlock_t diagchar_smd_qdsp_lock;
#endif
