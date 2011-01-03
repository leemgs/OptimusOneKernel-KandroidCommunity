#warning compile out
#if 0


#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/gpio_event.h>
#include <linux/interrupt.h>

struct gpio_axis_state {
	struct gpio_event_input_devs *input_devs;
	struct gpio_event_axis_info *info;
	uint32_t pos;
};

uint16_t gpio_axis_4bit_gray_map_table[] = {
	[0x0] = 0x0, [0x1] = 0x1, 
	[0x3] = 0x2, [0x2] = 0x3, 
	[0x6] = 0x4, [0x7] = 0x5, 
	[0x5] = 0x6, [0x4] = 0x7, 
	[0xc] = 0x8, [0xd] = 0x9, 
	[0xf] = 0xa, [0xe] = 0xb, 
	[0xa] = 0xc, [0xb] = 0xd, 
	[0x9] = 0xe, [0x8] = 0xf, 
};
uint16_t gpio_axis_4bit_gray_map(struct gpio_event_axis_info *info, uint16_t in)
{
	return gpio_axis_4bit_gray_map_table[in];
}

uint16_t gpio_axis_5bit_singletrack_map_table[] = {
	[0x10] = 0x00, [0x14] = 0x01, [0x1c] = 0x02, 
	[0x1e] = 0x03, [0x1a] = 0x04, [0x18] = 0x05, 
	[0x08] = 0x06, [0x0a] = 0x07, [0x0e] = 0x08, 
	[0x0f] = 0x09, [0x0d] = 0x0a, [0x0c] = 0x0b, 
	[0x04] = 0x0c, [0x05] = 0x0d, [0x07] = 0x0e, 
	[0x17] = 0x0f, [0x16] = 0x10, [0x06] = 0x11, 
	[0x02] = 0x12, [0x12] = 0x13, [0x13] = 0x14, 
	[0x1b] = 0x15, [0x0b] = 0x16, [0x03] = 0x17, 
	[0x01] = 0x18, [0x09] = 0x19, [0x19] = 0x1a, 
	[0x1d] = 0x1b, [0x15] = 0x1c, [0x11] = 0x1d, 
};
uint16_t gpio_axis_5bit_singletrack_map(
	struct gpio_event_axis_info *info, uint16_t in)
{
	return gpio_axis_5bit_singletrack_map_table[in];
}

static void gpio_event_update_axis(struct gpio_axis_state *as, int report)
{
	struct gpio_event_axis_info *ai = as->info;
	int i;
	int change;
	uint16_t state = 0;
	uint16_t pos;
	uint16_t old_pos = as->pos;
	for (i = ai->count - 1; i >= 0; i--)
		state = (state << 1) | gpio_get_value(ai->gpio[i]);
	pos = ai->map(ai, state);
	if (ai->flags & GPIOEAF_PRINT_RAW)
		pr_info("axis %d-%d raw %x, pos %d -> %d\n",
			ai->type, ai->code, state, old_pos, pos);
	if (report && pos != old_pos) {
		if (ai->type == EV_REL) {
			change = (ai->decoded_size + pos - old_pos) %
				  ai->decoded_size;
			if (change > ai->decoded_size / 2)
				change -= ai->decoded_size;
			if (change == ai->decoded_size / 2) {
				if (ai->flags & GPIOEAF_PRINT_EVENT)
					pr_info("axis %d-%d unknown direction, "
						"pos %d -> %d\n", ai->type,
						ai->code, old_pos, pos);
				change = 0; 
			}
			if (ai->flags & GPIOEAF_PRINT_EVENT)
				pr_info("axis %d-%d change %d\n",
					ai->type, ai->code, change);
			input_report_rel(as->input_devs->dev[ai->dev],
						ai->code, change);
		} else {
			if (ai->flags & GPIOEAF_PRINT_EVENT)
				pr_info("axis %d-%d now %d\n",
					ai->type, ai->code, pos);
			input_event(as->input_devs->dev[ai->dev],
					ai->type, ai->code, pos);
		}
		input_sync(as->input_devs->dev[ai->dev]);
	}
	as->pos = pos;
}

static irqreturn_t gpio_axis_irq_handler(int irq, void *dev_id)
{
	struct gpio_axis_state *as = dev_id;
	gpio_event_update_axis(as, 1);
	return IRQ_HANDLED;
}

int gpio_event_axis_func(struct gpio_event_input_devs *input_devs,
			 struct gpio_event_info *info, void **data, int func)
{
	int ret;
	int i;
	int irq;
	struct gpio_event_axis_info *ai;
	struct gpio_axis_state *as;

	ai = container_of(info, struct gpio_event_axis_info, info);
	if (func == GPIO_EVENT_FUNC_SUSPEND) {
		for (i = 0; i < ai->count; i++)
			disable_irq(gpio_to_irq(ai->gpio[i]));
		return 0;
	}
	if (func == GPIO_EVENT_FUNC_RESUME) {
		for (i = 0; i < ai->count; i++)
			enable_irq(gpio_to_irq(ai->gpio[i]));
		return 0;
	}

	if (func == GPIO_EVENT_FUNC_INIT) {
		*data = as = kmalloc(sizeof(*as), GFP_KERNEL);
		if (as == NULL) {
			ret = -ENOMEM;
			goto err_alloc_axis_state_failed;
		}
		as->input_devs = input_devs;
		as->info = ai;
		if (ai->dev >= input_devs->count) {
			pr_err("gpio_event_axis: bad device index %d >= %d "
				"for %d:%d\n", ai->dev, input_devs->count,
				ai->type, ai->code);
			ret = -EINVAL;
			goto err_bad_device_index;
		}

		input_set_capability(input_devs->dev[ai->dev],
				     ai->type, ai->code);
		if (ai->type == EV_ABS) {
			input_set_abs_params(input_devs->dev[ai->dev], ai->code,
					     0, ai->decoded_size - 1, 0, 0);
		}
		for (i = 0; i < ai->count; i++) {
			ret = gpio_request(ai->gpio[i], "gpio_event_axis");
			if (ret < 0)
				goto err_request_gpio_failed;
			ret = gpio_direction_input(ai->gpio[i]);
			if (ret < 0)
				goto err_gpio_direction_input_failed;
			ret = irq = gpio_to_irq(ai->gpio[i]);
			if (ret < 0)
				goto err_get_irq_num_failed;
			ret = request_irq(irq, gpio_axis_irq_handler,
					  IRQF_TRIGGER_RISING |
					  IRQF_TRIGGER_FALLING,
					  "gpio_event_axis", as);
			if (ret < 0)
				goto err_request_irq_failed;
		}
		gpio_event_update_axis(as, 0);
		return 0;
	}

	ret = 0;
	as = *data;
	for (i = ai->count - 1; i >= 0; i--) {
		free_irq(gpio_to_irq(ai->gpio[i]), as);
err_request_irq_failed:
err_get_irq_num_failed:
err_gpio_direction_input_failed:
		gpio_free(ai->gpio[i]);
err_request_gpio_failed:
		;
	}
err_bad_device_index:
	kfree(as);
	*data = NULL;
err_alloc_axis_state_failed:
	return ret;
}
#endif
