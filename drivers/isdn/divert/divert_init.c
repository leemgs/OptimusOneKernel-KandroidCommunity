

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include "isdn_divert.h"

MODULE_DESCRIPTION("ISDN4Linux: Call diversion support");
MODULE_AUTHOR("Werner Cornelius");
MODULE_LICENSE("GPL");




isdn_divert_if divert_if =
  { DIVERT_IF_MAGIC,  
    DIVERT_CMD_REG,   
    ll_callback,      
    NULL,             
    NULL,             
    NULL,             
  };





static int __init divert_init(void)
{ int i;

  if (divert_dev_init())
   { printk(KERN_WARNING "dss1_divert: cannot install device, not loaded\n");
     return(-EIO);
   }
  if ((i = DIVERT_REG_NAME(&divert_if)) != DIVERT_NO_ERR)
   { divert_dev_deinit();
     printk(KERN_WARNING "dss1_divert: error %d registering module, not loaded\n",i);
     return(-EIO);
   } 
  printk(KERN_INFO "dss1_divert module successfully installed\n");
  return(0);
}




static void __exit divert_exit(void)
{
  unsigned long flags;
  int i;

  spin_lock_irqsave(&divert_lock, flags);
  divert_if.cmd = DIVERT_CMD_REL; 
  if ((i = DIVERT_REG_NAME(&divert_if)) != DIVERT_NO_ERR)
   { printk(KERN_WARNING "dss1_divert: error %d releasing module\n",i);
     spin_unlock_irqrestore(&divert_lock, flags);
     return;
   } 
  if (divert_dev_deinit()) 
   { printk(KERN_WARNING "dss1_divert: device busy, remove cancelled\n");
     spin_unlock_irqrestore(&divert_lock, flags);
     return;
   }
  spin_unlock_irqrestore(&divert_lock, flags);
  deleterule(-1); 
  deleteprocs();
  printk(KERN_INFO "dss1_divert module successfully removed \n");
}

module_init(divert_init);
module_exit(divert_exit);

