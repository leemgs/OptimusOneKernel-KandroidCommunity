

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/dmi.h>
#include <linux/io.h>
#include <linux/rfkill.h>
#include <linux/power_supply.h>
#include <linux/acpi.h>
#include "../../firmware/dcdbas.h"

#define BRIGHTNESS_TOKEN 0x7d



struct calling_interface_buffer {
	u16 class;
	u16 select;
	volatile u32 input[4];
	volatile u32 output[4];
} __packed;

struct calling_interface_token {
	u16 tokenID;
	u16 location;
	union {
		u16 value;
		u16 stringlength;
	};
};

struct calling_interface_structure {
	struct dmi_header header;
	u16 cmdIOAddress;
	u8 cmdIOCode;
	u32 supportedCmds;
	struct calling_interface_token tokens[];
} __packed;

static int da_command_address;
static int da_command_code;
static int da_num_tokens;
static struct calling_interface_token *da_tokens;

static struct backlight_device *dell_backlight_device;
static struct rfkill *wifi_rfkill;
static struct rfkill *bluetooth_rfkill;
static struct rfkill *wwan_rfkill;

static const struct dmi_system_id __initdata dell_device_table[] = {
	{
		.ident = "Dell laptop",
		.matches = {
			DMI_MATCH(DMI_SYS_VENDOR, "Dell Inc."),
			DMI_MATCH(DMI_CHASSIS_TYPE, "8"),
		},
	},
	{ }
};

static void parse_da_table(const struct dmi_header *dm)
{
	
	int tokens = (dm->length-11)/sizeof(struct calling_interface_token)-1;
	struct calling_interface_structure *table =
		container_of(dm, struct calling_interface_structure, header);

	

	if (dm->length < 17)
		return;

	da_command_address = table->cmdIOAddress;
	da_command_code = table->cmdIOCode;

	da_tokens = krealloc(da_tokens, (da_num_tokens + tokens) *
			     sizeof(struct calling_interface_token),
			     GFP_KERNEL);

	if (!da_tokens)
		return;

	memcpy(da_tokens+da_num_tokens, table->tokens,
	       sizeof(struct calling_interface_token) * tokens);

	da_num_tokens += tokens;
}

static void find_tokens(const struct dmi_header *dm, void *dummy)
{
	switch (dm->type) {
	case 0xd4: 
		break;
	case 0xd5: 
		break;
	case 0xd6: 
		break;
	case 0xda: 
		parse_da_table(dm);
		break;
	}
}

static int find_token_location(int tokenid)
{
	int i;
	for (i = 0; i < da_num_tokens; i++) {
		if (da_tokens[i].tokenID == tokenid)
			return da_tokens[i].location;
	}

	return -1;
}

static struct calling_interface_buffer *
dell_send_request(struct calling_interface_buffer *buffer, int class,
		  int select)
{
	struct smi_cmd command;

	command.magic = SMI_CMD_MAGIC;
	command.command_address = da_command_address;
	command.command_code = da_command_code;
	command.ebx = virt_to_phys(buffer);
	command.ecx = 0x42534931;

	buffer->class = class;
	buffer->select = select;

	dcdbas_smi_request(&command);

	return buffer;
}



static int dell_rfkill_set(void *data, bool blocked)
{
	struct calling_interface_buffer buffer;
	int disable = blocked ? 1 : 0;
	unsigned long radio = (unsigned long)data;

	memset(&buffer, 0, sizeof(struct calling_interface_buffer));
	buffer.input[0] = (1 | (radio<<8) | (disable << 16));
	dell_send_request(&buffer, 17, 11);

	return 0;
}

static void dell_rfkill_query(struct rfkill *rfkill, void *data)
{
	struct calling_interface_buffer buffer;
	int status;
	int bit = (unsigned long)data + 16;

	memset(&buffer, 0, sizeof(struct calling_interface_buffer));
	dell_send_request(&buffer, 17, 11);
	status = buffer.output[1];

	if (status & BIT(bit))
		rfkill_set_hw_state(rfkill, !!(status & BIT(16)));
}

static const struct rfkill_ops dell_rfkill_ops = {
	.set_block = dell_rfkill_set,
	.query = dell_rfkill_query,
};

