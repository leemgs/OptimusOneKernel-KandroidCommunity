
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/i2c/sx150x.h>

struct sx150x_device_data {
	u8 reg_pullup;
	u8 reg_pulldn;
	u8 reg_drain;
	u8 reg_polarity;
	u8 reg_dir;
	u8 reg_data;
	u8 reg_irq_mask;
	u8 reg_irq_src;
	u8 reg_sense;
	u8 reg_clock;
	u8 reg_misc;
	u8 reg_reset;
	u8 ngpios;
};


#define CMD_MASK    0
#define CMD_UNMASK  1
#define CMD_RISING  2
#define CMD_FALLING 3
#define CMDS_PER_IO 4
#define NCMDBITS    (16 * CMDS_PER_IO)
#define CMD_BIT(pin, cmd) (((pin) * CMDS_PER_IO) + (cmd))

struct sx150x_chip {
	struct gpio_chip           gpio_chip;
	struct i2c_client         *client;
	struct sx150x_device_data *dev_cfg;
	struct mutex               mutex;
	int                        irq_summary;
	int                        irq_base;
	u32                        irq_sense;
	struct irq_chip            irq_chip;
	struct work_struct         irq_mask_ws;
	struct work_struct         irq_unmask_ws;
	struct work_struct         irq_set_type_ws;
	DECLARE_BITMAP(irq_cmds, NCMDBITS);
};

static struct sx150x_device_data sx150x_devices[] = {
	[0] = { 
		.reg_pullup   = 0x03,
		.reg_pulldn   = 0x04,
		.reg_drain    = 0x05,
		.reg_polarity = 0x06,
		.reg_dir      = 0x07,
		.reg_data     = 0x08,
		.reg_irq_mask = 0x09,
		.reg_irq_src  = 0x0c,
		.reg_sense    = 0x0b,
		.reg_clock    = 0x0f,
		.reg_misc     = 0x10,
		.reg_reset    = 0x7d,
		.ngpios       = 8
	},
	[1] = { 
		.reg_pullup   = 0x07,
		.reg_pulldn   = 0x09,
		.reg_drain    = 0x0b,
		.reg_polarity = 0x0d,
		.reg_dir      = 0x0f,
		.reg_data     = 0x11,
		.reg_irq_mask = 0x13,
		.reg_irq_src  = 0x19,
		.reg_sense    = 0x17,
		.reg_clock    = 0x1e,
		.reg_misc     = 0x1f,
		.reg_reset    = 0x7d,
		.ngpios       = 16
	},
};

static const struct i2c_device_id sx150x_id[] = {
	{"sx1508q", 0},
	{"sx1509q", 1},
	{}
};
MODULE_DEVICE_TABLE(i2c, sx150x_id);

static s32 sx150x_i2c_write(struct i2c_client *client, u8 reg, u8 val)
{
	s32 err = i2c_smbus_write_byte_data(client, reg, val);

	if (err < 0)
		dev_warn(&client->dev,
			"i2c write fail: can't write %02x to %02x: %d\n",
			val, reg, err);
	return err;
}

static s32 sx150x_i2c_read(struct i2c_client *client, u8 reg, u8 *val)
{
	s32 err = i2c_smbus_read_byte_data(client, reg);

	if (err >= 0)
		*val = err;
	else
		dev_warn(&client->dev,
			"i2c read fail: can't read from %02x: %d\n",
			reg, err);
	return err;
}

static inline bool offset_is_oscio(struct sx150x_chip *chip, unsigned offset)
{
	return (chip->dev_cfg->ngpios == offset);
}


static inline void sx150x_find_cfg(u8 offset, u8 width,
				u8 *reg, u8 *mask, u8 *shift)
{
	*reg   -= offset * width / 8;
	*mask   = (1 << width) - 1;
	*shift  = (offset * width) % 8;
	*mask <<= *shift;
}

static s32 sx150x_write_cfg(struct sx150x_chip *chip,
			u8 offset, u8 width, u8 reg, u8 val)
{
	u8  mask;
	u8  data;
	u8  shift;
	s32 err;

	sx150x_find_cfg(offset, width, &reg, &mask, &shift);
	err = sx150x_i2c_read(chip->client, reg, &data);
	if (err < 0)
		return err;

	data &= ~mask;
	data |= (val << shift) & mask;
	return sx150x_i2c_write(chip->client, reg, data);
}

