

#include <linux/init.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/uwb.h>
#include "dfu/i1480-dfu.h"



static struct uwb_est_entry i1480_est_fd00[] = {
	
	[i1480_EVT_CONFIRM] = { .size = sizeof(struct i1480_evt_confirm) },
	[i1480_CMD_SET_IP_MAS] = { .size = sizeof(struct i1480_evt_confirm) },
#ifdef i1480_RCEB_EXTENDED
	[0x09] = {
		.size = sizeof(struct i1480_rceb),
		.offset = 1 + offsetof(struct i1480_rceb, wParamLength),
	},
#endif
};


static struct uwb_est_entry i1480_est_fd01[] = {
	[0xff & i1480_EVT_RM_INIT_DONE] = { .size = sizeof(struct i1480_rceb) },
	[0xff & i1480_EVT_DEV_ADD] = { .size = sizeof(struct i1480_rceb) + 9 },
	[0xff & i1480_EVT_DEV_RM] = { .size = sizeof(struct i1480_rceb) + 9 },
	[0xff & i1480_EVT_DEV_ID_CHANGE] = {
		.size = sizeof(struct i1480_rceb) + 2 },
};

static int i1480_est_init(void)
{
	int result = uwb_est_register(i1480_CET_VS1, 0x00, 0x8086, 0x0c3b,
				      i1480_est_fd00,
				      ARRAY_SIZE(i1480_est_fd00));
	if (result < 0) {
		printk(KERN_ERR "Can't register EST table fd00: %d\n", result);
		return result;
	}
	result = uwb_est_register(i1480_CET_VS1, 0x01, 0x8086, 0x0c3b,
				  i1480_est_fd01, ARRAY_SIZE(i1480_est_fd01));
	if (result < 0) {
		printk(KERN_ERR "Can't register EST table fd01: %d\n", result);
		return result;
	}
	return 0;
}
module_init(i1480_est_init);

static void i1480_est_exit(void)
{
	uwb_est_unregister(i1480_CET_VS1, 0x00, 0x8086, 0x0c3b,
			   i1480_est_fd00, ARRAY_SIZE(i1480_est_fd00));
	uwb_est_unregister(i1480_CET_VS1, 0x01, 0x8086, 0x0c3b,
			   i1480_est_fd01, ARRAY_SIZE(i1480_est_fd01));
}
module_exit(i1480_est_exit);

MODULE_AUTHOR("Inaky Perez-Gonzalez <inaky.perez-gonzalez@intel.com>");
MODULE_DESCRIPTION("i1480's Vendor Specific Event Size Tables");
MODULE_LICENSE("GPL");


static struct usb_device_id i1480_est_id_table[] = {
	{ USB_DEVICE(0x8086, 0xdf3b), },
	{ USB_DEVICE(0x8086, 0x0c3b), },
	{ },
};
MODULE_DEVICE_TABLE(usb, i1480_est_id_table);
