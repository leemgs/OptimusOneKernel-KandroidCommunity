

#ifndef __LINUX_USB_AUDIO_H
#define __LINUX_USB_AUDIO_H

#include <linux/types.h>


#define USB_SUBCLASS_AUDIOCONTROL	0x01
#define USB_SUBCLASS_AUDIOSTREAMING	0x02
#define USB_SUBCLASS_MIDISTREAMING	0x03


#define UAC_HEADER			0x01
#define UAC_INPUT_TERMINAL		0x02
#define UAC_OUTPUT_TERMINAL		0x03
#define UAC_MIXER_UNIT			0x04
#define UAC_SELECTOR_UNIT		0x05
#define UAC_FEATURE_UNIT		0x06
#define UAC_PROCESSING_UNIT		0x07
#define UAC_EXTENSION_UNIT		0x08


#define UAC_AS_GENERAL			0x01
#define UAC_FORMAT_TYPE			0x02
#define UAC_FORMAT_SPECIFIC		0x03


#define UAC_EP_GENERAL			0x01


#define UAC_SET_			0x00
#define UAC_GET_			0x80

#define UAC__CUR			0x1
#define UAC__MIN			0x2
#define UAC__MAX			0x3
#define UAC__RES			0x4
#define UAC__MEM			0x5

#define UAC_SET_CUR			(UAC_SET_ | UAC__CUR)
#define UAC_GET_CUR			(UAC_GET_ | UAC__CUR)
#define UAC_SET_MIN			(UAC_SET_ | UAC__MIN)
#define UAC_GET_MIN			(UAC_GET_ | UAC__MIN)
#define UAC_SET_MAX			(UAC_SET_ | UAC__MAX)
#define UAC_GET_MAX			(UAC_GET_ | UAC__MAX)
#define UAC_SET_RES			(UAC_SET_ | UAC__RES)
#define UAC_GET_RES			(UAC_GET_ | UAC__RES)
#define UAC_SET_MEM			(UAC_SET_ | UAC__MEM)
#define UAC_GET_MEM			(UAC_GET_ | UAC__MEM)

#define UAC_GET_STAT			0xff


#define UAC_MS_HEADER			0x01
#define UAC_MIDI_IN_JACK		0x02
#define UAC_MIDI_OUT_JACK		0x03


#define UAC_MS_GENERAL			0x01


#define UAC_TERMINAL_UNDEFINED		0x100
#define UAC_TERMINAL_STREAMING		0x101
#define UAC_TERMINAL_VENDOR_SPEC	0x1FF



struct uac_ac_header_descriptor {
	__u8  bLength;			
	__u8  bDescriptorType;		
	__u8  bDescriptorSubtype;	
	__le16 bcdADC;			
	__le16 wTotalLength;		
	__u8  bInCollection;		
	__u8  baInterfaceNr[];		
} __attribute__ ((packed));

#define UAC_DT_AC_HEADER_SIZE(n)	(8 + (n))


#define DECLARE_UAC_AC_HEADER_DESCRIPTOR(n) 			\
struct uac_ac_header_descriptor_##n {				\
	__u8  bLength;						\
	__u8  bDescriptorType;					\
	__u8  bDescriptorSubtype;				\
	__le16 bcdADC;						\
	__le16 wTotalLength;					\
	__u8  bInCollection;					\
	__u8  baInterfaceNr[n];					\
} __attribute__ ((packed))


struct uac_input_terminal_descriptor {
	__u8  bLength;			
	__u8  bDescriptorType;		
	__u8  bDescriptorSubtype;	
	__u8  bTerminalID;		
	__le16 wTerminalType;		
	__u8  bAssocTerminal;		
	__u8  bNrChannels;		
	__le16 wChannelConfig;
	__u8  iChannelNames;
	__u8  iTerminal;
} __attribute__ ((packed));

#define UAC_DT_INPUT_TERMINAL_SIZE			12


