

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/completion.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/usb.h>
#include <linux/usbdevice_fs.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/freezer.h>

#include <asm/uaccess.h>
#include <asm/byteorder.h>

#include "usb.h"
#include "hcd.h"
#include "hub.h"


#ifdef DEBUG
#ifndef CONFIG_USB_ANNOUNCE_NEW_DEVICES
#define CONFIG_USB_ANNOUNCE_NEW_DEVICES
#endif
#endif

struct usb_hub {
	struct device		*intfdev;	
	struct usb_device	*hdev;
	struct kref		kref;
	struct urb		*urb;		

	
	char			(*buffer)[8];
	dma_addr_t		buffer_dma;	
	union {
		struct usb_hub_status	hub;
		struct usb_port_status	port;
	}			*status;	
	struct mutex		status_mutex;	

	int			error;		
	int			nerrors;	

	struct list_head	event_list;	
	unsigned long		event_bits[1];	
	unsigned long		change_bits[1];	
	unsigned long		busy_bits[1];	
#if USB_MAXCHILDREN > 31 
#error event_bits[] is too short!
#endif

	struct usb_hub_descriptor *descriptor;	
	struct usb_tt		tt;		

	unsigned		mA_per_port;	

	unsigned		limited_power:1;
	unsigned		quiescing:1;
	unsigned		disconnected:1;

	unsigned		has_indicators:1;
	u8			indicator[USB_MAXCHILDREN];
	struct delayed_work	leds;
	struct delayed_work	init_work;
	void			**port_owners;
};



static DEFINE_SPINLOCK(device_state_lock);


static DEFINE_SPINLOCK(hub_event_lock);
static LIST_HEAD(hub_event_list);	


static DECLARE_WAIT_QUEUE_HEAD(khubd_wait);

static struct task_struct *khubd_task;


static int blinkenlights = 0;
module_param (blinkenlights, bool, S_IRUGO);
MODULE_PARM_DESC (blinkenlights, "true to cycle leds on hubs");



static int initial_descriptor_timeout = USB_CTRL_GET_TIMEOUT;
module_param(initial_descriptor_timeout, int, S_IRUGO|S_IWUSR);
MODULE_PARM_DESC(initial_descriptor_timeout,
		"initial 64-byte descriptor request timeout in milliseconds "
		"(default 5000 - 5.0 seconds)");


static int old_scheme_first = 0;
module_param(old_scheme_first, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(old_scheme_first,
		 "start with the old device initialization scheme");

static int use_both_schemes = 1;
module_param(use_both_schemes, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(use_both_schemes,
		"try the other device initialization scheme if the "
		"first one fails");


DECLARE_RWSEM(ehci_cf_port_reset_rwsem);
EXPORT_SYMBOL_GPL(ehci_cf_port_reset_rwsem);

#define HUB_DEBOUNCE_TIMEOUT	1500
#define HUB_DEBOUNCE_STEP	  25
#define HUB_DEBOUNCE_STABLE	 100


static int usb_reset_and_verify_device(struct usb_device *udev);

static inline char *portspeed(int portstatus)
{
	if (portstatus & (1 << USB_PORT_FEAT_HIGHSPEED))
    		return "480 Mb/s";
	else if (portstatus & (1 << USB_PORT_FEAT_LOWSPEED))
		return "1.5 Mb/s";
	else if (portstatus & (1 << USB_PORT_FEAT_SUPERSPEED))
		return "5.0 Gb/s";
	else
		return "12 Mb/s";
}


static struct usb_hub *hdev_to_hub(struct usb_device *hdev)
{
	if (!hdev || !hdev->actconfig)
		return NULL;
	return usb_get_intfdata(hdev->actconfig->interface[0]);
}


static int get_hub_descriptor(struct usb_device *hdev, void *data, int size)
{
	int i, ret;

	for (i = 0; i < 3; i++) {
		ret = usb_control_msg(hdev, usb_rcvctrlpipe(hdev, 0),
			USB_REQ_GET_DESCRIPTOR, USB_DIR_IN | USB_RT_HUB,
			USB_DT_HUB << 8, 0, data, size,
			USB_CTRL_GET_TIMEOUT);
		if (ret >= (USB_DT_HUB_NONVAR_SIZE + 2))
			return ret;
	}
	return -EINVAL;
}


static int clear_hub_feature(struct usb_device *hdev, int feature)
{
	return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
		USB_REQ_CLEAR_FEATURE, USB_RT_HUB, feature, 0, NULL, 0, 1000);
}


static int clear_port_feature(struct usb_device *hdev, int port1, int feature)
{
	return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
		USB_REQ_CLEAR_FEATURE, USB_RT_PORT, feature, port1,
		NULL, 0, 1000);
}


static int set_port_feature(struct usb_device *hdev, int port1, int feature)
{
	return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
		USB_REQ_SET_FEATURE, USB_RT_PORT, feature, port1,
		NULL, 0, 1000);
}


static void set_port_led(
	struct usb_hub *hub,
	int port1,
	int selector
)
{
	int status = set_port_feature(hub->hdev, (selector << 8) | port1,
			USB_PORT_FEAT_INDICATOR);
	if (status < 0)
		dev_dbg (hub->intfdev,
			"port %d indicator %s status %d\n",
			port1,
			({ char *s; switch (selector) {
			case HUB_LED_AMBER: s = "amber"; break;
			case HUB_LED_GREEN: s = "green"; break;
			case HUB_LED_OFF: s = "off"; break;
			case HUB_LED_AUTO: s = "auto"; break;
			default: s = "??"; break;
			}; s; }),
			status);
}

#define	LED_CYCLE_PERIOD	((2*HZ)/3)

static void led_work (struct work_struct *work)
{
	struct usb_hub		*hub =
		container_of(work, struct usb_hub, leds.work);
	struct usb_device	*hdev = hub->hdev;
	unsigned		i;
	unsigned		changed = 0;
	int			cursor = -1;

	if (hdev->state != USB_STATE_CONFIGURED || hub->quiescing)
		return;

	for (i = 0; i < hub->descriptor->bNbrPorts; i++) {
		unsigned	selector, mode;

		

		switch (hub->indicator[i]) {
		
		case INDICATOR_CYCLE:
			cursor = i;
			selector = HUB_LED_AUTO;
			mode = INDICATOR_AUTO;
			break;
		
		case INDICATOR_GREEN_BLINK:
			selector = HUB_LED_GREEN;
			mode = INDICATOR_GREEN_BLINK_OFF;
			break;
		case INDICATOR_GREEN_BLINK_OFF:
			selector = HUB_LED_OFF;
			mode = INDICATOR_GREEN_BLINK;
			break;
		
		case INDICATOR_AMBER_BLINK:
			selector = HUB_LED_AMBER;
			mode = INDICATOR_AMBER_BLINK_OFF;
			break;
		case INDICATOR_AMBER_BLINK_OFF:
			selector = HUB_LED_OFF;
			mode = INDICATOR_AMBER_BLINK;
			break;
		
		case INDICATOR_ALT_BLINK:
			selector = HUB_LED_GREEN;
			mode = INDICATOR_ALT_BLINK_OFF;
			break;
		case INDICATOR_ALT_BLINK_OFF:
			selector = HUB_LED_AMBER;
			mode = INDICATOR_ALT_BLINK;
			break;
		default:
			continue;
		}
		if (selector != HUB_LED_AUTO)
			changed = 1;
		set_port_led(hub, i + 1, selector);
		hub->indicator[i] = mode;
	}
	if (!changed && blinkenlights) {
		cursor++;
		cursor %= hub->descriptor->bNbrPorts;
		set_port_led(hub, cursor + 1, HUB_LED_GREEN);
		hub->indicator[cursor] = INDICATOR_CYCLE;
		changed++;
	}
	if (changed)
		schedule_delayed_work(&hub->leds, LED_CYCLE_PERIOD);
}


#define	USB_STS_TIMEOUT		1000
#define	USB_STS_RETRIES		5


static int get_hub_status(struct usb_device *hdev,
		struct usb_hub_status *data)
{
	int i, status = -ETIMEDOUT;

	for (i = 0; i < USB_STS_RETRIES && status == -ETIMEDOUT; i++) {
		status = usb_control_msg(hdev, usb_rcvctrlpipe(hdev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_HUB, 0, 0,
			data, sizeof(*data), USB_STS_TIMEOUT);
	}
	return status;
}


static int get_port_status(struct usb_device *hdev, int port1,
		struct usb_port_status *data)
{
	int i, status = -ETIMEDOUT;

