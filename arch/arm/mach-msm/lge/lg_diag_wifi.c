

#include <linux/module.h>
#include <mach/lg_diag_wifi.h>
#include <mach/lg_diagcmd.h>
#include <mach/lg_diag_testmode.h>


extern PACK(void *) diagpkt_alloc (diagpkt_cmd_code_type code, unsigned int length);


PACK (void *)LGF_WIFI(
        PACK (void	*)req_pkt_ptr,	
        uint16		pkt_len )		      
{
  DIAG_LGE_WIFI_MAC_ADDRESS_req_tag	*req_ptr = (DIAG_LGE_WIFI_MAC_ADDRESS_req_tag *) req_pkt_ptr;
  DIAG_LGE_WIFI_MAC_ADDRESS_rsp_tag	*rsp_ptr = NULL;







  printk(KERN_ERR "[WIFI] SubCmd=<%d>\n",req_ptr->sub_cmd);

  switch( req_ptr->sub_cmd )
  {

    default:

      break;
  }

  return (rsp_ptr);	
}

EXPORT_SYMBOL(LGF_WIFI); 