static int dell_setup_rfkill(void)
{
	struct calling_interface_buffer buffer;
	int status;
	int ret;

	memset(&buffer, 0, sizeof(struct calling_interface_buffer));
	dell_send_request(&buffer, 17, 11);
	status = buffer.output[1];

	if ((status & (1<<2|1<<8)) == (1<<2|1<<8)) {
		wifi_rfkill = rfkill_alloc("dell-wifi", NULL, RFKILL_TYPE_WLAN,
					   &dell_rfkill_ops, (void *) 1);
		if (!wifi_rfkill) {
			ret = -ENOMEM;
			goto err_wifi;
		}
		ret = rfkill_register(wifi_rfkill);
		if (ret)
			goto err_wifi;
	}

	if ((status & (1<<3|1<<9)) == (1<<3|1<<9)) {
		bluetooth_rfkill = rfkill_alloc("dell-bluetooth", NULL,
						RFKILL_TYPE_BLUETOOTH,
						&dell_rfkill_ops, (void *) 2);
		if (!bluetooth_rfkill) {
			ret = -ENOMEM;
			goto err_bluetooth;
		}
		ret = rfkill_register(bluetooth_rfkill);
		if (ret)
			goto err_bluetooth;
	}

	if ((status & (1<<4|1<<10)) == (1<<4|1<<10)) {
		wwan_rfkill = rfkill_alloc("dell-wwan", NULL, RFKILL_TYPE_WWAN,
					   &dell_rfkill_ops, (void *) 3);
		if (!wwan_rfkill) {
			ret = -ENOMEM;
			goto err_wwan;
		}
		ret = rfkill_register(wwan_rfkill);
		if (ret)
			goto err_wwan;
	}

	return 0;
err_wwan:
	rfkill_destroy(wwan_rfkill);
	if (bluetooth_rfkill)
		rfkill_unregister(bluetooth_rfkill);
err_bluetooth:
	rfkill_destroy(bluetooth_rfkill);
	if (wifi_rfkill)
		rfkill_unregister(wifi_rfkill);
err_wifi:
	rfkill_destroy(wifi_rfkill);

	return ret;
}

static int dell_send_intensity(struct backlight_device *bd)
{
	struct calling_interface_buffer buffer;

	memset(&buffer, 0, sizeof(struct calling_interface_buffer));
	buffer.input[0] = find_token_location(BRIGHTNESS_TOKEN);
	buffer.input[1] = bd->props.brightness;

	if (buffer.input[0] == -1)
		return -ENODEV;

	if (power_supply_is_system_supplied() > 0)
		dell_send_request(&buffer, 1, 2);
	else
		dell_send_request(&buffer, 1, 1);

	return 0;
}

static int dell_get_intensity(struct backlight_device *bd)
{
	struct calling_interface_buffer buffer;

	memset(&buffer, 0, sizeof(struct calling_interface_buffer));
	buffer.input[0] = find_token_location(BRIGHTNESS_TOKEN);

	if (buffer.input[0] == -1)
		return -ENODEV;

	if (power_supply_is_system_supplied() > 0)
		dell_send_request(&buffer, 0, 2);
	else
		dell_send_request(&buffer, 0, 1);

	return buffer.output[1];
}

static struct backlight_ops dell_ops = {
	.get_brightness = dell_get_intensity,
	.update_status  = dell_send_intensity,
};

static int __init dell_init(void)
{
	struct calling_interface_buffer buffer;
	int max_intensity = 0;
	int ret;

	if (!dmi_check_system(dell_device_table))
		return -ENODEV;

	dmi_walk(find_tokens, NULL);

	if (!da_tokens)  {
		printk(KERN_INFO "dell-laptop: Unable to find dmi tokens\n");
		return -ENODEV;
	}

	ret = dell_setup_rfkill();

	if (ret) {
		printk(KERN_WARNING "dell-laptop: Unable to setup rfkill\n");
		goto out;
	}

#ifdef CONFIG_ACPI
	
	if (acpi_video_backlight_support())
		return 0;
#endif

	memset(&buffer, 0, sizeof(struct calling_interface_buffer));
	buffer.input[0] = find_token_location(BRIGHTNESS_TOKEN);

	if (buffer.input[0] != -1) {
		dell_send_request(&buffer, 0, 2);
		max_intensity = buffer.output[3];
	}

	if (max_intensity) {
		dell_backlight_device = backlight_device_register(
			"dell_backlight",
			NULL, NULL,
			&dell_ops);

		if (IS_ERR(dell_backlight_device)) {
			ret = PTR_ERR(dell_backlight_device);
			dell_backlight_device = NULL;
			goto out;
		}

		dell_backlight_device->props.max_brightness = max_intensity;
		dell_backlight_device->props.brightness =
			dell_get_intensity(dell_backlight_device);
		backlight_update_status(dell_backlight_device);
	}

	return 0;
out:
	if (wifi_rfkill)
		rfkill_unregister(wifi_rfkill);
	if (bluetooth_rfkill)
		rfkill_unregister(bluetooth_rfkill);
	if (wwan_rfkill)
		rfkill_unregister(wwan_rfkill);
	kfree(da_tokens);
	return ret;
}

static void __exit dell_exit(void)
{
	backlight_device_unregister(dell_backlight_device);
	if (wifi_rfkill)
		rfkill_unregister(wifi_rfkill);
	if (bluetooth_rfkill)
		rfkill_unregister(bluetooth_rfkill);
	if (wwan_rfkill)
		rfkill_unregister(wwan_rfkill);
}

module_init(dell_init);
module_exit(dell_exit);

MODULE_AUTHOR("Matthew Garrett <mjg@redhat.com>");
MODULE_DESCRIPTION("Dell laptop driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("dmi:*svnDellInc.:*:ct8:*");