#define UAC_INPUT_TERMINAL_UNDEFINED			0x200
#define UAC_INPUT_TERMINAL_MICROPHONE			0x201
#define UAC_INPUT_TERMINAL_DESKTOP_MICROPHONE		0x202
#define UAC_INPUT_TERMINAL_PERSONAL_MICROPHONE		0x203
#define UAC_INPUT_TERMINAL_OMNI_DIR_MICROPHONE		0x204
#define UAC_INPUT_TERMINAL_MICROPHONE_ARRAY		0x205
#define UAC_INPUT_TERMINAL_PROC_MICROPHONE_ARRAY	0x206


struct uac_output_terminal_descriptor {
	__u8  bLength;			
	__u8  bDescriptorType;		
	__u8  bDescriptorSubtype;	
	__u8  bTerminalID;		
	__le16 wTerminalType;		
	__u8  bAssocTerminal;		
	__u8  bSourceID;		
	__u8  iTerminal;
} __attribute__ ((packed));

#define UAC_DT_OUTPUT_TERMINAL_SIZE			9


#define UAC_OUTPUT_TERMINAL_UNDEFINED			0x300
#define UAC_OUTPUT_TERMINAL_SPEAKER			0x301
#define UAC_OUTPUT_TERMINAL_HEADPHONES			0x302
#define UAC_OUTPUT_TERMINAL_HEAD_MOUNTED_DISPLAY_AUDIO	0x303
#define UAC_OUTPUT_TERMINAL_DESKTOP_SPEAKER		0x304
#define UAC_OUTPUT_TERMINAL_ROOM_SPEAKER		0x305
#define UAC_OUTPUT_TERMINAL_COMMUNICATION_SPEAKER	0x306
#define UAC_OUTPUT_TERMINAL_LOW_FREQ_EFFECTS_SPEAKER	0x307


#define UAC_DT_FEATURE_UNIT_SIZE(ch)		(7 + ((ch) + 1) * 2)


#define DECLARE_UAC_FEATURE_UNIT_DESCRIPTOR(ch) 		\
struct uac_feature_unit_descriptor_##ch {			\
	__u8  bLength;						\
	__u8  bDescriptorType;					\
	__u8  bDescriptorSubtype;				\
	__u8  bUnitID;						\
	__u8  bSourceID;					\
	__u8  bControlSize;					\
	__le16 bmaControls[ch + 1];				\
	__u8  iFeature;						\
} __attribute__ ((packed))


struct uac_as_header_descriptor {
	__u8  bLength;			
	__u8  bDescriptorType;		
	__u8  bDescriptorSubtype;	
	__u8  bTerminalLink;		
	__u8  bDelay;			
	__le16 wFormatTag;		
} __attribute__ ((packed));

#define UAC_DT_AS_HEADER_SIZE		7


#define UAC_FORMAT_TYPE_I_UNDEFINED	0x0
#define UAC_FORMAT_TYPE_I_PCM		0x1
#define UAC_FORMAT_TYPE_I_PCM8		0x2
#define UAC_FORMAT_TYPE_I_IEEE_FLOAT	0x3
#define UAC_FORMAT_TYPE_I_ALAW		0x4
#define UAC_FORMAT_TYPE_I_MULAW		0x5

struct uac_format_type_i_continuous_descriptor {
	__u8  bLength;			
	__u8  bDescriptorType;		
	__u8  bDescriptorSubtype;	
	__u8  bFormatType;		
	__u8  bNrChannels;		
	__u8  bSubframeSize;		
	__u8  bBitResolution;
	__u8  bSamFreqType;
	__u8  tLowerSamFreq[3];
	__u8  tUpperSamFreq[3];
} __attribute__ ((packed));

#define UAC_FORMAT_TYPE_I_CONTINUOUS_DESC_SIZE	14

