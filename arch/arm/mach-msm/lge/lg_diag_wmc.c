#include <linux/module.h>
#include <linux/delay.h>
#include <mach/lg_diagcmd.h>
#include <mach/lg_diag_wmc.h>



#include <linux/unistd.h> 
#include <linux/fcntl.h> 
#include <linux/syscalls.h> 
#include <linux/fs.h> 





#ifdef CONFIG_LGE_DIAG_WMC

void* lg_diag_wmc_req_pkt_ptr;
uint16 lg_diag_wmc_req_pkt_length;
uint16 lg_diag_wmc_rsp_pkt_length;

PACK (void *)LGF_WMC (
        PACK (void	*)req_pkt_ptr,	
        uint16		pkt_len )		      
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

 
  
  
  
  

  lg_diag_wmc_req_pkt_ptr = req_pkt_ptr;
  lg_diag_wmc_req_pkt_length = pkt_len;


  if ( (fd = sys_open((const char __user *) "/system/bin/lg_diag_wmc", O_RDONLY ,0) ) < 0 )
  {
    
    sprintf(cmdstr, "/system/bin/lg_diag_wmc\n");
  }
  else
  {
    
    sprintf(cmdstr, "/system/bin/lg_diag_wmc\n");
    sys_close(fd);
  }

  

  if ((ret = call_usermodehelper("/system/bin/sh", argv, envp, UMH_WAIT_PROC)) != 0) {
  	
  }
  else
    
  
  return NULL;

}

EXPORT_SYMBOL(LGF_WMC);

#endif