	for (i = 0; i < USB_STS_RETRIES && status == -ETIMEDOUT; i++) {
		status = usb_control_msg(hdev, usb_rcvctrlpipe(hdev, 0),
			USB_REQ_GET_STATUS, USB_DIR_IN | USB_RT_PORT, 0, port1,
			data, sizeof(*data), USB_STS_TIMEOUT);
	}
	return status;
}

static int hub_port_status(struct usb_hub *hub, int port1,
		u16 *status, u16 *change)
{
	int ret;

	mutex_lock(&hub->status_mutex);
	ret = get_port_status(hub->hdev, port1, &hub->status->port);
	if (ret < 4) {
		dev_err(hub->intfdev,
			"%s failed (err = %d)\n", __func__, ret);
		if (ret >= 0)
			ret = -EIO;
	} else {
		*status = le16_to_cpu(hub->status->port.wPortStatus);
		*change = le16_to_cpu(hub->status->port.wPortChange);
		ret = 0;
	}
	mutex_unlock(&hub->status_mutex);
	return ret;
}

static void kick_khubd(struct usb_hub *hub)
{
	unsigned long	flags;

	
	atomic_set(&to_usb_interface(hub->intfdev)->pm_usage_cnt, 1);

	spin_lock_irqsave(&hub_event_lock, flags);
	if (!hub->disconnected && list_empty(&hub->event_list)) {
		list_add_tail(&hub->event_list, &hub_event_list);
		wake_up(&khubd_wait);
	}
	spin_unlock_irqrestore(&hub_event_lock, flags);
}

void usb_kick_khubd(struct usb_device *hdev)
{
	struct usb_hub *hub = hdev_to_hub(hdev);

	if (hub)
		kick_khubd(hub);
}



static void hub_irq(struct urb *urb)
{
	struct usb_hub *hub = urb->context;
	int status = urb->status;
	unsigned i;
	unsigned long bits;

	switch (status) {
	case -ENOENT:		
	case -ECONNRESET:	
	case -ESHUTDOWN:	
		return;

	default:		
		
		dev_dbg (hub->intfdev, "transfer --> %d\n", status);
		if ((++hub->nerrors < 10) || hub->error)
			goto resubmit;
		hub->error = status;
		

	
	case 0:			
		bits = 0;
		for (i = 0; i < urb->actual_length; ++i)
			bits |= ((unsigned long) ((*hub->buffer)[i]))
					<< (i*8);
		hub->event_bits[0] = bits;
		break;
	}

	hub->nerrors = 0;

	
	kick_khubd(hub);

resubmit:
	if (hub->quiescing)
		return;

	if ((status = usb_submit_urb (hub->urb, GFP_ATOMIC)) != 0
			&& status != -ENODEV && status != -EPERM)
		dev_err (hub->intfdev, "resubmit --> %d\n", status);
}


static inline int
hub_clear_tt_buffer (struct usb_device *hdev, u16 devinfo, u16 tt)
{
	return usb_control_msg(hdev, usb_sndctrlpipe(hdev, 0),
			       HUB_CLEAR_TT_BUFFER, USB_RT_PORT, devinfo,
			       tt, NULL, 0, 1000);
}


static void hub_tt_work(struct work_struct *work)
{
	struct usb_hub		*hub =
		container_of(work, struct usb_hub, tt.clear_work);
	unsigned long		flags;
	int			limit = 100;

	spin_lock_irqsave (&hub->tt.lock, flags);
	while (--limit && !list_empty (&hub->tt.clear_list)) {
		struct list_head	*next;
		struct usb_tt_clear	*clear;
		struct usb_device	*hdev = hub->hdev;
		const struct hc_driver	*drv;
		int			status;

		next = hub->tt.clear_list.next;
		clear = list_entry (next, struct usb_tt_clear, clear_list);
		list_del (&clear->clear_list);

		
		spin_unlock_irqrestore (&hub->tt.lock, flags);
		status = hub_clear_tt_buffer (hdev, clear->devinfo, clear->tt);
		if (status)
			dev_err (&hdev->dev,
				"clear tt %d (%04x) error %d\n",
				clear->tt, clear->devinfo, status);

		
		drv = clear->hcd->driver;
		if (drv->clear_tt_buffer_complete)
			(drv->clear_tt_buffer_complete)(clear->hcd, clear->ep);

		kfree(clear);
		spin_lock_irqsave(&hub->tt.lock, flags);
	}
	spin_unlock_irqrestore (&hub->tt.lock, flags);
}


int usb_hub_clear_tt_buffer(struct urb *urb)
{
	struct usb_device	*udev = urb->dev;
	int			pipe = urb->pipe;
	struct usb_tt		*tt = udev->tt;
	unsigned long		flags;
	struct usb_tt_clear	*clear;

	
	if ((clear = kmalloc (sizeof *clear, GFP_ATOMIC)) == NULL) {
		dev_err (&udev->dev, "can't save CLEAR_TT_BUFFER state\n");
		
		return -ENOMEM;
	}

	
	clear->tt = tt->multi ? udev->ttport : 1;
	clear->devinfo = usb_pipeendpoint (pipe);
	clear->devinfo |= udev->devnum << 4;
	clear->devinfo |= usb_pipecontrol (pipe)
			? (USB_ENDPOINT_XFER_CONTROL << 11)
			: (USB_ENDPOINT_XFER_BULK << 11);
	if (usb_pipein (pipe))
		clear->devinfo |= 1 << 15;

	
	clear->hcd = bus_to_hcd(udev->bus);
	clear->ep = urb->ep;

	
	spin_lock_irqsave (&tt->lock, flags);
	list_add_tail (&clear->clear_list, &tt->clear_list);
	schedule_work(&tt->clear_work);
	spin_unlock_irqrestore (&tt->lock, flags);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_hub_clear_tt_buffer);


static unsigned hub_power_on(struct usb_hub *hub, bool do_delay)
{
	int port1;
	unsigned pgood_delay = hub->descriptor->bPwrOn2PwrGood * 2;
	unsigned delay;
	u16 wHubCharacteristics =
			le16_to_cpu(hub->descriptor->wHubCharacteristics);

	
	if ((wHubCharacteristics & HUB_CHAR_LPSM) < 2)
		dev_dbg(hub->intfdev, "enabling power on all ports\n");
	else
		dev_dbg(hub->intfdev, "trying to enable port power on "
				"non-switchable hub\n");
	for (port1 = 1; port1 <= hub->descriptor->bNbrPorts; port1++)
		set_port_feature(hub->hdev, port1, USB_PORT_FEAT_POWER);

	
	delay = max(pgood_delay, (unsigned) 100);
	if (do_delay)
		msleep(delay);
	return delay;
}

static int hub_hub_status(struct usb_hub *hub,
		u16 *status, u16 *change)
{
	int ret;

	mutex_lock(&hub->status_mutex);
	ret = get_hub_status(hub->hdev, &hub->status->hub);
	if (ret < 0)
		dev_err (hub->intfdev,
			"%s failed (err = %d)\n", __func__, ret);
	else {
		*status = le16_to_cpu(hub->status->hub.wHubStatus);
		*change = le16_to_cpu(hub->status->hub.wHubChange); 
		ret = 0;
	}
	mutex_unlock(&hub->status_mutex);
	return ret;
}

static int hub_port_disable(struct usb_hub *hub, int port1, int set_state)
{
	struct usb_device *hdev = hub->hdev;
	int ret = 0;

	if (hdev->children[port1-1] && set_state)
		usb_set_device_state(hdev->children[port1-1],
				USB_STATE_NOTATTACHED);
	if (!hub->error)
		ret = clear_port_feature(hdev, port1, USB_PORT_FEAT_ENABLE);
	if (ret)
		dev_err(hub->intfdev, "cannot disable port %d (err = %d)\n",
				port1, ret);
	return ret;
}


static void hub_port_logical_disconnect(struct usb_hub *hub, int port1)
{
	dev_dbg(hub->intfdev, "logical disconnect on port %d\n", port1);
	hub_port_disable(hub, port1, 1);

	

	set_bit(port1, hub->change_bits);
 	kick_khubd(hub);
}

enum hub_activation_type {
	HUB_INIT, HUB_INIT2, HUB_INIT3,
	HUB_POST_RESET, HUB_RESUME, HUB_RESET_RESUME,
};

static void hub_init_func2(struct work_struct *ws);
static void hub_init_func3(struct work_struct *ws);

static void hub_activate(struct usb_hub *hub, enum hub_activation_type type)
{
	struct usb_device *hdev = hub->hdev;
	int port1;
	int status;
	bool need_debounce_delay = false;
	unsigned delay;

	
	if (type == HUB_INIT2)
		goto init2;
	if (type == HUB_INIT3)
		goto init3;

	
	if (type != HUB_RESUME) {

		
		if (type == HUB_INIT) {
			delay = hub_power_on(hub, false);
			PREPARE_DELAYED_WORK(&hub->init_work, hub_init_func2);
			schedule_delayed_work(&hub->init_work,
					msecs_to_jiffies(delay));

			
			atomic_set(&to_usb_interface(hub->intfdev)->
					pm_usage_cnt, 1);
			return;		
		} else {
			hub_power_on(hub, true);
		}
	}
 init2:

	
	for (port1 = 1; port1 <= hdev->maxchild; ++port1) {
		struct usb_device *udev = hdev->children[port1-1];
		u16 portstatus, portchange;

		portstatus = portchange = 0;
		status = hub_port_status(hub, port1, &portstatus, &portchange);
		if (udev || (portstatus & USB_PORT_STAT_CONNECTION))
			dev_dbg(hub->intfdev,
					"port %d: status %04x change %04x\n",
					port1, portstatus, portchange);

		
		if ((portstatus & USB_PORT_STAT_ENABLE) && (
				type != HUB_RESUME ||
				!(portstatus & USB_PORT_STAT_CONNECTION) ||
				!udev ||
				udev->state == USB_STATE_NOTATTACHED)) {
			clear_port_feature(hdev, port1, USB_PORT_FEAT_ENABLE);
			portstatus &= ~USB_PORT_STAT_ENABLE;
		}

		
		if (portchange & USB_PORT_STAT_C_CONNECTION) {
			need_debounce_delay = true;
			clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_CONNECTION);
		}
		if (portchange & USB_PORT_STAT_C_ENABLE) {
			need_debounce_delay = true;
			clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_ENABLE);
		}

		if (!udev || udev->state == USB_STATE_NOTATTACHED) {
			
			if (udev || (portstatus & USB_PORT_STAT_CONNECTION))
				set_bit(port1, hub->change_bits);

		} else if (portstatus & USB_PORT_STAT_ENABLE) {
			
			if (portchange)
				set_bit(port1, hub->change_bits);

		} else if (udev->persist_enabled) {
#ifdef CONFIG_PM
			udev->reset_resume = 1;
#endif
			set_bit(port1, hub->change_bits);

		} else {
			
			usb_set_device_state(udev, USB_STATE_NOTATTACHED);
			set_bit(port1, hub->change_bits);
		}
	}

	
	if (need_debounce_delay) {
		delay = HUB_DEBOUNCE_STABLE;

		
		if (type == HUB_INIT2) {
			PREPARE_DELAYED_WORK(&hub->init_work, hub_init_func3);
			schedule_delayed_work(&hub->init_work,
					msecs_to_jiffies(delay));
			return;		
		} else {
			msleep(delay);
		}
	}
 init3:
	hub->quiescing = 0;

	status = usb_submit_urb(hub->urb, GFP_NOIO);
	if (status < 0)
		dev_err(hub->intfdev, "activate --> %d\n", status);
	if (hub->has_indicators && blinkenlights)
		schedule_delayed_work(&hub->leds, LED_CYCLE_PERIOD);

	
	kick_khubd(hub);
}


static void hub_init_func2(struct work_struct *ws)
{
	struct usb_hub *hub = container_of(ws, struct usb_hub, init_work.work);

	hub_activate(hub, HUB_INIT2);
}

static void hub_init_func3(struct work_struct *ws)
{
	struct usb_hub *hub = container_of(ws, struct usb_hub, init_work.work);

	hub_activate(hub, HUB_INIT3);
}

enum hub_quiescing_type {
	HUB_DISCONNECT, HUB_PRE_RESET, HUB_SUSPEND
};

static void hub_quiesce(struct usb_hub *hub, enum hub_quiescing_type type)
{
	struct usb_device *hdev = hub->hdev;
	int i;

	cancel_delayed_work_sync(&hub->init_work);

	
	hub->quiescing = 1;

	if (type != HUB_SUSPEND) {
		
		for (i = 0; i < hdev->maxchild; ++i) {
			if (hdev->children[i])
				usb_disconnect(&hdev->children[i]);
		}
	}

	
	usb_kill_urb(hub->urb);
	if (hub->has_indicators)
		cancel_delayed_work_sync(&hub->leds);
	if (hub->tt.hub)
		cancel_work_sync(&hub->tt.clear_work);
}


static int hub_pre_reset(struct usb_interface *intf)
{
	struct usb_hub *hub = usb_get_intfdata(intf);

	hub_quiesce(hub, HUB_PRE_RESET);
	return 0;
}


static int hub_post_reset(struct usb_interface *intf)
{
	struct usb_hub *hub = usb_get_intfdata(intf);

	hub_activate(hub, HUB_POST_RESET);
	return 0;
}

static int hub_configure(struct usb_hub *hub,
	struct usb_endpoint_descriptor *endpoint)
{
	struct usb_hcd *hcd;
	struct usb_device *hdev = hub->hdev;
	struct device *hub_dev = hub->intfdev;
	u16 hubstatus, hubchange;
	u16 wHubCharacteristics;
	unsigned int pipe;
	int maxp, ret;
	char *message = "out of memory";

	hub->buffer = usb_buffer_alloc(hdev, sizeof(*hub->buffer), GFP_KERNEL,
			&hub->buffer_dma);
	if (!hub->buffer) {
		ret = -ENOMEM;
		goto fail;
	}

	hub->status = kmalloc(sizeof(*hub->status), GFP_KERNEL);
	if (!hub->status) {
		ret = -ENOMEM;
		goto fail;
	}
	mutex_init(&hub->status_mutex);

	hub->descriptor = kmalloc(sizeof(*hub->descriptor), GFP_KERNEL);
	if (!hub->descriptor) {
		ret = -ENOMEM;
		goto fail;
	}

	
	ret = get_hub_descriptor(hdev, hub->descriptor,
			sizeof(*hub->descriptor));
	if (ret < 0) {
		message = "can't read hub descriptor";
		goto fail;
	} else if (hub->descriptor->bNbrPorts > USB_MAXCHILDREN) {
		message = "hub has too many ports!";
		ret = -ENODEV;
		goto fail;
	}

	hdev->maxchild = hub->descriptor->bNbrPorts;
	dev_info (hub_dev, "%d port%s detected\n", hdev->maxchild,
		(hdev->maxchild == 1) ? "" : "s");