static int sx150x_get_io(struct sx150x_chip *chip, unsigned offset)
{
	u8  reg = chip->dev_cfg->reg_data;
	u8  mask;
	u8  data;
	u8  shift;
	s32 err;

	sx150x_find_cfg(offset, 1, &reg, &mask, &shift);
	mutex_lock(&chip->mutex);
	err = sx150x_i2c_read(chip->client, reg, &data);
	mutex_unlock(&chip->mutex);
	if (err >= 0)
		err = (data & mask) != 0 ? 1 : 0;

	return err;
}

static void sx150x_set_oscio(struct sx150x_chip *chip, int val)
{
	mutex_lock(&chip->mutex);
	sx150x_i2c_write(chip->client,
			chip->dev_cfg->reg_clock,
			(val ? 0x1f : 0x10));
	mutex_unlock(&chip->mutex);
}

static void sx150x_set_io(struct sx150x_chip *chip, unsigned offset, int val)
{
	mutex_lock(&chip->mutex);
	sx150x_write_cfg(chip,
			offset,
			1,
			chip->dev_cfg->reg_data,
			(val ? 1 : 0));
	mutex_unlock(&chip->mutex);
}

static int sx150x_io_input(struct sx150x_chip *chip, unsigned offset)
{
	int err;

	mutex_lock(&chip->mutex);
	err = sx150x_write_cfg(chip,
			offset,
			1,
			chip->dev_cfg->reg_dir,
			1);
	mutex_unlock(&chip->mutex);
	return err;
}

static int sx150x_io_output(struct sx150x_chip *chip, unsigned offset, int val)
{
	int err;

	mutex_lock(&chip->mutex);
	err = sx150x_write_cfg(chip,
			offset,
			1,
			chip->dev_cfg->reg_data,
			(val ? 1 : 0));
	if (err >= 0)
		err = sx150x_write_cfg(chip,
				offset,
				1,
				chip->dev_cfg->reg_dir,
				0);
	mutex_unlock(&chip->mutex);
	return err;
}

static int sx150x_gpio_get(struct gpio_chip *gc, unsigned offset)
{
	struct sx150x_chip *chip;

	chip = container_of(gc, struct sx150x_chip, gpio_chip);

	if (offset_is_oscio(chip, offset))
		return -ENOTSUPP;
	else
		return sx150x_get_io(chip, offset);
}

static void sx150x_gpio_set(struct gpio_chip *gc, unsigned offset, int val)
{
	struct sx150x_chip *chip;

	chip = container_of(gc, struct sx150x_chip, gpio_chip);

	if (offset_is_oscio(chip, offset))
		sx150x_set_oscio(chip, val);
	else
		sx150x_set_io(chip, offset, val);
}

static int sx150x_gpio_direction_input(struct gpio_chip *gc, unsigned offset)
{
	struct sx150x_chip *chip;

	chip = container_of(gc, struct sx150x_chip, gpio_chip);

	if (offset_is_oscio(chip, offset))
		return -ENOTSUPP;
	else
		return sx150x_io_input(chip, offset);
}

static int sx150x_gpio_direction_output(struct gpio_chip *gc,
					unsigned offset,
					int val)
{
	struct sx150x_chip *chip;

	chip = container_of(gc, struct sx150x_chip, gpio_chip);

	if (offset_is_oscio(chip, offset))
		return 0;
	else
		return sx150x_io_output(chip, offset, val);
}

static int sx150x_gpio_to_irq(struct gpio_chip *gc, unsigned offset)
{
	struct sx150x_chip *chip;

	chip = container_of(gc, struct sx150x_chip, gpio_chip);

	if (offset >= chip->dev_cfg->ngpios)
		return -EINVAL;

	if (chip->irq_base < 0)
		return -ENOTSUPP;

	return chip->irq_base + offset;
}

static void sx150x_irq_ack(unsigned int irq)
{
}

static void sx150x_irq_mask(unsigned int irq)
{
	struct irq_chip *ic;
	struct sx150x_chip *chip;

	ic   = get_irq_chip(irq);
	chip = container_of(ic, struct sx150x_chip, irq_chip);

	set_bit(CMD_BIT(irq - chip->irq_base, CMD_MASK), chip->irq_cmds);
	schedule_work(&chip->irq_mask_ws);
}

