
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <mach/msm_rpcrouter.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <mach/board_lge.h>
#include <linux/lge_alohag_at.h>
#include "lge_ats.h"

#define JIFFIES_TO_MS(t) ((t) * 1000 / HZ)



#if 1	
extern unsigned char g_diag_mtc_check;
#endif

unsigned int ats_mtc_log_mask = 0x00000000;

int base64_decode(char *, unsigned char *, int);
int base64_encode(char *, int, char *);

extern int event_log_start(void);
extern int event_log_end(void);

int eta_execute(char *string)
{
	int ret;
	char cmdstr[100];
	int fd;
	char *envp[] = {
		"HOME=/",
		"TERM=linux",
		NULL,
	};

	char *argv[] = {
		"sh",
		"-c",
			cmdstr,
		NULL,
	};

	if ( (fd = sys_open((const char __user *) "/system/bin/eta", O_RDONLY ,0) ) < 0 )
	{
		printk("\n [ETA]can not open /system/bin/eta - execute /system/bin/eta\n");
		sprintf(cmdstr, "/system/bin/eta %s", string);
	}
	else
	{
		printk("\n [ETA]execute /system/bin/eta\n");
		sprintf(cmdstr, "/system/bin/eta %s", string);
		sys_close(fd);
	}

	printk(KERN_INFO "[ETA]execute eta : data - %s\n", cmdstr);
	if ((ret =
	     call_usermodehelper("/system/bin/sh", argv, envp, UMH_WAIT_PROC)) != 0) {
		printk(KERN_ERR "[ETA]Eta failed to run \": %i\n",
		       ret);
	}
	else
		printk(KERN_INFO "[ETA]execute ok, ret = %d\n", ret);
	return ret;
}


const char MimeBase64[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	 '4', '5', '6', '7', '8', '9', '+', '/'
};


static int DecodeMimeBase64[256] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  
	52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  
	15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  
	-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  
	41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   
};

int base64_decode(char *text, unsigned char *dst, int numBytes)
{
	const char* cp; 
	int space_idx = 0, phase; 
	int d, prev_d = 0; 
	unsigned char c;

	printk(KERN_INFO "[ETA] text: 0x%X, dst: 0x%X, size: %d\n",  (unsigned int)text, (unsigned int)dst, numBytes);
		
	space_idx = 0; 
	phase = 0;
	
	for ( cp = text; *cp != '\0'; ++cp ) { 
		d = DecodeMimeBase64[(int) *cp]; 
	    if ( d != -1 ) { 
	    	switch ( phase ) { 
	        	case 0: 
		        	++phase; 
	        		break; 
	        	case 1: 
	          		c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
				printk(KERN_INFO "[ETA] space_idx: 0x%X, char: 0x%X\n",  space_idx, c);
	          		if ( space_idx < numBytes )
		            dst[space_idx++] = c; 
			        ++phase; 
	    		    break; 
		        case 2: 
		        	c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
				printk(KERN_INFO "[ETA] space_idx: 0x%X, char: 0x%X\n",  space_idx, c);
			        if ( space_idx < numBytes ) 
	         		dst[space_idx++] = c; 
	          		++phase; 
	          		break; 
	        	case 3: 
	          		c = ( ( ( prev_d & 0x03 ) << 6 ) | d ); 
				printk(KERN_INFO "[ETA] space_idx: 0x%X, char: 0x%X\n",  space_idx, c);
	          		if ( space_idx < numBytes ) 
	            	dst[space_idx++] = c; 
	          		phase = 0; 
	          		break; 
			} 	
	      	prev_d = d; 
		}
	}
	printk(KERN_INFO "[ETA] Complete..\n");
	return space_idx;
}