	hub->port_owners = kzalloc(hdev->maxchild * sizeof(void *), GFP_KERNEL);
	if (!hub->port_owners) {
		ret = -ENOMEM;
		goto fail;
	}

	wHubCharacteristics = le16_to_cpu(hub->descriptor->wHubCharacteristics);

	if (wHubCharacteristics & HUB_CHAR_COMPOUND) {
		int	i;
		char	portstr [USB_MAXCHILDREN + 1];

		for (i = 0; i < hdev->maxchild; i++)
			portstr[i] = hub->descriptor->DeviceRemovable
				    [((i + 1) / 8)] & (1 << ((i + 1) % 8))
				? 'F' : 'R';
		portstr[hdev->maxchild] = 0;
		dev_dbg(hub_dev, "compound device; port removable status: %s\n", portstr);
	} else
		dev_dbg(hub_dev, "standalone hub\n");

	switch (wHubCharacteristics & HUB_CHAR_LPSM) {
		case 0x00:
			dev_dbg(hub_dev, "ganged power switching\n");
			break;
		case 0x01:
			dev_dbg(hub_dev, "individual port power switching\n");
			break;
		case 0x02:
		case 0x03:
			dev_dbg(hub_dev, "no power switching (usb 1.0)\n");
			break;
	}

	switch (wHubCharacteristics & HUB_CHAR_OCPM) {
		case 0x00:
			dev_dbg(hub_dev, "global over-current protection\n");
			break;
		case 0x08:
			dev_dbg(hub_dev, "individual port over-current protection\n");
			break;
		case 0x10:
		case 0x18:
			dev_dbg(hub_dev, "no over-current protection\n");
                        break;
	}

	spin_lock_init (&hub->tt.lock);
	INIT_LIST_HEAD (&hub->tt.clear_list);
	INIT_WORK(&hub->tt.clear_work, hub_tt_work);
	switch (hdev->descriptor.bDeviceProtocol) {
		case 0:
			break;
		case 1:
			dev_dbg(hub_dev, "Single TT\n");
			hub->tt.hub = hdev;
			break;
		case 2:
			ret = usb_set_interface(hdev, 0, 1);
			if (ret == 0) {
				dev_dbg(hub_dev, "TT per port\n");
				hub->tt.multi = 1;
			} else
				dev_err(hub_dev, "Using single TT (err %d)\n",
					ret);
			hub->tt.hub = hdev;
			break;
		case 3:
			
			break;
		default:
			dev_dbg(hub_dev, "Unrecognized hub protocol %d\n",
				hdev->descriptor.bDeviceProtocol);
			break;
	}

	
	switch (wHubCharacteristics & HUB_CHAR_TTTT) {
		case HUB_TTTT_8_BITS:
			if (hdev->descriptor.bDeviceProtocol != 0) {
				hub->tt.think_time = 666;
				dev_dbg(hub_dev, "TT requires at most %d "
						"FS bit times (%d ns)\n",
					8, hub->tt.think_time);
			}
			break;
		case HUB_TTTT_16_BITS:
			hub->tt.think_time = 666 * 2;
			dev_dbg(hub_dev, "TT requires at most %d "
					"FS bit times (%d ns)\n",
				16, hub->tt.think_time);
			break;
		case HUB_TTTT_24_BITS:
			hub->tt.think_time = 666 * 3;
			dev_dbg(hub_dev, "TT requires at most %d "
					"FS bit times (%d ns)\n",
				24, hub->tt.think_time);
			break;
		case HUB_TTTT_32_BITS:
			hub->tt.think_time = 666 * 4;
			dev_dbg(hub_dev, "TT requires at most %d "
					"FS bit times (%d ns)\n",
				32, hub->tt.think_time);
			break;
	}

	
	if (wHubCharacteristics & HUB_CHAR_PORTIND) {
		hub->has_indicators = 1;
		dev_dbg(hub_dev, "Port indicators are supported\n");
	}

	dev_dbg(hub_dev, "power on to power good time: %dms\n",
		hub->descriptor->bPwrOn2PwrGood * 2);

	
	ret = usb_get_status(hdev, USB_RECIP_DEVICE, 0, &hubstatus);
	if (ret < 2) {
		message = "can't get hub status";
		goto fail;
	}
	le16_to_cpus(&hubstatus);
	if (hdev == hdev->bus->root_hub) {
		if (hdev->bus_mA == 0 || hdev->bus_mA >= 500)
			hub->mA_per_port = 500;
		else {
			hub->mA_per_port = hdev->bus_mA;
			hub->limited_power = 1;
		}
	} else if ((hubstatus & (1 << USB_DEVICE_SELF_POWERED)) == 0) {
		dev_dbg(hub_dev, "hub controller current requirement: %dmA\n",
			hub->descriptor->bHubContrCurrent);
		hub->limited_power = 1;
		if (hdev->maxchild > 0) {
			int remaining = hdev->bus_mA -
					hub->descriptor->bHubContrCurrent;

			if (remaining < hdev->maxchild * 100)
				dev_warn(hub_dev,
					"insufficient power available "
					"to use all downstream ports\n");
			hub->mA_per_port = 100;		
		}
	} else {	
		
		hub->mA_per_port = 500;
	}
	if (hub->mA_per_port < 500)
		dev_dbg(hub_dev, "%umA bus power budget for each child\n",
				hub->mA_per_port);

	
	hcd = bus_to_hcd(hdev->bus);
	if (hcd->driver->update_hub_device) {
		ret = hcd->driver->update_hub_device(hcd, hdev,
				&hub->tt, GFP_KERNEL);
		if (ret < 0) {
			message = "can't update HCD hub info";
			goto fail;
		}
	}

	ret = hub_hub_status(hub, &hubstatus, &hubchange);
	if (ret < 0) {
		message = "can't get hub status";
		goto fail;
	}

	
	if (hdev->actconfig->desc.bmAttributes & USB_CONFIG_ATT_SELFPOWER)
		dev_dbg(hub_dev, "local power source is %s\n",
			(hubstatus & HUB_STATUS_LOCAL_POWER)
			? "lost (inactive)" : "good");

	if ((wHubCharacteristics & HUB_CHAR_OCPM) == 0)
		dev_dbg(hub_dev, "%sover-current condition exists\n",
			(hubstatus & HUB_STATUS_OVERCURRENT) ? "" : "no ");

	
	pipe = usb_rcvintpipe(hdev, endpoint->bEndpointAddress);
	maxp = usb_maxpacket(hdev, pipe, usb_pipeout(pipe));

	if (maxp > sizeof(*hub->buffer))
		maxp = sizeof(*hub->buffer);

	hub->urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!hub->urb) {
		ret = -ENOMEM;
		goto fail;
	}

	usb_fill_int_urb(hub->urb, hdev, pipe, *hub->buffer, maxp, hub_irq,
		hub, endpoint->bInterval);
	hub->urb->transfer_dma = hub->buffer_dma;
	hub->urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	
	if (hub->has_indicators && blinkenlights)
		hub->indicator [0] = INDICATOR_CYCLE;

	hub_activate(hub, HUB_INIT);
	return 0;

fail:
	dev_err (hub_dev, "config failed, %s (err %d)\n",
			message, ret);
	
	return ret;
}

static void hub_release(struct kref *kref)
{
	struct usb_hub *hub = container_of(kref, struct usb_hub, kref);

	usb_put_intf(to_usb_interface(hub->intfdev));
	kfree(hub);
}

static unsigned highspeed_hubs;

static void hub_disconnect(struct usb_interface *intf)
{
	struct usb_hub *hub = usb_get_intfdata (intf);

	
	spin_lock_irq(&hub_event_lock);
	list_del_init(&hub->event_list);
	hub->disconnected = 1;
	spin_unlock_irq(&hub_event_lock);

	
	hub->error = 0;
	hub_quiesce(hub, HUB_DISCONNECT);

	usb_set_intfdata (intf, NULL);
	hub->hdev->maxchild = 0;

	if (hub->hdev->speed == USB_SPEED_HIGH)
		highspeed_hubs--;

	usb_free_urb(hub->urb);
	kfree(hub->port_owners);
	kfree(hub->descriptor);
	kfree(hub->status);
	usb_buffer_free(hub->hdev, sizeof(*hub->buffer), hub->buffer,
			hub->buffer_dma);

	kref_put(&hub->kref, hub_release);
}

static int hub_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_host_interface *desc;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_device *hdev;
	struct usb_hub *hub;

	desc = intf->cur_altsetting;
	hdev = interface_to_usbdev(intf);

	if (hdev->level == MAX_TOPO_LEVEL) {
		dev_err(&intf->dev,
			"Unsupported bus topology: hub nested too deep\n");
		return -E2BIG;
	}

#ifdef	CONFIG_USB_OTG_BLACKLIST_HUB
	if (hdev->parent) {
		dev_warn(&intf->dev, "ignoring external hub\n");
		return -ENODEV;
	}
#endif

	
	
	if ((desc->desc.bInterfaceSubClass != 0) &&
	    (desc->desc.bInterfaceSubClass != 1)) {
descriptor_error:
		dev_err (&intf->dev, "bad descriptor, ignoring hub\n");
		return -EIO;
	}

	
	if (desc->desc.bNumEndpoints != 1)
		goto descriptor_error;

	endpoint = &desc->endpoint[0].desc;

	
	if (!usb_endpoint_is_int_in(endpoint))
		goto descriptor_error;

	
	dev_info (&intf->dev, "USB hub found\n");

	hub = kzalloc(sizeof(*hub), GFP_KERNEL);
	if (!hub) {
		dev_dbg (&intf->dev, "couldn't kmalloc hub struct\n");
		return -ENOMEM;
	}

	kref_init(&hub->kref);
	INIT_LIST_HEAD(&hub->event_list);
	hub->intfdev = &intf->dev;
	hub->hdev = hdev;
	INIT_DELAYED_WORK(&hub->leds, led_work);
	INIT_DELAYED_WORK(&hub->init_work, NULL);
	usb_get_intf(intf);

	usb_set_intfdata (intf, hub);
	intf->needs_remote_wakeup = 1;

	if (hdev->speed == USB_SPEED_HIGH)
		highspeed_hubs++;

	if (hub_configure(hub, endpoint) >= 0)
		return 0;

	hub_disconnect (intf);
	return -ENODEV;
}

static int
hub_ioctl(struct usb_interface *intf, unsigned int code, void *user_data)
{
	struct usb_device *hdev = interface_to_usbdev (intf);

	
	switch (code) {
	case USBDEVFS_HUB_PORTINFO: {
		struct usbdevfs_hub_portinfo *info = user_data;
		int i;

		spin_lock_irq(&device_state_lock);
		if (hdev->devnum <= 0)
			info->nports = 0;
		else {
			info->nports = hdev->maxchild;
			for (i = 0; i < info->nports; i++) {
				if (hdev->children[i] == NULL)
					info->port[i] = 0;
				else
					info->port[i] =
						hdev->children[i]->devnum;
			}
		}
		spin_unlock_irq(&device_state_lock);

		return info->nports + 1;
		}

	default:
		return -ENOSYS;
	}
}


static int find_port_owner(struct usb_device *hdev, unsigned port1,
		void ***ppowner)
{
	if (hdev->state == USB_STATE_NOTATTACHED)
		return -ENODEV;
	if (port1 == 0 || port1 > hdev->maxchild)
		return -EINVAL;

	
	*ppowner = &(hdev_to_hub(hdev)->port_owners[port1 - 1]);
	return 0;
}


int usb_hub_claim_port(struct usb_device *hdev, unsigned port1, void *owner)
{
	int rc;
	void **powner;

	rc = find_port_owner(hdev, port1, &powner);
	if (rc)
		return rc;
	if (*powner)
		return -EBUSY;
	*powner = owner;
	return rc;
}