static void sx150x_irq_mask_wfn(struct work_struct *work)
{
	struct sx150x_chip *chip;
	unsigned n;

	chip = container_of(work, struct sx150x_chip, irq_mask_ws);

	mutex_lock(&chip->mutex);
	for (n = 0; n < chip->dev_cfg->ngpios; ++n) {
		if (test_and_clear_bit(CMD_BIT(n, CMD_MASK),
					chip->irq_cmds)) {
			sx150x_write_cfg(chip, n, 1,
					chip->dev_cfg->reg_irq_mask, 1);
			sx150x_write_cfg(chip, n, 2,
					chip->dev_cfg->reg_sense,
					0);
		}
	}
	mutex_unlock(&chip->mutex);
}

static void sx150x_irq_mask_ack(unsigned int irq)
{
	sx150x_irq_mask(irq);
}

static void sx150x_irq_unmask(unsigned int irq)
{
	struct irq_chip *ic;
	struct sx150x_chip *chip;

	ic   = get_irq_chip(irq);
	chip = container_of(ic, struct sx150x_chip, irq_chip);

	set_bit(CMD_BIT(irq - chip->irq_base, CMD_UNMASK), chip->irq_cmds);
	schedule_work(&chip->irq_unmask_ws);
}

static void sx150x_irq_unmask_wfn(struct work_struct *work)
{
	struct sx150x_chip *chip;
	unsigned n;

	chip = container_of(work, struct sx150x_chip, irq_unmask_ws);

	mutex_lock(&chip->mutex);
	for (n = 0; n < chip->dev_cfg->ngpios; ++n) {
		if (test_and_clear_bit(CMD_BIT(n, CMD_UNMASK),
					chip->irq_cmds)) {
			sx150x_write_cfg(chip, n, 1,
					chip->dev_cfg->reg_irq_mask, 0);
			sx150x_write_cfg(chip, n, 2,
					chip->dev_cfg->reg_sense,
					chip->irq_sense >> (n * 2));
		}
	}
	mutex_unlock(&chip->mutex);
}

static void sx150x_irq_eoi(unsigned int irq)
{
}

static int sx150x_irq_set_affinity(unsigned int irq,
				const struct cpumask *dest)
{
	return -ENOTSUPP;
}

static int sx150x_irq_retrigger(unsigned int irq)
{
	generic_handle_irq(irq);
	return 0;
}

static int sx150x_irq_set_type(unsigned int irq, unsigned int flow_type)
{
	struct irq_chip *ic;
	struct sx150x_chip *chip;
	unsigned offset;

	if (flow_type & (IRQ_TYPE_LEVEL_HIGH | IRQ_TYPE_LEVEL_LOW))
		return -ENOTSUPP;

	ic     = get_irq_chip(irq);
	chip   = container_of(ic, struct sx150x_chip, irq_chip);
	offset = irq - chip->irq_base;

	if (flow_type & IRQ_TYPE_EDGE_RISING)
		set_bit(CMD_BIT(offset, CMD_RISING), chip->irq_cmds);
	if (flow_type & IRQ_TYPE_EDGE_FALLING)
		set_bit(CMD_BIT(offset, CMD_FALLING), chip->irq_cmds);
	schedule_work(&chip->irq_set_type_ws);

	return 0;
}

static void sx150x_irq_set_type_wfn(struct work_struct *work)
{
	struct sx150x_chip *chip;
	unsigned n;
	unsigned val;
	int irq;

	chip = container_of(work, struct sx150x_chip, irq_set_type_ws);

	mutex_lock(&chip->mutex);
	for (n = 0; n < chip->dev_cfg->ngpios; ++n) {
		val = 0;
		if (test_and_clear_bit(CMD_BIT(n, CMD_RISING),
					chip->irq_cmds))
			val |= 0x1;
		if (test_and_clear_bit(CMD_BIT(n, CMD_FALLING),
					chip->irq_cmds))
			val |= 0x2;

		chip->irq_sense &= ~(3UL << (n * 2));
		chip->irq_sense |= val << (n * 2);

		irq = chip->irq_base + n;
		if (!(irq_to_desc(irq)->status & IRQ_MASKED))
			sx150x_write_cfg(chip, n, 2,
					chip->dev_cfg->reg_sense, val);
	}
	mutex_unlock(&chip->mutex);
}

static int sx150x_irq_set_wake(unsigned int irq, unsigned int on)
{
	return -ENOTSUPP;
}

