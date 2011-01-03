



#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/sysrq.h>
#include <linux/time.h>
#include "linux/proc_fs.h"
#include "linux/kernel_stat.h"
#include "asm/uaccess.h"
#include "cp15_registers.h"
#include "perf.h"


static __init int per_init(void)
{

  if (atomic_read(&pm_op_lock) == 1) {
	printk(KERN_INFO "Can not load KSAPI, monitors are in use\n");
	return -1;
  }
  atomic_set(&pm_op_lock, 1);
  per_process_perf_init();
  printk(KERN_INFO "ksapi init\n");
  return 0;
}

static void __exit per_exit(void)
{
  per_process_perf_exit();
  printk(KERN_INFO "ksapi exit\n");
  atomic_set(&pm_op_lock, 0);
}

MODULE_LICENSE("GPL v2");
module_init(per_init);
module_exit(per_exit);