int base64_encode(char *text, int numBytes, char *encodedText)
{
	unsigned char input[3] = {0,0,0}; 
	unsigned char output[4] = {0,0,0,0}; 
	int  index, i, j; 
	char *p, *plen; 

	plen = text + numBytes - 1; 

	j = 0;

	for (i = 0, p = text;p <= plen; i++, p++) { 
	    index = i % 3; 
	    input[index] = *p;

		if (index == 2 || p == plen) { 
	    	output[0] = ((input[0] & 0xFC) >> 2); 
			output[1] = ((input[0] & 0x3) << 4) | ((input[1] & 0xF0) >> 4); 
			output[2] = ((input[1] & 0xF) << 2) | ((input[2] & 0xC0) >> 6); 
			output[3] = (input[2] & 0x3F);

			encodedText[j++] = MimeBase64[output[0]]; 
			encodedText[j++] = MimeBase64[output[1]]; 
			encodedText[j++] = index == 0? '=' : MimeBase64[output[2]]; 
			encodedText[j++] = index < 2? '=' : MimeBase64[output[3]];

			input[0] = input[1] = input[2] = 0; 
		} 
	}
	encodedText[j] = '\0';

	return strlen(encodedText); 	
}

void ats_mtc_send_key_log_to_eta(struct ats_mtc_key_log_type* p_ats_mtc_key_log)
{
	unsigned char *eta_cmd_buf;
	unsigned char *eta_cmd_buf_encoded;
	int index =0;
	int lenb64 = 0;
	int exec_result = 0;
	unsigned long long eta_time_val = 0;
	
	eta_cmd_buf = kmalloc(sizeof(unsigned char)*50, GFP_KERNEL);
	if(!eta_cmd_buf) {
		printk(KERN_ERR "%s: Error in alloc memory!!\n", __func__);
		return;
	}
	eta_cmd_buf_encoded = kmalloc(sizeof(unsigned char)*50, GFP_KERNEL);
	if(!eta_cmd_buf_encoded) {
		printk(KERN_ERR "%s: Error in alloc memory!!\n", __func__);
		kfree(eta_cmd_buf);
		return;
	}
	memset(eta_cmd_buf,0x00, 50);
	memset(eta_cmd_buf_encoded,0x00, 50);
				
	index = 0;
	eta_cmd_buf[index++] = (unsigned char)0xF0; 
	eta_cmd_buf[index++] = (unsigned char)0x08; 

	eta_cmd_buf[index++] = (unsigned char)p_ats_mtc_key_log->log_id; 
	eta_cmd_buf[index++] = (unsigned char)p_ats_mtc_key_log->log_len; 
	eta_cmd_buf[index++] = (unsigned char)0; 

	eta_time_val = (unsigned long long)JIFFIES_TO_MS(jiffies);
	eta_cmd_buf[index++] = (unsigned char)(eta_time_val & 0xff); 
	eta_cmd_buf[index++] = (unsigned char)( (eta_time_val >> 8) & 0xff );
	eta_cmd_buf[index++] = (unsigned char)( (eta_time_val >> 16) & 0xff );
	eta_cmd_buf[index++] = (unsigned char)( (eta_time_val >> 24) & 0xff );
	eta_cmd_buf[index++] = (unsigned char)( (eta_time_val >> 32) & 0xff );
	eta_cmd_buf[index++] = (unsigned char)( (eta_time_val >> 40) & 0xff );
	eta_cmd_buf[index++] = (unsigned char)( (eta_time_val >> 48) & 0xff );
	eta_cmd_buf[index++] = (unsigned char)( (eta_time_val >> 56) & 0xff ); 

	index = 13;
	if(p_ats_mtc_key_log->log_id == ATS_MTC_KEY_LOG_ID_KEY)
	{
		eta_cmd_buf[index++] = (unsigned char)((p_ats_mtc_key_log->x_hold)&0xFF);
		eta_cmd_buf[index++] = (unsigned char)((p_ats_mtc_key_log->y_code)&0xFF);

		for(index = 15; index<23; index++) 
		{
			eta_cmd_buf[index] = 0;
		}
	}
	else if(p_ats_mtc_key_log->log_id == ATS_MTC_KEY_LOG_ID_TOUCH)
	{
		eta_cmd_buf[index++] = (unsigned char)1; 
		eta_cmd_buf[index++] = (unsigned char)p_ats_mtc_key_log->action;
		eta_cmd_buf[index++] = (unsigned char)((p_ats_mtc_key_log->x_hold)&0xFF);
		eta_cmd_buf[index++] = (unsigned char)(((p_ats_mtc_key_log->x_hold)>>8)&0xFF);
		eta_cmd_buf[index++] = (unsigned char)((p_ats_mtc_key_log->y_code)&0xFF);
		eta_cmd_buf[index++] = (unsigned char)(((p_ats_mtc_key_log->y_code)>>8)&0xFF);

		for(index = 19; index<27; index++) 
		{
			eta_cmd_buf[index] = 0;
		}
	}

	lenb64 = base64_encode((char *)eta_cmd_buf, index, (char *)eta_cmd_buf_encoded);
			
	exec_result = eta_execute(eta_cmd_buf_encoded);
	printk(KERN_INFO "[ETA]AT+MTC exec_result %d\n",exec_result);

	kfree(eta_cmd_buf);
	kfree(eta_cmd_buf_encoded);

}
EXPORT_SYMBOL(ats_mtc_send_key_log_to_eta);

