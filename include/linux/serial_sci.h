#ifndef __LINUX_SERIAL_SCI_H
#define __LINUX_SERIAL_SCI_H

#include <linux/serial_core.h>




enum {
	SCIx_ERI_IRQ,
	SCIx_RXI_IRQ,
	SCIx_TXI_IRQ,
	SCIx_BRI_IRQ,
	SCIx_NR_IRQS,
};


struct plat_sci_port {
	void __iomem	*membase;		
	unsigned long	mapbase;		
	unsigned int	irqs[SCIx_NR_IRQS];	
	unsigned int	type;			
	upf_t		flags;			
	char		*clk;			
};

#endif 
