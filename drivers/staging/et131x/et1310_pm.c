

#include "et131x_version.h"
#include "et131x_defs.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>

#include <linux/sched.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/in.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/bitops.h>
#include <asm/system.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/ioport.h>

#include "et1310_phy.h"
#include "et1310_pm.h"
#include "et1310_jagcore.h"
#include "et1310_mac.h"
#include "et1310_rx.h"

#include "et131x_adapter.h"
#include "et131x_initpci.h"


void EnablePhyComa(struct et131x_adapter *etdev)
{
	unsigned long flags;
	u32 GlobalPmCSR;

	GlobalPmCSR = readl(&etdev->regs->global.pm_csr);

	
	etdev->PoMgmt.PowerDownSpeed = etdev->AiForceSpeed;
	etdev->PoMgmt.PowerDownDuplex = etdev->AiForceDpx;

	
	spin_lock_irqsave(&etdev->SendHWLock, flags);
	etdev->Flags |= fMP_ADAPTER_LOWER_POWER;
	spin_unlock_irqrestore(&etdev->SendHWLock, flags);

	

	
	GlobalPmCSR &= ~ET_PMCSR_INIT;
	writel(GlobalPmCSR, &etdev->regs->global.pm_csr);

	
	GlobalPmCSR |= ET_PM_PHY_SW_COMA;
	writel(GlobalPmCSR, &etdev->regs->global.pm_csr);
}


void DisablePhyComa(struct et131x_adapter *etdev)
{
	u32 GlobalPmCSR;

	GlobalPmCSR = readl(&etdev->regs->global.pm_csr);

	
	GlobalPmCSR |= ET_PMCSR_INIT;
	GlobalPmCSR &= ~ET_PM_PHY_SW_COMA;
	writel(GlobalPmCSR, &etdev->regs->global.pm_csr);

	
	etdev->AiForceSpeed = etdev->PoMgmt.PowerDownSpeed;
	etdev->AiForceDpx = etdev->PoMgmt.PowerDownDuplex;

	
	et131x_init_send(etdev);

	
	et131x_reset_recv(etdev);

	
	et131x_soft_reset(etdev);

	
	et131x_adapter_setup(etdev);

	
	etdev->Flags &= ~fMP_ADAPTER_LOWER_POWER;

	
	et131x_rx_dma_enable(etdev);
}

