

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/io.h>

#include <asm/atomic.h>

#include "cm.h"



#define MAX_MODULE_READY_TIME			20000


#define OMAP4_PRCM_CM_CLKCTRL_IDLEST_MASK	(0x2 << 16)


#define OMAP4_PRCM_MOD_CM_ID_SHIFT		16
#define OMAP4_PRCM_MOD_OFFS_MASK		0xffff


int omap4_cm_wait_idlest_ready(u32 prcm_mod, u8 prcm_dev_offs)
{
	
	return 0;
}

