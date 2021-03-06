

#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <mach/spitz.h>
#include <mach/pxa2xx-gpio.h>

#define KB_ROWS			7
#define KB_COLS			11
#define KB_ROWMASK(r)		(1 << (r))
#define SCANCODE(r,c)		(((r)<<4) + (c) + 1)
#define	NR_SCANCODES		((KB_ROWS<<4) + 1)

#define SCAN_INTERVAL		(50) 
#define HINGE_SCAN_INTERVAL	(150) 

#define SPITZ_KEY_CALENDER	KEY_F1
#define SPITZ_KEY_ADDRESS	KEY_F2
#define SPITZ_KEY_FN		KEY_F3
#define SPITZ_KEY_CANCEL	KEY_F4
#define SPITZ_KEY_EXOK		KEY_F5
#define SPITZ_KEY_EXCANCEL	KEY_F6
#define SPITZ_KEY_EXJOGDOWN	KEY_F7
#define SPITZ_KEY_EXJOGUP	KEY_F8
#define SPITZ_KEY_JAP1		KEY_LEFTALT
#define SPITZ_KEY_JAP2		KEY_RIGHTCTRL
#define SPITZ_KEY_SYNC		KEY_F9
#define SPITZ_KEY_MAIL		KEY_F10
#define SPITZ_KEY_OK		KEY_F11
#define SPITZ_KEY_MENU		KEY_F12

static unsigned char spitzkbd_keycode[NR_SCANCODES] = {
	0,                                                                                                                
	KEY_LEFTCTRL, KEY_1, KEY_3, KEY_5, KEY_6, KEY_7, KEY_9, KEY_0, KEY_BACKSPACE, SPITZ_KEY_EXOK, SPITZ_KEY_EXCANCEL, 0, 0, 0, 0, 0,  
	0, KEY_2, KEY_4, KEY_R, KEY_Y, KEY_8, KEY_I, KEY_O, KEY_P, SPITZ_KEY_EXJOGDOWN, SPITZ_KEY_EXJOGUP, 0, 0, 0, 0, 0, 
	KEY_TAB, KEY_Q, KEY_E, KEY_T, KEY_G, KEY_U, KEY_J, KEY_K, 0, 0, 0, 0, 0, 0, 0, 0,                                 
	SPITZ_KEY_ADDRESS, KEY_W, KEY_S, KEY_F, KEY_V, KEY_H, KEY_M, KEY_L, 0, KEY_RIGHTSHIFT, 0, 0, 0, 0, 0, 0,         
	SPITZ_KEY_CALENDER, KEY_A, KEY_D, KEY_C, KEY_B, KEY_N, KEY_DOT, 0, KEY_ENTER, KEY_LEFTSHIFT, 0, 0, 0, 0, 0, 0,	  
	SPITZ_KEY_MAIL, KEY_Z, KEY_X, KEY_MINUS, KEY_SPACE, KEY_COMMA, 0, KEY_UP, 0, 0, SPITZ_KEY_FN, 0, 0, 0, 0, 0,      
	KEY_SYSRQ, SPITZ_KEY_JAP1, SPITZ_KEY_JAP2, SPITZ_KEY_CANCEL, SPITZ_KEY_OK, SPITZ_KEY_MENU, KEY_LEFT, KEY_DOWN, KEY_RIGHT, 0, 0, 0, 0, 0, 0, 0  
};

static int spitz_strobes[] = {
	SPITZ_GPIO_KEY_STROBE0,
	SPITZ_GPIO_KEY_STROBE1,
	SPITZ_GPIO_KEY_STROBE2,
	SPITZ_GPIO_KEY_STROBE3,
	SPITZ_GPIO_KEY_STROBE4,
	SPITZ_GPIO_KEY_STROBE5,
	SPITZ_GPIO_KEY_STROBE6,
	SPITZ_GPIO_KEY_STROBE7,
	SPITZ_GPIO_KEY_STROBE8,
	SPITZ_GPIO_KEY_STROBE9,
	SPITZ_GPIO_KEY_STROBE10,
};

