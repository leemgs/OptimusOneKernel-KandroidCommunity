
#include "dvb-usb-common.h"
#include <linux/usb/input.h>

static int dvb_usb_getkeycode(struct input_dev *dev,
				    int scancode, int *keycode)
{
	struct dvb_usb_device *d = input_get_drvdata(dev);

	struct dvb_usb_rc_key *keymap = d->props.rc_key_map;
	int i;

	
	for (i = 0; i < d->props.rc_key_map_size; i++)
		if (keymap[i].scan == scancode) {
			*keycode = keymap[i].event;
			return 0;
		}

	
	for (i = 0; i < d->props.rc_key_map_size; i++)
		if (keymap[i].event == KEY_RESERVED ||
		    keymap[i].event == KEY_UNKNOWN) {
			*keycode = KEY_RESERVED;
			return 0;
		}

	return -EINVAL;
}

static int dvb_usb_setkeycode(struct input_dev *dev,
				    int scancode, int keycode)
{
	struct dvb_usb_device *d = input_get_drvdata(dev);

	struct dvb_usb_rc_key *keymap = d->props.rc_key_map;
	int i;

	
	for (i = 0; i < d->props.rc_key_map_size; i++)
		if (keymap[i].scan == scancode) {
			keymap[i].event = keycode;
			return 0;
		}

	
	for (i = 0; i < d->props.rc_key_map_size; i++)
		if (keymap[i].event == KEY_RESERVED ||
		    keymap[i].event == KEY_UNKNOWN) {
			keymap[i].scan = scancode;
			keymap[i].event = keycode;
			return 0;
		}

	

	return -EINVAL;
}


static void dvb_usb_read_remote_control(struct work_struct *work)
{
	struct dvb_usb_device *d =
		container_of(work, struct dvb_usb_device, rc_query_work.work);
	u32 event;
	int state;

	

	
	if (dvb_usb_disable_rc_polling)
		return;

	if (d->props.rc_query(d,&event,&state)) {
		err("error while querying for an remote control event.");
		goto schedule;
	}


	switch (state) {
		case REMOTE_NO_KEY_PRESSED:
			break;
		case REMOTE_KEY_PRESSED:
			deb_rc("key pressed\n");
			d->last_event = event;
		case REMOTE_KEY_REPEAT:
			deb_rc("key repeated\n");
			input_event(d->rc_input_dev, EV_KEY, event, 1);
			input_event(d->rc_input_dev, EV_KEY, d->last_event, 0);
			input_sync(d->rc_input_dev);
			break;
		default:
			break;
	}



schedule:
	schedule_delayed_work(&d->rc_query_work,msecs_to_jiffies(d->props.rc_interval));
}

int dvb_usb_remote_init(struct dvb_usb_device *d)
{
	struct input_dev *input_dev;
	int i;
	int err;

	if (d->props.rc_key_map == NULL ||
		d->props.rc_query == NULL ||
		dvb_usb_disable_rc_polling)
		return 0;

	usb_make_path(d->udev, d->rc_phys, sizeof(d->rc_phys));
	strlcat(d->rc_phys, "/ir0", sizeof(d->rc_phys));

	input_dev = input_allocate_device();
	if (!input_dev)
		return -ENOMEM;

	input_dev->evbit[0] = BIT_MASK(EV_KEY);
	input_dev->name = "IR-receiver inside an USB DVB receiver";
	input_dev->phys = d->rc_phys;
	usb_to_input_id(d->udev, &input_dev->id);
	input_dev->dev.parent = &d->udev->dev;
	input_dev->getkeycode = dvb_usb_getkeycode;
	input_dev->setkeycode = dvb_usb_setkeycode;

	
	deb_rc("key map size: %d\n", d->props.rc_key_map_size);
	for (i = 0; i < d->props.rc_key_map_size; i++) {
		deb_rc("setting bit for event %d item %d\n",
			d->props.rc_key_map[i].event, i);
		set_bit(d->props.rc_key_map[i].event, input_dev->keybit);
	}

	
	if (d->props.rc_interval < 40)
		d->props.rc_interval = 100; 

	
	input_dev->rep[REP_PERIOD] = d->props.rc_interval;
	input_dev->rep[REP_DELAY]  = d->props.rc_interval + 150;

	input_set_drvdata(input_dev, d);

	err = input_register_device(input_dev);
	if (err) {
		input_free_device(input_dev);
		return err;
	}

	d->rc_input_dev = input_dev;

	INIT_DELAYED_WORK(&d->rc_query_work, dvb_usb_read_remote_control);

	info("schedule remote query interval to %d msecs.", d->props.rc_interval);
	schedule_delayed_work(&d->rc_query_work,msecs_to_jiffies(d->props.rc_interval));

	d->state |= DVB_USB_STATE_REMOTE;

	return 0;
}

int dvb_usb_remote_exit(struct dvb_usb_device *d)
{
	if (d->state & DVB_USB_STATE_REMOTE) {
		cancel_rearming_delayed_work(&d->rc_query_work);
		flush_scheduled_work();
		input_unregister_device(d->rc_input_dev);
	}
	d->state &= ~DVB_USB_STATE_REMOTE;
	return 0;
}

#define DVB_USB_RC_NEC_EMPTY           0x00
#define DVB_USB_RC_NEC_KEY_PRESSED     0x01
#define DVB_USB_RC_NEC_KEY_REPEATED    0x02
int dvb_usb_nec_rc_key_to_event(struct dvb_usb_device *d,
		u8 keybuf[5], u32 *event, int *state)
{
	int i;
	struct dvb_usb_rc_key *keymap = d->props.rc_key_map;
	*event = 0;
	*state = REMOTE_NO_KEY_PRESSED;
	switch (keybuf[0]) {
		case DVB_USB_RC_NEC_EMPTY:
			break;
		case DVB_USB_RC_NEC_KEY_PRESSED:
			if ((u8) ~keybuf[1] != keybuf[2] ||
				(u8) ~keybuf[3] != keybuf[4]) {
				deb_err("remote control checksum failed.\n");
				break;
			}
			
			for (i = 0; i < d->props.rc_key_map_size; i++)
				if (rc5_custom(&keymap[i]) == keybuf[1] &&
					rc5_data(&keymap[i]) == keybuf[3]) {
					*event = keymap[i].event;
					*state = REMOTE_KEY_PRESSED;
					return 0;
				}
			deb_err("key mapping failed - no appropriate key found in keymapping\n");
			break;
		case DVB_USB_RC_NEC_KEY_REPEATED:
			*state = REMOTE_KEY_REPEAT;
			break;
		default:
			deb_err("unkown type of remote status: %d\n",keybuf[0]);
			break;
	}
	return 0;
}
EXPORT_SYMBOL(dvb_usb_nec_rc_key_to_event);