int usb_hub_release_port(struct usb_device *hdev, unsigned port1, void *owner)
{
	int rc;
	void **powner;

	rc = find_port_owner(hdev, port1, &powner);
	if (rc)
		return rc;
	if (*powner != owner)
		return -ENOENT;
	*powner = NULL;
	return rc;
}

void usb_hub_release_all_ports(struct usb_device *hdev, void *owner)
{
	int n;
	void **powner;

	n = find_port_owner(hdev, 1, &powner);
	if (n == 0) {
		for (; n < hdev->maxchild; (++n, ++powner)) {
			if (*powner == owner)
				*powner = NULL;
		}
	}
}


bool usb_device_is_owned(struct usb_device *udev)
{
	struct usb_hub *hub;

	if (udev->state == USB_STATE_NOTATTACHED || !udev->parent)
		return false;
	hub = hdev_to_hub(udev->parent);
	return !!hub->port_owners[udev->portnum - 1];
}


static void recursively_mark_NOTATTACHED(struct usb_device *udev)
{
	int i;

	for (i = 0; i < udev->maxchild; ++i) {
		if (udev->children[i])
			recursively_mark_NOTATTACHED(udev->children[i]);
	}
	if (udev->state == USB_STATE_SUSPENDED) {
		udev->discon_suspended = 1;
		udev->active_duration -= jiffies;
	}
	udev->state = USB_STATE_NOTATTACHED;
}


void usb_set_device_state(struct usb_device *udev,
		enum usb_device_state new_state)
{
	unsigned long flags;

	spin_lock_irqsave(&device_state_lock, flags);
	if (udev->state == USB_STATE_NOTATTACHED)
		;	
	else if (new_state != USB_STATE_NOTATTACHED) {

		
		if (udev->parent) {
			if (udev->state == USB_STATE_SUSPENDED
					|| new_state == USB_STATE_SUSPENDED)
				;	
			else if (new_state == USB_STATE_CONFIGURED)
				device_init_wakeup(&udev->dev,
					(udev->actconfig->desc.bmAttributes
					 & USB_CONFIG_ATT_WAKEUP));
			else
				device_init_wakeup(&udev->dev, 0);
		}
		if (udev->state == USB_STATE_SUSPENDED &&
			new_state != USB_STATE_SUSPENDED)
			udev->active_duration -= jiffies;
		else if (new_state == USB_STATE_SUSPENDED &&
				udev->state != USB_STATE_SUSPENDED)
			udev->active_duration += jiffies;
		udev->state = new_state;
	} else
		recursively_mark_NOTATTACHED(udev);
	spin_unlock_irqrestore(&device_state_lock, flags);
}
EXPORT_SYMBOL_GPL(usb_set_device_state);


static void choose_address(struct usb_device *udev)
{
	int		devnum;
	struct usb_bus	*bus = udev->bus;

	
	if (udev->wusb) {
		devnum = udev->portnum + 1;
		BUG_ON(test_bit(devnum, bus->devmap.devicemap));
	} else {
		
		devnum = find_next_zero_bit(bus->devmap.devicemap, 128,
					    bus->devnum_next);
		if (devnum >= 128)
			devnum = find_next_zero_bit(bus->devmap.devicemap,
						    128, 1);
		bus->devnum_next = ( devnum >= 127 ? 1 : devnum + 1);
	}
	if (devnum < 128) {
		set_bit(devnum, bus->devmap.devicemap);
		udev->devnum = devnum;
	}
}

static void release_address(struct usb_device *udev)
{
	if (udev->devnum > 0) {
		clear_bit(udev->devnum, udev->bus->devmap.devicemap);
		udev->devnum = -1;
	}
}

static void update_address(struct usb_device *udev, int devnum)
{
	
	if (!udev->wusb)
		udev->devnum = devnum;
}

#ifdef	CONFIG_USB_SUSPEND

static void usb_stop_pm(struct usb_device *udev)
{
	
	usb_pm_lock(udev);
	if (udev->parent && !udev->discon_suspended)
		usb_autosuspend_device(udev->parent);
	usb_pm_unlock(udev);

	
	cancel_delayed_work_sync(&udev->autosuspend);
	cancel_work_sync(&udev->autoresume);
}

#else

static inline void usb_stop_pm(struct usb_device *udev)
{ }

#endif


void usb_disconnect(struct usb_device **pdev)
{
	struct usb_device	*udev = *pdev;
	int			i;

	if (!udev) {
		pr_debug ("%s nodev\n", __func__);
		return;
	}

	
	usb_set_device_state(udev, USB_STATE_NOTATTACHED);
	dev_info (&udev->dev, "USB disconnect, address %d\n", udev->devnum);

	usb_lock_device(udev);

	
	for (i = 0; i < USB_MAXCHILDREN; i++) {
		if (udev->children[i])
			usb_disconnect(&udev->children[i]);
	}

	
	dev_dbg (&udev->dev, "unregistering device\n");
	usb_disable_device(udev, 0);
	usb_hcd_synchronize_unlinks(udev);

	usb_remove_ep_devs(&udev->ep0);
	usb_unlock_device(udev);

	
	device_del(&udev->dev);

	
	release_address(udev);

	
	spin_lock_irq(&device_state_lock);
	*pdev = NULL;
	spin_unlock_irq(&device_state_lock);

	usb_stop_pm(udev);

	put_device(&udev->dev);
}

#ifdef CONFIG_USB_ANNOUNCE_NEW_DEVICES
static void show_string(struct usb_device *udev, char *id, char *string)
{
	if (!string)
		return;
	dev_printk(KERN_INFO, &udev->dev, "%s: %s\n", id, string);
}

static void announce_device(struct usb_device *udev)
{
	dev_info(&udev->dev, "New USB device found, idVendor=%04x, idProduct=%04x\n",
		le16_to_cpu(udev->descriptor.idVendor),
		le16_to_cpu(udev->descriptor.idProduct));
	dev_info(&udev->dev,
		"New USB device strings: Mfr=%d, Product=%d, SerialNumber=%d\n",
		udev->descriptor.iManufacturer,
		udev->descriptor.iProduct,
		udev->descriptor.iSerialNumber);
	show_string(udev, "Product", udev->product);
	show_string(udev, "Manufacturer", udev->manufacturer);
	show_string(udev, "SerialNumber", udev->serial);
}
#else
static inline void announce_device(struct usb_device *udev) { }
#endif

#ifdef	CONFIG_USB_OTG
#include "otg_whitelist.h"
#endif


static int usb_enumerate_device_otg(struct usb_device *udev)
{
	int err = 0;

#ifdef	CONFIG_USB_OTG
	
	if (!udev->bus->is_b_host
			&& udev->config
			&& udev->parent == udev->bus->root_hub) {
		struct usb_otg_descriptor	*desc = 0;
		struct usb_bus			*bus = udev->bus;

		
		if (__usb_get_extra_descriptor (udev->rawdescriptors[0],
					le16_to_cpu(udev->config[0].desc.wTotalLength),
					USB_DT_OTG, (void **) &desc) == 0) {
			if (desc->bmAttributes & USB_OTG_HNP) {
				unsigned		port1 = udev->portnum;

				dev_info(&udev->dev,
					"Dual-Role OTG device on %sHNP port\n",
					(port1 == bus->otg_port)
						? "" : "non-");

				
				if (port1 == bus->otg_port)
					bus->b_hnp_enable = 1;
				err = usb_control_msg(udev,
					usb_sndctrlpipe(udev, 0),
					USB_REQ_SET_FEATURE, 0,
					bus->b_hnp_enable
						? USB_DEVICE_B_HNP_ENABLE
						: USB_DEVICE_A_ALT_HNP_SUPPORT,
					0, NULL, 0, USB_CTRL_SET_TIMEOUT);
				if (err < 0) {
					
					dev_info(&udev->dev,
						"can't set HNP mode: %d\n",
						err);
					bus->b_hnp_enable = 0;
				}
			}
		}
	}

	if (!is_targeted(udev)) {

		
		if (udev->bus->b_hnp_enable || udev->bus->is_b_host) {
			err = usb_port_suspend(udev, PMSG_SUSPEND);
			if (err < 0)
				dev_dbg(&udev->dev, "HNP fail, %d\n", err);
		}
		err = -ENOTSUPP;
		goto fail;
	}
fail:
#endif
	return err;
}



static int usb_enumerate_device(struct usb_device *udev)
{
	int err;

	if (udev->config == NULL) {
		err = usb_get_configuration(udev);
		if (err < 0) {
			dev_err(&udev->dev, "can't read configurations, error %d\n",
				err);
			goto fail;
		}
	}
	if (udev->wusb == 1 && udev->authorized == 0) {
		udev->product = kstrdup("n/a (unauthorized)", GFP_KERNEL);
		udev->manufacturer = kstrdup("n/a (unauthorized)", GFP_KERNEL);
		udev->serial = kstrdup("n/a (unauthorized)", GFP_KERNEL);
	}
	else {
		
		udev->product = usb_cache_string(udev, udev->descriptor.iProduct);
		udev->manufacturer = usb_cache_string(udev,
						      udev->descriptor.iManufacturer);
		udev->serial = usb_cache_string(udev, udev->descriptor.iSerialNumber);
	}
	err = usb_enumerate_device_otg(udev);
fail:
	return err;
}



int usb_new_device(struct usb_device *udev)
{
	int err;

	
	if (udev->parent)
		usb_autoresume_device(udev->parent);

	usb_detect_quirks(udev);
	err = usb_enumerate_device(udev);	
	if (err < 0)
		goto fail;
	dev_dbg(&udev->dev, "udev %d, busnum %d, minor = %d\n",
			udev->devnum, udev->bus->busnum,
			(((udev->bus->busnum-1) * 128) + (udev->devnum-1)));
	
	udev->dev.devt = MKDEV(USB_DEVICE_MAJOR,
			(((udev->bus->busnum-1) * 128) + (udev->devnum-1)));

	
	announce_device(udev);

	
	err = device_add(&udev->dev);
	if (err) {
		dev_err(&udev->dev, "can't device_add, error %d\n", err);
		goto fail;
	}

	(void) usb_create_ep_devs(&udev->dev, &udev->ep0, udev);
	return err;

fail:
	usb_set_device_state(udev, USB_STATE_NOTATTACHED);
	usb_stop_pm(udev);
	return err;
}



int usb_deauthorize_device(struct usb_device *usb_dev)
{
	usb_lock_device(usb_dev);
	if (usb_dev->authorized == 0)
		goto out_unauthorized;

	usb_dev->authorized = 0;
	usb_set_configuration(usb_dev, -1);

	kfree(usb_dev->product);
	usb_dev->product = kstrdup("n/a (unauthorized)", GFP_KERNEL);
	kfree(usb_dev->manufacturer);
	usb_dev->manufacturer = kstrdup("n/a (unauthorized)", GFP_KERNEL);
	kfree(usb_dev->serial);
	usb_dev->serial = kstrdup("n/a (unauthorized)", GFP_KERNEL);

	usb_destroy_configuration(usb_dev);
	usb_dev->descriptor.bNumConfigurations = 0;

out_unauthorized:
	usb_unlock_device(usb_dev);
	return 0;
}