static int spitz_senses[] = {
	SPITZ_GPIO_KEY_SENSE0,
	SPITZ_GPIO_KEY_SENSE1,
	SPITZ_GPIO_KEY_SENSE2,
	SPITZ_GPIO_KEY_SENSE3,
	SPITZ_GPIO_KEY_SENSE4,
	SPITZ_GPIO_KEY_SENSE5,
	SPITZ_GPIO_KEY_SENSE6,
};

struct spitzkbd {
	unsigned char keycode[ARRAY_SIZE(spitzkbd_keycode)];
	struct input_dev *input;
	char phys[32];

	spinlock_t lock;
	struct timer_list timer;
	struct timer_list htimer;

	unsigned int suspended;
	unsigned long suspend_jiffies;
};

#define KB_DISCHARGE_DELAY	10
#define KB_ACTIVATE_DELAY	10


static inline void spitzkbd_discharge_all(void)
{
	
	GPCR0  =  SPITZ_GPIO_G0_STROBE_BIT;
	GPDR0 &= ~SPITZ_GPIO_G0_STROBE_BIT;
	GPCR1  =  SPITZ_GPIO_G1_STROBE_BIT;
	GPDR1 &= ~SPITZ_GPIO_G1_STROBE_BIT;
	GPCR2  =  SPITZ_GPIO_G2_STROBE_BIT;
	GPDR2 &= ~SPITZ_GPIO_G2_STROBE_BIT;
	GPCR3  =  SPITZ_GPIO_G3_STROBE_BIT;
	GPDR3 &= ~SPITZ_GPIO_G3_STROBE_BIT;
}

static inline void spitzkbd_activate_all(void)
{
	
	GPSR0  =  SPITZ_GPIO_G0_STROBE_BIT;
	GPDR0 |=  SPITZ_GPIO_G0_STROBE_BIT;
	GPSR1  =  SPITZ_GPIO_G1_STROBE_BIT;
	GPDR1 |=  SPITZ_GPIO_G1_STROBE_BIT;
	GPSR2  =  SPITZ_GPIO_G2_STROBE_BIT;
	GPDR2 |=  SPITZ_GPIO_G2_STROBE_BIT;
	GPSR3  =  SPITZ_GPIO_G3_STROBE_BIT;
	GPDR3 |=  SPITZ_GPIO_G3_STROBE_BIT;

	udelay(KB_DISCHARGE_DELAY);

	
	GEDR0 = SPITZ_GPIO_G0_SENSE_BIT;
	GEDR1 = SPITZ_GPIO_G1_SENSE_BIT;
	GEDR2 = SPITZ_GPIO_G2_SENSE_BIT;
	GEDR3 = SPITZ_GPIO_G3_SENSE_BIT;
}

static inline void spitzkbd_activate_col(int col)
{
	int gpio = spitz_strobes[col];
	GPDR0 &= ~SPITZ_GPIO_G0_STROBE_BIT;
	GPDR1 &= ~SPITZ_GPIO_G1_STROBE_BIT;
	GPDR2 &= ~SPITZ_GPIO_G2_STROBE_BIT;
	GPDR3 &= ~SPITZ_GPIO_G3_STROBE_BIT;
	GPSR(gpio) = GPIO_bit(gpio);
	GPDR(gpio) |= GPIO_bit(gpio);
}

static inline void spitzkbd_reset_col(int col)
{
	int gpio = spitz_strobes[col];
	GPDR0 &= ~SPITZ_GPIO_G0_STROBE_BIT;
	GPDR1 &= ~SPITZ_GPIO_G1_STROBE_BIT;
	GPDR2 &= ~SPITZ_GPIO_G2_STROBE_BIT;
	GPDR3 &= ~SPITZ_GPIO_G3_STROBE_BIT;
	GPCR(gpio) = GPIO_bit(gpio);
	GPDR(gpio) |= GPIO_bit(gpio);
}

static inline int spitzkbd_get_row_status(int col)
{
	return ((GPLR0 >> 12) & 0x01) | ((GPLR0 >> 16) & 0x02)
		| ((GPLR2 >> 25) & 0x04) | ((GPLR1 << 1) & 0x08)
		| ((GPLR1 >> 0) & 0x10) | ((GPLR1 >> 1) & 0x60);
}