static irqreturn_t sx150x_irq_handler(int irq, void *dev_id)
{
	return IRQ_WAKE_THREAD;
}

static irqreturn_t sx150x_irq_thread_fn(int irq, void *dev_id)
{
	struct sx150x_chip *chip = (struct sx150x_chip *)dev_id;
	int i;
	unsigned n;
	unsigned sub_irq;
	u8 val;
	unsigned nhandled = 0;

	mutex_lock(&chip->mutex);
	for (i = (chip->dev_cfg->ngpios / 8) - 1; i >= 0; --i) {
		sx150x_i2c_read(chip->client,
				chip->dev_cfg->reg_irq_src - i,
				&val);
		sx150x_i2c_write(chip->client,
				chip->dev_cfg->reg_irq_src - i,
				val);
		for (n = 0; n < 8; ++n) {
			if (val & (1 << n)) {
				sub_irq = chip->irq_base + (i * 8) + n;
				generic_handle_irq(sub_irq);
				++nhandled;
			}
		}
	}
	mutex_unlock(&chip->mutex);

	return (nhandled > 0 ? IRQ_HANDLED : IRQ_NONE);
}

static void sx150x_init_chip(struct sx150x_chip *chip,
			struct i2c_client *client,
			kernel_ulong_t driver_data,
			struct sx150x_platform_data *pdata)
{
	mutex_init(&chip->mutex);

	chip->client                     = client;
	chip->dev_cfg                    = &sx150x_devices[driver_data];
	chip->gpio_chip.label            = client->name;
	chip->gpio_chip.direction_input  = sx150x_gpio_direction_input;
	chip->gpio_chip.direction_output = sx150x_gpio_direction_output;
	chip->gpio_chip.get              = sx150x_gpio_get;
	chip->gpio_chip.set              = sx150x_gpio_set;
	chip->gpio_chip.to_irq           = sx150x_gpio_to_irq;
	chip->gpio_chip.base             = pdata->gpio_base;
	chip->gpio_chip.can_sleep        = 1;
	chip->gpio_chip.ngpio            = chip->dev_cfg->ngpios;
	if (pdata->oscio_is_gpo)
		++chip->gpio_chip.ngpio;

	chip->irq_chip.name         = client->name;
	chip->irq_chip.ack          = sx150x_irq_ack;
	chip->irq_chip.mask         = sx150x_irq_mask;
	chip->irq_chip.mask_ack     = sx150x_irq_mask_ack;
	chip->irq_chip.unmask       = sx150x_irq_unmask;
	chip->irq_chip.eoi          = sx150x_irq_eoi;
	chip->irq_chip.set_affinity = sx150x_irq_set_affinity;
	chip->irq_chip.retrigger    = sx150x_irq_retrigger;
	chip->irq_chip.set_type     = sx150x_irq_set_type;
	chip->irq_chip.set_wake     = sx150x_irq_set_wake;
	chip->irq_summary           = -1;
	chip->irq_base              = -1;
	chip->irq_sense             = 0;

	bitmap_zero(chip->irq_cmds, NCMDBITS);

	INIT_WORK(&chip->irq_mask_ws, sx150x_irq_mask_wfn);
	INIT_WORK(&chip->irq_unmask_ws, sx150x_irq_unmask_wfn);
	INIT_WORK(&chip->irq_set_type_ws, sx150x_irq_set_type_wfn);
}

static int sx150x_init_io(struct sx150x_chip *chip, u8 base, u16 cfg)
{
	int err = 0;
	unsigned n;

	for (n = 0; err >= 0 && n < (chip->dev_cfg->ngpios / 8); ++n)
		err = sx150x_i2c_write(chip->client, base - n, cfg >> (n * 8));
	return err;
}

static int sx150x_init_hw(struct sx150x_chip *chip,
			struct sx150x_platform_data *pdata)
{
	int err = 0;

	err = i2c_smbus_write_word_data(chip->client,
					chip->dev_cfg->reg_reset,
					0x3412);
	if (err < 0)
		return err;

	err = sx150x_i2c_write(chip->client,
			chip->dev_cfg->reg_misc,
			0x01);
	if (err < 0)
		return err;

	err = sx150x_init_io(chip, chip->dev_cfg->reg_pullup,
			pdata->io_pullup_ena);
	if (err < 0)
		return err;

