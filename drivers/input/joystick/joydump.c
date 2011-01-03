





#include <linux/module.h>
#include <linux/gameport.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/slab.h>

#define DRIVER_DESC	"Gameport data dumper module"

MODULE_AUTHOR("Vojtech Pavlik <vojtech@ucw.cz>");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

#define BUF_SIZE 256

struct joydump {
	unsigned int time;
	unsigned char data;
};

static int joydump_connect(struct gameport *gameport, struct gameport_driver *drv)
{
	struct joydump *buf;	
	struct joydump *dump, *prev;	
	int axes[4], buttons;
	int i, j, t, timeout;
	unsigned long flags;
	unsigned char u;

	printk(KERN_INFO "joydump: ,------------------ START ----------------.\n");
	printk(KERN_INFO "joydump: | Dumping: %30s |\n", gameport->phys);
	printk(KERN_INFO "joydump: | Speed: %28d kHz |\n", gameport->speed);

	if (gameport_open(gameport, drv, GAMEPORT_MODE_RAW)) {

		printk(KERN_INFO "joydump: | Raw mode not available - trying cooked.    |\n");

		if (gameport_open(gameport, drv, GAMEPORT_MODE_COOKED)) {

			printk(KERN_INFO "joydump: | Cooked not available either. Failing.   |\n");
			printk(KERN_INFO "joydump: `------------------- END -----------------'\n");
			return -ENODEV;
		}

		gameport_cooked_read(gameport, axes, &buttons);

		for (i = 0; i < 4; i++)
			printk(KERN_INFO "joydump: | Axis %d: %4d.                           |\n", i, axes[i]);
		printk(KERN_INFO "joydump: | Buttons %02x.                             |\n", buttons);
		printk(KERN_INFO "joydump: `------------------- END -----------------'\n");
	}

	timeout = gameport_time(gameport, 10000); 

	buf = kmalloc(BUF_SIZE * sizeof(struct joydump), GFP_KERNEL);
	if (!buf) {
		printk(KERN_INFO "joydump: no memory for testing\n");
		goto jd_end;
	}
	dump = buf;
	t = 0;
	i = 1;

	local_irq_save(flags);

	u = gameport_read(gameport);

	dump->data = u;
	dump->time = t;
	dump++;

	gameport_trigger(gameport);

	while (i < BUF_SIZE && t < timeout) {

		dump->data = gameport_read(gameport);

		if (dump->data ^ u) {
			u = dump->data;
			dump->time = t;
			i++;
			dump++;
		}
		t++;
	}

	local_irq_restore(flags);



	t = i;
	dump = buf;
	prev = dump;

	printk(KERN_INFO "joydump: >------------------ DATA -----------------<\n");
	printk(KERN_INFO "joydump: | index: %3d delta: %3d us data: ", 0, 0);
	for (j = 7; j >= 0; j--)
		printk("%d", (dump->data >> j) & 1);
	printk(" |\n");
	dump++;

	for (i = 1; i < t; i++, dump++, prev++) {
		printk(KERN_INFO "joydump: | index: %3d delta: %3d us data: ",
			i, dump->time - prev->time);
		for (j = 7; j >= 0; j--)
			printk("%d", (dump->data >> j) & 1);
		printk(" |\n");
	}
	kfree(buf);

jd_end:
	printk(KERN_INFO "joydump: `------------------- END -----------------'\n");

	return 0;
}

static void joydump_disconnect(struct gameport *gameport)
{
	gameport_close(gameport);
}

static struct gameport_driver joydump_drv = {
	.driver		= {
		.name	= "joydump",
	},
	.description	= DRIVER_DESC,
	.connect	= joydump_connect,
	.disconnect	= joydump_disconnect,
};

static int __init joydump_init(void)
{
	return gameport_register_driver(&joydump_drv);
}

static void __exit joydump_exit(void)
{
	gameport_unregister_driver(&joydump_drv);
}

module_init(joydump_init);
module_exit(joydump_exit);
