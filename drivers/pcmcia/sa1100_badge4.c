
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/init.h>

#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <mach/badge4.h>
#include <asm/hardware/sa1111.h>

#include "sa1111_generic.h"



static int badge4_pcmvcc = 50;  
static int badge4_pcmvpp = 50;  
static int badge4_cfvcc = 33;   

static void complain_about_jumpering(const char *whom,
				     const char *supply,
				     int given, int wanted)
{
	printk(KERN_ERR
	 "%s: %s %d.%dV wanted but board is jumpered for %s %d.%dV operation"
	 "; re-jumper the board and/or use pcmv=xx,xx,xx\n",
	       whom, supply,
	       wanted / 10, wanted % 10,
	       supply,
	       given / 10, given % 10);
}

static int
badge4_pcmcia_configure_socket(struct soc_pcmcia_socket *skt, const socket_state_t *state)
{
	int ret;

	switch (skt->nr) {
	case 0:
		if ((state->Vcc != 0) &&
		    (state->Vcc != badge4_pcmvcc)) {
			complain_about_jumpering(__func__, "pcmvcc",
						 badge4_pcmvcc, state->Vcc);
			
			
		}
		if ((state->Vpp != 0) &&
		    (state->Vpp != badge4_pcmvpp)) {
			complain_about_jumpering(__func__, "pcmvpp",
						 badge4_pcmvpp, state->Vpp);
			return -1;
		}
		break;

	case 1:
		if ((state->Vcc != 0) &&
		    (state->Vcc != badge4_cfvcc)) {
			complain_about_jumpering(__func__, "cfvcc",
						 badge4_cfvcc, state->Vcc);
			return -1;
		}
		break;

	default:
		return -1;
	}

	ret = sa1111_pcmcia_configure_socket(skt, state);
	if (ret == 0) {
		unsigned long flags;
		int need5V;

		local_irq_save(flags);

		need5V = ((state->Vcc == 50) || (state->Vpp == 50));

		badge4_set_5V(BADGE4_5V_PCMCIA_SOCK(skt->nr), need5V);

		local_irq_restore(flags);
	}

	return 0;
}

static struct pcmcia_low_level badge4_pcmcia_ops = {
	.owner			= THIS_MODULE,
	.hw_init		= sa1111_pcmcia_hw_init,
	.hw_shutdown		= sa1111_pcmcia_hw_shutdown,
	.socket_state		= sa1111_pcmcia_socket_state,
	.configure_socket	= badge4_pcmcia_configure_socket,

	.socket_init		= sa1111_pcmcia_socket_init,
	.socket_suspend		= sa1111_pcmcia_socket_suspend,
};

int pcmcia_badge4_init(struct device *dev)
{
	int ret = -ENODEV;

	if (machine_is_badge4()) {
		printk(KERN_INFO
		       "%s: badge4_pcmvcc=%d, badge4_pcmvpp=%d, badge4_cfvcc=%d\n",
		       __func__,
		       badge4_pcmvcc, badge4_pcmvpp, badge4_cfvcc);

		ret = sa11xx_drv_pcmcia_probe(dev, &badge4_pcmcia_ops, 0, 2);
	}

	return ret;
}

static int __init pcmv_setup(char *s)
{
	int v[4];

	s = get_options(s, ARRAY_SIZE(v), v);

	if (v[0] >= 1) badge4_pcmvcc = v[1];
	if (v[0] >= 2) badge4_pcmvpp = v[2];
	if (v[0] >= 3) badge4_cfvcc = v[3];

	return 1;
}

__setup("pcmv=", pcmv_setup);