static void spitzkbd_scankeyboard(struct spitzkbd *spitzkbd_data)
{
	unsigned int row, col, rowd;
	unsigned long flags;
	unsigned int num_pressed, pwrkey = ((GPLR(SPITZ_GPIO_ON_KEY) & GPIO_bit(SPITZ_GPIO_ON_KEY)) != 0);

	if (spitzkbd_data->suspended)
		return;

	spin_lock_irqsave(&spitzkbd_data->lock, flags);

	num_pressed = 0;
	for (col = 0; col < KB_COLS; col++) {
		

		spitzkbd_discharge_all();
		udelay(KB_DISCHARGE_DELAY);

		spitzkbd_activate_col(col);
		udelay(KB_ACTIVATE_DELAY);

		rowd = spitzkbd_get_row_status(col);
		for (row = 0; row < KB_ROWS; row++) {
			unsigned int scancode, pressed;

			scancode = SCANCODE(row, col);
			pressed = rowd & KB_ROWMASK(row);

			input_report_key(spitzkbd_data->input, spitzkbd_data->keycode[scancode], pressed);

			if (pressed)
				num_pressed++;
		}
		spitzkbd_reset_col(col);
	}

	spitzkbd_activate_all();

	input_report_key(spitzkbd_data->input, SPITZ_KEY_SYNC, (GPLR(SPITZ_GPIO_SYNC) & GPIO_bit(SPITZ_GPIO_SYNC)) != 0 );
	input_report_key(spitzkbd_data->input, KEY_SUSPEND, pwrkey);

	if (pwrkey && time_after(jiffies, spitzkbd_data->suspend_jiffies + msecs_to_jiffies(1000))) {
		input_event(spitzkbd_data->input, EV_PWR, KEY_SUSPEND, 1);
		spitzkbd_data->suspend_jiffies = jiffies;
	}

	input_sync(spitzkbd_data->input);

	
	if (num_pressed)
		mod_timer(&spitzkbd_data->timer, jiffies + msecs_to_jiffies(SCAN_INTERVAL));

	spin_unlock_irqrestore(&spitzkbd_data->lock, flags);
}


static irqreturn_t spitzkbd_interrupt(int irq, void *dev_id)
{
	struct spitzkbd *spitzkbd_data = dev_id;

	if (!timer_pending(&spitzkbd_data->timer)) {
		
		udelay(20);
		spitzkbd_scankeyboard(spitzkbd_data);
	}

	return IRQ_HANDLED;
}


static void spitzkbd_timer_callback(unsigned long data)
{
	struct spitzkbd *spitzkbd_data = (struct spitzkbd *) data;

	spitzkbd_scankeyboard(spitzkbd_data);
}



static irqreturn_t spitzkbd_hinge_isr(int irq, void *dev_id)
{
	struct spitzkbd *spitzkbd_data = dev_id;

	if (!timer_pending(&spitzkbd_data->htimer))
		mod_timer(&spitzkbd_data->htimer, jiffies + msecs_to_jiffies(HINGE_SCAN_INTERVAL));

	return IRQ_HANDLED;
}

#define HINGE_STABLE_COUNT 2
static int sharpsl_hinge_state;
static int hinge_count;

