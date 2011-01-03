
#include "dibusb.h"

static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "set debugging level (rc=1 (or-able))." DVB_USB_DEBUG_STATUS);

DVB_DEFINE_MOD_OPT_ADAPTER_NR(adapter_nr);

#define deb_rc(args...)   dprintk(debug,0x01,args)

static int a800_power_ctrl(struct dvb_usb_device *d, int onoff)
{
	
	return 0;
}


static int a800_identify_state(struct usb_device *udev, struct dvb_usb_device_properties *props,
	struct dvb_usb_device_description **desc, int *cold)
{
	*cold = udev->descriptor.iManufacturer != 1;
	return 0;
}

static struct dvb_usb_rc_key a800_rc_keys[] = {
	{ 0x0201, KEY_PROG1 },       
	{ 0x0200, KEY_POWER },       
	{ 0x0205, KEY_1 },           
	{ 0x0206, KEY_2 },           
	{ 0x0207, KEY_3 },           
	{ 0x0209, KEY_4 },           
	{ 0x020a, KEY_5 },           
	{ 0x020b, KEY_6 },           
	{ 0x020d, KEY_7 },           
	{ 0x020e, KEY_8 },           
	{ 0x020f, KEY_9 },           
	{ 0x0212, KEY_LEFT },        
	{ 0x0211, KEY_0 },           
	{ 0x0213, KEY_RIGHT },       
	{ 0x0217, KEY_PROG2 },       
	{ 0x0210, KEY_PROG3 },       
	{ 0x021e, KEY_VOLUMEDOWN },  
	{ 0x020c, KEY_ZOOM },        
	{ 0x021f, KEY_VOLUMEUP },    
	{ 0x0214, KEY_MUTE },        
	{ 0x0208, KEY_AUDIO },       
	{ 0x0219, KEY_RECORD },      
	{ 0x0218, KEY_PLAY },        
	{ 0x021b, KEY_STOP },        
	{ 0x021a, KEY_PLAYPAUSE },   
	{ 0x021d, KEY_BACK },        
	{ 0x021c, KEY_FORWARD },     
	{ 0x0203, KEY_TEXT },        
	{ 0x0204, KEY_EPG },         
	{ 0x0215, KEY_MENU },        

	{ 0x0303, KEY_CHANNELUP },   
	{ 0x0302, KEY_CHANNELDOWN }, 
	{ 0x0301, KEY_FIRST },       
	{ 0x0300, KEY_LAST },        

};

static int a800_rc_query(struct dvb_usb_device *d, u32 *event, int *state)
{
	u8 key[5];
	if (usb_control_msg(d->udev,usb_rcvctrlpipe(d->udev,0),
				0x04, USB_TYPE_VENDOR | USB_DIR_IN, 0, 0, key, 5,
				2000) != 5)
		return -ENODEV;

	
	dvb_usb_nec_rc_key_to_event(d,key,event,state);
	if (key[0] != 0)
		deb_rc("key: %x %x %x %x %x\n",key[0],key[1],key[2],key[3],key[4]);
	return 0;
}


static struct dvb_usb_device_properties a800_properties;

static int a800_probe(struct usb_interface *intf,
		const struct usb_device_id *id)
{
	return dvb_usb_device_init(intf, &a800_properties,
				   THIS_MODULE, NULL, adapter_nr);
}


static struct usb_device_id a800_table [] = {
	{ USB_DEVICE(USB_VID_AVERMEDIA,     USB_PID_AVERMEDIA_DVBT_USB2_COLD) },
	{ USB_DEVICE(USB_VID_AVERMEDIA,     USB_PID_AVERMEDIA_DVBT_USB2_WARM) },
			{ }		
};
MODULE_DEVICE_TABLE (usb, a800_table);

static struct dvb_usb_device_properties a800_properties = {
	.caps = DVB_USB_IS_AN_I2C_ADAPTER,

	.usb_ctrl = CYPRESS_FX2,
	.firmware = "dvb-usb-avertv-a800-02.fw",

	.num_adapters = 1,
	.adapter = {
		{
			.caps = DVB_USB_ADAP_HAS_PID_FILTER | DVB_USB_ADAP_PID_FILTER_CAN_BE_TURNED_OFF,
			.pid_filter_count = 32,
			.streaming_ctrl   = dibusb2_0_streaming_ctrl,
			.pid_filter       = dibusb_pid_filter,
			.pid_filter_ctrl  = dibusb_pid_filter_ctrl,

			.frontend_attach  = dibusb_dib3000mc_frontend_attach,
			.tuner_attach     = dibusb_dib3000mc_tuner_attach,

			
					.stream = {
						.type = USB_BULK,
				.count = 7,
				.endpoint = 0x06,
				.u = {
					.bulk = {
						.buffersize = 4096,
					}
				}
			},

			.size_of_priv     = sizeof(struct dibusb_state),
		},
	},

	.power_ctrl       = a800_power_ctrl,
	.identify_state   = a800_identify_state,

	.rc_interval      = DEFAULT_RC_INTERVAL,
	.rc_key_map       = a800_rc_keys,
	.rc_key_map_size  = ARRAY_SIZE(a800_rc_keys),
	.rc_query         = a800_rc_query,

	.i2c_algo         = &dibusb_i2c_algo,

	.generic_bulk_ctrl_endpoint = 0x01,
	.num_device_descs = 1,
	.devices = {
		{   "AVerMedia AverTV DVB-T USB 2.0 (A800)",
			{ &a800_table[0], NULL },
			{ &a800_table[1], NULL },
		},
	}
};

static struct usb_driver a800_driver = {
	.name		= "dvb_usb_a800",
	.probe		= a800_probe,
	.disconnect = dvb_usb_device_exit,
	.id_table	= a800_table,
};


static int __init a800_module_init(void)
{
	int result;
	if ((result = usb_register(&a800_driver))) {
		err("usb_register failed. Error number %d",result);
		return result;
	}

	return 0;
}

static void __exit a800_module_exit(void)
{
	
	usb_deregister(&a800_driver);
}

module_init (a800_module_init);
module_exit (a800_module_exit);

MODULE_AUTHOR("Patrick Boettcher <patrick.boettcher@desy.de>");
MODULE_DESCRIPTION("AVerMedia AverTV DVB-T USB 2.0 (A800)");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
