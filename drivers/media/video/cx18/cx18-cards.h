


#define CX18_HW_TUNER			(1 << 0)
#define CX18_HW_TVEEPROM		(1 << 1)
#define CX18_HW_CS5345			(1 << 2)
#define CX18_HW_DVB			(1 << 3)
#define CX18_HW_418_AV			(1 << 4)
#define CX18_HW_GPIO_MUX		(1 << 5)
#define CX18_HW_GPIO_RESET_CTRL		(1 << 6)
#define CX18_HW_Z8F0811_IR_TX_HAUP	(1 << 7)
#define CX18_HW_Z8F0811_IR_RX_HAUP	(1 << 8)
#define CX18_HW_Z8F0811_IR_HAUP	(CX18_HW_Z8F0811_IR_RX_HAUP | \
				 CX18_HW_Z8F0811_IR_TX_HAUP)


#define	CX18_CARD_INPUT_VID_TUNER	1
#define	CX18_CARD_INPUT_SVIDEO1 	2
#define	CX18_CARD_INPUT_SVIDEO2 	3
#define	CX18_CARD_INPUT_COMPOSITE1 	4
#define	CX18_CARD_INPUT_COMPOSITE2 	5
#define	CX18_CARD_INPUT_COMPOSITE3 	6


#define	CX18_CARD_INPUT_AUD_TUNER	1
#define	CX18_CARD_INPUT_LINE_IN1 	2
#define	CX18_CARD_INPUT_LINE_IN2 	3

#define CX18_CARD_MAX_VIDEO_INPUTS 6
#define CX18_CARD_MAX_AUDIO_INPUTS 3
#define CX18_CARD_MAX_TUNERS  	   2


#define CX18_CAP_ENCODER (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_TUNER | \
			  V4L2_CAP_AUDIO | V4L2_CAP_READWRITE | \
			  V4L2_CAP_VBI_CAPTURE | V4L2_CAP_SLICED_VBI_CAPTURE)

struct cx18_card_video_input {
	u8  video_type; 	
	u8  audio_index;	
	u16 video_input;	
};

struct cx18_card_audio_input {
	u8  audio_type;		
	u32 audio_input;	
	u16 muxer_input;	
};

struct cx18_card_pci_info {
	u16 device;
	u16 subsystem_vendor;
	u16 subsystem_device;
};





struct cx18_gpio_init { 
	u32 direction; 	
	u32 initial_value;
};

struct cx18_gpio_i2c_slave_reset {
	u32 active_lo_mask; 
	u32 active_hi_mask; 
	int msecs_asserted; 
	int msecs_recovery; 
	u32 ir_reset_mask;  
};

struct cx18_gpio_audio_input { 	
	u32 mask; 		
	u32 tuner;
	u32 linein;
	u32 radio;
};

struct cx18_card_tuner {
	v4l2_std_id std; 	
	int 	    tuner; 	
};

struct cx18_card_tuner_i2c {
	unsigned short radio[2];
	unsigned short demod[2];
	unsigned short tv[4];	
};

struct cx18_ddr {		
	u32 chip_config;
	u32 refresh;
	u32 timing1;
	u32 timing2;
	u32 tune_lane;
	u32 initial_emrs;
};


struct cx18_card {
	int type;
	char *name;
	char *comment;
	u32 v4l2_capabilities;
	u32 hw_audio_ctrl;	
	u32 hw_muxer;		
	u32 hw_all;		
	struct cx18_card_video_input video_inputs[CX18_CARD_MAX_VIDEO_INPUTS];
	struct cx18_card_audio_input audio_inputs[CX18_CARD_MAX_AUDIO_INPUTS];
	struct cx18_card_audio_input radio_input;

	
	u8 xceive_pin; 		
	struct cx18_gpio_init 		 gpio_init;
	struct cx18_gpio_i2c_slave_reset gpio_i2c_slave_reset;
	struct cx18_gpio_audio_input    gpio_audio_input;

	struct cx18_card_tuner tuners[CX18_CARD_MAX_TUNERS];
	struct cx18_card_tuner_i2c *i2c;

	struct cx18_ddr ddr;

	
	const struct cx18_card_pci_info *pci_list;
};

int cx18_get_input(struct cx18 *cx, u16 index, struct v4l2_input *input);
int cx18_get_audio_input(struct cx18 *cx, u16 index, struct v4l2_audio *input);
const struct cx18_card *cx18_get_card(u16 index);