int usb_authorize_device(struct usb_device *usb_dev)
{
	int result = 0, c;

	usb_lock_device(usb_dev);
	if (usb_dev->authorized == 1)
		goto out_authorized;

	result = usb_autoresume_device(usb_dev);
	if (result < 0) {
		dev_err(&usb_dev->dev,
			"can't autoresume for authorization: %d\n", result);
		goto error_autoresume;
	}
	result = usb_get_device_descriptor(usb_dev, sizeof(usb_dev->descriptor));
	if (result < 0) {
		dev_err(&usb_dev->dev, "can't re-read device descriptor for "
			"authorization: %d\n", result);
		goto error_device_descriptor;
	}

	kfree(usb_dev->product);
	usb_dev->product = NULL;
	kfree(usb_dev->manufacturer);
	usb_dev->manufacturer = NULL;
	kfree(usb_dev->serial);
	usb_dev->serial = NULL;

	usb_dev->authorized = 1;
	result = usb_enumerate_device(usb_dev);
	if (result < 0)
		goto error_enumerate;
	
	c = usb_choose_configuration(usb_dev);
	if (c >= 0) {
		result = usb_set_configuration(usb_dev, c);
		if (result) {
			dev_err(&usb_dev->dev,
				"can't set config #%d, error %d\n", c, result);
			
		}
	}
	dev_info(&usb_dev->dev, "authorized to connect\n");

error_enumerate:
error_device_descriptor:
	usb_autosuspend_device(usb_dev);
error_autoresume:
out_authorized:
	usb_unlock_device(usb_dev);	
	return result;
}



static unsigned hub_is_wusb(struct usb_hub *hub)
{
	struct usb_hcd *hcd;
	if (hub->hdev->parent != NULL)  
		return 0;
	hcd = container_of(hub->hdev->bus, struct usb_hcd, self);
	return hcd->wireless;
}


#define PORT_RESET_TRIES	5
#define SET_ADDRESS_TRIES	2
#define GET_DESCRIPTOR_TRIES	2
#define SET_CONFIG_TRIES	(2 * (use_both_schemes + 1))
#define USE_NEW_SCHEME(i)	((i) / 2 == old_scheme_first)

#define HUB_ROOT_RESET_TIME	50	
#define HUB_SHORT_RESET_TIME	10
#define HUB_LONG_RESET_TIME	200
#define HUB_RESET_TIMEOUT	500

static int hub_port_wait_reset(struct usb_hub *hub, int port1,
				struct usb_device *udev, unsigned int delay)
{
	int delay_time, ret;
	u16 portstatus;
	u16 portchange;

	for (delay_time = 0;
			delay_time < HUB_RESET_TIMEOUT;
			delay_time += delay) {
		
		msleep(delay);

		
		ret = hub_port_status(hub, port1, &portstatus, &portchange);
		if (ret < 0)
			return ret;

		
		if (!(portstatus & USB_PORT_STAT_CONNECTION))
			return -ENOTCONN;

		
		if ((portchange & USB_PORT_STAT_C_CONNECTION))
			return -ENOTCONN;

		
		if (!(portstatus & USB_PORT_STAT_RESET) &&
		    (portstatus & USB_PORT_STAT_ENABLE)) {
			if (hub_is_wusb(hub))
				udev->speed = USB_SPEED_VARIABLE;
			else if (portstatus & USB_PORT_STAT_HIGH_SPEED)
				udev->speed = USB_SPEED_HIGH;
			else if (portstatus & USB_PORT_STAT_LOW_SPEED)
				udev->speed = USB_SPEED_LOW;
			else
				udev->speed = USB_SPEED_FULL;
			return 0;
		}

		
		if (delay_time >= 2 * HUB_SHORT_RESET_TIME)
			delay = HUB_LONG_RESET_TIME;

		dev_dbg (hub->intfdev,
			"port %d not reset yet, waiting %dms\n",
			port1, delay);
	}

	return -EBUSY;
}

static int hub_port_reset(struct usb_hub *hub, int port1,
				struct usb_device *udev, unsigned int delay)
{
	int i, status;

	
	down_read(&ehci_cf_port_reset_rwsem);

	
	for (i = 0; i < PORT_RESET_TRIES; i++) {
		status = set_port_feature(hub->hdev,
				port1, USB_PORT_FEAT_RESET);
		if (status)
			dev_err(hub->intfdev,
					"cannot reset port %d (err = %d)\n",
					port1, status);
		else {
			status = hub_port_wait_reset(hub, port1, udev, delay);
			if (status && status != -ENOTCONN)
				dev_dbg(hub->intfdev,
						"port_wait_reset: err = %d\n",
						status);
		}

		
		switch (status) {
		case 0:
			
			msleep(10 + 40);
			update_address(udev, 0);
			
		case -ENOTCONN:
		case -ENODEV:
			clear_port_feature(hub->hdev,
				port1, USB_PORT_FEAT_C_RESET);
			
			usb_set_device_state(udev, status
					? USB_STATE_NOTATTACHED
					: USB_STATE_DEFAULT);
			goto done;
		}

		dev_dbg (hub->intfdev,
			"port %d not enabled, trying reset again...\n",
			port1);
		delay = HUB_LONG_RESET_TIME;
	}

	dev_err (hub->intfdev,
		"Cannot enable port %i.  Maybe the USB cable is bad?\n",
		port1);

 done:
	up_read(&ehci_cf_port_reset_rwsem);
	return status;
}

#ifdef	CONFIG_PM

#define MASK_BITS	(USB_PORT_STAT_POWER | USB_PORT_STAT_CONNECTION | \
				USB_PORT_STAT_SUSPEND)
#define WANT_BITS	(USB_PORT_STAT_POWER | USB_PORT_STAT_CONNECTION)


static int check_port_resume_type(struct usb_device *udev,
		struct usb_hub *hub, int port1,
		int status, unsigned portchange, unsigned portstatus)
{
	
	if (status || (portstatus & MASK_BITS) != WANT_BITS) {
		if (status >= 0)
			status = -ENODEV;
	}

	
	else if (!(portstatus & USB_PORT_STAT_ENABLE) && !udev->reset_resume) {
		if (udev->persist_enabled)
			udev->reset_resume = 1;
		else
			status = -ENODEV;
	}

	if (status) {
		dev_dbg(hub->intfdev,
				"port %d status %04x.%04x after resume, %d\n",
				port1, portchange, portstatus, status);
	} else if (udev->reset_resume) {

		
		if (portchange & USB_PORT_STAT_C_CONNECTION)
			clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_CONNECTION);
		if (portchange & USB_PORT_STAT_C_ENABLE)
			clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_ENABLE);
	}

	return status;
}

#ifdef	CONFIG_USB_SUSPEND


int usb_port_suspend(struct usb_device *udev, pm_message_t msg)
{
	struct usb_hub	*hub = hdev_to_hub(udev->parent);
	int		port1 = udev->portnum;
	int		status;

	

	
	if (udev->do_remote_wakeup) {
		status = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
				USB_REQ_SET_FEATURE, USB_RECIP_DEVICE,
				USB_DEVICE_REMOTE_WAKEUP, 0,
				NULL, 0,
				USB_CTRL_SET_TIMEOUT);
		if (status)
			dev_dbg(&udev->dev, "won't remote wakeup, status %d\n",
					status);
	}

	
	status = set_port_feature(hub->hdev, port1, USB_PORT_FEAT_SUSPEND);
	if (status) {
		dev_dbg(hub->intfdev, "can't suspend port %d, status %d\n",
				port1, status);
		
		(void) usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
				USB_REQ_CLEAR_FEATURE, USB_RECIP_DEVICE,
				USB_DEVICE_REMOTE_WAKEUP, 0,
				NULL, 0,
				USB_CTRL_SET_TIMEOUT);
	} else {
		
		dev_dbg(&udev->dev, "usb %ssuspend\n",
				(msg.event & PM_EVENT_AUTO ? "auto-" : ""));
		usb_set_device_state(udev, USB_STATE_SUSPENDED);
		msleep(10);
	}
	return status;
}


static int finish_port_resume(struct usb_device *udev)
{
	int	status = 0;
	u16	devstatus;

	
	dev_dbg(&udev->dev, "%s\n",
		udev->reset_resume ? "finish reset-resume" : "finish resume");

	
	usb_set_device_state(udev, udev->actconfig
			? USB_STATE_CONFIGURED
			: USB_STATE_ADDRESS);

	
	if (udev->reset_resume)
 retry_reset_resume:
		status = usb_reset_and_verify_device(udev);

 	
	if (status == 0) {
		devstatus = 0;
		status = usb_get_status(udev, USB_RECIP_DEVICE, 0, &devstatus);
		if (status >= 0)
			status = (status > 0 ? 0 : -ENODEV);

		
		if (status && !udev->reset_resume && udev->persist_enabled) {
			dev_dbg(&udev->dev, "retry with reset-resume\n");
			udev->reset_resume = 1;
			goto retry_reset_resume;
		}
	}

	if (status) {
		dev_dbg(&udev->dev, "gone after usb resume? status %d\n",
				status);
	} else if (udev->actconfig) {
		le16_to_cpus(&devstatus);
		if (devstatus & (1 << USB_DEVICE_REMOTE_WAKEUP)) {
			status = usb_control_msg(udev,
					usb_sndctrlpipe(udev, 0),
					USB_REQ_CLEAR_FEATURE,
						USB_RECIP_DEVICE,
					USB_DEVICE_REMOTE_WAKEUP, 0,
					NULL, 0,
					USB_CTRL_SET_TIMEOUT);
			if (status)
				dev_dbg(&udev->dev,
					"disable remote wakeup, status %d\n",
					status);
		}
		status = 0;
	}
	return status;
}


int usb_port_resume(struct usb_device *udev, pm_message_t msg)
{
	struct usb_hub	*hub = hdev_to_hub(udev->parent);
	int		port1 = udev->portnum;
	int		status;
	u16		portchange, portstatus;

	
	status = hub_port_status(hub, port1, &portstatus, &portchange);
	if (status == 0 && !(portstatus & USB_PORT_STAT_SUSPEND))
		goto SuspendCleared;

	

	set_bit(port1, hub->busy_bits);

	
	status = clear_port_feature(hub->hdev,
			port1, USB_PORT_FEAT_SUSPEND);
	if (status) {
		dev_dbg(hub->intfdev, "can't resume port %d, status %d\n",
				port1, status);
	} else {
		
		dev_dbg(&udev->dev, "usb %sresume\n",
				(msg.event & PM_EVENT_AUTO ? "auto-" : ""));
		msleep(25);

		
		status = hub_port_status(hub, port1, &portstatus, &portchange);

		
		msleep(10);
	}

 SuspendCleared:
	if (status == 0) {
		if (portchange & USB_PORT_STAT_C_SUSPEND)
			clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_SUSPEND);
	}

	clear_bit(port1, hub->busy_bits);

	status = check_port_resume_type(udev,
			hub, port1, status, portchange, portstatus);
	if (status == 0)
		status = finish_port_resume(udev);
	if (status < 0) {
		dev_dbg(&udev->dev, "can't resume, status %d\n", status);
		hub_port_logical_disconnect(hub, port1);
	}
	return status;
}


static int remote_wakeup(struct usb_device *udev)
{
	int	status = 0;

	if (udev->state == USB_STATE_SUSPENDED) {
		dev_dbg(&udev->dev, "usb %sresume\n", "wakeup-");
		usb_mark_last_busy(udev);
		status = usb_external_resume_device(udev, PMSG_REMOTE_RESUME);
	}
	return status;
}

#else	



int usb_port_suspend(struct usb_device *udev, pm_message_t msg)
{
	return 0;
}



int usb_port_resume(struct usb_device *udev, pm_message_t msg)
{
	struct usb_hub	*hub = hdev_to_hub(udev->parent);
	int		port1 = udev->portnum;
	int		status;
	u16		portchange, portstatus;

	status = hub_port_status(hub, port1, &portstatus, &portchange);
	status = check_port_resume_type(udev,
			hub, port1, status, portchange, portstatus);

	if (status) {
		dev_dbg(&udev->dev, "can't resume, status %d\n", status);
		hub_port_logical_disconnect(hub, port1);
	} else if (udev->reset_resume) {
		dev_dbg(&udev->dev, "reset-resume\n");
		status = usb_reset_and_verify_device(udev);
	}
	return status;
}