	err = sx150x_init_io(chip, chip->dev_cfg->reg_pulldn,
			pdata->io_pulldn_ena);
	if (err < 0)
		return err;

	err = sx150x_init_io(chip, chip->dev_cfg->reg_drain,
			pdata->io_open_drain_ena);
	if (err < 0)
		return err;

	err = sx150x_init_io(chip, chip->dev_cfg->reg_polarity,
			pdata->io_polarity);
	if (err < 0)
		return err;

	if (pdata->oscio_is_gpo)
		sx150x_set_oscio(chip, 0);

	return err;
}

static int sx150x_install_irq_chip(struct sx150x_chip *chip,
				int irq_summary,
				int irq_base)
{
	int err;
	unsigned n;
	unsigned irq;

	chip->irq_summary = irq_summary;
	chip->irq_base    = irq_base;

	for (n = 0; n < chip->dev_cfg->ngpios; ++n) {
		irq = irq_base + n;
		set_irq_chip(irq, &chip->irq_chip);
		set_irq_handler(irq, handle_edge_irq);
#ifdef CONFIG_ARM
		set_irq_flags(irq, IRQF_VALID);
#else
		set_irq_noprobe(irq);
#endif
	}

	err = request_threaded_irq(irq_summary,
				sx150x_irq_handler,
				sx150x_irq_thread_fn,
				IRQF_SHARED | IRQF_TRIGGER_FALLING,
				chip->irq_chip.name,
				chip);
	if (err < 0) {
		chip->irq_summary = -1;
		chip->irq_base    = -1;
	}

	return err;
}

static void sx150x_remove_irq_chip(struct sx150x_chip *chip)
{
	unsigned n;
	unsigned irq;

	free_irq(chip->irq_summary, chip);

	for (n = 0; n < chip->dev_cfg->ngpios; ++n) {
		irq = gpio_to_irq(chip->gpio_chip.base + n);
		set_irq_handler(irq, NULL);
		set_irq_chip(irq, NULL);
	}
}

static int __devinit sx150x_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct sx150x_platform_data *pdata;
	struct sx150x_chip *chip;
	int rc;

	pdata = client->dev.platform_data;
	if (!pdata)
		return -EINVAL;

	if (!i2c_check_functionality(client->adapter,
					I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENOTSUPP;

	chip = kzalloc(sizeof(struct sx150x_chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	sx150x_init_chip(chip, client, id->driver_data, pdata);
	rc = sx150x_init_hw(chip, pdata);
	if (rc < 0)
		goto probe_fail;

	rc = gpiochip_add(&chip->gpio_chip);
	if (rc < 0)
		goto probe_fail;

	i2c_set_clientdata(client, chip);

	if (pdata->irq_summary >= 0) {
		rc = sx150x_install_irq_chip(chip,
					pdata->irq_summary,
					pdata->irq_base);
		if (rc < 0)
			goto probe_fail;
	}

	return 0;
probe_fail:
	kfree(chip);
	return rc;
}

static int __devexit sx150x_remove(struct i2c_client *client)
{
	struct sx150x_chip *chip;
	int rc;

	chip = i2c_get_clientdata(client);
	i2c_set_clientdata(client, NULL);

	if (chip->irq_summary >= 0)
		sx150x_remove_irq_chip(chip);

	rc = gpiochip_remove(&(chip->gpio_chip));

	flush_work(&chip->irq_mask_ws);
	flush_work(&chip->irq_unmask_ws);
	flush_work(&chip->irq_set_type_ws);

	kfree(chip);

	return rc;
}

static struct i2c_driver sx150x_driver = {
	.driver = {
		.name = "sx150x",
		.owner = THIS_MODULE
	},
	.probe    = sx150x_probe,
	.remove   = __devexit_p(sx150x_remove),
	.id_table = sx150x_id
};

static int __init sx150x_init(void)
{
	return i2c_add_driver(&sx150x_driver);
}
subsys_initcall(sx150x_init);

static void __exit sx150x_exit(void)
{
	return i2c_del_driver(&sx150x_driver);
}
module_exit(sx150x_exit);

MODULE_AUTHOR("Gregory Bean <gbean@codeaurora.org>");
MODULE_DESCRIPTION("Driver for Semtech SX150X I2C GPIO Expanders");
MODULE_LICENSE("GPLv2");