static void spitzkbd_hinge_timer(unsigned long data)
{
	struct spitzkbd *spitzkbd_data = (struct spitzkbd *) data;
	unsigned long state;
	unsigned long flags;

	state = GPLR(SPITZ_GPIO_SWA) & (GPIO_bit(SPITZ_GPIO_SWA)|GPIO_bit(SPITZ_GPIO_SWB));
	state |= (GPLR(SPITZ_GPIO_AK_INT) & GPIO_bit(SPITZ_GPIO_AK_INT));
	if (state != sharpsl_hinge_state) {
		hinge_count = 0;
		sharpsl_hinge_state = state;
	} else if (hinge_count < HINGE_STABLE_COUNT) {
		hinge_count++;
	}

	if (hinge_count >= HINGE_STABLE_COUNT) {
		spin_lock_irqsave(&spitzkbd_data->lock, flags);

		input_report_switch(spitzkbd_data->input, SW_LID, ((GPLR(SPITZ_GPIO_SWA) & GPIO_bit(SPITZ_GPIO_SWA)) != 0));
		input_report_switch(spitzkbd_data->input, SW_TABLET_MODE, ((GPLR(SPITZ_GPIO_SWB) & GPIO_bit(SPITZ_GPIO_SWB)) != 0));
		input_report_switch(spitzkbd_data->input, SW_HEADPHONE_INSERT, ((GPLR(SPITZ_GPIO_AK_INT) & GPIO_bit(SPITZ_GPIO_AK_INT)) != 0));
		input_sync(spitzkbd_data->input);

		spin_unlock_irqrestore(&spitzkbd_data->lock, flags);
	} else {
		mod_timer(&spitzkbd_data->htimer, jiffies + msecs_to_jiffies(HINGE_SCAN_INTERVAL));
	}
}

#ifdef CONFIG_PM
static int spitzkbd_suspend(struct platform_device *dev, pm_message_t state)
{
	int i;
	struct spitzkbd *spitzkbd = platform_get_drvdata(dev);
	spitzkbd->suspended = 1;

	
	for (i = 1; i < SPITZ_KEY_STROBE_NUM; i++)
		pxa_gpio_mode(spitz_strobes[i] | GPIO_IN);

	return 0;
}

static int spitzkbd_resume(struct platform_device *dev)
{
	int i;
	struct spitzkbd *spitzkbd = platform_get_drvdata(dev);

	for (i = 0; i < SPITZ_KEY_STROBE_NUM; i++)
		pxa_gpio_mode(spitz_strobes[i] | GPIO_OUT | GPIO_DFLT_HIGH);

	
	spitzkbd->suspend_jiffies = jiffies;
	spitzkbd->suspended = 0;

	return 0;
}
#else
#define spitzkbd_suspend	NULL
#define spitzkbd_resume		NULL
#endif