static inline int remote_wakeup(struct usb_device *udev)
{
	return 0;
}

#endif

static int hub_suspend(struct usb_interface *intf, pm_message_t msg)
{
	struct usb_hub		*hub = usb_get_intfdata (intf);
	struct usb_device	*hdev = hub->hdev;
	unsigned		port1;

	
	for (port1 = 1; port1 <= hdev->maxchild; port1++) {
		struct usb_device	*udev;

		udev = hdev->children [port1-1];
		if (udev && udev->can_submit) {
			if (!(msg.event & PM_EVENT_AUTO))
				dev_dbg(&intf->dev, "port %d nyet suspended\n",
						port1);
			return -EBUSY;
		}
	}

	dev_dbg(&intf->dev, "%s\n", __func__);

	
	hub_quiesce(hub, HUB_SUSPEND);
	return 0;
}

static int hub_resume(struct usb_interface *intf)
{
	struct usb_hub *hub = usb_get_intfdata(intf);

	dev_dbg(&intf->dev, "%s\n", __func__);
	hub_activate(hub, HUB_RESUME);
	return 0;
}

static int hub_reset_resume(struct usb_interface *intf)
{
	struct usb_hub *hub = usb_get_intfdata(intf);

	dev_dbg(&intf->dev, "%s\n", __func__);
	hub_activate(hub, HUB_RESET_RESUME);
	return 0;
}


void usb_root_hub_lost_power(struct usb_device *rhdev)
{
	dev_warn(&rhdev->dev, "root hub lost power or was reset\n");
	rhdev->reset_resume = 1;
}
EXPORT_SYMBOL_GPL(usb_root_hub_lost_power);

#else	

static inline int remote_wakeup(struct usb_device *udev)
{
	return 0;
}

#define hub_suspend		NULL
#define hub_resume		NULL
#define hub_reset_resume	NULL
#endif



static int hub_port_debounce(struct usb_hub *hub, int port1)
{
	int ret;
	int total_time, stable_time = 0;
	u16 portchange, portstatus;
	unsigned connection = 0xffff;

	for (total_time = 0; ; total_time += HUB_DEBOUNCE_STEP) {
		ret = hub_port_status(hub, port1, &portstatus, &portchange);
		if (ret < 0)
			return ret;

		if (!(portchange & USB_PORT_STAT_C_CONNECTION) &&
		     (portstatus & USB_PORT_STAT_CONNECTION) == connection) {
			stable_time += HUB_DEBOUNCE_STEP;
			if (stable_time >= HUB_DEBOUNCE_STABLE)
				break;
		} else {
			stable_time = 0;
			connection = portstatus & USB_PORT_STAT_CONNECTION;
		}

		if (portchange & USB_PORT_STAT_C_CONNECTION) {
			clear_port_feature(hub->hdev, port1,
					USB_PORT_FEAT_C_CONNECTION);
		}

		if (total_time >= HUB_DEBOUNCE_TIMEOUT)
			break;
		msleep(HUB_DEBOUNCE_STEP);
	}

	dev_dbg (hub->intfdev,
		"debounce: port %d: total %dms stable %dms status 0x%x\n",
		port1, total_time, stable_time, portstatus);

	if (stable_time < HUB_DEBOUNCE_STABLE)
		return -ETIMEDOUT;
	return portstatus;
}

void usb_ep0_reinit(struct usb_device *udev)
{
	usb_disable_endpoint(udev, 0 + USB_DIR_IN, true);
	usb_disable_endpoint(udev, 0 + USB_DIR_OUT, true);
	usb_enable_endpoint(udev, &udev->ep0, true);
}
EXPORT_SYMBOL_GPL(usb_ep0_reinit);

#define usb_sndaddr0pipe()	(PIPE_CONTROL << 30)
#define usb_rcvaddr0pipe()	((PIPE_CONTROL << 30) | USB_DIR_IN)

static int hub_set_address(struct usb_device *udev, int devnum)
{
	int retval;
	struct usb_hcd *hcd = bus_to_hcd(udev->bus);

	
	if (!hcd->driver->address_device && devnum <= 1)
		return -EINVAL;
	if (udev->state == USB_STATE_ADDRESS)
		return 0;
	if (udev->state != USB_STATE_DEFAULT)
		return -EINVAL;
	if (hcd->driver->address_device) {
		retval = hcd->driver->address_device(hcd, udev);
	} else {
		retval = usb_control_msg(udev, usb_sndaddr0pipe(),
				USB_REQ_SET_ADDRESS, 0, devnum, 0,
				NULL, 0, USB_CTRL_SET_TIMEOUT);
		if (retval == 0)
			update_address(udev, devnum);
	}
	if (retval == 0) {
		
		usb_set_device_state(udev, USB_STATE_ADDRESS);
		usb_ep0_reinit(udev);
	}
	return retval;
}


static int
hub_port_init (struct usb_hub *hub, struct usb_device *udev, int port1,
		int retry_counter)
{
	static DEFINE_MUTEX(usb_address0_mutex);

	struct usb_device	*hdev = hub->hdev;
	struct usb_hcd		*hcd = bus_to_hcd(hdev->bus);
	int			i, j, retval;
	unsigned		delay = HUB_SHORT_RESET_TIME;
	enum usb_device_speed	oldspeed = udev->speed;
	char 			*speed, *type;
	int			devnum = udev->devnum;

	
	if (!hdev->parent) {
		delay = HUB_ROOT_RESET_TIME;
		if (port1 == hdev->bus->otg_port)
			hdev->bus->b_hnp_enable = 0;
	}

	
	
	if (oldspeed == USB_SPEED_LOW)
		delay = HUB_LONG_RESET_TIME;

	mutex_lock(&usb_address0_mutex);

	if ((hcd->driver->flags & HCD_USB3) && udev->config) {
		
		dev_dbg(&udev->dev,
				"xHCI reset of configured device "
				"not supported yet.\n");
		retval = -EINVAL;
		goto fail;
	} else if (!udev->config && oldspeed == USB_SPEED_SUPER) {
		
		usb_set_device_state(udev, USB_STATE_DEFAULT);
	} else {
		
		
		retval = hub_port_reset(hub, port1, udev, delay);
		if (retval < 0)		
			goto fail;
		
	}
	retval = -ENODEV;

	if (oldspeed != USB_SPEED_UNKNOWN && oldspeed != udev->speed) {
		dev_dbg(&udev->dev, "device reset changed speed!\n");
		goto fail;
	}
	oldspeed = udev->speed;

	
	switch (udev->speed) {
	case USB_SPEED_SUPER:
	case USB_SPEED_VARIABLE:	
		udev->ep0.desc.wMaxPacketSize = cpu_to_le16(512);
		break;
	case USB_SPEED_HIGH:		
		udev->ep0.desc.wMaxPacketSize = cpu_to_le16(64);
		break;
	case USB_SPEED_FULL:		
		
		udev->ep0.desc.wMaxPacketSize = cpu_to_le16(64);
		break;
	case USB_SPEED_LOW:		
		udev->ep0.desc.wMaxPacketSize = cpu_to_le16(8);
		break;
	default:
		goto fail;
	}
 
	type = "";
	switch (udev->speed) {
	case USB_SPEED_LOW:	speed = "low";	break;
	case USB_SPEED_FULL:	speed = "full";	break;
	case USB_SPEED_HIGH:	speed = "high";	break;
	case USB_SPEED_SUPER:
				speed = "super";
				break;
	case USB_SPEED_VARIABLE:
				speed = "variable";
				type = "Wireless ";
				break;
	default: 		speed = "?";	break;
	}
	if (udev->speed != USB_SPEED_SUPER)
		dev_info(&udev->dev,
				"%s %s speed %sUSB device using %s and address %d\n",
				(udev->config) ? "reset" : "new", speed, type,
				udev->bus->controller->driver->name, devnum);

	
	if (hdev->tt) {
		udev->tt = hdev->tt;
		udev->ttport = hdev->ttport;
	} else if (udev->speed != USB_SPEED_HIGH
			&& hdev->speed == USB_SPEED_HIGH) {
		udev->tt = &hub->tt;
		udev->ttport = port1;
	}
 
	
	for (i = 0; i < GET_DESCRIPTOR_TRIES; (++i, msleep(100))) {
		
		if (USE_NEW_SCHEME(retry_counter) && !(hcd->driver->flags & HCD_USB3)) {
			struct usb_device_descriptor *buf;
			int r = 0;

#define GET_DESCRIPTOR_BUFSIZE	64
			buf = kmalloc(GET_DESCRIPTOR_BUFSIZE, GFP_NOIO);
			if (!buf) {
				retval = -ENOMEM;
				continue;
			}

			
			for (j = 0; j < 3; ++j) {
				buf->bMaxPacketSize0 = 0;
				r = usb_control_msg(udev, usb_rcvaddr0pipe(),
					USB_REQ_GET_DESCRIPTOR, USB_DIR_IN,
					USB_DT_DEVICE << 8, 0,
					buf, GET_DESCRIPTOR_BUFSIZE,
					initial_descriptor_timeout);
				switch (buf->bMaxPacketSize0) {
				case 8: case 16: case 32: case 64: case 255:
					if (buf->bDescriptorType ==
							USB_DT_DEVICE) {
						r = 0;
						break;
					}
					
				default:
					if (r == 0)
						r = -EPROTO;
					break;
				}
				if (r == 0)
					break;
			}
			udev->descriptor.bMaxPacketSize0 =
					buf->bMaxPacketSize0;
			kfree(buf);

			retval = hub_port_reset(hub, port1, udev, delay);
			if (retval < 0)		
				goto fail;
			if (oldspeed != udev->speed) {
				dev_dbg(&udev->dev,
					"device reset changed speed!\n");
				retval = -ENODEV;
				goto fail;
			}
			if (r) {
				dev_err(&udev->dev,
					"device descriptor read/64, error %d\n",
					r);
				retval = -EMSGSIZE;
				continue;
			}
#undef GET_DESCRIPTOR_BUFSIZE
		}

 		
		if (udev->wusb == 0) {
			for (j = 0; j < SET_ADDRESS_TRIES; ++j) {
				retval = hub_set_address(udev, devnum);
				if (retval >= 0)
					break;
				msleep(200);
			}
			if (retval < 0) {
				dev_err(&udev->dev,
					"device not accepting address %d, error %d\n",
					devnum, retval);
				goto fail;
			}
			if (udev->speed == USB_SPEED_SUPER) {
				devnum = udev->devnum;
				dev_info(&udev->dev,
						"%s SuperSpeed USB device using %s and address %d\n",
						(udev->config) ? "reset" : "new",
						udev->bus->controller->driver->name, devnum);
			}

			
			msleep(10);
			if (USE_NEW_SCHEME(retry_counter) && !(hcd->driver->flags & HCD_USB3))
				break;
  		}

		retval = usb_get_device_descriptor(udev, 8);
		if (retval < 8) {
			dev_err(&udev->dev,
					"device descriptor read/8, error %d\n",
					retval);
			if (retval >= 0)
				retval = -EMSGSIZE;
		} else {
			retval = 0;
			break;
		}
	}
	if (retval)
		goto fail;

	if (udev->descriptor.bMaxPacketSize0 == 0xff ||
			udev->speed == USB_SPEED_SUPER)
		i = 512;
	else
		i = udev->descriptor.bMaxPacketSize0;
	if (le16_to_cpu(udev->ep0.desc.wMaxPacketSize) != i) {
		if (udev->speed != USB_SPEED_FULL ||
				!(i == 8 || i == 16 || i == 32 || i == 64)) {
			dev_err(&udev->dev, "ep0 maxpacket = %d\n", i);
			retval = -EMSGSIZE;
			goto fail;
		}
		dev_dbg(&udev->dev, "ep0 maxpacket = %d\n", i);
		udev->ep0.desc.wMaxPacketSize = cpu_to_le16(i);
		usb_ep0_reinit(udev);
	}
  
	retval = usb_get_device_descriptor(udev, USB_DT_DEVICE_SIZE);
	if (retval < (signed)sizeof(udev->descriptor)) {
		dev_err(&udev->dev, "device descriptor read/all, error %d\n",
			retval);
		if (retval >= 0)
			retval = -ENOMSG;
		goto fail;
	}

	retval = 0;

fail:
	if (retval) {
		hub_port_disable(hub, port1, 0);
		update_address(udev, devnum);	
	}
	mutex_unlock(&usb_address0_mutex);
	return retval;
}

