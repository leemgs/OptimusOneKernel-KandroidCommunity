
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>

#include <linux/i2c/dm355evm_msp.h>



struct dm355evm_keys {
	struct input_dev	*input;
	struct device		*dev;
	int			irq;
};


static struct {
	u16	event;
	u16	keycode;
} dm355evm_keys[] = {

	
	{ 0x00d8, KEY_OK, },		
	{ 0x00b8, KEY_UP, },		
	{ 0x00e8, KEY_DOWN, },		
	{ 0x0078, KEY_LEFT, },		
	{ 0x00f0, KEY_RIGHT, },		

	
	{ 0x300c, KEY_POWER, },		
	{ 0x3000, KEY_NUMERIC_0, },
	{ 0x3001, KEY_NUMERIC_1, },
	{ 0x3002, KEY_NUMERIC_2, },
	{ 0x3003, KEY_NUMERIC_3, },
	{ 0x3004, KEY_NUMERIC_4, },
	{ 0x3005, KEY_NUMERIC_5, },
	{ 0x3006, KEY_NUMERIC_6, },
	{ 0x3007, KEY_NUMERIC_7, },
	{ 0x3008, KEY_NUMERIC_8, },
	{ 0x3009, KEY_NUMERIC_9, },
	{ 0x3022, KEY_ENTER, },
	{ 0x30ec, KEY_MODE, },		
	{ 0x300f, KEY_SELECT, },	
	{ 0x3020, KEY_CHANNELUP, },	
	{ 0x302e, KEY_MENU, },		
	{ 0x3011, KEY_VOLUMEDOWN, },	
	{ 0x300d, KEY_MUTE, },		
	{ 0x3010, KEY_VOLUMEUP, },	
	{ 0x301e, KEY_SUBTITLE, },	
	{ 0x3021, KEY_CHANNELDOWN, },	
	{ 0x3022, KEY_PREVIOUS, },
	{ 0x3026, KEY_SLEEP, },
	{ 0x3172, KEY_REWIND, },	
	{ 0x3175, KEY_PLAY, },
	{ 0x3174, KEY_FASTFORWARD, },
	{ 0x3177, KEY_RECORD, },
	{ 0x3176, KEY_STOP, },
	{ 0x3169, KEY_PAUSE, },
};


static irqreturn_t dm355evm_keys_irq(int irq, void *_keys)
{
	struct dm355evm_keys	*keys = _keys;
	int			status;

	
	for (;;) {
		static u16	last_event;
		u16		event;
		int		keycode;
		int		i;

		status = dm355evm_msp_read(DM355EVM_MSP_INPUT_HIGH);
		if (status < 0) {
			dev_dbg(keys->dev, "input high err %d\n",
					status);
			break;
		}
		event = status << 8;

		status = dm355evm_msp_read(DM355EVM_MSP_INPUT_LOW);
		if (status < 0) {
			dev_dbg(keys->dev, "input low err %d\n",
					status);
			break;
		}
		event |= status;
		if (event == 0xdead)
			break;

		
		if (event == last_event) {
			last_event = 0;
			continue;
		}
		last_event = event;

		
		event &= ~0x0800;

		
		keycode = KEY_UNKNOWN;
		for (i = 0; i < ARRAY_SIZE(dm355evm_keys); i++) {
			if (dm355evm_keys[i].event != event)
				continue;
			keycode = dm355evm_keys[i].keycode;
			break;
		}
		dev_dbg(keys->dev,
			"input event 0x%04x--> keycode %d\n",
			event, keycode);

		
		input_report_key(keys->input, keycode, 1);
		input_sync(keys->input);
		input_report_key(keys->input, keycode, 0);
		input_sync(keys->input);
	}
	return IRQ_HANDLED;
}

static int dm355evm_setkeycode(struct input_dev *dev, int index, int keycode)
{
	u16		old_keycode;
	unsigned	i;

	if (((unsigned)index) >= ARRAY_SIZE(dm355evm_keys))
		return -EINVAL;

	old_keycode = dm355evm_keys[index].keycode;
	dm355evm_keys[index].keycode = keycode;
	set_bit(keycode, dev->keybit);

	for (i = 0; i < ARRAY_SIZE(dm355evm_keys); i++) {
		if (dm355evm_keys[index].keycode == old_keycode)
			goto done;
	}
	clear_bit(old_keycode, dev->keybit);
done:
	return 0;
}

static int dm355evm_getkeycode(struct input_dev *dev, int index, int *keycode)
{
	if (((unsigned)index) >= ARRAY_SIZE(dm355evm_keys))
		return -EINVAL;

	return dm355evm_keys[index].keycode;
}



static int __devinit dm355evm_keys_probe(struct platform_device *pdev)
{
	struct dm355evm_keys	*keys;
	struct input_dev	*input;
	int			status;
	int			i;

	
	keys = kzalloc(sizeof *keys, GFP_KERNEL);
	input = input_allocate_device();
	if (!keys || !input) {
		status = -ENOMEM;
		goto fail1;
	}

	keys->dev = &pdev->dev;
	keys->input = input;

	
	status = platform_get_irq(pdev, 0);
	if (status < 0)
		goto fail1;
	keys->irq = status;

	input_set_drvdata(input, keys);

	input->name = "DM355 EVM Controls";
	input->phys = "dm355evm/input0";
	input->dev.parent = &pdev->dev;

	input->id.bustype = BUS_I2C;
	input->id.product = 0x0355;
	input->id.version = dm355evm_msp_read(DM355EVM_MSP_FIRMREV);

	input->evbit[0] = BIT(EV_KEY);
	for (i = 0; i < ARRAY_SIZE(dm355evm_keys); i++)
		__set_bit(dm355evm_keys[i].keycode, input->keybit);

	input->setkeycode = dm355evm_setkeycode;
	input->getkeycode = dm355evm_getkeycode;

	

	status = request_threaded_irq(keys->irq, NULL, dm355evm_keys_irq,
			IRQF_TRIGGER_FALLING, dev_name(&pdev->dev), keys);
	if (status < 0)
		goto fail1;

	
	status = input_register_device(input);
	if (status < 0)
		goto fail2;

	platform_set_drvdata(pdev, keys);

	return 0;

fail2:
	free_irq(keys->irq, keys);
fail1:
	input_free_device(input);
	kfree(keys);
	dev_err(&pdev->dev, "can't register, err %d\n", status);

	return status;
}

static int __devexit dm355evm_keys_remove(struct platform_device *pdev)
{
	struct dm355evm_keys	*keys = platform_get_drvdata(pdev);

	free_irq(keys->irq, keys);
	input_unregister_device(keys->input);
	kfree(keys);

	return 0;
}




static struct platform_driver dm355evm_keys_driver = {
	.probe		= dm355evm_keys_probe,
	.remove		= __devexit_p(dm355evm_keys_remove),
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "dm355evm_keys",
	},
};

static int __init dm355evm_keys_init(void)
{
	return platform_driver_register(&dm355evm_keys_driver);
}
module_init(dm355evm_keys_init);

static void __exit dm355evm_keys_exit(void)
{
	platform_driver_unregister(&dm355evm_keys_driver);
}
module_exit(dm355evm_keys_exit);

MODULE_LICENSE("GPL");
