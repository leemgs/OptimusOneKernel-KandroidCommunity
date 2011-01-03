

#ifndef _CX18_MAILBOX_H_
#define _CX18_MAILBOX_H_


#define MAX_MB_ARGUMENTS 6

#define CX2341X_MBOX_MAX_DATA 16

#define MB_RESERVED_HANDLE_0 0
#define MB_RESERVED_HANDLE_1 0xFFFFFFFF

#define APU 0
#define CPU 1
#define EPU 2
#define HPU 3

struct cx18;


struct cx18_mdl_ack {
    u32 id;        
    u32 data_used; 
};


struct cx18_mailbox {
    
    u32       request;
    
    u32       ack;
    u32       reserved[6];
    
    u32       cmd;
    
    u32       args[MAX_MB_ARGUMENTS];
    
    u32       error;
};

struct cx18_stream;

struct cx18_api_func_private {
	struct cx18 *cx;
	struct cx18_stream *s;
};

int cx18_api(struct cx18 *cx, u32 cmd, int args, u32 data[]);
int cx18_vapi_result(struct cx18 *cx, u32 data[MAX_MB_ARGUMENTS], u32 cmd,
		int args, ...);
int cx18_vapi(struct cx18 *cx, u32 cmd, int args, ...);
int cx18_api_func(void *priv, u32 cmd, int in, int out,
		u32 data[CX2341X_MBOX_MAX_DATA]);

void cx18_api_epu_cmd_irq(struct cx18 *cx, int rpu);

void cx18_in_work_handler(struct work_struct *work);

#endif