static void
check_highspeed (struct usb_hub *hub, struct usb_device *udev, int port1)
{
	struct usb_qualifier_descriptor	*qual;
	int				status;

	qual = kmalloc (sizeof *qual, GFP_KERNEL);
	if (qual == NULL)
		return;

	status = usb_get_descriptor (udev, USB_DT_DEVICE_QUALIFIER, 0,
			qual, sizeof *qual);
	if (status == sizeof *qual) {
		dev_info(&udev->dev, "not running at top speed; "
			"connect to a high speed hub\n");
		
		if (hub->has_indicators) {
			hub->indicator[port1-1] = INDICATOR_GREEN_BLINK;
			schedule_delayed_work (&hub->leds, 0);
		}
	}
	kfree(qual);
}

static unsigned
hub_power_remaining (struct usb_hub *hub)
{
	struct usb_device *hdev = hub->hdev;
	int remaining;
	int port1;

	if (!hub->limited_power)
		return 0;

	remaining = hdev->bus_mA - hub->descriptor->bHubContrCurrent;
	for (port1 = 1; port1 <= hdev->maxchild; ++port1) {
		struct usb_device	*udev = hdev->children[port1 - 1];
		int			delta;

		if (!udev)
			continue;

		
		if (udev->actconfig)
			delta = udev->actconfig->desc.bMaxPower * 2;
		else if (port1 != udev->bus->otg_port || hdev->parent)
			delta = 100;
		else
			delta = 8;
		if (delta > hub->mA_per_port)
			dev_warn(&udev->dev,
				 "%dmA is over %umA budget for port %d!\n",
				 delta, hub->mA_per_port, port1);
		remaining -= delta;
	}
	if (remaining < 0) {
		dev_warn(hub->intfdev, "%dmA over power budget!\n",
			- remaining);
		remaining = 0;
	}
	return remaining;
}


static void hub_port_connect_change(struct usb_hub *hub, int port1,
					u16 portstatus, u16 portchange)
{
	struct usb_device *hdev = hub->hdev;
	struct device *hub_dev = hub->intfdev;
	struct usb_hcd *hcd = bus_to_hcd(hdev->bus);
	unsigned wHubCharacteristics =
			le16_to_cpu(hub->descriptor->wHubCharacteristics);
	struct usb_device *udev;
	int status, i;

	dev_dbg (hub_dev,
		"port %d, status %04x, change %04x, %s\n",
		port1, portstatus, portchange, portspeed (portstatus));

	if (hub->has_indicators) {
		set_port_led(hub, port1, HUB_LED_AUTO);
		hub->indicator[port1-1] = INDICATOR_AUTO;
	}

#ifdef	CONFIG_USB_OTG
	
	if (hdev->bus->is_b_host)
		portchange &= ~(USB_PORT_STAT_C_CONNECTION |
				USB_PORT_STAT_C_ENABLE);
#endif

	
	udev = hdev->children[port1-1];
	if ((portstatus & USB_PORT_STAT_CONNECTION) && udev &&
			udev->state != USB_STATE_NOTATTACHED) {
		usb_lock_device(udev);
		if (portstatus & USB_PORT_STAT_ENABLE) {
			status = 0;		

#ifdef CONFIG_USB_SUSPEND
		} else if (udev->state == USB_STATE_SUSPENDED &&
				udev->persist_enabled) {
			
			status = remote_wakeup(udev);
#endif

		} else {
			status = -ENODEV;	
		}
		usb_unlock_device(udev);

		if (status == 0) {
			clear_bit(port1, hub->change_bits);
			return;
		}
	}

	
	if (udev)
		usb_disconnect(&hdev->children[port1-1]);
	clear_bit(port1, hub->change_bits);

	if (portchange & (USB_PORT_STAT_C_CONNECTION |
				USB_PORT_STAT_C_ENABLE)) {
		status = hub_port_debounce(hub, port1);
		if (status < 0) {
			if (printk_ratelimit())
				dev_err(hub_dev, "connect-debounce failed, "
						"port %d disabled\n", port1);
			portstatus &= ~USB_PORT_STAT_CONNECTION;
		} else {
			portstatus = status;
		}
	}

	
	if (!(portstatus & USB_PORT_STAT_CONNECTION)) {

		
		if ((wHubCharacteristics & HUB_CHAR_LPSM) < 2
				&& !(portstatus & (1 << USB_PORT_FEAT_POWER)))
			set_port_feature(hdev, port1, USB_PORT_FEAT_POWER);

		if (portstatus & USB_PORT_STAT_ENABLE)
  			goto done;
		return;
	}

	for (i = 0; i < SET_CONFIG_TRIES; i++) {

		
		udev = usb_alloc_dev(hdev, hdev->bus, port1);
		if (!udev) {
			dev_err (hub_dev,
				"couldn't allocate port %d usb_device\n",
				port1);
			goto done;
		}

		usb_set_device_state(udev, USB_STATE_POWERED);
 		udev->bus_mA = hub->mA_per_port;
		udev->level = hdev->level + 1;
		udev->wusb = hub_is_wusb(hub);

		

		if (!(hcd->driver->flags & HCD_USB3))
			udev->speed = USB_SPEED_UNKNOWN;
		else if ((hdev->parent == NULL) &&
				(portstatus & (1 << USB_PORT_FEAT_SUPERSPEED)))
			udev->speed = USB_SPEED_SUPER;
		else
			udev->speed = USB_SPEED_UNKNOWN;

		
		if (!(hcd->driver->flags & HCD_USB3)) {
			
			choose_address(udev);
			if (udev->devnum <= 0) {
				status = -ENOTCONN;	
				goto loop;
			}
		}

		
		status = hub_port_init(hub, udev, port1, i);
		if (status < 0)
			goto loop;

		
		if (udev->descriptor.bDeviceClass == USB_CLASS_HUB
				&& udev->bus_mA <= 100) {
			u16	devstat;

			status = usb_get_status(udev, USB_RECIP_DEVICE, 0,
					&devstat);
			if (status < 2) {
				dev_dbg(&udev->dev, "get status %d ?\n", status);
				goto loop_disable;
			}
			le16_to_cpus(&devstat);
			if ((devstat & (1 << USB_DEVICE_SELF_POWERED)) == 0) {
				dev_err(&udev->dev,
					"can't connect bus-powered hub "
					"to this port\n");
				if (hub->has_indicators) {
					hub->indicator[port1-1] =
						INDICATOR_AMBER_BLINK;
					schedule_delayed_work (&hub->leds, 0);
				}
				status = -ENOTCONN;	
				goto loop_disable;
			}
		}
 
		
		if (le16_to_cpu(udev->descriptor.bcdUSB) >= 0x0200
				&& udev->speed == USB_SPEED_FULL
				&& highspeed_hubs != 0)
			check_highspeed (hub, udev, port1);

		
		status = 0;

		
		spin_lock_irq(&device_state_lock);
		if (hdev->state == USB_STATE_NOTATTACHED)
			status = -ENOTCONN;
		else
			hdev->children[port1-1] = udev;
		spin_unlock_irq(&device_state_lock);

		
		if (!status) {
			status = usb_new_device(udev);
			if (status) {
				spin_lock_irq(&device_state_lock);
				hdev->children[port1-1] = NULL;
				spin_unlock_irq(&device_state_lock);
			}
		}

		if (status)
			goto loop_disable;

		status = hub_power_remaining(hub);
		if (status)
			dev_dbg(hub_dev, "%dmA power budget left\n", status);

		return;

loop_disable:
		hub_port_disable(hub, port1, 1);
loop:
		usb_ep0_reinit(udev);
		release_address(udev);
		usb_put_dev(udev);
		if ((status == -ENOTCONN) || (status == -ENOTSUPP))
			break;
	}
	if (hub->hdev->parent ||
			!hcd->driver->port_handed_over ||
			!(hcd->driver->port_handed_over)(hcd, port1))
		dev_err(hub_dev, "unable to enumerate USB device on port %d\n",
				port1);
 
done:
	hub_port_disable(hub, port1, 1);
	if (hcd->driver->relinquish_port && !hub->hdev->parent)
		hcd->driver->relinquish_port(hcd, port1);
}

