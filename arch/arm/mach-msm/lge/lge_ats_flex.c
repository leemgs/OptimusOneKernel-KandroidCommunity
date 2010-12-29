

 
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <mach/msm_rpcrouter.h>
#include <mach/board_lge.h>
#include <linux/fcntl.h>
#include <linux/syscalls.h>


#include "lge_ats.h"
#include "lge_ats_flex.h"

int lg_get_flex_from_xml(char *strIndex, char* flexValue)
{
	int                       fd;
	int                       iFlexSize;
	signed long int      res;
	int                       iTotalCnt, iItemCnt,iValueCnt,j,iItemCntAll;
		
	char*                    s_bufFlexINI; 
	char                    s_bufItem[500];     
	
	char                    s_bufValue2[400]; 
	char                    s_bufValue3[100]; 

	iFlexSize = 500;	
	fd = sys_open((const char __user *) "/system/etc/flex/flex.xml", O_RDONLY,0);

	if (fd == -1)	
		{
		printk("read data fail");
		return 0;
		}

	

	
	s_bufFlexINI = kmalloc(500, GFP_KERNEL);
	
	res = sys_read(fd, (char __user*)s_bufFlexINI, sizeof(char)*iFlexSize);
	
	sys_close(fd);

	printk("read data flex.xml fd: %d iFlexSize : %d res;%d\n", fd, iFlexSize, res);

	iFlexSize=res;
	
	iItemCnt = 0;
	iItemCntAll = 0;
	
	for(iTotalCnt=0; iTotalCnt<iFlexSize;iTotalCnt++)  
	{
		
		if ((s_bufFlexINI[iItemCntAll]) != '\n')  
		{
			s_bufItem[iItemCnt]=s_bufFlexINI[iItemCntAll];
			iItemCnt ++;
			
		} 
		else  
		{	
			
			s_bufItem[iItemCnt]='\n';
				
			j = 0;
			iValueCnt = 0;
			memset(s_bufValue3,0x00,sizeof(char)*100);
			while((s_bufItem[j] != '=') && (s_bufItem[j] != '\n') )  
			{
				
				if(s_bufItem[j] != ' ' && s_bufItem[j] != '\t') 
				{
					s_bufValue3[iValueCnt++] = s_bufItem[j];
					
				}
				j++;
			}

			if(!strncmp(s_bufValue3,strIndex,strlen(strIndex)))
			{ 
				
				iValueCnt = 0;
				j++;
				while(s_bufItem[j] != '\n' )  
				{
					
					if(s_bufItem[j] != '"') 
					{
						s_bufValue2[iValueCnt++] = s_bufItem[j];
						
					}
					j++;
				}
				memcpy(flexValue,s_bufValue2, iValueCnt);
				if(flexValue[iValueCnt-1] == '\r')
				{
					flexValue[iValueCnt-1] = 0x00;
					if(iValueCnt == 1)
					{
						printk("\niValueCnt == 1");
						return 0;
					}
				}
				else
				{
					flexValue[iValueCnt] = 0x00;
					if(iValueCnt == 0)
					{
						printk("\niValueCnt == 0");
						return 0;
					}
				}
				
				return 1;
			}
			iItemCnt = 0;
		}
		iItemCntAll++;
	}
	return 0;

}


int lge_ats_handle_flex(struct msm_rpc_server *server,
							 struct rpc_request_hdr *req, unsigned len
							  )
{
	int result = HANDLE_OK;

	memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));

	switch (req->procedure)
	{
		case ONCRPC_LGE_GET_FLEX_MCC_PROC	:
		{		
			printk(KERN_INFO "ONCRPC_LGE_GET_FLEX_MCC_PROC\n");
			memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));		
			if(!lg_get_flex_from_xml("FLEX_MCC_CODE", server->retvalue.ret_string))
				result = HANLDE_FAIL;

			server->retvalue.ret_value1 = strlen(server->retvalue.ret_string);
			server->retvalue.ret_value2 = 0;
			printk(KERN_INFO "ONCRPC_LGE_GET_FLEX_MCC_PROC return string : %d , %s\n",
				  server->retvalue.ret_value1,server->retvalue.ret_string); 	
			break;
		}
		case ONCRPC_LGE_GET_FLEX_MNC_PROC	:
		{		
			printk(KERN_INFO"ONCRPC_LGE_GET_FLEX_MNC_PROC\n");
			memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));		
			if(!lg_get_flex_from_xml("FLEX_MNC_CODE", server->retvalue.ret_string))
				result = HANLDE_FAIL;

			server->retvalue.ret_value1 = strlen(server->retvalue.ret_string);
			server->retvalue.ret_value2 = 0;
			printk(KERN_INFO "ONCRPC_LGE_GET_FLEX_MNC_PROC return string : %d , %s\n",
				  server->retvalue.ret_value1,server->retvalue.ret_string); 
			break;	
		}
		case ONCRPC_LGE_GET_FLEX_OPERATOR_CODE_PROC :
		{		
			printk(KERN_INFO"ONCRPC_LGE_GET_FLEX_OPERATOR_CODE_PROC\n");
			memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));		
			if(!lg_get_flex_from_xml("FLEX_OPERATOR_CODE", server->retvalue.ret_string))
				result = HANLDE_FAIL;

			server->retvalue.ret_value1 = strlen(server->retvalue.ret_string);
			server->retvalue.ret_value2 = 0;
			printk(KERN_INFO "ONCRPC_LGE_GET_FLEX_OPERATOR_CODE_PROC return string : %d , %s\n",
				  server->retvalue.ret_value1,server->retvalue.ret_string); 
			
	#if 0
			if (!msm_fb_refesh_enabled && !fb_control_timer_init) { 
				printk("[Blue Debug] Set Timer\n");
				setup_timer(&lg_fb_control, lg_fb_control_timer, 0);	
		
				if( strncmp(server->retvalue.ret_string,"ORG",3) ==0 ) { 
					mod_timer (&lg_fb_control,jiffies + (ORG_FB_TIMEOUT * HZ / 1000) ); 
				} else {
					mod_timer (&lg_fb_control,jiffies + (OTHER_FB_TIMEOUT * HZ / 1000) );	
				}
		
				fb_control_timer_init = 1;
			}
	#endif
			
		
			break;
		}
		case ONCRPC_LGE_GET_FLEX_COUNTRY_CODE_PROC	:
		{		
			printk(KERN_INFO"ONCRPC_LGE_GET_FLEX_COUNTRY_PROC\n");
			memset(server->retvalue.ret_string, 0, sizeof(server->retvalue.ret_string));		
			if(!lg_get_flex_from_xml("FLEX_COUNTRY_CODE", server->retvalue.ret_string))
				result = HANLDE_FAIL;

			server->retvalue.ret_value1 = strlen(server->retvalue.ret_string);
			server->retvalue.ret_value2 = 0;
			printk(KERN_INFO "ONCRPC_LGE_GET_FLEX_COUNTRY_PROC return string : %d , %s\n",
				  server->retvalue.ret_value1,server->retvalue.ret_string); 	
			break;
		}

		default :
			result = HANDLE_ERROR;
			break;
	}

        
	
	if(result == HANDLE_OK)
					
		result = RPC_RETURN_RESULT_OK;
	else
		result= RPC_RETURN_RESULT_ERROR;
	
	printk(KERN_INFO "lge_ats_handle_flex result : %d \n",result);

	return result;
}
