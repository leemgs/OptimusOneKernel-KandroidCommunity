
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/types.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/ss.h>
#include <pcmcia/cistpl.h>
#include <pcmcia/bus_ops.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>

#include <asm/au1000.h>
#include <asm/au1000_pcmcia.h>

#define PCMCIA_MAX_SOCK		0
#define PCMCIA_NUM_SOCKS	(PCMCIA_MAX_SOCK + 1)
#define PCMCIA_IRQ		AU1000_GPIO_4

#if 0
#define DEBUG(x, args...)	printk(__func__ ": " x, ##args)
#else
#define DEBUG(x,args...)
#endif

static int xxs1500_pcmcia_init(struct pcmcia_init *init)
{
	return PCMCIA_NUM_SOCKS;
}

static int xxs1500_pcmcia_shutdown(void)
{
	
	au_writel(au_readl(GPIO2_PINSTATE) | (1<<14)|(1<<30),
			GPIO2_OUTPUT);
	au_sync_delay(100);

	
	au_writel(au_readl(GPIO2_PINSTATE) | (1<<4)|(1<<20),
			GPIO2_OUTPUT);
	au_sync_delay(100);
	return 0;
}


static int
xxs1500_pcmcia_socket_state(unsigned sock, struct pcmcia_state *state)
{
	u32 inserted; u32 vs;
	unsigned long gpio, gpio2;

	if(sock > PCMCIA_MAX_SOCK) return -1;

	gpio = au_readl(SYS_PINSTATERD);
	gpio2 = au_readl(GPIO2_PINSTATE);

	vs = gpio2 & ((1<<8) | (1<<9));
	inserted = (!(gpio & 0x1) && !(gpio & 0x2));

	state->ready = 0;
	state->vs_Xv = 0;
	state->vs_3v = 0;
	state->detect = 0;

	if (inserted) {
		switch (vs) {
			case 0:
			case 1:
			case 2:
				state->vs_3v=1;
				break;
			case 3: 
			default:
				
				printk(KERN_ERR "au1x00_cs: unsupported VS\n",
						vs);
				return;
		}
		state->detect = 1;
	}

	if (state->detect) {
		state->ready = 1;
	}

	state->bvd1= gpio2 & (1<<10);
	state->bvd2 = gpio2 & (1<<11);
	state->wrprot=0;
	return 1;
}


static int xxs1500_pcmcia_get_irq_info(struct pcmcia_irq_info *info)
{

	if(info->sock > PCMCIA_MAX_SOCK) return -1;
	info->irq = PCMCIA_IRQ;
	return 0;
}


static int
xxs1500_pcmcia_configure_socket(const struct pcmcia_configure *configure)
{

	if(configure->sock > PCMCIA_MAX_SOCK) return -1;

	DEBUG("Vcc %dV Vpp %dV, reset %d\n",
			configure->vcc, configure->vpp, configure->reset);

	switch(configure->vcc){
		case 33: 
			
			DEBUG("turn on power\n");
			au_writel((au_readl(GPIO2_PINSTATE) & ~(1<<14))|(1<<30),
					GPIO2_OUTPUT);
			au_sync_delay(100);
			break;
		case 50: 
		default: 
			printk(KERN_ERR "au1x00_cs: unsupported VCC\n");
		case 0:  
			
			au_sync_delay(100);
			au_writel(au_readl(GPIO2_PINSTATE) | (1<<14)|(1<<30),
					GPIO2_OUTPUT);
			break;
	}

	if (!configure->reset) {
		DEBUG("deassert reset\n");
		au_writel((au_readl(GPIO2_PINSTATE) & ~(1<<4))|(1<<20),
				GPIO2_OUTPUT);
		au_sync_delay(100);
		au_writel((au_readl(GPIO2_PINSTATE) & ~(1<<5))|(1<<21),
				GPIO2_OUTPUT);
	}
	else {
		DEBUG("assert reset\n");
		au_writel(au_readl(GPIO2_PINSTATE) | (1<<4)|(1<<20),
				GPIO2_OUTPUT);
	}
	au_sync_delay(100);
	return 0;
}

struct pcmcia_low_level xxs1500_pcmcia_ops = {
	xxs1500_pcmcia_init,
	xxs1500_pcmcia_shutdown,
	xxs1500_pcmcia_socket_state,
	xxs1500_pcmcia_get_irq_info,
	xxs1500_pcmcia_configure_socket
};