static void hub_events(void)
{
	struct list_head *tmp;
	struct usb_device *hdev;
	struct usb_interface *intf;
	struct usb_hub *hub;
	struct device *hub_dev;
	u16 hubstatus;
	u16 hubchange;
	u16 portstatus;
	u16 portchange;
	int i, ret;
	int connect_change;

	
	while (1) {

		
		spin_lock_irq(&hub_event_lock);
		if (list_empty(&hub_event_list)) {
			spin_unlock_irq(&hub_event_lock);
			break;
		}

		tmp = hub_event_list.next;
		list_del_init(tmp);

		hub = list_entry(tmp, struct usb_hub, event_list);
		kref_get(&hub->kref);
		spin_unlock_irq(&hub_event_lock);

		hdev = hub->hdev;
		hub_dev = hub->intfdev;
		intf = to_usb_interface(hub_dev);
		dev_dbg(hub_dev, "state %d ports %d chg %04x evt %04x\n",
				hdev->state, hub->descriptor
					? hub->descriptor->bNbrPorts
					: 0,
				
				(u16) hub->change_bits[0],
				(u16) hub->event_bits[0]);

		
		usb_lock_device(hdev);
		if (unlikely(hub->disconnected))
			goto loop;

		
		if (hdev->state == USB_STATE_NOTATTACHED) {
			hub->error = -ENODEV;
			hub_quiesce(hub, HUB_DISCONNECT);
			goto loop;
		}

		
		ret = usb_autopm_get_interface(intf);
		if (ret) {
			dev_dbg(hub_dev, "Can't autoresume: %d\n", ret);
			goto loop;
		}

		
		if (hub->quiescing)
			goto loop_autopm;

		if (hub->error) {
			dev_dbg (hub_dev, "resetting for error %d\n",
				hub->error);

			ret = usb_reset_device(hdev);
			if (ret) {
				dev_dbg (hub_dev,
					"error resetting hub: %d\n", ret);
				goto loop_autopm;
			}

			hub->nerrors = 0;
			hub->error = 0;
		}

		
		for (i = 1; i <= hub->descriptor->bNbrPorts; i++) {
			if (test_bit(i, hub->busy_bits))
				continue;
			connect_change = test_bit(i, hub->change_bits);
			if (!test_and_clear_bit(i, hub->event_bits) &&
					!connect_change)
				continue;

			ret = hub_port_status(hub, i,
					&portstatus, &portchange);
			if (ret < 0)
				continue;

			if (portchange & USB_PORT_STAT_C_CONNECTION) {
				clear_port_feature(hdev, i,
					USB_PORT_FEAT_C_CONNECTION);
				connect_change = 1;
			}

			if (portchange & USB_PORT_STAT_C_ENABLE) {
				if (!connect_change)
					dev_dbg (hub_dev,
						"port %d enable change, "
						"status %08x\n",
						i, portstatus);
				clear_port_feature(hdev, i,
					USB_PORT_FEAT_C_ENABLE);

				
				if (!(portstatus & USB_PORT_STAT_ENABLE)
				    && !connect_change
				    && hdev->children[i-1]) {
					dev_err (hub_dev,
					    "port %i "
					    "disabled by hub (EMI?), "
					    "re-enabling...\n",
						i);
					connect_change = 1;
				}
			}

			if (portchange & USB_PORT_STAT_C_SUSPEND) {
				struct usb_device *udev;

				clear_port_feature(hdev, i,
					USB_PORT_FEAT_C_SUSPEND);
				udev = hdev->children[i-1];
				if (udev) {
					
					msleep(10);

					usb_lock_device(udev);
					ret = remote_wakeup(hdev->
							children[i-1]);
					usb_unlock_device(udev);
					if (ret < 0)
						connect_change = 1;
				} else {
					ret = -ENODEV;
					hub_port_disable(hub, i, 1);
				}
				dev_dbg (hub_dev,
					"resume on port %d, status %d\n",
					i, ret);
			}
			
			if (portchange & USB_PORT_STAT_C_OVERCURRENT) {
				dev_err (hub_dev,
					"over-current change on port %d\n",
					i);
				clear_port_feature(hdev, i,
					USB_PORT_FEAT_C_OVER_CURRENT);
				hub_power_on(hub, true);
			}

			if (portchange & USB_PORT_STAT_C_RESET) {
				dev_dbg (hub_dev,
					"reset change on port %d\n",
					i);
				clear_port_feature(hdev, i,
					USB_PORT_FEAT_C_RESET);
			}

			if (connect_change)
				hub_port_connect_change(hub, i,
						portstatus, portchange);
		} 

		
		if (test_and_clear_bit(0, hub->event_bits) == 0)
			;	
		else if (hub_hub_status(hub, &hubstatus, &hubchange) < 0)
			dev_err (hub_dev, "get_hub_status failed\n");
		else {
			if (hubchange & HUB_CHANGE_LOCAL_POWER) {
				dev_dbg (hub_dev, "power change\n");
				clear_hub_feature(hdev, C_HUB_LOCAL_POWER);
				if (hubstatus & HUB_STATUS_LOCAL_POWER)
					
					hub->limited_power = 1;
				else
					hub->limited_power = 0;
			}
			if (hubchange & HUB_CHANGE_OVERCURRENT) {
				dev_dbg (hub_dev, "overcurrent change\n");
				msleep(500);	
				clear_hub_feature(hdev, C_HUB_OVER_CURRENT);
                        	hub_power_on(hub, true);
			}
		}

loop_autopm:
		
		if (list_empty(&hub->event_list))
			usb_autopm_enable(intf);
loop:
		usb_unlock_device(hdev);
		kref_put(&hub->kref, hub_release);

        } 
}

static int hub_thread(void *__unused)
{
	
	set_freezable();

	do {
		hub_events();
		wait_event_freezable(khubd_wait,
				!list_empty(&hub_event_list) ||
				kthread_should_stop());
	} while (!kthread_should_stop() || !list_empty(&hub_event_list));

	pr_debug("%s: khubd exiting\n", usbcore_name);
	return 0;
}

static struct usb_device_id hub_id_table [] = {
    { .match_flags = USB_DEVICE_ID_MATCH_DEV_CLASS,
      .bDeviceClass = USB_CLASS_HUB},
    { .match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
      .bInterfaceClass = USB_CLASS_HUB},
    { }						
};

MODULE_DEVICE_TABLE (usb, hub_id_table);

static struct usb_driver hub_driver = {
	.name =		"hub",
	.probe =	hub_probe,
	.disconnect =	hub_disconnect,
	.suspend =	hub_suspend,
	.resume =	hub_resume,
	.reset_resume =	hub_reset_resume,
	.pre_reset =	hub_pre_reset,
	.post_reset =	hub_post_reset,
	.ioctl =	hub_ioctl,
	.id_table =	hub_id_table,
	.supports_autosuspend =	1,
};

int usb_hub_init(void)
{
	if (usb_register(&hub_driver) < 0) {
		printk(KERN_ERR "%s: can't register hub driver\n",
			usbcore_name);
		return -1;
	}

	khubd_task = kthread_run(hub_thread, NULL, "khubd");
	if (!IS_ERR(khubd_task))
		return 0;

	
	usb_deregister(&hub_driver);
	printk(KERN_ERR "%s: can't start khubd\n", usbcore_name);

	return -1;
}

void usb_hub_cleanup(void)
{
	kthread_stop(khubd_task);

	
	usb_deregister(&hub_driver);
} 

static int descriptors_changed(struct usb_device *udev,
		struct usb_device_descriptor *old_device_descriptor)
{
	int		changed = 0;
	unsigned	index;
	unsigned	serial_len = 0;
	unsigned	len;
	unsigned	old_length;
	int		length;
	char		*buf;

	if (memcmp(&udev->descriptor, old_device_descriptor,
			sizeof(*old_device_descriptor)) != 0)
		return 1;

	
	if (udev->serial)
		serial_len = strlen(udev->serial) + 1;

	len = serial_len;
	for (index = 0; index < udev->descriptor.bNumConfigurations; index++) {
		old_length = le16_to_cpu(udev->config[index].desc.wTotalLength);
		len = max(len, old_length);
	}

	buf = kmalloc(len, GFP_NOIO);
	if (buf == NULL) {
		dev_err(&udev->dev, "no mem to re-read configs after reset\n");
		
		return 1;
	}
	for (index = 0; index < udev->descriptor.bNumConfigurations; index++) {
		old_length = le16_to_cpu(udev->config[index].desc.wTotalLength);
		length = usb_get_descriptor(udev, USB_DT_CONFIG, index, buf,
				old_length);
		if (length != old_length) {
			dev_dbg(&udev->dev, "config index %d, error %d\n",
					index, length);
			changed = 1;
			break;
		}
		if (memcmp (buf, udev->rawdescriptors[index], old_length)
				!= 0) {
			dev_dbg(&udev->dev, "config index %d changed (#%d)\n",
				index,
				((struct usb_config_descriptor *) buf)->
					bConfigurationValue);
			changed = 1;
			break;
		}
	}

	if (!changed && serial_len) {
		length = usb_string(udev, udev->descriptor.iSerialNumber,
				buf, serial_len);
		if (length + 1 != serial_len) {
			dev_dbg(&udev->dev, "serial string error %d\n",
					length);
			changed = 1;
		} else if (memcmp(buf, udev->serial, length) != 0) {
			dev_dbg(&udev->dev, "serial string changed\n");
			changed = 1;
		}
	}

	kfree(buf);
	return changed;
}


static int usb_reset_and_verify_device(struct usb_device *udev)
{
	struct usb_device		*parent_hdev = udev->parent;
	struct usb_hub			*parent_hub;
	struct usb_device_descriptor	descriptor = udev->descriptor;
	int 				i, ret = 0;
	int				port1 = udev->portnum;

	if (udev->state == USB_STATE_NOTATTACHED ||
			udev->state == USB_STATE_SUSPENDED) {
		dev_dbg(&udev->dev, "device reset not allowed in state %d\n",
				udev->state);
		return -EINVAL;
	}

	if (!parent_hdev) {
		
		dev_dbg(&udev->dev, "%s for root hub!\n", __func__);
		return -EISDIR;
	}
	parent_hub = hdev_to_hub(parent_hdev);

	set_bit(port1, parent_hub->busy_bits);
	for (i = 0; i < SET_CONFIG_TRIES; ++i) {

		
		usb_ep0_reinit(udev);
		ret = hub_port_init(parent_hub, udev, port1, i);
		if (ret >= 0 || ret == -ENOTCONN || ret == -ENODEV)
			break;
	}
	clear_bit(port1, parent_hub->busy_bits);

	if (ret < 0)
		goto re_enumerate;
 
	
	if (descriptors_changed(udev, &descriptor)) {
		dev_info(&udev->dev, "device firmware changed\n");
		udev->descriptor = descriptor;	
		goto re_enumerate;
  	}

	
	if (!udev->actconfig)
		goto done;
	ret = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			USB_REQ_SET_CONFIGURATION, 0,
			udev->actconfig->desc.bConfigurationValue, 0,
			NULL, 0, USB_CTRL_SET_TIMEOUT);
	if (ret < 0) {
		dev_err(&udev->dev,
			"can't restore configuration #%d (error=%d)\n",
			udev->actconfig->desc.bConfigurationValue, ret);
		goto re_enumerate;
  	}
	usb_set_device_state(udev, USB_STATE_CONFIGURED);

	
	for (i = 0; i < udev->actconfig->desc.bNumInterfaces; i++) {
		struct usb_interface *intf = udev->actconfig->interface[i];
		struct usb_interface_descriptor *desc;

		desc = &intf->cur_altsetting->desc;
		if (desc->bAlternateSetting == 0) {
			usb_disable_interface(udev, intf, true);
			usb_enable_interface(udev, intf, true);
			ret = 0;
		} else {
			ret = usb_set_interface(udev, desc->bInterfaceNumber,
					desc->bAlternateSetting);
		}
		if (ret < 0) {
			dev_err(&udev->dev, "failed to restore interface %d "
				"altsetting %d (error=%d)\n",
				desc->bInterfaceNumber,
				desc->bAlternateSetting,
				ret);
			goto re_enumerate;
		}
	}

done:
	return 0;
 
re_enumerate:
	hub_port_logical_disconnect(parent_hub, port1);
	return -ENODEV;
}


int usb_reset_device(struct usb_device *udev)
{
	int ret;
	int i;
	struct usb_host_config *config = udev->actconfig;

	if (udev->state == USB_STATE_NOTATTACHED ||
			udev->state == USB_STATE_SUSPENDED) {
		dev_dbg(&udev->dev, "device reset not allowed in state %d\n",
				udev->state);
		return -EINVAL;
	}

	
	usb_autoresume_device(udev);

	if (config) {
		for (i = 0; i < config->desc.bNumInterfaces; ++i) {
			struct usb_interface *cintf = config->interface[i];
			struct usb_driver *drv;
			int unbind = 0;

			if (cintf->dev.driver) {
				drv = to_usb_driver(cintf->dev.driver);
				if (drv->pre_reset && drv->post_reset)
					unbind = (drv->pre_reset)(cintf);
				else if (cintf->condition ==
						USB_INTERFACE_BOUND)
					unbind = 1;
				if (unbind)
					usb_forced_unbind_intf(cintf);
			}
		}
	}

	ret = usb_reset_and_verify_device(udev);

	if (config) {
		for (i = config->desc.bNumInterfaces - 1; i >= 0; --i) {
			struct usb_interface *cintf = config->interface[i];
			struct usb_driver *drv;
			int rebind = cintf->needs_binding;

			if (!rebind && cintf->dev.driver) {
				drv = to_usb_driver(cintf->dev.driver);
				if (drv->post_reset)
					rebind = (drv->post_reset)(cintf);
				else if (cintf->condition ==
						USB_INTERFACE_BOUND)
					rebind = 1;
			}
			if (ret == 0 && rebind)
				usb_rebind_intf(cintf);
		}
	}

	usb_autosuspend_device(udev);
	return ret;
}
EXPORT_SYMBOL_GPL(usb_reset_device);



void usb_queue_reset_device(struct usb_interface *iface)
{
	schedule_work(&iface->reset_ws);
}
EXPORT_SYMBOL_GPL(usb_queue_reset_device);