int lge_ats_handle_atcmd_eta(struct msm_rpc_server *server,
								 struct rpc_request_hdr *req, unsigned len)
{
	int result = HANDLE_OK;
	int loop = 0;
	char ret_string[MAX_STRING_RET];
	uint32_t ret_value1 =0;
	uint32_t ret_value2 = 0;
	static AT_SEND_BUFFER_t totalBuffer[LIMIT_MAX_SEND_SIZE_BUFFER];
	static uint32_t totalBufferSize = 0;
	uint32_t at_cmd,at_act;
	int len_b64;
	char *decoded_params;
	unsigned char b0;
	unsigned char b1;
	unsigned char b2;
	unsigned char b3;
	unsigned long logmask = 0x00;
	struct rpc_ats_atcmd_eta_args *args = (struct rpc_ats_atcmd_eta_args *)(req + 1);

	memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));

	memset (ret_string, 0, sizeof(ret_string));

	
	if(args->sendNum == 0)
	{
		
		memset(totalBuffer, 0, sizeof(totalBuffer));
		totalBufferSize = 0;
	}
	
	args->at_cmd = be32_to_cpu(args->at_cmd);
	args->at_act = be32_to_cpu(args->at_act);
	args->sendNum = be32_to_cpu(args->sendNum);
	args->endofBuffer = be32_to_cpu(args->endofBuffer);
	args->buffersize = be32_to_cpu(args->buffersize);
		
	printk(KERN_INFO "[ETA]handle_misc_rpc_call at_cmd = 0x%X, at_act=%d, sendNum=%d:\n",
	      args->at_cmd, args->at_act,args->sendNum);
	printk(KERN_INFO "[ETA]handle_misc_rpc_call endofBuffer = %d, buffersize=%d:\n",
	      args->endofBuffer, args->buffersize);
	printk(KERN_INFO "[ETA]input buff[0] = 0x%X,buff[1]=0x%X,buff[2]=0x%X:\n",args->buffer[0],args->buffer[1],args->buffer[2]);
	if(args->sendNum < MAX_SEND_LOOP_NUM)
	{
		for(loop = 0; loop < args->buffersize; loop++)
		{
			
			totalBuffer[MAX_SEND_SIZE_BUFFER*args->sendNum + loop] =  (args->buffer[loop]);
		}
		
		
		totalBufferSize += args->buffersize;
			
	}
	printk(KERN_INFO "[ETA]handle_misc_rpc_call buff[0] = 0x%X, buff[1]=0x%X, buff[2]=0x%X\n",
	      totalBuffer[0 + args->sendNum*MAX_SEND_SIZE_BUFFER], totalBuffer[1 + args->sendNum*MAX_SEND_SIZE_BUFFER], totalBuffer[2+args->sendNum*MAX_SEND_SIZE_BUFFER]);

	if(!args->endofBuffer )
		return HANDLE_OK_MIDDLE;

	at_cmd = args->at_cmd;
	at_act = args->at_act;




	switch (at_cmd)
	{
		case ATCMD_MTC:
		{
			int exec_result =0;

			printk(KERN_INFO "\n[ETA]ATCMD_MTC\n ");
			

#if 1	
			g_diag_mtc_check = 0;
#endif
			if(at_act != ATCMD_ACTION)
				result = HANLDE_FAIL;

			printk(KERN_INFO "[ETA]totalBuffer : [%s] size: %d\n", totalBuffer, totalBufferSize);
			exec_result = eta_execute(totalBuffer);
			printk(KERN_INFO "[ETA]AT+MTC exec_result %d\n",exec_result);
			
		
			decoded_params = kmalloc(sizeof(char)*totalBufferSize, GFP_KERNEL);
			if(!decoded_params) {
				printk(KERN_ERR "%s: Insufficent memory!!!\n", __func__);
				result = HANDLE_ERROR;
				break;
			}
			printk(KERN_INFO "[ETA] encoded_addr: 0x%X, decoded_addr: 0x%X, size: %d\n",  (unsigned int)totalBuffer, (unsigned int)decoded_params, sizeof(char)*totalBufferSize);
			
			len_b64 = base64_decode((char *)totalBuffer, (unsigned char *)decoded_params, totalBufferSize);
			printk(KERN_INFO "[ETA] sub cmd: 0x%X, param1: 0x%X, param2: 0x%X (length = %d)\n",  
			decoded_params[1], decoded_params[2], decoded_params[3], strlen(decoded_params));

			switch(decoded_params[1]) 
			{
				case 0x07:
					printk(KERN_INFO "[ETA] logging mask request cmd : %d\n", decoded_params[1]);

					b0 = decoded_params[2];
					b1 = decoded_params[3];
					b2 = decoded_params[4];
					b3 = decoded_params[5];

					logmask = b3<<24 | b2<<16 | b1<<8 | b0;

					switch(logmask)
					{
						case 0x00000000:
						case 0xFFFFFFFF:
						case 0x00000001:
						case 0x00000002:
						case 0x00000003:
							ats_mtc_log_mask = logmask;
							break;
						default:
							ats_mtc_log_mask = 0x00000000;
							break;
					}

					if(logmask & 0xFFFFFFFF)
						event_log_start();
					else
						event_log_end();
					break;
					
				default:
					break;
			}
			
			kfree(decoded_params);

			sprintf(ret_string, "edcb");
			ret_value1 = 10;
			ret_value2 = 20;

		}
		break;

		default :
			result = HANDLE_ERROR;
			break;
	}

	
	strncpy(server->retvalue.ret_string, ret_string, MAX_STRING_RET);
	server->retvalue.ret_string[MAX_STRING_RET-1] = 0;
	server->retvalue.ret_value1 = ret_value1;
	server->retvalue.ret_value2 = ret_value2;
	if(args->endofBuffer )
	{
		
		memset(totalBuffer, 0, sizeof(totalBuffer));
		totalBufferSize = 0;
	}

	if(result == HANDLE_OK)
		result = RPC_RETURN_RESULT_OK;
	else if(result == HANDLE_OK_MIDDLE)
		result = RPC_RETURN_RESULT_MIDDLE_OK;
	else
		result= RPC_RETURN_RESULT_ERROR;

	printk(KERN_INFO"%s: resulte = %d\n",
		   __func__, result);

	return result;
}

