
#include <linux/init.h>
#include <linux/module.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/of_platform.h>
#include <linux/nwpserial.h>

#include <asm/prom.h>

struct of_serial_info {
	int type;
	int line;
};


static int __devinit of_platform_serial_setup(struct of_device *ofdev,
					int type, struct uart_port *port)
{
	struct resource resource;
	struct device_node *np = ofdev->node;
	const unsigned int *clk, *spd;
	const u32 *prop;
	int ret, prop_size;

	memset(port, 0, sizeof *port);
	spd = of_get_property(np, "current-speed", NULL);
	clk = of_get_property(np, "clock-frequency", NULL);
	if (!clk) {
		dev_warn(&ofdev->dev, "no clock-frequency property set\n");
		return -ENODEV;
	}

	ret = of_address_to_resource(np, 0, &resource);
	if (ret) {
		dev_warn(&ofdev->dev, "invalid address\n");
		return ret;
	}

	spin_lock_init(&port->lock);
	port->mapbase = resource.start;

	
	prop = of_get_property(np, "reg-offset", &prop_size);
	if (prop && (prop_size == sizeof(u32)))
		port->mapbase += *prop;

	
	prop = of_get_property(np, "reg-shift", &prop_size);
	if (prop && (prop_size == sizeof(u32)))
		port->regshift = *prop;

	port->irq = irq_of_parse_and_map(np, 0);
	port->iotype = UPIO_MEM;
	port->type = type;
	port->uartclk = *clk;
	port->flags = UPF_SHARE_IRQ | UPF_BOOT_AUTOCONF | UPF_IOREMAP
		| UPF_FIXED_PORT | UPF_FIXED_TYPE;
	port->dev = &ofdev->dev;
	
	if (spd)
		port->custom_divisor = *clk / (16 * (*spd));

	return 0;
}


static int __devinit of_platform_serial_probe(struct of_device *ofdev,
						const struct of_device_id *id)
{
	struct of_serial_info *info;
	struct uart_port port;
	int port_type;
	int ret;

	if (of_find_property(ofdev->node, "used-by-rtas", NULL))
		return -EBUSY;

	info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;

	port_type = (unsigned long)id->data;
	ret = of_platform_serial_setup(ofdev, port_type, &port);
	if (ret)
		goto out;

	switch (port_type) {
#ifdef CONFIG_SERIAL_8250
	case PORT_8250 ... PORT_MAX_8250:
		ret = serial8250_register_port(&port);
		break;
#endif
#ifdef CONFIG_SERIAL_OF_PLATFORM_NWPSERIAL
	case PORT_NWPSERIAL:
		ret = nwpserial_register_port(&port);
		break;
#endif
	default:
		
	case PORT_UNKNOWN:
		dev_info(&ofdev->dev, "Unknown serial port found, ignored\n");
		ret = -ENODEV;
		break;
	}
	if (ret < 0)
		goto out;

	info->type = port_type;
	info->line = ret;
	dev_set_drvdata(&ofdev->dev, info);
	return 0;
out:
	kfree(info);
	irq_dispose_mapping(port.irq);
	return ret;
}


static int of_platform_serial_remove(struct of_device *ofdev)
{
	struct of_serial_info *info = dev_get_drvdata(&ofdev->dev);
	switch (info->type) {
#ifdef CONFIG_SERIAL_8250
	case PORT_8250 ... PORT_MAX_8250:
		serial8250_unregister_port(info->line);
		break;
#endif
#ifdef CONFIG_SERIAL_OF_PLATFORM_NWPSERIAL
	case PORT_NWPSERIAL:
		nwpserial_unregister_port(info->line);
		break;
#endif
	default:
		
		break;
	}
	kfree(info);
	return 0;
}


static struct of_device_id __devinitdata of_platform_serial_table[] = {
	{ .type = "serial", .compatible = "ns8250",   .data = (void *)PORT_8250, },
	{ .type = "serial", .compatible = "ns16450",  .data = (void *)PORT_16450, },
	{ .type = "serial", .compatible = "ns16550a", .data = (void *)PORT_16550A, },
	{ .type = "serial", .compatible = "ns16550",  .data = (void *)PORT_16550, },
	{ .type = "serial", .compatible = "ns16750",  .data = (void *)PORT_16750, },
	{ .type = "serial", .compatible = "ns16850",  .data = (void *)PORT_16850, },
#ifdef CONFIG_SERIAL_OF_PLATFORM_NWPSERIAL
	{ .type = "serial", .compatible = "ibm,qpace-nwp-serial",
					.data = (void *)PORT_NWPSERIAL, },
#endif
	{ .type = "serial",			      .data = (void *)PORT_UNKNOWN, },
	{  },
};

static struct of_platform_driver of_platform_serial_driver = {
	.owner = THIS_MODULE,
	.name = "of_serial",
	.probe = of_platform_serial_probe,
	.remove = of_platform_serial_remove,
	.match_table = of_platform_serial_table,
};

static int __init of_platform_serial_init(void)
{
	return of_register_platform_driver(&of_platform_serial_driver);
}
module_init(of_platform_serial_init);

static void __exit of_platform_serial_exit(void)
{
	return of_unregister_platform_driver(&of_platform_serial_driver);
};
module_exit(of_platform_serial_exit);

MODULE_AUTHOR("Arnd Bergmann <arnd@arndb.de>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Serial Port driver for Open Firmware platform devices");