static int __devinit spitzkbd_probe(struct platform_device *dev)
{
	struct spitzkbd *spitzkbd;
	struct input_dev *input_dev;
	int i, err = -ENOMEM;

	spitzkbd = kzalloc(sizeof(struct spitzkbd), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!spitzkbd || !input_dev)
		goto fail;

	platform_set_drvdata(dev, spitzkbd);
	strcpy(spitzkbd->phys, "spitzkbd/input0");

	spin_lock_init(&spitzkbd->lock);

	
	init_timer(&spitzkbd->timer);
	spitzkbd->timer.function = spitzkbd_timer_callback;
	spitzkbd->timer.data = (unsigned long) spitzkbd;

	
	init_timer(&spitzkbd->htimer);
	spitzkbd->htimer.function = spitzkbd_hinge_timer;
	spitzkbd->htimer.data = (unsigned long) spitzkbd;

	spitzkbd->suspend_jiffies = jiffies;

	spitzkbd->input = input_dev;

	input_dev->name = "Spitz Keyboard";
	input_dev->phys = spitzkbd->phys;
	input_dev->dev.parent = &dev->dev;

	input_dev->id.bustype = BUS_HOST;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0100;

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP) |
		BIT_MASK(EV_PWR) | BIT_MASK(EV_SW);
	input_dev->keycode = spitzkbd->keycode;
	input_dev->keycodesize = sizeof(unsigned char);
	input_dev->keycodemax = ARRAY_SIZE(spitzkbd_keycode);

	memcpy(spitzkbd->keycode, spitzkbd_keycode, sizeof(spitzkbd->keycode));
	for (i = 0; i < ARRAY_SIZE(spitzkbd_keycode); i++)
		set_bit(spitzkbd->keycode[i], input_dev->keybit);
	clear_bit(0, input_dev->keybit);
	set_bit(KEY_SUSPEND, input_dev->keybit);
	set_bit(SW_LID, input_dev->swbit);
	set_bit(SW_TABLET_MODE, input_dev->swbit);
	set_bit(SW_HEADPHONE_INSERT, input_dev->swbit);

	err = input_register_device(input_dev);
	if (err)
		goto fail;

	mod_timer(&spitzkbd->htimer, jiffies + msecs_to_jiffies(HINGE_SCAN_INTERVAL));

	
	for (i = 0; i < SPITZ_KEY_SENSE_NUM; i++) {
		pxa_gpio_mode(spitz_senses[i] | GPIO_IN);
		if (request_irq(IRQ_GPIO(spitz_senses[i]), spitzkbd_interrupt,
				IRQF_DISABLED|IRQF_TRIGGER_RISING,
				"Spitzkbd Sense", spitzkbd))
			printk(KERN_WARNING "spitzkbd: Can't get Sense IRQ: %d!\n", i);
	}

	
	for (i = 0; i < SPITZ_KEY_STROBE_NUM; i++)
		pxa_gpio_mode(spitz_strobes[i] | GPIO_OUT | GPIO_DFLT_HIGH);

	pxa_gpio_mode(SPITZ_GPIO_SYNC | GPIO_IN);
	pxa_gpio_mode(SPITZ_GPIO_ON_KEY | GPIO_IN);
	pxa_gpio_mode(SPITZ_GPIO_SWA | GPIO_IN);
	pxa_gpio_mode(SPITZ_GPIO_SWB | GPIO_IN);

	request_irq(SPITZ_IRQ_GPIO_SYNC, spitzkbd_interrupt,
		    IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		    "Spitzkbd Sync", spitzkbd);
	request_irq(SPITZ_IRQ_GPIO_ON_KEY, spitzkbd_interrupt,
		    IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		    "Spitzkbd PwrOn", spitzkbd);
	request_irq(SPITZ_IRQ_GPIO_SWA, spitzkbd_hinge_isr,
		    IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		    "Spitzkbd SWA", spitzkbd);
	request_irq(SPITZ_IRQ_GPIO_SWB, spitzkbd_hinge_isr,
		    IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		    "Spitzkbd SWB", spitzkbd);
	request_irq(SPITZ_IRQ_GPIO_AK_INT, spitzkbd_hinge_isr,
		    IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
		    "Spitzkbd HP", spitzkbd);

	return 0;

 fail:	input_free_device(input_dev);
	kfree(spitzkbd);
	return err;
}

static int __devexit spitzkbd_remove(struct platform_device *dev)
{
	int i;
	struct spitzkbd *spitzkbd = platform_get_drvdata(dev);

	for (i = 0; i < SPITZ_KEY_SENSE_NUM; i++)
		free_irq(IRQ_GPIO(spitz_senses[i]), spitzkbd);

	free_irq(SPITZ_IRQ_GPIO_SYNC, spitzkbd);
	free_irq(SPITZ_IRQ_GPIO_ON_KEY, spitzkbd);
	free_irq(SPITZ_IRQ_GPIO_SWA, spitzkbd);
	free_irq(SPITZ_IRQ_GPIO_SWB, spitzkbd);
	free_irq(SPITZ_IRQ_GPIO_AK_INT, spitzkbd);

	del_timer_sync(&spitzkbd->htimer);
	del_timer_sync(&spitzkbd->timer);

	input_unregister_device(spitzkbd->input);

	kfree(spitzkbd);

	return 0;
}

static struct platform_driver spitzkbd_driver = {
	.probe		= spitzkbd_probe,
	.remove		= __devexit_p(spitzkbd_remove),
	.suspend	= spitzkbd_suspend,
	.resume		= spitzkbd_resume,
	.driver		= {
		.name	= "spitz-keyboard",
		.owner	= THIS_MODULE,
	},
};

static int __init spitzkbd_init(void)
{
	return platform_driver_register(&spitzkbd_driver);
}

static void __exit spitzkbd_exit(void)
{
	platform_driver_unregister(&spitzkbd_driver);
}

module_init(spitzkbd_init);
module_exit(spitzkbd_exit);

MODULE_AUTHOR("Richard Purdie <rpurdie@rpsys.net>");
MODULE_DESCRIPTION("Spitz Keyboard Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:spitz-keyboard");
