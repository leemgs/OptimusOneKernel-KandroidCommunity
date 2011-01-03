
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <mach/irqs.h>

static struct i2c_board_info __initdata bus0_i2c_board_info[] = {
	{
		.type = "ab3100",
		.addr = 0x48,
		.irq = IRQ_U300_IRQ0_EXT,
	},
};

static struct i2c_board_info __initdata bus1_i2c_board_info[] = {
#ifdef CONFIG_MACH_U300_BS335
	{
		.type = "fwcam",
		.addr = 0x10,
	},
	{
		.type = "fwcam",
		.addr = 0x5d,
	},
#else
	{ },
#endif
};

void __init u300_i2c_register_board_devices(void)
{
	i2c_register_board_info(0, bus0_i2c_board_info,
				ARRAY_SIZE(bus0_i2c_board_info));
	i2c_register_board_info(1, bus1_i2c_board_info,
				ARRAY_SIZE(bus1_i2c_board_info));
}
