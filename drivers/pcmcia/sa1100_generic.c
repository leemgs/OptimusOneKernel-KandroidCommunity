

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <pcmcia/cs_types.h>
#include <pcmcia/cs.h>
#include <pcmcia/ss.h>

#include <asm/hardware/scoop.h>

#include "sa1100_generic.h"

int __init pcmcia_collie_init(struct device *dev);

static int (*sa11x0_pcmcia_hw_init[])(struct device *dev) = {
#ifdef CONFIG_SA1100_ASSABET
	pcmcia_assabet_init,
#endif
#ifdef CONFIG_SA1100_CERF
	pcmcia_cerf_init,
#endif
#ifdef CONFIG_SA1100_H3600
	pcmcia_h3600_init,
#endif
#ifdef CONFIG_SA1100_SHANNON
	pcmcia_shannon_init,
#endif
#ifdef CONFIG_SA1100_SIMPAD
	pcmcia_simpad_init,
#endif
#ifdef CONFIG_SA1100_COLLIE
       pcmcia_collie_init,
#endif
};

static int sa11x0_drv_pcmcia_probe(struct platform_device *dev)
{
	int i, ret = -ENODEV;

	
	for (i = 0; i < ARRAY_SIZE(sa11x0_pcmcia_hw_init); i++) {
		ret = sa11x0_pcmcia_hw_init[i](&dev->dev);
		if (ret == 0)
			break;
	}

	return ret;
}

static int sa11x0_drv_pcmcia_remove(struct platform_device *dev)
{
	return soc_common_drv_pcmcia_remove(&dev->dev);
}

static int sa11x0_drv_pcmcia_suspend(struct platform_device *dev,
				     pm_message_t state)
{
	return pcmcia_socket_dev_suspend(&dev->dev);
}

static int sa11x0_drv_pcmcia_resume(struct platform_device *dev)
{
	return pcmcia_socket_dev_resume(&dev->dev);
}

static struct platform_driver sa11x0_pcmcia_driver = {
	.driver = {
		.name		= "sa11x0-pcmcia",
		.owner		= THIS_MODULE,
	},
	.probe		= sa11x0_drv_pcmcia_probe,
	.remove		= sa11x0_drv_pcmcia_remove,
	.suspend 	= sa11x0_drv_pcmcia_suspend,
	.resume 	= sa11x0_drv_pcmcia_resume,
};


static int __init sa11x0_pcmcia_init(void)
{
	return platform_driver_register(&sa11x0_pcmcia_driver);
}


static void __exit sa11x0_pcmcia_exit(void)
{
	platform_driver_unregister(&sa11x0_pcmcia_driver);
}

MODULE_AUTHOR("John Dorsey <john+@cs.cmu.edu>");
MODULE_DESCRIPTION("Linux PCMCIA Card Services: SA-11x0 Socket Controller");
MODULE_LICENSE("Dual MPL/GPL");

fs_initcall(sa11x0_pcmcia_init);
module_exit(sa11x0_pcmcia_exit);