struct uac_format_type_i_discrete_descriptor {
	__u8  bLength;			
	__u8  bDescriptorType;		
	__u8  bDescriptorSubtype;	
	__u8  bFormatType;		
	__u8  bNrChannels;		
	__u8  bSubframeSize;		
	__u8  bBitResolution;
	__u8  bSamFreqType;
	__u8  tSamFreq[][3];
} __attribute__ ((packed));

#define DECLARE_UAC_FORMAT_TYPE_I_DISCRETE_DESC(n) 		\
struct uac_format_type_i_discrete_descriptor_##n {		\
	__u8  bLength;						\
	__u8  bDescriptorType;					\
	__u8  bDescriptorSubtype;				\
	__u8  bFormatType;					\
	__u8  bNrChannels;					\
	__u8  bSubframeSize;					\
	__u8  bBitResolution;					\
	__u8  bSamFreqType;					\
	__u8  tSamFreq[n][3];					\
} __attribute__ ((packed))

#define UAC_FORMAT_TYPE_I_DISCRETE_DESC_SIZE(n)	(8 + (n * 3))


#define UAC_FORMAT_TYPE_UNDEFINED	0x0
#define UAC_FORMAT_TYPE_I		0x1
#define UAC_FORMAT_TYPE_II		0x2
#define UAC_FORMAT_TYPE_III		0x3

struct uac_iso_endpoint_descriptor {
	__u8  bLength;			
	__u8  bDescriptorType;		
	__u8  bDescriptorSubtype;	
	__u8  bmAttributes;
	__u8  bLockDelayUnits;
	__le16 wLockDelay;
};
#define UAC_ISO_ENDPOINT_DESC_SIZE	7

#define UAC_EP_CS_ATTR_SAMPLE_RATE	0x01
#define UAC_EP_CS_ATTR_PITCH_CONTROL	0x02
#define UAC_EP_CS_ATTR_FILL_MAX		0x80


#define UAC_FU_CONTROL_UNDEFINED	0x00
#define UAC_MUTE_CONTROL		0x01
#define UAC_VOLUME_CONTROL		0x02
#define UAC_BASS_CONTROL		0x03
#define UAC_MID_CONTROL			0x04
#define UAC_TREBLE_CONTROL		0x05
#define UAC_GRAPHIC_EQUALIZER_CONTROL	0x06
#define UAC_AUTOMATIC_GAIN_CONTROL	0x07
#define UAC_DELAY_CONTROL		0x08
#define UAC_BASS_BOOST_CONTROL		0x09
#define UAC_LOUDNESS_CONTROL		0x0a

#define UAC_FU_MUTE		(1 << (UAC_MUTE_CONTROL - 1))
#define UAC_FU_VOLUME		(1 << (UAC_VOLUME_CONTROL - 1))
#define UAC_FU_BASS		(1 << (UAC_BASS_CONTROL - 1))
#define UAC_FU_MID		(1 << (UAC_MID_CONTROL - 1))
#define UAC_FU_TREBLE		(1 << (UAC_TREBLE_CONTROL - 1))
#define UAC_FU_GRAPHIC_EQ	(1 << (UAC_GRAPHIC_EQUALIZER_CONTROL - 1))
#define UAC_FU_AUTO_GAIN	(1 << (UAC_AUTOMATIC_GAIN_CONTROL - 1))
#define UAC_FU_DELAY		(1 << (UAC_DELAY_CONTROL - 1))
#define UAC_FU_BASS_BOOST	(1 << (UAC_BASS_BOOST_CONTROL - 1))
#define UAC_FU_LOUDNESS		(1 << (UAC_LOUDNESS_CONTROL - 1))

#ifdef __KERNEL__

struct usb_audio_control {
	struct list_head list;
	const char *name;
	u8 type;
	int data[5];
	int (*set)(struct usb_audio_control *con, u8 cmd, int value);
	int (*get)(struct usb_audio_control *con, u8 cmd);
};

struct usb_audio_control_selector {
	struct list_head list;
	struct list_head control;
	u8 id;
	const char *name;
	u8 type;
	struct usb_descriptor_header *desc;
};

#endif 

#endif 
